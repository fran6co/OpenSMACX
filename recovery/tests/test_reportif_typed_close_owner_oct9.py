from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def source_bodies():
    source = (ROOT / "src/reportif.cpp").read_text()
    close_start = source.index("void ReportIf::close_intel()")
    close_end = source.index("\n}\n", close_start) + 3
    done_start = source.index("void ReportIf::done()")
    done_end = source.index("\n}\n", done_start) + 3
    close_body = source[close_start:close_end]
    done_body = source[done_start:done_end]
    assert "listBox1_.close();" in close_body
    assert "listBox2_.close();" in close_body
    assert "reinterpret_cast" not in close_body
    assert "reinterpret_cast<SubInterface" not in done_body
    assert done_body.index("release_modal") < done_body.index("release_iface_mode")
    header = (ROOT / "src/reportif.h").read_text()
    assert "ListBox listBox1_;" in header and "ListBox listBox2_;" in header
    assert "class ReportIf : public SubInterface" in header
    return close_body, done_body


def close_sat_body():
    source = (ROOT / "src/reportif.cpp").read_text()
    signature = "void ReportIf::close_sat()"
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated ReportIf::close_sat body")


def host_source(close_body, done_body):
    return r'''#include <cassert>
#include <vector>
struct Event { int kind; const void *receiver; };
static std::vector<Event> events;
struct ListBox {
    void close() { events.push_back((Event){1, this}); }
};
struct SubInterface {
    void release_iface_mode() { events.push_back((Event){3, this}); }
};
struct Win {
    void release_modal() { events.push_back((Event){2, this}); }
};
struct ReportWin : Win {};
ReportWin ReportWinGlobal;
struct ReportIf : SubInterface {
    ListBox listBox1_;
    ListBox listBox2_;
    void close_intel();
    void done();
};
''' + close_body + "\n" + done_body + r'''
int main() {
    ReportIf report;
    report.close_intel();
    assert(events.size()==2);
    assert(events[0].kind==1 && events[0].receiver==&report.listBox1_);
    assert(events[1].kind==1 && events[1].receiver==&report.listBox2_);

    events.clear();
    report.done();
    assert(events.size()==2);
    assert(events[0].kind==2 && events[0].receiver==static_cast<Win *>(&ReportWinGlobal));
    assert(events[1].kind==3 && events[1].receiver==static_cast<SubInterface *>(&report));
    assert(static_cast<const void *>(&report)==
           static_cast<const void *>(static_cast<SubInterface *>(&report)));
}
'''


