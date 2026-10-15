"""Execute ReportWin's actual satellite animation body with a host fixture."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _extract_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated body for {signature}")


def _actual_sat_anim_source() -> str:
    header = (ROOT / "src/reportwin.h").read_text()
    source = (ROOT / "src/reportwin.cpp").read_text()
    assert "RECOVERY_MEMBER: v1 ReportWin satellite_index_ 0xE29C 0x0049FE40" in header
    assert "int satellite_index_;  // 0xE29C" in header
    assert "uint32_t field_E29C_" not in header
    return _extract_body(source, "void ReportWin::sat_anim()")


def _actual_draw_sat_loop_source() -> str:
    """Return the production loop; host hooks model stage-boundary writes only.

    These synthetic mutations verify cursor reread order. They are not a claim
    that the synchronous native helpers currently write this member.
    """
    source = (ROOT / "src/reportwin.cpp").read_text()
    draw_sat = _extract_body(source, "void ReportWin::draw_sat()")
    loop = _extract_body(
        draw_sat,
        "while (satellite_index_ < (ExpansionEnabled ? 5 : 4))",
    )
    assert "const int kind = satellite_index_" not in loop
    assert "report_sat_count" not in loop
    assert "spot_.add(satellite_index_, other," in loop
    return loop


HOST_FIXTURE = r"""
#include <cmath>
#include <cstdio>

struct RECT { int left, top, right, bottom; };
struct ReportWin;

static int failures = 0;
static void check(bool condition, const char *message) {
    if (!condition) {
        std::fprintf(stderr, "failed: %s\n", message);
        ++failures;
    }
}

struct Buffer {
    ReportWin *owner;
    void draw(Buffer *source, int style, int x, int y);
};

struct Flic {
    ReportWin *owner;
    void decode_stored_frame();
};

struct ReportWin : Buffer {
    void sat_anim();
    void fill(RECT *rect, int style);
    void fill(int x, int y, int width, int height, int color);
    void soft_update(RECT *rect);

    int satellite_index_;
    float satellite_x_[16];
    float satellite_y_[16];
    float satellite_dx_[16];
    float satellite_dy_[16];
    float satellite_acceleration_[16];
    int satellite_live_[16];
    float field_EE30_;
    int field_E2A0_;
    int field_E2A4_;
    int field_E2A8_;
    int field_E2AC_;
    RECT satellite_rect_;
    Flic flic_;
    Buffer buffer2_;

    int decode_index;
    int draw_index;
    int soft_update_index;
    int rect_fill_count;
    int marker_count;
    int marker_indices[16];
    int marker_x[16];
    int marker_y[16];

    ReportWin() : Buffer(), satellite_index_(0), field_EE30_(0.25f),
                  field_E2A0_(17), field_E2A4_(23), field_E2A8_(100),
                  field_E2AC_(200), satellite_rect_{1, 2, 3, 4},
                  decode_index(-1), draw_index(-1), soft_update_index(-1),
                  rect_fill_count(0), marker_count(0) {
        owner = this;
        flic_.owner = this;
        buffer2_.owner = this;
    }
};

void Flic::decode_stored_frame() {
    owner->decode_index = owner->satellite_index_;
    // Model a callback-visible write between the repeated member accesses.
    owner->satellite_index_ = 7;
    owner->satellite_live_[15] = 1;
    owner->satellite_x_[15] = 25.75f;
    owner->satellite_y_[15] = 34.75f;
}

void Buffer::draw(Buffer *source, int style, int x, int y) {
    check(source == static_cast<Buffer *>(owner), "buffer draw source is this ReportWin");
    check(style == 9 && x == 17 && y == 23, "buffer draw retains original arguments");
    owner->draw_index = owner->satellite_index_;
    // A second callback mutation is visible to the following member reset.
    owner->satellite_index_ = 11;
    owner->satellite_live_[2] = 1;
    owner->satellite_x_[2] = 12.75f;
    owner->satellite_y_[2] = 22.75f;
}

void ReportWin::fill(RECT *rect, int style) {
    check(rect == &satellite_rect_ && style == 0x41,
          "orbit rectangle is cleared before integration");
    ++rect_fill_count;
}

