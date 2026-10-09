"""Compile bounded controls around the actual typed 0x447D20 Midi helper."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.source import Mode, read_file
from match_recovery.binding_source import parse as parse_bindings


ROOT = Path(__file__).resolve().parents[2]
MIDI_CPP = ROOT / "src/midi.cpp"
MIDI_H = ROOT / "src/midi.h"
STRINGSTRUCT_CPP = ROOT / "src/stringstruct.cpp"
ADDRESS = 0x00447D20


def extract_definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[start : offset + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def source_parts() -> tuple[str, str]:
    text = MIDI_CPP.read_text(encoding="utf-8")
    declaration = "int MidiTracksetTransposition = -0x4B0;"
    assert text.count(declaration) == 1
    helper = extract_definition(
        text, "void __fastcall advance_midi_trackset_transposition(Midi *midi)"
    )
    return declaration, helper


HARNESS = r'''
#include <cassert>
#include <string>
#include <vector>
#define __fastcall

struct Midi {
    virtual bool is_trackset_playing(unsigned int trackset) = 0;
    virtual int xpose_trackset(unsigned int trackset, int amount) = 0;
    virtual int play_trackset(unsigned int trackset) = 0;
};

struct MidiSpy : Midi {
    std::vector<bool> answers;
    std::vector<std::string> events;
    unsigned int query = 0;
    int xpose_mutation = 0;
    int status_mutation_at = -1;
    int status_mutation_value = 0;

    bool is_trackset_playing(unsigned int trackset) override {
        events.push_back("S" + std::to_string(trackset));
        if (static_cast<int>(query) == status_mutation_at)
            MidiTracksetTransposition = status_mutation_value;
        return answers.at(query++);
    }

    int xpose_trackset(unsigned int trackset, int amount) override {
        events.push_back("X" + std::to_string(trackset) + ":" +
                         std::to_string(amount));
        if (xpose_mutation != 0)
            MidiTracksetTransposition = xpose_mutation;
        return -77; // The original caller ignores this provider result.
    }

    int play_trackset(unsigned int trackset) override {
        events.push_back("P" + std::to_string(trackset) + ":" +
                         std::to_string(MidiTracksetTransposition));
        return -88; // The original caller ignores this provider result.
    }
};

static bool same(const std::vector<std::string>& got,
                 std::initializer_list<const char*> want) {
    if (got.size() != want.size()) return false;
    unsigned int index = 0;
    for (const char* item : want) {
        if (got[index++] != item) return false;
    }
    return true;
}

int main() {
    // Both stopped: zero resets to -1200, then +300; transpose precedes play.
    MidiTracksetTransposition = 0;
    MidiSpy first;
    first.answers = {false, false};
    first.xpose_mutation = 71;
    advance_midi_trackset_transposition(&first);
    assert(same(first.events, {"S1", "S2", "X1:-900", "P1:71"}));
    assert(MidiTracksetTransposition == 71);

    // Nonzero positive and negative values are stepped as-is: reset applies
    // only to zero, not as a normalization of an existing transposition.
    MidiTracksetTransposition = 600;
    MidiSpy positive;
    positive.answers = {false, false};
    advance_midi_trackset_transposition(&positive);
    assert(same(positive.events, {"S1", "S2", "X1:900", "P1:900"}));
    assert(MidiTracksetTransposition == 900);

    MidiTracksetTransposition = -1200;
    MidiSpy negative;
    negative.answers = {false, false};
    advance_midi_trackset_transposition(&negative);
    assert(same(negative.events, {"S1", "S2", "X1:-900", "P1:-900"}));
    assert(MidiTracksetTransposition == -900);

    // Status(2)'s virtual callback changes the shared word before the helper
    // reads it for the increment; a cached/pre-callback read would yield 400.
    MidiTracksetTransposition = 100;
    MidiSpy callback_mutation;
    callback_mutation.answers = {false, false};
    callback_mutation.status_mutation_at = 1;
    callback_mutation.status_mutation_value = 600;
    advance_midi_trackset_transposition(&callback_mutation);
    assert(same(callback_mutation.events, {"S1", "S2", "X1:900", "P1:900"}));
    assert(MidiTracksetTransposition == 900);

    // A false second query makes the second set eligible. Both checks of set
    // 2 are observable; the last status callback mutates the shared word before
    // xpose reads it, and xpose mutates it again before play observes it.
    MidiTracksetTransposition = 31;
    MidiSpy changing;
    changing.answers = {false, true, false};
    changing.status_mutation_at = 2;
    changing.status_mutation_value = 77;
    changing.xpose_mutation = 88;
    advance_midi_trackset_transposition(&changing);
    assert(same(changing.events, {"S1", "S2", "S2", "X2:77", "P2:88"}));
    assert(MidiTracksetTransposition == 88);

    // Set 1 already running: query set 2 once, then use the existing word.
    MidiTracksetTransposition = -450;
    MidiSpy second;
    second.answers = {true, false};
    advance_midi_trackset_transposition(&second);
    assert(same(second.events, {"S1", "S2", "X2:-450", "P2:-450"}));

    // Set 1 stopped and set 2 still running: status(2) is queried twice and
    // no provider mutation happens.
    MidiTracksetTransposition = 123;
    MidiSpy no_start;
    no_start.answers = {false, true, true};
    advance_midi_trackset_transposition(&no_start);
    assert(same(no_start.events, {"S1", "S2", "S2"}));
    assert(MidiTracksetTransposition == 123);
    return 0;
}
'''


def compile_and_run(tmp_path: Path, body: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("c++") or shutil.which("clang++") or shutil.which("g++")
    assert compiler, "a host C++ compiler is required for actual-source controls"
    declaration, _ = source_parts()
    unit = tmp_path / "midi_trackset_actual.cpp"
    binary = tmp_path / "midi_trackset_actual"
    unit.write_text(
        HARNESS.replace("struct Midi {", declaration + "\nstruct Midi {")
        .replace("int main() {", body + "\nint main() {"),
        encoding="utf-8",
    )
    compiled = subprocess.run(
        [compiler, "-std=c++17", "-O0", str(unit), "-o", str(binary)],
        text=True,
        capture_output=True,
        check=False,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True, check=False)


def test_current_source_keeps_identity_and_removes_the_raw_dispatch_hypothesis():
    all_rows = read_file(MIDI_CPP)
    rows = [row for row in all_rows if row.address == ADDRESS]
    assert len(rows) == 1
    assert rows[0].mode is Mode.FILE
    assert rows[0].name == "sub_447d20"
    assert rows[0].image_spans == ((ADDRESS, 0x00447DA5),)
    assert not rows[0].byte_exact and not rows[0].semantic
    assert not rows[0].levers and not rows[0].ruled_out
    binding = parse_bindings(MIDI_CPP.read_text(), all_rows)[rows[0].line]
    assert any("Midi::is_trackset_playing" in item for item in binding["evidence"])
    assert any("full original allocation" in item for item in binding["evidence"])
    assert "Midi::is_trackset_playing" in rows[0].region
    assert "Midi::xpose_trackset" in rows[0].region
    assert "Midi::play_trackset" in rows[0].region
    assert "advance_midi_trackset_transposition(Midi *midi)" in MIDI_H.read_text()

    old_addresses = [row for row in read_file(STRINGSTRUCT_CPP) if row.address == ADDRESS]
    assert old_addresses == []
    old_source = STRINGSTRUCT_CPP.read_text()
    for obsolete in (
        "CascadeDelaySlot", "IntSlotShim", "IntIntSlotShim", "NullSlotShim",
        "ReturnSlotShim", "static int *const CascadeDelaySlot",
    ):
        assert obsolete not in old_source
    assert "Superseded RECOVERY_BINDING history retained verbatim" in old_source
    assert "settings-checkbox/repaint seed" in old_source


def test_actual_typed_helper_preserves_trackset_order_and_mutable_callbacks(tmp_path: Path):
    _declaration, helper = source_parts()
    result = compile_and_run(tmp_path, helper)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize(
    ("label", "mutate"),
    [
        ("wrong status id", lambda s: s.replace("playing(1)", "playing(2)", 1)),
        ("missing first play", lambda s: s.replace("            midi->play_trackset(1);\n", "", 1)),
        (
            "skipped repeated status query",
            lambda s: s.replace(
                "    if (!midi->is_trackset_playing(2)) {\n        midi->xpose_trackset(2,",
                "    if (false) {\n        midi->xpose_trackset(2,",
                1,
            ),
        ),
        (
            "wrong reset value",
            lambda s: s.replace("MidiTracksetTransposition = -0x4B0;\n            }",
                                "MidiTracksetTransposition = -0x4B1;\n            }", 1),
        ),
        (
            "missing transposition store",
            lambda s: s.replace(
                "MidiTracksetTransposition = MidiTracksetTransposition + 0x12C;",
                "int ignored_transposition = MidiTracksetTransposition + 0x12C;",
                1,
            ),
        ),
        (
            "play before transpose",
            lambda s: s.replace(
                "            midi->xpose_trackset(1, MidiTracksetTransposition);\n"
                "            midi->play_trackset(1);",
                "            midi->play_trackset(1);\n"
                "            midi->xpose_trackset(1, MidiTracksetTransposition);",
                1,
            ),
        ),
    ],
)
def test_actual_source_controls_reject_individual_mutants(tmp_path: Path, label, mutate):
    _declaration, helper = source_parts()
    mutant = mutate(helper)
    assert mutant != helper, f"mutant did not alter helper: {label}"
    result = compile_and_run(tmp_path, mutant)
    assert result.returncode != 0, f"behavioral control accepted mutant: {label}"
