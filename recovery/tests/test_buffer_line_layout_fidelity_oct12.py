"""Complete actual-method line-layout controls; not native ABI/game acceptance.

Buffer/Font declarations used by the projection come from current headers.
StringList's complete declarations, virtual inheritance and cursor accessors
are current source, with host-allocated nodes. Unrelated destruction and text
rendering services are explicit probes, not recovered providers. Native unsafe
preceding-byte order is pinned separately; every executed pointer has readable
preceding storage. Negative token lengths/null-font/null-RECT faults are not
executed or silently declared safe. No native layout assertion is inferred from
64-bit host layout, and no test grants matching or original-allocation proof.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.pe_coff import PeImage, Span
from match_recovery.source import read_file
import pytest

ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path(os.environ.get("OPENSMACX_IMAGE",
    "/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe"))
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SPECS = (
    (0x5DC410, 0x5DC70A, "char *Buffer::find_line_break_l(char * a1, int * a2, int a3)"),
    (0x5DB9E0, 0x5DBAF5, "int Buffer::wrap_cent(Font * a1, char * a2, int a3, int a4, int a5)"),
    (0x5DAD70, 0x5DB03E, "int Buffer::write_strings_height(StringList *strings, int width, int flags)"),
)


def body(source, signature):
    start = source.index(signature)
    assert source.find(signature, start + len(signature)) < 0
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError(signature)


def one(source, pattern):
    result, = re.findall(pattern, source, re.M)
    return result


def test_only_three_complete_bodies_change_and_all_obligations_survive():
    task = json.loads((ROOT / "recovery/tasks/buffer-line-layout-fidelity-oct12.json").read_text())
    baseline = subprocess.check_output(
        ["git", "show", task["base_revision"] + ":src/buffer.cpp"], cwd=ROOT).decode()
    current = (ROOT / "src/buffer.cpp").read_text()
    restored = current
    for address, end, signature in SPECS:
        current_body = body(current, signature)
        assert "BODY GOES HERE" not in current_body
        restored = restored.replace(current_body, body(baseline, signature), 1)
        row, = (r for r in read_file(ROOT / "src/buffer.cpp") if r.address == address)
        assert row.image_spans == ((address, end),)
    assert restored == baseline
    assert current.count("if (found[-1] == ' ' || found == segStart)") == 2
    # Full byte-for-byte restoration also preserves every annotation and seed
    # envelope, rather than relying on a same-count claim check.
    assert "char *find_line_break_l(char *text, int *break_len, int len);" in (ROOT / "src/buffer.h").read_text()


def test_pinned_complete_branch_evidence_and_dependencies():
    raw = IMAGE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {}
    for lo, hi in [(a, b) for a, b, _ in SPECS] + [
            (0x628AF0, 0x628B23), (0x619370, 0x6195A7),
            (0x5DBB00, 0x5DBBFA), (0x5DB040, 0x5DB3F6)]:
        decoded = list(decoder.disasm(image.read(Span(lo, hi)), lo))
        assert decoded[0].address == lo
        assert sum(i.size for i in decoded) == hi - lo
        assert decoded[-1].mnemonic == "ret"
        instructions.update({i.address: (i.mnemonic, i.op_str) for i in decoded})
    anchors = {
        0x5DC624: ("mov", "dword ptr [edi + 0x51c], 0"),
        0x5DC642: ("mov", "dword ptr [edi + 0x51c], 0"),
        0x5DC682: ("mov", "dword ptr [edi + 0x51c], 0"),
        0x5DC6C7: ("mov", "dword ptr [edi + 0x51c], 0"),
        0x5DC62E: ("mov", "cl, byte ptr [eax - 1]"),
        0x5DC64C: ("mov", "cl, byte ptr [eax - 1]"),
        0x5DC68C: ("mov", "cl, byte ptr [eax - 1]"),
        0x5DC6D1: ("mov", "cl, byte ptr [eax - 1]"),
        0x5DC63A: ("cmp", "eax, dword ptr [ebp - 4]"),
        0x5DC658: ("cmp", "eax, dword ptr [ebp - 4]"),
        0x5DC694: ("cmp", "eax, ebx"), 0x5DC6D9: ("cmp", "eax, ebx"),
        0x5DC69F: ("call", "0x628af0"), 0x5DC6E4: ("call", "0x628af0"),
        0x5DC69B: ("push", "0x20"), 0x5DC69D: ("push", "eax"),
        0x5DC698: ("mov", "ecx, dword ptr [ebp + 8]"),
        0x5DBA3B: ("mov", "ecx, dword ptr [esp + 0x30]"),
        0x5DBA43: ("mov", "dword ptr [esp + 0x20], ecx"),
        0x5DBA61: ("call", "0x5dc410"),
        0x5DBA6C: ("mov", "ecx, dword ptr [esp + 0x30]"),
        0x5DBA77: ("push", "ecx"),
        0x5DBA8A: ("mov", "eax, dword ptr [esp + 0x34]"),
        0x5DAF59: ("mov", "ebp, dword ptr [eax + 8]"),
        0x5DAF78: ("call", "ecx"),
        0x5DAF7D: ("cmp", "dword ptr [ebx + 8], edi"),
        0x5DAF88: ("call", "0x402530"),
        0x5DAF8D: ("cmp", "dword ptr [eax + 8], edi"),
        0x5DAF9F: ("cmp", "ebp, 3"), 0x5DAFC8: ("cmp", "ebp, 2"),
        0x628B03: ("mov", "eax, ecx"), 0x628B05: ("sub", "ecx, ebx"),
        0x628B0E: ("cmp", "byte ptr [eax], bl"),
        0x628B12: ("dec", "eax"), 0x628B13: ("dec", "ecx"),
        0x5DBBD8: ("jne", "0x5dbb5f"),
        0x5DBB65: ("mov", "dword ptr [esp + 0x24], eax"),
    }
    for address, expected in anchors.items():
        assert instructions[address] == expected, hex(address)
    # Every fast return keeps EAX unchanged. No imaginary pointer rebasing.
    assert instructions[0x5DC634] == ("je", "0x5dc701")
    assert instructions[0x5DC652] == ("je", "0x5dc701")
    assert instructions[0x5DC692] == ("je", "0x5dc701")
    assert instructions[0x5DC6D7] == ("je", "0x5dc701")


PRELUDE = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <functional>
#define __cdecl
#define MEASURED
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef void *HFONT;
typedef int BOOL;
// The unchanged native-layout assertions are not host-layout assertions.
#define static_assert(...)
@STRINGS@
#undef static_assert
Heap *StringAllocationHeap = 0;
StringAllocationBase::~StringAllocationBase() {}
StringStruct::~StringStruct() {}
StringList::~StringList() {}
int StringStruct::on_insert(void *) { return 0; }
void StringStruct::on_remove(void *) {}
int StringList::on_insert(void *) { return 0; }
void StringList::on_remove(void *) {}
@CURSORS@
struct Font {
  @FONT_FIELDS@
  @FONT_APIS@
};
Font *FontDefault;
@CALLBACK@
struct Write { char *text; int x,y,width,length; };
std::vector<Write> writes;
std::vector<std::string> events;
std::vector<Font *> measured_fonts;
std::vector<int> measured_lengths, measured_widths;
std::function<char *(Font *,char *,int *,size_t)> font_probe;
int BufferStrHeight, BufferFlyingWidth, BufferFlyingY;
int reverse_calls;
struct Buffer {
  @BUFFER_FIELDS@
  @BUFFER_APIS@
};
@ADVANCE@
LPSTR Font::find_line_break_l(LPSTR text,int *width,size_t length) {
  measured_fonts.push_back(this); measured_lengths.push_back(int(length));
  measured_widths.push_back(*width);
  return font_probe ? font_probe(this,text,width,length) : nullptr;
}
int Buffer::write_cent_l(char *text,int x,int y,int width,int length) {
  writes.push_back({text,x,y,width,length}); return x;
}
int Buffer::text_line_height() { events.push_back("advance"); return 10; }
int Buffer::wrap_height_flying(char *text) {
  events.push_back(std::string("fly:")+(text?text:"null")); return BufferFlyingY;
}
int Buffer::wrap_height_flying(char *text,int width) {
  events.push_back(std::string("init:")+(text?text:"null")+":"+std::to_string(width));
  return 19;
}
int Buffer::wrap_height_flying_flush() { events.push_back("flush"); return 7; }
int Buffer::wrap_height(char *text,int width) {
  events.push_back(std::string("height:")+(text?text:"null")+":"+std::to_string(width));
  return 30;
}
@SEARCH@
@TOKEN@
@BODIES@
'''


CONTROL = r'''
static Font primary, secondary, third, fourth, fallback;
static void initialize(Font &f,int height=8,int spacing=0) {
  f.font_obj_=&f; f.unk_1_=spacing; f.height_=height; f.line_height_=height+3;
}
static Buffer fresh() {
  Buffer b{}; b.font1_=&primary; b.font2_=&secondary; b.font3_=&third;
  b.font4_=&fourth; b.markup_enabled_=1; return b;
}
static void reset() {
  writes.clear();events.clear();measured_fonts.clear();measured_lengths.clear();
  measured_widths.clear();font_probe={};reverse_calls=0;BufferStrHeight=0;
  BufferFlyingWidth=0;BufferFlyingY=0;FontDefault=&fallback;
}
static void parser_controls() {
  // Readable sentinel before every input. The native previous-byte read
  // is deliberately NOT reordered for an initial-segment result.
  for(bool terminal:{false,true}) for(bool missing:{false,true})
    for(int disposition=0;disposition!=5;++disposition) {
      reset();Buffer b=fresh();if(missing)b.font2_=nullptr;
      char allocation[]="!aa bb{c d}";char *text=allocation+1;
      if(terminal)text[9]=0;
      char *returned=nullptr;char *expected=nullptr;
      int calls=0;
      font_probe=[&](Font *,char *segment,int *width,size_t) -> char* {
        ++calls;*width-=3;if(calls==1)return nullptr;
        if(calls>2) {assert(segment==text+10);return nullptr;}
        assert(segment==text+6);
        if(disposition==0) returned=segment+2; // preceding space
        if(disposition==1) returned=segment;   // segment identity
        if(disposition==2) returned=segment+1; // inclusive search end is space
        if(disposition==3) returned=segment+3; // backward search to segment space
        if(disposition==4) returned=nullptr;
        return returned;
      };
      expected=disposition==0?text+8:disposition==1?text+6:
               disposition==2||disposition==3?text+7:nullptr;
      int width=40;char *actual=b.find_line_break_l(text,&width,terminal?9:10);
      assert(actual==expected && b.markup_pending_==0);
      assert(reverse_calls==(disposition==2||disposition==3?1:0));
      assert(measured_fonts[1]==(missing?&primary:&secondary));
      assert(measured_lengths[0]==5 && measured_lengths[1]==3);
      assert(width==(disposition==4 && !terminal?31:34));
    }
  for(bool terminal:{false,true}) for(bool missing:{false,true}) {
    reset();Buffer b=fresh();if(missing)b.font2_=nullptr;
    char allocation[]="!aa bb{cd}";char *text=allocation+1;
    if(terminal)text[8]=0;
    int calls=0;font_probe=[&](Font *,char *segment,int *,size_t)->char* {
      return ++calls==1?nullptr:segment+1;
    };
    int width=40;assert(b.find_line_break_l(text,&width,terminal?8:9)==text+2);
    assert(reverse_calls==1); // search crosses the markup/segment boundary
  }
  for(bool terminal:{false,true}) for(bool missing:{false,true}) {
    reset();Buffer b=fresh();if(missing)b.font2_=nullptr;
    char allocation[]="! aa{cd}";char *text=allocation+1;
    if(terminal)text[6]=0;
    int calls=0; font_probe=[&](Font *,char *segment,int *,size_t)->char* {
      return ++calls==1?nullptr:segment+1;
    };
    int width=40;assert(b.find_line_break_l(text,&width,terminal?6:7)==nullptr);
    assert(reverse_calls==1); // start's space is excluded from reverse search
  }
  reset();Buffer b=fresh();b.font1_=nullptr;b.markup_pending_=2;
  char allocation[]="!plain";char *text=allocation+1;
  assert(b.find_line_break_l(text,nullptr,5)==nullptr && b.font1_==nullptr);
  b.markup_enabled_=0;
  font_probe=[](Font *,char *text,int *,size_t){return text+2;};
  int width=40;assert(b.find_line_break_l(text,&width,5)==text+2);
  assert(b.font1_==&fallback && b.markup_pending_==2 && reverse_calls==0);
  b.markup_enabled_=1;b.markup_pending_=1;
  font_probe=[](Font *,char *text,int *,size_t){return text;};
  assert(b.find_line_break_l(text,&width,5)==text && b.markup_pending_==0);
  assert(reverse_calls==0);
  reset();b=fresh();char token[]="$LINK<abc";
  assert(b.find_line_break_l(token,&width,9)==nullptr);
  assert(measured_fonts==std::vector<Font *>({&primary,&fourth}));
  assert(measured_lengths==std::vector<int>({0,3}));
  assert(b.field_514_==3 && b.markup_pending_==0);
  reset();b=fresh();char labeled[]="$LINK<x=q>y";
  assert(b.find_line_break_l(labeled,&width,11)==nullptr);
  assert(measured_fonts==std::vector<Font *>({&primary,&fourth,&primary}));
  assert(measured_lengths==std::vector<int>({0,1,1}));
  assert(b.field_514_==0 && b.markup_pending_==0);
  reset();b=fresh();char selections[]="{x}[y]";
  assert(b.find_line_break_l(selections,&width,6)==nullptr);
  assert(measured_fonts==std::vector<Font *>({&primary,&secondary,&primary,&third,&primary}));
  assert(b.field_514_==0);
  for(std::string escape:{"{{ab","[[ab","}}ab","]]ab"}) {
    reset();b=fresh();assert(b.find_line_break_l(escape.data(),&width,4)==nullptr);
    assert(measured_lengths==std::vector<int>({1,2}));
  }
  const char boundary[]=" x y";
  assert(memrchr(nullptr,boundary+3,' ')==nullptr);
  assert(memrchr(boundary,nullptr,' ')==nullptr);
  assert(memrchr(boundary,boundary,' ')==nullptr);
  assert(memrchr(boundary,boundary+2,' ')==boundary+2);
  assert(memrchr(boundary,boundary+1,' ')==nullptr);
}
static void centered_controls() {
  reset();Buffer b=fresh();b.markup_enabled_=0;char text[]="aa bb";
  font_probe=[](Font *,char *current,int *width,size_t)->char* {
    *width=3;return current[0]=='a'?current+3:nullptr;
  };
  assert(b.wrap_cent(&fallback,text,12,20,40)==36);
  assert(writes.size()==2 && writes[0].width==40 && writes[1].width==40);
  assert(writes[0].length==3 && writes[1].length==2);
  assert(writes[0].y==20 && writes[1].y==28);
  assert(measured_widths==std::vector<int>({40,40}));
  assert(b.field_18_==2 && b.font1_==&primary && b.font2_==&secondary && b.font3_==&third);
  assert(b.font4_==&fourth);
  reset();b=fresh();b.markup_enabled_=0;
  font_probe=[&](Font *,char *,int *width,size_t)->char* {
    *width=1;b.font1_=nullptr;b.font2_=&third;b.font3_=&fourth;return nullptr;
  };
  fallback.unk_1_=-1;
  assert(b.wrap_cent(nullptr,text,3,5,60)==16); // live default font/line height
  assert(writes[0].width==60 && b.font1_==&primary && b.font2_==&secondary && b.font3_==&third);
  fallback.unk_1_=0;
  reset();b=fresh();Font invalid{};
  assert(b.wrap_cent(&invalid,nullptr,4,17,20)==17 && writes.empty());
  assert(b.font1_==&primary && b.font2_==&secondary && b.font3_==&third);
  font_probe=[](Font *f,char *,int *width,size_t)->char* {
    assert(f==&primary);*width=0;return nullptr;
  };
  b.markup_enabled_=0;assert(b.wrap_cent(&invalid,text,4,17,20)==25);
}
static StringList *active;
static StringStructEntry *replacement;
static int old_kind,new_kind;
static bool replace_node,clear_head;
static void list_callback(int id,int kind) {
  events.push_back("callback:"+std::to_string(id)+":"+std::to_string(kind));
  if(id==11) {
    assert(kind==old_kind);
    if(replace_node)active->current_=replacement;
    active->current_->string_payload()->field_8_=new_kind;
    if(clear_head)active->head_=nullptr;
  }
}
static void list_controls() {
  for(int saved:{0,1,2,3}) for(int live:{0,2,3})
    for(bool replaced:{false,true}) {
      reset();Buffer b=fresh();b.field_C_=list_callback;
      StringList list;StringStructEntry nodes[3];StringStructPayload payloads[3];
      char first[]="first",second[]="second",other[]="other";
      char *texts[]={first,second,other};
      for(int i=0;i!=3;++i) {
        nodes[i].id=10+i;nodes[i].payload=&payloads[i];
        nodes[i].next=&nodes[(i+1)%2];nodes[i].previous=&nodes[(i+1)%2];
        payloads[i].text=texts[i];payloads[i].field_8_=i==1?saved:0;
      }
      list.head_=nodes;list.current_=nodes+1;list.entry_count_=2;list.current_position_=1;
      active=&list;replacement=nodes+2;old_kind=saved;new_kind=live;
      replace_node=replaced;clear_head=false;
      int expected=7;
      std::string current=replaced?"other":"second";
      std::vector<std::string> trace={"callback:10:0","fly:first",
        "callback:11:"+std::to_string(saved)};
      if(live==0)trace.push_back("fly:"+current);
      else {
        trace.push_back("flush");expected+=7;
        if(saved==3) {trace.push_back("advance");expected+=10;trace.push_back("init:null:40");}
        else if(saved==2) {trace.push_back("height:"+current+":40");expected+=30;trace.push_back("init:null:40");}
        else trace.push_back("init:"+current+":40");
      }
      trace.push_back("flush");
      assert(b.write_strings_height(&list,40,0)==expected);
      assert(events==trace && list.current_position_==1);
      // Callback gets the original metadata even when the later gate sees a
      // replacement node; dispatch does not normalize to replacement metadata.
    }
  reset();Buffer b=fresh();assert(b.write_strings_height(nullptr,40,0)==0 && events.empty());
  StringList empty;assert(b.write_strings_height(&empty,40,0)==7);
  assert(events==std::vector<std::string>({"fly:null","flush"}));
  reset();StringList list;StringStructEntry nodes[2];StringStructPayload payloads[2];
  char first[]="first",second[]="second";
  for(int i=0;i!=2;++i) {nodes[i].id=10+i;nodes[i].payload=&payloads[i];nodes[i].next=&nodes[1-i];
    payloads[i].text=i?second:first;payloads[i].field_8_=i?2:0;}
  list.head_=nodes;list.current_=nodes+1;list.entry_count_=2;list.current_position_=1;
  active=&list;old_kind=2;new_kind=0;replace_node=false;clear_head=true;b.field_C_=list_callback;
  assert(b.write_strings_height(&list,40,0)==7);
  assert(events==std::vector<std::string>({"callback:10:0","fly:first","callback:11:2","fly:null","flush"}));
  reset();list.head_=nodes;list.current_=nodes+1;list.entry_count_=2;list.current_position_=1;
  b.field_C_=nullptr;payloads[1].field_8_=3;
  assert(b.write_strings_height(&list,40,2)==15); // preserve caller position, initial kind3
  assert(events==std::vector<std::string>({"fly:null","flush"}));
  reset();list.current_=nodes;list.current_position_=0;list.entry_count_=1;payloads[0].field_8_=2;
  b.markup_enabled_=0;b.field_10_=5;
  assert(b.write_strings_height(&list,40,0)==17);
  assert(events==std::vector<std::string>({"advance","fly:null","flush"}));
}
int main() {
  initialize(primary);initialize(secondary);initialize(third);initialize(fourth);initialize(fallback);
  parser_controls();centered_controls();list_controls();
}
'''


def actual_source():
    product = (ROOT / "src/buffer.cpp").read_text()
    bh = (ROOT / "src/buffer.h").read_text()
    fh = (ROOT / "src/jackal/font.h").read_text()
    sh = (ROOT / "src/stringstruct.h").read_text().split('#include "name_table.h"')[0]
    # Full current list declarations/cursor methods; dependency includes and
    # native 32-bit layout assertions alone are projected for the host.
    sh = re.sub(r'^#include[^\n]*\n', '', sh, flags=re.M)
    source = (ROOT / "src/stringstruct.cpp").read_text()
    search = body((ROOT / "src/general.cpp").read_text(),
        "const char *__cdecl memrchr(LPCSTR start, LPCSTR end, char value)")
    search = search.replace('{', '{\n    ++reverse_calls;', 1)
    parts = {
        "STRINGS": sh,
        "CURSORS": '\n'.join(body(source, s) for s in
            ("StringStruct::StringStruct()", "StringStructPayload *StringStruct::current_entry()")),
        "FONT_FIELDS": '\n'.join(one(fh, r'^  (' + typ + r' ' + name + r';)')
            for typ, name in (("int", "unk_1_"), ("BOOL", "is_fot_set_"),
            ("HFONT", "font_obj_"), ("int", "line_height_"), ("int", "height_"),
            ("int", "internal_leading_"), ("int", "ascent_"), ("int", "descent_"),
            ("int", "pad_"), ("LPSTR", "fot_file_name_"))),
        "FONT_APIS": '\n'.join(one(fh, p) for p in
            (r'^  (LPSTR find_line_break_l\([^\n]+;)', r'^  (int line_advance\(\) const;)')),
        "CALLBACK": one(bh, r'^(typedef void \(__cdecl \*BufferWriteStringsCallback\)\(int, int\);)'),
        "BUFFER_FIELDS": '\n'.join(one(bh, r'^  (' + typ + r'\s*' + name + r';)')
            for typ, name in (("BufferWriteStringsCallback", "field_C_"),
            ("uint32_t", "field_10_"), ("uint32_t", "field_14_"), ("uint32_t", "field_18_"),
            ("uint32_t", "field_514_"), ("uint32_t", "markup_pending_"),
            ("uint32_t", "markup_enabled_"), (r'Font \*', "font1_"),
            (r'Font \*', "font2_"), (r'Font \*', "font3_"), (r'Font \*', "font4_"))),
        "BUFFER_APIS": '\n'.join(one(bh, p) for p in (
            r'^  (char \*find_line_break_l\([^\n]+;)',
            r'^  (int wrap_cent\(Font \*font, char \*text, int x_coord, int y_coord, int width\);)',
            r'^  (int write_strings_height\([^\n]+;)',
            r'^  (int wrap_height_flying\(char \*text\);)',
            r'^  (int wrap_height_flying\(char \*text, int width\);)',
            r'^  (int wrap_height_flying_flush\(\);)',
            r'^  (int wrap_height\(char \*text, int width\);)',
            r'^  (int text_line_height\(\);)')) + '\n' + one(bh,
            r'^  (int write_cent_l\(LPSTR text, int x_coord, int y_coord, int width, int len\);)'),
        "ADVANCE": body((ROOT / "src/jackal/font.cpp").read_text(), "int Font::line_advance() const"),
        "SEARCH": search,
        "TOKEN": one(product, r'^(char BufferMarkupLinkToken\[\] = [^\n]+;)'),
        "BODIES": '\n'.join(body(product, signature) for _, _, signature in SPECS),
    }
    result = PRELUDE
    for key, value in parts.items():
        result = result.replace('@' + key + '@', value)
    assert all('@' + key + '@' not in result for key in parts)
    return result + CONTROL


def execute(tmp_path, compiler_name, source, success):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    path = tmp_path / "control.cpp"
    executable = tmp_path / "control"
    path.write_text(source)
    built = subprocess.run([compiler, '-std=c++17', '-O0', str(path), '-o', str(executable)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    if success:
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0 and 'Assertion' in result.stderr, result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_complete_actual_result_centering_and_list_methods(tmp_path, compiler):
    execute(tmp_path, compiler, actual_source(), True)


MUTANTS = (
    ("pointer-rebase", "return found;", "return found - segStart + a1;"),
    ("tail-pointer-rebase", "return found;", "return found - segStart + a1;"),
    ("missing-reverse-search", "return const_cast<char *>(memrchr(a1, found, ' '));", "return found;"),
    ("search-start", "memrchr(a1, found, ' ')", "memrchr(segStart, found, ' ')"),
    ("space-before", "found[-1] == ' '", "found[0] == ' '"),
    ("segment-identity", "found == segStart", "found == a1"),
    ("pending-clear", "markup_pending_ = 0;", "markup_pending_ = 2;"),
    ("null-outparam-order", "if (!a2) {\n        return 0;", "if (!a2) {\n        font1_ = FontDefault; return 0;"),
    ("search-exclusive-end", "const char *p = end;", "const char *p = end - 1;"),
    ("search-inclusive-start", "int count = (int)(end - start);", "int count = (int)(end - start) + 1;"),
    ("residual-center-width", "write_cent_l(str, a3, y, a5, lineLen);", "write_cent_l(str, a3, y, outParam, lineLen);"),
    ("font-restore", "font3_ = savedC;", "font3_ = font1_;"),
    ("cached-list-gate", "strings->head_ == 0 || strings->current_entry()->field_8_ == 0", "strings->head_ == 0 || line_payload == 0"),
    ("live-list-dispatch", "if (line_payload == 3)", "if (strings->current_entry()->field_8_ == 3)"),
    ("lost-position-flags", "(flags & 2) == 0", "true"),
)


@pytest.mark.parametrize("label,before,after", MUTANTS, ids=[m[0] for m in MUTANTS])
def test_executed_damage_is_rejected(tmp_path, label, before, after):
    source = actual_source()
    assert before in source
    if label.startswith("tail-"):
        prefix, suffix = source.rsplit(before, 1)
        source = prefix + after + suffix
    else:
        source = source.replace(before, after)
    execute(tmp_path, "g++", source, False)
