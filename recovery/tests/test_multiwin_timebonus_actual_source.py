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


# 479330 is a separate original caller. After its first negative skip,
# TEST4794AF / MOV4794B1 / JE4794B3 skip zero too. Both methods retain
# their positive-only policy. The rejected zero-inclusive oracle is preserved
# only in ignored historical evidence. After
# draw_time479417,47941C..43F reload the sprite height and DIB width and rebuild
# all four rectangle fields. The service seam below poisons both the rectangle
# and these live owner fields; it does not assert a native host ABI or that a
# particular production draw_time invocation deliberately changes them.
def _draw_definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated actual drawing definition")


def _complete_draw_method():
    return _draw_definition((ROOT / "src/multiwin.cpp").read_text(),
                            "void MultiWin::draw(int draw_type)")


FULL_DRAW_HARNESS = r'''
#include <stdint.h>
#include <limits.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
typedef char *LPSTR;
struct RECT { int left, top, right, bottom; };
struct Font { int unused; };
class Buffer;
struct DrawWrite { Buffer *owner; std::string text; RECT rect; };
static std::vector<DrawWrite> draw_writes;
static std::vector<RECT> draw_clips, draw_spots;
static std::vector<int> draw_kills;
static int draw_time_calls, draw_soft_updates;
static std::string draw_scratch_after_bonus;
static const char *draw_current_scratch();
static bool poison_draw_time;
static void draw_require(bool ok, const char *why) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", why); std::abort(); }
}
class Buffer {
public:
    int marker;
    int write_cent_l(LPSTR, RECT *);
    int write_cent_l(char *text, RECT *rect, int length) {
        draw_require(length == (int)std::strlen(text), "actual wrapper length");
        DrawWrite value; value.owner=this; value.text=text; value.rect=*rect;
        draw_writes.push_back(value); return 23;
    }
    int write_cent_l(char *, int, int, int, unsigned long) { return 0; }
    int set_clip(RECT *rect) {
        draw_clips.push_back(*rect);
        if (draw_clips.size()==2) draw_scratch_after_bonus=draw_current_scratch();
        return 0;
    }
    void set_font(Font *, Font *, Font *, Font *) {}
    void set_text_color(int, int, int, int) {}
};
struct GraphicPrefix { char bytes[0x444]; };
class GraphicWin : public GraphicPrefix, public Buffer {
public:
    struct Dib { struct Header { int biWidth; } bmiHeader; } dib_;
    void fill(int color) { draw_require(color==9, "panel fill"); }
    void soft_update() { ++draw_soft_updates; }
};
struct Sprite {
    int height_value;
    int height() const { return height_value; }
    int transparent_index() const { return 0; }
    void draw(Buffer *, int, int, int) {}
};
struct Spot {
    void kill_type(int type) { draw_kills.push_back(type); }
    int add(int, int, RECT *);
    int add(int position, int type, int x, int y, int width, int height) {
        draw_require(position==3 && type==2, "bonus spot contract");
        RECT value={x,y,x+width,y+height}; draw_spots.push_back(value); return 19;
    }
};
struct PlayerState { int time_bonus_count; unsigned diplo_treaties[8]; };
static PlayerState PlayersData[8];
struct PlayerInfo { unsigned rule_flags; };
static PlayerInfo Players[8];
struct BaseInfo { int faction_id_current; };
static BaseInfo Bases[1];
struct ConsoleState { int faction_id_; };
static ConsoleState ConsoleGlobal;
struct GameFields {
    struct Masks { struct Bytes { unsigned char alive_mask_, human_mask_; } bytes_; } lock_masks_;
    struct Projects { int empath_guild() const { return -1; } } secret_project_;
    unsigned game_state_;
};
struct GameState { GameFields fields_; };
static GameState GameDataStateStorage;
static int IsMultiplayerNet, MultiWindowRedrawSuppressed;
static int MessageWinTextColor, MessageWinTextBackground;
static char StringTemp[2048];
static const char *draw_current_scratch() { return StringTemp; }
static char draw_late_label[]="late";
static char *label_get(int) { return draw_late_label; }
static Font draw_font;
static Font *find_font(int, int) { return &draw_font; }
static int council_votes(int) { return 1; }
static int auto_contact() { return 0; }
static char *_itoa(int value, char *out, int base) {
    draw_require(base==10, "number radix"); std::sprintf(out,"%d",value); return out;
}
class MultiWin : public GraphicWin {
public:
    Spot spot_;
    Sprite sprite3_, sprite4_, sprite5_, sprite6_, sprite7_, sprites1_[3];
    Font font1_;
    unsigned panel_open_;
    int val_2_, hovered_spot_;
    RECT rect2_;
    void draw(int);
    void draw_player(int, int, RECT *) {}
    void draw_time(RECT *rect, int type) {
        ++draw_time_calls;
        draw_require(type==5 && rect->left==0 && rect->top==57 &&
                     rect->right==100 && rect->bottom==70,
                     "pre-callback rectangle snapshot");
        if (poison_draw_time) {
            sprite4_.height_value=31; dib_.bmiHeader.biWidth=173;
            rect->left=-101; rect->top=-202; rect->right=-303; rect->bottom=-404;
        }
    }
};
/* ACTUAL_COMPLETE_DRAW_AND_CONVENIENCE_BODIES */
static bool equal_rect(const RECT &a, const RECT &b) {
    return a.left==b.left && a.top==b.top && a.right==b.right && a.bottom==b.bottom;
}
static void check_draw(int bonus, const char *bonus_text, bool poison) {
    MultiWin window;
    window.panel_open_=0; window.hovered_spot_=-1; window.val_2_=50;
    window.sprite3_.height_value=7; window.sprite4_.height_value=13;
    window.sprite5_.height_value=6; window.sprite6_.height_value=6;
    window.sprite7_.height_value=4;
    window.dib_.bmiHeader.biWidth=100;
    window.rect2_.left=0; window.rect2_.top=50;
    window.rect2_.right=100; window.rect2_.bottom=200;
    IsMultiplayerNet=1; MultiWindowRedrawSuppressed=1;
    ConsoleGlobal.faction_id_=2; PlayersData[2].time_bonus_count=bonus;
    draw_writes.clear(); draw_clips.clear(); draw_spots.clear(); draw_kills.clear();
    draw_time_calls=draw_soft_updates=0; poison_draw_time=poison;
    draw_scratch_after_bonus="not observed";
    std::strcpy(StringTemp,"old scratch");
    window.draw(5);
    draw_require(draw_time_calls==1 && draw_soft_updates==0, "callback/update paths");
    draw_require(draw_kills.size()==3 && draw_kills[0]==2 &&
                 draw_kills[1]==0 && draw_kills[2]==1, "spot kill order");
    RECT row={0,poison?88:70,poison?173:100,poison?102:84};
    draw_require(draw_spots.size()==1 && equal_rect(draw_spots[0],row) &&
                 draw_clips.size()==2 && equal_rect(draw_clips[0],row),
                 "post-draw_time live rectangle");
    draw_require(draw_writes.size()==(bonus_text?2u:1u), "draw bonus write count");
    draw_require(draw_scratch_after_bonus==(bonus_text?bonus_text:"old scratch"),
                 "negative/zero scratch untouched");
    if (bonus_text) {
        draw_require(draw_writes[0].text==bonus_text, "positive bonus bytes");
        draw_require(equal_rect(draw_writes[0].rect,row), "bonus original rectangle");
        draw_require(draw_writes[0].owner==static_cast<Buffer *>(&window),
                     "bonus secondary-base receiver");
    }
    draw_require(draw_writes.back().text=="late", "following empty-panel output");
}
int main() {
    for (int poison=0; poison!=2; ++poison) {
        check_draw(INT_MIN,0,poison!=0); check_draw(-1,0,poison!=0);
        check_draw(0,0,poison!=0); check_draw(1," ^",poison!=0);
        check_draw(3," ^^^",poison!=0); check_draw(4," ^^^",poison!=0);
        check_draw(INT_MAX," ^^^",poison!=0);
    }
    return 0;
}
'''


