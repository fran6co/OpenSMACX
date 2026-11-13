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
#define NETDAEMON_RESET_INLINE  // stand-in: see NetDaemon::reset in netdaemon.h
#define DIALOGS_DTOR_INLINE  // stand-in: see ~Dialogs in dialogs.h
#define POPUP_WRAPPERS_INLINE  // stand-in: see the BasePop wrappers in basepop.h
#include "stdafx.h"
#include "game.h"
#include "interface_palette.h"
#include "main.h"
#include "win.h"
#include "alpha.h"
#include "faction.h"
#include "base.h"
#include "map.h"
#include "path.h"
#include "text.h"
#include "general.h"
#include "strings.h"
#include "popup.h"
#include "xpops.h"
#include "setupwin.h"
#include "pickwin.h"
#include "font.h"
#include "popup_fonts.h"
#include "basepop.h"
#include "graphicwin.h"
#include "palette.h"
#include "wave.h"
#include "console.h"
#include "designwin.h"
#include "tutwin.h"
#include "worldwin.h"
#include "multiwin.h"
#include "maininterface.h"
#include "mapwin.h"
#include "veh.h"
#include "netwin.h"
#include "netdaemon.h"
#include "lock.h"
#include "log.h"
#include "time.h"
#include "editgroup.h"
#include "statuswin.h"
#include "flic.h"

// Existing process-wide audio owner defined in wave.cpp.
extern Wave g_TOP_MENU_WAVE;
void __cdecl world_build();

// New-game and menu ownership group; original TU boundary is not proven.
// Independent DWORD after FastUnitAnim; native map/size callers reset/set it.
// Provisional descriptive name; complete lifecycle is not established.
// RECOVERY_OBJECT: v2 0x00945820 0x00945824 ?MapSetupStatus@@3HA 0x0058D6B0
int MapSetupStatus;

static const char WorldSizePopupLabel[] = "WORLDSIZE";
static const char CustomMapPopupLabel[] = "CUSTOMMAP";
static const char CustomMapXKey[] = "Custom Map X";
static const char CustomMapYKey[] = "Custom Map Y";
static const char VeryLargeMapLabel[] = "VERYLARGEMAP";

// Newgame-owned shared faction preview and callback state.
// Generated initializer and destructor bindings below identify this global's startup lifetime.
// RECOVERY_OBJECT: v2 0x009403E0 0x0094575C ?ConfigFactionPreview@@3VPopup@@A 0x00589870
// ORIGINAL: 0x00589870 sub_589870 0x00589870-0x00589886 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: original initializer 0x00589870-0x00589886 constructs the Popup stored at 0x009403E0 and registers original teardown 0x00589890.", "Source owner: newgame.cpp declares and uses the genuine Popup ConfigFactionPreview at 0x009403E0; the global, not a fixed-address reinterpretation, owns this lifetime.", "The emitted initializer _$E21 references ConfigFactionPreview and Popup's constructor, then registers _$E19 with atexit; _$E19 references the same global and Popup destructor.", "The archived seed's prior general.cpp placement and compiler-spelling trials are historical; the complete original mapping and claim remain live here for restoration measurement."], "reason": "Bind the original Preview startup constructor to the real global Popup in newgame.cpp and the compiler-emitted constructor/atexit lifetime pair. The prior fixed-address experiment is superseded by this typed owner."}
// symbol    _$E16
// TRIED: homed into general.cpp (its real TU) 2026-09-03. Two source forms were
// measured there and both are archived here verbatim:
//   * as written (placement new + _atexit) it does not compile in the TU -
//     libc.lib has no _atexit, and the placement new buys VC6's null guard;
//   * retyped to a qualified constructor call plus plain atexit it compiles and
//     measures 5 of 6 instructions, the one divergence being VC6 cleaning the
//     atexit push with 'add esp,4' where the image has 'pop ecx'. general.cpp
//     compiles under /Oy- and every cleanup spelling tried kept 'add esp,4';
//     'pop ecx' is the /Oy form. Reverted so the BYTE_EXACT claim stays live
//     here rather than reading REGRESSED there. The qualified-ctor + atexit
//     spelling is the right one; it needs a TU on the /Oy flag set.
Popup ConfigFactionPreview;
// ORIGINAL: 0x00589890 sub_589890 0x00589890-0x0058989A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: original callback 0x00589890-0x0058989A destroys the Popup at 0x009403E0 registered by 0x00589870.", "Source owner: ConfigFactionPreview is the genuine Popup global declared and consumed by newgame.cpp; its lifetime is not independently owned by general.cpp.", "The emitted helper _$E19 references ConfigFactionPreview and Popup's destructor and is the callback registered by the same translation unit's _$E21 global initializer.", "The original identity, complete span and historical claim are preserved on the source-owned FILE mapping; the retired wrapper is not treated as independent recovered behavior."], "reason": "Bind the original ConfigFactionPreview teardown identity to the compiler-generated destructor callback of newgame.cpp's real Popup global, replacing the fixed-address wrapper in general.cpp."}
// symbol    _$E14
// RECOVERY_OBJECT: v2 0x00945818 0x0094581C ?ConfigFactionDetailMode@@3HA 0x005898A0
int ConfigFactionDetailMode;
// The image uses this zero-filled byte string only as initial prompt text.
// The adjacent PronounBuffer is independently owned; no overlay is introduced.
// RECOVERY_OBJECT: v1 0x00945834 ?ConfigRandomLeaderInitial@@3PADA
char ConfigRandomLeaderInitial[4];

// ORIGINAL: 0x005898A0 ?thumb_routine@@YAXH@Z 0x005898A0-0x00589A24 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/005898a0.cpp", "pinned:5898A0..589A24; preview9403E0; detail945818; diplomacy art78EE48 with65C faction stride", "source:Popup537C and PickWin redraw/ListBox cdecl(int) callback contract; allocation/lifetime selection still requires native qualification"], "reason": "Typed shared faction preview callback with real Popup/SpriteBox/FactionArt owners and genuine global storage; no executable-address aliases or manual vtables."}
// RECOVERY_LITERAL: v1 0x0068F5C0 0x0068F5CB ??_C@_0L@GOCJ@DATALINKS1?$AA@ 0x005898A0
// RECOVERY_LITERAL: v1 0x0068F5CC 0x0068F5D2 ??_C@_05BHI@BLURB?$AA@ 0x005898A0
// RECOVERY_LITERAL: v1 0x0068F5D4 0x0068F5DF ??_C@_0L@JBJA@DATALINKS2?$AA@ 0x005898A0
void __cdecl thumb_routine(int faction) {
// RECOVERY_OBJECT: v1 0x0068F5B8 _?preview_skip_filename@?1??thumb_routine@@YAXH@Z@4PADA
    static char preview_skip_filename[] = "JENN282";
    if (ExpansionEnabled) {
        PickWinGlobal.GraphicWin::redraw();
        if (strcmp(Players[faction].filename, preview_skip_filename) == 0) {
            ConfigFactionPreview.close();
            return;
        }
    }
    StringTemp[0] = 0;
    if (ConfigFactionDetailMode)
        strcat(StringTemp, "DATALINKS1");
    else
        strcat(StringTemp, "BLURB");
    ConfigFactionPreview.start(Players[faction].filename, StringTemp, -1, 0, 0x10500, 0);
    if (ConfigFactionDetailMode) {
        StringTemp[0] = 0;
        strcat(StringTemp, "DATALINKS2");
        ConfigFactionPreview.start(Players[faction].filename, StringTemp, -1, 0, 0x80, 0);
    }
    Sprite *const diplomacy = &FactionsArt[faction].diplomacy;
    ConfigFactionPreview.set_caller_picture(ConfigFactionPreview.picture_enabled()
        ? 0 : diplomacy);
    ConfigFactionPreview.dialogs().SpriteBox::sprite(diplomacy, 0, 0);
    ConfigFactionPreview.Win::set_caption(Players[faction].formal_name_faction);
    ConfigFactionPreview.set_string_font(PopupMenuFont, PopupMenuBoldFont, PopupMenuItalicFont, 0);
    PopupModalActive = 1;
    ConfigFactionPreview.set_loc(5, 0x136);
    ConfigFactionPreview.BasePop::exec(0, 0);
    PopupModalActive = 0;
}

/*
Purpose: Swap the opening logo the desktop shows: load the PCX named a1 into
         a local Buffer, blit it into the opening GraphicWin's Buffer base
         from the decoded DIB, then call the window's virtual on_update
         (Win slot 63) so it repaints.
         Returns 7 when no opening window is up.

// ORIGINAL: 0x00589A30 ?change_opening@@YAHPAD@Z 0x00589A30-0x00589B1D;0x00660255-0x0066026A BYTE_EXACT
// TRIED (seed): reproducing the SEH frame/unwind funclet for the local
//            `Buffer buf` explicitly - a real local of non-trivial type
//            (ctor/dtor declared, undefined) makes MSVC generate its own
//            construction/destruction and unwind automatically; landed
//            relying on that instead of hand-writing the funclet.
// size      258 bytes
// prototype int (__cdecl ?change_opening@@YAHPAD@Z)(int8* a1)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7DE0 0x005D9610
Return Value: 0 when the opening swapped, 7 when there is no opening window
The second ORIGINAL span is the local Buffer's unwind funclet and scope
descriptor. opening_window() still reaches original storage; its producer and
lifetime consumers need a coordinated migration. The window is reloaded after
load_pcx and before on_update, as in the image. The retained seed's SEH attempt
relies on the compiler-generated unwind for the real local Buffer rather than
hand-writing a funclet.
*/
int __cdecl change_opening(char *filename) {
    Buffer buf;
    if (opening_window() == 0) {
        return 7;
    }
    buf.load_pcx(filename, 0, 10, 0xec);
    GraphicWin *window = opening_window();
    // The dimensions use GraphicWin itself after the load callback; only the
    // Buffer copy argument conversion is null-conditional.
    buf.copy(window, 0, 0, buf.pixel_width(), buf.pixel_height(), 0, 0,
             window->pixel_width(), window->pixel_height());
    opening_window()->on_update();
    return 0;
}



