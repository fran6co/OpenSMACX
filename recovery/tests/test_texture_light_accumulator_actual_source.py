"""Actual-source controls for Texture's full-dword/low-word light cell."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/texture.cpp").is_file())
SOURCE = ROOT / "src/texture.cpp"


def extract_braced(source, signature):
    start = source.find(signature)
    assert start >= 0, "missing actual source declaration: " + signature
    opening = source.find("{", start)
    assert opening >= 0
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                end = source.find(";", index)
                assert end >= 0
                return source[start:end + 1]
    raise AssertionError("unterminated source declaration: " + signature)


def extract_real_low_word_updates(source):
    signature = "void __cdecl sub_620640(int param_1, int param_2)"
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    assert masked.count(signature) == 1
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    closing = None
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                closing = index
                break
    assert closing is not None
    body = source[opening + 1:closing]
    masked_body = masked[opening + 1:closing]
    update = re.compile(
        r"\bTextureRasterLightAccumB\.words\.low_word\s*=\s*"
        r"static_cast\s*<\s*unsigned\s+short\s*>\s*\(\s*"
        r"TextureRasterLightAccumB\.words\.low_word\s*\+\s*"
        r"TextureRasterLightPackA\.low_word\s*\)\s*;"
    )
    matches = list(update.finditer(masked_body))
    assert len(matches) == 2, "expected both real low-word updates in sub_620640"
    return [body[match.start():match.end()] for match in matches]


def compile_and_run(mutant=None):
    source = SOURCE.read_text(encoding="utf-8")
    pair = extract_braced(source, "struct TextureRasterWordPair {")
    view = extract_braced(source, "union TextureRasterDwordHighWordView {")
    declaration = (
        "static TextureRasterDwordHighWordView TextureRasterLightAccumB;"
    )
    assert source.count(declaration) == 1
    first, second = extract_real_low_word_updates(source)
    if mutant == "dword_carry":
        second = "TextureRasterLightAccumB.dword += TextureRasterLightPackA.low_word;"
    elif mutant == "erase_upper_word":
        second = "TextureRasterLightAccumB.dword = TextureRasterLightPackA.low_word;"
    elif mutant == "missing_mirrored_update":
        second = "(void)TextureRasterLightPackA.low_word;"

    fixture = r'''#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

''' + pair + "\n" + view + "\n" + declaration + r'''
static TextureRasterWordPair TextureRasterLightPackA;
typedef char cell_is_four_bytes[sizeof(TextureRasterLightAccumB) == 4 ? 1 : -1];
typedef char low_word_starts_at_zero[offsetof(TextureRasterDwordHighWordView, words) == 0 ? 1 : -1];

static void update_first() {
''' + first + r'''
}
static void update_second() {
''' + second + r'''
}

static int run_case(unsigned short high, unsigned short low,
                    unsigned short step, unsigned short expected) {
    TextureRasterLightAccumB.words.high_word = high;
    TextureRasterLightAccumB.words.low_word = low;
    TextureRasterLightPackA.low_word = step;
    update_first();
    if (TextureRasterLightAccumB.words.high_word != high ||
        TextureRasterLightAccumB.words.low_word != expected) return 1;

    TextureRasterLightAccumB.words.high_word = high;
    TextureRasterLightAccumB.words.low_word = low;
    update_second();
    if (TextureRasterLightAccumB.words.high_word != high ||
        TextureRasterLightAccumB.words.low_word != expected) return 2;
    return 0;
}

int main() {
    if (run_case(0x1234, 0x7fff, 1, 0x8000)) return 1;
    if (run_case(0xabcd, 0x8000, 0x8000, 0x0000)) return 2;
    if (run_case(0x5a5a, 0xffff, 1, 0x0000)) return 3;
    if (run_case(0x9876, 0x1234, 0x4321, 0x5555)) return 4;
    return 0;
}
'''
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    available = [compiler for compiler in compilers if compiler]
    if not available:
        pytest.skip("GCC or Clang required for actual-source storage controls")
    with tempfile.TemporaryDirectory(prefix="texture-light-word-") as directory:
        cpp = Path(directory) / "light_word.cpp"
        cpp.write_text(fixture, encoding="utf-8")
        for compiler in available:
            executable = Path(directory) / Path(compiler).name.replace("+", "p")
            built = subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
                 str(cpp), "-o", str(executable)],
                capture_output=True, text=True, timeout=30)
            assert built.returncode == 0, built.stdout + built.stderr
            run = subprocess.run([str(executable)], check=False, timeout=10)
            if mutant is None:
                assert run.returncode == 0
            else:
                assert run.returncode != 0, "mutation survived: " + mutant


def test_actual_both_low_word_updates_wrap_and_preserve_upper_word():
    compile_and_run()


@pytest.mark.parametrize("mutant", [
    "dword_carry",
    "erase_upper_word",
    "missing_mirrored_update",
])
def test_actual_low_word_mutants_are_rejected(mutant):
    compile_and_run(mutant)
