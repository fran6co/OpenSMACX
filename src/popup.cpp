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
// ORIGINAL: 0x00406BD0 ??_GPopup@@UAEPAXI@Z 0x00406BD0-0x00406BF1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00406BD0-0x00406BF1", "Pinned class constructor ??0Popup@@QAE@XZ at 0x004048A0 stores original class/view table VA 0x6695c8 at 0x004048D9 (mov dword ptr [esi], 0x6695c8); the current genuine owner vftable ??_7Popup@@6BWin@@@ selects generated ??_GPopup@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EPopup@@UAEPAXI@Z (mode 2).", "Pinned ??_GPopup@@UAEPAXI@Z at 0x00406BD0 calls the source-mapped ??1Popup@@QAE@XZ at 0x00406C00, then tests byte ptr [ebp + 8], 1; je 0x406bea bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/popup.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original Popup scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GPopup@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "interface_fonts.h"
#include <shellapi.h>
#include <mmsystem.h>
#include <cstring>
#include "popup.h"
#include "basepop_defaults.h"
#include "popup_fonts.h"
#include "buffer.h"
#include "diplopop.h"
#include "main.h"  // PopupModalActive, shared modal width override.
#include "wave.h"
#include "fx.h"
#include "ambience.h"
#include "win.h"
#include "spritebox.h"
#include "net_class.h"
#include "time.h"
#include "strings.h"    // StringTemp - the shared 0x1032 scratch buffer at 0x009B86A0
#include "mapwin.h"     // MainInterfaceGlobal - the iface window at 0x007AE820
#include "messagewin.h" // InterfaceMessageWin - the desktop window at 0x007F67F8
#include "base.h"       // BaseIDCurrentSelected - the base the exec result goes to
#include "lock.h"
#include "map.h"
#include "veh.h"
#include "alpha.h"
#include "xpops.h"
#include "console.h"
#include "game.h"
#include "interface_palette.h"
#include "palette.h"
#include "general.h"
#include "faction.h"
// RECOVERY_IMPORT: v1 __imp__ShellExecuteA@24 SHELL32.dll name:ShellExecuteA
// Borrowed FontTable entries selected by application popup policy.
// config_popups publishes these pointers; they own no Font lifetime.
// RECOVERY_OBJECT: v1 0x009403D8 ?PopupMainFont@@3PAVFont@@A
Font *PopupMainFont;          // 0x009403D8
// RECOVERY_OBJECT: v1 0x0094575C ?PopupMainBoldFont@@3PAVFont@@A
Font *PopupMainBoldFont;      // 0x0094575C
// RECOVERY_OBJECT: v1 0x00945760 ?PopupMainItalicFont@@3PAVFont@@A
Font *PopupMainItalicFont;    // 0x00945760
// RECOVERY_OBJECT: v1 0x009403DC ?PopupSmallFont@@3PAVFont@@A
Font *PopupSmallFont;         // 0x009403DC
// RECOVERY_OBJECT: v1 0x009457F4 ?PopupSmallBoldFont@@3PAVFont@@A
Font *PopupSmallBoldFont;     // 0x009457F4
// RECOVERY_OBJECT: v1 0x009457F0 ?PopupSmallItalicFont@@3PAVFont@@A
Font *PopupSmallItalicFont;   // 0x009457F0
// RECOVERY_OBJECT: v1 0x009457EC ?PopupMenuFont@@3PAVFont@@A
Font *PopupMenuFont;          // 0x009457EC
// RECOVERY_OBJECT: v1 0x00945764 ?PopupMenuBoldFont@@3PAVFont@@A
Font *PopupMenuBoldFont;      // 0x00945764
// RECOVERY_OBJECT: v1 0x00945768 ?PopupMenuItalicFont@@3PAVFont@@A
Font *PopupMenuItalicFont;    // 0x00945768
// RECOVERY_OBJECT: v1 0x0094576C ?PopupMediumFont@@3PAVFont@@A
Font *PopupMediumFont;        // 0x0094576C
// RECOVERY_OBJECT: v1 0x0094580C ?PopupMediumBoldFont@@3PAVFont@@A
Font *PopupMediumBoldFont;    // 0x0094580C
// RECOVERY_OBJECT: v1 0x00945808 ?PopupMediumItalicFont@@3PAVFont@@A
Font *PopupMediumItalicFont;  // 0x00945808
// RECOVERY_OBJECT: v1 0x00945770 ?InterludeNormalFont@@3PAVFont@@A
Font *InterludeNormalFont;    // 0x00945770
// RECOVERY_OBJECT: v1 0x009457FC ?InterludeSecondFont@@3PAVFont@@A
Font *InterludeSecondFont;    // 0x009457FC
// RECOVERY_OBJECT: v1 0x009457F8 ?InterludeThirdFont@@3PAVFont@@A
Font *InterludeThirdFont;     // 0x009457F8
// RECOVERY_OBJECT: v1 0x00945774 ?PopupLargeFont@@3PAVFont@@A
Font *PopupLargeFont;         // 0x00945774
// RECOVERY_OBJECT: v1 0x00945804 ?PopupLargeBoldFont@@3PAVFont@@A
Font *PopupLargeBoldFont;     // 0x00945804
// RECOVERY_OBJECT: v1 0x00945800 ?PopupLargeItalicFont@@3PAVFont@@A
Font *PopupLargeItalicFont;   // 0x00945800
// RECOVERY_OBJECT: v1 0x00945778 ?PopupMapLabelFont@@3PAVFont@@A
Font *PopupMapLabelFont;      // 0x00945778
// RECOVERY_OBJECT: v1 0x00945810 ?PopupDefaultWindowFont@@3PAVFont@@A
Font *PopupDefaultWindowFont; // 0x00945810

#include "game.h"
#include "font.h"
#include "menu.h"
#include "listbox.h"
#include "scroll.h"
#include "sprite.h"
#include "pulldown.h"
#include "diplowin.h" // DiploKnownTechColor shares the original menu palette cell.


extern int TextColourTable[195];
extern BoxSpriteParams InterfaceStandardPopupBoxStyle;
extern uint32_t EditBoxDefaultB20;
extern uint32_t EditBoxDefaultB24;
extern uint32_t EditBoxDefaultB28;
extern uint32_t EditBoxDefaultB2C;

