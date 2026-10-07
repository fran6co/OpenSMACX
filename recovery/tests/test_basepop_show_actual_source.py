"""Proposal control for BasePop's three source-level show calls.

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


def actual_expressions(body):
    # Each expression is read only from its own setup region; other show calls
    # elsewhere in this large function cannot affect this extraction.
    regions = (
        body[body.index("// THE FIRST ROW"):body.index("// THE ITEM ROWS")],
        body[body.index("rows:"):body.index("if ((field_30A8_ & 0x40) == 0)")],
        body[body.index("if ((field_30A8_ & 0x40) == 0)"):body.index("pump:")],
    )
    expressions = []
    for region, receiver in zip(regions, ("flat_button1_", "button", "flat_button2_")):
        matches = re.findall(r"\b" + re.escape(receiver) + r"(?:->|\.)show\(\s*\d+\s*\);", region)
        assert len(matches) == 1, (receiver, matches)
        expressions.append(matches[0])
    return expressions, body


def validate_proposal(expressions):
    return expressions == ["flat_button1_.show(0);", "button->show(0);", "flat_button2_.show(0);"]


def fixture(expressions, show_declaration):
    declaration_with_body = show_declaration[:-1] + r''' { std::cout << "Win " << identity << " " << visible << '\n'; }'''
    return r'''#include <iostream>
struct Win {
  int identity;
  explicit Win(int value) : identity(value) {}
''' + declaration_with_body + r'''
};
struct GraphicWin : Win { explicit GraphicWin(int value) : Win(value) {} };
struct BaseButton : GraphicWin { explicit BaseButton(int value) : GraphicWin(value) {} };
struct FlatButton : BaseButton { explicit FlatButton(int value) : BaseButton(value) {} };
struct DerivedFlatButton : FlatButton {
  explicit DerivedFlatButton(int value) : FlatButton(value) {}
  virtual void show(int visible) {
    std::cout << "Derived " << identity << " " << visible << '\n';
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


def test_live_source_hierarchy_and_dispatch_fixture(tmp_path):
    source = read("src/basepop.cpp")
    body = function_body(source, "int BasePop::exec(int flags,")
    expressions, transformed = actual_expressions(body)

    win = read("src/win.h")
    declaration = re.search(r"virtual\s+void\s+show\s*\(\s*int\s+visible\s*\)\s*;", win)
    assert declaration, "Win::show(int visible) virtual declaration missing"
    assert re.search(r"class\s+GraphicWin\s*:\s*public\s+Win\s*,\s*public\s+Buffer", read("src/graphicwin.h"))
    assert re.search(r"class\s+BaseButton\s*:\s*public\s+GraphicWin", read("src/basebutton.h"))
    assert re.search(r"class\s+FlatButton\s*:\s*public\s+BaseButton", read("src/flatbutton.h"))
    assert all(expr in transformed for expr in expressions)
    assert validate_proposal(expressions)

    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers, "neither GCC nor Clang is available"
    program = fixture(expressions, declaration.group(0))
    expected = "Win 1 0\nDerived 3 0\nWin 2 0\n"
    for compiler in compilers:
        binary = tmp_path / (compiler.replace("+", "p"))
        subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic", "-o", str(binary), "-x", "c++", "-"],
                       input=program, text=True, check=True, capture_output=True)
        output = subprocess.run([str(binary)], text=True, check=True, capture_output=True).stdout
        assert output == expected, (compiler, output)


def test_runtime_rejects_dispatch_argument_and_receiver_mutants(tmp_path):
    source = read("src/basepop.cpp")
    body = function_body(source, "int BasePop::exec(int flags,")
    expressions, _ = actual_expressions(body)
    expected = ["flat_button1_.show(0);", "button->show(0);", "flat_button2_.show(0);"]
    for mutant in (
        [expected[0], "button->Win::show(0);", expected[2]],
        [expected[0], "button->show(1);", expected[2]],
        ["flat_button2_.show(0);", expected[1], "flat_button1_.show(0);"],
    ):
        assert mutant != expected
        assert not validate_proposal(mutant)
    win = read("src/win.h")
    declaration = re.search(r"virtual\s+void\s+show\s*\(\s*int\s+visible\s*\)\s*;", win)
    assert declaration
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    expected_trace = "Win 1 0\nDerived 3 0\nWin 2 0\n"
    for compiler in compilers:
        for index, mutant in enumerate((
            [expressions[0], "button->Win::show(0);", expressions[2]],
            [expressions[0], "button->show(1);", expressions[2]],
            ["flat_button2_.show(0);", expressions[1], "flat_button1_.show(0);"],
        )):
            binary = tmp_path / (compiler.replace("+", "p") + "-mutant-" + str(index))
            program = fixture(mutant, declaration.group(0))
            subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic", "-o", str(binary), "-x", "c++", "-"],
                           input=program, text=True, check=True, capture_output=True)
            output = subprocess.run([str(binary)], text=True, check=True, capture_output=True).stdout
            assert output != expected_trace, (compiler, mutant, output)

