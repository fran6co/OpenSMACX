"""Actual bubble publication/clearing with borrowed mutable caller storage.

Bounded host controls do not prove every producer lifetime, GUI rendering,
native arithmetic/undefined paths, layout or linked preservation.
"""
from pathlib import Path
import re

import pytest

from match_recovery.object_source import parse_objects
from test_texture_sub61f0d0_actual_source import build_and_run, extract_method

ROOT=next(p for p in Path(__file__).resolve().parents if (p/'src/win.cpp').is_file())
SOURCE=ROOT/'src/win.cpp'
SET="void Win::set_bubble_text(char * text, RECT * rect)"
CLEAR="void Win::clear_bubble_text()"
ACTIVE="void __cdecl flip_active_bubble_text()"
PIN="01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"

HOST=r"""
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
#define nullptr 0
struct RECT { int left,top,right,bottom; };
class Win;
// STORAGE_INSERTION
static Win *WinBubbleCompanion;
static RECT WinBubbleRect;
static int WinScreenWidth=100;
static char WinBubblePrefixGlyph[]="+";
struct Font {
    int unk_1_,line_height_,height_;
    int width(char *s) { return static_cast<int>(strlen(s))*3; }
};
static Font font={2,9,4};
static Font *WinBubbleFont=&font;
static std::vector<int> events;
static std::vector<char *> observed;
static std::vector<std::string> painted;
class Win {
public:
    void set_bubble_text(char *,RECT *);
    void clear_bubble_text();
    static int update_screen(RECT *r,Win *w) {
        assert(r==&WinBubbleRect && w==0);
        assert(WinBubbleActive==0 && WinBubbleCompanion==0);
        events.push_back(1); return 0;
    }
    static void flip(RECT *r) {
        assert(r==&WinBubbleRect); events.push_back(2);
        observed.push_back(WinBubbleActive);
        painted.push_back(WinBubbleActive ? WinBubbleActive : "");
    }
};
// BODIES_INSERTION
int main() {
    Win first,second;
    RECT area={20,30,40,60};
    char text[]="ab^c",other[]="long";
    first.set_bubble_text(text,&area);
    assert(events.size()==1 && events[0]==2);
    assert(WinBubbleActive==text && WinBubbleCompanion==&first);
    assert(observed.back()==text && painted.back()=="ab^c");
    assert(strcmp(text,"ab^c")==0); // measurement restores every separator
    assert(WinBubbleRect.left==24 && WinBubbleRect.top==18);
    assert(WinBubbleRect.right==36 && WinBubbleRect.bottom==30);

    text[0]='z'; flip_active_bubble_text();
    assert(observed.back()==text && painted.back()=="zb^c"); // no private snapshot
    const size_t previous=events.size();
    first.set_bubble_text(0,&area); first.set_bubble_text(text,0);
    assert(events.size()==previous && WinBubbleActive==text);

    second.set_bubble_text(other,&area);
    assert(events[previous]==1 && events[previous+1]==2 && events[previous+2]==2);
    assert(observed[observed.size()-2]==0 && observed.back()==other);
    assert(WinBubbleActive==other && WinBubbleCompanion==&second);
    assert(strcmp(text,"zb^c")==0 && strcmp(other,"long")==0);
    first.clear_bubble_text(); // original clears globally, not only its own bubble
    assert(WinBubbleActive==0 && WinBubbleCompanion==0);
    const size_t cleared=events.size();
    first.clear_bubble_text(); flip_active_bubble_text();
    assert(events.size()==cleared);

    font.unk_1_=-1;
    RECT low={0,2,5,20}; first.set_bubble_text(text,&low);
    assert(WinBubbleRect.top==20 && WinBubbleRect.bottom==38);
    assert(WinBubbleRect.left==0 && WinBubbleRect.right==12);
    first.clear_bubble_text();
    RECT right={95,30,100,60}; first.set_bubble_text(other,&right);
    assert(WinBubbleRect.left==82 && WinBubbleRect.right==100);
}
"""


def unit(damage: tuple[str,str] | None=None) -> str:
    source=SOURCE.read_text()
    declaration=re.search(r"^char \*WinBubbleActive;.*$",source,re.M)
    assert declaration is not None
    bodies="\n".join(extract_method(source,s) for s in (SET,CLEAR,ACTIVE))
    if damage:
        old,new=damage
        assert old in bodies
        bodies=bodies.replace(old,new,1)
    return HOST.replace("// STORAGE_INSERTION",declaration[0]).replace("// BODIES_INSERTION",bodies)


def test_binding_preserves_old_identity_with_pointer_target():
    rows=parse_objects(SOURCE.read_text(),'src/win.cpp',PIN,address_space='pe:va')
    row=next(x for x in rows if x.origin.address==0x9b7a50)
    assert row.symbol=='?WinBubbleActive@@3HA'
    assert row.emitted_symbol=='?WinBubbleActive@@3PADA'
    assert row.span is None


@pytest.mark.parametrize('compiler',('g++','clang++'))
def test_actual_borrowed_bubble_text(compiler):
    build_and_run(unit(),compiler,expect_success=True)


@pytest.mark.parametrize('compiler',('g++','clang++'))
@pytest.mark.parametrize('old,new',(
    ('WinBubbleActive = text;','WinBubbleActive = text + 1;'),
    ("*caret = '^';","*caret = 0;"),
    ('WinBubbleCompanion = this;','WinBubbleCompanion = 0;'),
    ('WinBubbleActive = 0;','(void)0;'),
    ('if (WinBubbleActive != 0)\n','if (WinBubbleActive == 0)\n'),
))
def test_rejects_borrowing_and_state_regressions(compiler,old,new):
    build_and_run(unit((old,new)),compiler,expect_success=False)
