"""Actual complete load/link methods with allocated host data and native pins.

Current declarations, StringList insertion hook and Spot implementations are
extracted on each run. File/parser/list-append/raster services are explicit
controlled boundaries, not native-provider or game acceptance. The parser
template observes the source array extent; it is not a native ABI substitute.
Expired pending pointers are inspected only as object-representation bytes,
never dereferenced. Negative counts execute open links WITHOUT '='; original
negative owned-array access, malformed lengths and native overflow layout stay
pin-only/unknown. The 256-byte scratch window does not establish historical
array spelling, padding or exact original allocation extent.
Owned-slot and copy probes instrument C++ evaluation boundaries on valid host
allocations; they are deliberately NOT claims of actual native callbacks.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.pe_coff import PeImage, Span
from match_recovery.source import read_file
import pytest

from recovery.tests.test_buffer_line_layout_fidelity_oct12 import body, one

ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path(os.environ.get("OPENSMACX_IMAGE",
    "/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe"))
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
LOAD = "int StringList::load(LPSTR file_name, LPSTR section_id, int clear_existing,"
LINK = "int Buffer::write_multi_font_raw_l(LPSTR text, int x_coord, int y_coord,"


def test_original_owner_identities_and_unchanged_contracts():
    task = json.loads((ROOT / "recovery/tasks/text-load-link-contracts-oct12.json").read_text())
    for path, address, spans in (
        ("src/stringlist.cpp", 0x608980, ((0x608980, 0x608B9B), (0x662EA0, 0x662EB5))),
        ("src/buffer.cpp", 0x5DCAE0, ((0x5DCAE0, 0x5DCE23),)),
    ):
        current, = (r for r in read_file(ROOT / path) if r.address == address)
        baseline = subprocess.check_output(
            ["git", "show", task["base_revision"] + ":" + path], cwd=ROOT).decode()
        # Original marker/comment envelopes are outside the changed bodies.
        source = (ROOT / path).read_text()
        signature = LOAD if address == 0x608980 else LINK
        marker = f"// ORIGINAL: 0x{address:08X} "
        assert source.count(marker) == baseline.count(marker) == 1
        envelope = source[source.index(marker):source.index(signature)]
        if address == 0x5DCAE0:
            envelope = envelope.replace('link_labels_', 'owned_')
        assert envelope == baseline[
            baseline.index(marker):baseline.index(signature)]
        assert current.image_spans == spans
        assert "BODY GOES HERE" not in body(source, signature)
        # Full transaction restoration keeps the fixed task baseline and
        # proves no other source body or annotation changed in either TU.
        restored = source.replace(body(source, signature), body(baseline, signature), 1)
        if address == 0x608980:
            # This historical owner is still a complete transaction against
            # its fixed task base.
            assert restored == baseline
        else:
            # Later recovered Buffer owners are outside this historical
            # transaction. Its restoration obligation is the complete local
            # writer body; current whole-file token scope is checked against
            # the separate immutable base for the typed-owner task.
            assert body(restored, signature) == body(baseline, signature)
    bh = (ROOT / "src/buffer.h").read_text()
    assert one(bh, r'^  (uint32_t link_count_;)') == 'uint32_t link_count_;'
    writer = body((ROOT / "src/buffer.cpp").read_text(), LINK)
    assert 'link_labels_[link_count_ - 1]' in writer  # no new unsafe-index guard
    assert writer.count('static_cast<int>(link_count_) < 20') == 3
    assert 'static_cast<int>(++link_count_) > 20' in writer
    assert writer.count('link_labels_[link_count_ - 1]') == 4  # store plus three live reads


def test_complete_pinned_bodies_and_storage_branch_evidence():
    raw = IMAGE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    cs = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {}
    for lo, hi in ((0x608980, 0x608B9B), (0x5DCAE0, 0x5DCE23),
                   (0x5FD880, 0x5FD8CF), (0x5FA960, 0x5FA9C0),
                   (0x5D7670, 0x5D79BF), (0x5DEF90, 0x5DEFD4),
                   (0x5D7210, 0x5D740A), (0x5D7470, 0x5D7665)):
        decoded = list(cs.disasm(image.read(Span(lo, hi)), lo))
        assert sum(i.size for i in decoded) == hi - lo
        assert decoded[0].address == lo and decoded[-1].mnemonic == 'ret'
        instructions.update({i.address: (i.mnemonic, i.op_str) for i in decoded})
    anchors = {
        0x608995: ('sub', 'esp, 0x268'),
        0x6089AC: ('lea', 'ecx, [esp + 0x118]'), # constructor arg still pushed
        0x608A88: ('lea', 'ecx, [esp + 0x114]'),
        0x608ABD: ('lea', 'ecx, [esp + 0x14]'),
        0x608AC7: ('call', '0x625880'),
        0x608AD8: ('cmp', 'dword ptr [esp + 0xc], ebx'),
        0x608AB7: ('mov', 'dword ptr [esp + 0xc], ebx'),
        0x608B1B: ('mov', 'dword ptr [esi + 0x1c], ecx'),
        0x608B27: ('call', '0x401100'),
        0x608B2C: ('test', 'eax, eax'),
        0x608B2E: ('jne', '0x608b5c'),
        0x608B30: ('jmp', '0x608a88'),
        0x608B35: ('lea', 'ecx, [esp + 0x114]'),
        0x608B3C: ('call', '0x5fd9d0'),
        0x608B53: ('call', '0x5fd970'),
        0x608B5C: ('lea', 'ecx, [esp + 0x114]'),
        0x608B63: ('call', '0x5fd9d0'),
        0x608B7A: ('call', '0x5fd970'),
        0x5FD8A0: ('mov', 'dword ptr [esi + 0x15c], ebx'),
        0x5DCC07: ('inc', 'ecx'), 0x5DCC0F: ('mov', 'dword ptr [esi + 0x4ac], ecx'),
        0x5DCC18: ('jle', '0x5dcc55'),
        0x5DCCAF: ('jge', '0x5dcd01'),
        0x5DCD2E: ('jge', '0x5dcd97'),
        0x5DCDD8: ('jge', '0x5dce18'),
        0x5DCCCA: ('mov', 'dword ptr [esi + ecx*4 + 0x4b8], eax'),
        0x5DCCC1: ('mov', 'ecx, dword ptr [esi + 0x4ac]'),
        0x5DCCD1: ('mov', 'edx, dword ptr [esi + 0x4ac]'),
        0x5DCCD7: ('mov', 'eax, dword ptr [esi + edx*4 + 0x4b8]'),
        0x5DCCDE: ('test', 'eax, eax'),
        0x5DCCE8: ('call', '0x645930'),
        0x5DCCED: ('mov', 'edx, dword ptr [esi + 0x4ac]'),
        0x5DCCF6: ('mov', 'eax, dword ptr [esi + edx*4 + 0x4b8]'),
        0x5DCCFD: ('mov', 'byte ptr [eax + ebx], 0'),
        0x5D76EA: ('mov', 'dword ptr [esi + 0x4ac], 0'),
        0x5DEFA1: ('mov', 'dword ptr [esi + 0x4ac], 0'),
        0x5FA960: ('mov', 'eax, dword ptr [ecx + 8]'),
        0x5FA966: ('cmp', 'eax, edx'),
        0x5FA968: ('jl', '0x5fa970'),
        0x5FA983: ('mov', 'dword ptr [ecx + 8], edx'),
        0x5FA9AB: ('mov', 'dword ptr [esi + edx + 0x14], edi'),
    }
    for address, expected in anchors.items():
        assert instructions[address] == expected, hex(address)
    handler = list(cs.disasm(image.read(Span(0x662EA0, 0x662EB5)), 0x662EA0))
    assert sum(i.size for i in handler) == 0x15
    assert [(i.address, i.mnemonic, i.op_str) for i in handler] == [
        (0x662EA0, 'lea', 'ecx, [ebp - 0x16c]'),
        (0x662EA6, 'jmp', '0x608c00'),
        (0x662EAB, 'mov', 'eax, 0x67fb80'),
        (0x662EB0, 'jmp', '0x644fd6'),
    ]
    assert struct.unpack('<8I', image.read(Span(0x67FB80, 0x67FBA0))) == (
        0x19930520, 1, 0x67FBA0, 0, 0, 0, 0, 0)
    assert struct.unpack('<2I', image.read(Span(0x67FBA0, 0x67FBA8))) == (
        0xFFFFFFFF, 0x662EA0)
    destructor = list(cs.disasm(image.read(Span(0x608C00, 0x608C05)), 0x608C00))
    assert [(i.address, i.size, i.mnemonic, i.op_str) for i in destructor] == [
        (0x608C00, 5, 'jmp', '0x5fd970')]
    # Full linear bodies have no post-insertion +1C null store. The only
    # receiver+1C write is publication of the borrowed scratch pointer.
    writes = [i for i in cs.disasm(image.read(Span(0x608980, 0x608B9B)), 0x608980)
              if i.mnemonic == 'mov' and i.op_str.startswith('dword ptr [esi + 0x1c],')]
    assert [i.address for i in writes] == [0x608B1B]
    assert 0x114 - 0x14 == 256  # accessed frame window, NOT exact allocation proof
    # With '=' and count bits FFFFFFFD, the original one-based store aliases
    # the count at Buffer+4AC; the next indexed read then uses overwritten bits.
    # This is instruction/address evidence, NOT an executed safe alias case.
    assert 0x4B8 + (-3 * 4) == 0x4AC
    for lo, hi in ((0x5D7210, 0x5D740A), (0x5D7470, 0x5D7665)):
        assert not any('[esi + 0x4ac]' in i.op_str for i in cs.disasm(
            image.read(Span(lo, hi)), lo))  # don't initialize stale count


PRELUDE = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <new>
#include <string>
#include <vector>
#include <functional>
#define __cdecl
#define MEASURED
#define OPENSMACX_NOEXCEPT_FALSE noexcept(false)
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef void *LPVOID;
typedef void *HFONT;
typedef int BOOL;
struct RECT {int left,top,right,bottom;};
std::vector<std::string> events, input_lines;
std::vector<size_t> parser_extents;
std::vector<std::string> inserted_text;
std::vector<unsigned> inserted_kind;
std::vector<uintptr_t> pending_publications;
std::vector<StringStructEntry *> unused_declaration; // replaced below
size_t input_position;
int open_result, add_result, parse_result, callbacks;
bool parsed_hash;
bool throw_on_get;
std::function<void()> insertion_probe;
std::vector<StringStructEntry *> host_entries; // replaced below
std::vector<StringStructPayload *> host_payloads; // replaced below
std::vector<void *> label_allocations;
bool fail_next_alloc;
std::function<void(void*,size_t)> allocation_probe;
static void *mem_get(size_t size) {
  if(fail_next_alloc){fail_next_alloc=false;return nullptr;}
  void *p=malloc(size);assert(p);
  if(allocation_probe)allocation_probe(p,size);return p;
}
class Heap {public:void *get(size_t size) {return mem_get(size);}};
#define static_assert(...)
@STRINGS@
#undef static_assert
Heap *StringAllocationHeap=0;
StringAllocationBase::~StringAllocationBase() {}
StringStruct::~StringStruct() {}
StringList::~StringList() {}
int StringStruct::on_insert(void *) {return 0;}
void StringStruct::on_remove(void *) {}
void StringList::on_remove(void *) {}
@STRING_CTOR@
@INSERT@
// The append/clear services are probes. They publish real host allocations
// and invoke the ACTUAL StringList virtual insertion hook; not add/teardown
// provider acceptance or an invented replacement product body.
int StringStruct::add(int id) {
  events.push_back("add:"+std::to_string(id));
  StringList *list=static_cast<StringList *>(this);
  uintptr_t bits;std::memcpy(&bits,&list->pending_text_,sizeof(bits));
  pending_publications.push_back(bits);
  if(insertion_probe)insertion_probe();
  if(add_result)return add_result;
  auto *entry=::new(mem_get(sizeof(StringStructEntry))) StringStructEntry;
  auto *payload=::new(mem_get(sizeof(StringStructPayload))) StringStructPayload;
  host_entries.push_back(entry);host_payloads.push_back(payload);
  entry->id=id;entry->payload=payload;
  assert(on_insert(payload)==0);
  inserted_text.push_back(payload->text);inserted_kind.push_back(payload->field_8_);
  assert(payload->field_C_==0);
  if(!head_) {head_=entry;entry->next=entry->previous=entry;}
  else {entry->previous=head_->previous;entry->next=head_;head_->previous->next=entry;head_->previous=entry;}
  current_=entry;current_position_=entry_count_++;return 0;
}
void StringStruct::remove_all() {
  events.push_back("clear");head_=nullptr;current_position_=entry_count_=0;
}
struct Text {
  @TEXT_STORAGE@
  @TEXT_CTOR@
  @TEXT_DTOR@
  @TEXT_APIS@
};
BOOL Text::open(LPCSTR file,LPCSTR section) {
  events.push_back(std::string("open:")+(file?file:"null")+":"+(section?section:"null"));
  return open_result;
}
LPSTR Text::get() {
  events.push_back("get");if(throw_on_get)throw 41;
  return input_position<input_lines.size()?input_lines[input_position++].data():nullptr;
}
void Text::close() {events.push_back("close");text_file_=nullptr;}
void Text::shutdown() {
  events.push_back("shutdown");free(buffer_get_);free(buffer_item_);
  buffer_get_=buffer_item_=nullptr;text_file_=nullptr;current_pos_=nullptr;
}
template<size_t N> int parse_string(LPSTR input,char (&output)[N]) {
  events.push_back(std::string("parse:")+input);parser_extents.push_back(N);
  assert(N==256 && std::strlen(input)<N); // only bounded fixture inputs
  std::memcpy(output,input,std::strlen(input)+1);
  if(parsed_hash)output[0]='#';return parse_result;
}
@LOAD@
#define private public
@SPOT@
#undef private
@SPOT_METHODS@
struct Font {@FONT_FIELDS@};
struct Buffer {
  @BUFFER_FIELDS@
  @LINK_API@
  @CLEAR_API@
  int write_raw_l(LPSTR,int,int,int);
};
struct Raster {std::string text;int x,y,length;};
std::vector<Raster> rasters;
std::function<void(Buffer*,int)> raster_probe;
std::function<void()> warning_probe;
std::function<void()> spot_probe;
int warnings;
int MessageBoxA(void *,const char *,const char *,unsigned) {
  ++warnings;if(warning_probe)warning_probe();return 0;
}
int Buffer::write_raw_l(LPSTR text,int x,int y,int length) {
  rasters.push_back({std::string(text,size_t(length)),x,y,length});
  if(raster_probe)raster_probe(this,int(rasters.size()));return x+length;
}
@CLEAR@
int owned_lookups;
std::function<void(Buffer&,int,uint32_t)> owned_probe;
std::function<void()> copy_probe;
// Explicit host evaluation probes: no native callback/provider equivalence.
static char *&observe_link_label_slot(Buffer &b,uint32_t index) {
  assert(index<20);++owned_lookups;
  if(owned_probe)owned_probe(b,owned_lookups,index);return b.link_labels_[index];
}
static void observed_copy(void *destination,const void *source,size_t size) {
  std::memcpy(destination,source,size);if(copy_probe)copy_probe();
}
@LINK@
'''


CONTROL = r'''
static void reset_loader(std::vector<std::string> lines={}) {
  events.clear();input_lines=lines;input_position=0;parser_extents.clear();
  inserted_text.clear();inserted_kind.clear();pending_publications.clear();
  open_result=add_result=parse_result=callbacks=0;throw_on_get=parsed_hash=false;insertion_probe={};
}
static void release_host_nodes() {
  for(auto *p:host_payloads){free(p->text);p->~StringStructPayload();free(p);}
  for(auto *p:host_entries){p->~StringStructEntry();free(p);}
  host_payloads.clear();host_entries.clear();
}
static uintptr_t pointer_bits(const StringList &list) {
  uintptr_t value;std::memcpy(&value,&list.pending_text_,sizeof(value));return value;
}
static void callback(LPSTR) {++callbacks;}
static int run_load(StringList &list,int clear=0,const char *section="SECTION") {
  return list.load(const_cast<char *>("file"),const_cast<char *>(section),clear,callback);
}
static void loader_controls() {
  for(int clear:{0,1}) {
    reset_loader({"plain","^","^marked","^^center","^^^third","#END"});
    StringList list;list.pending_text_=nullptr;list.field_20_=list.field_24_=99;
    assert(run_load(list,clear)==0);
    assert(inserted_text==std::vector<std::string>({"plain","","marked","center","third"}));
    assert(inserted_kind==std::vector<unsigned>({0,3,1,2,2}));
    assert(list.field_20_==2 && list.field_24_==0 && list.entry_count_==5 && callbacks==0);
    assert(pointer_bits(list)==pending_publications.back()); // no expired dereference
    assert(events.front()=="ctor:512" && events[1]=="open:file:SECTION");
    assert(events[events.size()-2]=="close" && events.back()=="shutdown");
    assert(parser_extents==std::vector<size_t>(5,256));release_host_nodes();
  }
  for(int failure:{0,4}) {
    reset_loader(failure?std::vector<std::string>{"^value","#END"}:
                         std::vector<std::string>{"^value"});
    add_result=failure;StringList list;
    list.pending_text_=nullptr;assert(run_load(list)==1); // EOF or add failure
    assert(pointer_bits(list)==pending_publications.back());
    assert(list.field_20_==1 && list.field_24_==0 && callbacks==0);
    assert(events[events.size()-2]=="close" && events.back()=="shutdown");
    release_host_nodes();
  }
  // Failures before pointer publication leave prior pending storage intact.
  char seed[]="borrowed";
  for(int clear:{0,1}) for(int reason:{0,1,2}) {
    reset_loader({"#EMPTY"});StringList list;StringStructEntry existing;
    list.head_=&existing;list.entry_count_=1;list.pending_text_=seed;
    open_result=reason==1?6:0;
    const int result=run_load(list,clear,reason==0?nullptr:"SECTION");
    assert(result==(reason==0?3:1));assert(list.pending_text_==seed);
    assert(pending_publications.empty() && callbacks==0);
    assert(events.front()=="ctor:512");
    if(clear)assert(events[1]=="clear" && list.head_==nullptr);
    else assert(list.head_==&existing);
    assert(events.back()=="shutdown");
  }
  // A blank permanently suppresses all later additions, not only that line.
  reset_loader({"first","","ignored","^also ignored","#END"});StringList list;
  assert(run_load(list)==0 && inserted_text==std::vector<std::string>({"first"}));
  assert(parser_extents.size()==3 && pointer_bits(list)==pending_publications[0]);release_host_nodes();
  // Leading raw hashes are skipped; a later raw hash terminates after any
  // non-hash, whereas a parsed hash is skipped without insertion.
  reset_loader({"#HEADER","#OTHER","text","#END"});StringList hashes;
  assert(run_load(hashes)==0 && inserted_text==std::vector<std::string>({"text"}));
  assert(parser_extents.size()==3);release_host_nodes();
  reset_loader({"translated","#END"});parsed_hash=true;StringList parsed;
  parsed.pending_text_=seed;
  assert(run_load(parsed)==0 && inserted_text.empty() && parsed.pending_text_==seed);
  assert(parser_extents.size()==1);
  reset_loader({std::string(255,'a'),"#END"});StringList edge;
  assert(run_load(edge)==0 && inserted_text[0].size()==255 && parser_extents[0]==256);
  release_host_nodes();
  reset_loader({"value","#END"});parse_result=14;StringList ignored_error;
  assert(run_load(ignored_error)==0 && inserted_text==std::vector<std::string>({"value"}));
  release_host_nodes();
  reset_loader({"text"});throw_on_get=true;StringList throwing;throwing.pending_text_=seed;
  try {run_load(throwing);assert(false);}catch(int value){assert(value==41);}
  assert(events.back()=="shutdown" && throwing.pending_text_==seed);
  assert(pending_publications.empty());
}
static Font primary,link_font,replacement_font;
static void reset_links() {
  rasters.clear();warnings=0;raster_probe={};warning_probe={};spot_probe={};
  allocation_probe={};owned_probe={};copy_probe={};owned_lookups=0;
  primary.unk_1_=0;primary.height_=8;primary.line_height_=11;
  link_font.unk_1_=0;link_font.height_=12;link_font.line_height_=15;
  replacement_font.unk_1_=-1;replacement_font.height_=20;replacement_font.line_height_=24;
}
static void setup(Buffer &b) {
  b.markup_enabled_=1;b.font1_=&primary;b.font4_=&link_font;b.spot_.init(40);
}
static int draw(Buffer &b,const char *text,int x=5,int y=7) {
  return b.write_multi_font_raw_l(const_cast<char *>(text),x,y,int(std::strlen(text)));
}
static void check_row(Buffer &b,int index,int position,int x,int y,int width,int height) {
  const auto &row=b.spot_.spots_[index];
  assert(row.position==position && row.type==0);
  assert(row.rect.left==x && row.rect.top==y && row.rect.right==x+width && row.rect.bottom==y+height);
}
static void release_labels(Buffer &b) {
  for(char *&p:b.link_labels_){free(p);p=nullptr;}
}
static void link_controls() {
  for(uint32_t before:{0U,18U,19U,20U,0x7ffffffeU,0x7fffffffU,0x80000000U,0xfffffffeU,0xffffffffU}) {
    reset_links();Buffer b{};setup(b);b.link_count_=before;
    uint32_t after=before+1U;int signed_after=static_cast<int32_t>(after);
    assert(draw(b,"$LINK<abc")==8 && b.link_count_==after);
    assert(warnings==(signed_after>20?1:0));
    assert(b.spot_.add_count_==(signed_after<20?1U:0U));
    if(signed_after<20)check_row(b,0,static_cast<int32_t>(after-1U),5,7,3,12);
    for(char *p:b.link_labels_)assert(p==nullptr);
    assert(b.write_font_slot_==3 && b.markup_pending_==1);
  }
  // Label allocation controls only use positive valid array indexes. Native
  // intentionally excludes the twentieth label despite a twenty-slot array.
  for(uint32_t before:{0U,18U,19U,20U}) {
    reset_links();Buffer b{};setup(b);b.link_count_=before;
    assert(draw(b,"$LINK<ab=label>tail")==11);
    assert(warnings==(before>=20?1:0));
    if(before<19) {
      assert(std::strcmp(b.link_labels_[before],"label")==0);
      assert(b.spot_.add_count_==1);check_row(b,0,int(before),5,7,2,12);
    } else {
      for(char *p:b.link_labels_)assert(p==nullptr);
      assert(b.spot_.add_count_==0);
    }
    assert(b.write_font_slot_==0 && b.markup_pending_==0);release_labels(b);
  }
  // A pre-existing open run exercises the nonterminal signed guard without
  // carrying a pending header across another token or allocating a label.
  reset_links();Buffer negative{};setup(negative);negative.link_count_=0xfffffffeU;
  negative.write_font_slot_=3;
  assert(draw(negative,"ab}tail")==11 && warnings==0);
  assert(negative.spot_.add_count_==1);check_row(negative,0,-3,5,7,2,12);
  // A real public service may be called by the raster callback. It resets
  // count/Spot after warning selection; later region readers must be live.
  reset_links();Buffer live{};setup(live);live.link_count_=18;
  raster_probe=[](Buffer *b,int call){if(call==1)b->clear_links();};
  assert(draw(live,"$LINK<abc")==8 && live.link_count_==0);
  assert(live.spot_.add_count_==1);check_row(live,0,-1,5,7,3,12);
  reset_links();Buffer fonts{};setup(fonts);
  raster_probe=[](Buffer *b,int call){if(call==2)b->font4_=&replacement_font;};
  assert(draw(fonts,"$LINK<abc")==8);check_row(fonts,0,0,5,7,3,24);
  reset_links();Buffer warned{};setup(warned);warned.link_count_=20;
  warning_probe=[&](){warned.clear_links();};
  assert(draw(warned,"$LINK<abc")==8 && warnings==1 && warned.link_count_==0);
  assert(warned.spot_.add_count_==1);check_row(warned,0,-1,5,7,3,12);
  reset_links();Buffer capacity{};setup(capacity);capacity.spot_.add_count_=40;
  assert(draw(capacity,"$LINK<abc")==8 && capacity.spot_.add_count_==40);
  reset_links();Buffer ordinary{};setup(ordinary);ordinary.markup_enabled_=0;
  ordinary.link_count_=0xffffffffU;
  assert(draw(ordinary,"plain")==10 && warnings==0 && ordinary.spot_.add_count_==0);
  assert(ordinary.link_count_==0xffffffffU && rasters.size()==1);
  reset_links();Buffer fallback{};setup(fallback);fallback.font4_=nullptr;
  assert(draw(fallback,"$LINK<abc")==8);check_row(fallback,0,0,5,7,3,8);
  reset_links();Buffer absent{};setup(absent);absent.font4_=absent.font1_=nullptr;
  assert(draw(absent,"$LINK<abc")==8 && absent.spot_.add_count_==0);
  reset_links();Buffer allocation_failure{};setup(allocation_failure);
  fail_next_alloc=true;
  assert(draw(allocation_failure,"$LINK<ab=label>tail")==11);
  assert(!fail_next_alloc && allocation_failure.link_labels_[0]==nullptr);
  assert(allocation_failure.link_count_==1 && allocation_failure.spot_.add_count_==1);
  check_row(allocation_failure,0,0,5,7,2,12);
#if OWNED_BOUNDARY_PROBES
  // Allocation can alter live count before the STORE expression. Slot 1,
  // not the original slot 0, receives the returned ordinary valid allocation.
  reset_links();Buffer allocated{};setup(allocated);
  allocation_probe=[&](void *,size_t){allocated.link_count_=2;};
  assert(draw(allocated,"$LINK<ab=label>tail")==11);
  assert(!allocated.link_labels_[0] && allocated.link_labels_[1]);
  assert(std::strcmp(allocated.link_labels_[1],"label")==0);
  assert(owned_lookups==4);release_labels(allocated);
  // Getter instrumentation changes a valid owned pointer while evaluating
  // the nullcheck; the copy must evaluate the indexed pointer again.
  reset_links();Buffer pointers{};setup(pointers);
  void *original_allocation=nullptr;
  char replacement[6];std::memset(replacement,'Q',sizeof(replacement));
  allocation_probe=[&](void *p,size_t n){original_allocation=p;std::memset(p,'A',n);};
  owned_probe=[&](Buffer &b,int reads,uint32_t index){
    if(reads==2)b.link_labels_[index]=replacement;
  };
  assert(draw(pointers,"$LINK<ab=label>tail")==11);
  assert(std::strcmp(replacement,"label")==0);
  assert(std::memcmp(original_allocation,"AAAAAA",6)==0);
  pointers.link_labels_[0]=nullptr;free(original_allocation);
  // Copy-boundary instrumentation changes count after memcpy. The NUL store
  // must reload both count and pointer rather than terminate the copied slot.
  reset_links();Buffer termination{};setup(termination);
  char terminal[6];std::memset(terminal,'Q',sizeof(terminal));
  allocation_probe=[&](void *p,size_t n){std::memset(p,'A',n);};
  copy_probe=[&](){termination.link_count_=2;termination.link_labels_[1]=terminal;};
  assert(draw(termination,"$LINK<ab=label>tail")==11);
  assert(std::memcmp(termination.link_labels_[0],"labelA",6)==0);
  assert(std::memcmp(terminal,"QQQQQ",5)==0 && terminal[5]==0);
  termination.link_labels_[1]=nullptr;release_labels(termination);
#endif
}
int main(){loader_controls();link_controls();}
'''


def actual_source(instrument=True):
    bh = (ROOT / "src/buffer.h").read_text()
    fh = (ROOT / "src/jackal/font.h").read_text()
    th = (ROOT / "src/text.h").read_text()
    sh = (ROOT / "src/stringstruct.h").read_text().split('#include "name_table.h"')[0]
    sh = re.sub(r'^#include[^\n]*\n|^#pragma once\n', '', sh, flags=re.M)
    sc = (ROOT / "src/stringstruct.cpp").read_text()
    sl = (ROOT / "src/stringlist.cpp").read_text()
    bc = (ROOT / "src/buffer.cpp").read_text()
    sp = (ROOT / "src/spot.cpp").read_text()
    ctor = body(th, "MEASURED Text(int size)").replace('{',
        '{\n    events.push_back("ctor:"+std::to_string(size));', 1)
    parts = {
        "STRINGS": sh,
        "STRING_CTOR": body(sc, "StringStruct::StringStruct()"),
        "INSERT": body(sl, "int StringList::on_insert(void *raw_payload)"),
        "TEXT_STORAGE": '\n'.join(one(th, r'^  (' + pattern + r';)') for pattern in (
            r'char file_name_\[80\]', r'char file_path_\[256\]', r'LPSTR current_pos_',
            r'FILE \*text_file_', r'LPSTR buffer_get_', r'LPSTR buffer_item_')),
        "TEXT_CTOR": ctor,
        "TEXT_DTOR": one(th, r'^  (MEASURED ~Text\(\) OPENSMACX_NOEXCEPT_FALSE[^\n]+)'),
        "TEXT_APIS": '\n'.join(one(th, p) for p in (
            r'^  (BOOL open\([^\n]+;)', r'^  (LPSTR get\(\);)',
            r'^  (void close\(\);)', r'^  (void shutdown\(\);)')),
        "LOAD": body(sl, LOAD),
        "SPOT": (ROOT / "src/spot.h").read_text().replace('#pragma once',''),
        "SPOT_METHODS": '\n'.join(body(sp, s) for s in (
            "Spot::Spot()", "Spot::~Spot()", "void Spot::init(int count)",
            "int Spot::add(int position, int type, int left, int top, int length, int width)")),
        "FONT_FIELDS": '\n'.join(one(fh, r'^  (' + p + r';)') for p in (
            r'int unk_1_', r'int height_', r'int line_height_')),
        "BUFFER_FIELDS": '\n'.join(one(bh, r'^  (' + p + r';)') for p in (
            r'uint32_t link_count_', r'uint32_t write_font_slot_', r'uint32_t markup_pending_',
            r'uint32_t markup_enabled_', r'Font \*font1_', r'Font \*font2_',
            r'Font \*font3_', r'Font \*font4_', r'char \*link_labels_\[20\]', r'Spot spot_'))
            + '\n',
        "LINK_API": one(bh, r'^  (int write_multi_font_raw_l\([^\n]+;)'),
        "CLEAR_API": one(bh, r'^  (void clear_links\(\);)'),
        "CLEAR": body(bc, "void Buffer::clear_links()"),
        "LINK": body(bc, LINK),
    }
    if instrument:
        parts['LINK'] = parts['LINK'].replace('link_labels_[link_count_ - 1]',
            'observe_link_label_slot(*this,link_count_ - 1)').replace('memcpy(', 'observed_copy(')
    source = PRELUDE
    source = '#define OWNED_BOUNDARY_PROBES ' + str(int(instrument)) + '\n' + source
    source = '#include <cstddef>\n' + one(bc, r'^(static const size_t OwnedAllocationCount = 20;)') + '\n' + source
    # Forward declarations suffice for vectors declared before the full real
    # virtual-inheritance headers; their objects are allocated after definition.
    source = source.replace('std::vector<StringStructEntry *> unused_declaration; // replaced below',
        'struct StringStructEntry; class StringStructPayload;')
    for name, text in parts.items():
        source = source.replace('@' + name + '@', text)
    assert all('@' + key + '@' not in source for key in parts)
    return source + CONTROL


def execute(tmp_path, compiler_name, source, success):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    cpp = tmp_path / 'control.cpp'
    binary = tmp_path / 'control'
    cpp.write_text(source)
    built = subprocess.run([compiler, '-std=c++17', '-O0', str(cpp), '-o', str(binary)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    if success:
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0 and 'Assertion' in result.stderr, result.stderr


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
@pytest.mark.parametrize('instrument', [False, True], ids=['untouched-bodies', 'evaluation-probes'])
def test_complete_actual_load_and_link_methods(tmp_path, compiler, instrument):
    execute(tmp_path, compiler, actual_source(instrument), True)


MUTANTS = (
    ('pending-null', 'const int add_result = add(0);', 'const int add_result = add(0); pending_text_ = nullptr;'),
    ('oversized-scratch', 'char parse_buffer[256];', 'char parse_buffer[608];'),
    ('sticky-blank', 'has_more = false;', 'has_more = true;'),
    ('clear-policy', 'clear_existing != 0 && head_ != nullptr', 'head_ != nullptr'),
    ('caret-kind', 'kind = 3;', 'kind = 1;'),
    ('triple-caret', 'parse_buffer[2] == \'^\' ? &parse_buffer[3]', 'parse_buffer[2] == \'^\' ? &parse_buffer[2]'),
    ('add-failure', 'if (add_result != 0)', 'if (false)'),
    ('missing-close', 'text.close();', '(void)0;'),
    ('parse-result-propagation', 'parse_string(line, parse_buffer);', 'if(parse_string(line, parse_buffer)!=0)return 14;'),
    ('warning-unsigned', 'static_cast<int>(++link_count_) > 20', '++link_count_ > 20'),
    ('terminal-unsigned', 'static_cast<int>(link_count_) < 20', 'link_count_ < 20'),
    ('twentieth-label', 'static_cast<int>(link_count_) < 20', 'static_cast<int>(link_count_) <= 20'),
    ('lost-wrap', '++link_count_', '(link_count_ == 0xffffffffU ? link_count_ : ++link_count_)'),
    ('link-font-fallback', 'font4_ ? font4_ : font1_', 'font1_'),
    ('cached-label-nullcheck', 'if (observe_link_label_slot(*this,link_count_ - 1))', 'if (kept)'),
    ('cached-label-copy', 'observed_copy(observe_link_label_slot(*this,link_count_ - 1), text, label);',
        'observed_copy(kept, text, label);'),
    ('cached-label-terminator', "observe_link_label_slot(*this,link_count_ - 1)[label] = '\\0';",
        "kept[label] = '\\0';"),
    ('live-count', 'if (write_font_slot_ == 3 && static_cast<int>(link_count_) < 20)',
        'if (write_font_slot_ == 3 && static_cast<int>(saved_count) < 20)'),
)


@pytest.mark.parametrize('label,before,after', MUTANTS, ids=[m[0] for m in MUTANTS])
def test_executed_semantic_damage_is_rejected(tmp_path, label, before, after):
    source = actual_source()
    assert before in source
    source = source.replace(before, after)
    if label == 'live-count':
        source = source.replace('if (!markup_enabled_) {',
            'uint32_t saved_count=link_count_;\n    if (!markup_enabled_) {', 1)
    execute(tmp_path, 'g++', source, False)