void ReportWin::fill(int x, int y, int width, int height, int color) {
    check(width == 2 && height == 2 && color == 0xFF,
          "live satellite marker retains 2x2 white fill");
    if (marker_count < 16) {
        marker_indices[marker_count] = satellite_index_;
        marker_x[marker_count] = x;
        marker_y[marker_count] = y;
    }
    ++marker_count;
}

void ReportWin::soft_update(RECT *rect) {
    check(rect == &satellite_rect_, "published rectangle is the orbit rectangle");
    soft_update_index = satellite_index_;
}
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_sat_anim_uses_signed_shared_cursor_and_publishes_all_live_markers(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")

    body = _actual_sat_anim_source()
    main = r"""
int main() {
    ReportWin window;
    float expected_x[16], expected_y[16], expected_dx[16], expected_dy[16];
    for (int i = 0; i < 16; ++i) {
        window.satellite_x_[i] = expected_x[i] = static_cast<float>(i + 10);
        window.satellite_y_[i] = expected_y[i] = static_cast<float>(i + 20);
        window.satellite_dx_[i] = expected_dx[i] = static_cast<float>(i + 1);
        window.satellite_dy_[i] = expected_dy[i] = static_cast<float>(i + 2);
        window.satellite_acceleration_[i] = 0.5f;
        window.satellite_live_[i] = (i % 4 == 0);

        expected_dx[i] -= 0.5f * expected_x[i];
        expected_dy[i] -= 0.5f * expected_y[i];
        expected_x[i] += expected_dx[i] * window.field_EE30_;
        expected_y[i] += expected_dy[i] * window.field_EE30_;
    }

    window.sat_anim();
    expected_x[2] = 12.75f;
    expected_y[2] = 22.75f;
    expected_x[15] = 25.75f;
    expected_y[15] = 34.75f;

    check(window.rect_fill_count == 1, "one initial rectangle clear");
    check(window.decode_index == 16, "decoder observes completed first loop cursor");
    check(window.draw_index == 7, "buffer callback observes decoder's shared-cursor write");
    check(window.soft_update_index == 16, "publication observes completed marker loop cursor");
    check(window.satellite_index_ == 16, "member retains post-loop cursor value");
    for (int i = 0; i < 16; ++i) {
        check(std::fabs(window.satellite_dx_[i] - expected_dx[i]) < 0.0001f,
              "all sixteen x velocities are integrated");
        check(std::fabs(window.satellite_dy_[i] - expected_dy[i]) < 0.0001f,
              "all sixteen y velocities are integrated");
        check(std::fabs(window.satellite_x_[i] - expected_x[i]) < 0.0001f,
              "all sixteen x positions are integrated");
        check(std::fabs(window.satellite_y_[i] - expected_y[i]) < 0.0001f,
              "all sixteen y positions are integrated");
    }

    int expected_markers = 0;
    for (int i = 0; i < 16; ++i) {
        if (window.satellite_live_[i]) {
            check(expected_markers < window.marker_count,
                  "each callback-updated live satellite is painted");
            if (expected_markers < window.marker_count) {
                check(window.marker_indices[expected_markers] == i,
                      "marker reads the current shared member index");
                check(window.marker_x[expected_markers]
                          == static_cast<int>(window.satellite_x_[i]) + 100,
                      "marker uses the live x value after callbacks");
                check(window.marker_y[expected_markers]
                          == static_cast<int>(window.satellite_y_[i]) + 200,
                      "marker uses the live y value after callbacks");
            }
            ++expected_markers;
        }
    }
    check(window.marker_count == expected_markers,
          "only live satellites receive markers");
    return failures == 0 ? 0 : 1;
}
"""
    cpp = tmp_path / f"satellite-cursor-{compiler_name}.cpp"
    executable = tmp_path / f"satellite-cursor-{compiler_name}"
    cpp.write_text(HOST_FIXTURE + "\n" + body + "\n" + main)
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)],
        text=True,
        capture_output=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True)
    assert ran.returncode == 0, ran.stderr


