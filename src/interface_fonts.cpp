// ORIGINAL: 0x005882A0 _$E19 0x005882A0-0x005882C4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x00682648 selects initializer0x005882A0-0x005882C4 and its registration names fini0x005882D0-0x005882E4; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FC58; 48 Font objects, stride0x28, constructor/destructor618EA0/618EE0 through vector helpers6457C2/6456E4. Source object binding names ?FontTable@@3PAVFont@@A.", "Fresh owner COFF selects _$E19 via .CRT$XCU and registers _$E17, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original startup function to the genuine compiler-generated lifetime of source-owned FontTable; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E16
// ORIGINAL: 0x005882D0 _$E17 0x005882D0-0x005882E4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x00682648 selects initializer0x005882A0-0x005882C4 and its registration names fini0x005882D0-0x005882E4; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FC58; 48 Font objects, stride0x28, constructor/destructor618EA0/618EE0 through vector helpers6457C2/6456E4. Source object binding names ?FontTable@@3PAVFont@@A.", "Fresh owner COFF selects _$E19 via .CRT$XCU and registers _$E17, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original exit function to the genuine compiler-generated lifetime of source-owned FontTable; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E14
#include "stdafx.h"
#include "interface_fonts.h"
#include "popup.h"
#include "popup_fonts.h"
#include "menu.h"
#include "pulldown.h"
#include "game.h"
#include "veh.h"
#include "strings.h"
#include "alpha.h"
#include "basepop_defaults.h"
#include "messagewin.h"
#include "diplowin.h"   // DiploKnownTechColor
#include "scroll.h"
#include "sprite.h"
#include "listbox.h"
#include "font.h"
#include "faction.h"
#include "text.h"
#include "xpops.h"
#include "base.h"
#include "factionprofilewin.h"
#include "rectangle_operations.h"

extern int TextColourTable[195];
extern uint32_t EditBoxDefaultB20;
extern uint32_t EditBoxDefaultB24;
extern uint32_t EditBoxDefaultB28;
extern uint32_t EditBoxDefaultB2C;


// Application fonts, selected and initialized by config_popups and WinMain.
// FontTable stays with find_font; the entrypoint's singleton is in main.cpp.
// Original CRT order is singleton, FontQueue VAL2/VAL1, then FontTable.

// Face pointer read only by Credits::exec (0x00428AE4); its literal sits
// before "Arial" in this unit's data.
// RECOVERY_OBJECT: v1 0x00691B28 ?CreditsFontFace@@3PBDB
LPCSTR CreditsFontFace = "Alpha Centauri";  // 0x00691B28 -> 0x00691BC4
// RECOVERY_OBJECT: v2 0x00691B2C 0x00691B30 ?DefaultFontFace@@3PBDB 0x00408B8D
LPCSTR DefaultFontFace = "Arial";  // 0x00691B2C -> 0x00691BD4
// RECOVERY_OBJECT: v1 0x00691B30 ?DefaultLargeFontFace@@3PBDB
LPCSTR DefaultLargeFontFace = "Arial Narrow"; // 0x00691B30 -> 0x00691BDC