def _full_draw_source(method):
    from match_recovery.source import read_file
    owner, = [r for r in read_file(ROOT / "src/buffer.cpp") if r.address == 0x47A720]
    text = (ROOT / owner.body).read_text() if owner.body else owner.region
    helper = _draw_definition(text, "int Buffer::write_cent_l(LPSTR text, RECT *rect)")
    spot = _draw_definition((ROOT / "src/spot.cpp").read_text(),
                            "int Spot::add(int position, int type, RECT *rect)")
    return FULL_DRAW_HARNESS.replace(
        "/* ACTUAL_COMPLETE_DRAW_AND_CONVENIENCE_BODIES */",
        helper + "\n" + spot + "\n" + method)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_draw_preserves_zero_bonus_and_live_callback_geometry(compiler, tmp_path):
    if shutil.which(compiler) is None:
        pytest.skip(f"{compiler} unavailable")
    result = compile_run(compiler, _full_draw_source(_complete_draw_method()), tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["zero", "height", "width"])
def test_actual_draw_read_phase_damage_is_rejected(compiler, tmp_path, damage):
    if shutil.which(compiler) is None:
        pytest.skip(f"{compiler} unavailable")
    method = _complete_draw_method()
    if damage == "zero":
        old, new = "if (time_bonus > 0)", "if (time_bonus >= 0)"
    elif damage == "height":
        old, new = "y += sprite4_.height();", "y = rect.bottom;"
    else:
        old = "y += sprite4_.height();\n    rect.left = 0;\n    rect.top = y;\n    rect.right = dib_.bmiHeader.biWidth;"
        new = "y += sprite4_.height();\n    rect.left = 0;\n    rect.top = y;\n    rect.right = rect.right;"
    assert method.count(old) == 1
    result = compile_run(compiler, _full_draw_source(method.replace(old, new, 1)), tmp_path)
    assert result.returncode != 0
    assert "FAIL: draw bonus write count" in result.stderr or "FAIL: post-draw_time live rectangle" in result.stderr
