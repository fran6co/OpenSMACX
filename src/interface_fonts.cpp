// ORIGINAL: 0x005882A0 _$E19 0x005882A0-0x005882C4 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x00682648 selects initializer0x005882A0-0x005882C4 and its registration names fini0x005882D0-0x005882E4; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FC58; 48 Font objects, stride0x28, constructor/destructor618EA0/618EE0 through vector helpers6457C2/6456E4. Source object binding names ?FontTable@@3PAVFont@@A.", "Fresh owner COFF selects _$E19 via .CRT$XCU and registers _$E17, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original startup function to the genuine compiler-generated lifetime of source-owned FontTable; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E12
// ORIGINAL: 0x005882D0 _$E17 0x005882D0-0x005882E4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x00682648 selects initializer0x005882A0-0x005882C4 and its registration names fini0x005882D0-0x005882E4; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FC58; 48 Font objects, stride0x28, constructor/destructor618EA0/618EE0 through vector helpers6457C2/6456E4. Source object binding names ?FontTable@@3PAVFont@@A.", "Fresh owner COFF selects _$E19 via .CRT$XCU and registers _$E17, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original exit function to the genuine compiler-generated lifetime of source-owned FontTable; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E10
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


// Application fonts, selected and initialized by config_popups and WinMain.
// FontTable stays with find_font; the entrypoint's singleton is in main.cpp.
// Original CRT order is singleton, FontQueue VAL2/VAL1, then FontTable.

// RECOVERY_OBJECT: v1 0x00691B2C ?DefaultFontFace@@3PBDB
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
