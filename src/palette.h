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
#pragma once
#include "time.h"

class Win;     // forward declaration
struct Dib;    // <buffer.h>, where the DIB block is declared
class Buffer;  // forward declaration
struct IDirectDrawPalette;  // <ddraw.h>, included where it is called

 /*
  * Palette class
  */
class DLLEXPORT Palette {
 public:
  Palette();                         // 005FE2A0
  // A TAIL JUMP, not an empty body: the image's destructor is one
  // instruction, `jmp ?close@Palette@@QAEXXZ`.
  ~Palette() { close(); }   // 005FE2E0

  int get_rgbquad(RGBQUAD *output, int start, int count);
  // 0x005FE650. Not recovered; `Buffer::load_pcx` hands it the buffer's own
  // `Dib` after decoding, which is the one call site this tree has.
  int set_from_dib(Dib *dib);
  // `static`, and the image's name says so: `?init_palette_class@Palette@@
  // SAXH@Z`. It used to be catalogued `QAA`, which this comment described as
  // "a public member declared __cdecl, taking no receiver" - a contradiction,
  // because `QAA` DOES take one, as the first stack argument. The body read
  // its `int` one slot too high for as long as that name stood. Corrected
  // 2026-08-15 against two independent bytes; see the annotation on the body
  // in palette.cpp.
  int init();                        // 005FE330
  void close();                      // 005FE500
  int set();                         // 005FE460
  static void init_palette_class(int mode);
  static void set_active_window(Win *window);
  int get_pos(int value);
  // ?UNK7@Palette@@QAEHHHHHHH@Z at 0x005FF280, named from what it does:
  // walk `entries_[start .. start+count)` and return the index whose RGB is
  // nearest, by squared distance. `skip_animated` makes it ignore every
  // entry an active `internal_` slot has reserved.
  int closest(int red, int green, int blue, int start, int count,
              int skip_animated);
  // ?UNK3@Palette@@QAEXH@Z at 0x005FE950: drop the animation slot holding
  // `key` - stop its timer, put the colours it was cycling back, re-apply
  // the palette, free its buffer, and close the gap in the table.
  void remove_animation(int key);

 private:
  // Buffer caches this generation tag to skip republishing an unchanged
  // palette; see Buffer::sync_to_palette.
  friend class Buffer;
  // The 256 palette entries themselves, at offset 0. get_rgbquad and
  // init read and write them as bytes; GetSystemPaletteEntries fills them
  // directly in init.
  PALETTEENTRY entries_[256];
  uint32_t seed_;
  // FIVE ANIMATION SLOTS, named from what `?UNK3@Palette@@QAEXH@Z`
  // (0x005FE950) does with them: it finds the slot whose `key` matches its
  // argument, deletes the slot's timer, runs
  //
  //     memcpy(&entries_[first], colours, count * sizeof(PALETTEENTRY))
  //
  // then syncs a Buffer and calls `GDI32!AnimatePalette`. `get_pos` scans
  // `key` for a match or for -1, which is find-or-allocate; the constructor
  // and `close` set every slot back to that state.
  struct PaletteInternal {
      uint32_t key;         // the slot's id, -1 when free
      Time *time;           // OWNED: `close` deletes it through ??1Time
      uint8_t first;        // first entries_[] index this slot animates
      uint8_t count;        // how many entries, memcpy'd count * 4 bytes
      uint8_t field_A;      // nothing recovered touches these two - not the
      uint8_t field_B;      // constructor, not `close`, not UNK3
      void *colours;        // malloc'd PALETTEENTRY run; `close` frees it
  } internal_[5];
};

static_assert(sizeof(Palette) == 0x454,
              "Palette layout must match the legacy ABI");

// 0x009B8174, four bytes below PaletteInitialized: jackal_init_real stores
// the palette it was handed here before initialising it, so it is the one
// the process is using.
extern Palette *PaletteCurrent;

/*
 * 0x009B8178. THE HPALETTE ITSELF, not a flag: `init_palette_class` stores
 * `CreatePalette`'s result here and `DeleteObject`s whatever was here first,
 * so "initialised" and "non-null" are the same question and `get_rgbquad`
 * asks it that way. The name is the project's, not the image's - this binary
 * carries no symbols - and it is kept only because the tree already uses it.
 *
 * AN OBJECT, NOT AN ADDRESS. It was `static int *const ... = (int *)0x9B8178`,
 * which names a location in the SHIPPED process and nothing at all in
 * `build/OpenSMACX.exe`: 0x009B8178 lies past `.data`'s stored bytes, which
 * end at 0x006A8000, so the image itself only reserves it as zero-fill. A
 * real global is what the original declaration was - the linker places it, the
 * loader zeroes it, and the executable this repository now builds can actually
 * read it.
 *
 * IT COSTS NO BYTES. `mov eax, [0x9b8178]` through the constant and
 * `mov eax, [PaletteInitialized]` through the global are the same instruction;
 * the four bytes of address are a relocation, and the comparison masks a
 * relocated operand on either side. Same argument as `g_PALETTE1` below, and
 * the same measurement: `get_rgbquad` and `init_palette_class` both held their
 * verdict across the change.
 *
 * `HPALETTE` rather than `int` because that is what the two GDI calls at
 * either end of its life say it is, and it removes a cast from each.
 */
