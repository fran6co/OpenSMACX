"""Synthetic bytes only: no proprietary executable/object fixtures."""

from dataclasses import replace
import hashlib
import struct

import pytest

from match_recovery.binary import Relocation, RelocationKind, compare_bound_code
from match_recovery.component_binary import verify_component
from match_recovery.components import SourceFile, VendorBinding, VendorManifest
from match_recovery.model import Component, Delivery, Origin, Ownership, Result, Span
from match_recovery.pe_coff import Coff, PeImage


ABS = RelocationKind.ABSOLUTE32
REL = RelocationKind.RELATIVE32


def compare(original, candidate, relocations=(), targets=None, **kwargs):
    kwargs.setdefault("original_absolute_sites", frozenset(relocation.offset for relocation in relocations
                                                          if relocation.kind == ABS))
    return compare_bound_code(Span(0x401000, 0x401000 + len(original)), original, candidate,
                              relocations, targets or {}, **kwargs)


def test_wrong_global_is_not_hidden_by_relocation_mask():
    original = b"\xa1\x00\x20\x40\x00\xc3"
    candidate = b"\xa1\0\0\0\0\xc3"
    relocations = (Relocation(1, ABS, "global"),)
    assert compare(original, candidate, relocations, {"global": 0x402000})["result"] == Result.PASS
    wrong = compare(original, candidate, relocations, {"global": 0x403000})
    assert wrong["result"] == wrong["references"] == Result.FAIL
    assert compare(original, candidate, relocations)["result"] == Result.UNKNOWN


def test_hardcoded_original_address_is_not_a_resolved_global_reference():
    code = b"\xa1\x00\x20\x40\x00\xc3"
    report = compare(code, code, original_absolute_sites=frozenset({1}))
    assert report["bytes"] == Result.PASS
    assert report["references"] == report["result"] == Result.FAIL


def test_extra_candidate_absolute_relocation_changes_rebase_semantics():
    code = b"\xa1\x00\x20\x40\x00\xc3"
    report = compare(code, b"\xa1\0\0\0\0\xc3", (Relocation(1, ABS, "global"),),
                     {"global": 0x402000}, original_absolute_sites=frozenset())
    assert report["bytes"] == Result.PASS
    assert report["references"] == report["result"] == Result.FAIL


def test_unresolved_linker_constant_is_not_assumed_to_require_image_rebasing():
    code = b"\xa1\0\0\0\0\xc3"
    relocations = (Relocation(1, ABS, "unresolved_runtime_symbol"),)
    report = compare(code, code, relocations, original_absolute_sites=frozenset())
    assert report["result"] == report["references"] == Result.UNKNOWN
    assert report["extra_absolute_relocations"] == []
    assert report["unresolved_absolute_relocations"] == [1]
    # A declared image-address binding, even address zero, restores the strict
    # HIGHLOW requirement. No name-based exception or operand masking is used.
    report = compare(code, code, relocations, {"unresolved_runtime_symbol": 0},
                     original_absolute_sites=frozenset())
    assert report["result"] == Result.FAIL and report["extra_absolute_relocations"] == [1]


def test_members_use_candidate_addends_not_the_original_operand():
    original = b"\xa1\x90\x20\x40\x00\xc3"
    candidate = b"\xa1\x90\0\0\0\xc3"
    relocations = (Relocation(1, ABS, "object"),)
    assert compare(original, candidate, relocations, {"object": 0x402000})["result"] == Result.PASS
    wrong = candidate.replace(b"\x90", b"\x94")
    assert compare(original, wrong, relocations, {"object": 0x402000})["result"] == Result.FAIL


def test_relative_calls_apply_pc_bias_and_signed_addends():
    original = b"\xe8\xfb\xff\xff\xff\xc3"  # call the current function
    candidate = b"\xe8\0\0\0\0\xc3"
    relocs = (Relocation(1, REL, "self"),)
    assert compare(original, candidate, relocs, {"self": 0x401000})["result"] == Result.PASS
    candidate = b"\xe8\xfc\xff\xff\xff\xc3"  # symbol - 4
    assert compare(original, candidate, relocs, {"self": 0x401004})["result"] == Result.PASS


def test_unknown_relocation_does_not_hide_an_independent_opcode_failure():
    report = compare(b"\xa1\x00\x20\x40\x00\xc3", b"\xa3\0\0\0\0\xc3",
                     (Relocation(1, ABS, "missing"),))
    assert report["references"] == Result.UNKNOWN
    assert report["result"] == Result.FAIL


@pytest.mark.parametrize("candidate", [b"", b"\xc3\xc3", b"\xc3" + bytes(15), b"\xc3" + b"\x90" * 16])
def test_boundary_cannot_expand_to_fit_extra_code_or_data(candidate):
    assert compare(b"\xc3", candidate, alignment=16)["boundary"] == Result.FAIL


