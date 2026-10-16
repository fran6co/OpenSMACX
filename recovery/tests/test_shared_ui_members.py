"""Actual-source controls for shared UI members; host layouts are test doubles."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = Path(__file__).resolve().parents[2]


def _masked(source: str) -> str:
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )


def _function(source: str, signature: str) -> str:
    masked = _masked(source)
    starts = [m.start() for m in re.finditer(re.escape(signature), masked)]
    assert len(starts) == 1, (signature, starts)
    start = starts[0]
    opening = masked.index("{", start)
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError(f"unterminated function {signature}")


def _sources():
    cpp = (ROOT / "src/stringstruct.cpp").read_text()
    header = (ROOT / "src/sprite.h").read_text()
    assert "LocalFactionSlot" not in cpp and "WizardTransTable" not in cpp
    wizard = _function(cpp, "void __cdecl tech_wizard_unk2(")
    accessor = _function(header, "unsigned char transparent_index() const")
    assert "g_TECH_ICON_SPRITES[sprite_index].transparent_index()" in wizard
    assert "static_cast<Buffer *>(canvas)" in wizard
    return wizard, accessor


HOST = r"""
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#define __cdecl
struct Buffer { int nonempty_; };
struct Prefix { char bytes[0x444]; };
struct GraphicWin : Prefix, Buffer {};
struct Sprite {
  uint32_t name_,bits_;
  unsigned char cTransparentIndex_;
  char pad_[3];
  int fields_[8];
  ACTUAL_ACCESSOR
  int draw(Buffer *,int,int,int);
};
static_assert(offsetof(Sprite,cTransparentIndex_)==8,"Sprite member offset");
static_assert(sizeof(Sprite)==0x2C,"Sprite element stride");
Sprite g_TECH_ICON_SPRITES[89];
int SpriteScaleNumerator=1, SpriteScaleDenominator=1;
int MapWinCaptionScale=8, MapWinCaptionDivisor=16;
static int drawn_index=-1, drawn_transparency=-1;
static Buffer *drawn_buffer=0;
int Sprite::draw(Buffer *buffer,int transparency,int,int) {
  drawn_index=(int)(this-g_TECH_ICON_SPRITES);
  drawn_transparency=transparency;
  drawn_buffer=buffer;
  return 0;
}
ACTUAL_WIZARD

int main() {
  GraphicWin canvas={};
  Buffer *const expected=static_cast<Buffer *>(&canvas);
  const ptrdiff_t offset=reinterpret_cast<char *>(expected)
                       - reinterpret_cast<char *>(&canvas);
  assert(offset==0x444);
  Buffer *const unadjusted=reinterpret_cast<Buffer *>(
      reinterpret_cast<char *>(&canvas));
  assert(unadjusted!=expected);
  for (int i=0;i<89;++i)
    g_TECH_ICON_SPRITES[i].cTransparentIndex_=(unsigned char)(i^0xA5);
  for (int i=0;i<89;++i) {
    drawn_index=-1; drawn_transparency=-1; drawn_buffer=0;
    tech_wizard_unk2(&canvas,i,i&1,100,120,180);
    assert(drawn_index==i);
    assert(drawn_transparency==(i^0xA5));
    assert(drawn_buffer==expected);
    assert(SpriteScaleNumerator==1 && SpriteScaleDenominator==1);
  }
  drawn_index=-1; tech_wizard_unk2(0,89,0,0,0,0);
  assert(drawn_index==-1);
  drawn_index=-1; drawn_buffer=expected;
  tech_wizard_unk2(0,0,0,0,0,0);
  assert(drawn_index==0 && drawn_buffer==0);
}
"""


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_tech_wizard_uses_graphicwin_buffer_base(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    wizard, accessor = _sources()
    source = HOST.replace("ACTUAL_WIZARD", wizard).replace("ACTUAL_ACCESSOR", accessor)
    cpp, exe = tmp_path / "wizard.cpp", tmp_path / "wizard"
    cpp.write_text(source)
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True,
                   text=True, timeout=30)
    subprocess.run([str(exe)], check=True, timeout=10)

    damaged = wizard.replace("static_cast<Buffer *>(canvas)",
                             "reinterpret_cast<Buffer *>(reinterpret_cast<char *>(canvas))")
    assert damaged != wizard
    mutant_cpp, mutant_exe = tmp_path / "wizard_mutant.cpp", tmp_path / "wizard_mutant"
    mutant_cpp.write_text(HOST.replace("ACTUAL_WIZARD", damaged)
                              .replace("ACTUAL_ACCESSOR", accessor))
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(mutant_cpp), "-o", str(mutant_exe)], check=True,
                   capture_output=True, text=True, timeout=30)
    result = subprocess.run([str(mutant_exe)], capture_output=True, timeout=10)
    assert result.returncode != 0
