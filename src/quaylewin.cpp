// ORIGINAL: 0x004968F0 ??_GQuayleWin@@WEEE@AEPAXI@Z 0x004968F0-0x004968FB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original ??_GQuayleWin@@WEEE@AEPAXI@Z at 0x004968f0 is an 11-byte secondary scalar deleting adjustor; it subtracts 0x444 from ECX and tail-jumps to the complete-object QuayleWin scalar deleting entry without touching stack or flags.", "Historical seed correction: the earlier `??3` deallocation interpretation was refuted; the original is a `??_G` scalar-deleting adjustor. `WEEE` demangles to adjustor{1092}, equal to 0x444.", "Pinned QuayleWin constructor at 0x0049689c stores Buffer-view vftable 0x0066d590 at this+0x444; the source owner is the typed QuayleWin GraphicWin/Buffer multiple-inheritance view.", "Current src/quaylewin.cpp COFF Buffer-view vftable first slot selects generated ??_EQuayleWin@@WEEE@AEPAXI@Z; its body performs the same 0x444 adjustment and preserves the flag argument.", "Current QuayleWin COFF weak external ??_EQuayleWin@@UAEPAXI@Z records search mode 2 (library search) and fallback target ??_GQuayleWin@@UAEPAXI@Z; this linked image resolves both primary labels to the same physical class deleting body. The object fallback and linked address are per-build evidence, not a general E/G equivalence or owner-selection rule.", "That measured current primary body calls QuayleWin::~QuayleWin, tests flags bit 0, calls operator delete only when bit 0 is set, returns the saved object pointer, and uses ret 4; this is the per-class measured E-to-G scalar path, not a general vector/scalar equivalence.", "The removed transitional C++ names were ?adjust_this_quayle_win@@YIPAXPAX0I@Z and ?scalar_delete_quayle_win@@YIPAXPAX0I@Z; no callers exist outside deleting_thunks.cpp/.h, and the current generated Buffer vftable uses the class-owned E adjustor instead.", "Historical seed metadata noted flags hidden;sp_ready;purged_ok, zero direct callers, and no direct call targets. These remain historical observations only; they do not establish virtual-dispatch completeness or acceptance.", "The old secondary record had no BYTE_EXACT or SEMANTIC claim. Its original identity, full span, kind and parsed history are moved unchanged; no matching claim is granted."], "reason": "Bind the pinned secondary scalar deleting identity for QuayleWin to its measured genuine Buffer-view generated adjustor. Keep the original G identity and point its current symbol fact at the owner-emitted E adjustor; the per-object weak fallback and bit-0 path above establish why this exact current implementation preserves the original operation."}
// symbol    ??_EQuayleWin@@WEEE@AEPAXI@Z
// kind      game
// ORIGINAL: 0x004968C0 ??_GQuayleWin@@UAEPAXI@Z 0x004968C0-0x004968E1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004968C0-0x004968E1", "Pinned class constructor ??0QuayleWin@@QAE@XZ at 0x00496810 stores original class/view table VA 0x66d598 at 0x00496896 (mov dword ptr [esi], 0x66d598); the current genuine owner vftable ??_7QuayleWin@@6BWin@@@ selects generated ??_GQuayleWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EQuayleWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GQuayleWin@@UAEPAXI@Z at 0x004968C0 calls the source-mapped ??1QuayleWin@@QAE@XZ at 0x00496710, then tests byte ptr [ebp + 8], 1; je 0x4968da bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/quaylewin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original QuayleWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GQuayleWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x00495170 _$E21 0x00495170-0x00495186 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825e4 selects original initializer0x495170..0x495186, registering fini0x495190..0x49519a; complete independent decode establishes exclusive boundaries.", "Source-owned object?QuayleWinGlobal@@3VQuayleWin@@A identifies original storage0x872cb0. Startup arguments/calls: call 0x496810; push 0x495190. Cleanup receiver/calls: mov ecx, 0x872cb0; jmp 0x496710. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/quaylewin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?QuayleWinGlobal@@3VQuayleWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E16
// ORIGINAL: 0x00495190 _$E19 0x00495190-0x0049519A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825e4 selects original initializer0x495170..0x495186, registering fini0x495190..0x49519a; complete independent decode establishes exclusive boundaries.", "Source-owned object?QuayleWinGlobal@@3VQuayleWin@@A identifies original storage0x872cb0. Startup arguments/calls: call 0x496810; push 0x495190. Cleanup receiver/calls: mov ecx, 0x872cb0; jmp 0x496710. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/quaylewin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?QuayleWinGlobal@@3VQuayleWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E14
// WORKAROUND(noinline): on_redraw's StringList local calls
// StringStruct::StringStruct (0x00401000) out of line (0x00495723), giving
// its three-state EH layout; see stringstruct.h.
#define FONT_LINE_ADVANCE_INLINE
#define STRINGSTRUCT_CTOR_OUT_OF_LINE
#define WIN_RECT_EXTENT_INLINE  // stand-in: see rect_width in win.h
#include "stdafx.h"
#include "quaylewin.h"
#include "rectangle_operations.h"
#include "alpha.h"
#include "base.h"
#include "console.h"
#include "font.h"
#include "game.h"
#include "general.h"
#include "reportif.h"
#include "strings.h"
#include "netdaemon.h"
#include "tutwin.h"
#include "stringstruct.h"


