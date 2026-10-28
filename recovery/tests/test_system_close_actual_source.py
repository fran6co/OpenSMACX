"""Actual system_close ownership effects, with the pin as the ABI evidence.

The host doubles exercise typed destruction and pointer-clear ordering only;
they do not establish native layout, native vtable emission, or game reachability.
"""

from pathlib import Path
import os
import shutil
import subprocess

import pytest

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin
from test_game_logic_seed_batch_behavior import _definition


ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    "/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe"))
ADDRESS = 0x0058F250
SPAN = (0x0058F250, 0x0058F2E6)


def actual_body(damage=None):
    path = ROOT / "src/game.cpp"
    row, = (record for record in read_file(path) if record.address == ADDRESS)
    assert row.image_spans == (SPAN,)
    body = _definition(row.region, "void __cdecl system_close()")
    if damage is not None:
        old, new = damage
        assert body.count(old) == 1
        body = body.replace(old, new, 1)
    return body


HARNESS = r"""
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <string>
#include <vector>

#define __cdecl

static std::vector<std::string> events;
static void note(const std::string &value) { events.push_back(value); }
static void require(bool condition) { if (!condition) std::exit(91); }

struct Win;
struct GraphicWin;
struct MapWin;
struct Console;
static GraphicWin *OpeningWindow = 0;
static Win *BasePopDefaultField322C = 0;
static Win *expected_parent = 0;
static GraphicWin *expected_opening = 0;
static void *expected_opening_allocation = 0;
static MapWin *expected_map_views[8] = {};
static void *expected_map_allocations[8] = {};
static int opening_derived_destructors, opening_base_destructors;
static int opening_deallocations, map_destructors[8], map_deallocations[8];
static int show_calls;

struct Win { virtual ~Win() {} };
struct Buffer { virtual ~Buffer() {} };
struct GraphicWin : Win, Buffer {
    virtual ~GraphicWin() {
        if (this == expected_opening) {
            require(OpeningWindow == this);
            require(BasePopDefaultField322C == expected_parent);
            ++opening_base_destructors;
            note("opening.base-dtor");
        }
    }
    virtual void show(int value) {
        require(value == 1);
        ++show_calls;
        note("show");
    }
};
struct MapWin : virtual GraphicWin { virtual ~MapWin() {} };
typedef MapWin *MapWinSlots[8];
static MapWinSlots MapWinTable = {};


struct Opening : GraphicWin {
    ~Opening() {
        require(OpeningWindow == static_cast<GraphicWin *>(this));
        require(BasePopDefaultField322C == expected_parent);
        ++opening_derived_destructors;
        note("opening.derived-dtor");
    }
    static void *operator new(std::size_t size) {
        return ::operator new(size);
    }
    static void operator delete(void *allocation) noexcept {
        require(allocation == expected_opening_allocation);
        require(OpeningWindow == expected_opening);
        require(BasePopDefaultField322C == expected_parent);
        ++opening_deallocations;
        note("opening.free");
        ::operator delete(allocation);
    }
};

struct TrackedMap : MapWin {
    explicit TrackedMap(int slot) : slot_(slot) {}
    ~TrackedMap() {
        require(MapWinTable[slot_] == static_cast<MapWin *>(this));
        require(BasePopDefaultField322C == expected_parent);
        ++map_destructors[slot_];
        note(std::string("map") + char('0' + slot_) + ".dtor");
    }
    static void *operator new(std::size_t size) {
        return ::operator new(size);
    }
    static void operator delete(void *allocation) noexcept {
        for (int slot = 1; slot < 8; ++slot) {
            if (allocation == expected_map_allocations[slot]) {
                require(MapWinTable[slot] == expected_map_views[slot]);
                ++map_deallocations[slot];
                note(std::string("map") + char('0' + slot) + ".free");
                ::operator delete(allocation);
                return;
            }
        }
        require(false);
    }
    int slot_;
};

struct Console : MapWin { void close() { note("console.close"); } };
static Console ConsoleGlobal;
struct CloseStub {
    const char *name;
    void close() { note(name); }
};
static CloseStub TutWinDesWindow = {"tut.close"};
static CloseStub WinBubbleWindow = {"bubble.close"};
struct PathsStub { void shutdown() { note("paths.shutdown"); } };
static PathsStub Paths;
static void release_sound() { note("release_sound"); }
static void map_shutdown() { note("map.shutdown"); }

// ACTUAL_BODY

static void reset_observations() {
    events.clear();
    show_calls = 0;
}

int main() {
    expected_parent = static_cast<Win *>(&ConsoleGlobal);
    MapWinTable[0] = static_cast<MapWin *>(&ConsoleGlobal);
    MapWin *const primary = MapWinTable[0];

    Opening *const opening = new Opening;
    OpeningWindow = static_cast<GraphicWin *>(opening);
    expected_opening = OpeningWindow;
    expected_opening_allocation = opening;

    const int occupied[] = {1, 2, 4, 5, 6, 7};
    for (unsigned i = 0; i < sizeof(occupied) / sizeof(occupied[0]); ++i) {
        const int slot = occupied[i];
        TrackedMap *const map = new TrackedMap(slot);
        MapWinTable[slot] = static_cast<MapWin *>(map);
        expected_map_views[slot] = MapWinTable[slot];
        expected_map_allocations[slot] = map;
    }
    // Slot 3 is a null hole; slot 0 is the live Console and must be skipped.

    system_close();
    const std::vector<std::string> expected = {
        "release_sound", "opening.derived-dtor", "opening.base-dtor", "opening.free",
        "tut.close", "bubble.close", "console.close", "paths.shutdown", "map.shutdown",
        "map1.dtor", "map1.free", "map2.dtor", "map2.free",
        "map4.dtor", "map4.free", "map5.dtor", "map5.free",
        "map6.dtor", "map6.free", "map7.dtor", "map7.free"
    };
    require(events == expected);
    require(OpeningWindow == 0);
    require(MapWinTable[0] == primary);
    for (int slot = 1; slot < 8; ++slot) require(MapWinTable[slot] == 0);
    require(show_calls == 0);
    require(opening_derived_destructors == 1 && opening_base_destructors == 1);
    require(opening_deallocations == 1);
    for (int slot = 1; slot < 8; ++slot) {
        const bool occupied_slot = slot != 3;
        require(map_destructors[slot] == (occupied_slot ? 1 : 0));
        require(map_deallocations[slot] == (occupied_slot ? 1 : 0));
    }

    // A second shutdown has null opening and secondary slots; it still runs
    // the ordered global cleanup, without a second destruction or show call.
    reset_observations();
    system_close();
    const std::vector<std::string> null_expected = {
        "release_sound", "tut.close", "bubble.close", "console.close",
        "paths.shutdown", "map.shutdown"
    };
    require(events == null_expected);
    require(OpeningWindow == 0 && MapWinTable[0] == primary);
    require(show_calls == 0 && opening_deallocations == 1);
    return 0;
}
"""


