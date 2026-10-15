"""Source and runtime controls for the TextureRasterScratch504 pointer cell."""
from pathlib import Path
import shutil
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/texture.cpp"


def test_scratch504_retains_identity_and_has_only_three_typed_current_stores():
    source = SOURCE.read_text(encoding="utf-8")
    assert "// The pinned renderers reload this cell as the source base for byte loads.\n" \
           "// RECOVERY_OBJECT: v1 0x009BB504 _TextureRasterScratch504\n" \
           "static unsigned char *TextureRasterScratch504;" in source
    code = re.sub(r"//[^\n]*|/\*.*?\*/|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*'",
                  " ", source, flags=re.S)
    occurrences = list(re.finditer(r"\bTextureRasterScratch504\b", code))
    assert len(occurrences) == 4, "one declaration and exactly three existing stores"
    assert code.count("TextureRasterScratch504 = static_cast<unsigned char *>(texture_pointer_from_address(") == 2
    assert code.count("TextureRasterScratch504 = reinterpret_cast<unsigned char *>(src);") == 1
    assert "TextureRasterScratch504 = static_cast<int>(" not in code
    assert "TextureRasterScratch504 = reinterpret_cast<int>(src)" not in code

    assert re.search(
        r"// RECOVERY_OBJECT: v1 0x009BB504 _TextureRasterScratch504\n"
        r"static unsigned char \*TextureRasterScratch504;",
        source,
    )


def test_current_stores_preserve_target_uint32_address_wrap_and_direct_pointer():
    source = SOURCE.read_text(encoding="utf-8")
    coast61ead0 = source[source.index("// ORIGINAL: 0x0061EAD0"):source.index("// ORIGINAL:", source.index("// ORIGINAL: 0x0061EAD0") + 1)]
    coast623f80 = source[source.index("// ORIGINAL: 0x00623F80"):source.index("// ORIGINAL:", source.index("// ORIGINAL: 0x00623F80") + 1)]
    trans3 = source[source.index("// ORIGINAL: 0x00622DB0"):source.index("// ORIGINAL:", source.index("// ORIGINAL: 0x00622DB0") + 1)]

    assert "static_cast<uint32_t>(static_cast<uint32_t>(texture_pointer_address(TextureRasterScratch544)) +" in coast61ead0
    assert "row_source_offset)))" in coast61ead0
    assert "TextureRasterScratch504 = reinterpret_cast<unsigned char *>(src);" in coast623f80
    assert "static_cast<uint32_t>(static_cast<uint32_t>(" in trans3
    assert "texture_pointer_address(TextureRasterScratch544)) + source_offset)))" in trans3


def _function(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unclosed helper function")


def _actual_store(source, offset_name, marker):
    marker_at = source.index(marker)
    start = source.index("TextureRasterScratch504 =", marker_at)
    end = source.index(";", start) + 1
    statement = source[start:end]
    assert offset_name in statement
    return statement


def _unbounded(statement, offset_name):
    pattern = (r"static_cast<uint32_t>\s*\(\s*static_cast<uint32_t>\s*\(\s*"
               r"texture_pointer_address\(TextureRasterScratch544\)\s*\)\s*\+\s*" +
               re.escape(offset_name) + r"\s*\)")
    changed, count = re.subn(pattern,
        "static_cast<uintptr_t>(texture_pointer_address(TextureRasterScratch544) + " +
        offset_name + ")", statement)
    assert count == 1
    return changed


def _compile_and_run(tmp_path, stores):
    compiler = shutil.which("g++")
    if not compiler:
        import pytest
        pytest.skip("host g++ required")
    source_path, executable = tmp_path / "pointer-wrap.cpp", tmp_path / "pointer-wrap"
    source_path.write_text(r"""
#include <cstdint>
#include <cstdio>
#include <cstdlib>
// ACTUAL ADDRESS HELPER
// ACTUAL FROM-ADDRESS HELPER
static const void *TextureRasterScratch544;
static unsigned char *TextureRasterScratch504;
int main() {
    struct Case { uintptr_t base; uint32_t offset; uintptr_t expected; };
    const Case cases[] = {
        {static_cast<uintptr_t>(0xFFFFFFFEu), 3u, static_cast<uintptr_t>(1u)},
        {static_cast<uintptr_t>(0x12345678FFFFFFFEull), 3u, static_cast<uintptr_t>(1u)},
        {static_cast<uintptr_t>(0x7FFFFFFFu), 0x80000002u, static_cast<uintptr_t>(1u)},
        {static_cast<uintptr_t>(0x10000000u), 0x80000002u, static_cast<uintptr_t>(0x90000002u)}
    };
    for (unsigned i=0; i<sizeof(cases)/sizeof(cases[0]); ++i) {
        TextureRasterScratch544 = reinterpret_cast<const void *>(cases[i].base);
        const uint32_t row_source_offset = cases[i].offset;
        const uint32_t source_offset = cases[i].offset;
        // ACTUAL FIRST STORE
        if (reinterpret_cast<uintptr_t>(TextureRasterScratch504) != cases[i].expected) {
            std::fprintf(stderr, "61EAD0 address case %u mismatch\n", i); return 81;
        }
        // ACTUAL SECOND STORE
        if (reinterpret_cast<uintptr_t>(TextureRasterScratch504) != cases[i].expected) {
            std::fprintf(stderr, "622DB0 address case %u mismatch\n", i); return 82;
        }
    }
    return 0;
}
""")
    source = source_path.read_text()
    product_source = SOURCE.read_text(encoding="utf-8")
    addr = _function(product_source, "static uintptr_t texture_pointer_address(")
    from_addr = _function(product_source, "static void *texture_pointer_from_address(")
    source = source.replace("// ACTUAL ADDRESS HELPER", addr)
    source = source.replace("// ACTUAL FROM-ADDRESS HELPER", from_addr)
    source = source.replace("// ACTUAL FIRST STORE", stores[0])
    source = source.replace("// ACTUAL SECOND STORE", stores[1])
    source_path.write_text(source)
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", str(source_path),
                            "-o", str(executable)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def test_actual_61ead0_and_622db0_stores_wrap_synthetic_addresses(tmp_path):
    source = SOURCE.read_text(encoding="utf-8")
    first_start = source.index("// ORIGINAL: 0x0061EAD0")
    first_block = source[first_start:source.index("// ORIGINAL:", first_start + 1)]
    second_start = source.index("// ORIGINAL: 0x00622DB0")
    second_block = source[second_start:source.index("// ORIGINAL:", second_start + 1)]
    first = _actual_store(first_block, "row_source_offset", "const unsigned int row_source_offset =")
    second = _actual_store(second_block, "source_offset", "const unsigned int source_offset =")
    result = _compile_and_run(tmp_path, (first, second))
    assert result.returncode == 0, result.stdout + result.stderr
    # Mutate each store independently: rejection of the first must not hide
    # an untested second store. Only the named assertion exit is evidence.
    for stores, code, owner in (
        ((_unbounded(first, "row_source_offset"), second), 81, "61EAD0"),
        ((first, _unbounded(second, "source_offset")), 82, "622DB0"),
    ):
        rejected = _compile_and_run(tmp_path, stores)
        assert rejected.returncode == code, rejected.stdout + rejected.stderr
        assert owner + " address case 0 mismatch" in rejected.stderr

