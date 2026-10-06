"""Source-extracted behavior controls for the two popup producers.

The two companion C++ files are host adapter fixtures with a single insertion
slot. Product behavior is always extracted from its mapped source record; this
test has no dependency on the private body fragments or seed files.
"""

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import read_file


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/popup.cpp").is_file())
PACKET = Path(__file__).resolve().parent / "fixtures/popup_producers"
OWNERS = {
    "popb": {
        "source": ROOT / "src/popup.cpp",
        "address": 0x0048C650,
        "original": "?popb@@YAHPBDHHPBDPAUSprite@@@Z",
        "emitted": "?popb@@YAHPBDHH0PAVSprite@@@Z",
        "signature": "int __cdecl popb(",
        "spans": ((0x0048C650, 0x0048CEA2), (0x00658411, 0x006585CB)),
        "fixture": "popb-host.cpp",
        "slot": "/*__POPUP_BODY__*/",
    },
    "popm": {
        "source": ROOT / "src/popmenu.cpp",
        "address": 0x0059D3B0,
        "original": "?popm@@YAHPADPADHHP6AHXZ@Z",
        "emitted": "?popm@@YAHPAD0HHP6AHXZ@Z",
        "signature": "int __cdecl popm(char *text_file, char *text_label,",
        "spans": ((0x0059D3B0, 0x0059D690), (0x00661200, 0x006612E8)),
        "fixture": "popm-host.cpp",
        "slot": "/*__POPM_BODY__*/",
    },
}


def _masked(source: str) -> str:
    out = list(source)
    state = "code"
    quote = ""
    index = 0
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and next_char == "/":
                out[index] = out[index + 1] = " "
                state = "line"
                index += 2
                continue
            if char == "/" and next_char == "*":
                out[index] = out[index + 1] = " "
                state = "block"
                index += 2
                continue
            if char in ("'", '"'):
                quote = char
                out[index] = " "
                state = "string"
        elif state == "line":
            if char == "\n":
                state = "code"
            else:
                out[index] = " "
        elif state == "block":
            if char == "*" and next_char == "/":
                out[index] = out[index + 1] = " "
                state = "code"
                index += 2
                continue
            if char != "\n":
                out[index] = " "
        else:
            if char == "\\":
                out[index] = " "
                if index + 1 < len(source):
                    out[index + 1] = " "
                index += 2
                continue
            if char == quote:
                state = "code"
            out[index] = " "
        index += 1
    return "".join(out)


def _actual_body(owner: str) -> str:
    spec = OWNERS[owner]
    records = [row for row in read_file(spec["source"])
               if row.address == spec["address"]]
    assert len(records) == 1, f"missing/duplicate original owner {owner}"
    record = records[0]
    assert record.name == spec["original"]
    assert record.image_spans == spec["spans"], "primary and EH spans must both survive"
    assert record.region and "BODY GOES HERE." not in record.region
    assert "// symbol    " + spec["emitted"] in record.region
    masked = _masked(record.region)
    start = masked.find(spec["signature"])
    assert start >= 0, f"missing mapped definition for {owner}"
    brace = masked.find("{", start)
    assert brace >= 0
    depth = 0
    for index in range(brace, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return record.region[start:index + 1]
    raise AssertionError(f"unterminated actual {owner} definition")


def _compile_fixture(owner: str, body: str) -> subprocess.CompletedProcess[str]:
    spec = OWNERS[owner]
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "host control requires g++ or clang++"
    fixture = (PACKET / spec["fixture"]).read_text(encoding="utf-8")
    slot = spec["slot"]
    assert fixture.count(slot) == 1
    with tempfile.TemporaryDirectory(prefix="osmx-popup-actual-source-") as directory:
        temp = Path(directory)
        cpp = temp / spec["fixture"]
        binary = temp / "control"
        cpp.write_text(fixture.replace(slot, body, 1), encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
             str(cpp), "-o", str(binary)],
            capture_output=True, text=True, timeout=30,
        )
        assert built.returncode == 0, built.stderr
        return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)


def test_popb_actual_source_gates_focus_facility_warning_result_and_lifetime():
    result = _compile_fixture("popb", _actual_body("popb"))
    assert result.returncode == 0, result.stderr


def test_popm_actual_source_arguments_result_and_exception_cleanup():
    result = _compile_fixture("popm", _actual_body("popm"))
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize(("label", "before", "after"), [
    (
        "duplicate embedded owners",
        "    Popup popup;",
        "    Popup popup;\n    Dialogs duplicate_dialogs;\n    Sprite duplicate_picture;",
    ),
    (
        "early faction gate",
        "if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) == 0) {",
        "if (false) {",
    ),
    (
        "visibility focus",
        "|| (selected.visibility & viewer_bit) != 0;",
        "|| false;",
    ),
    (
        "facility threshold plus two",
        "(has_psi_facility ? 2 : 0)",
        "(has_psi_facility ? 0 : 0)",
    ),
    (
        "stored warning state rather than exec result",
        "if (popup.input_poll_state() != 0) {",
        "if (popup_result != 0) {",
    ),
    (
        "result three action",
        "action_staple(BaseIDCurrentSelected);",
        "prefs_save(BaseIDCurrentSelected);",
    ),
    (
        "sound fade",
        "g_FX.fade(sound_id);",
        "/* fade omitted */",
    ),
])
def test_popb_actual_source_mutants_are_rejected(label: str, before: str, after: str):
    body = _actual_body("popb")
    assert before in body, f"mutation anchor absent: {label}"
    mutant = body.replace(before, after, 1)
    assert mutant != body
    result = _compile_fixture("popb", mutant)
    assert result.returncode != 0, f"behavior mutant escaped control: {label}"
