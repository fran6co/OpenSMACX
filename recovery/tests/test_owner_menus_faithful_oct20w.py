"""Check Menu::on_left_up's original live PullDown-cell reload.

The C++ fixture extracts the actual product body. Its injected getter hook is
deliberately synthetic: it makes the post-get_selected cell reload observable;
the pinned instructions, not the hook, establish that the original reloads the
cell at 0x005FBB71.
"""
import hashlib
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SOURCE_PATH = ROOT / "src/menu.cpp"
SIGNATURE = "void Menu::on_left_up(int xCoord, int yCoord)"


def definition(source: str) -> str:
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unclosed actual Menu::on_left_up definition")


def harness_source(*, cached_pointer: bool = False) -> str:
    records = read_file(SOURCE_PATH)
    record, = [row for row in records if row.address == 0x005FBAC0]
    assert Path(record.path).resolve() == SOURCE_PATH.resolve()
    assert record.region and SIGNATURE in record.region
    body = definition(SOURCE_PATH.read_text())
    if cached_pointer:
        body = body.replace(
            "PullDown **pull_down_slot = &entries_[highlighted].pull_down;",
            "PullDown *pull_down = entries_[highlighted].pull_down;",
        )
        body = body.replace("(*pull_down_slot)->get_selected()",
                            "pull_down->get_selected()")
        body = body.replace("(*pull_down_slot)->items_[selected].id",
                            "pull_down->items_[selected].id")
    assert body.count("(*pull_down_slot)->get_selected()") == (0 if cached_pointer else 1)
    assert body.count("(*pull_down_slot)->items_[selected].id") == (0 if cached_pointer else 1)

    return r'''#include <cassert>

typedef void (*MenuProc)(int);
class Menu;
struct Sound {
    int playing;
    int plays;
    Sound() : playing(0), plays(0) {}
    int is_playing() const { return playing; }
    void play() { ++plays; }
};
struct Win {
    Sound *sound;
    Win() : sound(0) {}
    Sound *event_window() { return sound; }
};
struct PullDown {
    struct Item { int id; };
    Item items_[64];
    int selected_;
    Menu *owner_;
    PullDown() : selected_(-1), owner_(0) {}
    int get_selected();
};
struct Entry { PullDown *pull_down; Entry() : pull_down(0) {} };
struct Spot {
    int check(int, int, int *, int) { return -1; }
};
static void (*after_get_selected)(Menu *) = 0;
static int callback_id = -999;
static void record_id(int id) { callback_id = id; }
class Menu {
public:
    Entry entries_[15];
    int field_A24_;
    unsigned int field_A28_;
    MenuProc proc_;
    Win *win_parent_;
    Spot spot_;
    int draws_;
    int hides_;
    Menu() : field_A24_(0), field_A28_(4), proc_(&record_id),
             win_parent_(0), draws_(0), hides_(0) {}
    void hide_menus() { ++hides_; }
    void do_all_draws() { ++draws_; }
    void hide() { ++hides_; }
    void on_left_up(int xCoord, int yCoord);
};
int PullDown::get_selected() {
    const int selected = selected_;
    if (after_get_selected != 0) after_get_selected(owner_);
    return selected;
}
static PullDown *replacement;
static void replace_entry_cell(Menu *menu) {
    menu->entries_[0].pull_down = replacement;
}
''' + body + r'''
int main() {
    Menu menu;
    PullDown original, replacement_pull_down;
    Win parent;
    Sound sound;
    parent.sound = &sound;
    menu.win_parent_ = &parent;
    original.owner_ = &menu;
    original.selected_ = 0;
    original.items_[0].id = 11;
    replacement_pull_down.items_[0].id = 22;
    menu.entries_[0].pull_down = &original;
    replacement = &replacement_pull_down;
    after_get_selected = &replace_entry_cell;
    menu.on_left_up(3, 7);
    assert(callback_id == 22);
    assert(menu.draws_ == 1 && menu.hides_ == 2);
    assert(sound.plays == 1);
    return 0;
}
'''


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_menu_release_reloads_the_pull_down_cell(compiler, tmp_path):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "menu-release.cpp"
    binary = tmp_path / "menu-release"
    source.write_text(harness_source())
    built = subprocess.run(
        [executable, "-std=c++03", "-O1", "-Wall", "-Wextra", "-Werror",
         "-pedantic", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True,
                            timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr


def test_cached_pull_down_mutant_is_rejected(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no host C++ compiler")
    source = tmp_path / "menu-release-cached.cpp"
    binary = tmp_path / "menu-release-cached"
    source.write_text(harness_source(cached_pointer=True))
    built = subprocess.run(
        [compiler, "-std=c++03", "-O0", "-Wall", "-Wextra", "-Werror",
         "-pedantic", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True,
                            timeout=30)
    assert result.returncode != 0, "cached PullDown* mutant survived"
    assert "callback_id == 22" in result.stderr


def test_pinned_menu_release_has_post_get_selected_cell_reload():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"
    ))
    assert image_path.is_absolute() and image_path.is_file()
    image_bytes = image_path.read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == IMAGE_SHA256
    image = PeImage(image_bytes, image_pin(ROOT))
    span = Span(0x005FBAC0, 0x005FBC1A)
    dis = Cs(CS_ARCH_X86, CS_MODE_32)
    rows = [(row.address, row.mnemonic, row.op_str, row.size)
            for row in dis.disasm(image.read(span), span.start)]
    cursor = span.start
    for address, _, _, size in rows:
        assert address == cursor
        cursor += size
    assert cursor == span.end
    by_address = {address: (mnemonic, operands)
                  for address, mnemonic, operands, _ in rows}
    assert by_address[0x005FBB54] == (
        "lea", "edi, [esi + edx*4 + 0xa48]"
    )
    assert by_address[0x005FBB5B] == ("call", "0x5f9f40")
    assert by_address[0x005FBB71] == ("mov", "ecx, dword ptr [edi]")