/*
Purpose: Select the Font whose point size is closest to the requested one,
         then index that size's row by style.
// ORIGINAL: 0x005882F0 ?find_font@@YAHHH@Z 0x005882F0-0x0058833B BYTE_EXACT
// symbol    ?find_font@@YAPAVFont@@HH@Z
// size      75 bytes
// prototype 
// callers   31   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
Font *__cdecl find_font(int size, int style) {
    int best_index = 0;
    // The seed doubles as a threshold: a requested size further than 9999 from
    // every candidate matches nothing and falls back to the first entry.
    int best_delta = 9999;
    for (size_t index = 0; index < FontSizeTableCount; ++index) {
        const int candidate = FontSizeTable[index];
        // Absolute difference, branching on sign exactly as the legacy body.
        int delta = candidate - size;
        if (delta < 0) {
            delta = size - candidate;
        }
        if (delta < best_delta) {
            best_delta = delta;
            best_index = static_cast<int>(index);
        }
    }
    // Each size occupies four consecutive style slots in the table.
    return FontTable + (style + best_index * 4);
}

// The original setters populate the same twelve sprite slots in four
// triplets. These are source-owned sprite pointers, not addresses in the old
// executable. Index 14 is an untouched gap; index 16 is another process
// default that neither setter changes.
static __forceinline void set_scroll_icon_defaults(Sprite *arrows,
                                                    Sprite *filler,
                                                    int thickness) {
    ScrollDefaultSpriteUp[0] = &arrows[0];
    ScrollDefaultSpriteUp[1] = &arrows[1];
    ScrollDefaultSpriteUp[2] = &arrows[2];
    ScrollDefaultSpriteDown[0] = &arrows[3];
    ScrollDefaultSpriteDown[1] = &arrows[4];
    ScrollDefaultSpriteDown[2] = &arrows[5];
    ScrollDefaultSpriteLeft[0] = &arrows[6];
    ScrollDefaultSpriteLeft[1] = &arrows[7];
    ScrollDefaultSpriteLeft[2] = &arrows[8];
    ScrollDefaultSpriteRight[0] = &arrows[9];
    ScrollDefaultSpriteRight[1] = &arrows[10];
    ScrollDefaultSpriteRight[2] = &arrows[11];
    ScrollDefaultThumb = filler;
    ScrollDefaultThickness = thickness;
}

// ORIGINAL: 0x00588340 ?scroll_normal@@YAXXZ 0x00588340-0x005883CD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00588340.cpp","source:src/sprite.cpp","source:src/scroll.cpp:ScrollCloseDynamicDefaults"],"reason":"Populate the source-owned Scroll defaults from the normal-sized sprite objects and restore the normal thickness; retain the exact historical seed as a restoration obligation."}
// LEVER: plain sequential stores of the scaffold's own fixed-address globals (`*g_dst = (int)g_src;`) - no address arithmetic here, so the `static T *const` spelling is exactly right, no `extern` rebind needed.
void __cdecl scroll_normal() {
    set_scroll_icon_defaults(g_SCROLL_BAR_ARROW_ICON_SPRITES,
                             &g_SCROLL_BAR_FILLER_ICON_SPRITES, 20);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005883D0 ?scroll_small@@YAXXZ 0x005883D0-0x0058845D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["source:src/unrecovered/005883d0.cpp","source:src/sprite.cpp","source:src/scroll.cpp:ScrollCloseDynamicDefaults"],"reason":"Populate the same source-owned Scroll defaults from the small sprite objects and restore the small thickness; no fixed executable sprite addresses remain in this body."}
void __cdecl scroll_small() {
    set_scroll_icon_defaults(g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES,
                             &g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE, 12);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00588460 ?popups_normal@@YAXXZ 0x00588460-0x005887F2 BYTE_EXACT
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
    int bevel_upper = PopupThemeBevelUpper;
    int bevel_lower = PopupThemeBevelLower;
    BasePopDefaultField31B0[1] = PopupThemeSurfaceColor;
    MenuDefaultSelectionColor = PopupThemeSurfaceColor;
    MenuDefaultSelectionTextColor = MessageWinTextColor;
    MenuDefaultColor3 = DiploKnownTechColor;
    WinStaticDefaults[6] = InterfaceBottomBorderThickness;
    WinStaticDefaults[1] = InterfaceCaptionHeight;
    BasePopDefaultField3104[2] = 0;
    BasePopButtonDefaults.bevel_upper = bevel_upper;
    BasePopButtonDefaults.bevel_lower = bevel_lower;
    BasePopDefaultField31B0[0] = 0;
    MenuDefaultColor2 = MenuDefaultColor1 = 0;
    WinStaticDefaults[5] = InterfaceBorderThickness;
    DialogDefaults[3] = 2;
    DialogDefaults[1] = bevel_upper;
    DialogDefaults[2] = bevel_lower;
    BaseButton::set_def_text_color(0, -1, 1, 1);
    DialogDefaults[0] = PopupThemeSurfaceColor;
    BaseButtonA94Default = 0;
    BaseButton::set_def_font(0, 0, 0);
    BaseButtonA98Default = PopupThemeButtonColor;
    PullDownDefaultTextColor = TextColourTable[38];
    PullDownDefaultBackground = TextColourTable[40];
    PullDownDefaultDisabledTextShadow = TextColourTable[42];
    PullDownDefaultTextShadow = TextColourTable[44];
    PullDownDefaultSelectedTextColor = TextColourTable[46];
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
    ScrollDefaultSpriteUp[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[0];
    ScrollDefaultSpriteUp[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[1];
    ScrollDefaultSpriteUp[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[2];
    ScrollDefaultSpriteDown[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[3];
    ScrollDefaultSpriteDown[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[4];
    ScrollDefaultSpriteDown[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[5];
    ScrollDefaultSpriteLeft[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[6];
    ScrollDefaultSpriteLeft[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[7];
    ScrollDefaultSpriteLeft[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[8];
    ScrollDefaultSpriteRight[0] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[9];
    ScrollDefaultSpriteRight[1] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[10];
    ScrollDefaultSpriteRight[2] = &g_SCROLL_BAR_ARROW_ICON_SPRITES[11];
    ScrollDefaultThumb = &g_SCROLL_BAR_FILLER_ICON_SPRITES;
    ScrollDefaultThickness = 20;
    SpriteBoxDefaultField8 = PopupThemeWindowColor;
    EditBoxDefaultB28 = PopupThemeWindowColor;
    EditBoxDefaultB2C = PopupThemeTextShadow;
}

// ORIGINAL: 0x00588800 sub_588800 0x00588800-0x005888DE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00588800.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
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
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/005888e0.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
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


// ORIGINAL: 0x00588AA0 ?config_popups@@YAXXZ 0x00588AA0-0x00588F8E
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00588aa0.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// symbol    ?config_popups@@YAXXZ
// Current reconstructed ownership keeps this ordinary body beside find_font.
// Shared visibility is a compiler-context hypothesis, not historical TU proof.
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
    if (WinScreenWidth == 800) {
        PopupMenuFont = find_font(12, 0);
        PopupMenuItalicFont = find_font(12, 2);
        PopupMenuBoldFont = find_font(12, 1);
    } else {
        PopupMenuFont = find_font(16, 0);
        PopupMenuItalicFont = find_font(16, 2);
        PopupMenuBoldFont = find_font(16, 1);
    }
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

// ORIGINAL: 0x00588F90 ?alien_start@@YAXXZ 0x00588F90-0x0058917C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:6e78872480ef8b4a486516e322f23e9f8652665d:src/recovered/units/00588f90.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; preserve original extents and historical seed lessons. Ordinary owned literals have pinned content provenance, not exact original data-allocation claims."], "reason": "Recover first-turn alien faction suppression through real Player filename/rules, lock masks, descending Base/Veh lifetimes and faction records."}
void __cdecl alien_start() {
    static const char alien_filename_a[] = "FUNGBOY";
    static const char alien_filename_b[] = "FUNGBOY";

    if (GameDataStateStorage.fields_.turn_current_num_ != 0 || ExpansionEnabled == 0) {
        return;
    }

    const int local_faction = ConsoleGlobal.faction_id_;
    if ((Players[local_faction].rule_flags & RFLAG_ALIEN) != 0) {
        return;
    }

    StringTemp[0] = 0;
    strcat(StringTemp, Players[local_faction].filename);
    CharUpperA(StringTemp);
    if (strcmp(StringTemp, alien_filename_a) == 0) {
        return;
    }

    for (int faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
        if (is_human(faction_id) || IsMultiplayerPBEM != 0 ||
            IsMultiplayerNet != 0) {
            continue;
        }

        StringTemp[0] = 0;
        strcat(StringTemp, Players[faction_id].filename);
        CharUpperA(StringTemp);
        if ((Players[faction_id].rule_flags & RFLAG_ALIEN) == 0 &&
            strcmp(StringTemp, alien_filename_b) != 0) {
            continue;
        }

        GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ &= ~(1 << faction_id);
        GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &= ~(1 << faction_id);

        for (int base_id = GameDataStateStorage.fields_.base_current_count_ - 1; base_id >= 0; --base_id) {
            if (Bases[base_id].faction_id_current == faction_id) {
                base_kill(base_id);
            }
        }

        // veh_kill compacts the vehicle table; the original checks the live
        // count before dereferencing each descending id.
        for (int veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; veh_id >= 0; --veh_id) {
            if (veh_id < GameDataStateStorage.fields_.vehicle_current_count_ &&
                Vehs[veh_id].faction_id == faction_id) {
                veh_kill(veh_id);
            }
        }

        PlayersData[faction_id].current_num_bases = 0;
        PlayersData[faction_id].flags = 0xFFFFFFFFU;
        PlayersData[faction_id].diff_level = -1;
    }
}

/*
Purpose: Handle creation of pop-up message on Planetfall.
// ORIGINAL: 0x00589180 ?planetfall@@YAXH@Z 0x00589180-0x00589329 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x00589180-0x00589329 calls parse_says with slots 2, 3, 0, 1 in that order; the four text pointers are Player.title_leader (+0x4C), name_leader (+0x34), adj_name_faction (+0x304), and noun_faction (+0x2E4).", "Pinned default writes are ordered: PluralityDefault=0 before slot 2; GenderDefault=is_leader_female before slot 2; then GenderDefault and PluralityDefault=0 before slot 3; slot 0 inherits those defaults; finally noun_gender then is_noun_plural before slot 1. general.h binds those defaults to 0x009BBFEC/0x009BBFF0.", "Pinned Player records begin at 0x00946A50 with 0x59C stride; rule_flags is +0x508 and alien bit 0x80 is tested directly. The image uppercases the copied filename via CharUpperA then calls the pinned _strcmp entry at 0x00645660 for exact USURPER/FUNGBOY comparisons; this preserves the raw flag test and exact string behavior, not is_alien_faction's expansion gate or case-insensitive comparison.", "Pinned UI key construction clears and appends PLANETFALLX for an alien USURPER, PLANETFALLF for exact FUNGBOY, otherwise PLANETFALL. It uses shared StringTemp at 0x009B86A0. Nonzero TurnCurrentNum at 0x009A64D4 gates parse_num(0, turn), the 64-slot project scan, and a final appended '2' suffix before X_pop(StringTemp, 0); zero turn skips all three.", "The project scan reads the 64 dword SecretProject array at 0x009A6514, accepts nonnegative base owners matching faction_id, and calls the int-input parse_say(4, Facility[70+project].name, -1, -1) for each match. The typed sources are GameDataStateWindowFields::secret_project_, Base.faction_id_current, and RulesFacility.name; the generated body uses has_project without adding bounds or error-path changes.", "The previous TRIED finding remains verbatim as history; this rationale records the pinned call/global/UI mapping only. Existing ORIGINAL identity and complete 425-byte span remain; no exactness/semantic claim is granted."], "reason": "Recover Planetfall's ordered parser defaults, shared scratch-buffer popup key, current-turn project context and source-owned faction/project state from the pinned body."}
// size      425 bytes
// prototype void (__cdecl ?planetfall@@YAXH@Z)(int factionID)
// callers   2   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF310 0x00625E30 0x00625E50 0x00625EC0 0x00645470 0x00645660
// indirect  0x0058923D
// TRIED: not resolved this pass. The image's FOUR `parse_says`
//        (0x00625EC0) call sites write the gender/plurality arguments to
//        FIXED GLOBALS (0x9bbfec/0x9bbff0) individually before each call
//        rather than through `parse_set`, in an ORDER that does not match
//        this body's own call order (id 2, then 3, then 0, then 1 - not
//        0, 2, 3, 1), and the calls list also names 0x00625E30 (parse_num)
//        AND 0x00625E50 (parse_say, the int-input overload) - this body
//        calls parse_num once and never parse_say, so one of its calls is
//        the wrong callee. Left at 11/125 for a pass that can trace which
//        source line the int-input `parse_say` belongs to.
Return Value: n/a
Status: Complete
*/
void __cdecl planetfall(int faction_id) {
    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(2, Players[faction_id].title_leader, -1, -1);

    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(3, Players[faction_id].name_leader, -1, -1);
    parse_says(0, Players[faction_id].adj_name_faction, -1, -1);

    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(1, Players[faction_id].noun_faction, -1, -1);

    StringTemp[0] = 0;
    strcat(StringTemp, Players[faction_id].filename);
    CharUpperA(StringTemp);
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0
        && strcmp(StringTemp, "USURPER") == 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, "PLANETFALLX");
    } else if (strcmp(StringTemp, "FUNGBOY") == 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, "PLANETFALLF");
    } else {
        StringTemp[0] = 0;
        strcat(StringTemp, "PLANETFALL");
    }

    const int turn = GameDataStateStorage.fields_.turn_current_num_;
    if (turn != 0) {
        parse_num(0, turn);
        for (int project = 0; project < MaxSecretProjectNum; ++project) {
            // Open-coded has_project: the image materializes the owner test.
            int base_id = GameDataStateStorage.fields_.secret_project_.project_base[project];
            BOOL owned;
            if (base_id < 0) {
                owned = false;
            } else {
                owned = (Bases[base_id].faction_id_current == faction_id);
            }
            if (owned) {
                parse_say(4, Facility[FacilitySPStart + project].name,
                          -1, -1);
            }
        }
        strcat(StringTemp, "2");
    }

    X_pop(StringTemp, 0);
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

