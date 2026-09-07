"""System imports are independently named slots, not borrowed call operands."""

import hashlib
import json
import struct
from dataclasses import replace

import pytest

from match_recovery.pe_coff import PeImage
from match_recovery.pe_coff import Coff
from match_recovery.bound_objects import FunctionBinding
from match_recovery.import_source import parse_imports, read_imports, import_targets
from match_recovery.model import Origin, Span
from match_recovery.application_binary import verify_task, check_compiled
from match_recovery.tasks import Task, TaskStore
from match_recovery.variants import measure_variants
from test_binary import pe_fixture, coff_fixture


def image_imports(*, lookup=True, bound=False, ordinal=False):
    raw = bytearray(pe_fixture(b"\xff\x15\x60\x11\x40\0\xc3", absolute_site=2))
    # Descriptor pair at RVA 0x1120, lookup at 0x1150, IAT at 0x1160.
    struct.pack_into("<II", raw, 0x98 + 104, 0x1120, 40)
    struct.pack_into("<IIIII", raw, 0x320, 0x1150 if lookup else 0,
                     123 if bound else 0, 0, 0x1180, 0x1160)
    target = 0x80000007 if ordinal else 0x11A0
    struct.pack_into("<II", raw, 0x350, target, 0)
    struct.pack_into("<II", raw, 0x360, 0x71234567 if bound else target, 0)
    raw[0x380:0x389] = b"DEMO.dll\0"
    raw[0x3A0:0x3A8] = b"\x02\0Paint\0"
    return bytes(raw)


def pe(raw):
    return PeImage(raw, hashlib.sha256(raw).hexdigest())


@pytest.mark.parametrize("lookup,bound", [(True, False), (True, True), (False, False)])
def test_import_identity_comes_from_lookup_and_address_is_the_slot(lookup, bound):
    item, = pe(image_imports(lookup=lookup, bound=bound)).imports()
    assert (item.module, item.name, item.ordinal, item.address) == ("DEMO.dll", "Paint", None, 0x401160)


def test_ordinal_is_not_a_hint_or_a_synthesized_export_name():
    item, = pe(image_imports(ordinal=True)).imports()
    assert item.name is None and item.ordinal == 7 and item.address == 0x401160


def test_bound_address_without_lookup_cannot_become_an_import_name():
    with pytest.raises(ValueError, match="bound.*lookup"):
        pe(image_imports(lookup=False, bound=True)).imports()


@pytest.mark.parametrize("offset,value", [
    (0x98 + 108, 20),  # No terminating descriptor within the declared directory.
    (0x320 + 12, 0),  # Missing module name.
    (0x320 + 16, 0),  # Missing IAT.
    (0x320, 0x11FC),  # No complete terminated lookup table.
    (0x350, 0x80010007),  # Reserved ordinal bits.
    (0x350, 0x2000),  # Unbacked name.
    (0x360, 0x11A2),  # Unbound lookup/IAT disagreement.
    (0x364, 1),  # Different termination in IAT.
    (0x98 + 104, 0),  # Directory size without an RVA.
])
def test_corrupt_import_metadata_is_not_an_empty_or_partial_success(offset, value):
    raw = bytearray(image_imports())
    struct.pack_into("<I", raw, offset, value)
    if offset == 0x320:
        struct.pack_into("<I", raw, 0x3FC, 0x11A0)
    with pytest.raises(ValueError):
        pe(bytes(raw)).imports()


@pytest.mark.parametrize("payload", [b"\xff\0", b"\0", b"A" * 128])
def test_module_names_must_be_nonempty_terminated_ascii(payload):
    raw = bytearray(image_imports())
    raw[0x380:0x380 + len(payload)] = payload
    with pytest.raises(ValueError):
        pe(bytes(raw)).imports()


def test_overlapping_import_address_tables_are_refused():
    raw = bytearray(image_imports())
    struct.pack_into("<II", raw, 0x98 + 104, 0x1120, 60)
    raw[0x334:0x348] = raw[0x320:0x334]
    raw[0x348:0x35C] = bytes(20)
    # Move the shared lookup out of the enlarged descriptor array.
    struct.pack_into("<I", raw, 0x320, 0x1170)
    struct.pack_into("<I", raw, 0x334, 0x1170)
    struct.pack_into("<II", raw, 0x370, 0x11A0, 0)
    with pytest.raises(ValueError, match="overlapping.*import"):
        pe(bytes(raw)).imports()


def test_absent_directory_is_empty_but_truncated_optional_header_is_not():
    assert pe(pe_fixture()).imports() == ()
    raw = bytearray(pe_fixture())
    struct.pack_into("<I", raw, 0x98 + 92, 17)
    with pytest.raises(ValueError):
        pe(bytes(raw)).imports()


DECLARATION = "// RECOVERY_IMPORT: v1 _g DEMO.dll name:Paint\n"


def binding(text=DECLARATION):
    return parse_imports(text, "code/system.h")[0]


def compiled():
    return {"code/caller.cc": Coff(coff_fixture(b"\xff\x15\0\0\0\0\xc3", relocation=(2, 1, 6)))}