DRAW_SAT_FIXTURE = r"""
#include <cstdio>
#include <cstring>

struct RECT { int left, top, right, bottom; };
struct FacilityEntry { int name; };
struct PlayerDataStub { unsigned int satellites_[4]; unsigned int sat_odp_deployed; };
struct BaseStub { int faction_id_current; };
struct GameStateStub { struct { int base_current_count_; } fields_; };

static int failures = 0;
static bool ExpansionEnabled = true;
static char StringTemp[128];
static FacilityEntry Facility[100];
static PlayerDataStub PlayersData[8];
static BaseStub Bases[16];
static GameStateStub GameDataStateStorage;
static int facility_reads[16];
static int facility_read_count = 0;
static int number_values[32];
static int number_count = 0;
static struct ReportWin *active_window = 0;

static void check(bool condition, const char *message) {
    if (!condition) {
        std::fprintf(stderr, "failed: %s\n", message);
        ++failures;
    }
}

struct StringTableStub { const char *get(int id); };
static StringTableStub StringTable;
struct ReportWin;
struct CanvasStub {
    ReportWin *owner;
    void set_text_color(int color, int shadow, int use_shadow, int mode);
    void wrap_cent(char *text, RECT *rect);
    void write_cent_l(char *text, RECT *rect, unsigned int length);
};
struct SpotStub {
    ReportWin *owner;
    void add(int id, int faction, int left, int top, int width, int height);
};
static bool has_fac(int facility, int base, int flags);
static void report_sat_number(int value);

struct ReportWin {
    void draw_rows();
    int satellite_index_;
    int other;
    int row_width;
    RECT report_work_rect_;
    CanvasStub canvas;
    SpotStub spot_;

    int color_calls;
    int colors[16];
    int color_mutate_call;
    int color_mutate_value;
    bool append_stale_after_color;
    int has_fac_calls;
    int has_fac_mutate_call;
    int has_fac_mutate_value;
    int number_calls;
    int number_mutate_call;
    int number_mutate_value;
    int write_calls;
    int write_mutate_call;
    int write_mutate_value;
    int spot_calls;
    int spot_ids[16];
    int spot_mutate_call;
    int spot_mutate_value;
    int wrap_calls;
    char wrapped_text[16][128];
    char written_text[16][128];

    ReportWin() : satellite_index_(0), other(0), row_width(10),
                  report_work_rect_{0, 0, 100, 20}, color_calls(0),
                  color_mutate_call(0), color_mutate_value(0),
                  append_stale_after_color(false), has_fac_calls(0),
                  has_fac_mutate_call(0), has_fac_mutate_value(0),
                  number_calls(0), number_mutate_call(0), number_mutate_value(0),
                  write_calls(0), write_mutate_call(0), write_mutate_value(0),
                  spot_calls(0), spot_mutate_call(0), spot_mutate_value(0),
                  wrap_calls(0) {
        canvas.owner = this;
        spot_.owner = this;
        active_window = this;
    }
};

const char *StringTableStub::get(int id) {
    if (facility_read_count < 16) facility_reads[facility_read_count] = id;
    ++facility_read_count;
    static char value[24];
    std::snprintf(value, sizeof(value), "N%d", id);
    return value;
}

void CanvasStub::set_text_color(int color, int, int, int) {
    if (owner->color_calls < 16) owner->colors[owner->color_calls] = color;
    ++owner->color_calls;
    if (owner->color_mutate_call == owner->color_calls)
        owner->satellite_index_ = owner->color_mutate_value;
    if (owner->append_stale_after_color) std::strcat(StringTemp, "stale");
}

void CanvasStub::wrap_cent(char *text, RECT *) {
    if (owner->wrap_calls < 16)
        std::snprintf(owner->wrapped_text[owner->wrap_calls], 128, "%s", text);
    ++owner->wrap_calls;
}

void CanvasStub::write_cent_l(char *text, RECT *, unsigned int) {
    if (owner->write_calls < 16)
        std::snprintf(owner->written_text[owner->write_calls], 128, "%s", text);
    ++owner->write_calls;
    if (owner->write_mutate_call == owner->write_calls)
        owner->satellite_index_ = owner->write_mutate_value;
}

void SpotStub::add(int id, int, int, int, int, int) {
    if (owner->spot_calls < 16) owner->spot_ids[owner->spot_calls] = id;
    ++owner->spot_calls;
    if (owner->spot_mutate_call == owner->spot_calls)
        owner->satellite_index_ = owner->spot_mutate_value;
}

static bool has_fac(int, int, int) {
    ++active_window->has_fac_calls;
    if (active_window->has_fac_mutate_call == active_window->has_fac_calls)
        active_window->satellite_index_ = active_window->has_fac_mutate_value;
    return true;
}

static void report_sat_number(int value) {
    if (number_count < 32) number_values[number_count] = value;
    ++number_count;
    ++active_window->number_calls;
    char text[24];
    std::snprintf(text, sizeof(text), "%d", value);
    std::strcat(StringTemp, text);
    if (active_window->number_mutate_call == active_window->number_calls)
        active_window->satellite_index_ = active_window->number_mutate_value;
}

void ReportWin::draw_rows() {
    active_window = this;
    satellite_index_ = 0;
    __DRAW_SAT_LOOP__
}

static void reset_draw_fixture() {
    std::memset(StringTemp, 0, sizeof(StringTemp));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(Bases, 0, sizeof(Bases));
    std::memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
    std::memset(facility_reads, 0, sizeof(facility_reads));
    std::memset(number_values, 0, sizeof(number_values));
    facility_read_count = 0;
    number_count = 0;
    active_window = 0;
    for (int i = 0; i < 100; ++i) Facility[i].name = i;
}
"""