def test_only_exact_section_alignment_fill_is_permitted():
    assert compare(b"\xc3", b"\xc3" + b"\xcc" * 15, alignment=16)["result"] == Result.PASS
    assert compare(b"\xc3", b"\xc3\x90", alignment=16)["result"] == Result.FAIL
    # An immediate whose last byte is NOP is code, never stripped by byte value.
    code = b"\xb8\0\0\0\x90"
    assert compare(code, code)["result"] == Result.PASS


@pytest.mark.parametrize("kind", [ABS, REL])
@pytest.mark.parametrize("offset", [4, 8])
def test_valid_candidate_relocation_past_original_is_a_boundary_failure(kind, offset):
    report = compare(bytes(6), bytes(12), (Relocation(offset, kind, "target"),),
                     original_absolute_sites=frozenset())
    assert report["result"] == report["boundary"] == Result.FAIL
    assert report["bytes"] == report["references"] == Result.UNKNOWN
    assert report["original_size"] == 6 and report["candidate_size"] == 12
    assert report["candidate_relocations_outside_original"] == [offset]


@pytest.mark.parametrize("kind", [ABS, REL])
@pytest.mark.parametrize("offset", [0, 4])
def test_relocation_straddling_or_inside_alignment_fill_is_not_padding(kind, offset):
    report = compare(b"\xc3", b"\xc3" + b"\xcc" * 15,
                     (Relocation(offset, kind, "target"),), alignment=16,
                     original_absolute_sites=frozenset())
    assert report["result"] == report["boundary"] == Result.FAIL
    assert report["candidate_relocations_outside_original"] == [offset]


@pytest.mark.parametrize("original_size", [2, 8])
def test_malformed_candidate_relocations_are_refused_even_when_size_differs(original_size):
    with pytest.raises(ValueError, match="out-of-bound"):
        compare(bytes(original_size), bytes(4), (Relocation(1, REL, "target"),))
    with pytest.raises(ValueError, match="overlapping"):
        compare(bytes(original_size), bytes(6),
                (Relocation(1, REL, "a"), Relocation(2, REL, "b")))


def test_original_relocations_still_require_original_not_candidate_bounds():
    with pytest.raises(ValueError, match="original relocation"):
        compare(bytes(4), bytes(8), original_absolute_sites=frozenset({1}))


def test_ambiguous_and_out_of_bounds_relocations_are_refused():
    with pytest.raises(ValueError, match="overlapping"):
        compare(bytes(8), bytes(8), (Relocation(1, ABS, "a"), Relocation(2, REL, "b")))
    with pytest.raises(ValueError, match="out-of-bound"):
        compare(bytes(4), bytes(4), (Relocation(1, ABS, "a"),))
    with pytest.raises(ValueError, match="exactly"):
        compare_bound_code(Span(1, 3), b"a", b"a", (), {})


def coff_fixture(code=b"\xc3", *, relocation=None, storage=2):
    """One COMDAT and one external; ordinary little-endian COFF, no image bytes."""
    raw = code + b"\xcc" * (-len(code) % 16)
    reloc = struct.pack("<IIH", *relocation) if relocation is not None else b""
    symbols = (b"_f".ljust(8, b"\0") + struct.pack("<IhHBB", 0, 1, 0x20, storage, 0)
               + b"_g".ljust(8, b"\0") + struct.pack("<IhHBB", 0, 0, 0, 2, 0))
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, 60 + len(raw) + len(reloc), 2, 0, 0)
    section = b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(raw), 60,
                                               60 + len(raw) if reloc else 0, 0,
                                               int(bool(reloc)), 0, 0x60501020)
    return header + section + raw + reloc + symbols + struct.pack("<I", 4)


def pe_fixture(code=b"\xc3", *, absolute_site=None):
    """One raw-backed PE32 section with an optional synthetic relocation block."""
    data = bytearray(1024)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, 0x80)
    data[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", data, 0x84, 0x14C, 1, 0, 0, 0, 224, 0)
    struct.pack_into("<H", data, 0x98, 0x10B)
    struct.pack_into("<I", data, 0x98 + 28, 0x400000)
    struct.pack_into("<I", data, 0x98 + 92, 16)
    data[0x178:0x180] = b".text\0\0\0"
    struct.pack_into("<IIII", data, 0x180, 512, 0x1000, 512, 512)
    data[512:512 + len(code)] = code
    if absolute_site is not None:
        struct.pack_into("<II", data, 0x98 + 96 + 5 * 8, 0x1100, 12)
        struct.pack_into("<IIHH", data, 768, 0x1000, 12, 0x3000 + absolute_site, 0)
    return bytes(data)


@pytest.mark.parametrize("storage", [2, 3])
def test_coff_reads_external_and_static_comdats_with_exact_symbol_selection(storage):
    obj = Coff(coff_fixture(storage=storage))
    symbol = obj.function("_f")
    assert symbol.storage == storage
    code, relocs, alignment = obj.subject(symbol)
    assert code == b"\xc3" + b"\xcc" * 15
    assert alignment == 16
    assert not relocs
    with pytest.raises(ValueError, match="expected one"):
        obj.function("f")


def test_coff_keeps_relocation_identity_and_refuses_unsupported_kinds():
    obj = Coff(coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)))
    assert obj.subject(obj.function("_f"))[1] == (Relocation(1, REL, "1"),)
    assert obj.symbols[1].name == "_g"
    obj = Coff(coff_fixture(b"\xc3", relocation=(0, 1, 7)))
    with pytest.raises(ValueError, match="unsupported"):
        obj.subject(obj.function("_f"))
    with pytest.raises(ValueError, match="absent or auxiliary"):
        Coff(coff_fixture(relocation=(0, 2, 6)))


