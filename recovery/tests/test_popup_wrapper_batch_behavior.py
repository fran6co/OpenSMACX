"""Actual-owner extraction controls proposed for recovery/tests.

The tested C++ definitions are extracted from popup.cpp/xpops.cpp by ORIGINAL
address. No ignored body fragment or separately copied production function is
loaded by this test. Dependency objects and callee cores below are deliberate
minimal host doubles, not product ABI/layout or runtime proof.
"""

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

import pytest


HARNESS_PREFIX = r'''#include <cassert>
#include <cstdio>
#include <cstring>
#define __cdecl
struct Sprite {};
struct StringStructPayload { char *text; };
struct StringStructEntry { StringStructPayload *payload; StringStructEntry *next; StringStructEntry *previous; };
struct StringList {
    int entry_count_; StringStructEntry *head_; StringStructEntry *current_;
    char *pending_text_; unsigned int field_20_; unsigned int field_24_;
    int added_id;
    StringList() : entry_count_(0), head_(0), current_(0), pending_text_(0),
                   field_20_(0), field_24_(0), added_id(0) {}
    int seek_pos(int position) {
        if (entry_count_ == 0 || head_ == 0) return 0;
        current_ = position < 0 ? head_->previous : head_; return 1;
    }
    StringStructPayload *next_entry() {
        if (!current_) return 0;
        current_ = current_->next; return current_->payload;
    }
    int add(int id) { added_id = id; ++entry_count_; return 0; }
};
struct Popup {
    static int constructed, destroyed, start_calls, populate_history;
    static StringStructEntry entries[2]; static StringStructPayload payloads[2];
    static char texts[2][16]; StringList unit_ids_;
    Popup() { ++constructed; }
    ~Popup() { ++destroyed; }
    int start(char *, const char *, int, char *, int, void *) {
        ++start_calls;
        if (populate_history) {
            std::strcpy(texts[0], "first"); std::strcpy(texts[1], "last");
            payloads[0].text = texts[0]; payloads[1].text = texts[1];
            entries[0].payload = &payloads[0]; entries[1].payload = &payloads[1];
            entries[0].next = &entries[1]; entries[0].previous = &entries[1];
            entries[1].next = &entries[0]; entries[1].previous = &entries[0];
            unit_ids_.head_ = &entries[0]; unit_ids_.entry_count_ = 2;
        }
        return 0;
    }
};
int Popup::constructed=0; int Popup::destroyed=0; int Popup::start_calls=0;
int Popup::populate_history=0; StringStructEntry Popup::entries[2];
StringStructPayload Popup::payloads[2]; char Popup::texts[2][16] = {};
struct Player { int rule_flags; }; struct Console { int faction_id_; };
struct Interface {
    int calls, text_id, x, a, b; char *text;
    Interface() : calls(0),text_id(0),x(0),a(0),b(0),text(0) {}
    void set_message(char *s,int id,int xv,int aa,int bb) {
        ++calls;text=s;text_id=id;x=xv;a=aa;b=bb;
    }
};
static const int RFLAG_ALIEN=0x80; static const int PlayerMax=8;
Player Players[PlayerMax] = {}; Console ConsoleGlobal={0};
int ExpansionEnabled=0, PopupDialogFactionID=0, IsMultiplayerPBEM=0;
struct GameDataLockMaskView { unsigned int enable_mask_; };
struct GameDataStateWindowFields { GameDataLockMaskView lock_masks_; };
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
GameDataStateWindow GameDataStateStorage = {};
int MapLongitudeBounds=20, MapLatitudeBounds=10;
typedef const char *LPCSTR; char *PopupStartAltFile1=const_cast<char *>("ALIENTXT1");
char *PopupStartAltFile2=const_cast<char *>("ALIENTXT2");
char PopupStartCaption[256]="SCRIPT"; char *ScriptPopupFile=const_cast<char *>("SCRIPT.txt");
char StringTemp[1032]={}; StringList MessageWinDeferredBaseMessages[8]; Interface MainInterfaceGlobal;
static int core_result=17, core_calls, core_length, core_value, core_fa, core_fb;
static char *core_caption,*core_label,*core_text; static Sprite *core_sprite;
static int (__cdecl *core_callback)();
int __cdecl pops(char *c,char *l,int n,char *t,int v,Sprite *s,int a,int b,
                 int (__cdecl *cb)()) {
    ++core_calls;core_caption=c;core_label=l;core_length=n;core_text=t;
    core_value=v;core_sprite=s;core_fa=a;core_fb=b;core_callback=cb;
    return core_result;
}
static int ask_calls, ask_value, ask_flags; static char *ask_caption,*ask_label,*ask_text;
static int (__cdecl *ask_callback)();
int __cdecl X_pop_ask(char *c,const char *l,int v,char *t,
                      int (__cdecl *cb)(),int f) {
    ++ask_calls;ask_caption=c;ask_label=const_cast<char *>(l);ask_value=v;
    ask_text=t;ask_callback=cb;ask_flags=f;return 23;
}
void __cdecl POP3(const char *,int,int,int);
void __cdecl POP3(const char *,const char *,int,int,int);
int __cdecl pop(char *,char *,char *,int,int (__cdecl *)());
int __cdecl pops(char *,char *,char *,int,Sprite *,int (__cdecl *)());
int __cdecl pops(char *,char *,char *,int,Sprite *,int,int,int (__cdecl *)());
int __cdecl X_pop_ask(const char *,int,int (__cdecl *)(),int);
int __cdecl X_pop_ask(const char *,char *,int (__cdecl *)(),int);
int __cdecl X_pops(char *,const char *,int,char *,int,Sprite *,int,int,int (__cdecl *)());
'''

