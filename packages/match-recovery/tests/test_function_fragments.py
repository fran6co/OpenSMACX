"""Source-owned non-callable fragments and fail-closed measurement controls."""

import sys
import base64
import struct
from copy import deepcopy
from dataclasses import replace
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[3]

from match_recovery.source import read_text, writer  # noqa: E402
from match_recovery.source.model import FunctionFragmentRelation  # noqa: E402
from match_recovery.source.asm import (AsmComparison, compare_unit,
                                      resolve_fragment_pairs)  # noqa: E402
from match_recovery.source.recording import record_claims  # noqa: E402
from match_recovery.source_measure import measure_source  # noqa: E402
from match_recovery.bound_objects import (FunctionBinding, FunctionFragmentBinding,
                                          check_objects)  # noqa: E402
from match_recovery.application_binary import (select_bindings,
                                               select_fragment_bindings,
                                               check_compiled,
                                               verify_task)  # noqa: E402
from match_recovery.restoration import _binary_outcome  # noqa: E402
from match_recovery.model import Origin, Span  # noqa: E402
from match_recovery.component_original_routing import (  # noqa: E402
    component_record_identity_candidates, exact_component_record_bindings)
from match_recovery.component_source import render_component  # noqa: E402
from match_recovery.components import ToolchainBinding, ToolchainManifest  # noqa: E402
from match_recovery.model import Component, Delivery, Ownership  # noqa: E402
from match_recovery.pe_coff import Coff, PeImage  # noqa: E402
from match_recovery.tasks import Task, TaskStore  # noqa: E402
from match_recovery.ownership_source import project_view  # noqa: E402
from match_recovery.restoration import measure_restoration  # noqa: E402
sys.path.insert(0, str(ROOT / "packages/match-recovery/tests"))
from test_binary import coff_fixture, pe_fixture  # noqa: E402
from test_application_commands import project as application_project  # noqa: E402
from test_claim_integrity import git, commit, configuration  # noqa: E402
from test_match_check import database as configure_database  # noqa: E402
from test_recording import compiler as configure_compiler, project as match_base_project  # noqa: E402
from match_recovery.match_check import check_matches  # noqa: E402


PARENT = 0x00401000
CHILD = 0x00401001
SYMBOL = "_parent"
UNIT = Path("src/fixture.cpp")


def source(*, relation=True):
    text = (
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401001 FILE\n"
        "// symbol    _parent\n"
        "// ORIGINAL: 0x00401001 tail 0x00401001-0x00401002 FILE BYTE_EXACT\n"
    )
    if relation:
        text += ('// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
                 '"parent_symbol":"_parent","reason":"terminal tail is owned by parent",'
                 '"evidence":["pinned:adjacent-tail"]}\n')
    return text


def test_reader_retains_distinct_child_identity_claim_and_typed_parent_relation():
    rows = read_text(source(), UNIT)
    assert len(rows) == 2
    parent, child = rows
    assert (child.address, child.image_spans, child.byte_exact) == (
        CHILD, ((CHILD, CHILD + 1),), True)
    assert child.function_fragment == FunctionFragmentRelation(
        PARENT, SYMBOL, "terminal tail is owned by parent", ("pinned:adjacent-tail",))
    assert parent.function_fragment is None


@pytest.mark.parametrize("bad", [
    '{"parent_address":"0x00402000","parent_symbol":"_parent",'
    '"reason":"x","evidence":[],"extra":1}',
    '{"parent_address":"0x00401000","parent_symbol":"_other",'
    '"reason":"x","evidence":["e"]}',
    '{"parent_address":"0x00401000","parent_symbol":"_parent",'
    '"reason":"x","evidence":[]}',
    '{"parent_address":"0x00401000","parent_address":"0x00401000",'
    '"parent_symbol":"_parent","reason":"x","evidence":["e"]}',
    '{"parent_address":"0x00401000","parent_symbol":"_parent",'
    '"reason":"x","evidence":[NaN]}',
])
def test_reader_rejects_malformed_or_unproven_relations(bad):
    text = source(relation=False) + "// RECOVERY_FRAGMENT: v1 " + bad + "\n"
    # The final case has a mismatched parent symbol; the first is extra data;
    # the middle has an empty evidence list.
    with pytest.raises(ValueError):
        read_text(text, UNIT)


def test_reader_rejects_duplicate_parent_address_candidates():
    text = (
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401001 FILE\n"
        "// symbol _parent\nvoid parent() {}\n"
        "// ORIGINAL: 0x00401000 duplicate 0x00401000-0x00401001 FILE\n"
        "// symbol _other\nvoid other() {}\n"
        "// ORIGINAL: 0x00401001 tail 0x00401001-0x00401002 FILE BYTE_EXACT\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
        '"parent_symbol":"_parent","reason":"tail",'
        '"evidence":["fixture"]}\n')
    with pytest.raises(ValueError, match="one earlier non-fragment FILE mapping"):
        read_text(text, UNIT)


def test_reader_rejects_fragment_identity_not_at_its_slice_start():
    text = (
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401001 FILE\n"
        "// symbol _parent\nvoid parent() {}\n"
        "// ORIGINAL: 0x00401001 tail 0x00401002-0x00401003 FILE BYTE_EXACT\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
        '"parent_symbol":"_parent","reason":"tail",'
        '"evidence":["fixture"]}\n')
    with pytest.raises(ValueError, match="adjacent extent"):
        read_text(text, UNIT)


def test_relation_requires_immediate_following_child_marker():
    text = source(relation=False)
    text += "// intervening\n"
    text += ('// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
             '"parent_symbol":"_parent","reason":"tail", "evidence":["e"]}\n')
    with pytest.raises(ValueError, match="immediately follow"):
        read_text(text, UNIT)


def test_reader_rejects_fragment_parent_chains():
    text = (
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401001 FILE\n"
        "// symbol _parent\n"
        "// ORIGINAL: 0x00401001 child 0x00401001-0x00401002 FILE\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
        '"parent_symbol":"_parent","reason":"tail", "evidence":["e"]}\n'
        "// ORIGINAL: 0x00401002 grandchild 0x00401002-0x00401003 FILE\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401001",'
        '"parent_symbol":"child","reason":"nested tail", "evidence":["e"]}\n'
    )
    with pytest.raises(ValueError, match="non-fragment FILE mapping"):
        read_text(text, UNIT)


def test_writer_roundtrips_relation_and_removal_preserves_parent():
    text = source()
    parent, child = read_text(text, UNIT)
    rewritten = writer.write(text, [parent, child])
    rows = read_text(rewritten, UNIT)
    assert rows[1].function_fragment == child.function_fragment
    removed_child = writer.remove(rewritten, [rows[1]])
    remaining = read_text(removed_child, UNIT)
    assert [row.address for row in remaining] == [PARENT]


