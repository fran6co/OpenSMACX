"""Source-driven controls for BasePop's typed Dialog ownership accesses."""

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/basepop.cpp"


def definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unclosed definition: {signature}")


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>

struct Dialog {
    int selected_position_ = 0;
};
struct CheckBox : virtual Dialog {};
struct Dialogs : CheckBox {
    char *last_text = nullptr;
    int last_index = 0;
    int result = 0;
    int item(char *text, int index) {
        last_text = text;
        last_index = index;
        return result;
    }
};
class BasePop {
public:
    Dialogs dialogs_;
    int item(char *, int);
    uint32_t read_check();
};

/* ITEM_BODY */
/* CHECK_BODY */

int main() {
    BasePop pop;
    char text[] = "forward unchanged";
    pop.dialogs_.result = -0x1234567;
    assert(pop.item(text, -37) == -0x1234567);
    assert(pop.dialogs_.last_text == text);
    assert(pop.dialogs_.last_index == -37);

    const uint32_t states[] = {0u, 1u, 0x80000000u, 0x80000001u, 0xFFFFFFFFu};
    for (uint32_t expected : states) {
        std::memcpy(&pop.dialogs_.Dialog::selected_position_, &expected,
                    sizeof(expected));
        assert(pop.read_check() == expected);
    }
    return 0;
}
"""


def program() -> str:
    source = SOURCE.read_text()
    item = definition(source, "int BasePop::item(char *text, int index)")
    check = definition(source, "uint32_t BasePop::read_check()")
    assert "dialogs_.Dialogs::item(text, index)" in item
    assert "reinterpret_cast" not in item
    assert "static_cast<const CheckBox &>(dialogs_)" in check
    assert ".Dialog::selected_position_" in check
    assert "static_cast<uint32_t>" in check
    assert "reinterpret_cast" not in check
    return HARNESS.replace("/* ITEM_BODY */", item).replace(
        "/* CHECK_BODY */", check
    )


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_item_and_read_check_bodies_use_dialog_owners(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    with tempfile.TemporaryDirectory(prefix="basepop-dialog-owner-") as temp:
        cpp = Path(temp) / "actual.cpp"
        exe = Path(temp) / "actual"
        cpp.write_text(program())
        built = subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
             str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=30, check=False,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True,
                             timeout=10, check=False)
        assert ran.returncode == 0, ran.stdout + ran.stderr
