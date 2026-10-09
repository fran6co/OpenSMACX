"""Host controls for the actual BasePop::close source.

The projected fixture extracts production member declarations, inheritance
clauses, BasePop::close, and Heap lifetime bodies. It checks source-level
ownership and behavior only; it does not claim native offsets, VC6 output, or
game acceptance.
"""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


ROOT = next(
    path for path in Path(__file__).resolve().parents
    if (path / "src/IMAGE_PIN").is_file()
)


def extract_body(text, signature):
    match = re.search(r"(?m)^" + re.escape(signature) + r"\s*\{", text)
    assert match, f"missing unique production definition: {signature}"
    opening = text.index("{", match.start())
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():index + 1]
    raise AssertionError(f"unbalanced production body: {signature}")


def declaration_for(class_text, name):
    pattern = re.compile(
        r"^.*\b" + re.escape(name) + r"\s*(?:\[[^]]+\])?\s*;$"
    )
    found = []
    for line in class_text.splitlines():
        declaration = line.split("//", 1)[0].strip()
        if (pattern.fullmatch(declaration) and "=" not in declaration
                and not declaration.startswith("return ")):
            found.append(declaration)
    assert len(found) == 1, f"expected one declaration for {name}, got {found}"
    return found[0]


def class_source(header, name):
    match = re.search(
        r"(?m)^class\s+" + re.escape(name) + r"\b[^;{]*\{", header
    )
    assert match, f"missing class definition: {name}"
    opening = header.index("{", match.start())
    depth = 0
    for index in range(opening, len(header)):
        if header[index] == "{":
            depth += 1
        elif header[index] == "}":
            depth -= 1
            if depth == 0:
                return header[match.start():index + 1]
    raise AssertionError(f"unbalanced class declaration: {name}")


def inheritance_clause(header, name):
    match = re.search(r"(?ms)^class\s+" + re.escape(name) + r"\b(.*?)\{", header)
    assert match, f"missing class inheritance: {name}"
    return match.group(1).rstrip()


def instrument_body(body, statement):
    end = body.rfind("}")
    assert end >= 0
    return body[:end] + "\n  " + statement + "\n" + body[end:]


BASEPOP_MEMBERS = (
    "field_30A4_", "field_3094_", "text_block_a_", "text_block_b_",
    "ok_text_", "cancel_text_", "owner_window_", "default_block_",
    "dialogs_", "sprite_", "field_20F4_", "caller_sprite_", "selected_id_",
    "value_", "owned_text_", "field_21C8_", "field_21CC_", "flat_button1_",
    "flat_button2_", "unit_ids_", "item_ids_", "item_buttons_", "field_21B8_",
    "field_21BC_", "field_21B0_", "field_21B4_", "field_21C0_", "field_21C4_",
    "field_30B0_", "field_30B4_", "field_30B8_", "field_30BC_", "field_30C0_",
    "field_30C4_", "field_30C8_", "field_30CC_", "field_30D0_", "field_30D4_",
    "field_30D8_", "field_30DC_", "field_30E0_", "field_30E4_", "field_30E8_",
    "field_30EC_", "field_30F0_", "field_30F4_", "field_30A8_", "field_30AC_",
    "loc_a_", "loc_b_", "field_3100_", "field_3104_", "field_3108_",
    "field_310C_", "string_font1_", "string_color_a_", "string_color_b_",
    "string_color_c_", "string_color_d_", "string_font2_", "string_color_2a_",
    "string_color_2b_", "string_color_2c_", "string_color_2d_", "string_font3_",
    "string_color_3a_", "string_color_3b_", "string_color_3c_", "string_color_3d_",
    "string_font4_", "string_color_hyper_a_", "string_color_hyper_b_",
    "string_color_hyperc_", "string_color_hyper_d_", "sprite_click_callback_",
    "sprite_double_click_callback_", "redraw_callback_", "button_font1_",
    "button_color_a_", "button_color_b_", "button_color_c_", "button_color_d_",
    "button_font2_", "button_color_2a_", "button_color_2b_", "button_color_2c_",
    "button_color_2d_", "button_font3_", "button_color_3a_", "button_color_3b_",
    "button_color_3c_", "button_color_3d_", "field_31A4_", "field_31A8_",
    "field_31AC_", "field_31B0_", "field_31B4_", "field_214C_", "field_2148_",
    "field_322C_", "field_31A0_", "field_317B_", "external_window_",
    "default_loc_a_", "default_loc_b_", "dialog_width_", "dialog_height_", "heap_",
)


