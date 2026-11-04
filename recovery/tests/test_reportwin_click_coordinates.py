"""Original mouse coordinates survive Spot's separate output parameters."""
from pathlib import Path
import shutil
import subprocess

import pytest

from recovery.tests.test_pact_ascent import definition


ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "garrison", "menu"])
def test_actual_report_right_click_coordinates(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    body = definition((ROOT / "src/reportif.cpp").read_text(),
                      r"void\s+ReportWin::on_right_click\s*\(int x, int y\)")
    if damage:
        old, new = ("garrison_click(payload, x, y, 1)",
                    "garrison_click(payload, x, spot_id, 1)") if damage == "garrison" else (
                        "right_menu(first_id, x, y)", "right_menu(first_id, x, spot_id)")
        assert body.count(old) == 1
        body = body.replace(old, new, 1)
    harness = r'''
#include <cassert>
struct Base { int faction_id_current; int specialist_total; } Bases[8];
Base *BaseCurrent = &Bases[0];
int BaseIDCurrentSelected;
struct { int faction_id_; } ConsoleGlobal;
struct { int field_8_; int report_faction_id_; } ReportIfGlobal;
struct { struct { unsigned game_state_; } fields_; } GameDataStateStorage;
bool blocked;
int spot_kind, spot_payload, spot_result;
int menu_calls, menu_id, menu_x, menu_y;
int garrison_calls, garrison_id, garrison_x, garrison_y, garrison_flag;
int zoom_id, satellite_calls, satellite_report, satellite_payload, satellite_kind;
bool reportwin_input_blocked() { return blocked; }
void log_say(const char *, int, int, int) {}
void base_compute(int) {}
void sat_edit(int report, int payload, int kind) {
    ++satellite_calls; satellite_report = report;
    satellite_payload = payload; satellite_kind = kind;
}
struct {
    int lock_base(int, int, int, int) { return 0; }
    void unlock_base(int) {}
} NetDaemonNet;
struct {
    void soft_zoom(int id) { zoom_id = id; }
    void garrison_click(int id, int x, int y, int flag) {
        ++garrison_calls; garrison_id = id;
        garrison_x = x; garrison_y = y; garrison_flag = flag;
    }
} TutWinBaseWindow;
struct Spot {
    int check(int x, int y, int *kind, int *payload) {
        assert(x == 11 && y == 22);
        *kind = spot_kind; *payload = spot_payload;
        return spot_result;
    }
};
struct ReportWin {
    Spot spot_;
    int redraws = 0;
    void redraw() { ++redraws; }
    void right_menu(int id, int x, int y) {
        ++menu_calls; menu_id = id; menu_x = x; menu_y = y;
    }
    void on_right_click(int x, int y);
};
@@BODY@@
int main() {
    ReportWin window;
    ConsoleGlobal.faction_id_ = Bases[2].faction_id_current = 3;
    ReportIfGlobal.field_8_ = 4;
    spot_kind = 2050; spot_payload = 7;
    window.on_right_click(11, 22);
    assert(garrison_calls == 1 && garrison_id == 7 && zoom_id == 2);
    assert(garrison_x == 11 && garrison_y == 22 && garrison_flag == 1);
    assert(window.redraws == 0);
    spot_kind = 1026;
    window.on_right_click(11, 22);
    assert(menu_calls == 1 && menu_id == 2 && menu_x == 11 && menu_y == 22);
    assert(window.redraws == 0);
    ReportIfGlobal.field_8_ = 6;
    ReportIfGlobal.report_faction_id_ = 9;
    GameDataStateStorage.fields_.game_state_ = 0xC0;
    window.on_right_click(11, 22);
    assert(satellite_calls == 1 && satellite_report == 9);
    assert(satellite_payload == 7 && satellite_kind == 1026 && window.redraws == 1);
    blocked = true;
    window.on_right_click(11, 22);
    assert(menu_calls == 1 && garrison_calls == 1 && window.redraws == 1);
}
'''.replace("@@BODY@@", body)
    source, executable = tmp_path / "control.cpp", tmp_path / "control"
    source.write_text(harness)
    built = subprocess.run([compiler, "-std=c++11", str(source), "-o", str(executable)],
                           capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)
    if damage:
        assert result.returncode != 0, "a hit-test output replaced a mouse coordinate"
    else:
        assert result.returncode == 0, result.stdout + result.stderr
