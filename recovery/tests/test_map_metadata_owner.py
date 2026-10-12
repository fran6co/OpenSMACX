"""Actual Map owner/body host controls; no native layout or game acceptance."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from match_recovery.object_source import parse_objects
from match_recovery.pe_coff import PeImage
from match_recovery.model import Span
from match_recovery.source.asm import Listing

ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/map.cpp").is_file())
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
TOKENS = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]', re.S)


def definition(text, signature):
    start = text.index(signature)
    depth = 0
    opened = False
    for token in TOKENS.finditer(text, start):
        if token.group() == "{":
            opened = True
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if opened and depth == 0:
                return text[start:token.end()]
    raise AssertionError("unterminated actual definition")


def actual_bundle():
    header = (ROOT / "src/map.h").read_text()
    landmark = re.search(r"struct Landmark \{.*?\n\};", header, re.S).group()
    start = header.index("struct MapMetadataFields {")
    end = header.index("// The tile-flag mask", start)
    layout = header[start:end]
    record = next(r for r in read_file(ROOT / "src/map.cpp") if r.address == 0x5C86E0)
    assert record.image_spans == ((0x5C86E0, 0x5C899D),)
    body = definition(record.region, "void __cdecl world_build() {")
    assert "BODY GOES HERE" not in body
    paint = next(r for r in read_file(ROOT / "src/map.cpp") if r.address == 0x5C27F0)
    paint_body = definition(paint.region, "void __cdecl paint_land(")
    river = next(r for r in read_file(ROOT / "src/map.cpp") if r.address == 0x5C3680)
    river_body = definition(river.region, "void __cdecl world_riverbeds()")
    temperature = next(r for r in read_file(ROOT / "src/map.cpp") if r.address == 0x5C4170)
    temperature_body = definition(temperature.region, "void __cdecl world_temperature()")
    return landmark + "\nconst int MaxLandmarkNum = 64;\n" + layout + "\n" + body + "\n" + paint_body + "\n" + river_body + "\n" + temperature_body


@pytest.mark.parametrize("damage", [False, True])
def test_lock_header_uses_map_owner_without_duplicate_declaration(tmp_path, damage):
    """Compile actual Lock declarations with the real owner aliases in scope.

    Platform and unrelated included owners are fixture types, not ABI proof.
    The actual include explicitly supplies Map ownership to lock-only users.
    """
    compiler = shutil.which("c++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    lock = (ROOT / "src/lock.h").read_text()
    assert '#include "map.h"' in lock
    assert lock.index('#include "map.h"') < lock.index("class Lock {")
    lock = re.sub(r'^#include [^\n]+\n', '', lock, flags=re.M)
    if damage:
        lock += "\nextern uint32_t MapArea;\n"
    lock_header = tmp_path / "actual_lock.h"
    lock_header.write_text(lock)
    map_header = (ROOT / "src/map.h").read_text()
    landmark = re.search(r"struct Landmark \{.*?\n\};", map_header, re.S).group()
    start = map_header.index("struct MapMetadataFields {")
    layout = map_header[start:map_header.index("// The tile-flag mask", start)]
    source = tmp_path / "lock_owner.cpp"
    source.write_text("#include <stdint.h>\n#include <stddef.h>\n#define __cdecl\n"
                      "typedef int BOOL; struct Map {}; struct PlayerLock { char bytes[28]; };\n"
                      + landmark + "\nconst int MaxLandmarkNum=64;\n" + layout
                      + '\n#include "actual_lock.h"\nMapStorageWindow MapStorage;\n'
                      + 'int main() { MapArea=9; return MapStorage.metadata_.area_==9 ? 0 : 1; }\n')
    binary = tmp_path / "lock_owner"
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)], capture_output=True, timeout=60)
    if damage:
        assert built.returncode != 0
    else:
        assert built.returncode == 0, built.stderr.decode(errors="replace")
        assert subprocess.run([str(binary)], timeout=15).returncode == 0


def projected(damage=None):
    bundle = actual_bundle()
    if damage:
        old, new = damage
        assert bundle.count(old) == 1
        bundle = bundle.replace(old, new)
    fixture = (Path(__file__).with_name("map_metadata_control.cpp")).read_text()
    assert fixture.count("// @@ACTUAL_MAP_OWNER@@") == 1
    return fixture.replace("// @@ACTUAL_MAP_OWNER@@", bundle)


def run_control(tmp_path, damage=None):
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "map_owner.cpp"
    source.write_text(projected(damage))
    binary = tmp_path / "map_owner"
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-Wno-sign-compare",
                            str(source), "-o", str(binary)], capture_output=True, timeout=60)
    assert built.returncode == 0, built.stderr.decode(errors="replace")
    return subprocess.run([str(binary)], capture_output=True, timeout=15)


def test_actual_owner_and_complete_world_build(tmp_path):
    result = run_control(tmp_path)
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("damage", [
    ("if (*rules >= 0)", "if (*rules <= 0)"),
    ("planet_rules_ + 7;", "planet_rules_ + 6;"),
    ("*rules = rand() % 3;", "*rules = rand() % 4;"),
    ("if (*rules >= 0)", "if (true)"),
    ("memcpy(&threshold, &threshold_bits, sizeof(threshold));",
     "threshold = static_cast<int>(threshold_bits & 0x7fffffffu);"),
    ("static_cast<uint32_t>(target / 3)", "static_cast<uint32_t>(target) / 3u"),
    ("int max_riverbeds = scaled / 3200;", "int max_riverbeds = static_cast<uint32_t>(scaled) / 3200u;"),
    ("if (count > threshold)", "if (unk_val > threshold_bits)"),
    ("uint32_t orbit_bound_bits = static_cast<uint32_t>(MapPlanetaryOrbit) + 1u;",
     "uint32_t orbit_bound_bits = static_cast<uint32_t>(MapPlanetaryOrbit);"),
])
def test_compiling_world_build_damage_is_detected(tmp_path, damage):
    assert run_control(tmp_path, damage).returncode != 0


def test_live_object_identities_are_members():
    rows = parse_objects((ROOT / "src/map.cpp").read_text(), "src/map.cpp", PIN,
                         address_space="pe:va")
    expected = {
        0x949870: ("metadata_", "longitude_bounds_"),
        0x949874: ("metadata_", "latitude_bounds_"),
        0x949884: ("metadata_", "area_"),
        0x94988C: ("metadata_", "is_flat_"),
        0x94A30C: ("tiles_",),
        0x94A310: ("abstract_tiles_",),
    }
    for address, path in expected.items():
        row = next(r for r in rows if r.origin.address == address)
        assert row.parent_symbol == "?MapStorage@@3UMapStorageWindow@@A"
        assert row.member_path == path
    text = (ROOT / "src/map.cpp").read_text()
    assert "fwrite(&MapStorage, 0xAA4, 1, map_file)" in text
    assert "fread(&MapStorage, 0xAA4, 1, map_file)" in text
    # The successful-read branch resets the transported pointer words;
    # failure must return first, preserving the original partial-read quirk.
    read = next(r for r in read_file(ROOT / "src/map.cpp") if r.address == 0x591130)
    assert re.search(r"fread\(&MapStorage, 0xAA4, 1, map_file\)\) \{\s*"
                     r"return true;\s*\}\s*map_tiles\(\) = 0;\s*MapAbstract\(\) = 0;", read.region)


def test_pinned_native_map_windows_and_signed_rule_walk():
    explicit = os.environ.get("OPENSMACX_IMAGE")
    candidates = [Path(explicit)] if explicit else [
        p / ".opensmacx/game/terranx_original.exe" for p in (ROOT, *ROOT.parents)]
    image = next((p for p in candidates if p.is_file()), None)
    if image is None:
        pytest.skip("pinned original not available")
    pe = PeImage(image.read_bytes(), PIN)
    assert pe.read_initial(Span(0x949870, 0x94A314)) == bytes(0xAA4)
    assert not pe.absolute_sites(Span(0x949870, 0x94A314))
    for table, opcode, target in [(0x53412C, 2, 0x533342), (0x53420C, 2, 0x533A71)]:
        at = table + opcode * 4
        assert int.from_bytes(pe.read(Span(at, at + 4)), "little") == target
    assert Listing(pe.read(Span(0x533342, 0x533347)), 0x533342).mnemonics == ("mov",)
    rule = Listing(pe.read(Span(0x5C870C, 0x5C8730)), 0x5C870C).instructions
    assert [(i.mnemonic, i.op_str) for i in rule] == [
        ("mov", "esi, 0x94a2a0"), ("cmp", "dword ptr [esi], 0"),
        ("jge", "0x5c8725"), ("call", "0x64601d"), ("cdq", ""),
        ("mov", "ecx, 3"), ("idiv", "ecx"), ("mov", "dword ptr [esi], edx"),
        ("add", "esi, 4"), ("cmp", "esi, 0x94a2bc"), ("jl", "0x5c8711")]
    for address in (0x5910BA, 0x591167):
        instruction = Listing(pe.read(Span(address, address + 5)), address).instructions[0]
        assert (instruction.mnemonic, instruction.op_str) == ("push", "0xaa4")
    paint = Listing(pe.read(Span(0x5C2841, 0x5C2857)), 0x5C2841).instructions
    assert [(i.mnemonic, i.op_str) for i in paint] == [
        ("imul", "ecx, ecx"), ("mov", "dword ptr [ebp - 8], ebx"),
        ("lea", "edx, [ecx + 2]"), ("cmp", "ebx, edx"), ("jle", "0x5c2860"),
        ("sub", "eax, ecx"), ("lea", "eax, [eax + ebx - 2]"), ("and", "eax, 7")]
    river = Listing(pe.read(Span(0x5C36DC, 0x5C3704)), 0x5C36DC).instructions
    assert [i.op_str for i in river if i.mnemonic == "mov" and i.op_str.startswith("eax,")] == [
        "eax, 0x55555556", "eax, edx", "eax, 0x51eb851f"]
    assert any(i.mnemonic == "sar" and i.op_str == "edx, 0xa" for i in river)
    orbit = Listing(pe.read(Span(0x5C4234, 0x5C4245)), 0x5C4234).instructions
    assert [(i.mnemonic, i.op_str) for i in orbit] == [
        ("mov", "edx, dword ptr [0x94a2b0]"), ("mov", "esi, eax"),
        ("inc", "edx"), ("push", "edx"), ("push", "0"), ("call", "0x625810")]
    life = Listing(pe.read(Span(0x4EA137, 0x4EA148)), 0x4EA137).instructions
    assert [(i.mnemonic, i.op_str) for i in life] == [
        ("mov", "edx, dword ptr [0x94a2b8]"), ("imul", "ecx, ebx"),
        ("inc", "edx"), ("imul", "ecx, edx"),
        ("imul", "ecx, dword ptr [esi + 0x48]")]
    cloud = Listing(pe.read(Span(0x45BA7E, 0x45BA9B)), 0x45BA7E).instructions
    assert [(i.mnemonic, i.op_str) for i in cloud] == [
        ("mov", "eax, dword ptr [0x94a2b4]"), ("mov", "ecx, dword ptr [0x9b90f8]"),
        ("add", "esp, 0x18"), ("mov", "eax, dword ptr [ecx + eax*4 + 0x50]"),
        ("mov", "ecx, 0x9b90d8"), ("push", "eax"), ("call", "0x6169a0")]
    ocean = Listing(pe.read(Span(0x5B1CDD, 0x5B1CF8)), 0x5B1CDD).instructions
    assert [(i.mnemonic, i.op_str) for i in ocean] == [
        ("mov", "eax, dword ptr [0x94a2a4]"), ("lea", "edx, [ebx*8]"),
        ("sub", "edx, ebx"), ("lea", "esi, [eax + 1]"),
        ("mov", "eax, dword ptr [edx*4 + 0x9aa734]"), ("imul", "eax, esi")]
    subtraction_anchors = [
        (0x5C2A90, 0x5C2AA4, [("mov", "ecx, dword ptr [0x94a2a8]"), ("mov", "esi, 4"), ("sub", "esi, ecx"),
                             ("lea", "ecx, [esi - 1]"), ("test", "ecx, ecx"), ("jle", "0x5c2b07")]),
        (0x5C2C48, 0x5C2C5F, [("mov", "esi, dword ptr [0x94a2a4]"), ("mov", "edi, dword ptr [ebp + 8]"),
                             ("mov", "ecx, 2"), ("sub", "ecx, esi"), ("imul", "ecx, dword ptr [0x9502c4]")]),
        (0x5C3455, 0x5C3464, [("mov", "eax, dword ptr [0x94a2b8]"), ("xor", "edi, edi"),
                             ("dec", "eax"), ("imul", "eax, dword ptr [0x950308]")]),
        (0x5C8735, 0x5C874F, [("mov", "edi, dword ptr [0x94a2a4]"), ("mov", "ecx, 2"),
                             ("sub", "ecx, edi"), ("mov", "edx, dword ptr [0x9502a8]"), ("imul", "ecx, dword ptr [0x9502ac]")]),
        (0x5C87DB, 0x5C87F5, [("mov", "eax, dword ptr [0x94a2ac]"), ("cmp", "eax, 1"), ("jl", "0x5c8834"),
                             ("lea", "ecx, [eax - 1]"), ("mov", "edx, dword ptr [0x9502b8]"), ("imul", "ecx, dword ptr [0x9502bc]")]),
        (0x58C7E9, 0x58C7FC, [("mov", "edi, dword ptr [0x94a2a4]"), ("mov", "eax, 2"), ("sub", "eax, edi"),
                             ("push", "ebx"), ("mov", "dword ptr [0x94a2a8], eax")]),
        (0x58D7F4, 0x58D806, [("mov", "edx, 2"), ("mov", "dword ptr [0x94a2a4], eax"),
                             ("sub", "edx, eax"), ("mov", "dword ptr [0x94a2a8], edx")]),
        (0x58DD84, 0x58DD96, [("mov", "edx, 2"), ("mov", "dword ptr [0x94a2a4], eax"),
                             ("sub", "edx, eax"), ("mov", "dword ptr [0x94a2a8], edx")]),
    ]
    for lo, hi, expected in subtraction_anchors:
        instructions = Listing(pe.read(Span(lo, hi)), lo).instructions
        assert [(i.mnemonic, i.op_str) for i in instructions] == expected


@pytest.mark.parametrize("damage", [False, True])
def test_actual_setup_player_ocean_factor_expression(tmp_path, damage):
    """Only the signed-migration reader, not complete setup_player behavior."""
    compiler = shutil.which("c++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    record = next(r for r in read_file(ROOT / "src/faction.cpp") if r.address == 0x5B0E00)
    body = definition(record.region, "int __cdecl setup_player(")
    expression = re.search(r"static_cast<int>\(Continents\[region\]\.open_terrain.*? / 2", body, re.S).group()
    if damage:
        expression = expression.replace("(static_cast<uint32_t>(MapOceanCoverage) + 1u)", "(MapOceanCoverage + 1)")
    source = tmp_path / "ocean.cpp"
    source.write_text(r'''#include <cassert>
#include <stdint.h>
#include <climits>
struct Continent { uint32_t open_terrain; } Continents[1];
int MapOceanCoverage;
int apply() { int region=0; return ''' + expression + r'''; }
int main() {
    MapOceanCoverage=1; Continents[0].open_terrain=100; assert(apply()==100);
    MapOceanCoverage=INT_MAX; Continents[0].open_terrain=1; assert(apply()==INT_MIN/2);
    Continents[0].open_terrain=2; assert(apply()==0);
    MapOceanCoverage=-1; assert(apply()==0);
}
''')
    binary = tmp_path / "ocean"
    built = subprocess.run([compiler, "-std=c++11", "-fsanitize=undefined", "-fno-sanitize-recover=undefined", str(source), "-o", str(binary)], capture_output=True, timeout=60)
    assert built.returncode == 0, built.stderr.decode(errors="replace")
    result = subprocess.run([str(binary)], capture_output=True, timeout=15)
    assert (result.returncode != 0) == damage, result.stderr.decode(errors="replace")


SUBTRACTION_READERS = [
    ("map.cpp", 0x5C28F0, "void __cdecl build_continent(",
     r"uint32_t r_bound_bits = .*?memcpy\(&r_bound, &r_bound_bits, sizeof\(r_bound\)\);",
     "r_bound", "4u - static_cast<uint32_t>(MapLandCoverage)", "4 - MapLandCoverage", 3, "INT_MIN+4"),
    ("map.cpp", 0x5C2B40, "void __cdecl build_hills(",
     r"int plat_mod = .*?;", "plat_mod", "2u - static_cast<uint32_t>(MapOceanCoverage)", "2 - MapOceanCoverage", 1, "INT_MIN+2"),
    ("map.cpp", 0x5C3440, "void __cdecl world_fungus()",
     r"int fungus_level = .*?;", "fungus_level", "static_cast<uint32_t>(MapNativeLifeForms) - 1u", "MapNativeLifeForms - 1", 0, "INT_MAX"),
    ("map.cpp", 0x5C86E0, "void __cdecl world_build()",
     r"int land_target = .*?;", "land_target", "2u - static_cast<uint32_t>(MapOceanCoverage)", "2 - MapOceanCoverage", 1, "INT_MIN+2"),
    ("map.cpp", 0x5C86E0, "void __cdecl world_build()",
     r"int hills = .*?;", "hills", "static_cast<uint32_t>(MapErosiveForces) - 1u", "MapErosiveForces - 1", 0, "INT_MAX"),
    *[("newgame.cpp", address, signature,
       r"uint32_t land_bits = .*?memcpy\(&MapLandCoverage, &land_bits, sizeof\(MapLandCoverage\)\);",
       "MapLandCoverage", "2u - static_cast<uint32_t>(MapOceanCoverage)", "2 - MapOceanCoverage", 1, "INT_MIN+2")
      for address, signature in [(0x58C2A0, "int __cdecl custom_planet("),
                                 (0x58D6B0, "int __cdecl map_menu("),
                                 (0x58DB30, "int __cdecl multiplayer_init(")]],
]


@pytest.mark.parametrize("damage", [None,
    ("r_bound_bits - 1u", "static_cast<uint32_t>(r_bound - 1)"),
    ("bound_test > 0", "r_bound > 1"),
], ids=["source", "signed-overflow-mutant", "logical-bound-mutant"])
def test_actual_continent_wrapped_bound_branch_and_random_call(tmp_path, damage):
    """The complete touched branch, including original random-call timing."""
    compiler = shutil.which("c++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    record = next(r for r in read_file(ROOT / "src/map.cpp") if r.address == 0x5C28F0)
    body = definition(record.region, "void __cdecl build_continent(")
    branch = re.search(r"uint32_t r_bound_bits = .*?if \(\(bound_test > 0\).*?\{\s*radius /= 2;\s*\}", body, re.S).group()
    if damage:
        old, new = damage
        assert branch.count(old) == 1
        branch = branch.replace(old, new)
    source = tmp_path / "continent_bound.cpp"
    source.write_text(r'''#include <cassert>
#include <stdint.h>
#include <cstring>
static int calls;
static int fixture_rand() { ++calls; return 1; }
#define rand fixture_rand
int MapLandCoverage;
int apply(uint32_t land_bits) {
    memcpy(&MapLandCoverage,&land_bits,sizeof(MapLandCoverage));
    int radius=8;
''' + branch + r'''
    return radius;
}
int main() {
    calls=0; assert(apply(1u)==4 && calls==1);
    calls=0; assert(apply(4u)==8 && calls==0);
    calls=0; assert(apply(5u)==8 && calls==0);
    // r_bound is INT_MIN, but wrapped bound_test is INT_MAX: rand must
    // run, and its nonnegative result is divided by the negative bound.
    calls=0; assert(apply(0x80000004u)==4 && calls==1);
}
''')
    binary = tmp_path / "continent_bound"
    built = subprocess.run([compiler, "-std=c++11", "-fsanitize=undefined", "-fno-sanitize-recover=undefined", str(source), "-o", str(binary)], capture_output=True, timeout=60)
    assert built.returncode == 0, built.stderr.decode(errors="replace")
    result = subprocess.run([str(binary)], capture_output=True, timeout=15)
    assert (result.returncode != 0) == bool(damage), result.stderr.decode(errors="replace")


@pytest.mark.parametrize("case", SUBTRACTION_READERS, ids=[
    "continent", "plateau", "fungus", "world-land", "world-hills", "custom-land", "menu-land", "multiplayer-land"])
@pytest.mark.parametrize("damage", [False, True], ids=["source", "signed-overflow-mutant"])
def test_actual_signed_rule_subtraction_reader(tmp_path, case, damage):
    """Actual reader statements, not reachability or whole-function recovery.

    World-build's own normalization and menu producer bounds are not removed;
    the expression controls preserve the pre-migration word interpretation.
    """
    compiler = shutil.which("c++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    file, address, signature, pattern, result_name, old, new, ordinary, extreme = case
    record = next(r for r in read_file(ROOT / "src" / file) if r.address == address)
    body = definition(record.region, signature)
    statement = re.search(pattern, body, re.S).group()
    assert statement.count(old) == 1
    if damage:
        statement = statement.replace(old, new)
    source = tmp_path / "reader.cpp"
    source.write_text(r'''#include <cassert>
#include <stdint.h>
#include <climits>
#include <cstring>
int MapLandCoverage,MapOceanCoverage,MapNativeLifeForms,MapErosiveForces;
struct Builder { uint32_t plateau_mod,fungus,land_mod,land_base,hills_mod,hills_base; } WorldBuilder={1,1,1,0,1,0};
int apply(int rule) {
    MapLandCoverage=MapOceanCoverage=MapNativeLifeForms=MapErosiveForces=rule;
''' + statement + "\nreturn " + result_name + r''';
}
int main() { assert(apply(1)==''' + str(ordinary) + "); assert(apply(INT_MIN)==" + extreme + "); }\n")
    binary = tmp_path / "reader"
    built = subprocess.run([compiler, "-std=c++11", "-fsanitize=undefined", "-fno-sanitize-recover=undefined", str(source), "-o", str(binary)], capture_output=True, timeout=60)
    assert built.returncode == 0, built.stderr.decode(errors="replace")
    result = subprocess.run([str(binary)], capture_output=True, timeout=15)
    assert (result.returncode != 0) == damage, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("damage", [False, True])
def test_actual_base_minerals_life_factor_expression(tmp_path, damage):
    """Actual wrapped scale pipeline; the economy suite executes the whole body."""
    from test_base_economy_accounting_oct12 import helpers
    compiler = shutil.which("c++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    record = next(r for r in read_file(ROOT / "src/base.cpp") if r.address == 0x4E9CB0)
    body = definition(record.region, "void __cdecl base_minerals()")
    expression = re.search(r"int scaled_damage = .*?BaseCurrent->eco_damage = scaled_damage / 6;", body, re.S).group()
    if damage:
        assert expression.count("base_yield_add(MapNativeLifeForms, 1)") == 1
        expression = expression.replace("base_yield_add(MapNativeLifeForms, 1)", "(MapNativeLifeForms + 1)")
    source = tmp_path / "life.cpp"
    source.write_text(r'''#include <cassert>
#include <stdint.h>
#include <climits>
struct SocialEffect { enum { PLANET=0 }; };
struct Player { int tech_ranking, theory_of_everything; struct { int values_[1]; } soc_effect_pending; } PlayersData[1];
struct Base { int eco_damage; } base;
Base *BaseCurrent=&base;
int MapNativeLifeForms;
int range(int x,int low,int high) { return x<low?low:x>high?high:x; }
''' + helpers() + r'''
void apply() { int faction_id=0; int diff_factor=1;
''' + expression + r'''
}
int main() {
    PlayersData[0].tech_ranking=1; PlayersData[0].soc_effect_pending.values_[0]=2;
    MapNativeLifeForms=1; base.eco_damage=6; apply(); assert(base.eco_damage==2);
    MapNativeLifeForms=INT_MAX; base.eco_damage=1; apply();
    // Pinned4EA150 is signed IMUL by2AAAAAAB, with the signed /6 correction.
    assert(base.eco_damage==INT_MIN/6);
    MapNativeLifeForms=-1; base.eco_damage=6; apply(); assert(base.eco_damage==0);
}
''')
    binary = tmp_path / "life"
    built = subprocess.run([compiler, "-std=c++11", "-fsanitize=undefined", "-fno-sanitize-recover=undefined", str(source), "-o", str(binary)], capture_output=True, timeout=60)
    assert built.returncode == 0, built.stderr.decode(errors="replace")
    result = subprocess.run([str(binary)], capture_output=True, timeout=15)
    assert (result.returncode != 0) == damage, result.stderr.decode(errors="replace")