// Independent caption cells passed by the original initializer. Their writers
// and string extent remain recovery obligations; do not infer a larger buffer.
// RECOVERY_OBJECT: v1 0x00876470 ?QuayleStandardCaption@@3PADA
char QuayleStandardCaption[4];
// RECOVERY_OBJECT: v1 0x00876474 ?QuayleWideCaption@@3PADA
char QuayleWideCaption[4];

// The constructor at496896/49689C installs these compiler-generated tables.
// The +444 table belongs to GraphicWin's inherited Buffer subobject.
// Primary Win/GraphicWin table has 90 entries; the next original table starts
// at66D700. The secondary Buffer table has its two inherited entries.
// RECOVERY_GENERATED: v1 vtable 0x0066D598 0x0066D700 ??_7QuayleWin@@6BWin@@@ 0x00496810
// RECOVERY_GENERATED: v1 vtable 0x0066D590 0x0066D598 ??_7QuayleWin@@6BBuffer@@@ 0x00496810
// RECOVERY_OBJECT: v1 0x00872CB0 ?QuayleWinGlobal@@3VQuayleWin@@A
QuayleWin QuayleWinGlobal;

// ORIGINAL: 0x00496810 ??0QuayleWin@@QAE@XZ 0x00496810-0x004968B4;0x00658CB0-0x00658D08 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00496810; retained seed git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00496810.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
// TRIED: same shape as 0x00484AC0 (PickTech's ctor) - placement-new for the 7 subobjects (GraphicWin base, FlatButton x2, Spot, ListBox(1), Buffer x2, in disassembly order) plus the 2 vtable stores reproduce the call sequence but not the opaque scaffold's missing SEH-unwind prologue (see 0x00484AC0's note for why).
QuayleWin::QuayleWin() {}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00496710 ??1QuayleWin@@QAE@XZ 0x00496710-0x00496801;0x00658C40-0x00658CA8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00496710; retained seed src/recovered/units/00496710.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
// kind game
// compile_context src/quaylewin.cpp
// symbol    ??1QuayleWin@@UAE@XZ

// ORIGINAL: 0x004951A0 ?init@QuayleWin@@QAEHXZ 0x004951A0-0x004956C1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004951A0; retained seed git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004951a0.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
// TRIED: field-by-offset access on `char *self`, per the scaffold's own hint, plus GraphicWin::init/Buffer::load_pcx/BaseButton::init argument order re-derived from the disassembly's push order (Ghidra's own decompilation independently agreed on all three). Reaches MISMATCH #1 (`push esi` vs a `mov`, one instruction into the __thiscall prologue's register-save order) - not chased further, a register-allocation detail this pass's budget does not cover.
static inline void quayle_offset_rect(RECT *rect, int dx, int dy) {
  rect->left += dx;
  rect->right += dx;
  rect->top += dy;
  rect->bottom += dy;
}

static inline void quayle_place_rect(RECT *rect, int x, int y, int width, int height) {
  rect->left = x;
  rect->top = y;
  rect->right = x + width;
  rect->bottom = y + height;
}

