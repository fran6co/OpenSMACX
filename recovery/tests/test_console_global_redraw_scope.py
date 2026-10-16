"""Global redraw qualification at selected Console ownership boundaries.

The pinned call-site checks distinguish the free all-map renderer from a
MapWin receiver call. The editor_load host harness compiles the actual source
body against independent free/member spies; it is a bounded dispatch test, not
a full Console layout, ABI, or game-runtime proof.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]


def uncomment(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def braced(text, start):
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unclosed Console method")


def method_body(source, name):
    source = uncomment(source)
    match, = re.finditer(r"^(?:void|int)\s+Console::" + re.escape(name) +
                         r"\([^;{}]*\)\s*\{", source, re.M)
    return braced(source, match.start())


def source_methods():
    source = (ROOT / "src/console.cpp").read_text()
    return source, {name: method_body(source, name) for name in (
        "editor_clear", "editor_reset_tech", "editor_load", "editor_randomize",
        "editor_clear_random", "new_base", "on_key_click", "editor_edit_veh",
        "sentry", "hold", "editor_veh", "activate", "upgrade")}


def test_selected_console_redraw_sites_are_explicit_globals():
    _, bodies = source_methods()
    map_counts = {
        "editor_clear": 1, "editor_reset_tech": 1, "editor_load": 1,
        "editor_randomize": 1, "editor_clear_random": 2,
    }
    for name, expected in map_counts.items():
        body = bodies[name]
        assert body.count("::draw_map(1)") == expected, name
        assert not re.search(r"(?<![:>\w.])draw_map\s*\(\s*1\s*\)", body), name

    key_click = bodies["on_key_click"]
    assert "set_dirty();\n        ::draw_map(1);\n        WorldClimateWorldWin.redraw();" in key_click
    assert not re.search(r"(?<![:>\w.])draw_map\s*\(\s*1\s*\)", key_click)

    # new_base has more source branches than the two mapped native direct-call
    # sites; assert ownership spelling and absence of bare calls, not a false
    # source-branch/native-call count equality.
    assert bodies["new_base"].count("::draw_map(1)") == 10
    assert not re.search(r"(?<![:>\w.])draw_map\s*\(\s*1\s*\)", bodies["new_base"])

    tile_counts = {
        "editor_edit_veh": 2, "sentry": 1, "hold": 1,
        "editor_veh": 1, "activate": 1, "upgrade": 1,
    }
    for name, expected in tile_counts.items():
        body = bodies[name]
        assert body.count("::draw_tile(") == expected, name
        assert not re.search(r"(?<![:>\w.])draw_tile\s*\(", body), name

    # This unrelated free helper is already a global call and remains outside
    # the Console method edits.
    helper = re.search(r"void update_then_redraw_vehicle_editor_tile\([^)]*\)\s*\{([^}]+)\}",
                       (ROOT / "src/console.cpp").read_text())
    assert helper and "draw_tile(vehicle.x, vehicle.y, 1);" in helper.group(1)
    assert "::draw_tile(vehicle.x, vehicle.y, 1);" not in helper.group(1)


def test_original_annotated_spans_call_the_global_targets():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE",
                                    ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    # The source reader is authoritative for complete annotated extents,
    # including multi-span functions; this list records only the pinned
    # call-site PCs and targets under review.
    bodies = {
        "editor_clear": (0x004E1290, 0x0046B190, (0x004E12E9,)),
        "editor_reset_tech": (0x004DBC40, 0x0046B190, (0x004DBD0D,)),
        "editor_load": (0x004E0A00, 0x0046B190, (0x004E0A33,)),
        "editor_randomize": (0x004E1190, 0x0046B190, (0x004E11ED,)),
        "editor_clear_random": (0x004E1200, 0x0046B190,
                                (0x004E1227, 0x004E1281)),
        "new_base": (0x004CF7C0, 0x0046B190,
                     (0x004CFF75, 0x004D02D4)),
        "on_key_click": (0x005178C0, 0x0046B190, (0x00518637,)),
        "editor_edit_veh": (0x004DDA50, 0x0046AF40,
                            (0x004DDD7F, 0x004DE223, 0x004DE662)),
        "sentry": (0x004D2220, 0x0046AF40, (0x004D24AB,)),
        "hold": (0x004D24C0, 0x0046AF40, (0x004D260F,)),
        "editor_veh": (0x004DED00, 0x0046AF40, (0x004DF177,)),
        "activate": (0x004D2F00, 0x0046AF40, (0x004D3385,)),
        "upgrade": (0x004D06C0, 0x0046AF40,
                    (0x004D18D0, 0x004D1A9C)),
    }
    mapped = {row.address: row for row in read_file(ROOT / "src/console.cpp")}
    for name, (start, target, call_sites) in bodies.items():
        row = mapped[start]
        spans = row.image_spans
        assert spans, name
        by_address = {}
        for span_start, span_end in spans:
            for instruction in decoder.disasm(image.read(Span(span_start, span_end)), span_start):
                by_address[instruction.address] = instruction
        for pc in call_sites:
            assert any(span_start <= pc < span_end for span_start, span_end in spans), (name, hex(pc))
            instruction = by_address[pc]
            assert instruction.mnemonic == "call", (name, hex(pc), instruction.mnemonic)
            assert instruction.op_str == hex(target), (name, hex(pc), instruction.op_str)


def editor_load_body(source_mutation=None):
    source = (ROOT / "src/console.cpp").read_text()
    body = method_body(source, "editor_load")
    if source_mutation:
        assert body.count("::draw_map(1);") == 1
        body = body.replace("::draw_map(1);", "draw_map(1);", 1)
    return body


HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <string>
void require_at(bool value,int line){if(!value){fprintf(stderr,"assertion line %d\n",line);std::_Exit(99);}}
#define require(x) require_at((x),__LINE__)
struct ConsoleState { int cursor_active_,selected_vehicle_; } ConsoleGlobal;
struct GameFields { int game_state_,vehicle_current_count_,base_current_count_; };
struct GameState { GameFields fields_; } GameDataStateStorage;
std::string events;
struct InfoSpy { int phase,calls; void set_redraw_phase(int v){phase=v;events += 'P';}
 void on_redraw(){++calls;events += 'I';} } ConsoleInfoWin;
int member_draws,global_draws,member_argument,global_argument;
int loads,global_load_result;
struct MapWin { void draw_map(int v){++member_draws;member_argument=v;events += 'M';} };
void draw_map(int v){++global_draws;global_argument=v;events += 'G';}
int load_map(){++loads;events += 'L';return global_load_result;}
struct Console : MapWin { void editor_load(); };
EDITOR_LOAD_BODY
void reset(){ConsoleGlobal={1,55};GameDataStateStorage.fields_={0x20,17,9};
 ConsoleInfoWin={3,0};member_draws=global_draws=member_argument=global_argument=loads=0;global_load_result=0;events.clear();}
int main(int argc,char**argv){
 const int fail=argc>1?std::atoi(argv[1]):0;reset();Console c;global_load_result=fail;
 c.editor_load();
 if(fail){require(events=="L"&&loads==1);require(member_draws==0&&global_draws==0);
  require(GameDataStateStorage.fields_.game_state_==0x20);
  require(GameDataStateStorage.fields_.vehicle_current_count_==17&&GameDataStateStorage.fields_.base_current_count_==9);
  require(ConsoleGlobal.cursor_active_==1&&ConsoleGlobal.selected_vehicle_==55);
  require(ConsoleInfoWin.phase==3&&ConsoleInfoWin.calls==0);}
 else {require(events=="LGPI"&&loads==1);require(member_draws==0&&global_draws==1);
  require(global_argument==1);require(GameDataStateStorage.fields_.game_state_==0x24);
  require(GameDataStateStorage.fields_.vehicle_current_count_==0&&GameDataStateStorage.fields_.base_current_count_==0);
  require(ConsoleGlobal.cursor_active_==0&&ConsoleGlobal.selected_vehicle_==-1);
  require(ConsoleInfoWin.phase==7&&ConsoleInfoWin.calls==1);}
 return 0;
}
'''


def run_editor_load(tmp_path, mutate=False, fail=False):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    text = HARNESS.replace("EDITOR_LOAD_BODY", editor_load_body(mutate))
    source, exe = tmp_path / "console_redraw.cpp", tmp_path / "console_redraw"
    source.write_text(text)
    compiled = subprocess.run([compiler, "-std=c++17", "-O1", str(source), "-o", str(exe)],
                              text=True, capture_output=True, timeout=45)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(exe), "1" if fail else "0"], text=True,
                          capture_output=True, timeout=10)


@pytest.mark.parametrize("fail", [False, True], ids=["load-success", "load-failure"])
def test_actual_editor_load_dispatch_and_state(tmp_path, fail):
    result = run_editor_load(tmp_path, fail=fail)
    assert result.returncode == 0, result.stdout + result.stderr


def test_actual_editor_load_rejects_removed_global_qualifier(tmp_path):
    result = run_editor_load(tmp_path, mutate=True)
    assert result.returncode == 99 and "assertion line" in result.stderr, result.stdout + result.stderr