def compile_and_run(tmp_path, compiler, body):
    source = tmp_path / "system_close.cpp"
    binary = tmp_path / "system_close"
    source.write_text(HARNESS.replace("// ACTUAL_BODY", body))
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("name", ("g++", "clang++"))
def test_actual_system_close_destroys_owned_windows_in_pinned_order(tmp_path, name):
    compiler = shutil.which(name)
    if compiler is None:
        pytest.skip(name + " unavailable")
    result = compile_and_run(tmp_path, compiler, actual_body())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("name,old,new", (
    ("opening_show", "delete opening;", "opening->show(1);"),
    ("map_show", "delete MapWinTable[slot];", "MapWinTable[slot]->show(1);"),
))
def test_compiled_show_instead_of_delete_damages_lifecycle(tmp_path, name, old, new):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    body = actual_body((old, new))
    result = compile_and_run(tmp_path, compiler, body)
    assert result.returncode == 91, name + " should fail the destructor/slot oracle"


def test_pinned_system_close_dispatch_and_map_slot_range():
    if not IMAGE.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(IMAGE.read_bytes(), image_pin(ROOT))
    decoded = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(*SPAN)), SPAN[0]))
    by_address = {instruction.address: (instruction.mnemonic, instruction.op_str)
                  for instruction in decoded}
    assert by_address[0x0058F27D] == ("push", "1")
    assert by_address[0x0058F27F] == ("call", "dword ptr [edx]")
    assert by_address[0x0058F281] == ("mov", "dword ptr [0x945824], 0")
    assert by_address[0x0058F2B9] == ("mov", "esi, 0x7d3c40")
    assert by_address[0x0058F2C6] == ("push", "1")
    assert by_address[0x0058F2D1] == ("call", "dword ptr [eax]")
    assert by_address[0x0058F2D3] == ("mov", "dword ptr [esi], 0")
    assert by_address[0x0058F2DC] == ("cmp", "esi, 0x7d3c5c")
    assert by_address[0x0058F2E2] == ("jl", "0x58f2be")
    # The source-owned eight-slot array begins at 0x7D3C3C; iteration starts
    # at +4 and ends before +0x20, so only secondary slots 1 through 7 are used.
    assert 0x7D3C40 == 0x7D3C3C + 4
    assert 0x7D3C5C == 0x7D3C3C + 8 * 4
