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
#include <shellapi.h>
#include <mmsystem.h>
#include <cstring>
#include "original_seam.h"
#include "popup.h"
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
#include "palette.h"
#include "general.h"
#include "faction.h"

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048C320 ?popt@@YAHPADPBDHPADH@Z 0x0048C320-0x0048C353 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/0048c320.cpp", "image:terranx_original.exe:0x0048C320"], "reason": "Recover tutorial popup configuration around the existing X_pop builder, restoring normal configuration after capturing the int result. Original call order establishes ownership and corrects a stale pointer-return declaration."}
// symbol    ?popt@@YAHPADPBDH0H@Z
int __cdecl popt(char *caption, const char *label, int value, char *text, int flags) {
    popups_tutorial();
    const int result = X_pop(caption, label, value, text, flags, nullptr);
    popups_normal();
    return result;
}
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
// ORIGINAL: 0x00588AA0 ?config_popups@@YAXXZ 0x00588AA0-0x00588F8E
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00588aa0.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// symbol    ?config_popups@@YAXXZ
void __cdecl config_popups() {
    IfaceScreenWidth = WinScreenWidth;
    IfaceScreenHeight = WinScreenHeight;
    StringTemp[0] = 0;
    strcat(StringTemp, ScriptPopupFile);
    strcat(StringTemp, ".txt");
    BasePop::set_def_file(StringTemp);
    int main_size = prefs_get("MainFontSize", 16, FALSE);
    int interlude_size = prefs_get("InterludeFontSize", 16, FALSE);
    FastUnitAnim = prefs_get("FastUnitAnim", 0, FALSE);
    prefs_put("FastUnitAnim", FastUnitAnim, FALSE);
    if (!FastUnitAnim && prefs_get("SmoothUnitAnim", 0, FALSE)) FastUnitAnim = -1;
    prefs_put("SmoothUnitAnim", FastUnitAnim < 0, FALSE);
    prefs_put("MainFontSize", main_size, FALSE);
    prefs_put("InterludeFontSize", interlude_size, FALSE);
    if (WinScreenWidth == 800) { main_size -= 2; interlude_size -= 2; }
    for (int size = 0; size < 12; ++size) {
        for (int style = 0; style < 4; ++style) {
            const int height = FontSizeTable[size];
            FontTable[size * 4 + style].init(height > 12 ? DefaultLargeFontFace : DefaultFontFace,
                height, (height > 12 ? 1 : 0) | FontStyleTable[style]);
        }
    }
    // The nearest-size searches are repeated inline in the image. find_font
    // has the same strict tie rule and 9999 threshold, and returns these
    // same owned FontTable elements rather than creating another font run.
    PopupMainFont = find_font(main_size, 0);
    PopupMainItalicFont = find_font(main_size, 2);
    PopupMainBoldFont = find_font(main_size, 1);
    InterludeNormalFont = find_font(interlude_size, 0);
    InterludeThirdFont = find_font(interlude_size, 2);
    InterludeSecondFont = find_font(interlude_size, 1);
    PopupMediumFont = find_font(16, 0);
    PopupMediumItalicFont = find_font(16, 2);
    PopupMediumBoldFont = find_font(16, 1);
    PopupLargeFont = find_font(16, 0);
    PopupLargeItalicFont = find_font(16, 2);
    PopupLargeBoldFont = find_font(16, 1);
    const int menu_size = WinScreenWidth == 800 ? 12 : 16;
    PopupMenuFont = find_font(menu_size, 0);
    PopupMenuItalicFont = find_font(menu_size, 2);
    PopupMenuBoldFont = find_font(menu_size, 1);
    PopupSmallFont = find_font(12, 0);
    PopupSmallItalicFont = find_font(12, 2);
    PopupSmallBoldFont = find_font(12, 1);
    WinBubbleFont = PopupSmallFont;
    PopupDefaultWindowFont = find_font(12, 1);
    PopupMapLabelFont = find_font(10, 1);
    Menu::set_def_font(PopupMenuFont);
    PullDownDefaultFont = PopupMenuFont;
    WinDynamicDefaults.default_font_ = PopupDefaultWindowFont;
    popups_normal();
}

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
    BasePopDefaultField31A0[0] = (BasePopDefaultField31A0[0] & 0xFFFFFF00)
        | static_cast<uint8_t>(PopupThemeSurfaceColor);
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
    BasePopDefaultField31A0[1] = PopupThemeBevelUpper;
    BasePopDefaultField31A0[2] = PopupThemeBevelLower;
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
    ScrollCloseStaticDefaults[8] = 0;
    ScrollCloseStaticDefaults[3] = static_cast<uint32_t>(-1);
    ScrollCloseStaticDefaults[7] = 0x41;
    ScrollCloseStaticDefaults[5] = 0x41;
    ScrollCloseStaticDefaults[6] = 0x41;
    ScrollCloseDynamicDefaults[5] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[0]);
    ScrollCloseDynamicDefaults[6] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[1]);
    ScrollCloseDynamicDefaults[7] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[2]);
    ScrollCloseDynamicDefaults[11] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[3]);
    ScrollCloseDynamicDefaults[12] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[4]);
    ScrollCloseDynamicDefaults[13] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[5]);
    ScrollCloseDynamicDefaults[2] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[6]);
    ScrollCloseDynamicDefaults[3] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[7]);
    ScrollCloseDynamicDefaults[4] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[8]);
    ScrollCloseDynamicDefaults[8] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[9]);
    ScrollCloseDynamicDefaults[9] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[10]);
    ScrollCloseDynamicDefaults[10] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_ARROW_ICON_SPRITES[11]);
    ScrollCloseDynamicDefaults[15] = reinterpret_cast<uint32_t>(&g_SCROLL_BAR_FILLER_ICON_SPRITES);
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
    BasePopDefaultField21C0[0] = BasePopDefaultField21C0[1] = 3;
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
    BasePopDefaultField21C0[0] = BasePopDefaultField21C0[1] = 3;
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
    BasePopDefaultField21C0[0] = BasePopDefaultField21C0[1] = 2;
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
    BasePopDefaultField21C0[0] = BasePopDefaultField21C0[1] = 2;
}

// Popup owns these cdecl animation helpers; the image reaches them by direct
// calls. Their complete implementations and original mappings are below.
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
// ORIGINAL: 0x00406C00 ??1Popup@@QAE@XZ 0x00406C00-0x00406E52;0x00650AB0-0x00650BA2
// RECOVERY_BINDING: v1 {"evidence": ["image:0x406c00"], "reason": "Recover the full original operation using owned C++ members and base lifetimes; retain all original branches and expose unresolved callees instead of executing original-address forwarders."}
// symbol    ??1Popup@@UAE@XZ
Popup::~Popup() {
    close();
}



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


// AN ARRAY, NOT A POINTER. 0x009B8AA8 is in .bss - zero-filled, in no
// file section - and the image pushes its ADDRESS as an immediate
// (`push 0x9b8aa8`), which is a decayed array. Through a `char *`
// variable it is a load and a push instead.
// EXTENT IS A FLOOR: nothing recovered bounds the writes into it, so
// 256 is this tree's usual caption size rather than a measured one.
char PopupStartCaption[256];  // 0x009B8AA8

// Start's mod-redirection data (popup.h has the role note). As with the link
// strings, the image's packed copies carry text this build does not ship.
// INITIAL VALUES FROM THE IMAGE (image_data.py): these are the mod-
// redirection defaults the fopen probe falls back to, and the link
// handler's match/verb/target triple - declared zero at first landing,
// the data_values census caught all ten.
char PopupStartModTag[32] = "SCRIPT.txt";        // 0x006827E4
char PopupStartModFile1[32] = "alienIscript.txt"; // 0x006827F4
char PopupStartModMode1[8] = "rb";                // 0x006827F0
char PopupStartModFile2[32] = "alienuscript.txt"; // 0x0068280C
char PopupStartModMode2[8] = "rb";                // 0x00682808
// The image's pointer slots select these NUL-terminated text identifiers.
// Their allocation extents are unresolved; these arrays own the observed text.
static char PopupAlienScriptName[] = "alienIscript";
static char PopupAlienOtherScriptName[] = "alienuscript";
// RECOVERY_OBJECT: v1 0x00691B20 ?PopupStartAltFile1@@3PADA
char *PopupStartAltFile1 = PopupAlienScriptName;    // 0x00691B20
// RECOVERY_OBJECT: v1 0x00691B24 ?PopupStartAltFile2@@3PADA
char *PopupStartAltFile2 = PopupAlienOtherScriptName;    // 0x00691B24

// The separator between the unit-history entries, the same image .data string
// buffer.cpp and socialwin.cpp bind as SpaceText - it reads " ". Static, so
// each TU folds its own copy to the immediate the way theirs do.
static int *const SpaceText = (int *)0x00682820;

// 0x006A721C, the modal-popup flag every popup driver parks 6 in while its
// popup runs and clears the moment exec comes back. See the note in popup.h.
int PopupModalState;  // 0x006A721C

// The link handler's storage (popup.h has the role note). The image's copies
// of the three strings carry text this packed build does not ship, so ours
// initialise empty: the link branch compares against "" and opens nothing
// until something writes them.
char PopupLinkMatchText[64] = "homelink";              // 0x006827B8
char PopupLinkOpenVerb[16] = "open";                    // 0x006827DC
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
// symbol    ?on_dialog_back_draw@Popup@@QAEHPAVGraphicWin@@@Z
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
Purpose: The most minimal pop() form - just a label and callback; shared
         caption buffer, value -1, no override text, title 0, null sprite,
         both flags fixed at 1.
// ORIGINAL: 0x00627130 ?pop@@YAHPADP6AHXZ@Z 0x00627130-0x00627154 BYTE_EXACT
// symbol    ?pop_label_cb@@YAHPADP6AHXZ@Z
// size      36 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_label_cb(char *label, int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, 0, nullptr,
                            1, 1, callback);
}

