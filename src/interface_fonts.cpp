#include "stdafx.h"
#include "interface_fonts.h"

// Application fonts, selected and initialized by config_popups and WinMain.
// Keep FontTable before g_JACKAL_FONT in this TU: their original source-owned
// construction and reverse teardown ordering is not delegated to Jackal.

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

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
// Was `Font g_FONTS[48]` beside the `Font *const FontTable = (Font
// *)0x0093FC58` binding - the array is the binding's storage, so the object
// now carries the public name and find_font's `FontTable + N` indexes it
// directly. See interface_fonts.h.
Font FontTable[48];  // 0x0093FC58, 0x28 stride

// 0x0068F220, twelve point sizes find_font matches against. The values are
// the image's own .data dwords (image_data.py, 0x28 0x24 0x20 0x18 0x16 0x14
// 0x12 0x10 0x0e 0x0c 0x0a 0x08), largest first.
const int FontSizeTable[FontSizeTableCount] = {
    0x28, 0x24, 0x20, 0x18, 0x16, 0x14, 0x12, 0x10, 0x0e, 0x0c, 0x0a, 0x08};

// Original 0x0068F250..0x0068F260: config_popups indexes all four styles.
// RECOVERY_OBJECT: v1 0x0068F250 ?FontStyleTable@@3QBHB
const int FontStyleTable[4] = {0, 1, 2, 4};

// 0x007D3948 in the image - the single game-wide font, initialised by
// WinMain and closed at exit.
Font g_JACKAL_FONT;
