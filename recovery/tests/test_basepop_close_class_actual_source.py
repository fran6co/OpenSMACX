"""C++98 controls compiled from the live BasePop::close_class definition."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/basepop.cpp"


def definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError("unterminated BasePop::close_class")


def compile_and_run(compiler, body, tmp_path):
    fixture = r'''#include <assert.h>
#include <stdlib.h>
#include <string.h>
#define __cdecl

struct Font {};
class Win {
public:
    virtual ~Win();
};
class BasePop { public: static void close_class(); };
Font *BasePopDefaultStringFonts[4];
Font *BasePopDefaultButtonFonts[3];
char *BasePopDefaultOkText;
char *BasePopDefaultCancelText;
Win *PopupInstanceSlotA;
Win *PopupInstanceSlotB;

static int derived_destroyed[4];
static int delete_calls;
static bool first_dtor_saw_own_slot;
static Win *replacement;
static Win *original_second;
void operator delete(void *memory) throw() {
    ++delete_calls;
    free(memory);
}
void operator delete[](void *memory) throw() {
    ++delete_calls;
    free(memory);
}
Win::~Win() {
    if (this == PopupInstanceSlotA) first_dtor_saw_own_slot = true;
}
class TrackedWin : public Win {
public:
    explicit TrackedWin(int id) : id_(id) {}
    virtual ~TrackedWin() {
        derived_destroyed[id_] = 1;
        if (id_ == 1) PopupInstanceSlotB = replacement;
    }
private:
    int id_;
};

''' + body + r'''

int main() {
    Font string_font, button_font;
    BasePopDefaultStringFonts[0] = &string_font;
    BasePopDefaultButtonFonts[0] = &button_font;
    BasePopDefaultOkText = (char *)malloc(4);
    BasePopDefaultCancelText = (char *)malloc(4);
    strcpy(BasePopDefaultOkText, "ok");
    strcpy(BasePopDefaultCancelText, "no");
    PopupInstanceSlotA = new TrackedWin(1);
    original_second = new TrackedWin(2);
    replacement = new TrackedWin(3);
    PopupInstanceSlotB = original_second;
    BasePop::close_class();

    assert(BasePopDefaultStringFonts[0] == 0);
    assert(BasePopDefaultButtonFonts[0] == 0);
    assert(BasePopDefaultOkText == 0 && BasePopDefaultCancelText == 0);
    assert(derived_destroyed[1] && derived_destroyed[3]);
    assert(!derived_destroyed[2]);
    assert(first_dtor_saw_own_slot);
    assert(delete_calls == 2);
    assert(PopupInstanceSlotA == 0 && PopupInstanceSlotB == 0);
    delete original_second;
    assert(delete_calls == 3);

    // Each global has its own null branch; the other slot remains live.
    PopupInstanceSlotA = 0;
    PopupInstanceSlotB = new TrackedWin(2);
    BasePop::close_class();
    assert(derived_destroyed[2]);
    assert(delete_calls == 4);
    assert(PopupInstanceSlotA == 0 && PopupInstanceSlotB == 0);

    PopupInstanceSlotA = new TrackedWin(0);
    PopupInstanceSlotB = 0;
    BasePop::close_class();
    assert(derived_destroyed[0]);
    assert(delete_calls == 5);
    assert(PopupInstanceSlotA == 0 && PopupInstanceSlotB == 0);
    return 0;
}
'''
    cpp = tmp_path / "close_class.cpp"
    executable = tmp_path / "close_class"
    cpp.write_text(fixture)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True,
                          text=True, timeout=10)


def body_mutants(body):
    missing_delete = body.replace(
        "delete PopupInstanceSlotA;",
        "PopupInstanceSlotA->Win::~Win();", 1,
    )
    premature_clear = body.replace(
        "        delete PopupInstanceSlotA;\n        PopupInstanceSlotA = 0;",
        "        PopupInstanceSlotA = 0;\n        delete PopupInstanceSlotA;", 1,
    )
    cached_second = body.replace(
        "    if (PopupInstanceSlotA != 0) {",
        "    Win *cached_second = PopupInstanceSlotB;\n"
        "    if (PopupInstanceSlotA != 0) {", 1,
    ).replace("delete PopupInstanceSlotB;", "delete cached_second;", 1)
    assert missing_delete != body
    assert premature_clear != body
    assert cached_second != body
    return {
        "missing_delete": missing_delete,
        "premature_clear": premature_clear,
        "cached_second": cached_second,
    }


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_live_close_class_uses_virtual_delete_and_preserves_order(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    body = definition(source, "void __cdecl BasePop::close_class()")
    # Structural anchor: ensure this is the intended native lifecycle recovery.
    assert "delete PopupInstanceSlotA;" in body
    assert "delete PopupInstanceSlotB;" in body
    result = compile_and_run(compiler, body, tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", ["missing_delete", "premature_clear", "cached_second"])
def test_live_close_class_controls_reject_lifecycle_mutants(
        compiler_name, mutation, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    body = definition(source, "void __cdecl BasePop::close_class()")
    assert "delete PopupInstanceSlotA;" in body
    assert "delete PopupInstanceSlotB;" in body
    mutated = body_mutants(body)[mutation]
    result = compile_and_run(compiler, mutated, tmp_path)
    assert result.returncode != 0, mutation + " escaped the lifecycle controls"

