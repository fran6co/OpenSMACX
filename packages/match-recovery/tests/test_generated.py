from dataclasses import replace
import hashlib
import struct
from types import SimpleNamespace

import pytest

from match_recovery.generated_binary import check_generated
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


def test_unbounded_diagnostics_show_word_agreement_without_claiming_an_extent():
    binding, objects, image = fixture(bounded=False)
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    row, = rows
    diagnostic = row["diagnostic"]
    assert row["content"] == "unknown" and "proof" not in row
    assert diagnostic["original_extent"] is None and diagnostic["emitted_size"] == 4
    entry, = diagnostic["entries"]
    assert entry["within_declared_extent"] is None
    assert entry["original_word"] == entry["candidate_word"] == "0x00402000"
    assert entry["observation"] == "target_word_agrees"
    assert entry["original_absolute_relocation"] is True
    assert entry["relocations"] == [{"symbol": "_method", "coff_type": 6, "source_address": "0x00402000"}]


def test_unbounded_wrong_target_is_actionable_but_not_a_whole_table_verdict():
    binding, objects, image = fixture(bounded=False)
    _, rows = check_generated((binding,), objects, image, {"_method": 0x403000})
    assert rows[0]["content"] == "unknown"
    entry, = rows[0]["diagnostic"]["entries"]
    assert entry["observation"] == "target_word_differs"
    assert entry["original_word"] == "0x00402000" and entry["candidate_word"] == "0x00403000"


def test_reverse_source_correlation_does_not_resolve_an_unmapped_emitted_target():
    binding, objects, image = fixture()
    external = {"_old_wrapper": 0x402000}
    targets, rows = check_generated((binding,), objects, image, external)
    entry, = rows[0]["diagnostic"]["entries"]
    assert entry["source_symbols_at_original_word"] == ["_old_wrapper"]
    assert entry["observation"] == "unresolved_source_target" and entry["candidate_word"] is None
    assert rows[0]["content"] == "unknown" and rows[0]["proof"]["references"] == "unknown"
    assert targets == {TABLE: 0x401000} and external == {"_old_wrapper": 0x402000}


def test_entry_diagnostics_apply_candidate_addends_not_original_words():
    binding, objects, image = fixture(target=0x402004)
    obj = objects["src/widget.cpp"]
    obj.sections = (replace(obj.sections[0], data=(4).to_bytes(4, "little")),)
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    assert rows[0]["diagnostic"]["entries"][0]["candidate_word"] == "0x00402004"
    assert rows[0]["content"] == "pass"
    obj.sections = (replace(obj.sections[0], data=(8).to_bytes(4, "little")),)
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    assert rows[0]["diagnostic"]["entries"][0]["observation"] == "target_word_differs"
    assert rows[0]["content"] == "fail"


def test_static_slot_name_cannot_borrow_an_external_target_binding():
    binding, objects, image = fixture()
    obj = objects["src/widget.cpp"]
    obj.symbols[1] = replace(obj.symbols[1], storage=3)
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    entry, = rows[0]["diagnostic"]["entries"]
    assert entry["observation"] == "unresolved_source_target"
    assert entry["relocations"][0]["source_address"] is None
    assert rows[0]["content"] == "unknown"


@pytest.mark.parametrize("mode,search", [(1, "no_library"), (2, "library"), (3, "alias")])
def test_weak_slot_exposes_linker_fallback_without_resolving_it(mode, search):
    binding, objects, image = fixture()
    obj = objects["src/widget.cpp"]
    obj.symbols[1] = replace(obj.symbols[1], storage=105)
    obj.symbols[2] = Symbol(2, "_fallback", 0, 0, 0x20, 2)
    obj.auxiliary = {1: struct.pack("<II10x", 2, mode)}
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000, "_fallback": 0x402000})
    entry, = rows[0]["diagnostic"]["entries"]
    assert entry["observation"] == "linker_selection_required" and entry["candidate_word"] is None
    assert entry["relocations"][0]["linker_fallback"] == {
        "symbol": "_fallback", "search": search, "selection": "unverified"}
    assert rows[0]["content"] == "unknown"


def test_bad_weak_slot_metadata_is_reported_not_guessed():
    binding, objects, image = fixture()
    obj = objects["src/widget.cpp"]
    obj.symbols[1] = replace(obj.symbols[1], storage=105)
    obj.auxiliary = {1: b"invalid"}
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    entry, = rows[0]["diagnostic"]["entries"]
    assert "metadata_error" in entry["relocations"][0]
    assert rows[0]["content"] == "unknown"


