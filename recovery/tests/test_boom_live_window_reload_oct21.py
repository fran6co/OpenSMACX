"""Compile actual boom source fragments with live MapWinTable replacements.

These controls prove the source's receiver reload dataflow. The replacement is
injected by the host stubs; it is not a claim about a particular rendering
method invoking a callback in the game.
"""
from pathlib import Path
import shutil
import subprocess

from match_recovery.source import Mode, read_file


ROOT = Path(__file__).resolve().parents[2]
ADDRESS = 0x00504AA0
SIGNATURE = "int __cdecl boom(int x, int y, int flags)"


def _masked_code(source):
    out = list(source)
    index = 0
    state = "code"
    quote = ""
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                out[index] = out[index + 1] = " "
                index += 2
                state = "line"
                continue
            if char == "/" and nxt == "*":
                out[index] = out[index + 1] = " "
                index += 2
                state = "block"
                continue
            if char in ('"', "'"):
                quote = char
                out[index] = " "
                state = "quoted"
        elif state == "line":
            if char == "\n":
                state = "code"
            else:
                out[index] = " "
        elif state == "block":
            if char == "*" and nxt == "/":
                out[index] = out[index + 1] = " "
                index += 2
                state = "code"
                continue
            if char != "\n":
                out[index] = " "
        else:
            if char == "\\":
                out[index] = " "
                if index + 1 < len(source) and source[index + 1] != "\n":
                    out[index + 1] = " "
                    index += 2
                    continue
            elif char == quote:
                out[index] = " "
                state = "code"
            elif char != "\n":
                out[index] = " "
        index += 1
    return "".join(out)