/*
Purpose: Like pop_label_cb but with a caller-supplied caption in place of the
         shared buffer; value -1, no override text, title 0, null sprite,
         both flags fixed at 1.
// ORIGINAL: 0x00627160 ?pop@@YAHPADPADP6AHXZ@Z 0x00627160-0x00627184 BYTE_EXACT
// symbol    ?pop_caption@@YAHPAD0P6AHXZ@Z
// size      36 bytes
// prototype 
// callers   7   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_caption(char *caption, char *label,
                        int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, 0, nullptr, 1, 1,
                            callback);
}

/*
Purpose: Raise the caption-less, sprite-less popup with the shared caption
         buffer and caller-supplied value and title; no override text, both
         flags fixed at 1.
// ORIGINAL: 0x006271D0 ?pop@@YAHPADHHP6AHXZ@Z 0x006271D0-0x006271FA BYTE_EXACT
// symbol    ?pop_value_title@@YAHPADHHP6AHXZ@Z
// size      42 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_value_title(char *label, int value, int title,
                            int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, nullptr, title,
                            nullptr, 1, 1, callback);
}

/*
Purpose: Like pop_value_title but with a caller-supplied caption instead of
         the shared buffer; no override text, null sprite, both flags fixed
         at 1.
// ORIGINAL: 0x00627200 ?pop@@YAHPADPADHHP6AHXZ@Z 0x00627200-0x0062722A BYTE_EXACT
// symbol    ?pop_caption_value_title@@YAHPAD0HHP6AHXZ@Z
// size      42 bytes
// prototype 
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_caption_value_title(char *caption, char *label, int value,
                                    int title, int (__cdecl *callback)()) {
    return pops(caption, label, value, nullptr, title, nullptr, 1,
                            1, callback);
}

/*
Purpose: Raise the caption-less, sprite-less popup with the shared caption
         buffer, value defaulted to -1 and only the title exposed; no
         override text, both flags fixed at 1.
// ORIGINAL: 0x00627230 ?pop@@YAHPADHP6AHXZ@Z 0x00627230-0x00627257 BYTE_EXACT
// symbol    ?pop_title@@YAHPADHP6AHXZ@Z
// size      39 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_title(char *label, int title, int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, title,
                            nullptr, 1, 1, callback);
}

/*
Purpose: Like pop_title but with a caller-supplied caption instead of the
         shared buffer; value -1, no override text, null sprite, both flags
         fixed at 1.
// ORIGINAL: 0x00627260 ?pop@@YAHPADPADHP6AHXZ@Z 0x00627260-0x00627287 BYTE_EXACT
// symbol    ?pop_caption_title@@YAHPAD0HP6AHXZ@Z
// size      39 bytes
// prototype 
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_caption_title(char *caption, char *label, int title,
                              int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, title, nullptr, 1, 1,
                            callback);
}

/*
Purpose: Raise the caption-less, sprite-less popup against the shared
         caption buffer with value, override text and title all
         caller-supplied; both flags fixed at 1.
// ORIGINAL: 0x00627290 ?pop@@YAHPADHPADHP6AHXZ@Z 0x00627290-0x006272BD BYTE_EXACT
// symbol    ?pop_value_text_title@@YAHPADH0HP6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_value_text_title(char *label, int value, char *text,
                                 int title, int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, text, title,
                            nullptr, 1, 1, callback);
}

/*
Purpose: The full caption-less, sprite-less popup form - caption, value,
         override text and title all caller-supplied; null sprite, both
         flags fixed at 1.
// ORIGINAL: 0x006272C0 ?pop@@YAHPADPADHPADHP6AHXZ@Z 0x006272C0-0x006272ED BYTE_EXACT
// symbol    ?pop_full@@YAHPAD0H0HP6AHXZ@Z
// size      45 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_full(char *caption, char *label, int value, char *text,
                     int title, int (__cdecl *callback)()) {
    return pops(caption, label, value, text, title, nullptr, 1, 1,
                            callback);
}

/*
Purpose: The most minimal pops() form - label, sprite and callback; shared
         caption buffer, value -1, no override text, title 0, both flags
         fixed at 1.
// ORIGINAL: 0x00627310 ?pops@@YAHPADPAUSprite@@P6AHXZ@Z 0x00627310-0x00627337 BYTE_EXACT
// symbol    ?pops_minimal@@YAHPADPAVSprite@@P6AHXZ@Z
// size      39 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_minimal(char *label, Sprite *sprite,
                         int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, 0, sprite,
                            1, 1, callback);
}

/*
Purpose: Like pops_minimal but with both flags caller-supplied instead of
         fixed at 1; shared caption buffer, value -1, no override text,
         title 0.
// ORIGINAL: 0x00627340 ?pops@@YAHPADPAUSprite@@HHP6AHXZ@Z 0x00627340-0x0062736D BYTE_EXACT
// symbol    ?pops_flags@@YAHPADPAVSprite@@HHP6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_flags(char *label, Sprite *sprite, int flag_a, int flag_b,
                       int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, 0, sprite,
                            flag_a, flag_b, callback);
}

/*
Purpose: Like pops_minimal but with a caller-supplied caption instead of the
         shared buffer; value -1, no override text, title 0, both flags
         fixed at 1.
// ORIGINAL: 0x00627370 ?pops@@YAHPADPADPAUSprite@@P6AHXZ@Z 0x00627370-0x00627397 BYTE_EXACT
// symbol    ?pops_caption@@YAHPAD0PAVSprite@@P6AHXZ@Z
// size      39 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption(char *caption, char *label, Sprite *sprite,
                         int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, 0, sprite, 1, 1,
                            callback);
}

/*
Purpose: Like pops_flags but with a caller-supplied caption in place of the
         shared buffer; value -1, no override text, title 0.
// ORIGINAL: 0x006273A0 ?pops@@YAHPADPADPAUSprite@@HHP6AHXZ@Z 0x006273A0-0x006273CD BYTE_EXACT
// symbol    ?pops_caption_flags@@YAHPAD0PAVSprite@@HHP6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_flags(char *caption, char *label, Sprite *sprite,
                               int flag_a, int flag_b,
                               int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, 0, sprite, flag_a,
                            flag_b, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with value
         and title exposed and the sprite caller-supplied; no override
         text, both flags fixed at 1.
// ORIGINAL: 0x006273D0 ?pops@@YAHPADHHPAUSprite@@P6AHXZ@Z 0x006273D0-0x006273FD BYTE_EXACT
// symbol    ?pops_value_title@@YAHPADHHPAVSprite@@P6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_value_title(char *label, int value, int title,
                             Sprite *sprite, int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, nullptr, title,
                            sprite, 1, 1, callback);
}

/*
Purpose: Like pops_value_title but with both flags caller-supplied instead
         of fixed at 1; shared caption buffer, no override text.
// ORIGINAL: 0x00627400 ?pops@@YAHPADHHPAUSprite@@HHP6AHXZ@Z 0x00627400-0x00627433 BYTE_EXACT
// symbol    ?pops_value_title_flags@@YAHPADHHPAVSprite@@HHP6AHXZ@Z
// size      51 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_value_title_flags(char *label, int value, int title,
                                   Sprite *sprite, int flag_a, int flag_b,
                                   int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, nullptr, title,
                            sprite, flag_a, flag_b, callback);
}

/*
Purpose: Like pops_value_title but with a caller-supplied caption instead of
         the shared buffer; no override text, both flags fixed at 1.
// ORIGINAL: 0x006274D0 ?pops@@YAHPADPADHHPAUSprite@@P6AHXZ@Z 0x006274D0-0x006274FD BYTE_EXACT
// symbol    ?pops_caption_value_title@@YAHPAD0HHPAVSprite@@P6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_value_title(char *caption, char *label, int value,
                                     int title, Sprite *sprite,
                                     int (__cdecl *callback)()) {
    return pops(caption, label, value, nullptr, title, sprite, 1,
                            1, callback);
}

/*
Purpose: Raise the full popup with everything but the override text
         caller-supplied - caption, value, title, sprite and both flags.
// ORIGINAL: 0x00627500 ?pops@@YAHPADPADHHPAUSprite@@HHP6AHXZ@Z 0x00627500-0x00627533 BYTE_EXACT
// symbol    ?pops_no_text@@YAHPAD0HHPAVSprite@@HHP6AHXZ@Z
// size      51 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_no_text(char *caption, char *label, int value, int title,
                         Sprite *sprite, int flag_a, int flag_b,
                         int (__cdecl *callback)()) {
    return pops(caption, label, value, nullptr, title, sprite,
                            flag_a, flag_b, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with value
         defaulted to -1 and only the title and sprite exposed; no override
         text, both flags fixed at 1.
// ORIGINAL: 0x00627540 ?pops@@YAHPADHPAUSprite@@P6AHXZ@Z 0x00627540-0x0062756A BYTE_EXACT
// symbol    ?pops_title@@YAHPADHPAVSprite@@P6AHXZ@Z
// size      42 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_title(char *label, int title, Sprite *sprite,
                       int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, title,
                            sprite, 1, 1, callback);
}

/*
Purpose: Like pops_title but with both flags caller-supplied instead of
         fixed at 1; shared caption buffer, value -1, no override text.
// ORIGINAL: 0x00627570 ?pops@@YAHPADHPAUSprite@@HHP6AHXZ@Z 0x00627570-0x006275A0 BYTE_EXACT
// symbol    ?pops_title_flags@@YAHPADHPAVSprite@@HHP6AHXZ@Z
// size      48 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_title_flags(char *label, int title, Sprite *sprite,
                             int flag_a, int flag_b,
                             int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, title,
                            sprite, flag_a, flag_b, callback);
}

/*
Purpose: Like pops_title but with a caller-supplied caption instead of the
         shared buffer; value -1, no override text, both flags fixed at 1.
// ORIGINAL: 0x006275A0 ?pops@@YAHPADPADHPAUSprite@@P6AHXZ@Z 0x006275A0-0x006275CA BYTE_EXACT
// symbol    ?pops_caption_title@@YAHPAD0HPAVSprite@@P6AHXZ@Z
// size      42 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_title(char *caption, char *label, int title,
                               Sprite *sprite, int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, title, sprite, 1, 1,
                            callback);
}

/*
Purpose: Like pops_caption_title but with both flags caller-supplied instead
         of fixed at 1; value -1, no override text.
// ORIGINAL: 0x006275D0 ?pops@@YAHPADPADHPAUSprite@@HHP6AHXZ@Z 0x006275D0-0x00627600 BYTE_EXACT
// symbol    ?pops_caption_title_flags@@YAHPAD0HPAVSprite@@HHP6AHXZ@Z
// size      48 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_title_flags(char *caption, char *label, int title,
                                     Sprite *sprite, int flag_a, int flag_b,
                                     int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, title, sprite,
                            flag_a, flag_b, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with value,
         override text, title and sprite all caller-supplied; both flags
         fixed at 1.
// ORIGINAL: 0x00627600 ?pops@@YAHPADHPADHPAUSprite@@P6AHXZ@Z 0x00627600-0x00627630 BYTE_EXACT
// symbol    ?pops_value_text_title@@YAHPADH0HPAVSprite@@P6AHXZ@Z
// size      48 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_value_text_title(char *label, int value, char *text,
                                  int title, Sprite *sprite,
                                  int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, text, title,
                            sprite, 1, 1, callback);
}

/*
Purpose: Like pops_value_text_title but with both flags caller-supplied
         instead of fixed at 1; only the caption defaults to the shared
         buffer.
// ORIGINAL: 0x00627630 ?pops@@YAHPADHPADHPAUSprite@@HHP6AHXZ@Z 0x00627630-0x00627666 BYTE_EXACT
// symbol    ?pops_default_caption@@YAHPADH0HPAVSprite@@HHP6AHXZ@Z
// size      54 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_default_caption(char *label, int value, char *text,
                                 int title, Sprite *sprite, int flag_a,
                                 int flag_b, int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, text, title,
                            sprite, flag_a, flag_b, callback);
}

/*
Purpose: Raise the full popup with everything but the flags caller-supplied
         - caption, value, override text, title and sprite; both flags
         fixed at 1.
// ORIGINAL: 0x00627670 ?pops@@YAHPADPADHPADHPAUSprite@@P6AHXZ@Z 0x00627670-0x006276A0 BYTE_EXACT
// symbol    ?pops_no_flags@@YAHPAD0H0HPAVSprite@@P6AHXZ@Z
// size      48 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_no_flags(char *caption, char *label, int value, char *text,
                          int title, Sprite *sprite,
                          int (__cdecl *callback)()) {
    return pops(caption, label, value, text, title, sprite, 1, 1,
                            callback);
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
        if (FactionMusic) FactionMusic->general();
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
// ORIGINAL: 0x00404FB0 ?alloc@Popup@@QAAHXZ 0x00404FB0-0x0040501C;0x00650772-0x0065078F
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
    if (strcmp(caption, *reinterpret_cast<char *const *>(ClimatePopupTitle)) == 0
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

// WITHDRAWN 2026-08-22. This note used to say the runtime walk stops here
// because "DirectDraw FAILS under wine", that the error path into
// `report_error` -> `pops` was therefore faithful, and that no further source
// would move it. ALL THREE CLAIMS WERE WRONG, and they were wrong because the
// executable was being launched incorrectly, not because of anything here.
//
// Run with the project's own wine prefix, inside a wine virtual desktop, from
// `.opensmacx/game` - BOTH the shipped image and
// this build run with zero page faults, repeatably:
//
//   cd .opensmacx/game
//   WINEPREFIX=$HOME/opt/vc6/.wineprefix DISPLAY=:1 wine explorer /desktop=smac,1024x768 <exe>
//
// Under the DEFAULT prefix with no virtual desktop, the SHIPPED IMAGE faults
// identically - at 0x005F7E90, its own `Win::is_visible`, reading 0x9C off a
// null popup slot. So the crash was never evidence about this recovery at
// all; it reproduced on the original just as readily.
//
// Two singleton BasePop instances, allocated once by BasePop::init_class()
// (basepop.cpp, which stores the same two objects under the names
// PopupInstanceSlotA/B - one storage, one name) and chosen between here by
// which one is not already on screen. The slots are real pointer variables
// now: in the image they are the two zero-initialised pointer dwords at
// 0x009BC074/78, and the old `int *const` bindings into those addresses read
// and wrote unmapped memory in the standalone build - the DDRAWERROR path
// dereferenced slot A's garbage outright.
// The screen location saved across one call to `pops()` and restored on the
// next, then reset to the sentinel 0x2000 (BasePop::set_loc's own "let the
// popup centre itself" value elsewhere in this file). The image's .data
// carries 0x2000 in both dwords, so the objects initialise from it.
// RECOVERY_OBJECT: v1 0x006973DC _PopupSavedLocX
static int PopupSavedLocX = 0x2000;  // 0x006973DC
// RECOVERY_OBJECT: v1 0x006973E0 _PopupSavedLocY
static int PopupSavedLocY = 0x2000;  // 0x006973E0

// Read before write_check() and overwritten from read_check()'s result at
// the bottom of the non-modal-close tail - a request/response pair the two
// calls share across invocations. Zero-initialised .bss in the image at
// 0x009BC06C.
// BaseWin::governor (0x00419A90) shares this checkbox request/response
// word with the popup owner; it is not translation-unit-private storage.
// RECOVERY_OBJECT: v1 0x009BC06C ?PopupWriteCheckState@@3HA
int PopupWriteCheckState;  // 0x009BC06C

// The `a5 & 0x8000` tail's rectangle: BasePop's field_30B4_/_30B8_ are an
// origin, field_30BC_/_30C0_ a width and height added to that origin to
// close the rectangle. Inferred from the arithmetic pattern alone.
// Zero-initialised .bss in the image at 0x009BC058..0x009BC064.
static int PopupCloseRectLeft;    // 0x009BC058
static int PopupCloseRectTop;     // 0x009BC05C
static int PopupCloseRectRight;   // 0x009BC060
static int PopupCloseRectBottom;  // 0x009BC064

/*
// ORIGINAL: 0x006276A0 ?pops@@YAHPADPADHPADHPAUSprite@@HHP6AHXZ@Z 0x006276A0-0x006277F9
// TRIED: the pending_bodies scaffold this replaces had the two start()/sprite() success checks inverted (`== 0` where the image's `je` jumps to the CONTINUE label when the callee returns 0, i.e. success is 0 and only a nonzero result is an error), and folded read_check()'s return value into the *(self+0x3100) store instead of keeping them as the two separate globals (0x9BC06C and 0x9BC070) the image writes.
// TRIED: win-selection guard reads `*PopupInstanceSlotB` as the DEFAULT and only re-reads `*PopupInstanceSlotA` inside the `!is_visible()` branch (never binding `*PopupInstanceSlotA` to a local kept live across the call) - 91/110 -> 98/110. The `field_2274_` ternary needed its arms swapped (`== 0` first) to match the image placing the ELSE body as the fallthrough. The 0x8000 tail's four field reads declared in REVERSE offset order (0x30C0 down to 0x30B4, matching the image's load order) fixed its register choice - 87/110 -> 91/110. Plateaued at 98/110: one `add` operand order in that same tail and the prologue's exact register pick for the is_visible receiver did not yield to further reordering attempts.
// LEVER (2026-08-29): SAVE THE FLAG, THEN READ THE DEFAULT - `const BOOL winVisible = ...SlotA->is_visible(); Win *win = *SlotB; if (!winVisible) win = *SlotA;` puts the SlotB load AFTER the call like the image (0x006276AE), 98/110 -> 101/110.
// TRIED (2026-08-29): positive if/else and `c ? B : A` ternary for the win selection - both grow the `jmp` the image does not have (11/110); `if (sprite)` vs `if (sprite != 0)` identical.
// TRIED (2026-08-29): the last two clusters are scheduler placements no source shape reached. (a) The image tests the sprite pointer BEFORE the two 0x2000 stores (0x00627709); ours stores first - the branch is textually after the stores in every structured spelling. (b) The image computes w+x (into w's register edx) BEFORE the Left store; VC6 here sinks w+x below Left and Bottom, which COMMUTES the add into x's register (`add esi,edx`) because x is dead by then - named sums in the image's compute order rescheduled identically. --all-flags: 101/110 is the best across all 20 sets (the /O2 non-/Oy- family ties).
// TRIED (2026-09-04): spelling the Right sum `x + w` to give the add x's register as the DESTINATION instead - measured 103 of 110, IDENTICAL to `w + x`, still first-diverging at image instruction 86 on the same `add edx,esi` against our `add esi,edx`. (b) above still stands; 103/110 is today's census of this body, the two notes above were written at 98 and 101.
// LEVER (2026-08-29, slot conversion): the two instance slots as real `Win *`
// objects (basepop.h/basepop.cpp) instead of `int *const` bindings into
// 0x9BC074/78 - the identical folded loads, and 101/110 -> 103/110 (0.982
// similar, /O2 non-/Oy-): the win-selection `je` now agrees and the first
// divergence is the 0x8000 tail's add commute. Saved locs, write-check state
// and close rect converted the same way; every verdict on this page
// re-measured before and after, no other tier moved.
// symbol    ?pops@@YAHPAD0H0HPAVSprite@@HHP6AHXZ@Z
// size      345 bytes
// prototype
// callers   ?   call targets   9
// kind      game
// calls     0x005F7E90 0x00601BF0 0x00601B80 0x006108E0 0x00601BB0 0x00602600
//           0x005FCBB0 0x00601BD0 0x00600F00
Status: Complete - testing
*/
int __cdecl pops(char *caption, char *label, int value, char *override_text,
                 int flags, Sprite *sprite, int pair_value_a, int pair_value_b,
                 int (__cdecl *callback)()) {
    // SAVE THE FLAG, THEN READ THE DEFAULT: the image's order is
    // `mov ecx,[SlotA]; call is_visible; mov esi,[SlotB]; test eax,eax;
    // jne +6; mov esi,[SlotA]` - the SlotB default load sits AFTER the call,
    // so the visibility result has to be bound to a named local BEFORE the
    // default is read, with the guard testing the saved value. Reading the
    // default first hoisted its load above the call (98/110); if/else and
    // ternary spellings both grew a `jmp` the image does not have.
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    Win *win = PopupInstanceSlotB;
    if (!winVisible) {
        win = PopupInstanceSlotA;
    }
    BasePop *pop = reinterpret_cast<BasePop *>(win);

    if (pop->start(caption, label, value, override_text, flags, 0)) {
        return -1;
    }

    pop->set_loc(PopupSavedLocX, PopupSavedLocY);
    PopupSavedLocY = 0x2000;
    PopupSavedLocX = 0x2000;

    if (sprite) {
        if (pair_value_a != pair_value_b) {
            pop->field_3104_ = pair_value_a;
            pop->field_3108_ = pair_value_b;
        }
        if (pop->dialogs_.list_.entry_count_ == 0) {
            pop->caller_sprite_ = sprite;
        } else {
            pop->caller_sprite_ = 0;
        }

        if (pop->dialogs().sprite(sprite, 0, 0)) {
            return -1;
        }
    }

    pop->write_check(PopupWriteCheckState);
    int execResult = pop->exec(0, callback);

    if (pop->external_window_ == 0) {
        do_all_draws();
    }

    if (flags & 0x8000) {
        int h = pop->field_30C0_;
        int w = pop->field_30BC_;
        int y = pop->field_30B8_;
        int x = pop->field_30B4_;
        // NAMED SUMS IN THE IMAGE'S COMPUTE ORDER: the image adds w+x
        // (0x006277B4, into w's register) BEFORE y+h, and only then does it
        // store Left, Bottom, and finally Right - sunk past the pops. The
        // mutate-in-place spelling computed w+x last and commuted the add
        // into x's register.
        PopupCloseRectTop = y;
        const int closeRectRight = w + x;
        const int closeRectBottom = y + h;
        PopupCloseRectLeft = x;
        PopupCloseRectBottom = closeRectBottom;
        PopupCloseRectRight = closeRectRight;
        return execResult;
    }

    PopupWriteCheckState = pop->read_check();
    ConsoleGroupAction = pop->field_3100_;
    if ((flags & 0x300) == 0) {
        pop->close();
    }
    return execResult;
}

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
// symbol    ?on_listbox_scroll_delete@Popup@@QAEHPAVScroll@@@Z
int Popup::on_listbox_scroll_delete(Scroll *scroll) {
    if (static_cast<int>(field_30AC_) / 10000 == 1000) {
        return reinterpret_cast<int>(scroll);
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
        const int seconds = static_cast<int>(field_30AC_) / 1000;
        if (seconds != 1000) {
            if (external_window_ == 0) {
                if (MainInterfaceGlobal.is_visible()) {
                    if (seconds == 0 || seconds == 5) {
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

// ORIGINAL: 0x00627910 ?pop_ask@@YAHPADPADHPADP6AHXZH@Z 0x00627910-0x006279B7 FILE BYTE_EXACT
// symbol    ?pop_ask@@YAHPAD0H0P6AHXZH@Z
// working copy - scaffold materialised by --work
// size      167 bytes
// prototype int (__cdecl ?pop_ask@@YAHPADHPADP6AHXZH@Z)(int8* srcFileID, int8* sectionID, int, int8*, int (__cdecl *)(), int)
// callers   6   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90 0x005FCBB0 0x00600F00 0x00601BB0 0x00601BF0 0x00602600
//
// PRESERVED UNIT - scaffold preamble stripped by hand 2026-09-06: the body
// below is spelled against the real BasePop / Popup declarations and the
// tree's own named globals, so the homing carries no fixed-slot bindings and
// no cast. The +0x229c read is Dialogs' Dialog virtual base's
// item_list_.entry_count_, reached through the member the way
// BasePop::on_key_down reaches it.
// symbol    (above) the catalogue spells every PAD out; VC6 emits the `0`
//   back-reference for the repeated char * and that is the name this tree's
//   object really carries. Nothing about the parameters differs.

int __cdecl pop_ask(char *text_file, char *text_label, int value,
                    char *override_text, int (__cdecl *callback)(),
                    int flags) {
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    Win *win = PopupInstanceSlotB;
    if (!winVisible) {
        win = PopupInstanceSlotA;
    }
    BasePop *pop = reinterpret_cast<BasePop *>(win);

    if (pop->start(text_file, text_label, value, override_text, flags | 0x44,
                   0)) {
        return -1;
    }

    pop->write_check(PopupWriteCheckState);
    if (pop->dialogs().Dialog::item_list_.entry_count_ == 0) {
        return -1;
    }

    int execResult = pop->exec(0, callback);
    if (pop->external_window_ == 0) {
        do_all_draws();
    }
    ConsoleGroupAction = pop->field_3100_;
    pop->close();
    return execResult;
}

// ORIGINAL: 0x00627830 ?pop_ask@@YAHPADPADHP6AHXZH@Z 0x00627830-0x00627854 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["the complete original body forwards arguments 1-3 and 4-5 unchanged and supplies null for the six-argument implementation's override-text slot", "src/popup.cpp already owns the matching six-argument pop_ask implementation and the other convenience overloads", "no address call, base-slot change, or semantic normalization is required"], "reason": "Move the original five-argument pop_ask overload beside the existing six-argument implementation it forwards to, preserving the null override-text argument and return value."}
// symbol    ?pop_ask@@YAHPAD0HP6AHXZH@Z
// size      36 bytes
// prototype int (__cdecl ?pop_ask@@YAHPADPADHP6AHXZH@Z)(int8* srcFileID, int8* sectionID, int, int (__cdecl *)(), int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00627910
// historical Status: Complete
int __cdecl pop_ask(char *a1, char *a2, int a3, int (__cdecl *a4)(), int a5) {
    return pop_ask(a1, a2, a3, 0, a4, a5);
}

// The short popup overloads supply the process-wide default text file and
// delegate to the six-argument implementation above. Their original bodies
// contain no popup lifetime of their own.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00627800 ?pop_ask@@YAHPADHPADP6AHXZH@Z 0x00627800-0x00627827 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/00627800.cpp; image:0x00627800-0x00627827; source:src/popup.cpp:pop_ask"], "reason": "Original forwarding wrapper passes the process-wide PopupStartCaption and all five input arguments unchanged to the six-argument pop_ask; this is the matching product overload."}
// symbol    ?pop_ask@@YAHPADH0P6AHXZH@Z
int __cdecl pop_ask(char *section, int value, char *override_text,
                    int (__cdecl *callback)(), int flags) {
    return pop_ask(PopupStartCaption, section, value, override_text,
                   callback, flags);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00627860 ?pop_ask@@YAHPADHP6AHXZH@Z 0x00627860-0x00627884 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/00627860.cpp; image:0x00627860-0x00627884; source:src/popup.cpp:pop_ask"], "reason": "Original forwarding wrapper supplies PopupStartCaption and a null override-text argument to the six-argument pop_ask; this is the matching product overload."}
int __cdecl pop_ask(char *section, int value,
                    int (__cdecl *callback)(), int flags) {
    return pop_ask(PopupStartCaption, section, value, nullptr,
                   callback, flags);
}

// Text-taking overloads reject a null pointer before computing its length;
// the shorter one supplies the same process-wide default text file.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00627890 ?pop_ask@@YAHPADPADPADP6AHXZH@Z 0x00627890-0x006278C4 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00627890.cpp; image:0x00627890-0x006278C4; source:src/popup.cpp:pop_ask"], "reason": "Original text-taking popup overload returns -1 on null text, otherwise passes strlen(text) and all caller arguments to the six-argument implementation."}
// symbol    ?pop_ask@@YAHPAD00P6AHXZH@Z
int __cdecl pop_ask(char *text_file, char *section, char *override_text,
                    int (__cdecl *callback)(), int flags) {
    if (override_text == nullptr) {
        return -1;
    }
    return pop_ask(text_file, section, static_cast<int>(strlen(override_text)),
                   override_text, callback, flags);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x006278D0 ?pop_ask@@YAHPADPADP6AHXZH@Z 0x006278D0-0x00627904 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/006278d0.cpp; image:0x006278D0-0x00627904; source:src/popup.cpp:pop_ask"], "reason": "Original short text-taking popup overload returns -1 on null text, otherwise supplies PopupStartCaption, strlen(text), and caller arguments to the six-argument implementation."}
// symbol    ?pop_ask@@YAHPAD0P6AHXZH@Z
int __cdecl pop_ask(char *section, char *override_text,
                    int (__cdecl *callback)(), int flags) {
    if (override_text == nullptr) {
        return -1;
    }
    return pop_ask(PopupStartCaption, section,
                   static_cast<int>(strlen(override_text)), override_text,
                   callback, flags);
}

// ORIGINAL: 0x006279C0 ?pop_ask_number@@YAHPADHP6AHXZ@Z 0x006279C0-0x00627A8B FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      203 bytes
// prototype int (__cdecl ?pop_ask_number@@YAHPADHP6AHXZ@Z)(int8* sectionID, int, int (__cdecl *)())
// callers   0   call targets   9
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FCBB0 0x00600F00 0x00601BB0 0x00601BF0 0x00602600 0x00645470
//           0x0064FC88
//
// PRESERVED UNIT - measured MISMATCH. Scaffold preamble stripped by hand
// 2026-09-06: the body below is spelled against the real BasePop / Popup
// declarations and the tree's own named globals, so the homing carries no
// fixed-slot bindings and no cast.

int __cdecl pop_ask_number(char *label, int initial, int (__cdecl *callback)()) {
    char number_text[80];
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    Win *win = PopupInstanceSlotB;
    if (!winVisible) {
        win = PopupInstanceSlotA;
    }
    BasePop *pop = reinterpret_cast<BasePop *>(win);

    StringTemp[0] = 0;
    _itoa(initial, number_text, 10);
    strcat(StringTemp, number_text);

    if (pop->start(PopupStartCaption, label, 9, StringTemp, 0x44, 0)) {
        return -1;
    }

    pop->write_check(PopupWriteCheckState);
    int execResult = pop->exec(0, callback);
    if (pop->external_window_ == 0) {
        do_all_draws();
    }
    ConsoleGroupAction = pop->field_3100_;
    ParseNumTable[0] = atoi(ParseStrBuffer[0].str);
    pop->close();
    return execResult;
}

// Caption-and-label numeric prompt. The other numeric forms intentionally keep
// their own original identities; this body uses the caller's caption rather
// than the shared default and retains the same popup state/lifetime path.
// ORIGINAL: 0x00627C30 ?pop_ask_number@@YAHPADPADHP6AHXZ@Z 0x00627C30-0x00627CFC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f34af2cde626ce68311a71f2a29818e510298d7:src/unrecovered/00627c30.cpp"], "reason": "Home the original caption-and-label numeric popup in its typed popup owner; the X_pop_ask_number routing requires both arguments and this independent original span is established by the retained seed."}
// symbol    ?pop_ask_number@@YAHPAD0HP6AHXZ@Z
int __cdecl pop_ask_number(char *caption, char *label, int initial,
                           int (__cdecl *callback)()) {
    char number_text[80];
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    Win *win = PopupInstanceSlotB;
    if (!winVisible) {
        win = PopupInstanceSlotA;
    }
    BasePop *pop = reinterpret_cast<BasePop *>(win);

    StringTemp[0] = 0;
    _itoa(initial, number_text, 10);
    strcat(StringTemp, number_text);

    if (pop->start(caption, label, 9, StringTemp, 0x44, 0)) {
        return -1;
    }

    pop->write_check(PopupWriteCheckState);
    int execResult = pop->exec(0, callback);
    if (pop->external_window_ == 0) {
        do_all_draws();
    }
    ConsoleGroupAction = pop->field_3100_;
    ParseNumTable[0] = atoi(ParseStrBuffer[0].str);
    pop->close();
    return execResult;
}

// ORIGINAL: 0x00627B60 ?pop_ask_number@@YAHPADP6AHXZ@Z 0x00627B60-0x00627C28 FILE BYTE_EXACT
// size      200 bytes
// prototype
// callers   2   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FCBB0 0x00600F00 0x00601BB0 0x00601BF0 0x00602600 0x00645470
//           0x0064FC88
//
// PRESERVED UNIT - measured MISMATCH. Scaffold preamble stripped by hand
// 2026-09-06: the body below is spelled against the real BasePop / Popup
// declarations and the tree's own named globals (StringTemp, ParseNumTable,
// ParseStrBuffer, PopupWriteCheckState, BaseAtKeyPollFlag), so the homing
// carries no fixed-slot bindings and no cast. The artifact transcription this
// replaces passed the CALLBACK as start's label argument; the image reads the
// label from [esp+0x6c], which is the first parameter, so a1 it is.

int __cdecl pop_ask_number(char *label, int (__cdecl *callback)()) {
    char number_text[80];
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    Win *win = PopupInstanceSlotB;
    if (!winVisible) {
        win = PopupInstanceSlotA;
    }
    BasePop *pop = reinterpret_cast<BasePop *>(win);

    StringTemp[0] = 0;
    _itoa(0, number_text, 10);
    strcat(StringTemp, number_text);

    if (pop->start(PopupStartCaption, label, 9, StringTemp, 0x44, 0)) {
        return -1;
    }

    pop->write_check(PopupWriteCheckState);
    int execResult = pop->exec(0, callback);
    if (pop->external_window_ == 0) {
        do_all_draws();
    }
    ConsoleGroupAction = pop->field_3100_;
    ParseNumTable[0] = atoi(ParseStrBuffer[0].str);
    pop->close();
    return execResult;
}

// ORIGINAL: 0x00404EF0 ?on_post_window_init@Popup@@QAEXXZ 0x00404EF0-0x00404F7A FILE
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
        set_bottom_border_thickness(0);
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

// ORIGINAL: 0x00406380 ?start@Popup@@QAEXPADPBDHPADHPAUGraphicWin@@@Z 0x00406380-0x004064CC FILE
// size      332 bytes
// prototype void (__thiscall ?start@Popup@@QAEXPADPBDHPADHPAUGraphicWin@@@Z)(Popup* this, int8* srcFileID, int8* sectionID, int, int8*, int, GraphicWin*)
// callers   84   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00601BF0 0x00645598 0x00645646 0x00645660
//
// PRESERVED UNIT - measured MISMATCH. Scaffold preamble stripped by hand
// 2026-09-06: the body below is spelled against the real BasePop / Player
// declarations and the tree's own names - ClimatePopupTitle, LocalFaction,
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
    if (strcmp(text_file, *reinterpret_cast<char *const *>(ClimatePopupTitle)) == 0
        || strcmp(text_file, PopupStartModTag) == 0) {
        if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
            if (ExpansionEnabled == 0) {
                goto fallback;
            }
            FILE *const fp = fopen(PopupStartModFile1, PopupStartModMode1);
            if (fp == 0) {
                goto fallback;
            }
            fclose(fp);
            const int result = BasePop::start(PopupStartAltFile1,
                           text_label, value, override_text, flags,
                           owner_window);
            if (result != 0) {
                goto fallback;
            }
            return result;
        }

        if (ExpansionEnabled == 0) {
            goto fallback;
        }
        if (PopupDialogFactionID <= 0 || PopupDialogFactionID >= 8) {
            goto fallback;
        }
        if ((Players[PopupDialogFactionID].rule_flags & RFLAG_ALIEN) == 0) {
            goto fallback;
        }
        {
            FILE *const fp = fopen(PopupStartModFile2, PopupStartModMode2);
            if (fp == 0) {
                goto fallback;
            }
            fclose(fp);
            const int result = BasePop::start(
                    PopupStartAltFile2,
                    text_label, value, override_text, flags,
                    owner_window);
            if (result != 0) {
                goto fallback;
            }
            return result;
        }
    }

fallback:
    return BasePop::start(text_file, text_label, value, override_text, flags,
                   owner_window);
}

// ORIGINAL: 0x00406260 ?on_listbox_scroll_create@Popup@@QAEHXZ 0x00406260-0x00406323 FILE
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

int Popup::on_listbox_scroll_create() {
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
    return reinterpret_cast<int>(&scroll_);
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
            strcat(StringTemp, reinterpret_cast<const char *>(SpaceText));
        }

        const uint32_t head = reinterpret_cast<uint32_t>(unit_ids_.head_);
        if (head != 0) {
            unit_ids_.current_ = unit_ids_.current_->next;
            const int next = unit_ids_.current_position_ + 1;
            unit_ids_.current_position_ = next;
            if (next == unit_ids_.entry_count_) {
                unit_ids_.current_position_ = 0;
            }
        }

        const char *msg = unit_ids_.current_ == 0
            ? 0
            : *(const char **)((head == 0
                                ? 0
                                : reinterpret_cast<uint32_t>(
                                      unit_ids_.current_->string_payload())) + 4);
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

// ORIGINAL: 0x006272F0 ?pop_close@@YAXXZ 0x006272F0-0x00627306 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned6272F0..627306; retained git:b0d788d3e0ebc4d5b4c20acb834cd37455e53c67:src/recovered/006272f0.cpp historical byte obligation.", "PopupInstanceSlotA/B are the existing init_class-owned singleton pointer cells9BC074/78; BasePop derives through GraphicWin from the nonvirtual Win base."], "reason": "Close both source-owned singleton popups through the genuine BasePop receiver; retain the original sequential reload across the first close callback."}
void __cdecl pop_close() {
    static_cast<BasePop *>(PopupInstanceSlotA)->close();
    static_cast<BasePop *>(PopupInstanceSlotB)->close();
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
                    entry = *reinterpret_cast<StringStructEntry **>(
                        reinterpret_cast<char *>(entry) + 0x10);
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
            text = reinterpret_cast<const char *>(*reinterpret_cast<int *>(4));
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
    int result = pop.exec(0, reinterpret_cast<int (__cdecl *)()>(0));
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
            const int alive = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (faction_mask << 8)) != 0;
            const int human = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) != 0;
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

// ORIGINAL: 0x00627190 ?pop@@YAHPADPADPADHP6AHXZ@Z 0x00627190-0x006271CA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x00627190-0x006271CA", "source:src/popup.cpp:typed owner", "source:src/popup.h:nine-argument pops core", "source:src/popup.cpp:existing popup implementation", "seed:retained original source identity 0x00627190"], "reason": "The pop convenience wrapper preserves the original null-text result and forwards strlen plus the native default sprite and flags to the real popup builder."}
// symbol    ?pop@@YAHPAD00HP6AHXZ@Z
int __cdecl pop(char *caption, char *label, char *text, int value,
                int (__cdecl *callback)()) {
    if (text == 0) {
        return -1;
    }
    const int text_length = static_cast<int>(std::strlen(text));
    return pops(caption, label, text_length, text, value, 0, 1, 1,
                callback);
}

// ORIGINAL: 0x00627440 ?pops@@YAHPADPADPADHPAUSprite@@P6AHXZ@Z 0x00627440-0x0062747D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x00627440-0x0062747D", "source:src/popup.cpp:typed owner", "source:src/popup.h:nine-argument pops core", "source:src/popup.cpp:existing popup implementation", "seed:retained original source identity 0x00627440"], "reason": "The sprite-bearing text overload preserves the null-text return and forwards its measured length with the original default flag pair."}
// symbol    ?pops@@YAHPAD00HPAVSprite@@P6AHXZ@Z
int __cdecl pops(char *caption, char *label, char *text, int value,
                 Sprite *sprite, int (__cdecl *callback)()) {
    if (text == 0) {
        return -1;
    }
    const int text_length = static_cast<int>(std::strlen(text));
    return pops(caption, label, text_length, text, value, sprite, 1, 1,
                callback);
}

// This overload calls the CRT length routine in the original, rather than
// expanding the compiler intrinsic. Keep the ordinary C API spelling.
#pragma function(strlen)

// ORIGINAL: 0x00627480 ?pops@@YAHPADPADPADHPAUSprite@@HHP6AHXZ@Z 0x00627480-0x006274C3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x00627480-0x006274C3", "source:src/popup.cpp:typed owner", "source:src/popup.h:nine-argument pops core", "source:src/popup.cpp:existing popup implementation", "seed:retained original source identity 0x00627480"], "reason": "The sprite-and-flags text overload forwards the measured text length and both caller flags without changing callback or sprite ownership."}
// symbol    ?pops@@YAHPAD00HPAVSprite@@HHP6AHXZ@Z
int __cdecl pops(char *caption, char *label, char *text, int value,
                 Sprite *sprite, int flag_a, int flag_b,
                 int (__cdecl *callback)()) {
    if (text == 0) {
        return -1;
    }
    return pops(caption, label, static_cast<int>(std::strlen(text)), text, value, sprite,
                flag_a, flag_b, callback);
}

// The original numeric prompt calls strcat out of line; retain the ordinary
// CRT API and its call rather than VC6's intrinsic expansion.
#pragma function(strcat)

// ORIGINAL: 0x00627A90 ?pop_ask_number@@YAHPADPADP6AHXZ@Z 0x00627A90-0x00627B58 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x00627A90-0x00627B58", "source:src/basepop.h:BasePop hierarchy and popup instance slots", "source:src/popup.cpp:existing numeric popup lifecycle", "seed:retained original source identity 0x00627A90"], "reason": "Recover the complete zero-default numeric prompt through the real BasePop owner, preserving window selection, startup failure, callback execution, conditional redraw, parsed result and close order."}
// symbol    ?pop_ask_number@@YAHPAD0P6AHXZ@Z
int __cdecl pop_ask_number(char *caption, char *label,
                           int (__cdecl *callback)()) {
    char number_text[80];
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    Win *win = PopupInstanceSlotB;
    if (!winVisible) {
        win = PopupInstanceSlotA;
    }
    BasePop *pop = static_cast<BasePop *>(win);

    StringTemp[0] = 0;
    _itoa(0, number_text, 10);
    strcat(StringTemp, number_text);

    if (pop->start(caption, label, 9, StringTemp, 0x44, 0)) {
        return -1;
    }
    pop->write_check(PopupWriteCheckState);
    int execResult = pop->exec(0, callback);
    if (pop->external_window_ == 0) {
        do_all_draws();
    }
    ConsoleGroupAction = pop->field_3100_;
    ParseNumTable[0] = atoi(ParseStrBuffer[0].str);
    pop->close();
    return execResult;
}

// Scroll animation state: distinct original DWORD starts, not interface/Wave members.
// Allocation extents beyond the observed DWORD accesses remain unverified.
// RECOVERY_OBJECT: v1 0x007D38B0 ?PopupScrollHorizontalStep@@3HA
int PopupScrollHorizontalStep;
// RECOVERY_OBJECT: v1 0x007D3924 ?PopupScrollVerticalFrameCount@@3HA
int PopupScrollVerticalFrameCount;
extern int WindowAnimationFrameDelay;
extern Wave g_MAININTERFACE_WAVE;

// ORIGINAL: 0x0045D440 ?slide_show@@YAXPAUGraphicWin@@H@Z 0x0045D440-0x0045D966;0x00655757-0x00655777
// RECOVERY_BINDING: v1 {"evidence": ["git:3e11f655:src/recovered/units/0045d440.cpp complete original identity/spans and historical discoveries", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x45d440 full native stack/register/call/lifetime audit.", "source:src/maininterface.h existing Sprite arrays; src/buffer.h source-receiver copy and signed DIB height; src/console.h real virtual GraphicWin base, no raw-offset alias."], "reason": "Complete animation through typed GraphicWin/Console Buffer bases, existing MainInterface Sprite members and automatic Buffer lifetime; preserve native geometry, callbacks and virtual Scroll operations."}
// symbol    ?slide_show@@YAXPAVGraphicWin@@H@Z
void __cdecl slide_show(GraphicWin *window, int mode) {
    Buffer frame_buffer;
    Buffer background;

    if (window != 0 && window->Win::is_visible()) {
        return;
    }

    if (window != 0) {
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SOUND_EFFECTS) != 0) {
            g_MAININTERFACE_WAVE.play();
        }

        const int frame_edge = WinScreenWidth == 800 ? 0x5E : 0xEE;
        const int frame_divisor = IsMultiplayerNet != 0 ? 3 : 5;
        RECT area = window->Buffer::clip_rect();
        window->client_to_screen(&area);
        background.init(MainInterfaceGlobal.Buffer::pixel_width(),
                        MainInterfaceGlobal.Buffer::pixel_height(), 0, 0);
        background.fill(WinFillColour);
        MainInterfaceGlobal.update_window_to_buffer(&background);

        frame_buffer.resize(window->Buffer::pixel_width(),
                            window->Buffer::pixel_height());
        frame_buffer.fill(WinFillColour);
        window->update_window_to_buffer(&frame_buffer);

        int frame_count = (area.bottom - area.top) / frame_divisor;
        if (area.top < area.bottom - frame_count * frame_divisor) {
            ++frame_count;
        }

        ScreenBuffer.set_clip(&area);
        RECT animation_strip = { area.left, 0, area.right, 0x39 };
        window->client_to_screen(&animation_strip);

        Sprite &left_outer = MainInterfaceGlobal.slide_frame_sprite(0);
        Sprite &left_inner = MainInterfaceGlobal.slide_frame_sprite(1);
        Sprite &right_inner = MainInterfaceGlobal.slide_frame_sprite(2);
        Sprite &right_outer = MainInterfaceGlobal.slide_frame_sprite(3);
        const int left_x = area.left;
        const int left_inner_x = left_x + left_outer.width();
        const int right_inner_x = area.left + area.right
            - right_inner.width() - right_outer.width() - 0x20;
        const int right_x = area.left + area.right
            - right_outer.width() - 0x20;

        int animation_y = area.bottom - frame_count;
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SLIDING_WINDOWS) != 0
            && IsMultiplayerNet == 0) {
            WinFlipClipped = 1;
            for (int frame = 0; frame < frame_divisor; ++frame) {
                const DWORD started = timeGetTime();
                background.draw(&ScreenBuffer, 9, area.left, animation_y, 1, 1);

                int left_y = animation_y;
                if (left_y <= frame_edge) left_y = frame_edge;
                left_outer.draw(&ScreenBuffer, left_outer.transparent_index(),
                                left_x, left_y, 1, 1);
                left_inner.draw(&ScreenBuffer, left_inner.transparent_index(),
                                left_inner_x, animation_y, 1, 1);
                right_inner.draw(&ScreenBuffer, right_inner.transparent_index(),
                                 right_inner_x, animation_y, 1, 1);
                int right_y = animation_y;
                if (right_y <= frame_edge) right_y = frame_edge;
                right_outer.draw(&ScreenBuffer, right_outer.transparent_index(),
                                 right_x, right_y, 1, 1);

                ScreenBuffer.set_clip(&animation_strip);
                background.draw(&ScreenBuffer, 9,
                    MainInterfaceGlobal.outer_rect().left,
                    MainInterfaceGlobal.outer_rect().top, 1, 1);
                Win::flip(&area);
                animation_y -= frame_count;
                if (animation_y <= area.top) animation_y = area.top;
                ScreenBuffer.set_clip(&area);
                do_sound();
                while (static_cast<DWORD>(timeGetTime() - started)
                       < static_cast<DWORD>(WindowAnimationFrameDelay)) {
                    do_sound();
                }
            }
            ScreenBuffer.set_clip(&WinScreenClipRect);
            WinFlipClipped = 0;
        }

        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SOUND_EFFECTS) != 0) {
            g_MAININTERFACE_WAVE.fade(0x32);
        }

        RECT window_clip = window->Buffer::clip_rect();
        window->Buffer::set_clip(&window_clip);
        int left_y = animation_y;
        if (left_y <= frame_edge) left_y = frame_edge;
        left_outer.draw(static_cast<Buffer *>(window), left_outer.transparent_index(),
                        0, left_y - area.top, 1, 1);
        left_inner.draw(static_cast<Buffer *>(window), left_inner.transparent_index(),
                        left_inner_x - area.left, 0, 1, 1);
        right_inner.draw(static_cast<Buffer *>(window), right_inner.transparent_index(),
                         right_inner_x - area.left, 0, 1, 1);
        int right_y = animation_y;
        if (right_y <= frame_edge) right_y = frame_edge;
        right_outer.draw(static_cast<Buffer *>(window), right_outer.transparent_index(),
                         right_x - area.left, right_y - area.top, 1, 1);
    }

    if ((mode & 2) != 0) {
        MainInterfaceGlobal.interface_down_sprite().draw(
            static_cast<Buffer *>(&MainInterfaceGlobal),
            MainInterfaceGlobal.interface_down_sprite().transparent_index(),
            MainInterfaceGlobal.bottom_bounds().left,
            MainInterfaceGlobal.bottom_bounds().top, 1, 1);
        MainInterfaceTileFlag = 1;
        RECT interface_clip = MainInterfaceGlobal.main_layout_rect();
        MainInterfaceGlobal.Buffer::set_clip(&interface_clip);
        MainInterfaceGlobal.restore_back(0);
        RECT main_buffer_clip = MainInterfaceGlobal.Buffer::clip_rect();
        MainInterfaceGlobal.Buffer::set_clip(&main_buffer_clip);
        MainInterfaceGlobal.soft_update();
    }
    WinFlipClipped = 0;
}

