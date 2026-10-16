"""Behavioral host control for Menu's re-entrant selected PullDown cell."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_definition(source: str, signature: str) -> str:
    assert source.count(signature) == 1
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unclosed function body: " + signature)


def fixture(tmp_path: Path, *, cached: bool = False) -> Path:
    source = (ROOT / "src/menu.cpp").read_text(encoding="utf-8")
    event = extract_definition(source, "void Menu::menu_event(int id)")
    hide = extract_definition(source, "void Menu::hide_menus()")
    if cached:
        event = event.replace("PullDown *&pull_down =", "PullDown *pull_down =")
        hide = hide.replace("PullDown *&pull_down =", "PullDown *pull_down =")
    prelude = r'''#include <cassert>
#include <vector>
struct Sound { int is_playing() { return 1; } int play() { return 0; } };
struct Menu;
struct Win {
    virtual ~Win() {}
    virtual void hide();
    virtual void redraw();
    virtual void flush_input();
    Sound *event_window() { return 0; }
};
struct PullDown : Win { void hide(); };
struct MenuEntry { PullDown *pull_down; };
typedef void (*MenuProc)(int);
struct Menu : Win {
    int field_A24_;
    unsigned int field_A28_;
    MenuProc proc_;
    Win *win_parent_;
    MenuEntry entries_[2];
    Menu() : field_A24_(0), field_A28_(4), proc_(0), win_parent_(0) {}
    void hide();
    void redraw();
    void flush_input();
    void menu_event(int);
    void hide_menus();
};
static int MenuSelection = 0;
static std::vector<int> events;
static Menu *active_menu = 0;
static PullDown *replacement[3];
enum { PULL_HIDE=1, MENU_HIDE, REDRAW, FLUSH, TASK_PULL, TASK_MENU, CALLBACK };
void do_all_tasks(Win *);
void Win::hide() { events.push_back(MENU_HIDE); }
void Win::redraw() { events.push_back(REDRAW); assert(active_menu->field_A24_ == -1 && MenuSelection == -1); active_menu->entries_[0].pull_down = replacement[1]; active_menu->field_A24_ = 1; }
void Win::flush_input() { events.push_back(FLUSH); assert(MenuSelection == -1); active_menu->entries_[0].pull_down = replacement[2]; active_menu->field_A24_ = 1; }
void PullDown::hide() { events.push_back(PULL_HIDE); assert(active_menu->field_A24_ == 0); active_menu->entries_[0].pull_down = replacement[0]; }
void Menu::hide() { events.push_back(MENU_HIDE); }
void Menu::redraw() { Win::redraw(); }
void Menu::flush_input() { Win::flush_input(); }
void do_all_tasks(Win *window) { events.push_back(window == active_menu ? TASK_MENU : TASK_PULL); if (window != active_menu) assert(window == replacement[2]); }
void callback(int) { events.push_back(CALLBACK); }
'''
    scenarios = r'''
static void reset(Menu &menu, PullDown &initial, PullDown &a, PullDown &b, PullDown &c) {
    events.clear(); active_menu=&menu; replacement[0]=&a; replacement[1]=&b; replacement[2]=&c;
    menu.field_A24_=0; menu.field_A28_=4; menu.proc_=0; menu.win_parent_=0;
    menu.entries_[0].pull_down=&initial; MenuSelection=8;
}
static void check_event() {
    Menu menu; PullDown initial, a, b, c; reset(menu,initial,a,b,c); menu.proc_=callback;
    menu.menu_event(9);
    const int expected[] = {PULL_HIDE,REDRAW,FLUSH,TASK_PULL,TASK_MENU,CALLBACK};
    assert(events.size()==sizeof(expected)/sizeof(expected[0]));
    for (unsigned int i=0;i<events.size();++i) assert(events[i]==expected[i]);
    assert(menu.entries_[0].pull_down==&c && menu.field_A24_==1 && MenuSelection==-1);
}
static void check_hide_menus() {
    Menu menu; PullDown initial, a, b, c; reset(menu,initial,a,b,c);
    menu.hide_menus();
    const int expected[] = {PULL_HIDE,REDRAW,FLUSH,TASK_PULL,TASK_MENU};
    assert(events.size()==sizeof(expected)/sizeof(expected[0]));
    for (unsigned int i=0;i<events.size();++i) assert(events[i]==expected[i]);
    assert(menu.entries_[0].pull_down==&c && menu.field_A24_==1 && MenuSelection==-1);
    events.clear(); menu.field_A24_=-1; menu.hide_menus(); assert(events.empty());
}
int main() { check_event(); check_hide_menus(); }
'''
    path = tmp_path / ("menu-live-cell-cached.cpp" if cached else "menu-live-cell.cpp")
    path.write_text(prelude + event + "\n" + hide + scenarios, encoding="utf-8")
    return path


def compile_and_run(source: Path, tmp_path: Path, compiler: str):
    binary = tmp_path / source.stem
    built = subprocess.run([compiler, "-std=c++03", "-O0", "-Wall", "-Wextra", "-Werror", "-pedantic", str(source), "-o", str(binary)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


def test_actual_menu_selected_cell_is_reloaded_after_callbacks(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no host C++ compiler")
    result = compile_and_run(fixture(tmp_path), tmp_path, compiler)
    assert result.returncode == 0, result.stderr


def test_cached_pull_down_value_is_rejected(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no host C++ compiler")
    result = compile_and_run(fixture(tmp_path, cached=True), tmp_path, compiler)
    assert result.returncode != 0, "cached-pointer mutant survived callback replacement"
    assert "window == replacement[2]" in result.stderr