/*
Purpose: Publish the Console's Win base for popup parenting, then destroy
         the owned opening window and clear its pointer.
// ORIGINAL: 0x00589B20 ?close_opening@@YAXXZ 0x00589B20-0x00589B57 FILE BYTE_EXACT
// size      55 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00589B4A
// The original converts Console through MapWin's virtual GraphicWin base.
// At 0x00589B4A it calls the opening object's scalar deleting destructor
// with flag one, not on_dialog_focus. GraphicWin's original primary table
// at 0x0066FC50 selects 0x005D7140, which destroys and frees the allocation.
Return Value: n/a
*/
void __cdecl close_opening() {
    Win *console_window = &ConsoleGlobal;
    GraphicWin *opening = opening_window();
    // Publish the ordinary C++ conversion to the fallback parent window.
    BasePopDefaultField322C = console_window;
    if (opening != 0) {
        delete opening;
        opening_window() = 0;
    }
}


// ORIGINAL: 0x00589B60 ?init_opening@@YAHPBD@Z 0x00589B60-0x00589D29;0x0066026A-0x0066028A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:opening-window-lifetime-owner", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:589B60..589D29;66026A..66028A", "source:src/unrecovered/00589b60.cpp complete original identity/spans and seed lessons retained; Buffer::pixel_height already negates the top-down DIB height."], "reason": "Recover opening-window construction, loading, presentation and popup-parent publication through genuine GraphicWin and Buffer lifetimes. Each callback-sensitive shared-slot reload is independent; no added null guard or failure-path behavior change."}
int __cdecl init_opening(const char *filename) {
    if (OpeningWindow == 0) {
        GraphicWin *const window = new GraphicWin();
        OpeningWindow = window;
        if (window == 0) {
            return 4;
        }

        const int result = window->init(0, 0, WinScreenWidth, WinScreenHeight,
                                        0, 0x400000, 0, 0,
                                        0);
        if (result != 0) {
            close_opening();
            return result;
        }
    }

    OpeningWindow->fill(0);
    {
        Buffer local_buffer;
        if (OpeningWindow != 0) {
            local_buffer.load_pcx(filename, 0, 10, 0xEC);

            // Native reload after load_pcx supplies the copy destination and
            // its metrics. The null arm only nulls the copy destination; the
            // following metrics reads are unconditional in the original.
            GraphicWin *const copy_window = OpeningWindow;
            Buffer *const destination =
                copy_window != 0 ? static_cast<Buffer *>(copy_window) : 0;
            const int target_width = copy_window->pixel_width();
            const int target_height = copy_window->pixel_height();
            local_buffer.copy(destination, 0, 0, local_buffer.pixel_width(),
                              local_buffer.pixel_height(), 0, 0, target_width,
                              target_height);

            // The native body reloads the slot after copy, independently of
            // the receiver used above, before virtual on_update dispatch.
            OpeningWindow->on_update();
        }
    } // Buffer's natural destructor runs before palette realization.

    g_PALETTE1.set();
    OpeningWindow->show(0);
    do_all_draws();
    BasePopDefaultField322C = static_cast<Win *>(OpeningWindow);
    return 0;
}


