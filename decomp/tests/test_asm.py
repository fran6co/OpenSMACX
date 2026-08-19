"""Tests for decomp.asm.

The hermetic half runs anywhere: fact reading, span extraction, the COFF
reader against a hand-built object, and the clear refusals. The other half
needs the recovery environment (pinned exe, wine, VC6, a configured build)
and skips itself on a box without it; the end-to-end agreement check is a
smoke test, with the ratchet in tools/ keeping the strict machinery.
"""

import shutil
import struct
from pathlib import Path

import pytest

from decomp import read_file
from decomp import asm
from decomp.asm import (_coff_function, compare_asm, compiled_asm,
                        original_asm)
from decomp.reader import REPO_ROOT

# The environment's facts live HERE, not in the package: where the pinned
# image and the build database are on this box. The compiler itself is the
# build's own business - the compile command names it.
EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
COMPILE_COMMANDS = REPO_ROOT / "build" / "compile_commands.json"
# THE FLAG SETS BELONG TO THE CALLER, and these tests are one. `/O2` implies
# `/Oy`, which omits the frame pointer, and the shipped image is mixed - so
# which invocation reproduces a given function is a property of the function,
# and asking only the first of these answers about half the image. They are
# not a build setting either: `/Oy-` appears nowhere in this tree's CMake
# configuration, which is a Debug database of `/Od /Ob0`.
FLAGS = "/c /O2 /Gy /GR- /Oy- /GX"
FRAMELESS_FLAGS = "/c /O2 /Gy /GR- /GX"
FLAG_SETS = (FLAGS, FRAMELESS_FLAGS,
             "/c /O1 /Gy /GR- /Oy- /GX", "/c /O1 /Gy /GR- /GX")

HAVE_EXE = EXE.is_file()
HAVE_BUILD = COMPILE_COMMANDS.is_file() and shutil.which("wine") is not None

MARKED = """// ORIGINAL: 0x00401000 ?f@C@@QAEXXZ 0x00401000-0x00401008
// size      8 bytes
void C::f() {
}
"""

NO_FACTS = """// ORIGINAL: 0x00401000
void f() {
}
"""


def record_in(tmp_path, text):
    """A record anchored to a real file - asm functions read record.path."""
    path = tmp_path / "x.cpp"
    path.write_text(text)
    return read_file(path)[0]


def test_record_carries_name_and_spans(tmp_path):
    record = record_in(tmp_path, MARKED)
    assert record.name == "?f@C@@QAEXXZ"
    assert record.image_spans == ((0x00401000, 0x00401008),)


def test_span_refused_without_the_fact(tmp_path):
    with pytest.raises(ValueError, match="no spans"):
        original_asm(record_in(tmp_path, NO_FACTS),
                     Path("/nonexistent/image.exe"))


def test_original_asm_refused_without_the_exe(tmp_path):
    with pytest.raises(ValueError, match="no executable"):
        original_asm(record_in(tmp_path, MARKED),
                     Path("/nonexistent/image.exe"))


def test_compiled_asm_refused_without_the_name_fact(tmp_path):
    with pytest.raises(ValueError, match="no name"):
        compiled_asm(record_in(tmp_path, NO_FACTS), COMPILE_COMMANDS, FLAGS)


# --------------------------------------------------------------- COFF reader


def tiny_coff(code: bytes, symbol: str) -> bytes:
    """A minimal one-section COFF object carrying one external .text symbol."""
    raw_ptr = 20 + 40
    sym_ptr = raw_ptr + len(code)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, sym_ptr, 1, 0, 0)
    section = struct.pack("<8sIIIIIIHHI", b".text\x00\x00\x00", 0, 0,
                          len(code), raw_ptr, 0, 0, 0, 0, 0x60000020)
    sym_bytes = symbol.encode() + b"\x00"
    entry = (b"\x00\x00\x00\x00" + struct.pack("<I", 4)
             + struct.pack("<IhHBB", 0, 1, 0, 2, 0))
    strtab = struct.pack("<I", 4 + len(sym_bytes)) + sym_bytes
    return header + section + code + entry + strtab


def test_coff_function_extracts_and_strips_padding():
    code = b"\x55\x8B\xEC\xC3"
    obj = tiny_coff(code + b"\xCC\xCC\x90\x90", "?f@@YAXXZ")
    assert _coff_function(obj, "?f@@YAXXZ") == code


def test_coff_function_refuses_an_absent_symbol():
    obj = tiny_coff(b"\xC3", "?f@@YAXXZ")
    with pytest.raises(ValueError, match="not found"):
        _coff_function(obj, "?g@@YAXXZ")


# ------------------------------------------------------- listing comparison

