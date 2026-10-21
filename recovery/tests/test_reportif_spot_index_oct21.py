"""Exercise the recovered ReportIf spot-result branch using its actual body."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_buffer_typed_font_consumers_oct10 import extract_definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/reportif.cpp"
IDENTITY = (
    "0x004A5F40",
    "void ReportIf::on_iface_left_click(int x_coord, int y_coord)",
)

FIXTURE = r'''#include <assert.h>

struct MainInterface {};
struct ReportIf;
extern ReportIf ReportIfGlobal;

static int events[16];
static int event_count;
static void record(int event) { events[event_count++] = event; }

struct Spot {
    int result;
    int output_position;
    int output_type;
    int input_x;
    int input_y;
    int calls;
    int check(int x, int y, int *position, int *type);
};

struct Win {
    int modal_calls;
    int modal_mode;
    void (*modal_callback)();
    MainInterface *modal_owner;
    void set_modal(int mode, void (*callback)(), MainInterface *owner);
};

struct ReportIf {
    int field_8_;
    int field_1E244_;
    int report_faction_id_;
    int report_calls;
    int report_args[3];
    Spot spot_;
    void on_iface_left_click(int x_coord, int y_coord);
    void report(int report_id, int faction_id, int selection);
    ReportIf() : field_8_(0), field_1E244_(-1), report_faction_id_(0),
                 report_calls(0) {
        report_args[0] = report_args[1] = report_args[2] = -1;
        spot_.result = -1;
        spot_.output_position = -1;
        spot_.output_type = -1;
        spot_.input_x = spot_.input_y = -1;
        spot_.calls = 0;
    }
};

struct ReportWin : Win {
    int topic_calls;
    ReportWin() : topic_calls(0) {}
    void select_intel_topic();
};

static MainInterface MainInterfaceGlobal;
ReportIf ReportIfGlobal;
ReportWin ReportWinGlobal;
int ActiveIfaceIdStorage;
int IfaceIdToSlotStorage[4];
static int wait_calls;
static void pop_wait() { ++wait_calls; }

int Spot::check(int x, int y, int *position, int *type) {
    ++calls;
    input_x = x;
    input_y = y;
    *position = output_position;
    *type = output_type;
    record(1);
    return result;
}

void ReportWin::select_intel_topic() {
    ++topic_calls;
    record(2);
}

void Win::set_modal(int mode, void (*callback)(), MainInterface *owner) {
    ++modal_calls;
    modal_mode = mode;
    modal_callback = callback;
    modal_owner = owner;
    record(3);
}

void ReportIf::report(int report_id, int faction_id, int selection) {
    ++report_calls;
    report_args[0] = report_id;
    report_args[1] = faction_id;
    report_args[2] = selection;
    record(4);
}

static void reset_fixture(ReportIf &receiver) {
    event_count = 0;
    wait_calls = 0;
    ReportWinGlobal.topic_calls = 0;
    ReportWinGlobal.modal_calls = 0;
    ReportWinGlobal.modal_mode = -1;
    ReportWinGlobal.modal_callback = 0;
    ReportWinGlobal.modal_owner = 0;
    ReportIfGlobal.field_8_ = 31;
    ReportIfGlobal.field_1E244_ = -1;
    ReportIfGlobal.report_faction_id_ = 9;
    ReportIfGlobal.report_calls = 0;
    ReportIfGlobal.report_args[0] = ReportIfGlobal.report_args[1] =
        ReportIfGlobal.report_args[2] = -1;
    receiver.field_8_ = 7;
    receiver.field_1E244_ = -1;
    receiver.report_faction_id_ = 12;
    receiver.report_calls = 0;
    receiver.spot_.result = -1;
    receiver.spot_.output_position = 444;
    receiver.spot_.output_type = 37;
    receiver.spot_.input_x = receiver.spot_.input_y = -1;
    receiver.spot_.calls = 0;
    ActiveIfaceIdStorage = 2;
    IfaceIdToSlotStorage[2] = 4;
}
'''

MAIN = r'''
int main() {
    ReportIf receiver;

    // The initial mode guard is on this receiver, and prevents the check.
    reset_fixture(receiver);
    receiver.field_8_ = 6;
    ReportIfGlobal.field_8_ = 7;
    receiver.on_iface_left_click(10, 20);
    assert(receiver.spot_.calls == 0 && event_count == 0);
    assert(receiver.field_8_ == 6 && ReportIfGlobal.field_8_ == 7);

    // -1 is a miss: output temporaries change, but no member/global effect
    // follows the check call.
    reset_fixture(receiver);
    receiver.spot_.result = -1;
    receiver.on_iface_left_click(10, 20);
    assert(receiver.spot_.calls == 1);
    assert(receiver.spot_.input_x == 10 && receiver.spot_.input_y == 20);
    assert(receiver.field_1E244_ == -1 && ReportIfGlobal.field_1E244_ == -1);
    assert(receiver.field_8_ == 7 && ReportIfGlobal.field_8_ == 31);
    assert(ReportWinGlobal.topic_calls == 0 && ReportWinGlobal.modal_calls == 0);
    assert(ReportIfGlobal.report_calls == 0 && event_count == 1 && events[0] == 1);

    // Index zero is a hit. Position/type are written through the original
    // aliased argument slots; modal dispatch follows topic then singleton mode.
    reset_fixture(receiver);
    receiver.spot_.result = 0;
    receiver.on_iface_left_click(30, 40);
    assert(receiver.spot_.input_x == 30 && receiver.spot_.input_y == 40);
    assert(receiver.field_1E244_ == 444 && ReportIfGlobal.field_1E244_ == -1);
    assert(receiver.field_8_ == 7 && ReportIfGlobal.field_8_ == 7);
    assert(receiver.spot_.output_type == 37);
    assert(ReportWinGlobal.topic_calls == 1 && ReportWinGlobal.modal_calls == 1);
    assert(ReportWinGlobal.modal_mode == 0 &&
           ReportWinGlobal.modal_callback == &pop_wait &&
           ReportWinGlobal.modal_owner == &MainInterfaceGlobal);
    assert(ReportIfGlobal.report_calls == 0 && wait_calls == 0);
    assert(event_count == 3 && events[0] == 1 && events[1] == 2 && events[2] == 3);

    // The last valid index is also a hit and takes the world-map report path.
    reset_fixture(receiver);
    receiver.spot_.result = 255;
    IfaceIdToSlotStorage[2] = 5;
    ReportIfGlobal.report_faction_id_ = 17;
    receiver.on_iface_left_click(50, 60);
    assert(receiver.field_1E244_ == 444 && receiver.field_8_ == 7);
    assert(ReportIfGlobal.field_8_ == 7 && ReportIfGlobal.report_calls == 1);
    assert(ReportIfGlobal.report_args[0] == 7);
    assert(ReportIfGlobal.report_args[1] == 17);
    assert(ReportIfGlobal.report_args[2] == 0);
    assert(receiver.report_calls == 0 && receiver.report_faction_id_ == 12);
    assert(ReportWinGlobal.topic_calls == 1 && ReportWinGlobal.modal_calls == 0);
    assert(event_count == 3 && events[0] == 1 && events[1] == 2 && events[2] == 4);
    return 0;
}
'''


def source_program(mutation=None):
    body = extract_definition(SOURCE.read_text(encoding="utf-8"), *IDENTITY)
    if mutation == "invert_hit_test":
        old = ") + 1 != 0) {"
        assert body.count(old) == 1
        body = body.replace(old, ") + 1 == 0) {", 1)
    elif mutation == "wrong_singleton_receiver":
        old = "ReportIfGlobal.field_8_ = 7;"
        assert body.count(old) == 1
        body = body.replace(old, "field_8_ = 7;", 1)
    elif mutation is not None:
        raise AssertionError("unknown mutation: " + mutation)
    return FIXTURE + "\n" + body + "\n" + MAIN


def compile_and_run(compiler, program, tmp_path, label):
    cpp = tmp_path / ("reportif_spot_" + Path(compiler).name + "_" + label + ".cpp")
    exe = cpp.with_suffix("")
    cpp.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=45)
    if built.returncode != 0:
        return built, None
    return built, subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_reportif_spot_result_and_effect_order(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    built, result = compile_and_run(compiler, source_program(), tmp_path, "actual")
    assert built.returncode == 0, built.stdout + built.stderr
    assert result is not None and result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", ("invert_hit_test", "wrong_singleton_receiver"))
def test_compile_success_damage_controls_fail(compiler_name, mutation, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    built, result = compile_and_run(compiler, source_program(mutation), tmp_path, mutation)
    assert built.returncode == 0, built.stdout + built.stderr
    assert result is not None and result.returncode != 0, mutation + " escaped the behavioral witness"
