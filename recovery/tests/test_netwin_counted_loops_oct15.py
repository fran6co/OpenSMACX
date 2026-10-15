"""Execute the current NetWin painter bodies against host-side doubles."""

from pathlib import Path
import importlib.util
import os
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
EXTRACTOR = ROOT / "recovery/tests/test_netwin_owner_behavior.py"
SPEC = importlib.util.spec_from_file_location("netwin_loop_extract", EXTRACTOR)
EXTRACT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(EXTRACT)


def _painter_bodies():
    source = (ROOT / "src/netwin.cpp").read_text()
    return {
        "BODY_NAMES": EXTRACT.extract_function(source, "void NetWin::draw_player_names()"),
        "BODY_DIFFS": EXTRACT.extract_function(source, "void NetWin::draw_diffs()"),
        "BODY_FACTIONS": EXTRACT.extract_function(source, "void NetWin::draw_factions()"),
    }


def _host_source(bodies):
    source = HOST
    for marker, body in bodies.items():
        source = source.replace(marker, body)
    return source


def _compile_and_run(source, compiler, tmp_path, stem):
    cpp = tmp_path / f"{stem}.cpp"
    exe = tmp_path / stem
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Wno-unused-parameter", str(cpp), "-o", str(exe)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ))


HOST = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>

struct RECT { int left=0, top=0, right=20, bottom=10; };
static int UnionRect(RECT *out, const RECT *a, const RECT *b) {
    out->left = a->left < b->left ? a->left : b->left;
    out->top = a->top < b->top ? a->top : b->top;
    out->right = a->right > b->right ? a->right : b->right;
    out->bottom = a->bottom > b->bottom ? a->bottom : b->bottom;
    return 1;
}

struct NetWinLobbyRow {
    uint8_t prefix_=0, ready_or_occupied_=0, field_2_=0;
    int8_t faction_or_color_=-1;
    uint8_t field_4_=0;
    char leader_name_[24]="";
    char noun_faction_[24]="";
    uint32_t owner_=0;
};
static NetWinLobbyRow NetWinLobbyRows[8];
struct NetWinFactionRecord { uint8_t record_type_=7; uint8_t pad[3]{}; uint32_t owner_=0; char gap[0x5A]{}; char noun_faction_[24]="Faction"; };
struct NetWinSessionBlock { int mode_=0; };
static NetWinSessionBlock NetWinSessionData;
struct AlphaNetProcessSlot { char name_[0x20]{}; uint32_t pid_=0; };
struct AlphaNet {
    AlphaNetProcessSlot process_slots_[8];
    struct Primary { uint32_t leading_words_[2]{}; } primary_state_;
    uint32_t pid_2_idx(uint32_t) { return 99; }
    uint32_t local_player_id() { return 1; }
    uint32_t host_player_id() { return 2; }
    char *get_player_name(uint32_t) { return const_cast<char *>("remote"); }
};
static AlphaNet NetDaemonNet;

static std::vector<int> box_indices;
static std::vector<int> spot_indices;
static std::vector<int> write_indices;
static int callback_mode = 0;
static NetWinFactionRecord *callback_records = nullptr;
static RECT *callback_rect = nullptr;
static int callback_stride = 1;
static int callbacks = 0;
struct Buffer {
    void box_sprite(RECT *r, int *) {
        for (int i=0;i<7;++i) if (r == callback_rect + i*callback_stride) box_indices.push_back(i);
        if (callbacks++ == 0) {
            if (callback_mode & 1) NetWinSessionData.mode_=3;
            if ((callback_mode & 2) && callback_records) callback_records[0].record_type_=2;
            if (callback_mode & 4) NetWinLobbyRows[7].field_2_=6;
        }
    }
    void set_font(int, int, int, int) {}
    void set_text_color(int, int, int, int) {}
    void write_cent_l(const char *, RECT *, size_t) {}
    void set_clip(RECT *) {}
    void write_l(const char *, RECT *, size_t) {}
    void restore_clip() {}
};
struct Sprite {
    int transparent_index() { return 0; }
    int height() { return 4; }
    void draw(Buffer *, int, int, int, int=1, int=1) {}
};
static int InterfaceBoxStyles[64];
static Sprite g_IFACE_LOCK_SPRITES[2], g_IFACE_MP_COMBO_ARROW_SPRITES[1];
static char *Difficulty[8] = {const_cast<char *>("D0"),const_cast<char *>("D1"),
    const_cast<char *>("D2"),const_cast<char *>("D3"),const_cast<char *>("D4"),
    const_cast<char *>("D5"),const_cast<char *>("D6"),const_cast<char *>("D7")};
