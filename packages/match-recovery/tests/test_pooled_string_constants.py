"""TU-private pooled string constants resolve by their pooled content."""

import hashlib
import struct

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Result, Span
from match_recovery.pe_coff import Coff, PeImage

LITERAL_NAME = b"??_C@_0"  # short inline form of a VC6 pooled string symbol


def object_with_literal(code, literal):
    """One code COMDAT, one read-only literal section, one DIR32 reference."""
    code_bytes = code + b"\xcc" * (-len(code) % 16)
    literal_bytes = literal + b"\0"
    reloc = struct.pack("<IIH", 1, 1, 6)  # DIR32 at code offset 1 -> literal symbol index 1
    symbols = (b"_f".ljust(8, b"\0") + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
               + LITERAL_NAME.ljust(8, b"\0") + struct.pack("<IhHBB", 0, 2, 0, 3, 0))
    base = 20 + 2 * 40  # header plus two section table rows
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0,
                         base + len(code_bytes) + len(literal_bytes) + len(reloc), 2, 0, 0)
    first = b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(code_bytes), base,
                                         base + len(code_bytes) + len(literal_bytes), 0, 1, 0, 0x60501020)
    second = b".rdata\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(literal_bytes),
                                         base + len(code_bytes), 0, 0, 0, 0, 0x40401040)
    return (header + first + second + code_bytes + literal_bytes + reloc
            + symbols + struct.pack("<I", 4))


def image_with_literal(code, literal, *, twice=False):
    """Two raw-backed sections: .text code and read-only .rdata with the literal."""
    rdata = literal + b"\0" + (literal + b"\0" if twice else b"X\0")
    data = bytearray(2048)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, 0x80)
    data[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", data, 0x84, 0x14C, 2, 0, 0, 0, 224, 0)
    struct.pack_into("<H", data, 0x98, 0x10B)
    struct.pack_into("<I", data, 0x98 + 28, 0x400000)
    struct.pack_into("<I", data, 0x98 + 92, 16)
    data[0x178:0x180] = b".text\0\0\0"
    struct.pack_into("<IIII", data, 0x180, 512, 0x1000, 512, 512)
    data[0x1A0:0x1A8] = b".rdata\0\0"
    struct.pack_into("<IIII", data, 0x1A8, len(rdata), 0x2000, len(rdata), 1024)
    struct.pack_into("<I", data, 0x1A0 + 36, 0x40000040)
    data[512:512 + len(code)] = code
    data[1024:1024 + len(rdata)] = rdata
    # A base-relocation block marks the code's DIR32 operand site, exactly as
    # a real image records it for a loader HIGHLOW fixup.
    struct.pack_into("<II", data, 0x98 + 96 + 5 * 8, 0x1100, 12)
    struct.pack_into("<IIHH", data, 768, 0x1000, 12, 0x3000 + 1, 0)
    return bytes(data)


def binding(pin):
    return (FunctionBinding(Origin(pin, "pe:va", 0x401000), (Span(0x401000, 0x401000 + 16),), "_f", "unit.obj"),)


def check(code, literal, *, twice=False, image_literal=None):
    original = b"\xa1" + struct.pack("<I", 0x402000) + b"\x90" * 11
    pooled = image_literal if image_literal is not None else literal
    raw = object_with_literal(code, literal)
    img = image_with_literal(original, pooled, twice=twice)
    pin = hashlib.sha256(img).hexdigest()
    row, = check_objects(binding(pin), {"unit.obj": Coff(raw)}, PeImage(img, pin))
    return row


def test_pooled_literal_resolves_by_unique_content():
    row = check(b"\xa1\x00\x00\x00\x00" + b"\x90" * 11, b"JKAF")
    assert row["result"] == Result.PASS, row
    pooled, = row["pooled_string_constants"]
    assert pooled["literal"] == b"JKAF\0" and pooled["address"] == "0x00402000"


def test_ambiguous_literal_stays_unresolved():
    row = check(b"\xa1\x00\x00\x00\x00" + b"\x90" * 11, b"JKAF", twice=True)
    assert row["result"] == Result.UNKNOWN and "pooled_string_constants" not in row


def test_absent_literal_stays_unresolved():
    row = check(b"\xa1\x00\x00\x00\x00" + b"\x90" * 11, b"JKAF", image_literal=b"OTHER")
    assert row["result"] == Result.UNKNOWN and "pooled_string_constants" not in row
