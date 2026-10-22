"""The mapped PopMenu destructor is inline where its real class is declared.

The host control checks C++ base teardown order and once-only destruction; it
does not establish target ABI, linked selection, or GUI/runtime behavior.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery import binding_source
from match_recovery.source import read_file, read_text
from test_netwin_owner_behavior import extract_function


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/popmenu.h"
OWNER = ROOT / "src/popmenu.cpp"
ADDRESS = 0x00421400
NAME = "??1PopMenu@@QAE@XZ"
SYMBOL = "??1PopMenu@@UAE@XZ"
SPANS = ((0x00421400, 0x00421634), (0x00652AB0, 0x00652B9F))


HARNESS = r'''#include <cassert>
#include <string>
#include <vector>

static std::vector<std::string> events;
static int popup_destructors;
static int pulldown_destructors;
struct Popup {
    Popup() { events.push_back("popup+"); }
    virtual ~Popup() { ++popup_destructors; events.push_back("popup-"); }
};
struct PullDown {
    PullDown() { events.push_back("pulldown+"); }
    virtual ~PullDown() { ++pulldown_destructors; events.push_back("pulldown-"); }
    int get_selected_id() { return 17; }
};
/* ACTUAL POPMENU CLASS */
int PopMenu::init() { return 0; }
int PopMenu::exec(int, int, int (*)()) { return 0; }

static void expect_events(const char *const *expected, unsigned count) {
    assert(events.size() == count);
    for (unsigned i = 0; i < count; ++i) assert(events[i] == expected[i]);
}

int main() {
    {
        PopMenu menu;
        const char *constructed[] = {"popup+", "pulldown+"};
        expect_events(constructed, 2);
        assert(menu.get_selected_id() == 17);
    }
    const char *all[] = {"popup+", "pulldown+", "pulldown-", "popup-"};
    expect_events(all, 4);
    assert(pulldown_destructors == 1);
    assert(popup_destructors == 1);

    events.clear();
    popup_destructors = pulldown_destructors = 0;
    Popup *base_view = new PopMenu;
    delete base_view;
    expect_events(all, 4);
    assert(pulldown_destructors == 1);
    assert(popup_destructors == 1);
    return 0;
}
'''


def source_program(menu_class):
    assert HARNESS.count("/* ACTUAL POPMENU CLASS */") == 1
    return HARNESS.replace("/* ACTUAL POPMENU CLASS */", menu_class + ";")


def compile_run(tmp_path, compiler, source, name):
    cpp = tmp_path / (name + ".cpp")
    executable = tmp_path / name
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=10)


def test_inline_popmenu_destruction_is_once_only_in_reverse_base_order(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler is not None, "a host C++ compiler is required for this control"
    menu_class = extract_function(HEADER.read_text(),
                                  "class PopMenu : public Popup, public PullDown")
    result = compile_run(tmp_path, compiler, source_program(menu_class), "valid")
    assert result.returncode == 0, result.stdout + result.stderr

    # Both mutations compile. One explicitly destroys a base a second time;
    # the other reverses declaration order and therefore observable teardown.
    double_destroy = menu_class.replace(
        "~PopMenu() {}", "~PopMenu() { PullDown::~PullDown(); }")
    assert double_destroy != menu_class
    result = compile_run(tmp_path, compiler, source_program(double_destroy),
                         "double-destroy")
    assert result.returncode != 0, "double destruction survived the control"

    reversed_bases = menu_class.replace(
        "class PopMenu : public Popup, public PullDown",
        "class PopMenu : public PullDown, public Popup")
    assert reversed_bases != menu_class
    result = compile_run(tmp_path, compiler, source_program(reversed_bases),
                         "reversed-bases")
    assert result.returncode != 0, "reversed base teardown survived the control"


def test_popmenu_original_mapping_binding_history_and_header_body_hint():
    cpp_text = OWNER.read_text()
    header_text = HEADER.read_text()
    rows = read_file(OWNER)
    matches = [row for row in rows if row.address == ADDRESS]
    assert len(matches) == 1
    row = matches[0]
    base = subprocess.run(
        ["git", "show", "7fb7f072926c248045b4f70eaaf1cf8e3fd456ce:src/popmenu.cpp"],
        cwd=ROOT, capture_output=True, text=True, check=True).stdout
    baseline_rows = read_text(base, "src/popmenu.cpp")
    baseline_matches = [old for old in baseline_rows if old.address == ADDRESS]
    assert len(baseline_matches) == 1
    baseline = baseline_matches[0]
    assert row.name == NAME
    assert row.symbol == SYMBOL
    assert row.image_spans == SPANS
    assert (row.name, row.symbol, row.image_spans) == (
        baseline.name, baseline.symbol, baseline.image_spans)
    assert row.origin == baseline.origin
    assert row.levers == baseline.levers
    assert row.ruled_out == baseline.ruled_out
    assert row.unrecoverable == baseline.unrecoverable
    assert row.deferred == baseline.deferred
    assert row.body == "src/popmenu.h"
    assert row.path == OWNER
    if baseline.byte_exact:
        assert row.byte_exact
    if baseline.semantic:
        assert row.semantic
    assert "~PopMenu() {}" in extract_function(
        header_text, "class PopMenu : public Popup, public PullDown")
    assert "PopMenu::~PopMenu()" not in cpp_text

    rationale = binding_source.parse(cpp_text, rows)
    old_rationale = binding_source.parse(base, baseline_rows)
    assert rationale[row.line] == old_rationale[baseline.line]
    assert rationale[row.line]["evidence"] == ["image:0x421400"]
