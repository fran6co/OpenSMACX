"""Host controls for the complete current MultiWin owner method."""
from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]



def proposal_method() -> str:
    text = (ROOT / "src/multiwin.cpp").read_text()
    signature = "void MultiWin::draw_time_bonus_markers(RECT *rect) {"
    start = text.index(signature)
    depth = 0
    for pos in range(text.index("{", start), len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("unterminated migrated product method")


HARNESS = r'''
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <stddef.h>

struct RECT { int left, top, right, bottom; };
struct Spot {
    int calls, args[6];
    void add(int a, int b, int c, int d, int e, int f) {
        ++calls; args[0]=a; args[1]=b; args[2]=c;
        args[3]=d; args[4]=e; args[5]=f;
    }
};
struct PlayerData { unsigned char prefix[0x2054]; int time_bonus_count; };
typedef char player_data_field_offset_check[
    (offsetof(PlayerData, time_bonus_count) == 0x2054) ? 1 : -1];
struct ConsoleState { int faction_id_; };
static ConsoleState ConsoleGlobal;
static PlayerData PlayersData[8];
static char StringTemp[1032];
static char JoinSpace[] = " ";
static char Caret[] = "^";
#define BufferJoinSpace JoinSpace
#define CaretSeparator Caret

class GraphicWin { public: char prefix_[0x444]; };
class Buffer {
public:
    int writes, seen_length;
    RECT *seen_rect;
    Buffer *seen_receiver;
    char seen_text[32];
    Buffer() : writes(0), seen_length(-1), seen_rect(0),
               seen_receiver(0) { seen_text[0]=0; }
    int write_cent_l(char *text, RECT *rect, int length) {
        ++writes; seen_receiver=this; seen_rect=rect; seen_length=length;
        strcpy(seen_text, text); return 0x5678;
    }
};
class MultiWin : public GraphicWin, public Buffer {
public:
    Spot spot_;
    void draw_time_bonus_markers(RECT *rect);
};
static void require(bool ok, const char *name) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", name); std::abort(); }
}
'''


def translation_unit(method: str) -> str:
    return HARNESS + "\n" + method + r'''
static void reset(MultiWin &window, int bonus) {
    window.spot_.calls=0;
    window.writes=0;
    window.seen_length=-1;
    window.seen_rect=0;
    window.seen_text[0]=0;
    StringTemp[0]='Q'; // Negative/zero paths must not clear it.
    ConsoleGlobal.faction_id_=2;
    PlayersData[2].time_bonus_count=bonus;
}
static void verify_marker(MultiWin &window, RECT &area, int bonus,
                          const char *expected) {
    reset(window, bonus);
    window.draw_time_bonus_markers(&area);
    require(window.spot_.calls==1, "spot append occurs exactly once");
    int expected_args[6]={3,2,area.left,area.top,
                          area.right-area.left,area.bottom-area.top};
    for (int i=0;i<6;++i) require(window.spot_.args[i]==expected_args[i], "spot arguments");
    const bool should_write=bonus>0;
    require(window.writes==(should_write?1:0), "write branch count");
    if (should_write) {
        require(strcmp(window.seen_text,expected)==0, "caret text bytes");
        require(window.seen_length==(int)strlen(expected), "strlen passed unchanged");
        require(window.seen_rect==&area, "original RECT pointer passed");
        require(window.seen_receiver==static_cast<Buffer *>(&window),
                "ordinary secondary-base receiver adjustment");
    } else {
        require(StringTemp[0]=='Q', "non-write path leaves shared scratch untouched");
    }
}
int main() {
    MultiWin window; RECT area={-9,4,37,29};
    reset(window,-1); window.draw_time_bonus_markers(&area);
    require(window.spot_.calls==1 && window.writes==0 && StringTemp[0]=='Q',
            "negative count: hotspot only, no scratch clear or text write");
    verify_marker(window,area,1," ^");
    verify_marker(window,area,3," ^^^");
    reset(window,4); window.draw_time_bonus_markers(&area);
    require(window.writes==1 && strcmp(window.seen_text," ^^^")==0,
            "counts above three saturate to three markers");
    reset(window,0); window.draw_time_bonus_markers(&area);
    require(window.spot_.calls==1 && window.writes==0 && StringTemp[0]=='Q',
            "zero count: hotspot only, no scratch clear or text write");
    return 0;
}
'''


def compile_run(compiler: str, source: str, tmp_path: Path) -> subprocess.CompletedProcess[str]:
    src = tmp_path / "multwin.cpp"
    exe = tmp_path / "multwin"
    src.write_text(source)
    cmd = [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
           "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(src), "-o", str(exe)]
    built = subprocess.run(cmd, text=True, capture_output=True)
    assert built.returncode == 0, built.stdout + built.stderr
    env = dict(os.environ)
    # LeakSanitizer cannot inspect this provider's ptraced child; retain ASan
    # and UBSan while disabling only its unsupported leak phase.
    env["ASAN_OPTIONS"] = "detect_leaks=0"
    return subprocess.run([str(exe)], text=True, capture_output=True, env=env)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_complete_candidate_method_preserves_original_branches_and_receivers(compiler, tmp_path):
    if shutil.which(compiler) is None:
        pytest.skip(f"{compiler} unavailable")
    method = proposal_method()
    assert "PlayersData[ConsoleGlobal.faction_id_].time_bonus_count" in method
    assert "spot_.add(3, 2, left, top, width, height)" in method
    assert "static_cast<Buffer *>(this)->write_cent_l(StringTemp, rect, len)" in method
    assert "reinterpret_cast" not in method
    source = translation_unit(method)
    # The method's ordinary C++ base conversion must select the second Buffer
    # base, not a forged address. Have the fake write method expose `this`.
    result = compile_run(compiler, source, tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_cap_mutation_is_killed_by_three_marker_boundary(compiler, tmp_path):
    if shutil.which(compiler) is None:
        pytest.skip(f"{compiler} unavailable")
    method = proposal_method().replace("(time_bonus > 3) ? 3", "(time_bonus > 2) ? 2")
    source = translation_unit(method)
    # Strengthen the boundary check to require exactly three caret bytes for 3.
    built = compile_run(compiler, source, tmp_path)
    assert built.returncode != 0
    assert "FAIL: caret text bytes" in built.stderr
