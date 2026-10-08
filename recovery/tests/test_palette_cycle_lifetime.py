"""Actual Palette::init_cycle with observed allocation and timer dependencies.

The complete method, PaletteInternal and palette-entry declaration are copied
unchanged from source. A fixture namespace resolves free/memcpy/mem_get to
recording doubles; Time owns matching global-new/global-delete storage. These
tests establish bounded state transitions, not native layouts, VC6 SEH, real
timer callbacks or game acceptance. Exhausting all five slots is deliberately
outside the valid domain, not silently given a fabricated sixth slot.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated actual definition")


def actual_source():
    header = (ROOT / "src/jackal/palette.h").read_text()
    fields = braced(header, "struct PaletteInternal {") + " internal_[5];"
    entries, = re.findall(r"PALETTEENTRY entries_\[256\];", header)
    body = braced((ROOT / "src/jackal/palette.cpp").read_text(), "int Palette::init_cycle(")
    return entries + "\n" + fields, body


HARNESS = r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <new>
#define __cdecl
void check(bool ok,int line) {
    if(!ok) {std::fprintf(stderr,"assertion line %d\n",line);std::exit(79);}
}
#define require(x) check((x),__LINE__)
namespace fixture { void observe_delete(void *); }
// Pair class allocation and the product's explicit global delete through
// the same allocator. No malloc/new mismatch is hidden from ASan.
void *operator new(std::size_t size) {
    if(void *p=std::malloc(size)) return p;
    throw std::bad_alloc();
}
void operator delete(void *p) noexcept {
    fixture::observe_delete(p);std::free(p);
}
namespace fixture {
char events[100];unsigned event_count;
bool recording=true,fail_mem,fail_new,throw_ctor;
bool check_teardown;
uint32_t old_key;uint8_t old_first,old_count;
void *timer_allocation,*colour_allocation;
unsigned request_bytes,copy_bytes;
const void *copy_source;
int expected_key,expected_first,expected_count;
uint32_t expected_interval;
int constructor_calls,destructor_calls,delete_calls;
void observe_destructor();
void event(char c) {if(recording){require(event_count+1<sizeof(events));events[event_count++]=c;events[event_count]=0;}}
void observe_delete(void *p) {
    if(p && p==timer_allocation) {event('D');++delete_calls;timer_allocation=nullptr;}
}
struct Time {
    unsigned char storage[40];
    static void *operator new(std::size_t size) noexcept {
        event('N');require(size==40);
        if(fail_new)return nullptr;
        timer_allocation=::operator new(size);return timer_allocation;
    }
    static void operator delete(void *p) noexcept {::operator delete(p);}
    Time() {event('C');++constructor_calls;if(throw_ctor)throw 123;}
    ~Time() {event('T');++destructor_calls;observe_destructor();}
    void init(void (*)(int,int),int,int,uint32_t,uint32_t);
};
struct PALETTEENTRY {uint8_t red,green,blue,flags;};
static_assert(sizeof(PALETTEENTRY)==4,"target palette entry byte width");
struct Palette {
    ACTUAL_FIELDS
    static void timer_callback(int,int) {}
    int init_cycle(int,int,int,unsigned long);
} palette;
void *PaletteInitialized;
Palette::PaletteInternal *target;
void observe_destructor() {
    if(recording && check_teardown)
        require(target->key==old_key && target->first==old_first && target->count==old_count);
}
void Time::init(void (*callback)(int,int),int key,int context,uint32_t interval,uint32_t resolution) {
    event('I');require(callback==Palette::timer_callback && key==expected_key);
    require(context==static_cast<int>(reinterpret_cast<intptr_t>(&palette)));
    require(interval==expected_interval && resolution==5);
    require(target->time==this);
}
void *mem_get(unsigned int bytes) {
    event('M');request_bytes=bytes;
    require(target->key==static_cast<uint32_t>(expected_key));
    require(target->first==expected_first && target->count==static_cast<uint8_t>(expected_count));
    require(target->time==nullptr && target->colours==nullptr);
    check_teardown=false;
    if(fail_mem)return nullptr;
    require(bytes<=1024);
    colour_allocation=std::malloc(bytes ? bytes : 1);require(colour_allocation);
    return colour_allocation;
}
void *memcpy(void *destination,const void *source,std::size_t bytes) {
    event('P');copy_bytes=bytes;copy_source=source;
    require(destination==colour_allocation && destination==target->colours);
    require(bytes==request_bytes && source==&palette.entries_[expected_first]);
    return std::memcpy(destination,source,bytes);
}
void free(void *p) {
    event('F');require(p==colour_allocation);
    if(check_teardown) {
        require(target->time==nullptr);
        require(target->key==old_key && target->first==old_first && target->count==old_count);
    }
    std::free(p);colour_allocation=nullptr;
}
ACTUAL_BODY
void cleanup() {
    recording=false;
    for(auto &entry:palette.internal_) {
        if(entry.time){entry.time->~Time();::operator delete(entry.time);entry.time=nullptr;}
        if(entry.colours){std::free(entry.colours);entry.colours=nullptr;}
    }
    colour_allocation=nullptr;require(timer_allocation==nullptr);recording=true;
}
void reset() {
    cleanup();std::memset(&palette,0,sizeof(palette));
    for(auto &entry:palette.internal_)entry.key=uint32_t(-1);
    for(unsigned i=0;i<256;++i)palette.entries_[i]={uint8_t(i),uint8_t(i+17),uint8_t(i^0x55),uint8_t(i+93)};
    PaletteInitialized=&palette;target=&palette.internal_[0];
    fail_mem=fail_new=throw_ctor=false;event_count=0;events[0]=0;
    check_teardown=false;
    request_bytes=copy_bytes=0;copy_source=nullptr;
    constructor_calls=destructor_calls=delete_calls=0;
}
void expect(int key,int first,int count,uint32_t interval,int slot=0) {
    expected_key=key;expected_first=first;expected_count=count;expected_interval=interval;
    target=&palette.internal_[slot];
}
void verify_success(int key,int first,int count,unsigned long interval,const char *sequence) {
    require(palette.init_cycle(key,first,count,interval)==0);
    require(std::strcmp(events,sequence)==0);
    require(request_bytes==4u*static_cast<unsigned>(count) && copy_bytes==request_bytes);
    require(target->key==static_cast<uint32_t>(key) && target->first==(first&255));
    require(target->count==static_cast<uint8_t>(count) && target->time);
    require(std::memcmp(target->colours,&palette.entries_[first&255],request_bytes)==0);
}
int main() {
    reset();expect(41,12,5,900);verify_success(41,12,5,900,"MPNCI");
    // Reuse closes the old timer before its storage, then frees colours;
    // no new metadata or allocations precede this teardown.
    event_count=0;events[0]=0;expect(41,20,3,77);
    check_teardown=true;old_key=41;old_first=12;old_count=5;
    verify_success(41,20,3,77,"TDFMPNCI");
    require(destructor_calls==1 && delete_calls==1);

    // Low-byte first normalization applies before both signed guards.
    for(int first : {12,268,-244}) {
        reset();expect(2,12,5,900);verify_success(2,first,5,900,"MPNCI");
    }
    reset();expect(3,245,1,0);verify_success(3,245,1,0,"MPNCI");
    reset();expect(3,246,0,0);verify_success(3,246,0,0,"MPNCI");
    // The timer's target32 interval truncates an LP64 caller too.
    reset();expect(3,10,2,7);verify_success(3,10,2,0x100000007ul,"MPNCI");

    struct Rejection {int key,first,count;};
    const Rejection invalid[]={{1,9,1},{1,265,1},{1,245,2},{1,255,0},{-1,12,5}};
    for(const auto &c:invalid) {
        reset();PaletteInitialized=nullptr;
        require(palette.init_cycle(c.key,c.first,c.count,900)==3);
        require(events[0]==0 && target->key==uint32_t(-1));
    }
    reset();PaletteInitialized=nullptr;
    require(palette.init_cycle(1,12,5,900)==7 && events[0]==0);

    // First free or matching slot wins; a later matching key does not
    // override an earlier free slot. No sixth-slot contract is invented.
    reset();palette.internal_[0].key=90;palette.internal_[1].key=91;
    expect(7,12,5,900,2);verify_success(7,12,5,900,"MPNCI");
    require(palette.internal_[0].key==90 && palette.internal_[1].key==91);
    reset();palette.internal_[3].key=7;
    expect(7,12,5,900);verify_success(7,12,5,900,"MPNCI");
    require(palette.internal_[3].key==7 && palette.internal_[3].colours==nullptr);

    reset();expect(8,12,5,900);fail_mem=true;
    require(palette.init_cycle(8,12,5,900)==4 && std::strcmp(events,"M")==0);
    require(request_bytes==20 && target->time==nullptr && target->colours==nullptr);
    // Negative counts pass the original signed bound. Failing allocation
    // observes unsigned32 LEA wrap, never attempts a huge host copy.
    for(int count : {-1,-257,-2147483647-1}) {
        reset();expect(8,12,count,900);fail_mem=true;
        require(palette.init_cycle(8,12,count,900)==4 && std::strcmp(events,"M")==0);
        require(request_bytes==4u*static_cast<uint32_t>(count));
        require(target->count==static_cast<uint8_t>(count));
    }

    reset();expect(9,12,5,900);fail_new=true;
    require(palette.init_cycle(9,12,5,900)==4 && std::strcmp(events,"MPN")==0);
    require(target->colours && target->time==nullptr && constructor_calls==0);
    require(std::memcmp(target->colours,&palette.entries_[12],20)==0);
    // Retry after failed new preserves the key and frees only the retained
    // colour allocation; there never was a timer to destroy.
    event_count=0;events[0]=0;fail_new=false;expect(9,20,3,77);
    verify_success(9,20,3,77,"FMPNCI");
    // Constructor exception frees the unfinished Time allocation, not the
    // already published copied colours. Cleanup of retained state is separate.
    reset();expect(10,12,5,900);throw_ctor=true;
    bool caught=false;
    try {palette.init_cycle(10,12,5,900);} catch(int error){require(error==123);caught=true;}
    require(caught && std::strcmp(events,"MPNCD")==0);
    require(target->colours && target->time==nullptr && destructor_calls==0 && delete_calls==1);
    // Allocation failures during reuse occur AFTER complete old teardown.
    for(int failure : {0,1}) {
        reset();expect(11,12,5,900);verify_success(11,12,5,900,"MPNCI");
        event_count=0;events[0]=0;expect(11,20,3,77);
        check_teardown=true;old_key=11;old_first=12;old_count=5;
        fail_mem=failure==0;fail_new=failure==1;
        require(palette.init_cycle(11,20,3,77)==4);
        require(std::strcmp(events,failure==0 ? "TDFM" : "TDFMPN")==0);
        require(target->time==nullptr && bool(target->colours)==(failure==1));
    }
    cleanup();return 0;
}
}
int main(){return fixture::main();}
'''


def run_host(tmp_path, fields, body):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    source = tmp_path / "cycle.cpp"
    source.write_text(HARNESS.replace("ACTUAL_FIELDS", fields).replace("ACTUAL_BODY", body))
    binary = tmp_path / "cycle"
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_palette_cycle_lifetime(tmp_path):
    result = run_host(tmp_path, *actual_source())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("(unsigned char)first", "first"),
    ("(int)count)", "(int)interval)"),
    ("< (int)count", "<= (int)count"),
    ("entry->count = (unsigned char)count", "entry->count = (unsigned char)interval"),
    ("static_cast<unsigned int>(count) * 4u", "static_cast<unsigned int>(interval) * 4u"),
    ("static_cast<unsigned int>(count) * 4u", "static_cast<unsigned int>(count) * 3u"),
    ("static_cast<uint32_t>(interval), 5", "static_cast<uint32_t>(count), 5"),
    ("static_cast<uint32_t>(interval), 5", "static_cast<uint32_t>(interval), 4"),
    ("key == -1", "key == 0"),
    ("return 7;", "return 3;"),
    ("key == id || id == -1", "slot == 0 || key == id || id == -1"),
    ("t->~Time();", ";"),
    ("entry->time = 0;", ";"),
    ("free(p);", ";"),
    ("entry->colours = 0;", ";"),
    ("&entries_[first_index]", "&entries_[first_index + 1]"),
    ("return 4;", "return 7;"),
    ("entry->time = t;", ";"),
])
def test_palette_cycle_lifetime_damage_is_observed(tmp_path, before, after):
    fields, body = actual_source()
    assert before in body
    result = run_host(tmp_path, fields, body.replace(before, after))
    assert result.returncode == 79, result.stderr