def test_writer_refuses_removing_parent_before_child_relation():
    rows = read_text(source(), UNIT)
    with pytest.raises(ValueError, match="fragment parent"):
        writer.remove(source(), [rows[0]])


def test_writer_refuses_silently_dropping_relation_while_child_identity_remains():
    text = source()
    parent, child = read_text(text, UNIT)
    child.function_fragment = None
    with pytest.raises(ValueError, match="drop a fragment relation"):
        writer.write(text, [parent, child])


def fragment_fixture(candidate=b"\x90\xc3"):
    raw = pe_fixture(b"\x90\xc3")
    import hashlib
    pin = hashlib.sha256(raw).hexdigest()
    image = PeImage(raw, pin)
    parent = FunctionBinding(Origin(pin, "pe:va", PARENT),
                             (Span(PARENT, CHILD + 1),), "_f", UNIT.as_posix(),
                             source_spans=(Span(PARENT, CHILD),))
    child = FunctionFragmentBinding(Origin(pin, "pe:va", CHILD),
                                    Span(CHILD, CHILD + 1),
                                    parent.origin.entity_id, "_f",
                                    Span(PARENT, CHILD), UNIT.as_posix())
    return parent, child, {UNIT.as_posix(): Coff(coff_fixture(candidate))}, image


def test_fragment_is_measured_only_inside_complete_parent_section():
    parent, child, objects, image = fragment_fixture()
    rows = check_objects([parent], objects, image, fragments=[child])
    by_id = {row["entity_id"]: row for row in rows}
    parent_row, child_row = by_id[parent.origin.entity_id], by_id[child.origin.entity_id]
    assert parent_row["result"] == "pass"
    assert child_row["fragment_result"] == "pass"
    assert child_row["parent_whole_body_result"] == "pass"
    assert child_row["result"] == "pass"
    assert child_row["callable"] is child_row["link_target"] is False


def test_application_compiled_path_carries_fragment_to_object_checker():
    parent, child, objects, image = fragment_fixture()
    rows = check_compiled([parent], [], [], objects, image, {}, fragments=[child])[0]
    child_row = next(row for row in rows if row["entity_id"] == child.origin.entity_id)
    assert child_row["result"] == "pass"
    assert child_row["parent_whole_body_result"] == "pass"


def test_parent_mismatch_cannot_be_hidden_by_passing_child_slice():
    parent, child, objects, image = fragment_fixture(b"\x91\xc3")
    rows = check_objects([parent], objects, image, fragments=[child])
    child_row = next(row for row in rows if row["entity_id"] == child.origin.entity_id)
    assert child_row["fragment_result"] == "pass"
    assert child_row["parent_whole_body_result"] == "fail"
    assert child_row["result"] == "unknown"


def test_fragment_with_changed_section_extent_stays_unknown():
    parent, child, objects, image = fragment_fixture(b"\x90\x90\xc3")
    rows = check_objects([parent], objects, image, fragments=[child])
    child_row = next(row for row in rows if row["entity_id"] == child.origin.entity_id)
    assert child_row["parent_whole_body_result"] == "fail"
    assert child_row["result"] == "unknown"


def test_relocation_crossing_fragment_boundary_is_unknown_even_when_parent_passes():
    import hashlib
    target = 0x00401020
    original = b"\x90\x90" + target.to_bytes(4, "little") + b"\xc3\xc3"
    candidate = b"\x90\x90" + bytes(4) + b"\xc3\xc3"
    raw = pe_fixture(original, absolute_site=2)
    pin = hashlib.sha256(raw).hexdigest()
    image = PeImage(raw, pin)
    parent = FunctionBinding(Origin(pin, "pe:va", PARENT),
                             (Span(PARENT, PARENT + 8),), "_f", UNIT.as_posix(),
                             source_spans=(Span(PARENT, PARENT + 4),))
    child = FunctionFragmentBinding(Origin(pin, "pe:va", PARENT + 4),
                                    Span(PARENT + 4, PARENT + 8),
                                    parent.origin.entity_id, "_f",
                                    Span(PARENT, PARENT + 4), UNIT.as_posix())
    obj = Coff(coff_fixture(candidate, relocation=(2, 1, 6)))
    rows = check_objects([parent], {UNIT.as_posix(): obj}, image,
                         external_targets={"_g": target}, fragments=[child])
    by_id = {row["entity_id"]: row for row in rows}
    assert by_id[parent.origin.entity_id]["result"] == "pass"
    assert by_id[child.origin.entity_id]["fragment_result"] == "unknown"
    assert by_id[child.origin.entity_id]["result"] == "unknown"


def test_fragment_binding_rejects_wrong_parent_extent_or_translation_unit():
    parent, child, objects, image = fragment_fixture()
    with pytest.raises(ValueError, match="combined extent"):
        wrong = FunctionBinding(parent.origin, (Span(PARENT, CHILD),), "_f", UNIT.as_posix(),
                                source_spans=(Span(PARENT, CHILD),))
        check_objects([wrong], objects, image, fragments=[child])
    with pytest.raises(ValueError, match="combined extent"):
        wrong = FunctionBinding(parent.origin, parent.spans, "_other", UNIT.as_posix())
        check_objects([wrong], objects, image, fragments=[child])


def fragment_view():
    parent_origin = Origin("a" * 64, "pe:va", PARENT)
    child_origin = Origin("a" * 64, "pe:va", CHILD)
    parent_id = parent_origin.entity_id
    child_id = child_origin.entity_id
    relation = {"parent_address": PARENT, "parent_symbol": SYMBOL,
                "reason": "terminal tail", "evidence": ["pinned:adjacency"]}
    parent = {"entity_id": parent_id, "origin": {"image_sha256": "a" * 64,
              "address_space": "pe:va", "address": PARENT}, "conflicts": [],
              "candidates": [{"seed": False, "component": None, "path": UNIT.as_posix(),
                              "symbol": SYMBOL, "spans": [{"start": PARENT, "end": CHILD}]}]}
    child = {"entity_id": child_id, "origin": {"image_sha256": "a" * 64,
             "address_space": "pe:va", "address": CHILD}, "conflicts": [],
             "candidates": [{"seed": False, "component": None, "path": UNIT.as_posix(),
                             "symbol": "tail", "spans": [{"start": CHILD, "end": CHILD + 1}],
                             "function_fragment": relation}]}
    return {"groups": [{"entities": [parent, child]}]}, parent_id, child_id