def production_parts():
    basepop_h = (ROOT / "src/basepop.h").read_text(encoding="utf-8")
    basepop_cpp = (ROOT / "src/basepop.cpp").read_text(encoding="utf-8")
    heap_h = (ROOT / "src/heap.h").read_text(encoding="utf-8")
    heap_cpp = (ROOT / "src/heap.cpp").read_text(encoding="utf-8")
    dialog_h = (ROOT / "src/dialog.h").read_text(encoding="utf-8")

    basepop_class = class_source(basepop_h, "BasePop")
    heap_class = class_source(heap_h, "Heap")
    basepop_fields = "\n".join(
        "  " + declaration_for(basepop_class, name) for name in BASEPOP_MEMBERS
    )
    heap_fields = "\n".join(
        "  " + declaration_for(heap_class, name)
        for name in ("err_flags_", "base_", "current_", "base_size_", "free_size_")
    )
    dialog_fields = "\n".join(
        "  " + declaration_for(class_source(dialog_h, "Dialog"), name)
        for name in ("field_20_", "field_2C_", "field_30_")
    )
    text_block_match = re.search(r"(?ms)^struct\s+BasePopTextBlock\s*\{.*?^};", basepop_h)
    assert text_block_match

    close = extract_body(basepop_cpp, "void BasePop::close()")
    close = re.sub(r"(?<![\w:])free\(", "tracked_free(", close)
    heap_reset = extract_body(heap_cpp, "void Heap::reset()")
    heap_destructor = extract_body(heap_cpp, "Heap::~Heap()")
    heap_init = extract_body(heap_cpp, "BOOL Heap::init(size_t req_size)")
    heap_reset = instrument_body(heap_reset, "++heap_reset_calls; record(50, this);")
    heap_destructor = instrument_body(
        heap_destructor, "++heap_destructor_calls; record(51, this);"
    )

    controls = {
        name: inheritance_clause(
            (ROOT / f"src/{file_name}").read_text(encoding="utf-8"), name
        )
        for name, file_name in (
            ("ListBox", "listbox.h"), ("RadioButton", "jackal/radiobutton.h"),
            ("CheckBox", "jackal/checkbox.h"), ("SpriteBox", "spritebox.h"),
            ("EditGroup", "editgroup.h"), ("Dialogs", "dialogs.h"),
        )
    }
    basepop_inheritance = inheritance_clause(basepop_h, "BasePop")
    heap_inheritance = inheritance_clause(heap_h, "Heap")
    assert heap_inheritance == ""
    return {
        "basepop_fields": basepop_fields,
        "heap_fields": heap_fields,
        "dialog_fields": dialog_fields,
        "text_block": text_block_match.group(0),
        "close": close,
        "heap_reset": heap_reset,
        "heap_destructor": heap_destructor,
        "heap_init": heap_init,
        "controls": controls,
        "basepop_inheritance": basepop_inheritance,
    }