LISTING = [
    "0x00401000  55                        push ebp",
    "0x00401001  8B EC                     mov ebp, esp",
    "0x00401003  B8 01 00 00 00              mov eax, 1",
    "0x00401008  C3                        ret",
]


def test_compare_byte_exact():
    result = compare_asm(LISTING, list(LISTING))
    assert result.verdict == "BYTE_EXACT"
    assert result.original_count == result.compiled_count == 4
    assert result.matching_lines == 4
    assert result.mnemonic_similarity == 1.0
    assert result.first_divergence is None


def test_compare_mnemonic_only():
    compiled = list(LISTING)
    compiled[2] = "0x00401003  B8 02 00 00 00              mov eax, 2"
    result = compare_asm(LISTING, compiled)
    assert result.verdict == "MNEMONIC_ONLY"
    assert result.matching_lines == 3
    assert result.mnemonic_similarity == 1.0
    assert result.first_divergence == 2


def test_compare_mismatch_reports_the_divergence():
    compiled = list(LISTING)
    compiled[1] = "0x00401001  33 C0                     xor eax, eax"
    result = compare_asm(LISTING, compiled)
    assert result.verdict == "MISMATCH"
    assert result.first_divergence == 1
    assert result.mnemonic_similarity < 1.0
    assert any("xor" in line for line in result.context[1])


def test_compare_ret_keeps_its_pop_count():
    # `ret` and `ret 4` are different instructions; the sequence must say so.
    a = ["0x00401000  C3                        ret"]
    b = ["0x00401000  C2 04 00                  ret 4"]
    result = compare_asm(a, b)
    assert result.verdict == "MISMATCH"


def test_compare_length_difference():
    result = compare_asm(LISTING, LISTING[:-1])
    assert result.verdict == "MISMATCH"
    assert result.first_divergence == 3
    assert result.original_count == 4
    assert result.compiled_count == 3


# ------------------------------------------------------- the real thing


@pytest.mark.skipif(not HAVE_EXE, reason="no pinned executable")
def test_original_asm_on_a_known_record():
    records = read_file(REPO_ROOT / "src" / "buffer.cpp")
    record = next(r for r in records if r.address == 0x005D7210)
    lines = original_asm(record, EXE)
    assert lines
    assert lines[0].startswith("0x005D7210")


@pytest.mark.skipif(not (HAVE_EXE and HAVE_BUILD),
                    reason="no build environment")
def test_byte_exact_body_compiles_to_the_same_shape():
    """Smoke: a BYTE_EXACT claim reproduces byte for byte. The strict
    multi-flag machinery stays with the ratchet in tools/byte_match.py;
    this is the single-flag comparison the package offers."""
    records = read_file(REPO_ROOT / "src" / "caviar.cpp")
    record = next(r for r in records if r.address == 0x00616BC0)
    assert record.byte_exact
    original = original_asm(record, EXE)
    compiled = compiled_asm(record, COMPILE_COMMANDS, FLAGS)
    result = compare_asm(original, compiled)
    assert result.verdict == "BYTE_EXACT", \
        f"{result.verdict}: {result.matching_lines}/{result.original_count} " \
        f"lines match, similarity {result.mnemonic_similarity:.2f}"
    assert result.matching_lines == result.original_count


# ------------------------------------------- naming the subject in an object


def coff(functions, relocations=()):
    """A one-section COFF object carrying several external .text symbols.

    `functions` is [(symbol, code)] laid out in order; `relocations` is
    [(offset, type)] against the section. Enough to exercise subject
    selection and the relocation mask without a compiler.
    """
    blob, placed = b"", []
    for symbol, code in functions:
        placed.append((symbol, len(blob)))
        blob += code
    n_syms = len(placed)
    raw_ptr = 20 + 40
    reloc_ptr = raw_ptr + len(blob) if relocations else 0
    sym_ptr = raw_ptr + len(blob) + len(relocations) * 10
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, sym_ptr, n_syms, 0, 0)
    section = struct.pack("<8sIIIIIIHHI", b".text\x00\x00\x00", 0, 0,
                          len(blob), raw_ptr, reloc_ptr, 0,
                          len(relocations), 0, 0x60000020)
    relocs = b"".join(struct.pack("<IIH", at, 0, kind)
                      for at, kind in relocations)
    entries, strtab, offset = b"", b"", 4
    for symbol, value in placed:
        entries += (b"\x00\x00\x00\x00" + struct.pack("<I", offset)
                    + struct.pack("<IhHBB", value, 1, 0, 2, 0))
        strtab += symbol.encode() + b"\x00"
        offset += len(symbol) + 1
    return (header + section + blob + relocs + entries
            + struct.pack("<I", 4 + len(strtab)) + strtab)