def associated_parent_coff():
    """Two valid COMDATs, with the mapped function in an associative child."""
    section_names = (b".text$R", b".text$C")
    raw_sections = (b"\xcc" * 16, b"\x90\xc3" + b"\xcc" * 14)
    section_table = 20
    raw_offsets = (section_table + 2 * 40, section_table + 2 * 40 + 16)
    symbol_offset = section_table + 2 * 40 + sum(map(len, raw_sections))
    headers = []
    for index, (name, code, offset) in enumerate(zip(section_names, raw_sections,
                                                       raw_offsets, strict=True)):
        headers.append(name.ljust(8, b"\0") + struct.pack(
            "<IIIIIIHHI", 0, 0, len(code), offset, 0, 0, 0, 0, 0x60501020))

    symbols = bytearray()
    for index, (name, selection, parent_number) in enumerate((
            (section_names[0], 2, 0), (section_names[1], 5, 1))):
        symbols.extend(name.ljust(8, b"\0"))
        symbols.extend(struct.pack("<IhHBB", 0, index + 1, 0, 3, 1))
        aux = struct.pack("<IHHIHB", 16, 0, 0, 0, parent_number, selection)
        symbols.extend(aux + bytes(3))
    symbols.extend(b"_f".ljust(8, b"\0"))
    symbols.extend(struct.pack("<IhHBB", 0, 2, 0x20, 2, 0))
    file_header = struct.pack("<HHIIIHH", 0x14C, 2, 0, symbol_offset, 5, 0, 0)
    return (file_header + b"".join(headers) + b"".join(raw_sections)
            + bytes(symbols) + struct.pack("<I", 4))


def test_selected_child_creates_parent_support_not_child_callable_binding():
    view, parent_id, child_id = fragment_view()
    bindings, refused, _externals = select_bindings(view, frozenset({child_id}))
    assert bindings == [] and refused == []
    expanded, fragments, support, refused = select_fragment_bindings(
        view, frozenset({child_id}), bindings)
    assert not refused
    assert len(expanded) == len(fragments) == 1
    assert expanded[0].origin.entity_id == parent_id
    assert expanded[0].spans == (Span(PARENT, CHILD + 1),)
    assert expanded[0].source_spans == (Span(PARENT, CHILD),)
    assert support == {parent_id}
    assert fragments[0].origin.address == CHILD
    assert fragments[0].parent_entity_id == parent_id


def test_selected_parent_expands_to_complete_relation_extent_without_selecting_child():
    view, parent_id, child_id = fragment_view()
    bindings, refused, _externals = select_bindings(view, frozenset({parent_id}))
    assert not refused and len(bindings) == 1
    expanded, fragments, support, refused = select_fragment_bindings(
        view, frozenset({parent_id}), bindings)
    assert not refused and not fragments and not support
    assert len(expanded) == 1 and expanded[0].spans == (Span(PARENT, CHILD + 1),)


@pytest.mark.parametrize("damage", ["symbol", "path", "component"])
def test_parent_only_selection_refuses_bad_fragment_parent_metadata(damage):
    view, parent_id, child_id = fragment_view()
    parent_candidate = view["groups"][0]["entities"][0]["candidates"][0]
    if damage == "symbol":
        parent_candidate["symbol"] = "_wrong_parent"
    elif damage == "path":
        parent_candidate["path"] = "src/other.cpp"
    else:
        parent_candidate["component"] = "unverified-runtime"

    bindings, _refused, _externals = select_bindings(view, frozenset({parent_id}))
    expanded, fragments, support, refused = select_fragment_bindings(
        view, frozenset({parent_id}), bindings)
    assert not expanded and not fragments and not support
    assert any(row["entity_id"] == parent_id and row["result"] == "unknown"
               for row in refused)

    # If both identities are selected, neither can disappear behind the
    # other's invalid relation. The child remains non-callable throughout.
    selected = frozenset({parent_id, child_id})
    bindings, _refused, _externals = select_bindings(view, selected)
    expanded, fragments, support, refused = select_fragment_bindings(
        view, selected, bindings)
    assert not expanded and not fragments and not support
    assert {row["entity_id"] for row in refused if row["result"] == "unknown"} >= selected


@pytest.mark.parametrize("component_owner", ["parent", "child", "both"])
def test_component_parent_and_child_cannot_bypass_noncallable_fragment_routing(component_owner):
    view, parent_id, child_id = fragment_view()
    parent = view["groups"][0]["entities"][0]["candidates"][0]
    child = view["groups"][0]["entities"][1]["candidates"][0]
    if component_owner in ("parent", "both"):
        parent["component"] = "runtime"
    if component_owner in ("child", "both"):
        child["component"] = "runtime"

    selected = frozenset({parent_id, child_id})
    bindings, _ordinary_refused, _externals = select_bindings(view, selected)
    expanded, fragments, support, refused = select_fragment_bindings(view, selected, bindings)

    assert not expanded and not fragments and not support
    assert {row["entity_id"] for row in refused if row["result"] == "unknown"} >= selected
    child_refusal = next(row for row in refused if row["entity_id"] == child_id)
    if component_owner in ("child", "both"):
        assert "component-owned" in child_refusal["reason"]


def test_exact_component_join_does_not_reclassify_fragment_as_callable_dependency():
    pin = "a" * 64
    child_origin = Origin(pin, "pe:va", CHILD)
    binding = ToolchainBinding(child_origin, (Span(CHILD, CHILD + 1),),
                               "tail", "runtime.obj", "synthetic component control")
    manifest = ToolchainManifest(
        Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
                  ("synthetic component control",), "fixture"),
        "b" * 64, "synthetic pinned archive provenance", (binding,))
    companion = render_component(manifest)
    relation = {"parent_address": PARENT, "parent_symbol": SYMBOL,
                "reason": "terminal tail", "evidence": ["pinned:fixture"]}
    parent_origin = Origin(pin, "pe:va", PARENT)
    snapshot = {
        "image_sha256": pin,
        "source_configuration": {"image_pin": "original/PIN", "address_space": "pe:va",
                                 "roots": ["engine"], "extensions": [".h"]},
        "records": [
            {"path": "engine/runtime.recovery.h", "entity_id": parent_origin.entity_id,
             "invalid_spans": [], "spans": [[PARENT, CHILD]], "function_fragment": None},
            {"path": "engine/runtime.recovery.h", "entity_id": child_origin.entity_id,
             "invalid_spans": [], "spans": [[CHILD, CHILD + 1]],
             "function_fragment": relation}],
        "files": {"engine/runtime.recovery.h": {
            "content_base64": base64.b64encode(companion.encode()).decode("ascii")}},
    }

    # A verified component row must not turn the fragment's exact marker into
    # an independent archive function measurement. It remains unresolved.
    assert exact_component_record_bindings(snapshot) == {}
    raw_candidates = component_record_identity_candidates(snapshot)
    assert raw_candidates == {("engine/runtime.recovery.h", child_origin.entity_id)}
    rows = read_text(
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401001 FILE\n"
        "// symbol _parent\nvoid parent() {}\n"
        "// ORIGINAL: 0x00401001 tail 0x00401001-0x00401002 FILE BYTE_EXACT\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
        '"parent_symbol":"_parent","reason":"terminal tail",'
        '"evidence":["pinned:fixture"]}\n',
        Path("engine/runtime.recovery.h"))
    parent_row, child_row = rows
    component_keys = {(child_row.path.resolve(), child_row.address, child_row.line)}
    child_pairs, child_refused = resolve_fragment_pairs(
        [parent_row, child_row], [child_row], component_keys=component_keys)
    parent_pairs, parent_refused = resolve_fragment_pairs(
        [parent_row, child_row], [parent_row], component_keys=component_keys)
    assert not child_pairs and not parent_pairs
    assert (child_row.path.resolve(), child_row.address, child_row.line) in child_refused
    assert (parent_row.path.resolve(), parent_row.address, parent_row.line) in parent_refused


