"""Exercise the mapped ReportIf callback's original singleton accesses."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest

from test_buffer_typed_font_consumers_oct10 import ROOT, extract_definition


SOURCE = ROOT / "src/reportif.cpp"
IDENTITY = ("0x004A5F40", "void ReportIf::on_iface_left_click(int x_coord, int y_coord)")

FIXTURE = r'''#include <assert.h>

struct MainInterface {};
struct ReportIf;
extern ReportIf ReportIfGlobal;

static int events[32];
static int event_count;
static void record(int event) { events[event_count++] = event; }

struct Spot {
    ReportIf *owner;
    int result;
    int mutate;
    int input_x;
    int input_y;
    int output_position;
    int output_type;
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
    unsigned int field_8_;
    int field_1E244_;
    int report_faction_id_;
    int report_selection_;
    Spot spot_;
    int report_calls;
    int report_args[3];
    void on_iface_left_click(int x_coord, int y_coord);
    void report(int report_id, int faction_id, int selection);
    ReportIf() : field_8_(0), field_1E244_(-1), report_faction_id_(0),
                 report_selection_(-1),
                 report_calls(0) {
        report_args[0] = report_args[1] = report_args[2] = 0;
        spot_.owner = this;
        spot_.result = -1;
        spot_.mutate = 0;
        spot_.input_x = spot_.input_y = -1;
        spot_.output_position = spot_.output_type = 0;
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
    if (mutate) {
        owner->field_8_ = 61;
        owner->report_faction_id_ = 55;
        ReportIfGlobal.field_8_ = 62;
        ReportIfGlobal.report_faction_id_ = 17;
    }
    record(1);
    return result;
}

void ReportWin::select_intel_topic() {
    ++topic_calls;
    ReportIfGlobal.field_8_ = 88;
    record(2);
}

void Win::set_modal(int mode, void (*callback)(), MainInterface *owner) {
    ++modal_calls;
    modal_mode = mode;
    modal_callback = callback;
    modal_owner = owner;
    ReportIfGlobal.report_faction_id_ = 91;
    record(3);
}

void ReportIf::report(int report_id, int faction_id, int selection) {
    ++report_calls;
    report_args[0] = report_id;
    report_args[1] = faction_id;
    report_args[2] = selection;
    field_8_ = 66;
    record(4);
}

static void reset_fixture(ReportIf &receiver) {
    event_count = 0;
    wait_calls = 0;
    ReportWinGlobal.topic_calls = 0;
    ReportWinGlobal.modal_calls = 0;
    ReportIfGlobal.field_8_ = 31;
    ReportIfGlobal.field_1E244_ = -1;
    ReportIfGlobal.report_faction_id_ = 9;
    ReportIfGlobal.report_selection_ = 202;
    ReportIfGlobal.report_calls = 0;
    ReportIfGlobal.report_args[0] = ReportIfGlobal.report_args[1] =
        ReportIfGlobal.report_args[2] = -1;
    receiver.field_8_ = 7;
    receiver.field_1E244_ = -1;
    receiver.report_faction_id_ = 12;
    receiver.report_selection_ = 101;
    receiver.report_calls = 0;
    receiver.spot_.calls = 0;
    receiver.spot_.result = 5;
    receiver.spot_.mutate = 1;
    receiver.spot_.output_position = 444;
    receiver.spot_.output_type = 37;
    ActiveIfaceIdStorage = 2;
    IfaceIdToSlotStorage[2] = 4;
}
'''

MAIN = r'''
int main() {
    ReportIf receiver;

    // The initial guard belongs to the actual callback receiver, not the
    // singleton whose later state is changed by this callback.
    reset_fixture(receiver);
    receiver.field_8_ = 6;
    ReportIfGlobal.field_8_ = 7;
    receiver.on_iface_left_click(10, 20);
    assert(receiver.spot_.calls == 0);
    assert(ReportIfGlobal.field_8_ == 7 && receiver.field_8_ == 6);
    assert(ReportWinGlobal.topic_calls == 0 && event_count == 0);

    // A no-hit result writes the output temporaries but exits before any
    // member/global state transition or downstream callback.
    reset_fixture(receiver);
    receiver.spot_.result = -1;
    receiver.on_iface_left_click(10, 20);
    assert(receiver.spot_.calls == 1);
    assert(receiver.spot_.input_x == 10 && receiver.spot_.input_y == 20);
    assert(receiver.field_1E244_ == -1);
    assert(ReportIfGlobal.field_8_ == 62 && ReportIfGlobal.report_faction_id_ == 17);
    assert(receiver.field_8_ == 61 && receiver.report_faction_id_ == 55);
    assert(ReportWinGlobal.topic_calls == 0 && ReportWinGlobal.modal_calls == 0);
    assert(ReportIfGlobal.report_calls == 0 && event_count == 1 && events[0] == 1);

    // Non-world-map path: Spot mutates distinct receiver/global state;
    // topic selection mutates the singleton mode before its explicit reset;
    // modal dispatch observes and mutates only after the clicked topic store.
    reset_fixture(receiver);
    receiver.on_iface_left_click(10, 20);
    assert(receiver.spot_.input_x == 10 && receiver.spot_.input_y == 20);
    assert(receiver.field_1E244_ == 444);
    assert(receiver.field_8_ == 61 && receiver.report_faction_id_ == 55);
    assert(ReportIfGlobal.field_1E244_ == -1);
    assert(ReportIfGlobal.field_8_ == 7 && ReportIfGlobal.report_faction_id_ == 91);
    assert(ReportWinGlobal.topic_calls == 1 && ReportWinGlobal.modal_calls == 1);
    assert(ReportWinGlobal.modal_mode == 0 &&
           ReportWinGlobal.modal_callback == &pop_wait &&
           ReportWinGlobal.modal_owner == &MainInterfaceGlobal);
    assert(ReportIfGlobal.report_calls == 0 && wait_calls == 0);
    assert(event_count == 3 && events[0] == 1 && events[1] == 2 && events[2] == 3);

    // World-map path: faction is read from the singleton after Spot's
    // mutation, and report receives (7, faction, 0) in original order.
    reset_fixture(receiver);
    IfaceIdToSlotStorage[2] = 5;
    receiver.on_iface_left_click(30, 40);
    assert(receiver.field_1E244_ == 444 && receiver.field_8_ == 61);
    assert(ReportIfGlobal.field_8_ == 66);
    assert(ReportIfGlobal.report_calls == 1);
    assert(ReportIfGlobal.report_args[0] == 7);
    assert(ReportIfGlobal.report_args[1] == 17);
    assert(ReportIfGlobal.report_args[2] == 0);
    assert(receiver.report_selection_ == 101 && ReportIfGlobal.report_selection_ == 202);
    assert(ReportWinGlobal.topic_calls == 1 && ReportWinGlobal.modal_calls == 0);
    assert(event_count == 3 && events[0] == 1 && events[1] == 2 && events[2] == 4);
    return 0;
}
'''


def source_program(mutation: str | None = None) -> str:
    source = SOURCE.read_text(encoding="utf-8")
    body = extract_definition(source, *IDENTITY)
    if mutation == "receiver_mode":
        old = "ReportIfGlobal.field_8_ = 7;"
        assert body.count(old) == 1
        body = body.replace(old, "field_8_ = 7;", 1)
    elif mutation == "receiver_faction":
        old = "ReportIfGlobal.report(7, ReportIfGlobal.report_faction_id_, 0);"
        assert body.count(old) == 1
        body = body.replace(old, "ReportIfGlobal.report(7, report_faction_id_, 0);", 1)
    elif mutation is not None:
        raise AssertionError(f"unknown mutation: {mutation}")
    return FIXTURE + "\n" + body + "\n" + MAIN


def compile_and_run(compiler: str, program: str, tmp_path: Path,
                    label: str) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / f"reportif_click_{Path(compiler).name}_{label}.cpp"
    exe = tmp_path / f"reportif_click_{Path(compiler).name}_{label}"
    cpp.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30)
    if built.returncode != 0:
        return built
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_reportif_click_global_receiver_effects(compiler_name: str,
                                                        tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(compiler, source_program(), tmp_path, "actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", ("receiver_mode", "receiver_faction"))
def test_successful_wrong_receiver_mutants_fail(compiler_name: str, mutation: str,
                                                tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(compiler, source_program(mutation), tmp_path, mutation)
    assert result.returncode != 0, f"{compiler_name} accepted {mutation}: {result.stdout}"
