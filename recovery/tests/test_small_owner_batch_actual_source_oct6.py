"""Bounded host controls extracted from the current genuine source owners.

Host doubles do not qualify native ABI, virtual tables or unwind layout.
"""
from __future__ import annotations

import shutil
import subprocess
import re
from pathlib import Path

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_text

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
SOURCE_ROOT = ROOT


def region(relative: str, address: int) -> str:
    path = SOURCE_ROOT / relative
    rows = [r for r in read_text(path.read_text(encoding="utf-8"), path.resolve())
            if r.address == address]
    assert len(rows) == 1
    signatures = {
        0x00455E50: "void __cdecl load_deswin_sprites() {",
        0x005BECA0: "int __cdecl X_text_open(char *src_file_id, const char *section_id) {",
        0x00609150: "int Dialog::init() {",
        0x00627830: "int __cdecl pop_ask(char *a1, char *a2, int a3, int (__cdecl *a4)(), int a5) {",
        0x00405140: "int __cdecl POP2(const char *a1, const char *a2, int a3) {",
    }
    source = path.read_text(encoding="utf-8")
    marker = f"// ORIGINAL: 0x{address:08X}"
    marker_at = source.index(marker)
    start = source.index(signatures[address], marker_at)
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    assert masked[start:start + len(signatures[address])] == signatures[address], (
        f"definition is not C++ code: {relative}:0x{address:08X}"
    )
    assert rows[0].mode.value == "body" and rows[0].region and not rows[0].extract_error
    opening = masked.index("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated projected function 0x{address:08X}")


