"""Actual-source shared-member controls; host layouts are test doubles only."""
from pathlib import Path
import re
import shutil
import subprocess
import pytest
ROOT = Path(__file__).resolve().parents[2]

def _masked(source: str) -> str:
    result = list(source)
    state, quote, i = "code", "", 0
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if ch == "/" and nxt in ("/", "*"):
                result[i] = result[i + 1] = " "
                state = "line" if nxt == "/" else "block"
                i += 2
                continue
            if ch in ('"', "'"):
                quote = ch
                result[i] = " "
                state = "quoted"
        elif state == "line":
            if ch == "\n":
                state = "code"
            else:
                result[i] = " "
        elif state == "block":
            if ch == "*" and nxt == "/":
                result[i] = result[i + 1] = " "
                state = "code"
                i += 2
                continue
            if ch != "\n":
                result[i] = " "
        else:
            if ch == "\\":
                result[i] = " "
                if i + 1 < len(source):
                    result[i + 1] = " "
                    i += 2
                    continue
            elif ch == quote:
                result[i] = " "
                state = "code"
            elif ch != "\n":
                result[i] = " "
        i += 1
    return "".join(result)


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
    multi = _function(cpp, "void __fastcall MultiWin_UNK1(")
    wizard = _function(cpp, "void __cdecl tech_wizard_unk2(")
    accessor = _function(header, "unsigned char transparent_index() const")
    assert "ConsoleGlobal.faction_id_" in multi
    assert "g_TECH_ICON_SPRITES[sprite_index].transparent_index()" in wizard
    return multi, wizard, accessor

HOST = r"""
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#define __fastcall
#define __cdecl
struct RECT { int left, top, right, bottom; };
static int spot_calls, write_calls, last_spot[6], last_write_len;
static char last_write[128];
struct Buffer { void write_cent_l(char *s, RECT *, unsigned int len) {
  ++write_calls; last_write_len=(int)len; strcpy(last_write,s);
} };
struct Spot { void add(int a,int b,int x,int y,int w,int h) {
  ++spot_calls; last_spot[0]=a; last_spot[1]=b; last_spot[2]=x;
  last_spot[3]=y; last_spot[4]=w; last_spot[5]=h;
} };
#pragma pack(push,1)
struct MultiWin {
  char prefix[0x444]; Buffer buffer;
  char gap[0x28AC-0x444-sizeof(Buffer)]; Spot spot;
};
#pragma pack(pop)
static_assert(offsetof(MultiWin,buffer)==0x444,"measured buffer receiver");
static_assert(offsetof(MultiWin,spot)==0x28AC,"measured spot receiver");
struct ConsoleFixture { char prefix[0x23BD4]; int faction_id_; };
static_assert(offsetof(ConsoleFixture,faction_id_)==0x23BD4,"Console member offset");
ConsoleFixture ConsoleGlobal;
unsigned char PlayersData[8][0x2100];
static void set_difficulty(int faction,int value) {
  memcpy(&PlayersData[faction][0x2054],&value,sizeof(value));
}
char StringTemp[128];
const char BufferJoinSpace[]=" ";
char caret[]="^"; char *const CaretSeparator=caret;
ACTUAL_MULTWIN

struct Sprite {
  uint32_t name_,bits_;
  unsigned char cTransparentIndex_;
  char pad_[3];
  int fields_[8];
  ACTUAL_ACCESSOR
  int draw(struct Buffer *,int,int,int);
};
static_assert(offsetof(Sprite,cTransparentIndex_)==8,"Sprite member offset");
static_assert(sizeof(Sprite)==0x2C,"Sprite element stride");
Sprite g_TECH_ICON_SPRITES[89];
int SpriteScaleNumerator=1, SpriteScaleDenominator=1;
int MapWinCaptionScale=8, MapWinCaptionDivisor=16;
static int drawn_index=-1, drawn_transparency=-1;
int Sprite::draw(Buffer *,int transparency,int,int) {
  drawn_index=(int)(this-g_TECH_ICON_SPRITES);
  drawn_transparency=transparency;
  return 0;
}
ACTUAL_WIZARD

int main() {
  RECT r={3,4,23,34}; MultiWin w={};
  ConsoleGlobal.faction_id_=2;
  set_difficulty(1,3); // would yield three if the old unrelated id were used
  set_difficulty(2,2);
  MultiWin_UNK1(&w,0,&r);
  assert(spot_calls==1 && last_spot[0]==3 && last_spot[1]==2);
  assert(last_spot[2]==3 && last_spot[3]==4 && last_spot[4]==20 && last_spot[5]==30);
  assert(write_calls==1 && last_write_len==3 && strcmp(last_write," ^^")==0);
  assert(strcmp(StringTemp," ^^")==0);
  set_difficulty(2,4);
  MultiWin_UNK1(&w,0,&r);
  assert(write_calls==2 && last_write_len==4 && strcmp(last_write," ^^^")==0);
  set_difficulty(2,0);
  MultiWin_UNK1(&w,0,&r);
  assert(write_calls==2); // zero carets preserves the original no-write branch

  for (int i=0;i<89;++i) g_TECH_ICON_SPRITES[i].cTransparentIndex_=(unsigned char)(i^0xA5);
  for (int i=0;i<89;++i) {
    drawn_index=-1; drawn_transparency=-1;
    tech_wizard_unk2(0,i,i&1,100,120,180);
    assert(drawn_index==i);
    assert(drawn_transparency==(i^0xA5));
    assert(SpriteScaleNumerator==1 && SpriteScaleDenominator==1);
  }
  drawn_index=-1; tech_wizard_unk2(0,89,0,0,0,0); assert(drawn_index==-1);
}
"""



@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_members_follow_current_owner_state(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    multi, wizard, accessor = _sources()
    source = HOST.replace("ACTUAL_MULTWIN", multi).replace("ACTUAL_WIZARD", wizard).replace("ACTUAL_ACCESSOR", accessor)
    cpp, exe = tmp_path / "owners.cpp", tmp_path / "owners"
    cpp.write_text(source)
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True, text=True, timeout=30)
    subprocess.run([str(exe)], check=True, timeout=10)

