"""Actual startup allocation fragment; host EH is not native ABI/runtime proof."""
from dataclasses import fields
from pathlib import Path
import re
import subprocess

import pytest
from match_recovery.source import read_text, write
from test_net_hotseat_owner_actual_source_oct6 import extract_definition
from test_console_typed_owner_members_oct10 import run_control

ROOT = Path(__file__).resolve().parents[2]
BASE = "5fab410a265e5cb34ef31c1e44da946cb77bb652"


def test_whole_source_changes_only_the_allocation_walk_and_preserves_identity():
    path = ROOT / "src/game.cpp"
    current = path.read_text()
    old = subprocess.check_output(["git", "show", BASE + ":src/game.cpp"], cwd=ROOT).decode()
    signature = "int __cdecl system_init()"
    previous_body = extract_definition(old, signature)
    body = extract_definition(current, signature)
    start = body.index("    // Construct only slots")
    end = body.index("    SystemInterfaceFlags &=", start)
    old_start = previous_body.index("    // Slots 1..7.")
    old_end = previous_body.index("    SystemInterfaceFlags &=", old_start)
    restored = body[:start] + previous_body[old_start:old_end] + body[end:]
    assert restored == previous_body
    assert current.replace(body, previous_body, 1) == old
    rows = read_text(current, path)
    assert write(current, rows) == current
    previous = {r.address: r for r in read_text(old, path)}
    assert {r.address for r in rows} == set(previous)
    for row in rows:
        baseline = previous[row.address]
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(baseline, field.name)
        def lessons(r):
            return [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines)) for x in r.origin]
        assert lessons(row) == lessons(baseline)
    assert body.index("MapWinTable[slot] = new MapWin;") < body.index("MapWinTable[0] = &ConsoleGlobal;")


def control():
    header = (ROOT / "src/mapwin.h").read_text()
    array = re.search(r"(?m)^typedef MapWin \*MapWinSlots\[8\];", header)[0]
    bound = re.search(r"(?m)^static const size_t MapWinTableSlots = 8;", header)[0]
    body = extract_definition((ROOT / "src/game.cpp").read_text(), "int __cdecl system_init()")
    start = body.index("    for (int slot = 1;")
    end = body.index("    SystemInterfaceFlags &=", start)
    loop = body[start:end]
    assert "reinterpret_cast" not in loop and "(int)" not in loop
    return r'''
#include <cassert>
#include <cstddef>
#include <new>
struct MapWin;
ARRAY
BOUND
MapWinSlots MapWinTable;
static int attempts,allocated,released,destructed,fail_at;
static MapWin *sentinel;
struct ConstructionFailure {};
struct MapWin {
    int id;
    explicit MapWin(int n):id(n){}
    MapWin():id(++attempts){
        assert(MapWinTable[id]==sentinel); // Publication follows the constructor.
        assert(MapWinTable[0]==sentinel);
        for(int earlier=1;earlier<id;++earlier)assert(MapWinTable[earlier]->id==earlier);
        if(id==fail_at)throw ConstructionFailure();
    }
    ~MapWin(){++destructed;}
    static void *operator new(size_t n){++allocated;return ::operator new(n);}
    static void operator delete(void *p){++released;::operator delete(p);}
};
static void allocate_windows(){
LOOP
}
int main(){
    MapWin old_window(99);sentinel=&old_window;
    for(int failing: {0,1,4,7}){
        for(auto &slot:MapWinTable)slot=sentinel;
        attempts=allocated=released=destructed=0;fail_at=failing;
        bool caught=false;
        try{allocate_windows();}catch(const ConstructionFailure&){caught=true;}
        const int completed=failing?failing-1:7;
        assert(caught==(failing!=0));
        assert(attempts==(failing?failing:7) && allocated==attempts);
        assert(released==(failing?1:0) && destructed==0);
        assert(MapWinTable[0]==sentinel);
        for(int i=1;i<8;++i){
            if(i<=completed)assert(MapWinTable[i]!=sentinel && MapWinTable[i]->id==i);
            else assert(MapWinTable[i]==sentinel);
        }
        for(int i=1;i<=completed;++i)delete MapWinTable[i];
        assert(released==allocated && destructed==completed);
    }
}
'''.replace("#include <new>", "#include <new>\n#include <initializer_list>").replace(
        "ARRAY", array).replace("BOUND", bound).replace("LOOP", loop)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_allocation_publication_and_constructor_unwind(tmp_path, compiler):
    program = control()
    run_control(tmp_path, compiler, program, None)
    for damage in (
        ("int slot = 1;", "int slot = 2;"),
        ("slot < static_cast<int>(MapWinTableSlots)", "slot < static_cast<int>(MapWinTableSlots) - 1"),
        ("MapWinTable[slot] = new MapWin;", "MapWinTable[0] = new MapWin;"),
        ("MapWinTable[slot] = new MapWin;", "MapWinTable[slot] = nullptr;"),
    ):
        run_control(tmp_path, compiler, program, damage)
