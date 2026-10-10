"""Bounded actual-source icon-branch controls; not full SelectPartWin runtime coverage."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "selectpartwin.cpp"
VEH_HEADER = ROOT / "src" / "veh.h"


def function_body(text: str) -> str:
    marker = "int SelectPartWin::draw_weapon(int a1, RECT *a2, int a3)"
    start = text.index(marker)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1 : offset]
    raise AssertionError("unterminated SelectPartWin::draw_weapon body")


def declaration(text: str, name: str) -> str:
    marker = f"struct {name} {{"
    start = text.index(marker)
    end = text.index("};", start) + 2
    following = text[end:]
    assertion_start = following.index(f"static_assert(sizeof({name})")
    assertion_end = following.index(";", assertion_start) + 1
    return text[start:end] + following[assertion_start:assertion_end]


def icon_branch(text: str) -> str:
    body = function_body(text)
    start = body.index("    caviar->set_scaling(scale_imm);")
    end = body.index("\n    if (weaponMissing != 0)", start)
    return body[start:end]


def compile_run(tmp_path: Path, source: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("g++")
    if compiler is None:
        raise AssertionError("g++ is required for this actual-source host control")
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp = tmp_path / "control.cpp"
    exe = tmp_path / "control"
    cpp.write_text(source)
    compiled = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        text=True,
        capture_output=True,
        check=False,
        timeout=30,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True, check=False, timeout=10)


def host_program(source: str, main: str, branch: str | None = None) -> str:
    veh_header = VEH_HEADER.read_text()
    veh_prototype = declaration(veh_header, "VehPrototype")
    rules_weapon = declaration(veh_header, "RulesWeapon")
    return r"""
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
// veh.h's rules-table pointer occupies a 32-bit slot in the target image.
typedef uint32_t LPSTR;
""" + rules_weapon + "\n" + veh_prototype + r"""
static_assert(offsetof(VehPrototype, icon_offset)==0x2e, "icon offset");
static_assert(sizeof(VehPrototype)==0x34, "prototype stride");
static_assert(sizeof(VehPrototype[512])==512*0x34, "prototype table stride");
static_assert(sizeof(RulesWeapon)==0x10, "weapon rule size");
RulesWeapon Weapon[26] = {};
VehPrototype VehPrototypes[512] = {};
struct RECT { int left, top, right, bottom; };
struct Buffer {};
struct Draw { int sprite, transparent, x, y; };
static std::vector<Draw> draws;
static std::vector<int> events;
static void (*scale_callback)() = 0;
struct Sprite {
  int index;
  int transparent_index() const { return index & 7; }
  void draw(Buffer *, int transparent, int x, int y) {
    draws.push_back(Draw{index, transparent, x, y}); events.push_back(2);
  }
};
Sprite g_VEH_SPRITES[2200] = {};
struct Caviar {
  void set_scaling(float) { events.push_back(1); if (scale_callback) scale_callback(); }
} renderer;
class SelectPartWin : public Buffer {
public:
  Caviar caviar_;
  void icon_branch(int idx, RECT *a2, float scale_imm, int weaponMissing) {
    Caviar *const caviar = &caviar_;
    (void)weaponMissing;
""" + (icon_branch(source) if branch is None else branch) + r"""
  }
};
static void reset() {
  draws.clear(); events.clear(); scale_callback=0;
  for (int i=0;i!=26;++i) Weapon[i]=RulesWeapon();
  for (int i=0;i!=512;++i) VehPrototypes[i]=VehPrototype();
  for (int i=0;i!=2200;++i) g_VEH_SPRITES[i].index=i;
}
static RECT rect = {10,20,210,120};
""" + main


def test_typed_prototype_icon_branch_signed_values_and_post_scale_reload(tmp_path: Path) -> None:
    text = SOURCE.read_text()
    body = function_body(text)
    assert "WeaponSpriteIndexSlot" not in text
    assert "(signed char *)0x009AB9CE" not in text
    assert "MOVSX" in text and "0x00432E63" in text and "0x009AB9CE" in text
    scale = body.index("caviar->set_scaling(scale_imm);")
    mode = body.index("const unsigned char cat_byte = Weapon[idx].mode;", scale)
    icon = body.index("sprite_idx = VehPrototypes[6].icon_offset;", mode)
    assert scale < mode < icon

    main = r"""