// Superseded seed deleted; seed paths below are historical Git provenance.


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00588460 ?popups_normal@@YAXXZ 0x00588460-0x005887F2
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00588460.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// symbol    ?popups_normal@@YAXXZ
void __cdecl popups_normal() {
    PopupModalState = 0;
    BasePopDefaultField30A4 = 0x1000080;
    BasePopDefaultField21B0[0] = 0x1A;
    BasePopDefaultField21B0[1] = 0x3A;
    BasePopDefaultField21B0[2] = 0x32;
    BasePopDefaultField21B0[3] = 3;
    BasePop::set_def_string_font(PopupMainFont, PopupMainBoldFont, PopupMainItalicFont, 0);
    Dialog::set_def_dialog_font(PopupMainFont, PopupMainBoldFont, PopupMainItalicFont);
    BasePop::set_def_button_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont);
    WinStaticDefaults[7] = PopupThemeWindowColor;
    BasePopDefaultField21C0[0] = 0;
    BasePopDefaultField21C0[1] = 4;
    WinStaticDefaults[0] = 0;
    WinDynamicDefaults.rest_[1] = PopupThemeTextShadow;
    WinDynamicDefaults.rest_[0] = MessageWinTextColor & 0xFF;
    WinStaticDefaults[2] = 0;
    WinStaticDefaults[3] = 1;
    WinStaticDefaults[4] = 1;
    BasePopButtonDefaults.button_color =
        static_cast<uint8_t>(PopupThemeSurfaceColor);
    BasePop::set_def_button_color(0, -1, 1, 1);
    BasePop::set_def_button_color2(0, -1, 1, 1);
    BasePop::set_def_button_color3(0, -1, 1, 1);
    BasePop::set_def_string_color(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color2(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color3(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color_hyper(2, -1, 1, 1);
    BasePopDefaultField31B0[1] = PopupThemeSurfaceColor;
    MenuDefaultSelectionColor = PopupThemeSurfaceColor;
    MenuDefaultSelectionTextColor = MessageWinTextColor;
    MenuDefaultColor3 = DiploKnownTechColor;
    WinStaticDefaults[6] = InterfaceBottomBorderThickness;
    WinStaticDefaults[1] = InterfaceCaptionHeight;
    BasePopDefaultField3104[2] = 0;
    BasePopButtonDefaults.bevel_upper = PopupThemeBevelUpper;
    BasePopButtonDefaults.bevel_lower = PopupThemeBevelLower;
    BasePopDefaultField31B0[0] = 0;
    MenuDefaultColor1 = MenuDefaultColor2 = 0;
    WinStaticDefaults[5] = InterfaceBorderThickness;
    DialogDefaults[3] = 2;
    DialogDefaults[1] = PopupThemeBevelUpper;
    DialogDefaults[2] = PopupThemeBevelLower;
    BaseButton::set_def_text_color(0, -1, 1, 1);
    DialogDefaults[0] = PopupThemeSurfaceColor;
    BaseButtonA94Default = 0;
    BaseButton::set_def_font(0, 0, 0);
    BaseButtonA98Default = PopupThemeButtonColor;
    PullDownDefaultBackground = TextColourTable[40];
    PullDownDefaultTextColor = TextColourTable[38];
    PullDownDefaultDisabledTextShadow = TextColourTable[42];
    PullDownDefaultSelectedTextColor = TextColourTable[46];
    PullDownDefaultTextShadow = TextColourTable[44];
    PullDownDefaultSelectedTextShadow = TextColourTable[48];
    PullDownDefaultDisabledTextColor = TextColourTable[48];
    PullDownBorderPadding = 1;
    PullDownDefaultFont = PopupMenuFont;
    DialogDefaults[23] = 0;
    dialog_set_def_dialog_text_color(0xE0, -1, 1, 1, 0);
    DialogFieldACDefault = PopupThemeBevelUpper;
    DialogFieldB0Default = PopupThemeBevelLower;
    ListBoxDefault24 = PopupThemeBevelUpper;
    ListBoxCloseDynamicDefault = PopupThemeBevelLower;
    DialogFieldB4Default = 1;
    DialogFieldB8Default = static_cast<uint32_t>(-1);
    ListBoxDefault14 = PopupThemeSurfaceColor;
    ListBoxDefault10 = 0x41;
    EditBoxDefaultB24 = PopupThemeSurfaceColor;
    EditBoxDefaultB20 = 0;
    ScrollCloseStaticDefaults.bevel_thickness = 0;
    ScrollCloseStaticDefaults.border_color = -1;
    ScrollCloseStaticDefaults.field_A64 = 0x41;
    ScrollCloseStaticDefaults.field_A48 = 0x41;
    ScrollCloseStaticDefaults.color = 0x41;
    ScrollCloseDynamicDefaults.sprite_up[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[0];
    ScrollCloseDynamicDefaults.sprite_up[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[1];
    ScrollCloseDynamicDefaults.sprite_up[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[2];
    ScrollCloseDynamicDefaults.sprite_down[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[3];
    ScrollCloseDynamicDefaults.sprite_down[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[4];
    ScrollCloseDynamicDefaults.sprite_down[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[5];
    ScrollCloseDynamicDefaults.sprite_left[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[6];
    ScrollCloseDynamicDefaults.sprite_left[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[7];
    ScrollCloseDynamicDefaults.sprite_left[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[8];
    ScrollCloseDynamicDefaults.sprite_right[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[9];
    ScrollCloseDynamicDefaults.sprite_right[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[10];
    ScrollCloseDynamicDefaults.sprite_right[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[11];
    ScrollCloseDynamicDefaults.thumb = &g_SCROLL_BAR_FILLER_ICON_SPRITES;
    ScrollDefaultThickness = 20;
    SpriteBoxDefaultField8 = PopupThemeWindowColor;
    EditBoxDefaultB28 = PopupThemeWindowColor;
    EditBoxDefaultB2C = PopupThemeTextShadow;
}

// ORIGINAL: 0x00588800 sub_588800 0x00588800-0x005888DE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00588800.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// TRIED: the 3 set_def_* callees needed `static` added by hand (the emitter leaves BasePop/Dialog/BaseButton members non-static); after that, MISMATCH #3 is /O2 hoisting the three unrelated global stores (6a721c/696ed0/696ed4/696ee0/697058) past the call-argument loads for scheduling - not a source-order issue, statement order already matches the Ghidra order.
// symbol    ?popups_small@@YAXXZ
void __cdecl popups_small() {
    PopupModalState = 1;
    BasePop::set_def_string_font(PopupSmallFont, PopupSmallBoldFont, PopupSmallItalicFont, 0);
    Dialog::set_def_dialog_font(PopupSmallFont, PopupSmallBoldFont, PopupSmallItalicFont);
    BasePop::set_def_button_font(PopupSmallFont, PopupSmallBoldFont, PopupSmallItalicFont);
    BasePopDefaultField21C0[0] = 3;
    BasePopDefaultField21C0[1] = 3;
    BasePopDefaultField3104[2] = 0;
    BasePop::set_def_string_color(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color2(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color3(MessageWinTextColor, -1, 1, 1);
    DialogDefaults[3] = 1;
    BaseButton::set_def_font(PopupSmallFont, 0, 0);
    BaseButton::set_def_text_color(0, -1, 1, 1);
    DialogDefaults[0] = PopupThemeSurfaceColor;
}

// ORIGINAL: 0x005888E0 ?popups_tutorial@@YAXXZ 0x005888E0-0x005889BE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005888e0.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// TRIED: the 8 callee declarations (set_def_string_font, set_def_dialog_font, set_def_button_font, set_def_string_color[/2/3], set_def_font, set_def_text_color) had to be marked `static` - the scaffold omitted `static` even though the mangled names (QAA, not QAE) and the disassembly (no ecx/this setup at any call site) both say static; a non-static call is C2352. Statement order matters where the original interleaves an unrelated global store between an argument load and its push/call (e.g. `*g_006a721c = 2;` sits between loading the three Font* values and the call, and `*g_00696ee0 = 0;`/`*g_00697058 = 1;` sit between the last argument load and their calls) - hoisting the load into a local first and placing the store right after reproduces it (MISMATCH #3 -> MNEMONIC_ONLY, all 61 mnemonics match).
// TRIED: nothing further found the last 1-byte size difference (rebuilt 221B vs original 222B, no masked-byte disagreement) - likely an add-esp or push-immediate encoding choice the optimizer makes independent of source form.
// symbol    ?popups_tutorial@@YAXXZ
void __cdecl popups_tutorial() {
    PopupModalState = 2;
    BasePop::set_def_string_font(InterludeNormalFont, InterludeSecondFont, InterludeThirdFont, 0);
    Dialog::set_def_dialog_font(InterludeNormalFont, InterludeSecondFont, InterludeThirdFont);
    BasePop::set_def_button_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont);
    BasePopDefaultField21C0[0] = 3;
    BasePopDefaultField21C0[1] = 3;
    BasePopDefaultField3104[2] = 0;
    BasePop::set_def_string_color(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color2(MessageWinTextColor, -1, 1, 1);
    BasePop::set_def_string_color3(MessageWinTextColor, -1, 1, 1);
    DialogDefaults[3] = 1;
    BaseButton::set_def_font(PopupSmallFont, 0, 0);
    BaseButton::set_def_text_color(0, -1, 1, 1);
    DialogDefaults[0] = PopupThemeSurfaceColor;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005889C0 ?popups_medium@@YAXXZ 0x005889C0-0x00588A2A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005889c0.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// symbol    ?popups_medium@@YAXXZ
void __cdecl popups_medium() {
    PopupModalState = 3;
    BasePop::set_def_string_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont, 0);
    Dialog::set_def_dialog_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont);
    BasePop::set_def_button_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont);
    BasePopDefaultField21C0[0] = 2;
    BasePopDefaultField21C0[1] = 2;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00588A30 sub_588a30 0x00588A30-0x00588A9A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00588a30.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// symbol    ?popups_interlude_medium@@YAXXZ
void __cdecl popups_interlude_medium() {
    PopupModalState = 4;
    BasePop::set_def_string_font(InterludeNormalFont, InterludeSecondFont, InterludeThirdFont, 0);
    Dialog::set_def_dialog_font(InterludeNormalFont, InterludeSecondFont, InterludeThirdFont);
    BasePop::set_def_button_font(PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont);
    BasePopDefaultField21C0[0] = 2;
    BasePopDefaultField21C0[1] = 2;
}

// MainInterface provides these shared cdecl UI transitions; Popup calls them
// through their existing free-function ABI.
void __cdecl slide_show(GraphicWin *window, int mode);   // 0x0045D440
void __cdecl slide_hide(GraphicWin *window, int mode);   // 0x0045D970
void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll);  // 0x0045E540
void __cdecl scroll_show(GraphicWin *window, Scroll *scroll);  // 0x0045DEA0


/*
Purpose: Close the popup - its scroll bar first, then the popup base.
// ORIGINAL: 0x00404900 ?close@Popup@@QAEXXZ 0x00404900-0x00404917 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?close@Popup@@QAEXXZ)(Popup* this)
// callers   104   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00600F00 0x00605370
Return Value: n/a
Status: Complete
*/
void Popup::close() {
    scroll_.close();
    BasePop::close();
}

// Close the live popup before C++ tears down its owned Scroll and BasePop.
// The original destructor inlines close, then those ordinary destructors;
// it does not destroy another complete Popup through an executable address.
// ORIGINAL: 0x00406C00 ??1Popup@@QAE@XZ 0x00406C00-0x00406E52;0x00650AB0-0x00650BA2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:0x406c00"], "reason": "Recover the full original operation using owned C++ members and base lifetimes; retain all original branches and expose unresolved callees instead of executing original-address forwarders."}
// symbol    ??1Popup@@UAE@XZ


/*
Purpose: The five-argument start form, forwarding to the six-argument one with
         no parent graphic window.
// ORIGINAL: 0x0043EF70 ?start@Popup@@QAEXPADPBDHPADH@Z 0x0043EF70-0x0043EF92 BYTE_EXACT
// symbol    ?start@Popup@@QAEXPADPBDH0H@Z
// size      34 bytes
// prototype void (__thiscall ?start@Popup@@QAEXPADPBDHPADH@Z)(Popup* this, int8* srcFileID, int8* sectionID, int, int8*, int)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00406380
Return Value: n/a
Status: Complete
*/
void Popup::start(char *text_file, const char *text_label, int value,
                  char *override_text, int flags) {
    start(text_file, text_label, value, override_text, flags, nullptr);
}


// Start's mod-redirection data (popup.h has the role note). These current
// mutable arrays cover only the observed NUL-terminated text. Original
// allocation capacities, source mutability and literal provenance remain
// unresolved; the start-argument bindings below claim no allocation spans.
// INITIAL VALUES FROM THE IMAGE (image_data.py): these are the mod-
// redirection defaults the fopen probe falls back to, and the link
// handler's match/verb/target triple - declared zero at first landing,
// the data_values census caught all ten.
// RECOVERY_OBJECT: v1 0x006827E4 ?PopupStartModTag@@3PADA
char PopupStartModTag[] = "SCRIPT.txt";        // 0x006827E4
// RECOVERY_OBJECT: v1 0x006827F4 ?PopupStartModFile1@@3PADA
char PopupStartModFile1[] = "alienIscript.txt"; // 0x006827F4
// RECOVERY_OBJECT: v1 0x006827F0 ?PopupStartModMode1@@3PADA
char PopupStartModMode1[] = "rb";              // 0x006827F0
// RECOVERY_OBJECT: v1 0x0068280C ?PopupStartModFile2@@3PADA
char PopupStartModFile2[] = "alienuscript.txt"; // 0x0068280C
// RECOVERY_OBJECT: v1 0x00682808 ?PopupStartModMode2@@3PADA
char PopupStartModMode2[] = "rb";              // 0x00682808
// The image's pointer slots select these NUL-terminated text identifiers.
// Their allocation extents are unresolved; these arrays own the observed text.
static char PopupAlienScriptName[] = "alienIscript";
static char PopupAlienOtherScriptName[] = "alienuscript";
// RECOVERY_OBJECT: v1 0x00691B20 ?PopupStartAltFile1@@3PADA
char *PopupStartAltFile1 = PopupAlienScriptName;    // 0x00691B20
// RECOVERY_OBJECT: v1 0x00691B24 ?PopupStartAltFile2@@3PADA
char *PopupStartAltFile2 = PopupAlienOtherScriptName;    // 0x00691B24

// Unit-history entries use the existing BufferJoinSpace source owner.

// 0x006A721C, the modal-popup flag every popup driver parks 6 in while its
// popup runs and clears the moment exec comes back. See the note in popup.h.
// RECOVERY_OBJECT: v2 0x006A721C 0x006A7220 ?PopupModalState@@3HA 0x004E0EA0
int PopupModalState;  // 0x006A721C

// The link handler's storage (popup.h has the role note). The image's copies
// of the three strings carry text this packed build does not ship, so ours
// initialise empty: the link branch compares against "" and opens nothing
// until something writes them.
// RECOVERY_OBJECT: v1 0x006827B8 ?PopupLinkMatchText@@3PADA
char PopupLinkMatchText[64] = "homelink";              // 0x006827B8
// RECOVERY_OBJECT: v1 0x006827DC ?PopupLinkOpenVerb@@3PADA
char PopupLinkOpenVerb[16] = "open";                    // 0x006827DC
// RECOVERY_OBJECT: v1 0x006827C4 ?PopupLinkTarget@@3PADA
char PopupLinkTarget[128] = "http:\\\\www.firaxis.com"; // 0x006827C4
// Native4050A0 publishes this buffer and44CBD0 consumes it. The existing
// 256-byte source capacity is retained; original allocation bounds unverified.
// RECOVERY_OBJECT: v1 0x007591C8 ?PopupPendingLink@@3PADA
char PopupPendingLink[256];    // 0x007591C8
// RECOVERY_OBJECT: v1 0x007592C8 ?PopupPendingLinkFlag@@3HA
int PopupPendingLinkFlag;      // 0x007592C8

/*
Purpose: Start a popup from just a label, defaulting the rest.
// ORIGINAL: 0x005A5990 ?start@Popup@@QAEXPBD@Z 0x005A5990-0x005A59AD BYTE_EXACT
// size      29 bytes
// prototype void (__thiscall ?start@Popup@@QAEXPBD@Z)(Popup* this, int8*)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00406380
Return Value: n/a
Status: Complete
*/
void Popup::start(const char *label) {
    start(PopupStartCaption, label, -1, nullptr, 0, nullptr);
}

/*
Purpose: Start a popup from a label and a value, defaulting the rest.
// ORIGINAL: 0x00559040 ?start@Popup@@QAEXPBDH@Z 0x00559040-0x0055905F BYTE_EXACT
// size      31 bytes
// prototype void (__thiscall ?start@Popup@@QAEXPBDH@Z)(Popup* this, int8* sectionID, int)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00406380
Return Value: n/a
Status: Complete
*/
void Popup::start(const char *label, int value) {
    start(PopupStartCaption, label, -1, nullptr, value, nullptr);
}



/*
Purpose: Draw a popup's dialog background; the legacy implementation does
         nothing and returns 0.
// ORIGINAL: 0x00405B10 ?on_dialog_back_draw@Popup@@QAEHPAUGraphicWin@@@Z 0x00405B10-0x00405B15 BYTE_EXACT
// symbol    ?on_dialog_back_draw@Popup@@UAEHPAVGraphicWin@@@Z
// size      5 bytes
// prototype int (__thiscall ?on_dialog_back_draw@Popup@@QAEHPAUGraphicWin@@@Z)(Popup* this, GraphicWin*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Popup::on_dialog_back_draw(::GraphicWin *) {
    return 0;
}


























/*
Purpose: Sound a popup's wave. Nothing sounds unless bit 0x400 of the flag
         word is set, the popup is armed, and its index is inside the
         forty-five entry bank. Wave 0x19 becomes 0x25 when the selected
         base is producing a secret project; wave 0x2B only sounds when the
         millisecond clock modulo three is one. With both the voiceover
         wave and the last-played bank wave idle, the index is remembered
         and its bank entry loads and plays. Afterwards - whether or not
         anything sounded - wave 0x19 fires the owner's virtual at 0x138
         and wave 0x10 plays effect 0x38.
// ORIGINAL: 0x004456B0 ?popup_wave_callback@@YAXPAUPopupWave@@H@Z 0x004456B0-0x004457BE SEMANTIC
// symbol    ?popup_wave_callback@@YAXPAUBasePopTextBlock@@H@Z
// TRIED     the two differing instructions are the vtable temp - image `mov eax, [ecx]` against the compiled `mov edx, [ecx]` - and the register is chosen by the SEAM, not by spelling. VC6 reserves `__thiscall` (C4234), so a plain `(*(Slot **)owner)[78](owner)` cannot be written at all, which is why the ORIGINAL()/pointer-to-member machinery exists; that machinery is what takes eax and pushes the vtable load into edx. Byte-exactness needs a REAL class with a no-argument virtual at slot 78. It is not Win: 0x138 is Win's on_key slot (0x005F5D10) and on_key takes four arguments while this call pushes none, so naming Win here would be invention.
// size      270 bytes
// prototype 
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00446A00 0x004C6920 0x004C6B10 0x004C6CE0
// indirect  0x00445715 0x004457A0
Return Value: n/a
Status: Complete
*/
// The last-played entry of the wave bank (popup.h). Zero-initialised, as
// the image's dword at 0x0074DAA4 is.
int PopupWaveLastIndex;  // 0x0074DAA4

void __cdecl popup_wave_callback(BasePopTextBlock *popup, int) {
    if (!(GameDataStateStorage.fields_.game_preferences_ & 0x400)) {
        return;
    }
    if (!popup) {
        return;
    }
    const int32_t index = popup->field_104_;
    if (index < 0) {
        return;
    }
    if (!popup->field_108_) {
        return;
    }
    if (index >= 0x2D) {
        return;
    }
    if (index == 0x19) {
        if (BaseCurrent->queue_production_id[0] < -FacilitySPStart) {
            popup->field_104_ = 0x25;
        }
    }
    if (popup->field_104_ == 0x2B && timeGetTime() % 3 != 1) {
        return;
    }
    const int32_t chosen = popup->field_104_;
    if (GameDataStateStorage.fields_.game_preferences_ & 0x400) {
        if (!g_WAVE_GENERAL.Wave::is_playing() &&
            !(g_CPU_WAVES + PopupWaveLastIndex)->Wave::is_playing()) {
            PopupWaveLastIndex = chosen;
            Wave *const wave = g_CPU_WAVES + chosen;
            wave->Wave::load();
            wave->Wave::play();
        }
    }
    if (popup->field_104_ == 0x19) {
        if (FactionMusic) FactionMusic->production();
    }
    if (popup->field_104_ == 0x10) {
        g_FX.FX::play(0x38);
    }
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x00404F80 ?on_redraw_nc@Popup@@QAEXPAURECT@@H@Z 0x00404F80-0x00404F83 BYTE_EXACT
// symbol    ?on_redraw_nc@Popup@@UAEXPAUtagRECT@@H@Z
// size      3 bytes
// prototype void (__thiscall ?on_redraw_nc@Popup@@QAEXPAURECT@@H@Z)(Popup* this, RECT*, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Popup::on_redraw_nc(RECT *, int) {
}


/*
Purpose: Reset the button width to 20 unless the field at 0x30AC scales to
         exactly 1000.

             mov edx,[ecx+0x30AC] / mov eax,0x68DB8BAD / imul edx
             sar edx,0xC / mov eax,edx / shr eax,0x1F / add edx,eax
             cmp edx,0x3E8 / je done
             mov dword [ecx+0x30D0],0x14

         The magic multiply is a signed divide by 10000 - the same idiom
         Datalink::UNK1 encodes in the other direction - checked against C++
         truncating division over 200,000 random dividends and the boundaries.
         0x3E8 is 1000 and 0x14 is 20.

         Both values are existing BasePop members. The original signed
         quotient is preserved explicitly despite their unsigned storage.
// ORIGINAL: 0x00405020 ?on_adjust_button_width@Popup@@QAEXXZ 0x00405020-0x0040504A BYTE_EXACT
// symbol    ?on_adjust_button_width@Popup@@UAEXXZ
// size      42 bytes
// prototype void (__thiscall ?on_adjust_button_width@Popup@@QAEXXZ)(Popup* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Popup::on_adjust_button_width() {
    const int32_t scaled = static_cast<int32_t>(field_30AC_) / 10000;
    if (scaled != 1000) {
        field_30D0_ = 20;
    }
}


/*
// ORIGINAL: 0x00404F90 ?on_nc_hittest@Popup@@QAEHHH@Z 0x00404F90-0x00404FA4 BYTE_EXACT
// symbol    ?on_nc_hittest@Popup@@UAEHHH@Z
// size      20 bytes
// prototype int (__thiscall ?on_nc_hittest@Popup@@QAEHHH@Z)(Popup* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005F5AD0
Status: Complete
*/
int Popup::on_nc_hittest(int x, int y) {
    return Win::on_nc_hittest(x, y);
}

/*
// ORIGINAL: 0x00404FB0 ?alloc@Popup@@QAAHXZ 0x00404FB0-0x0040501C;0x00650772-0x0065078F BYTE_EXACT
// symbol    ?alloc@Popup@@SAPAVBasePop@@XZ
// TRIED: spelling the allocation as manual operator-new + placement-new of BasePop diverges at the EH state-byte store and runs longer than the original; plain `new Popup` is the shape that matches.
// TRIED: manual operator-new + placement-new of Popup itself (2026-08-21) - measured 7/32 vs plain `new Popup`'s 8/32; the ternary null-check adds a `sub esp,8` the image never emits. The image inlines ??0Popup@@QAE@XZ's body here while keeping it a real out-of-line call at its other 104 sites; that per-callsite inlining split is not reachable by resurfacing the new-expression, only by a compiler heuristic this tree cannot force without breaking the other 104 callers. Left as plain `new Popup` (8/32, WRONG CALLEE stands).
// size      137 bytes
// prototype
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00600860 0x006051D0 0x0064557F 0x0064558A
//
// Promoted 2026-08-15 from src/unrecovered/00404fb0.cpp to retire its
// pending_bodies forwarder. WinMain stores this function's address into the
// popup allocator hook, so it must stay a real symbol.
Status: Complete
*/
// The image spells the return H (an int) for the allocator hook's wire
// format; the value carried is the new Popup. The hook's type is the BASE -
// the allocator contract every popup allocator satisfies.
BasePop *Popup::alloc() {
    return new Popup;
}


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x004048A0 ??0Popup@@QAE@XZ 0x004048A0-0x004048F7;0x00650760-0x00650772 BYTE_EXACT
// body      src/popup.h
// size      105 bytes
// prototype void (__thiscall ??0Popup@@QAE@XZ)(Popup* this)
// callers   104   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00600860 0x006051D0
*/

// LEVER: 0x00600860 is `??0BasePop@@QAE@XZ`, a REAL constructor (unlike
// GraphicWin/BaseButton's `construct()` seam), and 0x006051D0 is
// `??0Scroll@@QAE@XZ` for the real `scroll_` member at 0x3230 - so both
// calls are the ordinary implicit base+member construction sequence, not
// explicit body-level calls. This is why the image needs an SEH frame here
// (Scroll's construction must be able to unwind the already-built BasePop
// base) where FlatButton/PullDown/NetWin - which only call a construct()
// METHOD from the body - do not pay for one at all.
Popup::Popup() {
    // EMPTY ON PURPOSE. Both vtable stores the body used to make by hand -
    // 0x006695C8 at offset 0 and 0x006695C0 at the Buffer subobject's 0x444 -
    // are the compiler's to emit now that Buffer is a real base of GraphicWin,
    // and it puts them where the image does rather than where a body would.
}

// The shared scratch slot at 0x009BC070 base_at() (base.cpp) already names -
// its own comment there says the purpose is "inferred only from this one
// call site", so it is reused here by the same storage rather than declared
// a second time under a name this function would have to guess too.
extern int ConsoleGroupAction;  // 0x009BC070

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048CF00 sub_48cf00 0x0048CF00-0x0048CF4A BYTE_EXACT
// symbol    ?sub_48cf00@@YAHPAVBasePop@@H@Z

// 0x0048CF00 is a cdecl operation with an explicit BasePop receiver, not a
// member call. Its old exact seed and complete span remain in the mapping.
// Retained seed facts: 74 bytes at 0x0048CF00..0x0048CF4A, zero mapped
// callers, one direct target 0x006108E0; SpriteBox is the real +0x2240
// subobject and the redraw callback slot is BasePop +0x3168.
int __cdecl sub_48cf00(BasePop *owner, int vehicle) {
    Sprite *const first_vehicle_sprite = &g_VEH_SPRITES[0];
    if (!owner->picture_enabled()) {
        owner->set_caller_picture(first_vehicle_sprite);
    } else {
        owner->set_caller_picture(nullptr);
    }
    const int result = owner->dialogs().SpriteBox::sprite(
        first_vehicle_sprite, nullptr, vehicle);
    owner->set_redraw_callback(veh_sprite_draw);
    return result;
}

// ORIGINAL: 0x0048CEB0 ?veh_sprite_draw@@YAHPAUSprite@@PAUGraphicWin@@HHH@Z 0x0048CEB0-0x0048CEF2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0048ceb0.cpp: canonical original identity, complete spans and retained historical obligations", "source:src/basepop.h: real owned Dialogs, Sprite and draw callback contract", "source:src/popup.h: Popup lifetime and start interface"], "reason": "The original vehicle draw callback is installed by popv in BasePop draw callback storage3168; real Sprite and GraphicWin pointer formals, GraphicWin Buffer base and source-owned scale globals express its actual owner relationships."}
// symbol    ?veh_sprite_draw@@YAXPAVSprite@@PAVGraphicWin@@HHH@Z
void __cdecl veh_sprite_draw(Sprite *, GraphicWin *owner, int vehicle,
                           int x, int y) {
    const int scale = (SpriteScaleNumerator << 4) / SpriteScaleDenominator - 16;
    veh_draw(owner ? static_cast<Buffer *>(owner) : nullptr,
                    vehicle, x, y, scale, 0, 0);
}

// ORIGINAL: 0x0048C360 ?popv@@YAHPADPBDHH@Z 0x0048C360-0x0048C648;0x00658337-0x00658411
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0048c360.cpp: canonical original identity, complete spans and retained historical obligations", "source:src/basepop.h: real owned Dialogs, Sprite and draw callback contract", "source:src/popup.h: Popup lifetime and start interface"], "reason": "The original vehicle popup owns real Popup and Sprite locals; original48C3D0..48C400 selects caller picture when the SpriteBox list is empty but always passes the local Sprite to its embedded SpriteBox."}
// TRIED: Popup popup; Sprite sprite; as real locals (LEVER 1) reproduce the SEH frame/alloca/destructor cascade shape; MISMATCH #14 remains because the ~20 close()/dtor calls after popup.start() are done via raw ebp-relative reinterpret_cast rather than named embedded members (BasePop's dialogs_ region isn't typed granularly enough here to let the calls fall out of automatic C++ destruction).
int __cdecl popv(char *caption, const char *label, int vehicle, int flags) {
    Popup popup;
    Sprite sprite;
    sprite.create_blank(100, 75, WinFillColour);
    popup.start(caption, label, -1, nullptr, flags, nullptr);
    popup.set_caller_picture(popup.picture_enabled() ? nullptr : &sprite);
    popup.set_redraw_callback(veh_sprite_draw);
    popup.dialogs().SpriteBox::sprite(&sprite, nullptr, vehicle);
    const int result = popup.exec(0, nullptr);
    ConsoleGroupAction = popup.input_poll_state();
    return result;
}

// ORIGINAL: 0x0048C0A0 ?popp@@YAHPADPBDHPBDP6AHXZ@Z 0x0048C0A0-0x0048C31F;0x006582E0-0x00658337
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0048c0a0.cpp: canonical original identity, complete spans and retained historical obligations", "source:src/basepop.h: real owned Dialogs, Sprite and draw callback contract", "source:src/popup.h: Popup lifetime and start interface"], "reason": "The original popup picture helper belongs to the Popup owner and uses real local Popup/member lifetimes, with integer start status controlling alternate-file retry."}
// TRIED: a real automatic-storage `Popup popup;` local - BasePop already declares a non-trivial `~BasePop()`, so the compiler is forced to emit ITS OWN scope-exit destructor chain the moment such a local exists, which fights the explicit, interleaved close()/dtor/vtable-reset sequence the original shows (SEH funclet table at 0x6582E0, matched only by span - the byte comparison covers 0x48C0A0-0x48C31F only). Landed instead: a raw `unsigned char popupStorage[0x5390]` plus placement `new` to call the real constructor explicitly, then hand-written calls mirroring the disasm's own teardown order (Popup::close, Scroll::close, FlatButton::close, then explicit qualified destructors BaseButton::~BaseButton/~FlatButton/GraphicWin::~GraphicWin/ BasePop::~BasePop) and the two vtable-reset writes per stage (offset 0 and offset 0x444 - a second vtable slot common to every GraphicWin-derived sub-object here). [ebp-0x3278] traced to BasePop's own `sprite_` member via its declared offset (0x2118) plus Sprite's real size (0x2C, not the field list's apparent 13*4). Reaches MISMATCH, 100/181 mnemonics shared (0.53) over the 0x48C0A0-0x48C31F span, first divergence at #3 (no real SEH prologue, expected from the traded-off approach).
// symbol    ?popp@@YAHPBD0H0P6AHXZ@Z
int __cdecl popp(LPCSTR caption, LPCSTR label, int flags, LPCSTR picture,
                 int (__cdecl *callback)()) {
    Popup popup;
    char *alternative = nullptr;
    if (strcmp(caption, ScriptPopupFile) == 0
        || strcmp(caption, "SCRIPT.txt") == 0) {
        if (Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) {
            if (ExpansionEnabled) {
                FILE *file = fopen("alienIscript.txt", "rb");
                if (file) {
                    fclose(file);
                    alternative = PopupStartAltFile1;
                }
            }
        } else if (ExpansionEnabled && PopupDialogFactionID > 0
                   && PopupDialogFactionID < 8
                   && (Players[PopupDialogFactionID].rule_flags & RFLAG_ALIEN)) {
            FILE *file = fopen("alienuscript.txt", "rb");
            if (file) {
                fclose(file);
                alternative = PopupStartAltFile2;
            }
        }
    }
    if (!alternative || popup.start(alternative, label, -1, nullptr,
                                    flags, nullptr) != 0) {
        popup.start(const_cast<char *>(caption), label, -1, nullptr,
                    flags, nullptr);
    }
    if (picture) {
        int width, height;
        if (Buffer::get_pcx_dimensions(picture, &width, &height) == 0
            && popup.owned_picture().init(const_cast<char *>(picture), width,
                                          height) == 0) {
            popup.select_owned_picture();
        }
    }
    const int result = popup.exec(0, callback);
    ConsoleGroupAction = popup.input_poll_state();
    return result;
}

// Read before write_check() and overwritten from read_check()'s result at
// the bottom of the non-modal-close tail - a request/response pair the two
// calls share across invocations. Zero-initialised .bss in the image at
// 0x009BC06C.
// BaseWin::governor (0x00419A90) shares this checkbox request/response
// word with the popup owner; it is not translation-unit-private storage.
// RECOVERY_OBJECT: v1 0x009BC06C ?PopupWriteCheckState@@3HA
int PopupWriteCheckState;  // 0x009BC06C



/*
Purpose: Step the receiver back to the subobject ??_GPopup@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004070D0 ??_GPopup@@WEEE@AEPAXI@Z 0x004070D0-0x004070DB BYTE_EXACT
// symbol    ??_EPopup@@WEEE@AEPAXI@Z
// CORRECTED from ??3Popup@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00406BD0` into
//   ??_GPopup@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/





// ORIGINAL: 0x00404B00 ?on_redraw@Popup@@QAEXXZ 0x00404B00-0x00404B05 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:243a2cfd:src/unrecovered/00404b00.cpp;src/basepop.cpp:BasePop::on_redraw"], "reason": "Popup draw callback forwards to its real BasePop owner; image and historical seed establish the five-byte tail-call identity"}
// LEVER: The retired seed matched through a synthetic reinterpret-cast shim with masked relocation targets. This real inherited call preserves its measured body without retaining that false class shape.
// symbol    ?on_redraw@Popup@@UAEXXZ
void Popup::on_redraw() {
    BasePop::on_redraw();
}

// ORIGINAL: 0x00405050 ?on_adjust_button_placement@Popup@@QAEXPAH00@Z 0x00405050-0x00405093 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:243a2cfd:src/recovered/units/00405050.cpp;src/basepop.h:field_30AC_"], "reason": "Popup button-placement adjustment reads its inherited auto-hide mode and external window pointer, matching the original owner fields"}
// symbol    ?on_adjust_button_placement@Popup@@UAEXPAH00@Z
void Popup::on_adjust_button_placement(int *, int *y, int *) {
    if (static_cast<int>(field_30AC_) / 10000 != 1000) {
        if (external_window_ != 0) {
            *y = -30;
        } else {
            *y += 3;
        }
    }
}

// ORIGINAL: 0x00406330 ?on_listbox_scroll_delete@Popup@@QAEHPAUScroll@@@Z 0x00406330-0x0040637C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:243a2cfd:src/recovered/units/00406330.cpp;src/popup.cpp:scroll_hide"], "reason": "Popup owns the scroll lifecycle callback and forwards visibility cleanup to the existing scroll_hide helper"}
// symbol    ?on_listbox_scroll_delete@Popup@@UAEPAVScroll@@PAV2@@Z
Scroll *Popup::on_listbox_scroll_delete(Scroll *scroll) {
    if (static_cast<int>(field_30AC_) / 10000 == 1000) {
        return scroll;
    }
    if (Win::is_visible()) {
        scroll_hide(this, scroll);
    }
    return 0;
}

// ORIGINAL: 0x00404A80 ?hide@Popup@@QAEXXZ 0x00404A80-0x00404AFF FILE
// symbol    ?hide@Popup@@UAEXXZ
// size      127 bytes
// prototype void (__thiscall ?hide@Popup@@QAEXXZ)(Popup* this)
// callers   4   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D970 0x0045E540 0x005EDCD0 0x005F7E90
//
// The catalogue spells Q (a non-virtual member), but Win::hide is virtual at
// slot 2 and this has the same signature, so VC6 has no way to emit a
// non-virtual same-signature member of the same name - it is an override and
// demangles U. on_nc_hittest below carries the same catalogue-Q versus
// emitted-U gap. The tail call is `this->Win::hide()` qualified for exactly
// the reason the virtual-ness would otherwise break: unqualified `hide()`
// would dispatch through the vtable the image reaches with `call 0x5edcd0`.
// LEVER: field_30AC_ is uint32_t, so the naive `/ 1000` lowers to an unsigned
//   `mul`/`shr` pair where the image has `imul`/`sar` - reading it as
//   `static_cast<int>` first is the whole signedness story (35/45 -> 45
//   instructions on both sides).
// TRIED: plain `int` over `const int`, `register int`, and one-exit
//   `&&`-chained guards for the four-test nest - none moved the remaining
//   divergence, which is one instruction: the image pushes edi in the
//   prologue (its callee-saved home for `seconds`) while this tree pushes it
//   at first use, eleven slots in. Same register-allocation placement wall
//   pops() records below; 35/45 agreeing is this body's census.

void Popup::hide() {
    if (is_visible()) {
        const int auto_hide_selector = static_cast<int>(field_30AC_) / 10000;
        if (auto_hide_selector != 1000) {
            if (external_window_ == 0) {
                if (MainInterfaceGlobal.is_visible()) {
                    if (auto_hide_selector == 0 || auto_hide_selector == 5) {
                        if (scroll_.is_visible()) {
                            scroll_hide(this, &scroll_);
                        }
                        slide_hide(this, 0);
                    }
                }
            }
        }
        this->Win::hide();
    }
}

// ORIGINAL: 0x004455F0 ?wave_it@@YAXH@Z 0x004455F0-0x0044564D FILE
// size      93 bytes
// prototype
// callers   7   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6920 0x004C6B10 0x004C6CE0
//
// PRESERVED UNIT - measured MNEMONIC_ONLY. Scaffold preamble stripped by hand
// 2026-09-06: the body below is spelled against the real Wave / GamePreferences
// / PopupWaveLastIndex declarations, so the homing carries no fixed-slot
// bindings and no cast.
//
// The old forwarder in pending_bodies.cpp came out by hand: home_bodies.py
// clears a forwarder against the CLASS it names, and a free function has
// none, so the tool never saw this one.
// TRIED: computing `wave` before the store, `g_CPU_WAVES[n]` subscript over
//   `g_CPU_WAVES + n`, and a `const int chosen` copy of the argument - all
//   three compile to the same 29 instructions. The two that disagree with the
//   image are the second `lea` of the index chain landing in ecx where the
//   image keeps it in eax (a register-allocation placement, the same family
//   hide() records above), and the `mov [PopupWaveLastIndex], eax` store,
//   whose absolute operand is a relocation on this side and a fixed address
//   on the image's.

void __cdecl wave_it(int wave_id) {
    if (GameDataStateStorage.fields_.game_preferences_ & 0x400) {
        if (!g_WAVE_GENERAL.Wave::is_playing()) {
            if (!(g_CPU_WAVES + PopupWaveLastIndex)->Wave::is_playing()) {
                PopupWaveLastIndex = wave_id;
                Wave *const wave = g_CPU_WAVES + wave_id;
                wave->Wave::load();
                wave->Wave::play();
            }
        }
    }
}

// ORIGINAL: 0x00404EF0 ?on_post_window_init@Popup@@QAEXXZ 0x00404EF0-0x00404F7A FILE BYTE_EXACT
// symbol    ?on_post_window_init@Popup@@UAEXXZ
// working copy - scaffold materialised by --work
// size      138 bytes
// prototype void (__thiscall ?on_post_window_init@Popup@@QAEXXZ)(Popup* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F0580 0x005F05D0 0x005F0610
//
// Clear the RadioButton subobject's sprite pair at 0x2218 / 0x221c so this
// popup draws without those icons; these are pointers, not integer flags.
// TRIED: the image reads field_30B0_ (0x30b0) and `test`s it immediately
//   before the closing `call set_bottom_border_thickness(0)`, with no branch
//   anywhere near - a load and a flag store whose result nothing reads. Two
//   spellings aimed at it, `field_30B0_ != 0 ? 0 : 0` as the argument and a
//   `const int bottom = field_30B0_` local feeding `bottom - bottom`; VC6
//   folds both away and the pair stays missing. 25 of 36 agreeing, and the
//   remaining two are that dead read.

void Popup::on_post_window_init() {
    if (external_window_ == 0
        && static_cast<int>(field_30AC_) / 10000 != 1000) {
        dialogs_.RadioButton::unchecked_sprite_ = 0;
        dialogs_.RadioButton::checked_sprite_ = 0;
        set_border_thickness(0);
        set_caption_height(0);
        if (owned_text_ != 0) {
            field_16C_ = 0x1a;
            iFlags_ |= WinFlagHitTestIndirect;
        }
        if (field_30B0_ != 0) {
            set_bottom_border_thickness(0);
        } else {
            set_bottom_border_thickness(0);
        }
    }
}

// ORIGINAL: 0x004050A0 ?on_link_clicked@Popup@@QAEHHPAD@Z 0x004050A0-0x00405131 FILE BYTE_EXACT
// symbol    ?on_link_clicked@Popup@@UAEHHPAD@Z
// size      145 bytes
// prototype int (__thiscall ?on_link_clicked@Popup@@QAEHHPAD@Z)(Popup* this, int, int8*)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00604A90 0x00645470 0x0064FD20
// indirect  0x004050DE 0x004050F6
//
// Scaffold preamble stripped by hand 2026-09-06: the body below is spelled
// against the tree's own names (ExitTurnLoop, HandleMain, BasePop::fallout,
// _stricmp, strcat) plus the four link strings named by their role at the call
// site - see the block by that name in popup.cpp.

int Popup::on_link_clicked(int id, char *link) {
    if (link == 0) {
        return 1;
    }
    if (_stricmp(link, PopupLinkMatchText) == 0) {
        ExitTurnLoop = 1;
        fallout();
        ShowWindow(HandleMain, 0);
        ::ShellExecuteA(0, PopupLinkOpenVerb, PopupLinkTarget, 0, 0, 1);
        return 1;
    }
    PopupPendingLink[0] = 0;
    strcat(PopupPendingLink, link);
    PopupPendingLinkFlag = 1;
    fallout();
    return 0;
}

// ORIGINAL: 0x00406380 ?start@Popup@@QAEXPADPBDHPADHPAUGraphicWin@@@Z 0x00406380-0x004064CC FILE BYTE_EXACT
// size      332 bytes
// prototype void (__thiscall ?start@Popup@@QAEXPADPBDHPADHPAUGraphicWin@@@Z)(Popup* this, int8* srcFileID, int8* sectionID, int, int8*, int, GraphicWin*)
// callers   84   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00601BF0 0x00645598 0x00645646 0x00645660
//
// PRESERVED UNIT - measured MISMATCH. Scaffold preamble stripped by hand
// 2026-09-06: the body below is spelled against the real BasePop / Player
// declarations and the tree's own names - ScriptPopupFile (formerly exposed
// through ClimatePopupTitle), LocalFaction,
// ExpansionEnabled, PopupDialogFactionID, Players[].rule_flags - so the homing
// carries no fixed-slot bindings and no cast of this. It is the member twin of
// X_pop (xpops.cpp), which identified the same gate: only an expansion alien
// faction redirects these text files, hence the RFLAG_ALIEN probe and the two
// fopen-and-close existence tests. The seven strings and the two substitute
// pointers are the packed image's data; popup.h names each by its role.
// symbol    ?start@Popup@@QAEHPADPBDH0HPAVGraphicWin@@@Z
//   The catalogue spells both `PAD`s out and calls GraphicWin a struct (`PAU`);
//   VC6 emits the `0` back-reference for the repeated char * and `PAV` for a
//   class - and this tree's GraphicWin is a class. Same convention as the
//   five-argument overload above.
// TRIED: the frame. The image opens `push ebp / mov ebp,esp / push ecx` and
//   reads its six arguments off ebp, while this tree's version addresses them
//   off esp and lands 115 instructions against the image's 131 - the call
//   sets agree exactly (call_diff: 0 disagreements), so the gap is the frame
//   and the register homes, not the work. Structuring the same logic as one
//   exit over a hoisted `alternative` local, and splitting the two fopen
//   probes into separate scopes, both still lower without the frame.

int Popup::start(char *text_file, const char *text_label, int value,
                 char *override_text, int flags, GraphicWin *owner_window) {
    if (strcmp(text_file, ScriptPopupFile) != 0
        && strcmp(text_file, PopupStartModTag) != 0) {
        return BasePop::start(text_file, text_label, value, override_text, flags,
                              owner_window);
    }

    if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
        if (ExpansionEnabled == 0) goto fallback;
        FILE *const fp = fopen(PopupStartModFile1, PopupStartModMode1);
        if (fp == 0) goto fallback;
        fclose(fp);
        const int result = BasePop::start(PopupStartAltFile1, text_label, value,
                                           override_text, flags, owner_window);
        if (result != 0) goto fallback;
        return result;
    }

    if (ExpansionEnabled == 0) goto fallback;
    if (PopupDialogFactionID <= 0 || PopupDialogFactionID >= 8) goto fallback;
    if ((Players[PopupDialogFactionID].rule_flags & RFLAG_ALIEN) == 0) goto fallback;
    {
        FILE *const fp = fopen(PopupStartModFile2, PopupStartModMode2);
        if (fp == 0) goto fallback;
        fclose(fp);
        const int result = BasePop::start(PopupStartAltFile2, text_label, value,
                                           override_text, flags, owner_window);
        if (result != 0) goto fallback;
        return result;
    }
fallback:
    return BasePop::start(text_file, text_label, value, override_text, flags,
                          owner_window);
}

// ORIGINAL: 0x00406260 ?on_listbox_scroll_create@Popup@@QAEHXZ 0x00406260-0x00406323 FILE
// symbol    ?on_listbox_scroll_create@Popup@@UAEPAVScroll@@XZ
// size      195 bytes
// prototype int (__thiscall ?on_listbox_scroll_create@Popup@@QAEHXZ)(Popup* this)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045DEA0 0x00588340 0x005F7E90 0x006054D0
// indirect  0x00406302
//
// Scaffold preamble stripped by hand 2026-09-06: the body below is spelled
// against the real Popup/Scroll declarations. The 0x474 rectangle is the
// popup's own Buffer subobject's rect2_ (0x444 + 0x30) and the 0x4c4 width is
// the same buffer's DIB header width (0x444 + 0x7c + sizeof(BITMAPINFOHEADER))
// - reached as qualified base members, no raw offsets. The three 0x7CFF
// geometry reads are MainInterface's sprites1_[2/0/1] height members,
// initialized by its existing asset loader, not independent zeroed globals.
// TRIED: taking the rectangle as a `const RECT *const` pointer to
//   `Buffer::rect2_` - the image walks it through a register (`lea
//   ecx,[esi+0x474]; mov edx,ecx`) - and copying it into a `RECT` local
//   instead; the pointer form drops to 21 of 73 and the copy keeps the
//   committed 25, so the direct qualified reads stay. What is left is
//   register homing: the image loads the two inset globals into ebx/edx ahead
//   of the rectangle walk while this tree loads them at first use.

Scroll *Popup::on_listbox_scroll_create() {
    if (static_cast<int>(field_30AC_) / 10000 == 1000) {
        return 0;
    }
    scroll_normal();

    const RECT area = this->Buffer::rect2_;
    const int inset_top = MainInterfaceGlobal.scroll_frame_sprite(2).height();
    const int row_height = MainInterfaceGlobal.scroll_frame_sprite(1).height();
    const int gap = MainInterfaceGlobal.scroll_frame_sprite(0).height();
    const int rows = (area.bottom - inset_top - gap
                      - (area.top + 0xd) - 0x2c) / row_height;
    const int top = rows * row_height + inset_top + gap - 0xa;
    const int left = this->Buffer::dib_.bmiHeader.biWidth - 0x2b;

    scroll_.init(left, 0x15, 0x18, top, this, -1, 1);
    scroll_.hide();

    if (is_visible()) {
        scroll_show(this, &scroll_);
    }
    return &scroll_;
}

// ORIGINAL: 0x00405910 ?on_exec@Popup@@QAEHXZ 0x00405910-0x00405B01 FILE
// symbol    ?on_exec@Popup@@UAEHXZ
// TRIED: extra named locals (list_head/msg/next_idx) force `sub esp,N` frame vs original's single [ebp-4] slot + ebx/esi/edi pushes.
// working copy - scaffold materialised by --work
// size      497 bytes
// prototype int (__thiscall ?on_exec@Popup@@QAEHXZ)(Popup* this)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045C0F0 0x004713B0 0x00644F3A 0x00645470
//
// Scaffold preamble stripped by hand 2026-09-06: the body below is spelled
// against the real Popup / StringList / MainInterface / MessageWin
// declarations and the tree's own names - StringTemp, the unit_ids_ list
// LEVER: the image's two opening guards share ONE `return 1` epilogue
//   (`je 0x405a5a` twice), which only the merged `||` spelling gives VC6 -
//   separate guard clauses each grow their own epilogue and the body starts
//   by disagreeing at instruction 14 (14/158 -> 68/158).
// TRIED: reordering the switch's case labels into numeric order (1, 2, 0x3ea,
//   3, 4) to follow the image's `jg 3 / je / je 1 / je 2` test ladder, and
//   spelling the position bump as `++current_position_` instead of a `next`
//   temp - 67 and 67 of 158 against 68, so the committed form stays.
// TRIED: the residual is register homing inside the walk (the image keeps the
//   entry pointer in eax and the count in edx while this tree walks it through
//   edx) and the message ternary's zero arms, which the image lays out of line
//   and this tree in line.
// fields, field_30A8_, field_3094_, text_block_a_, MainInterfaceGlobal,
// InterfaceMessageWin, BaseIDCurrentSelected - so the homing carries no
// fixed-slot bindings and no cast of this.

int Popup::on_exec() {
    const int value = static_cast<int>(field_30AC_);
    if (value / 10000 == 1000 || value % 10000 == 0) {
        return 1;
    }
    const int code = value % 10000;

    const int total = unit_ids_.entry_count_;
    StringTemp[0] = 0;

    if (unit_ids_.entry_count_ - 1 >= -1) {
        unit_ids_.current_ = unit_ids_.head_;
        if (abs(-1) <= unit_ids_.entry_count_) {
            int n = abs(-1);
            if (n > 0) {
                do {
                    n--;
                    unit_ids_.current_ = unit_ids_.current_->previous;
                } while (n != 0);
            }
            unit_ids_.current_position_ = unit_ids_.entry_count_ - 1;
        }
    }

    for (int i = 0; i < total; i++) {
        if (i != 0) {
            strcat(StringTemp, BufferJoinSpace);
        }

        StringStructEntry *const head = unit_ids_.head_;
        if (head != 0) {
            unit_ids_.current_ = unit_ids_.current_->next;
            const int next = unit_ids_.current_position_ + 1;
            unit_ids_.current_position_ = next;
            if (next == unit_ids_.entry_count_) {
                unit_ids_.current_position_ = 0;
            }
        }

        StringStructEntry *const entry = unit_ids_.current_;
        const char *msg;
        if (entry != 0) {
            if (head != 0) {
                msg = entry->string_payload()->text;
            } else {
                // A cursor without a list head retains the original null
                // payload's text read at absolute address 4.
                StringStructPayload *payload = 0;
                msg = payload->text;
            }
        } else {
            msg = 0;
        }
        strcat(StringTemp, msg);
    }

    switch (code) {
    case 3:
    case 4:
        return 1;
    case 1:
        MainInterfaceGlobal.set_message(StringTemp, 0x5f, 0, 1, 1);
        if ((field_30A8_ & 0x400000) == 0 && field_3094_ != 0) {
            field_3094_(&text_block_a_, 1);
        }
        return 0;
    case 2:
    case 0x3ea:
        InterfaceMessageWin.add_base(BaseIDCurrentSelected, StringTemp, 0);
        if ((field_30A8_ & 0x400000) == 0 && field_3094_ != 0) {
            field_3094_(&text_block_a_, 1);
        }
        return code == 0x3ea;
    default:
        return 1;
    }
}

// ORIGINAL: 0x00559000 sub_559000 0x00559000-0x0055903A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055468E-0x005546A5:shared-Popup-receiver", "original:0x00559000-0x0055903A", "source:src/basepop.h:caller_sprite_,dialogs_", "git:b94da21d:src/recovered/units/00559000.cpp"], "reason": "The original communicate caller uses the same local Popup receiver for start and this member. Existing BasePop caller_sprite_ and owned Dialogs/SpriteBox state explain the observed member accesses. Clear only the stored caller picture when the list is nonempty; always forward the incoming picture to SpriteBox::sprite. The behavior-derived method name is provisional; the original name remains unknown."}
// symbol    ?set_picture@Popup@@QAEHPAVSprite@@HPAD@Z
// Provisional behavior-derived name; original method name is unknown.
// Historical seed evidence: direct ECX-relative field accesses identify a
// thiscall receiver, despite the old scaffold's initial free-stdcall guess.
int Popup::set_picture(Sprite *picture, int mode, char *text) {
    if (!picture_enabled()) {
        set_caller_picture(picture);
    } else {
        set_caller_picture(0);
    }
    return dialogs().sprite(picture, text, mode);
}

// ORIGINAL: 0x0050B8D0 ?POP3@@YAXPBDHHH@Z 0x0050B8D0-0x0050B8EF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x0050B8D0-0x0050B8EF", "source:src/popup.cpp:typed owner", "source:src/popup.h:POP3 overload declarations", "original:0x004055A0 full POP3 target", "seed:retained original source identity 0x0050B8D0"], "reason": "The short POP3 entry is a typed overload that delegates its original arguments to the full POP3 owner."}
// symbol    ?POP3@@YAXPBDHHH@Z
void __cdecl POP3(const char *section, int mode, int x, int y) {
    POP3(section, static_cast<const char *>(0), mode, x, y);
}

// ORIGINAL: 0x00405140 ?POP2@@YAHPBDPBDH@Z 0x00405140-0x0040559B;0x0065078F-0x00650839
// RECOVERY_BINDING: v1 {"evidence": ["pinned 0x405140..0x40559B builds BasePop at EBP-0x5390 and Scroll at EBP-0x2160 (= BasePop+0x3230), then installs Popup primary vptr 0x6695C8 and Buffer-base vptr 0x6695C0", "pinned normal and cold cleanup spans 0x65078F..0x650839 show Scroll/BasePop close and destructor order; current Popup::~Popup calls close, then compiler-owned Scroll and BasePop teardown", "source:src/popup.h Popup inherits BasePop and owns Scroll scroll_ at 0x3230; current sizeof(Popup) is 0x537C", "source:src/basepop.h unit_ids_, owned_picture(), set_caller_picture(); source:src/stringstruct.h entry payload/previous links and StringList::add", "source:src/strings.h StringTemp; src/popup.h PopupStartCaption; src/game.h IsMultiplayerPBEM; src/game_data_state_window.h lock_masks_.bytes_.human_mask_/alive_mask_ and game_warnings_; src/messagewin.h MessageWinDeferredBaseMessages[8]; src/base.h Bases[].faction_id_current; src/mapwin.h MainInterfaceGlobal", "the original rec=0x93A800..<0x93A950 walk advances by 0x30, yielding exactly queue indexes 1..7; keep that bound and the unchecked indexing behavior", "both original spans and seed history remain attached to this identity; the historical TRIED/mismatch notes remain obligations, not current owner claims"], "reason": "Recover POP2 on the real Popup object: its pinned local has Popup's vptr plus Buffer-base vptr, with BasePop at frame offset -0x5390 and its owned Scroll exactly at +0x3230. Popup construction/destruction now follows the actual class lifetime; the body keeps the original queue, text, warning, image and return paths."}
// size      1285 bytes
// prototype
// callers   4   call targets   18
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401100 0x00402500 0x00406380 0x004064D0 0x00406880 0x00406E60 0x0045C0F0 0x005D4DD0 0x005E2480 0x005E3890 0x00600860 0x00600F00 0x00602600 0x006051D0 0x00605370 0x00644F3A 0x00645470 0x00645550
// symbol    ?POP2@@YAHPBD0H@Z
// historical Status: Coverage transcription - unclaimed body; the BasePop member walks
// TRIED: a real `Popup pop;` local forces the compiler to synthesise and EMIT its own ~Popup() (Scroll has a declared, non-trivial dtor), which fails the one-external-symbol check; split into `BasePop pop; Scroll scroll;` locals instead (both already declare their own ctor/dtor) and call Popup::start() through a `(Popup*)&pop` cast. Reaches MISMATCH #19 in the construction/frame-setup area; did not chase the exact stack-slot layout (BasePop's true size vs. the scaffold's estimate) further.
int __cdecl POP2(const char *a1, const char *a2, int a3) {
    // Popup's real base/member lifetime is the native local object.
    Popup pop;

    pop.start(
        PopupStartCaption, a1, -1,
        static_cast<char *>(0), 0, static_cast<GraphicWin *>(0));

    int total = pop.unit_ids_.entry_count_;
    *StringTemp = 0;

    if (!(total - 1 < -1)) {
        StringStructEntry *entry = pop.unit_ids_.head_;
        pop.unit_ids_.current_ = entry;
        int n = abs(-1);
        if (n <= pop.unit_ids_.entry_count_) {
            n = abs(-1);
            if (n > 0) {
                do {
                    entry = entry->previous;
                    n--;
                } while (n != 0);
                pop.unit_ids_.current_ = entry;
            }
            pop.unit_ids_.current_position_ =
                pop.unit_ids_.entry_count_ - 1;
        }
    }

    int i;
    for (i = 0; i < total; i++) {
        if (i != 0) {
            strcat(StringTemp,
                   BufferJoinSpace);
        }
        pop.unit_ids_.next_entry();
        const char *text;
        StringStructEntry *entry = pop.unit_ids_.current_;
        if (entry == 0) {
            text = 0;
        } else if (pop.unit_ids_.head_ == 0) {
            // The image reads payload+4 even when the current entry exists
            // without a list head; preserve that unchecked address-4 load.
            StringStructPayload *payload = 0;
            text = payload->text;
        } else {
            text = entry->string_payload()->text;
        }
        strcat(StringTemp,
               text);
    }

    if (IsMultiplayerPBEM != 0) {
        if (a3 == -1) {
            int idx = 1;
            do {
                const unsigned char bit = static_cast<unsigned char>(
                    1u << (idx & 0x1F));
                if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & bit) != 0 &&
                    (GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & bit) != 0) {
                    StringList &pending = MessageWinDeferredBaseMessages[idx];
                    pending.pending_text_ = StringTemp;
                    pending.field_20_ = 0;
                    pending.field_24_ = 0;
                    pending.add(0x10000000);
                }
                ++idx;
            } while (idx < 8);
        } else if (a3 >= 0) {
            const uint8_t idx2 = Bases[a3].faction_id_current;
            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ &
                 static_cast<unsigned char>(1u << (idx2 & 0x1Fu))) != 0) {
                StringList &pending = MessageWinDeferredBaseMessages[idx2];
                pending.pending_text_ = StringTemp;
                pending.field_20_ = 0;
                pending.field_24_ = 0;
                pending.add(a3 | 0x40000000);
            }
        } else if (a3 != -1) {
            int neg = -1 - a3;
            unsigned char bit = static_cast<unsigned char>(1 << (neg & 0x1F));
            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & bit) != 0 &&
                (GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & bit) != 0) {
                StringList &pending = MessageWinDeferredBaseMessages[neg];
                pending.pending_text_ = StringTemp;
                pending.field_20_ = 0;
                pending.field_24_ = 0;
                pending.add(0x10000000);
            }
        }
        return 0;
    }

    MainInterfaceGlobal.set_message(
        StringTemp, 0x5F, 0, 1, 1);
    if ((GameDataStateStorage.fields_.game_warnings_ & 0x20000) == 0) {
        return 0;
    }

    if (a2 != 0) {
        int w;
        int h;
        if (Buffer::get_pcx_dimensions(a2, &h, &w) ==
            0) {
            // These are the owned picture and caller-picture roles of the real Popup.
            Sprite &sprite = pop.owned_picture();
            if (sprite.init(const_cast<char *>(a2), h, w) == 0) {
                pop.set_caller_picture(&sprite);
            }
        }
    }
    int result = pop.exec(0, nullptr);
    return result;
}

// ORIGINAL: 0x004055A0 ?POP3@@YAXPBDPBDHHH@Z 0x004055A0-0x00405910;0x00650839-0x006508CD
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x004055A0-0x00405910", "original:terranx_original.exe:0x00650839-0x006508CD", "source:src/popup.cpp:typed owner", "source:src/popup.h:Popup lifecycle", "source:src/basepop.h:unit_ids_ StringList member", "source:src/stringstruct.h:StringList cursor and payload API", "source:src/messagewin.cpp:MessageWinDeferredBaseMessages owner", "source:src/map.h:map bounds", "seed:retained original source identity 0x004055A0"], "reason": "The full POP3 operation belongs to the real Popup entry lifetime; it builds the unit-history text from the owned StringList, then routes to the existing interface message or typed per-faction PBEM queue."}
// symbol    ?POP3@@YAXPBD0HHH@Z
void __cdecl POP3(const char *section, const char *unused, int recipient,
                  int x, int y) {
    (void)unused;
    Popup popup;
    popup.start(PopupStartCaption, section, -1, 0, 0, 0);

    StringList &history = popup.unit_ids_;
    const int history_count = history.entry_count_;
    StringTemp[0] = 0;
    history.seek_pos(-1);
    for (int history_index = 0; history_index < history_count;
         ++history_index) {
        if (history_index != 0) {
            strcat(StringTemp, " ");
        }
        StringStructPayload *const history_payload = history.next_entry();
        char *const history_text = history_payload != 0
            ? history_payload->text : 0;
        strcat(StringTemp, history_text);
    }

    if (IsMultiplayerPBEM == 0) {
        MainInterfaceGlobal.set_message(StringTemp, 0x5F, 0, 1, 1);
    } else if (recipient < 1) {
        for (int faction = 1; faction < 8; ++faction) {
            const unsigned int faction_mask = 1u << faction;
            const int alive = (GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & faction_mask) != 0;
            const int human = (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_mask) != 0;
            if (alive && human && y >= 0 && y < MapLatitudeBounds &&
                x >= 0 && x < MapLongitudeBounds) {
                StringList &pending = MessageWinDeferredBaseMessages[faction];
                pending.pending_text_ = StringTemp;
                pending.field_20_ = 0;
                pending.field_24_ = 0;
                pending.add((MapLongitudeBounds * y + x) | 0x80000000);
            }
        }
    } else if (y >= 0 && y < MapLatitudeBounds && x >= 0 &&
               x < MapLongitudeBounds) {
        StringList &pending = MessageWinDeferredBaseMessages[recipient];
        pending.pending_text_ = StringTemp;
        pending.field_20_ = 0;
        pending.field_24_ = 0;
        pending.add((MapLongitudeBounds * y + x) | 0x80000000);
    }
}

// This overload calls the CRT length routine in the original, rather than
// expanding the compiler intrinsic. Keep the ordinary C API spelling.
#pragma function(strlen)

// The original numeric prompt calls strcat out of line; retain the ordinary
// CRT API and its call rather than VC6's intrinsic expansion.
#pragma function(strcat)




extern Wave g_MENU_UP_WAVE;

// ORIGINAL: 0x00404920 ?show@Popup@@QAEXH@Z 0x00404920-0x00404A7C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:0fc167ee:src/recovered/units/00404920.cpp original identity/spans and historical exact obligation", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x404920 branch/call audit including second external_window_ test at0x404A2E", "source:src/basepop.h dialogs_ and external_window_; src/listbox.h capacity; src/maininterface.h panel_height; src/menu.cpp menu-up Wave owner"], "reason": "Restore Popup show behavior through real BasePop state, Dialog virtual base, Scroll member and qualified Win base operations; historical opaque exact scaffolding is not the product owner."}
// symbol    ?show@Popup@@UAEXH@Z
void Popup::show(int requested_mode) {
    if (Win::is_visible()) {
        return;
    }

    const int auto_hide_mode = static_cast<int>(field_30AC_) / 10000;
    if (auto_hide_mode == 1000) {
        Win::show(requested_mode);
        return;
    }

    if (external_window_ == 0) {
        const int main_visible = MainInterfaceGlobal.Win::is_visible();
        if (main_visible != 0
            && (auto_hide_mode == 0 || auto_hide_mode == 5)) {
            // Native reads the top-down DIB's negative biHeight at this+4C8;
            // pixel_height() exposes its positive magnitude instead.
            const int move_y = WinScreenHeight - this->Buffer::pixel_height()
                - MainInterfaceGlobal.panel_height();
            const int move_x = (WinScreenWidth - this->Buffer::pixel_width()
                                + 0x20) / 2;
            Win::move(move_x, move_y);
            g_MENU_UP_WAVE.play();
            slide_show(this, 0);

            if (dialogs_.kind_ == 2
                && dialogs_.Dialog::item_list_.entry_count_
                       > dialogs_.ListBox::visible_item_capacity()) {
                scroll_show(this, &scroll_);
            }

            Win::show(3);
            g_MENU_UP_WAVE.fade(0xFA);
            return;
        }
    }

    if (external_window_ != 0 && dialogs_.kind_ == 2
        && dialogs_.Dialog::item_list_.entry_count_
               > dialogs_.ListBox::visible_item_capacity()) {
        scroll_show(this, &scroll_);
    }
    Win::show(requested_mode);
}



// ORIGINAL: 0x0051DCC0 sub_51dcc0 0x0051DCC0-0x0051DCF6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:short-network-popup-owner", "historical seed:src/recovered/0051dcc0.cpp", "pinned:592DE0 increment/send/decrement;51DCC0 two stack arguments and masked x86 SHL count with set/clear behavior", "source:existing NetBusyCount, NetDaemonNet.send_message and PopupWriteCheckState; no new storage or executable calls"], "reason": "Recover the original helper in its existing typed storage owner, preserving historical identity, complete span and byte obligation; descriptive source name is not original debug evidence."}
// symbol    ?popup_set_write_check_bit@@YAXHH@Z
void __cdecl popup_set_write_check_bit(int bit_index, int enabled) {
    #if !defined(_MSC_VER) || _MSC_VER != 1200 // VC6 emits the native count-masked SHL; other compilers need explicit wrapping.
    bit_index = static_cast<int>(static_cast<unsigned int>(bit_index) & 31U);
    #endif
    if (enabled != 0) { const unsigned int mask = 1U << static_cast<unsigned int>(bit_index);
        PopupWriteCheckState |= static_cast<int>(mask); } else { int value = PopupWriteCheckState;
        const unsigned int clear_mask = 1U << static_cast<unsigned int>(bit_index);
        value &= ~static_cast<int>(clear_mask);
        PopupWriteCheckState = value;
    }
}

// ORIGINAL: 0x00404860 ?sprite_draw_func@@YAXPAUSprite@@PAUGraphicWin@@HHH@Z 0x00404860-0x00404896 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/units/00404860.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x404860 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Draw callback using the real GraphicWin Buffer secondary base, Sprite transparency and native coordinate offsets."}
// symbol    ?sprite_draw_func@@YAXPAVSprite@@PAVGraphicWin@@HHH@Z
void __cdecl sprite_draw_func(Sprite *sprite, GraphicWin *owner,
                              int unused, int x, int y) {
    (void)unused;
    x -= 5;
    y -= 6;
    Buffer *buffer = owner ? static_cast<Buffer *>(owner) : 0;
    const unsigned char transparent = sprite->transparent_index();
    sprite->draw(buffer, transparent, x, y, 1, 1);
}

// ORIGINAL: 0x00404B10 ?calculate_dimensions@Popup@@QAEHXZ 0x00404B10-0x00404EEF
// RECOVERY_BINDING: v1 {"evidence": ["task:popup-dimensions-background-owner", "Native primary tables6695C8/6698D4 slots90..96 and pinned original body", "historical seed:src/unrecovered/00404b10.cpp"], "reason": "Recover a complete typed Popup override of the native BasePop callback hierarchy, using actual shared Dialog/GraphicWin bases and source-owned global objects. Preserve original identity, spans and seed discoveries; no new matching or acceptance claim."}
// symbol    ?calculate_dimensions@Popup@@UAEHXZ
// TRIED: Ghidra's decompile reads `*(int*)(self+0x21d0)+8` / `+4` as plain pointer ARITHMETIC on the vbtable slot; the raw disasm (`mov edx,[eax+8]` after `mov eax,[esi+ 0x21d0]`) shows it is a SECOND dereference - vb8/vb4 are themselves loaded from that address, not offset by it. Fixed three sites (0x222c/0x21fc, the 0x2284/0x2210/ 0x2218/0x2214/0x2204 cluster, and 0x2268) that were silently wrong single-indirection before this. Compiles and diverges at #8 (an early `mov`-vs-`cmp` reorder in the `field_0xa14==&DAT_00733990 && DAT_009b7b1c==0x320` guard); not chased further given the function's size.
int Popup::calculate_dimensions() {
    if (caller_sprite_ != nullptr) {
        const Win *owner = external_window_;
        const int screen_width = WinScreenWidth;
        if (owner == static_cast<Win *>(&DiplomacyPopup)
            && screen_width == 800 && dialogs_.kind_ != 8) {
            if (dialogs_.get_num_items() > 5) caller_sprite_ = nullptr;
        } else {
            const int sprite_width = caller_sprite_->width();
            const int border = static_cast<int>(field_30E0_);
            int width = sprite_width * static_cast<int>(field_3104_)
                / static_cast<int>(field_3108_) + border;
            int extra_width = dialogs_.Dialog::field_5C_;
            if (extra_width == 0) extra_width = dialogs_.Dialog::field_2C_;
            width += extra_width;
            if (PopupModalActive != 0) {
                width = 700;
            } else if (screen_width == 800) {
                if (width > 0x1ED) width = 0x1EE;
            } else if (screen_width - 0x16E <= width) {
                width = screen_width - 0x16E;
            }
            dialogs_.Dialog::field_2C_ = width;
        }
    }

    if (redraw_callback_ == nullptr) redraw_callback_ = sprite_draw_func;

    const int active_id = ActiveIfaceIdStorage;
    int mode = PopupModalState;
    const int active_slot = IfaceIdToSlotStorage[active_id];
    if (active_slot != 0 && active_slot != 10
        && (active_slot != 4 || external_window_ == nullptr)
        && (mode == 0 || mode == 5)) {
        mode = 6;
    }
    field_30AC_ += mode * 10000;
    if (TurnRedrawPending != 0 || mode == 6) field_30A8_ |= 0x200000;

    if (external_window_ == static_cast<Win *>(&DiplomacyPopup)) {
        string_color_a_ = TextColourTable[182];
        string_color_b_ = TextColourTable[183];
        string_color_2a_ = TextColourTable[184];
        string_color_2b_ = TextColourTable[185];
        string_color_3a_ = TextColourTable[186];
        string_color_3b_ = TextColourTable[187];
    } else {
        string_color_a_ = TextColourTable[70];
        string_color_b_ = TextColourTable[71];
        string_color_2a_ = TextColourTable[72];
        string_color_2b_ = TextColourTable[73];
        string_color_3a_ = TextColourTable[74];
        string_color_3b_ = TextColourTable[75];
    }

    if (mode == 1000) return BasePop::calculate_dimensions();

    dialogs_.Dialog::field_B4_ = 0;
    dialogs_.Dialog::field_40_ = 0;
    if (mode != 5) dialogs_.Dialog::field_48_ = 0x13;
    dialogs_.Dialog::field_44_ = 3;
    field_30D0_ = 0x14;
    if (mode != 5) {
        dialogs_.Dialog::field_34_ = dialogs_.kind_ == 1 ? 0x1B : 3;
    }

    field_21B8_ = owned_text_ != nullptr ? 0x26 : 0x12;
    const bool interface_visible = MainInterfaceGlobal.Win::is_visible() != 0;
    if (!interface_visible || (mode != 0 && mode != 5)) {
        field_21B0_ = 10;
        field_21B4_ = 0xC;
        field_21BC_ = 0x12;
        fallback_parent_window_ = nullptr;
        if (mode == 6 || mode == 2) {
            field_21B0_ = 0x13;
            field_21B4_ = 0x15;
            field_21B8_ += 9;
        }
    } else {
        field_21B0_ = 0x21;
        field_21B4_ = 0x43;
        field_21BC_ = WinScreenWidth == 800 ? 0x1E : 0x32;
        fallback_parent_window_ = static_cast<Win *>(static_cast<GraphicWin *>(&ConsoleGlobal));
    }

    field_21C4_ = 9;
    field_30A4_ = 0x10000C0;
    static_cast<GraphicWin &>(dialogs_).iFlags_ = 0x40;
    if (mode == 6) {
        field_30A4_ &= 0xFFFFFFBF;
        static_cast<GraphicWin &>(dialogs_).iFlags_ &= 0xFFFFFFBF;
    }
    field_21C0_ = 0;
    field_21C4_ = 4;

    const int result = BasePop::calculate_dimensions();
    if (result != 0) return result;

    if (external_window_ == nullptr && MainInterfaceGlobal.Win::is_visible() != 0
        && (mode == 0 || mode == 5)) {
        const int current_height = static_cast<int>(field_30C0_);
        if (current_height < 300) {
            const int delta = 300 - current_height;
            field_30C0_ = 300;
            field_30EC_ += delta;
        }
    }
    return 0;
}


// ORIGINAL: 0x00405C80 ?on_draw_background@Popup@@QAEHXZ 0x00405C80-0x00406251
// RECOVERY_BINDING: v1 {"evidence": ["task:popup-dimensions-background-owner", "Native primary tables6695C8/6698D4 slots90..96 and pinned original body", "historical seed:src/unrecovered/00405c80.cpp"], "reason": "Recover a complete typed Popup override of the native BasePop callback hierarchy, using actual shared Dialog/GraphicWin bases and source-owned global objects. Preserve original identity, spans and seed discoveries; no new matching or acceptance claim."}
// symbol    ?on_draw_background@Popup@@UAEHXZ
// TRIED: structural transcription (branch-local Buffer* target, register-vs-global box_sprite receiver per branch) compiles and matches to instr #6 (prologue push/mov order for callee-saved regs); moving the `self` cast local closer to first use did not change it.
int Popup::on_draw_background() {
  const int type = static_cast<int>(field_30AC_) / 10000;
  if (type == 1000) {
    return 0;
  }

  RECT outer;
  RECT title;
  RECT anchor;
  RECT description;
  Buffer *drawing_buffer;
  int final_left;
  int final_top;
  int final_right;
  int final_bottom;

  if (external_window_ == &DiplomacyPopup) {
    GraphicWin &diplomacy_window = DiplomacyPopup;
    RECT *diplomacy_rect = diplomacy_window.buffer_working_rect();

    Buffer::set_clip(buffer_working_rect());
    outer.left = diplomacy_rect->left + 0x17;
    outer.top = diplomacy_rect->top;
    outer.right = diplomacy_rect->right - 0x37;
    outer.bottom = diplomacy_rect->bottom + 5;
    diplomacy_window.fill(&outer, 9);

    drawing_buffer = &static_cast<Buffer &>(diplomacy_window);
    title = outer;
    drawing_buffer->box_sprite(&title, &InterfaceBoxStyles[9]);

    if (owned_text_ != 0) {
      CharUpperA(owned_text_);

      title.left = diplomacy_rect->left + 0x1b;
      title.right = diplomacy_rect->right - 0x3b;
      title.top = diplomacy_rect->top + 4;
      title.bottom = diplomacy_rect->top + 0x1c;
      drawing_buffer->box_sprite(&title, &InterfaceBoxStyles[3]);

      title.left += 3;
      title.right -= 3;
      title.bottom -= 3;
      title.top += 3;
      drawing_buffer->box_sprite(&title, &InterfaceBoxStyles[6]);

      title.left += 3;
      title.bottom -= 3;
      title.right -= 3;
      title.top += 3;
      drawing_buffer->set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
      drawing_buffer->set_text_color(TextColourTable[180],
                                     TextColourTable[181], 1, 1);
      drawing_buffer->write_cent_l(owned_text_, &title,
                                   static_cast<int>(strlen(owned_text_)));
    }

    anchor.top = field_30E8_ + field_30E0_;
    anchor.left = field_30F0_ + field_30D8_ + field_21C8_
                  + field_30E0_ - 3;
    anchor.bottom = field_30D4_ + anchor.top;
    anchor.top -= 3;
    anchor.right = outer.right - 7;

    final_left = anchor.left;
    final_top = anchor.top;
    final_right = anchor.right;
    final_bottom = anchor.bottom;
    if (field_30D4_ != 0) {
      drawing_buffer->box_sprite(&anchor, &InterfaceBoxStyles[0]);
      final_left = anchor.left;
      final_top = anchor.top;
      final_right = anchor.right;
      final_bottom = anchor.bottom;
    }
  } else {
    drawing_buffer = static_cast<Buffer *>(this);
    Buffer::set_clip(buffer_working_rect());
    GraphicWin::fill(9);

    const RECT &working_rect = Buffer::clip_rect();
    outer.left = working_rect.left;
    outer.top = working_rect.top;
    outer.right = working_rect.right;
    outer.bottom = working_rect.bottom;

    int interface_visible = MainInterfaceGlobal.Win::is_visible();
    if (interface_visible && (type == 0 || type == 5)) {
      outer.left += 0x17;
      outer.right -= 0x37;
      outer.bottom += 5;
    }

    title = outer;
    if (type == 6 || type == 2) {
      drawing_buffer->box_sprite(&title, &InterfaceStandardPopupBoxStyle);
    } else {
      drawing_buffer->box_sprite(&title, &InterfaceBoxStyles[9]);
    }

    if (owned_text_ != 0) {
      CharUpperA(owned_text_);

      title = working_rect;
      if (type == 6 || type == 2) {
        title.left += 9;
        title.right -= 9;
        title.top += 9;
        title.bottom -= 9;
      }

      interface_visible = MainInterfaceGlobal.Win::is_visible();
      if (!interface_visible || (type != 0 && type != 5)) {
        title.left += 3;
        title.right -= 3;
      } else {
        title.left += 0x1a;
        title.right -= 0x3a;
      }
      title.left += 1;
      title.right -= 1;
      title.bottom = title.top + 0x1c;
      title.top += 4;

      drawing_buffer->box_sprite(&title, &InterfaceBoxStyles[3]);
      title.left += 3;
      title.right -= 3;
      title.top += 3;
      title.bottom -= 3;
      drawing_buffer->box_sprite(&title, &InterfaceBoxStyles[6]);
      title.left += 3;
      title.right -= 3;
      title.top += 3;
      title.bottom -= 3;

      drawing_buffer->set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
      drawing_buffer->set_text_color(TextColourTable[68],
                                     TextColourTable[69], 1, 1);
      drawing_buffer->write_cent_l(owned_text_, &title,
                                   static_cast<int>(strlen(owned_text_)));
    }

    int x = field_30F0_ + field_30D8_ + field_21C8_ + field_30E0_;
    int y = field_30E8_ + field_30E0_;
    anchor.right = outer.right - 10;
    anchor.bottom = field_30D4_ + y;
    if (type == 6 || type == 2) {
      anchor.right = outer.right - 0x13;
    }
    anchor.left = x - 3;
    anchor.right += 3;
    anchor.top = y - 3;

    int has_anchor = 0;
    if (field_30D4_ != 0) {
      drawing_buffer->box_sprite(&anchor, &InterfaceBoxStyles[0]);
      has_anchor = field_30D4_;
      y = y + field_30E4_ + has_anchor;
    }

    if (field_21CC_ >= 0) {
      x = field_21CC_;
    }

    // Preserve the original branch's conditional writes. With kind_ == -1,
    // 0x406181 skips the stores at 0x40619D..0x4061AD; with no anchor,
    // 0x406205 reads the unwritten rectangle. Preserve that original error path.
    if (dialogs_.kind_ != -1) {
      description.bottom = dialogs_.Dialog::field_60_ + y;
      description.right = dialogs_.Dialog::field_5C_ + x;
      description.left = x;
      description.top = y;
      if (has_anchor != 0) {
        description.left = anchor.left;
        description.right = anchor.right;
      }
    }

    if (has_anchor != 0) {
      final_left = anchor.left;
      final_top = anchor.top;
      final_right = anchor.right;
      final_bottom = anchor.bottom;
      if (dialogs_.kind_ != -1 && caller_sprite_ == 0) {
        UnionRect(&outer, &description, &anchor);
        final_left = outer.left;
        final_top = outer.top;
        final_right = outer.right;
        final_bottom = outer.bottom;
      }
    } else {
      // Native reads these locals here even when kind_ == -1 skipped their
      // writes; preserve only that original no-anchor path.
      final_left = description.left;
      final_top = description.top;
      final_right = description.right;
      final_bottom = description.bottom;
    }
  }

  outer.bottom = final_bottom + 3;
  outer.right = final_right + 3;
  outer.top = final_top - 3;
  outer.left = final_left - 3;
  drawing_buffer->box_sprite(&outer, &InterfaceBoxStyles[3]);
  return external_window_ == 0;
}

// ORIGINAL: 0x00589330 ?time_controls_dialog@@YAXPAUPopup@@@Z 0x00589330-0x0058954A
// RECOVERY_BINDING: v1 {"evidence": ["task:time-control-calculation-and-dialog", "historical seed:src/recovered/units/00589330.cpp", "Pinned original native instructions and existing typed source layout; seed aliases independently reconciled."], "reason": "Recover all six original time-control rows through Popup-owned Dialogs, StringTable and existing label_get; preserve numeric field order and punctuation."}
// symbol    ?time_controls_dialog@@YAXPAVPopup@@@Z
void __cdecl time_controls_dialog(Popup *popup) {
    popup->start(PopupStartCaption, "TIMECONTROLS", -1, nullptr, 0x40, nullptr);
    for (int index = 0; index < MaxTimeControlNum; ++index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, StringTable.get(TimeControl[index].name));
        strcat(StringTemp, "|");
        strcat(StringTemp, "(");

        if (index != 0) {
            char turn_text[80];
            char base_text[80];
            char unit_text[80];
            _itoa(TimeControl[index].turn, turn_text, 10);
            strcat(StringTemp, turn_text);
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x468 / 4));
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x1094 / 4));
            strcat(StringTemp, " ");

            _itoa(TimeControl[index].base, base_text, 10);
            strcat(StringTemp, base_text);
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x46C / 4));
            strcat(StringTemp, " + ");

            _itoa(TimeControl[index].unit, unit_text, 10);
            strcat(StringTemp, unit_text);
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x470 / 4));
        } else {
            strcat(StringTemp, label_get(0x474 / 4));
        }

        strcat(StringTemp, ")");
        popup->dialogs().item(StringTemp, index);
    }
}

// Superseded seed removal retains historical Git provenance below.
// ORIGINAL: 0x00406B70 sub_406b70 0x00406B70-0x00406B9C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/00406b70.cpp", "task:popup-linked-dialog-helper-owners", "Pinned Popup6695C8 slot48 forwards through actual Dialogs shared GraphicWin/Win virtual base. Other observed tables66A968/66CC20/66DE44 remain unresolved cross-owner occurrences, not new owner claims."], "reason": "Recover this complete operation in its genuine C++ popup/diplomacy owner using actual members, virtual dispatch and typed shared dependencies; retain original identity, spans, claims and seed discoveries."}
// symbol    ?on_scrolled@Popup@@UAEXHH@Z
void Popup::on_scrolled(int first, int second) {
    dialogs_.on_scrolled(first, second);
}

// Superseded seed removal retains historical Git provenance below.
// ORIGINAL: 0x00406BA0 sub_406ba0 0x00406BA0-0x00406BCC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/00406ba0.cpp", "task:popup-linked-dialog-helper-owners", "Pinned Popup6695C8 slot49 forwards through actual Dialogs shared GraphicWin/Win virtual base. Other observed tables66A968/66CC20/66DE44 remain unresolved cross-owner occurrences, not new owner claims."], "reason": "Recover this complete operation in its genuine C++ popup/diplomacy owner using actual members, virtual dispatch and typed shared dependencies; retain original identity, spans, claims and seed discoveries."}
// symbol    ?on_scrolling@Popup@@UAEXHH@Z
void Popup::on_scrolling(int first, int second) {
    dialogs_.on_scrolling(first, second);
}

// Superseded seed removal retains historical Git provenance below.
// ORIGINAL: 0x0044CBD0 sub_44cbd0 0x0044CBD0-0x0044CC18 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/0044cbd0.cpp", "task:popup-linked-dialog-helper-owners", "Pinned44CBD0 clears pending flag before each two-argument X_pop5BF310. Existing Popup::on_link_clicked produces link and flag; caller/name remain provisional."], "reason": "Recover this complete operation in its genuine C++ popup/diplomacy owner using actual members, virtual dispatch and typed shared dependencies; retain original identity, spans, claims and seed discoveries."}
// symbol    ?popup_pending_link_dialog@@YAXPBD@Z
void __cdecl popup_pending_link_dialog(const char *label) {
    PopupPendingLinkFlag = 0;
    X_pop(label, 0);
    while (PopupPendingLinkFlag != 0) {
        PopupPendingLinkFlag = 0;
        X_pop(PopupPendingLink, 0);
    }
}

// Superseded seed removal retains historical Git provenance below.
// ORIGINAL: 0x0044CA90 sub_44ca90 0x0044CA90-0x0044CABA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/0044ca90.cpp", "task:popup-linked-dialog-helper-owners", "Pinned44CA90 reads Players[faction].filename with59C stride, literal DATALINKS68469C and typed caption-label wrapper5BF330; original caller and helper name remain unestablished."], "reason": "Recover this complete operation in its genuine C++ popup/diplomacy owner using actual members, virtual dispatch and typed shared dependencies; retain original identity, spans, claims and seed discoveries."}
// symbol    ?popup_faction_datalinks@@YAHH@Z
int __cdecl popup_faction_datalinks(int faction_id) {
    // Native44CA90 uses this exact NUL-terminated label. Both comparison
    // profiles emit its independent ten-byte literal COMDAT in this TU.
    // RECOVERY_LITERAL: v1 0x0068469C 0x006846A6 ??_C@_09COKO@DATALINKS?$AA@ 0x0044CA90
    return X_pop(Players[faction_id].filename, "DATALINKS",
                               0);
}

// Superseded seed removed; retained findings below are historical evidence.
// ORIGINAL: 0x0048C650 ?popb@@YAHPBDHHPBDPAUSprite@@@Z 0x0048C650-0x0048CEA2;0x00658411-0x006585CB FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:67e9243d4c82aaa8b046653cd25ad7281d0e1bef:src/unrecovered/0048c650.cpp", "pinned:terranx_original:0x48c650", "source:src/popup.cpp"], "reason": "Recover complete popup operation with native early gates, warning/focus/result effects and genuine owned Sprite/Dialogs members, never duplicate locals. Preserve complete spans, original identity and historical obligations; scoped compiler/host observations are not game acceptance."}
// symbol    ?popb@@YAHPBDHH0PAVSprite@@@Z
// TRIED: no Ghidra hypothesis was available for this one (brief carried none). This function registers a real SEH frame (mov fs:[0],esp) around a huge local Popup (which embeds Scroll/2xFlatButton/Sprite/Heap per the scaffold's own already- reverse-engineered layout) plus several more locals (Dialogs embedding Dialog+GraphicWin, a second Sprite, SpriteBox) - the destructor cascades visible in the disasm are the INLINED expansion of each object's own close()/dtor. A plain `Popup popup_;` local compiled but left `??1Popup@@QAE@XZ` as a SECOND external .text symbol (the compiler declined to inline it here, unlike the proven pattern from other landings) - fixed by placement-new into a raw byte buffer with explicit `popup_->close(); popup_->~Popup();` at every exit instead of relying on scope-exit RAII. Also needed `struct Sprite` (not `class`) and a `static` fix on `Buffer::get_pcx_dimensions` to match the catalogued mangled name (PAU not PAV; QAA not QAE). The early-return bailout conditions (3 checks on 9a64c0/9a64e8/939284/93a95c) and the mid-function dialog-population logic are a good-faith reading of the raw disasm, not independently re-verified argument- by-argument the way the other 7 addresses were - lowest-confidence landing in this batch. Compiles; not chased to byte-exact given size (2572 bytes) and time.
int __cdecl popb(LPCSTR text_label, int warning_mask, int sound_id,
                 LPCSTR picture_file, Sprite *caller_picture) {
    Popup popup;
    const int faction = BaseCurrent->faction_id_current;
    const uint8_t faction_bit = static_cast<uint8_t>(1u << faction);

    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0) {
        return 0;
    }
    if ((GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) != 0 && (GameDataStateStorage.fields_.game_state_ & STATE_UNK_8) == 0) {
        return 0;
    }
    if (faction != ConsoleGlobal.faction_id_
        && CurrentFactionEnergyReserves == 0) {
        return 0;
    }

    char popup_text[0x114];
    strcpy(popup_text, text_label);
    parse_says(0, BaseCurrent->name_string, -1, -1);

    if (sound_id >= 0) {
        g_FX.play(sound_id);
    } else if (IsMultiplayerNet != 0) {
        g_FX.play(0x21);
    }

    popup.start(PopupStartCaption, popup_text, -1, 0, 0, 0);

    popup.set_auto_hide_value(0x3EA);
    const bool warning_enabled = warning_mask != 0
        && (GameDataStateStorage.fields_.game_warnings_ & static_cast<uint32_t>(warning_mask)) != 0
        && CurrentFactionEnergyReserves == 0;
    if (!warning_enabled) {
        popup.set_auto_hide_value(2);
    } else {
        bool focus_allowed = faction == ConsoleGlobal.faction_id_
            || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0;
        if (!focus_allowed) {
            const Base &selected = Bases[BaseIDCurrentSelected];
            const uint8_t selected_owner = selected.faction_id_current;
            const uint8_t viewer_bit = static_cast<uint8_t>(
                1u << ConsoleGlobal.faction_id_);
            focus_allowed = selected_owner == ConsoleGlobal.faction_id_
                || (selected.visibility & viewer_bit) != 0;
        }
        if (focus_allowed
            && (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_ADV_ZOOM_BASE_NO_RECENTER_MAP) == 0) {
            ConsoleGlobal.focus(BaseCurrent->x, BaseCurrent->y, faction);
        }
    }

    int picture_width;
    int picture_height;
    if (picture_file != 0
        && Buffer::get_pcx_dimensions(picture_file, &picture_width,
                                      &picture_height) == 0
        && popup.owned_picture().init(const_cast<char *>(picture_file),
                                      picture_width, picture_height) == 0) {
        popup.set_caller_picture(&popup.owned_picture());
    }
    if (BaseProductionZoomRequested == 0
        && faction == ConsoleGlobal.faction_id_
        && IsMultiplayerNet == 0
        && CurrentFactionEnergyReserves == 0) {
        popup.dialogs().item(label_get(0xB0 / 4), 0);
        popup.dialogs().item(label_get(0xB4 / 4), 1);

        if ((GameDataStateStorage.fields_.game_state_ & STATE_UNK_8) != 0 && BaseCurrent->population_size > 1) {
            int facility_offset;
            int facility_mask;
            bitmask(0x23, &facility_offset, &facility_mask);
            const bool has_psi_facility =
                (Bases[BaseIDCurrentSelected].facilities_built[facility_offset]
                 & static_cast<uint8_t>(facility_mask)) != 0;
            const int specialist_flag = static_cast<int>(
                PlayersData[faction].flags) + (has_psi_facility ? 2 : 0);
            if (specialist_flag >= 0) {
                popup.dialogs().item(label_get(0x7AC / 4), 3);
            }
        }
    }

    if (warning_mask != 0) {
        popup.button(label_get(0x6EC / 4));
    }
    popup.set_value(0);
    if (caller_picture != 0) {
        popup.configure_sprite_dialog(caller_picture);
    }

    const int popup_result = popup.exec(0, 0);
    int result = popup_result;
    if (IsMultiplayerNet != 0 || CurrentFactionEnergyReserves != 0) {
        result = 1;
    }

    // Native tests BasePop::field_3100_, independently of exec's return.
    if (popup.input_poll_state() != 0) {
        GameDataStateStorage.fields_.game_warnings_ &= ~static_cast<uint32_t>(warning_mask);
        AlphaIniPrefs.announce = GameDataStateStorage.fields_.game_warnings_;
        prefs_save(0);
    }

    if (sound_id >= 0) {
        g_FX.fade(sound_id);
    }

    if (result == 0 && CurrentFactionEnergyReserves == 0) {
        BaseProductionZoomRequested = 1;
    } else if (result == 2) {
        BaseCurrent->state |= BSTATE_UNK_8000000000 | BSTATE_RESEARCH_DATA_STOLEN;
    } else if (result == 3) {
        action_staple(BaseIDCurrentSelected);
    }
    return result;
}

// Historical seed lifetime hypotheses used duplicate locals and raw placement storage.
// Native receiver evidence supersedes those models; genuine embedded member lifetimes now own cleanup.


/*
Purpose: Draw one dialog item's background for the popup: restore the
         entered window's saved clip, centre the item's close-X sprite in
         the frame when the dialog is in the icon mode, draw the panel box
         from the interface style table, then pick the item text colours
         from the pair of tables the title-bar list selects against.

// HISTORICAL SCAFFOLD ROLE: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x00405B20 ?on_dialog_item_back_draw@Popup@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x00405B20-0x00405C72 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/popup.h Popup : BasePop and existing field/dialog callback API", "source:src/basepop.h and src/dialogs.h establish the protected dialog kind member", "source:src/interface_palette.cpp textcolor_init populates source-owned TextColourTable", "source:src/diplopop.cpp source-owned DiplomacyPopup object"], "reason": "Own the dialog-item background callback as a nonvirtual Popup member: it reads Popup/BasePop state and uses the already-owned palette table, style tables and diplomacy singleton."}
// symbol    ?on_dialog_item_back_draw@Popup@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
// TRIED: RECT-by-value local plus Sprite/BoxSpriteParams table lookups (a3-indexed, stride 0x2C/0x28) compiles and matches structure; first divergence is #4 push/mov, not chased further.
// HISTORICAL emitted-symbol fact (old free-function form): ?Popup_on_dialog_item_back_draw@@YIHPAVPopup@@PAXPAVGraphicWin@@HHPAUtagRECT@@@Z
// size      338 bytes
// prototype int (__thiscall ?on_dialog_item_back_draw@Popup@@QAEHPAUGraphicWin@@HHPAURECT@@@Z)(Popup* this, GraphicWin*, int, int, RECT*)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DACB0 0x005E2DD3 0x005E4B4A
// notes     Seed provenance (TRIED): RECT-by-value local and the sprite/
//           style-table lookups (a3-indexed, stride 0x2C/0x28) match
//           structure; first divergence is #4 push/mov, not chased further.
//           The protected BasePop state stays raw-offset (field_30AC_,
//           the unnamed 0x2350 slot, external_window_) because friendship
//           does not reach a free function.
Return Value: 1 the item frame was drawn, 0 the guards rejected it
Status: Coverage transcription - unclaimed body, seed
        git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00405b20.cpp retained.
*/

int Popup::on_dialog_item_back_draw(GraphicWin *a1, int, int a3, RECT *a4) {
    if (a1 == 0) {
        return 0;
    }
    if (a4 == 0) {
        return 0;
    }
    if (static_cast<int>(field_30AC_) / 10000 == 1000) {
        return 0;
    }

    Buffer *const buffer = static_cast<Buffer *>(a1);
    buffer->restore_clip();

    RECT local_rect;
    local_rect.left = a4->left;
    local_rect.top = a4->top;
    local_rect.right = a4->right;
    local_rect.bottom = a4->bottom;

    if (dialogs_.kind_ == 1) {
        Sprite *const spr = &g_IFACE_CLOSE_X_SPRITES[a3];
        local_rect.left += 3;
        const int y = local_rect.top +
                      (local_rect.bottom - spr->height() - local_rect.top) / 2;
        spr->draw(buffer, spr->transparent_index(), local_rect.left, y, 1, 1);
        local_rect.left += 0x15;
    }

    buffer->box_sprite(&local_rect, &InterfaceBoxStyles[27 + a3]);

    if (external_window_ == static_cast<Win *>(&DiplomacyPopup)) {
        buffer->set_text_color(TextColourTable[192 - a3 * 2],
                               TextColourTable[193 - a3 * 2], 1, 1);
        return 1;
    }
    buffer->set_text_color(TextColourTable[80 - a3 * 2],
                           TextColourTable[81 - a3 * 2], 1, 1);
    return 1;
}

// The original Popup constructor at 0x004048A0 installs its Win view at
// 0x004048D9 (0x006695C8) and Buffer view at 0x004048DF (0x006695C0,
// this+0x444). The real BasePop-derived owner emits these two tables.
// These are addresspoint bindings only; original table extents and contents
// remain unverified. Current compiler table sizes do not establish them.
// RECOVERY_GENERATED: v1 vtable 0x006695C8 0x0066974C ??_7Popup@@6BWin@@@ 0x004048A0
// RECOVERY_GENERATED: v1 vtable 0x006695C0 ? ??_7Popup@@6BBuffer@@@ 0x004048A0
