"""Declared post-link image patches (`RECOVERY_IMAGE_PATCH`).

Hermetic: listings are built from bytes and the image side is supplied by a
patched `original_asm`, so no compiler or pinned image is needed.
"""

import pytest

from match_recovery.source import asm, read_file
from match_recovery.source.asm import SpanComparisonRefused, apply_image_patches
from match_recovery.source.image_patch import (ImagePatch, parse_image_patches,
                                                patches_for_spans)
from match_recovery.source.model import Tier

BASE = 0x00401000
# push esi; mov esi, ecx; push 0; push 0; mov ecx, esi; call rel32; pop esi; ret
PREFIX_CODE = bytes.fromhex("56" "8BF1")
CALL = bytes.fromhex("6A00" "6A00" "8BCE" "E800000000")       # 11 bytes at +3
SUFFIX_CODE = bytes.fromhex("5E" "C3")
COMPILED = PREFIX_CODE + CALL + SUFFIX_CODE
PATCH_START, PATCH_END = BASE + 3, BASE + 3 + len(CALL)
IMAGE = PREFIX_CODE + b"\x90" * len(CALL) + SUFFIX_CODE
CALL_FIELD = 3 + 7                                             # the rel32 field


def source(*declarations, span_end=BASE + len(IMAGE)):
    return ("".join(f"// RECOVERY_IMAGE_PATCH: v1 {text}\n" for text in declarations)
            + f"// ORIGINAL: 0x{BASE:08X} ?f@C@@QAEXXZ 0x{BASE:08X}-0x{span_end:08X}\n"
            "void C::f() {\n}\n")


def record_in(tmp_path, text):
    path = tmp_path / "x.cpp"
    path.write_text(text)
    return read_file(path)[0]


def compiled_listing(code=COMPILED):
    reference = asm.CodeReference(CALL_FIELD, True, "?exec@Popup@@QAEHHH@Z", 2, 0)
    return asm.Listing(code=code, base=BASE,
                       mask=frozenset(range(CALL_FIELD, CALL_FIELD + 4)),
                       references=(reference,))


def compare(tmp_path, monkeypatch, text, *, image=IMAGE, compiled=None, image_mask=frozenset()):
    record = record_in(tmp_path, text)
    monkeypatch.setattr(asm, "original_asm", lambda *args, **kwargs:
                        asm.Listing(code=image, base=BASE, mask=image_mask))
    monkeypatch.setattr(asm, "image_reader", lambda exe: None)
    return asm.compare_subject(record, "unused.exe", compiled or compiled_listing())


NOP = f"0x{PATCH_START:08X} 0x{PATCH_END:08X} nop"


# ---------------------------------------------------------------- grammar

def test_parse_both_forms_in_address_order():
    text = ("  // RECOVERY_IMAGE_PATCH: v1 0x0061874B 0x00618763 nop-jmp 0x0061875D 0x00618873\n"
            "// RECOVERY_IMAGE_PATCH: v1 0x00600545 0x0060054E nop\n")
    nop, jump = parse_image_patches(text)
    assert (nop.start, nop.end, nop.form, nop.line) == (0x600545, 0x60054E, "nop", 2)
    assert nop.expected == b"\x90" * 9
    assert jump.expected == (b"\x90" * 18 + b"\xE9" + (0x618873 - 0x618762).to_bytes(4, "little")
                             + b"\x90")
    assert jump.describe() == "0x0061874B-0x00618763 nop-jmp 0x0061875D 0x00618873"


@pytest.mark.parametrize("line", [
    "// RECOVERY_IMAGE_PATCH: v1 0x00600545 0x0060054E",
    "// RECOVERY_IMAGE_PATCH: v1 0x00600545 0x0060054E nops",
    "// RECOVERY_IMAGE_PATCH: v2 0x00600545 0x0060054E nop",
    "// RECOVERY_IMAGE_PATCH: v1 0x600545 0x60054E nop",
    "// RECOVERY_IMAGE_PATCH: v1 0x00600545 0x0060054E nop trailing",
    "// RECOVERY_IMAGE_PATCH: v1 0x0060054E 0x00600545 nop",
    "// RECOVERY_IMAGE_PATCH: v1 0x00600545 0x00600545 nop",
    # The five-byte jump must lie inside the span.
    "// RECOVERY_IMAGE_PATCH: v1 0x0061874B 0x00618763 nop-jmp 0x0061875F 0x00618873",
    "// RECOVERY_IMAGE_PATCH: v1 0x0061874B 0x00618763 nop-jmp 0x00618740 0x00618873",
])
def test_malformed_declarations_refuse(line):
    with pytest.raises(ValueError, match="image patch"):
        parse_image_patches(line + "\n")