static void change_mode_and_icon_after_scale() {
  Weapon[3].mode=0x0b;
  VehPrototypes[6].icon_offset=6;
}
int main() {
  SelectPartWin win;
  reset(); Weapon[3].mode=8; Weapon[3].icon=4;
  win.icon_branch(3,&rect,0.2f,0);
  assert(draws.size()==1 && draws[0].sprite==104);
  assert(draws[0].transparent==(104&7) && draws[0].x==60 && draws[0].y==32);
  assert(events.size()==2 && events[0]==1 && events[1]==2);

  reset(); Weapon[3].mode=9; Weapon[3].icon=5;
  win.icon_branch(3,&rect,0.2f,0);
  assert(draws.size()==1 && draws[0].sprite==112);

  reset(); Weapon[3].mode=0x0b;
  VehPrototypes[6].icon_offset=static_cast<int8_t>(-128);
  win.icon_branch(3,&rect,0.2f,0);
  assert(draws.empty() && events.size()==1 && events[0]==1);

  reset(); Weapon[3].mode=0x0b;
  VehPrototypes[6].icon_offset=static_cast<int8_t>(-1);
  win.icon_branch(3,&rect,0.2f,0);
  assert(draws.empty() && events.size()==1 && events[0]==1);

  reset(); Weapon[3].mode=0x0b; VehPrototypes[6].icon_offset=0;
  win.icon_branch(3,&rect,0.2f,0);
  assert(draws.size()==1 && draws[0].sprite==72);

  reset(); Weapon[3].mode=0x0b; VehPrototypes[6].icon_offset=127;
  win.icon_branch(3,&rect,0.2f,0);
  assert(draws.size()==1 && draws[0].sprite==1088);

  reset(); Weapon[3].mode=7; Weapon[3].icon=4; VehPrototypes[6].icon_offset=5;
  win.icon_branch(3,&rect,0.2f,0);
  assert(draws.empty() && events.size()==1);

  reset(); Weapon[3].mode=8; Weapon[3].icon=1;
  VehPrototypes[6].icon_offset=-1;
  scale_callback=change_mode_and_icon_after_scale;
  win.icon_branch(3,&rect,0.2f,0);
  assert(draws.size()==1 && draws[0].sprite==120);
  assert(events.size()==2 && events[0]==1 && events[1]==2);
  return 0;
}
"""
    result = compile_run(tmp_path, host_program(text, main))
    assert result.returncode == 0, result.stdout + result.stderr


def test_compiling_wrong_row_member_and_unsigned_mutants_are_rejected(tmp_path: Path) -> None:
    original = SOURCE.read_text()
    branch = icon_branch(original)
    main = r"""
int main() {
  SelectPartWin win;
  reset(); Weapon[3].mode=0x0b;
  VehPrototypes[5].icon_offset=3;
  VehPrototypes[6].icon_offset=10;
  VehPrototypes[6].padding=4;
  win.icon_branch(3,&rect,0.2f,0);
  assert(draws.size()==1 && draws[0].sprite==152);
  return 0;
}
"""
    correct = compile_run(tmp_path / "correct", host_program(original, main))
    assert correct.returncode == 0, correct.stdout + correct.stderr

    wrong_row = branch.replace("VehPrototypes[6]", "VehPrototypes[5]")
    assert wrong_row != branch
    result = compile_run(tmp_path / "wrong-row", host_program(original, main, wrong_row))
    assert result.returncode != 0

    wrong_member = branch.replace(".icon_offset", ".padding")
    assert wrong_member != branch
    result = compile_run(tmp_path / "wrong-member", host_program(original, main, wrong_member))
    assert result.returncode != 0

    negative_main = r"""
int main() {
  SelectPartWin win;
  reset(); Weapon[3].mode=0x0b; VehPrototypes[6].icon_offset=-1;
  win.icon_branch(3,&rect,0.2f,0);
  assert(draws.empty());
  return 0;
}
"""
    unsigned = branch.replace(
        "sprite_idx = VehPrototypes[6].icon_offset;",
        "sprite_idx = static_cast<unsigned char>(VehPrototypes[6].icon_offset);",
    )
    assert unsigned != branch
    result = compile_run(tmp_path / "unsigned-read", host_program(original, negative_main, unsigned))
    assert result.returncode != 0