PREAMBLE = r'''#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#define __cdecl
using std::memcpy;
using std::memcmp;
using std::free;
typedef uint32_t BOOL;
typedef void *LPVOID;
typedef char *LPSTR;
static int WinScreenWidth = 1024;
struct Font {};
struct Sprite;
struct GraphicWin;
struct Dialog;
struct StringStructEntry {};
struct BasePopTextBlock;
typedef void (*BasePopConfigCallback)(BasePopTextBlock *, int);
struct Event { int tag; const void *object; const void *dialog; const void *graphic; };
static Event events[128];
static int event_count;
static int heap_reset_calls;
static int heap_destructor_calls;
static int tracked_free_count;
static void *tracked_frees[16];
static int item_button_destructor_count;
static void record(int tag, const void *object = 0,
                   const void *dialog = 0, const void *graphic = 0) {
  assert(event_count < 128);
  events[event_count++] = Event{tag, object, dialog, graphic};
}
static void tracked_free(void *pointer) {
  assert(pointer != 0);
  assert(tracked_free_count < 16);
  tracked_frees[tracked_free_count++] = pointer;
  record(40, pointer);
  std::free(pointer);
}
struct GraphicWin {
  virtual ~GraphicWin() {}
  void close() { record(6, this); }
};
struct Win : GraphicWin {};
struct Dialog {
  DIALOG_FIELDS
};
struct ListBox LISTBOX_BASES {
  uint32_t close() {
    record(1, this, static_cast<Dialog *>(this), static_cast<GraphicWin *>(this));
    return 0;
  }
};
struct RadioButton RADIO_BASES {
  uint32_t close() {
    record(3, this, static_cast<Dialog *>(this), static_cast<GraphicWin *>(this));
    return 0;
  }
};
struct CheckBox CHECK_BASES {
  uint32_t close() {
    record(2, this, static_cast<Dialog *>(this), static_cast<GraphicWin *>(this));
    return 0;
  }
};
struct SpriteBox SPRITEBOX_BASES {
  typedef void (*ClickCallback)();
  typedef void (*DrawCallback)();
  void close() {
    record(4, this, static_cast<Dialog *>(this), static_cast<GraphicWin *>(this));
  }
};
struct EditGroup EDITGROUP_BASES {
  void close() {
    record(5, this, static_cast<Dialog *>(this), static_cast<GraphicWin *>(this));
  }
};
struct Dialogs DIALOGS_BASES {};
struct Sprite {
  void close() { record(7, this); }
};
struct FlatButton {
  int id;
  bool heap_allocated;
  FlatButton(int value = 0) : id(value), heap_allocated(false) {}
  virtual ~FlatButton() {
    if (heap_allocated) {
      ++item_button_destructor_count;
      record(60 + id, this);
    }
  }
  void close() { record(7 + id, this); }
};
struct StringList {
  StringStructEntry *head_;
  int current_position_;
  int id;
  int remove_calls;
  StringList() : head_(0), current_position_(0), id(0), remove_calls(0) {}
  void remove_all() {
    ++remove_calls;
    record(10 + id, this);
    head_ = 0;
  }
};
struct Heap {
  HEAP_FIELDS
  Heap() : err_flags_(0), base_(0), current_(0), base_size_(0), free_size_(0) {}
  ~Heap();
  void reset();
  BOOL init(size_t req_size);
};
static void *mem_get(size_t size) { return std::malloc(size); }
HEAP_DESTRUCTOR
HEAP_RESET
HEAP_INIT
struct BasePop BASEPOP_BASE {
public:
  BASEPOP_FIELDS
  BasePop() : flat_button1_(1), flat_button2_(2) {
    unit_ids_.id = 0;
    item_ids_.id = 1;
  }
  void close();
};
static BasePopConfigCallback BasePopDefaultField3094;
static uint32_t BasePopDefaultField30A4;
static uint32_t BasePopDefaultBlock2E64[70];
static uint32_t BasePopDefaultBlock2F7C[70];
static Sprite *BasePopDefaultBlock31B8[27];
static uint32_t BasePopDefaultField21C0[2];
static uint32_t BasePopDefaultField3104[3];
static uint32_t BasePopDefaultField31A0[4];
static uint32_t BasePopDefaultField21B0[4];
static uint32_t BasePopDefaultField31B0[2];
static uint32_t BasePopDefaultField2148[2];
static uint32_t BasePopDefaultStringColors[4][4];
static uint32_t BasePopDefaultButtonColors[4][3];
static Font *BasePopDefaultStringFonts[4];
static Font *BasePopDefaultButtonFonts[3];
static Win *BasePopDefaultField322C;
static uint32_t BasePopDefaultLayout[3];
static uint32_t BasePopDefaultDialogSize[2];
#define tracked_free free
BASEPOP_CLOSE
#undef tracked_free
'''