// ORIGINAL: 0x0045DEA0 ?scroll_show@@YAXPAUGraphicWin@@PAUScroll@@@Z 0x0045DEA0-0x0045E535;0x006557A2-0x006557CD
// RECOVERY_BINDING: v1 {"evidence": ["git:3e11f655:src/unrecovered/0045dea0.cpp complete original identity/spans and historical discoveries", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x45dea0 full native stack/register/call/lifetime audit.", "source:src/maininterface.h existing Sprite arrays; src/buffer.h source-receiver copy and signed DIB height; src/console.h real virtual GraphicWin base, no raw-offset alias."], "reason": "Complete animation through typed GraphicWin/Console Buffer bases, existing MainInterface Sprite members and automatic Buffer lifetime; preserve native geometry, callbacks and virtual Scroll operations."}
// symbol    ?scroll_show@@YAXPAVGraphicWin@@PAVScroll@@@Z
// TRIED: full transcription of scroll_show (twin of the landed scroll_hide at 0x0045E540), using automatic Buffer ctor/dtor for the 3 local RAII objects (relying on the compiler's own SEH frame for unwind, not manual tracking), plain-register-name locals for the straight-line arithmetic, and ORIGINAL()/original_slot() for the two indirect vtable calls (slots 1 and 3). Compiles and matches through instr #9 (push vs lea) - not chased further, large SEH-heavy function, landed for coverage.
// The retained TRIED note describes the superseded initial transcription,
// not this owner body with real members, virtual calls and automatic lifetime.
void __cdecl scroll_show(GraphicWin *window, Scroll *scroll) {
    Buffer scroll_frame;
    Buffer interface_background;
    Buffer screen_save;

    if (scroll->Win::is_visible()) {
        return;
    }

    const int frame_edge = WinScreenWidth == 800 ? 0x5E : 0xEE;
    const int vertical_step = IsMultiplayerNet != 0 ? 3 : 6;

    RECT area = window->Buffer::clip_rect();
    window->client_to_screen(&area);
    const RECT original_area = area;

    interface_background.init(MainInterfaceGlobal.Buffer::pixel_width(),
                              MainInterfaceGlobal.Buffer::pixel_height(), 0, 0);
    interface_background.fill(WinFillColour);
    MainInterfaceGlobal.update_window_to_buffer(&interface_background);

    Sprite &right_edge = MainInterfaceGlobal.slide_frame_sprite(3);
    const int right_edge_x = area.right - right_edge.width() - 0x20;
    RECT vertical_strip = { right_edge_x, 0, area.right, 0x39 };
    MainInterfaceGlobal.client_to_screen(&vertical_strip);

    const int area_width = area.right - area.left;
    const int area_height = area.bottom - area.top;
    screen_save.init(area_width, area_height, 0, 0);
    RECT screen_save_area = area;
    ConsoleGlobal.screen_to_client(&screen_save_area);
    // `Buffer::copy` reads from its receiver and writes to its first argument.
    // Native 0x45E071..0x45E0A0 uses ConsoleGlobal's GraphicWin Buffer as
    // source and local F49C as destination.
    static_cast<Buffer *>(static_cast<GraphicWin *>(&ConsoleGlobal))->copy(
        &screen_save, screen_save_area.left, screen_save_area.top,
        0, 0, area_width, area_height);

    int edge_y = frame_edge;
    if (edge_y <= area.top) edge_y = area.top;
    int first_y = area.top + 0xD;
    if (first_y >= edge_y) first_y = edge_y;
    PopupScrollVerticalFrameCount = (edge_y - first_y) / vertical_step;
    // The native stores first_y over the transformed area.top at 0x45E0C1.
    area.top = first_y;
    if (PopupScrollVerticalFrameCount > 1) {
        ScreenBuffer.set_clip(&area);
        edge_y -= PopupScrollVerticalFrameCount;
        if ((GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AV_SLIDING_SCROLLBARS) != 0
            && IsMultiplayerNet == 0) {
            WinFlipClipped = 1;
            for (int frame = vertical_step; frame > 0; --frame) {
                const DWORD started = timeGetTime();
                int sprite_y = edge_y;
                if (sprite_y <= first_y) sprite_y = first_y;
                right_edge.draw(&ScreenBuffer, right_edge.transparent_index(),
                                right_edge_x, sprite_y, 1, 1);
                ScreenBuffer.set_clip(&vertical_strip);
                interface_background.draw(&ScreenBuffer, 9,
                    MainInterfaceGlobal.outer_rect().left,
                    MainInterfaceGlobal.outer_rect().top, 1, 1);
                Win::flip(&area);
                edge_y -= PopupScrollVerticalFrameCount;
                ScreenBuffer.set_clip(&area);
                do_sound();
                while (static_cast<DWORD>(timeGetTime() - started)
                       < static_cast<DWORD>(WindowAnimationFrameDelay) / 2) {
                    do_sound();
                }
            }
            ScreenBuffer.set_clip(&WinScreenClipRect);
            WinFlipClipped = 0;
        }
        RECT window_clip = window->Buffer::clip_rect();
        window->Buffer::set_clip(&window_clip);
    }

    // The captured image keeps its original height. The panel is created
    // after area.top changes, so its negative DIB height is different.
    const int frame_height = area.bottom - area.top;
    scroll_frame.init(area_width, frame_height, 0, 0);
    scroll_frame.fill(9);
    Sprite &upper_left = MainInterfaceGlobal.scroll_frame_sprite(0);
    Sprite &top_edge = MainInterfaceGlobal.scroll_frame_sprite(1);
    Sprite &lower_left = MainInterfaceGlobal.scroll_frame_sprite(2);
    Sprite &side_edge = MainInterfaceGlobal.scroll_frame_sprite(3);

    const int panel_bottom = upper_left.height()
        - ((-frame_height + lower_left.height() + 0x2C + upper_left.height())
           / top_edge.height()) * top_edge.height()
        + lower_left.height();
    scroll_frame.tile(&top_edge, 0, 0, 0, 3, top_edge.width(), panel_bottom);
    scroll_frame.tile(&side_edge, 0, panel_bottom + 3, 0, panel_bottom,
                      side_edge.width(), frame_height - panel_bottom);
    upper_left.draw(&scroll_frame, upper_left.transparent_index(), 0, 3, 1, 1);
    lower_left.draw(&scroll_frame, lower_left.transparent_index(), 0,
                    panel_bottom - lower_left.height() + 3, 1, 1);
    right_edge.draw(&scroll_frame, right_edge.transparent_index(),
                    area_width - right_edge.width(), 0, 1, 1);

    ScreenBuffer.set_clip(&area);
    const int horizontal_frame_count = IsMultiplayerNet != 0 ? 4 : 8;
    PopupScrollHorizontalStep = IsMultiplayerNet != 0 ? 8 : 4;
    int frame_x = area.left + PopupScrollHorizontalStep - 0x20;
    if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SLIDING_WINDOWS) != 0
        && IsMultiplayerNet == 0) {
        WinFlipClipped = 1;
        for (int frame = horizontal_frame_count; frame > 0; --frame) {
            const DWORD started = timeGetTime();
            // Restore the saved screen image at its original screen position.
            screen_save.copy(&ScreenBuffer, 0, 0, area.left, area.top,
                             area_width, area_height);
            scroll_frame.draw(&ScreenBuffer, 9, frame_x, area.top, 1, 1);
            ScreenBuffer.set_clip(&vertical_strip);
            interface_background.draw(&ScreenBuffer, 9,
                MainInterfaceGlobal.outer_rect().left,
                MainInterfaceGlobal.outer_rect().top, 1, 1);
            Win::flip(&area);
            frame_x += PopupScrollHorizontalStep;
            ScreenBuffer.set_clip(&area);
            do_sound();
            while (static_cast<DWORD>(timeGetTime() - started)
                   < static_cast<DWORD>(WindowAnimationFrameDelay) / 2) {
                do_sound();
            }
        }
        ScreenBuffer.set_clip(&WinScreenClipRect);
        WinFlipClipped = 0;
    }

    const int destination_y = area.top - original_area.top;
    const int destination_x = window->Buffer::pixel_width()
        - scroll_frame.pixel_width();
    scroll_frame.draw(static_cast<Buffer *>(window), 9,
                      destination_x, destination_y, 1, 1);

    scroll->resize(0x18, panel_bottom - 0xA, 0);
    scroll->move(window->Buffer::pixel_width() - 0x2B, 0x15);
    scroll->show(0);
}

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