extern HPALETTE PaletteInitialized;

// 0x009B8188. The argument `init_palette_class` was called with, kept so the
// rest of the engine can ask which palette it got. Non-zero means the entries
// were taken from the system palette with `GetSystemPaletteEntries`; zero
// means the built-in twenty were installed and the middle 236 left free.
// Zero-fill like `PaletteInitialized` above, and an object for the same
// reason.
extern int PaletteUsesSystemColours;

/*
 * 0x0067022C. Twenty RGBQUADs - the Windows static colours. The first ten
 * become palette entries 0-9 and the last ten entries 246-255, which is the
 * layout GDI reserves. Bytes, not a struct, because `RGBQUAD` is opaque in a
 * measured unit; index it as `[entry * 4 + BLUE|GREEN|RED]`.
 *
 * THE VALUES ARE CARRIED, not the address, and this one had to be: unlike
 * `PaletteInitialized` above, 0x0067022C is BELOW `.data`'s stored bytes,
 * so these eighty bytes are real content in the shipped image rather than
 * zero-fill. A constant address would read whatever happens to sit there in
 * `build/OpenSMACX.exe` - which is how a global that "works" under injection
 * turns into a wrong palette in a standalone binary, silently. They were read
 * back out of the image and they are the standard Windows twenty.
 */
extern const uint8_t SystemColours[80];

/*
 * 0x009B8180. The palette currently being realised: Palette::set stores
 * `this` here before syncing, so the rest of the engine can ask which palette
 * is live. Zero-fill like PaletteInitialized, an object pointer for the same
 * reason.
 */
extern Palette *PaletteActive;

/*
 * 0x009B7490. The screen buffer Palette::set realises against (the same
 * buffer Win::init_class sets up). Named here rather than re-derived in each
 * caller.
 *
 * AN OBJECT: `Palette::set` sets the receiver up as `mov ecx, 0x9b7490` - an
 * immediate, the address of a global - before calling `sync_to_palette`.
 * Through
 * `extern Buffer *` the same source compiles to
 * `mov ecx, dword ptr [ScreenBuffer]`, a load the image does not perform, and
 * it lands one instruction earlier than the `push esi` beside it. An `extern`
 * declaration of an incomplete type is legal; the DEFINITION in palette.cpp is
 * where `buffer.h` is needed.
 */
extern Buffer ScreenBuffer;

/*
 * 0x009B8184. The seed_ value Palette::set last animated, so it can skip
 * re-animating an unchanged palette (Buffer caches the same comparison).
 */
extern int PaletteSeedCache;

/*
 * 0x009BC4A0. NOT A SURFACE - an IDirectDrawPalette. `Palette::set` reaches it
 * when BufferDirectDraw is on and calls vtable slot 6 with
 * `(0, 0, 0x100, entries)`, which is
 * `SetEntries(DWORD dwFlags, DWORD dwStartingEntry, DWORD dwCount,
 * LPPALETTEENTRY lpEntries)` - slot 6 of IDirectDrawPalette, after
 * QueryInterface, AddRef, Release, GetCaps, GetEntries and Initialize. A
 * surface's slot 6 is BltBatch, which takes different arguments and would
 * make no sense of a 256-entry count. Opaque here because only that one call
 * is recovered; naming it wrongly is what made the call look arbitrary.
 */
extern IDirectDrawPalette *DirectDrawPalette;

/*
 * The process palette, at 0x0094C590 in the image. The name is the image's
 * own: its dynamic initialiser is `??__Eg_PALETTE1@@YAXXZ`, recovered in
 * src/init_thunks.cpp, and its teardown `??__Fg_PALETTE1@@YAXXZ`.
 *
 * AN OBJECT, NOT A `Palette *` TO A FIXED ADDRESS. Both spellings name the
 * same storage while the DLL is injected into the shipped image, but they are
 * not the same C++: `WinMain` passes the palette to `jackal_init_real` as
 * `push 0x94c590`, which is the address of a global, and a `Palette *`
 * variable would compile that same source into `push dword ptr [g_PALETTE1]`
 * - a load the image does not perform. Declaring it as what it is costs
 * nothing and removes an indirection from every future call site.
 */
extern Palette g_PALETTE1;
int __fastcall palette_get_rgbquad_redirect(
    Palette *self, void *, RGBQUAD *output, int start, int count);

void __cdecl palette_set_active_window_redirect(Win *window);
int __fastcall palette_get_pos_redirect(Palette *self, void *, int value);
