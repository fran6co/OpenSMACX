/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
// ORIGINAL: 0x005FC6D0 ??_GMenu@@UAEPAXI@Z 0x005FC6D0-0x005FC6EE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x005FC6D0-0x005FC6EE", "Pinned class constructor ??0Menu@@QAE@XZ at 0x005FAC60 stores original class/view table VA 0x6700b0 at 0x005FAC94 (mov dword ptr [esi], 0x6700b0); the current genuine owner vftable ??_7Menu@@6BWin@@@ selects generated ??_GMenu@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EMenu@@UAEPAXI@Z (mode 2).", "Pinned ??_GMenu@@UAEPAXI@Z at 0x005FC6D0 calls the source-mapped ??1Menu@@QAE@XZ at 0x005FAD00, then tests byte ptr [esp + 8], 1; je 0x5fc6e8 bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/menu.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original Menu scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GMenu@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x004454A0 _$E21 0x004454A0-0x004454B6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682090 selects init 0x004454A0, which registers exit callback 0x004454C0.", "Original init span 0x004454A0-0x004454B6 and exit span 0x004454C0-0x004454CA are independently disassembled; both identify receiver 0x00749C88 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Current menu.cpp physical COFF selects local STATIC init _$E21 via .CRT$XCU, relocating to g_MENU_UP_WAVE, Wave construction, paired local fini _$E19 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_MENU_UP_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// ORIGINAL: 0x004454C0 _$E19 0x004454C0-0x004454CA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682090 selects init 0x004454A0, which registers exit callback 0x004454C0.", "Original init span 0x004454A0-0x004454B6 and exit span 0x004454C0-0x004454CA are independently disassembled; both identify receiver 0x00749C88 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Current menu.cpp physical COFF selects local STATIC init _$E21 via .CRT$XCU, relocating to g_MENU_UP_WAVE, Wave construction, paired local fini _$E19 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_MENU_UP_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// ORIGINAL: 0x004454D0 _$E26 0x004454D0-0x004454E6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682094 selects init 0x004454D0, which registers exit callback 0x004454F0.", "Original init span 0x004454D0-0x004454E6 and exit span 0x004454F0-0x004454FA are independently disassembled; both identify receiver 0x0074D960 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Current menu.cpp physical COFF selects local STATIC init _$E26 via .CRT$XCU, relocating to g_MENU_DOWN_WAVE, Wave construction, paired local fini _$E24 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_MENU_DOWN_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// ORIGINAL: 0x004454F0 _$E24 0x004454F0-0x004454FA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682094 selects init 0x004454D0, which registers exit callback 0x004454F0.", "Original init span 0x004454D0-0x004454E6 and exit span 0x004454F0-0x004454FA are independently disassembled; both identify receiver 0x0074D960 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Current menu.cpp physical COFF selects local STATIC init _$E26 via .CRT$XCU, relocating to g_MENU_DOWN_WAVE, Wave construction, paired local fini _$E24 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_MENU_DOWN_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
#include "stdafx.h"
#include "sound.h"
#include "wave.h"
#include "original_seam.h"
#include "menu.h"
#include "pulldown.h"
#include "filewin.h"
#include "win.h"

#include "console.h"
#include "general.h"  // mem_get for Menu's owned label copies

// Their containing source object/literal identities still need a coordinated
// ownership decision; don't bind them at fixed addresses.

// Complete source-level transcriptions established so far:

namespace {
char MenuEntryPadding[] = "  ";
char MenuComputeLeftSpace[] = " ";
char MenuComputeRightSpace[] = " ";
char MenuLeftDownMargin[] = " ";
char MenuMouseMoveMargin[] = " ";
char MenuMnemonicMargin[] = " ";
char MenuRedrawMargin[] = " ";
}

// Provisional Menu-owned source objects only: the pinned image places both
// coordinates in zero-initialized data. Their complete native xref census is
// still needed before declaring these names/ownership final.
// RECOVERY_OBJECT: v1 0x009B7B80 ?MenuLastMouseY@@3HA
int MenuLastMouseY;
// RECOVERY_OBJECT: v1 0x009B7B84 ?MenuLastMouseX@@3HA
int MenuLastMouseX;
extern int MenuDefaultColor1;
extern int MenuDefaultColor2;
extern int MenuDefaultColor3;
extern int MenuDefaultSelectionColor;
extern int MenuDefaultSelectionTextColor;