// ORIGINAL: 0x00589550 j_GraphicWin::close 0x00589550-0x00589555 FILE BYTE_EXACT
// symbol    ?j_close@GraphicWin@@QAEXXZ
// LEVER: implicit-thiscall member forwarding; explicit __thiscall on a free fn is C4234
// working copy - scaffold materialised by --work
// size      5 bytes
// prototype 
// callers   0   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)


// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: j_GraphicWin::close  at 0x00589550  (5 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;
// GraphicWin is graphicwin.h's real class, which general.cpp already includes -
// the thunk is a member of it, not of a local shim.
void GraphicWin::j_close() {
    close();
}

// ORIGINAL: 0x00589560 sub_589560 0x00589560-0x00589748 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00589560-0x00589748 reads receiver fields and Popup at +0xA3C, builds the faction profile content and finishes through the update slot", "source:src/faction.h Player formal_name_faction is +0x384; source:src/game.cpp owns InterfaceBoxStyles[51], populated by load_box_sprites; 0x78D690 is entry 9 at 0x28 stride"], "reason": "Carry the complete seed identity and body span into the typed show_profile member, preserving fresh owner-field reads across callback-capable Popup and SpriteBox operations. The class role is provisional and does not claim original identity or allocation registration."}
// symbol    ?show_profile@FactionProfileWin@@QAEXXZ
#pragma optimize("y", off)
void FactionProfileWin::show_profile() {
  RECT local_bounds = clip_rect();
  box_sprite(&local_bounds, &InterfaceBoxStyles[9]);

  set_caption(Players[faction_id_].formal_name_faction);
  StringTemp[0] = 0;
  if (profile_mode_ != 0) {
    strcat(StringTemp, "DATALINKS1");
  } else {
    strcat(StringTemp, "BLURB");
  }
  FactionDiplomacyPortraits[faction_id_]->draw(
      static_cast<Buffer *>(this),
      FactionDiplomacyPortraits[faction_id_]->transparent_index(), 0, 2);

  const int popup_width = width_ - 0xA9;
  popup_.start(Players[faction_id_].filename, StringTemp, -1, 0, 0x400,
               static_cast<GraphicWin *>(this));
  if (profile_mode_ != 0) {
    StringTemp[0] = 0;
    strcat(StringTemp, "DATALINKS2");
    popup_.start(Players[faction_id_].filename, StringTemp, -1, 0, 0x80,
                 static_cast<GraphicWin *>(this));
  }

  Popup *const popup = &popup_;
  Sprite *const popup_portrait = FactionDiplomacyPortraits[faction_id_];
  if (!popup->picture_enabled()) {
    popup->set_caller_picture(popup_portrait);
  } else {
    popup->set_caller_picture(0);
  }
  popup->dialogs().sprite(popup_portrait, 0, 0);
  set_caption(Players[faction_id_].formal_name_faction);
  popup_.set_string_font(PopupMenuFont, PopupMenuBoldFont,
                         PopupMenuItalicFont, 0);
  popup_.set_loc(0xA2, 2);
  popup_.set_width(popup_width);
  popup_.exec(0, 0);

  on_update();
}
#pragma optimize("", on)