// ORIGINAL: 0x00589D30 ?config_game@@YAHH@Z 0x00589D30-0x0058C29D;0x0066028A-0x006605A5
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/unrecovered/00589d30.cpp; existing original identity and useful lessons preserved", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:589D30..58C29D and66028A..6605A5", "pinned:5898A0..589A24 typed cdecl(int) preview callback; ListBox dispatch retains current declaration", "source:BasePop +A44 value setter,+3100 input poll state; Popup picture/dialog/font and PickWin owned ListBox", "pinned:58A9BA passes zero-filled945834 initial string, allocation extent remains unknown", "source:recovery/tests/test_config_game_actual_source.py actual installed body, bounded API fixture and six compile-success runtime damage controls", "source:589870 initializer and589890 teardown identities now bind to the generated ConfigFactionPreview lifetime in this owner"], "reason": "Complete new-game rules, faction selection, custom naming, PBEM/network and world/setup flow with genuine Popup/SetupWin automatic local lifetimes and a source-owned shared preview. Original undefined and unchecked paths and historical main/cleanup spans remain restoration obligations; no byte or runtime acceptance is granted."}
// TRIED: mismatch #0 - only the entry (a1&1 defaults shortcut) and the first two do_menu() results (0=default_rules toggle, 1=difficulty rules-flag encode) are transcribed from the 2618-instruction original; the disassembly shows several more do_menu() results driving a PickWin faction picker, Wave/Sound playback, EditGroup name entry and a time_controls_dialog that are NOT modelled - this body loops back to do_menu() for any other result rather than fabricating their bit-flag encodings.
// Full config_game candidate. Original identities/claims remain on the owning
// TU; this ignored include is a body proposal, not a measurement or annotation.
int __cdecl config_game(int mode) {
    Popup popup;
    SetupWin setup;
    int selected;       // [ebp-0x14]
    int human_mask;     // [ebp-0x10]
    int time_choice;    // register only (0x0058A5B2 -> 0x0058B98A)
    char gender_text[0x50];     // [ebp-0xA8]
    char first_special[0x18];   // [ebp-0x58]

    if (mode & 1) {
        GameDataStateStorage.fields_.diff_level_current_ = DefaultPrefs.difficulty;
        GameDataStateStorage.fields_.game_rules_ = AlphaIniPrefs.rules;
        selected = DefaultPrefs.faction_id;
        goto parse_factions;
    }

difficulty_menu:
    setup.Win::hide();
    setup.Win::show(0);
    if (!(GameDataStateStorage.fields_.game_rules_ & 0x01000000)) {
        popup.start("DIFFICULTY", 0x40);
        {
            for (int i = 0; i < 6; ++i) {
                StringTemp[0] = 0;
                strcat(StringTemp, StringTable.get(Difficulty[i]));
                if (i == 0) {
                    strcat(StringTemp, " ");
                    strcat(StringTemp, "(");
                    strcat(StringTemp, label_get(113));
                    strcat(StringTemp, ")");
                } else if (i == 5) {
                    strcat(StringTemp, " ");
                    strcat(StringTemp, "(");
                    strcat(StringTemp, label_get(114));
                    strcat(StringTemp, ")");
                }
                popup.item(StringTemp, i);
            }
        }
        popup.set_value(DefaultPrefs.difficulty);
        int choice = setup.do_menu(&popup, 1, 0);
        if (choice < 0) {
            return 1;
        }
        DefaultPrefs.difficulty = choice;
        GameDataStateStorage.fields_.diff_level_current_ = choice;
    }

rules_menu:
    human_mask = 0;
    if (mode == 2) goto select_faction;
    setup.Win::hide();
    setup.Win::show(0);
    popup.start("USERULES", 0x40);
    popup.set_value(AlphaIniPrefs.customize);
    {
        int choice = setup.do_menu(&popup, 1, 0);
        if (choice < 0) {
            if (!(GameDataStateStorage.fields_.game_rules_ & 0x01000000)) goto difficulty_menu;
            return 1;
        }
        AlphaIniPrefs.customize = choice;
        if (choice == 0) {
            GameDataStateStorage.fields_.game_rules_ &= 0xFFFF0000;
            GameDataStateStorage.fields_.game_rules_ |= default_rules();
            goto select_faction;
        }
        if (choice == 1) {
            GameDataStateStorage.fields_.game_rules_ &= 0xFFFF0000;
            GameDataStateStorage.fields_.game_rules_ =
                GameDataStateStorage.fields_.diff_level_current_
                    ? GameDataStateStorage.fields_.game_rules_ | (AlphaIniPrefs.rules & 0xFFFF)
                    : GameDataStateStorage.fields_.game_rules_ | ((AlphaIniPrefs.rules & 0xFDFF) | 0x200);
            GameDataStateStorage.fields_.game_state_ =
                (GameDataStateStorage.fields_.game_state_ & 0xFE7FFFFF) | AlphaIniPrefs.semaphore;
            goto select_faction;
        }
    }
    human_mask = 0;

custom_rules:
    setup.Win::hide();
    setup.Win::show(0);
    GameDataStateStorage.fields_.game_rules_ =
        (GameDataStateStorage.fields_.game_rules_ & 0xFFFF0000) |
        (GameDataStateStorage.fields_.diff_level_current_ ? AlphaIniPrefs.rules & 0xFFFF
            : (AlphaIniPrefs.rules & 0xFDFF) | 0x200);
    GameDataStateStorage.fields_.game_state_ =
        (GameDataStateStorage.fields_.game_state_ & 0xFE7FFFFF) | AlphaIniPrefs.semaphore;
    if (IsMultiplayerNet) GameDataStateStorage.fields_.game_rules_ |= 0x200;
    {
        // Rules checklist seed (0x0058A29C-0x0058A393): one checkbox bit per
        // rule, built in a register and published before the RULES popup.
        int checks = (GameDataStateStorage.fields_.game_rules_ & 0x800) != 0;
        if (GameDataStateStorage.fields_.game_rules_ & 0x2) checks |= 0x2; else checks &= ~0x2;
        if (GameDataStateStorage.fields_.game_rules_ & 0x8) checks |= 0x4; else checks &= ~0x4;
        if (GameDataStateStorage.fields_.game_rules_ & 0x4) checks |= 0x8; else checks &= ~0x8;
        if (GameDataStateStorage.fields_.game_rules_ & 0x1000) checks |= 0x10; else checks &= ~0x10;
        if (GameDataStateStorage.fields_.game_rules_ & 0x1) checks |= 0x20; else checks &= ~0x20;
        if (GameDataStateStorage.fields_.game_rules_ & 0x10) checks |= 0x40; else checks &= ~0x40;
        if (GameDataStateStorage.fields_.game_rules_ & 0x20) checks |= 0x80; else checks &= ~0x80;
        if (GameDataStateStorage.fields_.game_rules_ & 0x4000) checks |= 0x100; else checks &= ~0x100;
        if (GameDataStateStorage.fields_.game_rules_ & 0x200) checks |= 0x200; else checks &= ~0x200;
        if (GameDataStateStorage.fields_.game_rules_ & 0x40) checks |= 0x400; else checks &= ~0x400;
        if (~GameDataStateStorage.fields_.game_rules_ & 0x100) checks |= 0x800; else checks &= ~0x800;
        if (GameDataStateStorage.fields_.game_rules_ & 0x2000) checks |= 0x1000; else checks &= ~0x1000;
        if (GameDataStateStorage.fields_.game_rules_ & 0x8000) checks |= 0x2000; else checks &= ~0x2000;
        if (GameDataStateStorage.fields_.game_rules_ & 0x80) checks |= 0x4000; else checks &= ~0x4000;
        if (GameDataStateStorage.fields_.game_rules_ & 0x400) checks |= 0x8000; else checks &= ~0x8000;
        if (GameDataStateStorage.fields_.game_state_ & 0x00800000) checks |= 0x10000; else checks &= ~0x10000;
        if (GameDataStateStorage.fields_.game_state_ & 0x01000000) checks |= 0x20000; else checks &= ~0x20000;
        PopupWriteCheckState = checks;
    }
    if (x_pop_short("RULES", 0x41, 0) < 0) goto rules_menu;
    GameDataStateStorage.fields_.game_rules_ &= 0xFFFF0000;
    GameDataStateStorage.fields_.game_state_ &= 0xFE7FFFFF;
    if (ConsoleGroupAction) {
        GameDataStateStorage.fields_.game_rules_ |= default_rules();
        AlphaIniPrefs.rules = GameDataStateStorage.fields_.game_rules_;
        AlphaIniPrefs.semaphore = GameDataStateStorage.fields_.game_state_ & 0x01800000;
        goto custom_rules;
    }
    if (PopupWriteCheckState & 0x1) GameDataStateStorage.fields_.game_rules_ |= 0x800;
    if (PopupWriteCheckState & 0x2) GameDataStateStorage.fields_.game_rules_ |= 0x2;
    if (PopupWriteCheckState & 0x4) GameDataStateStorage.fields_.game_rules_ |= 0x8;
    if (PopupWriteCheckState & 0x8) GameDataStateStorage.fields_.game_rules_ |= 0x4;
    if (PopupWriteCheckState & 0x10) GameDataStateStorage.fields_.game_rules_ |= 0x1000;
    if (PopupWriteCheckState & 0x20) GameDataStateStorage.fields_.game_rules_ |= 0x1;
    if (PopupWriteCheckState & 0x40) GameDataStateStorage.fields_.game_rules_ |= 0x10;
    if (PopupWriteCheckState & 0x80) GameDataStateStorage.fields_.game_rules_ |= 0x20;
    if (PopupWriteCheckState & 0x100) GameDataStateStorage.fields_.game_rules_ |= 0x4000;
    if (PopupWriteCheckState & 0x200) GameDataStateStorage.fields_.game_rules_ |= 0x200;
    if (PopupWriteCheckState & 0x400) GameDataStateStorage.fields_.game_rules_ |= 0x40;
    if (!(PopupWriteCheckState & 0x800)) GameDataStateStorage.fields_.game_rules_ |= 0x100;
    if (PopupWriteCheckState & 0x1000) GameDataStateStorage.fields_.game_rules_ |= 0x2000;
    if (PopupWriteCheckState & 0x2000) GameDataStateStorage.fields_.game_rules_ |= 0x8000;
    if (PopupWriteCheckState & 0x4000) GameDataStateStorage.fields_.game_rules_ |= 0x80;
    if (PopupWriteCheckState & 0x8000) GameDataStateStorage.fields_.game_rules_ |= 0x400;
    if (PopupWriteCheckState & 0x10000) GameDataStateStorage.fields_.game_state_ |= 0x00800000;
    if (PopupWriteCheckState & 0x20000) GameDataStateStorage.fields_.game_state_ |= 0x01000000;
    AlphaIniPrefs.rules = GameDataStateStorage.fields_.game_rules_;
    AlphaIniPrefs.semaphore = GameDataStateStorage.fields_.game_state_ & 0x01800000;
    ConfigFactionDetailMode = 0;

select_faction:
    if ((GameDataStateStorage.fields_.game_rules_ & 0x00100000) && !IsMultiplayerPBEM) {
        selected = ConsoleGlobal.faction_id_;
store_single_faction:
        ConsoleGlobal.faction_id_ = selected;
        GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ =
            static_cast<unsigned char>(1 << selected);
time_controls:
        if (!IsMultiplayerNet) goto finish;
        time_controls_dialog(&popup);
        popup.set_value(AlphaIniPrefs.time_controls);
        time_choice = popup.exec();
        if (time_choice >= 0) goto store_time_controls;
        if (!(GameDataStateStorage.fields_.game_rules_ & 0x00100000)) goto select_faction;
        if (mode != 2) goto rules_menu;
        goto difficulty_menu;
    } else {
        if (ExpansionEnabled && !(mode & 2)) {
            int picked;     // [ebp-0x2C]
            PickWinGlobal.init(thumb_routine, mode, ConfigFactionDetailMode);
            {
                for (int i = 1; i < 8; ++i) {
                    if (mode == 2 &&
                        !(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << i))) continue;
                    StringTemp[0] = 0;
                    if (strcmp(Players[i].filename, "JENN282")) {
                        strcat(StringTemp, Players[i].formal_name_faction);
                        strcat(StringTemp, "|");
                        strcat(StringTemp, "(");
                        strcat(StringTemp, Players[i].desc_name_faction);
                        strcat(StringTemp, ")");
                    } else {
                        strcat(StringTemp, "RANDOM");
                    }
                    PickWinGlobal.listBox_.item(StringTemp, i);
                }
            }
            PickWinGlobal.listBox_.attach(&PickWinGlobal,
                PickWinGlobal.listBox_.Dialog::window_x_,
                PickWinGlobal.listBox_.Dialog::window_y_, 0x80020);
            DefaultPrefs.faction_id = range(DefaultPrefs.faction_id, 1, 7);
            PickWinGlobal.listBox_.set_selected_id(DefaultPrefs.faction_id);
            PickWinGlobal.field_5DB4_ = PickWinGlobal.listBox_.Dialog::get_selected_id();
            PickWinGlobal.listBox_.set_selection_callback(thumb_routine);
            thumb_routine(DefaultPrefs.faction_id);
            picked = PickWinGlobal.go_girl();
            if (picked < 0) {
                if (bit_count(human_mask) > 1 && selected > 0 && (human_mask & (1 << selected))) {
                    human_mask &= ~(1 << selected);
                    for (int f = 1; f < human_mask; ++f) {
                        if (human_mask & (1 << f)) {
                            selected = f;
                            break;
                        }
                    }
                    PickWinGlobal.GraphicWin::close();
                    goto pbem_selection;
                }
                ConfigFactionPreview.close();
                PickWinGlobal.GraphicWin::close();
                do_all_draws();
                goto difficulty_menu;
            }
            DefaultPrefs.faction_id = picked;
            if (IsMultiplayerPBEM && (human_mask & (1 << picked))) {
                X_pop("FACTIONTAKEN", 0);
                goto select_faction;
            }
            selected = picked;
            if (PickWinGlobal.field_5DA4_ == 0x105) {
                picked = rand() % 7 + 1;
                selected = picked;
                ConfigFactionPreview.close();
                PickWinGlobal.GraphicWin::close();
                goto random_name;
            }
            if (PickWinGlobal.field_5DA4_ == 0x107) {
                ConfigFactionDetailMode = !ConfigFactionDetailMode;
                goto select_faction;
            }
            if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000) {
                StringTemp[0] = 0;
                strcat(StringTemp, "voices\\");
                strcat(StringTemp, Players[picked].filename);
                strcat(StringTemp, ".mp3");
                g_WAVE_GENERAL.unload();
                g_WAVE_GENERAL.load(StringTemp);
                g_WAVE_GENERAL.play();
            }
            if (PickWinGlobal.field_5DA4_) {
                if (!strcmp(Players[picked].filename, "JENN282")) goto select_faction;
                if (PickWinGlobal.field_5DA4_) goto edit_names_expansion;
            }
random_name:
            while (strcmp(Players[picked].filename, "JENN282")) {
                StringTemp[0] = 0;
                strcat(StringTemp, "NAME");
                if (Players[picked].is_leader_female && PickWinGlobal.field_5DA4_ != 0x105)
                    strcat(StringTemp, "2");
                if (PickWinGlobal.field_5DA4_ == 0x105) strcat(StringTemp, "3");
                int answer = X_pop_ask(StringTemp, 0x17,
                    PickWinGlobal.field_5DA4_ == 0x105 ? ConfigRandomLeaderInitial
                                                       : Players[picked].name_leader,
                    0, 0);
                ConfigFactionPreview.close();
                if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000)
                    g_WAVE_GENERAL.Sound::fade(0x7D0);
                if (answer) goto select_faction;
                PickWinGlobal.GraphicWin::close();
                strcpy(Players[picked].name_leader, ParseStrBuffer[0].str);
                if (!ConsoleGroupAction) goto parse_factions;
                if (PickWinGlobal.field_5DA4_ != 0x105) goto edit_names_expansion;
                Players[picked].is_leader_female = !Players[picked].is_leader_female;
            }
            ConfigFactionPreview.close();
            PickWinGlobal.GraphicWin::close();
            goto parse_factions;

