from dataclasses import replace
import hashlib
import struct
from types import SimpleNamespace

import pytest

from match_recovery.generated_binary import check_generated, definition_absent, generated_coverage
from match_recovery.generated_source import (GeneratedBinding, add_generated, format_generated,
                                             parse_generated, read_generated)
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


def vbtable_fixture(*, bounded=True, candidate=None, absolute_site=None, symbol="??_8Widget@@7B@",
                    relocations=()):
    import struct

    words = struct.pack("<ii", -4, 12)
    pin = hashlib.sha256(pe_fixture(words, absolute_site=absolute_site)).hexdigest()
    image = PeImage(pe_fixture(words, absolute_site=absolute_site), pin)
    binding = GeneratedBinding(Origin(pin, "pe:va", 0x401000),
                               Span(0x401000, 0x401008) if bounded else None,
                               symbol, Origin(pin, "pe:va", 0x401020), "src/widget.h", 1, "vbtable")
    symbols = {0: Symbol(0, symbol, 0, 1, 0, 2)}
    section = Section(words if candidate is None else candidate, relocations, 0x40301040, ".rdata")
    return binding, {"src/widget.cpp": SimpleNamespace(symbols=symbols, sections=(section,))}, image


def test_vbtable_compares_bounded_signed_offset_bytes_without_relocations():
    binding, objects, image = vbtable_fixture()
    targets, rows = check_generated((binding,), objects, image, {})
    row, = rows
    assert targets == {binding.symbol: binding.origin.address}
    assert row["definition"] == row["content"] == "pass"
    assert row["diagnostic"]["original_signed_offsets"] == [-4, 12]
    assert row["diagnostic"]["candidate_signed_offsets"] == [-4, 12]
    assert row["diagnostic"]["original_absolute_relocations"] == []
    assert row["proof"]["references"] == "pass"
    assert binding.interpretation["kind"] == "compiler_generated_vbtable"


def test_vbtable_wrong_word_and_original_absolute_relocation_fail_content():
    binding, objects, image = vbtable_fixture(candidate=struct.pack("<ii", -4, 13))
    _, rows = check_generated((binding,), objects, image, {})
    assert rows[0]["content"] == "fail" and rows[0]["proof"]["bytes"] == "fail"

    binding, objects, image = vbtable_fixture(absolute_site=0)
    _, rows = check_generated((binding,), objects, image, {})
    assert rows[0]["content"] == "fail"
    assert rows[0]["diagnostic"]["original_absolute_relocations"] == [0]


@pytest.mark.parametrize("candidate,relocations", [(b"", ()), (b"\0\0\0", ()), (bytes(8), ((0, 1, 6),))])
def test_vbtable_partial_words_and_candidate_relocations_are_refused(candidate, relocations):
    binding, objects, image = vbtable_fixture(candidate=candidate, relocations=relocations)
    with pytest.raises(ValueError, match="at least one complete|complete signed|without COFF relocations"):
        check_generated((binding,), objects, image, {})


def test_vbtable_requires_aligned_original_start_and_four_byte_coff_alignment():
    binding, objects, image = vbtable_fixture()
    misaligned = replace(binding, origin=Origin(binding.origin.image_sha256, "pe:va", 0x401001),
                         span=Span(0x401001, 0x401009))
    with pytest.raises(ValueError, match="original start must be four-byte aligned"):
        check_generated((misaligned,), objects, image, {})

    obj = objects["src/widget.cpp"]
    obj.sections = (replace(obj.sections[0], flags=0x40201040),)  # encoded two-byte alignment
    with pytest.raises(ValueError, match="alignment must be at least four bytes"):
        check_generated((binding,), objects, image, {})

    obj.sections = (replace(obj.sections[0], flags=0x40001040),)  # no alignment encoding
    with pytest.raises(ValueError, match="missing or unsupported COFF section alignment"):
        check_generated((binding,), objects, image, {})


@pytest.mark.parametrize("symbol", ["??_8Bad Name@@", "??_8Bad\\Name@@", "??_8Bad/Name@@",
                                     "??_8Bad??=Name@@", "??_8RECOVERY_GENERATED:Name@@",
                                     "??_8Bad\x7fName@@"])
def test_generated_symbols_must_be_single_annotation_safe_tokens(symbol):
    binding, _, _ = vbtable_fixture()
    with pytest.raises(ValueError, match="annotation-safe token"):
        replace(binding, symbol=symbol)


