import base64
import hashlib
import json
import struct
import sys

import pytest

from match_recovery.application_binary import select_bindings, select_fragment_bindings
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Result, Span
from match_recovery.partition import Candidate, partition
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.tasks import Task, TaskStore
from test_application_commands import project as _application_project
from test_binary import coff_fixture, pe_fixture


def _candidate(pin, address, unit, symbol="_f", **kwargs):
    return Candidate(Origin(pin, "pe:va", address), (Span(address, address + 1),),
                     unit, 1, kwargs.pop("seed", False), symbol, **kwargs)


def _select(candidates, selected=None, component_targets=None):
    view = partition(candidates)
    entities = {entity["entity_id"] for group in view["groups"] for entity in group["entities"]}
    bindings, refused, externals = select_bindings(
        view, frozenset(entities if selected is None else selected), component_targets)
    return bindings, refused, externals, entities


def _section(name, data, relocations):
    return (name.encode().ljust(8, b"\0")
            + struct.pack("<IIIIIIHHI", 0, 0, len(data), 0,
                          relocations, 0, int(bool(relocations)), 0, 0x60501020))


def _call_object(caller, *, define_local=True):
    """Two physical code COMDATs, with a local function call when defined."""
    caller_code = b"\xe8\0\0\0\0\xc3" + b"\xcc" * 10
    local_code = b"\xc3" + b"\xcc" * 15
    if define_local:
        sections = [_section(".text$A", caller_code, 132),
                    _section(".text$B", local_code, 0)]
        section_count = 2
        symbol_rows = (
            caller.encode().ljust(8, b"\0") + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
            + b"_f".ljust(8, b"\0") + struct.pack("<IhHBB", 0, 2, 0x20, 3, 0)
        )
        relocations = struct.pack("<IIH", 1, 1, 20)
        raw = caller_code + local_code
    else:
        section_count = 1
        sections = [_section(".text$A", caller_code, 116)]
        symbol_rows = (
            caller.encode().ljust(8, b"\0") + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
            + b"_f".ljust(8, b"\0") + struct.pack("<IhHBB", 0, 0, 0, 2, 0)
        )
        relocations = struct.pack("<IIH", 1, 1, 20)
        raw = caller_code
    section_table = b"".join(sections)
    raw_offsets = 20 + 40 * section_count
    section_table = bytearray(section_table)
    struct.pack_into("<I", section_table, 20, raw_offsets)
    if define_local:
        struct.pack_into("<I", section_table, 40 + 20, raw_offsets + 16)
        reloc_at = raw_offsets + len(raw)
        struct.pack_into("<I", section_table, 24, reloc_at)
        symbol_at = reloc_at + len(relocations)
    else:
        reloc_at = raw_offsets + len(raw)
        struct.pack_into("<I", section_table, 24, reloc_at)
        symbol_at = reloc_at + len(relocations)
    header = struct.pack("<HHIIIHH", 0x14C, section_count, 0, symbol_at, 2, 0, 0)
    return Coff(header + bytes(section_table) + raw + relocations + symbol_rows + struct.pack("<I", 4))


def _bind(pin, address, symbol, unit, *, local_only=False, size=1):
    return FunctionBinding(Origin(pin, "pe:va", address), (Span(address, address + size),),
                           symbol, unit, local_only=local_only)


def test_cross_tu_names_are_tentative_local_bindings_not_external_targets():
    image_data = pe_fixture()
    pin = hashlib.sha256(image_data).hexdigest()
    first = _candidate(pin, 0x401000, "a.cc")
    second = _candidate(pin, 0x401020, "b.cc")
    bindings, refused, externals, _ = _select([first, second])

    assert not refused
    assert {(item.unit, item.symbol, item.local_only) for item in bindings} == {
        ("a.cc", "_f", True), ("b.cc", "_f", True)}
    assert "_f" not in externals


def test_unambiguous_external_binding_keeps_existing_global_behavior():
    image_data = pe_fixture()
    pin = hashlib.sha256(image_data).hexdigest()
    bindings, refused, externals, _ = _select([_candidate(pin, 0x401000, "a.cc")])
    assert not refused
    assert len(bindings) == 1 and bindings[0].local_only is False
    assert externals == {"_f": 0x401000}


