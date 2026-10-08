// ORIGINAL: 0x00488740 ??_GPickTech@@WEEE@AEPAXI@Z 0x00488740-0x0048874B FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original ??_GPickTech@@WEEE@AEPAXI@Z at 0x00488740 is an 11-byte secondary scalar deleting adjustor; it subtracts 0x444 from ECX and tail-jumps to the complete-object PickTech scalar deleting entry without touching stack or flags.", "Historical seed correction: the earlier `??3` deallocation interpretation was refuted; the original is a `??_G` scalar-deleting adjustor. `WEEE` demangles to adjustor{1092}, equal to 0x444.", "Pinned PickTech constructor at 0x00484b43 stores Buffer-view vftable 0x0066cfc4 at this+0x444; the source owner is the typed PickTech GraphicWin/Buffer multiple-inheritance view.", "Current src/picktech.cpp COFF Buffer-view vftable first slot selects generated ??_EPickTech@@WEEE@AEPAXI@Z; its body performs the same 0x444 adjustment and preserves the flag argument.", "Current PickTech COFF weak external ??_EPickTech@@UAEPAXI@Z records search mode 2 (library search) and fallback target ??_GPickTech@@UAEPAXI@Z; this linked image resolves both primary labels to the same physical class deleting body. The object fallback and linked address are per-build evidence, not a general E/G equivalence or owner-selection rule.", "That measured current primary body calls PickTech::~PickTech, tests flags bit 0, calls operator delete only when bit 0 is set, returns the saved object pointer, and uses ret 4; this is the per-class measured E-to-G scalar path, not a general vector/scalar equivalence.", "The removed transitional C++ names were ?adjust_this_pick_tech@@YIPAXPAX0I@Z and ?scalar_delete_pick_tech@@YIPAXPAX0I@Z; no callers exist outside deleting_thunks.cpp/.h, and the current generated Buffer vftable uses the class-owned E adjustor instead.", "Historical seed metadata noted flags hidden;sp_ready;purged_ok, zero direct callers, and no direct call targets. These remain historical observations only; they do not establish virtual-dispatch completeness or acceptance.", "The old secondary record had no BYTE_EXACT or SEMANTIC claim. Its original identity, full span, kind and parsed history are moved unchanged; no matching claim is granted."], "reason": "Bind the pinned secondary scalar deleting identity for PickTech to its measured genuine Buffer-view generated adjustor. Keep the original G identity and point its current symbol fact at the owner-emitted E adjustor; the per-object weak fallback and bit-0 path above establish why this exact current implementation preserves the original operation."}
// symbol    ??_EPickTech@@WEEE@AEPAXI@Z
// kind      game
// ORIGINAL: 0x00488660 ??_GPickTech@@UAEPAXI@Z 0x00488660-0x00488681 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00488660-0x00488681", "Pinned class constructor ??0PickTech@@QAE@XZ at 0x00484AC0 stores original class/view table VA 0x66cfcc at 0x00484B3D (mov dword ptr [esi], 0x66cfcc); the current genuine owner vftable ??_7PickTech@@6BWin@@@ selects generated ??_GPickTech@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EPickTech@@UAEPAXI@Z (mode 2).", "Pinned ??_GPickTech@@UAEPAXI@Z at 0x00488660 calls the source-mapped ??1PickTech@@QAE@XZ at 0x00488690, then tests byte ptr [ebp + 8], 1; je 0x48867a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/picktech.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original PickTech scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GPickTech@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
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
void __cdecl help_any();
int __cdecl tut_check2(int);
extern char *TutorialPopupCaption;
extern Sprite g_TECH_ICON_SPRITES[MaxTechnologyNum];

// Native484AC0 installs the primary table66CFCC and secondary table66CFC4.
// C++ owns the entries; the primary complete original extent remains unproved.
// RECOVERY_GENERATED: v1 vtable 0x0066CFCC ? ??_7PickTech@@6BWin@@@ 0x00484AC0
// RECOVERY_GENERATED: v1 vtable 0x0066CFC4 0x0066CFCC ??_7PickTech@@6BBuffer@@@ 0x00484AC0

// Constructor evidence: the body calls GraphicWin::GraphicWin, constructs
// Font at +BBC,+BE4,+C0C,+C34 and zeroes +A24,+A28,+A38,+A3C. Real C++ base
// and member construction supplies those lifetimes; no placement-new or
// explicit vtable stores are retained.
// ORIGINAL: 0x00484AC0 ??0PickTech@@QAE@XZ 0x00484AC0-0x00484B5C;0x00657630-0x0065766C
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00484AC0; retired seed git:6e445109:src/unrecovered/00484ac0.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
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

// ORIGINAL: 0x00488690 ??1PickTech@@QAE@XZ 0x00488690-0x0048873F;0x00657C20-0x00657C6A
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00488690; retired seed git:6e445109:src/unrecovered/00488690.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ??1PickTech@@UAE@XZ
// TRIED: same class as 0x00484AC0 (PickTech's ctor, already landed MISMATCH #1) - the scaffold's PickTech is opaque (no real GraphicWin/Font subobjects), so explicit `->close()`/`->~Font()`/`->~GraphicWin()` calls reproduce the CALL SEQUENCE but not the `push -1/push handler/mov fs:[0]` SEH-unwind prologue, which only a compiler-recognised subobject with a non-trivial destructor would trigger. Field-store order (vtables first, then the four zeroed fields, matches disassembly) is otherwise exact.
PickTech::~PickTech() {
    // Native4886C9..4886DB clears the four picker fields before base close.
    close();
    // C++ performs the Font and GraphicWin destructor sequence automatically.
}

