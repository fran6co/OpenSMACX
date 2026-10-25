"""Actual-source ReportIf::draw_sat workspace-rectangle control.

The complete mapped body is compiled from src/reportif.cpp.  Buffer doubles
record call-entry RECTs; they do not reimplement the panel algorithm.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/reportif.cpp"
SIGNATURE = "void ReportIf::draw_sat()"


def _actual_body():
    source = SOURCE.read_text(encoding="utf-8")
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if c == "\n" else " " for c in match[0]),
        source,
    )
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
    raise AssertionError("unterminated ReportIf::draw_sat body")


FIXTURE = r'''#include <assert.h>
#include <string.h>

struct RECT { int left, top, right, bottom; };
static RECT make_rect(int left, int top, int right, int bottom) {
    RECT rect; rect.left = left; rect.top = top; rect.right = right; rect.bottom = bottom;
    return rect;
}
struct BoxSpriteParams { int unused; };
static BoxSpriteParams InterfaceBoxStyles[1];
static RECT observed_rects[16];
static const BoxSpriteParams *observed_styles[16];
static int observed_box_count;

struct Buffer {
    void set_clip(RECT *) {}
    int box_sprite(RECT *rect, BoxSpriteParams *style) {
        if (observed_box_count >= 16) return 0;
        observed_rects[observed_box_count] = *rect;
        observed_styles[observed_box_count] = style;
        ++observed_box_count;
        return 0;
    }
    void set_font(void *, void *, void *, int) {}
    void set_text_color(int, int, int, int) {}
    void wrap_flying(char *) {}
    void wrap_flying(char *, int, int, int) {}
    void wrap_flying_flush() {}
    void write_cent_l(char *, int, int, int, int) {}
};

struct MainInterface : Buffer {
    RECT working_rect;
    RECT *buffer_working_rect() { return &working_rect; }
};
static MainInterface MainInterfaceGlobal;

struct Win { void hide() {} };
struct FlatButton : Buffer, Win {
    void hide() {}
    void show(int) {}
    void init(char *, int, int, int, int, int, MainInterface *, int) {}
    void redraw() {}
    void reset_field_A9C_if_ungrouped() {}
};
struct ButtonGroup {
    int field_84_;
    void init(int, int) {}
    void add(FlatButton *) {}
    void set(int, int) {}
};
struct ReportWin {
    ButtonGroup buttonGroup2_;
    FlatButton flatButton9_;
};
static ReportWin ReportWinGlobal;

struct Console { int faction_id_; };
static Console ConsoleGlobal;
struct Player { char *adj_name_faction; };
struct PlayerData { int satellites_[4]; };
static Player Players[8];
static PlayerData PlayersData[8];
struct Base { int faction_id_current; };
static Base Bases[1];
struct FacilityInfo { char *name; };
static FacilityInfo Facility[70];
struct GameFields { int base_current_count_; };
struct GameStorage { GameFields fields_; };
static GameStorage GameDataStateStorage;
static int ExpansionEnabled;

struct StringTableType { char *get(char *) { return (char *)"facility"; } };
static StringTableType StringTable;
static char StringTemp[1024];
static char ScriptPopupFile[] = "popup";
static char ReportSatelliteScratchUpperText[] = "upper";
static char ReportSatelliteScratchUpperLine[] = "upper line";
static char ReportSatelliteScratchLowerText[] = "lower";
static char ReportSatelliteScratchLowerLine[] = "lower line";
struct TutBase { int font1_; };
static TutBase TutWinBaseWindow;

static char *label_get(int) { return (char *)"label"; }
static char *CharUpperA(char *text) { return text; }
static void say_year(char *) {}
static int X_text_open(char *, char *) { return 1; }
static void text_close() {}
static char *text_get() { return 0; }
static void parse_string(char *, char *) {}
static int has_fac(int, int, int) { return 0; }

struct ReportIf {
    RECT field_20_, field_50_, field_30_, draw_workspace_rect_;
    int field_C_, field_10_, field_14_, field_18_, field_1C_;
    int field_1E230_, field_1E234_, report_faction_id_;
    void *field_1E184_, *field_1E190_, *field_1E194_, *field_1E198_, *field_1E180_;
    ButtonGroup buttonGroup3_, buttonGroup4_;
    FlatButton flatButtonsC_[7], flatButtonsD_[5];
    void draw_sat();
};
''' 

MAIN = r'''
static void reset_case(ReportIf &report, int help) {
    observed_box_count = 0;
    report.field_20_ = make_rect(10, 20, 210, 170);
    report.field_50_ = make_rect(30, 40, 230, 180);
    report.field_30_ = make_rect(30, 40, 230, 180);
    report.field_C_ = report.field_10_ = report.field_14_ = report.field_18_ = 0;
    report.field_1C_ = 50;
    report.field_1E230_ = report.field_1E234_ = report.report_faction_id_ = 0;
    report.field_1E184_ = report.field_1E190_ = 0;
    report.field_1E194_ = report.field_1E198_ = report.field_1E180_ = 0;
    MainInterfaceGlobal.working_rect = make_rect(0, 0, 300, 200);
    ReportWinGlobal.buttonGroup2_.field_84_ = help ? 0xD3C : 0;
    report.buttonGroup3_.field_84_ = 0; // invalid target avoids facility/player loop
    report.buttonGroup4_.field_84_ = 0;
    ConsoleGlobal.faction_id_ = 0;
    Players[0].adj_name_faction = (char *)"faction";
    GameDataStateStorage.fields_.base_current_count_ = 0;
    ExpansionEnabled = 0;
    StringTemp[0] = 0;
}

static bool same_rect(int index, int l, int t, int r, int b) {
    const RECT &v = observed_rects[index];
    return v.left == l && v.top == t && v.right == r && v.bottom == b &&
           observed_styles[index] == &InterfaceBoxStyles[0];
}

int main() {
    ReportIf report;
    reset_case(report, 0);
    report.draw_sat();
    // The pin has three ordered draws in each non-help panel.
    if (observed_box_count != 6) return 10;
    if (!same_rect(0, 16, 26, 204, 82) ||
        !same_rect(1, 16, 84, 204, 164) ||
        !same_rect(2, 16, 84, 166, 164) ||
        !same_rect(3, 36, 46, 224, 102) ||
        !same_rect(4, 36, 104, 224, 164) ||
        !same_rect(5, 36, 104, 166, 164)) return 11;

    reset_case(report, 1);
    report.draw_sat();
    // The two help branches retain their outer draws and skip both panel draws.
    if (observed_box_count != 2) return 12;
    if (!same_rect(0, 16, 26, 204, 164) ||
        !same_rect(1, 36, 46, 224, 174)) return 13;
    return 0;
}
'''


def _program(body):
    return FIXTURE + "\n" + body + "\n" + MAIN


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_draw_sat_box_calls_and_rectangles(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    cpp = tmp_path / (compiler_name.replace("+", "p") + "-reportif-draw-sat.cpp")
    exe = cpp.with_suffix("")
    cpp.write_text(_program(_actual_body()), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert run.returncode == 0, (
        "actual body must produce six non-help rectangles and two help rectangles; "
        "return 10 means one or both restored calls are missing; "
        + run.stdout + run.stderr
    )
