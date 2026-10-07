"""Proposal control for BasePop's three source-level redraw calls.

This bounded host fixture checks C++ virtual dispatch syntax and ordering. It
does not establish VC6 ABI details or behavior in the game executable.
"""
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def read(path):
    return (ROOT / path).read_text(encoding="utf-8")


def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 1
    for pos in range(brace + 1, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:pos]
    raise AssertionError("unterminated function body")


def actual_calls(body):
    # Bound extraction to each setup region so unrelated redraws elsewhere in
    # BasePop::exec cannot satisfy or disturb the control.
    regions = (
        body[body.index("// THE FIRST ROW"):body.index("// THE ITEM ROWS")],
        body[body.index("rows:"):body.index("if ((field_30A8_ & 0x40) == 0)")],
        body[body.index("// THE LAST ROW"):body.index("flat_button2_.show(0);") + len("flat_button2_.show(0);")],
    )
    expected = ("flat_button1_", "button", "flat_button2_")
    expressions = []
    for region, receiver in zip(regions, expected):
        matches = re.findall(
            r"\b" + re.escape(receiver) + r"(?:->|\.)redraw\(\s*\);", region
        )
        assert len(matches) == 1, (receiver, matches)
        expressions.append(matches[0])
    return expressions, regions


def validate_proposal(expressions):
    return expressions == [
        "flat_button1_.redraw();", "button->redraw();", "flat_button2_.redraw();"
    ]


def fixture(expressions, win_declaration, graphic_declaration):
    win_body = re.sub(r"\s*\{\s*\}$", "", win_declaration) + r''' {
    std::cout << "Win " << identity << '\n';
  }'''
    return r'''#include <iostream>
struct Win {
  int identity;
  explicit Win(int value) : identity(value) {}
''' + win_body + r'''
};
struct Buffer {};
struct GraphicWin : public Win, public Buffer {
  explicit GraphicWin(int value) : Win(value) {}
''' + re.sub(r";\s*$", "", graphic_declaration) + r''' {
    std::cout << "Graphic " << identity << '\n';
  }
};
struct BaseButton : public GraphicWin {
  explicit BaseButton(int value) : GraphicWin(value) {}
};
struct FlatButton : public BaseButton {
  explicit FlatButton(int value) : BaseButton(value) {}
};
struct DerivedFlatButton : public FlatButton {
  explicit DerivedFlatButton(int value) : FlatButton(value) {}
  virtual void redraw() {
    std::cout << "Derived " << identity << '\n';
  }
};
int main() {
  FlatButton flat_button1_(1);
  FlatButton flat_button2_(2);
  DerivedFlatButton derived(3);
  FlatButton *button = &derived;
''' + "\n".join("  " + expression for expression in expressions) + r'''
  return 0;
}
'''


def live_source():
    source = read("src/basepop.cpp")
    body = function_body(source, "int BasePop::exec(int flags,")
    expressions, regions = actual_calls(body)

    win = read("src/win.h")
    win_declaration = re.search(r"virtual\s+void\s+redraw\s*\(\s*\)\s*\{\s*\}", win)
    graphic = read("src/graphicwin.h")
    graphic_declaration = re.search(r"virtual\s+void\s+redraw\s*\(\s*\);", graphic)
    assert win_declaration, "Win::redraw() virtual body missing"
    assert graphic_declaration, "GraphicWin::redraw() virtual declaration missing"
    assert re.search(r"class\s+GraphicWin\s*:\s*public\s+Win\s*,\s*public\s+Buffer", graphic)
    assert re.search(r"class\s+BaseButton\s*:\s*public\s+GraphicWin", read("src/basebutton.h"))
    assert re.search(r"class\s+FlatButton\s*:\s*public\s+BaseButton", read("src/flatbutton.h"))
    assert validate_proposal(expressions)

    for region, receiver in zip(regions, ("flat_button1_", "button", "flat_button2_")):
        font = region.index(".set_font(") if receiver != "button" else region.index("->set_font(")
        redraw = region.index(".redraw(") if receiver != "button" else region.index("->redraw(")
        first_color = region.index(".set_text_color(") if receiver != "button" else region.index("->set_text_color(")
        show = region.index(".show(0);") if receiver != "button" else region.index("->show(0);")
        assert font < redraw < first_color < show, receiver
    assert ".show(0);" in regions[0]
    assert "->show(0);" in regions[1]
    assert ".show(0);" in regions[2]
    return expressions, win_declaration.group(0), graphic_declaration.group(0)


def compiler_names():
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers, "neither GCC nor Clang is available"
    return compilers


def run_fixture(tmp_path, compiler, expressions, win_decl, graphic_decl, stem):
    binary = tmp_path / (compiler.replace("+", "p") + stem)
    program = fixture(expressions, win_decl, graphic_decl)
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic", "-o", str(binary), "-x", "c++", "-"],
        input=program, text=True, check=True, capture_output=True,
    )
    return subprocess.run([str(binary)], text=True, check=True, capture_output=True).stdout


def test_live_source_hierarchy_and_dispatch_fixture(tmp_path):
    expressions, win_decl, graphic_decl = live_source()
    expected = "Graphic 1\nDerived 3\nGraphic 2\n"
    for compiler in compiler_names():
        assert run_fixture(tmp_path, compiler, expressions, win_decl, graphic_decl, "-live") == expected


def test_runtime_rejects_qualified_swapped_and_omitted_calls(tmp_path):
    expressions, win_decl, graphic_decl = live_source()
    expected = "Graphic 1\nDerived 3\nGraphic 2\n"
    mutants = (
        [expressions[0], "button->Win::redraw();", expressions[2]],
        [expressions[2], expressions[1], expressions[0]],
        [expressions[0], expressions[2]],
    )
    for compiler in compiler_names():
        for index, mutant in enumerate(mutants):
            output = run_fixture(tmp_path, compiler, mutant, win_decl, graphic_decl, "-mutant-" + str(index))
            assert output != expected, (compiler, mutant, output)