// ORIGINAL: 0x0045D970 ?slide_hide@@YAXPAUGraphicWin@@H@Z 0x0045D970-0x0045DEA0;0x00655777-0x006557A2
// RECOVERY_BINDING: v1 {"evidence": ["git:14a0bcaf:src/recovered/units/0045d970.cpp original hot/cold spans and retained seed findings.", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x45d970 full native register, rectangle mutation, receiver, call and lifetime audit.", "source:src/console.h virtual GraphicWin base; src/maininterface.h existing Sprite arrays and interface-up member; src/buffer.h source-receiver copy, full bounds and DIB geometry."], "reason": "Popup owns complete hide-animation behavior through genuine GraphicWin/Console Buffer bases, existing MainInterface Sprite members and automatic local Buffer lifetime; retain native mutable clipping, callback reloads and virtual Scroll hide."}
// symbol    ?slide_hide@@YAXPAVGraphicWin@@H@Z
void __cdecl slide_hide(GraphicWin *window, int mode) {
    Buffer window_frame;          // native F498, constructed first
    Buffer interface_background;  // native FA20, constructed second
    Buffer screen_restore;        // native EF10, constructed third

    if (window != 0 && window->Win::is_visible() != 0
        && (GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SLIDING_WINDOWS) != 0
        && IsMultiplayerNet == 0) {
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SOUND_EFFECTS) != 0)
            g_MAININTERFACE_WAVE.play();

        const int frame_edge = WinScreenWidth == 0x320 ? 0x5e : 0xee;
        const int frame_divisor = (IsMultiplayerNet != 0
                                   || MapMouseFeedbackBusy != 0
                                   || ConsoleControlTurnActive != 0) ? 3 : 5;
        RECT area = window->Buffer::clip_rect();
        window->client_to_screen(&area);

        interface_background.init(MainInterfaceGlobal.Buffer::pixel_width(),
                                  MainInterfaceGlobal.Buffer::pixel_height(), 0, 0);
        interface_background.fill(WinFillColour);
        MainInterfaceGlobal.update_window_to_buffer(&interface_background);

        // Native initializes the third local before initializing/filling the first.
        // The declarations above still preserve compiler cleanup order.
        screen_restore.init(window->Buffer::pixel_width(),
                            window->Buffer::pixel_height(), 0, 0);

        window_frame.init(window->Buffer::pixel_width(),
                          window->Buffer::pixel_height(), 0, 0);
        window_frame.fill(WinFillColour);

        // Native 0x45DB05/1B clears the two client-side bands before capturing
        // the target frame at 0x45DB3A.
        window->fill(0, 0, 0x17, window->Buffer::pixel_height(), 9);
        window->fill(window->Buffer::pixel_width() - 0x17, 0, 0x17,
                     window->Buffer::pixel_height(), 9);
        window->update_window_to_buffer(&window_frame);
        window->update_back_to_window(&screen_restore);

        RECT animation_strip = { area.left, 0, area.right, 0x39 };
        MainInterfaceGlobal.client_to_screen(&animation_strip);
        const int frame_count = (area.bottom - area.top) / frame_divisor;

        Sprite &left_outer = MainInterfaceGlobal.slide_frame_sprite(0);
        Sprite &left_inner = MainInterfaceGlobal.slide_frame_sprite(1);
        Sprite &right_inner = MainInterfaceGlobal.slide_frame_sprite(2);
        Sprite &right_outer = MainInterfaceGlobal.slide_frame_sprite(3);
        const int left_x = area.left;
        const int left_inner_x = left_x + left_outer.width();
        const int right_inner_x = area.left + window_frame.pixel_width()
            - right_outer.width()
            - right_inner.width() - 0x20;
        const int right_outer_x = area.left + window_frame.pixel_width()
            - right_outer.width() - 0x20;
        int frame_y = area.top + frame_count;

        ScreenBuffer.set_clip(&area);
        WinFlipClipped = 1;
        // Native stores frame_divisor into its reused [ebp+8] scratch slot at
        // 0x45DA52; incoming mode at [ebp+0xC] is only tested for bit 2.
        for (int frame = 0; frame < frame_divisor; ++frame) {
            const DWORD started = timeGetTime();
            screen_restore.copy(&ScreenBuffer, 0, 0, area.left, area.top,
                                window->Buffer::pixel_width(),
                                window->Buffer::pixel_height());
            window_frame.draw(&ScreenBuffer, 9, area.left, frame_y, 1, 1);

            int left_y = frame_y;
            if (left_y <= frame_edge) left_y = frame_edge;
            left_outer.draw(&ScreenBuffer, left_outer.transparent_index(),
                            left_x, left_y, 1, 1);
            left_inner.draw(&ScreenBuffer, left_inner.transparent_index(),
                            left_inner_x, frame_y, 1, 1);
            right_inner.draw(&ScreenBuffer, right_inner.transparent_index(),
                             right_inner_x, frame_y, 1, 1);
            int right_y = frame_y;
            if (right_y <= frame_edge) right_y = frame_edge;
            right_outer.draw(&ScreenBuffer, right_outer.transparent_index(),
                             right_outer_x, right_y, 1, 1);

            ScreenBuffer.set_clip(&animation_strip);
            interface_background.draw(&ScreenBuffer, 9,
                MainInterfaceGlobal.outer_rect().left,
                MainInterfaceGlobal.outer_rect().top, 1, 1);
            Win::flip(&area);
            frame_y += frame_count;
            ScreenBuffer.set_clip(&area);
            do {
                do_sound();
                int delay = WindowAnimationFrameDelay;
                if (MapMouseFeedbackBusy != 0 || ConsoleControlTurnActive != 0)
                    delay = delay / 2;
                if (static_cast<DWORD>(timeGetTime() - started)
                    >= static_cast<DWORD>(delay))
                    break;
            } while (true);
        }
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SOUND_EFFECTS) != 0)
            g_MAININTERFACE_WAVE.fade(0x32);
        ScreenBuffer.set_clip(&WinScreenClipRect);
    }

    if ((mode & 2) != 0) {
        MainInterfaceGlobal.interface_up_sprite().draw(
            static_cast<Buffer *>(&MainInterfaceGlobal),
            MainInterfaceGlobal.interface_up_sprite().transparent_index(),
            MainInterfaceGlobal.bottom_bounds().left,
            MainInterfaceGlobal.bottom_bounds().top, 1, 1);
        MainInterfaceTileFlag = 0;
        RECT interface_clip = MainInterfaceGlobal.info_frame_rect();
        interface_clip.bottom += 0x64;
        RECT original_interface_clip = MainInterfaceGlobal.Buffer::clip_rect();
        MainInterfaceGlobal.Buffer::set_clip(&interface_clip);
        MainInterfaceGlobal.restore_back(0);
        MainInterfaceGlobal.Buffer::set_clip(&original_interface_clip);
        MainInterfaceGlobal.soft_update();
    }
    WinFlipClipped = 0;
}