def test_legacy_fragment_wrong_parent_symbol_refuses_selected_parent_by_address():
    parent, child = read_text(source(), UNIT)
    wrong_parent = replace(parent, symbol="_wrong")
    pairs, refused = resolve_fragment_pairs(
        [wrong_parent, child], [wrong_parent])
    assert not pairs
    assert (wrong_parent.path.resolve(), wrong_parent.address, wrong_parent.line) in refused


def test_nonexclusive_parent_comdat_cannot_prove_fragment_bytes():
    parent, child, _objects, image = fragment_fixture()
    raw = bytearray(coff_fixture(b"\x90\xc3"))
    symbol_offset, = struct.unpack_from("<I", raw, 8)
    symbol_count, = struct.unpack_from("<I", raw, 12)
    alias = (b"_alias".ljust(8, b"\0")
             + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0))
    insertion = symbol_offset + symbol_count * 18
    raw[insertion:insertion] = alias
    struct.pack_into("<I", raw, 12, symbol_count + 1)
    obj = Coff(bytes(raw))

    with pytest.raises(ValueError, match="exclusively own"):
        obj.function("_f")
    rows = check_objects([parent], {UNIT.as_posix(): obj}, image,
                         fragments=[child])
    child_row = next(row for row in rows if row["entity_id"] == child.origin.entity_id)
    parent_row = next(row for row in rows if row["entity_id"] == parent.origin.entity_id)
    assert parent_row["result"] == "unknown"
    assert child_row["result"] == child_row["parent_whole_body_result"] == "unknown"


def test_associative_comdat_parent_cannot_be_claimed_as_independent_owner():
    parent, child, _objects, image = fragment_fixture()
    obj = Coff(associated_parent_coff())
    assert obj.associations == {2: 1}
    assert obj.function("_f").section == 2

    rows = check_objects([parent], {UNIT.as_posix(): obj}, image,
                         fragments=[child])
    parent_row = next(row for row in rows if row["entity_id"] == parent.origin.entity_id)
    child_row = next(row for row in rows if row["entity_id"] == child.origin.entity_id)
    assert parent_row["result"] == "unknown"
    assert "independent root code COMDAT" in parent_row["reason"]
    assert child_row["result"] == child_row["parent_whole_body_result"] == "unknown"


def test_parent_only_fragment_extent_still_requires_root_comdat_ownership():
    parent, _child, _objects, image = fragment_fixture()
    objects = {UNIT.as_posix(): Coff(associated_parent_coff())}
    rows = check_objects([parent], objects, image)
    parent_row, = rows
    assert parent_row["result"] == "unknown"
    assert "independent root code COMDAT" in parent_row["reason"]


def test_parent_only_selection_refuses_disagreeing_selected_binding():
    view, parent_id, child_id = fragment_view()
    wrong = FunctionBinding(
        Origin("a" * 64, "pe:va", PARENT), (Span(PARENT, CHILD),),
        "_wrong_parent", UNIT.as_posix())
    expanded, fragments, support, refused = select_fragment_bindings(
        view, frozenset({parent_id}), [wrong])
    assert not expanded and not fragments and not support
    assert any(row["entity_id"] == parent_id and row["result"] == "unknown"
               for row in refused)

    selected = frozenset({parent_id, child_id})
    expanded, fragments, support, refused = select_fragment_bindings(
        view, selected, [wrong])
    assert not expanded and not fragments and not support
    assert {row["entity_id"] for row in refused if row["result"] == "unknown"} >= selected


def test_raw_view_rejects_fragment_parent_chains_for_selected_rows():
    view, parent_id, child_id = fragment_view()
    grandchild_address = CHILD + 1
    grandchild_origin = Origin("a" * 64, "pe:va", grandchild_address)
    view["groups"][0]["entities"].append({
        "entity_id": grandchild_origin.entity_id,
        "origin": {"image_sha256": "a" * 64, "address_space": "pe:va",
                   "address": grandchild_address},
        "conflicts": [],
        "candidates": [{"seed": False, "component": None, "path": UNIT.as_posix(),
                         "symbol": "grandchild", "spans": [{"start": grandchild_address,
                                                               "end": grandchild_address + 1}],
                         "function_fragment": {"parent_address": CHILD,
                                               "parent_symbol": "tail",
                                               "reason": "nested tail",
                                               "evidence": ["pinned:relation"]}}],
    })
    child_candidate = view["groups"][0]["entities"][1]["candidates"][0]
    child_candidate["symbol"] = "tail"
    selected = frozenset({child_id, grandchild_origin.entity_id})
    bindings, _refused, _externals = select_bindings(view, selected)
    expanded, fragments, support, refused = select_fragment_bindings(view, selected, bindings)
    assert not expanded and not fragments and not support
    assert {row["entity_id"] for row in refused if row["result"] == "unknown"} >= selected


def legacy_fragment_records(path):
    text = (
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401001 FILE\n"
        "// symbol _f\n"
        "// ORIGINAL: 0x00401001 tail 0x00401001-0x00401002 FILE BYTE_EXACT\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
        '"parent_symbol":"_f","reason":"adjacent terminal byte",'
        '"evidence":["pinned fixture"]}\n')
    return read_text(text, path)


def resolved_legacy_pair(parent, child, selected):
    pairs, refused = resolve_fragment_pairs([parent, child], selected)
    assert not refused and len(pairs) == 1
    return pairs