@pytest.mark.parametrize("text", [DECLARATION.replace("v1", "v2"),
                                 DECLARATION.replace("name:Paint", "Paint"),
                                 DECLARATION.replace("name:Paint", "name:"),
                                 DECLARATION.replace("name:Paint", "ordinal:65536"),
                                 DECLARATION.replace("Paint", "Pa\0int"),
                                 DECLARATION.replace("Paint", "Páint")])
def test_invalid_source_imports_refuse_instead_of_becoming_unmapped(text):
    with pytest.raises(ValueError, match="import"):
        binding(text)


def test_source_identity_does_not_guess_decorations_or_export_case():
    image, objects = pe(image_imports()), compiled()
    targets, rows = import_targets([binding()], objects, image)
    assert targets == {"_g": 0x401160} and rows[0]["identity"] == "pass"
    assert rows[0]["link_selection"] == "unknown"
    assert import_targets([replace(binding(), module="demo.DLL")], objects, image)[0] == targets
    for wrong in (replace(binding(), module="WRONG.dll"), replace(binding(), name="paint"),
                  replace(binding(), name="Paint@4"), replace(binding(), name=None, ordinal=2)):
        targets, rows = import_targets([wrong], objects, image)
        assert not targets and rows[0]["identity"] == "unknown"


@pytest.mark.parametrize("damage", ["defined", "static", "weak", "common"])
def test_import_binding_cannot_borrow_a_defined_or_link_selected_symbol(damage):
    objects = compiled()
    obj = objects["code/caller.cc"]
    symbol = obj.symbols[1]
    values = {"defined": {"section": 1}, "static": {"storage": 3}, "weak": {"storage": 105},
              "common": {"value": 4}}
    obj.symbols[1] = replace(symbol, **values[damage])
    targets, rows = import_targets([binding()], objects, pe(image_imports()))
    assert not targets and rows[0]["identity"] == "unknown"


def check_import(raw, declaration=DECLARATION, objects=None):
    image = pe(raw)
    origin = Origin(hashlib.sha256(raw).hexdigest(), "pe:va", 0x401000)
    subject = FunctionBinding(origin, (Span(0x401000, 0x401007),), "_f", "code/caller.cc")
    return check_compiled([subject], (), (), objects or compiled(), image, {},
                          import_bindings=parse_imports(declaration, "code/system.h"))


def test_strict_import_reference_checks_have_positive_and_wrong_slot_controls():
    raw = image_imports()
    assert check_import(raw)[0][0]["result"] == "pass"
    assert check_import(raw, "")[0][0]["result"] == "unknown"
    damaged = bytearray(raw)
    struct.pack_into("<I", damaged, 514, 0x401164)  # Same code, wrong slot operand.
    row = check_import(bytes(damaged))[0][0]
    assert row["result"] == row["references"] == "fail"
    # Keeping the original absolute address with no relocation is still wrong.
    objects = {"code/caller.cc": Coff(coff_fixture(raw[512:519]))}
    assert check_import(raw, objects=objects)[0][0]["result"] == "fail"


def test_function_type_hint_does_not_disqualify_an_explicit_import_pointer():
    # VC6 emits this type hint for undefined __imp__ symbols too. It does not
    # establish whether a symbol is a function body or an import pointer.
    objects = compiled()
    obj = objects["code/caller.cc"]
    obj.symbols[1] = replace(obj.symbols[1], type=0x20)
    assert check_import(image_imports(), objects=objects)[0][0]["result"] == "pass"
    assert check_import(image_imports(), "", objects=objects)[0][0]["result"] == "unknown"


def test_conflicting_import_and_function_bindings_are_not_last_writer_wins():
    with pytest.raises(ValueError, match="also has"):
        check_compiled([], (), (), compiled(), pe(image_imports()), {"_g": 0x401160},
                       import_bindings=[binding()])


def with_second_import(raw, module):
    raw = bytearray(raw) + bytearray(512)
    struct.pack_into("<IIII", raw, 0x180, 1024, 0x1000, 1024, 512)
    struct.pack_into("<II", raw, 0x98 + 104, 0x1120, 60)
    struct.pack_into("<I", raw, 0x320, 0x1170)
    struct.pack_into("<II", raw, 0x370, 0x11A0, 0)
    struct.pack_into("<IIIII", raw, 0x334, 0x1200, 0, 0, 0x1220, 0x1210)
    raw[0x348:0x35C] = bytes(20)
    struct.pack_into("<II", raw, 0x400, 0x11A0, 0)
    struct.pack_into("<II", raw, 0x410, 0x11A0, 0)
    raw[0x420:0x420 + len(module) + 1] = module.encode("ascii") + b"\0"
    return bytes(raw)


def test_duplicate_import_identity_cannot_pick_the_slot_from_the_operand():
    raw = with_second_import(image_imports(), "DEMO.dll")
    rows, _, _, imports = check_import(raw)
    assert rows[0]["result"] == imports[0]["identity"] == "unknown"
    assert "exactly one" in imports[0]["reason"]


