"""Host checks against the actual mapped Console::give_hints C++ body."""
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/console.cpp"
HARNESS = Path(__file__).with_name("console_give_hints_harness.cpp")
HELPERS = (
    "give_hints_wrap_x",
    "give_hints_in_map",
    "give_hints_parse_faction_identity",
    "give_hints_map",
)


def _definition(text, name, signature_pattern):
    match = re.search(signature_pattern, text)
    assert match, "missing source definition: " + name
    brace = text.find("{", match.end())
    assert brace >= 0, "definition has no body: " + name
    depth = 0
    quote = None
    escaped = False
    line_comment = block_comment = False
    i = brace
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if line_comment:
            if ch == "\n":
                line_comment = False
        elif block_comment:
            if ch == "*" and nxt == "/":
                block_comment = False
                i += 1
        elif quote:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                quote = None
        elif ch == "/" and nxt == "/":
            line_comment = True
            i += 1
        elif ch == "/" and nxt == "*":
            block_comment = True
            i += 1
        elif ch in ('"', "'"):
            quote = ch
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():i + 1]
        i += 1
    raise AssertionError("unterminated C++ body: " + name)


def _actual_source():
    text = SOURCE.read_text(encoding="utf-8")
    records = [record for record in read_file(SOURCE) if record.address == 0x00511BB0]
    assert len(records) == 1, "give_hints lost its unique original source identity"
    method = _definition(text, "Console::give_hints", r"void\s+Console::give_hints\s*\(\s*\)")
    assert "BODY GOES HERE" not in method, "give_hints is still an unfinished placeholder"
    helpers = []
    for name in HELPERS:
        helpers.append(_definition(
            text, name,
            r"static\s+(?:int|bool|void)\s+" + re.escape(name) + r"\s*\([^;]*?\)",
        ))
    return "\n\n".join(helpers + [method])


def _compile(tmp_path, actual):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    template = HARNESS.read_text(encoding="utf-8")
    marker = "/* __EXTRACTED_GIVE_HINTS_SOURCE__ */"
    assert template.count(marker) == 1
    cpp = tmp_path / "console_give_hints_host.cpp"
    exe = tmp_path / "console_give_hints_host"
    cpp.write_text(template.replace(marker, actual), encoding="utf-8")
    command = [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
               "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
               str(cpp), "-o", str(exe)]
    result = subprocess.run(command, capture_output=True, text=True, timeout=45)
    assert result.returncode == 0, result.stdout + result.stderr
    return exe


@pytest.fixture(scope="module")
def host_binary(tmp_path_factory):
    return _compile(tmp_path_factory.mktemp("give-hints"), _actual_source())


@pytest.mark.parametrize("scenario", [
    "state-clear", "colonist", "stage-owner", "game-state-order",
    "site-owner", "no-base-check", "valid-unity",
])
def test_actual_source_scenarios(host_binary, scenario):
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0", UBSAN_OPTIONS="halt_on_error=1")
    result = subprocess.run([str(host_binary), scenario], capture_output=True,
                            text=True, timeout=15, env=env)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("name,scenario,old,new", [
    ("state-bit enclosure", "state-clear",
     "if ((Vehs[selected_vehicle_].state & 0x2000u) != 0)", "if (true)"),
    ("stage owner", "stage-owner",
     "TutorialWindow.set_tour_stage(2);", "DesignTutorState = 2;"),
    ("GameState timing", "game-state-order",
     "GameState |= 0x400;", "GameState |= 0x400; /* timing control */"),
    ("site-owner exclusion", "site-owner",
     "!= candidate_faction", "== candidate_faction"),
    ("no-base exploration suppression", "no-base-check",
     "if (tut_check(0x2000)", "if (tut_check(0x2000)"),
])
def test_damage_controls_are_detected(tmp_path, name, scenario, old, new):
    source = _actual_source()
    if name == "GameState timing":
        # Move the state update before the map callback: the fixture records
        # GameState at callback entry, so this should violate the oracle.
        source = source.replace(old, "", 1)
        source = source.replace('TutorialWindow.tut_map("NEWTERRAFORMER"',
                                "GameState |= 0x400;\n" +
                                'TutorialWindow.tut_map("NEWTERRAFORMER"', 1)
    elif name == "no-base exploration suppression":
        # Inject the erroneous no-goody checker into the base-scan path.
        anchor = "if ((TutorialFlags & 0x8000000) == 0)"
        assert anchor in source
        source = source.replace(anchor, "(void)tut_check(0x2000);\n    " + anchor, 1)
    else:
        assert old in source, name + " mutation anchor missing"
        source = source.replace(old, new, 1)
    binary = _compile(tmp_path, source)
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0", UBSAN_OPTIONS="halt_on_error=1")
    result = subprocess.run([str(binary), scenario], capture_output=True,
                            text=True, timeout=15, env=env)
    assert result.returncode != 0, name + " damage control unexpectedly survived"
