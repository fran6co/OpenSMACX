"""Exercise StringBox's real class lifetime with logged subobjects.

This is a host C++ ordering test for the extracted class, constructor and close
body. Stub collaborators make no claim about VC6 ABI, original EH tables, or
the game's runtime.
"""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_game_logic_seed_batch_behavior import _definition, _masked_cpp


ROOT = Path(__file__).resolve().parents[2]


def _class_declaration(text: str) -> str:
    signature = "class StringBox : public GraphicWin"
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
                    raise AssertionError("StringBox declaration has no terminator")
                return text[start:end + 1]
    raise AssertionError("unterminated StringBox declaration")


def _actual_fragments():
    header = (ROOT / "src/stringbox.h").read_text(encoding="utf-8")
    source = (ROOT / "src/stringbox.cpp").read_text(encoding="utf-8")
    declaration = _class_declaration(header)
    assert "~StringBox() { close(); }" in declaration
    return (
        declaration,
        _definition(source, "StringBox::StringBox()"),
        _definition(source, "void StringBox::close()"),
    )


STUBS = r'''#include <cassert>
#include <string>
#include <vector>
#include <stdint.h>

static std::vector<std::string> events;
static const char *throw_from_constructor;

static void construct(const char *name) {
    events.push_back(std::string(name) + ".ctor");
    if (throw_from_constructor != 0 &&
        std::string(throw_from_constructor) == name)
        throw 17;
}

class Win {};
class StringList {
public:
    int current_position_;
    StringList() : current_position_(13) { construct("StringList"); }
    ~StringList() {
        events.push_back(current_position_ == 0 ? "StringList.dtor.zero"
                                                : "StringList.dtor.nonzero");
    }
    void remove_all() { events.push_back("StringList.remove_all"); }
};
class Scroll {
public:
    Scroll() { construct("Scroll"); }
    ~Scroll() { events.push_back("Scroll.dtor"); }
    void close() { events.push_back("Scroll.close"); }
};
class GraphicWin {
public:
    GraphicWin() { construct("GraphicWin"); }
    virtual ~GraphicWin() { events.push_back("GraphicWin.dtor"); }
    void close() { events.push_back("GraphicWin.close"); }
};
'''


MAIN = r'''
int StringBox::on_link_clicked(int, char *) { return 0; }
void StringBox::on_redraw() {}

static void reset_events() {
    events.clear();
    throw_from_constructor = 0;
}

static std::vector<std::string> complete_lifetime() {
    return {
        "GraphicWin.ctor", "Scroll.ctor", "StringList.ctor",
        "GraphicWin.close", "StringList.remove_all", "Scroll.close",
        "StringList.dtor.zero", "Scroll.dtor", "GraphicWin.dtor"
    };
}

static void make_and_destroy() {
    StringBox box;
}

int main() {
    reset_events();
    make_and_destroy();
    assert(events == complete_lifetime());

    reset_events();
    try {
        StringBox box;
        throw 23;
    } catch (int value) {
        assert(value == 23);
    }
    assert(events == complete_lifetime());

    reset_events();
    GraphicWin *base = new StringBox;
    delete base;
    assert(events == complete_lifetime());

    reset_events();
    throw_from_constructor = "StringList";
    try {
        StringBox failed;
        assert(!"StringList construction should throw");
    } catch (int value) {
        assert(value == 17);
    }
    const std::vector<std::string> prefix_unwind = {
        "GraphicWin.ctor", "Scroll.ctor", "StringList.ctor",
        "Scroll.dtor", "GraphicWin.dtor"
    };
    assert(events == prefix_unwind);
    return 0;
}
'''


def _harness(*, declaration=None, omit_close=False):
    actual_declaration, constructor, close = _actual_fragments()
    if declaration is None:
        declaration = actual_declaration
    if omit_close:
        original = "~StringBox() { close(); }"
        assert declaration.count(original) == 1
        declaration = declaration.replace(original, "~StringBox() {}", 1)
    return (STUBS + "\n" + declaration + "\n" + constructor + "\n"
            + close + "\n" + MAIN)


def _compile_and_run(compiler: str, tmp_path: Path, source: str):
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp = tmp_path / "stringbox_inline_lifetime.cpp"
    exe = tmp_path / "stringbox_inline_lifetime"
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
def test_actual_stringbox_lifetime_and_prefix_unwind(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    result = _compile_and_run(compiler, tmp_path, _harness())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_lifetime_mutants_are_detected(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    no_close = _compile_and_run(
        compiler, tmp_path / "no_close", _harness(omit_close=True))
    assert no_close.returncode != 0, "StringBox destructor without close passed"

    declaration, _, _ = _actual_fragments()
    assert declaration.count("  Scroll scroll_;") == 1
    assert declaration.count("  StringList stringList_;") == 1
    reordered = declaration.replace(
        "  Scroll scroll_;", "  StringList stringList_;\n  Scroll scroll_;", 1)
    duplicate = reordered.rfind("  StringList stringList_;")
    assert duplicate > reordered.index("  Scroll scroll_;\n")
    reordered = (reordered[:duplicate]
                 + reordered[duplicate + len("  StringList stringList_;\n"):])
    changed_order = _compile_and_run(
        compiler, tmp_path / "reordered_members",
        _harness(declaration=reordered))
    assert changed_order.returncode != 0, "reordered StringBox members passed"