// ORIGINAL: 0x0045E540 ?scroll_hide@@YAXPAUGraphicWin@@PAUScroll@@@Z 0x0045E540-0x0045EB5A;0x006557CD-0x006557F8
// RECOVERY_BINDING: v1 {"evidence": ["git:14a0bcaf:src/unrecovered/0045e540.cpp original hot/cold spans and retained seed findings.", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x45e540 full native register, rectangle mutation, receiver, call and lifetime audit.", "source:src/console.h virtual GraphicWin base; src/maininterface.h existing Sprite arrays and interface-up member; src/buffer.h source-receiver copy, full bounds and DIB geometry."], "reason": "Popup owns complete hide-animation behavior through genuine GraphicWin/Console Buffer bases, existing MainInterface Sprite members and automatic local Buffer lifetime; retain native mutable clipping, callback reloads and virtual Scroll hide."}
// symbol    ?scroll_hide@@YAXPAVGraphicWin@@PAVScroll@@@Z
// TRIED: Ghidra's decompilation was unusable here (stack-frame analysis fell apart on the SEH-heavy huge frame); rebuilt from raw disassembly only. Three real `Buffer` locals (RAII) matching construction/destruction order, register-variable/goto transcription for the two scroll-animation throttle loops (timeGetTime via IAT-slot fn ptr + do_sound), the fixed global screen Buffer at 0x9B7490, and a pointer-to-member vtable-slot call (slot 2) on `a2` at the very end. Compiles and matches through instruction #35 (diverges #36, mov vs push) - the deepest match of this batch's giant functions. The two `0x9156b0`/`0x915af4`-relative receiver computations (`*(int*)(*(int*)g_009156b0+4) + <base>`) are transcribed literally; their semantic meaning (which sub-object they reach) was not resolved.
void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll) {
    // The seed's 0x9156B0/0x915AF4 vbtable arithmetic reaches Console's real
    // GraphicWin virtual base and its Buffer base. Buffer::copy names its
    // source receiver; the final hide is the inherited virtual operation.
    Buffer window_frame;          // native F490, constructed first
    Buffer interface_background;  // native EF08, constructed second
    Buffer screen_restore;        // native FA18, constructed third

    if (scroll->Win::is_visible() == 0)
        return;

    const int frame_edge = WinScreenWidth == 0x320 ? 0x5e : 0xee;
    RECT area = window->Buffer::clip_rect();
    window->client_to_screen(&area);
    const int screen_left = area.left;
    const int screen_top = area.top;
    const int area_height = area.bottom - area.top;

    interface_background.init(MainInterfaceGlobal.Buffer::pixel_width(),
                              MainInterfaceGlobal.Buffer::pixel_height(), 0, 0);
    interface_background.fill(WinFillColour);
    MainInterfaceGlobal.update_window_to_buffer(&interface_background);

    Sprite &right_edge = MainInterfaceGlobal.slide_frame_sprite(3);
    const int right_edge_x = area.right - right_edge.width() - 0x20;
    area.left = right_edge_x;
    const int area_width = area.right - area.left;

    RECT vertical_strip = { right_edge_x, 0, area.right, 0x39 };
    MainInterfaceGlobal.client_to_screen(&vertical_strip);

    screen_restore.init(area_width, area_height, 0, 0);
    RECT console_area = area;
    ConsoleGlobal.screen_to_client(&console_area);
    static_cast<Buffer *>(static_cast<GraphicWin *>(&ConsoleGlobal))->copy(
        &screen_restore, console_area.left, console_area.top, 0, 0,
        area_width, area_height);

    window_frame.init(area_width, area_height, 0, 0);
    static_cast<Buffer *>(window)->copy(
        &window_frame, area.left - screen_left, area.top - screen_top,
        0, 0, area_width, area_height);

    const int horizontal_step = IsMultiplayerNet != 0 ? 8 : 4;
    PopupScrollHorizontalStep = horizontal_step;
    int frame_x = area.left - horizontal_step;
    const int horizontal_frames = IsMultiplayerNet != 0 ? 4 : 8;
    ScreenBuffer.set_clip(&area);
    if ((GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AV_SLIDING_SCROLLBARS) != 0
        && IsMultiplayerNet == 0) {
        WinFlipClipped = 1;
        for (int frame = 0; frame < horizontal_frames; ++frame) {
            const DWORD started = timeGetTime();
            screen_restore.copy(&ScreenBuffer, 0, 0, area.left, screen_top,
                                area_width, area_height);
            window_frame.draw(&ScreenBuffer, 9, frame_x, screen_top, 1, 1);
            ScreenBuffer.set_clip(&vertical_strip);
            interface_background.draw(&ScreenBuffer, 9,
                MainInterfaceGlobal.outer_rect().left,
                MainInterfaceGlobal.outer_rect().top, 1, 1);
            Win::flip(&area);
            frame_x -= PopupScrollHorizontalStep;
            ScreenBuffer.set_clip(&area);
            do_sound();
            while (static_cast<DWORD>(timeGetTime() - started)
                   < static_cast<DWORD>(WindowAnimationFrameDelay) / 2)
                do_sound();
        }
        ScreenBuffer.set_clip(&WinScreenClipRect);
        WinFlipClipped = 0;
        frame_x += PopupScrollHorizontalStep;
    } else {
        frame_x += horizontal_step;
    }

    const int vertical_frame_step = IsMultiplayerNet != 0 ? 3 : 6;
    const int vertical_iterations = vertical_frame_step + 1;
    int closing_y = screen_top + PopupScrollVerticalFrameCount;
    const bool enter_vertical_close = PopupScrollVerticalFrameCount > 1;
    if (enter_vertical_close) {
        ScreenBuffer.set_clip(&area);
        int edge_y = frame_edge - 0xD;
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SLIDING_WINDOWS) != 0
            && IsMultiplayerNet == 0) {
            WinFlipClipped = 1;
            for (int frame = 0; frame < vertical_iterations; ++frame) {
                const DWORD started = timeGetTime();
                screen_restore.copy(&ScreenBuffer, 0, 0, area.left, screen_top,
                                    area_width, area_height);
                int draw_y = closing_y;
                if (draw_y >= edge_y) draw_y = edge_y;
                window_frame.draw(&ScreenBuffer, 9, frame_x, draw_y, 1, 1);
                ScreenBuffer.set_clip(&vertical_strip);
                interface_background.draw(&ScreenBuffer, 9,
                    MainInterfaceGlobal.outer_rect().left,
                    MainInterfaceGlobal.outer_rect().top, 1, 1);
                Win::flip(&area);
                closing_y += PopupScrollVerticalFrameCount;
                ScreenBuffer.set_clip(&area);
                do_sound();
                while (static_cast<DWORD>(timeGetTime() - started)
                       < static_cast<DWORD>(WindowAnimationFrameDelay) / 2)
                    do_sound();
            }
            ScreenBuffer.set_clip(&WinScreenClipRect);
            WinFlipClipped = 0;
        }
    }

    RECT original_buffer_clip = window->Buffer::clip_rect();
    if (enter_vertical_close)
        window->Buffer::set_clip(&original_buffer_clip);

    RECT local_area = area;
    local_area.left -= screen_left;
    local_area.right -= screen_left;
    local_area.top -= screen_top;
    local_area.bottom -= screen_top;
    window->Buffer::set_clip(&local_area);
    window->fill(9);
    int final_y_screen = closing_y;
    if (final_y_screen < local_area.top) final_y_screen = local_area.top;
    const int final_y = final_y_screen - screen_top;
    window_frame.draw(static_cast<Buffer *>(window), 9,
                      frame_x - screen_left, final_y, 1, 1);
    window->Buffer::set_clip(&original_buffer_clip);
    scroll->hide();
}

