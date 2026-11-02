"""Permanent-ready host controls for the installed Base/Veh source owners.

These tests intentionally read product files only. They do not import, invoke,
or fall back to the proposal emitter; before owner integration, extraction
fails on the missing active definitions.
"""
from __future__ import annotations

import re
import os
import struct
import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.associated_source import read_associated
from match_recovery.associated_binary import _ENGINE, _memory_operand, _prove_dispatch_extent
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
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
  uint32_t state=0; int intake_2[4]={}; int resource_sqr_radius=0;
  const char *name_string="";
};
struct Map { uint8_t climate=0, contour=0, val2=0, region=0;
  uint8_t visibility=0,val3=0,unk_1=0,unk_2=0; uint32_t bit=0; };
struct { struct {
  union { uint32_t enable_mask_;
    struct { uint8_t human_mask_, alive_mask_, unassigned_[2]; } bytes_;
  } lock_masks_;
  int base_current_count_=0; } fields_; } GameDataStateStorage;
Base bases[8], *Bases=bases, *BaseCurrent=bases;
Map tiles[128];
int MapLongitude=8, MapLongitudeBounds=8, MapLatitudeBounds=8, MapIsFlat=1;
int RadiusOffsetX[16]={0}, RadiusOffsetY[16]={0};
int BaseIDCurrentSelected=0, TileYieldRestricted=9;
Map *map_tiles(){ return tiles; }
static int resets=0;
static int reset_new_count=0, reset_visit_count=0, reset_visits[8]={};
void base_reset(int i, int) {
  ++resets; reset_visits[reset_visit_count++]=i;
  if (i==0) {
    MapLongitude=7;
    if (reset_new_count!=0)
      GameDataStateStorage.fields_.base_current_count_=reset_new_count;
  }
}
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



# Closed, pinned protocol migrations. The seed's first body end omitted RET;
# neither seed span recorded the trailing direct-target table. Preserve the
# historical body bytes, correct that one endpoint, and independently require
# the complete owned data instead of treating it as code or dropping it.
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
HISTORY_EXTENTS = {
    0x004E7E50: (0x004E7F63, 0x004E7F64, 0x004E7F74, 0x004E7EF7,
                 (0x004E7EFE, 0x004E7F0E, 0x004E7F1E, 0x004E7F2E)),
    0x004E7F80: (0x004E8094, 0x004E8094, 0x004E80A4, 0x004E8027,
                 (0x004E802E, 0x004E803E, 0x004E804E, 0x004E805E)),
}
EMITTED_SYMBOL_MIGRATIONS = {
    0x004E7E50: "_sub_4e7e50",
    0x004E7F80: "_sub_4e7f80",
    0x004D5FF0: "_sub_4d5ff0",
}


def _pinned_history_image():
    return PeImage(Path(os.environ["OPENSMACX_IMAGE"]).read_bytes(), PIN)


def _assert_history_extent(address, current, seed, associated, image):
    owned = [item for item in associated if item.owner.address == address]
    if address not in HISTORY_EXTENTS:
        assert current.image_spans == seed.image_spans
        assert not owned, "unexpected associated extent added to a historical owner"
        return
    seed_end, code_end, table_end, evidence, targets = HISTORY_EXTENTS[address]
    assert seed.image_spans == ((address, seed_end),)
    assert current.image_spans == ((address, code_end),)
    assert len(owned) == 1, "missing, duplicate or wrong-owner associated binding"
    binding, = owned
    assert (binding.owner.image_sha256, binding.owner.address_space,
            binding.owner.address, binding.unit, binding.kind,
            binding.span.start, binding.span.end,
            binding.evidence.image_sha256, binding.evidence.address_space,
            binding.evidence.address) == (
        PIN, "pe:va", address, OWNER[address][0], "switch_targets",
        code_end, table_end, PIN, "pe:va", evidence)
    # These adjacent half-open ranges exhaust the independently proved extent;
    # no historical byte disappears and no table byte is counted as code.
    assert address < seed_end <= code_end == binding.span.start < table_end
    assert code_end - seed_end == (1 if address == 0x004E7E50 else 0)
    body = Span(address, code_end)
    instructions = list(_ENGINE.disasm(image.read(body), address))
    assert sum(item.size for item in instructions) == code_end - address
    assert instructions[-1].mnemonic == "ret"
    assert instructions[-1].address + instructions[-1].size == code_end
    if seed_end != code_end:
        assert (instructions[-1].address, instructions[-1].size) == (seed_end, 1)
    assert image.absolute_sites(binding.span) == frozenset((0, 4, 8, 12))
    cells = struct.unpack("<4I", image.read(binding.span))
    assert cells == targets
    starts = {item.address for item in instructions}
    assert all(address <= target < code_end and target in starts for target in cells)
    dispatch, = [item for item in instructions if item.address == evidence]
    operand = _memory_operand(dispatch, binding.kind)
    assert operand.mem.disp == binding.span.start
    field = Span(dispatch.address + dispatch.disp_offset,
                 dispatch.address + dispatch.disp_offset + dispatch.disp_size)
    assert dispatch.disp_size == 4 and image.absolute_sites(field) == frozenset((0,))
    witness = _prove_dispatch_extent(
        instructions, body, image, binding, dispatch, operand, None, None, None,
        indirect_targets=frozenset(cells))
    assert witness["mode"] == "direct-index"
    assert witness["target_count"] == 4 and witness["selector_count"] == 0
    assert witness["selector_offset"] is None and witness["index_register"] == "ebx"
    assert witness["guard"]["compare"] == evidence - 8
    assert witness["guard"]["branch"] == evidence - 2
    assert witness["guard"]["maximum_index"] == 3


