"""Focused controls for RadioButton's class-defined destructor lifetime.

The source-reader check protects the task's complete pinned identity set while
the coordinator relocates the five destructor phase mappings. The small host
control observes C++ destruction order only; it says nothing about VC6 ABI,
linked selection, or original-image matching.
"""

from __future__ import annotations

import json
import re
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.binding_source import parse as parse_bindings
from match_recovery.source import Mode, SourceRole, read_file, read_text

ROOT = next(
    path for path in Path(__file__).resolve().parents
    if (path / "src/jackal/radiobutton.h").is_file()
)
TASK = ROOT / "recovery/tasks/radiobutton-inline-lifetime-oct12c.json"
HEADER = ROOT / "src/jackal/radiobutton.h"
COMPILERS = [compiler for compiler in
             (shutil.which("g++"), shutil.which("clang++")) if compiler]

PHASES = {
    0x00406A90: (
        (0x00406A90, 0x00406AE3),
        "sub_406a90",
        "??1RadioButton@@UAE@XZ",
    ),
    0x00406F60: (
        (0x00406F60, 0x00406FE2),
        "??1RadioButton@@QAE@H@Z",
        "??_GRadioButton@@UAEPAXI@Z",
    ),
    0x00406F50: (
        (0x00406F50, 0x00406F58),
        "??1thunk1_RadioButton@@QAE@H@Z",
        "??_ERadioButton@@$4PPPPPPPM@A@AEPAXI@Z",
    ),
    0x00407060: (
        (0x00407060, 0x00407071),
        "??1thunk2_RadioButton@@QAE@H@Z",
        "??_ERadioButton@@$4PPPPPLLI@EEE@AEPAXI@Z",
    ),
    0x00407080: (
        (0x00407080, 0x0040708E),
        "??1thunk3_RadioButton@@QAE@H@Z",
        "??_ERadioButton@@$4PPPPPPPM@KBI@AEPAXI@Z",
    ),
}


def _inline_destructor() -> str:
    source = HEADER.read_text(encoding="utf-8")
    match = re.search(r"(?m)^  virtual ~RadioButton\(\)\s*\{", source)
    assert match, "expected an active class-scope RadioButton destructor definition"
    start = match.start()
    signature = "virtual ~RadioButton()"
    assert source.startswith(signature, start + 2)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start : position + 1].lstrip()
    raise AssertionError("unclosed inline RadioButton destructor")


def test_actual_radio_destructor_is_inline_and_only_closes_once():
    destructor = _inline_destructor()
    assert destructor == "virtual ~RadioButton() { close(); }"

    cpp = (ROOT / "src/jackal/radiobutton.cpp").read_text(encoding="utf-8")
    assert "RadioButton::~RadioButton()" not in cpp


def test_all_task_identities_and_destructor_phase_spans_remain_native_source_mappings():
    task = json.loads(TASK.read_text(encoding="utf-8"))
    expected = {subject["address"] for subject in task["subjects"]}
    assert len(expected) == len(task["subjects"])

    locations = (
        "src/jackal/radiobutton.h",
        "src/jackal/radiobutton.cpp",
        "src/dialogs.cpp",
    )
    found = {}
    for relative in locations:
        for row in read_file(ROOT / relative):
            if row.address in expected:
                found.setdefault(row.address, []).append((relative, row))

    assert set(found) == expected
    assert all(len(rows) == 1 for rows in found.values())
    dialog_path = ROOT / "src/dialogs.cpp"
    dialog_text = dialog_path.read_text(encoding="utf-8")
    dialog_rows = read_file(dialog_path)
    bindings = parse_bindings(dialog_text, dialog_rows)
    for address, (span, catalogued_name, emitted_symbol) in PHASES.items():
        assert address in expected
        relative, row = found[address][0]
        assert relative == "src/dialogs.cpp"
        assert row.mode is Mode.FILE
        assert row.image_spans == (span,)
        assert row.name == catalogued_name
        assert row.symbol == emitted_symbol
        assert row.byte_exact is True
        assert row.source_role is SourceRole.UNCLASSIFIED
        assert row.line in bindings
        assert bindings[row.line]["reason"]
        assert bindings[row.line]["evidence"]


def test_removing_one_historical_byte_claim_is_detected():
    path = ROOT / "src/dialogs.cpp"
    source = path.read_text(encoding="utf-8")
    marker = "// ORIGINAL: 0x00406A90 sub_406a90 0x00406A90-0x00406AE3 FILE BYTE_EXACT"
    assert source.count(marker) == 1
    mutated = source.replace(marker, marker.removesuffix(" BYTE_EXACT"), 1)
    records = read_text(mutated, path)
    row, = [record for record in records if record.address == 0x00406A90]
    assert not row.byte_exact
    with pytest.raises(AssertionError, match="historical BYTE_EXACT obligation"):
        _assert_historical_claim(row)


def _assert_historical_claim(row):
    assert row.byte_exact, "historical BYTE_EXACT obligation was removed"


@pytest.fixture(params=COMPILERS or [None],
                ids=[Path(path).name for path in COMPILERS] or ["no-host-compiler"])
def host_compiler(request):
    if request.param is None:
        pytest.skip("no GCC or Clang host compiler available")
    return request.param


def test_extracted_inline_body_precedes_virtual_base_destruction(host_compiler, tmp_path):
    body = _inline_destructor().split("{", 1)[1].rsplit("}", 1)[0]
    source = r"""#include <vector>
static std::vector<int> events;
struct GraphicWin { virtual ~GraphicWin() { events.push_back(2); } };
struct Dialog { virtual ~Dialog() { events.push_back(1); } };
struct RadioButton : public virtual GraphicWin, public virtual Dialog {
  void close() { events.push_back(0); }
  virtual ~RadioButton() {""" + body + r"""}
};
int main() {
  { RadioButton radio; }
  return events == std::vector<int>{0, 1, 2} ? 0 : 1;
}
"""
    def compile_and_run(name, text):
        source_path = tmp_path / (name + ".cpp")
        executable = tmp_path / name
        source_path.write_text(text, encoding="utf-8")
        built = subprocess.run(
            [host_compiler, "-std=c++11", str(source_path), "-o", str(executable)],
            check=True,
            capture_output=True,
            text=True,
        )
        return subprocess.run([str(executable)], capture_output=True, text=True)

    good = compile_and_run("radiobutton_lifetime", source)
    assert good.returncode == 0, good.stdout + good.stderr
    omitted_close = source.replace("virtual ~RadioButton() {" + body + "}",
                                   "virtual ~RadioButton() {}", 1)
    assert omitted_close != source
    mutant = compile_and_run("radiobutton_lifetime_no_close", omitted_close)
    assert mutant.returncode != 0
