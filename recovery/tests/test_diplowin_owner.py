"""Actual-source DiploWin controls; not native layout/runtime proof."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

_HERE = Path(__file__).resolve()
ROOT = next((parent for parent in (_HERE.parent, *_HERE.parents)
             if (parent / "src/diplowin.cpp").is_file()), None)
if ROOT is None:
    _configured_root = os.environ.get("OPENSMACX_ROOT")
    if not _configured_root:
        raise RuntimeError("set OPENSMACX_ROOT when running this proposal from /tmp")
    ROOT = Path(_configured_root).resolve()


def definition(text, signature):
    masked = list(text)
    mode = "code"
    index = 0
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if mode == "code":
            if char == "/" and next_char == "/":
                masked[index] = masked[index + 1] = " "
                index += 2
                mode = "line"
                continue
            if char == "/" and next_char == "*":
                masked[index] = masked[index + 1] = " "
                index += 2
                mode = "block"
                continue
            if char == '"':
                masked[index] = " "
                mode = "string"
            elif char == "'":
                masked[index] = " "
                mode = "char"
        elif mode == "line":
            if char == "\n":
                mode = "code"
            else:
                masked[index] = " "
        elif mode == "block":
            masked[index] = " "
            if char == "*" and next_char == "/":
                masked[index + 1] = " "
                index += 2
                mode = "code"
                continue
        else:
            masked[index] = " "
            if char == "\\" and index + 1 < len(text):
                masked[index + 1] = " "
                index += 2
                continue
            if (mode == "string" and char == '"') or (mode == "char" and char == "'"):
                mode = "code"
        index += 1
    code = "".join(masked)
    match = re.search(signature + r"\s*\{", code)
    assert match, "missing product definition: " + signature
    opening = code.index("{", match.start())
    depth = 0
    for end in range(opening, len(code)):
        depth += (code[end] == "{") - (code[end] == "}")
        if depth == 0:
            return text[match.start():end + 1]
    raise AssertionError("unterminated product definition: " + signature)


def product_source():
    path = ROOT / "src/diplowin.cpp"
    text = path.read_text()
    signatures = (
        r"DiploWin::DiploWin\(\)",
        r"void DiploWin::on_redraw\(\)",
        r"void DiploWin::on_status\(int\s+\w+\)",
        r"int DiploWin::on_iface_dialog_item_back_draw\(GraphicWin\s*\*\s*window,\s*int(?:\s+\w+)?,\s*int\s+item,\s*RECT\s*\*\s*rect\)",
        r"void DiploWin::on_iface_scrolled\(int(?:\s+\w+)?,\s*int(?:\s+\w+)?\)",
        r"void DiploWin::on_iface_scrolling\(int(?:\s+\w+)?,\s*int(?:\s+\w+)?\)",
        r"int DiploWin::on_iface_button_pressed\(int(?:\s+\w+)?\)",
        r"void DiploWin::on_iface_group_clicked\(int(?:\s+\w+)?,\s*int(?:\s+\w+)?,\s*int(?:\s+\w+)?\)",
        r"int DiploWin::on_iface_dialog_back_draw\(GraphicWin\s*\*(?:\s*\w+)?\)",
    )
    definitions = [definition(text, signature) for signature in signatures]
    globals_ = []
    for name in ("DiploLocalizedTradeTitle", "DiploLocalizedDiplomacyTitle"):
        matches = re.findall(r"(?m)^(?:static\s+)?char " + name +
                             r"(?:\s*=\s*0)?;\s*$", text)
        assert len(matches) == 1, "expected one owned mutable title cell: " + name
        globals_.append(matches[0])
    return "\n\n".join(globals_ + definitions)


def test_product_hierarchy_and_member_construction_order():
    # The behavioral harness is not a native layout witness. Tie its intended
    # construction ordering to the actual header; native compilation separately
    # verifies the resulting ABI and constructor spelling.
    header = (ROOT / "src/diplowin.h").read_text()
    assert re.search(r"class DiploWin\s*:\s*public GraphicWin,\s*public SubInterface", header)
    assert "field_A14_" not in header and "field_A18_" not in header
    declarations = ("Spot spot_;", "FlatButton flatButtons_[13];",
                    "StringBox stringBox_;", "ListBox listBox_;", "EditBox editBox_;")
    positions = [header.index(declaration) for declaration in declarations]
    assert positions == sorted(positions)


def run(tmp_path, product, scenario, flags=()):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    template = Path(__file__).with_name("diplowin_owner_harness.cpp").read_text()
    cpp, exe = tmp_path / "owner.cpp", tmp_path / "owner"
    cpp.write_text(template.replace("/* PRODUCT */", product))
    command = [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
               "-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    command.extend(flags)
    command.extend([str(cpp), "-o", str(exe)])
    subprocess.run(command, check=True, capture_output=True, text=True)
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0",
               UBSAN_OPTIONS="halt_on_error=1")
    return subprocess.run([str(exe), scenario], capture_output=True, text=True,
                          timeout=15, env=env)


@pytest.mark.parametrize("scenario", [
    "ctor", "painter", "redraw", "titles", "status_on", "status_off",
])
def test_actual_diplowin_product_source(tmp_path, scenario):
    result = run(tmp_path, product_source(), scenario)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("old,new,scenario", [
    ("TextColourTable[80 - 2 * item]", "TextColourTable[80 - item]", "painter"),
    ("TextColourTable[81 - 2 * item]", "TextColourTable[81 - item]", "painter"),
    ("stringBox_.show(0);\n    editBox_.show(0);",
     "editBox_.show(0);\n    stringBox_.show(0);", "status_on"),
    ("front->Win::show(3)", "front->show(3)", "status_on"),
    ("editBox_.show(0)", "editBox_.children_[87]->show(0)", "status_on"),
    ('strcat(StringTemp, ": ");', 'strcat(StringTemp, ")");', "redraw"),
])
def test_diplo_native_branch_damage_is_caught(tmp_path, old, new, scenario):
    source = product_source()
    assert old in source
    result = run(tmp_path, source.replace(old, new, 1), scenario)
    assert result.returncode != 0, "deliberate method damage escaped actual-source check"


@pytest.mark.parametrize("damage_flag", ["-DDAMAGE_BASE_ORDER", "-DDAMAGE_MEMBER_ORDER"])
def test_constructor_order_damage_is_caught(tmp_path, damage_flag):
    result = run(tmp_path, product_source(), "ctor", [damage_flag])
    assert result.returncode != 0, "deliberate typed subobject order damage escaped control"
