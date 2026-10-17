"""Actual-source scenario undo behavior, original anchors and damage controls.

Host doubles check call/path/state effects, not VC6 ABI or game runtime acceptance.
"""
from __future__ import annotations

import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/general.cpp").is_file())
SOURCE = ROOT / "src/scenario_undo.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
PREFIX = "saves\\auto\\Scenario Editor Undo "
TOKEN = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]', re.S)


def definition(text: str, signature: str) -> str:
    hits = [match for match in re.finditer(re.escape(signature), text)]
    assert len(hits) == 1, (signature, len(hits))
    start = hits[0].start()
    opened = False
    depth = 0
    for token in TOKEN.finditer(text, hits[0].end()):
        if token.group() == "{":
            opened = True
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if opened and depth == 0:
                return text[start:token.end()]
    raise AssertionError("unterminated actual body: " + signature)


SIGNATURES = {
    "load": "void __cdecl load_undo(int type)",
    "wipe": "void __cdecl wipe_undo()",
    "auto": "void __cdecl auto_undo()",
}


def bodies(source_text: str, mutant: str | None = None) -> str:
    extracted = {name: definition(source_text, signature)
                 for name, signature in SIGNATURES.items()}
    if mutant == "load_any_negative_only_minus_one":
        old = "if (type < 0 && static_cast<int>(ScenEditorUndoPosition) > 1)"
        assert extracted["load"].count(old) == 1
        extracted["load"] = extracted["load"].replace(
            old, "if (type == -1 && static_cast<int>(ScenEditorUndoPosition) > 1)", 1)
    elif mutant == "load_unsigned_lower_bound":
        old = "static_cast<int>(ScenEditorUndoPosition) > 1"
        assert extracted["load"].count(old) == 1
        extracted["load"] = extracted["load"].replace(old, "ScenEditorUndoPosition > 1", 1)
    elif mutant == "load_unsigned_upper_bound":
        old = "static_cast<int>(ScenEditorUndoPosition) < 9"
        assert extracted["load"].count(old) == 1
        extracted["load"] = extracted["load"].replace(old, "ScenEditorUndoPosition < 9", 1)
    elif mutant == "load_add_save_suffix":
        old = '"saves\\\\auto\\\\Scenario Editor Undo "'
        assert extracted["load"].count(old) == 1
        extracted["load"] = extracted["load"].replace(
            old, '"saves\\\\auto\\\\Scenario Editor Undo .SAV"', 1)
    elif mutant == "auto_reverse_rename":
        old = "rename(previous, current);"
        assert extracted["auto"].count(old) == 1
        extracted["auto"] = extracted["auto"].replace(old, "rename(current, previous);", 1)
    elif mutant == "auto_save_current":
        old = "save_daemon(previous);"
        assert extracted["auto"].count(old) == 1
        extracted["auto"] = extracted["auto"].replace(old, "save_daemon(current);", 1)
    elif mutant == "auto_invert_pref_guard":
        old = "if (!(GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_AUTOSAVE_EACH_TURN))"
        assert extracted["auto"].count(old) == 1
        extracted["auto"] = extracted["auto"].replace(
            old, "if (GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_AUTOSAVE_EACH_TURN)", 1)
    elif mutant is not None:
        raise AssertionError("unknown mutation: " + mutant)
    return "\n".join(extracted.values())


