"""Extracted filename bodies with allocator doubles, not native audio proof.

The setter double records release but retains that block until the case ends,
so the original failure-path pointer value can be inspected without examining
an invalid host pointer. This does not qualify the original heap's behavior.
"""

from pathlib import Path
import re

import pytest

from test_console_typed_owner_members_oct10 import run_control
from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]


def member():
    header = (ROOT / "src/sound.h").read_text()
    declaration, = re.findall(r"(?m)^\s*(char \*fname_;)", header)
    return declaration


def setter_control(owner):
    body = extract((ROOT / ("src/" + owner.lower() + ".cpp")).read_text(),
                   "int " + owner + "::set_fname(const char *fname)")
    return r'''
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <new>
struct Sound { MEMBER Sound():fname_(nullptr){} };
struct OWNER:Sound {int set_fname(const char *);};
static Sound *active;
static char *old_name;
static bool fail_allocation;
static int events[4],count;
static size_t capacity;
void *operator new(size_t n){
    events[count++]=2;capacity=n;
    // MIDI clears the freed pointer before allocation; Wave does not.
    assert(active->fname_==EXPECTED);
    if(fail_allocation)throw std::bad_alloc();
    return std::malloc(n);
}
void operator delete(void *p) noexcept {
    assert(p==old_name);events[count++]=1;
}
static char *tracked_copy(char *p,const char *s){
    assert(p && p==active->fname_ && capacity==std::strlen(s)+1);
    events[count++]=3;return std::strcpy(p,s);
}
#define strcpy tracked_copy
BODY
#undef strcpy
int main(){
    const char *inputs[]={nullptr,"","a.mid"};
    for(int i=0;i!=3;++i)for(int fail=0;fail!=2;++fail){
        OWNER value;active=&value;count=0;capacity=0;fail_allocation=fail;
        old_name=static_cast<char *>(std::malloc(8));
        std::strcpy(old_name,"old");value.fname_=old_name;
        int result=-1;bool threw=false;
        try{result=value.set_fname(inputs[i]);}catch(const std::bad_alloc&){threw=true;}
        if(!i){
            assert(!threw && result==10 && !count && value.fname_==old_name);
            std::free(old_name);
        }else if(fail){
            assert(threw && count==2 && events[0]==1 && events[1]==2);
            assert(value.fname_==EXPECTED); // Preserve the original failure state.
            std::free(old_name);
        }else{
            assert(!threw && result==0 && count==3);
            assert(events[0]==1 && events[1]==2 && events[2]==3);
            assert(!std::strcmp(value.fname_,inputs[i]));std::free(value.fname_);
            std::free(old_name);
        }
    }
}
'''.replace("MEMBER", member()).replace("OWNER", owner).replace(
        "EXPECTED", "nullptr" if owner == "Midi" else "old_name").replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("owner", ("Wave", "Midi"))
@pytest.mark.parametrize("damage", (None,
    ("return 0xA;", "return 0;"),
    ("operator delete(fname_);", "(void)fname_;"),
    ("operator new(strlen(fname) + 1)", "operator new(strlen(fname) + 2)"),
    ("strcpy(fname_, fname);", "strcpy(nullptr, fname);")))
def test_actual_filename_setters(tmp_path, compiler, owner, damage):
    run_control(tmp_path, compiler, setter_control(owner), damage)


def voice_control():
    source = (ROOT / "src/voice.cpp").read_text()
    bodies = "\n".join(extract(source, name + "::" + name + "()")
                       for name in ("VoiceRx", "VoiceTx"))
    return r'''
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <new>
#include <cstdint>
static int event_count,events[4];
struct Sound {
    MEMBER
    int type_;
    Sound():fname_(nullptr),type_(-1){}
    void set_type(unsigned int n){type_=n;events[event_count++]=static_cast<int>(n);}
};
struct VoiceRx:Sound {VoiceRx();};
struct VoiceTx:Sound {uint32_t flags_54_=0xffffffff;VoiceTx();};
static Sound *active;
static int kind;
void *operator new(size_t n){
    assert(n==10 && active->fname_==nullptr);
    assert(active->type_==(kind==6?6:-1));events[event_count++]=10;
    return std::malloc(n);
}
void operator delete(void *p) noexcept {std::free(p);}
static char *tracked_copy(char *p,const char *s){
    assert(p && p==active->fname_);events[event_count++]=20;return std::strcpy(p,s);
}
#define strcpy tracked_copy
BODIES
#undef strcpy
int main(){
    alignas(VoiceTx) unsigned char storage[sizeof(VoiceTx)];
    kind=6;event_count=0;active=reinterpret_cast<Sound *>(storage);
    VoiceRx *rx=new(storage) VoiceRx;
    assert(!std::strcmp(rx->fname_,"Voice Rx") && rx->type_==6);
    assert(event_count==3 && events[0]==6 && events[1]==10 && events[2]==20);
    std::free(rx->fname_);rx->~VoiceRx();
    kind=7;event_count=0;active=reinterpret_cast<Sound *>(storage);
    VoiceTx *tx=new(storage) VoiceTx;
    assert(!std::strcmp(tx->fname_,"Voice Tx") && tx->type_==7 && tx->flags_54_==0);
    assert(event_count==3 && events[0]==10 && events[1]==20 && events[2]==7);
    std::free(tx->fname_);tx->~VoiceTx();
}
'''.replace("MEMBER", member()).replace("BODIES", bodies)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("Sound::set_type(6);", "Sound::set_type(7);"),
    ("memset(&flags_54_, 0, sizeof(flags_54_));", "(void)flags_54_;")))
def test_actual_voice_filename_construction(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, voice_control(), damage)