static char StringStorage[1024];
static char *StringTemp=StringStorage;
struct Strings {
    char *get(char *s) {
        if (s == Difficulty[6]) write_indices.push_back(700);
        return s;
    }
} StringTable;
static char *label_get(int) { return const_cast<char *>("lobby"); }
typedef char *LPSTR;
struct LabelsBlock { char *strings_ptr[0x1060/4]{}; };
static LabelsBlock Labels;
static char *CharUpperA(char *s) { for (char *p=s;*p;++p) if (*p>='a'&&*p<='z') *p-=32; return s; }
static int find_font(int, int) { return 0; }
static int get_player_color(int) { return 1; }
static void soft_update(RECT *) {}
static char *_itoa(int value, char *out, int) { std::sprintf(out,"%d",value); return out; }
struct Spot {
    void kill_type(int) {}
    void add(int row, int, int, int, int, int) { spot_indices.push_back(row); }
};
struct FlatButton { void set_name(char *) {} void redraw() {} };
struct NetWin : Buffer {
    RECT setup_rects_[50];
    Spot spot_;
    FlatButton flatButton1_;
    int pid_idx=1;
    NetWinFactionRecord *faction_records_=nullptr;
    int get_player_color(int) { return 1; }
    void draw_player_names();
    void draw_diffs();
    void draw_factions();
};
BODY_NAMES
BODY_DIFFS
BODY_FACTIONS