def test_vbtable_requires_vbtable_symbol_kind_and_unknown_extent_stays_unknown():
    binding, _, _ = vbtable_fixture()
    with pytest.raises(ValueError, match="vbtable interpretation requires an exact \\?\\?_8"):
        replace(binding, symbol=TABLE)
    with pytest.raises(ValueError, match="vtable interpretation requires an exact \\?\\?_7"):
        replace(fixture()[0], symbol="??_8Widget@@7B@")

    binding, objects, image = vbtable_fixture(bounded=False)
    targets, rows = check_generated((binding,), objects, image, {})
    assert targets == {binding.symbol: binding.origin.address}
    assert rows[0]["definition"] == "pass" and rows[0]["content"] == "unknown"
    assert rows[0]["diagnostic"]["original_extent"] is None
    assert rows[0]["diagnostic"]["original_signed_offsets"] is None
    assert rows[0]["diagnostic"]["candidate_signed_offsets"] == [-4, 12]


def test_vbtable_nonemission_remains_a_source_symbol_absence_witness():
    binding, objects, _ = vbtable_fixture()
    objects["src/caller.cpp"] = SimpleNamespace(symbols={}, sections=())
    rows = generated_coverage((binding,), objects)
    row, = [row for row in rows if row["unit"] == "src/caller.cpp"]
    assert row["observation"] == "not_emitted" and definition_absent(row["witness"], source_kind="vbtable")
    assert not definition_absent(row["witness"])


def test_generated_writer_is_add_only_and_roundtrips_new_kind_without_rewriting_existing():
    binding, _, _ = vbtable_fixture()
    text = "// existing source\r\n"
    formatted = format_generated(binding)
    assert formatted.startswith("// RECOVERY_GENERATED: v1 vbtable ")
    updated = add_generated(text, binding)
    assert updated.startswith(text) and updated.endswith(formatted + "\n")
    parsed, = parse_generated(updated, binding.path, binding.origin.image_sha256, address_space="pe:va")
    assert parsed.interpretation_id == binding.interpretation_id
    with pytest.raises(ValueError, match="already exists"):
        add_generated(updated, binding)


@pytest.mark.parametrize("text", ["/* unterminated", 'R"tag(unterminated', "#define VALUE x \\",
                                 "source ??/", "/* closed */ #define VALUE x \\"])
def test_generated_writer_refuses_unsafe_eof_continuations(text):
    binding, _, _ = vbtable_fixture()
    with pytest.raises(ValueError, match="unterminated block comment or raw string|continued source line or macro"):
        add_generated(text, binding)


def test_generated_writer_can_terminate_a_final_line_comment_before_appending():
    binding, _, _ = vbtable_fixture()
    updated = add_generated("// final line comment", binding)
    assert updated.startswith("// final line comment\n// RECOVERY_GENERATED:")


def test_legacy_vtable_interpretation_identity_remains_unchanged():
    from dataclasses import asdict
    from match_recovery.model import digest

    binding, _, _ = fixture()
    legacy_value = {"kind": "compiler_generated_vtable", "origin": asdict(binding.origin),
                    "span": asdict(binding.span), "symbol": binding.symbol,
                    "original_evidence": asdict(binding.evidence)}
    assert binding.source_kind == "vtable"
    assert binding.interpretation == legacy_value
    assert binding.interpretation_id == digest({"entity_id": binding.evidence.entity_id, "value": legacy_value})


@pytest.mark.parametrize("undefined", [False, True])
def test_nonemitting_object_coverage_uses_its_actual_symbols(undefined):
    binding, objects, image = fixture()
    objects["src/caller.cpp"] = SimpleNamespace(symbols={
        0: Symbol(0, TABLE, 0, 0, 0, 2)} if undefined else {}, sections=())
    row, = generated_coverage((binding,), objects)
    assert row["unit"] == "src/caller.cpp" and row["observation"] == "not_emitted"
    assert row["interpretation_id"] == binding.interpretation_id
    assert definition_absent(row["witness"])
    targets, emitted = check_generated((binding,), objects, image, {"_method": 0x402000})
    assert targets == {TABLE: 0x401000} and len(emitted) == 1
    assert emitted[0]["unit"] == "src/widget.cpp" and emitted[0]["content"] == "pass"
    assert generated_coverage((binding,), {}) == []  # No object is not an absent definition.


@pytest.mark.parametrize("fields", [dict(storage=3), dict(storage=105), dict(section=-1),
                                   dict(value=4), dict(type=0x20)])
def test_nonordinary_symbols_never_attest_generated_definition_absence(fields):
    binding, objects, _ = fixture()
    objects["src/dependency.cpp"] = SimpleNamespace(symbols={
        0: replace(Symbol(0, TABLE, 0, 0, 0, 2), **fields)}, sections=())
    row, = generated_coverage((binding,), objects)
    assert row["observation"] == "unsupported_symbol" and not definition_absent(row["witness"])


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
