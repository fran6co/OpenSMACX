"""Actual-source loader controls, not asset-content or game acceptance."""
from pathlib import Path
import struct
import os
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file

ROOT = Path(__file__).resolve().parents[2]

# Current proposal homes; coordinator may adjust these with actual integration.
OWNERS = {
    0x00454D10: ("src/sprite.cpp", "void __cdecl load_misc()"),
    0x00455EB0: ("src/game.cpp", "void __cdecl load_sprites()"),
    0x00451A10: ("src/interface_palette.cpp", "void __cdecl load_palette()"),
}

BASE = '97ad25151bb1c72962e41f711299c54a7912ed9d'
# Pre-Palette-boundary checkpoint for unchanged neighboring bodies. BASE above
# remains the seed/provenance baseline. Subsequent genuine storage migrations
# (e.g. TurnCurrentNum -> GameDataStateStorage) changed their external references;
# comparing today's complete bodies to BASE is not an unchanged-source control.
PALETTE_BOUNDARY_BASE = '3d36b6a854332711f5168676086cde8d58caa550'


@pytest.mark.parametrize('address', tuple(OWNERS))
def test_loader_original_spans_claims_and_findings_retained(address):
    from match_recovery.source import read_text
    seed = 'src/recovered/units/%08x.cpp' % address
    old, = read_text(subprocess.check_output(['git', 'show', BASE + ':' + seed], cwd=ROOT).decode(), ROOT / seed)
    owner, _ = OWNERS[address]
    current, = (r for r in read_file(ROOT / owner) if r.address == address)
    for field in ('address','name','image_spans','levers','ruled_out','deferred','unrecoverable'):
        assert getattr(current, field) == getattr(old, field), field
    # Retain old obligations; the guarded recorder may bank a new exact body.
    assert not old.byte_exact or current.byte_exact
    assert not old.semantic or current.semantic


def test_original_sprite_initializer_bounds_and_teardown():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_IMM
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    image = PeImage((ROOT / '.opensmacx/game/terranx_original.exe').read_bytes(),image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86,CS_MODE_32)
    decoder.detail = True
    wrappers = ((0x44F8F0,0x44F910,0x779200,1), (0x44F920,0x44F940,0x76DC30,1),
                (0x44F950,0x44F970,0x78A4A0,1), (0x44F980,0x44F9A0,0x787E70,1),
                (0x44F9B0,0x44F9D0,0x7AC000,1), (0x44F9E0,0x44FA00,0x779570,1),
                (0x44FA10,0x44FA40,0x759E28,89), (0x44FA60,0x44FA90,0x787200,70),
                (0x44FAB0,0x44FAE0,0x759320,64))
    assert struct.unpack('<9I',image.read(Span(0x6823A4,0x6823C8))) == tuple(w[0] for w in wrappers)
    def instructions(address,length):
        result = []
        for instruction in decoder.disasm(image.read(Span(address,address+length)),address):
            result.append(instruction)
            if instruction.mnemonic in ('ret','jmp'):
                break
        return result
    def immediates(ops,mnemonic):
        return [o.operands[0].imm for o in ops if o.mnemonic==mnemonic and o.operands[0].type==X86_OP_IMM]
    for ctor,dtor,start,count in wrappers:
        init,done = instructions(ctor,48),instructions(dtor,32)
        if count == 1:
            assert init[0].mnemonic == done[0].mnemonic == 'mov'
            assert init[0].op_str == done[0].op_str == 'ecx, '+hex(start)
            assert immediates(init,'call') == [0x5E37E0,0x645398]
            assert immediates(init,'push') == [dtor]
            assert immediates(done,'jmp') == [0x5E3820]
        else:
            assert immediates(init,'push') == [0x406850,0x5E37E0,count,0x2C,start,dtor]
            assert immediates(init,'call') == [0x6457C2,0x645398]
            assert immediates(done,'push') == [0x406850,count,0x2C,start]
            assert immediates(done,'call') == [0x6456E4]