HOST = r"""
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#define __cdecl
using uint32_t = unsigned int;
static const uint32_t PREF_BSC_AUTOSAVE_EACH_TURN = 2u;
static const char *const undo_prefix = "saves\\auto\\Scenario Editor Undo ";
struct GameState { uint32_t game_preferences_; };
struct GameStorage { GameState fields_; };
GameStorage GameDataStateStorage = {};
uint32_t ScenEditorUndoPosition = 1;
char StringTemp[1032] = {};
static std::vector<std::string> events;
static char *current_buffer = nullptr;
static char *previous_buffer = nullptr;
static char *number_buffer = nullptr;

static std::string text(const char *value) { return value ? std::string(value) : "<null>"; }
static std::string buffer_name(const char *buffer) {
    if (buffer == StringTemp) return "global";
    if (buffer == current_buffer) return "current";
    if (buffer == previous_buffer) return "previous";
    if (buffer == number_buffer) return "number";
    return "other";
}
static void reset_fixture() {
    events.clear(); current_buffer = nullptr; previous_buffer = nullptr; number_buffer = nullptr;
    std::memset(StringTemp, 0, sizeof StringTemp);
}
static char *host_itoa(uint32_t bits, char *buffer, int radix) {
    assert(radix == 10);
    number_buffer = buffer;
    int32_t signed_value;
    std::memcpy(&signed_value, &bits, sizeof signed_value);
    std::snprintf(buffer, 80, "%d", static_cast<int>(signed_value));
    events.push_back("itoa:" + text(buffer) + ":" + std::to_string(radix));
    return buffer;
}
static char *host_strcat(char *destination, const char *source) {
    if (destination != StringTemp && destination != current_buffer && destination != previous_buffer)
        current_buffer = destination;
    events.push_back("strcat:" + buffer_name(destination) + ":" + text(source));
    return std::strcat(destination, source);
}
static char *host_strcpy(char *destination, const char *source) {
    if (destination != StringTemp && destination != current_buffer && destination != previous_buffer)
        previous_buffer = destination;
    events.push_back("strcpy:" + buffer_name(destination) + ":" + text(source));
    return std::strcpy(destination, source);
}
static size_t host_strlen(const char *value) {
    if (value != StringTemp && value != current_buffer && value != previous_buffer && value != number_buffer)
        previous_buffer = const_cast<char *>(value);
    const size_t length = std::strlen(value);
    events.push_back("strlen:" + buffer_name(value) + ":" + std::to_string(length));
    return length;
}
static int host_remove(const char *path) {
    events.push_back("remove:" + text(path));
    return 0;
}
static int host_rename(const char *old_path, const char *new_path) {
    events.push_back("rename:" + buffer_name(old_path) + ":" + text(old_path) + ":" +
                     buffer_name(new_path) + ":" + text(new_path));
    return 0;
}
static int load_daemon(const char *path, bool linearize_contours) {
    events.push_back("load:" + buffer_name(path) + ":" + text(path) + ":" +
                     (linearize_contours ? "true" : "false"));
    return 0;
}
static int save_daemon(char *path) {
    events.push_back("save:" + buffer_name(path) + ":" + text(path));
    return 0;
}
#define strcat host_strcat
#define strcpy host_strcpy
#define strlen host_strlen
#define remove host_remove
#define rename host_rename
#define _itoa host_itoa

@@BODIES@@

static void expect_events(const std::vector<std::string> &expected) {
    if (events != expected) {
        std::fprintf(stderr, "event count %zu expected %zu\n", events.size(), expected.size());
        for (const auto &event : events) std::fprintf(stderr, "  %s\n", event.c_str());
        std::abort();
    }
}
static std::vector<std::string> load_events(uint32_t value, const std::string &path) {
    return {"strcat:global:" + std::string(undo_prefix),
            "itoa:" + std::to_string(static_cast<int32_t>(value)) + ":10",
            "strcat:global:" + std::to_string(static_cast<int32_t>(value)),
            "load:global:" + path + ":false"};
}
static std::vector<std::string> auto_events() {
    std::vector<std::string> expected;
    std::string previous;
    for (int i = 9; i >= 1; --i) {
        const std::string current = std::string(undo_prefix) + std::to_string(i);
        expected.push_back("strcat:current:" + std::string(undo_prefix));
        expected.push_back("itoa:" + std::to_string(i) + ":10");
        expected.push_back("strcat:current:" + std::to_string(i));
        expected.push_back("strlen:previous:" + std::to_string(previous.size()));
        if (previous.empty()) {
            expected.push_back("remove:" + current);
        } else {
            expected.push_back("rename:previous:" + previous + ":current:" + current);
        }
        expected.push_back("strcpy:previous:" + current);
        previous = current;
    }
    expected.push_back("save:previous:" + previous);
    return expected;
}
static std::vector<std::string> wipe_events() {
    std::vector<std::string> expected;
    for (int i = 9; i >= 1; --i) {
        expected.push_back("strcat:global:" + std::string(undo_prefix));
        expected.push_back("itoa:" + std::to_string(i) + ":10");
        expected.push_back("strcat:global:" + std::to_string(i));
        expected.push_back("remove:" + std::string(undo_prefix) + std::to_string(i));
    }
    return expected;
}

int main() {
    reset_fixture();
    ScenEditorUndoPosition = 8;
    load_undo(1);
    expect_events(load_events(8, std::string(undo_prefix) + "8"));
    assert(ScenEditorUndoPosition == 9);

    reset_fixture();
    ScenEditorUndoPosition = 2;
    load_undo(-2);
    expect_events(load_events(1, std::string(undo_prefix) + "1"));
    assert(ScenEditorUndoPosition == 1);

    reset_fixture();
    ScenEditorUndoPosition = 1;
    load_undo(-1);
    expect_events(load_events(1, std::string(undo_prefix) + "1"));
    assert(ScenEditorUndoPosition == 1);

    reset_fixture();
    ScenEditorUndoPosition = 9;
    load_undo(1);
    expect_events(load_events(9, std::string(undo_prefix) + "9"));
    assert(ScenEditorUndoPosition == 9);

    // The original CMP/JLE and CMP/JGE are signed. Preserve x86 DWORD bits
    // when the host _itoa stub interprets the call's int argument.
    reset_fixture();
    ScenEditorUndoPosition = 0x80000000u;
    load_undo(-1);
    expect_events(load_events(0x80000000u, std::string(undo_prefix) + "-2147483648"));
    assert(ScenEditorUndoPosition == 0x80000000u);

    reset_fixture();
    ScenEditorUndoPosition = 0x80000000u;
    load_undo(1);
    expect_events(load_events(0x80000000u, std::string(undo_prefix) + "-2147483648"));
    assert(ScenEditorUndoPosition == 0x80000001u);

    reset_fixture();
    wipe_undo();
    expect_events(wipe_events());
    assert(ScenEditorUndoPosition == 0x80000001u);

    reset_fixture();
    GameDataStateStorage.fields_.game_preferences_ = 0;
    ScenEditorUndoPosition = 77;
    auto_undo();
    expect_events({});
    assert(ScenEditorUndoPosition == 77);

    reset_fixture();
    GameDataStateStorage.fields_.game_preferences_ = PREF_BSC_AUTOSAVE_EACH_TURN;
    ScenEditorUndoPosition = 0xdeadbeefu;
    auto_undo();
    expect_events(auto_events());
    assert(ScenEditorUndoPosition == 1);
    return 0;
}
"""