def _actual_boom_body():
    rows = [row for row in read_file(ROOT / "src" / "veh.cpp")
            if row.address == ADDRESS]
    assert len(rows) == 1, "missing or duplicate boom owner mapping"
    row = rows[0]
    assert row.mode == Mode.FILE
    assert row.name == "?boom@@YAHHHH@Z"
    assert row.image_spans[0] == (0x00504AA0, 0x00505D1B)
    assert row.region and SIGNATURE in row.region
    masked = _masked_code(row.region)
    start = masked.index(SIGNATURE)
    opening = masked.index("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return row.region[start:index + 1]
    raise AssertionError("source-reader boom owner has no balanced function body")


def _fragment(body, start, end):
    first = body.index(start)
    last = body.index(end, first) + len(end)
    return body[first:last]


def _fragments(body):
    initial = _fragment(
        body,
        "original_screen_snapshots[slot].init(width, height, 0, nullptr);",
        "original_screen_snapshots[slot].pixel_height());",
    )
    redraw = _fragment(
        body,
        "redrawn_map_snapshots[slot].init(width, height, 0, nullptr);",
        "original_screen_snapshots[slot].pixel_height());",
    )
    frame_start = body.index("if (flic_mode) {", body.index("for (int slot = 0; slot < 8; ++slot)"))
    update_end = body.index("window->update(&saved_rectangles[slot], 0);", frame_start)
    frame = body[frame_start:update_end + len("window->update(&saved_rectangles[slot], 0);")]
    draw_tile = body.index("window->draw_tile(x, y, 0);")
    reload = body.rfind("window = MapWinTable[slot];", 0, draw_tile)
    assert reload >= 0
    pre_draw_tile = body[reload:draw_tile + len("window->draw_tile(x, y, 0);")]
    restore_start = body.index("const bool use_redrawn_snapshot = flic_mode")
    restore_call = body.index("snapshot.copy(destination, 0, 0,", restore_start)
    restore_end = body.index("snapshot.pixel_height());", restore_call) + len("snapshot.pixel_height());")
    restore = body[restore_start:restore_end]
    assert "window = MapWinTable[slot];" in initial
    assert "window = MapWinTable[slot];" in redraw
    assert "window = MapWinTable[slot];" in frame
    assert "working_buffer.draw_multi_table_dest(" in frame
    assert "sprite.draw_multi_table_dest(" in frame
    assert "snapshot.pixel_width(), snapshot.pixel_height()" in restore
    assert "pixel_width()" in initial and "pixel_height()" in initial
    assert "pixel_width()" in redraw and "pixel_height()" in redraw
    return initial, redraw, frame, pre_draw_tile, restore


HARNESS = r"""
#include <stdint.h>

struct RECT { int left, top, right, bottom; };
struct MapWin;
struct Buffer {
    int stored_width;
    int stored_height;
    Buffer() : stored_width(0), stored_height(0) {}
    int pixel_width() const { return stored_width; }
    int pixel_height() const { return stored_height; }
    int init(int, int, int, void *);
    int copy(Buffer *, int, int, int, int, int, int);
    int draw_multi_table_dest(Buffer *, int, int, int, int, int, uint8_t *);
};
struct GraphicWin : Buffer {
    void update(RECT *, int);
};
struct MapWin : GraphicWin {
    int zoom_offset_;
    void draw_tile(int, int, int);
};
struct Sprite {
    int transparent_index() const { return 0xff; }
    int draw_multi_table_dest(Buffer *, int, int, int, uint8_t *);
};
struct Flic { void decode_stored_frame() {} };
struct Veh { int faction_id, proto_id; };
struct VehPrototype { int reactor_id; };

MapWin *MapWinTable[8];
MapWin *switch_on_init_to;
MapWin *switch_on_copy_to;
MapWin *switch_on_draw_to;
Buffer *last_copy_this;
Buffer *last_copy_other;
Buffer *copy_receivers[8];
Buffer *copy_arguments[8];
int copy_widths[8];
int copy_heights[8];
int copy_count;
int last_copy_width;
int last_copy_height;
MapWin *last_updated;
MapWin *last_draw_tile;
int replace_on_init;
int replace_on_copy;
int replace_on_draw;
int fail_next_init;
int flic_mode;
int flags;
int sprite_bank;
int palette_row;
int animation_height;
int frame_span;
int saved_selection;
int second_selection;
int draw_type;
unsigned long frame;
RECT saved_rectangles[8];
uint8_t color_tables[8][1];
uint8_t *ExplosionColorTables[8] = {
    color_tables[0], color_tables[1], color_tables[2], color_tables[3],
    color_tables[4], color_tables[5], color_tables[6], color_tables[7]
};
Veh Vehs[4];
VehPrototype VehPrototypes[4];
int faction_color_group[32];
Sprite xi_sprites[64];
Sprite xf_sprites[64];
Sprite *g_XI_BOOM_VEH_SPRITES = xi_sprites;
Sprite *g_XF_BOOM_VEH_SPRITES = xf_sprites;
Flic animation;
Buffer working_buffer;

int Buffer::init(int width, int height, int, void *) {
    if (replace_on_init) MapWinTable[0] = switch_on_init_to;
    if (fail_next_init) {
        fail_next_init = 0;
        return -1;
    }
    stored_width = width;
    stored_height = height;
    return 0;
}
int Buffer::copy(Buffer *other, int, int, int, int, int width, int height) {
    last_copy_this = this;
    last_copy_other = other;
    last_copy_width = width;
    last_copy_height = height;
    if (copy_count < 8) {
        copy_receivers[copy_count] = this;
        copy_arguments[copy_count] = other;
        copy_widths[copy_count] = width;
        copy_heights[copy_count] = height;
    }
    ++copy_count;
    if (replace_on_copy) MapWinTable[0] = switch_on_copy_to;
    return 0;
}
int Buffer::draw_multi_table_dest(Buffer *destination, int, int, int, int, int,
                                  uint8_t *) {
    if (replace_on_draw) MapWinTable[0] = switch_on_draw_to;
    return destination == 0;
}
int Sprite::draw_multi_table_dest(Buffer *destination, int, int, int, uint8_t *) {
    if (replace_on_draw) MapWinTable[0] = switch_on_draw_to;
    return destination == 0;
}
void GraphicWin::update(RECT *, int) { last_updated = static_cast<MapWin *>(this); }
void MapWin::draw_tile(int, int, int) { last_draw_tile = this; }

static void clear_controls() {
    replace_on_init = replace_on_copy = replace_on_draw = 0;
    last_copy_this = last_copy_other = 0;
    last_copy_width = last_copy_height = 0;
    copy_count = 0;
    for (int i = 0; i < 8; ++i) {
        copy_receivers[i] = copy_arguments[i] = 0;
        copy_widths[i] = copy_heights[i] = 0;
    }
    last_updated = last_draw_tile = 0;
    fail_next_init = 0;
}

static int snapshot_controls() {
    MapWin first, after_init, after_redrawn_init, after_copy;
    Buffer original_screen_snapshots[1];
    Buffer redrawn_map_snapshots[1];
    RECT area = {1, 2, 8, 9};
    const int slot = 0;
    const int width = 7;
    const int height = 7;
    const int x = 4;
    const int y = 5;
    MapWinTable[0] = &first;
    MapWin *window = MapWinTable[slot];
    Buffer *window_buffer = static_cast<Buffer *>(static_cast<GraphicWin *>(window));
    original_screen_snapshots[slot].stored_width = 13;
    original_screen_snapshots[slot].stored_height = 17;
    switch_on_init_to = &after_init;
    switch_on_copy_to = &after_copy;
    replace_on_init = 1;
    replace_on_copy = 1;
    fail_next_init = 1;
    INITIAL_FRAGMENT
    if (MapWinTable[0] != &after_copy || last_copy_this != static_cast<Buffer *>(&after_init)
        || last_copy_other != &original_screen_snapshots[slot]
        || copy_widths[0] != 13 || copy_heights[0] != 17) return 1;
    PRE_DRAW_TILE_FRAGMENT
    if (last_draw_tile != &after_copy) return 2;

    clear_controls();
    window = MapWinTable[slot];
    window_buffer = static_cast<Buffer *>(static_cast<GraphicWin *>(window));
    original_screen_snapshots[slot].stored_width = 13;
    original_screen_snapshots[slot].stored_height = 17;
    redrawn_map_snapshots[slot].stored_width = 19;
    redrawn_map_snapshots[slot].stored_height = 23;
    switch_on_init_to = &after_redrawn_init;
    switch_on_copy_to = &after_copy;
    replace_on_init = 1;
    replace_on_copy = 1;
    fail_next_init = 1;
    REDRAW_FRAGMENT
    if (copy_count != 2
        || copy_receivers[0] != static_cast<Buffer *>(&after_redrawn_init)
        || copy_arguments[0] != &redrawn_map_snapshots[slot]
        || copy_widths[0] != 19 || copy_heights[0] != 23
        || copy_receivers[1] != &original_screen_snapshots[slot]
        || copy_arguments[1] != static_cast<Buffer *>(&after_copy)
        || copy_widths[1] != 13 || copy_heights[1] != 17) return 3;
    return 0;
}

static int frame_control(int use_flic) {
    MapWin before, after;
    const int slot = 0;
    flic_mode = use_flic;
    flags = 0;
    sprite_bank = palette_row = animation_height = frame_span = 0;
    saved_selection = second_selection = 0;
    frame = 0;
    Vehs[0].faction_id = Vehs[0].proto_id = 0;
    VehPrototypes[0].reactor_id = 0;
    saved_rectangles[0] = RECT{1, 2, 8, 9};
    before.zoom_offset_ = after.zoom_offset_ = 0;
    MapWinTable[0] = &before;
    switch_on_draw_to = &after;
    replace_on_draw = 1;
    MapWin *window = MapWinTable[slot];
    Buffer *destination = static_cast<Buffer *>(static_cast<GraphicWin *>(window));
    FRAME_FRAGMENT
    return last_updated == &after ? 0 : 3;
}

static int restore_control(int use_redrawn) {
    Buffer original_screen_snapshots[1];
    Buffer redrawn_map_snapshots[1];
    Buffer destination_storage;
    Buffer *destination = &destination_storage;
    const int restore_slot = 0;
    int flic_mode = use_redrawn ? 0 : 1;
    int play_animation = use_redrawn ? 0 : 1;
    int flags = use_redrawn ? 2 : 0;
    unsigned long frame = use_redrawn ? 8 : 0;
    const int width = 7;
    const int height = 7;
    (void)width;
    (void)height;
    RECT saved_rectangles[1] = {{1, 2, 8, 9}};
    original_screen_snapshots[0].stored_width = 13;
    original_screen_snapshots[0].stored_height = 17;
    redrawn_map_snapshots[0].stored_width = 19;
    redrawn_map_snapshots[0].stored_height = 23;
    RESTORE_FRAGMENT
    Buffer *expected = use_redrawn
        ? &redrawn_map_snapshots[restore_slot]
        : &original_screen_snapshots[restore_slot];
    return last_copy_this == expected && last_copy_other == destination
        && last_copy_width == (use_redrawn ? 19 : 13)
        && last_copy_height == (use_redrawn ? 23 : 17) ? 0 : 5;
}

int main() {
    clear_controls();
    int failures = snapshot_controls();
    clear_controls();
    if (frame_control(1) != 0) failures |= 4;
    clear_controls();
    if (frame_control(0) != 0) failures |= 8;
    clear_controls();
    if (restore_control(1) != 0) failures |= 16;
    clear_controls();
    if (restore_control(0) != 0) failures |= 32;
    return failures;
}
"""


def _drop_reload(fragment, before, after):
    start = fragment.index(before) + len(before)
    end = fragment.index(after, start)
    between = fragment[start:end]
    reload = "\n            window = MapWinTable[slot];\n            window_buffer =\n                static_cast<Buffer *>(static_cast<GraphicWin *>(window));"
    if reload not in between:
        reload = "\n        window = MapWinTable[slot];\n        window_buffer =\n            static_cast<Buffer *>(static_cast<GraphicWin *>(window));"
    assert reload in between, f"missing reload between {before!r} and {after!r}"
    return fragment[:start] + between.replace(reload, "", 1) + fragment[end:]


def _replace_once(fragment, old, new):
    assert old in fragment, f"unanchored semantic mutant {old!r}"
    return fragment.replace(old, new, 1)


def _compile_run(tmp_path, fragments, compiler, variant):
    initial, redraw, frame, pre_draw_tile, restore = fragments
    source = (HARNESS.replace("INITIAL_FRAGMENT", initial)
              .replace("REDRAW_FRAGMENT", redraw)
              .replace("FRAME_FRAGMENT", frame)
              .replace("PRE_DRAW_TILE_FRAGMENT", pre_draw_tile)
              .replace("RESTORE_FRAGMENT", restore))
    cpp = tmp_path / f"boom_live_reload_{variant}.cpp"
    binary = tmp_path / f"boom_live_reload_{variant}"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, f"{compiler} {variant} compile failed:\n{built.stderr}"
    return subprocess.run([str(binary)], capture_output=True, timeout=10)


def test_boom_live_window_reloads_and_compile_success_mutants(tmp_path):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for actual-source controls"
    body = _actual_boom_body()
    fragments = _fragments(body)
    initial, redraw, frame, pre_draw_tile, restore = fragments

    mutants = (
        ("initial-init-reload", (
            _drop_reload(initial,
                         "original_screen_snapshots[slot].init(width, height, 0, nullptr);",
                         "window_buffer->copy("),
            redraw, frame, pre_draw_tile, restore)),
        ("redrawn-init-reload", (
            initial,
            _drop_reload(redraw,
                         "redrawn_map_snapshots[slot].init(width, height, 0, nullptr);",
                         "window_buffer->copy("), frame, pre_draw_tile, restore)),
        ("redrawn-copy-reload", (
            initial,
            _drop_reload(redraw,
                         "window_buffer->copy(&redrawn_map_snapshots[slot],",
                         "original_screen_snapshots[slot].copy("), frame,
            pre_draw_tile, restore)),
        ("post-draw-reload", (
            initial, redraw,
            frame.replace("\n                window = MapWinTable[slot];", "", 1),
            pre_draw_tile, restore)),
        ("pre-draw-tile-reload", (
            initial, redraw, frame,
            pre_draw_tile.replace("window = MapWinTable[slot];", "", 1),
            restore)),
        ("initial-copy-requested-dimensions", (
            _replace_once(
                initial,
                "original_screen_snapshots[slot].pixel_width(),\n                                original_screen_snapshots[slot].pixel_height()",
                "width, -height"),
            redraw, frame, pre_draw_tile, restore)),
        ("redrawn-copy-requested-dimensions", (
            initial,
            _replace_once(
                redraw,
                "redrawn_map_snapshots[slot].pixel_width(),\n                            redrawn_map_snapshots[slot].pixel_height()",
                "width, -height"),
            frame, pre_draw_tile, restore)),
        ("snapshot-to-window-requested-dimensions", (
            initial,
            _replace_once(
                redraw,
                "original_screen_snapshots[slot].pixel_width(),\n                                             original_screen_snapshots[slot].pixel_height()",
                "width, -height"),
            frame, pre_draw_tile, restore)),
        ("frame-restore-requested-dimensions", (
            initial, redraw, frame, pre_draw_tile,
            _replace_once(restore,
                          "snapshot.pixel_width(), snapshot.pixel_height()",
                          "width, -height"))),
    )

    for compiler in compilers:
        actual = _compile_run(tmp_path, fragments, compiler, "actual")
        assert actual.returncode == 0, (
            f"{compiler}: actual fragment returned {actual.returncode}; "
            f"{actual.stderr.decode(errors='replace')}"
        )
        for name, mutant in mutants:
            result = _compile_run(tmp_path, mutant, compiler, name)
            assert result.returncode != 0, (
                f"{compiler} compiled but failed to detect {name}"
            )
