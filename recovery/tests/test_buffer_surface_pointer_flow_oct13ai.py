"""Exact task-scope source/history and pointer-snapshot controls for Buffer."""
from pathlib import Path
import subprocess

from test_buffer_change_color_owner import ROOT


BASE_REVISION = "221d2539e44d74f140ac8fa9bfbe35b37d52932d"
SOURCE = ROOT / "src/buffer.cpp"


def _replace_once(text, old, new, label):
    count = text.count(old)
    assert count > 0, f"baseline recipe {label!r}: anchor missing"
    return text.replace(old, new, 1)


def _authorized_buffer_source(baseline):
    """Apply only the reviewed pointer-flow replacements in four bodies."""
    changes = (
        (
            "const uint32_t storage = reinterpret_cast<uint32_t>(dib_bits_);\n"
            "        locked_bits_ = reinterpret_cast<void *>(storage);",
            "LPVOID const storage = dib_bits_;\n        locked_bits_ = storage;",
            "two-argument DIB storage snapshot",
        ),
        (
            "pixel_base = static_cast<int>(storage);",
            "pixel_base = reinterpret_cast<int>(storage);",
            "two-argument numeric pixel base",
        ),
        (
            "uint32_t data = reinterpret_cast<uint32_t>(descriptor.lpSurface);",
            "LPVOID const data = descriptor.lpSurface;",
            "two-argument Lock storage snapshot",
        ),
        (
            "locked_bits_ = reinterpret_cast<void *>(data);\n"
            "            pixel_base = static_cast<int>(data);",
            "locked_bits_ = data;\n            pixel_base = reinterpret_cast<int>(data);",
            "two-argument captured Lock pointer and numeric base",
        ),
        (
            "const uint32_t data2 = reinterpret_cast<uint32_t>(locked_bits_);\n"
            "            if (data2 != 0 && remaining <= 0) {\n"
            "                surface2->Unlock(reinterpret_cast<void *>(data2));",
            "LPVOID const data2 = locked_bits_;\n"
            "            if (data2 != nullptr && remaining <= 0) {\n"
            "                surface2->Unlock(data2);",
            "two-argument captured Unlock pointer",
        ),
        (
            "uint32_t bits = reinterpret_cast<uint32_t>(dib_bits_);\n"
            "        locked_bits_ = reinterpret_cast<void *>(bits);\n"
            "        if (bits == 0) {",
            "LPVOID const bits = dib_bits_;\n"
            "        locked_bits_ = bits;\n"
            "        if (bits == nullptr) {",
            "six-argument DIB storage snapshot",
        ),
        (
            "int bits = reinterpret_cast<int>(locked_bits_);\n"
            "        if (bits != 0) {",
            "LPVOID const bits = locked_bits_;\n"
            "        if (bits != nullptr) {",
            "draw_dest null snapshot",
        ),
        (
            "int cachedBits = reinterpret_cast<int>(locked_bits_);\n"
            "            if (cachedBits == 0 || count > 0) {",
            "LPVOID const cachedBits = locked_bits_;\n"
            "            if (cachedBits == nullptr || count > 0) {",
            "draw_dest Unlock snapshot",
        ),
        (
            "surface->Unlock(reinterpret_cast<void *>(cachedBits));",
            "surface->Unlock(cachedBits);",
            "draw_dest direct Unlock",
        ),
        (
            "int bits = reinterpret_cast<int>(locked_bits_);\n"
            "        if (bits != 0) {",
            "LPVOID const bits = locked_bits_;\n"
            "        if (bits != nullptr) {",
            "draw_multi_table_dest null snapshot",
        ),
        (
            "int cachedBits = reinterpret_cast<int>(locked_bits_);\n"
            "            if (cachedBits == 0 || count > 0) {",
            "LPVOID const cachedBits = locked_bits_;\n"
            "            if (cachedBits == nullptr || count > 0) {",
            "draw_multi_table_dest Unlock snapshot",
        ),
        (
            "surface->Unlock(reinterpret_cast<void *>(cachedBits));",
            "surface->Unlock(cachedBits);",
            "draw_multi_table_dest direct Unlock",
        ),
    )
    expected = baseline
    for old, new, label in changes:
        expected = _replace_once(expected, old, new, label)
    return expected


def test_complete_buffer_source_matches_only_four_pointer_flow_recipes():
    baseline = subprocess.run(
        ["git", "show", f"{BASE_REVISION}:src/buffer.cpp"],
        cwd=ROOT, check=True, capture_output=True,
    ).stdout.decode("utf-8")
    expected = _authorized_buffer_source(baseline)
    assert SOURCE.read_bytes() == expected.encode("utf-8"), (
        "buffer.cpp has a change outside the four authorized pointer-flow recipes"
    )


def test_actual_pointer_snapshots_and_unlock_identity_are_direct():
    source = SOURCE.read_text(encoding="utf-8")
    two_arg_start = source.index("int Buffer::change_color(int a1, int a2) {")
    two_arg = source[two_arg_start:source.index("// ORIGINAL:", two_arg_start)]
    assert "LPVOID const storage = dib_bits_;" in two_arg
    assert "locked_bits_ = storage;" in two_arg
    assert "const uint32_t existing = reinterpret_cast<uint32_t>(locked_bits_);" in two_arg
    assert "LPVOID const data = descriptor.lpSurface;" in two_arg
    assert "locked_bits_ = data;" in two_arg
    assert "LPVOID const data2 = locked_bits_;" in two_arg
    assert "surface2->Unlock(data2);" in two_arg
    assert "reinterpret_cast<void *>(storage)" not in two_arg
    assert "reinterpret_cast<uint32_t>(descriptor.lpSurface)" not in two_arg
    assert "pixel_base = reinterpret_cast<int>(storage);" in two_arg
    assert "pixel_base = static_cast<int>(existing);" in two_arg
    assert "pixel_base = reinterpret_cast<int>(data);" in two_arg

    six_arg_start = source.index(
        "int Buffer::change_color(int a1, int a2, int a3, int a4, int a5, int a6) {"
    )
    six_arg = source[six_arg_start:source.index("// ORIGINAL:", six_arg_start)]
    assert "LPVOID const bits = dib_bits_;" in six_arg
    assert "locked_bits_ = bits;" in six_arg

    for signature in (
        "int Buffer::draw_dest(Buffer * a1, int a2, int a3, int a4, int a5, int a6, unsigned char * a7) {",
        "int Buffer::draw_multi_table_dest(Buffer * a1, int a2, int a3, int a4, int a5, int a6, unsigned char * a7) {",
    ):
        start = source.index(signature)
        end = source.index("\n}", start) + 2
        body = source[start:end]
        assert body.count("LPVOID const bits = locked_bits_;") == 1
        assert body.count("LPVOID const cachedBits = locked_bits_;") == 1
        assert body.count("surface->Unlock(cachedBits);") == 1
        assert body.index("LPVOID const cachedBits = locked_bits_;") < body.index(
            "surface->Unlock(cachedBits);"
        )
        assert "reinterpret_cast<int>(locked_bits_)" not in body
        assert "reinterpret_cast<void *>(cachedBits)" not in body