// Native Menu slot 90, formerly represented by the raw Menu_init facade in
// stringstruct.cpp. The only status returns are owner-null, missing font,
// GraphicWin::init failure, or zero after the two virtual tail calls.
/*
Purpose: Size and initialise the menu bar window against its owner.
*/
// ORIGINAL: 0x005FADD0 ?init@Menu@@QAEHPAUWin@@@Z 0x005FADD0-0x005FAE69
// RECOVERY_BINDING: v1 {"evidence": ["git:5190d27f:src/stringstruct.cpp:Menu_init", "image:5FAE14..5FAE3F uses parent RECT.left/top/right at13C/140/144, subtracts twice font height from top, and propagates GraphicWin init status", "image:Menu6700B0 slots90/93 and Win slot1; inherited Buffer markup cell+964", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover the genuine virtual Menu initializer through the parent frame, borrowed default font and Buffer base. Retire the raw receiver facade without preserving its incorrectly named bottom coordinate or discarded initialization status; original identity and seed history remain obligations."}
// symbol    ?init@Menu@@UAEHPAVWin@@@Z
int Menu::init(Win *owner) {
    if (owner == 0) return 3;

    field_A28_ = 0;
    if (MenuDefaultFont == 0) {
        MenuDefaultFont = FontDefault;
        if (MenuDefaultFont == 0) return 7;
    }

    Font *const font = MenuDefaultFont;
    const RECT &bounds = owner->outer_rect();
    const int height = font->height_ * 2;
    const int result = GraphicWin::init(
        bounds.left, bounds.top - height, bounds.right - bounds.left,
        height, 0, 0x8020, owner, 0, 0);
    if (result != 0) return result;

    static_cast<Buffer *>(this)->set_markup_enabled(0);
    adjust();
    show(0);
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FAEF0 ?add_menu@Menu@@QAEHHPADH@Z 0x005FAEF0-0x005FB0FA;0x00662D22-0x00662D37
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005faef0.cpp", "image:Menu6700B0/AlphaMenu66ED88/PullDown66FF40 constructor-selected primary tables; Buffer secondary subobject+444", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover this complete operation in its genuine Menu/PullDown owner, using native constructor-selected virtual dispatch, real members and shared source-owned dependencies; preserve original identity, retained seed and historical restoration obligations."}
// symbol    ?add_menu@Menu@@QAEHHPADH@Z
// TRIED: SEH frame (push -1/push handler/fs:[0]) not reproduced by the plain operator-new + placement-new PullDown construction (same gap as the 0x00428DF0 precedent, which drops it the same way); not chased further. Field layout (entries_[idx].id/text/ mnemonic/flags/pull_down), the ampersand-mnemonic split, and the duplicate-id/table-full/alloc-fail return codes are a faithful transcription off the raw disassembly.
int Menu::add_menu(int menu_id, char *text, int left_justified) {
    ++count_;
    if (count_ > 15) {
        --count_;
        return 1;
    }
    if (!text || menu_id == -1) return 3;

    int index = 0;
    while (index < 15 && entries_[index].id != -1) {
        if (entries_[index].id == menu_id) return 1;
        ++index;
    }
    if (index == 15) return 1;

    MenuEntry &entry = entries_[index];
    entry.id = menu_id;
    if (entry.text) free(entry.text);
    entry.text = static_cast<char *>(mem_get(strlen(text) + 3));
    if (!entry.text) {
        --count_;
        return 4;
    }
    entry.text[0] = '\0';
    entry.mnemonic = 0;

    char *ampersand = strchr(text, '&');
    if (!ampersand || ampersand[1] == ' ') {
        strcat(entry.text, text);
    } else {
        char *after_ampersand = ampersand + 1;
        *ampersand = '\0';
        strcat(entry.text, text);
        *ampersand = '&';
        if (after_ampersand[0] != '&')
            entry.mnemonic = entry.text + strlen(entry.text);
        strcat(entry.text, after_ampersand);
    }
    strcat(entry.text, MenuEntryPadding);

    if (entry.pull_down) delete entry.pull_down;
    entry.pull_down = new (std::nothrow) PullDown;
    if (!entry.pull_down) {
        --count_;
        return 4;
    }
    entry.pull_down->init(this);
    entry.flags = left_justified ? 5 : 1;
    compute();
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FB5A0 ?UNK10@Menu@@QAEHHPAD@Z 0x005FB5A0-0x005FB6FD
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005fb5a0.cpp", "image:Menu6700B0/AlphaMenu66ED88/PullDown66FF40 constructor-selected primary tables; Buffer secondary subobject+444", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover this complete operation in its genuine Menu/PullDown owner, using native constructor-selected virtual dispatch, real members and shared source-owned dependencies; preserve original identity, retained seed and historical restoration obligations."}
// symbol    ?UNK10@Menu@@QAEHHPAD@Z
// TRIED: the vtable call at [vtable]+0x170 (ordinal 92, nullary) has no auto-emitted VCall shim for this address, so a small local shim (VCall92) declares the 93 ordinals it needs by position; a bare __thiscall function-pointer cast is C4234 on VC6. Landed at 0.57 mnemonic similarity - the mnemonic- for-mnemonic transcription of the ampersand/mnemonic-marker string logic is right, register scheduling around the reused ebx/ebp/edi is not.
int Menu::UNK10(int menu_id, char *text) {
    if (!text) return 3;

    int index = 0;
    while (index < 15 && entries_[index].id != -1 &&
           entries_[index].id != menu_id)
        ++index;
    if (index == 15 || entries_[index].id != menu_id) return 11;

    MenuEntry &entry = entries_[index];
    if (entry.text) free(entry.text);
    entry.text = static_cast<char *>(mem_get(strlen(text) + 3));
    if (!entry.text) {
        --count_;
        return 4;
    }
    entry.text[0] = '\0';
    entry.mnemonic = 0;

    char *ampersand = strchr(text, '&');
    if (!ampersand || ampersand[1] == ' ') {
        strcat(entry.text, text);
    } else {
        char *after_ampersand = ampersand + 1;
        *ampersand = '\0';
        strcat(entry.text, text);

        *ampersand = '&';
        if (after_ampersand[0] != '&')
            entry.mnemonic = entry.text + strlen(entry.text);
        strcat(entry.text, after_ampersand);
    }
    strcat(entry.text, MenuEntryPadding);
    compute();
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FB9C0 ?compute@Menu@@QAEXXZ 0x005FB9C0-0x005FBABD
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005fb9c0.cpp", "image:Menu6700B0/AlphaMenu66ED88/PullDown66FF40 constructor-selected primary tables; Buffer secondary subobject+444", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover this complete operation in its genuine Menu/PullDown owner, using native constructor-selected virtual dispatch, real members and shared source-owned dependencies; preserve original identity, retained seed and historical restoration obligations."}
// symbol    ?compute@Menu@@UAEXXZ
// TRIED: `(seed = DAT_9bb484, DAT_9b7b94 = seed, seed != 0)` (single load reused for both the store and the test) instead of a comma expression that rereads the global twice, which forced an extra reload. `reinterpret_cast<Spot*>(self+0xa2c)` kept INLINE at each call site, not hoisted to a named local - hoisting moves the `lea` ahead of the `DAT_9bb484` fallback branch and regresses to MISMATCH #9. Best form lands at MISMATCH #33 (deep past the loop setup); divergence is a 'mov' vs 'test' pairing whose surrounding bytes still match, not a missed field.
void Menu::compute() {
    Font *font = MenuDefaultFont;
    if (!font) {
        font = FontDefault;
        MenuDefaultFont = font;
    }
    if (!font) return;

    spot_.init(count_);
    int left_space = pixel_width() - font->width(MenuComputeLeftSpace);
    int right_space = font->width(MenuComputeRightSpace);
    const int line_height = font->height_ * 2;
    for (int i = 0; i < count_; ++i) {
        MenuEntry &entry = entries_[i];
        if ((entry.flags & 1) == 0) continue;
        const int width = font->width(entry.text);
        if ((entry.flags & 4) == 0) {
            spot_.add(i, 0, right_space, 0, width, line_height);
            right_space += width;
        } else {
            left_space -= width;
            spot_.add(i, 0, left_space, 0, width, line_height);
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FC5A0 ?adjust@Menu@@QAEXXZ 0x005FC5A0-0x005FC61B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005fc5a0.cpp", "image:5FC5E3 virtual resize;5FC5E6/5FC5EC reload font and parent;5FC607/5FC608 move to parent.left,parent.top minus twice reloaded font height", "image:5FC613 invokes Menu compute slot92", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover the genuine Menu adjustment with real virtual resize/compute and post-resize borrowed-owner reloads. Preserve original frame placement, reentrant call order and the historical byte obligation without raw vtable or self-offset shims."}
// symbol    ?adjust@Menu@@UAEXXZ
// LEVER: emitter's VCall is always nullary; declared a second, own-named vtable shim (MenuVCall) with slot003 given the real (int,int,int) signature instead of trying to redeclare the fixed one.
void Menu::adjust() {
    if (!MenuDefaultFont) {
        MenuDefaultFont = FontDefault;
    }
    if (!MenuDefaultFont) return;

    if (win_parent_) {
        resize(win_parent_->outer_rect().right - win_parent_->outer_rect().left,
               MenuDefaultFont->height_ * 2, 0);
        // Resize is virtual: the original reloads both borrowed owners after it.
        move(win_parent_->outer_rect().left,
             win_parent_->outer_rect().top - MenuDefaultFont->height_ * 2);
    }
    // Native 0x005FC613 is Menu vtable slot 92 (+0x170), not a function at
    // an executable address.
    compute();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FBC20 ?on_left_down@Menu@@QAEXHH@Z 0x005FBC20-0x005FBDAB
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fbc20.cpp", "image:Menu6700B0/AlphaMenu66ED88/PullDown66FF40 constructor-selected primary tables; Buffer secondary subobject+444", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover this complete operation in its genuine Menu/PullDown owner, using native constructor-selected virtual dispatch, real members and shared source-owned dependencies; preserve original identity, retained seed and historical restoration obligations."}
// symbol    ?on_left_down@Menu@@UAEXHH@Z
void Menu::on_left_down(int x, int y) {
    MenuSelection = -1;
    int new_index = 0;
    const int hit = spot_.check(x, y, &new_index, 0);
    int current = static_cast<int>(field_A24_);
    if (hit == -1) {
        if (current == -1) return;
    } else if (current != new_index) {
        if (current != -1) {
            PullDown *old_menu = entries_[current].pull_down;
            old_menu->hide();
            field_A24_ = 0xFFFFFFFFu;
            MenuSelection = -1;
            redraw();
            flush_input();
            do_all_tasks(old_menu);
            do_all_tasks(this);
        }
        field_A24_ = static_cast<unsigned int>(new_index);
        RECT spot_rect;
        spot_.get_rect_at(new_index, &spot_rect);
        x = spot_rect.left - text_width(MenuLeftDownMargin);
        y = -pixel_height();
        client_to_screen(&x, &y);
        redraw();
        MenuPullDownInputOrigin = 0;
        on_adjust_pulldown_pos(&x, &y); // native Menu slot 94
        entries_[static_cast<int>(field_A24_)].pull_down->show(x, y, 0);
        return;
    } else if (current == -1) {
        // The native hit can name no menu despite a successful Spot::check.
        // It joins the common close/hide tail without indexing entries_.
        if ((field_A28_ & 4) == 0) hide();
        return;
    }

    PullDown *old_menu = entries_[current].pull_down;
    old_menu->hide();
    field_A24_ = 0xFFFFFFFFu;
    MenuSelection = -1;
    redraw();
    flush_input();
    do_all_tasks(old_menu);
    do_all_tasks(this);
    if ((field_A28_ & 4) == 0) hide();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FBDB0 ?mouse_move@Menu@@QAEXHH@Z 0x005FBDB0-0x005FBEFD
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fbdb0.cpp", "image:Menu6700B0/AlphaMenu66ED88/PullDown66FF40 constructor-selected primary tables; Buffer secondary subobject+444", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover this complete operation in its genuine Menu/PullDown owner, using native constructor-selected virtual dispatch, real members and shared source-owned dependencies; preserve original identity, retained seed and historical restoration obligations."}
// symbol    ?mouse_move@Menu@@QAEXHH@Z
void Menu::mouse_move(int x, int y) {
    if (static_cast<int>(field_A24_) == -1) return;
    if (MenuLastMouseX == x && MenuLastMouseY == y) return;
    MenuLastMouseX = x;
    MenuLastMouseY = y;

    POINT point;
    point.x = x;
    point.y = y;
    ScreenToClient(HandleMain, &point);
    int new_index = 0;
    if (spot_.check(point.x, point.y, &new_index, 0) == -1 ||
        static_cast<int>(field_A24_) == new_index || (field_A28_ & 1) != 0)
        return;

    entries_[static_cast<int>(field_A24_)].pull_down->hide();
    field_A24_ = static_cast<unsigned int>(new_index);
    RECT spot_rect;
    spot_.get_rect_at(new_index, &spot_rect);
    point.x = spot_rect.left - text_width(MenuMouseMoveMargin);
    point.y = -pixel_height();
    ClientToScreen(HandleMain, &point);
    redraw();
    int left = point.x;
    int top = point.y;
    on_adjust_pulldown_pos(&left, &top); // native Menu slot 94
    entries_[static_cast<int>(field_A24_)].pull_down->show(left, top, 0);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FBF00 ?on_key_click@Menu@@QAEHHH@Z 0x005FBF00-0x005FC21F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005fbf00.cpp", "image:Menu6700B0/AlphaMenu66ED88/PullDown66FF40 constructor-selected primary tables; Buffer secondary subobject+444", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover this complete operation in its genuine Menu/PullDown owner, using native constructor-selected virtual dispatch, real members and shared source-owned dependencies; preserve original identity, retained seed and historical restoration obligations."}
// symbol    ?on_key_click@Menu@@UAEHHH@Z
// TRIED: MISMATCH #5 (mov vs or) - pure prologue scheduling: the original loads a2 into a register before `or ebp,-1`, ours after. Full control flow transcribed from Ghidra/disasm: entries_ array confirmed at self+0xa44 (mnemonic char* at +0, PullDown* at +4, stride 0x14 - matches the sibling 0x005FBDB0 mouse_move's self+0xa48 pulldown access exactly). slot023 retyped to `int` since its return value is tested.
int Menu::on_key_click(int, int key) {
    int selected_menu = static_cast<int>(field_A24_);
    const int no_menu = -1;
    if (selected_menu != no_menu && (key & 0xFFFBFFFF) == 0x12) {
        PullDown *closing = entries_[selected_menu].pull_down;
        closing->hide();
        field_A24_ = 0xFFFFFFFFu;
        MenuSelection = -1;
        redraw();

        flush_input();
        do_all_tasks(closing);
        do_all_tasks(this);
        return 1;
    }

    if (selected_menu != no_menu) {
        if (key == 0x0D) {
            PullDown *closing = entries_[selected_menu].pull_down;
            closing->hide();
            field_A24_ = 0xFFFFFFFFu;
            MenuSelection = -1;
            redraw();
            flush_input();
            do_all_tasks(closing);
            do_all_tasks(this);
            do_all_draws();

            const int item_index = closing->get_selected();
            if (item_index == -1) return 1;
            const int item_id = item_index < 0x40
                ? closing->items_[item_index].id : -1;
            hide_menus();
            if ((field_A28_ & 4) == 0) hide();
            MenuProc callback = proc_;
            if (!callback) return 1;
            callback(item_id);

            Win *parent = win_parent_;
            if (!parent) return 1;
            Sound *target = parent->event_window();
            if (!target || target->is_playing()) return 1;
            target->play();
            return 1;
        }
        if (key == 0x1B) {
            PullDown *closing = entries_[selected_menu].pull_down;
            closing->hide();
            field_A24_ = 0xFFFFFFFFu;
            MenuSelection = -1;
            redraw();
            flush_input();
            do_all_tasks(closing);
            do_all_tasks(this);
            return 1;
        }
        if (key == 0x25 || key == 0x64) {
            if (selected_menu == 0) selected_menu = count_;
            --selected_menu;
        } else if (key == 0x27 || key == 0x66) {
            if (selected_menu == count_ - 1) selected_menu = 0;
            else ++selected_menu;
        } else {
            goto search_menu_mnemonics;
        }

        RECT rect;
        spot_.get_rect_at(selected_menu, &rect);
        POINT point;
        point.x = rect.left;
        point.y = rect.top;
        ClientToScreen(HandleMain, &point);
        MenuPullDownInputOrigin = 1;
        mouse_move(point.x, point.y);
        return 1;
    }

search_menu_mnemonics:
    for (int index = 0; index < count_; ++index) {
        char *mnemonic = entries_[index].mnemonic;
        if (!mnemonic) continue;
        if (*mnemonic != tolower(key) && *mnemonic != toupper(key)) continue;

        if (static_cast<int>(field_A24_) != no_menu) {
            PullDown *closing = entries_[static_cast<int>(field_A24_)].pull_down;
            closing->hide();
            field_A24_ = 0xFFFFFFFFu;
            MenuSelection = -1;
            redraw();
            flush_input();
            do_all_tasks(closing);
            do_all_tasks(this);
        }
        field_A24_ = static_cast<unsigned int>(index);
        RECT rect;
        spot_.get_rect_at(index, &rect);
        POINT point;
        point.x = rect.left - text_width(MenuMnemonicMargin);
        point.y = -pixel_height();
        ClientToScreen(HandleMain, &point);
        flush_input();
        redraw();
        MenuPullDownInputOrigin = 1;
        entries_[index].pull_down->show(point.x, point.y, 0);
        return 1;
    }
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FC290 ?on_redraw@Menu@@QAEXXZ 0x005FC290-0x005FC594
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fc290.cpp", "image:Menu6700B0/AlphaMenu66ED88/PullDown66FF40 constructor-selected primary tables; Buffer secondary subobject+444", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover this complete operation in its genuine Menu/PullDown owner, using native constructor-selected virtual dispatch, real members and shared source-owned dependencies; preserve original identity, retained seed and historical restoration obligations."}
// symbol    ?on_redraw@Menu@@UAEXXZ
void Menu::on_redraw() {
    Font *font = MenuDefaultFont;
    if (!font) {
        font = FontDefault;
        MenuDefaultFont = font;
    }
    if (!font) return;

    // Native entry 5FC2B3 fills the window before measuring the one-space
    // inset; every later drawing operation is through the real Buffer base.
    fill(MenuDefaultColor1);
    const int margin = text_width(MenuRedrawMargin);
    RECT text_area = clip_rect();
    text_area.left += margin;
    text_area.right -= margin;
    set_font(font, 0, 0, 0);

    for (int index = 0; index < count_; ++index) {
        MenuEntry &entry = entries_[index];
        if ((entry.flags & 1) == 0) continue;

        RECT spot_rect;
        spot_.get_rect_at(index, &spot_rect);
        RECT row = text_area;
        if ((entry.flags & 4) != 0) row.right = spot_rect.right;
        else row.left = spot_rect.left;
        const bool selected = static_cast<int>(field_A24_) == index;
        int text_color;
        if (selected) {
            text_color = (entry.flags & 2) ? MenuDefaultColor1 : MenuDefaultColor2;
        } else {
            text_color = (entry.flags & 2) ? MenuDefaultColor3
                                           : MenuDefaultSelectionTextColor;
        }
        set_text_color(text_color, -1, 1, 1);
        if (selected) {
            fill(spot_rect.left, text_area.top, spot_rect.right,
                 text_area.bottom, MenuDefaultSelectionColor);
        }

        if (entry.text) {
            const int text_length = static_cast<int>(strlen(entry.text));
            const int rendered_width = text_width(entry.text);
            if ((entry.flags & 4) != 0)
                write_right_l(entry.text, &row, text_length);
            else
                write_l(entry.text, &row, text_length);

            if (entry.mnemonic) {
                const int underline_bottom = row.bottom;
                const int underline_height = row.bottom - row.top;
                const int underline_color = foreground_color();
                const int leading = font->internal_leading();
                const int letter_width = text_width(entry.mnemonic, 1);
                const int rendered_height = text_height();
                const int underline_y =
                    underline_bottom - ((underline_height - rendered_height) / 2)
                    - leading;
                const int prefix_width = text_width(
                    entry.text, static_cast<int>(entry.mnemonic - entry.text));
                const int underline_left = (entry.flags & 4)
                    ? row.right - rendered_width + prefix_width
                    : row.left + prefix_width;
                hline(underline_left, underline_left + letter_width,
                      underline_y, underline_color);
            }
        }
    }
}

// RECOVERY_OBJECT: v2 0x00696E3C 0x00696E40 ?MenuSelection@@3HA 0x005FB830
int MenuSelection = -1;
// RECOVERY_OBJECT: v1 0x009B7B94 ?MenuDefaultFont@@3PAVFont@@A
Font *MenuDefaultFont; // 0x009B7B94, shared by menu sizing and the setter
// RECOVERY_OBJECT: v1 0x00696E30 ?MenuDefaultColor1@@3HA
int MenuDefaultColor1 = 247;         // 0x00696E30, paint/text palette index
// RECOVERY_OBJECT: v1 0x00696E34 ?MenuDefaultColor2@@3HA
int MenuDefaultColor2 = 255;         // 0x00696E34, text palette index
// RECOVERY_OBJECT: v1 0x00696E38 ?MenuDefaultColor3@@3HA
int MenuDefaultColor3 = 7;           // 0x00696E38, text palette index
// RECOVERY_OBJECT: v1 0x009B7B8C ?MenuDefaultSelectionColor@@3HA
int MenuDefaultSelectionColor;       // 0x009B7B8C
// RECOVERY_OBJECT: v1 0x009B7B90 ?MenuDefaultSelectionTextColor@@3HA
int MenuDefaultSelectionTextColor;   // 0x009B7B90

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FAEE0 ?set_def_font@Menu@@QAAXPAUFont@@@Z 0x005FAEE0-0x005FAEEE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005faee0.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets", "Historical mapping included5FAEE0..5FAEEE and5F0520..5F0535; the latter is preserved as the independent Win::update_menus mapping, not discarded or treated as cold caller code.", "Pinned5FAEE9 tail-jumps to5F0520 after storing MenuDefaultFont;5F0520 has its own callable entry and terminal RET. The separate source functions already implement this relationship."], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim. Correct the old combined caller/callee extent without withdrawing the caller identity or historical byte obligation."}
// symbol    ?set_def_font@Menu@@SAXPAVFont@@@Z
void Menu::set_def_font(Font *font) {
    MenuDefaultFont = font;
    Win::update_menus();
}

// ORIGINAL: 0x005FB830 ?menu_event@Menu@@QAEHH@Z 0x005FB830-0x005FB8D8
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005FB830-0x005FB8D8"], "reason": "Recovered original behavior through real source classes and owned members; source remains subject to byte and structural verification."}
// symbol    ?menu_event@Menu@@QAEXH@Z
void Menu::menu_event(int id) {
    if (field_A24_ != 0xFFFFFFFF) {
        MenuEntry &entry = entries_[field_A24_];
        entry.pull_down->hide();
        field_A24_ = 0xFFFFFFFF;
        MenuSelection = -1;
        redraw();
        flush_input();
        do_all_tasks(entry.pull_down);
        do_all_tasks(this);
    }
    if (!(field_A28_ & 4)) hide();
    if (proc_) {
        proc_(id);
        if (win_parent_) {
            Sound *target = win_parent_->event_window();
            if (target && !target->is_playing()) target->play();
        }
    }
}

/*
Purpose: Construct the GraphicWin base and the embedded Spot, install
         Menu's own vtables, then zero the scalar fields and the 15-entry
         entries_ table.
// ORIGINAL: 0x005FAC60 ??0Menu@@QAE@XZ 0x005FAC60-0x005FACF6;0x00662CF0-0x00662D02 BYTE_EXACT
// size      150 bytes
// prototype void (__thiscall ??0Menu@@QAE@XZ)(Menu* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x005D4CF0 0x005FA860
// notes     field_A1C_ and field_A28_ are never touched by this constructor
//        (the image writes proc_, count_, field_A20_ and field_A24_ only).
// LEVER: a REAL CONSTRUCTOR, not a `construct()` method. This was
//        `Menu *Menu::construct()` doing `new (&spot_) Spot()`, and the
//        placement-new is what cost it. VC6 guards a placement new-expression
//        with a null test on the pointer - `cmp ecx, ebx; je` - because
//        `operator new` may return null, and the guard needs a spill slot.
//        That is where `sub esp, 8` came from against the image's `push ecx`.
//        The earlier note called this "a stack-frame-size difference from
//        spill-slot count, not control flow", which had the cause backwards:
//        there IS control flow, and the frame size is downstream of it.
//        As an ordinary member of a real constructor, `spot_` is constructed
//        implicitly with no guard at all - and the image agrees, calling
//        GraphicWin's real constructor at 0x005D4CF0 and then Spot's at
//        0x005FA860, which is exactly base-then-member in declaration order.
//        10/38 -> 15/38, and the compiled body drops from 42 instructions to
//        39 against the image's 38.
// LEVER: STORE ORDER inside the entries_ loop closed the last gap, and it
//        took out two divergences at once. Writing `text` before `id` (not
//        the declaration order `id`, `text`) makes VC6 aim the induction
//        pointer at `entries_[i].id` - `lea eax, [esi + 0xa38]`, stores at
//        [eax+4] then [eax] - which is what the image does; the id-first
//        spelling aimed it at `.text` and stored at [eax-4]. And the same
//        reorder stops VC6 materialising the -1 in edx (`or edx, 0xffffffff`
//        shared between field_A24_ and the fifteen `entries_[i].id`): the
//        image writes the immediate twice, once to [esi+0xa24] and once to
//        [eax] per iteration. 15/38 -> BYTE_EXACT 38/38. The earlier TRIED
//        note here said the hoisted constant was "not reachable by
//        reordering the assignments"; it was, and that note was measured
//        before the store order was tried.
Return Value: n/a
Status: Complete
*/
Menu::Menu() {
    proc_ = nullptr;
    count_ = 0;
    field_A20_ = 0;
    field_A24_ = 0xFFFFFFFF;
    for (int i = 0; i < 15; i++) {
        entries_[i].text = nullptr;
        entries_[i].id = -1;
        entries_[i].flags = 0;
        entries_[i].mnemonic = nullptr;
        entries_[i].pull_down = nullptr;
    }
}

/*
Purpose: Set the callback invoked for menu events.
// ORIGINAL: 0x005FB820 ?set_menu_proc@Menu@@QAEXP6AXH@Z@Z 0x005FB820-0x005FB82D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_menu_proc@Menu@@QAEXP6AXH@Z@Z)(Menu* this, void (__cdecl *)(int this))
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Menu::set_menu_proc(MenuProc proc) {
    proc_ = proc;
}

// The retained 0x005FAE70 seed clears the menu's entry run without tearing
// down its GraphicWin base. Menu::~Menu has the same entry ownership pattern;
// keep this operation limited to the exact Menu fields the seed resets.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FAE70 ?UNK1@Menu@@QAEXXZ 0x005FAE70-0x005FAED8
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fae70.cpp", "source:src/menu.h", "source:src/menu.cpp"], "reason": "Clear Menu-owned entry text, IDs, flags, mnemonics and PullDowns, then reset only the Menu scalar state that the retained seed clears; keep the GraphicWin base alive."}
void Menu::UNK1() {
    for (int i = 0; i < 15; ++i) {
        if (entries_[i].text != nullptr) {
            free(entries_[i].text);
        }
        entries_[i].text = nullptr;
        entries_[i].id = -1;
        entries_[i].flags = 0;
        entries_[i].mnemonic = nullptr;
        if (entries_[i].pull_down != nullptr) {
            delete entries_[i].pull_down;
        }
        entries_[i].pull_down = nullptr;
    }
    proc_ = nullptr;
    count_ = 0;
    field_A20_ = 0;
    field_A24_ = 0xFFFFFFFF;
}

/*
Purpose: Find the first menu entry with the requested ID.
// ORIGINAL: 0x005FB990 ?id_to_index@Menu@@QAEHH@Z 0x005FB990-0x005FB9B8 BYTE_EXACT
// size      40 bytes
// prototype int (__thiscall ?id_to_index@Menu@@QAEHH@Z)(Menu* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int Menu::id_to_index(int id) {
    for (int index = 0; index < 15; ++index) {
        if (entries_[index].id == -1) {
            break;
        }
        if (entries_[index].id == id) {
            return index;
        }
    }
    return -1;
}



/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005FB1C0 ?UNK2@Menu@@QAEHH@Z 0x005FB1C0-0x005FB1C5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK2@Menu@@QAEHH@Z)(Menu* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Menu::UNK2(int) {
    return 0;
}


/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005FB230 ?UNK4@Menu@@QAEHHHH@Z 0x005FB230-0x005FB235 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK4@Menu@@QAEHHHH@Z)(Menu* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Menu::UNK4(int, int, int) {
    return 0;
}



/*
Purpose: Report the height a menu requests, two lines of its font.
// ORIGINAL: 0x005FC6A0 ?requested_height@Menu@@QAEHXZ 0x005FC6A0-0x005FC6B4 BYTE_EXACT
// symbol    ?requested_height@Menu@@UAEHXZ
// size      20 bytes
// prototype int (__thiscall ?requested_height@Menu@@QAEHXZ)(Menu* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Twice the resolved font's height
Status: Complete
*/
int Menu::requested_height() {
    // The menu's own font wins when set; otherwise the process default is
    // resolved at the point of use rather than cached.
    Font *font = MenuFont();
    if (!font) {
        font = FontDefault;
    }
    return font->height_ * 2;
}


/*
 * The seven menu-item operations below are ONE function seven times over: the
 * image holds them as 84-byte clones differing at exactly two byte offsets,
 * the displacement of the single call at the end. Everything else - the
 * inlined search, the two 0xB exits, the entry stride - is byte-identical
 * across all seven, which is why they are transcribed as a family rather than
 * derived one at a time.
 *
 * The search is INLINED in each, not delegated to Menu::id_to_index, because
 * that is what the original does: id_to_index exists separately at 0x005FB280
 * and every one of these carries its own copy of the same loop. Factoring
 * them onto a shared helper would leave the mutation sweep with one body to
 * perturb instead of seven.
 */

/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB1D0 ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0-0x005FB224 BYTE_EXACT
// size      84 bytes
// prototype int (__thiscall ?UNK3@Menu@@QAEHHH@Z)(Menu* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8C90
Return Value: PullDown::UNK2's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original,
         and transcribed: see the BUG IN THE ORIGINAL note on the body.
*/
int Menu::UNK3(int menu_id, int item_id) {
    int index;
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own inline
    // 0xB epilogue and the break edge lands on the dead `-1` test below.
    // BYTE_EXACT here, where the range-bounded
    // `for (index = 0; index < 15; index++)` spelling left a residual range
    // compare on the found path (30 of 32) and the committed `for(;;)` plus
    // `if (++index >= 15)` form peeled the first entry read above the
    // callee-saved pushes (1 of 32).
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // (0x005FB1FC) before dispatching. The index is a loop counter that starts
    // at 0 and only increments, so this branch can never fire. Transcribed
    // anyway: it is the branch point the found path really takes, and every
    // live spelling of it (the range test) cost the two instructions. Left
    // alone deliberately - fixing it breaks the match.
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->UNK2(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB300 ?hide_menu_item@Menu@@QAEHHH@Z 0x005FB300-0x005FB354 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::hide_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::hide_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->hide_item(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB3C0 ?show_menu_item@Menu@@QAEHHH@Z 0x005FB3C0-0x005FB414 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::show_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::show_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->show_item(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB480 ?disable_menu_item@Menu@@QAEHHH@Z 0x005FB480-0x005FB4D4 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::disable_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::disable_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->disable_item(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB540 ?enable_menu_item@Menu@@QAEHHH@Z 0x005FB540-0x005FB594 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::enable_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::enable_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->enable_item(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB760 ?check_menu_item@Menu@@QAEHHH@Z 0x005FB760-0x005FB7B4 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::check_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::check_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->check_item(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB7C0 ?uncheck_menu_item@Menu@@QAEHHH@Z 0x005FB7C0-0x005FB814 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::uncheck_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::uncheck_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->uncheck_item(item_id);
}


/*
Purpose: Find the menu whose id matches and add a text item to its PullDown.
         An unknown menu answers 0xB, and so does a table that runs out before
         matching.
// ORIGINAL: 0x005FB100 ?add_menu_item@Menu@@QAEHHHPAD@Z 0x005FB100-0x005FB159 BYTE_EXACT
Return Value: PullDown::add_item's, or 0xB when no entry matches
Status: Complete
*/
int Menu::add_menu_item(int menu_id, int item_id, char *text) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: same body as the seven-clone family - the range guard at the TOP
    // of a `for (;;)` body, and the dead `-1` test below it. Written straight
    // from ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0), which holds the six spellings
    // measured for this shape.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead. Left alone deliberately.
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->add_item(item_id, text);
}


/*
Purpose: Find the menu whose id matches and append a separator to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching; a match answers 0 whatever the PullDown said.
// ORIGINAL: 0x005FB160 ?add_separator@Menu@@QAEHHH@Z 0x005FB160-0x005FB1B6 BYTE_EXACT
Return Value: 0, or 0xB when no entry matches
Status: Complete
*/
int Menu::add_separator(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead. Left alone deliberately - see
    // ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
    if (index == -1) {
        return 0xB;
    }
    // The image zeroes eax after the call: add_separator's answer is
    // discarded and this always answers 0.
    entries_[index].pull_down->add_separator(item_id);
    return 0;
}


/*
Purpose: Find the menu whose id matches and rename an item of its PullDown.
         An unknown menu answers 0xB, and so does a table that runs out before
         matching.
// ORIGINAL: 0x005FB700 ?rename_menu_item@Menu@@QAEHHHPAD@Z 0x005FB700-0x005FB759 BYTE_EXACT
Return Value: PullDown::rename_item's, or 0xB when no entry matches
Status: Complete
*/
int Menu::rename_menu_item(int menu_id, int item_id, char *text) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead. Left alone deliberately - see
    // ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->rename_item(item_id, text);
}


/*
Purpose: Find the menu whose id matches and answer the item index its
         PullDown holds for the item id. An unknown menu answers -1, and so
         does a table that runs out before matching.
// ORIGINAL: 0x005FB8E0 ?UNK11@Menu@@QAEHHH@Z 0x005FB8E0-0x005FB930 BYTE_EXACT
// size      80 bytes
// prototype int (__thiscall ?UNK11@Menu@@QAEHHH@Z)(Menu* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F9D00
Return Value: PullDown::id_to_index's, or -1 when no entry matches
Status: Complete
*/
int Menu::UNK11(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return -1;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return -1;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline -1 epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return -1;
    }
    return entries_[index].pull_down->id_to_index(item_id);
}


/*
Purpose: Find the menu whose id matches and answer the id its PullDown holds
         in the item slot. An unknown menu answers -1, and so does a table that
         runs out before matching.
// ORIGINAL: 0x005FB930 ?UNK12@Menu@@QAEHHH@Z 0x005FB930-0x005FB981 BYTE_EXACT
Return Value: the entry's PullDown item id, or -1 when nothing matches
Status: Complete
*/
int Menu::UNK12(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return -1;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return -1;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard at the TOP of a `for (;;)` body, as the seven-
    // clone family (see ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0). What this body adds
    // is the slot bound, and the image merges its exit with the dead `-1`
    // test's - ONE -1 epilogue at the loop's exhausted edge, both tests
    // jumping back to it (`je 0x5fb952`, `jge 0x5fb952`). Three separate
    // `return -1;` statements did NOT merge: VC6 emitted three epilogues, one
    // spelled `or eax, eax` for the path where it knows the index is already
    // -1 (18 of 30 against this). The chained spelling is what reproduces the
    // image's shared exit - the mirror of the family's "test them separately"
    // rule, which is a rule about what the image shows, not about `||`.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so that half of the test is dead. Left alone
    // deliberately - fixing it breaks the match.
    if (index == -1 || item_id >= 64) {
        return -1;
    }
    return entries_[index].pull_down->items_[item_id].id;
}



/*
Purpose: Find the menu whose id matches and forward three arguments to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB240 ?UNK5@Menu@@QAEHHHHH@Z 0x005FB240-0x005FB29E BYTE_EXACT
// size      94 bytes
// prototype int (__thiscall ?UNK5@Menu@@QAEHHHHH@Z)(Menu* this, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8D90
Return Value: PullDown::UNK3's, or 0xB when no entry matches
Status: Complete
*/
int Menu::UNK5(int menu_id, int a2, int a3, int a4) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->UNK3(a2, a3, a4);
}


/*
 * A second Menu clone family: four 96-byte bodies differing at two byte
 * offsets, which are the flag instruction itself - `and dl, 0xFE`,
 * `or dl, 1`, `or dl, 2`, `and dl, 0xFD`. Two bits, set and cleared. As with
 * the item operations above, the search is inlined in each rather than
 * delegated, because the original carries four copies of it.
 */

/*
Purpose: Find the menu whose id matches and clear bit 0 of its entry's flag byte,
         then let the window repaint itself. An unknown menu answers 0xB.
// ORIGINAL: 0x005FB2A0 ?UNK6@Menu@@QAEHH@Z 0x005FB2A0-0x005FB300 BYTE_EXACT
// size      96 bytes
// prototype int (__thiscall ?UNK6@Menu@@QAEHH@Z)(Menu* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x005FB2F3
Return Value: 0 once the flag is written, 0xB when no entry matches
Status: Complete
Verification note: the repaint is a virtual dispatch on THIS object's own
         vtable - `mov edx, [ecx]` / `call [edx + 0xF8]` - not on the entry or
         its PullDown, and it takes no arguments. It runs only on the found
         path, after the flag is written. That is Win's slot 62, so the call is
         spelled `redraw()`; the ORIGINAL()/vtable-slot shim this body used
         before compiled to the same two instructions and is gone, with its
         `reinterpret_cast` of `this`.
Verification note: as in the item operations, the image's post-search test
         compares the index against -1 and cannot fire. Dead in the original,
         and transcribed: see the BUG IN THE ORIGINAL note on the body.
*/
int Menu::UNK6(int menu_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    entries_[index].flags &= 0xFE;
    redraw();
    return 0;
}


/*
Purpose: Find the menu whose id matches and set bit 0 of its entry's flag byte,
         then let the window repaint itself. An unknown menu answers 0xB.
// ORIGINAL: 0x005FB360 ?UNK7@Menu@@QAEHH@Z 0x005FB360-0x005FB3C0 BYTE_EXACT
// size      96 bytes
// prototype int (__thiscall ?UNK7@Menu@@QAEHH@Z)(Menu* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005FB3B3
Return Value: 0 once the flag is written, 0xB when no entry matches
Status: Complete
Verification note: the repaint is a virtual dispatch on THIS object's own
         vtable - `mov edx, [ecx]` / `call [edx + 0xF8]` - not on the entry or
         its PullDown, and it takes no arguments. It runs only on the found
         path, after the flag is written. That is Win's slot 62, so the call is
         spelled `redraw()`; the ORIGINAL()/vtable-slot shim this body used
         before compiled to the same two instructions and is gone, with its
         `reinterpret_cast` of `this`.
Verification note: as in the item operations, the image's post-search test
         compares the index against -1 and cannot fire. Dead in the original,
         and transcribed: see the BUG IN THE ORIGINAL note on the body.
*/
int Menu::UNK7(int menu_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    entries_[index].flags |= 0x01;
    redraw();
    return 0;
}


/*
Purpose: Find the menu whose id matches and set bit 1 of its entry's flag byte,
         then let the window repaint itself. An unknown menu answers 0xB.
// ORIGINAL: 0x005FB420 ?UNK8@Menu@@QAEHH@Z 0x005FB420-0x005FB480 BYTE_EXACT
// size      96 bytes
// prototype int (__thiscall ?UNK8@Menu@@QAEHH@Z)(Menu* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005FB473
Return Value: 0 once the flag is written, 0xB when no entry matches
Status: Complete
Verification note: the repaint is a virtual dispatch on THIS object's own
         vtable - `mov edx, [ecx]` / `call [edx + 0xF8]` - not on the entry or
         its PullDown, and it takes no arguments. It runs only on the found
         path, after the flag is written. That is Win's slot 62, so the call is
         spelled `redraw()`; the ORIGINAL()/vtable-slot shim this body used
         before compiled to the same two instructions and is gone, with its
         `reinterpret_cast` of `this`.
Verification note: as in the item operations, the image's post-search test
         compares the index against -1 and cannot fire. Dead in the original,
         and transcribed: see the BUG IN THE ORIGINAL note on the body.
*/
int Menu::UNK8(int menu_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    entries_[index].flags |= 0x02;
    redraw();
    return 0;
}


/*
Purpose: Find the menu whose id matches and clear bit 1 of its entry's flag byte,
         then let the window repaint itself. An unknown menu answers 0xB.
// ORIGINAL: 0x005FB4E0 ?UNK9@Menu@@QAEHH@Z 0x005FB4E0-0x005FB540 BYTE_EXACT
// size      96 bytes
// prototype int (__thiscall ?UNK9@Menu@@QAEHH@Z)(Menu* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x005FB533
Return Value: 0 once the flag is written, 0xB when no entry matches
Status: Complete
Verification note: the repaint is a virtual dispatch on THIS object's own
         vtable - `mov edx, [ecx]` / `call [edx + 0xF8]` - not on the entry or
         its PullDown, and it takes no arguments. It runs only on the found
         path, after the flag is written. That is Win's slot 62, so the call is
         spelled `redraw()`; the ORIGINAL()/vtable-slot shim this body used
         before compiled to the same two instructions and is gone, with its
         `reinterpret_cast` of `this`.
Verification note: as in the item operations, the image's post-search test
         compares the index against -1 and cannot fire. Dead in the original,
         and transcribed: see the BUG IN THE ORIGINAL note on the body.
*/
int Menu::UNK9(int menu_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    entries_[index].flags &= 0xFD;
    redraw();
    return 0;
}


/*
Purpose: Legacy hook for adjusting a pull-down's position. The original body is
         a single `ret 8`: it accepts two pointers, reads neither, writes
         neither, and returns. Recovered as an empty body rather than left
         unrecovered, because "does nothing" is a behaviour the catalogue can
         own and a trap cannot.

         The mangled name and the stack cleanup AGREE here, which is why this
         one is safe where ?on_redraw@Win@@QAEHXZ beside it in the queue is
         not: on_redraw declares no parameters and still cleans eight bytes, so
         its arity is unresolved and it stays unrecovered until that is
         settled.
// ORIGINAL: 0x005FC6C0 ?on_adjust_pulldown_pos@Menu@@QAEXPAH0@Z 0x005FC6C0-0x005FC6C3 BYTE_EXACT
// symbol    ?on_adjust_pulldown_pos@Menu@@UAEXPAH0@Z
// size      3 bytes
// prototype void (__thiscall ?on_adjust_pulldown_pos@Menu@@QAEXPAH0@Z)(Menu* this, int*, int*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Menu::on_adjust_pulldown_pos(int *, int *) {
}

// Close the highlighted pull-down and publish the cleared selection through
// the same redraw, input-flush, and task-pump sequence used by menu events.
// ORIGINAL: 0x005FC620 ?hide_menus@Menu@@QAEXXZ 0x005FC620-0x005FC67C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retained 0x005FC620 seed, typed MenuEntry and PullDown owners, MenuSelection source global, redraw and task-pump callees", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/005fc620.cpp"], "reason": "Home the highlighted pull-down close sequence in Menu owner source Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void Menu::hide_menus() {
    if (field_A24_ == -1) {
        return;
    }

    MenuEntry &entry = entries_[field_A24_];
    entry.pull_down->hide();
    field_A24_ = -1;
    MenuSelection = -1;
    redraw();
    flush_input();
    do_all_tasks(entry.pull_down);
    do_all_tasks(this);
}

// ORIGINAL: 0x005FBAC0 ?on_left_up@Menu@@QAEXHH@Z 0x005FBAC0-0x005FBC1A
// RECOVERY_BINDING: v1 {"evidence": ["Retained 0x005FBAC0 seed, typed MenuEntry and PullDown item storage, Spot, callback, window parent and task helpers"], "reason": "Home the Menu release and selected-item dispatch in its owner translation unit"}
// symbol    ?on_left_up@Menu@@UAEXHH@Z
void Menu::on_left_up(int xCoord, int yCoord) {
    if (field_A24_ == -1) {
        return;
    }

    if (spot_.check(xCoord, yCoord, &yCoord, 0) != -1) {
        return;
    }

    const int highlighted = field_A24_;
    hide_menus();
    do_all_draws();

    PullDown *pull_down = entries_[highlighted].pull_down;
    const int selected = pull_down->get_selected();
    if (selected == -1) {
        return;
    }

    int selected_id = -1;
    if (selected < 0x40) {
        selected_id = pull_down->items_[selected].id;
    }

    hide_menus();
    if ((field_A28_ & 4) == 0) {
        hide();
    }

    MenuProc callback = proc_;
    if (callback == 0) {
        return;
    }
    callback(selected_id);

    if (win_parent_ == 0) {
        return;
    }
    Sound *target = win_parent_->event_window();
    if (target != 0 && !target->is_playing()) {
        target->play();
    }
}


/*
// ORIGINAL: 0x005FC680 ?on_mouse_move@Menu@@QAEXHH@Z 0x005FC680-0x005FC692 BYTE_EXACT
// symbol    ?on_mouse_move@Menu@@UAEXHH@Z
// size      18 bytes
// prototype void (__thiscall ?on_mouse_move@Menu@@QAEXHH@Z)(Menu* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FBDB0
Status: Complete
*/
void Menu::on_mouse_move(int a1, int a2) {
    mouse_move(a1, a2);
}

/*
Purpose: Step the receiver back to the subobject ??_GMenu@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x005FC6F0 ??_GMenu@@WEEE@AEPAXI@Z 0x005FC6F0-0x005FC6FB BYTE_EXACT
// symbol    ??_EMenu@@WEEE@AEPAXI@Z
// CORRECTED from ??3Menu@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005FC6D0` into ??_GMenu@@UAEPAXI@Z,
//   which executes `ret 4`; no stack access and the receiver stays in
//   ECX. `WEEE@` re-demangles to adjustor{1092} and 1092 == 0x444, the
//   constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
// FX::init loads and configures these existing menu sound owners.
// RECOVERY_OBJECT: v1 0x00749C88 ?g_MENU_UP_WAVE@@3VWave@@A
Wave g_MENU_UP_WAVE;  // 0x00749C88
// RECOVERY_OBJECT: v1 0x0074D960 ?g_MENU_DOWN_WAVE@@3VWave@@A
Wave g_MENU_DOWN_WAVE;  // 0x0074D960
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

/*
Purpose: Tear the menu bar down: free every entry's label text, reset the
         entry (id, flags, mnemonic), release the entry's pull-down through
         its virtual deleting destructor, and return the menu to its
         just-constructed state - no proc, no items, no selection.

// ORIGINAL: 0x005FAD00 ??1Menu@@QAE@XZ 0x005FAD00-0x005FADC1;0x00662D02-0x00662D22 FILE
// symbol    ??1Menu@@UAE@XZ
// size      225 bytes
// prototype void (__thiscall ??1Menu@@QAE@XZ)(Menu* this)
// callers   3   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x005FA870 0x00644EF2
// indirect  0x005FAD6E
// TRIED (seed): the fs:[0] SEH frame is reachable with real Spot/GraphicWin
//         members and auto member/base destruction - entries_[15] cleanup is
//         hand-written (free()/vtable-slot0 delete), MenuEntry has no dtor;
//         bytes otherwise matched exactly 193/193, with only the pull_down
//         null-check hoisted one step early in the working copy.
// Homed from src/unrecovered/005fad00.cpp. The per-entry walk matches the
// constructor's own reset loop field for field (menu.cpp above); the image
// reaches each pull-down through vtable slot 0 with flag one, which
// `delete` spells. The two stores at offsets 0 and 0x444 are the
// destruction-stage table stores (the seed's fixed-global values), emitted
// with the rest of the destructor machinery.
Status: Coverage transcription - unclaimed body
*/
Menu::~Menu() {
    for (int i = 0; i < 15; i++) {
        if (entries_[i].text != nullptr) {
            free(entries_[i].text);
        }
        entries_[i].text = nullptr;
        entries_[i].id = -1;
        entries_[i].flags = 0;
        entries_[i].mnemonic = nullptr;
        if (entries_[i].pull_down != nullptr) {
            delete entries_[i].pull_down;
        }
        entries_[i].pull_down = nullptr;
    }
    proc_ = nullptr;
    count_ = 0;
    field_A20_ = 0;
    field_A24_ = 0xFFFFFFFF;
}