def _compile_and_run_draw_sat(tmp_path, compiler, loop_source, main_source):
    fixture = DRAW_SAT_FIXTURE.replace("__DRAW_SAT_LOOP__", loop_source)
    cpp = tmp_path / f"draw-sat-{Path(compiler).name}.cpp"
    executable = tmp_path / f"draw-sat-{Path(compiler).name}"
    cpp.write_text(fixture + "\n" + main_source)
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)],
        text=True,
        capture_output=True,
    )
    return compiled, subprocess.run([str(executable)], text=True, capture_output=True) if compiled.returncode == 0 else None


DRAW_SAT_NORMAL_MAIN = r"""
int main() {
    reset_draw_fixture();
    ExpansionEnabled = true;
    GameDataStateStorage.fields_.base_current_count_ = 1;
    Bases[0].faction_id_current = 2;
    PlayersData[2].satellites_[0] = 1;
    PlayersData[2].satellites_[1] = 2;
    PlayersData[2].satellites_[2] = 0;
    PlayersData[2].satellites_[3] = 4;
    PlayersData[2].sat_odp_deployed = 1;
    ReportWin other_rows;
    other_rows.other = 2;
    other_rows.draw_rows();
    const int expected_colors[5] = {0x98, 0xDF, 0xF8, 0xF9, 0xFF};
    check(other_rows.color_calls == 5, "one color call per recognized selector");
    for (int i = 0; i < 5; ++i) {
        check(other_rows.colors[i] == expected_colors[i], "native color case selection");
        check(other_rows.spot_ids[i] == i, "normal spot ids use the current cursor");
    }
    const int expected_numbers[6] = {1, 2, 0, 3, 4, 1};
    check(number_count == 6, "ODP and satellite numbers are both emitted");
    for (int i = 0; i < 6; ++i)
        check(number_values[i] == expected_numbers[i], "direct satellite counts and ODP prefix");
    check(other_rows.write_calls == 5, "five rows are written");
    check(std::strcmp(other_rows.written_text[3], "3/4") == 0,
          "ODP row retains the original number separator");
    check(other_rows.satellite_index_ == 5, "expanded rows finish after selector four");

    reset_draw_fixture();
    ExpansionEnabled = true;
    ReportWin headings;
    headings.other = 0;
    headings.append_stale_after_color = true;
    headings.draw_rows();
    const int expected_facilities[5] = {65, 66, 67, 68, 41};
    check(facility_read_count == 5, "five heading names are loaded");
    check(headings.wrap_calls == 5, "five heading names are wrapped");
    for (int i = 0; i < 5; ++i) {
        check(facility_reads[i] == expected_facilities[i], "heading facility selection");
        char expected_text[24];
        std::snprintf(expected_text, sizeof(expected_text), "N%d", expected_facilities[i]);
        check(std::strcmp(headings.wrapped_text[i], expected_text) == 0,
              "StringTemp is cleared after color setup");
    }

    reset_draw_fixture();
    ExpansionEnabled = false;
    ReportWin no_expansion;
    no_expansion.other = 0;
    no_expansion.color_mutate_call = 1;
    no_expansion.color_mutate_value = 4;
    no_expansion.draw_rows();
    check(facility_read_count == 1 && facility_reads[0] == 69,
          "without expansion, live selector four uses the ordinary facility row");
    check(no_expansion.satellite_index_ == 5,
          "the loop rereads its live member bound after callback mutation");
    return failures == 0 ? 0 : 1;
}
"""