def test_native_loader_objects_references_and_layout():
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff, PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    from match_recovery.codeview import class_layout
    from match_recovery.object_source import read_objects, object_targets
    from match_recovery.object_data import check_initial_data
    from match_recovery.coff_replica import function_signature
    from match_recovery.source_history import revision_files
    from match_recovery.source_replica import replica_roots
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / 'BIN/CL.EXE').is_file() or not shutil.which('wine32'):
        pytest.skip('VC6 and Wine32 required')
    image = PeImage((ROOT / '.opensmacx/game/terranx_original.exe').read_bytes(), image_pin(ROOT))
    assert image.read(Span(0x66C540,0x66C544)) == struct.pack('<f',0.01)
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    # A historical TU requires its historical headers and retired includes.
    # Overlaying just game.cpp/sprite.h against current sources cannot preserve
    # that context (temp.h and application palette declarations have moved).
    roots = replica_roots((*settings.source.roots, *settings.input_roots))
    historical = revision_files(ROOT, PALETTE_BOUNDARY_BASE, ('src', 'cmake'))
    # Retain the current measurement recipe's generated compiler context, not
    # current source headers. Both versions use today's compiler policy; this
    # is a bounded body/reference comparison, not a historical build proof.
    for name in roots:
        if name not in ('src', 'cmake'):
            for path in (ROOT / name).rglob('*'):
                if path.is_file():
                    historical[path.relative_to(ROOT).as_posix()] = path.read_bytes()
    for flags in settings.flag_sets:
        objects = {unit: Coff(compiler(ROOT / unit,(),flags+settings.layout_flags))
                   for unit in sorted({owner for owner,_ in OWNERS.values()})}
        def targets(address,name):
            obj = objects[OWNERS[address][0]]
            body,relocs,_ = obj.subject(obj.function(name))
            assert body
            return {obj.symbols[int(r.target)].name for r in relocs}
        misc = targets(0x454D10,'?load_misc@@YAXXZ')
        for name in ('g_TECH_ICON_SPRITES','g_FACILITY_ICON_SPRITES','g_SECRET_PROJECT_ICON_SPRITES',
                     'g_BATTLE_MIND_WORM_SPRITE','g_BATTLE_ISLE_DEEP_SPRITE','g_BATTLE_LOCUSTS_CHIRON_SPRITE',
                     'g_BATTLE_FUNGAL_TOWER_SPRITE','g_BATTLE_SPORE_LAUNCHER_SPRITE','g_BATTLE_SEALURK_SPRITE'):
            assert any(s.startswith('?'+name+'@@') for s in misc), (name,misc)
        sprite = class_layout(objects['src/sprite.cpp'],'Sprite')
        assert sprite['size'] == 0x2C
        assert sprite['members']['iWidth_'] == 0x18
        assert sprite['members']['iHeight_'] == 0x1C
        bindings = read_objects(ROOT,image_pin(ROOT))
        bound,_ = object_targets(bindings,objects)
        expected = {'g_TECH_ICON_SPRITES':0x759E28,'g_FACILITY_ICON_SPRITES':0x787200,
                    'g_SECRET_PROJECT_ICON_SPRITES':0x759320,'g_BATTLE_MIND_WORM_SPRITE':0x779200,
                    'g_BATTLE_ISLE_DEEP_SPRITE':0x76DC30,'g_BATTLE_LOCUSTS_CHIRON_SPRITE':0x78A4A0,
                    'g_BATTLE_FUNGAL_TOWER_SPRITE':0x787E70,'g_BATTLE_SPORE_LAUNCHER_SPRITE':0x7AC000,
                    'g_BATTLE_SEALURK_SPRITE':0x779570}
        for name,address in expected.items():
            symbol, = (s for s in misc if s.startswith('?'+name+'@@'))
            assert bound[symbol] == address
            binding, = (b for b in bindings if b.symbol == symbol)
            count = {'g_TECH_ICON_SPRITES':89,'g_FACILITY_ICON_SPRITES':70,
                     'g_SECRET_PROJECT_ICON_SPRITES':64}.get(name,1)
            assert binding.span == Span(address,address+count*0x2C)
            assert binding.evidence.address == 0x454D10
        selected_bindings = tuple(b for b in bindings if b.origin.address in expected.values())
        data_rows = check_initial_data(selected_bindings,objects,image,bound)
        assert len(data_rows) == 9
        for row in data_rows:
            assert row['definition'] == row['content'] == 'pass', row
            # Initial image/COFF storage is not post-construction state.
            assert row['lifetime'] == 'unknown'
        sprites = targets(0x455EB0,'?load_sprites@@YAXXZ')
        for name in ('load_vehicles','load_terrain','load_icons','load_cursors','FactionsArt','FactionBaseSprites'):
            assert any(s.startswith('?'+name+'@@') for s in sprites), (name,sprites)
        assert class_layout(objects['src/game.cpp'],'FactionArt')['size'] == 0x65C
        palette = targets(0x451A10,'?load_palette@@YAXXZ')
        for name in ('g_PALETTE1','g_PALETTE2','PrefWinGammaCorrection','prefs_get'):
            assert any(s.startswith('?'+name+'@@') for s in palette), (name,palette)
        assert any('load_pcx@Buffer@@' in s for s in palette)
        assert any('copy_from@Palette@@' in s for s in palette)
        assert any('exec@Gamma@@' in s for s in palette)
        # Compare fresh pre-boundary/current native code and external references.
        # This does not discharge either body's original restoration obligation
        # or establish linked preservation.
        old_game = Coff(compiler.materialize(ROOT/'src/game.cpp', roots,
                                            historical, (), flags+settings.layout_flags))
        assert function_signature(old_game,'?mon_enemy_destroyed@@YAXHH@Z') == function_signature(objects['src/game.cpp'],'?mon_enemy_destroyed@@YAXHH@Z')
        # Observe the real TexHeap neighbor under the same bounded checkpoint.
        old_sprite = Coff(compiler.materialize(ROOT/'src/sprite.cpp', roots,
                                              historical, (), flags+settings.layout_flags))
        assert function_signature(old_sprite,'?get_mem@TexHeap@@QAEPAXH@Z') == function_signature(objects['src/sprite.cpp'],'?get_mem@TexHeap@@QAEPAXH@Z')

