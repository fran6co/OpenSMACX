"""Host controls for recovered StatusWin active-panel color behavior.

The executable checks use contiguous snippets cut from the actual owner body,
plus lexical anchors for their placement in draw_active; they are not a full
draw_active runtime proof. Hostile renderer callbacks change the palette
between snippets, so a cached pair or a reset at the wrong point is observable.
"""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src" / "statuswin.cpp").is_file()
)
SOURCE = ROOT / "src" / "statuswin.cpp"
SIGNATURE = "void StatusWin::draw_active(int vehicle_id, int base_id, int add_spot)"


def extract_function(source: str) -> str:
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char in ('"', "'"):
                state = "string" if char == '"' else "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        else:
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise AssertionError("unterminated StatusWin::draw_active")


EXPECTED = (
    "screen->set_text_color(TextColourTable[12], TextColourTable[13], 1, 1);",
    "screen->set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);",
    "screen->set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);",
    "screen->set_text_color(MessageWinTextColor, MessageWinTextBackground, 1, 1);",
    "screen->set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);",
    "screen->set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);",
    "screen->set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);",
    "screen->set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);",
)


def source_sites(body: str) -> list[tuple[int, str]]:
    calls = [
        (match.start(), " ".join(match.group(0).split()))
        for match in re.finditer(r"screen->set_text_color\([^;]+;", body)
    ]
    sites: list[tuple[int, str]] = []

    def before(anchor: str, expected: str, prefix: str = "") -> None:
        position = body.index(anchor)
        prior = [(offset, call) for offset, call in calls if offset < position]
        assert prior, f"no color state immediately before {anchor}"
        offset, call = prior[-1]
        assert body[offset + len(call):position].strip() == prefix, anchor
        assert call == expected
        sites.append((offset, call))

    def after(anchor: str, expected: str) -> None:
        position = body.index(anchor) + len(anchor)
        following = [(offset, call) for offset, call in calls if offset >= position]
        assert following, f"no color state immediately after {anchor}"
        offset, call = following[0]
        assert body[position:offset].strip() == "", anchor
        assert call == expected
        sites.append((offset, call))

    title = "screen->wrap_cent(&font3_, StringTemp, content_x,"
    before(title, EXPECTED[0], "int line_y =")
    after("content_y + title_offset + 8, content_width);", EXPECTED[1])
    before("const int capacity = veh_cargo(vehicle_id);", EXPECTED[2])
    before("if ((vehicle.state & VSTATE_DESIGNATE_DEFENDER) && known_orders)", EXPECTED[3])
    before("if (known_orders) {", EXPECTED[4])
    before("screen->restore_clip();", EXPECTED[5])
    second_draw = "base_draw(screen, base_id, sprite_x, content_y, 0, 0x1800);"
    after(second_draw, EXPECTED[6])
    before("if (owner_id == ConsoleGlobal.faction_id_ ||", EXPECTED[7])
    sites.sort()
    assert [call for _, call in sites] == list(EXPECTED)
    # The first fresh reload brackets the actual vehicle title callback.
    assert sites[0][0] < body.index(title) < sites[1][0]
    # Base colors are reloaded after both native draw calls, before font setup.
    assert body.index("base_draw(screen, base_id, sprite_x, content_y, -4, 0x40);") < body.index(second_draw)
    assert body.index(second_draw) < sites[6][0] < body.index(
        "screen->set_font(&font1_, 0, 0, 0);", body.index(second_draw)
    )
    return sites