DRAW_SAT_MUTATION_MAIN = r"""
int main() {
    reset_draw_fixture();
    ExpansionEnabled = true;
    GameDataStateStorage.fields_.base_current_count_ = 1;
    Bases[0].faction_id_current = 2;
    PlayersData[2].satellites_[0] = 8;
    PlayersData[2].satellites_[3] = 4;
    PlayersData[2].sat_odp_deployed = 1;
    ReportWin window;
    window.other = 2;
    window.has_fac_mutate_call = 1;
    window.has_fac_mutate_value = 4;
    window.color_mutate_call = 1;
    window.color_mutate_value = 3;
    window.append_stale_after_color = true;
    window.number_mutate_call = 1;
    window.number_mutate_value = 4;
    window.write_mutate_call = 1;
    window.write_mutate_value = 2;
    window.spot_mutate_call = 1;
    window.spot_mutate_value = 4;
    window.draw_rows();
    check(window.has_fac_calls == 1, "scan callback runs before first selector reload");
    check(window.color_calls == 1 && window.colors[0] == 0xFF,
          "color group snapshots selector four after base scan");
    check(number_count == 2 && number_values[0] == 3 && number_values[1] == 1,
          "post-color and post-number selector reloads choose ODP and elevator paths");
    check(window.write_calls == 1 && std::strcmp(window.written_text[0], "3/1") == 0,
          "post-color StringTemp reset removes callback-written stale text");
    check(window.spot_calls == 1 && window.spot_ids[0] == 2,
          "Spot receives the live selector after write callback mutation");
    check(window.satellite_index_ == 5,
          "callback mutation after Spot is followed by the original increment");
    return failures == 0 ? 0 : 1;
}
"""