def harness(source_text: str, mutant: str | None = None) -> str:
    return HOST.replace("@@BODIES@@", bodies(source_text, mutant))


def pinned_evidence() -> None:
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source.asm import Listing

    image = Path(os.environ.get("OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not image.is_file():
        pytest.skip("pinned image unavailable; host controls do not replace original anchors")
    pe = PeImage(image.read_bytes(), PIN)

    def listing(start: int, end: int) -> list[str]:
        return Listing(pe.read(Span(start, end)), start).lines

    load = listing(0x005ABE40, 0x005ABEBF)
    assert any("jle 0x5abe5e" in row for row in load)
    assert any("jge 0x5abeac" in row for row in load)
    assert any("call 0x64fc88" in row for row in load)  # _itoa(value, number, 10)
    assert any("call 0x5a9760" in row for row in load)  # load_daemon(StringTemp, false)
    assert not any(".SAV" in row or "sprintf" in row for row in load)

    wipe = listing(0x005ABEC0, 0x005ABF14)
    assert any("call 0x646950" in row for row in wipe)  # remove(path)
    assert any("cmp esi, 1" in row for row in wipe)
    assert any("jge 0x5abecc" in row for row in wipe)

    auto = listing(0x005ABF20, 0x005ABFEF)
    assert any("test byte ptr [0x9a6490], 2" in row for row in auto)
    assert any("call 0x6469f7" in row for row in auto)  # rename(previous, current)
    assert any("call 0x646950" in row for row in auto)  # remove(current), first pass
    assert any("push ecx" in row and row.startswith("0x005ABFE1") for row in auto)
    assert any("call 0x5a94f0" in row for row in auto)  # save_daemon(previous)

    # Verify that the explicit source path still carries these exact three
    # original spans; no unrelated caller/ABI claim is made by this host test.
    general = SOURCE.read_text(encoding="utf-8")
    for address, end, symbol in (
        ("0x005ABE40", "0x005ABEBF", "?load_undo@@YAXH@Z"),
        ("0x005ABEC0", "0x005ABF14", "?wipe_undo@@YAXXZ"),
        ("0x005ABF20", "0x005ABFEF", "?auto_undo@@YAXXZ"),
    ):
        assert f"// ORIGINAL: {address} {symbol} {address}-{end} BYTE_EXACT" in general


def compile_and_run(compiler: str, source_text: str, mutant: str | None) -> bool:
    generated = harness(source_text, mutant)
    with tempfile.TemporaryDirectory(prefix="osmx-scenario-undo-host-") as directory:
        cpp = Path(directory) / "scenario_undo.cpp"
        executable = Path(directory) / "scenario_undo"
        cpp.write_text(generated, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
             str(cpp), "-o", str(executable)],
            capture_output=True, text=True, timeout=60)
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
        return result.returncode == 0


def test_pinned_original_undo_anchors():
    pinned_evidence()


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutant", (
    None,
    "load_any_negative_only_minus_one",
    "load_unsigned_lower_bound",
    "load_unsigned_upper_bound",
    "load_add_save_suffix",
    "auto_reverse_rename",
    "auto_save_current",
    "auto_invert_pref_guard",
))
def test_actual_undo_effects_and_compiling_mutations(compiler_name, mutant):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable: " + compiler_name)
    source_text = SOURCE.read_text(encoding="utf-8")
    assert "BODY GOES HERE" not in bodies(source_text)
    passed = compile_and_run(compiler, source_text, mutant)
    assert passed == (mutant is None), (compiler_name, mutant, passed)
