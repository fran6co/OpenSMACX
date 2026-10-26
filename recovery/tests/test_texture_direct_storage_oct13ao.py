"""Direct Texture raster-global storage and representation controls.

The C++ host check compiles the literal mapped globals and private bit-copy
helper. It checks object aliasing/representation behavior only; host layout is
not evidence for the original 32-bit ABI, allocation, or pixel semantics.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from recovery.tests.texture_direct_storage_host import (
    direct_storage_prelude,
)


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/texture.cpp"
HOST_MAIN = r"""
static void require98(bool ok, const char *message) {
    if (!ok) { fprintf(stderr, "ASSERT98: %s\n", message); exit(98); }
}
static unsigned char mutated_raster_byte;
static void mutate_raster_cell() { TextureRasterScratch504 = &mutated_raster_byte; }
static unsigned char *callback_then_read(void (*callback)()) {
    callback();
    return TextureRasterScratch504;
}
int main() {
    unsigned char byte_value = 0xA7;
    TextureRasterScratch494 = byte_value;
    require98(TextureRasterScratch494 == byte_value, "Scratch494 typed byte storage");

    TextureRasterScratch4C8.dword = static_cast<int>(0x11223344u);
    require98(TextureRasterScratch4C8.bytes[2] == 0x22,
              "Scratch4CA uses byte offset two");
    TextureRasterScratch4E4.dword = static_cast<int>(0x55667788u);
    require98(TextureRasterScratch4E4.bytes[2] == 0x66,
              "Scratch4E6 uses byte offset two");
    TextureRasterUAccumulator.dword = static_cast<int>(0xA1B2C3D4u);
    TextureRasterVAccumulator.dword = static_cast<int>(0xE5F60718u);
    require98(TextureRasterUAccumulator.words.high_word == 0xA1B2,
              "U high-word remains a 16-bit view");
    require98(TextureRasterVAccumulator.words.high_word == 0xE5F6,
              "V high-word remains a 16-bit view");

    const unsigned int key_patterns[] = {0u, 0x7fffffffu, 0x80000000u, 0xffffffffu};
    for (unsigned int i = 0; i < sizeof(key_patterns) / sizeof(key_patterns[0]); ++i) {
        int expected;
        TextureRasterKey = key_patterns[i];
        memcpy(&expected, &key_patterns[i], sizeof(expected));
        const int actual = texture_raster_key_signed_bits();
        require98(memcmp(&actual, &expected, sizeof(actual)) == 0,
                  "signed key bit pattern must be preserved");
    }

    TextureRasterEdgeVerticesA = 0;
    Vert *row = 0;
    TextureRasterEdgeVerticesA = row;
    Vert **forwarded = &TextureRasterEdgeVerticesA;
    require98(forwarded == &TextureRasterEdgeVerticesA && *forwarded == row,
              "forwarded edge pointer retains actual global identity");
    int source_bits = static_cast<int>(0x89ABCDEFu);
    EdgeScan left;
    left.accumD = source_bits;
    TextureRasterScratch498 = 0x13572468;
    actual_scratch498_copy(left);
    require98(memcmp(&TextureRasterScratch498, &source_bits, 4) == 0,
              "Scratch498 receives the full four-byte representation");
    require98(callback_then_read(mutate_raster_cell) == &mutated_raster_byte,
              "global is reread after callback mutation");
    return 0;
}
"""


SCRATCH_COPY = "memcpy(&(TextureRasterScratch498), &left.accumD, sizeof(TextureRasterScratch498));"


def _translation_unit(source, main=HOST_MAIN):
    prelude = direct_storage_prelude(source)
    copy_match = re.search(
        r"(?m)^\s*memcpy\(&\(TextureRasterScratch498\), &left\.accumD, [^;]+;",
        source)
    assert copy_match is not None
    copy_statement = copy_match.group(0).strip()
    return ("#include <stdint.h>\n#include <string.h>\n#include <stdio.h>\n#include <stdlib.h>\n"
            "struct Vert;\nstruct EdgeScan { int accumD; };\n" + prelude + "\n"
            "static void actual_scratch498_copy(const EdgeScan &left) {\n"
            + copy_statement + "\n}\n" + main)


def _build_run(source, compiler_name, tmp_path, name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    cpp = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-function",
         "-Wno-unused-variable", str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], check=False, capture_output=True,
                          text=True, timeout=30)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_direct_storage_globals_and_representation_controls(compiler, tmp_path):
    source = SOURCE.read_text(encoding="utf-8")
    result = _build_run(_translation_unit(source), compiler, tmp_path,
                        f"texture_direct_storage_{compiler}")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_signed_key_truncation_mutant_compiles_but_fails_with_named_98(
    compiler, tmp_path
):
    source = SOURCE.read_text(encoding="utf-8")
    mutant = source.replace(
        "memcpy(&value, &TextureRasterKey, sizeof(value));",
        "value = static_cast<int>(TextureRasterKey & 0x7fffffffu);", 1)
    assert mutant != source
    result = _build_run(_translation_unit(mutant), compiler, tmp_path,
                        f"texture_key_truncation_{compiler}")
    assert result.returncode == 98, (result.returncode, result.stdout, result.stderr)
    assert "signed key bit pattern must be preserved" in result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_wrong_union_byte_mutant_compiles_but_fails_with_named_98(compiler, tmp_path):
    main = HOST_MAIN.replace(
        "TextureRasterScratch4C8.bytes[2] == 0x22",
        "TextureRasterScratch4C8.bytes[1] == 0x22", 1)
    assert main != HOST_MAIN
    result = _build_run(_translation_unit(SOURCE.read_text(encoding="utf-8"), main),
                        compiler, tmp_path, f"texture_wrong_byte_{compiler}")
    assert result.returncode == 98, (result.returncode, result.stdout, result.stderr)
    assert "Scratch4CA uses byte offset two" in result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_cached_global_mutant_compiles_but_fails_after_callback(compiler, tmp_path):
    main = HOST_MAIN.replace(
        "static unsigned char *callback_then_read(void (*callback)()) {\n    callback();\n    return TextureRasterScratch504;\n}",
        "static unsigned char *callback_then_read(void (*callback)()) {\n"
        "    unsigned char *cached = TextureRasterScratch504;\n"
        "    callback();\n    return cached;\n}", 1)
    assert main != HOST_MAIN
    result = _build_run(_translation_unit(SOURCE.read_text(encoding="utf-8"), main),
                        compiler, tmp_path, f"texture_cached_global_{compiler}")
    assert result.returncode == 98, (result.returncode, result.stdout, result.stderr)
    assert "global is reread after callback mutation" in result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_short_scratch498_copy_mutant_compiles_but_fails_full_word_control(
    compiler, tmp_path
):
    source = SOURCE.read_text(encoding="utf-8")
    assert source.count(SCRATCH_COPY) == 1
    mutant = source.replace(SCRATCH_COPY,
                            "memcpy(&(TextureRasterScratch498), &left.accumD, 2);", 1)
    result = _build_run(_translation_unit(mutant), compiler, tmp_path,
                        f"texture_short_scratch_copy_{compiler}")
    assert result.returncode == 98, (result.returncode, result.stdout, result.stderr)
    assert "Scratch498 receives the full four-byte representation" in result.stderr