def projection(parts, close=None):
    control_map = {
        "DIALOG_FIELDS": parts["dialog_fields"],
        "LISTBOX_BASES": parts["controls"]["ListBox"],
        "RADIO_BASES": parts["controls"]["RadioButton"],
        "CHECK_BASES": parts["controls"]["CheckBox"],
        "SPRITEBOX_BASES": parts["controls"]["SpriteBox"],
        "EDITGROUP_BASES": parts["controls"]["EditGroup"],
        "DIALOGS_BASES": parts["controls"]["Dialogs"],
        "BASEPOP_BASE": parts["basepop_inheritance"],
        "HEAP_FIELDS": parts["heap_fields"],
        "HEAP_DESTRUCTOR": parts["heap_destructor"],
        "HEAP_RESET": parts["heap_reset"],
        "HEAP_INIT": parts["heap_init"],
        "BASEPOP_FIELDS": parts["basepop_fields"],
        "BASEPOP_CLOSE": close or parts["close"],
    }
    code = PREAMBLE
    code = code.replace("#define tracked_free free", "")
    # BasePop::close's three unqualified frees are instrumented only in this
    # projection. Heap::reset and Heap::~Heap retain their qualified std::free.
    code = code.replace("BASEPOP_CLOSE", control_map["BASEPOP_CLOSE"])
    for key, value in control_map.items():
        if key != "BASEPOP_CLOSE":
            code = code.replace(key, value)
    return parts["text_block"] + "\n" + code + HOST_MAIN


