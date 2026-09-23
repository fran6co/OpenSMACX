"""Actual FactionArt ownership and load_bases453A20 publication controls.

The original owner has37 Sprite subobjects:24 base sprites, two council logos,
six single sprites, three small report logos and two report logos. Eight owners
construct296 Sprites; the producer publishes192 borrowed base-sprite pointers.
The actual declaration, empty lifetime bodies, definitions and complete producer
are extracted unchanged. Sprite is an instrumented service double with44-byte
storage, not the real graphics implementation. Host C++ lifetime/unwind controls
do not prove native VC6 ABI, original exception metadata, asset contents, the
other load_sprites producer, startup integration or game acceptance. ASan and
UBSan remain enabled; LeakSanitizer is disabled because this sandbox uses ptrace.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated actual definition")


def actual_source():
    header = (ROOT / "src/faction.h").read_text()
    source = (ROOT / "src/faction.cpp").read_text()
    game = (ROOT / "src/game.cpp").read_text()
    maximum, = re.findall(r"^static const int MaxPlayerNum = \d+;", header, re.M)
    objects, = re.findall(r"^FactionArt FactionsArt\[MaxPlayerNum\];", source, re.M)
    table, = re.findall(r"^Sprite \*FactionBaseSprites\[MaxPlayerNum\]\[2\]\[4\]\[3\];", source, re.M)
    object_extern, = re.findall(r"^extern " + re.escape(objects), header, re.M)
    table_extern, = re.findall(r"^extern " + re.escape(table), header, re.M)
    return {
        "MAXIMUM": maximum,
        "ART": braced(header, "struct FactionArt {") + ";",
        "CTOR": braced(source, "FactionArt::FactionArt()"),
        "DTOR": braced(source, "FactionArt::~FactionArt()"),
        "PUBLIC": "\n".join((object_extern, table_extern)),
        "OBJECTS": objects + "\n" + table,
        "LOADER": braced(game, "void __cdecl load_bases()"),
    }


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <new>
#define __cdecl
// Failed mutants stop at their first oracle failure; only successful runs
// perform normal shutdown and its independently checked global teardown.
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::_Exit(94);}}
#define require(x) check((x),__LINE__)
uintptr_t built[4096],destroyed[4096],global_order[296];
int attempts,builds,deaths,copies,assignments,throw_at;
bool shutdown_ready;
struct Sprite {
    uint32_t words[11];
    Sprite(){++attempts;if(throw_at&&attempts==throw_at)throw 17;
        require(builds<4096);built[builds++]=reinterpret_cast<uintptr_t>(this);
        for(int n=0;n<11;++n)words[n]=uint32_t(attempts*31+n);}
    Sprite(const Sprite&other){++copies;for(int n=0;n<11;++n)words[n]=other.words[n];}
    Sprite&operator=(const Sprite&other){++assignments;for(int n=0;n<11;++n)words[n]=other.words[n];return *this;}
    ~Sprite(){require(deaths<4096);destroyed[deaths++]=reinterpret_cast<uintptr_t>(this);}
};
MAXIMUM
ART
PUBLIC
CTOR
DTOR
struct GlobalDestructionVerifier {
    ~GlobalDestructionVerifier(){require(shutdown_ready&&deaths==296&&copies==0&&assignments==0);
        for(int n=0;n<296;++n)require(destroyed[n]==global_order[295-n]);}
} destruction_verifier;
OBJECTS
LOADER

// Named-subobject order is independently specified from the original layout;
// it is not inferred by walking bytes or copied from the declaration order.
void member_order(FactionArt*art,uintptr_t*out){int n=0;
    for(int faction=0;faction<8;++faction){FactionArt&a=art[faction];
        for(int terrain=0;terrain<2;++terrain)for(int size=0;size<4;++size)for(int defense=0;defense<3;++defense)
            out[n++]=reinterpret_cast<uintptr_t>(&a.base[terrain][size][defense]);
        out[n++]=reinterpret_cast<uintptr_t>(&a.council_logo[0]);
        out[n++]=reinterpret_cast<uintptr_t>(&a.council_logo[1]);
        out[n++]=reinterpret_cast<uintptr_t>(&a.council_small);
        out[n++]=reinterpret_cast<uintptr_t>(&a.council_big);
        out[n++]=reinterpret_cast<uintptr_t>(&a.diplomacy);
        out[n++]=reinterpret_cast<uintptr_t>(&a.diplomacy_logo);
        out[n++]=reinterpret_cast<uintptr_t>(&a.diplomacy_landscape);
        out[n++]=reinterpret_cast<uintptr_t>(&a.datalink);
        for(int j=0;j<3;++j)out[n++]=reinterpret_cast<uintptr_t>(&a.report_logo_small[j]);
        for(int j=0;j<2;++j)out[n++]=reinterpret_cast<uintptr_t>(&a.report_logo[j]);
    }require(n==296);
}
void construction_order(FactionArt*art){uintptr_t expected[296];member_order(art,expected);
    require(attempts==296&&builds==296&&deaths==0&&copies==0&&assignments==0);
    require(sizeof(FactionArt)==37*sizeof(Sprite));
    for(int n=0;n<296;++n)require(built[n]==expected[n]);
}
void clean_events(){attempts=builds=deaths=copies=assignments=throw_at=0;}
void publication(){
    // No asset is copied or reconstructed by publication; distinct existing
    // council sprites poison every entry before each of two full calls.
    uint32_t original_words[296][11];
    for(int n=0;n<296;++n){Sprite*asset=reinterpret_cast<Sprite*>(global_order[n]);
        for(int j=0;j<11;++j)original_words[n][j]=asset->words[j];}
    for(int pass=0;pass<2;++pass){
        for(int faction=0;faction<8;++faction)for(int terrain=0;terrain<2;++terrain)
            for(int size=0;size<4;++size)for(int defense=0;defense<3;++defense){
                Sprite*&slot=FactionBaseSprites[faction][terrain][size][defense];
                if(pass==0)require(slot==nullptr);
                slot=&FactionsArt[(faction+pass)%8].council_logo[pass];}
        load_bases();
        int checked=0;
        for(int faction=0;faction<8;++faction)for(int terrain=0;terrain<2;++terrain)
            for(int size=0;size<4;++size)for(int defense=0;defense<3;++defense){
                // Original faction stride0x65C and base entry stride0x2C
                // reduce to ordinal37*faction+12*terrain+3*size+defense.
                const int ordinal=faction*37+terrain*12+size*3+defense;
                require(reinterpret_cast<uintptr_t>(FactionBaseSprites[faction][terrain][size][defense])==global_order[ordinal]);
                ++checked;}
        require(checked==192&&attempts==296&&builds==296&&deaths==0&&copies==0&&assignments==0);
        for(int n=0;n<296;++n){Sprite*asset=reinterpret_cast<Sprite*>(global_order[n]);
            for(int j=0;j<11;++j)require(asset->words[j]==original_words[n][j]);}
    }
}
void exceptions(){
    // Every exact Sprite boundary, including member-array and FactionArt-array
    // transitions. A throwing Sprite never has its own destructor invoked.
    for(int failure=1;failure<=296;++failure){clean_events();throw_at=failure;
        bool caught=false;try {FactionArt*p=new FactionArt[8];delete[]p;}catch(int value){require(value==17);caught=true;}
        require(caught&&attempts==failure&&builds==failure-1&&deaths==failure-1&&copies==0&&assignments==0);
        for(int n=0;n<deaths;++n)require(destroyed[n]==built[builds-1-n]);
    }
    clean_events();FactionArt*local=new FactionArt[8];construction_order(local);delete[]local;
    require(deaths==296&&copies==0&&assignments==0);
    for(int n=0;n<296;++n)require(destroyed[n]==built[295-n]);
}
int main(){
    construction_order(FactionsArt);
    for(int n=0;n<296;++n)global_order[n]=built[n];
    publication();exceptions();clean_events();shutdown_ready=true;
}
'''


def run_host(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source, binary = tmp_path / "art.cpp", tmp_path / "art"
    source.write_text(re.sub(r"\b(" + "|".join(parts) + r")\b", lambda m: parts[m.group()], HARNESS))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                               str(source), "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=20,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_faction_art_lifetime_and_publication(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


DAMAGE = [
    ("council_member_order", "ART", "Sprite council_small;\n    Sprite council_big;", "Sprite council_big;\n    Sprite council_small;"),
    ("diplomacy_member_order", "ART", "Sprite diplomacy_logo;\n    Sprite diplomacy_landscape;", "Sprite diplomacy_landscape;\n    Sprite diplomacy_logo;"),
    ("report_array_order", "ART", "Sprite report_logo_small[3];\n    Sprite report_logo[2];", "Sprite report_logo[2];\n    Sprite report_logo_small[3];"),
    ("ctor_reassignment", "CTOR", "{}", "{ base[0][0][0] = base[0][0][1]; }"),
    ("ctor_extra_sprite", "CTOR", "{}", "{ Sprite extra; }"),
    ("dtor_reassignment", "DTOR", "{}", "{ council_small = council_big; }"),
    ("faction_count", "LOADER", "faction < MaxPlayerNum", "faction < MaxPlayerNum - 1"),
    ("terrain_count", "LOADER", "terrain < 2", "terrain < 1"),
    ("size_count", "LOADER", "size < 4", "size < 3"),
    ("defense_count", "LOADER", "defense < 3", "defense < 2"),
    ("faction_owner", "LOADER", "&FactionsArt[faction]", "&FactionsArt[faction ^ 1]"),
    ("terrain_owner", "LOADER", ".base[terrain]", ".base[terrain ^ 1]"),
    ("size_owner", "LOADER", ".base[terrain][size]", ".base[terrain][(size + 1) % 4]"),
    ("defense_owner", "LOADER", ".base[terrain][size][defense]", ".base[terrain][size][(defense + 1) % 3]"),
    ("repeat_call_skips_existing", "LOADER", "FactionBaseSprites[faction][terrain][size][defense] =", "if (!FactionBaseSprites[faction][terrain][size][defense]) FactionBaseSprites[faction][terrain][size][defense] ="),
]


@pytest.mark.parametrize("name,part,before,after", DAMAGE, ids=[row[0] for row in DAMAGE])
def test_faction_art_damage_is_observed(tmp_path, name, part, before, after):
    parts = actual_source()
    assert before in parts[part], (name, part)
    parts[part] = parts[part].replace(before, after, 1)
    result = run_host(tmp_path, parts)
    assert result.returncode == 94 and "assertion line" in result.stderr, (name, result.returncode, result.stderr)