def test_overlapping_declarations_refuse():
    with pytest.raises(ValueError, match="overlap"):
        parse_image_patches("// RECOVERY_IMAGE_PATCH: v1 0x00600545 0x0060054E nop\n"
                            "// RECOVERY_IMAGE_PATCH: v1 0x0060054D 0x00600550 nop\n")


def test_patch_crossing_a_mapped_span_boundary_refuses():
    patch = ImagePatch(0x401006, 0x40100A, "nop", 1)
    assert patches_for_spans((patch,), ((0x401000, 0x401010),)) == (patch,)
    assert patches_for_spans((patch,), ((0x402000, 0x402010),)) == ()
    with pytest.raises(ValueError, match="crosses the boundary"):
        patches_for_spans((patch,), ((0x401000, 0x401008),))


# ------------------------------------------------------------- comparison

def test_without_a_declaration_the_patched_image_is_a_mismatch(tmp_path, monkeypatch):
    result = compare(tmp_path, monkeypatch, source())
    assert result.verdict is not Tier.BYTE_EXACT
    assert result.image_patches == ()


def test_declared_nop_patch_is_exact_and_says_so(tmp_path, monkeypatch):
    result = compare(tmp_path, monkeypatch, source(NOP))
    assert result.verdict is Tier.BYTE_EXACT
    (applied,) = result.image_patches
    assert applied["start"] == f"0x{PATCH_START:08X}" and applied["end"] == f"0x{PATCH_END:08X}"
    assert applied["form"] == "nop" and applied["image_bytes"] == "90" * len(CALL)
    assert applied["compiled_bytes_not_compared"] == CALL.hex()
    assert len(applied["compiled_instructions_not_compared"]) == 4
    assert applied["compiled_relocations_not_compared"] == ["?exec@Popup@@QAEHHH@Z"]
    assert "declared post-link image patch" in result.diagnostic


def test_applying_drops_the_relocation_inside_the_span():
    patch = ImagePatch(PATCH_START, PATCH_END, "nop", 1)
    patched, applied, notes = apply_image_patches(
        (patch,), asm.Listing(code=IMAGE, base=BASE), compiled_listing())
    assert patched.code == IMAGE and patched.mask == frozenset() and patched.references == ()
    assert len(applied) == 1 and notes == ()


def test_span_that_does_not_cover_the_image_nops_is_not_exact(tmp_path, monkeypatch):
    # One byte short on the right: the image's last nop is outside the span,
    # and the compiled call straddles the declared edge.
    short = f"0x{PATCH_START:08X} 0x{PATCH_END - 1:08X} nop"
    result = compare(tmp_path, monkeypatch, source(short))
    assert result.verdict is not Tier.BYTE_EXACT
    assert result.image_patches == ()
    assert "not applied" in result.diagnostic


def test_span_wider_than_the_image_patch_refuses(tmp_path, monkeypatch):
    # One byte too far on the left: the image holds `mov esi, ecx` there.
    wide = f"0x{PATCH_START - 1:08X} 0x{PATCH_END:08X} nop"
    with pytest.raises(SpanComparisonRefused, match="does not describe the image"):
        compare(tmp_path, monkeypatch, source(wide))


def test_non_nop_image_bytes_refuse(tmp_path, monkeypatch):
    image = bytearray(IMAGE)
    image[5] = 0xCC
    with pytest.raises(SpanComparisonRefused, match="does not describe the image"):
        compare(tmp_path, monkeypatch, source(NOP), image=bytes(image))


def test_stale_image_relocations_must_meet_compiled_absolute_fields(tmp_path, monkeypatch):
    # The image's relocation table still names four bytes of the span, and
    # the compiled call there is REL32: the declared code is not what the
    # linker relocated, so the patch is not applied.
    stale = frozenset({5, 6, 7, 8})
    result = compare(tmp_path, monkeypatch, source(NOP), image_mask=stale)
    assert result.verdict is not Tier.BYTE_EXACT and result.image_patches == ()
    assert "keeps base relocations" in result.diagnostic

    # push 0; push 0; push abs32; pop eax; pop ecx  - same eleven bytes long,
    # with a DIR32 field exactly on the stale entry.
    body = bytes.fromhex("6A00" "68" "00000000" "58" "59" "9090")
    assert len(body) == len(CALL)
    reference = asm.CodeReference(3 + 3, False, "?StringTemp@@3PADA", 2, 0)
    compiled = asm.Listing(code=PREFIX_CODE + body + SUFFIX_CODE, base=BASE,
                           mask=frozenset(range(6, 10)), references=(reference,))
    stale = frozenset({6, 7, 8, 9})
    result = compare(tmp_path, monkeypatch, source(NOP), image_mask=stale, compiled=compiled)
    assert result.verdict is Tier.BYTE_EXACT
    assert result.image_patches[0]["image_stale_base_relocation_bytes"] == [
        f"0x{BASE + offset:08X}" for offset in sorted(stale)]

    # The same compiled span against an image with no stale entry is refused
    # too: an absolute field the image's table never knew about.
    result = compare(tmp_path, monkeypatch, source(NOP), compiled=compiled)
    assert result.verdict is not Tier.BYTE_EXACT and result.image_patches == ()