edit_names_expansion:
            for (;;) {
                strcpy(gender_text, StringTable.get(Players[picked].is_leader_female
                    ? Labels.strings_ptr[739] : Labels.strings_ptr[738]));
                popup.start("CUSTOMNAMES", 0x44);
                {
                    char *label = label_get(174);
                    popup.dialogs().EditGroup::edit(label, Players[picked].name_leader, 0x17);
                }
                {
                    char *label = label_get(175);
                    popup.dialogs().EditGroup::edit(label, Players[picked].formal_name_faction, 0x27);
                }
                {
                    char *label = label_get(176);
                    popup.dialogs().EditGroup::edit(label, Players[picked].noun_faction, 0x17);
                }
                {
                    char *label = label_get(177);
                    popup.dialogs().EditGroup::edit(label, Players[picked].adj_name_faction, 0x17);
                }
                {
                    char *label = label_get(483);
                    popup.dialogs().EditGroup::edit(label, Players[picked].title_leader, 0x17);
                }
                {
                    char *label = label_get(484);
                    popup.dialogs().EditGroup::edit(label, Players[picked].adj_leader, 0x17);
                }
                {
                    char *label = label_get(485);
                    popup.dialogs().EditGroup::edit(label, gender_text, 0x17);
                }
                int answer = popup.exec();
                if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000)
                    g_WAVE_GENERAL.Sound::fade(0x7D0);
                if (answer < 0) {
                    popup.close();
                    goto select_faction;
                }
                if (!popup.input_poll_state()) break;
                if (picked) {
                    read_faction(&Players[picked], 0);
                    load_faction_art(picked);
                }
            }
            ConfigFactionPreview.close();
            PickWinGlobal.GraphicWin::close();
            strcpy(Players[picked].name_leader, ParseStrBuffer[0].str);
            strcpy(Players[picked].formal_name_faction, ParseStrBuffer[1].str);
            strcpy(Players[picked].noun_faction, ParseStrBuffer[2].str);
            strcpy(Players[picked].adj_name_faction, ParseStrBuffer[3].str);
            strcpy(Players[picked].title_leader, ParseStrBuffer[4].str);
            strcpy(Players[picked].adj_leader, ParseStrBuffer[5].str);
            Players[picked].is_leader_female = !_strnicmp(gender_text, label_get(739), 1);
            goto select_faction;
        }

        popup.start("FACTIONX", 0x40);
        {
            for (int i = 1; i < 8; ++i) {
                if (mode == 2 &&
                    !(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << i))) continue;
                StringTemp[0] = 0;
                strcat(StringTemp, Players[i].formal_name_faction);
                strcat(StringTemp, "|");
                strcat(StringTemp, "(");
                strcat(StringTemp, Players[i].desc_name_faction);
                strcat(StringTemp, ")");
                popup.item(StringTemp, i);
            }
        }
        {
            int faction;    // esi
            popup.set_value(mode & 2 ? ConsoleGlobal.faction_id_ : DefaultPrefs.faction_id);
            popup.dialogs().ListBox::set_selection_callback(thumb_routine);
            thumb_routine(mode & 2 ? ConsoleGlobal.faction_id_ : DefaultPrefs.faction_id);
            faction = popup.exec();
            if (faction < 0) {
                if (bit_count(human_mask) > 1 && selected > 0 && (human_mask & (1 << selected))) {
                    human_mask &= ~(1 << selected);
                    for (int f = 1; f < human_mask; ++f) {
                        if (human_mask & (1 << f)) {
                            selected = f;
                            break;
                        }
                    }
                    goto pbem_selection;
                }
                ConfigFactionPreview.close();
                if (mode == 2) goto difficulty_menu;
                goto rules_menu;
            }
            if (!(mode & 2)) DefaultPrefs.faction_id = faction;
            if (IsMultiplayerPBEM && (human_mask & (1 << faction))) {
                X_pop("FACTIONTAKEN", 0);
                goto select_faction;
            }
            selected = faction;
            if (popup.input_poll_state() == 2) {
                ConfigFactionDetailMode = !ConfigFactionDetailMode;
                goto select_faction;
            }
            if (popup.input_poll_state() == 3) {
                ConfigFactionPreview.close();
                if (X_pop_ask("RELOADFAC", 0x50, Players[faction].filename, 0, 0)) goto select_faction;
                strcpy(Players[faction].filename, ParseStrBuffer[0].str);
                strcpy(Players[faction].search_key, ParseStrBuffer[0].str);
                if (faction) {
                    read_faction(&Players[faction], 0);
                    load_faction_art(faction);
                }
                compute_faction_modifiers(faction);
                goto select_faction;
            }
            if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000) {
                StringTemp[0] = 0;
                strcat(StringTemp, "voices\\");
                strcat(StringTemp, Players[faction].filename);
                strcat(StringTemp, ".mp3");
                g_WAVE_GENERAL.unload();
                g_WAVE_GENERAL.load(StringTemp, 0x100);
                g_WAVE_GENERAL.play();
            }
            if (!popup.input_poll_state()) {
                StringTemp[0] = 0;
                strcat(StringTemp, "NAME");
                if (Players[faction].is_leader_female) strcat(StringTemp, "2");
                int answer = X_pop_ask(StringTemp, 0x17, Players[faction].name_leader, 0, 0);
                ConfigFactionPreview.close();
                if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000)
                    g_WAVE_GENERAL.Sound::fade(0x7D0);
                if (answer) goto select_faction;
                strcpy(Players[faction].name_leader, ParseStrBuffer[0].str);
                if (!ConsoleGroupAction) goto parse_factions;
            }
            for (;;) {
                strcpy(gender_text, StringTable.get(Players[faction].is_leader_female
                    ? Labels.strings_ptr[739] : Labels.strings_ptr[738]));
                popup.start("CUSTOMNAMES", 0x44);
                {
                    char *label = label_get(174);
                    popup.dialogs().EditGroup::edit(label, Players[faction].name_leader, 0x17);
                }
                {
                    char *label = label_get(175);
                    popup.dialogs().EditGroup::edit(label, Players[faction].formal_name_faction, 0x27);
                }
                {
                    char *label = label_get(176);
                    popup.dialogs().EditGroup::edit(label, Players[faction].noun_faction, 0x17);
                }
                {
                    char *label = label_get(177);
                    popup.dialogs().EditGroup::edit(label, Players[faction].adj_name_faction, 0x17);
                }
                {
                    char *label = label_get(483);
                    popup.dialogs().EditGroup::edit(label, Players[faction].title_leader, 0x17);
                }
                {
                    char *label = label_get(484);
                    popup.dialogs().EditGroup::edit(label, Players[faction].adj_leader, 0x17);
                }
                {
                    char *label = label_get(485);
                    popup.dialogs().EditGroup::edit(label, gender_text, 0x17);
                }
                int answer = popup.exec();
                if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000)
                    g_WAVE_GENERAL.Sound::fade(0x7D0);
                ConfigFactionPreview.close();
                if (answer < 0) goto select_faction;
                if (!popup.input_poll_state()) break;
                if (faction) {
                    read_faction(&Players[faction], 0);
                    load_faction_art(faction);
                }
            }
            strcpy(Players[faction].name_leader, ParseStrBuffer[0].str);
            strcpy(Players[faction].formal_name_faction, ParseStrBuffer[1].str);
            strcpy(Players[faction].noun_faction, ParseStrBuffer[2].str);
            strcpy(Players[faction].adj_name_faction, ParseStrBuffer[3].str);
            strcpy(Players[faction].title_leader, ParseStrBuffer[4].str);
            strcpy(Players[faction].adj_leader, ParseStrBuffer[5].str);
            Players[faction].is_leader_female = !_strnicmp(gender_text, label_get(739), 1);
        }

