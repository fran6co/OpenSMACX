"""Actual-source controls for the ReportWin/InfoWin recovery batch."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.pe_coff import PeImage, Span
from match_recovery.source.asm import Listing


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/infowin.cpp").is_file())
SOURCE = ROOT / "src/infowin.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SIGNATURE = "void InfoWin::timer_change(int)"


def _masked(source):
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )


def _method(source):
    masked = _masked(source)
    assert masked.count(SIGNATURE) == 1
    start = masked.index(SIGNATURE)
    opening = masked.index("{", start + len(SIGNATURE))
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated InfoWin::timer_change definition")


def _fixture(method):
    return r'''#include <stddef.h>

struct RECT { int value; };
struct BoxSpriteParams { int value; };
struct EventLog { char values[16]; int count; void add(char value) { values[count++] = value; } };
struct InfoWin;
static InfoWin *observed_info;
static EventLog events;
static int WinScreenWidth = 1024;
static int InterfaceBoxStyles[19];

bool active_interface_slot_occupied() { return false; }

struct MainInterface {
    void reset_text_state() { events.add('R'); }
    void box_sprite(RECT *, int *);
};

struct InfoWin {
    int active_pane_;
    int field_94_;
    int pane_states_[2];
    int redraw_phase_;
    int visible_characters_[2];
    int field_A8_;
    RECT pane_rects_[2];
    void timer_change(int);
    bool state_valid(int) { return true; }
    void setup_text() {
        events.add('S');
        if (visible_characters_[0] != 0 || visible_characters_[1] != 0)
            events.add('!');
    }
    void draw_text(int pane) { events.add(pane == 0 ? '0' : '1'); }
};

static MainInterface MainInterfaceGlobal;
void MainInterface::box_sprite(RECT *, int *) {
    if (observed_info->visible_characters_[0] != 0 ||
        observed_info->visible_characters_[1] != 0)
        events.add('!');
    events.add('B');
    observed_info->visible_characters_[0] = 13;
    observed_info->visible_characters_[1] = 17;
}

''' + method + r'''

int main() {
    InfoWin info = {1, 1, {0, 0}, -1, {4, 5}, 1, {{0}, {0}}};
    observed_info = &info;
    info.timer_change(37);
    if (info.visible_characters_[0] != 0 || info.visible_characters_[1] != 0)
        return 1;
    if (events.count != 4) return 2;
    if (events.values[0] != 'R' || events.values[1] != 'B' ||
        events.values[2] != 'S' || events.values[3] != '0') return 3;
    for (int index = 0; index < events.count; ++index)
        if (events.values[index] == '!') return 4;
    if (info.field_A8_ != 0 || info.field_94_ != 0 || info.pane_states_[0] != 1)
        return 5;
    return 0;
}
'''


def _compile(method, compiler, output):
    cpp = output.with_suffix(".cpp")
    cpp.write_text(_fixture(method), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
         str(cpp), "-o", str(output)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr


def test_pinned_timer_change_store_call_store_order():
    explicit = os.environ.get("OPENSMACX_IMAGE")
    image = Path(explicit) if explicit else ROOT / ".opensmacx/game/terranx_original.exe"
    if not image.is_file():
        pytest.skip("pinned original not available")
    pe = PeImage(image.read_bytes(), PIN)

    def at(address, length):
        return Listing(pe.read(Span(address, address + length)), address).instructions

    assert [(item.mnemonic, item.op_str) for item in at(0x4590F5, 6)] == [
        ("mov", "dword ptr [esi + 0x9c8], ebx")]
    assert [(item.mnemonic, item.op_str) for item in at(0x4590FC, 6)] == [
        ("mov", "dword ptr [esi + 0x9cc], ebx")]
    assert [(item.mnemonic, item.op_str) for item in at(0x459108, 5)] == [
        ("call", "0x5e2dd3")]
    assert [(item.mnemonic, item.op_str) for item in at(0x45910D, 6)] == [
        ("lea", "ecx, [esi + 0x88]")]
    assert [(item.mnemonic, item.op_str) for item in at(0x459114, 4)] == [
        ("test", "ecx, ecx"), ("je", "0x459124")]
    assert [(item.mnemonic, item.op_str) for item in at(0x459118, 6)] == [
        ("mov", "dword ptr [esi + 0x9c8], ebx")]
    assert [(item.mnemonic, item.op_str) for item in at(0x45911E, 6)] == [
        ("mov", "dword ptr [esi + 0x9cc], ebx")]
    assert [(item.mnemonic, item.op_str) for item in at(0x459126, 5)] == [
        ("call", "0x456300")]


def test_actual_timer_change_reclears_visible_counts_after_box_draw(tmp_path):
    source = SOURCE.read_text(encoding="utf-8")
    method = _method(source)
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("GCC or Clang required for actual-source InfoWin control")

    executable = tmp_path / "infowin-timer-change"
    _compile(method, compiler, executable)
    run = subprocess.run([str(executable)], capture_output=True, timeout=10)
    assert run.returncode == 0, run.stdout + run.stderr

    masked = _masked(method)
    post_call_pair = re.compile(
        r"MainInterfaceGlobal\.box_sprite\(\s*&pane_rects_\[active_pane_\]"
        r"\s*,\s*&InterfaceBoxStyles\[18\]\s*\);\s*"
        r"visible_characters_\[0\]\s*=\s*0;\s*"
        r"visible_characters_\[1\]\s*=\s*0;"
    )
    assert len(list(post_call_pair.finditer(masked))) == 1


def test_post_call_store_mutant_is_rejected(tmp_path):
    source = SOURCE.read_text(encoding="utf-8")
    method = _method(source)
    masked = _masked(method)
    post_call_pair = re.compile(
        r"(MainInterfaceGlobal\.box_sprite\(\s*&pane_rects_\[active_pane_\]"
        r"\s*,\s*&InterfaceBoxStyles\[18\]\s*\);)\s*"
        r"visible_characters_\[0\]\s*=\s*0;\s*"
        r"visible_characters_\[1\]\s*=\s*0;"
    )
    matches = list(post_call_pair.finditer(masked))
    assert len(matches) == 1
    match = matches[0]
    # The lexical mask preserves source offsets, so remove the real stores
    # despite the explanatory source comment between call and assignments.
    mutant = method[:match.end(1)] + method[match.end():]

    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("GCC or Clang required for actual-source InfoWin control")
    executable = tmp_path / "infowin-timer-change-mutant"
    _compile(mutant, compiler, executable)
    run = subprocess.run([str(executable)], capture_output=True, timeout=10)
    assert run.returncode == 1, "removing the post-call reset must leave the callback mutation visible"


def test_pre_call_store_mutant_is_rejected(tmp_path):
    source = SOURCE.read_text(encoding="utf-8")
    method = _method(source)
    masked = _masked(method)
    pre_call_pair = re.compile(
        r"visible_characters_\[0\]\s*=\s*0;\s*"
        r"visible_characters_\[1\]\s*=\s*0;\s*field_A8_\s*=\s*0;"
    )
    matches = list(pre_call_pair.finditer(masked))
    assert len(matches) == 1
    match = matches[0]
    pre_pair = re.match(
        r"visible_characters_\[0\]\s*=\s*0;\s*"
        r"visible_characters_\[1\]\s*=\s*0;", masked[match.start():]
    )
    assert pre_pair is not None
    # The lexical mask preserves offsets; remove the source statements despite
    # the intervening comments/formatting in the actual method text.
    mutant = method[:match.start()] + method[match.start() + pre_pair.end():]

    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("GCC or Clang required for actual-source InfoWin control")
    executable = tmp_path / "infowin-timer-change-pre-call-mutant"
    _compile(mutant, compiler, executable)
    run = subprocess.run([str(executable)], capture_output=True, timeout=10)
    assert run.returncode == 2, "removing the pre-call reset must be detected by box_sprite"

