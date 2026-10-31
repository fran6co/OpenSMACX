"""Actual-source controls for BOOM live reads across playback callbacks."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_boom_live_window_reload_oct21 import _actual_boom_body


def _source_fragments():
    body = _actual_boom_body()

    weapon_start = body.index(
        "if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 1)")
    weapon_end = body.index(
        "if (Vehs[saved_selection].proto_id == BSC_SPORE_LAUNCHER)",
        weapon_start,
    )
    weapon = body[weapon_start:weapon_end]
    assert weapon.count(".weapon_id ==") == 14
    assert "switch (sound_weapon)" not in weapon

    latch_start = body.index("for (int frame = 0;", body.index("unsigned long last_tick"))
    latch_end = body.index("{", latch_start) + 1
    frame_latch = body[latch_start:latch_end]
    assert "animation.frame_count() >> 1" in frame_latch

    clip_start = body.index("if (flic_mode && (flags & 2) != 0)",
                            body.index("TimerHoldFlag = 1"))
    clip_end = body.index("if (!flic_mode || (flags & 2) != 0)", clip_start)
    clips = body[clip_start:clip_end]
    assert clips.count("console->set_clip(") == 2
    assert "working_buffer.set_clip(" not in clips
    return weapon, frame_latch, clips


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <vector>

struct Veh { int proto_id, x, y; };
struct VehPrototype { int weapon_id; };
Veh Vehs[4];
VehPrototype VehPrototypes[4];
std::vector<int> sound_events;
std::vector<int> clip_order;
int mutate_weapon_after_sound4;
struct FX {
    void play(int sound,int=0,int=0) {
        sound_events.push_back(sound);
        // Synthetic boundary sentinel; no claim that the pinned FX path
        // mutates weapon state in gameplay.
        if(mutate_weapon_after_sound4 && sound==4)
            VehPrototypes[Vehs[0].proto_id].weapon_id=2;
    }
} g_FX;

static int weapon_control() {
    const int second_selection=0;
    Vehs[0]={0,12,34};
    Vehs[1]={1,0,0};
    VehPrototypes[0].weapon_id=1;
    VehPrototypes[1].weapon_id=99;
    const int captured_weapon=VehPrototypes[Vehs[second_selection].proto_id].weapon_id;
    (void)captured_weapon;
    sound_events.clear();
    mutate_weapon_after_sound4=1;
    WEAPON_FRAGMENT
    return sound_events.size()==2 && sound_events[0]==4
        && sound_events[1]==0x49 ? 0 : 1;
}

struct Animation {
    int values[8];
    int value_count;
    int calls;
    int close_calls;
    Animation() : values{}, value_count(0), calls(0), close_calls(0) {}
    void set(const int *next,int count) {
        for(int i=0;i<count;++i) values[i]=next[i];
        value_count=count; calls=0;
    }
    int frame_count() {
        const int index=calls<value_count?calls:value_count-1;
        ++calls;
        return values[index];
    }
    void close() { ++close_calls; clip_order.push_back(2); }
} animation;
static int frame_control(const int *counts,int count,int expected_frames,
                         int expected_reads) {
    // Return changing and odd values at each actual loop-condition read.
    animation.set(counts,count);
    const int flic_mode=1;
    const int cached_frames=animation.values[0] >> 1;
    (void)cached_frames;
    int frame_bodies=0;
    FRAME_LATCH
        ++frame_bodies;
    }
    return frame_bodies==expected_frames && animation.calls==expected_reads
        ? 0 : 2;
}

struct RECT { int left,top,right,bottom; };
void *last_clip_receiver;
RECT *last_clip_rect;
struct Buffer {
    int close_calls;
    Buffer() : close_calls(0) {}
    void close() { ++close_calls; clip_order.push_back(1); }
    void set_clip(RECT *rect) {
        last_clip_receiver=this; last_clip_rect=rect; clip_order.push_back(3);
    }
} working_buffer;
struct GraphicWin : Buffer {
    RECT work_rect;
    RECT *buffer_working_rect() { return &work_rect; }
};
struct Console : GraphicWin {} ConsoleGlobal;
struct Flic { void close() { clip_order.push_back(2); } };
RECT saved_rectangles[8];
static int clip_control(int requested_flags,RECT *expected_rect,
                        int expect_closes) {
    const int flic_mode=1;
    const int flags=requested_flags;
    clip_order.clear(); last_clip_receiver=0; last_clip_rect=0;
    working_buffer.close_calls=0;
    animation.close_calls=0;
    CLIP_FRAGMENT
    if(last_clip_receiver!=static_cast<GraphicWin *>(&ConsoleGlobal)
       || last_clip_rect!=expected_rect) return 3;
    if(expect_closes) {
        if(clip_order.size()!=3 || clip_order[0]!=1 || clip_order[1]!=2
           || clip_order[2]!=3 || working_buffer.close_calls!=1
           || animation.close_calls!=1) return 4;
    } else if(clip_order.size()!=1 || clip_order[0]!=3
              || working_buffer.close_calls!=0 || animation.close_calls!=0) {
        return 5;
    }
    return 0;
}

int main() {
    if(weapon_control()) return 1;
    const int changing_counts[2]={6,2};
    if(frame_control(changing_counts,2,1,2)) return 2;
    const int odd_count[1]={1};
    if(frame_control(odd_count,1,0,1)) return 3;
    if(clip_control(2,ConsoleGlobal.buffer_working_rect(),1)) return 4;
    if(clip_control(1,saved_rectangles,0)) return 5;
    return 0;
}
'''


def _compile_run(tmp_path,fragments,compiler,variant):
    weapon,frame_latch,clips=fragments
    source=(HARNESS.replace("WEAPON_FRAGMENT",weapon)
            .replace("FRAME_LATCH",frame_latch)
            .replace("CLIP_FRAGMENT",clips))
    cpp=tmp_path/(variant+".cpp")
    binary=tmp_path/variant
    cpp.write_text(source,encoding="utf-8")
    built=subprocess.run([compiler,"-std=c++11","-Wall","-Wextra","-Werror",
                          str(cpp),"-o",str(binary)],capture_output=True,
                         text=True,timeout=30)
    assert built.returncode==0,f"{compiler} {variant} compile failed:\n{built.stderr}"
    return subprocess.run([str(binary)],capture_output=True,timeout=10)


def _replace_once(text,old,new):
    assert text.count(old)==1, f"unanchored mutant {old!r}"
    return text.replace(old,new,1)


def test_boom_live_state_reads_and_console_clip_receiver(tmp_path):
    compilers=[shutil.which(name) for name in ("g++","clang++")]
    compilers=[compiler for compiler in compilers if compiler]
    assert compilers
    fragments=_source_fragments()
    weapon,frame_latch,clips=fragments
    mutants=(
        ("cached-weapon-read",(
            _replace_once(weapon,
                "if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 2)",
                "if (captured_weapon == 2)"),frame_latch,clips)),
        ("cached-frame-limit",(
            weapon,_replace_once(frame_latch,"animation.frame_count() >> 1",
                                 "cached_frames"),clips)),
        ("working-buffer-first-clip",(
            weapon,frame_latch,_replace_once(clips,
                "console->set_clip(console->buffer_working_rect());",
                "working_buffer.set_clip(console->buffer_working_rect());"))),
        ("working-buffer-second-clip",(
            weapon,frame_latch,_replace_once(clips,
                "console->set_clip(saved_rectangles);",
                "working_buffer.set_clip(saved_rectangles); (void)console;"))),
    )
    for compiler in compilers:
        actual=_compile_run(tmp_path,fragments,compiler,"actual")
        assert actual.returncode==0,(
            f"{compiler}: actual source returned {actual.returncode}: "
            f"{actual.stderr.decode(errors='replace')}"
        )
        for name,mutant in mutants:
            damaged=_compile_run(tmp_path,mutant,compiler,name)
            assert damaged.returncode!=0,f"{compiler} missed {name}"
