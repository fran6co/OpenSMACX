"""Permanent-ready host controls for the installed Base/Veh source owners.

These tests intentionally read product files only. They do not import, invoke,
or fall back to the proposal emitter; before owner integration, extraction
fails on the missing active definitions.
"""
from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.binding_source import parse as parse_bindings
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import SourceRole, read_text

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/IMAGE_PIN").is_file())
SEED_COMMIT = "d174efdf60d3c4f00b816c45e4a5165795e0c79b"
SEED_SOURCE = "src/leaf_recoveries.cpp"
OWNER = {
    0x004E6300: ("src/base.cpp", r"void\s+__cdecl\s+bases_reset\s*\(\s*int\s+region_id\s*,\s*int\s+faction_id\s*,\s*int\s+skip_occupied\s*\)\s*\{"),
    0x004E7E50: ("src/base.cpp", r"int\s+__cdecl\s+sub_4e7e50\s*\(\s*int\s+radius_index\s*\)\s*\{"),
    0x004E7F80: ("src/base.cpp", r"int\s+__cdecl\s+sub_4e7f80\s*\(\s*int\s+radius_index\s*\)\s*\{"),
    0x004D5FF0: ("src/base.cpp", r"void\s+__cdecl\s+sub_4d5ff0\s*\(\s*int\s*\*\s*id_list\s*\)\s*\{"),
    0x00584B50: ("src/veh.cpp", r"int\s+__cdecl\s+abil_cond\s*\(\s*int\s+ability_id\s*,\s*int\s+chassis_id\s*,\s*int\s+weapon_id\s*,\s*int\s+armor_id\s*\)\s*\{"),
    0x00564D90: ("src/veh.cpp", r"int\s+__cdecl\s+can_convoy\s*\(\s*int\s+x\s*,\s*int\s+y\s*\)\s*\{"),
    0x0048CF50: ("src/veh.cpp", r"int\s+__cdecl\s+veh_sprite_click\s*\(\s*int\s+veh_id\s*\)\s*\{"),
    0x0048D010: ("src/veh.cpp", r"int\s+__cdecl\s+veh_sprite_double_click\s*\(\s*int\s+veh_id\s*\)\s*\{"),
}
PRESERVED_FIELDS = (
    "name", "image_spans", "mode", "symbol", "body", "kind", "exclusion",
    "recipe", "byte_exact", "semantic", "levers", "ruled_out",
    "unrecoverable", "deferred",
)


def _mask(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        text,
    )


def _active_definition(source: str, address: int, pattern: str) -> tuple[int, int]:
    masked = _mask(source)
    matches = list(re.finditer(pattern, masked, re.MULTILINE))
    assert len(matches) == 1, f"active C++ definition for 0x{address:08X} is missing/ambiguous"
    start = matches[0].start()
    opening = masked.find("{", matches[0].start(), matches[0].end())
    assert opening >= 0, f"definition signature for 0x{address:08X} has no active body"
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return start, index + 1
    raise AssertionError(f"unterminated actual owner body 0x{address:08X}")


def _extract(relative: str, address: int, pattern: str) -> tuple[str, object]:
    path = ROOT / relative
    source = path.read_text(encoding="utf-8")
    rows = [row for row in read_text(source, path.resolve()) if row.address == address]
    assert len(rows) == 1, f"missing or duplicate active mapping 0x{address:08X} in {relative}"
    row = rows[0]
    assert row.path.name == Path(relative).name
    assert row.mode.value in ("body", "file") and row.region and not row.extract_error, (
        f"0x{address:08X} has no error-free FILE/BODY region: {row.mode} {row.extract_error}"
    )
    start, end = _active_definition(source, address, pattern)
    # Return the original, unmasked source slice; comments and string literals
    # inside the actual body must reach the host compiler.
    return source[start:end], row


def _current_functions() -> tuple[dict[int, str], dict[int, object]]:
    functions, rows = {}, {}
    for address, (relative, pattern) in OWNER.items():
        functions[address], rows[address] = _extract(relative, address, pattern)
    return functions, rows


def _lesson_history(row: object) -> tuple[tuple[object, ...], ...] | None:
    origin = row.origin
    if origin is None:
        return None
    # The reader normalizes seed labels and wraps into structured prose; line
    # numbers/wrapping legitimately change when the whole record moves files.
    return tuple(
        (lesson.kind, lesson.key, lesson.prose)
        for lesson in origin
    )


def _seed_rows() -> dict[int, object]:
    proc = subprocess.run(
        ["git", "show", f"{SEED_COMMIT}:{SEED_SOURCE}"], cwd=ROOT,
        check=True, capture_output=True, text=True, timeout=20,
    )
    rows = {row.address: row for row in read_text(proc.stdout, ROOT / SEED_SOURCE)}
    missing = set(OWNER) - set(rows)
    assert not missing, f"committed source snapshot is missing seed rows: {sorted(missing)}"
    return {address: rows[address] for address in OWNER}