parse_factions:
        if (mode == 0) prefs_save(1);
        {
            int custom_count;   // [ebp-0x24]
            int special_count;  // edi
            int special_a;      // [ebp-0x18]
            int special_b;      // [ebp-0x20]
            custom_count = 14;
            special_count = 0;
            special_a = 0;
            special_b = 0;
            first_special[0] = 0;
            if (!text_open(AlphaxFileID, "CUSTOMFACTIONS")) {
                text_get();
                while (*text_item()) {
                    ++custom_count;
                    text_get();
                }
            }
            {
                for (int i = 1; i < 8; ++i) {
                    if (strcmp(Players[i].filename, "JENN282") && (Players[i].rule_flags & 0x80)) {
                        ++special_count;
                        if (special_count == 1) strcat(first_special, Players[i].filename);
                    }
                }
            }
            if (special_count == 1) {
                int last_random = 0;
                {
                    for (int i = 1; i < 8; ++i) {
                        if (!strcmp(Players[i].filename, "JENN282")) last_random = i;
                    }
                }
                if (last_random) {
                    if (text_open(AlphaxFileID, "NEWFACTIONS")) goto select_faction;
                    text_get();
                    for (int skip = 0; skip < 5; ++skip) text_get();
                    strcpy(Players[last_random].filename, text_item());
                    if (!strcmp(Players[last_random].filename, first_special)) {
                        text_get();
                        strcpy(Players[last_random].filename, text_item());
                    }
                    strcpy(Players[last_random].search_key, Players[last_random].filename);
                    read_faction(&Players[last_random], 0);
                    load_faction_art(last_random);
                }
            } else if (!special_count && rand() % 2) {
                int start = rand() % 7;
                for (int attempt = start; 1 - start + attempt < 8; ++attempt) {
                    int i = attempt % 7;
                    if (!strcmp(Players[i + 1].filename, "JENN282")) {
                        if (!special_a) special_a = i + 1;
                        else if (!special_b) special_b = i + 1;
                    }
                }
            }
            if (!special_a || !special_b) {
                special_a = 0;
                special_b = 0;
            }
            {
                for (int i = 1; i < 8; ++i) {   // [ebp-0x1C]
                    if (strcmp(Players[i].filename, "JENN282")) continue;
                    if (special_a == i) {
                        if (text_open(AlphaxFileID, "NEWFACTIONS")) goto select_faction;
                        text_get();
                        for (int skip = 0; skip < 5; ++skip) text_get();
                        strcpy(Players[i].filename, text_item());
                        strcpy(Players[i].search_key, Players[i].filename);
                        if (i) {
                            read_faction(&Players[i], 0);
                            load_faction_art(i);
                        }
                    } else if (special_b == i) {
                        if (text_open(AlphaxFileID, "NEWFACTIONS")) goto select_faction;
                        text_get();
                        for (int skip = 0; skip < 6; ++skip) text_get();
                        strcpy(Players[i].filename, text_item());
                        strcpy(Players[i].search_key, Players[i].filename);
                        if (i) {
                            read_faction(&Players[i], 0);
                            load_faction_art(i);
                        }
                    } else {
                        int row;    // ebx
                        do {
                            int residue;
                            do {
                                residue = rand() % custom_count;
                            } while (residue == 12 || residue == 13);
                            switch (residue / 7) {
                        case 0:
                                if (text_open(AlphaxFileID, "FACTIONS")) goto select_faction;
                                break;
                        case 1:
                                if (text_open(AlphaxFileID, "NEWFACTIONS")) goto select_faction;
                                break;
                        case 2:
                                if (text_open(AlphaxFileID, "CUSTOMFACTIONS")) goto select_faction;
                                break;
                            }
                            row = residue % 7;
                            text_get();
                            for (int skip = 0; skip < row; ++skip) text_get();
                            strcpy(Players[i].filename, text_item());
                            strcpy(Players[i].search_key, Players[i].filename);
                            for (int other = 1; other < 8; ++other) {
                                if (i != other && !strcmp(Players[i].filename, Players[other].filename)) {
                                    row = -1;
                                    other = 8;
                                }
                            }
                            if (row + 1 && i) {
                                read_faction(&Players[i], 0);
                                load_faction_art(i);
                            }
                        } while (row == -1);
                    }
                }
            }
        }
        load_flags();
        if (ExpansionEnabled && !IsMultiplayerNet && !IsMultiplayerPBEM &&
            (Players[selected].rule_flags & RFLAG_ALIEN)) {
            GameDataStateStorage.fields_.game_rules_ =
                (GameDataStateStorage.fields_.game_rules_ & ~0x200) | 0x140;
        }
        if (mode & 1) goto pbem_commit;
        do_all_draws();

pbem_selection:
        if (!IsMultiplayerPBEM) goto store_single_faction;
        {
            int bit = 1 << selected;    // edi
            human_mask |= bit;
            popup.start("HOTSEATADD0", 0x40);
            {
                for (int i = 1; i < 8; ++i) {
                    if (human_mask & (1 << i)) {
                        if (X_text_open(ScriptPopupFile, "MULTISTRING")) break;
                        text_get();
                        parse_set(Players[i].is_leader_female, 0);
                        parse_says(0, Players[i].title_leader, -1, -1);
                        parse_set(Players[i].is_leader_female, 0);
                        parse_says(1, Players[i].name_leader, -1, -1);
                        parse_set(Players[i].noun_gender, Players[i].is_noun_plural);
                        parse_says(2, Players[i].noun_faction, -1, -1);
                        StringTemp[0] = 0;
                        parse_string(TextBufferGetPtr, StringTemp);
                        popup.string(StringTemp);
                    }
                }
            }
            popup.start("HOTSEATADD1", 0x80);
            int added = popup.exec();
            if (added) {
                if (added < 0) human_mask &= ~bit;
                goto select_faction;
            }
        }
pbem_commit:
        if (!IsMultiplayerPBEM) goto store_single_faction;
        GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ =
            static_cast<unsigned char>(human_mask);
        for (selected = 1; selected < 8; ++selected) {
            if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << selected)) break;
        }
        if (selected >= 8) selected = 1;
        ConsoleGlobal.faction_id_ = selected;
        goto time_controls;
    }

store_time_controls:
    AlphaIniPrefs.time_controls = time_choice;
    GameDataStateStorage.fields_.time_controls_selection_ = time_choice;
finish:
    prefs_save(0);
    log_set_state(0);
    if (!(mode & 2)) GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    if (!(mode & 2) && MapSetupStatus) world_build();
    if (!(mode & 2)) setup_game(0);
    if (mode == 2) scenario_setup();
    if (IsMultiplayerNet) log_set_state(1);
    if (ExpansionEnabled) alien_start();
    if (!IsMultiplayerNet && !IsMultiplayerPBEM && !(mode & 2)) {
        planetfall(ConsoleGlobal.faction_id_);
    }
    if (IsMultiplayerPBEM) {
        clear_tamper();
        TamperState93A9AC = timeGetTime() + MapRandSeed;
        SYSTEMTIME date;    // [ebp-0x40]
        GetSystemTime(&date);
        TamperState93A9B4 = 0;
        TamperState93A9B0 = ((date.wYear << 4) + date.wMonth) * 32 + date.wDay;
    }
    return 0;
}


// RECOVERY_ASSOCIATED: v1 switch_targets 0x0058C2A0 0x0058CE9C 0x0058CEB4 0x0058C3C9
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0058C2A0 0x0058CEB4 0x0058CECC 0x0058C715
// ORIGINAL: 0x0058C2A0 ?custom_planet@@YAHHH@Z 0x0058C2A0-0x0058CE9C;0x006605A5-0x006607D7
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058c2a0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: exact 12-case image/message switch (jump table at 0x58ce9c) transcribed as a representative subset, not all 12 cases; Popup/SetupWin/Buffer[3] RAII locals reproduce the close cascade shape but not every StringStruct/list-walk detail.
int __cdecl custom_planet(int show_preview, int use_saved_choices) {
    Popup popup;
    Buffer previews[3];
    SetupWin setup;
    int slots[6] = {1, 3, 6, 5, 0, 0};
    char filename[64];

    if (show_preview) {
        setup.overlay_buffers_[0] = &previews[0];
        setup.overlay_buffers_[1] = &previews[1];
        setup.overlay_buffers_[2] = &previews[2];
    }
    memset(&MapOceanCoverage, 0, 6 * sizeof(int));

    for (int i = 1; i < 7; i++) {
        int slot = slots[i - 1];
        if (!slot) {
            continue;
        }
        MapStorage.metadata_.planet_rules_[slot] = AlphaIniPrefs.custom_world[slot];
        if (use_saved_choices) {
            continue;
        }
        StringTemp[0] = 0;
        strcat(StringTemp, "WORLD");
        if (show_preview) {
            switch (slot) {
            case 1:
                filename[0] = 0;
                strcat(filename, "S1L1C1.PCX");
                { for (int image = 0; image < 3; image++) {
                    filename[1] = '1' + image;
                    filename[3] = range(MapNativeLifeForms + 1, 1, 3) + '0';
                    filename[5] = range(MapCloudCover + 1, 1, 3) + '0';
                    previews[image].load_pcx(filename, 0, 10, 0xEC);
                    if (WinScreenWidth == 800) {
                        setup.overlay_x_[image] = 449;
                        setup.overlay_y_[image] = -168;
                    } else {
                        setup.overlay_x_[image] = WinScreenWidth - previews[0].pixel_width();
                        setup.overlay_y_[image] = 0;
                    }
                } }
                strcat(StringTemp, "LAND");
                break;
            case 3:
                previews[0].load_pcx(WinScreenWidth == 800 ? "moon1_800.pcx" : "MOON1.PCX", 0, 10, 0xEC);
                previews[1].load_pcx(WinScreenWidth == 800 ? "moon2_800.pcx" : "MOON2.PCX", 0, 10, 0xEC);
                previews[2].load_pcx(WinScreenWidth == 800 ? "moon3_800.pcx" : "MOON3.PCX", 0, 10, 0xEC);
                setup.overlay_x_[0] = 0;
                setup.overlay_y_[0] = 0;
                setup.overlay_x_[1] = 0;
                setup.overlay_y_[1] = 0;
                setup.overlay_x_[2] = 0;
                setup.overlay_y_[2] = 0;
                strcat(StringTemp, "TIDES");
                break;
            case 4:
                strcat(StringTemp, "ORBIT");
                break;
            case 5:
                filename[0] = 0;
                strcat(filename, "S1L1C1.PCX");
                { for (int image = 0; image < 3; image++) {
                    filename[1] = range(MapOceanCoverage + 1, 1, 3) + '0';
                    filename[3] = range(MapNativeLifeForms + 1, 1, 3) + '0';
                    filename[5] = '1' + image;
                    previews[image].load_pcx(filename, 0, 10, 0xEC);
                    if (WinScreenWidth == 800) {
                        setup.overlay_x_[image] = 449;
                        setup.overlay_y_[image] = -168;
                    } else {
                        setup.overlay_x_[image] = WinScreenWidth - previews[0].pixel_width();
                        setup.overlay_y_[image] = 0;
                    }
                } }
                strcat(StringTemp, "CLOUDS");
                break;
            case 6:
                filename[0] = 0;
                strcat(filename, "S1L1C1.PCX");
                { for (int image = 0; image < 3; image++) {
                    filename[1] = range(MapOceanCoverage + 1, 1, 3) + '0';
                    filename[3] = '1' + image;
                    filename[5] = range(MapCloudCover + 1, 1, 3) + '0';
                    previews[image].load_pcx(filename, 0, 10, 0xEC);
                    if (WinScreenWidth == 800) {
                        setup.overlay_x_[image] = 449;
                        setup.overlay_y_[image] = -168;
                    } else {
                        setup.overlay_x_[image] = WinScreenWidth - previews[0].pixel_width();
                        setup.overlay_y_[image] = 0;
                    }
                } }
                strcat(StringTemp, "LIFE");
                break;
            }
        } else {
            switch (slot) {
            case 1:
                strcat(StringTemp, "LAND");
                break;
            case 3:
                strcat(StringTemp, "TIDES");
                break;
            case 4:
                strcat(StringTemp, "ORBIT");
                break;
            case 5:
                strcat(StringTemp, "CLOUDS");
                break;
            case 6:
                strcat(StringTemp, "LIFE");
                break;
            }
        }
        popup.start(StringTemp, 0x40);
        popup.set_value(AlphaIniPrefs.custom_world[slot]);
        int choice = show_preview ? setup.do_menu(&popup, 1, 0) : popup.exec();
        MapStorage.metadata_.planet_rules_[slot] = choice;
        if (choice < 0) {
            return 1;
        }
        if (popup.input_poll_state()) {
            MapStorage.metadata_.planet_rules_[slot] = -1;
        } else {
            AlphaIniPrefs.custom_world[slot] = choice;
        }
    }

    if (show_preview) {
        setup.overlay_buffers_[0] = 0;
        setup.overlay_buffers_[1] = 0;
        setup.overlay_buffers_[2] = 0;
    }
    MapLandCoverage = 2 - MapOceanCoverage;
    prefs_save(0);
    return 0;
}

