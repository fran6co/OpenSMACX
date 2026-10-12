"""Bounded actual-source hover dispatch/compaction controls, not game proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def between(text, start, end):
    begin = text.index(start)
    return text[begin:text.index(end, begin)]


def test_hover_uses_existing_status_storage(tmp_path):
    interface = (ROOT / "src/maininterface.cpp").read_text()
    vehicle = (ROOT / "src/veh.cpp").read_text()
    header = (ROOT / "src/statuswin.h").read_text()
    for alias in ("IfaceCursorSpot", "StatusHoverBaseId", "StatusHoverVehId"):
        assert alias not in interface
    assert "int *const StatusHoverVehId =" not in vehicle
    assert "StatusHoverVehIdAlt" not in vehicle
    assert "ConsoleStatusWin.field_15CC_" in vehicle
    assert "int32_t field_15CC_;" in header
    assert "hover_vehicle_id_" in header and "hover_base_id_" in header
    hover = between(interface, "    int check2 = ConsoleStatusWin.spot_.check",
                    "    if (iface_redraw_pending_ == 1)")
    compact = between(vehicle, "    if (ConsoleStatusWin.hover_vehicle_id_ == veh_id)",
                      "    if (GameDataStateStorage.fields_.vehicle_selection_follow_limit_")
    harness = r'''
    #include <cassert>
    struct Spot { int result, pos, type;
      int check(int,int,int *p,int *t) {*p=pos;*t=type;return result;} };
    struct StatusWin {
      int hover_vehicle_id_, hover_base_id_, field_15CC_; Spot spot_;
      int draws, resets, vehicle, base;
      void draw_active(int v,int b,int) {++draws;vehicle=v;base=b;}
      void reset_active() {++resets;hover_vehicle_id_=hover_base_id_=-1;}
    } ConsoleStatusWin;
    struct MainInterface {
      void dispatch(int a1,int a2) { ACTUAL_HOVER }
    };
    void compact(int veh_id) { ACTUAL_COMPACT }
    int main() {
      MainInterface iface;
      ConsoleStatusWin.hover_vehicle_id_=42;ConsoleStatusWin.hover_base_id_=-1;
      ConsoleStatusWin.spot_={0,9,11};iface.dispatch(4,5);
      assert(ConsoleStatusWin.hover_base_id_==9 && ConsoleStatusWin.hover_vehicle_id_==-1);
      assert(ConsoleStatusWin.draws==1 && ConsoleStatusWin.vehicle==-1 && ConsoleStatusWin.base==9);
      iface.dispatch(4,5);assert(ConsoleStatusWin.draws==1);
      ConsoleStatusWin.spot_={0,7,0};iface.dispatch(4,5);
      assert(ConsoleStatusWin.hover_base_id_==-1 && ConsoleStatusWin.hover_vehicle_id_==7);
      assert(ConsoleStatusWin.draws==2 && ConsoleStatusWin.vehicle==7 && ConsoleStatusWin.base==-1);
      ConsoleStatusWin.spot_.result=-1;iface.dispatch(4,5);
      assert(ConsoleStatusWin.resets==1);
      ConsoleStatusWin.hover_vehicle_id_=5;compact(5);assert(ConsoleStatusWin.hover_vehicle_id_==-1);
      ConsoleStatusWin.hover_vehicle_id_=9;compact(5);assert(ConsoleStatusWin.hover_vehicle_id_==8);
      ConsoleStatusWin.hover_vehicle_id_=4;compact(5);assert(ConsoleStatusWin.hover_vehicle_id_==4);
      ConsoleStatusWin.field_15CC_=5;compact(5);assert(ConsoleStatusWin.field_15CC_==-1);
      ConsoleStatusWin.field_15CC_=9;compact(5);assert(ConsoleStatusWin.field_15CC_==8);
      ConsoleStatusWin.field_15CC_=4;compact(5);assert(ConsoleStatusWin.field_15CC_==4);
    }
    '''.replace("ACTUAL_HOVER", hover).replace("ACTUAL_COMPACT", compact)
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source = tmp_path / "hover.cpp"
    source.write_text(harness)
    binary = tmp_path / "hover"
    result = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                             str(source), "-o", str(binary)], capture_output=True,
                            text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    subprocess.run([str(binary)], check=True, timeout=10)
