"""Host controls compiled from the actual PickWin/ListBox source bodies.

These doubles check body branch effects, call/receiver order, typed extraction
rectangles, and automatic member lifetime. They do not prove VC6 layout, byte
equivalence, original allocation extent, or game runtime behavior.
"""

import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


BODY_MARKERS = (
    "// PICKWIN_OWNER_BODY_EXTRACTS",
    "// LISTBOX_PICKER_STATE_EXTRACT",
    "// PICKWIN_GLOBAL_EXTRACT",
)
COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]


def repository_root() -> Path:
    configured = os.environ.get("OPENSMACX_SOURCE_ROOT")
    if configured:
        return Path(configured).resolve()
    return Path(__file__).resolve().parents[2]


def source_path(variable: str, relative: str) -> Path:
    configured = os.environ.get(variable)
    return Path(configured).resolve() if configured else repository_root() / relative


def extract_braced(text: str, pattern: str, description: str) -> str:
    match = re.search(pattern, text, re.MULTILINE)
    assert match is not None, f"missing {description}"
    opening = text.find("{", match.start())
    assert opening >= 0, f"missing opening brace for {description}"
    depth = 0
    quote = None
    escaped = False
    line_comment = False
    block_comment = False
    index = opening
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and following == "/":
                block_comment = False
                index += 1
        elif quote is not None:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char == "/" and following == "/":
            line_comment = True
            index += 1
        elif char == "/" and following == "*":
            block_comment = True
            index += 1
        elif char in ('"', "'"):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return text[match.start() : index + 1]
        index += 1
    raise AssertionError(f"unterminated {description}")


def extract_pickwin_sources() -> tuple[str, str, str]:
    cpp = source_path("PICKWIN_OWNER_SOURCE", "src/pickwin.cpp").read_text()
    header = source_path("PICKWIN_OWNER_LISTBOX_HEADER", "src/listbox.h").read_text()
    helper = extract_braced(cpp, r"^static void pickwin_init_text_button\s*\(",
                            "PickWin button initialization helper")
    signatures = (
        (r"^void PickWin::init\s*\(", "PickWin::init"),
        (r"^void PickWin::on_button_clicked\s*\(", "PickWin::on_button_clicked"),
        (r"^void PickWin::on_group_clicked\s*\(", "PickWin::on_group_clicked"),
        (r"^int PickWin::on_dialog_item_back_draw\s*\(", "PickWin::on_dialog_item_back_draw"),
        (r"^void PickWin::on_redraw\s*\(", "PickWin::on_redraw"),
        (r"^PickWin::PickWin\s*\(", "PickWin::PickWin"),
        (r"^PickWin::~PickWin\s*\(", "PickWin::~PickWin"),
    )
    bodies = [extract_braced(cpp, pattern, label) for pattern, label in signatures]
    list_method = extract_braced(header,
        r"^\s*void initialize_picker_state\s*\(\s*\)",
        "ListBox::initialize_picker_state")
    list_method = list_method.replace("void initialize_picker_state()",
                                     "void ListBox::initialize_picker_state()", 1)
    callback = extract_braced(cpp,
        r"^void __cdecl list_was_dblclicked\s*\([^;]*\)\s*\{",
        "list_was_dblclicked")
    global_match = re.search(r"(?m)^\s*PickWin PickWinGlobal;\s*$", cpp)
    assert global_match is not None, "PickWinGlobal source definition is missing"
    global_definition = global_match.group(0).strip()
    return "\n\n".join([helper] + bodies), list_method, "\n\n".join([global_definition, callback])


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_pickwin_actual_owner_bodies_and_lifetime(compiler):
    if compiler is None:
        pytest.skip("neither g++ nor clang++ is available")

    fixture_path = Path(__file__).with_name("pickwin_owner_host_control.cpp")
    fixture = fixture_path.read_text()
    body_extracts, list_extract, global_extract = extract_pickwin_sources()
    for marker in BODY_MARKERS:
        assert fixture.count(marker) == 1, f"fixture marker missing or repeated: {marker}"
    generated = fixture.replace(BODY_MARKERS[0], body_extracts)
    generated = generated.replace(BODY_MARKERS[1], list_extract)
    generated = generated.replace(BODY_MARKERS[2], global_extract)

    with tempfile.TemporaryDirectory(prefix="pickwin-owner-control-") as temp:
        temp_path = Path(temp)
        generated_path = temp_path / "pickwin_owner_host_control.cpp"
        binary_path = temp_path / "pickwin_owner_host_control"
        generated_path.write_text(generated)
        compiled = subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", str(generated_path),
             "-o", str(binary_path)],
            check=False, capture_output=True, text=True, timeout=30,
        )
        assert compiled.returncode == 0, compiled.stdout + compiled.stderr
        executed = subprocess.run([str(binary_path)], check=False,
                                  capture_output=True, text=True, timeout=30)
        assert executed.returncode == 0, executed.stdout + executed.stderr