@pytest.mark.parametrize("damage", ["storage", "section", "value", "size", "reserved", "target", "mode"])
def test_shared_weak_metadata_parser_refuses_invalid_records(damage):
    from match_recovery.pe_coff import weak_external
    symbol = Symbol(1, "_weak", 0, 0, 0x20, 105)
    raw = struct.pack("<II10x", 2, 2)
    if damage == "storage": symbol = replace(symbol, storage=2)
    elif damage == "section": symbol = replace(symbol, section=1)
    elif damage == "value": symbol = replace(symbol, value=4)
    elif damage == "size": raw = raw[:-1]
    elif damage == "reserved": raw = raw[:-1] + b"x"
    elif damage == "target": raw = struct.pack("<II10x", 99, 2)
    else: raw = struct.pack("<II10x", 2, 4)
    obj = SimpleNamespace(symbols={1: symbol, 2: Symbol(2, "_fallback", 0, 0, 0x20, 2)}, auxiliary={1: raw})
    with pytest.raises(ValueError, match="weak-external"):
        weak_external(obj, symbol)


def test_unavailable_original_word_cannot_be_filled_from_a_candidate_binding():
    binding, objects, image = fixture(bounded=False)
    obj = objects["src/widget.cpp"]
    obj.sections = (replace(obj.sections[0], data=bytes(8), relocations=((0, 1, 6), (4, 1, 6))),)
    read = image.read
    def unavailable(span):
        if span.start > binding.origin.address:
            raise ValueError("synthetic unavailable image word")
        return read(span)
    image.read = unavailable
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    entry = rows[0]["diagnostic"]["entries"][1]
    assert entry["original_word"] is None and entry["candidate_word"] == "0x00402000"
    assert entry["observation"] == "unavailable"
    assert rows[0]["content"] == "unknown"


def test_diagnostics_do_not_read_past_a_declared_original_table_extent():
    binding, objects, image = fixture()
    obj = objects["src/widget.cpp"]
    obj.sections = (replace(obj.sections[0], data=bytes(8), relocations=((0, 1, 6), (4, 1, 6))),)
    read = image.read
    def bounded(span):
        assert span.end <= binding.span.end
        return read(span)
    image.read = bounded
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    entry = rows[0]["diagnostic"]["entries"][1]
    assert entry["within_declared_extent"] is False and entry["original_word"] is None
    assert rows[0]["content"] == "fail"


@pytest.mark.parametrize("relocations", [((0, 1, 6), (0, 1, 6)), ((1, 1, 6),), ((0, 1, 20),)])
def test_unsupported_slot_layout_never_passes(relocations):
    binding, objects, image = fixture()
    obj = objects["src/widget.cpp"]
    obj.sections = (replace(obj.sections[0], relocations=relocations),)
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    assert rows[0]["content"] == "unknown"
    assert rows[0]["diagnostic"]["relocation_layout_error"]


def test_partial_pointer_slot_is_not_a_valid_vtable_even_if_bytes_agree():
    binding, objects, image = fixture(raw=True)
    binding = replace(binding, span=Span(0x401000, 0x401003))
    obj = objects["src/widget.cpp"]
    obj.sections = (replace(obj.sections[0], data=obj.sections[0].data[:3]),)
    _, rows = check_generated((binding,), objects, image, {})
    assert rows[0]["content"] == "unknown"
    assert rows[0]["diagnostic"]["trailing_bytes"] == 3


def test_diagnostic_limit_does_not_narrow_whole_table_verification(monkeypatch):
    monkeypatch.setattr("match_recovery.generated_binary.MAX_ENTRY_DIAGNOSTICS", 1)
    binding, objects, image = fixture(raw=True)
    binding = replace(binding, span=Span(0x401000, 0x401008))
    obj = objects["src/widget.cpp"]
    obj.sections = (replace(obj.sections[0], data=bytes(4) + b"bad!", relocations=((0, 1, 6),)),)
    _, rows = check_generated((binding,), objects, image, {"_method": 0x402000})
    diagnostic = rows[0]["diagnostic"]
    assert len(diagnostic["entries"]) == diagnostic["omitted_entries"] == 1
    assert diagnostic["entries"][0]["observation"] == "target_word_agrees"
    assert rows[0]["content"] == "fail"


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
    assert rows[0]["diagnostic"]["entries"][0]["observation"] == "literal_without_relocation"


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
    import json
    (tmp_path / "recovery").mkdir()
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1, "source": {
        "image_pin": "src/IMAGE_PIN", "address_space": "pe:va"}}))
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