def harness(body: str) -> str:
    sites = source_sites(body)
    calls = [call for _, call in sites]
    # Keep the real title-wrap/reset fragment and the real two-base_draw/reset/
    # set_font fragment contiguous, so moving either reload across its callback
    # changes the observed values. Other sites execute their actual source call.
    title_fragment = body[sites[0][0]:sites[1][0] + len(sites[1][1])]
    first_base_draw = "base_draw(screen, base_id, sprite_x, content_y, -4, 0x40);"
    base_start = body.index(first_base_draw)
    font_call = "screen->set_font(&font1_, 0, 0, 0);"
    base_end = body.index(
        font_call,
        body.index("base_draw(screen, base_id, sprite_x, content_y, 0, 0x1800);"),
    ) + len(font_call)
    base_fragment = body[base_start:base_end]
    steps = [
        "  hostile_callback(0);\n" + title_fragment + "\n",
        f"  hostile_callback(2);\n  {calls[2]}\n",
        f"  hostile_callback(3);\n  {calls[3]}\n",
        f"  hostile_callback(4);\n  {calls[4]}\n",
        f"  hostile_callback(5);\n  {calls[5]}\n",
        "  hostile_callback(6);\n" + base_fragment + "\n",
        f"  hostile_callback(9);\n  {calls[7]}\n",
    ]
    expected_pairs = [
        (12, 13), (114, 115), (214, 215), (314, 315),
        (414, 415), (514, 515), (814, 815), (914, 915),
    ]
    expected_checks = "\n".join(
        f"  assert(screen->calls[{i}].first == {a} && screen->calls[{i}].second == {b});"
        for i, (a, b) in enumerate(expected_pairs)
    )
    return f'''#include <assert.h>
struct Pair {{ int first; int second; }};
static int TextColourTable[195];
static int MessageWinTextColor;
static int MessageWinTextBackground;
static void hostile_callback(int n) {{
  for (int i = 0; i < 195; ++i) TextColourTable[i] = n * 100 + i;
  MessageWinTextColor = n * 100 + 14;
  MessageWinTextBackground = n * 100 + 15;
}}
struct Screen {{
  Pair calls[8]; int count;
  Screen() : count(0) {{}}
  void set_text_color(int a, int b, int, int) {{ calls[count].first=a; calls[count].second=b; ++count; }}
  int wrap_cent(int*, char*, int, int, int) {{ hostile_callback(1); return 7; }}
  void set_font(int*, int, int, int) {{}}
}};
static int font1_, font3_, content_x, content_y, title_offset, content_width;
static int base_id, sprite_x;
static char StringTemp[256];
static void base_draw(Screen*, int, int, int, int, int) {{
  static int draw_number = 7;
  hostile_callback(draw_number++);
}}
int main() {{
  Screen screen_storage;
  Screen *screen = &screen_storage;
{''.join(steps)}
  (void)line_y;
  assert(screen->count == 8);
{expected_checks}
}}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_active_drawing_color_calls_execute_fresh_source_values(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_function(SOURCE.read_text(encoding="utf-8"))
    program = harness(body)
    cpp = tmp_path / f"actual_color_calls_{compiler_name}.cpp"
    exe = tmp_path / f"actual_color_calls_{compiler_name}"
    cpp.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30, check=False)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def test_each_missing_wrong_or_reordered_reload_is_rejected() -> None:
    original = SOURCE.read_text(encoding="utf-8")
    body = extract_function(original)
    sites = source_sites(body)
    # Each native reset can be independently omitted or given the wrong pair.
    for index, (offset, call) in enumerate(sites):
        missing = body[:offset] + body[offset + len(call):]
        with pytest.raises(AssertionError):
            source_sites(missing)
        wrong = (
            "screen->set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);"
            if index == 0
            else "screen->set_text_color(TextColourTable[12], TextColourTable[13], 1, 1);"
        )
        changed = body[:offset] + wrong + body[offset + len(call):]
        with pytest.raises(AssertionError):
            harness(changed)

    # Swapping the two title colors across wrap_cent is observably wrong: the
    # source anchor rejects it, and the executable baseline checks both fresh
    # palette values around the hostile wrap callback.
    first, second = sites[0], sites[1]
    reordered = (
        body[:first[0]] + second[1] + body[first[0] + len(first[1]):second[0]]
        + first[1] + body[second[0] + len(second[1]):]
    )
    with pytest.raises(AssertionError):
        harness(reordered)


def range_color_fragment(body: str) -> str:
    """Return the contiguous FA/FB/FB/F9 branch from the real owner body."""
    start = body.index("screen->set_text_color(0xfa, -1, 1, 1);")
    end_call = "screen->set_text_color(0xf9, -1, 1, 1);"
    end = body.index("}", body.index(end_call, start)) + 1
    fragment = body[start:end]
    assert fragment.count("screen->set_text_color(0xfb, -1, 1, 1);") == 2
    assert fragment.index("vehicle.terraforming_turns != 0") < fragment.index(
        "chassis.range == 1"
    )
    return fragment


def range_color_harness(fragment: str) -> str:
    return f'''#include <assert.h>