// ORIGINAL: 0x00485FC0 ?on_mouse_move@PickTech@@QAEXHH@Z 0x00485FC0-0x004860A6
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00485FC0; retired seed git:6e445109:src/unrecovered/00485fc0.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?on_mouse_move@PickTech@@UAEXHH@Z
// TRIED: flipping `field*(field+1) <= a1` to `a1 >= field*(field+1)` fixed two jge/jle polarity flips, reaching 0.987 mnemonic similarity; the one remaining `a2 - 0x1d` compiles to `add eax,-0x1d` no matter how the subtraction is spelled (single temp, split statement, reordered operands all tried).
void PickTech::on_mouse_move(int x, int y) {
    const int local_y = y - 0x1D;
    if (x < 0 || local_y < 0 ||
        x >= column_count_ * (cell_width_ + 1) ||
        local_y >= row_count_ * (cell_height_ + 1)) {
        const int current = selected_index_;
        if (current >= 0) {
            draw_item(current, 0);
            soft_update();
        }
        selection_active_ = 0;
        selected_index_ = -1;
    } else {
        const int index = (local_y / (cell_height_ + 1)) * column_count_
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

// ORIGINAL: 0x004863F0 ?draw_item@PickTech@@QAEHHH@Z 0x004863F0-0x0048650C
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004863F0; retired seed git:6e445109:src/unrecovered/004863f0.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?draw_item@PickTech@@QAEHHH@Z
// TRIED: a `Rect4{left,top,right,bottom}` local (matches the RECT the original builds on the stack field-by-field) and the `-(int)(a2!=0)&2` neg/sbb/and idiom for the sprite-params table offset, per the lever for a `guard ? obj : 0` shape. Landed at 0.71 similarity: the original keeps a stack frame (`push ebp; mov ebp,esp`) that this body's simpler local set let VC6 omit, the same shape seen on 0x00458DC0/0x004863F0-sized leaf-ish functions this batch.
int PickTech::draw_item(int item, int selected) {
    if (item_count_ <= item) return 0;

    RECT rect;
    rect.left = (item % column_count_) * (cell_width_ + 1) + 1;
    rect.right = cell_width_ + rect.left;
    rect.top = (item / column_count_) * (cell_height_ + 1) + 0x1E;
    rect.bottom = cell_height_ + rect.top;

    Buffer *const canvas = static_cast<Buffer *>(this);
    canvas->set_clip(&rect);
    rect.right -= 3;
    rect.bottom -= 3;
    if (rect.left < 4) rect.left = 3;
    const int max_right = canvas->pixel_width() - 3;
    if (max_right <= rect.right) rect.right = max_right;
    if (rect.top < 0x1E) rect.top = 0x1D;
    const int min_bottom = -3 + canvas->pixel_height();
    if (min_bottom <= rect.bottom) rect.bottom = min_bottom;

    const unsigned int style = selected != 0 ? 2u : 0u;
    canvas->box_sprite(&rect, &InterfaceBoxStyles[style]);
    const int tech_id = technology_ids_[item];
    const int result = draw_tech(tech_id, &rect, selected);
    canvas->restore_clip();
    return result;
}

// This is the owned member counterpart of the current stringstruct.cpp
// receiver-offset transcription. All five addresses are established by the
// seed accesses; Buffer is GraphicWin's real secondary base, not a byte view.
// ORIGINAL: 0x00484B80 ?UNK1@PickTech@@QAEXPAD@Z 0x00484B80-0x00484C5D
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00484B80; source:src/leaf_recoveries.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?UNK1@PickTech@@QAEXPAD@Z
void PickTech::UNK1(char *text) {
    Buffer *const canvas = static_cast<Buffer *>(this);
    int line_height;
    if (page_state_ == 0) {
        canvas->set_font(&header_font_, &header_font_, &header_font_, 0);
        line_height = header_font_.line_advance();
        page_state_ = 1;
    } else {
        canvas->set_font(&title_font_, &body_font_primary_,
                         &body_font_secondary_, 0);
        line_height = title_font_.line_advance();
    }
    if (page_bottom_ - line_height >= page_pen_y_) {
        const int wrapped_height = canvas->wrap_height(text, page_width_ - 2);
        const int y = page_bottom_ - wrapped_height;
        page_bottom_ = y;
        canvas->wrap_cent(text, page_x_ + 1, y, page_width_ - 2);
    }
}

// ORIGINAL: 0x004860B0 ?on_left_click@PickTech@@QAEXHH@Z 0x004860B0-0x00486109 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004860B0; retired seed git:6e445109:src/recovered/units/004860b0.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
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
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00486110; retired seed git:6e445109:src/recovered/units/00486110.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
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

// ORIGINAL: 0x00486170 ?on_key_click@PickTech@@QAEHHH@Z 0x00486170-0x004863AD
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00486170; retired seed git:6e445109:src/unrecovered/00486170.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
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
        if (previous < 0 || previous >= item_count_) {
            if (previous >= 0) draw_item(previous, 0);
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
    if (character != 0 && item_count_ > 0) {
        for (i = 0; i < item_count_; ++i) {
            StringTemp[0] = '\0';
            say_tech(StringTemp, technology_ids_[i], 0);
            const int upper_character = toupper(character);
            const signed char first_character =
                static_cast<signed char>(StringTemp[0]);
            if (upper_character == first_character) {
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
    return StringTable.get(static_cast<int>(reinterpret_cast<intptr_t>(address)));
}

static void picktech_add_label(int index) {
    strcat(StringTemp, label_get(index));
}

// ORIGINAL: 0x00484C60 ?draw_tech@PickTech@@QAEHHPAURECT@@H@Z 0x00484C60-0x00485FBD;0x0065766C-0x006576A0
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00484C60; retired seed git:6e445109:src/unrecovered/00484c60.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
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
    Buffer *const canvas = static_cast<Buffer *>(this);
    canvas->set_text_color(MessageWinTextColor, 0, 1, 1);
    canvas->set_text_color2(MessageWinTextColor, 0, 1, 1);
    page_width_ = rect->right - rect->left;
    title_baseline_ = title_font_.line_advance();
    page_x_ = rect->left;
    page_pen_y_ = row_pitch_ + rect->top;
    page_bottom_ = rect->bottom - row_pitch_ / 2;

    StringTemp[0] = '\0';
    const int font_size = item_count_ < 9
        ? (WinScreenWidth == 800 ? 0x12 : 0x14) : 0x0C;
    Font *const font_a = find_font(font_size, 2);
    Font *const font_b = find_font(font_size, 1);
    Font *const font_c = find_font(font_size, 1);
    canvas->set_font(font_c, font_b, font_a, 0);
    say_tech(StringTemp, tech, 0);
    if (tech == TECH_TRANT) {
        strcat(StringTemp, " ");
        _itoa(PlayersData[faction_id_].theory_of_everything + 1,
              number_buffer, 10);
        strcat(StringTemp, number_buffer);
    }
    strcpy(text_buffer, StringTemp);
    if (canvas->text_width(StringTemp) > page_width_ - 2 &&
        strchr(StringTemp, ' ') == 0) {
        if (canvas->text_width(text_buffer) > page_width_ - 2) {
            do {
                const size_t length = strlen(text_buffer);
                if (length == 0) break;
                text_buffer[length - 1] = '\0';
            } while (canvas->text_width(text_buffer) > page_width_ - 2);
        }
        strcat(text_buffer, " ");
        const size_t suffix_offset = strlen(text_buffer);
        strcat(text_buffer, StringTemp + suffix_offset - 1);
    }

    const int category = tech_category(tech);
    const int requirement_count = tech_recurse(tech, 0);
    if (item_count_ > 8) {
        canvas->set_text_color(mandate_color(category), 0, 1, 1);
        const int width = rect->right - rect->left - 4;
        const int wrapped_height = canvas->wrap_height(text_buffer, width);
        const int y = (rect->bottom - rect->top - wrapped_height) / 2 + rect->top;
        page_pen_y_ = canvas->wrap_cent(text_buffer, rect->left + 2, y, width);
    } else {
        page_pen_y_ = canvas->wrap_cent(text_buffer, page_x_ + 2,
                                      page_pen_y_, page_width_ - 6);
    }
    if (item_count_ > 8) return 1;

    canvas->set_font(&title_font_, &body_font_primary_, &body_font_secondary_, 0);
    canvas->set_text_color(mandate_color(category), 0, 1, 1);
    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(static_cast<int>(
        reinterpret_cast<intptr_t>(Mandate[category].name_caps))));
    strcat(StringTemp, " ");
    _itoa(requirement_count, number_buffer, 10);
    strcat(StringTemp, number_buffer);
    page_pen_y_ = canvas->wrap_cent(StringTemp, page_x_ + 1, page_pen_y_,
                                    page_width_ - 2);
    canvas->set_text_color(MessageWinTextColor, 0, 1, 1);
    canvas->set_text_color2(0x5F, 0, 1, 1);

    int metric;
    if (WinScreenWidth == 800) metric = cell_width_ < 0xF0 ? -0x0E : -0x0C;
    else if (cell_width_ < 0xB4) metric = cell_width_ <= 0x8C ? -0x0E : -0x0C;
    else metric = -0x0A;

    if (tech < MaxTechnologyNum) {
        page_pen_y_ += row_pitch_;
        int icon_width;
        if (MapWinCaptionScale < 1) {
            SpriteScaleNumerator = metric + 0x10;
            SpriteScaleDenominator = 0x10;
            icon_width = ((metric + 0x10) * 0xD7) / 0x10;
        } else {
            SpriteScaleNumerator = MapWinCaptionScale;
            SpriteScaleDenominator = MapWinCaptionDivisor;
            icon_width = MapWinCaptionScale * 0xD7 / MapWinCaptionDivisor;
        }
        const int icon_category = tech_category(tech);
        const int icon_color = TextColourTable[162 + icon_category * 2];
        const int icon_x = (page_width_ - icon_width) / 2 + rect->left;
        const int transparent = g_TECH_ICON_SPRITES[tech].transparent_index();
        g_TECH_ICON_SPRITES[tech].draw_mono(canvas, transparent, icon_x,
                                             page_pen_y_, icon_color);
        const int icon_height = MapWinCaptionScale < 1
            ? ((metric + 0x10) * 0xD7) / 0x10
            : MapWinCaptionScale * 0xD7 / MapWinCaptionDivisor;
        page_pen_y_ += icon_height + row_pitch_;
        SpriteScaleNumerator = 1;
        SpriteScaleDenominator = 1;
    }
    if (tech > 0x60) {
        if (tech >= 0x59) return 1;
        page_pen_y_ += row_pitch_;
        const int fake_vehicle = veh_fake(tech - 0x61, faction_id_);
        const int vehicle_width = MapWinCaptionScale < 1
            ? ((metric * 5 + 0x50) * 20) / 16
            : MapWinCaptionScale * 100 / MapWinCaptionDivisor;
        const int vehicle_x = (page_width_ - vehicle_width) / 2 + rect->left;
        veh_draw(canvas, fake_vehicle, vehicle_x, page_pen_y_, metric, 0, 0);
        const int vehicle_height = MapWinCaptionScale < 1
            ? ((metric * 5 + 0x50) * 0x0F) / 16
            : MapWinCaptionScale * 0x4B / MapWinCaptionDivisor;
        page_pen_y_ += vehicle_height + row_pitch_;
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
            page_pen_y_ = canvas->write_strings(&technology_notes, page_x_ + 4,
                                                 page_pen_y_, page_width_ - 8, 0);
            technology_notes.remove_all();
            // Native485408 resets the cursor even when remove_all's head
            // guard returns without clearing it.
            technology_notes.current_position_ = 0;
        }
    page_state_ = 1;

    // Typed RuleTechnology association scan. The native cursor is the
    // preq_tech_2 word in record zero and advances by the 0x2c stride.
    for (i = 0; i < MaxTechnologyNum; ++i) {
        const RulesTechnology &rule = Technology[i];
        if (rule.preq_tech_1 == tech || rule.preq_tech_2 == tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "[");
            strcat(StringTemp, picktech_name(rule.name));
            strcat(StringTemp, "]");
            UNK1(StringTemp);
        }
    }

    // Facility.free_tech occupies the ordinary facility range only; the
    // native cursor stops exactly at Facility[FacilitySPStart], where the
    // special-project rules records begin.
    for (i = 1; i < FacilitySPStart; ++i) {
        const RulesFacility &rule = Facility[i];
        if (rule.free_tech == tech) {
            StringTemp[0] = '\0';
            if (page_state_ == 0) strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(rule.name));
            strcat(StringTemp, " ");
            picktech_add_label(0x4dc / 4);
            if (page_state_ == 0) strcat(StringTemp, "}");
            UNK1(StringTemp);
        }
    }

    // Citizen records (not SecretProject records) carry the +8 prerequisite.
    // Native end 0x9460E4 is seven 0x1c-byte rule records, before the next
    // independently owned data object.
    for (i = 0; i < 7; ++i) {
        const RulesCitizen &rule = Citizen[i];
        if (rule.preq_tech == tech) {
            StringTemp[0] = '\0';
            if (page_state_ == 0) strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(rule.singular_name));
            if (page_state_ == 0) strcat(StringTemp, "}");
            UNK1(StringTemp);
        }
    }

    for (i = 0; i < MaxAbilityNum; ++i) {
        const RulesAbility &rule = Ability[i];
        if (rule.preq_tech == tech) {
            StringTemp[0] = '\0';
            if (page_state_ == 0) strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(rule.name));
            if (page_state_ == 0) strcat(StringTemp, "}");
            UNK1(StringTemp);
        }
    }

    // Native 0x691880 is Terraforming[0].preq_tech; the 0x20-byte cursor
    // visits the 19 non-monolith rules. Caption IDs are the actual parsed
    // Order records at i+4 (land and sea text are separate columns).
    for (int row = 0; row < MaxTerrainNum - 1; ++row) {
        const RulesTerraforming &rule = Terraforming[row];
        for (int model = 0; model < 2; ++model) {
            const int required_tech = model == 0
                ? rule.preq_tech : rule.preq_tech_sea;
            if (tech != required_tech ||
                (model == 1 && rule.preq_tech == rule.preq_tech_sea)) {
                continue;
            }
            StringTemp[0] = '\0';
            if (page_state_ == 0) strcat(StringTemp, "{");
            const LPSTR order_text = model == 0
                ? Order[row + 4].order : Order[row + 4].order_sea;
            strcat(StringTemp, picktech_name(order_text));
            if (page_state_ == 0) strcat(StringTemp, "}");
            UNK1(StringTemp);
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
        const RulesFacility &rule = Facility[i];
        if (rule.preq_tech == tech) {
            const bool ordinary = i < FacilitySPStart;
            StringTemp[0] = '\0';
            if (ordinary || page_state_ == 0) strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(rule.name));
            if (ordinary) {
                strcat(StringTemp, " ");
                strcat(StringTemp, "(");
                strcat(StringTemp, picktech_name(rule.effect));
                strcat(StringTemp, ")");
                strcat(StringTemp, "}");
            } else if (page_state_ == 0) {
                strcat(StringTemp, "}");
            }
            UNK1(StringTemp);
        }
    }

    // RulesSocialCategory.name[] and preq_tech[] are interleaved in the
    // native cursor but paired within each true 0xD4 RulesSocialCategory.
    for (int category_id = 0; category_id < 4; ++category_id) {
        const RulesSocialCategory &rule = SocialCategories[category_id];
        for (int model = 0; model < 4; ++model) {
            if (rule.preq_tech[model] == tech) {
                StringTemp[0] = '\0';
                strcat(StringTemp, "{");
                strcat(StringTemp, picktech_name(rule.name[model]));
                strcat(StringTemp, "}");
                UNK1(StringTemp);
            }
        }
    }

    for (int chassis_id = 0; chassis_id < MaxChassisNum; ++chassis_id) {
        const RulesChassis &rule = Chassis[chassis_id];
        if (rule.preq_tech == tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(rule.offsv_1_name));
            strcat(StringTemp, " (");
            strcat(StringTemp, StringTable.get(static_cast<int>(
                reinterpret_cast<intptr_t>(Triad[rule.triad]))));
            strcat(StringTemp, " ");
            _itoa(static_cast<int>(rule.speed),
                  number_buffer, 10);
            strcat(StringTemp, number_buffer);
            strcat(StringTemp, ")}");
            UNK1(StringTemp);
        }
    }

    for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
        const RulesArmor &rule = Armor[armor_id];
        if (rule.preq_tech == tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(rule.name));
            strcat(StringTemp, " (");
            if (rule.defense_rating < 0) {
                picktech_add_label(0x310 / 4);
            } else {
                _itoa(static_cast<int>(rule.defense_rating), number_buffer, 10);
                strcat(StringTemp, number_buffer);
                strcat(StringTemp, ")}");
            }
            UNK1(StringTemp);
        }
    }

    for (int weapon_id = 0; weapon_id < MaxWeaponNum; ++weapon_id) {
        const RulesWeapon &rule = Weapon[weapon_id];
        if (rule.preq_tech == tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "{");
            strcat(StringTemp, picktech_name(rule.name));
            strcat(StringTemp, " (");
            if (rule.offense_rating == 0) {
                text_buffer[0] = '\0';
                strcat(text_buffer, label_get(0x200 / 4));
                // Native485C81 retains only the label's first byte.
                text_buffer[1] = '\0';
                strcat(StringTemp, text_buffer);
                strcat(StringTemp, " ");
                _itoa(static_cast<int>(static_cast<uint8_t>(rule.cost)),
                      secondary_buffer, 10);
                strcat(StringTemp, secondary_buffer);
            } else if (rule.offense_rating < 0) {
                picktech_add_label(0x310 / 4);
            } else {
                _itoa(static_cast<int>(rule.offense_rating), number_buffer, 10);
                strcat(StringTemp, number_buffer);
            }
            strcat(StringTemp, ")}");
            UNK1(StringTemp);
        }
    }

    for (int reactor_id = 0; reactor_id < MaxReactorNum; ++reactor_id) {
        const RulesReactor &rule = Reactor[reactor_id];
        if (rule.preq_tech == tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, picktech_name(rule.name));
            strcat(StringTemp, " (");
            // Native485D8B truncates this label to one byte too.
            text_buffer[0] = '\0';
            strcat(text_buffer, label_get(0x13c / 4));
            text_buffer[1] = '\0';
            strcat(StringTemp, text_buffer);
            strcat(StringTemp, ": ");
            _itoa(reactor_id + 1, secondary_buffer, 10);
            strcat(StringTemp, secondary_buffer);
            strcat(StringTemp, ")");
            UNK1(StringTemp);
        }
    }

    for (int proto_id = 0; proto_id < MaxVehProtoFactionNum; ++proto_id) {
        const VehPrototype &rule = VehPrototypes[proto_id];
        if ((rule.flags & 1u) != 0 && rule.preq_tech == tech) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "{");
            picktech_add_label(0x314 / 4);
            strcat(StringTemp, ": ");
            strcat(StringTemp, rule.veh_name);
            strcat(StringTemp, "}");
            UNK1(StringTemp);
        }
    }

    // C++ StringList ownership runs its real virtual on_remove hooks and
    // deallocates the circular entries at scope exit. The source seed's
    // handwritten linked-list approximation is deliberately not retained.
    return 1;
}

