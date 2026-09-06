from dataclasses import replace
import hashlib
from types import SimpleNamespace

import pytest

from match_recovery.adapters.vc6_generated import check_generated
from match_recovery.generated_source import GeneratedBinding, read_generated
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import PeImage, Section, Symbol
from test_binary import pe_fixture


TABLE = "??_7Widget@@6B@"


def fixture(*, bounded=True, target=0x402000, raw=False):
    image_data = pe_fixture(target.to_bytes(4, "little"), absolute_site=0)
    pin = hashlib.sha256(image_data).hexdigest()
    image = PeImage(image_data, pin)
    binding = GeneratedBinding(Origin(pin, "pe:va", 0x401000), Span(0x401000, 0x401004) if bounded else None,
                               TABLE, Origin(pin, "pe:va", 0x401020), "src/widget.h", 1)
    symbols = {0: Symbol(0, TABLE, 0, 1, 0, 2), 1: Symbol(1, "_method", 0, 0, 0x20, 2)}
    section = Section(target.to_bytes(4, "little") if raw else bytes(4),
                      () if raw else ((0, 1, 6),), 0x40001040, ".rdata")
    return binding, {"src/widget.cpp": SimpleNamespace(symbols=symbols, sections=(section,))}, image


def test_generated_start_binding_does_not_imply_table_content_proof():
    binding, objects, image = fixture(bounded=False)
    targets, rows = check_generated((binding,), objects, image, {})
    assert targets == {TABLE: 0x401000}
    assert rows[0]["definition"] == "pass" and rows[0]["content"] == "unknown"


def test_bounded_table_requires_resolved_slot_targets():
    binding, objects, image = fixture()
    _, rows = check_generated((binding,), objects, image, {})
    assert rows[0]["content"] == "unknown"
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    assert rows[0]["content"] == "pass"
    _, rows = check_generated((binding,), objects, image, {"_method": 0x403000})
    assert rows[0]["content"] == "fail"


def test_copied_absolute_slot_bytes_are_not_valid_relocatable_tables():
    binding, objects, image = fixture(raw=True)
    _, rows = check_generated((binding,), objects, image, {})
    assert rows[0]["content"] == "fail"


def test_candidate_length_cannot_supply_or_extend_original_extent():
    binding, objects, image = fixture()
    obj = next(iter(objects.values()))
    obj.sections = (replace(obj.sections[0], data=bytes(8)),)
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    assert rows[0]["content"] == "fail" and rows[0]["proof"]["boundary"] == "fail"


@pytest.mark.parametrize("flags", [0xC0001040, 0x40000040, 0x40001020])
def test_writable_noncomdat_or_code_definitions_cannot_pose_as_vtables(flags):
    binding, objects, image = fixture()
    obj = next(iter(objects.values()))
    obj.sections = (replace(obj.sections[0], flags=flags),)
    with pytest.raises(ValueError, match="isolated read-only"):
        check_generated((binding,), objects, image, {})


def test_missing_compiled_definition_does_not_resolve_a_reference():
    binding, _, image = fixture()
    assert check_generated((binding,), {}, image, {}) == ({}, [])


def test_all_emitted_copies_are_checked_not_just_one_passing_definition():
    binding, objects, image = fixture()
    obj = next(iter(objects.values()))
    objects["src/other.cpp"] = SimpleNamespace(symbols=obj.symbols,
        sections=(replace(obj.sections[0], data=(4).to_bytes(4, "little")),))
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    assert {row["content"] for row in rows} == {"pass", "fail"}


def test_source_binding_keeps_unknown_extent_and_refuses_duplicates(tmp_path):
    (tmp_path / "src").mkdir()
    path = tmp_path / "src/widget.h"
    text = "// RECOVERY_GENERATED: v1 vtable 0x00401000 ? " + TABLE + " 0x00401020\n"
    path.write_text(text)
    binding, = read_generated(tmp_path, "a" * 64)
    assert binding.span is None and binding.origin.address == 0x401000
    path.write_text(text + text)
    with pytest.raises(ValueError, match="duplicate"):
        read_generated(tmp_path, "a" * 64)
    path.write_text("// RECOVERY_GENERATED: invalid\n")
    with pytest.raises(ValueError, match="invalid"):
        read_generated(tmp_path, "a" * 64)