@pytest.mark.parametrize("candidate", [
    lambda pin: _candidate(pin, 0x401020, "b.cc", component="runtime"),
    lambda pin: _candidate(pin, 0x401020, "b.cc", context_path="host.cc"),
    lambda pin: _candidate(pin, 0x401020, "b.cc", function_fragment={
        "parent_address": 0x401100, "parent_symbol": "_parent", "reason": "control", "evidence": "synthetic"}),
])
def test_nonordinary_names_do_not_gain_local_or_global_qualification(candidate):
    image_data = pe_fixture()
    pin = hashlib.sha256(image_data).hexdigest()
    left = _candidate(pin, 0x401000, "a.cc")
    bindings, refused, externals, _ = _select([left, candidate(pin)])

    assert not any(item.local_only for item in bindings)
    assert "_f" not in externals
    assert refused


def test_same_unit_duplicate_originals_remain_ambiguous():
    image_data = pe_fixture()
    pin = hashlib.sha256(image_data).hexdigest()
    bindings, refused, externals, _ = _select([
        _candidate(pin, 0x401000, "a.cc"),
        _candidate(pin, 0x401020, "a.cc"),
    ])
    assert not bindings
    assert "_f" not in externals
    assert len(refused) == 2


def test_conflicted_original_and_unbound_interpretation_block_local_binding():
    image_data = pe_fixture()
    pin = hashlib.sha256(image_data).hexdigest()
    conflict = [
        _candidate(pin, 0x401020, "b.cc"),
        _candidate(pin, 0x401000, "a.cc"),
        _candidate(pin, 0x401000, "alternate.cc"),
    ]
    bindings, refused, externals, _ = _select(conflict)
    assert not any(item.local_only for item in bindings)
    assert "_f" not in externals
    assert refused

    unbound = Candidate(Origin(pin, "pe:va", 0x401000), (), "a.cc", 1, False, "",
                        unbound=True)
    bindings, refused, externals, _ = _select([
        unbound, _candidate(pin, 0x401000, "a.cc"), _candidate(pin, 0x401020, "b.cc")])
    assert not any(item.local_only for item in bindings)
    assert "_f" not in externals
    assert refused


def test_seed_is_not_selected_as_a_local_function_candidate():
    image_data = pe_fixture()
    pin = hashlib.sha256(image_data).hexdigest()
    seed = _candidate(pin, 0x401040, "seed.cc", seed=True)
    bindings, refused, externals, entities = _select([
        _candidate(pin, 0x401000, "a.cc"), _candidate(pin, 0x401020, "b.cc"), seed])
    assert len([item for item in bindings if item.local_only]) == 2
    assert "_f" not in externals
    assert any(row["entity_id"] in entities and row.get("reason", "").find("unambiguous") >= 0
               for row in refused)


def test_verified_component_name_blocks_local_tentative_binding():
    image_data = pe_fixture()
    pin = hashlib.sha256(image_data).hexdigest()
    bindings, refused, externals, _ = _select([
        _candidate(pin, 0x401000, "a.cc"), _candidate(pin, 0x401020, "b.cc")],
        component_targets={"_f": 0x401100})
    assert not any(item.local_only for item in bindings)
    assert "_f" not in externals
    assert refused


def test_fragment_support_parent_retains_tu_local_qualification():
    image = pe_fixture()
    pin = hashlib.sha256(image).hexdigest()
    parent = Candidate(Origin(pin, "pe:va", 0x401000), (Span(0x401000, 0x401010),),
                       "a.cc", 1, False, "_f")
    child = Candidate(Origin(pin, "pe:va", 0x401010), (Span(0x401010, 0x401011),),
                      "a.cc", 2, False, "_child", function_fragment={
                          "parent_address": 0x401000, "parent_symbol": "_f",
                          "reason": "mapped adjacent extent", "evidence": "synthetic"})
    peer = _candidate(pin, 0x401020, "b.cc")
    view = partition([parent, child, peer])
    entities = {entity["origin"]["address"]: entity["entity_id"]
                for group in view["groups"] for entity in group["entities"]}
    bindings, _refused, _externals = select_bindings(
        view, frozenset({entities[0x401000], entities[0x401010]}))
    expanded, _fragments, support, refused = select_fragment_bindings(
        view, frozenset({entities[0x401010]}), bindings)
    parent_binding, = [binding for binding in expanded if binding.origin.address == 0x401000]
    assert parent_binding.local_only is True
    assert entities[0x401000] not in support
    assert not refused


