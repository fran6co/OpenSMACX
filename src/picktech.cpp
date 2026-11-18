// ORIGINAL: 0x00488740 ??_GPickTech@@WEEE@AEPAXI@Z 0x00488740-0x0048874B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original ??_GPickTech@@WEEE@AEPAXI@Z at 0x00488740 is an 11-byte secondary scalar deleting adjustor; it subtracts 0x444 from ECX and tail-jumps to the complete-object PickTech scalar deleting entry without touching stack or flags.", "Historical seed correction: the earlier `??3` deallocation interpretation was refuted; the original is a `??_G` scalar-deleting adjustor. `WEEE` demangles to adjustor{1092}, equal to 0x444.", "Pinned PickTech constructor at 0x00484b43 stores Buffer-view vftable 0x0066cfc4 at this+0x444; the source owner is the typed PickTech GraphicWin/Buffer multiple-inheritance view.", "Current src/picktech.cpp COFF Buffer-view vftable first slot selects generated ??_EPickTech@@WEEE@AEPAXI@Z; its body performs the same 0x444 adjustment and preserves the flag argument.", "Current PickTech COFF weak external ??_EPickTech@@UAEPAXI@Z records search mode 2 (library search) and fallback target ??_GPickTech@@UAEPAXI@Z; this linked image resolves both primary labels to the same physical class deleting body. The object fallback and linked address are per-build evidence, not a general E/G equivalence or owner-selection rule.", "That measured current primary body calls PickTech::~PickTech, tests flags bit 0, calls operator delete only when bit 0 is set, returns the saved object pointer, and uses ret 4; this is the per-class measured E-to-G scalar path, not a general vector/scalar equivalence.", "The removed transitional C++ names were ?adjust_this_pick_tech@@YIPAXPAX0I@Z and ?scalar_delete_pick_tech@@YIPAXPAX0I@Z; no callers exist outside deleting_thunks.cpp/.h, and the current generated Buffer vftable uses the class-owned E adjustor instead.", "Historical seed metadata noted flags hidden;sp_ready;purged_ok, zero direct callers, and no direct call targets. These remain historical observations only; they do not establish virtual-dispatch completeness or acceptance.", "The old secondary record had no BYTE_EXACT or SEMANTIC claim. Its original identity, full span, kind and parsed history are moved unchanged; no matching claim is granted."], "reason": "Bind the pinned secondary scalar deleting identity for PickTech to its measured genuine Buffer-view generated adjustor. Keep the original G identity and point its current symbol fact at the owner-emitted E adjustor; the per-object weak fallback and bit-0 path above establish why this exact current implementation preserves the original operation."}
// symbol    ??_EPickTech@@WEEE@AEPAXI@Z
// kind      game
// ORIGINAL: 0x00488660 ??_GPickTech@@UAEPAXI@Z 0x00488660-0x00488681 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00488660-0x00488681", "Pinned class constructor ??0PickTech@@QAE@XZ at 0x00484AC0 stores original class/view table VA 0x66cfcc at 0x00484B3D (mov dword ptr [esi], 0x66cfcc); the current genuine owner vftable ??_7PickTech@@6BWin@@@ selects generated ??_GPickTech@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EPickTech@@UAEPAXI@Z (mode 2).", "Pinned ??_GPickTech@@UAEPAXI@Z at 0x00488660 calls the source-mapped ??1PickTech@@QAE@XZ at 0x00488690, then tests byte ptr [ebp + 8], 1; je 0x48867a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/picktech.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original PickTech scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GPickTech@@UAEPAXI@Z
// kind      game
// WORKAROUND(noinline): draw_tech's StringList local calls
// StringStruct::StringStruct (0x00401000) out of line (0x00485361), giving
// its three-state EH layout; see stringstruct.h.
#define FONT_LINE_ADVANCE_INLINE
#define STRINGSTRUCT_CTOR_OUT_OF_LINE
// WORKAROUND(noinline): draw_tech's explicit clear calls StringStruct::remove_all
// (0x00402970) out of line (0x00485403) while its closing destructor walk
// expands the same body inline (0x00485EB0); see stringstruct.h.
#define STRINGSTRUCT_REMOVE_ALL_CALLED
#define WIN_EXPAND_RECT_INLINE  // stand-in: see expand_rect in win.h
#include "stdafx.h"
#define POPUP_WRAPPERS_INLINE  // stand-in: see the BasePop wrappers in basepop.h
#include "interface_fonts.h"
#include "picktech.h"
#include "palette.h"
#include "interface_palette.h"
#include "technology.h"
#include "strings.h"
#include "stringstruct.h"
#include "datalink.h"
#include "base.h"
#include "faction.h"
#include "veh.h"
#include "alpha.h"
#include "messagewin.h"
#include "mapwin.h"
#include "win.h"
#include "terraforming.h"
#include "popup.h"
#include "random.h"
#include "game.h"
#include "console.h"
#include "fx.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "win_pointer.h"

// Shared punctuation objects the image pushes instead of unit literals.
extern char SocialWinCloseParenText[];
extern char SocialWinOpenParenText[];
void __cdecl help_any();
int __cdecl tut_check2(int);
extern char *TutorialPopupCaption;
extern char SocialWinColonSpaceText[];
extern Sprite g_TECH_ICON_SPRITES[MaxTechnologyNum];

// Native484AC0 installs the primary table66CFCC and secondary table66CFC4.
// C++ owns the entries; the primary complete original extent remains unproved.
// RECOVERY_GENERATED: v1 vtable 0x0066CFCC ? ??_7PickTech@@6BWin@@@ 0x00484AC0
// RECOVERY_GENERATED: v1 vtable 0x0066CFC4 0x0066CFCC ??_7PickTech@@6BBuffer@@@ 0x00484AC0

