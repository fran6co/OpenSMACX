"""Guard the reviewed zlib leaf-to-component history migration."""

from hashlib import sha256
from pathlib import Path
import re

from match_recovery.component_source import parse_component
from test_zlib_main_state_machine_bindings_oct10 import EXPECTED as MAIN_STATE_MACHINES
from test_zlib_inflate_group_bindings_oct10 import EXPECTED as FOURTEEN_MEMBERS, UNITS
from match_recovery.source import read_file, read_text
from owner_transfer_test_support import assert_current_owner, assert_leaf_collection_retired
from match_recovery.source.writer import write
from test_zlib_deflate_tree_group_bindings_oct10 import EXPECTED as SIXTEEN_MEMBERS


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
UPSTREAM_SHA256 = {
    "trees.c": "b40b8b101f12e90c8aaa85284fc993f3e588ef59765c8c84cf3a7f21cfb04371",
    "deflate.c": "ba5534261c7460779d9ad69e35816b12a93c680ae1d7bca37e8702fb394a81c5",
    "deflate.h": "a9bc4798decd29e8e4a6b39967d4cc1910581003f0039c76e80ec5d933726d9a",
    "PROVENANCE.md": "df09276a688bc522ecca0198e84b78c8d209120b70280d18aeec81836ea17ef3",
}
EXPECTED = {
    0x00642940: (0x0064295F, "_bi_reverse", "trees.c", True,
                 "c579bff2e4892df8170e987ea84aecc3d169588dc2c7b5864ff9b689e7b7b372"),
    0x0063D420: (0x0063D44F, "_putShortMSB", "deflate.c", False,
                 "9cfd33d95749e665c48515c9727d062ad88f3662c0f07a19b231ca8f09a3c3b7"),
}


def envelope(text, address):
    marker = f"// ORIGINAL: 0x{address:08X} "
    at = text.index(marker)
    start = text.rfind("/*", 0, at)
    end = text.index("*/", at) + 2
    return text[start:end]