struct Screen {{
  int colors[4]; int count;
  Screen() : count(0) {{}}
  void set_text_color(int color, int, int, int) {{ colors[count++] = color; }}
}};
struct Vehicle {{ int terraforming_turns; }};
struct Chassis {{ int range; }};
static int action_home(int, int) {{ return 1; }}
static void check_case(int turns, int range_value, int remaining_value,
                       int movement_value, const int *expected, int count) {{
  Screen screen_storage;
  Screen *screen = &screen_storage;
  Vehicle vehicle = {{turns}};
  Chassis chassis = {{range_value}};
  int remaining = remaining_value;
  int movement = movement_value;
  int vehicle_id = 0;
{fragment}
  (void)remaining; (void)movement;
  assert(screen->count == count);
  for (int i = 0; i < count; ++i) assert(screen->colors[i] == expected[i]);
}}
int main() {{
  const int neither[] = {{0xfa, 0xf9}};
  const int first[] = {{0xfa, 0xfb, 0xf9}};
  const int second[] = {{0xfa, 0xfb, 0xf9}};
  const int both[] = {{0xfa, 0xfb, 0xfb, 0xf9}};
  check_case(0, 4, 8, 4, neither, 2);
  check_case(2, 4, 8, 4, first, 3);
  check_case(0, 1, 2, 4, second, 3);
  check_case(1, 1, 2, 4, both, 4);
}}
'''


def compile_and_run(compiler: str, program: str, tmp_path: Path, name: str) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / f"{name}.cpp"
    exe = tmp_path / name
    cpp.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=30, check=False)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_native_range_color_predicates_write_independently(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_function(SOURCE.read_text(encoding="utf-8"))
    fragment = range_color_fragment(body)
    ran = compile_and_run(compiler, range_color_harness(fragment), tmp_path, "range_colors")
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_range_color_runtime_controls_reject_missing_or_merged_write(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_function(SOURCE.read_text(encoding="utf-8"))
    fragment = range_color_fragment(body)
    second_if = "if (chassis.range == 1 && remaining <= movement / 2)"
    omitted = fragment.replace(second_if, "if (false)", 1)
    assert omitted != fragment
    omitted_run = compile_and_run(
        compiler, range_color_harness(omitted), tmp_path, "range_colors_missing_second"
    )
    assert omitted_run.returncode != 0

    independent = (
        "if (vehicle.terraforming_turns != 0 &&\n"
        "                    chassis.range / 2 <= vehicle.terraforming_turns) {\n"
        "                    screen->set_text_color(0xfb, -1, 1, 1);\n"
        "                }\n"
        "                if (chassis.range == 1 && remaining <= movement / 2) {\n"
        "                    screen->set_text_color(0xfb, -1, 1, 1);\n"
        "                }"
    )
    merged = (
        "if ((vehicle.terraforming_turns != 0 &&\n"
        "                     chassis.range / 2 <= vehicle.terraforming_turns) ||\n"
        "                    (chassis.range == 1 && remaining <= movement / 2)) {\n"
        "                    screen->set_text_color(0xfb, -1, 1, 1);\n"
        "                }"
    )
    assert independent in fragment
    merged_fragment = fragment.replace(independent, merged, 1)
    assert merged_fragment != fragment
    merged_run = compile_and_run(
        compiler, range_color_harness(merged_fragment), tmp_path, "range_colors_merged"
    )
    assert merged_run.returncode != 0