@pytest.mark.parametrize("peer_unit", ["b.cc", "a.cc"])
def test_fragment_child_only_does_not_synthesize_ambiguous_support_parent(peer_unit):
    raw_image = pe_fixture()
    pin = hashlib.sha256(raw_image).hexdigest()
    parent = Candidate(Origin(pin, "pe:va", 0x401000), (Span(0x401000, 0x401010),),
                       "a.cc", 1, False, "_f")
    child = Candidate(Origin(pin, "pe:va", 0x401010), (Span(0x401010, 0x401011),),
                      "a.cc", 2, False, "_child", function_fragment={
                          "parent_address": 0x401000, "parent_symbol": "_f",
                          "reason": "mapped adjacent extent", "evidence": "synthetic"})
    peer = _candidate(pin, 0x401020, peer_unit)
    view = partition([parent, child, peer])
    entities = {entity["origin"]["address"]: entity["entity_id"]
                for group in view["groups"] for entity in group["entities"]}
    # The component target is intentionally not available to the fragment
    # support selector. The duplicate source name must fail closed regardless.
    bindings, _refused, _externals = select_bindings(
        view, frozenset({entities[0x401010]}), {"_f": 0x401100})
    expanded, _fragments, support, refused = select_fragment_bindings(
        view, frozenset({entities[0x401010]}), bindings)
    assert not any(binding.origin.address == 0x401000 for binding in expanded)
    assert entities[0x401000] not in support
    assert any(row["entity_id"] == entities[0x401010]
               and row["result"] == Result.UNKNOWN for row in refused)


def test_local_only_check_requires_fresh_static_coff_and_blocks_global_leakage():
    raw_image = pe_fixture()
    pin = hashlib.sha256(raw_image).hexdigest()
    image = PeImage(raw_image, pin)
    local = _bind(pin, 0x401000, "_f", "unit.obj", local_only=True)
    ordinary = Coff(coff_fixture(storage=2))
    row, = check_objects([local], {"unit.obj": ordinary}, image)
    assert row["result"] == Result.UNKNOWN
    assert row["local_only"] is True
    assert "static function definition" in row["reason"]
    with pytest.raises(ValueError, match="global target binding"):
        check_objects([local], {"unit.obj": ordinary}, image, external_targets={"_f": 0x401000})
    with pytest.raises(ValueError, match="global target binding"):
        check_objects([local], {"unit.obj": ordinary}, image, import_targets={"_f": 0x401000})
    row, = check_objects([local], {}, image)
    assert row["result"] == Result.UNKNOWN and "not compiled" in row["reason"]
    row, = check_objects([local], {"unit.obj": Coff(coff_fixture(storage=2))}, image)
    assert row["result"] == Result.UNKNOWN and "static function definition" in row["reason"]
    external = _bind(pin, 0x401020, "_f", "another.obj")
    with pytest.raises(ValueError, match="global target binding"):
        check_objects([local, external], {"unit.obj": ordinary,
                                          "another.obj": Coff(coff_fixture(storage=2))}, image)
    with pytest.raises(ValueError, match="must be boolean"):
        FunctionBinding(local.origin, local.spans, local.symbol, local.unit, local_only=1)