HOST_MAIN = r'''
static void dummy_callback(BasePopTextBlock *, int) {}
static void initialize_defaults() {
  BasePopDefaultField3094 = dummy_callback;
  BasePopDefaultField30A4 = 0xA0B0C0D0u;
  for (int i=0; i<70; ++i) {
    BasePopDefaultBlock2E64[i] = 0x11000000u + (uint32_t)i;
    BasePopDefaultBlock2F7C[i] = 0x22000000u + (uint32_t)i;
  }
  for (int i=0; i<27; ++i)
    BasePopDefaultBlock31B8[i] = reinterpret_cast<Sprite *>(uintptr_t(0x1000+i));
  for (int i=0; i<4; ++i) {
    BasePopDefaultField21B0[i] = 0x2100u + (uint32_t)i;
    BasePopDefaultField31A0[i] = 0x3100u + (uint32_t)i;
    BasePopDefaultStringFonts[i] = reinterpret_cast<Font *>(uintptr_t(0x2000+i));
    for (int j=0; j<4; ++j)
      BasePopDefaultStringColors[i][j] = 0x4000u + (uint32_t)(i*16+j);
  }
  BasePopDefaultField21C0[0] = 0x21C0;
  BasePopDefaultField21C0[1] = 0x21C1;
  BasePopDefaultField3104[0] = 0x310400;
  BasePopDefaultField3104[1] = 0x310401;
  BasePopDefaultField3104[2] = 0x310402;
  BasePopDefaultField31B0[0] = 0x31B000;
  BasePopDefaultField31B0[1] = 0x31B001;
  BasePopDefaultField2148[0] = 0x214800;
  BasePopDefaultField2148[1] = 0x214801;
  for (int i=0; i<3; ++i)
    BasePopDefaultButtonFonts[i] = reinterpret_cast<Font *>(uintptr_t(0x3000+i));
  for (int i=0; i<4; ++i)
    for (int j=0; j<3; ++j)
      BasePopDefaultButtonColors[i][j] = 0x5000u + (uint32_t)(i*16+j);
  BasePopDefaultField322C = reinterpret_cast<Win *>(uintptr_t(0x4455));
  BasePopDefaultLayout[0] = 1;
  BasePopDefaultLayout[1] = 0x1234;
  BasePopDefaultLayout[2] = 0x2000;
  BasePopDefaultDialogSize[0] = 301;
  BasePopDefaultDialogSize[1] = 101;
}
static FlatButton *new_item_button(FlatButton **slots, int index) {
  FlatButton *button = new FlatButton(index);
  button->heap_allocated = true;
  slots[index] = button;
  return button;
}
static void assert_dialog_views(BasePop &pop) {
  Dialogs &d = pop.dialogs_;
  Dialog *views[] = {
    static_cast<Dialog *>(static_cast<ListBox *>(&d)),
    static_cast<Dialog *>(static_cast<CheckBox *>(&d)),
    static_cast<Dialog *>(static_cast<RadioButton *>(&d)),
    static_cast<Dialog *>(static_cast<SpriteBox *>(&d)),
    static_cast<Dialog *>(static_cast<EditGroup *>(&d))
  };
  GraphicWin *graphics[] = {
    static_cast<GraphicWin *>(static_cast<ListBox *>(&d)),
    static_cast<GraphicWin *>(static_cast<CheckBox *>(&d)),
    static_cast<GraphicWin *>(static_cast<RadioButton *>(&d)),
    static_cast<GraphicWin *>(static_cast<SpriteBox *>(&d)),
    static_cast<GraphicWin *>(static_cast<EditGroup *>(&d))
  };
  for (int i=1; i<5; ++i) {
    assert(views[i] == views[0]);
    assert(graphics[i] == graphics[0]);
  }
}
static void seed_first_close(BasePop &pop) {
  pop.field_30A4_ = 1;
  pop.field_3094_ = 0;
  memset(&pop.text_block_a_, 0xAA, sizeof(pop.text_block_a_));
  memset(&pop.text_block_b_, 0xBB, sizeof(pop.text_block_b_));
  pop.ok_text_ = static_cast<char *>(std::malloc(5));
  pop.cancel_text_ = static_cast<char *>(std::malloc(6));
  pop.owned_text_ = static_cast<char *>(std::malloc(7));
  assert(pop.ok_text_ && pop.cancel_text_ && pop.owned_text_);
  pop.external_window_ = 0;
  pop.field_20F4_ = 1;
  pop.caller_sprite_ = reinterpret_cast<Sprite *>(uintptr_t(0x7777));
  pop.selected_id_ = 88;
  pop.value_ = 99;
  pop.field_21C8_ = 44;
  pop.field_21CC_ = 45;
  pop.field_317B_ = 0x7D;
  pop.dialogs_.Dialog::field_20_ = 0x10;
  pop.dialogs_.Dialog::field_2C_ = 0xAA;
  pop.dialogs_.Dialog::field_30_ = 0xBB;
  pop.loc_a_ = 0x1111;
  pop.loc_b_ = 0x2222;
  pop.unit_ids_.head_ = reinterpret_cast<StringStructEntry *>(uintptr_t(0x11));
  pop.item_ids_.head_ = reinterpret_cast<StringStructEntry *>(uintptr_t(0x22));
  pop.unit_ids_.current_position_ = 8;
  pop.item_ids_.current_position_ = 9;
  pop.unit_ids_.remove_calls = pop.item_ids_.remove_calls = 0;
  for (int i=0; i<8; ++i) pop.item_buttons_[i] = 0;
  new_item_button(pop.item_buttons_, 0);
  new_item_button(pop.item_buttons_, 3);
  new_item_button(pop.item_buttons_, 7);
}
static void verify_first_close(BasePop &pop, const void *ok, const void *cancel,
                               const void *owned) {
  assert(pop.field_30A4_ == 0xA0B0C0D0u);
  assert(pop.field_3094_ == dummy_callback);
  assert(memcmp(&pop.text_block_a_, BasePopDefaultBlock2E64, 0x118) == 0);
  assert(memcmp(&pop.text_block_b_, BasePopDefaultBlock2F7C, 0x118) == 0);
  assert(pop.ok_text_ == 0 && pop.cancel_text_ == 0 && pop.owned_text_ == 0);
  assert(tracked_frees[0] == ok && tracked_frees[1] == cancel && tracked_frees[2] == owned);
  assert(pop.owner_window_ == static_cast<GraphicWin *>(&pop));
  for (int i=0; i<27; ++i) assert(pop.default_block_[i] == BasePopDefaultBlock31B8[i]);
  assert(pop.field_20F4_ == 0 && pop.caller_sprite_ == 0);
  assert(pop.selected_id_ == 0 && pop.value_ == -1);
  assert(pop.field_21C8_ == 0 && pop.field_21CC_ == -1);
  assert(pop.unit_ids_.head_ == 0 && pop.item_ids_.head_ == 0);
  assert(pop.unit_ids_.current_position_ == 0 && pop.item_ids_.current_position_ == 0);
  assert(pop.unit_ids_.remove_calls == 1 && pop.item_ids_.remove_calls == 1);
  for (int i=0; i<8; ++i) assert(pop.item_buttons_[i] == 0);
  assert(pop.field_21B0_ == 0x2100 && pop.field_21B4_ == 0x2101);
  assert(pop.field_21BC_ == 0x2102 && pop.field_21B8_ == 0x2103);
  assert(pop.field_21C0_ == 0x21C0 && pop.field_21C4_ == 0x21C1);
  assert(pop.field_30B0_ == 0 && pop.field_30F4_ == 0);
  assert(pop.loc_a_ == 0x1234 && pop.loc_b_ == 0x1000);
  assert(pop.field_3104_ == 0x310400 && pop.field_3108_ == 0x310400);
  assert(pop.field_310C_ == 0x310402);
  assert(pop.string_font1_ == BasePopDefaultStringFonts[0]);
  assert(pop.string_font2_ == BasePopDefaultStringFonts[1]);
  assert(pop.string_font3_ == BasePopDefaultStringFonts[2]);
  assert(pop.string_font4_ == BasePopDefaultStringFonts[3]);
  assert(pop.string_color_a_ == BasePopDefaultStringColors[0][0]);
  assert(pop.string_color_hyper_d_ == BasePopDefaultStringColors[3][3]);
  assert(pop.sprite_click_callback_ == 0 && pop.sprite_double_click_callback_ == 0);
  assert(pop.redraw_callback_ == 0);
  assert(pop.button_font1_ == BasePopDefaultButtonFonts[0]);
  assert(pop.button_font2_ == BasePopDefaultButtonFonts[1]);
  assert(pop.button_font3_ == BasePopDefaultButtonFonts[2]);
  assert(pop.button_color_a_ == (uint8_t)BasePopDefaultButtonColors[0][0]);
  assert(pop.button_color_2a_ == (uint8_t)BasePopDefaultButtonColors[0][1]);
  assert(pop.button_color_3a_ == (uint8_t)BasePopDefaultButtonColors[0][2]);
  assert(pop.field_317B_ == 0x7D); // source leaves the spare byte untouched
  assert(pop.button_color_b_ == BasePopDefaultButtonColors[1][0]);
  assert(pop.button_color_3d_ == BasePopDefaultButtonColors[3][2]);
  assert(pop.field_31A0_ == (uint8_t)BasePopDefaultField31A0[0]);
  assert(pop.field_31A4_ == BasePopDefaultField31A0[1]);
  assert(pop.field_31A8_ == BasePopDefaultField31A0[2]);
  assert(pop.field_31AC_ == BasePopDefaultField31A0[3]);
  assert(pop.field_31B0_ == BasePopDefaultField31B0[0]);
  assert(pop.field_31B4_ == BasePopDefaultField31B0[1]);
  assert(pop.field_2148_ == reinterpret_cast<Sprite *>(BasePopDefaultField2148[0]));
  assert(pop.field_214C_ == reinterpret_cast<Sprite *>(BasePopDefaultField2148[1]));
  assert(pop.field_322C_ == BasePopDefaultField322C);
  assert(pop.default_loc_a_ == 0x1234 && pop.default_loc_b_ == 0x2000);
  assert(pop.dialog_width_ == 301 && pop.dialog_height_ == 101);
  assert(pop.dialogs_.Dialog::field_2C_ == 301);
  assert(pop.dialogs_.Dialog::field_30_ == 101); // contradictory inner null test stays false
  assert(pop.dialogs_.Dialog::field_20_ == 0x13);
  assert(pop.heap_.base_ == 0 && pop.heap_.current_ == 0);
  assert(pop.heap_.free_size_ == 0 && pop.heap_.base_size_ == 0);
}
int main() {
  initialize_defaults();
  {
    BasePop pop;
    assert_dialog_views(pop);
    seed_first_close(pop);
    assert(pop.heap_.init(64) == 0);
    const void *ok = pop.ok_text_, *cancel = pop.cancel_text_, *owned = pop.owned_text_;
    pop.close();
    verify_first_close(pop, ok, cancel, owned);
    const int expected[] = {40,40,1,2,3,4,5,6,7,40,8,9,10,11,60,63,67,50};
    assert(event_count == (int)(sizeof(expected)/sizeof(expected[0])));
    for (int i=0; i<event_count; ++i) assert(events[i].tag == expected[i]);
    assert(tracked_free_count == 3 && item_button_destructor_count == 3);
    assert(heap_reset_calls == 1 && heap_destructor_calls == 0);
    assert(events[2].object == static_cast<ListBox *>(&pop.dialogs_));
    assert(events[3].object == static_cast<CheckBox *>(&pop.dialogs_));
    assert(events[4].object == static_cast<RadioButton *>(&pop.dialogs_));
    assert(events[5].object == static_cast<SpriteBox *>(&pop.dialogs_));
    assert(events[6].object == static_cast<EditGroup *>(&pop.dialogs_));
    assert(events[2].dialog == events[3].dialog && events[3].dialog == events[4].dialog);
    assert(events[4].dialog == events[5].dialog && events[5].dialog == events[6].dialog);
    assert(events[2].graphic == events[3].graphic && events[3].graphic == events[4].graphic);
    assert(events[4].graphic == events[5].graphic && events[5].graphic == events[6].graphic);
    assert(events[7].object == static_cast<GraphicWin *>(&pop));
    assert(events[8].object == &pop.sprite_);
    assert(pop.heap_.init(32) == 0); // live Heap member remains reusable after close
    BasePopDefaultLayout[0] = 0;
    BasePopDefaultLayout[1] = 0x2000;
    BasePopDefaultLayout[2] = 0x2000;
    pop.dialogs_.Dialog::field_2C_ = 0xAAAA;
    pop.dialogs_.Dialog::field_30_ = 0xBBBB;
    pop.dialogs_.Dialog::field_20_ = 0xCCCC;
    pop.owned_text_ = static_cast<char *>(std::malloc(8));
    assert(pop.owned_text_ != 0);
    pop.unit_ids_.head_ = reinterpret_cast<StringStructEntry *>(uintptr_t(0x33));
    pop.item_ids_.head_ = reinterpret_cast<StringStructEntry *>(uintptr_t(0x44));
    pop.unit_ids_.current_position_ = 4;
    pop.item_ids_.current_position_ = 5;
    pop.item_buttons_[7] = new_item_button(pop.item_buttons_, 7);
    const int event_start = event_count;
    pop.close();
    assert(pop.dialogs_.Dialog::field_2C_ == 0xAAAA);
    assert(pop.dialogs_.Dialog::field_30_ == 0xBBBB);
    assert(pop.dialogs_.Dialog::field_20_ == 0xCCCC);
    assert(pop.heap_.base_ == 0 && pop.heap_.current_ == 0);
    assert(pop.unit_ids_.remove_calls == 2 && pop.item_ids_.remove_calls == 2);
    assert(pop.item_buttons_[7] == 0);
    assert(heap_reset_calls == 2 && heap_destructor_calls == 0);
    assert(tracked_free_count == 4 && item_button_destructor_count == 4);
    assert(events[event_count-1].tag == 50);
    assert(event_count > event_start);
  }
  assert(heap_destructor_calls == 1); // only compiler-owned object teardown ends Heap lifetime
  return 0;
}
'''