def test_object_relocations_become_a_mask():
    # `push imm32` with a DIR32 relocation on the immediate at offset 1.
    obj = coff([("?f@@YAXXZ", b"\x68\x00\x00\x00\x00\xC3")],
               relocations=((1, 0x0006),))
    code, mask = asm._coff_function_masked(obj, "?f@@YAXXZ")
    assert code == b"\x68\x00\x00\x00\x00\xC3"
    assert mask == frozenset({1, 2, 3, 4})


# --------------------------------------------------- discounting relocations


def test_relocated_bytes_do_not_count_as_a_divergence():
    """The image's linked address against the object's zeros is not a
    difference in the source, and this is the pair that used to read
    MNEMONIC_ONLY."""
    image = b"\x68\x74\x81\x9B\x00\xC3"          # push 0x009B8174
    obj = b"\x68\x00\x00\x00\x00\xC3"            # push 0 + a relocation
    mask = frozenset({1, 2, 3, 4})
    left = asm.Listing(asm._disasm(image, 0x401000), code=image,
                       base=0x401000, mask=frozenset())
    right = asm.Listing(asm._disasm(obj, 0x401000), code=obj,
                        base=0x401000, mask=mask)
    assert compare_asm(list(left), list(right)).verdict == "MNEMONIC_ONLY"
    result = compare_asm(left, right)
    assert result.verdict == "BYTE_EXACT"
    assert result.masked_bytes == 4
    assert result.compared_bytes == 2


def test_a_real_difference_survives_the_mask():
    """Masking discounts relocated bytes, not the instruction around them."""
    image = b"\x68\x74\x81\x9B\x00\xC3"
    obj = b"\xB8\x00\x00\x00\x00\xC3"            # mov eax, imm32, not push
    mask = frozenset({1, 2, 3, 4})
    left = asm.Listing(asm._disasm(image, 0x401000), code=image,
                       base=0x401000, mask=frozenset())
    right = asm.Listing(asm._disasm(obj, 0x401000), code=obj,
                        base=0x401000, mask=mask)
    assert compare_asm(left, right).verdict == "MISMATCH"


def test_padding_is_stripped_from_both_sides():
    """A catalogued span may include COMDAT padding; the object's is already
    stripped, so the image's must be too or the lengths never agree."""
    body = b"\x55\x8B\xEC\xC3"
    obj = coff([("?f@@YAXXZ", body + b"\xCC\xCC")])
    code, _mask = asm._coff_function_masked(obj, "?f@@YAXXZ")
    assert code == body


def test_a_mask_does_not_reshape_a_body_of_another_length():
    """Offsets only name the same bytes in bodies laid out alike.

    `0x004E0F80` is seven instructions in the image; against an object body
    of a different length, zeroing the image at the OBJECT's relocation
    offsets re-split it into nine. The count a reader is given must be the
    body's own, so the mask is not applied across a length difference.
    """
    image = bytes.fromhex("E89BAF0C00" "E856490E00" "E8A14A0E00"
                          "6A01" "E8FAA1F8FF" "59" "C3")
    obj = bytes.fromhex("56" "8BF1" "E800000000" "E800000000")
    left = asm.Listing(asm._disasm(image, 0x004E0F80), code=image,
                       base=0x004E0F80, mask=frozenset())
    right = asm.Listing(asm._disasm(obj, 0x004E0F80), code=obj,
                        base=0x004E0F80,
                        mask=frozenset({4, 5, 6, 7, 9, 10, 11, 12}))
    assert len(left) == 7
    result = compare_asm(left, right)
    assert result.verdict == "MISMATCH"
    assert result.original_count == 7, \
        "the mask reshaped the image's own listing"


def test_byte_equality_decides_not_the_rendered_text():
    """A pair that agrees on every unmasked byte is BYTE_EXACT even though
    the rendered lines differ - the verdict reads the bytes."""
    image = bytes.fromhex("A1 74819B00 C3".replace(" ", ""))
    obj = bytes.fromhex("A1 00000000 C3".replace(" ", ""))
    mask = frozenset({1, 2, 3, 4})
    left = asm.Listing(asm._disasm(image, 0x401000), code=image,
                       base=0x401000, mask=frozenset())
    right = asm.Listing(asm._disasm(obj, 0x401000), code=obj,
                        base=0x401000, mask=mask)
    assert list(left) != list(right)          # the text differs
    assert compare_asm(left, right).verdict == "BYTE_EXACT"


def test_differing_lengths_are_never_byte_exact():
    a = b"\x90\xC3"
    b = b"\x90\x90\xC3"
    left = asm.Listing(asm._disasm(a, 0x401000), code=a, base=0x401000)
    right = asm.Listing(asm._disasm(b, 0x401000), code=b, base=0x401000)
    assert compare_asm(left, right).verdict == "MISMATCH"
