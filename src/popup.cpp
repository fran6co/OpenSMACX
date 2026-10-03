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
#include "stdafx.h"
#include <mmsystem.h>
#include <cstring>
#include "original_seam.h"
#include "popup.h"
#include "wave.h"
#include "fx.h"
#include "ambience.h"
#include "win.h"
#include "spritebox.h"
#include "net_class.h"
#include "temp.h"
#include "strings.h"    // StringTemp - the shared 0x1032 scratch buffer at 0x009B86A0
#include "mapwin.h"     // MainInterfaceGlobal - the iface window at 0x007AE820
#include "messagewin.h" // InterfaceMessageWin - the desktop window at 0x007F67F8
#include "base.h"       // BaseIDCurrentSelected - the base the exec result goes to
#include "lock.h"
#include "map.h"
#include "veh.h"
#include "alpha.h"
#include "xpops.h"

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

// Two of the popup's hide-path helpers are still original bodies (0x0045D970 /
// 0x0045E540, archive copies the build does not compile). They are __cdecl
// free functions the image reaches with a plain `call rel32`, so they are
// declared here; slide_hide is defined as a forwarder at the foot of
// councwin.cpp and scroll_hide has no home yet.
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

// The list-geometry metrics (popup.h has the role note).
int PopupScrollAreaTop;    // 0x007CFF88
int PopupScrollAreaGap;    // 0x007CFF30
int PopupScrollRowHeight;  // 0x007CFF5C

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
char PopupPendingLink[256];    // 0x007591C8
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
    if (!(GamePreferences & 0x400)) {
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
    if (GamePreferences & 0x400) {
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

         Popup models fields only as far as its Scroll member, so both offsets
         are reached through documented raw offsets, as BasePop::UNK3 does.
// ORIGINAL: 0x00405020 ?on_adjust_button_width@Popup@@QAEXXZ 0x00405020-0x0040504A BYTE_EXACT
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
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    const int32_t scaled =
        *reinterpret_cast<const int32_t *>(self + 0x30AC) / 10000;
    if (scaled != 1000) {
        *reinterpret_cast<int32_t *>(self + 0x30D0) = 20;
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
    return reinterpret_cast<Win *>(this)->Win::on_nc_hittest(x, y);
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
static int PopupSavedLocX = 0x2000;  // 0x006973DC
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

// The scroll hide helper is still an original body (0x0045E540, an archive
// copy the build does not compile). Same pattern as councwin.cpp's
// slide_show/slide_hide forwarders: declared above, defined here as the
// __cdecl call the image's own `call rel32` reaches. Retire this definition
// the moment 0045e540.cpp is homed - a forwarder left standing over a real
// body is exactly the landmine address_index.py gates on.
void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll) {
    typedef void(__cdecl *pending)(GraphicWin *, Scroll *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0045E540))(window,
                                                                      scroll);
}

void __cdecl scroll_show(GraphicWin *window, Scroll *scroll) {
    typedef void(__cdecl *pending)(GraphicWin *, Scroll *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0045DEA0))(window,
                                                                      scroll);
}

// ORIGINAL: 0x00404B00 ?on_redraw@Popup@@QAEXXZ 0x00404B00-0x00404B05 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:243a2cfd:src/unrecovered/00404b00.cpp;src/basepop.cpp:BasePop::on_redraw"], "reason": "Popup draw callback forwards to its real BasePop owner; image and historical seed establish the five-byte tail-call identity"}
// LEVER: The retired seed matched through a synthetic reinterpret-cast shim with masked relocation targets. This real inherited call preserves its measured body without retaining that false class shape.
// symbol    ?on_redraw@Popup@@UAEXXZ
void Popup::on_redraw() {
    BasePop::on_redraw();
}

// ORIGINAL: 0x00405050 ?on_adjust_button_placement@Popup@@QAEXPAH00@Z 0x00405050-0x00405093 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:243a2cfd:src/recovered/units/00405050.cpp;src/basepop.h:field_30AC_"], "reason": "Popup button-placement adjustment reads its inherited auto-hide mode and external window pointer, matching the original owner fields"}
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
    if (GamePreferences & 0x400) {
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
        PopupShellExecuteSlot()(0, PopupLinkOpenVerb, PopupLinkTarget, 0, 0,
                               1);
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
// - reached as qualified base members, no raw offsets. The three 0x7CFF globals are screen
// metrics no dump in this tree names; popup.cpp names them by their role in
// this arithmetic.
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
    const int inset_top = PopupScrollAreaTop;
    const int row_height = PopupScrollRowHeight;
    const int rows = (area.bottom - inset_top - PopupScrollAreaGap
                      - (area.top + 0xd) - 0x2c) / row_height;
    const int top = rows * row_height + inset_top + PopupScrollAreaGap - 0xa;
    const int left = this->Buffer::dib_.bmiHeader.biWidth - 0x2b;

    scroll_.init(left, 0x15, 0x18, top, this, -1, 1);
    scroll_.hide();

    if (is_visible()) {
        scroll_show(this, &scroll_);
    }
    return reinterpret_cast<int>(&scroll_);
}

// ORIGINAL: 0x00405910 ?on_exec@Popup@@QAEHXZ 0x00405910-0x00405B01 FILE
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
                                      unit_ids_.current_->payload)) + 4);
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
            const int alive = (LockEnableMask & (faction_mask << 8)) != 0;
            const int human = (LockEnableMask & faction_mask) != 0;
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
