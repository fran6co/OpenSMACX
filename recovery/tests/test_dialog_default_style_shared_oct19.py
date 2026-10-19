"""The style accessor must alias the actual defaults table, including slot 3."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/dialog_defaults.h"
SOURCE = ROOT / "src/dialog.cpp"


def extract_definition(text: str, signature: str) -> str:
    start = text.rindex(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                if signature.startswith("uint32_t DialogDefaults"):
                    end = text.index(";", end) + 1
                    return text[start:end]
                return text[start:end + 1]
    raise AssertionError(f"unterminated production definition: {signature}")


TABLE_WITNESS = r"""
    const uint32_t expected[37] = {
        247, 7, 248, 2, 125, 0,
        0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
        1, 1, 1, 1, 1, 1,
        0x002E2E2Eu, 0x002E2E2Eu,
        320, 6, 1, 12, 255, 0,
        0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
        1, 1, 1, 1, 1, 1,
        3,
    };
    for (int i = 0; i < 37; ++i) assert(DialogDefaults[i] == expected[i]);
"""


BEHAVIOR = r"""
int main() {
    assert(&dialog_default_style() == &DialogDefaults[36]);
    TABLE_WITNESS

    dialog_default_style() = 0xA5u;
    assert(DialogDefaults[36] == 0xA5u);
    DialogDefaults[36] = 0x3Cu;
    assert(dialog_default_style() == 0x3Cu);

    // The pinned slot-3 color row writes [33 + 3], the same style word.
    dialog_set_def_dialog_text_color(101, 102, 103, 104, 3);
    assert(DialogDefaults[27] == 101u);
    assert(DialogDefaults[30] == 102u);
    assert(DialogDefaults[33] == 103u);
    assert(DialogDefaults[36] == 104u);
    assert(dialog_default_style() == 104u);

    // Keep the original out-of-range fallback to slot zero.
    dialog_set_def_dialog_text_color(201, 202, 203, 204, 4);
    assert(DialogDefaults[24] == 201u);
    assert(DialogDefaults[27] == 202u);
    assert(DialogDefaults[30] == 203u);
    assert(DialogDefaults[33] == 204u);
    return 0;
}
"""


def compile_and_run(tmp_path, compiler, *, mutant=False):
    source_text = SOURCE.read_text()
    header_text = HEADER.read_text()
    table = extract_definition(source_text, "uint32_t DialogDefaults[37] = {")
    setter = extract_definition(
        source_text, "void __cdecl dialog_set_def_dialog_text_color("
    )
    assert "RECOVERY_OBJECT: v1 0x0069704C ?DialogDefaults@@3PAIA" in source_text
    assert "0x00609D84" in header_text
    assert "0x00609D50" in source_text

    if mutant:
        old = "inline uint32_t &dialog_default_style() {\n    return DialogDefaults[36];"
        new = ("extern uint32_t detached_style;\n"
               "inline uint32_t &dialog_default_style() {\n    return detached_style;")
        assert old in header_text
        (tmp_path / "dialog_defaults.h").write_text(header_text.replace(old, new, 1))
    else:
        (tmp_path / "dialog_defaults.h").write_text(header_text)

    # Header is included before the body, so the mutant owns a real detached
    # global in this test executable rather than a copied/fake accessor body.
    prefix = ("#include <assert.h>\n#include <stdint.h>\n"
              "#define __cdecl\n#include \"dialog_defaults.h\"\n")
    detached = "uint32_t detached_style = 3u;\n" if mutant else ""
    call = "void __cdecl dialog_set_def_dialog_text_color(int, int, int, int, int);\n"
    body = BEHAVIOR.replace("TABLE_WITNESS", TABLE_WITNESS)
    text = prefix + detached + call + table + "\n" + setter + "\n" + body
    path = tmp_path / ("dialog-style-mutant.cpp" if mutant else "dialog-style.cpp")
    executable = tmp_path / ("dialog-style-mutant" if mutant else "dialog-style")
    path.write_text(text)
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(path), "-o", str(executable)],
        check=True, capture_output=True, text=True, timeout=30,
    )
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_table_accessor_and_slot_three_share_one_cell(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(tmp_path, compiler)
    assert result.returncode == 0, result.stderr


def test_detached_style_cell_mutant_is_rejected(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    result = compile_and_run(tmp_path, compiler, mutant=True)
    assert result.returncode != 0, "a detached style cell must fail shared-storage witnesses"