def _compiler_list() -> list[str]:
    return [compiler for compiler in (shutil.which("g++"), shutil.which("clang++"))
            if compiler]


def _compile_run(tmp_path: Path, source: str, compiler: str) -> subprocess.CompletedProcess:
    cpp, exe = tmp_path / "actual_owners.cpp", tmp_path / "actual_owners"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++14", "-Wall", "-Wextra", "-Werror",
         "-Wno-parentheses", "-Wno-shift-op-parentheses", str(cpp), "-o", str(exe)],
        cwd=tmp_path, capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], cwd=tmp_path, capture_output=True,
                          text=True, timeout=10)


HOST = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#define __cdecl
#define ORDER_NONE 0
struct Base {
  short x=0, y=0; unsigned char faction_id_current=0; unsigned char pad=0;
  uint32_t state=0; int nutrient_intake_2=0, mineral_intake_2=0;
  int energy_intake_2=0, unused_intake_2=0; int resource_sqr_radius=0;
  const char *name_string="";
};
struct Map { uint8_t climate=0, contour=0, val2=0, region=0;
  uint8_t visibility=0,val3=0,unk_1=0,unk_2=0; uint32_t bit=0; };
struct { struct { struct { uint8_t enable_mask_=0; } lock_masks_;
  int base_current_count_=0; } fields_; } GameDataStateStorage;
Base bases[8], *Bases=bases, *BaseCurrent=bases;
Map tiles[128];
int MapLongitude=8, MapLongitudeBounds=8, MapLatitudeBounds=8, MapIsFlat=1;
int RadiusOffsetX[16]={0}, RadiusOffsetY[16]={0};
int BaseIDCurrentSelected=0, TileYieldRestricted=9;
Map *map_tiles(){ return tiles; }
static int resets=0;
void base_reset(int i, int) { ++resets; if (i==0) MapLongitude=7; }
static bool move_current_in_crop=false;
int crop_yield(uint8_t,int,int,int,int){
  if (move_current_in_crop) { BaseCurrent=bases+1; move_current_in_crop=false; }
  return 3;
}
int mine_yield(uint8_t,int,int,int,int){ return 4; }
int energy_yield(uint8_t,int,int,int,int){ return 5; }
int _stricmp(const char *a,const char *b){ return strcasecmp(a,b); }
void swap(int *a,int *b){ int t=*a;*a=*b;*b=t; }
struct Veh { int order=0; uint32_t state=0; } Vehs[8];
struct AbilityRec { uint32_t flags=0; } Ability[64];
struct ChassisRec { int missile=0, triad=0; uint8_t speed=1; } Chassis[64];
struct WeaponRec { int offense_rating=1, mode=0; } Weapon[64];
struct ArmorRec { int defense_rating=1; } Armor[64];
int ExpansionEnabled=1, IsMultiplayerNet=0;
int selectable_result=0, jail_result=0;
int veh_selectable(int){return selectable_result;} int veh_jail(int){return jail_result;}
int veh_wake(int){return 0;} void synch_veh(int){}
struct Msg { int calls=0; const char *text=0; void pop(const char *s,int,int,int){++calls;text=s;} } NetMsgPrimary;
struct Daemon { int lock_result=0,calls=0,awaits=0,unlocks=0;
 int lock_veh(int*,int,int,int,int){++calls;return lock_result;}
 void await_synch(){++awaits;} void unlock_veh(){++unlocks;} } NetDaemonNet;