def _assert_emitted_symbol_history(address, current, seed):
    if address not in EMITTED_SYMBOL_MIGRATIONS:
        assert current.symbol == seed.symbol
        return
    # Blank seed selection became the ordinary extern-C cdecl symbol. Keep
    # original identity/name unchanged and admit only these observed bindings.
    assert seed.symbol == "" and current.symbol == EMITTED_SYMBOL_MIGRATIONS[address]
    assert current.name == seed.name == current.symbol[1:]
    source = (ROOT / OWNER[address][0]).read_text()
    start, _ = _active_definition(source, address, OWNER[address][1])
    line_start = source.rfind(chr(10), 0, start) + 1
    assert source[line_start:start].strip() == 'extern "C"'



def test_actual_source_owns_all_eight_functions_and_preserves_committed_history() -> None:
    current, current_rows = _current_functions()
    seed = _seed_rows()
    associated = read_associated(ROOT, PIN)
    image = _pinned_history_image()
    assert set(current) == set(seed) == set(OWNER)
    for address, new in current_rows.items():
        old = seed[address]
        assert old.source_role is SourceRole.SCAFFOLD
        assert old.source_role_reason
        for field in PRESERVED_FIELDS:
            if field == "image_spans":
                _assert_history_extent(address, new, old, associated, image)
                continue
            if field == "symbol":
                _assert_emitted_symbol_history(address, new, old)
                continue
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