// ORIGINAL: 0x0051DCC0 sub_51dcc0 0x0051DCC0-0x0051DCF6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:short-network-popup-owner", "historical seed:src/recovered/0051dcc0.cpp", "pinned:592DE0 increment/send/decrement;51DCC0 two stack arguments and masked x86 SHL count with set/clear behavior", "source:existing NetBusyCount, NetDaemonNet.send_message and PopupWriteCheckState; no new storage or executable calls"], "reason": "Recover the original helper in its existing typed storage owner, preserving historical identity, complete span and byte obligation; descriptive source name is not original debug evidence."}
// symbol    ?popup_set_write_check_bit@@YAXHH@Z
void __cdecl popup_set_write_check_bit(int bit_index, int enabled) {
    if (enabled != 0) {
        const unsigned int mask = 1U << (static_cast<unsigned int>(bit_index) & 31U);
        PopupWriteCheckState |= static_cast<int>(mask);
    } else {
        int value = PopupWriteCheckState;
        const unsigned int clear_mask = 1U << (static_cast<unsigned int>(bit_index) & 31U);
        value &= ~static_cast<int>(clear_mask);
        PopupWriteCheckState = value;
    }
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
        field_322C_ = nullptr;
        if (mode == 6 || mode == 2) {
            field_21B0_ = 0x13;
            field_21B4_ = 0x15;
            field_21B8_ += 9;
        }
    } else {
        field_21B0_ = 0x21;
        field_21B4_ = 0x43;
        field_21BC_ = WinScreenWidth == 800 ? 0x1E : 0x32;
        field_322C_ = static_cast<Win *>(static_cast<GraphicWin *>(&ConsoleGlobal));
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

    if (static_cast<int>(field_21CC_) >= 0) {
      x = static_cast<int>(field_21CC_);
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
        strcat(StringTemp, StringTable.get(
            reinterpret_cast<int>(TimeControl[index].name)));
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
    static_cast<GraphicWin &>(dialogs()).on_scrolled(first, second);
}

// Superseded seed removal retains historical Git provenance below.
// ORIGINAL: 0x00406BA0 sub_406ba0 0x00406BA0-0x00406BCC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/00406ba0.cpp", "task:popup-linked-dialog-helper-owners", "Pinned Popup6695C8 slot49 forwards through actual Dialogs shared GraphicWin/Win virtual base. Other observed tables66A968/66CC20/66DE44 remain unresolved cross-owner occurrences, not new owner claims."], "reason": "Recover this complete operation in its genuine C++ popup/diplomacy owner using actual members, virtual dispatch and typed shared dependencies; retain original identity, spans, claims and seed discoveries."}
// symbol    ?on_scrolling@Popup@@UAEXHH@Z
void Popup::on_scrolling(int first, int second) {
    static_cast<GraphicWin &>(dialogs()).on_scrolling(first, second);
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
    return x_pop_caption_label(Players[faction_id].filename, "DATALINKS",
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

    if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) == 0) {
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
            bitmask_call(0x23, &facility_offset, &facility_mask);
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


// Original627110, alongside the existing saved-location objects in popup.cpp.
// ORIGINAL: 0x00627110 sub_627110 0x00627110-0x00627124 BYTE_EXACT
// symbol    ?set_popup_saved_location@@YAHHH@Z
// kind      game
int __cdecl set_popup_saved_location(int x, int y) {
    PopupSavedLocX = x;
    PopupSavedLocY = y;
    return x;
}

/*
Purpose: Draw one dialog item's background for the popup: restore the
         entered window's saved clip, centre the item's close-X sprite in
         the frame when the dialog is in the icon mode, draw the panel box
         from the interface style table, then pick the item text colours
         from the pair of tables the title-bar list selects against.

// HISTORICAL SCAFFOLD ROLE: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x00405B20 ?on_dialog_item_back_draw@Popup@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x00405B20-0x00405C72 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/popup.h Popup : BasePop and existing field/dialog callback API", "source:src/basepop.h and src/dialogs.h establish the protected dialog kind member", "source:src/palette.cpp textcolor_init populates source-owned TextColourTable", "source:src/diplopop.cpp source-owned DiplomacyPopup object"], "reason": "Own the dialog-item background callback as a nonvirtual Popup member: it reads Popup/BasePop state and uses the already-owned palette table, style tables and diplomacy singleton."}
// symbol    ?on_dialog_item_back_draw@Popup@@QAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
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