int veh_at(int,int){return 0;} int stack_result=0;
int stack_check(int,int,int,int,int){return stack_result;}
int radius_move(int,int,int,int,int){return -1;}
'''


def test_actual_source_owns_all_eight_functions_and_preserves_committed_history() -> None:
    current, current_rows = _current_functions()
    seed = _seed_rows()
    assert set(current) == set(seed) == set(OWNER)
    for address, new in current_rows.items():
        old = seed[address]
        assert old.source_role is SourceRole.SCAFFOLD
        assert old.source_role_reason
        for field in PRESERVED_FIELDS:
            assert getattr(new, field) == getattr(old, field), (
                f"0x{address:08X}: historical {field} changed"
            )
        assert _lesson_history(new) == _lesson_history(old), (
            f"0x{address:08X}: full historical lesson prose/line wrapping changed"
        )
        assert new.source_role is SourceRole.UNCLASSIFIED, (
            f"0x{address:08X}: active function lacks genuine-owner classification"
        )
        assert new.region and not new.extract_error
        assert new.mode.value in ("body", "file")
        relative, _pattern = OWNER[address]
        active = (ROOT / relative).read_text(encoding="utf-8")
        parse_bindings(active, read_text(active, (ROOT / relative).resolve()))
    assert not (ROOT / SEED_SOURCE).exists(), "retired leaf owner source reappeared"


def test_full_translation_unit_mask_rejects_comment_hidden_definition() -> None:
    address = 0x004E7E50
    relative, pattern = OWNER[address]
    path = ROOT / relative
    source = path.read_text(encoding="utf-8")
    body, _row = _extract(relative, address, pattern)
    assert source.count(body) == 1
    hidden = source.replace(body, "/*\n" + body + "\n*/", 1)
    with pytest.raises(AssertionError, match="active C\\+\\+ definition"):
        _active_definition(hidden, address, pattern)


@pytest.mark.parametrize("compiler", _compiler_list())
def test_actual_source_owner_behaviors_and_compiling_mutant(tmp_path: Path,
                                                            compiler: str) -> None:
    functions, _rows = _current_functions()
    main = r'''
  GameDataStateStorage.fields_.base_current_count_=2;
  bases[0].faction_id_current=1; bases[0].x=0; bases[0].y=0;
  bases[1].faction_id_current=1; bases[1].x=2; bases[1].y=0;
  tiles[0].region=3; tiles[1].region=3;
  tiles[0].bit=2; tiles[0].val2=1; tiles[1].bit=0;
  bases_reset(3,1,1);
  assert(resets==1);
  move_current_in_crop=true;
  assert(sub_4e7e50(0)==12 && bases[0].nutrient_intake_2==0
         && bases[1].nutrient_intake_2==3 && bases[1].mineral_intake_2==4
         && bases[1].energy_intake_2==5 && bases[1].unused_intake_2==0
         && TileYieldRestricted==0);
  BaseCurrent=bases;
  bases[0].nutrient_intake_2=10; bases[0].mineral_intake_2=10;
  bases[0].energy_intake_2=10; bases[0].unused_intake_2=10;
  assert(sub_4e7f80(0)==12 && bases[0].nutrient_intake_2==7
         && bases[0].unused_intake_2==10);
  const char *n0="Zulu", *n1="alpha"; bases[0].name_string=n0;
  bases[1].name_string=n1; int ids[2]={0,1};
  sub_4d5ff0(ids); assert(ids[0]==1 && ids[1]==0);
  Chassis[0].triad=0; Chassis[0].missile=0; Chassis[0].speed=1;
  Weapon[0].offense_rating=1; Weapon[0].mode=0; Armor[0].defense_rating=1;
  Ability[0].flags=1|8; assert(abil_cond(0,0,0,0)==1);
  Ability[0].flags=8; assert(abil_cond(0,0,0,0)==0);
  assert(can_convoy(0,0)==1);
  Chassis[0].missile=1; assert(abil_cond(0,0,0,0)==0);
  assert(veh_sprite_click(0)==0 && NetDaemonNet.calls==1 &&
         NetDaemonNet.awaits==1 && NetDaemonNet.unlocks==1);
  assert(veh_sprite_double_click(0)==1 && NetDaemonNet.calls==2);
  IsMultiplayerNet=1; selectable_result=1;
  assert(veh_sprite_click(0)==1 && NetDaemonNet.calls==2);
  selectable_result=0; jail_result=1;
  assert(veh_sprite_double_click(0)==0 && NetMsgPrimary.calls==1 &&
         std::strcmp(NetMsgPrimary.text,"INJAIL")==0 && NetDaemonNet.calls==2);
  jail_result=0; IsMultiplayerNet=0; NetDaemonNet.lock_result=1;
  assert(veh_sprite_click(0)==0 && NetDaemonNet.calls==3 &&
         NetDaemonNet.awaits==2 && NetDaemonNet.unlocks==2);
  assert(veh_sprite_double_click(0)==0 && NetDaemonNet.calls==4 &&
         NetDaemonNet.awaits==2 && NetDaemonNet.unlocks==2);
'''
    code = HOST + "\n".join(functions.values()) + "\nint main(){\n" + main + "\n}\n"
    passed = _compile_run(tmp_path, code, compiler)
    assert passed.returncode == 0, passed.stdout + passed.stderr

    mutant = dict(functions)
    old = "        base = BaseCurrent;"
    assert mutant[0x004E7E50].count(old) == 1
    mutant[0x004E7E50] = mutant[0x004E7E50].replace(
        old, "base = Bases;", 1
    )
    mutated = HOST + "\n".join(mutant.values()) + "\nint main(){\n" + main + "\n}\n"
    failed = _compile_run(tmp_path, mutated, compiler)
    assert failed.returncode != 0, "the actual-body predicate mutation survived"

    convoy_mutant = dict(functions)
    convoy_predicate = "if (stack_check(veh_id, 9, 3, -1, -1) != 0)"
    assert convoy_mutant[0x00564D90].count(convoy_predicate) == 1
    convoy_mutant[0x00564D90] = convoy_mutant[0x00564D90].replace(
        convoy_predicate,
        "if (stack_check(veh_id, 9, 3, -1, -1) == 0)", 1,
    )
    convoy_changed = HOST + "\n".join(convoy_mutant.values()) + "\nint main(){\n" + main + "\n}\n"
    failed = _compile_run(tmp_path, convoy_changed, compiler)
    assert failed.returncode != 0, "the convoy predicate mutation survived"