def _old_cached_loop(loop_source: str) -> str:
    opening = loop_source.index("{") + 1
    result = loop_source[:opening] + "\n                const int kind = satellite_index_;" + loop_source[opening:]
    result = result.replace("const int color_kind = satellite_index_;", "const int color_kind = kind;")
    result = result.replace("const int formatted_kind = satellite_index_;", "const int formatted_kind = kind;")
    result = result.replace("const int amount_kind = satellite_index_;", "const int amount_kind = kind;")
    result = result.replace("spot_.add(satellite_index_, other,", "spot_.add(kind, other,")
    return result


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_draw_sat_rows_preserve_native_stage_reads_and_reset_order(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    loop = _actual_draw_sat_loop_source()
    compiled, ran = _compile_and_run_draw_sat(tmp_path, compiler, loop, DRAW_SAT_NORMAL_MAIN)
    assert compiled.returncode == 0, compiled.stderr
    assert ran.returncode == 0, ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_draw_sat_reloads_member_between_injected_stage_mutations(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    loop = _actual_draw_sat_loop_source()
    compiled, ran = _compile_and_run_draw_sat(tmp_path, compiler, loop, DRAW_SAT_MUTATION_MAIN)
    assert compiled.returncode == 0, compiled.stderr
    assert ran.returncode == 0, ran.stderr


def test_actual_draw_sat_controls_reject_the_old_entry_cached_cursor(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler required for source damage control")
    mutant = _old_cached_loop(_actual_draw_sat_loop_source())
    compiled, ran = _compile_and_run_draw_sat(tmp_path, compiler, mutant, DRAW_SAT_MUTATION_MAIN)
    assert compiled.returncode == 0, compiled.stderr
    assert ran is not None and ran.returncode != 0


def _actual_on_selected_source() -> str:
    source = (ROOT / "src/reportwin.cpp").read_text()
    body = _extract_body(source, "void ReportWin::on_selected(int selection_id, int)")
    assert "selection_id >= 0x5F5E0FE" in body
    return body


ON_SELECTED_FIXTURE = r"""
#include <climits>
#include <cstdio>

struct Event { int kind, first, second; };
static Event events[16];
static int event_count = 0;
static int failures = 0;
static void record(int kind, int first, int second) {
    if (event_count < 16) events[event_count] = {kind, first, second};
    ++event_count;
}
static void check(bool condition, const char *message) {
    if (!condition) { std::fprintf(stderr, "failed: %s\n", message); ++failures; }
}

struct ListBoxStub {
    int tag;
    explicit ListBoxStub(int value = 0) : tag(value) {}
    void set_selected_pos(int position, int mode) {
        record(tag == 1 ? 1 : 3, position, mode);
    }
    void set_selected_id(int id) { record(tag == 1 ? 2 : tag == 2 ? 4 : 5, id, 0); }
};
struct ReportIfStub {
    ListBoxStub listBox1_, listBox2_;
    ReportIfStub() : listBox1_(1), listBox2_(2) {}
};
static ReportIfStub ReportIfGlobal;

struct ReportWin {
    int field_59AC_, field_59B0_;
    ListBoxStub listBox_;
    ReportWin() : field_59AC_(0), field_59B0_(0), listBox_(3) {}
    void on_selected(int selection_id, int ignored);
    void reset() { record(6, 0, 0); }
    void draw_energy() { record(7, 0, 0); }
    void soft_update() { record(8, 0, 0); }
};

static void clear_events() { event_count = 0; }
static void expect_event(int index, int kind, int first, int second) {
    check(index < event_count, "expected event exists");
    if (index < event_count) {
        check(events[index].kind == kind && events[index].first == first
                  && events[index].second == second,
              "call order and arguments match");
    }
}
static void verify_selection(int id, int branch) {
    clear_events();
    ReportWin window;
    window.field_59AC_ = 12345;
    window.field_59B0_ = 77;
    window.on_selected(id, 123);
    if (branch == 0) {
        check(window.field_59AC_ == id && window.field_59B0_ == 0,
              "ordinary selection stores the id and kind zero");
        check(event_count == 5, "ordinary selection updates lists then redraws");
        expect_event(0, 1, -1, 1);
        expect_event(1, 4, id, 0);
        expect_event(2, 6, 0, 0);
        expect_event(3, 7, 0, 0);
        expect_event(4, 8, 0, 0);
    } else if (branch == 1) {
        check(window.field_59AC_ == id - 0x400 && window.field_59B0_ == 1,
              "report selection stores offset id and kind one");
        check(event_count == 5, "report selection updates lists then redraws");
        expect_event(0, 2, id, 0);
        expect_event(1, 3, -1, 1);
        expect_event(2, 6, 0, 0);
        expect_event(3, 7, 0, 0);
        expect_event(4, 8, 0, 0);
    } else {
        check(window.field_59AC_ == 12345 && window.field_59B0_ == 77,
              "sentinel selection leaves remembered fields untouched");
        check(event_count == 1, "sentinel returns without reset or redraw");
        expect_event(0, 5, 99999999, 0);
    }
}
"""


ON_SELECTED_MAIN = r"""
int main() {
    verify_selection(INT_MIN, 0);
    verify_selection(-1, 0);
    verify_selection(0, 0);
    verify_selection(1023, 0);
    verify_selection(1024, 1);
    verify_selection(99999997, 1);
    verify_selection(99999998, 2);
    verify_selection(99999999, 2);
    verify_selection(INT_MAX, 2);
    return failures == 0 ? 0 : 1;
}
"""


def _compile_and_run_on_selected(tmp_path, compiler, body, main_source):
    cpp = tmp_path / f"on-selected-{Path(compiler).name}.cpp"
    executable = tmp_path / f"on-selected-{Path(compiler).name}"
    cpp.write_text(ON_SELECTED_FIXTURE + "\n" + body + "\n" + main_source)
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)],
        text=True,
        capture_output=True,
    )
    ran = subprocess.run([str(executable)], text=True, capture_output=True) if compiled.returncode == 0 else None
    return compiled, ran


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_on_selected_preserves_original_boundary_and_call_order(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    compiled, ran = _compile_and_run_on_selected(
        tmp_path, compiler, _actual_on_selected_source(), ON_SELECTED_MAIN
    )
    assert compiled.returncode == 0, compiled.stderr
    assert ran is not None and ran.returncode == 0, ran.stderr if ran else "not run"


def test_actual_on_selected_boundary_control_rejects_wrong_threshold(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler required for source damage control")
    body = _actual_on_selected_source()
    mutant = body.replace("selection_id >= 0x5F5E0FE", "selection_id > 0x5F5E0FE", 1)
    assert mutant != body
    main = r"""
int main() {
    verify_selection(99999998, 2);
    return failures == 0 ? 0 : 1;
}
"""
    compiled, ran = _compile_and_run_on_selected(tmp_path, compiler, mutant, main)
    assert compiled.returncode == 0, compiled.stderr
    assert ran is not None and ran.returncode != 0