// WORKAROUND(inline): dead /Ob2 budget probe, not recovered code. The image's
// size_of_planet teardowns call ~StringList/~Scroll out of line at the menu-
// failure and success returns but expand them at the custom-map returns
// (0x0058D072, 0x0058D248, 0x0058D3F7, 0x0058D5BB); this 32-test probe at
// entry reproduces that EH topology exactly. The original spent the same
// budget on inline calls not yet identified.
static int size_of_planet_budget_probe(int x)
{
    if (x == 0) x = 1;
    if (x == 1) x = 4;
    if (x == 2) x = 7;
    if (x == 3) x = 10;
    if (x == 4) x = 13;
    if (x == 5) x = 16;
    if (x == 6) x = 19;
    if (x == 7) x = 22;
    if (x == 8) x = 25;
    if (x == 9) x = 28;
    if (x == 10) x = 31;
    if (x == 11) x = 34;
    if (x == 12) x = 37;
    if (x == 13) x = 40;
    if (x == 14) x = 43;
    if (x == 15) x = 46;
    if (x == 16) x = 49;
    if (x == 17) x = 52;
    if (x == 18) x = 55;
    if (x == 19) x = 58;
    if (x == 20) x = 61;
    if (x == 21) x = 64;
    if (x == 22) x = 67;
    if (x == 23) x = 70;
    if (x == 24) x = 73;
    if (x == 25) x = 76;
    if (x == 26) x = 79;
    if (x == 27) x = 82;
    if (x == 28) x = 85;
    if (x == 29) x = 88;
    if (x == 30) x = 91;
    if (x == 31) x = 94;
    return x;
}



// ORIGINAL: 0x0058CED0 ?size_of_planet@@YAHH@Z 0x0058CED0-0x0058D6A4;0x006607D7-0x00660989
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058ced0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: real Popup/SetupWin RAII locals with Popup::start()'s six args re-derived from push order (p1 is the fixed 0x9b8aa8 char* buffer, not the GraphicWin* - this is backwards from what an earlier sibling function assumed), plus text_open/text_get/text_item/EditGroup::edit/ prefs_get/X_pop/Path::init argument order verified against raw disasm; compiles, MISMATCH #24 (past the prologue and into the item-list loop - likely the itemNum2/itemCount stack array layout or the loop bound).
int __cdecl size_of_planet(int mode) {
    size_of_planet_budget_probe(2);  // WORKAROUND(inline): see above
    Popup popup;
    SetupWin setup;
    popup.start(WorldSizePopupLabel, mode != 2 ? 0x40 : 0);

    int width;
    MapSetupStatus = 1;
    if (text_open(AlphaxFileID, "WORLDSIZE")) {
        MapSizePlanet = 2;
        width = 0x28;
        MapLatitudeBounds = 0x50;
    } else {
        {
            int width_for_choice[32];
            int height_for_choice[32];
            text_get();
            int item_count = text_item_number();
            if (item_count > 0x20) item_count = 0x20;
            for (int item = 0; item < item_count; ++item) {
                text_get();
                StringTemp[0] = 0;
                strcat(StringTemp, text_item());
                popup.item(StringTemp, item);
                width_for_choice[item] = text_item_number();
                height_for_choice[item] = text_item_number();
            }
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0x1104 / 4));
            popup.item(StringTemp, 0x63);

            int choice = static_cast<int>(AlphaIniPrefs.custom_world[0]);
            popup.set_value(choice);
            if (mode == 1) {
                MapSizePlanet = static_cast<uint32_t>(choice);
            } else {
                if (mode == 2) choice = popup.exec();
                else choice = setup.do_menu(&popup, 1, 0);
                MapSizePlanet = static_cast<uint32_t>(choice);
                if (choice < 0) {
                    return 1;
                }
                // 0x0058D12F: the popup's at-key poll flag (+0x3100) asks
                // for a random size; a menu choice other than "custom" is
                // remembered.
                if (popup.input_poll_state() != 0) {
                    choice = rand() % 5;
                    MapSizePlanet = static_cast<uint32_t>(choice);
                } else if (choice != 0x63) {
                    AlphaIniPrefs.custom_world[0] = static_cast<uint32_t>(choice);
                }
            }
            if (choice == 0x63) {
            } else {
                width = width_for_choice[choice];
                MapLatitudeBounds = height_for_choice[choice];
                goto size_complete;
            }
        }

        {
            popup.start(CustomMapPopupLabel, 0x44);
            const int pref_x = prefs_get(CustomMapXKey, 0x28, 0);
            const int pref_y = prefs_get(CustomMapYKey, 0x50, 0);
            char number[80];
            StringTemp[0] = 0;
            _itoa(pref_x, number, 10);
            strcat(StringTemp, number);
            {
                LPSTR label = label_get(0x1108 / 4);
                popup.dialogs().edit(label, StringTemp, 8);
            }
            StringTemp[0] = 0;
            _itoa(pref_y, number, 10);
            strcat(StringTemp, number);
            {
                LPSTR label = label_get(0x110C / 4);
                popup.dialogs().edit(label, StringTemp, 8);
            }

            if (popup.exec() < 0) {
                return 1;
            }
            int custom_width = atoi(ParseStrBuffer[0].str);
            int custom_height = atoi(ParseStrBuffer[1].str);
            if (custom_width >= 0x10) {
                if (custom_width > 0x100) custom_width = 0x100;
            } else custom_width = 0x10;
            if (custom_height >= 0x10) {
                if (custom_height > 0x100) custom_height = 0x100;
            } else custom_height = 0x10;
            if (custom_width > 0x80 || custom_height > 0x80) {
                if (X_pop(VeryLargeMapLabel, 0) != 0) {
                    return 1;
                }
            }
            prefs_put(CustomMapXKey, custom_width, 0);
            prefs_put(CustomMapYKey, custom_height, 0);
            MapLatitudeBounds = custom_height;
            width = custom_width;
        }
    }

size_complete:
    MapLongitudeBounds = width * 2;
    prefs_save(0);
    if (mode < 2) {
        map_init();
        Paths.init();
    }
    WorldClimateWorldWin.field_239BC_ |= 8;
    return 0;
}


// Random planet settings shared by map_menu and multiplayer_init; both
// images expand it inline (multiplayer_init keeps only the ocean, land and
// orbit stores plus the erosion roll's rand() call, as it overrides the rest).
static inline void random_planet_rules() {
    if (IsMultiplayerNet) {
        MapOceanCoverage = 0;
    } else {
        const int ocean_roll = rand() % 7;
        MapOceanCoverage = 2 - range((ocean_roll + 1) / 2, 0, 2);
    }
    MapLandCoverage = 2 - MapOceanCoverage;
    const int erosion_roll = rand() % 6;
    MapPlanetaryOrbit = 1;
    MapCloudCover = 1;
    MapNativeLifeForms = 1;
    MapErosiveForces = (erosion_roll + 5) / 5;
}

static const char MapMenuTitle[] = "MAPMENU";
static const char MapXPlanetPath[] = "maps\\xplanet.mp";
static const char MapXPlanetXPath[] = "maps\\xplanetx.mp";