HARNESS_SUFFIX = r'''
static int callback_one(){return 1;}
int main() {
    char caption[]="CAPTION", label[]="LABEL", text[]="abc"; Sprite sprite;
    assert(pop(caption,label,0,4,callback_one)==-1 && core_calls==0);
    assert(pop(caption,label,text,4,callback_one)==core_result);
    assert(core_length==3 && core_value==4 && core_caption==caption && core_label==label);
    assert(core_text==text && core_sprite==0 && core_fa==1 && core_fb==1);
    assert(core_callback==callback_one);
    core_calls=0;
    assert(pops(caption,label,text,4,&sprite,callback_one)==core_result);
    assert(core_calls==1 && core_length==3 && core_sprite==&sprite && core_fa==1 && core_fb==1);
    assert(pops(caption,label,text,4,&sprite,7,9,callback_one)==core_result);
    assert(core_calls==2 && core_fa==7 && core_fb==9);
    ask_calls=0;
    assert(X_pop_ask(label,12,callback_one,0x44)==23);
    assert(ask_calls==1 && ask_caption==PopupStartCaption && ask_label==label);
    assert(ask_value==12 && ask_text==0 && ask_callback==callback_one && ask_flags==0x44);
    assert(X_pop_ask(label,text,callback_one,0x80)==23 && ask_value==3 && ask_text==text);
    assert(X_pop_ask(label,static_cast<char *>(0),callback_one,0x80)==-1 && ask_calls==2);
    core_calls=0; ExpansionEnabled=0;
    assert(X_pops(caption,label,3,text,5,&sprite,7,9,callback_one)==core_result);
    assert(core_calls==1 && core_length==3 && core_text==text && core_value==5);
    assert(core_sprite==&sprite && core_fa==7 && core_fb==9 && core_callback==callback_one);
    MainInterfaceGlobal.calls=0; IsMultiplayerPBEM=0; Popup::populate_history=0;
    POP3("SECTION",0,12,34);
    assert(MainInterfaceGlobal.calls==1 && MainInterfaceGlobal.text_id==0x5F);
    assert(MainInterfaceGlobal.x==0 && MainInterfaceGlobal.a==1 && MainInterfaceGlobal.b==1);
    assert(Popup::constructed==Popup::destroyed && Popup::start_calls==1);
    Popup::populate_history=1;
    POP3("SECTION",0,12,34);
    assert(std::strcmp(StringTemp,"first last")==0);
    assert(Popup::constructed==Popup::destroyed);
    Popup::populate_history=0; IsMultiplayerPBEM=1;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_=
        (0xFFu<<8)|(1u<<2)|(1u<<4);
    POP3("SECTION",0,12,3);
    assert(MessageWinDeferredBaseMessages[2].added_id==static_cast<int>((20*3+12)|0x80000000u));
    assert(MessageWinDeferredBaseMessages[4].added_id==static_cast<int>((20*3+12)|0x80000000u));
    assert(MessageWinDeferredBaseMessages[1].added_id==0);
    assert(MessageWinDeferredBaseMessages[2].pending_text_==StringTemp);
    POP3("SECTION",3,7,2);
    assert(MessageWinDeferredBaseMessages[3].added_id==static_cast<int>((20*2+7)|0x80000000u));
    assert(MessageWinDeferredBaseMessages[3].pending_text_==StringTemp);
    assert(Popup::constructed==Popup::destroyed);
    return 0;
}
'''


