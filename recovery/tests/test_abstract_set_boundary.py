"""abstract_set retains its mapped owner boundary and byte parameter."""

from pathlib import Path
import os
import re

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def _body(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def _mapped_span(address):
    rows = [row for path in (ROOT / "src/path.cpp", ROOT / "src/map.cpp")
            for row in read_file(path) if row.address == address]
    row, = rows
    assert len(row.image_spans) == 1
    from match_recovery.model import Span
    return Span(*row.image_spans[0])


def test_abstract_set_is_an_ordinary_mapped_owner_with_uint8_parameter():
    header = (ROOT / "src/map.h").read_text(encoding="utf-8")
    map_cpp = (ROOT / "src/map.cpp").read_text(encoding="utf-8")
    path_cpp = (ROOT / "src/path.cpp").read_text(encoding="utf-8")
    signature = "void __cdecl abstract_set(int x, int y, uint8_t region)"
    assert header.count(signature + ";") == 1
    assert "MEASURED inline void __cdecl abstract_set" not in header
    assert map_cpp.count(signature) == 1
    body = _body(map_cpp, signature)
    assert "(MapAbstract())[(x >> 1) + y * (MapAbstractLongBounds >> 1)] = region;" in body
    path_code = re.sub(r"/\*.*?\*/|//[^\n]*", "", path_cpp, flags=re.S)
    assert "abstract_set_call" not in path_code
    assert len(re.findall(r"\babstract_set\s*\(", path_code)) == 1
    assert "abstract_set(x, y, region);" in path_code
    record, = [row for row in read_file(ROOT / "src/map.cpp")
               if row.address == 0x00591230]
    assert record.body == "src/map.cpp"


def test_pinned_make_abstract_calls_mapped_abstract_set_at_original_site():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_IMM
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required for call-boundary verification")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    def decode_complete(address):
        span = _mapped_span(address)
        decoded = list(decoder.disasm(image.read(span), span.start))
        cursor = span.start
        for insn in decoded:
            assert insn.address == cursor, f"undecoded byte at {cursor:#x}"
            cursor += insn.size
        assert cursor == span.end
        return decoded

    caller = decode_complete(0x0059C200)
    target = decode_complete(0x00591230)
    assert len(target) > 0
    call, = [insn for insn in caller if insn.address == 0x0059C2F6]
    assert call.mnemonic == "call"
    assert call.operands[0].type == X86_OP_IMM
    assert call.operands[0].imm == 0x00591230