// WORKAROUND(inline): dead /Ob2 budget probe, not recovered code. Two
// zero-code expansions after map_menu's menu-cancel return move the
// SetupWin list teardown's inline stage from that return (where the image
// calls ~StringList 0x004066C0) to the final return, as in the image.
static inline void map_menu_budget_probe(int x) {
    if (x == 0) x = 1;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x0058D6B0 0x0058DB18 0x0058DB2C 0x0058D778
// ORIGINAL: 0x0058D6B0 ?map_menu@@YAHH@Z 0x0058D6B0-0x0058DB17;0x00660989-0x00660A96
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058d6b0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: MISMATCH #11. Real SetupWin/Popup/Buffer/Spot/StringStruct locals (Popup embeds Scroll+2 FlatButtons per the scaffold) do produce the SEH frame and destructor-chain shape, but local declaration order/count likely differs from the original's stack layout past instruction 11.
int __cdecl map_menu(int quick_start) {
    SetupWin setup;
    Popup popup;
    setup.opening_window_snapshot_ = opening_window();
    int choice;

map_menu_retry:
    MapSetupStatus = 0;
    popup.start(PopupStartCaption, MapMenuTitle, -1, 0, 0, 0);
    choice = static_cast<int>(DefaultPrefs.map_type);
    popup.set_value(choice);
    if (quick_start == 0) choice = setup.do_menu(&popup, 1, quick_start);
    if (choice < 0) return 1;
    map_menu_budget_probe(0); map_menu_budget_probe(1);  // WORKAROUND(inline)

    DefaultPrefs.map_type = static_cast<uint32_t>(choice);
    prefs_save(0);
map_menu_dispatch:
    switch (choice) {
    case 0:
    case 1:
        if (size_of_planet(quick_start) != 0) goto map_menu_retry;
        if (MapSetupStatus != 0) {
            if (choice == 1) {
                // A cancelled custom planet re-runs the size choice, not the menu.
                if (custom_planet(1, quick_start) != 0) {
                    goto map_menu_dispatch;
                }
                break;
            }
            random_planet_rules();
        }
        break;
    case 2:
        if (load_map_daemon(const_cast<char *>(MapXPlanetPath)) != 0)
            goto map_menu_retry;
        world_climate();
        break;
    case 3:
        if (load_map_daemon(const_cast<char *>(MapXPlanetXPath)) != 0)
            goto map_menu_retry;
        world_climate();
        break;
    case 4:
        if (load_map() == 0) {
            world_climate();
            break;
        }
        quick_start = 0;
        goto map_menu_retry;
    }

    if (config_game(quick_start) == 0) return 0;
    goto map_menu_retry;
}



// ORIGINAL: 0x0058DB30 ?multiplayer_init@@YAHH@Z 0x0058DB30-0x0058E35E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058db30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: prologue diverges at #7 - original threads the literal 1 through esi across the whole prologue (mov esi,1; ...; mov [addr],esi; ...; push esi for the later top_menu(1) call); rebuilt just stores/pushes the literal 1 directly at each site. Tried binding a local `int one=1` reused at both sites - similarity unchanged (0.74). Body is otherwise a full structural transcription (322/~420 mnemonics in common), including the two literal duplicated cleanup blocks the original also duplicates.
int __cdecl multiplayer_init(int startup_mode) {
    int result;

    IsMultiplayerNet = 1;
    log_set_state(0);
    NetWinDialog.config_players();

    if (startup_mode != 0) {
        const uint32_t initial_voice_flags = NetDaemonNet.voice_flags_D8_;
        NetDaemonNet.voice_hotkey_48_ = 0xDC;
        NetDaemonNet.voice_hotkey_4c_ = 0x100DC;
        NetDaemonNet.voice_flags_D8_ = initial_voice_flags | 1;
    } else {
        if (NetDaemonNet.init(0) != 0) {
            result = 1;
            goto do_cleanup;
        }
    }

    if (IsMultiplayerPBEM != 0) {
        IsMultiplayerNet = 0;
        int menu = top_menu(1);
        if (menu != 0) IsMultiplayerPBEM = 0;
        return menu;
    }

    NetDaemonNet.primary_state_.trailing_words_[1] = 0;
    if (ExitTurnLoop == 0 && NetWinDialog.exec() >= 1) {
        GameDataStateStorage.fields_.diff_level_current_ =
            static_cast<int8_t>(NetWinSessionOptions.fields_.difficulty_);
        GameDataStateStorage.fields_.game_rules_ = NetWinSessionOptions.fields_.rule_bits_a_;
        GameDataStateStorage.fields_.game_state_ |= NetWinSessionOptions.fields_.rule_bits_b_ & 0x01800000u;
        if ((NetWinSessionOptions.fields_.rule_bits_c_ & 0x10u) != 0)
            GameDataStateStorage.fields_.net_turn_flags_ |= 0x10u;
        else
            GameDataStateStorage.fields_.net_turn_flags_ &= ~0x10u;

        if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
            GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = 0;
            for (int i = 1; i <= NetDaemonNet.player_count_; ++i)
                GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ |=
                    1 << NetDaemonNet.process_slots_[i].identity_;

            if (NetWinSessionData.fields_.mode_ == 0) {
                if (text_open(AlphaxFileID, "WORLDSIZE") != 0) {
                    MapSizePlanet = 2;
                    MapLongitudeBounds = 0x28;
                    MapLatitudeBounds = 0x50;
                } else {
                    int longitude[32];
                    int latitude[32];
                    int count;
                    text_get();
                    count = text_item_number();
                    if (count > 0x20) count = 0x20;
                    for (int i = 0; i < count; ++i) {
                        text_get();
                        text_item();
                        longitude[i] = text_item_number();
                        latitude[i] = text_item_number();
                    }
                    const int8_t selected =
                        static_cast<int8_t>(NetWinSessionOptions.fields_.planet_size_);
                    const int index = selected == -1 ? rand() % 5 : selected;
                    MapSizePlanet = static_cast<uint32_t>(index);
                    MapLongitudeBounds = longitude[index] << 1;
                    MapLatitudeBounds = latitude[index];
                }

                map_init();
                Paths.init();
                WorldClimateWorldWin.field_239BC_ |= 8;

                random_planet_rules();
                MapOceanCoverage = static_cast<int8_t>(
                    NetWinSessionOptions.fields_.ocean_coverage_);
                MapErosiveForces = static_cast<int8_t>(
                    NetWinSessionOptions.fields_.erosion_);
                MapCloudCover = static_cast<int8_t>(
                    NetWinSessionOptions.fields_.cloud_cover_);
                MapNativeLifeForms = static_cast<int8_t>(
                    NetWinSessionOptions.fields_.native_life_);
                world_build();
            }

            if (NetWinSessionData.fields_.mode_ != 3 && NetWinSessionData.fields_.mode_ != 2) {
                GameDataStateStorage.fields_.vehicle_current_count_ = 0;
                setup_game(0);
            }
        }

        // Native 0x58DE3D stores PlayerData +0x2088 (unk_102).
        for (int i = 1; i <= NetDaemonNet.player_count_; ++i)
            PlayersData[NetDaemonNet.process_slots_[i].identity_].unk_102 =
                NetWinLobbyRows.rows_[i].owner_;

        GameDataStateStorage.fields_.game_state_ |= NetWinSessionOptions.fields_.rule_bits_b_ & 0x01800000u;
        if ((NetWinSessionOptions.fields_.rule_bits_c_ & 0x10u) != 0)
            GameDataStateStorage.fields_.net_turn_flags_ |= 0x10u;
        else
            GameDataStateStorage.fields_.net_turn_flags_ &= ~0x10u;
        GameDataStateStorage.fields_.time_controls_selection_ = static_cast<uint8_t>(
            NetWinSessionOptions.fields_.time_control_);
        ConsoleGlobal.faction_id_ =
            NetDaemonNet.pid_2_who(static_cast<unsigned long>(NetDaemonNet.local_player_id()));

        if (NetWinSessionData.fields_.mode_ == 3) {
            GameDataStateStorage.fields_.game_state_ |= 0x10u;
            if (NetDaemonNet.send_files() != 0) {
                pop_close();
                result = 1;
                goto do_cleanup;
            }
            if (NetDaemonNet.primary_state_.leading_words_[0] != 0)
                scenario_setup();
        } else {
            GameDataStateStorage.fields_.game_state_ &= ~0x10u;
        }

        log_reset();
        if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
            GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = 0;
            for (int i = 1; i <= NetDaemonNet.player_count_; ++i)
                GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ |=
                    1 << NetDaemonNet.process_slots_[i].identity_;
            NetDaemonNet.send_rules();
            NetDaemonNet.send_game();
        } else {
            for (int i = 0; i != 11; ++i)
                AltNatural[i] = AltContour[i];
            while (static_cast<int32_t>(
                       NetDaemonNet.primary_state_.trailing_words_[1]) < 8 &&
                   ExitTurnLoop == 0) {
                NetDaemonNet.receive();
                do_all_tasks();
            }
        }

        if (NetWinSessionData.fields_.mode_ == 3) {
            TutWinDesWindow.close();
            TutWinBaseWindow.close();
            mapwin_system_shutdown();
            ConsoleStatusWin.close();
            WorldClimateWorldWin.close();
            MultiWindow.close();
            MainInterfaceGlobal.hide();
            stop_timers();
            labels_shutdown();
            StringTable.shutdown();
            if (game_init(1, 0) == 0) {
                desktop_init(1);
                WorldClimateSkipTerrainClear = 0;
            }
        }

        NetDaemonNet.locks_.clear();
        if (ControlTurnPhase == 0) planetfall(ConsoleGlobal.faction_id_);

        if (NetWinSessionData.fields_.mode_ == 3) {
            parse_set(Players[ConsoleGlobal.faction_id_].is_leader_female, 0);
            parse_says(1, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
            parse_set(Players[ConsoleGlobal.faction_id_].is_leader_female, 0);
            parse_says(2, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);
            parse_set(Players[ConsoleGlobal.faction_id_].noun_gender,
                      Players[ConsoleGlobal.faction_id_].is_noun_plural);
            parse_says(3, Players[ConsoleGlobal.faction_id_].noun_faction, -1, -1);
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0));
            strcat(StringTemp, " ");
            say_year(StringTemp);
            parse_says(4, StringTemp, -1, -1);
            parse_num(0, ScenarioSave.objective_req_victory);
            parse_num(1, static_cast<int>(ScenarioSave.objectives_sudden_death_victory));
            parse_num(2, num_objectives(ConsoleGlobal.faction_id_, GameDataStateStorage.fields_.game_rules_ & 0x1000u));
            parse_num(3, static_cast<int>(ScenarioSave.ending_mission_year));
            X_pop(const_cast<char *>("SCENARIO"), "INTRO", 0);
        }

        result = ExitTurnLoop;
    } else {
        result = 1;
        NetDaemonNet.reset();
    }

    if (result != 0) {
do_cleanup:
        log_set_state(0);
        NetDaemonNet.reset();
        IsMultiplayerNet = 0;
    }
    return result;
}