ADDRESS_SIGNATURES = {
    0x0050B8D0: "void __cdecl POP3(const char *section, int mode",
    0x004055A0: "void __cdecl POP3(const char *section, const char *unused",
    0x00627190: "int __cdecl pop(char *caption",
    0x00627440: "int __cdecl pops(char *caption, char *label, char *text, int value,\n                 Sprite *sprite, int (__cdecl *callback",
    0x00627480: "int __cdecl pops(char *caption, char *label, char *text, int value,\n                 Sprite *sprite, int flag_a",
    0x005BFB10: "int __cdecl X_pop_ask(const char *label, int value",
    0x005BFB80: "int __cdecl X_pop_ask(const char *label, char *text",
    0x005BF930: "int __cdecl X_pops(char *caption, const char *label, int text_length",
}


def extract_actual_definition(path: Path, address: int) -> str:
    text = path.read_text(encoding="utf-8")
    marker = f"// ORIGINAL: 0x{address:08X} "
    matches = [m.start() for m in __import__("re").finditer(
        r"(?m)^// ORIGINAL: 0x[0-9A-Fa-f]{8} .*?$", text)
        if text[m.start():m.end()].startswith(marker)]
    assert len(matches) == 1, f"expected one source marker for {address:08X} in {path}"
    tail = text[matches[0]:]
    expected = ADDRESS_SIGNATURES[address]
    sig_at = tail.find(expected)
    assert sig_at >= 0, f"missing actual owner definition for {address:08X}"
    start = matches[0] + sig_at
    depth = 0
    state = "code"
    escape = False
    i = text.find("{", start)
    assert i >= 0
    begin = start
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if state == "line":
            if c == "\n": state = "code"
        elif state == "block":
            if c == "*" and n == "/": state = "code"; i += 1
        elif state in ("string", "char"):
            if escape: escape = False
            elif c == "\\": escape = True
            elif (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        else:
            if c == "/" and n == "/": state = "line"; i += 1
            elif c == "/" and n == "*": state = "block"; i += 1
            elif c == '"': state = "string"
            elif c == "'": state = "char"
            elif c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return text[begin:i + 1]
        i += 1
    raise AssertionError(f"unterminated actual body {address:08X}")


def source_root() -> Path:
    explicit = os.environ.get("OPENSMACX_POPUP_TEST_SOURCE_ROOT")
    if explicit:
        return Path(explicit).resolve()
    return Path(__file__).resolve().parents[2]


def build_program(root: Path, damage_cursor: bool = False) -> str:
    popups = root / "src/popup.cpp"
    xpops = root / "src/xpops.cpp"
    definitions = []
    for address in ADDRESS_SIGNATURES:
        owner = xpops if address in (0x5BFB10, 0x5BFB80, 0x5BF930) else popups
        definition = extract_actual_definition(owner, address)
        if damage_cursor and address == 0x004055A0:
            good = "StringStructPayload *const history_payload = history.next_entry();"
            assert good in definition
            definition = definition.replace(
                good,
                "StringStructPayload *const history_payload = history.head_->previous->payload;",
                1)
        definitions.append(definition)
    helper_start = xpops.read_text(encoding="utf-8").find(
        "static const char XPopModCheckString2Text[]")
    helper_end = xpops.read_text(encoding="utf-8").find(
        "/*\nPurpose: The full builder", helper_start)
    assert helper_start >= 0 and helper_end > helper_start
    helpers = xpops.read_text(encoding="utf-8")[helper_start:helper_end]
    return HARNESS_PREFIX + helpers + "\n".join(definitions) + HARNESS_SUFFIX


def compile_and_run(tmp_path: Path, compiler: str, source: str) -> subprocess.CompletedProcess[str]:
    source_path = tmp_path / f"popup_actual_{Path(compiler).name}.cpp"
    binary = tmp_path / f"popup_actual_{Path(compiler).name}"
    source_path.write_text(source, encoding="utf-8")
    built = subprocess.run([
        compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=undefined", "-fno-sanitize-recover=all",
        str(source_path), "-o", str(binary)], capture_output=True, text=True)
    if built.returncode != 0:
        return built
    return subprocess.run([str(binary)], capture_output=True, text=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_popup_owners_and_cursor_damage(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    root = source_root()
    completed = compile_and_run(tmp_path, compiler, build_program(root))
    assert completed.returncode == 0, completed.stdout + completed.stderr
    damaged = compile_and_run(tmp_path, compiler, build_program(root, damage_cursor=True))
    assert damaged.returncode != 0 and "Assertion" in damaged.stderr, (
        "the multi-entry cursor control failed to detect a stale-tail regression")


NUMERIC_HARNESS = r'''#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#define __cdecl
typedef int BOOL;
struct PopupStartCall {
    char *caption; char *label; int title; char *text; int flags; void *window;
};
struct Win {
    int visible;
    Win() : visible(1) {}
    int is_visible() { return visible; }
};
struct BasePop : Win {
    int external_window_, field_3100_, start_result, exec_result;
    int write_value, write_calls, exec_calls, close_calls, start_calls;
    int event_count, events[8];
    PopupStartCall start_args;
    int (__cdecl *exec_callback)();
    BasePop() : external_window_(0), field_3100_(0), start_result(0),
        exec_result(37), write_value(0), write_calls(0), exec_calls(0),
        close_calls(0), start_calls(0), event_count(0), exec_callback(0) {}
    int start(char *caption, char *label, int title, char *text,
              int flags, void *window) {
        ++start_calls; events[event_count++]=1;
        start_args.caption=caption;start_args.label=label;start_args.title=title;
        start_args.text=text;start_args.flags=flags;start_args.window=window;
        return start_result;
    }
    void write_check(long value) {
        ++write_calls;write_value=static_cast<int>(value);events[event_count++]=2;
    }
    int exec(int mode,int (__cdecl *callback)()) {
        ++exec_calls;exec_callback=callback;events[event_count++]=3;
        assert(mode==0);return exec_result;
    }
    void close() { ++close_calls;events[event_count++]=5; }
};
BasePop *PopupInstanceSlotA;
BasePop *PopupInstanceSlotB;
char StringTemp[1032];
char PopupStartCaption[256]="SCRIPT";
int PopupWriteCheckState=0x1234;
int ConsoleGroupAction=-1;
struct ParseText { char str[64]; };
ParseText ParseStrBuffer[10];
int ParseNumTable[10];
int draw_calls;
void do_all_draws() { ++draw_calls; }
char *_itoa(int value,char *buffer,int radix) {
    assert(radix==10); std::sprintf(buffer,"%d",value); return buffer;
}
static int callback_result() { return 1; }
'''


def extract_numeric_body(source: str) -> str:
    marker = "// ORIGINAL: 0x00627A90 "
    starts = [i for i in range(len(source)) if source.startswith(marker, i)]
    assert len(starts) == 1, "numeric overload marker must exist exactly once"
    expected = "int __cdecl pop_ask_number(char *caption, char *label,"
    start = source.find(expected, starts[0])
    assert start >= 0, "full numeric overload must be defined in its real owner"
    open_brace = source.find("{", start)
    assert open_brace >= 0
    depth = 0
    state = "code"
    escape = False
    i = open_brace
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "line":
            if c == "\n": state = "code"
        elif state == "block":
            if c == "*" and n == "/": state = "code"; i += 1
        elif state in ("string", "char"):
            if escape: escape = False
            elif c == "\\": escape = True
            elif (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        else:
            if c == "/" and n == "/": state = "line"; i += 1
            elif c == "/" and n == "*": state = "block"; i += 1
            elif c == '"': state = "string"
            elif c == "'": state = "char"
            elif c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0: return source[start:i + 1]
        i += 1
    raise AssertionError("unterminated numeric popup body")


def numeric_source_root() -> Path:
    explicit = os.environ.get("OPENSMACX_POPUP_TEST_SOURCE_ROOT")
    if explicit: return Path(explicit).resolve()
    return Path(__file__).resolve().parents[2]


def test_popup_wrappers_keep_allocator_owned_slot_type() -> None:
    root = numeric_source_root()
    header = (root / "src/basepop.h").read_text(encoding="utf-8")
    implementation = (root / "src/basepop.cpp").read_text(encoding="utf-8")
    popup = (root / "src/popup.cpp").read_text(encoding="utf-8")
    assert "extern BasePop *PopupInstanceSlotA;" in header
    assert "extern BasePop *PopupInstanceSlotB;" in header
    assert "BasePop *PopupInstanceSlotA;" in implementation
    assert "BasePop *PopupInstanceSlotB;" in implementation
    assert "Win *win = PopupInstanceSlotB;" not in popup
    assert "reinterpret_cast<BasePop *>(win)" not in popup
    assert "static_cast<BasePop *>(PopupInstanceSlot" not in popup


def make_numeric_source(root: Path, damage: str = "") -> str:
    definition = extract_numeric_body((root / "src/popup.cpp").read_text(encoding="utf-8"))
    if damage == "nonzero-seed":
        old = "_itoa(0, number_text, 10);"
        assert old in definition
        definition = definition.replace(old, "_itoa(5, number_text, 10);", 1)
    return NUMERIC_HARNESS + definition + r'''
int main() {
    char label[]="ASK_NUMBER", custom_caption[]="CUSTOM";
    char failure_caption[]="FAIL", external_caption[]="EXTERNAL";
    BasePop popup_a, popup_b;
    PopupInstanceSlotA=&popup_a; PopupInstanceSlotB=&popup_b;
    PopupInstanceSlotA->visible=1; PopupInstanceSlotB->visible=1;
    popup_b.field_3100_=0x77; ParseNumTable[0]=-1;
    std::strcpy(ParseStrBuffer[0].str,"42");
    ConsoleGroupAction=-1; draw_calls=0;
    const int result=pop_ask_number(custom_caption,label,callback_result);
    assert(result==37 && popup_b.start_calls==1 && popup_a.start_calls==0);
    assert(popup_b.start_args.caption==custom_caption);
    assert(popup_b.start_args.label==label && popup_b.start_args.title==9);
    assert(std::strcmp(popup_b.start_args.text,"0")==0);
    assert(popup_b.start_args.flags==0x44 && popup_b.start_args.window==0);
    assert(popup_b.write_calls==1 && popup_b.write_value==PopupWriteCheckState);
    assert(popup_b.exec_calls==1 && popup_b.exec_callback==callback_result);
    assert(popup_b.close_calls==1 && popup_b.event_count==4);
    assert(popup_b.events[0]==1 && popup_b.events[1]==2 &&
           popup_b.events[2]==3 && popup_b.events[3]==5);
    assert(draw_calls==1 && ConsoleGroupAction==0x77 && ParseNumTable[0]==42);

    popup_a.visible=0; popup_a.start_result=1; popup_a.event_count=0;
    popup_a.write_calls=popup_a.exec_calls=popup_a.close_calls=0;
    std::strcpy(ParseStrBuffer[0].str,"8"); ParseNumTable[0]=99;
    assert(pop_ask_number(failure_caption,label,callback_result)==-1);
    assert(popup_a.start_calls==1 && popup_a.start_args.caption==failure_caption);
    assert(popup_a.start_args.label==label && std::strcmp(popup_a.start_args.text,"0")==0);
    assert(popup_a.event_count==1 && popup_a.events[0]==1);
    assert(popup_a.write_calls==0 && popup_a.exec_calls==0 && popup_a.close_calls==0);
    assert(ParseNumTable[0]==99 && draw_calls==1);

    popup_a.visible=0; popup_a.start_result=0; popup_a.external_window_=1;
    popup_a.field_3100_=0x55; popup_a.event_count=0;
    popup_a.write_calls=popup_a.exec_calls=popup_a.close_calls=0;
    std::strcpy(ParseStrBuffer[0].str,"-7");
    assert(pop_ask_number(external_caption,label,callback_result)==37);
    assert(draw_calls==1 && ConsoleGroupAction==0x55 && ParseNumTable[0]==-7);
    assert(popup_a.close_calls==1 && popup_a.events[0]==1 && popup_a.events[1]==2 &&
           popup_a.events[2]==3 && popup_a.events[3]==5);
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_numeric_prompt_actual_source_and_damage(tmp_path: Path, compiler_name: str) -> None:
    compiler=shutil.which(compiler_name)
    if compiler is None: pytest.skip(f"{compiler_name} unavailable")
    def run(source: str, suffix: str) -> subprocess.CompletedProcess[str]:
        cpp=tmp_path / f"numeric_{compiler_name}_{suffix}.cpp"
        exe=tmp_path / f"numeric_{compiler_name}_{suffix}"
        cpp.write_text(source,encoding="utf-8")
        built=subprocess.run([compiler,"-std=c++98","-Wall","-Wextra","-Werror",
            "-fsanitize=undefined","-fno-sanitize-recover=all",str(cpp),"-o",str(exe)],
            capture_output=True,text=True)
        if built.returncode: return built
        return subprocess.run([str(exe)],capture_output=True,text=True)
    good=run(make_numeric_source(numeric_source_root()),"good")
    assert good.returncode==0,good.stdout+good.stderr
    damaged=run(make_numeric_source(numeric_source_root(),"nonzero-seed"),"damaged")
    assert damaged.returncode!=0 and "Assertion" in damaged.stderr, (
        "the zero-seed native workflow control missed the mutation")