static void fail(const char *message) { std::fprintf(stderr,"FAIL: %s\n",message); std::exit(1); }
static void reset() { box_indices.clear(); spot_indices.clear(); write_indices.clear(); callbacks=0; callback_mode=0; callback_records=nullptr; callback_rect=nullptr; callback_stride=1; }
static void init_rows() {
    std::memset(NetWinLobbyRows,0,sizeof(NetWinLobbyRows));
    for (int i=1;i<=7;++i) {
        NetWinLobbyRows[i].prefix_=0;
        NetWinLobbyRows[i].ready_or_occupied_=1;
        NetWinLobbyRows[i].field_2_=1;
        NetWinLobbyRows[i].faction_or_color_=-1;
        std::strcpy(NetWinLobbyRows[i].leader_name_,"Leader");
        std::strcpy(NetWinLobbyRows[i].noun_faction_,"Noun");
        NetDaemonNet.process_slots_[i].pid_=i;
    }
}
int main() {
    NetWin win;
    NetWinFactionRecord records[8]{};
    win.faction_records_=records;
    init_rows();

    // Seven records reach row 7; a drawing callback can mutate a later row
    // before that row is consumed, and a live type bound stops on reread.
    reset(); NetWinSessionData.mode_=0; win.faction_records_=nullptr;
    NetWinLobbyRows[1].prefix_=0xFF;
    callback_rect=&win.setup_rects_[20]; callback_mode=5;
    win.draw_diffs();
    if (box_indices.size()!=7 || spot_indices.size()!=6 || spot_indices[0]!=1) fail("draw_diffs omitted row 7 or lost callback-driven mode/continue");
    if (std::find(write_indices.begin(),write_indices.end(),700)==write_indices.end()) fail("draw_diffs did not read callback-updated row 7 difficulty");

    reset(); init_rows(); win.faction_records_=records; records[0].record_type_=7;
    NetWinSessionData.mode_=3; NetWinLobbyRows[1].prefix_=0xFF;
    for (int i=1;i<=7;++i) NetWinLobbyRows[i].prefix_=0xFF;
    NetWinLobbyRows[2].prefix_=0;
    callback_rect=&win.setup_rects_[13]; callback_mode=2; callback_records=records;
    win.draw_factions();
    if (box_indices.size()!=2) fail("draw_factions did not reread callback-mutated record bound");
    if (spot_indices.size()!=1 || spot_indices[0]!=1) {
        std::fprintf(stderr,"faction spots=%zu first=%d\n",spot_indices.size(),
                     spot_indices.empty() ? -1 : spot_indices[0]);
        fail("draw_factions lost prefix continue/order");
    }

    reset(); init_rows(); win.faction_records_=nullptr; NetWinSessionData.mode_=0;
    callback_rect=&win.setup_rects_[13];
    win.draw_factions();
    if (box_indices.size()!=7 || spot_indices.size()!=7) fail("draw_factions omitted row 7");

    reset(); init_rows(); win.faction_records_=records; records[0].record_type_=7;
    NetWinSessionData.mode_=3; NetWinLobbyRows[1].prefix_=0xFF;
    for (int i=1;i<=7;++i) NetWinLobbyRows[i].prefix_=0xFF;
    callback_rect=&win.setup_rects_[12]; callback_mode=2; callback_records=records;
    callback_stride=3;
    win.draw_player_names();
    if (box_indices.size()!=2) fail("draw_player_names did not reread callback-mutated record bound");
    if (!spot_indices.empty()) fail("draw_player_names prefix continue was not preserved");

    reset(); init_rows(); win.faction_records_=nullptr; NetWinSessionData.mode_=0;
    NetDaemonNet.primary_state_.leading_words_[0]=1;
    for (int i=1;i<=7;++i) NetWinLobbyRows[i].field_2_=1;
    callback_rect=&win.setup_rects_[12]; callback_mode=3;
    callback_stride=3;
    win.draw_player_names();
    if (box_indices.size()!=7) fail("draw_player_names omitted row 7");
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_extracted_painters_execute_seven_rows_and_live_controls(
    compiler_name, tmp_path
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    run = _compile_and_run(_host_source(_painter_bodies()), compiler, tmp_path,
                           "netwin-counted-painters")
    assert run.returncode == 0, run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(
    ("method", "marker", "loop_head"),
    [
        ("draw_diffs", "BODY_DIFFS", "for (int index = 0; index < 7;"),
        ("draw_factions", "BODY_FACTIONS", "for (int index = 0; index < 7;"),
        ("draw_player_names", "BODY_NAMES", "for (int row = 0; row < 7;"),
    ],
)
def test_compiled_six_row_mutants_fail_runtime(
    compiler_name, method, marker, loop_head, tmp_path
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = _painter_bodies()
    assert loop_head in bodies[marker]
    bodies[marker] = bodies[marker].replace(loop_head, loop_head.replace("< 7", "< 6"), 1)
    run = _compile_and_run(_host_source(bodies), compiler, tmp_path,
                           f"netwin-short-{method}")
    assert run.returncode != 0
    assert "FAIL:" in run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compiled_hoisted_record_count_mutant_fails_runtime(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = _painter_bodies()
    body = bodies["BODY_FACTIONS"]
    live_check = (
        "if (NetWinSessionData.mode_ == 3 && faction_records_ != 0\n"
        "            && index >= faction_records_[0].record_type_)"
    )
    assert live_check in body
    body = body.replace(
        "    for (int index = 0; index < 7;",
        "    const int record_type_snapshot = faction_records_ != 0\n"
        "        ? faction_records_[0].record_type_ : 0;\n"
        "    for (int index = 0; index < 7;",
        1,
    )
    body = body.replace(
        live_check,
        "if (NetWinSessionData.mode_ == 3 && faction_records_ != 0\n"
        "            && index >= record_type_snapshot)",
        1,
    )
    assert body != bodies["BODY_FACTIONS"]
    bodies["BODY_FACTIONS"] = body
    run = _compile_and_run(_host_source(bodies), compiler, tmp_path,
                           "netwin-hoisted-record-count")
    assert run.returncode != 0
    assert "draw_factions did not reread callback-mutated record bound" in run.stderr