// Constructor evidence: the body calls GraphicWin::GraphicWin, constructs
// Font at +BBC,+BE4,+C0C,+C34 and zeroes +A24,+A28,+A38,+A3C. Real C++ base
// and member construction supplies those lifetimes; no placement-new or
// explicit vtable stores are retained.
// ORIGINAL: 0x00484AC0 ??0PickTech@@QAE@XZ 0x00484AC0-0x00484B5C;0x00657630-0x0065766C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00484AC0; retired seed git:288b790f:src/unrecovered/00484ac0.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ??0PickTech@@QAE@XZ
// TRIED: placement-new base/Font subobjects w/o the fs:[0] SEH unwind frame (no local of a destructor-modelling class declared)
PickTech::PickTech()
    : GraphicWin() {
    // Native484B25..37 performs these stores after all four Font constructors.
    column_count_ = 0;
    row_count_ = 0;
    selected_index_ = 0;
    selection_active_ = 0;
    // Only the four zero stores are native ctor evidence; initialization of
    // the intervening fields and technology list still needs exact proof.
}

// Native destruction calls the GraphicWin close path, then destroys four
// Font members in reverse construction order and the GraphicWin base.
// ORIGINAL: 0x00484B60 ?close@PickTech@@QAEXXZ 0x00484B60-0x00484B7F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00484B60; source:src/leaf_recoveries.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?close@PickTech@@QAEXXZ
void PickTech::close() {
    column_count_ = 0;
    row_count_ = 0;
    selected_index_ = 0;
    selection_active_ = 0;
    GraphicWin::close();
}

// ORIGINAL: 0x00488690 ??1PickTech@@QAE@XZ 0x00488690-0x0048873F;0x00657C20-0x00657C6A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00488690; retired seed git:288b790f:src/unrecovered/00488690.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// body src/picktech.h
// symbol    ??1PickTech@@UAE@XZ
// TRIED: same class as 0x00484AC0 (PickTech's ctor, already landed MISMATCH #1) - the scaffold's PickTech is opaque (no real GraphicWin/Font subobjects), so explicit `->close()`/`->~Font()`/`->~GraphicWin()` calls reproduce the CALL SEQUENCE but not the `push -1/push handler/mov fs:[0]` SEH-unwind prologue, which only a compiler-recognised subobject with a non-trivial destructor would trigger. Field-store order (vtables first, then the four zeroed fields, matches disassembly) is otherwise exact.

// ORIGINAL: 0x00485FC0 ?on_mouse_move@PickTech@@QAEXHH@Z 0x00485FC0-0x004860A6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00485FC0; retired seed git:288b790f:src/unrecovered/00485fc0.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?on_mouse_move@PickTech@@UAEXHH@Z
// TRIED: flipping `field*(field+1) <= a1` to `a1 >= field*(field+1)` fixed two jge/jle polarity flips, reaching 0.987 mnemonic similarity; the one remaining `a2 - 0x1d` compiles to `add eax,-0x1d` no matter how the subtraction is spelled (single temp, split statement, reordered operands all tried).
void PickTech::on_mouse_move(int x, int y) {
    y -= 0x1D;
    if (x < 0 || y < 0 ||
        x >= column_count_ * (cell_width_ + 1) ||
        y >= row_count_ * (cell_height_ + 1)) {
        const int current = selected_index_;
        if (current >= 0) {
            draw_item(current, 0);
            soft_update();
        }
        selection_active_ = 0;
        selected_index_ = -1;
    } else {
        const int index = (y / (cell_height_ + 1)) * column_count_
                        + x / (cell_width_ + 1);
        if (index != selected_index_ || selection_active_ == 0) {
            const int active = draw_item(index, 1);
            selection_active_ = active;
            const int previous = selected_index_;
            if (previous >= 0 && previous != index) draw_item(previous, 0);
            selected_index_ = index;
            on_update();
        }
    }
}

// ORIGINAL: 0x004863F0 ?draw_item@PickTech@@QAEHHH@Z 0x004863F0-0x0048650C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004863F0; retired seed git:288b790f:src/unrecovered/004863f0.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?draw_item@PickTech@@QAEHHH@Z
// TRIED: a `Rect4{left,top,right,bottom}` local (matches the RECT the original builds on the stack field-by-field) and the `-(int)(a2!=0)&2` neg/sbb/and idiom for the sprite-params table offset, per the lever for a `guard ? obj : 0` shape. Landed at 0.71 similarity: the original keeps a stack frame (`push ebp; mov ebp,esp`) that this body's simpler local set let VC6 omit, the same shape seen on 0x00458DC0/0x004863F0-sized leaf-ish functions this batch.
int PickTech::draw_item(int item, int selected) {
    if (item >= item_count_) return 0;

    RECT rect;
    rect.right = cell_width_;
    rect.bottom = cell_height_;
    rect.left = (item % column_count_) * (cell_width_ + 1) + 1;
    rect.top = (item / column_count_) * (cell_height_ + 1) + 0x1E;
    rect.right += rect.left;
    rect.bottom += rect.top;

    Buffer::set_clip(&rect);
    rect.right -= 3;
    rect.bottom -= 3;
    rect.left = max(rect.left, 3);
    rect.right = min(rect.right, Buffer::pixel_width() - 3);
    rect.top = max(rect.top, 0x1D);
    rect.bottom = min(rect.bottom, Buffer::pixel_height() - 3);
    Buffer::box_sprite(&rect, &InterfaceBoxStyles[selected ? 2 : 0]);
    const int result = draw_tech(technology_ids_[item], &rect, selected);
    Buffer::restore_clip();
    return result;
}

// This is the owned member counterpart of the current stringstruct.cpp
// receiver-offset transcription. All five addresses are established by the
// seed accesses; Buffer is GraphicWin's real secondary base, not a byte view.
// ORIGINAL: 0x00484B80 ?UNK1@PickTech@@QAEXPAD@Z 0x00484B80-0x00484C5D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00484B80; source:src/leaf_recoveries.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?UNK1@PickTech@@QAEXPAD@Z
void PickTech::UNK1(char *text) {
    int line_height;
    if (page_state_ == 0) {
        Buffer::set_font(&header_font_, &header_font_, &header_font_, 0);
        line_height = header_font_.line_advance();
        page_state_ = 1;
    } else {
        Buffer::set_font(&title_font_, &body_font_primary_,
                         &body_font_secondary_, 0);
        line_height = title_font_.line_advance();
    }
    if (page_bottom_ - line_height >= page_pen_y_) {
        page_bottom_ -= Buffer::wrap_height(text, page_width_ - 2);
        Buffer::wrap_cent(text, page_x_ + 1, page_bottom_, page_width_ - 2);
    }
}

