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
static struct NetWinLobbyRowsMock { NetWinLobbyRow rows_[8]; } NetWinLobbyRows;
struct NetWinFactionRecord { uint8_t record_type_=7; uint8_t pad[3]{}; uint32_t owner_=0; char gap[0x5A]{}; char noun_faction_[24]="Faction"; };
struct NetWinSessionBlock { int mode_=0; };
static struct NetWinSessionDataMock { NetWinSessionBlock fields_; } NetWinSessionData;
static RECT *rect_to_mutate_during_pid = nullptr;
static bool mutate_rect_on_pid = false;
static int forced_pid_index = 99;
struct AlphaNetProcessSlot { char name_[0x20]{}; uint32_t pid_=0; };
struct AlphaNet {
    AlphaNetProcessSlot process_slots_[8];
    struct Primary { uint32_t leading_words_[2]{}; } primary_state_;
    uint32_t pid_2_idx(uint32_t) {
        if (mutate_rect_on_pid && rect_to_mutate_during_pid) {
            rect_to_mutate_during_pid->left=30;
            rect_to_mutate_during_pid->top=40;
            rect_to_mutate_during_pid->right=90;
            rect_to_mutate_during_pid->bottom=80;
            mutate_rect_on_pid=false;
        }
        return forced_pid_index;
    }
    uint32_t local_player_id() { return 1; }
    uint32_t host_player_id() { return 2; }
    char *get_player_name(uint32_t) { return const_cast<char *>("remote"); }
};
static AlphaNet NetDaemonNet;

static std::vector<int> box_indices;
static std::vector<int> spot_indices;
static std::vector<int> write_indices;
static std::vector<RECT> clipped_rects;
static std::vector<RECT> spot_rects;
static std::vector<int> arrow_x;
static std::vector<int> arrow_y;
static int callback_mode = 0;
static NetWinFactionRecord *callback_records = nullptr;
static RECT *callback_rect = nullptr;
static int callback_stride = 1;
static int callbacks = 0;
static int label_get_calls = 0;
static bool vary_label_lookup = false;
struct Buffer {
    void box_sprite(RECT *r, int *) {
        for (int i=0;i<7;++i) if (r == callback_rect + i*callback_stride) box_indices.push_back(i);
        if (callbacks++ == 0) {
            if (callback_mode & 1) NetWinSessionData.fields_.mode_=3;
            if ((callback_mode & 2) && callback_records) callback_records[0].record_type_=2;
            if (callback_mode & 4) NetWinLobbyRows.rows_[7].field_2_=6;
        }
    }
    void set_font(int, int, int, int) {}
    void set_text_color(int, int, int, int) {}
    void write_cent_l(const char *, RECT *, size_t) {}
    void set_clip(RECT *r) { clipped_rects.push_back(*r); }
    void write_l(const char *, RECT *, size_t) {}
    void restore_clip() {}
};
struct Sprite {
    int transparent_index() { return 0; }
    int height() { return 4; }
    void draw(Buffer *, int, int x, int y, int=1, int=1) {
        arrow_x.push_back(x); arrow_y.push_back(y);
    }
};
static int InterfaceBoxStyles[64];
static Sprite g_IFACE_LOCK_SPRITES[2], g_IFACE_MP_COMBO_ARROW_SPRITES[1];
// The physical backing covers both the original signed-byte index and the
// wrong unsigned interpretation. It adds no product table bounds or clamp.
static char *DifficultyBacking[384];
static char **Difficulty = DifficultyBacking + 128;
static char *DifficultyNames[8] = {const_cast<char *>("D0"),const_cast<char *>("D1"),
    const_cast<char *>("D2"),const_cast<char *>("D3"),const_cast<char *>("D4"),
    const_cast<char *>("D5"),const_cast<char *>("D6"),const_cast<char *>("D7")};
