"""Exercise Gamma's real declaration and close body with logging subobjects.

This host fixture checks C++ ordering around the recovered class, not Gamma's
game ABI, original constructor code generation, or Win32 runtime behavior.
"""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_game_logic_seed_batch_behavior import _definition, _masked_cpp


ROOT = Path(__file__).resolve().parents[2]


def class_declaration(text: str) -> str:
    signature = "class Gamma : public GraphicWin"
    masked = _masked_cpp(text)
    start = masked.index(signature)
    opening = masked.index("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                end = index + 1
                while end < len(masked) and masked[end].isspace():
                    end += 1
                if end >= len(masked) or masked[end] != ";":
                    raise AssertionError("Gamma class declaration has no terminator")
                return text[start:end + 1]
    raise AssertionError("unterminated Gamma class declaration")


def actual_fragments():
    header = (ROOT / "src/gamma.h").read_text(encoding="utf-8")
    source = (ROOT / "src/gamma.cpp").read_text(encoding="utf-8")
    return (
        class_declaration(header),
        _definition(source, "Gamma::Gamma()"),
        _definition(source, "void Gamma::close()"),
    )


STUBS = r'''#include <cassert>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
static std::vector<std::string> events;
static const char *throw_from_constructor;
static struct Palette **watched_target;

static void construct(const char *name) {
    events.push_back(std::string(name) + ".ctor");
    if (throw_from_constructor != 0 && std::string(throw_from_constructor) == name)
        throw 17;
}

struct Palette {
    Palette() { construct("Palette"); }
    ~Palette() { events.push_back("Palette.dtor"); }
    void close() {
        events.push_back(watched_target != 0 && *watched_target == 0
                         ? "Palette.close.null" : "Palette.close.nonnull");
    }
};
struct Font {
    Font() { construct("Font"); }
    ~Font() { events.push_back("Font.dtor"); }
};
struct Scroll {
    Scroll() { construct("Scroll"); }
    ~Scroll() { events.push_back("Scroll.dtor"); }
    void close() { events.push_back("Scroll.close"); }
};
struct EditBox {
    EditBox() { construct("EditBox"); }
    ~EditBox() { events.push_back("EditBox.dtor"); }
    void close() { events.push_back("EditBox.close"); }
};
struct PushButton {
    static int next_id;
    int id;
    PushButton() : id(++next_id) {
        construct(id == 1 ? "PushButton1" : "PushButton2");
    }
    ~PushButton() {
        events.push_back(id == 1 ? "PushButton1.dtor" : "PushButton2.dtor");
    }
    void close() {
        events.push_back(id == 1 ? "PushButton1.close" : "PushButton2.close");
    }
};
int PushButton::next_id;

struct GraphicWin {
    GraphicWin() { construct("GraphicWin"); }
    virtual ~GraphicWin() { events.push_back("GraphicWin.dtor"); }
    void close() { events.push_back("GraphicWin.close"); }
};
'''


MAIN = r'''
void Gamma::on_scrolled(int, int) {}
void Gamma::on_scrolling(int, int) {}
void Gamma::on_redraw() {}

static void reset_events() {
    events.clear();
    throw_from_constructor = 0;
    watched_target = 0;
    PushButton::next_id = 0;
}

static std::vector<std::string> live_object_events() {
    return {
        "GraphicWin.ctor", "Palette.ctor", "Font.ctor", "Scroll.ctor",
        "EditBox.ctor", "PushButton1.ctor", "PushButton2.ctor",
        "Palette.close.null", "Scroll.close", "EditBox.close",
        "PushButton1.close", "PushButton2.close", "GraphicWin.close",
        "PushButton2.dtor", "PushButton1.dtor", "EditBox.dtor",
        "Scroll.dtor", "Font.dtor", "Palette.dtor", "GraphicWin.dtor"
    };
}

int main() {
    Palette target;

    reset_events();
    {
        Gamma gamma;
        gamma.target_palette_ = &target;
        watched_target = &gamma.target_palette_;
    }
    assert(events == live_object_events());

    reset_events();
    try {
        Gamma gamma;
        gamma.target_palette_ = &target;
        watched_target = &gamma.target_palette_;
        throw 23;
    } catch (int value) {
        assert(value == 23);
    }
    assert(events == live_object_events());

    reset_events();
    GraphicWin *base = new Gamma;
    Gamma *gamma = static_cast<Gamma *>(base);
    gamma->target_palette_ = &target;
    watched_target = &gamma->target_palette_;
    delete base;
    assert(events == live_object_events());

    reset_events();
    throw_from_constructor = "EditBox";
    try {
        Gamma failed;
        assert(!"EditBox construction should throw");
    } catch (int value) {
        assert(value == 17);
    }
    const std::vector<std::string> failed_prefix = {
        "GraphicWin.ctor", "Palette.ctor", "Font.ctor", "Scroll.ctor",
        "EditBox.ctor", "Scroll.dtor", "Font.dtor", "Palette.dtor",
        "GraphicWin.dtor"
    };
    assert(events == failed_prefix);
    return 0;
}
'''


def harness_source(*, declaration=None, destructor_mutant=False):
    declaration, constructor, close = actual_fragments() if declaration is None else (
        declaration, *actual_fragments()[1:])
    if destructor_mutant:
        original = "~Gamma() { close(); }"
        assert declaration.count(original) == 1
        declaration = declaration.replace(original, "~Gamma() {}", 1)
    return (STUBS + "\n" + declaration + "\n" + constructor + "\n"
            + close + "\n" + MAIN)


def compile_and_run(compiler: str, tmp_path: Path, source: str):
    cpp = tmp_path / "gamma_inline_lifetime.cpp"
    exe = tmp_path / "gamma_inline_lifetime"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True,
                          timeout=10, check=False)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_gamma_lifetime_order_and_prefix_unwind(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    result = compile_and_run(compiler, tmp_path, harness_source())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_omitted_inline_close_destructor_mutant_is_detected(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    result = compile_and_run(
        compiler, tmp_path, harness_source(destructor_mutant=True))
    assert result.returncode != 0, "destructor without Gamma::close passed lifetime assertions"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_reordered_actual_member_declarations_are_detected(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    declaration, _constructor, _close = actual_fragments()
    scroll = "  Scroll scroll_;  // 0xEF0, IDB `scroll`, size == sizeof(Scroll)"
    edit_box = "  EditBox editBox_;  // 0x303C, IDB `editBox`, size == sizeof(EditBox)"
    original_pair = scroll + "\n" + edit_box
    reordered_pair = edit_box + "\n" + scroll
    assert declaration.count(original_pair) == 1
    reordered = declaration.replace(original_pair, reordered_pair, 1)
    result = compile_and_run(compiler, tmp_path, harness_source(declaration=reordered))
    assert result.returncode != 0, "reordered Gamma member declaration passed lifetime assertions"
