"""Typed local debug references never authorize discarding an unknown child."""
from dataclasses import replace
import json
import struct

import pytest

from match_recovery.associated_binary import compare_associated_code
from match_recovery.codeview_procedures import (decode_procedure, S_LDATA32_ST,
                                               S_LABEL32_ST)
from match_recovery.linked_codeview import source_procedure, procedure_correspondence
from match_recovery.model import Result
from match_recovery.linked_fpo import validate_fpo_object
from match_recovery.pe_coff import Coff
from test_associated_binary import fixture, _with_fpo_child, BODY, GAP
from test_codeview_procedures import procedure, name


def payload(**options):
    data_name = options.get("data_name", "")
    return procedure(length=len(BODY) + len(GAP) + 5, debug_start=0,
                     debug_end=len(BODY), label="_f", locals_=[
        (S_LDATA32_ST, struct.pack("<IIH", options.get("data_type", 0),
                                  options.get("addend", 0), 0) + name(data_name)),
        (S_LABEL32_ST, struct.pack("<IHB", 0, 0, options.get("flags", 0))
         + name(options.get("label_name", ".Lcase")))])


def with_debug(raw, data):
    """Append physical CV metadata without changing existing symbol identities."""
    obj = Coff(raw)
    count = len(obj.sections)
    symbol_at, symbol_count = struct.unpack_from("<II", raw, 8)
    old_header_end = 20 + 40 * count
    body = raw[old_header_end:symbol_at]
    symbols_end = symbol_at + 18 * symbol_count
    symbols, strings = raw[symbol_at:symbols_end], raw[symbols_end:]
    decoded = decode_procedure(data, local_addresses=True)
    relocations = [(32, 0, 11), (36, 0, 10)]
    for address in decoded.addresses:
        at = address.record_offset + (8 if address.kind == S_LDATA32_ST else 4)
        target = 3 if address.kind == S_LDATA32_ST else 5
        relocations.extend(((at, target, 11), (at + 4, target, 10)))
    reloc_blob = b"".join(struct.pack("<IIH", *row) for row in relocations)
    data_at = symbol_at + 40
    reloc_at = data_at + len(data)
    new_symbols_at = reloc_at + len(reloc_blob)
    header = bytearray(raw[:20])
    struct.pack_into("<H", header, 2, count + 1)
    struct.pack_into("<II", header, 8, new_symbols_at, symbol_count + 2)
    headers = bytearray(raw[20:old_header_end])
    for at in range(0, len(headers), 40):
        for field in (20, 24, 28):
            value, = struct.unpack_from("<I", headers, at + field)
            if value:
                struct.pack_into("<I", headers, at + field, value + 40)
    child_header = b".debug$S" + struct.pack("<IIIIIIHHI", 0, 0, len(data),
        data_at, reloc_at, 0, len(relocations), 0, 0x42101048)
    child_symbol = b".debug$S" + struct.pack("<IhHBB", 0, count + 1, 0, 3, 1)
    auxiliary = struct.pack("<IHHIHB3x", len(data), len(relocations), 0, 0, 1, 5)
    return bytes(header + headers + child_header + body + data + reloc_blob
                 + symbols + child_symbol + auxiliary + strings)


def case(*, fpo=False, **options):
    obj, subject, span, image, bindings, targets = fixture()
    raw = _with_fpo_child(obj.raw_data) if fpo else obj.raw_data
    candidate = Coff(with_debug(raw, payload(**options)))
    return candidate, candidate.function(subject.name), span, image, bindings, targets


@pytest.mark.parametrize("fpo", [False, True])
def test_complete_local_debug_metadata_is_separate_from_optional_fpo(fpo):
    args = case(fpo=fpo)
    report = compare_associated_code(*args)
    assert report["result"] == Result.PASS
    assert bool(report["associated_metadata"]) == fpo
    assert len(report["source_debug_metadata"]) == 1
    assert report["original_metadata"] == report["linked_selection"] == report["runtime"] == Result.UNKNOWN
    # Reports must remain JSON-safe, including exact record padding.
    json.dumps(report)


def test_addressed_decoding_is_opt_in_and_linked_correspondence_stays_refused():
    data = payload()
    with pytest.raises(ValueError, match="unsupported CodeView procedure record"):
        decode_procedure(data)
    parsed = decode_procedure(data, local_addresses=True)
    assert [row.kind for row in parsed.addresses] == [S_LDATA32_ST, S_LABEL32_ST]
    assert parsed.addresses[0].type_index == 0
    assert parsed.addresses[1].name == ".Lcase"
    with pytest.raises(ValueError, match="local address correspondence"):
        procedure_correspondence(parsed, parsed, (), ())
    obj, root, *_ = case()
    with pytest.raises(ValueError):
        source_procedure(obj, 2, root)