def test_static_local_relocations_resolve_per_tu_without_cross_tu_leakage():
    image_data = bytearray(pe_fixture())
    image_data[512:518] = b"\xe8\x1b\0\0\0\xc3"
    image_data[512 + 0x20] = 0xC3
    image_data[512 + 0x40:512 + 0x46] = b"\xe8\x1b\0\0\0\xc3"
    image_data[512 + 0x60] = 0xC3
    image_data[512 + 0x80:512 + 0x86] = b"\xe8\x9b\xff\xff\xff\xc3"
    image = bytes(image_data)
    pin = hashlib.sha256(image).hexdigest()
    pe = PeImage(image, pin)
    objects = {"a.cc": _call_object("_callerA"), "b.cc": _call_object("_callerB"),
               "external.cc": _call_object("_cons", define_local=False)}
    bindings = [
        _bind(pin, 0x401000, "_callerA", "a.cc", size=6),
        _bind(pin, 0x401020, "_f", "a.cc", local_only=True),
        _bind(pin, 0x401040, "_callerB", "b.cc", size=6),
        _bind(pin, 0x401060, "_f", "b.cc", local_only=True),
        _bind(pin, 0x401080, "_cons", "external.cc", size=6),
    ]
    rows = check_objects(bindings, objects, pe)
    by_unit_symbol = {(row["unit"], row["symbol"]): row for row in rows}
    assert by_unit_symbol[("a.cc", "_callerA")]["result"] == Result.PASS
    assert by_unit_symbol[("b.cc", "_callerB")]["result"] == Result.PASS
    assert by_unit_symbol[("a.cc", "_f")]["result"] == Result.PASS
    assert by_unit_symbol[("b.cc", "_f")]["result"] == Result.PASS
    external = by_unit_symbol[("external.cc", "_cons")]
    assert external["result"] == Result.UNKNOWN
    assert external["references"] == Result.UNKNOWN
    assert "_f" not in {key for key in by_unit_symbol}


def test_local_storage_mixed_across_tus_does_not_promote_external_definition():
    image_data = bytearray(pe_fixture())
    image_data[512 + 0x20] = 0xC3
    image = bytes(image_data)
    pin = hashlib.sha256(image).hexdigest()
    pe = PeImage(image, pin)
    bindings = [_bind(pin, 0x401000, "_f", "external.obj", local_only=True),
                _bind(pin, 0x401020, "_f", "static.obj", local_only=True)]
    rows = check_objects(bindings, {"external.obj": Coff(coff_fixture(storage=2)),
                                    "static.obj": Coff(coff_fixture(storage=3))}, pe)
    assert [row["result"] for row in rows] == [Result.UNKNOWN, Result.PASS]
    assert "static function definition" in rows[0]["reason"]


@pytest.fixture
def project(tmp_path):
    from test_application_commands import project as fixture

    # Call the fixture body directly: this stays a synthetic host-only project.
    base = fixture.__wrapped__(tmp_path)
    return base


def test_task_schedules_each_tentative_local_tu_with_fake_compiler(project, capsys):
    image = bytearray(pe_fixture())
    image[512 + 0x20] = 0xC3
    image_path = project / "original.exe"
    image_path.write_bytes(image)
    pin = hashlib.sha256(image).hexdigest()
    (project / "code/PIN").write_text("sha256 " + pin + "\n")
    first = project / "code/unit.cc"
    first.write_text('// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\nstatic void f() {}\n')
    second = project / "code/second.cc"
    second.write_text('// ORIGINAL: 0x00401020 _f 0x00401020-0x00401021\nstatic void f() {}\n')
    TaskStore(project).create(Task("local-owner", "unregistered-game", "coordinator", "local functions",
                                  "b" * 40, (Origin(pin, "pe:va", 0x401000), Origin(pin, "pe:va", 0x401020)),
                                  ("code/unit.cc", "code/second.cc"), ("two static source names",)))

    object_bytes = coff_fixture(storage=3)
    calls_file = project / "compiler-calls.json"
    script = project / "recovery/compile.py"
    script.write_text(
        "import base64,json,sys\n"
        "request=json.load(sys.stdin)\n"
        f"path={str(calls_file)!r}\n"
        "rows=json.load(open(path)) if __import__('os').path.exists(path) else []\n"
        "rows.append(request['source'])\n"
        "json.dump(rows,open(path,'w'))\n"
        f"payload={base64.b64encode(object_bytes).decode()!r}\n"
        "print(json.dumps({'object_base64':payload}))\n")

    from match_recovery.cli import main

    assert main(["tasks", "check-binary", "local-owner", "--root", str(project),
                 "--image", "original.exe"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["result"] == Result.PASS
    assert set(report["translation_units"]) == {"code/unit.cc", "code/second.cc"}
    assert json.loads(calls_file.read_text()) == ["code/second.cc", "code/unit.cc"]
    function_rows = report["functions"]
    assert {row["unit"] for row in function_rows} == {"code/unit.cc", "code/second.cc"}
    assert all(variant.get("local_only") is True
               for row in function_rows for variant in row["variants"])