def test_legacy_unit_measures_child_through_one_physical_parent_object(tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    parent, child = legacy_fragment_records(source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    calls = []

    def compiler(path, command, flags):
        calls.append((path, command, flags))
        return coff_fixture(b"\x90\xc3")

    rows = compare_unit([child], image, source_path, [], ((),), compiler=compiler,
                        fragment_pairs=resolved_legacy_pair(parent, child, [child]))
    assert len(rows.results) == 1 and rows.results[0][0] is child
    assert rows.results[0][1].verdict.value == "BYTE_EXACT"
    assert len(calls) == rows.compile_attempts == 1


def test_legacy_unit_rejects_fabricated_or_stale_pair_admission(tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    parent, child = legacy_fragment_records(source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    raw = coff_fixture(b"\x90\xc3")
    calls = []
    compiler = lambda *args: (calls.append(args) or raw)

    unbound = compare_unit([child], image, source_path, [], ((),), compiler=compiler,
                           fragment_pairs=[(child, parent)])
    assert isinstance(unbound.results[0][1], str)
    stale_child = replace(child, name="different selected relation")
    admitted = resolved_legacy_pair(parent, child, [child])
    stale = compare_unit([stale_child], image, source_path, [], ((),),
                         compiler=compiler, fragment_pairs=admitted)
    assert isinstance(stale.results[0][1], str)
    parent_only_unbound = compare_unit([parent], image, source_path, [], ((),),
                                       compiler=compiler,
                                       fragment_pairs=[(child, parent)])
    assert isinstance(parent_only_unbound.results[0][1], str)
    assert calls == []


@pytest.mark.parametrize("trial", ["/c /O2", ("/c", "/O2")])
def test_fragment_compile_preserves_command_and_tokenized_flag_contract(tmp_path, trial):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    parent, child = legacy_fragment_records(source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    calls = []

    def compiler(path, command, flags):
        calls.append((path, command, flags))
        assert command == ("compiler", "/nologo")
        assert flags == ("/c", "/O2")
        return coff_fixture(b"\x90\xc3")

    rows = compare_unit([parent, child], image, source_path,
                        ["compiler", "/nologo"], (trial,), compiler=compiler,
                        fragment_pairs=resolved_legacy_pair(parent, child,
                                                            [parent, child]))
    assert calls == [(source_path, ("compiler", "/nologo"), ("/c", "/O2"))]
    assert rows.compile_attempts == 1
    assert all(value.verdict.value == "BYTE_EXACT" for _row, value in rows.results)


def test_legacy_fragment_prefix_mismatch_never_promotes_child(tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    parent, child = legacy_fragment_records(source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    rows = compare_unit([parent, child], image, source_path, [], ((),),
                        compiler=lambda *_: coff_fixture(b"\x91\xc3"),
                        fragment_pairs=resolved_legacy_pair(parent, child, [parent, child]))
    by_address = {row.address: value for row, value in rows.results}
    assert by_address[parent.address].verdict.value == "MISMATCH"
    assert by_address[child.address].verdict.value == "REFUSED"


def test_legacy_fragment_declared_span_includes_terminal_bytes(tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    parent, child = legacy_fragment_records(source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    rows = compare_unit([parent, child], image, source_path, [], ((),),
                        compiler=lambda *_: coff_fixture(b"\xc3"),
                        fragment_pairs=resolved_legacy_pair(parent, child, [parent, child]))
    by_address = {row.address: value for row, value in rows.results}
    assert by_address[parent.address].verdict.value == "MISMATCH"
    assert by_address[child.address].verdict.value == "REFUSED"


def test_legacy_fragment_missing_or_ambiguous_parent_stays_unresolved(tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    parent, child = legacy_fragment_records(source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    unresolved = compare_unit([child], image, source_path, [], ((),),
                              compiler=lambda *_: coff_fixture(b"\x90\xc3"))
    assert isinstance(unresolved.results[0][1], str)
    duplicate = replace(parent, line=parent.line + 1, symbol="_other")
    _pairs, refused = resolve_fragment_pairs([parent, duplicate, child], [child])
    assert (child.path.resolve(), child.address, child.line) in refused


def test_legacy_fragment_crossing_coff_relocation_is_refused(tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    text = (
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401001 FILE\n"
        "// symbol _f\n"
        "// ORIGINAL: 0x00401001 tail 0x00401001-0x00401006 FILE BYTE_EXACT\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
        '"parent_symbol":"_f","reason":"adjacent code",'
        '"evidence":["fixture"]}\n')
    parent, child = read_text(text, source_path)
    image = tmp_path / "fixture.exe"
    original = b"\xe8\0\0\0\0\xc3"
    image.write_bytes(pe_fixture(original))
    rows = compare_unit([child], image, source_path, [], ((),),
                        compiler=lambda *_: coff_fixture(original,
                                                         relocation=(0, 1, 20)),
                        fragment_pairs=resolved_legacy_pair(parent, child, [child]))
    assert isinstance(rows.results[0][1], str)
    assert "crosses" in rows.results[0][1]


def test_legacy_fragment_rejects_unsupported_relocation_even_when_bytes_match(tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    parent, child = legacy_fragment_records(source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    rows = compare_unit([child], image, source_path, [], ((),),
                        compiler=lambda *_: coff_fixture(b"\x90\xc3",
                                                         relocation=(0, 1, 7)),
                        fragment_pairs=resolved_legacy_pair(parent, child, [child]))
    assert isinstance(rows.results[0][1], str)
    assert "unsupported COFF relocation kind" in rows.results[0][1]


def test_legacy_fragment_rejects_padding_beyond_section_alignment_bound(tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    parent, child = legacy_fragment_records(source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    oversized = b"\x90\xc3" + b"\x90" * 15
    rows = compare_unit([child], image, source_path, [], ((),),
                        compiler=lambda *_: coff_fixture(oversized),
                        fragment_pairs=resolved_legacy_pair(parent, child, [child]))
    assert isinstance(rows.results[0][1], str)
    assert "exceeds its bounded section-alignment padding" in rows.results[0][1]


def test_legacy_fragment_rejects_relocation_in_alignment_padding(tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    parent, child = legacy_fragment_records(source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    rows = compare_unit([child], image, source_path, [], ((),),
                        compiler=lambda *_: coff_fixture(b"\x90\xc3",
                                                         relocation=(8, 1, 6)),
                        fragment_pairs=resolved_legacy_pair(parent, child, [child]))
    assert isinstance(rows.results[0][1], str)
    assert "relocation lies outside" in rows.results[0][1]


def test_duplicate_admitted_pairs_cannot_leave_a_child_measurement(tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    parent, child = legacy_fragment_records(source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    admitted = resolved_legacy_pair(parent, child, [parent, child])
    calls = []
    rows = compare_unit([parent, child], image, source_path, [], ((),),
                        compiler=lambda *args: (calls.append(args)
                                                 or coff_fixture(b"\x90\xc3")),
                        fragment_pairs=admitted + admitted)
    assert all(isinstance(value, str) and "duplicate fragment relation" in value
               for _row, value in rows.results)
    assert calls == []


@pytest.mark.parametrize("original,compiled", [
    (b"\x90\xcc\x90\xcc", b"\x90\xcc\x90\x90"),
    (b"\x90\xcc\x90\x90", b"\x90\xcc\x90\xcc"),
])
def test_fragment_exact_sized_parent_compares_terminal_padding_bytes(
        original, compiled, tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    text = (
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401002 FILE\n"
        "// symbol _f\n"
        "// ORIGINAL: 0x00401002 tail 0x00401002-0x00401004 FILE BYTE_EXACT\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
        '"parent_symbol":"_f","reason":"adjacent terminal padding",'
        '"evidence":["physical fixture"]}\n')
    parent, child = read_text(text, source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(original))
    rows = compare_unit([parent, child], image, source_path, [], ((),),
                        compiler=lambda *_: coff_fixture(compiled),
                        fragment_pairs=resolved_legacy_pair(parent, child,
                                                            [parent, child]))
    by_address = {row.address: value for row, value in rows.results}
    assert by_address[parent.address].verdict.value == "MISMATCH"
    assert by_address[parent.address].compared_bytes == len(original)
    assert by_address[child.address].verdict.value == "REFUSED"


@pytest.mark.parametrize("tail", [b"\x90", b"\xcc"])
def test_fragment_child_slice_covers_terminal_nop_and_int3(tail, tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    original = b"\xc3" + tail
    text = (
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401001 FILE\n"
        "// symbol _f\n"
        "// ORIGINAL: 0x00401001 tail 0x00401001-0x00401002 FILE BYTE_EXACT\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
        '"parent_symbol":"_f","reason":"terminal child byte",'
        '"evidence":["physical fixture"]}\n')
    parent, child = read_text(text, source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(original))
    rows = compare_unit([parent, child], image, source_path, [], ((),),
                        compiler=lambda *_: coff_fixture(original),
                        fragment_pairs=resolved_legacy_pair(parent, child,
                                                            [parent, child]))
    by_address = {row.address: value for row, value in rows.results}
    assert by_address[parent.address].verdict.value == "BYTE_EXACT"
    assert by_address[parent.address].compared_bytes == len(original)
    assert by_address[child.address].verdict.value == "BYTE_EXACT"
    assert by_address[child.address].compared_bytes == 1


@pytest.mark.parametrize("damage", ["duplicate-owner", "associative-owner"])
def test_legacy_fragment_requires_exclusive_non_associative_parent(damage, tmp_path):
    source_path = tmp_path / "fixture.cpp"
    source_path.write_text("// fixture TU\n")
    parent, child = legacy_fragment_records(source_path)
    image = tmp_path / "fixture.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    if damage == "duplicate-owner":
        raw = bytearray(coff_fixture(b"\x90\xc3"))
        symbol_offset, = struct.unpack_from("<I", raw, 8)
        symbol_count, = struct.unpack_from("<I", raw, 12)
        insertion = symbol_offset + symbol_count * 18
        raw[insertion:insertion] = (b"_alias".ljust(8, b"\0")
                                    + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0))
        struct.pack_into("<I", raw, 12, symbol_count + 1)
        object_bytes = bytes(raw)
    else:
        object_bytes = associated_parent_coff()
    rows = compare_unit([child], image, source_path, [], ((),),
                        compiler=lambda *_: object_bytes,
                        fragment_pairs=resolved_legacy_pair(parent, child, [child]))
    assert isinstance(rows.results[0][1], str)
    assert ("exclusive" in rows.results[0][1]
            or "associative" in rows.results[0][1])


def test_fragment_recording_refuses_before_injected_exact_measurement_or_publication(tmp_path):
    source_path = tmp_path / UNIT
    source_path.parent.mkdir(parents=True)
    source_path.write_text(source())
    rows = read_text(source_path.read_text(), source_path)
    child = rows[1]
    before = source_path.read_bytes()
    called = []

    def injected_exact():
        called.append("measure")
        from match_recovery.source.model import Tier
        return [(child, AsmComparison(Tier.BYTE_EXACT))]

    def validate():
        called.append("publish")

    with pytest.raises(ValueError, match="non-callable function fragment"):
        record_claims([child], measure=injected_exact, validate=validate,
                      staging_directory=tmp_path / "staging")
    assert called == []
    assert source_path.read_bytes() == before


def test_parent_recording_refuses_attached_fragment_before_injected_exact_result(tmp_path):
    source_path = tmp_path / UNIT
    source_path.parent.mkdir(parents=True)
    source_path.write_text(source())
    parent = read_text(source_path.read_text(), source_path)[0]
    before = source_path.read_bytes()
    called = []

    def injected_exact():
        called.append("measure")
        return [(parent, AsmComparison(Tier.BYTE_EXACT))]

    def validate():
        called.append("validate")

    with pytest.raises(ValueError, match="parent with an attached function fragment"):
        record_claims([parent], measure=injected_exact, validate=validate,
                      staging_directory=tmp_path / "staging")
    assert source_path.read_bytes() == before
    assert called == []


def test_public_measure_routes_fragment_through_parent_unit(match_base_project):
    root = match_base_project
    path = root / "engine/family.cc"
    path.write_text(
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401001 FILE\n"
        "// symbol _f\nvoid f() {}\n"
        "// ORIGINAL: 0x00401001 tail 0x00401001-0x00401002 FILE BYTE_EXACT\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
        '"parent_symbol":"_f","reason":"terminal instruction tail",'
        '"evidence":["pinned-image"]}\n'
        "// ORIGINAL: 0x00401002 _h 0x00401002-0x00401004\n"
        "int h() {return 0;}\n")
    report = measure_source(root, ("tail",), image=Path("original/input.exe"))
    assert report["compile_attempts"] > 0
    row, = report["measurements"]
    assert row["historical_byte_obligation"] is True
    assert row["verdict"] != "UNMEASURED_FRAGMENT"
    assert report["persisted"] is False and report["ready_for_collection"] is False


def configure_legacy_fragment_project(root, object_code):
    path = root / "engine/family.cc"
    path.write_text(
        "// ORIGINAL: 0x00401000 parent 0x00401000-0x00401001 FILE\n"
        "// symbol _f\nvoid f() {}\n"
        "// ORIGINAL: 0x00401001 tail 0x00401001-0x00401002 FILE BYTE_EXACT\n"
        '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
        '"parent_symbol":"_f","reason":"adjacent terminal byte",'
        '"evidence":["physical control"]}\n')
    image = root / "original/input.exe"
    image.write_bytes(pe_fixture(b"\x90\xc3"))
    import hashlib
    (root / "original/PIN").write_text(
        "sha256 " + hashlib.sha256(image.read_bytes()).hexdigest() + "\n")
    configure_compiler(root, raw_object=coff_fixture(object_code))


@pytest.fixture
def fragment_match_project(match_base_project):
    root = match_base_project
    (root / "build").mkdir(exist_ok=True)
    configure_database(root, ["engine/family.cc"])
    configuration(root, lambda value: value.update(mappings={
        "compile_commands": "build/commands.json", "forwarder_macros": []}))
    return root


def test_match_check_measures_claimed_fragment_in_parent_comdat(fragment_match_project):
    configure_legacy_fragment_project(fragment_match_project, b"\x90\xc3")
    report = check_matches(fragment_match_project, Path("original/input.exe"))
    child = next(row for row in report["measurements"] if row["address"] == "0x00401001")
    assert child["result"] == "pass" and child["verdict"] == "BYTE_EXACT"
    assert report["counts"] == {"pass": 1}
    assert report["compile_attempts"] == 1


def test_match_check_parallel_worker_preserves_fragment_admission(fragment_match_project):
    configure_legacy_fragment_project(fragment_match_project, b"\x90\xc3")
    (fragment_match_project / "engine/other.cc").write_text(
        "// ORIGINAL: 0x00401010 _g 0x00401010-0x00401011\nvoid g() {}\n")
    configure_database(fragment_match_project, ["engine/family.cc", "engine/other.cc"])
    report = check_matches(fragment_match_project, Path("original/input.exe"), jobs=2)
    child = next(row for row in report["measurements"] if row["address"] == "0x00401001")
    assert child["result"] == "pass" and child["verdict"] == "BYTE_EXACT"
    assert report["translation_units"] == 2


def test_match_check_keeps_fragment_unknown_when_parent_prefix_does_not_match(fragment_match_project):
    configure_legacy_fragment_project(fragment_match_project, b"\x91\xc3")
    report = check_matches(fragment_match_project, Path("original/input.exe"))
    child = next(row for row in report["measurements"] if row["address"] == "0x00401001")
    assert child["result"] == "unknown" and child["verdict"] == "REFUSED"
    assert report["counts"] == {"unknown": 1}


def test_selected_child_with_missing_parent_is_unknown_not_dropped():
    view, _parent_id, child_id = fragment_view()
    view["groups"][0]["entities"].pop(0)
    bindings, refused, _externals = select_bindings(view, frozenset({child_id}))
    expanded, fragments, support, more_refused = select_fragment_bindings(
        view, frozenset({child_id}), bindings)
    assert not expanded and not fragments and not support
    assert not refused and more_refused[0]["entity_id"] == child_id
    assert more_refused[0]["result"] == "unknown"


def test_competing_child_relations_make_parent_extent_unknown_not_order_dependent():
    view, parent_id, child_id = fragment_view()
    other = deepcopy(view["groups"][0]["entities"][1])
    other["origin"]["address"] = CHILD + 1
    other["entity_id"] = Origin("a" * 64, "pe:va", CHILD + 1).entity_id
    other["candidates"][0]["spans"] = [{"start": CHILD + 1, "end": CHILD + 2}]
    view["groups"][0]["entities"].append(other)
    bindings, refused, _ = select_bindings(view, frozenset({parent_id}))
    expanded, fragments, support, fragment_refused = select_fragment_bindings(
        view, frozenset({parent_id}), bindings)
    assert not expanded and not fragments and not support
    assert any(row["entity_id"] == parent_id and row["result"] == "unknown"
               for row in fragment_refused)


@pytest.mark.parametrize("damage", ["conflicts", "multiple-candidates"])
def test_conflicted_parent_cannot_become_fragment_support(damage):
    view, parent_id, child_id = fragment_view()
    parent = view["groups"][0]["entities"][0]
    if damage == "conflicts":
        parent["conflicts"] = [{"reason": "competing source ownership"}]
    else:
        parent["candidates"].append(deepcopy(parent["candidates"][0]))
    bindings, _refused, _ = select_bindings(view, frozenset({child_id}))
    expanded, fragments, support, refused = select_fragment_bindings(
        view, frozenset({child_id}), bindings)
    assert not expanded and not fragments and not support
    assert any(row["entity_id"] == child_id and row["result"] == "unknown"
               for row in refused)

    bindings, _refused, _ = select_bindings(view, frozenset({parent_id}))
    expanded, fragments, support, refused = select_fragment_bindings(
        view, frozenset({parent_id}), bindings)
    assert not expanded and not fragments and not support
    assert any(row["entity_id"] == parent_id and row["result"] == "unknown"
               for row in refused)

    bindings, refused, _ = select_bindings(view, frozenset({child_id}))
    expanded, fragments, support, fragment_refused = select_fragment_bindings(
        view, frozenset({child_id}), bindings)
    assert not expanded and not fragments and not support
    assert any(row["entity_id"] == child_id and row["result"] == "unknown"
               for row in fragment_refused)


def test_restoration_does_not_accept_child_slice_when_parent_whole_body_misses():
    row = {"variants": [{"result": "pass", "boundary": "pass", "bytes": "pass",
                         "references": "pass", "flags": [],
                         "parent_whole_body_result": "fail",
                         "fragment_result": "pass", "relocations": []}]}
    exact, references, _reason = _binary_outcome(row)
    assert exact == "unknown"
    assert references == "pass"


def test_restoration_does_not_infer_parent_proof_from_fragment_only_row():
    row = {"variants": [{"result": "pass", "boundary": "pass", "bytes": "pass",
                         "references": "pass", "flags": [], "fragment_result": "pass",
                         "callable": False, "link_target": False, "relocations": []}]}
    exact, references, _reason = _binary_outcome(row)
    assert exact == "unknown"
    assert references == "pass"


@pytest.mark.parametrize(("candidate", "expected", "parent_result"), [
    (b"\x90\xc3", "pass", "pass"),
    (b"\x91\xc3", "unknown", "fail"),
])
def test_full_task_route_selects_child_slice_and_parent_support(
        application_project, candidate, expected, parent_result):
    root = application_project
    pin = configure_fragment_task(root, CHILD)

    def compiler(_source_path, _command, _flags):
        return coff_fixture(candidate)

    report = verify_task(root, "owner", root / "original.exe",
                         compiler=compiler, command_for=lambda _path: ())
    child_rows = [row for row in report["functions"] if row["entity_id"] == Origin(
        pin, "pe:va", CHILD).entity_id]
    assert len(child_rows) == 1
    assert child_rows[0]["result"] == expected
    trial, = child_rows[0]["variants"]
    assert trial.get("parent_whole_body_result") == parent_result
    assert trial["parent_primary_spans"] == [{"start": PARENT, "end": CHILD}]
    assert trial["parent_comparison_span"] == {"start": PARENT, "end": CHILD + 1}
    assert trial["callable"] is trial["link_target"] is False
    assert report["translation_units"] == ["code/unit.cc"]
    assert report["selected_entities"] == 1
    assert all(row["entity_id"] != Origin(pin, "pe:va", PARENT).entity_id
               for row in report["functions"])


def configure_fragment_task(root, selected_address):
    import hashlib
    from dataclasses import replace
    image_data = pe_fixture(b"\x90\xc3")
    pin = hashlib.sha256(image_data).hexdigest()
    (root / "original.exe").write_bytes(image_data)
    (root / "code/PIN").write_text("sha256 " + pin + "\n")
    (root / "code/unit.cc").write_text(
        "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001 FILE\n"
        "// symbol    _f\n"
        "void f() {}\n"
        "// ORIGINAL: 0x00401001 tail 0x00401001-0x00401002 FILE BYTE_EXACT\n"
        "// RECOVERY_FRAGMENT: v1 {\"parent_address\":\"0x00401000\","
        "\"parent_symbol\":\"_f\",\"reason\":\"terminal tail\","
        "\"evidence\":[\"synthetic:pinned-adjacency\"]}\n")
    store = TaskStore(root)
    task = store.read("owner")
    store_path = root / "recovery/tasks/owner.json"
    store_path.write_text(__import__("json").dumps(replace(
        task, subjects=(Origin(pin, "pe:va", selected_address),)).to_dict()))
    view, snapshot = project_view(root)
    fragment_records = [candidate for group in view["groups"] for entity in group["entities"]
                        for candidate in entity["candidates"]
                        if candidate.get("function_fragment") is not None]
    assert len(fragment_records) == 1, snapshot["records"]
    return pin


def test_parent_only_task_does_not_report_child_claim_as_selected(application_project):
    root = application_project
    pin = configure_fragment_task(root, PARENT)
    report = verify_task(root, "owner", root / "original.exe",
                         compiler=lambda *_: coff_fixture(b"\x90\xc3"),
                         command_for=lambda _path: ())
    rows = report["functions"]
    assert [row["entity_id"] for row in rows] == [Origin(pin, "pe:va", PARENT).entity_id]
    trial, = rows[0]["variants"]
    assert trial["result"] == "pass"
    assert trial["source_spans"] == [{"start": PARENT, "end": CHILD}]
    assert trial["comparison_spans"] == [{"start": PARENT, "end": CHILD + 1}]
    assert trial["comparison_extent_includes_fragment"] is True
    # Child claim remains unselected and therefore is not promoted by parent support.
    assert report["selected_entities"] == 1


def test_historical_match_check_keeps_noncallable_child_claim_unknown(match_base_project):
    root = match_base_project
    (root / "build").mkdir(exist_ok=True)
    configure_database(root, ["engine/family.cc"])
    configuration(root, lambda value: value.update(mappings={
        "compile_commands": "build/commands.json", "forwarder_macros": []}))
    source_path = root / "engine/family.cc"
    source_path.write_text(
        "// ORIGINAL: 0x00401000 _parent 0x00401000-0x00401001 FILE\n"
        "// symbol _parent\n"
        "void parent() {}\n"
        "// ORIGINAL: 0x00401001 tail 0x00401001-0x00401002 FILE BYTE_EXACT\n"
        "// RECOVERY_FRAGMENT: v1 {\"parent_address\":\"0x00401000\","
        "\"parent_symbol\":\"_parent\",\"reason\":\"terminal mapped tail\","
        "\"evidence\":[\"synthetic:pinned-control\"]}\n")
    configure_compiler(root, functions=[("_parent", b"\xc3")])
    report = check_matches(root, Path("original/input.exe"))
    child = next(row for row in report["measurements"] if row["address"] == "0x00401001")
    assert child["byte_obligation"] is True
    assert child["built"] is True
    assert child["result"] == "unknown"
    assert child["verdict"] == "UNMEASURED_FRAGMENT"
    assert "independently measurable" in child["diagnostic"]
    assert report["result"] == "unknown"


@pytest.mark.parametrize(("candidate", "expected"), [
    (b"\x90\xc3", "pass"),
    (b"\x91\xc3", "unknown"),
])
def test_restoration_routes_historical_child_claim_through_parent_extent(
        application_project, candidate, expected):
    root = application_project
    pin = configure_fragment_task(root, CHILD)
    (root / "recovery/tasks/owner.json").unlink()
    git(root, "init", "-b", "main")
    git(root, "config", "user.name", "Fragment test")
    git(root, "config", "user.email", "fragment@example.test")
    baseline = commit(root, "Pinned child identity and claim")
    configuration(root, lambda value: value.update(source_history={
        "baseline_revision": baseline,
        "evidence": ["synthetic source baseline for restoration control"]}))
    commit(root, "Declare restoration history baseline")
    task = Task("owner", "synthetic-game", "coordinator", "Measure mapped fragment",
                baseline, (Origin(pin, "pe:va", CHILD),),
                ("code/unit.cc",), ("synthetic fragment fixture",))
    TaskStore(root).create(task)
    commit(root, "Authorize child-fragment measurement")

    report = measure_restoration(
        root, "owner", root / "original.exe",
        compiler=lambda *_: coff_fixture(candidate), command_for=lambda _path: ())
    # The fragment's byte/restoration route is tested here, while the fixture
    # deliberately lacks the independent layout witness required for the
    # enclosing application check. That unrelated proof remains UNKNOWN.
    assert report["result"] == "unknown"
    assert report["candidate"]["checks"]["binary"]["result"] == expected
    assert report["restoration"] == expected
    detail = next(row for row in report["details"] if row["entity_id"] == Origin(
        pin, "pe:va", CHILD).entity_id)
    assert detail["result"] == expected
    assert detail["references"] == "pass"
    assert report["scope_complete"] is False and report["ready_for_collection"] is False