def compile_run(tmp_path: Path, source: str) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "host GCC or Clang is required for the source controls"
    cpp = tmp_path / "control.cpp"
    exe = tmp_path / "control"
    cpp.write_text(source, encoding="utf-8")
    subprocess.run([compiler, "-std=c++14", "-Wall", "-Wextra", "-Werror",
                    "-Wno-int-to-pointer-cast",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True, text=True)
    subprocess.run([str(exe)], check=True, capture_output=True, text=True)


def test_x_text_open_actual_projected_region_preserves_fallback_order(tmp_path: Path) -> None:
    body = region("src/text.cpp", 0x005BECA0)
    harness = r'''
#include <cassert>
#include <cstring>
#define __cdecl
using std::strcmp;
struct Console { int faction_id_; } ConsoleGlobal{2};
struct Player { unsigned rule_flags; } Players[8]{};
int ExpansionEnabled = 1;
char *ScriptPopupFile = const_cast<char *>("SCRIPT");
char *PopupStartAltFile1 = const_cast<char *>("ALIEN");
static int results[4], calls;
static const char *files[4], *sections[4];
int text_open(const char *file, const char *section) {
  files[calls] = file; sections[calls] = section;
  return results[calls++];
}
enum { RFLAG_ALIEN = 0x80 };
''' + body + r'''
int main() {
  const char *section = "ATTACKSTRING";
  char normal[] = "OTHER";
  results[0] = 7;
  assert(X_text_open(normal, section) == 7);
  assert(calls == 1 && files[0] == normal && sections[0] == section);

  calls = 0; Players[2].rule_flags = 0;
  char script[] = "SCRIPT"; results[0] = 9;
  assert(X_text_open(script, section) == 9);
  assert(calls == 1 && files[0] == script);

  calls = 0; Players[2].rule_flags = RFLAG_ALIEN;
  results[0] = 0;
  assert(X_text_open(script, section) == 0);
  assert(calls == 1 && std::strcmp(files[0], "ALIEN") == 0);

  calls = 0; results[0] = 3; results[1] = 8;
  assert(X_text_open(script, section) == 8);
  assert(calls == 2 && std::strcmp(files[0], "ALIEN") == 0
         && files[1] == script);
}
'''
    compile_run(tmp_path, harness)


def test_dialog_init_actual_projected_region_keeps_three_cleanup_sites(tmp_path: Path) -> None:
    body = region("src/dialog.cpp", 0x00609150)
    harness = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#define __cdecl
struct StringStructPayload { int value; };
struct StringStructEntry {
  StringStructEntry *next; StringStructPayload *payload;
  StringStructPayload *string_payload() { return payload; }
};
struct StringList {
  StringStructEntry *head_ = 0, *current_ = 0;
  int current_position_ = 0, entry_count_ = 0;
  void *allocator_ = 0; int removed = 0;
  void on_remove(StringStructPayload *) { ++removed; }
};
struct Heap {
  int result = 0, calls = 0;
  int init(std::size_t) { ++calls; return result; }
};
struct Dialog {
  Heap heap_; Heap *heap_ptr_ = 0; uint32_t field_1C_ = 0;
  StringList item_list_;
  int init();
};
''' + body + r'''
static StringStructEntry *one_entry(Dialog &d) {
  StringStructEntry *e = new StringStructEntry;
  e->next = 0; e->payload = new StringStructPayload{3};
  d.item_list_.head_ = e; d.item_list_.entry_count_ = 1;
  return e;
}
int main() {
  Dialog negative; negative.field_1C_ = 0xffffffffu; one_entry(negative);
  assert(negative.init() == 1 && negative.item_list_.removed == 0
         && negative.heap_.calls == 0 && negative.item_list_.head_ != 0);
  delete negative.item_list_.head_->payload; delete negative.item_list_.head_;

  Dialog zero; zero.field_1C_ = 0; one_entry(zero);
  assert(zero.init() == 0 && zero.item_list_.removed == 1
         && zero.item_list_.head_ == 0 && zero.item_list_.allocator_ == 0);

  Dialog failed; failed.field_1C_ = 12; failed.item_list_.allocator_ = (void *)1;
  one_entry(failed); failed.heap_.result = 1;
  assert(failed.init() == 4 && failed.heap_.calls == 1
         && failed.item_list_.removed == 1 && failed.item_list_.head_ == 0
         && failed.item_list_.allocator_ == (void *)1);

  Dialog success; success.field_1C_ = 12; one_entry(success);
  assert(success.init() == 0 && success.heap_.calls == 1
         && success.item_list_.allocator_ == &success.heap_
         && success.item_list_.head_ == 0);
}
'''
    compile_run(tmp_path, harness)


def test_pop_ask_actual_projected_region_forwards_without_rewriting_arguments(
        tmp_path: Path) -> None:
    body = region("src/popup.cpp", 0x00627830)
    harness = r'''
#include <cassert>
#define __cdecl
static char *got_a, *got_b, *got_d; static int got_value, got_flags, calls;
static int (__cdecl *got_callback)();
int __cdecl pop_ask(char *, char *, int, char *, int (__cdecl *)(), int);
int __cdecl pop_ask(char *a, char *b, int value, char *d,
                    int (__cdecl *callback)(), int flags) {
  ++calls; got_a=a; got_b=b; got_value=value; got_d=d;
  got_callback=callback; got_flags=flags; return 73;
}
''' + body + r'''
int callback() { return 5; }
int main() {
  char a[]="file", b[]="section";
  assert(pop_ask(a,b,-7,callback,0x44) == 73);
  assert(calls == 1 && got_a == a && got_b == b && got_value == -7
         && got_d == 0 && got_callback == callback && got_flags == 0x44);
}
'''
    compile_run(tmp_path, harness)


def test_deswin_lifetime_actual_projected_region(tmp_path: Path) -> None:
    body = region("src/game.cpp", 0x00455E50)
    assert "Buffer scratch;" in body
    assert "storage[0x588]" not in body
    compile_run(tmp_path, r'''
#include <cassert>
#define __cdecl
static int trace=0;
struct Buffer { Buffer() { trace=trace*10+1; } ~Buffer() { trace=trace*10+2; } };
''' + body + r'''
int main() { load_deswin_sprites(); assert(trace==12); }
''')
    assert "0x004" not in body and "0x005" not in body and "0x006" not in body


def test_control_rejects_a_commented_out_buffer_definition(tmp_path, monkeypatch) -> None:
    source = (SOURCE_ROOT / "src/game.cpp").read_text()
    body = region("src/game.cpp", 0x00455E50)
    assert source.count(body) == 1
    damaged = source.replace(body, "/*\n" + body + "\n*/", 1)
    (tmp_path / "src").mkdir()
    (tmp_path / "src/game.cpp").write_text(damaged)
    monkeypatch.setitem(globals(), "SOURCE_ROOT", tmp_path)
    with pytest.raises(AssertionError, match="definition is not C\\+\\+ code"):
        region("src/game.cpp", 0x00455E50)


def test_pop2_owner_views_and_complete_cold_span_are_retained_in_projection() -> None:
    pop2_body = region("src/popup.cpp", 0x00405140)
    assert "Popup pop;" in pop2_body
    assert "BasePop pop;" not in pop2_body and "Scroll scroll;" not in pop2_body
    assert "MessageWinDeferredBaseMessages[idx]" in pop2_body
    assert "while (idx < 8)" in pop2_body
    assert "while (idx < 9)" not in pop2_body
    assert "Bases[a3].faction_id_current" in pop2_body
    assert "GameDataStateStorage.fields_.game_warnings_" in pop2_body
    assert "pop.owned_picture()" in pop2_body
    assert "pop.set_caller_picture(&sprite)" in pop2_body
    assert "pop.close()" not in pop2_body and "scroll.close()" not in pop2_body
    assert "entry->string_payload()->text" in pop2_body
    assert "reinterpret_cast<char *>(0x" not in pop2_body
    assert pop2_body.count("return 0;") == 2
    popup_path = SOURCE_ROOT / "src/popup.cpp"
    popup_records = [r for r in read_text(popup_path.read_text(encoding="utf-8"),
                                          popup_path.resolve())
                     if r.address == 0x00405140]
    assert len(popup_records) == 1
    assert popup_records[0].image_spans == ((0x00405140, 0x0040559B),
                                             (0x0065078F, 0x00650839))


def test_pop2_actual_projected_region_preserves_popup_queue_and_message_paths(
        tmp_path: Path) -> None:
    body = region("src/popup.cpp", 0x00405140)
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#define __cdecl
struct StringStructPayload { const char *text; };
struct StringStructEntry {
  int id; StringStructPayload *payload; StringStructEntry *next, *previous;
  StringStructPayload *string_payload() { return payload; }
};
struct StringList {
  StringStructEntry *head_=0, *current_=0; int entry_count_=0, current_position_=0;
  const char *pending_text_=0; uint32_t field_20_=0, field_24_=0;
  int add_count=0, last_id=0;
  int add(int id) { ++add_count; last_id=id; return 0; }
  void next_entry() {
    if (entry_count_ == 0) return;
    current_position_ = (current_position_ + 1) % entry_count_;
    current_ = head_;
    for (int i=0; i<current_position_; ++i) current_=current_->next;
  }
};
static int sprite_result=0, sprite_calls=0, sprite_h=0, sprite_w=0;
static const char *sprite_file=0; static bool caller_was_own=false;
struct Sprite {
  int init(char *f, int h, int w) {
    ++sprite_calls; sprite_file=f; sprite_h=h; sprite_w=w; return sprite_result;
  }
};
struct GraphicWin {};
struct BaseRecord { uint8_t faction_id_current=0; } Bases[16];
int IsMultiplayerPBEM=0;
char StringTemp[1032];
char PopupStartCaption[256]="CAPTION";
const char BufferJoinSpace[]=" ";
StringList MessageWinDeferredBaseMessages[8];
struct LockMaskBytes { uint8_t human_mask_=0, alive_mask_=0, unused_[2]{}; };
struct LockMaskView { uint32_t enable_mask_=0; LockMaskBytes bytes_; };
struct StateFields { uint32_t game_warnings_=0; LockMaskView lock_masks_; };
struct StateStorage { StateFields fields_; } GameDataStateStorage;
struct MainInterface {
  int calls=0, a=0,b=0,c=0,d=0; char text[1032];
  void set_message(char *s,int a_,int b_,int c_,int d_) {
    ++calls; std::strcpy(text,s); a=a_; b=b_; c=c_; d=d_;
  }
} MainInterfaceGlobal;
static StringStructEntry *input_head=0; static int input_count=0;
static int scroll_closes=0, base_closes=0, exec_result=0;
static int dims_result=0, dim_h=0, dim_w=0;
static const char *started_file=0, *started_label=0;
class Buffer {
 public: static int get_pcx_dimensions(const char *,int *h,int *w) {
    if (!dims_result) { *h=dim_h; *w=dim_w; } return dims_result;
  }
};
struct Popup {
  StringList unit_ids_; Sprite sprite_; Sprite *caller=0;
  Popup() { unit_ids_.head_=input_head; unit_ids_.entry_count_=input_count; }
  ~Popup() { close(); }
  void start(char *file,const char *label,int,char *,int,GraphicWin *) {
    started_file=file; started_label=label;
  }
  void close() { ++scroll_closes; ++base_closes; }
  int exec(int,int (__cdecl *)()) { return exec_result; }
  Sprite &owned_picture() { return sprite_; }
  void set_caller_picture(Sprite *p) { caller=p; caller_was_own=(p==&sprite_); }
};
''' + body + r'''
static StringStructEntry first, second;
static StringStructPayload first_payload, second_payload;
static void setup_list() {
  first_payload.text="A"; second_payload.text="B";
  first={1,&first_payload,&second,0}; second={2,&second_payload,0,&first};
  input_head=&first; input_count=2;
}
static void reset() {
  std::memset(StringTemp,0,sizeof StringTemp);
  scroll_closes=base_closes=MainInterfaceGlobal.calls=0;
  sprite_result=0; sprite_calls=0; sprite_h=sprite_w=0; sprite_file=0;
  caller_was_own=false;
  for (int i=0;i<8;++i) MessageWinDeferredBaseMessages[i]=StringList();
  dims_result=0; sprite_result=0; exec_result=37; dim_h=22; dim_w=11;
  GameDataStateStorage.fields_.game_warnings_=0;
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_=0;
  GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_=0;
}
int main() {
  setup_list();

  reset(); IsMultiplayerPBEM=1;
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_=(1u<<1)|(1u<<3);
  GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_=(1u<<1)|(1u<<3);
  assert(POP2("QUESTION",0,-1)==0);
  assert(std::strcmp(StringTemp,"A B")==0);
  assert(started_file==PopupStartCaption && std::strcmp(started_label,"QUESTION")==0);
  assert(MessageWinDeferredBaseMessages[1].add_count==1
         && MessageWinDeferredBaseMessages[3].add_count==1
         && MessageWinDeferredBaseMessages[1].pending_text_==StringTemp
         && MessageWinDeferredBaseMessages[3].last_id==0x10000000);
  assert(MessageWinDeferredBaseMessages[0].add_count==0
         && MessageWinDeferredBaseMessages[7].add_count==0);
  assert(scroll_closes==1 && base_closes==1 && MainInterfaceGlobal.calls==0);

  reset(); IsMultiplayerPBEM=1;
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_=1u<<3;
  Bases[2].faction_id_current=3;
  assert(POP2("QUESTION",0,2)==0);
  assert(MessageWinDeferredBaseMessages[3].add_count==1
         && MessageWinDeferredBaseMessages[3].last_id==(2|0x40000000));
  assert(scroll_closes==1 && base_closes==1);

  reset(); IsMultiplayerPBEM=1;
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_=1u<<1;
  GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_=1u<<1;
  assert(POP2("QUESTION",0,-2)==0);
  assert(MessageWinDeferredBaseMessages[1].add_count==1
         && MessageWinDeferredBaseMessages[1].last_id==0x10000000);

  reset(); IsMultiplayerPBEM=0;
  assert(POP2("QUESTION",0,0)==0);
  assert(MainInterfaceGlobal.calls==1 && std::strcmp(MainInterfaceGlobal.text,"A B")==0
         && MainInterfaceGlobal.a==0x5F && MainInterfaceGlobal.b==0
         && MainInterfaceGlobal.c==1 && MainInterfaceGlobal.d==1);
  assert(scroll_closes==1 && base_closes==1);

  reset(); IsMultiplayerPBEM=0;
  GameDataStateStorage.fields_.game_warnings_=0x20000;
  exec_result=19;
  assert(POP2("QUESTION","picture.pcx",0)==19);
  assert(MainInterfaceGlobal.calls==1 && scroll_closes==1 && base_closes==1);
  assert(sprite_calls==1 && sprite_file!=0
         && std::strcmp(sprite_file,"picture.pcx")==0
         && sprite_h==22 && sprite_w==11 && caller_was_own);

  reset(); IsMultiplayerPBEM=0;
  GameDataStateStorage.fields_.game_warnings_=0x20000;
  sprite_result=1;
  assert(POP2("QUESTION","picture.pcx",0)==37);
  assert(sprite_calls==1 && !caller_was_own);
}
'''
    compile_run(tmp_path, harness)