// ORIGINAL: 0x004860B0 ?on_left_click@PickTech@@QAEXHH@Z 0x004860B0-0x00486109 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004860B0; retired seed git:288b790f:src/recovered/units/004860b0.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?on_left_click@PickTech@@UAEXHH@Z
void PickTech::on_left_click(int x, int y) {
    on_mouse_move(x, y);
    if (selection_active_ != 0) {
        if (WinPointerOwner3 == this) {
            WinPointerOwner3 = 0;
            on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == this) WinPointerOwner4 = 0;
        release_modal();
    }
}

// ORIGINAL: 0x00486110 ?on_right_click@PickTech@@QAEXHH@Z 0x00486110-0x0048616A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00486110; retired seed git:288b790f:src/recovered/units/00486110.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?on_right_click@PickTech@@UAEXHH@Z
void PickTech::on_right_click(int x, int y) {
    on_mouse_move(x, y);
    if (selection_active_ != 0) {
        hide();
        do_all_draws();
        const int tech_id = technology_ids_[selected_index_];
        DatalinkMain.exec(0x0E, tech_id);
        set_modal(0, 0, 0);
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00486170 0x004863B0 0x004863C4 0x004861B7
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00486170 0x004863C4 0x004863E4 0x004861B1
// ORIGINAL: 0x00486170 ?on_key_click@PickTech@@QAEHHH@Z 0x00486170-0x004863AD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00486170; retired seed git:288b790f:src/unrecovered/00486170.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?on_key_click@PickTech@@UAEHHH@Z
int PickTech::on_key_click(int character, int key) {
    int i;
    const int previous = selected_index_;
    int column = previous % column_count_;
    int row = previous / column_count_;

    switch (key) {
    case 8:
    case 0x25:
    case 0x64:
        --column;
        if (column < 0) column = column_count_ - 1;
        break;
    case 9:
    case 0x27:
    case 0x66:
        column = (column + 1) % column_count_;
        break;
    case 0x26:
    case 0x68:
        --row;
        if (row < 0) row = row_count_ - 1;
        break;
    case 0x28:
    case 0x62:
        row = (row + 1) % row_count_;
        break;
    case 0x0D:
    case 0x20:
    case 0x48:
    case 0x70:
    case 0x1000D: {
        if (selected_index_ < 0 || selected_index_ >= item_count_) {
            if (selected_index_ >= 0) draw_item(selected_index_, 0);
            selected_index_ = 0;
            draw_item(0, 1);
            on_update();
        }
        const int tech_id = technology_ids_[selected_index_];
        if (tech_id < 0) return 1;
        if (key == 0x70 || key == 0x48) {
            help_any();
            return 1;
        }
        if (WinPointerOwner3 == this) {
            WinPointerOwner3 = 0;
            on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == this) WinPointerOwner4 = 0;
        release_modal();
        return 1;
    }
    default:
        break;
    }

    int selected = column_count_ * row + column;
    if (previous < 0) selected_index_ = 0;
    if (character != 0) {
        for (i = 0; i < item_count_; ++i) {
            const int tech = technology_ids_[i];
            StringTemp[0] = '\0';
            say_tech(StringTemp, tech, 0);
            if (toupper(character) == StringTemp[0]) {
                selected = i;
                break;
            }
        }
    }
    if (selected != selected_index_) {
        selection_active_ = draw_item(selected, 1);
        if (selected_index_ >= 0) draw_item(selected_index_, 0);
        selected_index_ = selected;
        on_update();
    }
    return 1;
}

static const char *picktech_name(LPSTR address) {
    return StringTable.get(address);
}

static void picktech_add_label(int index) {
    strcat(StringTemp, label_get(index));
}

// ORIGINAL: 0x00484C60 ?draw_tech@PickTech@@QAEHHPAURECT@@H@Z 0x00484C60-0x00485FBD;0x0065766C-0x006576A0
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00484C60; retired seed git:288b790f:src/unrecovered/00484c60.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?draw_tech@PickTech@@QAEHHPAUtagRECT@@H@Z
// TRIED: full transcription of all branches/tables/calls (~95% of instructions); approximated the StringStruct/StringList entry-teardown loop (~0x00485EB0-0x00485F9C, ~9% of the function) as a simplified linked-list walk instead of the adjustor-thunk virtual dispatch the disassembly performs, since the scaffold's StringStruct/StringList carry no vtable to reproduce it exactly; find_font/mandate_color call sites re-derived from raw push/pop counts against the declared 2-arg/1-arg signatures rather than Ghidra's (wrong) 3-arg/4-arg attribution of leftover stack pushes.
int PickTech::draw_tech(int tech, RECT *rect, int) {
    int i;
    char number_buffer[80];
    // The original title copy and later truncated labels share EBP-0x198.
    // The numeric scratch starts at EBP-0x1e8, with the next proven local at
    // EBP-0x198 leaving an 80-byte address window; its exact original bound
    // within that window is unresolved.
    char text_buffer[256];
    char secondary_buffer[80];
    if (tech < 0) return 0;

    page_state_ = 0;
    Buffer::set_text_color(MessageWinTextColor, 0, 1, 1);
    Buffer::set_text_color2(MessageWinTextColor, 0, 1, 1);
    page_width_ = rect->right - rect->left;
    title_baseline_ = title_font_.line_advance();
    page_x_ = rect->left;
    page_pen_y_ = row_pitch_ + rect->top;
    page_bottom_ = rect->bottom - row_pitch_ / 2;

    StringTemp[0] = '\0';
    if (item_count_ <= 8) {
        if (WinScreenWidth != 800)
            Buffer::set_font(find_font(0x14, 1), find_font(0x14, 1), find_font(0x14, 2), 0);
        else
            Buffer::set_font(find_font(0x12, 1), find_font(0x12, 1), find_font(0x12, 2), 0);
    } else {
        Buffer::set_font(find_font(0x0C, 1), find_font(0x0C, 1), find_font(0x0C, 2), 0);
    }
    say_tech(StringTemp, tech, 0);
    if (tech == TECH_TRANT) {
        strcat(StringTemp, BufferJoinSpace);
        _itoa(PlayersData[faction_id_].theory_of_everything + 1,
              number_buffer, 10);
        strcat(StringTemp, number_buffer);
    }
    strcpy(text_buffer, StringTemp);
    if (Buffer::text_width(StringTemp) > page_width_ - 2 &&
        strchr(StringTemp, ' ') == 0) {
        while (Buffer::text_width(text_buffer) > page_width_ - 2 && strlen(text_buffer) != 0)
            text_buffer[strlen(text_buffer) - 1] = '\0';
        strcat(text_buffer, BufferJoinSpace);
        const size_t suffix_offset = strlen(text_buffer);
        strcat(text_buffer, StringTemp + suffix_offset - 1);
    }

    const int category = tech_category(tech);
    const int requirement_count = tech_recurse(tech, 0);
    if (item_count_ > 8)
        Buffer::set_text_color(mandate_color(category), 0, 1, 1);
    if (!(item_count_ > 8)) {
        page_pen_y_ = Buffer::wrap_cent(text_buffer, page_x_ + 2,
                                      page_pen_y_, page_width_ - 6);
    } else {
        const int wrapped_height = Buffer::wrap_height(text_buffer, rect->right - rect->left - 4);
        page_pen_y_ = Buffer::wrap_cent(text_buffer, rect->left + 2,
            (rect->bottom - rect->top - wrapped_height) / 2 + rect->top,
            rect->right - rect->left - 4);
    }
    if (item_count_ > 8) return 1;

    Buffer::set_font(&title_font_, &body_font_primary_, &body_font_secondary_, 0);
    Buffer::set_text_color(mandate_color(category), 0, 1, 1);
    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(Mandate[category].name_caps));
    strcat(StringTemp, BufferJoinSpace);
    _itoa(requirement_count, number_buffer, 10);
    strcat(StringTemp, number_buffer);
    page_pen_y_ = Buffer::wrap_cent(StringTemp, page_x_ + 1, page_pen_y_,
                                    page_width_ - 2);
    Buffer::set_text_color(MessageWinTextColor, 0, 1, 1);
    Buffer::set_text_color2(0x5F, 0, 1, 1);

    int metric;
    if (WinScreenWidth == 800) {
        if (cell_width_ >= 0xF0) metric = -0x0C;
        else metric = -0x0E;
    } else if (cell_width_ >= 0xB4) {
        metric = -0x0A;
    } else {
        metric = cell_width_ > 0x8C ? -0x0C : -0x0E;
    }

    if (tech < MaxTechnologyNum) {
        page_pen_y_ += row_pitch_;
        if (MapWinCaptionScale > 0) {
            SpriteScaleNumerator = MapWinCaptionScale;
            SpriteScaleDenominator = MapWinCaptionDivisor;
        } else {
            SpriteScaleDenominator = 0x10;
            SpriteScaleNumerator = metric + 0x10;
        }
        const int icon_width = MapWinCaptionScale > 0
            ? MapWinCaptionScale * 0xD7 / MapWinCaptionDivisor
            : ((metric + 0x10) * 0xD7) / 0x10;
        const int icon_color = TextColourTable[162 + tech_category(tech) * 2];
        g_TECH_ICON_SPRITES[tech].draw_mono(this,
            (page_width_ - icon_width) / 2 + rect->left, page_pen_y_, icon_color);
        page_pen_y_ += (MapWinCaptionScale > 0
            ? MapWinCaptionScale * 0xD7 / MapWinCaptionDivisor
            : ((metric + 0x10) * 0xD7) / 0x10) + row_pitch_;
        SpriteScaleNumerator = 1;
        SpriteScaleDenominator = 1;
    }
    if (tech >= 0x61) {
        if (tech >= 0x59) return 1;
        page_pen_y_ += row_pitch_;
        const int fake_vehicle = veh_fake(tech - 0x61, faction_id_);
        const int vehicle_width = MapWinCaptionScale > 0
            ? MapWinCaptionScale * 100 / MapWinCaptionDivisor
            : ((metric * 5 + 0x50) * 20) / 16;
        veh_draw(this, fake_vehicle, (page_width_ - vehicle_width) / 2 + rect->left,
                 page_pen_y_, metric, 0, 0);
        page_pen_y_ += (MapWinCaptionScale > 0
            ? MapWinCaptionScale * 0x4B / MapWinCaptionDivisor
            : ((metric * 5 + 0x50) * 0x0F) / 16) + row_pitch_;
    }
    if (tech >= 0x59) return 1;

    StringList technology_notes;
        StringTemp[0] = '\0';
        strcat(StringTemp, "TECH");
        _itoa(tech, number_buffer, 10);
        strcat(StringTemp, number_buffer);
        const int loaded = technology_notes.load(
            const_cast<char *>("TECHSHORTS.TXT"), StringTemp, 1, 0);
        if (loaded == 0) {
            page_pen_y_ = Buffer::write_strings(&technology_notes, page_x_ + 4,
                                                 page_pen_y_, page_width_ - 8, 0);
            technology_notes.StringStruct::remove_all();
            // Native485408 resets the cursor even when remove_all's head
            // guard returns without clearing it.
            technology_notes.current_position_ = 0;
        }
    page_state_ = 1;

    // Typed RuleTechnology association scan. The native cursor is the
    // preq_tech_2 word in record zero and advances by the 0x2c stride.
    for (i = 0; i < MaxTechnologyNum; ++i) {
        if (Technology[i].preq_tech_1 == tech || Technology[i].preq_tech_2 == tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "[");
            strcat(StringTemp, picktech_name(Technology[i].name));
            strcat(StringTemp, "]");
            UNK1(StringTemp);
        }
    }

    // Facility.free_tech is read for every record; the native cursor runs to
    // the end of Facility and tests Facility[FacilitySPStart] per entry.
    for (i = 1; i < MaxFacilityNum; ++i) {
        if (Facility[i].free_tech == tech && i < FacilitySPStart) {
            StringTemp[0] = '\0';
            if (page_state_ == 0) strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(Facility[i].name));
            strcat(StringTemp, BufferJoinSpace);
            picktech_add_label(0x4dc / 4);
            if (page_state_ == 0) strcat(StringTemp, "}");
            UNK1(StringTemp);
        }
    }

    // Citizen records (not SecretProject records) carry the +8 prerequisite.
    // Native end 0x9460E4 is seven 0x1c-byte rule records, before the next
    // independently owned data object.
    for (i = 0; i < 7; ++i) {
        if (Citizen[i].preq_tech == tech) {
            StringTemp[0] = '\0';
            if (page_state_ == 0) strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(Citizen[i].singular_name));
            if (page_state_ == 0) strcat(StringTemp, "}");
            UNK1(StringTemp);
        }
    }

    for (i = 0; i < MaxAbilityNum; ++i) {
        if (Ability[i].preq_tech == tech) {
            StringTemp[0] = '\0';
            if (page_state_ == 0) strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(Ability[i].name));
            if (page_state_ == 0) strcat(StringTemp, "}");
            UNK1(StringTemp);
        }
    }

    // Native 0x691880 is Terraforming[0].preq_tech; the 0x20-byte cursor
    // visits the 19 non-monolith rules. Caption IDs are the actual parsed
    // Order records at i+4 (land and sea text are separate columns).
    for (int row = 0; row < MaxTerrainNum - 1; ++row) {
        for (int model = 0; model <= 1; ++model) {
            if (tech == Terraforming[row].prerequisite_tech[model] &&
                (model != 1 || Terraforming[row].prerequisite_tech[0] !=
                               Terraforming[row].prerequisite_tech[1])) {
                StringTemp[0] = '\0';
                if (page_state_ == 0) strcat(StringTemp, "{");
                strcat(StringTemp, picktech_name(Order[row + 4].order[model]));
                if (page_state_ == 0) strcat(StringTemp, "}");
                UNK1(StringTemp);
            }
        }
    }

    if (tech == Rules.tech_three_energy_sqr) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "{");
        picktech_add_label(0x460 / 4);
        strcat(StringTemp, "}");
        UNK1(StringTemp);
    }
    if (tech == Rules.tech_three_minerals_sqr) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "{");
        picktech_add_label(0x45c / 4);
        strcat(StringTemp, "}");
        UNK1(StringTemp);
    }
    if (tech == Rules.tech_three_nutrients_sqr) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "{");
        picktech_add_label(0x458 / 4);
        strcat(StringTemp, "}");
        UNK1(StringTemp);
    }

    // Facility.pre_req spans ordinary facilities and their in-place
    // special-project rules records, both represented by RulesFacility.
    for (i = 1; i < MaxFacilityNum; ++i) {
        if (Facility[i].preq_tech == tech) {
            StringTemp[0] = '\0';
            if (i < FacilitySPStart || page_state_ == 0) strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(Facility[i].name));
            if (i < FacilitySPStart) {
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, SocialWinOpenParenText);
                strcat(StringTemp, picktech_name(Facility[i].effect));
                strcat(StringTemp, SocialWinCloseParenText);
            }
            if (i < FacilitySPStart || page_state_ == 0) strcat(StringTemp, "}");
            UNK1(StringTemp);
        }
    }

    // RulesSocialCategory.name[] and preq_tech[] are interleaved in the
    // native cursor but paired within each true 0xD4 RulesSocialCategory.
    for (int category_id = 0; category_id < 4; ++category_id) {
        for (int model = 0; model < 4; ++model) {
            if (SocialCategories[category_id].preq_tech[model] == tech) {
                StringTemp[0] = '\0';
                strcat(StringTemp, "{");
                strcat(StringTemp, picktech_name(SocialCategories[category_id].name[model]));
                strcat(StringTemp, "}");
                UNK1(StringTemp);
            }
        }
    }

    for (int chassis_id = 0; chassis_id < MaxChassisNum; ++chassis_id) {
        if (Chassis[chassis_id].preq_tech == tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(Chassis[chassis_id].offsv_1_name));
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, SocialWinOpenParenText);
            strcat(StringTemp, StringTable.get(Triad[Chassis[chassis_id].triad]));
            strcat(StringTemp, BufferJoinSpace);
            _itoa(static_cast<int>(Chassis[chassis_id].speed),
                  number_buffer, 10);
            strcat(StringTemp, number_buffer);
            strcat(StringTemp, SocialWinCloseParenText);
            strcat(StringTemp, "}");
            UNK1(StringTemp);
        }
    }

    for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
        if (tech == Armor[armor_id].preq_tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(Armor[armor_id].name));
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, SocialWinOpenParenText);
            if (Armor[armor_id].defense_rating < 0) {
                picktech_add_label(0x310 / 4);
            } else {
                _itoa(static_cast<int>(Armor[armor_id].defense_rating), number_buffer, 10);
                strcat(StringTemp, number_buffer);
                strcat(StringTemp, SocialWinCloseParenText);
                strcat(StringTemp, "}");
            }
            UNK1(StringTemp);
        }
    }

    for (int weapon_id = 0; weapon_id < MaxWeaponNum; ++weapon_id) {
        if (Weapon[weapon_id].preq_tech == tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(Weapon[weapon_id].name));
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, SocialWinOpenParenText);
            if (Weapon[weapon_id].offense_rating != 0) {
                if (Weapon[weapon_id].offense_rating < 0) {
                    picktech_add_label(0x310 / 4);
                } else {
                    _itoa(static_cast<int>(Weapon[weapon_id].offense_rating), number_buffer, 10);
                    strcat(StringTemp, number_buffer);
                }
            } else {
                text_buffer[0] = '\0';
                strcat(text_buffer, StringTable.get(Labels.strings_ptr[0x200 / 4]));
                // Native485C81 retains only the label's first byte.
                text_buffer[1] = '\0';
                strcat(StringTemp, text_buffer);
                strcat(StringTemp, BufferJoinSpace);
                _itoa(static_cast<int>(static_cast<uint8_t>(Weapon[weapon_id].cost)),
                      secondary_buffer, 10);
                strcat(StringTemp, secondary_buffer);
            }
            strcat(StringTemp, SocialWinCloseParenText);
            strcat(StringTemp, "}");
            UNK1(StringTemp);
        }
    }

    for (int reactor_id = 0; reactor_id < MaxReactorNum; ++reactor_id) {
        if (Reactor[reactor_id].preq_tech == tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, picktech_name(Reactor[reactor_id].name));
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, SocialWinOpenParenText);
            // Native485D8B truncates this label to one byte too.
            text_buffer[0] = '\0';
            strcat(text_buffer, StringTable.get(Labels.strings_ptr[0x13c / 4]));
            text_buffer[1] = '\0';
            strcat(StringTemp, text_buffer);
            strcat(StringTemp, SocialWinColonSpaceText);
            _itoa(reactor_id + 1, secondary_buffer, 10);
            strcat(StringTemp, secondary_buffer);
            strcat(StringTemp, SocialWinCloseParenText);
            UNK1(StringTemp);
        }
    }

    for (int proto_id = 0; proto_id < MaxVehProtoFactionNum; ++proto_id) {
        if ((VehPrototypes[proto_id].flags & 1u) != 0 && VehPrototypes[proto_id].preq_tech == tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "{");
            picktech_add_label(0x314 / 4);
            strcat(StringTemp, SocialWinColonSpaceText);
            strcat(StringTemp, VehPrototypes[proto_id].veh_name);
            strcat(StringTemp, "}");
            UNK1(StringTemp);
        }
    }

    // C++ StringList ownership runs its real virtual on_remove hooks and
    // deallocates the circular entries at scope exit. The source seed's
    // handwritten linked-list approximation is deliberately not retained.
    return 1;
}

