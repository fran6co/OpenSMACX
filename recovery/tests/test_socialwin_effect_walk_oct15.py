"""Actual-source controls for SocialWin's native 11-slot effect walk.

The C++ harness compiles the count and rendering loops directly from
SocialWin::draw_social. It is intentionally a behavioral control, not an ABI
or runtime-acceptance claim.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/socialwin.cpp"


def braced(text: str, anchor: str) -> str:
    at = text.index(anchor)
    opening = text.index("{", at)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[opening + 1 : end]
    raise AssertionError(f"unterminated block at {anchor!r}")


def source_loops(text: str) -> tuple[str, str]:
    draw = text[text.index("void SocialWin::draw_social(") :]
    count_at = draw.index("for (int effect_index = 0; effect_index < MaxSocialEffectNum;")
    count_body = braced(draw, "for (int effect_index = 0; effect_index < MaxSocialEffectNum;")
    count_open = draw.index("{", count_at)
    count_header = draw[count_at:count_open]
    count_loop = count_header + "{" + count_body + "}"
    render_at = draw.index("for (int phase = 0; phase < 3; phase++)")
    render_body = braced(draw, "for (int phase = 0; phase < 3; phase++)")
    render_header = draw[render_at : draw.index("{", render_at)]
    render_loop = render_header + "{" + render_body + "}"
    assert count_at < render_at
    assert "countNonzero +=" in count_loop and "countBig +=" in count_loop
    assert "smallSprite->draw" in render_loop and "bigSprite->draw" in render_loop
    return count_loop, render_loop


HARNESS = r'''#include <cassert>
#include <cstdlib>
#include <vector>
struct RECT { int left, top, right, bottom; };
struct Effect { int values_[11]; };
struct Category { Effect model_effect[4]; };
static Category SocialCategories[4];
struct Base { unsigned char faction_id_current; };
static Base Bases[8];
struct SecretProject { int cloning, backbone;
  int cloning_vats() const { return cloning; }
  int network_backbone() const { return backbone; }
};
struct GameData { struct { SecretProject secret_project_; } fields_; };
static GameData GameDataStateStorage;
static constexpr int MaxSocialEffectNum = 11;
static int expected_nonzero, expected_big;
static int *mutate_faction_after_draw;
struct Buffer {};
struct Draw { int kind, slot, x, y; };
static std::vector<Draw> draws;
struct Sprite {
  int kind, slot;
  int width() const { return 12; }
  int source_width() const { return slot == 0 ? 7 : 20 + slot; }
  int height() const { return 9; }
  int transparent_index() const { return -1; }
  void draw(Buffer *, int, int x, int y, int, int) {
    draws.push_back({kind, slot, x, y});
    if (mutate_faction_after_draw) {
      *mutate_faction_after_draw = 3;
      mutate_faction_after_draw = nullptr;
    }
  }
};
struct FakeWin {
  int factionID_ = 2;
  Sprite effectIcons_[90];
  Sprite socEngModifierNums_[20];
  void run(int a1, int row) {
    RECT body{10, 20, 210, 80};
    Buffer local;
    Buffer *buffer = &local;
    int faction = factionID_;
    int countNonzero = 0;
    int countBig = 0;
    int *const social_effect_row = SocialCategories[3].model_effect[1].values_;
    int *p = social_effect_row;
    __COUNT_LOOP__
    assert(countNonzero == expected_nonzero);
    assert(countBig == expected_big);
    int drawX = (body.right - (effectIcons_[0].width() + 1) * countNonzero -
                 socEngModifierNums_[0].source_width() * countBig - body.left) / 2 + body.left;
    int drawYBase = (body.bottom - effectIcons_[0].width() - body.top) / 2 + body.top;
    int drawX2 = drawX;
    __RENDER_LOOP__
  }
};
static void check_draws(const FakeWin &w) {
  const Draw expected[] = {
    {2, 0, 27, 71}, {1, 26, 35, 44}, {2, 4, 48, 71}, {1, 71, 56, 44},
    {1, 8, 69, 44}, {1, 44, 82, 44}, {1, 80, 95, 44},
    {1, 16, 108, 44}, {2, 1, 121, 71}, {1, 34, 129, 44},
    {2, 3, 142, 71}, {1, 52, 150, 44}, {1, 61, 163, 44},
    {2, 1, 176, 71}, {1, 88, 184, 44}
  };
  assert(draws.size() == sizeof(expected) / sizeof(expected[0]));
  for (size_t i = 0; i < draws.size(); ++i) {
    assert(draws[i].kind == expected[i].kind && draws[i].slot == expected[i].slot &&
           draws[i].x == expected[i].x && draws[i].y == expected[i].y);
  }
}
int main() {
  FakeWin w;
  for (int i = 0; i < 90; ++i) w.effectIcons_[i] = {1, i};
  for (int i = 0; i < 20; ++i) w.socEngModifierNums_[i] = {2, i};
  const int values[11] = {1, -1, 2, 9, -2, 1, -3, -1, 4, 1, -2};
  for (int i = 0; i < 11; ++i) {
    SocialCategories[3].model_effect[1].values_[i] = values[i];
    for (int category = 0; category < 4; ++category)
      for (int row = 0; row < 4; ++row)
        SocialCategories[category].model_effect[row].values_[i] = values[i];
  }
  // Native table index 3 is reserved/skipped; remaining ten are rendered.
  expected_nonzero = 10; expected_big = 5;
  w.run(0, 0);
  check_draws(w);

  // Sprite drawing can change faction state; the next effect must use the live
  // member value again, as the native body reloads this+0xCF4 at each owner test.
  draws.clear();
  for (int category = 0; category < 4; ++category)
    for (int row = 0; row < 4; ++row)
      for (int i = 0; i < 11; ++i)
        SocialCategories[category].model_effect[row].values_[i] = 0;
  SocialCategories[2].model_effect[1].values_[0] = 1;
  SocialCategories[2].model_effect[1].values_[4] = -1;
  SocialCategories[2].model_effect[1].values_[5] = -1;
  GameDataStateStorage.fields_.secret_project_ = {1, -1};
  Bases[1].faction_id_current = 2;
  w.factionID_ = 2;
  expected_nonzero = 1; expected_big = 0;
  mutate_faction_after_draw = &w.factionID_;
  w.run(2, 1);
  assert(w.factionID_ == 3 && draws.size() == 3);
  assert(draws[0].kind == 1 && draws[0].slot == 8);
  assert(draws[1].kind == 1 && draws[1].slot == 34);
  assert(draws[2].kind == 1 && draws[2].slot == 43);
  w.factionID_ = 2;

  // Cloning-vats ownership suppresses negative selected effects in cat 2,row 1.
  draws.clear();
  for (int i = 0; i < 11; ++i)
    for (int category = 0; category < 4; ++category)
      for (int row = 0; row < 4; ++row)
        SocialCategories[category].model_effect[row].values_[i] = values[i];
  GameDataStateStorage.fields_.secret_project_ = {1, -1};
  Bases[1].faction_id_current = 2;
  expected_nonzero = 5; expected_big = 2;
  w.run(2, 1);
  assert(draws.size() == 7);
  for (const Draw &d : draws) assert(d.kind != 1 || d.slot == 8 || d.slot == 26 || d.slot == 44 || d.slot == 71 || d.slot == 80);

  // Network-backbone ownership suppresses negative row-table effects in cat 3,row 1.
  draws.clear();
  GameDataStateStorage.fields_.secret_project_ = {-1, 3};
  Bases[3].faction_id_current = 2;
  expected_nonzero = 5; expected_big = 2;
  w.run(3, 1);
  assert(draws.size() == 7);
  for (const Draw &d : draws) assert(d.kind != 1 || d.slot == 8 || d.slot == 26 || d.slot == 44 || d.slot == 71 || d.slot == 80);
}
'''


def make_harness(count: str, render: str) -> str:
    return HARNESS.replace("__COUNT_LOOP__", count).replace("__RENDER_LOOP__", render)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_socialwin_effect_walk_actual_source_and_mutants(tmp_path, compiler):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} unavailable")
    count, render = source_loops(SOURCE.read_text())
    variants = {
        "pinned-source": (count, render, True),
        "truncate-at-three": (count.replace("MaxSocialEffectNum", "3"),
                              render.replace("MaxSocialEffectNum", "3"), False),
        "remove-reserved-skip": (count.replace("effect_index != 3", "true"),
                                 render.replace("effect_index != 3", "true"), False),
        "wrong-remap-threshold": (count, render.replace("icon_effect_index >= 3",
                                                        "icon_effect_index >= 5"), False),
        "omit-nine-stride": (count, render.replace("* 9", "* 1"), False),
        "absolute-value-phase-zero": (count, render.replace("draw = (v >= 2);", "draw = (mag > 1);"), False),
        "reset-position-each-phase": (count, render.replace(
            "for (int phase = 0; phase < 3; phase++) {",
            "for (int phase = 0; phase < 3; phase++) { drawX2 = drawX;"), False),
        "selected-big-width": (count, render.replace(
            "socEngModifierNums_[0].source_width()", "bigSprite->source_width()"), False),
        "cached-faction": (count.replace("factionID_", "faction"),
                           render.replace("factionID_", "faction"), False),
    }
    for name, (count_body, render_body, should_pass) in variants.items():
        harness = make_harness(count_body, render_body)
        cpp = tmp_path / f"{name}.cpp"
        exe = tmp_path / name
        cpp.write_text(harness)
        built = subprocess.run([compiler_path, "-std=c++17", "-O0", str(cpp), "-o", str(exe)],
                               capture_output=True, text=True)
        assert built.returncode == 0, built.stderr
        run = subprocess.run([str(exe)], capture_output=True, text=True)
        assert (run.returncode == 0) == should_pass, (name, run.stdout, run.stderr)
