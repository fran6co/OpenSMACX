#pragma once
#include "font.h"

#if defined(_M_IX86) || defined(__i386__)
// Twelve candidate point sizes and the Font table they index - REAL OBJECTS,
// both defined in interface_fonts.cpp. They used to be fixed-address bindings
// (`int *const FontSizeTable = (int *)0x0068F220` and
// `Font *const FontTable = (Font *)0x0093FC58`): the table at 0x68F220 is
// file-backed .data the image carries real values in, and 0x93FC58 is
// zero-fill, so the object forms lose nothing - and the bindings named
// terranx.exe storage, unmapped in a standalone build. `FontSizeTable[index]`
// and `FontTable + N` read identically off the arrays: the loop-indexed loads
// fold to the same `[disp32 + index*4]` form, the displacement relocated.
static const size_t FontSizeTableCount = 12;
extern const int FontSizeTable[FontSizeTableCount];  // 0x0068F220
extern const int FontStyleTable[4];                  // 0x0068F250
extern Font FontTable[48];                           // 0x0093FC58, 0x28 stride

Font *__cdecl find_font(int size, int style);
#endif

/*
 * The face name every `Font::init` call site passes when it does not name one
 * of its own. WinMain passes it as the FACE while naming "arialn.ttf" as the
 * FILE, which is why the four-argument `init` overload exists.
 *
 * ITS VALUE IS RECOVERED, NOT POINTED AT. The variable is at 0x00691B2C and
 * the image STORES its initial value there - `.data` is stored in the file up
 * to 0x006A8000 and this is below that line - so the four bytes read
 * 0x00691BD4, which is the literal "Arial". That is a fact about the shipped
 * binary and it belongs in the source as the string it is.
 *
 * The pointer-to-fixed-address spelling this replaces also compiled wrong:
 * WinMain's `mov ecx, [0x691b2c]` is ONE load of a `char *` variable, and
 * `*DefaultFontFace` through an `LPCSTR *` is two.
 */
extern LPCSTR DefaultFontFace;
extern LPCSTR DefaultLargeFontFace;


// 0x007D3948 in the image - the single game-wide font, initialised by
// WinMain and closed at exit.
extern Font g_JACKAL_FONT;