def compile_and_run(compiler, path, close_body, done_body):
    source = path.with_suffix(".cpp")
    source.write_text(host_source(close_body, done_body))
    executable = path
    compiled = subprocess.run(
        [compiler, "-std=c++98", str(source), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True)


def satellite_host_source(body):
    return r'''#include <cassert>
#include <vector>
enum { button_hide = 1, button_close = 2, win_hide = 3,
       reset_ungrouped = 4, timer_stop = 5 };
struct Event { int action; const void *receiver; int value; };
static std::vector<Event> events;
static void record(int action, const void *receiver, int value = 0) {
    Event event = { action, receiver, value };
    events.push_back(event);
}
struct Win {
    void hide() { record(win_hide, this); }
};
struct FlatButton : Win {
    int destroyed;
    FlatButton() : destroyed(0) {}
    void hide() { record(button_hide, this); }
    void close() { destroyed = 1; record(button_close, this); }
    void reset_field_A9C_if_ungrouped() { record(reset_ungrouped, this); }
};
struct Timer {
    int id;
    void stop() { record(timer_stop, this, id); }
};
struct Console { int faction_id_; } ConsoleGlobal;
struct ReportWin {
    FlatButton flatButton9_, flatButton7_, flatButton8_;
    Timer time2_, time3_;
} ReportWinGlobal;
struct ReportIf {
    FlatButton flatButtonsC_[7];
    FlatButton flatButtonsD_[5];
    void close_sat();
};
''' + body + r'''
int main() {
    ConsoleGlobal.faction_id_ = 3;
    ReportWinGlobal.time2_.id = 2;
    ReportWinGlobal.time3_.id = 3;
    ReportIf report;
    report.close_sat();

    const FlatButton *first[] = {
        &report.flatButtonsC_[0], &report.flatButtonsC_[1],
        &report.flatButtonsC_[3], &report.flatButtonsC_[4],
        &report.flatButtonsC_[5], &report.flatButtonsC_[6]
    };
    assert(events.size() == 19);
    for (unsigned i = 0; i < 6; ++i) {
        assert(events[i].action == button_hide && events[i].receiver == first[i]);
    }
    for (unsigned i = 0; i < 5; ++i) {
        assert(events[6 + i].action == button_hide);
        assert(events[6 + i].receiver == &report.flatButtonsD_[i]);
    }
    assert(events[2].receiver != &report.flatButtonsC_[2]);

    FlatButton *tail[] = { &ReportWinGlobal.flatButton9_,
                           &ReportWinGlobal.flatButton7_,
                           &ReportWinGlobal.flatButton8_ };
    for (unsigned i = 0; i < 3; ++i) {
        assert(events[11 + i * 2].action == reset_ungrouped);
        assert(events[11 + i * 2].receiver == tail[i]);
        assert(events[12 + i * 2].action == win_hide);
        assert(events[12 + i * 2].receiver == static_cast<Win *>(tail[i]));
    }
    assert(events[17].action == timer_stop && events[17].receiver == &ReportWinGlobal.time2_);
    assert(events[17].value == 2);
    assert(events[18].action == timer_stop && events[18].receiver == &ReportWinGlobal.time3_);
    assert(events[18].value == 3);
    for (unsigned i = 0; i < 7; ++i) assert(report.flatButtonsC_[i].destroyed == 0);
    for (unsigned i = 0; i < 5; ++i) assert(report.flatButtonsD_[i].destroyed == 0);
    for (unsigned i = 0; i < events.size(); ++i) assert(events[i].action != button_close);
}
'''


def compile_satellite(compiler, path, body):
    source = path.with_suffix(".cpp")
    source.write_text(satellite_host_source(body))
    executable = path
    compiled = subprocess.run(
        [compiler, "-std=c++98", str(source), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_reportif_members_and_release_order_with_executed_mutants(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("requested host C++ compiler unavailable")
    close_body, done_body = source_bodies()
    actual = compile_and_run(compiler, tmp_path / "reportif_actual",
                             close_body, done_body)
    assert actual.returncode == 0, actual.stderr

    wrong_member = close_body.replace("listBox2_.close();", "listBox1_.close();")
    assert wrong_member != close_body
    mutant_member = compile_and_run(compiler, tmp_path / "reportif_wrong_member",
                                    wrong_member, done_body)
    assert mutant_member.returncode != 0

    swapped_release = done_body.replace(
        "ReportWinGlobal.Win::release_modal();\n    SubInterface::release_iface_mode();",
        "SubInterface::release_iface_mode();\n    ReportWinGlobal.Win::release_modal();",
    )
    assert swapped_release != done_body
    mutant_order = compile_and_run(compiler, tmp_path / "reportif_swapped_release",
                                   close_body, swapped_release)
    assert mutant_order.returncode != 0


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_close_sat_hides_controls_in_original_order_and_rejects_close_mutant(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("requested host C++ compiler unavailable")
    body = close_sat_body()
    assert body.count("button->hide();") == 2
    assert "button->close();" not in body
    actual = compile_satellite(compiler, tmp_path / "close_sat_actual", body)
    assert actual.returncode == 0, actual.stderr

    old_close = body.replace("button->hide();", "button->close();")
    assert old_close != body and old_close.count("button->close();") == 2
    mutant = compile_satellite(compiler, tmp_path / "close_sat_old_close", old_close)
    assert mutant.returncode != 0