def extract_definition(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError(f"unterminated upstream definition: {signature}")


def normalized_history(block, address):
    block = re.sub(
        r"^// Historical source-role note: (.*)$",
        r"// RECOVERY_ROLE: \1", block, flags=re.M,
    )
    block = re.sub(
        r"^// Historical emitted symbol: (.*)$",
        r"// symbol    \1", block, flags=re.M,
    )
    return block


def test_migrated_history_identity_and_native_writer_roundtrip():
    pin_path = ROOT / "src/IMAGE_PIN"
    assert pin_path.is_file()
    assert pin_path.read_text(encoding="utf-8").split()[-1] == PIN
    assert_leaf_collection_retired(ROOT)
    header = (ROOT / "src/vendor/zlib.recovery.h").read_text(encoding="utf-8")
    for address in EXPECTED:
        assert_current_owner(ROOT, address, "src/vendor/zlib.recovery.h")
    assert all(f"ORIGINAL: 0x{address:08X}" in header for address in EXPECTED)
    # Historical symbol comments are provenance, not active substitute bodies.
    assert "leaf_00642940_redirect(" not in header
    assert "leaf_0063d420_redirect(" not in header
    rows = {row.address: row for row in read_file(ROOT / "src/vendor/zlib.recovery.h")}
    manifest = parse_component(header)
    bindings = {binding.origin.address: binding for binding in manifest.bindings}
    assert set(bindings) == {0x00640A80, 0x00644930, *EXPECTED,
                            0x0063CF00, 0x0063D4D0, 0x0063DF90, 0x0063E0F0, 0x0063D450,
                            0x00642960, 0x006429F0, 0x00642A70,
                            0x0063CF30, 0x0063D120, 0x0063D1A0, 0x0063D560,
                            0x00640BB0, 0x00640C30, 0x00640E50, 0x00640EC0,
                            0x00640F40, 0x00641000} | set(SIXTEEN_MEMBERS) | set(FOURTEEN_MEMBERS) | set(MAIN_STATE_MACHINES)
    assert set(manifest.units) == UNITS
    for address, (end, symbol, unit, claimed, history_digest) in EXPECTED.items():
        row = rows[address]
        assert row.name == symbol
        assert row.image_spans == ((address, end),)
        assert row.byte_exact is claimed
        binding = bindings[address]
        assert binding.symbol == symbol and binding.unit == unit
        assert "Application retained-selection status is UNKNOWN" in binding.evidence
        assert "This records source ownership only, not a current byte claim" in binding.evidence
        assert "Component-binary bytes, boundary, and references require a fresh configured component-verifier measurement" in binding.evidence
        assert "Whole-library version uniqueness and runtime equivalence are unproved" in binding.evidence
        assert "expected one defined COFF function" not in binding.evidence
        assert "Pinned zlib 1.0.2 source correspondence" in binding.evidence
        assert "ignored owner proposal" not in binding.evidence
        if address == 0x00642940:
            assert "tr_static_init" in binding.evidence and "gen_codes" in binding.evidence
            assert "legal Huffman lengths 1..15" in binding.evidence
        else:
            assert "zlib header" in binding.evidence and "Adler-32 trailer" in binding.evidence
            assert "pending_buf at +0x08 and pending at +0x10" in binding.evidence
        block = envelope(header, address)
        assert sha256(normalized_history(block, address).encode()).hexdigest() == history_digest
    rewritten = write(header, read_text(header, ROOT / "src/vendor/zlib.recovery.h"))
    assert rewritten == header
    assert parse_component(rewritten) == manifest


def test_pinned_upstream_sources_call_domains_and_i386_state_offsets():
    import hashlib

    vendor = ROOT / "src/vendor/zlib-1.0.2"
    for filename, digest in UPSTREAM_SHA256.items():
        assert hashlib.sha256((vendor / filename).read_bytes()).hexdigest() == digest
    trees = (vendor / "trees.c").read_text(encoding="latin1")
    deflate = (vendor / "deflate.c").read_text(encoding="latin1")
    deflate_h = (vendor / "deflate.h").read_text(encoding="latin1")
    provenance = (vendor / "PROVENANCE.md").read_text(encoding="utf-8")
    assert "zlib 1.0.2" in provenance
    assert "local unsigned bi_reverse(code, len)" in trees
    assert "do {" in trees and "code >>= 1, res <<= 1;" in trees
    assert "} while (--len > 0);" in trees and "return res >> 1;" in trees
    assert "1 <= len <= 15" in trees
    assert "bi_reverse((unsigned)n, 5)" in trees
    assert "if (len == 0) continue;" in trees
    assert "bi_reverse(next_code[len]++, len)" in trees
    assert "local void putShortMSB (s, b)" in deflate
    assert "put_byte(s, (Byte)(b >> 8));" in deflate
    assert "put_byte(s, (Byte)(b & 0xff));" in deflate
    calls = [line.strip() for line in deflate.splitlines() if "putShortMSB(s," in line]
    assert len(calls) == 5
    assert all("s->" in line or "strm->adler" in line or "header" in line for line in calls)
    assert "#define put_byte(s, c) {s->pending_buf[s->pending++] = (c);}" in deflate_h
    fields = ["z_stream *strm", "int   status", "Bytef *pending_buf", "Bytef *pending_out", "int   pending"]
    positions = [deflate_h.index(field) for field in fields]
    assert positions == sorted(positions)
    # The pinned PE is i386: every leading pointer/int occupies four bytes.
    offsets = {field: index * 4 for index, field in enumerate(fields)}
    assert offsets["Bytef *pending_buf"] == 0x08
    assert offsets["int   pending"] == 0x10
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    assert "if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)" in cmake


def test_actual_pinned_static_bodies_in_host_fixture(tmp_path):
    import shutil
    import subprocess
    import pytest

    compiler = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
    if not compiler:
        pytest.skip("no host C compiler installed")
    vendor = ROOT / "src/vendor/zlib-1.0.2"
    trees = (vendor / "trees.c").read_text(encoding="latin1")
    deflate = (vendor / "deflate.c").read_text(encoding="latin1")
    deflate_h = (vendor / "deflate.h").read_text(encoding="latin1")
    reverse = extract_definition(trees, "local unsigned bi_reverse(code, len)")
    put_short = extract_definition(deflate, "local void putShortMSB (s, b)")
    put_byte = next(line for line in deflate_h.splitlines()
                    if line.startswith("#define put_byte(s, c)"))
    fixture = "\n".join((
        "#include <assert.h>",
        "typedef unsigned int uInt;",
        "typedef unsigned char Byte;",
        "typedef unsigned char Bytef;",
        "typedef struct deflate_state_s { Bytef *pending_buf; int pending; } deflate_state;",
        "#define local static",
        put_byte,
        reverse,
        put_short,
        "int main(void) {",
        "  unsigned char output[4] = {0, 0, 0, 0};",
        "  deflate_state state = {output, 0};",
        "  putShortMSB(&state, 0x1234U);",
        "  assert(state.pending == 2);",
        "  assert(output[0] == 0x12 && output[1] == 0x34);",
        "  assert(bi_reverse(0xBU, 4) == 0xDU);",
        "  assert(bi_reverse(0x1DU, 5) == 0x17U);",
        "  return 0;",
        "}",
    ))
    source = tmp_path / "pinned_zlib_static_fixture.c"
    binary = tmp_path / "pinned_zlib_static_fixture"
    source.write_text(fixture, encoding="latin1")
    built = subprocess.run(
        [compiler, "-std=gnu89", "-O0", "-Wall", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