def assert_source_contract(parts):
    body = parts["close"]
    assert "self +" not in body
    assert "reinterpret_cast<GraphicWin *>(BasePopDefaultLayout[0])" in body
    assert "reinterpret_cast<Sprite *>(BasePopDefaultField2148[" in body
    assert "dialogs_.ListBox::close();" in body
    assert "dialogs_.CheckBox::close();" in body
    assert "dialogs_.RadioButton::close();" in body
    assert "dialogs_.SpriteBox::close();" in body
    assert "dialogs_.EditGroup::close();" in body
    assert "GraphicWin::close();" in body and "sprite_.close();" in body
    assert "heap_.reset();" in body
    assert "heap_.~Heap()" not in body
    assert "field_3108_ = BasePopDefaultField3104[0];" in body
    assert "field_317B_" not in body
    assert "if (external_window_ != 0)" in body
    assert re.search(
        r"if \(external_window_ != 0\).*?if \(external_window_ == 0\)",
        body, re.S
    )
    ordered = (
        "field_30A4_ =", "field_3094_ =", "memcpy(&text_block_a_",
        "if (ok_text_", "if (cancel_text_", "owner_window_ = this",
        "for (int column", "dialogs_.ListBox::close();",
        "dialogs_.CheckBox::close();", "dialogs_.RadioButton::close();",
        "dialogs_.SpriteBox::close();", "dialogs_.EditGroup::close();",
        "GraphicWin::close();", "sprite_.close();", "if (owned_text_",
        "flat_button1_.close();", "if (unit_ids_.head_", "if (item_ids_.head_",
        "for (i = 0; i < 8", "field_21B8_ =", "field_30B0_ =",
        "field_3104_ =", "sprite_click_callback_ =", "button_color_a_ =",
        "field_214C_ =", "external_window_ = reinterpret_cast<GraphicWin *>",
        "if (external_window_ != 0)", "dialogs_.Dialog::field_2C_ =",
        "if (external_window_ == 0)", "dialogs_.Dialog::field_30_ =",
        "dialogs_.Dialog::field_20_ |= 3;", "heap_.reset();",
    )
    positions = [body.index(item) for item in ordered]
    assert positions == sorted(positions), "BasePop::close cleanup/default order changed"