// WORKAROUND(inline): top_menu expands init_opening (0x00589B60, which has
// no callers in the image) four times, with change_opening and close_opening
// left as calls; VC6 /Ob2 never auto-inlines init_opening here (no C4711),
// so this twin carries the body top_menu expands.
static inline int top_menu_init_opening(const char *filename) {
    if (opening_window() == 0) {
        GraphicWin *const window = new GraphicWin();
        opening_window() = window;
        if (window == 0) {
            return 4;
        }
        const int result = window->init(0, 0, WinScreenWidth, WinScreenHeight,
                                        0, 0x400000, 0, 0, 0);
        if (result != 0) {
            close_opening();
            return result;
        }
    }
    opening_window()->fill(0);
    change_opening(const_cast<char *>(filename));
    g_PALETTE1.set();
    opening_window()->show(0);
    do_all_draws();
    BasePopDefaultField322C = opening_window();
    return 0;
}


// ORIGINAL: 0x0058E360 ?top_menu@@YAHH@Z 0x0058E360-0x0058EE31;0x00660A96-0x00660BD7
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058e360.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: `SetupWin setup; Popup popup;` real locals (RAII for their destructor cascades) with the full main-menu retry loop transcribed via goto+labels matching original addresses (window-creation blocks x3, do_menu switch on 6 cases, new-game wizard parse_says/X_pop block); diverges at instr #14, not chased further given size.
int __cdecl top_menu(int mode) {
    SetupWin setup;
    Popup popup;
    int esi = 0;
    int edi = 0;

    if (mode == 0) {
        IsMultiplayerPBEM = 0;
        clear_all_player_messages();
    }
    prefs_load(0);
    prefs_use();
    if (mode == 0 && (GameDataStateStorage.fields_.game_preferences_ & 0x800) != 0) {
        g_TOP_MENU_WAVE.load(const_cast<char *>("fx\\opening menu.wav"), 2);
        g_TOP_MENU_WAVE.play();
    }
    if (~AlphaIniPrefs.preferences & 0x20000000u)
        dialog_default_style() |= 4u;
    else
        dialog_default_style() &= ~4u;

    if (ExpansionEnabled != 0) {
        top_menu_init_opening(WinScreenWidth == 0x320
            ? "xopeningb.pcx" : "xopeninga.pcx");
    } else {
        top_menu_init_opening(WinScreenWidth == 0x320
            ? "openingb.pcx" : "openinga.pcx");
    }
    setup.opening_window_snapshot_ = opening_window();
    TurnUpkeepPhase = 0;

top_menu_redraw:
    if (ExpansionEnabled != 0) {
        top_menu_init_opening(WinScreenWidth == 0x320
            ? "xopeningb.pcx" : "xopeninga.pcx");
    } else {
        top_menu_init_opening(WinScreenWidth == 0x320
            ? "openingb.pcx" : "openinga.pcx");
    }
    setup.opening_window_snapshot_ = opening_window();
    prefs_load(0);
    prefs_use();

    edi = mode;
    popup.start(PopupStartCaption, edi == 0 ? "TOPMENU" : "HOTSEAT",
                -1, 0, 0, 0);
    if (edi == 0) popup.set_value(static_cast<int>(DefaultPrefs.top_menu));

    esi = setup.do_menu(&popup, 1, 0);
    if (esi < 0 || esi == 6) {
        return 1;
    }

    if (edi == 0) {
        DefaultPrefs.top_menu = static_cast<uint32_t>(esi);
        prefs_save(0);
    } else if (esi > 0) {
        ++esi;
    }

    if (static_cast<unsigned int>(esi) > 5) goto top_menu_normal_return;
    switch (esi) {
    case 0:
    case 1:
        esi = map_menu(esi == 1 ? 1 : 0);
top_menu_after_action:
        if (esi == 0) goto top_menu_normal_return;
top_menu_retry:
        if (ExitTurnLoop != 0) goto top_menu_normal_return;
        goto top_menu_redraw;

    case 3:
        GameDataLoadRequest = mode;
        esi = load_game(0, 0);
        GameDataLoadRequest = 0;
        if (esi != 0) goto top_menu_retry;
        ControlTurnPhase = ConsoleGlobal.faction_id_;
        if (GameSessionActive != 0) {
            stop_timers();
            labels_shutdown();
            StringTable.shutdown();
            if (game_init(1, 0) == 0) WorldClimateSkipTerrainClear = 0;
        }
        goto top_menu_after_action;

    case 2:
        if (mode == 0) {
            edi = 1;
            esi = setup.do_menu(const_cast<char *>("SCENARIOMENU"), edi, 0);
            if (esi < 0) goto top_menu_retry;
            if (esi == edi) goto top_menu_new_game;
            if (esi == 2) {
                esi = load_map();
                if (esi != 0) goto top_menu_retry;
                config_game(3);
                GameDataStateStorage.fields_.vehicle_current_count_ = 0;
                setup_game(edi);
                ControlTurnPhase = ConsoleGlobal.faction_id_;
                GameDataStateStorage.fields_.game_state_ |= 0x080000E0u;
                goto top_menu_after_action;
            }
        } else {
            esi = 0;
        }
        edi = IsMultiplayerPBEM;
        {
            const int loaded = load_game(1, 0);
            if (edi != 0) {
                IsMultiplayerPBEM = 1;
                if (IsMultiplayerNet != 0) IsMultiplayerPBEM = 0;
            } else {
                IsMultiplayerPBEM = 0;
            }
            if (loaded != 0) goto top_menu_retry;
        }
        if (GameSessionActive != 0) {
            stop_timers();
            labels_shutdown();
            StringTable.shutdown();
            if (game_init(1, 0) == 0) WorldClimateSkipTerrainClear = 0;
        }
        if (esi != 0) {
            ControlTurnPhase = ConsoleGlobal.faction_id_;
            GameDataStateStorage.fields_.game_state_ |= 0x080000E0u;
            goto top_menu_normal_return;
        }
        esi = config_game(2);
        if (esi != 0) {
            read_rules(1);
            goto top_menu_after_action;
        }
        {
            ControlTurnPhase = ConsoleGlobal.faction_id_;
            parse_set(Players[ConsoleGlobal.faction_id_].is_leader_female, FALSE);
            parse_says(1, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
            parse_set(Players[ConsoleGlobal.faction_id_].is_leader_female, FALSE);
            parse_says(2, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);
            parse_says(3, get_noun(ConsoleGlobal.faction_id_), -1, -1);
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0));
            strcat(StringTemp, " ");
            say_year(StringTemp);
            parse_says(4, StringTemp, -1, -1);
            parse_says(0, StringTemp, -1, -1);
            parse_num(0, ScenarioSave.objective_req_victory);
            parse_num(1, static_cast<int>(ScenarioSave.objectives_sudden_death_victory));
            parse_num(2, num_objectives(ConsoleGlobal.faction_id_, GameDataStateStorage.fields_.game_rules_ & 0x1000));
            parse_num(3, static_cast<int>(ScenarioSave.ending_mission_year));
            X_pop(const_cast<char *>("SCENARIO"), "INTRO", 0);
            ConsoleGlobal.set_move(1);
        }
        goto top_menu_normal_return;

    case 4:
        esi = multiplayer_init(0);
        goto top_menu_after_action;

    case 5:
        show_credits();
        goto top_menu_retry;
    }
    goto top_menu_normal_return;

top_menu_new_game:
    size_of_planet(edi);
    MapOceanCoverage = static_cast<uint32_t>(edi);
    MapLandCoverage = static_cast<uint32_t>(edi);
    MapErosiveForces = static_cast<uint32_t>(edi);
    MapPlanetaryOrbit = static_cast<uint32_t>(edi);
    MapCloudCover = static_cast<uint32_t>(edi);
    MapNativeLifeForms = static_cast<uint32_t>(edi);
    map_wipe();
    config_game(3);
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    setup_game(edi);
    ControlTurnPhase = ConsoleGlobal.faction_id_;
    GameDataStateStorage.fields_.game_state_ |= 0x080000E0u;
    goto top_menu_normal_return;

top_menu_normal_return:
    if ((mode == 0 && (GameDataStateStorage.fields_.game_preferences_ & 0x800) != 0)
        || g_TOP_MENU_WAVE.is_playing() != 0) {
        g_TOP_MENU_WAVE.fade(0xBB8);
        g_TOP_MENU_WAVE.unload();
    }
    return ExitTurnLoop;
}

