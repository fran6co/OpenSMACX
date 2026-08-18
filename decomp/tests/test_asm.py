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
from decomp.asm import _coff_function, _span, compiled_asm, original_asm
from decomp.reader import REPO_ROOT

# The environment's facts live HERE, not in the package: where the pinned
# image and the build database are on this box. The compiler itself is the
# build's own business - the compile command names it.
EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
COMPILE_COMMANDS = REPO_ROOT / "build" / "compile_commands.json"
FLAGS = "/c /O2 /Gy /GR- /Oy- /GX"     # the ratchet's measured flag set

HAVE_EXE = EXE.is_file()
HAVE_BUILD = COMPILE_COMMANDS.is_file() and shutil.which("wine") is not None

MARKED = """// ORIGINAL: 0x00401000
// name      ?f@C@@QAEXXZ
// size      8 bytes
// spans     0x00401000-0x00401008
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
    assert record.spans == ((0x00401000, 0x00401008),)
    assert _span(record) == (0x00401000, 0x00401008)


def test_span_refused_without_the_fact(tmp_path):
    with pytest.raises(ValueError, match="no spans"):
        _span(record_in(tmp_path, NO_FACTS))


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
    """Smoke: a BYTE_EXACT claim reproduces its instruction count. Strict
    byte verdicts stay with the ratchet in tools/byte_match.py."""
    records = read_file(REPO_ROOT / "src" / "caviar.cpp")
    record = next(r for r in records if r.address == 0x00616BC0)
    assert record.byte_exact
    original = original_asm(record, EXE)
    compiled = compiled_asm(record, COMPILE_COMMANDS, FLAGS)
    assert len(compiled) == len(original), \
        f"instruction count: compiled {len(compiled)} vs original {len(original)}"