static void picktech_append_rule_annotation(bool &has_annotation,
                                            int label_index,
                                            bool label_has_colon,
                                            const char *name,
                                            bool name_is_string_reference) {
    if (has_annotation) {
        strcat(StringTemp, "+");
        return;
    }
    strcat(StringTemp, "(");
    if (label_index >= 0) {
        // Native resolves/appends the label before resolving the rule name.
        strcat(StringTemp, label_get(label_index));
        if (label_has_colon) strcat(StringTemp, ": ");
    }
    if (name_is_string_reference || name != 0) {
        strcat(StringTemp, name_is_string_reference
            ? picktech_name(const_cast<char *>(name)) : name);
    }
    strcat(StringTemp, ")");
    has_annotation = true;
}


// Full typed PickTech::pick candidate. Repository integration remains coordinator-owned.
// Four simultaneous automatic Popups own all Scroll, Spot, Dialogs and button lifetimes.
// ORIGINAL: 0x00486510 ?pick@PickTech@@QAEHHHHPAD@Z 0x00486510-0x00488655;0x006576A0-0x00657C12
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00486510; retired seed git:6e445109:src/unrecovered/00486510.cpp; constructor484AC0/destructor488690 establish native C5C ownership.", "image:66CFCC callbackslots17/19/22/25; native rule-bank and UI call sites retain owner and evaluation order.", "source:src/stringstruct.cpp historical484B80 transcription; the canonical caption owns Font line-advance and Buffer wrap semantics."], "reason": "Genuine GraphicWin-derived PickTech owner with four automatic Font members, source-owned rule/global references, real callback dispatch and lexical popup/list lifetimes."}
// symbol    ?pick@PickTech@@QAEHHHHPAD@Z
// TRIED: mismatch #2 - the 4 Popup locals (popup_a/b/c/d at -0x15010/-0xA918/-0x559C/-0xFC94) are real C++ locals with automatic construction, not matching the original's explicit ctor-call-per-slot ordering; only popup_d (2nd constructed) is ever start()ed - the other three are built then unconditionally destroyed, confirmed by exactly one `call start@Popup` and zero `call exec@BasePop` in the whole function. The category(0..3) x tech(0..0x58) eligibility scan is transcribed (has_tech x2, tech_avail, tech_category) but the "best recommendation" slot bookkeeping (weighted random tie-break across military/ colonize/infra techs) is dropped - it only affects which item is pre-highlighted, not list membership.
int PickTech::pick(int faction, int mode, int count, char *section_id) {
    int i;
    Popup popup_d;
    Popup popup_a;
    Popup popup_b;
    Popup popup_c;
    faction_id_ = faction;

    int automated_research = 0;
    if ((GameDataStateStorage.fields_.game_rules_ & 0x200u) != 0 && count <= 0) {
        automated_research = 1;
        if (faction == ConsoleGlobal.faction_id_ && IsMultiplayerNet == 0 &&
            (((PlayersData[faction].tech_ranking >> 1) % 5) == 1 ||
             GameDataStateStorage.fields_.turn_current_num_ <= 11) && TechAchievedGate == 0) {
            if ((GameDataStateStorage.fields_.game_preferences_ & 0x20u) != 0 && tut_check2(1))
                popt(TutorialPopupCaption, "TECHNOLOGY", -1, 0, 0);
            ConsoleGlobal.set_tech(faction);
        }
    }
    if (mode == 0 && count < 0 &&
        (GameDataStateStorage.fields_.turn_current_num_ == 0 ||
         (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (1u << faction)) == 0 ||
         automated_research != 0)) {
        return tech_ai(faction);
    }
    if ((GameDataStateStorage.fields_.game_preferences_ & 0x20u) != 0 && TechAchievedGate == 0 &&
        automated_research == 0 && faction == ConsoleGlobal.faction_id_ &&
        tut_check2(1)) {
        popt(TutorialPopupCaption, "TECHNOLOGY", -1, 0, 0);
    }
    if (IsMultiplayerNet != 0 && ConsoleControlTurnActive != 0)
        g_FX.play(0x21);

    popup_d.start(PopupStartCaption, section_id ? section_id : "TECHNOLOGY",
                  -1, 0, 2, 0);
    int best_value = -1;
    int recommendation = -1;
    int best_military_score = -1;
    int best_economic_score = -1;
    int recommendation_candidate_count = 0;
    item_count_ = 0;
    for (int clear = 0; clear < MaxTechnologyNum; ++clear)
        technology_ids_[clear] = -1;
    char number_buffer[80];
    for (int category = 0; category < 4; ++category) {
        for (int tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
            if (count >= 0) {
                if (has_tech(tech_id, faction) || !has_tech(tech_id, count))
                    continue;
                if (!has_tech(Technology[tech_id].preq_tech_1, faction) &&
                    !has_tech(Technology[tech_id].preq_tech_2, faction))
                    continue;
            } else {
                if (!tech_avail(tech_id, faction)) continue;
                if (recommendation_candidate_count != 0 && mode == 0) {
                    const int ranking = PlayersData[faction].tech_ranking;
                    if ((tech_id - ranking + faction) % 3 == 0 &&
                        (tech_id >= 7 || ranking != 0)) continue;
                }
            }
            if (tech_category(tech_id) != category) continue;
            ++recommendation_candidate_count;
            const int military = 2 * tech_mil(tech_id) + tech_tech(tech_id);
            const int military_random = static_cast<int>(
                random(0, (2 * military) / 3 + 1));
            const int military_score = (military + 2) / 3 + military_random;
            if (military_score >= best_military_score)
                best_military_score = military_score;
            const int economic_colonize = tech_colonize(tech_id);
            const int economic_infra = tech_infra(tech_id);
            const int economic = 2 * (economic_colonize + economic_infra)
                               + tech_tech(tech_id);
            const int economic_random = static_cast<int>(
                random(0, (2 * economic) / 3 + 1));
            const int economic_score = (economic + 2) / 3 + economic_random;
            if (economic_score >= best_economic_score)
                best_economic_score = economic_score;
            technology_ids_[item_count_++] = tech_id;
            const int value = tech_val(tech_id, faction, FALSE);
            if (value >= best_value) {
                best_value = value;
                recommendation = tech_id;
            }
            StringTemp[0] = '\0';
            say_tech(StringTemp, tech_id, FALSE);
            if (tech_id == TECH_TRANT) {
                strcat(StringTemp, " ");
                _itoa(PlayersData[faction].theory_of_everything + 1,
                      number_buffer, 10);
                strcat(StringTemp, number_buffer);
            }
            if (count == 0 &&
                (!has_tech(Technology[tech_id].preq_tech_1, faction) ||
                 !has_tech(Technology[tech_id].preq_tech_2, faction)))
                strcat(StringTemp, "(!)");
            strcat(StringTemp, "|");
            bool has_rule_annotation = false;
            // Native later-benefit arms append only "+": defer StringTable
            // lookups until the first-benefit arm, including argument evaluation.
            const int basic_techs[3] = {
                Rules.tech_three_nutrients_sqr,
                Rules.tech_three_minerals_sqr,
                Rules.tech_three_energy_sqr
            };
            for (i = 0; i < 3; ++i)
                if (basic_techs[i] == tech_id)
                    picktech_append_rule_annotation(has_rule_annotation, 278 + i,
                                                    false, 0, false);
            for (i = 0; i < MaxVehProtoFactionNum; ++i) {
                const VehPrototype &prototype = VehPrototypes[i];
                if ((prototype.flags & 1u) != 0 && prototype.preq_tech == tech_id)
                    picktech_append_rule_annotation(has_rule_annotation,
                        0x314 / 4, true, prototype.veh_name, false);
            }
            for (i = 0; i < MaxReactorNum; ++i)
                if (Reactor[i].preq_tech == tech_id)
                    picktech_append_rule_annotation(has_rule_annotation,
                        0x13C / 4, true, Reactor[i].name, true);
            for (i = 0; i < MaxWeaponNum; ++i)
                if (Weapon[i].preq_tech == tech_id)
                    picktech_append_rule_annotation(has_rule_annotation,
                        0x80 + (Weapon[i].offense_rating != 0),
                        true, Weapon[i].name, true);
            for (i = 0; i < MaxArmorNum; ++i)
                if (Armor[i].preq_tech == tech_id)
                    picktech_append_rule_annotation(has_rule_annotation,
                        0x154 / 4, true, Armor[i].name, true);
            for (i = 0; i < MaxChassisNum; ++i)
                if (Chassis[i].preq_tech == tech_id)
                    picktech_append_rule_annotation(has_rule_annotation,
                        0x140 / 4, true, Chassis[i].offsv_1_name, true);
            for (i = 1; i < MaxFacilityNum; ++i)
                if (Facility[i].preq_tech == tech_id &&
                    (has_rule_annotation || i < FacilitySPStart ||
                     base_project(i - FacilitySPStart) == -1))
                    picktech_append_rule_annotation(has_rule_annotation,
                        i >= FacilitySPStart ? 0x2CC / 4 : 0x26C / 4,
                        true, Facility[i].name, true);
            for (i = 0; i < MaxTerrainNum - 1; ++i) {
                const RulesTerraforming &terrain = Terraforming[i];
                if (terrain.preq_tech == tech_id)
                    picktech_append_rule_annotation(has_rule_annotation, -1,
                        false, Order[i + 4].order, true);
                if (terrain.preq_tech_sea == tech_id && terrain.preq_tech != terrain.preq_tech_sea)
                    picktech_append_rule_annotation(has_rule_annotation, -1,
                        false, Order[i + 4].order_sea, true);
            }
            for (i = 0; i < MaxAbilityNum; ++i)
                if (Ability[i].preq_tech == tech_id)
                    picktech_append_rule_annotation(has_rule_annotation,
                        0x270 / 4, true, Ability[i].name, true);
            for (i = 0; i < 7; ++i)
                if (Citizen[i].preq_tech == tech_id)
                    picktech_append_rule_annotation(has_rule_annotation,
                        0x4D0 / 4, true, Citizen[i].singular_name, true);
            for (i = 0; i < MaxTechnologyNum; ++i) {
                const RulesTechnology &dependent = Technology[i];
                if (!has_rule_annotation &&
                    (dependent.preq_tech_1 == tech_id || dependent.preq_tech_2 == tech_id))
                    picktech_append_rule_annotation(has_rule_annotation,
                        0x274 / 4, true, dependent.name, true);
            }
            popup_d.dialogs().item(StringTemp, tech_id);
            // Native BasePop's default selection stores the sole candidate.
            if (item_count_ == 1) popup_d.set_value(tech_id);
        }
    }
    if (item_count_ == 0) return -1;
    if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (1u << faction)) == 0)
        return recommendation;

    const bool small_layout = item_count_ <= 8;
    int title_height = small_layout ? (WinScreenWidth == 800 ? 12 : 14) : 10;
    int header_height = small_layout ? (WinScreenWidth == 800 ? 14 : 18) : 12;
    title_font_.init(DefaultFontFace, title_height, 0);
    body_font_primary_.init(DefaultFontFace, title_height, 1);
    body_font_secondary_.init(DefaultFontFace, title_height, 2);
    header_font_.init(DefaultFontFace, header_height, 1);
    Buffer *const canvas = static_cast<Buffer *>(this);
    canvas->set_font(&title_font_, &body_font_primary_, &body_font_secondary_, 0);
    int columns_by_count[100] = {
        0,1,2,3,4,5,3,4,4,5,5,6,4,5,5,5,4,5,6,5,
        5,7,6,6,6,5,7,7,7,8,8,8,8,7,7,7,8,8,8,8
    };
    // Native EDI is 10 from487358 through487D37; tail stores are tens.
    for (i = 40; i < 100; ++i) columns_by_count[i] = 10;
    const int columns = columns_by_count[item_count_];
    const int rows = item_count_ / columns + (item_count_ % columns != 0);
    column_count_ = columns;
    row_count_ = rows;
    int width = small_layout ? WinScreenWidth / columns - 20
                             : (WinScreenWidth - 20) / columns;
    if (width >= WinScreenWidth / 3) width = WinScreenWidth / 3;
    cell_width_ = width;
    cell_height_ = (3 * width) / 2;
    if (cell_height_ <= WinScreenHeight / 2) cell_height_ = WinScreenHeight / 2;
    // Native bounds against rows*height+29, without adding row border pixels.
    if (rows * cell_height_ + 29 > WinScreenHeight)
        cell_height_ = small_layout ? WinScreenHeight / rows - 40
                                    : (WinScreenHeight - 69) / rows;
    row_pitch_ = 10;
    GraphicWin::init(0, 0, (cell_width_ + 1) * columns + 1,
        (cell_height_ + 1) * rows + 30, 0, 0x05000800, 0, 0, 0);
    Win::center();
    // Native487D6C is an unconditional inherited-field store. Win's public
    // setter additionally gates on flags and resizes, neither done here.
    caption_height_ = 29;
    // Native487D66 reads GraphicWin+474: Buffer+30, its owned rect2_.
    RECT frame = canvas->clip_rect();
    canvas->box_sprite(&frame, &InterfaceBoxStyles[9]);
    frame.right -= 3;
    frame.top += 3;
    frame.left += 3;
    frame.bottom = frame.top + 23;
    canvas->box_sprite(&frame, &InterfaceBoxStyles[6]);
    frame.right -= 3;
    frame.left += 3;
    frame.bottom -= 3;
    frame.top += 3;
    StringTemp[0] = '\0';
    strcat(StringTemp, popup_d.caption_text());
    CharUpperA(StringTemp);
    canvas->set_text_color(0x9F, -1, 1, 1);
    canvas->set_font(find_font(18, 1), 0, 0, 0);
    canvas->write_cent_l(StringTemp, &frame, static_cast<int>(strlen(StringTemp)));
    selected_index_ = 0;
    selection_active_ = 0;
    for (int item = 0; item < item_count_; ++item)
        draw_item(item, item == selected_index_);
    show(0);
    if (WinPointerOwner3 == 0) {
        WinPointerOwner3 = this;
        WinPointerOwner4 = 0;
    }
    set_modal(1, 0, 0);
    hide();
    do_all_draws();
    return technology_ids_[selected_index_];
}