// Full typed PickTech::pick candidate. Repository integration remains coordinator-owned.
// Four simultaneous automatic Popups own all Scroll, Spot, Dialogs and button lifetimes.
// ORIGINAL: 0x00486510 ?pick@PickTech@@QAEHHHHPAD@Z 0x00486510-0x00488655;0x006576A0-0x00657C12
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00486510; retired seed git:288b790f:src/unrecovered/00486510.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?pick@PickTech@@QAEHHHHPAD@Z
// TRIED: mismatch #2 - the 4 Popup locals (popup_a/b/c/d at -0x15010/-0xA918/-0x559C/-0xFC94) are real C++ locals with automatic construction, not matching the original's explicit ctor-call-per-slot ordering; only popup_d (2nd constructed) is ever start()ed - the other three are built then unconditionally destroyed, confirmed by exactly one `call start@Popup` and zero `call exec@BasePop` in the whole function. The category(0..3) x tech(0..0x58) eligibility scan is transcribed (has_tech x2, tech_avail, tech_category) but the "best recommendation" slot bookkeeping (weighted random tie-break across military/ colonize/infra techs) is dropped - it only affects which item is pre-highlighted, not list membership.
int PickTech::pick(int faction, int mode, int count, char *section_id) {
    Popup popup;
    Popup popup_b;
    Popup popup_c;
    Popup popup_d;
    int i;
    faction_id_ = faction;

    // One flag first marks automated research, then counts the candidates.
    int flag;
    if ((GameDataStateStorage.fields_.game_rules_ & 0x200u) != 0 && count <= 0) {
        flag = 1;
        if (faction == ConsoleGlobal.faction_id_ && IsMultiplayerNet == 0 &&
            (((PlayersData[faction].tech_ranking >> 1) % 5) == 1 ||
             GameDataStateStorage.fields_.turn_current_num_ <= 11) && TechAchievedGate == 0) {
            if ((GameDataStateStorage.fields_.game_preferences_ & 0x20u) != 0 && tut_check2(1))
                popt(TutorialPopupCaption, "TECHNOLOGY", -1, 0, 0);
            ConsoleGlobal.set_ai(faction);
        }
    } else {
        flag = 0;
    }
    if (mode == 0 &&
        (GameDataStateStorage.fields_.turn_current_num_ == 0 ||
         !is_human(faction) ||
         flag != 0) &&
        count < 0) {
        return tech_ai(faction);
    }
    if ((GameDataStateStorage.fields_.game_preferences_ & 0x20u) != 0 && TechAchievedGate == 0 &&
        flag == 0 && faction == ConsoleGlobal.faction_id_ && tut_check2(1)) {
        popt(TutorialPopupCaption, "TECHNOLOGY", -1, 0, 0);
    }
    if (IsMultiplayerNet != 0 && ConsoleControlTurnActive != 0)
        g_FX.play(0x21);

    popup.start(section_id ? section_id : "TECHNOLOGY", 2);
    int best_value = -1;
    int recommendation = -1;
    int best_military_score = -1;
    int best_economic_score = -1;
    item_count_ = 0;
    for (i = 0; i < MaxTechnologyNum; ++i)
        technology_ids_[i] = -1;
    flag = 0;
    for (int category = 0; category < 4; ++category) {
        for (int tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
            if (count >= 0) {
                if (has_tech(tech_id, faction)) continue;
                if (!has_tech(tech_id, count)) continue;
                if (!has_tech(Technology[tech_id].preq_tech_1, faction) &&
                    !has_tech(Technology[tech_id].preq_tech_2, faction))
                    continue;
            } else {
                if (!tech_avail(tech_id, faction)) continue;
                if (flag != 0 && mode == 0) {
                    const int ranking = PlayersData[faction].tech_ranking;
                    const int remainder = (tech_id - ranking + faction) % 3;
                    if ((tech_id < MaxTechnologyNum ? remainder == 0 : remainder) &&
                        (tech_id >= 7 || ranking != 0))
                        continue;
                }
            }
            if (tech_category(tech_id) != category) continue;
            ++flag;
            const int military = 2 * tech_mil(tech_id) + tech_tech(tech_id);
            const int military_score = (military + 2) / 3 +
                static_cast<int>(random(0, (2 * military) / 3 + 1));
            if (military_score >= best_military_score)
                best_military_score = military_score;
            const int economic = 2 * (tech_colonize(tech_id) + tech_infra(tech_id)) +
                tech_tech(tech_id);
            const int economic_score = (economic + 2) / 3 +
                static_cast<int>(random(0, (2 * economic) / 3 + 1));
            if (economic_score >= best_economic_score)
                best_economic_score = economic_score;
            technology_ids_[item_count_] = tech_id;
            ++item_count_;
            const int value = tech_val(tech_id, faction, FALSE);
            if (value >= best_value) {
                best_value = value;
                recommendation = tech_id;
            }
            StringTemp[0] = '\0';
            say_tech(StringTemp, tech_id, FALSE);
            if (tech_id == TECH_TRANT) {
                strcat(StringTemp, BufferJoinSpace);
                say_num(PlayersData[faction].theory_of_everything + 1);
            }
            if (count == 0 &&
                (!has_tech(Technology[tech_id].preq_tech_1, faction) ||
                 !has_tech(Technology[tech_id].preq_tech_2, faction)))
                strcat(StringTemp, "(!)");
            if (tech_id < MaxTechnologyNum) {
                strcat(StringTemp, "|");
                int noted = 0;
                if (tech_id == Rules.tech_three_nutrients_sqr) {
                    if (noted) {
                        strcat(StringTemp, "+");
                    } else {
                        noted = 1;
                        strcat(StringTemp, SocialWinOpenParenText);
                        strcat(StringTemp, label_get(0x458 / 4));
                        strcat(StringTemp, SocialWinCloseParenText);
                    }
                }
                if (tech_id == Rules.tech_three_minerals_sqr) {
                    if (noted) {
                        strcat(StringTemp, "+");
                    } else {
                        noted = 1;
                        strcat(StringTemp, SocialWinOpenParenText);
                        strcat(StringTemp, label_get(0x45C / 4));
                        strcat(StringTemp, SocialWinCloseParenText);
                    }
                }
                if (tech_id == Rules.tech_three_energy_sqr) {
                    if (noted) {
                        strcat(StringTemp, "+");
                    } else {
                        noted = 1;
                        strcat(StringTemp, SocialWinOpenParenText);
                        strcat(StringTemp, label_get(0x460 / 4));
                        strcat(StringTemp, SocialWinCloseParenText);
                    }
                }
                for (i = 0; i < MaxVehProtoFactionNum; ++i) {
                    if ((VehPrototypes[i].flags & 1u) != 0 &&
                        VehPrototypes[i].preq_tech == tech_id) {
                        if (noted) {
                            strcat(StringTemp, "+");
                        } else {
                            strcat(StringTemp, SocialWinOpenParenText);
                            strcat(StringTemp, label_get(0x314 / 4));
                            strcat(StringTemp, SocialWinColonSpaceText);
                            strcat(StringTemp, VehPrototypes[i].veh_name);
                            strcat(StringTemp, SocialWinCloseParenText);
                            noted = 1;
                        }
                    }
                }
                for (i = 0; i < MaxReactorNum; ++i) {
                    if (Reactor[i].preq_tech == tech_id) {
                        if (noted) {
                            strcat(StringTemp, "+");
                        } else {
                            strcat(StringTemp, SocialWinOpenParenText);
                            strcat(StringTemp, label_get(0x13C / 4));
                            strcat(StringTemp, SocialWinColonSpaceText);
                            strcat(StringTemp, StringTable.get(Reactor[i].name));
                            strcat(StringTemp, SocialWinCloseParenText);
                            noted = 1;
                        }
                    }
                }
                for (i = 0; i < MaxWeaponNum; ++i) {
                    if (Weapon[i].preq_tech == tech_id) {
                        if (noted) {
                            strcat(StringTemp, "+");
                        } else {
                            noted = 1;
                            strcat(StringTemp, SocialWinOpenParenText);
                            strcat(StringTemp, label_get(0x80 + (Weapon[i].offense_rating != 0)));
                            strcat(StringTemp, SocialWinColonSpaceText);
                            strcat(StringTemp, StringTable.get(Weapon[i].name));
                            strcat(StringTemp, SocialWinCloseParenText);
                        }
                    }
                }
                for (i = 0; i < MaxArmorNum; ++i) {
                    if (Armor[i].preq_tech == tech_id) {
                        if (noted) {
                            strcat(StringTemp, "+");
                        } else {
                            noted = 1;
                            strcat(StringTemp, SocialWinOpenParenText);
                            strcat(StringTemp, label_get(0x154 / 4));
                            strcat(StringTemp, SocialWinColonSpaceText);
                            strcat(StringTemp, StringTable.get(Armor[i].name));
                            strcat(StringTemp, SocialWinCloseParenText);
                        }
                    }
                }
                for (i = 0; i < MaxChassisNum; ++i) {
                    if (Chassis[i].preq_tech == tech_id) {
                        if (noted) {
                            strcat(StringTemp, "+");
                        } else {
                            noted = 1;
                            strcat(StringTemp, SocialWinOpenParenText);
                            strcat(StringTemp, label_get(0x140 / 4));
                            strcat(StringTemp, SocialWinColonSpaceText);
                            strcat(StringTemp, StringTable.get(Chassis[i].offsv_1_name));
                            strcat(StringTemp, SocialWinCloseParenText);
                        }
                    }
                }
                for (i = 1; i < MaxFacilityNum; ++i) {
                    if (Facility[i].preq_tech == tech_id) {
                        if (noted) {
                            strcat(StringTemp, "+");
                        } else if (i < FacilitySPStart || base_project(i - FacilitySPStart) == -1) {
                            strcat(StringTemp, SocialWinOpenParenText);
                            strcat(StringTemp, StringTable.get(i >= FacilitySPStart
                                ? Labels.strings_ptr[0x2CC / 4]
                                : Labels.strings_ptr[0x26C / 4]));
                            noted = 1;
                            strcat(StringTemp, SocialWinColonSpaceText);
                            strcat(StringTemp, StringTable.get(Facility[i].name));
                            strcat(StringTemp, SocialWinCloseParenText);
                        }
                    }
                }
                for (i = 0; i < MaxTerrainNum - 1; ++i) {
                    for (int sea = 0; sea <= 1; ++sea) {
                        if (tech_id == Terraforming[i].prerequisite_tech[sea] &&
                            (sea != 1 || Terraforming[i].prerequisite_tech[0] !=
                                         Terraforming[i].prerequisite_tech[1])) {
                            if (noted) {
                                strcat(StringTemp, "+");
                            } else {
                                noted = 1;
                                strcat(StringTemp, SocialWinOpenParenText);
                                strcat(StringTemp, StringTable.get(Order[i + 4].order[sea]));
                                strcat(StringTemp, SocialWinCloseParenText);
                            }
                        }
                    }
                }
                for (i = 0; i < MaxAbilityNum; ++i) {
                    if (Ability[i].preq_tech == tech_id) {
                        if (noted) {
                            strcat(StringTemp, "+");
                        } else {
                            noted = 1;
                            strcat(StringTemp, SocialWinOpenParenText);
                            strcat(StringTemp, label_get(0x270 / 4));
                            strcat(StringTemp, SocialWinColonSpaceText);
                            strcat(StringTemp, StringTable.get(Ability[i].name));
                            strcat(StringTemp, SocialWinCloseParenText);
                        }
                    }
                }
                for (i = 0; i < 7; ++i) {
                    if (Citizen[i].preq_tech == tech_id) {
                        if (noted) {
                            strcat(StringTemp, "+");
                        } else {
                            noted = 1;
                            strcat(StringTemp, SocialWinOpenParenText);
                            strcat(StringTemp, label_get(0x4D0 / 4));
                            strcat(StringTemp, SocialWinColonSpaceText);
                            strcat(StringTemp, StringTable.get(Citizen[i].singular_name));
                            strcat(StringTemp, SocialWinCloseParenText);
                        }
                    }
                }
                for (i = 0; i < MaxTechnologyNum; ++i) {
                    if ((Technology[i].preq_tech_1 == tech_id ||
                         Technology[i].preq_tech_2 == tech_id) && !noted) {
                        noted = 1;
                        strcat(StringTemp, SocialWinOpenParenText);
                        strcat(StringTemp, label_get(0x274 / 4));
                        strcat(StringTemp, SocialWinColonSpaceText);
                        strcat(StringTemp, StringTable.get(Technology[i].name));
                        strcat(StringTemp, SocialWinCloseParenText);
                    }
                }
            }
            popup.dialogs().item(StringTemp, tech_id);
            if (item_count_ == 1) popup.set_value(tech_id);
        }
    }
    if (item_count_ == 0) return -1;
    if (!(!is_human(faction))) {

        int title_height;
        int header_height;
        if (item_count_ <= 8) {
            if (WinScreenWidth == 800) {
                title_height = 12;
                header_height = 14;
            } else {
                title_height = 14;
                header_height = 18;
            }
        } else if (WinScreenWidth == 800) {
            // 0x0048797C keeps a dead `cmp eax, 0x320` here: both arms of
            // the screen-width test store the same sizes.
            title_height = 10;
            header_height = 12;
        } else {
            title_height = 10;
            header_height = 12;
        }
        title_font_.init(DefaultFontFace, title_height, 0);
        body_font_primary_.init(DefaultFontFace, title_height, 1);
        body_font_secondary_.init(DefaultFontFace, title_height, 2);
        header_font_.init(DefaultFontFace, header_height, 1);
        Buffer::set_font(&title_font_, &body_font_primary_, &body_font_secondary_, 0);
        int columns_by_count[100] = {
            0, 1, 2, 3, 4, 5, 3, 4, 4, 5, 5, 6, 4, 5, 5, 5, 4, 5, 6, 5,
            5, 7, 6, 6, 6, 5, 7, 7, 7, 8, 8, 8, 8, 7, 7, 7, 8, 8, 8, 8,
            10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
            10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
            10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10
        };
        const int items = item_count_;
        const int columns = columns_by_count[items];
        column_count_ = columns;
        const int rows = items / columns + (items % columns != 0);
        row_count_ = rows;
        cell_width_ = items <= 8 ? WinScreenWidth / columns - 20
                                 : (WinScreenWidth - 20) / columns;
        const int cell_width = min(cell_width_, WinScreenWidth / 3);
        cell_width_ = cell_width;
        const int cell_height = (3 * cell_width) / 2;
        cell_height_ = cell_height;
        cell_height_ = max(cell_height, WinScreenHeight / 2);
        if (cell_height_ * rows + 29 > WinScreenHeight)
            cell_height_ = items <= 8 ? WinScreenHeight / rows - 40
                                      : (WinScreenHeight - 69) / rows;
        row_pitch_ = 10;
        GraphicWin::init(0, 0, (cell_width + 1) * columns + 1,
            (cell_height_ + 1) * rows + 30, 0, 0x05000800, 0, 0, 0);
        Win::center();
        field_16C_ = 29;
        RECT frame = Buffer::clip_rect();
        Buffer::box_sprite(&frame, &InterfaceBoxStyles[9]);
        expand_rect(&frame, -3, -3);
        frame.bottom = frame.top + 23;
        Buffer::box_sprite(&frame, &InterfaceBoxStyles[6]);
        expand_rect(&frame, -3, -3);
        StringTemp[0] = '\0';
        strcat(StringTemp, popup.caption_text());
        CharUpperA(StringTemp);
        Buffer::set_text_color(0x9F, -1, 1, 1);
        Buffer::set_font(find_font(18, 1), 0, 0, 0);
        Buffer::write_cent_l(StringTemp, &frame);
        selected_index_ = 0;
        selection_active_ = 0;
        for (i = 0; i < item_count_; ++i)
            draw_item(i, i == selected_index_);
        show(0);
        grab_pointer();
        set_modal(1, 0, 0);
        hide();
        do_all_draws();
        return technology_ids_[selected_index_];
    } else {
        return recommendation;
    }
}