from test_game_logic_seed_batch_behavior import _definition


def _mapped_definition(address):
    relative, signature = OWNERS[address]
    rows = [row for row in read_file(ROOT / relative) if row.address == address]
    assert len(rows) == 1, "missing/duplicate current mapping " + hex(address)
    row, = rows
    assert row.mode == Mode.BODY and row.region
    assert signature in row.region and "BODY GOES HERE." not in row.region
    return _definition(row.region, signature)


HARNESS = r'''
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
struct Palette;
struct TexHeap;
struct Sprite {
  int iWidth_, iHeight_;
  std::string last_name;
  int calls;
  Sprite() : iWidth_(0), iHeight_(0), calls(0) {}
  int init(const char *name, int width, int height);
};
Sprite g_TECH_ICON_SPRITES[89];
Sprite g_FACILITY_ICON_SPRITES[70];
Sprite g_SECRET_PROJECT_ICON_SPRITES[64];
Sprite g_BATTLE_MIND_WORM_SPRITE, g_BATTLE_ISLE_DEEP_SPRITE;
Sprite g_BATTLE_LOCUSTS_CHIRON_SPRITE, g_BATTLE_FUNGAL_TOWER_SPRITE;
Sprite g_BATTLE_SPORE_LAUNCHER_SPRITE, g_BATTLE_SEALURK_SPRITE;
char StringTemp[1032];
char *_itoa(int value, char *buffer, int radix) {
  assert(radix == 10); std::sprintf(buffer, "%d", value); return buffer;
}
struct FactionArt { Sprite base[2][4][3]; } FactionsArt[8];
Sprite *FactionBaseSprites[8][2][4][3];
const int MaxPlayerNum = 8;
static std::vector<std::string> events;
int Sprite::init(const char *name, int width, int height) {
  last_name = name; ++calls;
  events.push_back(last_name + ":" + std::to_string(width) + "," + std::to_string(height));
  return 1; // Native loader ignores this service status.
}
void load_vehicles() { events.push_back("vehicles"); }
void load_terrain() { events.push_back("terrain"); }
void load_icons() { events.push_back("icons"); }
void load_cursors() { events.push_back("cursors"); }
void __cdecl load_misc();
void __cdecl load_sprites();

struct Palette { int tag; int copies; Palette &copy_from(Palette *); };
Palette g_PALETTE1 = {1,0}, g_PALETTE2 = {2,0};
float PrefWinGammaCorrection;
static int prefs_calls, pref_result, buffer_calls, gamma_calls;
static std::string pref_key, pcx_name;
static int pref_default, pref_flags, pcx_first, pcx_count, pcx_result;
static Palette *pcx_palette, *copy_source, *gamma_palette;
static float gamma_value;
static int gamma_apply;
static std::vector<std::string> life;
int prefs_get(const char *key, int default_value, int flags) {
  life.push_back("prefs"); ++prefs_calls; pref_key=key; pref_default=default_value; pref_flags=flags; return pref_result;
}
struct Buffer {
  Buffer();
  ~Buffer();
  int load_pcx(const char *, Palette *, int, int);
};
struct Gamma {
  Gamma();
  ~Gamma();
  double exec(Palette *, float, int);
};
static Buffer *expected_buffer;
static Gamma *expected_gamma;
Buffer::Buffer() { expected_buffer=this; life.push_back("buffer+ "); }
Buffer::~Buffer() { assert(this==expected_buffer); life.push_back("buffer-"); }
Gamma::Gamma() { expected_gamma=this; life.push_back("gamma+"); }
Gamma::~Gamma() { assert(this==expected_gamma); life.push_back("gamma-"); }
int Buffer::load_pcx(const char *name, Palette *palette, int first, int count) {
  assert(this==expected_buffer);
  ++buffer_calls; pcx_name=name; pcx_palette=palette; pcx_first=first; pcx_count=count;
  life.push_back("pcx"); return pcx_result;
}
Palette &Palette::copy_from(Palette *source) {
  ++copies; copy_source=source; life.push_back("copy"); tag=source->tag+1; return *this;
}
double Gamma::exec(Palette *palette, float gamma, int apply) {
  assert(this==expected_gamma);
  ++gamma_calls; gamma_palette=palette; gamma_value=gamma; gamma_apply=apply;
  life.push_back("gamma.exec"); return 123.0f;
}
void __cdecl load_palette();

/* BODIES */

static std::string decimal(int value) { char b[32]; std::sprintf(b,"%d",value); return b; }
static std::string expected_tech(int i) {
  char b[32]; std::sprintf(b,"techs\\tech%02d",i); return b;
}
static std::string expected_project(int i) {
  char b[32]; std::sprintf(b,"projs\\proj%03d",i); return b;
}
static std::string expected_facility(int i) {
  std::string s="facs\\";
  if (i>=34 && i<=64) s += "x";
  s += "fac0";
  if (i<10) s += "0";
  s += decimal(i>64 ? i-31 : i);
  return s;
}
static void reset() {
  events.clear(); life.clear(); prefs_calls=buffer_calls=gamma_calls=0;
  pref_result=100; pcx_result=9; PrefWinGammaCorrection=-9.0f;
  g_PALETTE1.tag=1; g_PALETTE2.tag=2; g_PALETTE2.copies=0;
  for (int i=0;i<89;++i) g_TECH_ICON_SPRITES[i]=Sprite();
  for (int i=0;i<70;++i) g_FACILITY_ICON_SPRITES[i]=Sprite();
  for (int i=0;i<64;++i) g_SECRET_PROJECT_ICON_SPRITES[i]=Sprite();
}
static void check_misc() {
  reset(); load_misc(); assert(events.size()==229);
  size_t n=0;
  for (int i=0;i<89;++i,++n) {
    Sprite &s=g_TECH_ICON_SPRITES[i];
    assert(events[n]==expected_tech(i)+":0,0");
    assert(s.calls==1 && s.last_name==expected_tech(i));
    assert(s.iWidth_==215 && s.iHeight_==215);
  }
  for (int i=0;i<70;++i,++n) {
    Sprite &s=g_FACILITY_ICON_SPRITES[i];
    assert(events[n]==expected_facility(i)+":0,0");
    assert(s.calls==1 && s.last_name==expected_facility(i));
    assert(s.iWidth_==215 && s.iHeight_==215);
  }
  for (int i=0;i<64;++i,++n) {
    Sprite &s=g_SECRET_PROJECT_ICON_SPRITES[i];
    assert(events[n]==expected_project(i)+":0,0");
    assert(s.calls==1 && s.last_name==expected_project(i));
    assert(s.iWidth_==215 && s.iHeight_==215);
  }
  const char *names[]={"btwrm.pcx","btisl.pcx","btchi.pcx","BTfungtow.pcx","BTsporlnch.pcx","BTsealrk.pcx"};
  Sprite *battle[]={&g_BATTLE_MIND_WORM_SPRITE,&g_BATTLE_ISLE_DEEP_SPRITE,
    &g_BATTLE_LOCUSTS_CHIRON_SPRITE,&g_BATTLE_FUNGAL_TOWER_SPRITE,
    &g_BATTLE_SPORE_LAUNCHER_SPRITE,&g_BATTLE_SEALURK_SPRITE};
  for(int i=0;i<6;++i,++n) {
    assert(events[n]==std::string(names[i])+":0,0");
    assert(battle[i]->calls==1 && battle[i]->last_name==names[i]);
    assert(battle[i]->iWidth_==96 && battle[i]->iHeight_==115);
  }
}
static void check_sprites() {
  reset(); load_sprites();
  assert(events.size()==4 && events[0]=="vehicles" && events[1]=="terrain"
         && events[2]=="icons" && events[3]=="cursors");
  for(int f=0;f<8;++f) for(int t=0;t<2;++t) for(int s=0;s<4;++s) for(int d=0;d<3;++d)
    assert(FactionBaseSprites[f][t][s][d]==&FactionsArt[f].base[t][s][d]);
  events.clear(); load_sprites();
  assert(events.size()==4 && events[0]=="vehicles" && events[1]=="terrain"
         && events[2]=="icons" && events[3]=="cursors");
}
static void check_palette() {
  reset(); pref_result=137; pcx_result=77; load_palette();
  assert(prefs_calls==1 && pref_key=="Gamma Correction" && pref_default==100 && pref_flags==0);
  assert(PrefWinGammaCorrection==1.37f);
  assert(buffer_calls==1 && pcx_name=="PALETTE.PCX" && pcx_palette==&g_PALETTE1);
  assert(pcx_first==10 && pcx_count==236);
  assert(g_PALETTE2.copies==1 && copy_source==&g_PALETTE1 && g_PALETTE2.tag==2);
  assert(gamma_calls==1 && gamma_palette==&g_PALETTE1 && gamma_value==1.37f && gamma_apply==1);
  const std::vector<std::string> expected={"buffer+ ","gamma+","prefs","pcx","copy","gamma.exec","gamma-","buffer-"};
  assert(life==expected); // includes ignored nonzero PCX status and reverse teardown
}
int main() { check_misc(); check_sprites(); check_palette(); }
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_loader_source_host_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host C++ compiler required")
    addresses = (0x00454D10, 0x00455EB0, 0x00451A10)
    bodies = {address: _mapped_definition(address) for address in addresses}

    def run_case(case, expect_success):
        source = HARNESS.replace("/* BODIES */", "\n\n".join(bodies[a] for a in addresses))
        cpp, exe = tmp_path / (case + ".cpp"), tmp_path / case
        cpp.write_text(source)
        built = subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=address,undefined",
                                "-fno-sanitize-recover=all",str(cpp),"-o",str(exe)],
                               capture_output=True,text=True,timeout=30)
        assert built.returncode == 0, case + ": " + built.stderr
        run = subprocess.run([str(exe)],capture_output=True,text=True,timeout=10,
                             env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))
        if expect_success:
            assert run.returncode == 0, run.stderr
        else:
            assert run.returncode != 0, case + " mutation survived"

    run_case("loader_control", True)
    mutations = (
        (0x00454D10, "index < 89", "index < 88"),
        (0x00454D10, "index < 70", "index < 69"),
        (0x00454D10, "index < 64", "index < 63"),
        (0x00454D10, '"projs\\\\proj0"', '"projs\\\\proj1"'),
        (0x00454D10, 'g_TECH_ICON_SPRITES[index].iHeight_ = 0xD7;', 'g_TECH_ICON_SPRITES[index].iHeight_ = 0xD6;'),
        (0x00454D10, "index >= 34 && index <= 64", "index >= 34 && index <= 63"),
        (0x00454D10, "_itoa(index - 0x1F, number, 10);", "_itoa(index - 0x1E, number, 10);"),
        (0x00454D10, '"BTsporlnch.pcx"', '"BTsporlunch.pcx"'),
        (0x00454D10, 'g_BATTLE_MIND_WORM_SPRITE.init("btwrm.pcx", 0, 0);', 'g_BATTLE_ISLE_DEEP_SPRITE.init("btwrm.pcx", 0, 0);'),
        (0x00454D10, 'g_BATTLE_SEALURK_SPRITE.iWidth_ = 0x60;', 'g_BATTLE_SEALURK_SPRITE.iWidth_ = 0x61;'),
        (0x00454D10, 'g_BATTLE_SEALURK_SPRITE.iHeight_ = 0x73;', 'g_BATTLE_SEALURK_SPRITE.iHeight_ = 0x74;'),
        (0x00451A10, 'buffer.load_pcx("PALETTE.PCX", &g_PALETTE1, 0x0A, 0xEC);', 'Buffer other; other.load_pcx("PALETTE.PCX", &g_PALETTE1, 0x0A, 0xEC);'),
        (0x00451A10, '''    gamma_percent = prefs_get("Gamma Correction", 100, 0);
    gamma_value = static_cast<float>(gamma_percent) * 0.01f;
    PrefWinGammaCorrection = gamma_value;

    buffer.load_pcx("PALETTE.PCX", &g_PALETTE1, 0x0A, 0xEC);''', '''    buffer.load_pcx("PALETTE.PCX", &g_PALETTE1, 0x0A, 0xEC);
    gamma_percent = prefs_get("Gamma Correction", 100, 0);
    gamma_value = static_cast<float>(gamma_percent) * 0.01f;
    PrefWinGammaCorrection = gamma_value;'''),
        (0x00455EB0, "load_vehicles();\n    load_terrain();", "load_terrain();\n    load_vehicles();"),
        (0x00455EB0, "FactionBaseSprites[faction][terrain][size][defense]", "FactionBaseSprites[faction][terrain][defense][size]"),
        (0x00451A10, '"PALETTE.PCX", &g_PALETTE1, 0x0A, 0xEC', '"PALETTE.PCX", &g_PALETTE1, 0x00, 0xEC'),
        (0x00451A10, "g_PALETTE2.copy_from(&g_PALETTE1)", "g_PALETTE1.copy_from(&g_PALETTE2)"),
        (0x00451A10, "gamma_value, 1", "gamma_value, 0"),
        (0x00451A10, "* 0.01f", "* 0.02f"),
    )
    for number, (address, before, after) in enumerate(mutations):
        mutant = dict(bodies)
        assert before in mutant[address], "mutation anchor drift " + hex(address) + ": " + before
        mutant[address] = mutant[address].replace(before, after, 1)
        saved = bodies
        bodies = mutant
        run_case("loader_mutation_%02d" % number, False)
        bodies = saved