// Native 5BEB8F..95 constructs the complete local PickTech; 5BEBB7 calls pick.
// The 484B60 close and reverse Font/base cleanup at 5BEBDF..5BEC27 are the
// caller-inlined complete destructor. Do not add a second explicit close.
// EH action662A1E calls complete PickTech::~PickTech (488690).
// ORIGINAL: 0x005BEB70 ?tech_pick@@YAHHHHPAD@Z 0x005BEB70-0x005BEC3D;0x00662A1E-0x00662A64
// RECOVERY_BINDING: v1 {"evidence": ["image:5BEB70-5BEC3D and662A1E-662A64; retired seed provenance git:820312f0:src/unrecovered/005beb70.cpp; historical FILE/TRIED lessons retained.", "image:5BEB8F constructor484AC0, 5BEBB7 pick486510, 5BEBDF close484B60 and reverse Fonts/base cleanup are the inlined complete destructor; EH state0 action662A1E calls destructor488690.", "Historical symbol tech_pick PAD is mutable char*; both current product callers pass null. Authoritative declaration removes stale const overloads without changing arguments."], "reason": "Real automatic PickTech owns complete construction, modal pick and cleanup; close belongs to its destructor, not an extra caller operation."}
// symbol    ?tech_pick@@YAHHHHPAD@Z
// TRIED: this is the project's known EH-heavy-constructor wall (see src/recovered/units/00408490.cpp's BaseWin::BaseWin note): a local `PickTech` (GraphicWin base + 4 embedded `Font`s, constructed via placement new at the offsets the disassembly uses: 0xc34/0xc0c/0xbe4/0xbbc from the object base) is a faithful call-by-call translation, but the compiler- synthesized SEH try-frame (fs:[0] chain for unwind-safe partial construction, plus the ~PickTech funclet at 0x00662A1E which the scaffold never declares because it is reached only by unwind tables, not a `call`) is not something this reproduces. Landed FILE-mode because BODY mode's naive brace-balance extraction (`extract_forward_text`) stops at the first top-level `{...}` it meets - the placement-new `operator new` helper closes before `tech_pick` even starts, so a body-mode file here silently truncates to zero external symbols (measured: NO_COMPILE, "found 0"). Not expected exact (mnemonic_similarity 0.71).
int __cdecl tech_pick(int faction, int mode, int count, char *section) {
    PickTech picker;
    return picker.pick(faction, mode, count, section);
}