@pytest.mark.parametrize("cut", range(len(payload())))
def test_every_addressed_procedure_truncation_refuses(cut):
    with pytest.raises(ValueError):
        decode_procedure(payload()[:cut], local_addresses=True)


@pytest.mark.parametrize("options", [{"addend": 1}, {"data_type": 0x74},
    {"data_name": "static"}, {"flags": 1}, {"label_name": "other"}])
def test_unsupported_local_record_semantics_never_pass(options):
    with pytest.raises(ValueError):
        compare_associated_code(*case(**options))


@pytest.mark.parametrize("damage", ["unknown_child", "flags", "relocation_kind", "split_pair",
    "duplicate_target", "outside_root", "outside_extent", "alias", "incoming", "extra_relocation"])
def test_local_metadata_ownership_damage_refuses(damage):
    obj, root, *rest = case()
    child = obj.sections[1]
    relocations = list(child.relocations)
    if damage == "unknown_child":
        obj.sections = (obj.sections[0], replace(child, name=".xdata$x"))
    elif damage == "flags":
        obj.sections = (obj.sections[0], replace(child, flags=0x62501040))
    elif damage == "relocation_kind":
        relocations[2] = (*relocations[2][:2], 6)
    elif damage == "split_pair":
        relocations[3] = (relocations[3][0], 5, 10)
    elif damage == "duplicate_target":
        relocations[4:] = [(offset, 3, kind) for offset, _, kind in relocations[4:]]
    elif damage == "outside_root":
        obj.symbols[5] = replace(obj.symbols[5], section=2)
    elif damage == "outside_extent":
        obj.symbols[5] = replace(obj.symbols[5], value=29)
    elif damage == "alias":
        obj.symbols[99] = replace(obj.symbols[5], index=99, name="alias")
    elif damage == "incoming":
        obj.sections = (replace(obj.sections[0], relocations=(*obj.sections[0].relocations, (0, 6, 6))), child)
    else:
        relocations.append((1, 0, 11))
        raw = bytearray(obj.auxiliary[6]); struct.pack_into("<H", raw, 4, len(relocations)); obj.auxiliary[6] = bytes(raw)
    if relocations != list(child.relocations):
        obj.sections = (obj.sections[0], replace(obj.sections[1], relocations=tuple(relocations)))
    with pytest.raises(ValueError):
        compare_associated_code(obj, root, *rest)


def test_root_referencing_fpo_cannot_lose_its_physical_association():
    obj, root, *rest = case(fpo=True)
    del obj.associations[2]
    with pytest.raises(ValueError, match="FPO metadata"):
        compare_associated_code(obj, root, *rest)


def test_fpo_sibling_opt_in_runs_complete_source_codeview_proof():
    obj, root, *_ = case(fpo=True)
    with pytest.raises(ValueError):
        validate_fpo_object(obj, 2, root)
    assert validate_fpo_object(obj, 2, root, source_codeview=True)["code_size"] == 29
    obj.sections = (*obj.sections[:2], replace(obj.sections[2], flags=0x62501040))
    with pytest.raises(ValueError):
        validate_fpo_object(obj, 2, root, source_codeview=True)


def user_label_case(coff_name):
    obj, root, *rest = case(label_name="select_page_2")
    obj.symbols[5] = replace(obj.symbols[5], name=coff_name)
    return (obj, root, *rest)


def test_vc6_decorated_user_label_matches_its_codeview_identifier():
    # VC6 names a goto/__asm label `$<identifier>$<serial>` in COFF while
    # S_LABEL32 keeps the bare identifier (observed in real VC6 objects, e.g.
    # `$failed$51461` / `failed`, `$count_loop$60502` / `count_loop`).
    report = compare_associated_code(*user_label_case("$select_page_2$61732"))
    assert report["result"] == Result.PASS


@pytest.mark.parametrize("coff_name", ["$select_page_2$", "$select_page_2$0617",
    "$select_page_2$61x", "$select_page_2$61$2", "$select_page$61732", "select_page_2$61732",
    "$$select_page_2$61732", "$select_page_2$-1", "$L61732"])
def test_other_user_label_spellings_never_pass(coff_name):
    with pytest.raises(ValueError, match="unique owned root label"):
        compare_associated_code(*user_label_case(coff_name))


def test_label_name_rule_is_exact_or_vc6_decoration():
    from match_recovery.linked_codeview import label_names_agree
    assert label_names_agree("$L52070", "$L52070")
    assert label_names_agree("failed", "$failed$51461")
    assert not label_names_agree("$L52070", "$$L52070$1")
    assert not label_names_agree("a.b", "$a.b$1")
    assert not label_names_agree("", "$$1")