static std::vector<char *> difficulty_inputs;
static char StringStorage[1024];
static char *StringTemp=StringStorage;
struct Strings {
    char *get(char *s) {
        difficulty_inputs.push_back(s);
        if (s == Difficulty[6]) write_indices.push_back(700);
        return s;
    }
} StringTable;
static char *label_get(int) {
    ++label_get_calls;
    return const_cast<char *>(vary_label_lookup && label_get_calls == 2 ? "Noun" : "Lobby");
}
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
    void add(int row, int, int x, int y, int width, int height) {
        spot_indices.push_back(row);
        RECT r; r.left=x; r.top=y; r.right=x+width; r.bottom=y+height;
        spot_rects.push_back(r);
    }
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
static void reset() { box_indices.clear(); spot_indices.clear(); write_indices.clear(); clipped_rects.clear(); spot_rects.clear(); arrow_x.clear(); arrow_y.clear(); callbacks=0; label_get_calls=0; vary_label_lookup=false; callback_mode=0; callback_records=nullptr; callback_rect=nullptr; callback_stride=1; rect_to_mutate_during_pid=nullptr; mutate_rect_on_pid=false; forced_pid_index=99;
    difficulty_inputs.clear();
    std::fill(DifficultyBacking, DifficultyBacking + 384,
              const_cast<char *>("unsigned-index-backing"));
    for (int i=0;i!=8;++i) Difficulty[i]=DifficultyNames[i];
    Difficulty[-128]=const_cast<char *>("signed-minus128");
    Difficulty[-2]=const_cast<char *>("signed-minus2");
}
static void init_rows() {
    std::memset(NetWinLobbyRows.rows_,0,sizeof(NetWinLobbyRows.rows_));
    for (int i=1;i<=7;++i) {
        NetWinLobbyRows.rows_[i].prefix_=0;
        NetWinLobbyRows.rows_[i].ready_or_occupied_=1;
        NetWinLobbyRows.rows_[i].field_2_=1;
        NetWinLobbyRows.rows_[i].faction_or_color_=-1;
        std::strcpy(NetWinLobbyRows.rows_[i].leader_name_,"Leader");
        std::strcpy(NetWinLobbyRows.rows_[i].noun_faction_,"Noun");
        NetDaemonNet.process_slots_[i].pid_=i;
    }
}
int main() {
    NetWin win;
    NetWinFactionRecord records[8]{};
    win.faction_records_=records;
    init_rows();

    // A null record pointer is not read outside mode 3.
    reset(); NetWinSessionData.fields_.mode_=2; win.faction_records_=nullptr;
    callback_rect=&win.setup_rects_[20];
    win.draw_diffs();
    if (box_indices.size()!=7 || spot_indices.size()!=7)
        fail("draw_diffs read a null record pointer outside mode 3");

    // Seven records reach row 7; a drawing callback can switch mode and
    // mutate a later row before it is consumed, and the live bound is reread.
    reset(); NetWinSessionData.fields_.mode_=0; win.faction_records_=records;
    records[0].record_type_=7;
    NetWinLobbyRows.rows_[1].prefix_=0xFF;
    callback_rect=&win.setup_rects_[20]; callback_mode=5;
    win.draw_diffs();
    if (box_indices.size()!=7 || spot_indices.size()!=6 || spot_indices[0]!=1) fail("draw_diffs omitted row 7 or lost callback-driven mode/continue");
    if (std::find(write_indices.begin(),write_indices.end(),700)==write_indices.end()) fail("draw_diffs did not read callback-updated row 7 difficulty");

    reset(); init_rows(); win.faction_records_=records; records[0].record_type_=7;
    NetWinSessionData.fields_.mode_=3; NetWinLobbyRows.rows_[1].prefix_=0xFF;
    for (int i=1;i<=7;++i) NetWinLobbyRows.rows_[i].prefix_=0xFF;
    NetWinLobbyRows.rows_[2].prefix_=0;
    callback_rect=&win.setup_rects_[13]; callback_mode=2; callback_records=records;
    win.draw_factions();
    if (box_indices.size()!=2) fail("draw_factions did not reread callback-mutated record bound");
    if (spot_indices.size()!=1 || spot_indices[0]!=1) {
        std::fprintf(stderr,"faction spots=%zu first=%d\n",spot_indices.size(),
                     spot_indices.empty() ? -1 : spot_indices[0]);
        fail("draw_factions lost prefix continue/order");
    }

    reset(); init_rows(); win.faction_records_=nullptr; NetWinSessionData.fields_.mode_=0;
    callback_rect=&win.setup_rects_[13];
    win.draw_factions();
    if (box_indices.size()!=7 || spot_indices.size()!=7) fail("draw_factions omitted row 7");

    reset(); init_rows(); win.faction_records_=records; records[0].record_type_=7;
    NetWinSessionData.fields_.mode_=3; NetWinLobbyRows.rows_[1].prefix_=0xFF;
    for (int i=1;i<=7;++i) NetWinLobbyRows.rows_[i].prefix_=0xFF;
    callback_rect=&win.setup_rects_[12]; callback_mode=2; callback_records=records;
    callback_stride=3;
    win.draw_player_names();
    if (box_indices.size()!=2) fail("draw_player_names did not reread callback-mutated record bound");
    if (!spot_indices.empty()) fail("draw_player_names prefix continue was not preserved");

    reset(); init_rows(); win.faction_records_=records; records[0].record_type_=7;
    NetWinSessionData.fields_.mode_=0;
    NetDaemonNet.primary_state_.leading_words_[0]=1;
    for (int i=1;i<=7;++i) NetWinLobbyRows.rows_[i].field_2_=1;
    callback_rect=&win.setup_rects_[12]; callback_mode=3;
    callback_stride=3;
    win.draw_player_names();
    if (box_indices.size()!=7) fail("draw_player_names omitted row 7");

    // The arrow geometry is captured before pid_2_idx; text clipping and the
    // later spot registration use the live rectangle after that helper.
    reset(); init_rows(); win.faction_records_=records; records[0].record_type_=1;
    NetWinSessionData.fields_.mode_=3; NetWinLobbyRows.rows_[1].prefix_=0;
    win.setup_rects_[20].left=10; win.setup_rects_[20].top=20;
    win.setup_rects_[20].right=50; win.setup_rects_[20].bottom=40;
    callback_rect=&win.setup_rects_[20];
    forced_pid_index=1; rect_to_mutate_during_pid=&win.setup_rects_[20];
    mutate_rect_on_pid=true;
    win.draw_diffs();
    if (arrow_x.size()!=1 || arrow_y.size()!=1 || arrow_x[0]!=40 || arrow_y[0]!=28)
        fail("draw_diffs arrow did not use pre-helper RECT snapshot");
    if (clipped_rects.size()!=1 || clipped_rects[0].left!=34 ||
        clipped_rects[0].top!=40 || clipped_rects[0].right!=86 ||
        clipped_rects[0].bottom!=80)
        fail("draw_diffs text clip did not use live post-helper RECT");
    if (spot_rects.size()!=1 || spot_rects[0].left!=30 ||
        spot_rects[0].top!=40 || spot_rects[0].right!=90 ||
        spot_rects[0].bottom!=80)
        fail("draw_diffs spot registration did not use live post-helper RECT");

    // draw_factions uses the pre-helper rectangle for arrow and clipping;
    // its spot registration reads the live rectangle after the helper.
    reset(); init_rows(); win.faction_records_=records; records[0].record_type_=1;
    NetWinSessionData.fields_.mode_=3; NetWinLobbyRows.rows_[1].faction_or_color_=0;
    win.setup_rects_[13].left=10; win.setup_rects_[13].top=20;
    win.setup_rects_[13].right=50; win.setup_rects_[13].bottom=40;
    callback_rect=&win.setup_rects_[13]; forced_pid_index=1;
    rect_to_mutate_during_pid=&win.setup_rects_[13]; mutate_rect_on_pid=true;
    win.draw_factions();
    if (arrow_x.size()!=1 || arrow_y.size()!=1 || arrow_x[0]!=40 || arrow_y[0]!=28)
        fail("draw_factions arrow did not use pre-helper RECT snapshot");
    if (clipped_rects.size()!=1 || clipped_rects[0].left!=14 ||
        clipped_rects[0].top!=20 || clipped_rects[0].right!=39 ||
        clipped_rects[0].bottom!=40)
        fail("draw_factions clipping did not use pre-helper RECT snapshot");
    if (spot_rects.size()!=1 || spot_rects[0].left!=30 ||
        spot_rects[0].top!=40 || spot_rects[0].right!=90 ||
        spot_rects[0].bottom!=80)
        fail("draw_factions spot registration did not use live post-helper RECT");

    // The source reacquires the fallback label for strcmp after strcat.
    // This double is a call-count/value seam model, not a claim that the CRT
    // mutates labels or invokes callbacks.
    reset(); init_rows(); win.faction_records_=records; records[0].record_type_=1;
    NetWinSessionData.fields_.mode_=3;
    std::strcpy(NetWinLobbyRows.rows_[1].noun_faction_, "Noun");
    vary_label_lookup=true;
    win.draw_factions();
    if (label_get_calls!=2 || std::strcmp(StringTemp, "Lobby")!=0)
        fail("draw_factions did not reacquire fallback label for strcmp");

    for (int raw : {0x80, 0xFE}) {
        reset(); init_rows(); win.faction_records_=records;
        records[0].record_type_=1; NetWinSessionData.fields_.mode_=3;
        NetWinLobbyRows.rows_[1].field_2_=static_cast<uint8_t>(raw);
        callback_rect=&win.setup_rects_[20];
        win.draw_diffs();
        char *expected=Difficulty[raw == 0x80 ? -128 : -2];
        if (difficulty_inputs.size()!=1 || difficulty_inputs[0]!=expected)
            fail("draw_diffs must sign-extend the difficulty byte");
    }
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
def test_unsigned_difficulty_index_mutant_rejected(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = _painter_bodies()
    before = "Difficulty[static_cast<int8_t>(row->field_2_)]"
    assert bodies["BODY_DIFFS"].count(before) == 1
    bodies["BODY_DIFFS"] = bodies["BODY_DIFFS"].replace(
        before, "Difficulty[row->field_2_]", 1)
    run = _compile_and_run(_host_source(bodies), compiler, tmp_path,
                           "netwin-unsigned-difficulty-mutant")
    assert run.returncode == 1
    assert "draw_diffs must sign-extend the difficulty byte" in run.stderr


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
        "if (NetWinSessionData.fields_.mode_ == 3\n"
        "            && index >= faction_records_[0].record_type_)"
    )
    assert live_check in body
    body = body.replace(
        "    for (int index = 0; index < 7;",
        "    const int record_type_snapshot = NetWinSessionData.fields_.mode_ == 3\n"
        "        ? faction_records_[0].record_type_ : 0;\n"
        "    for (int index = 0; index < 7;",
        1,
    )
    body = body.replace(
        live_check,
        "if (NetWinSessionData.fields_.mode_ == 3\n"
        "            && index >= record_type_snapshot)",
        1,
    )
    assert body != bodies["BODY_FACTIONS"]
    bodies["BODY_FACTIONS"] = body
    run = _compile_and_run(_host_source(bodies), compiler, tmp_path,
                           "netwin-hoisted-record-count")
    assert run.returncode != 0
    assert "draw_factions did not reread callback-mutated record bound" in run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compiled_live_arrow_rectangle_mutant_fails_runtime(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = _painter_bodies()
    body = bodies["BODY_DIFFS"]
    assert body.count("const RECT arrow_rect = *rect;") == 1
    old = ("arrow_rect.right - 0xA,\n"
           "                       arrow_rect.top\n"
           "                           + (arrow_rect.bottom - arrow.height() - arrow_rect.top) / 2")
    new = ("rect->right - 0xA,\n"
           "                       rect->top\n"
           "                           + (rect->bottom - arrow.height() - rect->top) / 2")
    assert body.count(old) == 1
    bodies["BODY_DIFFS"] = body.replace(old, new, 1)
    run = _compile_and_run(_host_source(bodies), compiler, tmp_path,
                           "netwin-live-arrow-rectangle-mutant")
    assert run.returncode != 0
    assert "draw_diffs arrow did not use pre-helper RECT snapshot" in run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compiled_live_faction_arrow_rectangle_mutant_fails_runtime(
    compiler_name, tmp_path
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = _painter_bodies()
    body = bodies["BODY_FACTIONS"]
    old = ("field.right - 0xA,\n"
           "                       field.top\n"
           "                           + (field.bottom - arrow.height() - field.top) / 2")
    new = ("rect->right - 0xA,\n"
           "                       rect->top\n"
           "                           + (rect->bottom - arrow.height() - rect->top) / 2")
    assert body.count(old) == 1
    bodies["BODY_FACTIONS"] = body.replace(old, new, 1)
    run = _compile_and_run(_host_source(bodies), compiler, tmp_path,
                           "netwin-live-faction-arrow-rectangle-mutant")
    assert run.returncode != 0
    assert "draw_factions arrow did not use pre-helper RECT snapshot" in run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compiled_live_faction_clip_rectangle_mutant_fails_runtime(
    compiler_name, tmp_path
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = _painter_bodies()
    body = bodies["BODY_FACTIONS"]
    old = "        field.left += 4;"
    new = "        field = *rect;\n        field.left += 4;"
    assert body.count(old) == 1
    bodies["BODY_FACTIONS"] = body.replace(old, new, 1)
    run = _compile_and_run(_host_source(bodies), compiler, tmp_path,
                           "netwin-live-faction-clip-rectangle-mutant")
    assert run.returncode != 0
    assert "draw_factions clipping did not use pre-helper RECT snapshot" in run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compiled_wrong_vertical_faction_inset_mutant_fails_runtime(
    compiler_name, tmp_path
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = _painter_bodies()
    body = bodies["BODY_FACTIONS"]
    old = "        field.left += 4;\n        field.right -= 0xB;"
    new = "        field.top += 4;\n        field.bottom -= 0xB;"
    assert body.count(old) == 1
    bodies["BODY_FACTIONS"] = body.replace(old, new, 1)
    run = _compile_and_run(_host_source(bodies), compiler, tmp_path,
                           "netwin-wrong-vertical-faction-inset-mutant")
    assert run.returncode != 0
    assert "draw_factions clipping did not use pre-helper RECT snapshot" in run.stderr