def test_compiled_instruction_straddling_the_left_edge_is_not_applied(tmp_path, monkeypatch):
    # mov esi, ecx becomes a three-byte `mov esi, [ecx+0]`-style instruction
    # that runs one byte into the declared span.
    code = bytes.fromhex("56" "8B7100") + CALL[1:] + SUFFIX_CODE
    assert len(code) == len(COMPILED)
    result = compare(tmp_path, monkeypatch, source(NOP),
                     compiled=asm.Listing(code=code, base=BASE))
    assert result.verdict is not Tier.BYTE_EXACT
    assert result.image_patches == ()
    assert "no run of complete compiled instructions covers exactly that span" in result.diagnostic


def test_compiled_instruction_straddling_the_right_edge_is_not_applied(tmp_path, monkeypatch):
    # The span's last five bytes begin a six-byte `call dword ptr [...]`
    # whose final byte lies past the declared end.
    code = PREFIX_CODE + bytes.fromhex("6A00" "6A00" "8BCE") + bytes.fromhex("FF1500000000") + b"\xC3"
    assert len(code) == len(COMPILED)
    result = compare(tmp_path, monkeypatch, source(NOP),
                     compiled=asm.Listing(code=code, base=BASE))
    assert result.verdict is not Tier.BYTE_EXACT
    assert result.image_patches == ()
    assert "not applied" in result.diagnostic


def test_bytes_differing_outside_the_span_still_fail(tmp_path, monkeypatch):
    code = bytes.fromhex("57") + COMPILED[1:]                 # push edi, not push esi
    result = compare(tmp_path, monkeypatch, source(NOP), compiled=compiled_listing(code))
    assert result.verdict is not Tier.BYTE_EXACT
    assert len(result.image_patches) == 1                     # applied, and still not exact
    assert "exact outside" not in result.diagnostic


def test_declaration_past_the_compared_span_refuses(tmp_path, monkeypatch):
    # The mapped span is longer than what the image listing holds once its
    # trailing padding is stripped; a patch there cannot be checked.
    beyond = f"0x{BASE + len(IMAGE):08X} 0x{BASE + len(IMAGE) + 2:08X} nop"
    with pytest.raises(SpanComparisonRefused, match="outside the compared original span"):
        compare(tmp_path, monkeypatch, source(beyond, span_end=BASE + len(IMAGE) + 4))


def test_malformed_declaration_in_the_host_refuses_the_record(tmp_path, monkeypatch):
    with pytest.raises(SpanComparisonRefused, match="declared image patch refused"):
        compare(tmp_path, monkeypatch, source(NOP + " extra"))


def test_declared_jump_patch_checks_the_target(tmp_path, monkeypatch):
    # cmp eax, 1; jne +5; call rel32  ->  7 nops, jmp target, 1 nop (13 bytes)
    original_code = bytes.fromhex("83F801" "7505" "E800000000" "909090")
    start, end, jmp_at = BASE + 3, BASE + 3 + 13, BASE + 3 + 7
    target = BASE + 3 + 13 + 1
    patch = ImagePatch(start, end, "nop-jmp", 1, jmp_at, target)
    image = PREFIX_CODE + patch.expected + SUFFIX_CODE
    compiled = asm.Listing(code=PREFIX_CODE + original_code + SUFFIX_CODE, base=BASE)
    good = f"0x{start:08X} 0x{end:08X} nop-jmp 0x{jmp_at:08X} 0x{target:08X}"
    text = source(good, span_end=BASE + len(image))
    result = compare(tmp_path, monkeypatch, text, image=image, compiled=compiled)
    assert result.verdict is Tier.BYTE_EXACT
    assert result.image_patches[0]["jmp_target"] == f"0x{target:08X}"
    wrong = f"0x{start:08X} 0x{end:08X} nop-jmp 0x{jmp_at:08X} 0x{target + 4:08X}"
    with pytest.raises(SpanComparisonRefused, match="does not describe the image"):
        compare(tmp_path, monkeypatch, source(wrong, span_end=BASE + len(image)),
                image=image, compiled=compiled)