def test_bases_reset_original_reads_live_count_after_callback() -> None:
    image = _pinned_history_image()
    instructions = {item.address: item for item in _ENGINE.disasm(
        image.read(Span(0x004E6300, 0x004E63FC)), 0x004E6300)}
    assert instructions[0x004E63CD].mnemonic == "call"
    assert instructions[0x004E63CD].op_str == "0x4e5fe0"
    assert instructions[0x004E63E1].mnemonic == "mov"
    assert instructions[0x004E63E1].op_str == "eax, dword ptr [0x9a64cc]"
    assert instructions[0x004E63ED].op_str == "ebx, eax"
    assert instructions[0x004E63EF].mnemonic == "jl"
    assert instructions[0x004E63EF].op_str == "0x4e6342"


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
  reset_new_count=1; reset_visit_count=0; resets=0;
  GameDataStateStorage.fields_.base_current_count_=2;
  bases_reset(-1,1,0);
  assert(resets==1 && reset_visit_count==1 && reset_visits[0]==0);
  reset_new_count=3; reset_visit_count=0; resets=0;
  GameDataStateStorage.fields_.base_current_count_=1;
  bases[2].faction_id_current=1; bases[2].x=4; bases[2].y=0;
  bases_reset(-1,1,0);
  assert(resets==3 && reset_visit_count==3 && reset_visits[0]==0
         && reset_visits[1]==1 && reset_visits[2]==2);
  reset_new_count=0; reset_visit_count=0; resets=1;
  GameDataStateStorage.fields_.base_current_count_=2; MapLongitude=8;
  move_current_in_crop=true;
  assert(sub_4e7e50(0)==12 && bases[0].intake_2[0]==0
         && bases[1].intake_2[0]==3 && bases[1].intake_2[1]==4
         && bases[1].intake_2[2]==5 && bases[1].intake_2[3]==0
         && TileYieldRestricted==0);
  BaseCurrent=bases;
  bases[0].intake_2[0]=10; bases[0].intake_2[1]=10;
  bases[0].intake_2[2]=10; bases[0].intake_2[3]=10;
  assert(sub_4e7f80(0)==12 && bases[0].intake_2[0]==7
         && bases[0].intake_2[3]==10);
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
    from test_mapwin_cache import braced
    base_source = (ROOT / "src/base.cpp").read_text()
    arithmetic = "\n".join(braced(base_source, head) for head in (
        "int base_yield_signed_bits(uint32_t value)", "int base_yield_sub(int left, int right)"))
    map_header = (ROOT / "src/map.h").read_text()
    arithmetic += "\n" + braced(map_header, "enum TerrainBit") + ";\n"
    arithmetic += "\n".join(braced(map_header, head) for head in (
        "inline Map *__cdecl map_loc(int x, int y)",
        "inline int __cdecl veh_who(int x, int y)"))
    code = "#include <stdint.h>\n" + HOST + arithmetic + "\n" + "\n".join(functions.values()) + "\nint main(){\n" + main + "\n}\n"
    passed = _compile_run(tmp_path, code, compiler)
    assert passed.returncode == 0, passed.stdout + passed.stderr

    cached_count_mutant = dict(functions)
    live_bound = "} while (i < GameDataStateStorage.fields_.base_current_count_);"
    assert cached_count_mutant[0x004E6300].count(live_bound) == 1
    cached_count_mutant[0x004E6300] = cached_count_mutant[0x004E6300].replace(
        live_bound, "} while (i < count);", 1
    )
    stale_bound = "#include <stdint.h>\n" + HOST + arithmetic + "\n" + "\n".join(cached_count_mutant.values()) + "\nint main(){\n" + main + "\n}\n"
    failed = _compile_run(tmp_path, stale_bound, compiler)
    assert failed.returncode != 0, "the cached count mutation survived"
    assert "resets==1 && reset_visit_count==1" in failed.stderr

    mutant = dict(functions)
    old = "        base = BaseCurrent;"
    assert mutant[0x004E7E50].count(old) == 1
    mutant[0x004E7E50] = mutant[0x004E7E50].replace(
        old, "base = Bases;", 1
    )
    mutated = "#include <stdint.h>\n" + HOST + arithmetic + "\n" + "\n".join(mutant.values()) + "\nint main(){\n" + main + "\n}\n"
    failed = _compile_run(tmp_path, mutated, compiler)
    assert failed.returncode != 0, "the actual-body predicate mutation survived"

    convoy_mutant = dict(functions)
    convoy_predicate = "if (stack_check(veh_id, 9, 3, -1, -1) != 0)"
    assert convoy_mutant[0x00564D90].count(convoy_predicate) == 1
    convoy_mutant[0x00564D90] = convoy_mutant[0x00564D90].replace(
        convoy_predicate,
        "if (stack_check(veh_id, 9, 3, -1, -1) == 0)", 1,
    )
    convoy_changed = "#include <stdint.h>\n" + HOST + arithmetic + "\n" + "\n".join(convoy_mutant.values()) + "\nint main(){\n" + main + "\n}\n"
    failed = _compile_run(tmp_path, convoy_changed, compiler)
    assert failed.returncode != 0, "the convoy predicate mutation survived"


@pytest.mark.parametrize("address", sorted(HISTORY_EXTENTS))
@pytest.mark.parametrize("damage", ["missing", "duplicate", "owner", "kind", "evidence", "truncated", "unit"])
def test_historical_owned_data_cannot_be_lost_or_reassigned(address, damage):
    from dataclasses import replace
    from match_recovery.model import Origin
    _, current = _current_functions()
    seed = _seed_rows()
    associated = list(read_associated(ROOT, PIN))
    index, binding = next((index, item) for index, item in enumerate(associated)
                          if item.owner.address == address)
    if damage == "missing":
        del associated[index]
    elif damage == "duplicate":
        associated.append(binding)
    elif damage == "owner":
        other = next(item for item in HISTORY_EXTENTS if item != address)
        associated[index] = replace(binding, owner=Origin(PIN, "pe:va", other))
    elif damage == "kind":
        associated[index] = replace(binding, kind="switch_selector")
    elif damage == "evidence":
        associated[index] = replace(binding, evidence=Origin(PIN, "pe:va", binding.evidence.address + 1))
    elif damage == "truncated":
        associated[index] = replace(binding, span=Span(binding.span.start, binding.span.end - 4))
    else:
        associated[index] = replace(binding, unit="src/veh.cpp")
    with pytest.raises(AssertionError):
        _assert_history_extent(address, current[address], seed[address], associated,
                               _pinned_history_image())