// ORIGINAL: 0x00589750 sub_589750 0x00589750-0x00589760 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x00589750-0x00589760 stores its argument at receiver+0xA34", "source:src/factionprofilewin.h RECOVERY_MEMBER faction_id_ at +0xA34, anchored by 0x00589760", "pinned/current owner:0x00589760 initialize writes faction_id_ and profile_mode_ at +0xA34/+0xA38"], "reason": "Carry the one-word receiver store into the existing FactionProfileWin member that owns the anchored faction_id_ field; class role and method spelling remain provisional."}
// symbol    ?set_faction_id@FactionProfileWin@@QAEXH@Z
// kind      game
void FactionProfileWin::set_faction_id(int faction_id) {
  faction_id_ = faction_id;
}

// ORIGINAL: 0x00589760 sub_589760 0x00589760-0x0058986A;0x00660240-0x00660255 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00589760-0x0058986A stores the arguments at receiver+0xA34/+0xA38, frame fields and RECT, then calls GraphicWin initialization/fill and the slot-1 show dispatch", "image:0x00660240-0x00660255 is the local Buffer unwind funclet and remains the stack lifetime in the source body"], "reason": "Carry the complete seed identity and both image spans into the typed initialize member. FactionProfileWin remains a provisional role name; original class identity and allocation/caller registration are unproved."}
// symbol    ?initialize@FactionProfileWin@@QAEXHH@Z
#pragma optimize("y", off)
void FactionProfileWin::initialize(int faction_id, int profile_mode) {
  Buffer local_buffer;

  faction_id_ = faction_id;
  profile_mode_ = profile_mode;
  field_A14_ = 5;
  field_A18_ = 0x2D;
  width_ = 0x226;
  height_ = 0x12C;
  if (IfaceScreenWidth >= 1000) {
    width_ = 0x2C0;
    height_ = 0x180;
  }

  set_rectangle_dimensions(&bounds_, 5, 0x2D, width_, height_);
  init(&bounds_, const_cast<char *>("..."), 0x1400000, OpeningWindow, 0,
       0);
  fill(9);
  show(0);
}
#pragma optimize("", on)


// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (dispatcher at 0x00644EEB, table at 0x00682000..0x006826CC).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
// Was `Font g_FONTS[48]` beside the `Font *const FontTable = (Font
// *)0x0093FC58` binding - the array is the binding's storage, so the object
// now carries the public name and find_font's `FontTable + N` indexes it
// directly. See interface_fonts.h.
// Original startup 0x005882A0 and teardown 0x005882D0 both pass this
// storage, count 48 and stride 0x28 to the Font vector lifetime helpers.
// That constructed array run establishes [0x0093FC58, 0x009403D8),
// independently of the next global or this reconstruction's sizeof.
// RECOVERY_OBJECT: v2 0x0093FC58 0x009403D8 ?FontTable@@3PAVFont@@A 0x005882A0
Font FontTable[48];  // 0x0093FC58, 0x28 stride

// 0x0068F220, twelve point sizes find_font matches against. The values are
// the image's own .data dwords (image_data.py, 0x28 0x24 0x20 0x18 0x16 0x14
// 0x12 0x10 0x0e 0x0c 0x0a 0x08), largest first.
// RECOVERY_OBJECT: v1 0x0068F220 ?FontSizeTable@@3QBHB
const int FontSizeTable[FontSizeTableCount] = {
    0x28, 0x24, 0x20, 0x18, 0x16, 0x14, 0x12, 0x10, 0x0e, 0x0c, 0x0a, 0x08};

// Original 0x0068F250..0x0068F260: config_popups indexes all four styles.
// RECOVERY_OBJECT: v1 0x0068F250 ?FontStyleTable@@3QBHB
const int FontStyleTable[4] = {0, 1, 2, 4};