def test_partial_import_slot_overlap_is_not_mistaken_for_two_independent_slots():
    raw = bytearray(with_second_import(image_imports(bound=True), "OTHER.dll"))
    # A bound second IAT starts two bytes into the first slot. Both can be
    # read from file bytes and have terminated lookup tables, but the loader
    # would overwrite overlapping pointer storage.
    struct.pack_into("<I", raw, 0x334 + 4, 123)
    struct.pack_into("<I", raw, 0x334 + 16, 0x1162)
    with pytest.raises(ValueError, match="overlapping.*import"):
        pe(bytes(raw)).imports()


def test_same_export_in_another_dll_is_not_the_same_dependency():
    raw = with_second_import(image_imports(), "OTHER.dll")
    assert check_import(raw)[0][0]["result"] == "pass"
    assert check_import(raw, DECLARATION.replace("DEMO.dll", "OTHER.dll"))[0][0]["references"] == "fail"


@pytest.fixture
def project(tmp_path):
    root = tmp_path
    (root / "code").mkdir()
    (root / "recovery").mkdir()
    image = image_imports()
    pin = hashlib.sha256(image).hexdigest()
    (root / "image.exe").write_bytes(image)
    (root / "code/PIN").write_text("sha256 " + pin)
    (root / "code/caller.cc").write_text(
        "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401007\nvoid f() { /* baseline */ }\n")
    (root / "code/system.h").write_text(DECLARATION)
    config = {"schema_version": 1, "source": {"image_pin": "code/PIN", "address_space": "pe:va",
              "roots": ["code"], "include_paths": ["code"], "seed_roots": ["code/seeds"]},
              "measurement": {"backend": "pe-coff-i386", "compiler_profile": "synthetic",
              "flag_sets": [["trial"]], "layout_flags": [], "unit_extensions": [".cc"],
              "inputs": [], "input_roots": []}}
    (root / "recovery/project.json").write_text(json.dumps(config))
    task = Task("imports", "synthetic", "coordinator", "Recover caller", "a" * 40,
                (Origin(pin, "pe:va", 0x401000),), ("code/caller.cc",), ("synthetic source evidence",))
    TaskStore(root).create(task)
    return root


def test_configured_task_uses_source_imports_without_a_project_checker(project):
    calls = []
    def compiler(path, *_):
        calls.append(path)
        return coff_fixture(b"\xff\x15\0\0\0\0\xc3", relocation=(2, 1, 6))
    report = verify_task(project, "imports", project / "image.exe", compiler=compiler)
    assert report["result"] == "pass" and report["compile_attempts"] == len(calls) == 1
    assert report["import_bindings"][0]["address"] == "0x00401160"
    assert not report["ready_for_collection"]


def test_seeds_cannot_supply_import_bindings_or_hide_product_duplicates(project):
    (project / "code/seeds").mkdir()
    (project / "code/seeds/old.h").write_text(DECLARATION.replace("DEMO.dll", "OTHER.dll"))
    assert len(read_imports(project)) == 1
    (project / "code/duplicate.h").write_text(DECLARATION)
    with pytest.raises(ValueError, match="duplicate"):
        read_imports(project)


def test_candidate_deck_uses_import_identities_and_cannot_redefine_them(project):
    source = project / "code/caller.cc"
    entity = TaskStore(project).read("imports").subjects[0].entity_id
    deck = {"schema_version": 2, "source": "code/caller.cc", "entity_id": entity,
            "files": {"code/caller.cc": {"source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
                      "before": "/* baseline */", "control": "/* damaged */", "entity_id": entity}},
            "candidates": {"same": {"code/caller.cc": "/* proposed */"}}}
    class Compiler:
        def __call__(self, path, *_):
            return self.compile(path.read_bytes())

        def overlay(self, path, contents, *_):
            return self.compile(contents["code/caller.cc"])

        def compile(self, contents):
            opcode = b"\xcc" if b"damaged" in contents else b"\xff"
            return coff_fixture(opcode + b"\x15\0\0\0\0\xc3", relocation=(2, 1, 6))

    before = source.read_bytes()
    report = measure_variants(project, "imports", project / "image.exe", deck, compiler=Compiler())
    assert report["status"] == "scored"
    assert report["candidates"][0]["target"]["result"] == "pass"
    assert report["candidates"][0]["import_bindings"][0]["identity"] == "pass"
    assert source.read_bytes() == before
    deck["candidates"]["same"]["code/caller.cc"] = "\n" + DECLARATION
    with pytest.raises(ValueError, match="cannot change import identities"):
        measure_variants(project, "imports", project / "image.exe", deck, compiler=object())


def test_legacy_call_navigation_uses_the_same_bounded_parser(tmp_path):
    from match_recovery.source.calls import imported_names
    path = tmp_path / "sample.exe"
    path.write_bytes(image_imports(bound=True))
    assert imported_names(path) == {0x401160: "DEMO.dll!Paint"}
    path.write_bytes(image_imports(ordinal=True))
    assert imported_names(path) == {0x401160: "DEMO.dll!#7"}
    raw = bytearray(image_imports())
    struct.pack_into("<I", raw, 0x98 + 108, 20)
    path.write_bytes(raw)
    with pytest.raises(ValueError, match="unterminated"):
        imported_names(path)
