"""Strict host controls extracted only from the selected product source bodies.

Set OPENSMACX_ROOT when this scratch copy lives outside the repository. The
imported recovery/tests copy resolves the project root from its own location.
There is deliberately no proposal-tree or /tmp source fallback.
"""
import os
from pathlib import Path
import shutil
import subprocess

import pytest


def project_root() -> Path:
    configured = os.environ.get("OPENSMACX_ROOT")
    if configured:
        return Path(configured).resolve()
    test_path = Path(__file__).resolve()
    if len(test_path.parents) > 2 and (test_path.parents[2] / "src").is_dir():
        return test_path.parents[2]
    raise AssertionError("set OPENSMACX_ROOT; no scratch/proposal fallback is allowed")


ROOT = project_root()
HARNESS = Path(__file__).with_name("menu-owner-harness.cpp")
METHODS = (
    ("src/menu.cpp", "int Menu::init(Win *owner)"),
    ("src/menu.cpp", "int Menu::add_menu(int menu_id, char *text, int left_justified)"),
    ("src/menu.cpp", "int Menu::UNK10(int menu_id, char *text)"),
    ("src/menu.cpp", "void Menu::compute()"),
    ("src/menu.cpp", "void Menu::adjust()"),
    ("src/menu.cpp", "void Menu::on_left_down(int x, int y)"),
    ("src/menu.cpp", "void Menu::mouse_move(int x, int y)"),
    ("src/menu.cpp", "int Menu::on_key_click(int, int key)"),
    ("src/menu.cpp", "void Menu::on_redraw()"),
    ("src/pulldown.cpp", "int PullDown::on_key_click(int key_input, int key)"),
)


def extract_definition(source: str, signature: str) -> str:
    count = source.count(signature)
    if count != 1:
        raise AssertionError(f"expected one {signature!r} in product source, got {count}")
    start = source.index(signature)
    opening = source.find("{", start)
    if opening < 0:
        raise AssertionError(f"function body opening brace missing: {signature}")
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == "/" and n == "/":
                state = "line_comment"
                i += 2
                continue
            if c == "/" and n == "*":
                state = "block_comment"
                i += 2
                continue
            if c == '"':
                state = "string"
            elif c == "'":
                state = "char"
            elif c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    definition = source[start : i + 1]
                    if "BODY GOES HERE" in definition:
                        raise AssertionError(f"selected definition is still a placeholder: {signature}")
                    return definition
        elif state == "line_comment":
            if c == "\n":
                state = "code"
        elif state == "block_comment":
            if c == "*" and n == "/":
                state = "code"
                i += 2
                continue
        elif state in ("string", "char"):
            if c == "\\":
                i += 2
                continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    raise AssertionError(f"unclosed function body: {signature}")


def build_actual_source_translation_unit(tmp_path: Path) -> Path:
    if not ROOT.is_dir() or not (ROOT / "src").is_dir():
        raise AssertionError(f"project root is missing src/: {ROOT}")
    extracted = []
    for relative, signature in METHODS:
        source_path = ROOT / relative
        if not source_path.is_file():
            raise AssertionError(f"product source missing: {source_path}")
        extracted.append(extract_definition(source_path.read_text(encoding="utf-8"), signature))
    template = HARNESS.read_text(encoding="utf-8")
    marker = '#include "GENERATED_MENU_SOURCE"'
    if template.count(marker) != 1:
        raise AssertionError("host fixture insertion point is not unique")
    generated = tmp_path / "menu-product-source-host.cpp"
    generated.write_text(template.replace(marker, "\n\n".join(extracted)), encoding="utf-8")
    return generated


def test_vc6_mouse_overload_declaration_order_is_explicit():
    """Host tests cannot establish this ABI; native physical tables must."""
    header = (ROOT / "src/win.h").read_text(encoding="utf-8")
    two_arg = "virtual void on_mouse_move(int, int)"
    four_arg = "virtual void on_mouse_move(int x, int y, unsigned int keys, int from_parent)"
    assert header.count(two_arg) == 1
    assert header.count(four_arg) == 1
    assert header.index(two_arg) < header.index(four_arg)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_menu_and_pulldown_source_controls(tmp_path, compiler):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    source = build_actual_source_translation_unit(tmp_path)
    binary = tmp_path / f"menu-product-{compiler}"
    subprocess.run(
        [executable, "-std=c++03", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(source), "-o", str(binary)],
        check=True,
    )
    env = dict(os.environ)
    env["ASAN_OPTIONS"] = "detect_leaks=0"
    result = subprocess.run([str(binary)], check=True, capture_output=True, text=True, env=env)
    assert "host mock ABI only" in result.stdout