def compile_run(compiler, code, expected):
    with tempfile.TemporaryDirectory(prefix="basepop-owned-close-host-") as directory:
        source = Path(directory) / "control.cpp"
        executable = Path(directory) / "control"
        source.write_text(code, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-variable",
             str(source), "-o", str(executable)],
            text=True, capture_output=True, timeout=30,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run(
            [str(executable)], text=True, capture_output=True, timeout=10
        )
        assert (result.returncode == 0) == expected, result.stdout + result.stderr


def test_actual_body_declarations_order_and_lifetime_contract():
    parts = production_parts()
    assert_source_contract(parts)
    assert "StringList unit_ids_;" in parts["basepop_fields"]
    assert "StringList item_ids_;" in parts["basepop_fields"]
    assert "Dialogs dialogs_;" in parts["basepop_fields"]
    assert "Heap heap_;" in parts["basepop_fields"]
    assert "Sprite sprite_;" in parts["basepop_fields"]
    assert "Heap::reset()" in parts["heap_reset"]


def test_host_projection_and_structural_damage_controls():
    parts = production_parts()
    assert_source_contract(parts)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), "both GCC and Clang are required for the host control"
    code = projection(parts)
    compile_run(compilers[0], code, True)
    compile_run(compilers[1], code, True)

    damage = []
    damage.append(parts["close"].replace(
        "field_3108_ = BasePopDefaultField3104[0];",
        "field_3108_ = BasePopDefaultField3104[1];", 1
    ))
    damage.append(parts["close"].replace(
        "dialogs_.CheckBox::close();", "dialogs_.ListBox::close();", 1
    ))
    damage.append(parts["close"].replace("heap_.reset();", "heap_.~Heap();", 1))
    for mutant in damage:
        assert mutant != parts["close"]
        compile_run(compilers[0], projection(parts, mutant), False)
        compile_run(compilers[1], projection(parts, mutant), False)
