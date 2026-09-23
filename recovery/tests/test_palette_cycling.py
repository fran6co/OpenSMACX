"""Actual load_cycling parser/Palette interaction, not working-cycle proof.

The whole loader and mutable OceanDetailShade initializer are extracted from
product source unchanged. Scripted parser and Palette calls independently
observe original 455400..4554C0 sequencing and arguments. Native original-data
matching is separate; these host controls do not validate init_cycle internals,
timers, real parser state, native ABI or game behavior.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def actual_source():
    source = (ROOT / "src/game.cpp").read_text()
    initializer, = re.findall(
        r"\bint\s+OceanDetailShade\s*\[80\]\s*=\s*\{[^}]*\}\s*;", source)
    start = source.index("void __cdecl load_cycling() {")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return initializer, source[start:end + 1]
    raise AssertionError("unterminated actual loader")


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
void check(bool pass,int line) {
    if (!pass) { std::fprintf(stderr,"assertion line %d\n",line); std::exit(73); }
}
#define require(test) check((test),__LINE__)
struct Step {
    char kind;
    std::string text;
    bool null_text;
    int value, key, first, count;
    unsigned long interval;
};
std::vector<Step> script;
size_t cursor;
Step &take(char kind) {
    require(cursor<script.size());
    Step &step=script[cursor++];
    require(step.kind==kind);
    return step;
}
int text_open(const char *file,const char *section) {
    Step &step=take('O');
    require(std::string(file)=="system" && section==step.text);
    return step.value;
}
char *text_get() {
    Step &step=take('G');
    return step.null_text ? nullptr : const_cast<char *>(step.text.c_str());
}
int text_item_number() { return take('N').value; }
struct Palette {
    int init_cycle(int key,int first,int count,unsigned long interval) {
        Step &step=take('I');
        require(key==step.key && first==step.first && count==step.count && interval==step.interval);
        return step.value;
    }
    int start_cycle(int key) {
        Step &step=take('S'); require(key==step.key); return step.value;
    }
} g_PALETTE1;
ACTUAL_INITIALIZER
static_assert(sizeof(OceanDetailShade)==80*sizeof(int),"real initializer bounds");
ACTUAL_BODY
int initial[80],expected[80];
void open(const char *section,int result=0) {
    Step s={};s.kind='O';s.text=section;s.value=result;script.push_back(s);
}
void line(const char *text) {
    Step s={};s.kind='G';s.null_text=!text;if(text)s.text=text;script.push_back(s);
}
void number(int value) {
    Step s={};s.kind='N';s.value=value;script.push_back(s);
}
void cycle(int key,int first,int count,unsigned long interval,int result) {
    Step s={};s.kind='I';s.key=key;s.first=first;s.count=count;
    s.interval=interval;s.value=result;script.push_back(s);
    s.kind='S';s.value=9;script.push_back(s);
}
void begin(bool restore=true) {
    script.clear();cursor=0;
    if(restore) std::memcpy(OceanDetailShade,initial,sizeof(initial));
    std::memcpy(expected,OceanDetailShade,sizeof(expected));
}
void finish() {
    load_cycling();
    require(cursor==script.size());
    require(std::memcmp(OceanDetailShade,expected,sizeof(expected))==0);
}
void no_cycles(int count=0) {
    open("cycling");line("count");number(count);
}
int main() {
    std::memcpy(initial,OceanDetailShade,sizeof(initial));
    // Failures are status !=0, not truthiness of a pointer. Cycling failure
    // prevents even attempting the independent ocean section.
    for(int error : {-1,1,7}) {begin();open("cycling",error);finish();}
    for(int count : {0,-1,-2147483647-1}) {
        begin();no_cycles(count);open("oceanoffsets",1);finish();
    }
    // Original parser order is a count line then three sequential numbers
    // per cycle line. Failed initialization does NOT suppress start_cycle.
    begin();open("cycling");line("count");number(3);
    line("row0");number(12);number(7);number(45);cycle(0,22,7,45,0);
    line("row1");number(2147483647);number(-3);number(-1);
    cycle(1,-2147483639,-3,4294967295ul,1);
    line("row2");number(-12);number(0);number(-2147483647-1);
    cycle(2,-2,0,2147483648ul,-7);
    open("oceanoffsets",2);finish();

    // ASCII first-byte gate, not numeric-string recognition. Only accepted
    // lines call text_item_number; their parsed values are not clamped.
    begin();no_cycles();open("oceanoffsets");
    for(const char *text : {"", "+7", "-3", " 8", "\t9", "/", ":", "A", "\x80"}) line(text);
    line("0suffix");number(-100);expected[0]=-100;
    line("9suffix");number(2147483647);expected[1]=2147483647;
    line("5");number(-2147483647-1);expected[2]=-2147483647-1;
    line(nullptr);finish();

    // Reload updates only the accepted prefix; the old suffix survives.
    begin(false);no_cycles();open("oceanoffsets");
    line("2");number(777);expected[0]=777;line(nullptr);finish();
    begin(false);no_cycles();open("oceanoffsets",-1);finish();
    begin(false);no_cycles();open("oceanoffsets");line(nullptr);finish();

    // 80 accepted rows, not 80 reads. The original fetches one extra line
    // after the last write, then checks the capacity before parsing it.
    for(bool extra : {false,true}) {
        begin();no_cycles();open("oceanoffsets");
        for(int i=0;i<80;++i) {
            if(i%3==0) line("skip");
            line("1");number(1000+i);expected[i]=1000+i;
        }
        line(extra ? "9extra-not-parsed" : nullptr);finish();
    }
    return 0;
}
'''


def run_host(tmp_path, initializer, body):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    source = tmp_path / "cycling.cpp"
    source.write_text(HARNESS.replace("ACTUAL_INITIALIZER", initializer)
                      .replace("ACTUAL_BODY", body))
    binary = tmp_path / "cycling"
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=10, env=env)


def test_actual_palette_cycling_loader(tmp_path):
    result = run_host(tmp_path, *actual_source())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ('text_open("system", "cycling") != 0', 'text_open("system", "cycling") == 0'),
    ('const int count = text_item_number();',
     'const int count = text_item_number(); if (count <= 0) return;'),
    ('cycle < count', 'cycle <= count'),
    ('first) + 10u', 'first) + 9u'),
    ('const int first = text_item_number();\n        const int colors = text_item_number();',
     'const int colors = text_item_number();\n        const int first = text_item_number();'),
    ('static_cast<unsigned int>(text_item_number())',
     'static_cast<unsigned long>(text_item_number())'),
    ('g_PALETTE1.init_cycle(cycle, palette_first, colors, interval);',
     'if (g_PALETTE1.init_cycle(cycle, palette_first, colors, interval) != 0) continue;'),
    ('g_PALETTE1.start_cycle(cycle);', ';'),
    ('text_open("system", "oceanoffsets") != 0', 'text_open("system", "oceanoffsets") == 0'),
    ("*line >= '0'", "*line > '0'"),
    ("*line <= '9'", "*line < '9'"),
    ("*line >= '0' && *line <= '9'", "(*line >= '0' && *line <= '9') || *line == '+'"),
    ('OceanDetailShade[offset++]', 'OceanDetailShade[offset]'),
    ('offset >= 80', 'offset >= 79'),
    ('OceanDetailShade[offset++] = text_item_number();',
     'OceanDetailShade[offset++] = text_item_number(); if (offset == 80) break;'),
    ('int offset = 0;', 'int offset = 0; std::memset(OceanDetailShade,0,sizeof(OceanDetailShade));'),
])
def test_palette_loader_damage_is_observed(tmp_path, before, after):
    initializer, body = actual_source()
    assert before in body
    body = body.replace(before, after)
    result = run_host(tmp_path, initializer, body)
    assert result.returncode == 73, result.stderr