int QuayleWin::init() {
  field_A1C_ = 0x320;
  field_A20_ = 0x258;
  monitor_mode_ = ReportIfGlobal.field_1E230_;
  spot_.init(0x20);

  if (monitor_mode_ == 0) {
    field_A14_ = 0;
    field_A18_ = 0;
    quayle_place_rect(&window_rect_, 0, 0, field_A1C_, field_A20_);
    GraphicWin::init(0, 0, 0x320, 0x258, QuayleStandardCaption, 0x1000000, 0, 0, 0);
  } else {
    field_A14_ = 0x77;
    field_A18_ = 0x50;
    quayle_place_rect(&window_rect_, 0x77, 0x50, field_A1C_, field_A20_);
    GraphicWin::init(0, 0, 0x400, 0x300, QuayleWideCaption, 0x1000000, 0, 0, 0);
  }

  line_advance_ = ReportIfGlobal.field_1E184_->line_advance();
  if (buffer1_.load_pcx("frame.pcx", 0, 0xA, 0xEC) != 0) return 1;
  if (buffer2_.load_pcx("holobook.pcx", 0, 0xA, 0xEC) != 0) return 1;

  set_rect(&illustration_rect_, 0x14C, 0x26, 0x2F0, 0x22E);
  set_rect(&chapter_list_rect_, 0x13A, 0x19, 0x301, 0x235);
  set_rect(&navigation_button_rect_, 0x289, 0x239, 0x301, 0x24D);
  set_rect(&rating_rect_, 0x16C, 0x239, 0x27A, 0x24D);
  set_rect(&title_rect_, 0x2A, 0x22, 0x116, 0x81);
  set_rect(&introduction_rect_, 0x1E, 0x13E, 0x11E, 0x1B0);

  quayle_offset_rect(&illustration_rect_, window_rect_.left, window_rect_.top);
  quayle_offset_rect(&chapter_list_rect_, window_rect_.left, window_rect_.top);
  quayle_offset_rect(&navigation_button_rect_, window_rect_.left, window_rect_.top);
  quayle_offset_rect(&rating_rect_, window_rect_.left, window_rect_.top);
  quayle_offset_rect(&title_rect_, window_rect_.left, window_rect_.top);
  quayle_offset_rect(&introduction_rect_, window_rect_.left, window_rect_.top);

  StringTemp[0] = 0;
  strcat(StringTemp, label_get(0xA64 / 4));
  FlatButton *button = &chapter_button_;
  button->init(StringTemp, 999, navigation_button_rect_.left, navigation_button_rect_.top,
                       navigation_button_rect_.right - navigation_button_rect_.left,
                       navigation_button_rect_.bottom - navigation_button_rect_.top,
                       this, 0);
  button->set_border_styles(&InterfaceBoxStyles[47],
                                    &InterfaceBoxStyles[46],
                                    &InterfaceBoxStyles[45]);
  button->Buffer::set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
  button->redraw();
  button->hide();

  StringTemp[0] = 0;
  strcat(StringTemp, label_get(0xB60 / 4));
  button = &pages_button_;
  button->init(StringTemp, 999, navigation_button_rect_.left, navigation_button_rect_.top,
                     navigation_button_rect_.right - navigation_button_rect_.left,
                     navigation_button_rect_.bottom - navigation_button_rect_.top, this, 0);
  button->set_border_styles(&InterfaceBoxStyles[47],
                                 &InterfaceBoxStyles[46],
                                 &InterfaceBoxStyles[45]);
  button->Buffer::set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
  button->redraw();
  button->hide();
  fill(0);
  return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004964A0 ?close@QuayleWin@@QAEXXZ 0x004964A0-0x004964C1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004964A0; retained seed src/recovered/004964a0.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
void QuayleWin::close() {
  list_box_.close();
  release_modal();
  GraphicWin::close();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004964D0 ?on_left_click@QuayleWin@@QAEXHH@Z 0x004964D0-0x00496533 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004964D0; retained seed src/recovered/004964d0.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
// symbol    ?on_left_click@QuayleWin@@UAEXHH@Z
void QuayleWin::on_left_click(int x, int y) {
  const unsigned int faction_bit = 1u << ConsoleGlobal.faction_id_;
  if ((NetDaemonNet.primary_state_.trailing_words_[6] & faction_bit) != 0) return;
  if (NetDaemonNet.field_1BB4_ != 0) return;
  if (NetDaemonNet.primary_state_.leading_words_[2] != 0) return;
  if (spot_.check(x, y, &y, &x) + 1 != 0) field_2144_ = y;
  redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00496540 ?on_selected@QuayleWin@@QAEXHH@Z 0x00496540-0x00496558 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00496540; retained seed src/recovered/00496540.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
// symbol    ?on_selected@QuayleWin@@UAEXHH@Z
void QuayleWin::on_selected(int selection, int) {
  field_2144_ = selection;
  redraw();
}

// ORIGINAL: 0x00496560 ?on_button_clicked@QuayleWin@@QAEXH@Z 0x00496560-0x004965B1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00496560; retained seed git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/00496560.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/00496560.cpp"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?on_button_clicked@QuayleWin@@UAEXH@Z
void QuayleWin::on_button_clicked(int button_id) {
  if (button_id == 999) {
    if (field_2144_ + 1 != 0) {
      field_2144_ = -1;
      redraw();
      return;
    }
    close();
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004965C0 ?on_key_click@QuayleWin@@QAEHHH@Z 0x004965C0-0x0049660C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004965C0; retained seed src/recovered/units/004965c0.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
// symbol    ?on_key_click@QuayleWin@@UAEHHH@Z
int QuayleWin::on_key_click(int, int) {
  if (field_2144_ + 1 != 0) {
    field_2144_ = -1;
    redraw();
  } else {
    close();
  }
  return 1;
}


// ORIGINAL: 0x004956D0 ?on_redraw@QuayleWin@@QAEXXZ 0x004956D0-0x0049649D;0x00658D10-0x00658D4F
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004956D0; retained seed git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004956d0.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
// TRIED: local StringStruct/Buffer objects reproduce the ctor/dtor calls but MSVC did not regenerate the original's SEH/EH unwind frame (push -1; push handler; fs:[0] chain) from source-level RAII alone here; body is a faithful, compiling transcription (MISMATCH #0, prologue only) - see call order, field offsets and vtable-adjustor reads below for the parts that are semantically verified against raw disassembly.
// symbol    ?on_redraw@QuayleWin@@UAEXXZ
// WORKAROUND(inline): the image expands the RECT form of Spot::add
// (0x0047A6F0, emitted out of line by MultiWin) at 0x00495FF3.
static inline int quayle_spot_add(Spot *spot, int position, int type, RECT *rect) {
  return spot->add(position, type, rect->left, rect->top, rect->right - rect->left,
                   rect->bottom - rect->top);
}

void QuayleWin::on_redraw() {
  StringList strings;
  int faction = ConsoleGlobal.faction_id_;
  int page;
  char number[0x50];
  char temporary[0x100];
  Buffer local_buffer;

  set_font(ReportIfGlobal.field_1E1AC_, 0, 0, 0);
  set_text_color(0x9B, -1, 1, 1);
  restore_clip();
  spot_.kill_type(0);
  if (monitor_mode_) {
    buffer1_.copy(this, 0, 0, 0, 0, 0x400, 0x300);
  }
  buffer2_.copy(this, 0, 0, window_rect_.left, window_rect_.top, 0x320, 0x258);
  set_clip(&window_rect_);
  set_clip(&title_rect_);
  parse_it("Holobook", "BOOKTITLE");
  wrap_cent(StringTemp, title_rect_.left, title_rect_.top,
            rect_width(&title_rect_));
  set_font(ReportIfGlobal.field_1E1B0_, 0, 0, 0);
  set_text_color(0x63, -1, 1, 1);

  if (field_2144_ + 1 != 0) {
    list_box_.close();
    set_clip(&introduction_rect_);
    if (Language == 0) {
      parse_it("Holobook", "BOOKINTRO");
      wrap_cent(StringTemp, introduction_rect_.left, introduction_rect_.top,
                rect_width(&introduction_rect_));
    } else {
      temporary[0] = 0;
      strcat(temporary, "BOOKLIST");
      _itoa(field_2144_, number, 10);
      strcat(temporary, number);
      parse_says(0, get_title(faction), Players[faction].is_leader_female, -1);
      parse_says(1, get_name(faction), Players[faction].is_leader_female, -1);
      parse_it("Holobook", temporary);
      wrap_cent(StringTemp, introduction_rect_.left, introduction_rect_.top,
                rect_width(&introduction_rect_));
    }
    restore_clip();
    StringTemp[0] = 0;
    strcat(StringTemp, "artbox");
    _itoa(field_2144_, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, ".pcx");
    local_buffer.load_pcx(StringTemp, 0, 10, 0xEC);
    local_buffer.draw(this, 9, illustration_rect_.left, illustration_rect_.top);
    if (field_2144_ == 1) {
      set_clip(&illustration_rect_);
      quayle_place_rect(&text_scratch_rect_, window_rect_.left + 0x15B,
                        window_rect_.top + 0x1F6, 0x187, 0x26);
      StringTemp[0] = 0;
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      strcat(StringTemp, Players[faction].title_leader);
      strcat(StringTemp, BufferJoinSpace);
      strcat(StringTemp, get_name(faction));
      set_font(ReportIfGlobal.field_1E1AC_, 0, 0, 0);
      set_text_color(0x45, -1, 1, 1);
      write_cent_l(StringTemp, &text_scratch_rect_);
      restore_clip();
    } else if (field_2144_ == 2) {
      set_clip(&illustration_rect_);
      quayle_place_rect(&text_scratch_rect_, window_rect_.left + 0x136,
                        window_rect_.top + 0x1A5, 0x1CB, 0x32);
      StringTemp[0] = 0;
      strcat(StringTemp, get_title(faction));
      strcat(StringTemp, BufferJoinSpace);
      strcat(StringTemp, get_name(faction));
      set_font(ReportIfGlobal.field_1E1B8_, 0, 0, 0);
      set_text_color(0x45, -1, 1, 1);
      write_cent_l(StringTemp, &text_scratch_rect_);
      restore_clip();
    } else if (field_2144_ == 8) {
      set_clip(&illustration_rect_);
      quayle_place_rect(&text_scratch_rect_, window_rect_.left + 0x13C,
                        window_rect_.top + 0x1DF, 0x1C1, 0x54);
      StringTemp[0] = 0;
      strcat(StringTemp, get_title(faction));
      strcat(StringTemp, BufferJoinSpace);
      strcat(StringTemp, get_name(faction));
      set_font(ReportIfGlobal.field_1E1B8_, 0, 0, 0);
      set_text_color(0x5E, -1, 1, 1);
      wrap_cent(StringTemp, &text_scratch_rect_);
      restore_clip();
    }
    chapter_button_.hide();
    pages_button_.show(0);
  } else {
    set_clip(&introduction_rect_);
    parse_it("Holobook", "BOOKINTRO");
    wrap_cent(StringTemp, introduction_rect_.left, introduction_rect_.top,
              rect_width(&introduction_rect_));
    if (Language == 0) {
      restore_clip();
      Font *font = ReportIfGlobal.field_1E1A8_;
      set_font(font, font, font, 0);
      set_text_color(0x63, -1, 1, 1);
      set_text_color2(0x9B, -1, 1, 1);
      set_text_color3(0x9B, -1, 1, 1);
      set_rect(&text_scratch_rect_, chapter_list_rect_.left, chapter_list_rect_.bottom + 6,
               chapter_list_rect_.right, chapter_list_rect_.bottom + 10);
      for (page = 0; page <= field_2148_; ++page) {
        temporary[0] = 0;
        strcat(temporary, "BOOKLIST");
        _itoa(page, number, 10);
        strcat(temporary, number);
        parse_says(0, get_title(faction), Players[faction].is_leader_female, -1);
        parse_says(1, get_name(faction), Players[faction].is_leader_female, -1);
        parse_it("Holobook", temporary);
        if (page == field_2148_) {
          temporary[0] = 0;
          strcpy(temporary, StringTemp);
          StringTemp[0] = 0;
          strcat(StringTemp, "[");
          strcat(StringTemp, temporary);
          strcat(StringTemp, "]");
        }
        field_A14_ = chapter_list_rect_.left;
        field_A18_ = text_scratch_rect_.top;
        field_A1C_ = rect_width(&chapter_list_rect_);
        field_A20_ = wrap_height(StringTemp, field_A1C_);
        quayle_place_rect(&text_scratch_rect_, field_A14_, field_A18_ - field_A20_ - 6, field_A1C_, field_A20_);
        quayle_spot_add(&spot_, page, 0, &text_scratch_rect_);
        wrap(StringTemp, &text_scratch_rect_);
      }
    } else {
      list_box_.init(&chapter_list_rect_, 0);
      list_box_.set_dialog_font(ReportIfGlobal.field_1E1A8_,
                                ReportIfGlobal.field_1E1A8_,
                                ReportIfGlobal.field_1E1A8_);
      list_box_.set_dialog_text_color(0x63, -1, 1, 1);
      list_box_.set_report_text_color(0x9B);
      for (page = field_2148_; page >= 0; --page) {
        temporary[0] = 0;
        strcat(temporary, "BOOKLIST");
        _itoa(page, number, 10);
        strcat(temporary, number);
        parse_says(0, get_title(faction), Players[faction].is_leader_female, -1);
        parse_says(1, get_name(faction), Players[faction].is_leader_female, -1);
        parse_it("Holobook", temporary);
        list_box_.item(StringTemp, page);
      }
      ListBox *list = &list_box_;
      list_box_.field_2C_ = 0;
      list_box_.field_14_ = static_cast<uint32_t>(-1);
      list_box_.field_8_ = 1;
      list->Dialog::field_B4_ = 0;
      list->ListBox::attach(this, list->Dialog::window_x_, list->Dialog::window_y_, 0x80020);
      list_box_.set_selected_pos(0, 1);
      list_box_.show(0);
    }
    field_2144_ = -1;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x424));
    strcat(StringTemp, ":");
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    const int rating = get_rating(ConsoleGlobal.faction_id_,
                                  PlayersData[ConsoleGlobal.faction_id_].unk_107);
    _itoa(rating, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, "%");
    set_font(ReportIfGlobal.field_1E1B0_, 0, 0, 0);
    set_text_color(0x9B, -1, 1, 1);
    restore_clip();
    write_l(StringTemp, &rating_rect_);
    pages_button_.hide();
    chapter_button_.show(0);
  }
}

// Promoted historical seed; its superseded source path is Git provenance only.
// ORIGINAL: 0x00496610 sub_496610 0x00496610-0x00496668 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recover the direct-page entry through QuayleWinGlobal, its existing page/chapter members and inherited Win modal virtual; preserve the original guards and clamp without fixed-address globals or manual virtual dispatch.","evidence":["Pinned original 00496610 reads page at [ebp+8], tests multiplayer DWORD 0093F660 and calls QuayleWin::init at 004951A0; nonzero init skips state/modal writes.","Source-owned QuayleWinGlobal at 00872CB0: original page 00874DF8 and chapter 00874DF4 are its +2148/+2144 members, exposed by typed setters.","Original indirect call at 00496660 uses vtable +E4, Win::set_modal slot 57, with three zero arguments and ignored result.","Clamp order is page >=25 to24, otherwise page <-1 to-1. Historical seed src/recovered/units/00496610.cpp in Git preserves the prior unit-level mismatch and scaffold experiments; they are not current matching claims."]}
// symbol    _sub_496610
// size      88 bytes
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004951A0
// indirect  0x00496660
extern "C" void __cdecl sub_496610(int page) {
    if (IsMultiplayerNet == 0) {
        if (QuayleWinGlobal.init() == 0) {
            if (page >= 0x19) {
                page = 0x18;
            } else if (page < -1) {
                page = -1;
            }
            QuayleWinGlobal.set_page(page);
            QuayleWinGlobal.set_chapter(-1);
            Win &modal_window = QuayleWinGlobal;
            modal_window.set_modal(0, 0, 0);
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: MISMATCH #15 add/mov, table-index lea chain differs from the do/while division idiom form used
// ORIGINAL: 0x00496670 ?quayle@@YAXH@Z 0x00496670-0x0049670F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00496670; retained seed src/unrecovered/00496670.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
#pragma optimize("y", off)
void __cdecl quayle(int faction_id) {
  int rating = get_rating(faction_id, PlayersData[faction_id].unk_107);
  int rank = 0;
  for (int step = 2; step - 2 < 0x18; ++step) {
    if (rating < (step * step) / 2) break;
    ++rank;
  }
  if (IsMultiplayerNet) return;
  if (QuayleWinGlobal.init() != 0) return;
  if (rank >= 0x19) {
    rank = 0x18;
  } else if (rank < -1) {
    rank = -1;
  }
  QuayleWinGlobal.set_page(rank);
  QuayleWinGlobal.set_chapter(-1);
  Win &modal_window = QuayleWinGlobal;
  modal_window.set_modal(0, 0, 0);
}
#pragma optimize("", on)
