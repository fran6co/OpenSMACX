"""Compile and exercise the actual StatusWin::UNK3 definition.

The host harness checks source behavior and mutation sensitivity. The pinned
instruction check separately confirms that the original keeps the selected
unit in ESI across the wake call.
"""
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source.asm import Listing


HERE = Path(__file__).resolve().parent
PINNED_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def project_root():
    override = os.environ.get("OPENSMACX_SOURCE_ROOT")
    return Path(override).resolve() if override else HERE.parents[1]


def extract_unk3(source):
    signature = "void StatusWin::UNK3()"
    start = source.find(signature)
    assert start >= 0
    assert source.find(signature, start + len(signature)) < 0
    opening = source.find("{", start + len(signature))
    assert opening >= 0
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated StatusWin::UNK3 definition")


HARNESS = r"""
#include <assert.h>

enum { WAKE = 1, MOVE = 2, ZOOM = 3 };
struct StatusWin {
    int iUnitIndex_;
    int iCityIndex_;
    void UNK3();
};
static StatusWin *active_status;
static int events[4];
static int event_values[4];
static int event_count;
static int zoom_args[2] = {-1, -1};
int veh_wake(int unit_id) {
    events[event_count] = WAKE;
    event_values[event_count++] = unit_id;
    active_status->iUnitIndex_ = 99;
    return 0;
}
struct Console {
    int selected_vehicle_;
    int move_calls;
    int move_arg;
    Console() : selected_vehicle_(-7), move_calls(0), move_arg(-1) {}
    void set_move(int value) {
        events[event_count] = MOVE;
        event_values[event_count++] = value;
        ++move_calls;
        move_arg = value;
    }
} ConsoleGlobal;
struct TutorialWindow {
    int calls;
    void zoom(int city_id, int mode) {
        events[event_count] = ZOOM;
        event_values[event_count++] = city_id;
        ++calls;
        zoom_args[0] = city_id;
        zoom_args[1] = mode;
    }
} TutWinBaseWindow;

/*__ACTUAL_UNK3__*/

static void reset_events() {
    event_count = 0;
    TutWinBaseWindow.calls = 0;
    ConsoleGlobal.move_calls = 0;
    ConsoleGlobal.move_arg = -1;
}

int main() {
    StatusWin status;
    active_status = &status;

    status.iUnitIndex_ = 7;
    status.iCityIndex_ = 12;
    reset_events();
    status.UNK3();
    assert(ConsoleGlobal.selected_vehicle_ == 7);
    assert(status.iUnitIndex_ == 99);
    assert(event_count == 2 && events[0] == WAKE && event_values[0] == 7);
    assert(events[1] == MOVE && event_values[1] == 1);
    assert(ConsoleGlobal.move_calls == 1 && ConsoleGlobal.move_arg == 1);
    assert(TutWinBaseWindow.calls == 0);

    status.iUnitIndex_ = -1;
    status.iCityIndex_ = 6;
    ConsoleGlobal.selected_vehicle_ = -7;
    reset_events();
    status.UNK3();
    assert(event_count == 1 && events[0] == ZOOM && event_values[0] == 6);
    assert(zoom_args[0] == 6 && zoom_args[1] == 0);
    assert(TutWinBaseWindow.calls == 1);
    assert(ConsoleGlobal.selected_vehicle_ == -7);
    assert(ConsoleGlobal.move_calls == 0);

    status.iUnitIndex_ = -2;
    status.iCityIndex_ = -1;
    reset_events();
    status.UNK3();
    assert(event_count == 0 && TutWinBaseWindow.calls == 0);
    assert(ConsoleGlobal.move_calls == 0);
    assert(ConsoleGlobal.selected_vehicle_ == -7);
    return 0;
}
"""


def compile_and_run(compiler, tmp_path, body, label):
    source = tmp_path / (label + ".cpp")
    executable = tmp_path / label
    assert HARNESS.count("/*__ACTUAL_UNK3__*/") == 1
    source.write_text(HARNESS.replace("/*__ACTUAL_UNK3__*/", body), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=60, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=30, check=False)


def test_actual_statuswin_selection_preserves_original_snapshot_and_branches(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler is unavailable: g++")
    path = project_root() / "src" / "statuswin.cpp"
    source = path.read_text(encoding="utf-8")
    body = extract_unk3(source)
    ran = compile_and_run(compiler, tmp_path, body, "actual-unk3")
    assert ran.returncode == 0, ran.stdout + ran.stderr


def test_reload_and_wrong_target_mutants_are_rejected_by_compiled_control(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler is unavailable: g++")
    source = (project_root() / "src" / "statuswin.cpp").read_text(encoding="utf-8")
    body = extract_unk3(source)
    reload_mutant = body.replace(
        "ConsoleGlobal.selected_vehicle_ = unit_id;",
        "ConsoleGlobal.selected_vehicle_ = iUnitIndex_;",
        1,
    )
    wrong_target_mutant = body.replace(
        "ConsoleGlobal.selected_vehicle_ = unit_id;",
        "ConsoleGlobal.selected_vehicle_ = iCityIndex_;",
        1,
    )
    assert reload_mutant != body and wrong_target_mutant != body
    for label, mutant in (("reload-mutant", reload_mutant),
                          ("wrong-target-mutant", wrong_target_mutant)):
        ran = compile_and_run(compiler, tmp_path, mutant, label)
        assert ran.returncode != 0, label + " unexpectedly passed the host controls"


def test_pinned_unit_branch_saves_index_in_esi_before_calls():
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE",
        project_root() / ".opensmacx" / "game" / "terranx_original.exe",
    ))
    if not image_path.is_file():
        pytest.skip("pinned original image is unavailable")
    image = PeImage(image_path.read_bytes(), PINNED_SHA256)
    code = image.read(Span(0x004B8DF0, 0x004B8E31))
    lines = Listing(code, 0x004B8DF0).lines
    assert lines[:13] == [
        "0x004B8DF0  56                        push esi",
        "0x004B8DF1  8B B1 BC 15 00 00         mov esi, dword ptr [ecx + 0x15bc]",
        "0x004B8DF7  85 F6                     test esi, esi",
        "0x004B8DF9  7C 1D                     jl 0x4b8e18",
        "0x004B8DFB  56                        push esi",
        "0x004B8DFC  E8 6F 8F 10 00            call 0x5c1d70",
        "0x004B8E01  83 C4 04                  add esp, 4",
        "0x004B8E04  B9 B0 56 91 00            mov ecx, 0x9156b0",
        "0x004B8E09  89 35 8C 92 93 00         mov dword ptr [0x93928c], esi",
        "0x004B8E0F  6A 01                     push 1",
        "0x004B8E11  E8 0A 87 05 00            call 0x511520",
        "0x004B8E16  5E                        pop esi",
        "0x004B8E17  C3                        ret",
    ]