@pytest.mark.parametrize("length", [0, 10, 35, 60, 80])
def test_truncated_coff_inputs_do_not_produce_empty_success(length):
    with pytest.raises(ValueError):
        Coff(coff_fixture()[:length])


def test_pe_requires_pin_and_does_not_read_beyond_declared_backing():
    raw = pe_fixture(b"\xa1\0\x20\x40\0\xc3", absolute_site=1)
    image = PeImage(raw, hashlib.sha256(raw).hexdigest())
    span = Span(0x401000, 0x401006)
    assert image.read(span) == b"\xa1\0\x20\x40\0\xc3"
    assert image.absolute_sites(span) == frozenset({1})
    with pytest.raises(ValueError, match="digest"):
        PeImage(raw, "a" * 64)
    with pytest.raises(ValueError, match="backed"):
        image.read(Span(0x401000, 0x401300))
    with pytest.raises(ValueError, match="cuts through"):
        image.absolute_sites(Span(0x401002, 0x401006))


@pytest.mark.parametrize("absolute_site", [None, 0, 1, 4, 15])
def test_pe_relocation_index_preserves_every_overlap_boundary(absolute_site):
    raw = pe_fixture(absolute_site=absolute_site)
    image = PeImage(raw, hashlib.sha256(raw).hexdigest())
    for start in range(0x400FFC, 0x401018):
        for length in range(1, 10):
            span = Span(start, start + length)
            sites = {site for site in image.relocations if site < span.end and site + 4 > span.start}
            if any(not span.start <= site <= span.end - 4 for site in sites):
                with pytest.raises(ValueError, match="cuts through"):
                    image.absolute_sites(span)
            else:
                assert image.absolute_sites(span) == frozenset(site - span.start for site in sites)


def component_fixture(raw):
    origin = Origin(hashlib.sha256(raw).hexdigest(), "pe:va", 0x401000)
    return VendorManifest(Component("fixture", Ownership.THIRD_PARTY, Delivery.UPSTREAM_SOURCE,
                                    ("synthetic",), "1"), "vendor", "https://invalid.test/fixture",
                          "a" * 64, "f.c", (SourceFile("f.c", "f.c"),), "c", ("f.c",), ("/Gy",),
                          (VendorBinding(origin, (Span(origin.address, origin.address + 6),),
                                         "_f", "f.c", "synthetic"),))


def test_component_pipeline_checks_self_references_without_guessing(tmp_path):
    image = pe_fixture(b"\xe8\xfb\xff\xff\xff\xc3")
    manifest = component_fixture(image)
    compiles = []

    def compile_source(path, flags):
        compiles.append(path)
        return coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 0, 20))

    result = verify_component(tmp_path, manifest, image, compile_source)
    assert result["result"] == Result.PASS
    assert result["compile_attempts"] == len(compiles) == 1
    assert result["application_recoveries"] == 0
    assert result["structure_verification"] == Result.UNKNOWN
    # No serialized report/cache is required for the same answer on a new run.
    assert verify_component(tmp_path, manifest, image, compile_source) == result


def test_component_missing_external_and_compile_failure_remain_unknown(tmp_path):
    image = pe_fixture(b"\xe8\xfb\xff\xff\xff\xc3")
    manifest = component_fixture(image)
    result = verify_component(tmp_path, manifest, image,
                              lambda *_: coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)))
    assert result["result"] == Result.UNKNOWN
    assert result["functions"][0]["relocations"][0]["symbol"] == "_g"

    def failed(*_):
        raise ValueError("deliberate compiler failure")

    assert verify_component(tmp_path, manifest, image, failed)["result"] == Result.UNKNOWN
    stale = replace(manifest, bindings=(replace(manifest.bindings[0], symbol="_missing"),))
    assert verify_component(tmp_path, stale, image, lambda *_: coff_fixture())["result"] == Result.UNKNOWN


def test_source_mutation_during_compilation_invalidates_the_measurement(tmp_path):
    image = pe_fixture(b"\xe8\xfb\xff\xff\xff\xc3")
    manifest = component_fixture(image)
    source = tmp_path / "vendor" / "f.c"
    source.parent.mkdir()
    source.write_text("int f() { return 1; }\n")

    def compile_source(path, flags):
        path.write_text("int f() { return 2; }\n")
        return coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 0, 20))

    with pytest.raises(ValueError, match="changed during measurement"):
        verify_component(tmp_path, manifest, image, compile_source)
