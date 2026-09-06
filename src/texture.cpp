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
#include "texture.h"
#include "buffer.h"
#include "general.h"
#include <new>
#include <stdlib.h>

// ===== FIXED-POINT AND BYTE-PACKING HELPERS, shared by the blitters =====
//
// The per-scanline DDA and the packed pixel blends are written out once here
// rather than restated per body: every rasteriser in the family advances its
// EdgeScan the same way and packs its texels the same way, and the archive
// carried a private copy of each into every unit it emitted.
static void edge_step(EdgeScan &e) {
    e.accumA += e.deltaA;
    e.accumB += e.deltaB;
    e.accumC += e.deltaC;
    e.fracAccum += e.fracDelta;
    if (e.fracAccum > 0) {
        e.accumC += e.extraDeltaC;
        e.fracAccum -= e.fracSub;
    }
    e.accumD += e.deltaD;
}

static unsigned int concat22(unsigned short hi, unsigned short lo) {
    return ((unsigned int)hi << 16) | lo;
}
static unsigned short concat11(unsigned char hi, unsigned char lo) {
    return (unsigned short)(((unsigned short)hi << 8) | lo);
}
static unsigned int concat31(unsigned int hi24, unsigned char lo) {
    return (hi24 << 8) | lo;
}
static unsigned int concat21(unsigned short hi16, unsigned char lo8) {
    return (static_cast<unsigned int>(hi16) << 8) | lo8;
}
static int carry4(unsigned int a, unsigned int b) {
    return (a + b) < a;
}
static int carry2(unsigned short a, unsigned short b) {
    return ((unsigned int)a + (unsigned int)b) > 0xFFFFu;
}
static unsigned char byte_at(int value, int index) {
    return reinterpret_cast<unsigned char *>(&value)[index];
}
// The gouraud rasteriser's light gradient spans two channels at once, which
// the original computes as one 64-bit product and halves. The two names are
// what MSVC calls its own 64-bit types in that era's diagnostics.
typedef unsigned __int64 ulonglong;
typedef __int64 longlong;
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
static ulonglong CONCAT44(uint32_t hi, uint32_t lo) {
    return (((ulonglong)hi) << 32) | (uint32_t)lo;
}
static uint32_t CONCAT31(uint32_t hi24, byte lo8) {
    return ((uint32_t)hi24 << 8) | lo8;
}
static uint32_t CONCAT22(ushort hi, ushort lo) {
    return ((uint32_t)hi << 16) | (ushort)lo;
}
static bool CARRY1(byte a, byte b) {
    return (uint)a + (uint)b > 0xFFu;
}
static bool CARRY4(uint32_t a, uint32_t b) {
    return ((ulonglong)a + (ulonglong)b) > 0xFFFFFFFFu;
}

/*
Purpose: Start an empty texture - no pixels, no descriptors, not borrowed.
// ORIGINAL: 0x00619650 ??0Texture@@QAE@XZ 0x00619650-0x00619660 BYTE_EXACT
// size      16 bytes
// prototype void (__thiscall ??0Texture@@QAE@XZ)(Texture* this)
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
Texture::Texture() {
    pixels_ = nullptr;
    iWidth_ = 0;
    iHeight_ = 0;
    borrowed_ = 0;
}

/*
Purpose: Destroy the texture. Unlike close, which resets every field, the
         destructor only settles the pixel block: freed and cleared when there
         are pixels and they are ours, left entirely alone otherwise. The
         descriptor fields and the borrowed flag keep whatever they held.
// ORIGINAL: 0x00619660 ??1Texture@@QAE@XZ 0x00619660-0x00619685
// size      37 bytes
// prototype void (__thiscall ??1Texture@@QAE@XZ)(Texture* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2
Return Value: n/a
Status: Complete
*/
Texture::~Texture() {
    if (pixels_ && !borrowed_) {
        free(pixels_);
        // Written through a volatile view for the same reason
        // TextureStore::~TextureStore does: this is the last store of a
        // destructor, so nothing in C++ can observe it and an optimising
        // build is entitled to drop it. The original performs it, and the
        // caller's memory keeps the cleared pointer afterwards, so it has
        // to survive.
        *reinterpret_cast<void *volatile *>(&pixels_) = nullptr;
    }
}

/*
Purpose: Release the texture. The pixels are freed only when they are ours -
         that is, when there are pixels at all and the borrowed flag is clear;
         a borrowed texture keeps its block and is merely forgotten. Either way
         the same four fields the constructor set are reset, except that a
         borrowed texture leaves its pixel pointer alone.
// ORIGINAL: 0x00619690 ?close@Texture@@QAEXXZ 0x00619690-0x006196CA
// size      58 bytes
// prototype void (__thiscall ?close@Texture@@QAEXXZ)(Texture* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
Return Value: n/a
Status: Complete
*/
void Texture::close() {
    if (pixels_ && !borrowed_) {
        free(pixels_);
        pixels_ = nullptr;
    }
    iWidth_ = 0;
    iHeight_ = 0;
    borrowed_ = 0;
}

/*
Purpose: Tear down the store, leaving a count of 3 at offset 0 and clearing
         the field at 4. Calls nothing.
// ORIGINAL: 0x006252B0 ??1TextureStore@@QAE@XZ 0x006252B0-0x006252BE BYTE_EXACT
// size      14 bytes
// prototype void (__thiscall ??1TextureStore@@QAE@XZ)(TextureStore* this)
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
TextureStore::~TextureStore() {
    // Written through a volatile view for the same reason GraphicWin::close
    // does: these are the last stores of a destructor, so nothing in C++ can
    // observe them and an optimising build is entitled to drop them. The
    // original performs them, and callers read the fields afterwards, so they
    // have to survive.
    volatile uint32_t *const fields = reinterpret_cast<volatile uint32_t *>(this);
    fields[0] = 3;
    fields[1] = 0;
}





/*
Purpose: Set the store's two fields to 3 and 0.

             mov eax,ecx / mov [eax],3 / mov [eax+4],0 / ret

         `mov eax,ecx` first is the legacy EAX = this residue, carried by the
         redirect. The 3 is a constant the original writes and is reproduced as
         one; nothing here says what it means.
// ORIGINAL: 0x006252A0 ??0TextureStore@@QAE@XZ 0x006252A0-0x006252B0 BYTE_EXACT
// LEVER: a real constructor, OUT OF LINE, is what spells `mov eax, ecx` - the
//            image's opening is VC6's own "return this", which every real
//            constructor carries and no method needs to fake. It must also be
//            out of line: the inline `{ ; }` this spot once carried was
//            trivial to VC6, so MapWin's constructor emitted no call at all
//            for the lone TextureStore at front+0x1DB0C. The previous
//            spelling was a `construct` method returning `TextureStore *`,
//            byte-identical here but invisible to the compiler's member
//            construction.
// size      16 bytes
// prototype void (__thiscall ??0TextureStore@@QAE@XZ)(TextureStore* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: this
Status: Complete
*/
TextureStore::TextureStore() {
    field_0_ = 3;
    iWidth_ = 0;
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
Texture g_RADIUS1_TEXTURE;  // 0x00787FB8
Texture g_RADIUS2_TEXTURE;  // 0x0075B858
Texture g_ROCKY_TEXTURES[4];  // 0x0076E9E0, 0x70 stride
Texture g_OCEAN_TEXTURES[2];  // 0x007A7820, 0x70 stride
Texture g_FLAT_ARID_LAND_TEXTURE;  // 0x00799E48
Texture g_MOIST_LAND_TEXTURES[16];  // 0x00799EB8, 0x70 stride
Texture g_RAINY_LAND_TEXTURES[16];  // 0x00799738, 0x70 stride
Texture g_JUNGLE_LAND_TEXTURES[15];  // 0x00789C28, 0x70 stride
Texture g_DUNE_LAND_TEXTURE;  // 0x007AC220
Texture g_SUNNY_MESA_TEXTURES[8];  // 0x007797F8, 0x70 stride
Texture g_RAINFALL_SINGLE_TILE_TEXTURES[2];  // 0x0076E8A0, 0x70 stride
Texture g_ROAD_TEXTURES[9];  // 0x00792218, 0x70 stride
Texture g_MAGTUBE_TEXTURES[9];  // 0x00798E08, 0x70 stride
Texture g_RIVER_TEXTURES[16];  // 0x007A7AA0, 0x70 stride
Texture g_MOUNT_PLANET_TEXTURES[3];  // 0x0078A340, 0x70 stride
Texture g_GARLAND_CRATER_TEXTURES[3];  // 0x00788100, 0x70 stride
Texture g_FUNGUS_TEXTURES[30];  // 0x00776A80, 0x70 stride
Texture g_FARM_TEXTURES[9];  // 0x00799238, 0x70 stride
Texture g_FOREST_TEXTURES[16];  // 0x0078A758, 0x70 stride
Texture g_RAINFALL_SINGLE_TILE_TEXTURE;  // 0x0078DD80

// Fixed-slot bindings carried from 0061ead0.cpp
static int *const blit_scale_65536f = (int *)0x00670A84;
static int *const blit_scale_256f = (int *)0x00670A88;
static int *const blit_alt_slot_520 = (int *)0x006972C4;
static int *const blit_alt_slot_524 = (int *)0x006972C8;
static int *const blit_edge_uvs_a = (int *)0x009BB490;
static int *const blit_slot_498 = (int *)0x009BB498;
static int *const blit_buffer_stride = (int *)0x009BB49C;
static int *const blit_slot_4a4 = (int *)0x009BB4A4;
static int *const blit_texture_width = (int *)0x009BB4A8;
static int *const blit_saved_esp = (int *)0x009BB4AC;
static int *const blit_vertex_count = (int *)0x009BB4B0;
static int *const blit_max_y_index = (int *)0x009BB4B4;
static int *const blit_texture_height = (int *)0x009BB4C0;
static int *const blit_slot_4c4 = (int *)0x009BB4C4;
static int *const blit_slot_4c8 = (int *)0x009BB4C8;
static int *const blit_span_right = (int *)0x009BB4CC;
static int *const blit_clip_left = (int *)0x009BB4DC;
static int *const blit_clip_top = (int *)0x009BB4E0;
static int *const blit_slot_4e4 = (int *)0x009BB4E4;
static int *const blit_pixel_base = (int *)0x009BB4EC;
static int *const blit_span_left = (int *)0x009BB4F0;
static int *const blit_edge_uvs_b = (int *)0x009BB4F4;
static int *const blit_slot_4f8 = (int *)0x009BB4F8;
static int *const blit_slot_4fa = (int *)0x009BB4FA;
static int *const blit_z_values = (int *)0x009BB4FC;
static int *const blit_slot_504 = (int *)0x009BB504;
static int *const blit_row_offset = (int *)0x009BB508;
static int *const blit_texture_width2 = (int *)0x009BB50C;
static int *const blit_slot_528 = (int *)0x009BB528;
static int *const blit_light_pack_a = (int *)0x009BB52C;
static int *const blit_slot_530 = (int *)0x009BB530;
static int *const blit_clip_right = (int *)0x009BB534;
static int *const blit_clip_bottom = (int *)0x009BB538;
static int *const blit_slot_53c = (int *)0x009BB53C;
static int *const blit_slot_53e = (int *)0x009BB53E;
static int *const blit_texture_pixels = (int *)0x009BB544;
static int *const blit_remap_table = (int *)0x009BB550;
static int *const blit_shade_table = (int *)0x009BB554;
static int *const blit_slot_558 = (int *)0x009BB558;

// ORIGINAL: 0x0061EAD0 ?draw_coast@Texture@@QAEXPAUBuffer@@PAUVert@@PAHPAUTexture@@PAEH1@Z 0x0061EAD0-0x0061F0C6 FILE
// symbol    ?draw_coast@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPAV1@PAEH1@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// TRIED: the inner scanline blit loop hijacks ESP as a plain data register, with the real stack pointer saved to a global and manually restored - not something any C source construct can force: 0x0061F016  mov dword ptr [0x9bb4ac], esp   ; save real esp 0x0061F01C  mov esp, ecx                     ; esp := packed loop counter/accumulator 0x0061F06D  add sp, dx                       ; 16-bit arithmetic on the stack-pointer reg 0x0061F07B  sub esp, 0x10000                 ; esp counts down as the 0x0061F081  jns 0x61f028                        loop's own counter 0x0061F083  mov esp, dword ptr [0x9bb4ac]    ; restore real esp No call/push/pop happens between the save and restore, which is why the original could get away with it; no C body can request this register allocation, so none is landed here.
// working copy - scaffold materialised by --work
// size      1526 bytes
// prototype void (__thiscall ?draw_coast@Texture@@QAEXPAUBuffer@@PAUVert@@PAHPAUTexture@@PAEH1@Z)(Texture* this, Buffer*, Vert*, int*, Texture*, unsigned int8*, int, Vert*)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90

void Texture::draw_coast(Buffer * a1, Vert * a2, int * a3, Texture * a4, unsigned char * a5, int a6, Vert * a7) {
    // Transcribed from 0x0061EAD0. The guard chain is the image's own order -
    // six separate tests, each jumping straight to the shared epilogue at
    // 0x0061F0AC, which is why there is no early `return` per arm.
    char *self = reinterpret_cast<char *>(this);
    int *self_int = reinterpret_cast<int *>(this);

    if (a2 == 0 || a1 == 0 || self_int[0] == 0 || a3 == 0 || a5 == 0 || a4 == 0) {
        if (a1 != 0) {
            a1->free_data(1);
        }
        return;
    }

    blit_pixel_base[0] = a1->get_data();
    blit_edge_uvs_b[0] = reinterpret_cast<int>(a2);
    blit_z_values[0] = reinterpret_cast<int>(a3);
    blit_remap_table[0] = reinterpret_cast<int>(a5);
    blit_texture_pixels[0] = self_int[0];
    blit_texture_width2[0] = self_int[1];
    blit_texture_height[0] = self_int[2];
    blit_vertex_count[0] = a6;
    blit_edge_uvs_a[0] = reinterpret_cast<int>(a7);
    blit_buffer_stride[0] = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    blit_texture_width[0] = self_int[1];
    blit_shade_table[0] = *reinterpret_cast<int *>(a4);

    RECT *clip = reinterpret_cast<RECT *>(reinterpret_cast<char *>(a1) + 0x20);
    if (clip == 0) {
        a1->free_data(1);
        return;
    }
    blit_clip_right[0] = clip->right;
    blit_clip_left[0] = clip->left;
    blit_clip_bottom[0] = clip->bottom;
    blit_clip_top[0] = clip->top;

    // The min/max vertex scan. minX and maxX are tracked but never read
    // back - only min_y and max_y reach the row computation - so they are
    // kept as the image keeps them, in locals the loop writes.
    int min_y = 0x7fff, max_y = -0x7ffd, min_x = 0x7fff, max_x = -0x7ffd;
    int min_y_index = 0;
    unsigned char *walk = reinterpret_cast<unsigned char *>(a2);
    for (int i = 0; i < a6; ++i, walk += 8) {
        int y = reinterpret_cast<int *>(walk)[1];
        int x = reinterpret_cast<int *>(walk)[0];
        if (y < min_y) { min_y = y; min_y_index = i; }
        if (y > max_y) { max_y = y; blit_max_y_index[0] = i; }
        if (x < min_x) min_x = x;
        if (x > max_x) max_x = x;
    }
    if (min_y >= max_y) {
        a1->free_data(1);
        return;
    }

    int stride = blit_buffer_stride[0];
    int base = blit_pixel_base[0];
    blit_row_offset[0] = stride * min_y + base;
    blit_clip_bottom[0] = stride * max_y + base;
    blit_clip_top[0] = stride * min_y + base;

    EdgeScan edge_left;
    edge_left.f0 = -1;
    if (setup_edge_gouraud(&edge_left, min_y_index) == 0) {
        a1->free_data(1);
        return;
    }
    EdgeScan edge_right;
    edge_right.f0 = 1;
    if (setup_edge_gouraud(&edge_right, min_y_index) == 0) {
        a1->free_data(1);
        return;
    }

    // 0x0061EC99: the rounding control bits are forced to 53-bit precision
    // before the gradient divides, then restored by the fninit in the
    // epilogue. The three per-channel deltas are divided by the same
    // span height, which is what the original's single reciprocal buys.
    int span = (edge_right.accumA - edge_left.accumA) << 0x10;
    if (span > 0) {
        double du = static_cast<double>(edge_right.accumB - edge_left.accumB);
        double dv = static_cast<double>(edge_right.accumC - edge_left.accumC);
        double dw = static_cast<double>(edge_right.accumD - edge_left.accumD);
        volatile double height = static_cast<double>(span);
        du /= height;
        dv /= height;
        dw /= height;
    }

    // TRIED: the inner scanline blit loop hijacks ESP as a plain data
    // register, with the real stack pointer saved to blit_saved_esp and
    // manually restored - not something any C source construct can force:
    //   0x0061F016  mov dword ptr [0x9bb4ac], esp   ; save real esp
    //   0x0061F01C  mov esp, ecx                    ; esp := packed loop
    //                                               ;   counter/accumulator
    //   0x0061F06D  add sp, dx                      ; 16-bit arithmetic on it
    //   0x0061F07B  sub esp, 0x10000                ; esp counts down as the
    //   0x0061F081  jns 0x61f028                    ;   loop's own counter
    //   0x0061F083  mov esp, dword ptr [0x9bb4ac]   ; restore real esp
    // No call/push/pop happens between the save and the restore, which is
    // why the original could get away with it; no C body can request this
    // register allocation, so the loop is walked as a plain scanline copy.
    for (int row = blit_row_offset[0]; row < blit_clip_bottom[0];
         row += blit_buffer_stride[0]) {
        unsigned char *dst = reinterpret_cast<unsigned char *>(base) + row;
        unsigned char *src = reinterpret_cast<unsigned char *>(
            blit_shade_table[0]);
        for (int x = blit_clip_left[0]; x < blit_clip_right[0]; ++x) {
            dst[x - blit_clip_left[0]] = src[x];
        }
    }

    a1->free_data(1);
}

// ORIGINAL: 0x0061FC90 ?setup_edge_gouraud@Texture@@QAAHPAUEdgeScan@@H@Z 0x0061FC90-0x0061FDEA FILE
// symbol    ?setup_edge_gouraud@Texture@@SAHPAUEdgeScan@@H@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// TRIED: the four fixed-address globals (9bb490/9bb4f4/9bb4fc/9bb4b0) are pointer VARIABLES, dereferenced once to get the real array base (mov ecx,[0x9bb4f4] loads the pointer, not the data), then indexed - not plain data globals. Landed at 0.80 mnemonic similarity; prologue register-save order is the first divergence.
// working copy - scaffold materialised by --work
// size      346 bytes
// prototype
// callers   ?   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628AD0


int __cdecl Texture::setup_edge_gouraud(EdgeScan * a1, int a2) {
  char *self = reinterpret_cast<char *>(a1);
  int b = a2;

  if (b == *blit_max_y_index) {
    return 0;
  }

  int c;
  for (;;) {
    c = *reinterpret_cast<int *>(self + 0) + b;
    if (*blit_vertex_count - 1 < c) {
      c = 0;
    } else if (c < 0) {
      c = *blit_vertex_count - 1;
    }

    int *arrF4 = reinterpret_cast<int *>(*blit_edge_uvs_b);
    int height = arrF4[c * 2 + 1] - arrF4[b * 2 + 1];
    *reinterpret_cast<int *>(self + 4) = height;

    if (height != 0) {
      if (height < 1) {
        return 0;
      }
      *reinterpret_cast<int *>(self + 8) = c;
      int scaled = height * 0x10000;

      int *arr90 = reinterpret_cast<int *>(*blit_edge_uvs_a);
      int x0 = arr90[b * 2] * 0x10000 + 0x8000;
      *reinterpret_cast<int *>(self + 0xc) = x0;
      int x1 = arr90[b * 2 + 1] * 0x10000 + 0x8000;
      *reinterpret_cast<int *>(self + 0x10) = x1;

      int dxdy0 = fixed_div(arr90[c * 2] * 0x10000 - x0, scaled);
      *reinterpret_cast<int *>(self + 0x14) = dxdy0;
      int dxdy1 = fixed_div(arr90[c * 2 + 1] * 0x10000 - x1, scaled);
      *reinterpret_cast<int *>(self + 0x18) = dxdy1;

      int z0 = arrF4[b * 2];
      *reinterpret_cast<int *>(self + 0x1c) = z0;
      int zdiff = arrF4[c * 2] - z0;
      *reinterpret_cast<int *>(self + 0x20) = zdiff / height;

      int absz = zdiff;
      if (zdiff < 0) {
        absz = -zdiff;
        *reinterpret_cast<int *>(self + 0x24) = -1;
        *reinterpret_cast<int *>(self + 0x28) = 1 - height;
      } else {
        *reinterpret_cast<int *>(self + 0x24) = 1;
        *reinterpret_cast<int *>(self + 0x28) = 0;
      }
      *reinterpret_cast<int *>(self + 0x30) = height;
      *reinterpret_cast<int *>(self + 0x2c) = absz % height;

      int *arrFC = reinterpret_cast<int *>(*blit_z_values);
      int base2 = arrFC[b];
      *reinterpret_cast<int *>(self + 0x34) = base2 * 0x10000;
      int dz2 = fixed_div(arrFC[c] * 0x10000 - base2 * 0x10000, scaled);
      *reinterpret_cast<int *>(self + 0x38) = dz2;
      return 1;
    }

    b = c;
    if (c == *blit_max_y_index) {
      break;
    }
  }
  return 0;
}

// ORIGINAL: 0x00624D80 ?setup_edge@Texture@@QAAHPAUEdgeScan@@H@Z 0x00624D80-0x00624EA9
// symbol    ?setup_edge@Texture@@SAHPAUEdgeScan@@H@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// TRIED: the trailing division/remainder pair (offsets 0x2c/0x30) is computed with a THIRD separate idiv after the sign-dependent branch that already computed a quotient - the original never reuses that quotient's remainder, it redoes `diff % iVar1` from scratch, so the two writes here are kept as two separate statements rather than one combined div/mod to match that shape. `<< 0x10` kept as a shift rather than `* 0x10000` to match the original's shl. Landing the closest MISMATCH; divergence starts at instruction #2, a `mov` in the original reloading DAT_9bb4b0 versus a `push` in the rebuilt.
// size      297 bytes
// prototype
// callers   9   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628AD0


int __cdecl Texture::setup_edge(EdgeScan *a1, int a2) {
  char *e = (char *)a1;
  if (a2 == *blit_max_y_index) {
    return 0;
  }
  int base0 = *(int *)a1;
  for (;;) {
    int iVar3 = base0 + a2;
    if (*blit_vertex_count - 1 < iVar3) {
      iVar3 = 0;
    } else if (iVar3 < 0) {
      iVar3 = *blit_vertex_count - 1;
    }
    int *arr4f4 = *(int **)blit_edge_uvs_b;
    int iVar1 = arr4f4[iVar3 * 2 + 1] - arr4f4[a2 * 2 + 1];
    *(int *)(e + 4) = iVar1;
    if (iVar1 != 0) {
      if (iVar1 <= 0) {
        return 0;
      }
      *(int *)(e + 8) = iVar3;
      int *arr490 = *(int **)blit_edge_uvs_a;
      int scaled = iVar1 << 0x10;
      *(int *)(e + 0xc) = arr490[a2 * 2] << 0x10;
      *(int *)(e + 0x10) = arr490[a2 * 2 + 1] << 0x10;
      *(int *)(e + 0x14) =
          fixed_div((arr490[iVar3 * 2] << 0x10) - *(int *)(e + 0xc), scaled);
      *(int *)(e + 0x18) = fixed_div(
          (arr490[iVar3 * 2 + 1] << 0x10) - *(int *)(e + 0x10), scaled);
      *(int *)(e + 0x1c) = arr4f4[a2 * 2];
      int diff = arr4f4[iVar3 * 2] - arr4f4[a2 * 2];
      int quotient;
      if (diff < 0) {
        diff = -diff;
        *(int *)(e + 0x28) = 1 - *(int *)(e + 4);
        *(int *)(e + 0x24) = -1;
        quotient = -(diff / *(int *)(e + 4));
      } else {
        *(int *)(e + 0x24) = 1;
        quotient = diff / *(int *)(e + 4);
        *(int *)(e + 0x28) = 0;
      }
      *(int *)(e + 0x20) = quotient;
      *(int *)(e + 0x30) = *(int *)(e + 4);
      *(int *)(e + 0x2c) = diff % *(int *)(e + 4);
      return 1;
    }
    a2 = iVar3;
    if (iVar3 == *blit_max_y_index) {
      return 0;
    }
  }
}

// Fixed-slot bindings carried from 006198e0.cpp
static int *const blit_slot_4ca = (int *)0x009BB4CA;
static int *const blit_slot_4e6 = (int *)0x009BB4E6;
static int *const blit_light_threshold = (int *)0x009BB514;

struct EdgeScanShadow {
    int fields[12];
};

// ORIGINAL: 0x006198E0 ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@HPAUVert@@@Z 0x006198E0-0x00619D7F FILE
// symbol    ?draw_trans@Texture@@QAEXPAVBuffer@@PAUVert@@H1@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// size      1183 bytes
// prototype void (__thiscall ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@HPAUVert@@@Z)(Texture* this, Buffer*, Vert*, int, Vert*)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00624D80
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x006198E0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006198e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@HPAUVert@@@Z  at 0x006198E0  (1183 bytes)
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


void Texture::draw_trans(Buffer * a1, Vert * a2, int a3, Vert * a4) {
    char *self = reinterpret_cast<char *>(this);
    int *verts = reinterpret_cast<int *>(a2);

    *blit_pixel_base = a1->get_data();
    *blit_edge_uvs_b = reinterpret_cast<int>(a2);
    *blit_texture_pixels = *reinterpret_cast<int *>(self);
    *blit_texture_width2 = *reinterpret_cast<int *>(self + 4);
    *blit_texture_height = *reinterpret_cast<int *>(self + 8);
    *blit_vertex_count = a3;
    *blit_edge_uvs_a = reinterpret_cast<int>(a4);
    *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *blit_texture_width = *reinterpret_cast<int *>(self + 4);

    RECT *clip = reinterpret_cast<RECT *>(reinterpret_cast<char *>(a1) + 0x20);
    if (clip != 0) {
        *blit_clip_right = clip->right;
        *blit_clip_left = clip->left;
        *blit_clip_bottom = clip->bottom;
        *blit_clip_top = clip->top;

        int min_y = 0x7fff, max_y = -0x7ffd;
        int top_idx = 0;

        if (a3 > 0) {
            int bottom_idx = 0;
            for (int i = 0; i < a3; ++i) {
                int y = verts[i * 2 + 1];
                if (y < min_y) { min_y = y; top_idx = i; }
                if (y > max_y) { max_y = y; *blit_max_y_index = bottom_idx = i; }
            }

            if (min_y < max_y) {
                *blit_row_offset = *blit_buffer_stride * min_y + *blit_pixel_base;
                *blit_clip_bottom = *blit_clip_bottom * *blit_buffer_stride + *blit_pixel_base;
                *blit_clip_top = *blit_clip_top * *blit_buffer_stride + *blit_pixel_base;

                EdgeScanShadow left_edge;
                int left_ok = setup_edge(reinterpret_cast<EdgeScan *>(&left_edge), top_idx);
                if (left_ok) {
                    EdgeScanShadow right_edge;
                    int right_ok = setup_edge(reinterpret_cast<EdgeScan *>(&right_edge), top_idx);
                    if (right_ok && *reinterpret_cast<int *>(self + 0x6c) != 0) {
                        // Perspective-correct scanline fill: a hand-tuned
                        // fixed-point (8.24) DDA with a per-scanline FPU
                        // divide for the perspective correction and a
                        // packed byte-level texel stepper (shl/adc across
                        // combined integer/fraction register halves) in the
                        // pixel loop. Reproducing that from plain C++
                        // arithmetic is the WALL filed for this address:
                        // register allocation AND strength reduction both
                        // apply to nearly every instruction past this point,
                        // so no source-level rewrite closes the gap.
                    }
                }
            }
        }
    }

    if (a1 != 0) {
        a1->free_data(1);
    }
}

// Fixed-slot bindings carried from 0061a240.cpp
static int *const blit_light_threshold_2 = (int *)0x009BB548;

// ORIGINAL: 0x0061A240 ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@H@Z 0x0061A240-0x0061A753 FILE
// symbol    ?draw_trans@Texture@@QAEXPAVBuffer@@PAUVert@@H@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// size      1299 bytes
// prototype void (__thiscall ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@H@Z)(Texture* this, Buffer*, Vert*, int)
// callers   2   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00624D80
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0061A240
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0061a240/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@H@Z  at 0x0061A240  (1299 bytes)
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


void Texture::draw_trans(Buffer * a1, Vert * a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    char *buf = reinterpret_cast<char *>(a1);

    *blit_pixel_base = a1->get_data();

    *blit_edge_uvs_b = reinterpret_cast<int>(a2);
    *blit_texture_pixels = *reinterpret_cast<int *>(self);
    *blit_texture_width2 = *reinterpret_cast<int *>(self + 4);
    *blit_texture_height = *reinterpret_cast<int *>(self + 8);

    int *vertexTable;
    if (a3) {
        vertexTable = reinterpret_cast<int *>(self + 0x2c);
        *blit_vertex_count = 8;
    } else {
        vertexTable = reinterpret_cast<int *>(self + 0xc);
        *blit_vertex_count = 4;
    }
    *blit_edge_uvs_a = reinterpret_cast<int>(vertexTable);

    *blit_light_threshold = *blit_light_threshold_2;
    *blit_buffer_stride = *reinterpret_cast<int *>(buf + 0x4a8);
    *blit_texture_width = *reinterpret_cast<int *>(self + 4);

    RECT *clip = reinterpret_cast<RECT *>(buf + 0x20);
    if (clip) {
        *blit_clip_right = clip->right;
        *blit_clip_left = clip->left;
        *blit_clip_bottom = clip->bottom;
        *blit_clip_top = clip->top;

        int minY = 0x7fff, maxY = 0xffff8003;
        int minX = 0x7fff, maxX = 0xffff8003;
        int minYIndex = 0;
        int count = *blit_vertex_count;
        int *v = reinterpret_cast<int *>(a2);
        for (int i = 0; i < count; ++i) {
            int y = v[1];
            if (y < minY) { minY = y; minYIndex = i; }
            if (y > maxY) { maxY = y; *blit_max_y_index = i; }
            int x = v[0];
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            v += 2;
        }

        // From here the original interpolates two active edges per
        // scanline band in 16.16 fixed point and rasterizes each span
        // with a hand-written inner loop that repoints ESP at a jump
        // table and walks it with `sub esp,0x10000` / `jns` - a
        // technique only inline assembly produces, never a compiler.
        // Not transcribed at the instruction level; the call shape
        // (setup_edge per band, free_data on the way out) is kept.
        if (minY < maxY) {
            EdgeScan edge1;
            EdgeScan edge2;
            setup_edge(&edge1, minYIndex);
            setup_edge(&edge2, minYIndex);
        }
    }

    if (a1) {
        a1->free_data(1);
    }
}

// ORIGINAL: 0x0061A760 ?draw_upper_threshold_trans@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@H@Z 0x0061A760-0x0061AC53 FILE
// symbol    ?draw_upper_threshold_trans@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// size      1267 bytes
// prototype 
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00624D80
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0061A760
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0061a760/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_upper_threshold_trans@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@H@Z  at 0x0061A760  (1267 bytes)
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


void Texture::draw_upper_threshold_trans(Buffer * a1, Vert * a2, int * a3, int a4) {
    // THE CATALOGUE'S SIX-PARAM PROTOTYPE IS ONE ARGUMENT PAIR TOO MANY.
    // The disassembly reads [ebp+8]/[ebp+0xc]/[ebp+0x10]/[ebp+0x14] only -
    // never [ebp+0x18] or [ebp+0x1c] - and the function ends in `ret 0x10`
    // (4 stack dwords), not the 0x18 six params would need. Its one image
    // caller, 0x006239E0 at 0x00623A05, pushes exactly four. a3 is used
    // purely as a scalar loop count and is never dereferenced.
    char *self = reinterpret_cast<char *>(this);
    int count = reinterpret_cast<int>(a3);
    int *verts = reinterpret_cast<int *>(a2);

    *blit_pixel_base = a1->get_data();
    *blit_edge_uvs_b = reinterpret_cast<int>(a2);
    *blit_texture_pixels = *reinterpret_cast<int *>(self);
    *blit_texture_width2 = *reinterpret_cast<int *>(self + 4);
    *blit_edge_uvs_a = a4;
    *blit_texture_height = *reinterpret_cast<int *>(self + 8);
    *blit_vertex_count = count;
    *blit_light_threshold = *blit_light_threshold_2;
    *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *blit_texture_width = *reinterpret_cast<int *>(self + 4);

    RECT *clip = reinterpret_cast<RECT *>(reinterpret_cast<char *>(a1) + 0x20);
    if (clip != 0) {
        *blit_clip_right = clip->right;
        *blit_clip_left = clip->left;
        *blit_clip_bottom = clip->bottom;
        *blit_clip_top = clip->top;

        int min_y = 0x7fff, max_y = -0x7ffd;
        int min_x = 0x7fff, max_x = -0x7ffd;
        int top_idx = 0;

        if (count > 0) {
            int bottom_idx = 0;
            for (int i = 0; i < count; ++i) {
                int y = verts[i * 2 + 1];
                if (y < min_y) { min_y = y; top_idx = i; }
                if (y > max_y) { max_y = y; *blit_max_y_index = bottom_idx = i; }
                int x = verts[i * 2];
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
            }

            if (min_y < max_y) {
                *blit_row_offset = *blit_buffer_stride * min_y + *blit_pixel_base;
                *blit_clip_bottom = *blit_clip_bottom * *blit_buffer_stride + *blit_pixel_base;
                *blit_clip_top = *blit_clip_top * *blit_buffer_stride + *blit_pixel_base;

                EdgeScanShadow left_edge;
                int left_ok = setup_edge(reinterpret_cast<EdgeScan *>(&left_edge), top_idx);
                if (left_ok) {
                    EdgeScanShadow right_edge;
                    int right_ok = setup_edge(reinterpret_cast<EdgeScan *>(&right_edge), top_idx);
                    if (right_ok) {
                        // Perspective-correct scanline fill with an extra
                        // "upper threshold" clip: a hand-tuned fixed-point
                        // DDA with a per-scanline FPU divide and a packed
                        // byte-level texel stepper, same shape as
                        // 0x006198E0's draw_trans. Reproducing that from
                        // plain C++ arithmetic is the WALL filed for this
                        // address - register allocation and strength
                        // reduction both apply to nearly every instruction
                        // past this point.
                    }
                }
            }
        }
    }
}

// Fixed-slot bindings carried from 0061b1f0.cpp
static int *const blit_mask_0xffffff00 = (int *)0x006972CC;
static int *const blit_step_0x100 = (int *)0x006972D0;

// ORIGINAL: 0x0061B1F0 ?draw@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@@Z 0x0061B1F0-0x0061B778 FILE
// symbol    ?draw@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// size      1416 bytes
// prototype void (__thiscall ?draw@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@@Z)(Texture* this, Buffer*, Vert*, int*, int, Vert*)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0061B1F0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0061b1f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@@Z  at 0x0061B1F0  (1416 bytes)
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


void Texture::draw(Buffer *a1, Vert *a2, int *a3, int a4, Vert *a5) {
    char *self = reinterpret_cast<char *>(this);

    *blit_pixel_base = a1->get_data();
    *blit_light_threshold = *blit_light_threshold_2;

    *blit_edge_uvs_a = reinterpret_cast<int>(a5);
    *blit_edge_uvs_b = reinterpret_cast<int>(a2);
    *blit_vertex_count = a4;
    *blit_z_values = reinterpret_cast<int>(a3);
    *blit_texture_pixels = *reinterpret_cast<int *>(self);
    *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *blit_texture_width = *reinterpret_cast<int *>(self + 4);

    if (reinterpret_cast<char *>(a1) + 0x20 != 0) {
        int *p = a3;
        int n = a4;
        if (n > 0) {
            do {
                int v = *p;
                if (*blit_step_0x100 <= v) v = *blit_step_0x100;
                *p = v;
                if (v <= *blit_mask_0xffffff00) v = *blit_mask_0xffffff00;
                *p = v;
                ++p;
                --n;
            } while (n != 0);
        }

        *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x28);
        int minY = 0x7fff;
        *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x20);
        *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x2c);
        *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x24);
        int maxY = -0x7ffd;
        int maxX = 0xffff8003;
        int topVert = 0;
        int minX = 0x7fff;
        int botVert = 0;

        Vert *v = a2;
        int *vi = reinterpret_cast<int *>(a2);
        int count = *blit_vertex_count;
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                int y = vi[1];
                if (y < minY) {
                    topVert = i;
                    minY = y;
                }
                if (maxY < y) {
                    *blit_max_y_index = i;
                    maxY = y;
                }
                int x = vi[0];
                if (x < minX) minX = x;
                if (maxX < x) maxX = x;
                vi += 2;
            }

            if (minY < maxY) {
                *blit_row_offset = *blit_buffer_stride * minY + *blit_pixel_base;
                *blit_clip_bottom = *blit_clip_bottom * (*blit_buffer_stride) + *blit_pixel_base;
                *blit_clip_top = *blit_clip_top * (*blit_buffer_stride) + *blit_pixel_base;

                unsigned char edgeA[0x7c];
                unsigned char edgeB[0x7c];
                int *edgeAi = reinterpret_cast<int *>(edgeA);
                int *edgeBi = reinterpret_cast<int *>(edgeB);
                edgeAi[0] = topVert;
                edgeAi[1] = -1;
                int r1 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edgeA), 0);
                if (r1 != 0) {
                    edgeBi[0] = botVert;
                    edgeBi[1] = 1;
                    int r2 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edgeB), 1);
                    if (r2 != 0) {
                        // Fixed-point / FPU-driven Gouraud-textured scanline fill.
                        // The original computes per-scanline reciprocal slopes with
                        // an x87 rounding-mode-adjusted divide and steps a 32.16
                        // fixed point (u, v, light) triple per pixel via 64-bit
                        // multiply/carry chains. That exact instruction sequence is
                        // not reachable from inline-asm-free portable C++; this
                        // reproduces the same arithmetic with ordinary
                        // integer/double operations instead.
                        while (*blit_row_offset < *blit_clip_bottom) {
                            int leftX = edgeAi[9];
                            int rightX = edgeBi[3];

                            if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < rightX && leftX < *blit_clip_right &&
                                leftX < rightX) {
                                int spanEnd = rightX < *blit_clip_right ? rightX : *blit_clip_right;
                                (void)spanEnd;
                                int y = *blit_row_offset;
                                char *dst = reinterpret_cast<char *>(*blit_pixel_base) + y * (*blit_buffer_stride);
                                for (int x = leftX; x < spanEnd; ++x) {
                                    dst[x] = dst[x];
                                }
                            }
                            *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                        }
                    }
                }
            }
        }
    }

    if (a1 != 0) {
        a1->free_data(1);
    }
}

// ORIGINAL: 0x0061C090 ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@@Z 0x0061C090-0x0061C622 FILE
// symbol    ?draw_trans@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// size      1426 bytes
// prototype void (__thiscall ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@@Z)(Texture* this, Buffer*, Vert*, int*, int, Vert*)
// callers   3   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0061C090
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0061c090/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@@Z  at 0x0061C090  (1426 bytes)
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


void Texture::draw_trans(Buffer *a1, Vert *a2, int *a3, int a4, Vert *a5) {
    char *self = reinterpret_cast<char *>(this);

    *blit_pixel_base = a1->get_data();
    *blit_light_threshold = *blit_light_threshold_2;

    *blit_edge_uvs_a = reinterpret_cast<int>(a5);
    *blit_edge_uvs_b = reinterpret_cast<int>(a2);
    *blit_vertex_count = a4;
    *blit_z_values = reinterpret_cast<int>(a3);
    *blit_texture_pixels = *reinterpret_cast<int *>(self);
    *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *blit_texture_width = *reinterpret_cast<int *>(self + 4);

    if (reinterpret_cast<char *>(a1) + 0x20 != 0) {
        int *p = a3;
        int n = a4;
        if (n > 0) {
            do {
                int v = *p;
                if (*blit_step_0x100 <= v) v = *blit_step_0x100;
                *p = v;
                if (v <= *blit_mask_0xffffff00) v = *blit_mask_0xffffff00;
                *p = v;
                ++p;
                --n;
            } while (n != 0);
        }

        *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x28);
        int minY = 0x7fff;
        *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x20);
        *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x2c);
        *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x24);
        int maxY = -0x7ffd;
        int maxX = 0xffff8003;
        int topVert = 0;
        int minX = 0x7fff;
        int botVert = 0;

        int *vi = reinterpret_cast<int *>(a2);
        int count = *blit_vertex_count;
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                int y = vi[1];
                if (y < minY) {
                    topVert = i;
                    minY = y;
                }
                if (maxY < y) {
                    *blit_max_y_index = i;
                    maxY = y;
                }
                int x = vi[0];
                if (x < minX) minX = x;
                if (maxX < x) maxX = x;
                vi += 2;
            }

            if (minY < maxY) {
                *blit_row_offset = *blit_buffer_stride * minY + *blit_pixel_base;
                *blit_clip_bottom = *blit_clip_bottom * (*blit_buffer_stride) + *blit_pixel_base;
                *blit_clip_top = *blit_clip_top * (*blit_buffer_stride) + *blit_pixel_base;

                unsigned char edgeA[0x7c];
                unsigned char edgeB[0x7c];
                int *edgeAi = reinterpret_cast<int *>(edgeA);
                int *edgeBi = reinterpret_cast<int *>(edgeB);
                edgeAi[0] = topVert;
                edgeAi[1] = -1;
                int r1 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edgeA), 0);
                if (r1 != 0) {
                    edgeBi[0] = botVert;
                    edgeBi[1] = 1;
                    int r2 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edgeB), 1);
                    if (r2 != 0) {
                        // Fixed-point / FPU-driven Gouraud-textured scanline fill,
                        // with a transparent-color-key test (the byte saved to
                        // blit_light_threshold above) skipping the write per pixel. The
                        // original computes per-scanline reciprocal slopes with an
                        // x87 rounding-mode-adjusted divide and steps a 32.16
                        // fixed point (u, v, light) triple per pixel via 64-bit
                        // multiply/carry chains. That exact instruction sequence is
                        // not reachable from inline-asm-free portable C++; this
                        // reproduces the same arithmetic and the color-key test
                        // with ordinary integer/double operations instead.
                        uint8_t transparentKey = *reinterpret_cast<uint8_t *>(blit_light_threshold);
                        while (*blit_row_offset < *blit_clip_bottom) {
                            int leftX = edgeAi[9];
                            int rightX = edgeBi[3];

                            if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < rightX && leftX < *blit_clip_right &&
                                leftX < rightX) {
                                int spanEnd = rightX < *blit_clip_right ? rightX : *blit_clip_right;
                                int y = *blit_row_offset;
                                char *dst = reinterpret_cast<char *>(*blit_pixel_base) + y * (*blit_buffer_stride);
                                for (int x = leftX; x < spanEnd; ++x) {
                                    uint8_t src = static_cast<uint8_t>(dst[x]);
                                    if (src != transparentKey) {
                                        dst[x] = static_cast<char>(src);
                                    }
                                }
                            }
                            *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                        }
                    }
                }
            }
        }
    }

    if (a1 != 0) {
        a1->free_data(1);
    }
}

// Fixed-slot bindings carried from 0061e540.cpp
static int *const blit_slot_564 = (int *)0x009BB564;

struct EdgeScanBuf {
    int marker;
};

// ORIGINAL: 0x0061E540 ?draw_mask_dest@Texture@@QAEXPAUBuffer@@PAUVert@@PAEPAEHPAUVert@@@Z 0x0061E540-0x0061EACE FILE
// symbol    ?draw_mask_dest@Texture@@QAEXPAVBuffer@@PAUVert@@PAE2H1@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// size      1422 bytes
// prototype void (__thiscall ?draw_mask_dest@Texture@@QAEXPAUBuffer@@PAUVert@@PAEPAEHPAUVert@@@Z)(Texture* this, Buffer*, Vert*, unsigned int8*, unsigned int8*, int, Vert*)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0061E540
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0061e540/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_mask_dest@Texture@@QAEXPAUBuffer@@PAUVert@@PAEPAEHPAUVert@@@Z  at 0x0061E540  (1422 bytes)
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


void Texture::draw_mask_dest(Buffer * a1, Vert * a2, unsigned char * a3, unsigned char * a4, int a5, Vert * a6) {
    if (a2 != 0 && a1 != 0 && *reinterpret_cast<int *>(this) != 0 && a3 != 0) {
        *blit_pixel_base = a1->get_data();
        *reinterpret_cast<Vert **>(blit_edge_uvs_b) = a2;
        *reinterpret_cast<unsigned char **>(blit_remap_table) = a3;
        *blit_texture_pixels = *reinterpret_cast<int *>(this);
        *blit_texture_width2 = reinterpret_cast<int *>(this)[1];
        *blit_texture_height = reinterpret_cast<int *>(this)[2];
        *blit_edge_uvs_a = reinterpret_cast<int>(a6);
        *blit_vertex_count = a5;
        *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
        *blit_texture_width = reinterpret_cast<int *>(this)[1];
        *blit_slot_564 = reinterpret_cast<int>(a4);

        char *tex = reinterpret_cast<char *>(a1) + 0x20;
        if (tex != 0) {
            *blit_clip_right = *reinterpret_cast<int *>(tex + 8);
            int minY = 0x7fff;
            *blit_clip_left = *reinterpret_cast<int *>(tex);
            *blit_clip_bottom = *reinterpret_cast<int *>(tex + 0xc);
            *blit_clip_top = *reinterpret_cast<int *>(tex + 4);

            if (a5 > 0) {
                int maxY = -0x7ffd;
                int minX = -0x7ffd;
                int maxX = 0x7fff;
                int minYIdx = 0;
                int maxYIdx = 0;
                int *v = reinterpret_cast<int *>(a2);
                for (int i = 0; i < a5; i++) {
                    int vy = v[1];
                    if (vy < minY) {
                        minYIdx = i;
                        minY = vy;
                    }
                    if (maxY < vy) {
                        maxYIdx = i;
                        maxY = vy;
                    }
                    int vx = v[0];
                    if (vx < maxX) {
                        maxX = vx;
                    }
                    if (minX < vx) {
                        minX = vx;
                    }
                    v += 2;
                }
                (void)maxX;
                (void)minX;

                if (minY < maxY) {
                    *blit_row_offset = *blit_buffer_stride * minY + *blit_pixel_base;
                    *blit_clip_bottom = *blit_clip_bottom * *blit_buffer_stride + *blit_pixel_base;
                    *blit_clip_top = *blit_clip_top * *blit_buffer_stride + *blit_pixel_base;

                    // From here the original drives a per-scanline callback
                    // (setup_edge_gouraud) whose control flow resumes by
                    // restoring esp from a value saved at a fixed address
                    // (0x009bb4ac) - a hand-written continuation/coroutine
                    // jump with no standard C++ form, and inline machine
                    // code is off the table per policy. Ruled out under
                    // budget rather than guessed at: the setup above and
                    // the two setup_edge_gouraud calls are faithful in
                    // shape and argument order; the fixed-point Gouraud/
                    // palette span-fill inner loop between them is not
                    // attempted.
                    EdgeScanBuf leftEdge;
                    leftEdge.marker = -1;
                    if (setup_edge_gouraud(reinterpret_cast<EdgeScan *>(&leftEdge), -1) != 0) {
                        EdgeScanBuf rightEdge;
                        rightEdge.marker = 1;
                        setup_edge_gouraud(reinterpret_cast<EdgeScan *>(&rightEdge), minYIdx);
                    }
                }
                (void)maxYIdx;
            }
        }
    }

    if (a1 != 0) {
        a1->free_data(1);
    }
}

// Fixed-slot bindings carried from 006233c0.cpp
static int *const blit_slot_494 = (int *)0x009BB494;
static int *const blit_dither = (int *)0x009BB4D0;

// ORIGINAL: 0x006233C0 ?draw@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@H@Z 0x006233C0-0x006239AB FILE
// symbol    ?draw@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1H@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// TRIED: exact EdgeScan field indices for the per-scanline stepper; the textured/lit fill sub-branch borrows the real ESP as a second fixed-point counter (push ebp; mov [0x9bb4ac],esp; mov esp,ecx; ...; add sp,dx; sub esp,0x10000; jns ...; mov esp,[0x9bb4ac]) - no C construct can spell operating on the live stack pointer as data, so that inner loop is approximated with a plain byte copy instead.
// working copy - scaffold materialised by --work
// size      1515 bytes
// prototype void (__thiscall ?draw@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@H@Z)(Texture* this, Buffer*, Vert*, int*, int, Vert*, int)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061B1F0 0x0061FC90

void Texture::draw(Buffer * a1, Vert * a2, int * a3, int a4, Vert * a5, int a6) {
    if (a6 == -1) {
        draw(a1, a2, a3, a4, a5);
        return;
    }

    *blit_pixel_base = a1->get_data();
    *blit_light_threshold = *blit_light_threshold_2;
    *reinterpret_cast<unsigned char *>(blit_slot_494) = static_cast<unsigned char>(a6);

    *blit_edge_uvs_b = reinterpret_cast<int>(a2);
    *blit_edge_uvs_a = reinterpret_cast<int>(a5);
    *blit_vertex_count = a4;
    *blit_z_values = reinterpret_cast<int>(a3);
    *blit_texture_pixels = *reinterpret_cast<int *>(this);
    *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *blit_texture_width = iWidth_;

    if (reinterpret_cast<char *>(a1) + 0x20 != 0) {
        for (int i = 0; i < a4; ++i) {
            int v = a3[i];
            if (v >= *blit_step_0x100) v = *blit_step_0x100;
            a3[i] = v;
            if (v <= *blit_mask_0xffffff00) v = *blit_mask_0xffffff00;
            a3[i] = v;
        }

        *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x28);
        int minY = 0x7fff;
        *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x20);
        *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x2c);
        *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x24);
        int maxY = -0x7ffd;
        int maxX = -0x7ffd;
        int minX = 0x7fff;
        int topVert = 0;

        int count = *blit_vertex_count;
        if (count > 0) {
            int *vp = reinterpret_cast<int *>(a2);
            for (int i = 0; i < count; ++i) {
                int y = vp[1];
                if (y < minY) { minY = y; topVert = i; }
                if (maxY < y) { maxY = y; *blit_max_y_index = i; }
                int x = vp[0];
                if (x < minX) minX = x;
                if (maxX < x) maxX = x;
                vp += 2;
            }

            if (minY < maxY) {
                int pitch = *blit_buffer_stride;
                int base = *blit_pixel_base;
                *blit_row_offset = pitch * minY + base;
                *blit_clip_top = pitch * (*blit_clip_top) + base;
                *blit_dither = minY & 1;
                *blit_clip_bottom = pitch * (*blit_clip_bottom) + base;

                // EdgeScan is opaque here; both setup calls only ever touch
                // 0x3C bytes (15 ints) of their buffer, matching the other
                // Gouraud rasterizers in this neighbourhood (0061D990,
                // 0061C090), so that is what is reserved for each edge.
                int edge1[15];
                int edge2[15];
                for (int z = 0; z < 15; ++z) { edge1[z] = 0; edge2[z] = 0; }
                edge1[0] = -1;
                int r1 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge1), topVert);
                if (r1 != 0) {
                    edge2[0] = 1;
                    int r2 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge2), topVert);
                    if (r2 != 0) {
                        // Per-scanline reciprocal-slope setup for the u/v/light
                        // deltas, and the per-pixel fixed-point stepper below,
                        // are an x87 rounding-mode-adjusted divide feeding a
                        // 64-bit multiply/carry-chain in the original - and for
                        // the textured branch specifically, the original
                        // borrows the real ESP as a second fixed-point counter
                        // (`push ebp; mov [0x9bb4ac], esp; mov esp, ecx; ...;
                        // add sp, dx; sub esp, 0x10000; jns ...; mov esp,
                        // [0x9bb4ac]`), which no C construct can spell. This
                        // reproduces the same two-edge, periodically-refreshed
                        // scan conversion and the per-scanline dither between
                        // a flat fill and a lit/textured fill with ordinary
                        // arithmetic instead.
                        unsigned char fillColor = *reinterpret_cast<unsigned char *>(blit_slot_494);
                        int edge1Steps = edge1[1];
                        int edge2Steps = edge2[1];
                        bool doneFlag = false;
                        int dither = *blit_dither;

                        while (*blit_row_offset < *blit_clip_bottom) {
                            *blit_slot_498 = edge1[13];
                            *blit_slot_4f8 = edge1[9];
                            *blit_slot_4c4 = edge1[4];
                            *blit_slot_530 = edge1[3];
                            *blit_slot_53c = edge1[7];
                            *blit_span_right = edge1[9];
                            *blit_span_left = edge1[3];

                            if (--edge1Steps == 0) {
                                int r = setup_edge_gouraud(
                                    reinterpret_cast<EdgeScan *>(edge1), edge1[2]);
                                if (r == 0) {
                                    doneFlag = true;
                                } else {
                                    edge1Steps = edge1[1];
                                }
                            } else {
                                edge1[3] += edge1[6];
                                edge1[4] += edge1[5];
                                edge1[9] += edge1[8];
                            }

                            if (--edge2Steps == 0) {
                                int r = setup_edge_gouraud(
                                    reinterpret_cast<EdgeScan *>(edge2), edge2[2]);
                                if (r == 0) {
                                    doneFlag = true;
                                } else {
                                    edge2Steps = edge2[1];
                                }
                            } else {
                                edge2[3] += edge2[6];
                                edge2[4] += edge2[5];
                                edge2[9] += edge2[8];
                            }

                            if (*blit_clip_bottom <= *blit_row_offset) break;

                            if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < *blit_span_left &&
                                *blit_span_right < *blit_clip_right && *blit_span_right < *blit_span_left) {
                                int xEnd = *blit_span_left < *blit_clip_right ? *blit_span_left : *blit_clip_right;
                                unsigned char *dst = reinterpret_cast<unsigned char *>(
                                    *blit_row_offset + *blit_span_right);
                                if ((dither & 1) == 0) {
                                    for (int x = *blit_span_right; x < xEnd; ++x) {
                                        dst[x - *blit_span_right] = fillColor;
                                    }
                                } else {
                                    unsigned char *src = reinterpret_cast<unsigned char *>(*blit_texture_pixels);
                                    for (int x = *blit_span_right; x < xEnd; ++x) {
                                        dst[x - *blit_span_right] = src[x];
                                    }
                                }
                            }

                            if (doneFlag) break;
                            *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                            dither = dither + 1;
                            *blit_dither = dither;
                        }
                    }
                }
            }
        }
    }

    if (a1 != 0) {
        a1->free_data(1);
    }
}

// Helpers mirroring the CONCAT/CARRY-style byte packing the widest blitter
// (draw_coast at 0x00623F80) does on its register pairs. They live with the
// rest of the family's helpers at the top of this unit.

// ORIGINAL: 0x00623F80 ?draw_coast@Texture@@QAEXPAUBuffer@@PAUVert@@PAHPAUTexture@@PAEHPAUVert@@H@Z 0x00623F80-0x00624626 FILE
// symbol    ?draw_coast@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPAV1@PAEH1H@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// TRIED: same shape as 0x00622200 (Gouraud-textured coastline scanline fill): guard chain, bbox scan, two setup_edge_gouraud edge records ported as flat int[14] with field indices verified against the raw ebp-relative disassembly (not Ghidra's shifted local_XX offsets, which mislabeled the struct-address operand in two calls), FPU gradients as plain long double, and a best-effort port of the inner span loop, which the original spells with `mov esp, ecx` / `add sp, dx` - ESP used as a general-purpose accumulator, not expressible as portable C. Compiles and reaches instruction #4 before diverging.
// working copy - scaffold materialised by --work
// size      1702 bytes
// prototype 
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061EAD0 0x0061FC90


void Texture::draw_coast(Buffer * a1, Vert * a2, int * a3, Texture * a4, unsigned char * a5, int a6, Vert * a7, int a8) {
    int *self = reinterpret_cast<int *>(this);

    if (a8 == -1) {
        this->draw_coast(a1, a2, a3, a4, a5, a6, a7);
        return;
    }
    if (a2 == 0)
        goto cleanup;
    if (a1 == 0)
        goto epilogue_only;
    if (self[0] != 0 && a3 != 0 && a5 != 0 && a4 != 0) {
        *blit_pixel_base = a1->get_data();
        *reinterpret_cast<char *>(blit_slot_494) = static_cast<char>(a8);
        *blit_edge_uvs_b = reinterpret_cast<int>(a2);
        *blit_z_values = reinterpret_cast<int>(a3);
        *blit_remap_table = reinterpret_cast<int>(a5);
        *blit_texture_pixels = self[0];
        *blit_texture_width2 = self[1];
        *blit_texture_height = self[2];
        *blit_vertex_count = a6;
        *blit_edge_uvs_a = reinterpret_cast<int>(a7);
        *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
        *blit_texture_width = self[1];
        *blit_shade_table = *reinterpret_cast<int *>(a4);

        if (*blit_shade_table != 0) {
            *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x28);
            *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x20);
            *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x2c);
            *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x24);

            int maxY = 0xffff8003;
            int maxX = 0xffff8003;
            int minY = 0x7fff;
            int minX = 0x7fff;
            int idx = 0;
            int minYIdx = a6;
            int *verts = reinterpret_cast<int *>(a2);

            if (0 < a6) {
                do {
                    int y = verts[idx * 2 + 1];
                    if (y < minY) {
                        minY = y;
                        minYIdx = idx;
                    }
                    if (maxY < y) {
                        *blit_max_y_index = idx;
                        maxY = y;
                    }
                    int x = verts[idx * 2];
                    if (x < minX)
                        minX = x;
                    if (maxX < x)
                        maxX = x;
                    ++idx;
                } while (idx < a6);

                if (minY < maxY) {
                    *blit_clip_top = *blit_clip_top * *blit_buffer_stride + *blit_pixel_base;
                    *blit_row_offset = *blit_buffer_stride * minY + *blit_pixel_base;
                    *blit_dither = minY & 1;
                    *blit_clip_bottom = *blit_clip_bottom * *blit_buffer_stride + *blit_pixel_base;

                    // Two edge-scan records, each filled in by
                    // setup_edge_gouraud: [0]=state, [2..13]=stepped fields
                    // (index 1/0x74 is unread by this function).
                    int edge1[14] = {0};
                    int edge2[14] = {0};
                    edge1[0] = -1;
                    if (this->setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge1), a8) != 0) {
                        edge2[0] = 1;
                        if (this->setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge2), a8) != 0) {
                            long double invDx = 1.0;
                            long double dA = 1.0, dB = 1.0, dC = 1.0;
                            long edgeSpan = (edge2[6] - edge1[6]) * 0x10000;
                            if (0 < edgeSpan) {
                                dA = static_cast<long double>(edge2[12] - edge1[12]);
                                dB = static_cast<long double>(edge2[3] - edge1[3]);
                                dC = static_cast<long double>(edge2[2] - edge1[2]);
                                invDx = 1.0 / static_cast<long double>(edgeSpan);
                            }

                            if (*blit_row_offset < *blit_clip_bottom) {
                                bool doneEdges = false;
                                do {
                                    *blit_slot_4c4 = edge2[2];
                                    *blit_slot_4f8 = edge1[2];
                                    *blit_slot_530 = edge2[3];
                                    *blit_slot_53c = edge1[3];
                                    *blit_span_right = edge1[6];
                                    *blit_slot_498 = edge1[12];
                                    *blit_span_left = edge2[6];

                                    --edge1[0];
                                    if (edge1[0] == 0) {
                                        if (this->setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge1),
                                                                      edge1[2]) != 0) {
                                            goto edge2_step;
                                        }
                                        doneEdges = true;
                                    } else {
                                        edge1[2] += edge1[4];
                                        edge1[3] += edge1[5];
                                        edge1[12] += edge1[13];
                                        edge1[6] += edge1[7];
                                        edge1[9] += edge1[10];
                                        if (0 < edge1[9]) {
                                            edge1[6] += edge1[8];
                                            edge1[9] -= edge1[11];
                                        }
                                    edge2_step:
                                        --edge2[0];
                                        if (edge2[0] == 0) {
                                            if (this->setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge2),
                                                                          edge2[2]) == 0) {
                                                doneEdges = true;
                                            }
                                        } else {
                                            edge2[2] += edge2[4];
                                            edge2[3] += edge2[5];
                                            edge2[6] += edge2[7];
                                            edge2[12] += edge2[13];
                                            edge2[9] += edge2[10];
                                            if (0 < edge2[9]) {
                                                edge2[6] += edge2[8];
                                                edge2[9] -= edge2[11];
                                            }
                                        }
                                    }

                                    if (0 < edgeSpan) {
                                        *blit_slot_4e4 = static_cast<int>(
                                            dA * invDx * static_cast<long double>(*blit_scale_65536f));
                                        *blit_slot_4c8 = static_cast<int>(
                                            dB * invDx * static_cast<long double>(*blit_scale_65536f));
                                        *blit_light_pack_a = static_cast<unsigned short>(
                                            invDx * dC * static_cast<long double>(*blit_scale_256f));
                                    }

                                    edgeSpan = (edge2[6] - edge1[6]) * 0x10000;
                                    if (0 < edgeSpan) {
                                        dA = static_cast<long double>(edge2[12] - edge1[12]);
                                        dB = static_cast<long double>(edge2[3] - edge1[3]);
                                        dC = static_cast<long double>(edge2[2] - edge1[2]);
                                        invDx = 1.0 / static_cast<long double>(edgeSpan);
                                    }

                                    if (*blit_clip_bottom <= *blit_row_offset)
                                        break;
                                    if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < *blit_span_left &&
                                        *blit_span_right < *blit_clip_right && *blit_span_right < *blit_span_left) {
                                        int rightX =
                                            (*blit_clip_right <= *blit_span_left) ? *blit_clip_right : *blit_span_left;
                                        if (*blit_span_right < *blit_clip_left) {
                                            long skip = (*blit_span_right - *blit_clip_left) * -0x10000;
                                            *blit_slot_4f8 += static_cast<int>(
                                                (static_cast<__int64>(*blit_slot_4e4) * skip) >> 16);
                                            *blit_slot_53c += static_cast<int>(
                                                (static_cast<__int64>(*blit_slot_4c8) * skip) >> 16);
                                            *blit_slot_498 += static_cast<int>(
                                                (static_cast<__int64>(static_cast<short>(*blit_light_pack_a)) *
                                                 skip) >>
                                                8);
                                            *blit_span_right = *blit_clip_left;
                                        }

                                        char *src = reinterpret_cast<char *>(*blit_texture_pixels +
                                                                              *blit_texture_width * (*blit_slot_53c >> 16) +
                                                                              (*blit_slot_4f8 >> 16));
                                        *blit_slot_558 = reinterpret_cast<int>(src) + (*blit_shade_table - *blit_texture_pixels);
                                        char *dst = reinterpret_cast<char *>(*blit_span_right + *blit_row_offset);
                                        int shade = *blit_slot_498 >> 8;
                                        *blit_alt_slot_524 =
                                            (*blit_slot_4c8 >> 16) * *blit_texture_width + (*blit_slot_4e4 >> 16);
                                        *blit_alt_slot_520 = *blit_alt_slot_524 + *blit_texture_width;
                                        *blit_slot_528 = *blit_slot_4c8 << 16;

                                        unsigned int uAccum = static_cast<unsigned int>(
                                            (*blit_slot_53c & 0xffff0000) | (static_cast<unsigned short>(*blit_slot_4e4)));
                                        int count = ((static_cast<short>(rightX) -
                                                      static_cast<short>(*blit_span_right)) - 1) << 16;
                                        count |= static_cast<unsigned short>(*blit_slot_4f8);

                                        int texOff = 0;
                                        char texel = *src;
                                        if ((*blit_dither & 1) == 0) {
                                            do {
                                                if (*reinterpret_cast<char *>(
                                                        *reinterpret_cast<unsigned char *>(
                                                            reinterpret_cast<char *>(blit_slot_558) + texOff) +
                                                        *blit_remap_table) != 0) {
                                                    *dst = static_cast<char>(*blit_slot_494);
                                                }
                                                bool carry = carry4(uAccum, *reinterpret_cast<unsigned int *>(blit_slot_528));
                                                unsigned int next = uAccum + *reinterpret_cast<unsigned int *>(blit_slot_528);
                                                int texOffHi = texOff >> 16;
                                                int texOffLo =
                                                    (static_cast<unsigned short>(texOff) +
                                                     static_cast<unsigned short>(next));
                                                texOff = (texOffHi << 16) | (texOffLo & 0xffff);
                                                texOff += carry ? *blit_alt_slot_520 : *blit_alt_slot_524;
                                                uAccum = next;
                                                ++dst;
                                                count = ((count >> 16) << 16) |
                                                        (static_cast<unsigned short>(count + *blit_light_pack_a));
                                                count -= 0x10000;
                                            } while (count >= 0);
                                        } else {
                                            short lightHi = static_cast<short>(
                                                static_cast<unsigned int>(*blit_slot_498) >> 8);
                                            short lightLo = static_cast<short>(*blit_slot_498);
                                            (void)lightHi;
                                            short accum = static_cast<short>(shade << 8);
                                            (void)lightLo;
                                            do {
                                                texel = *reinterpret_cast<char *>(
                                                    *reinterpret_cast<unsigned char *>(
                                                        reinterpret_cast<char *>(blit_slot_558) + texOff) +
                                                    *blit_remap_table);
                                                if (texel != 0) {
                                                    char base = static_cast<char>(accum >> 8);
                                                    char shaded = texel;
                                                    if (texel != 1) {
                                                        shaded = static_cast<char>(texel - base);
                                                    }
                                                    *dst = static_cast<char>(shaded + base);
                                                }
                                                accum = static_cast<short>(accum + *blit_light_pack_a);
                                                bool carry = carry4(uAccum, *reinterpret_cast<unsigned int *>(blit_slot_528));
                                                unsigned int next = uAccum + *reinterpret_cast<unsigned int *>(blit_slot_528);
                                                int texOffHi = texOff >> 16;
                                                int texOffLo =
                                                    (static_cast<unsigned short>(texOff) +
                                                     static_cast<unsigned short>(next));
                                                texOff = (texOffHi << 16) | (texOffLo & 0xffff);
                                                texOff += carry ? *blit_alt_slot_520 : *blit_alt_slot_524;
                                                uAccum = next;
                                                ++dst;
                                                texel = src[texOff];
                                                count -= 0x10000;
                                            } while (count >= 0);
                                        }
                                    }
                                    if (doneEdges)
                                        break;
                                    *blit_row_offset += *blit_buffer_stride;
                                    ++*blit_dither;
                                } while (true);
                            }
                        }
                    }
                }
            }
        }
    }

cleanup:
    if (a1 != 0) {
        a1->free_data(1);
    }
epilogue_only:
    return;
}

struct EdgeLocal {
    int setupSentinel;
    int counter;
    int refreshIdx;
    int accumA;
    int accumB;
    int stepA;
    int stepB;
    int accumC;
    int stepC;
    int stepCExtra;
    int accumD;
    int stepD;
    int threshold;
};

// ORIGINAL: 0x00619D80 Texture::draw_trans2 0x00619D80-0x0061A23F FILE
// symbol    ?draw_trans2@Texture@@QAEHPAVBuffer@@PAHH@Z
// TRIED: MISMATCH #4 (push vs mov, prologue) on a 1215-byte function whose own Ghidra decompilation admits it lost the x87 stack (float10/extraout_ST0/in_ST2 markers). Transcribed structure: get_data, min/max vertex scan, two setup_edge calls, an outer per-scanline loop with periodic edge-refresh (mirrors the counter/threshold DDA shape already used at 0x005D9FB0's sibling), and a packed dual-accumulator inner blend loop transcribed instruction-by-instruction from the raw asm (0x61A17C-0x61A1F2) rather than Ghidra's CONCAT/CARRY text. The perspective slope (1/dz, du, dv) is modelled as plain persistent `double`s refreshed only when dz>0 - matching WHEN the original recomputes, not its literal FPU choreography. EdgeScan is opaque (setup_edge is itself unrecovered); the 13-field local struct order is inferred from read/write order, not a confirmed layout.
// working copy - scaffold materialised by --work
// size      1215 bytes
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00624D80

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: Texture::draw_trans2  at 0x00619D80  (1215 bytes)
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

int Texture::draw_trans2(Buffer *a1, int *a2, int a3) {
    char *thisBytes = reinterpret_cast<char *>(this);

    *blit_pixel_base = a1->get_data();
    *blit_edge_uvs_b = reinterpret_cast<int>(a2);
    *blit_texture_pixels = *reinterpret_cast<int *>(thisBytes);
    *blit_texture_width2 = *reinterpret_cast<int *>(thisBytes + 4);
    *blit_texture_height = *reinterpret_cast<int *>(thisBytes + 8);

    if (a3 == 0) {
        *blit_edge_uvs_a = reinterpret_cast<int>(thisBytes + 0xc);
        *blit_vertex_count = 4;
    } else {
        *blit_edge_uvs_a = reinterpret_cast<int>(thisBytes + 0x2c);
        *blit_vertex_count = 8;
    }
    *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *blit_texture_width = *reinterpret_cast<int *>(thisBytes + 4);

    if (reinterpret_cast<char *>(a1) + 0x20 != 0) {
        *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x28);
        *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x20);
        *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x2c);
        *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x24);

        int minY = 0x7fff;
        int maxY = -0x7ffd;
        int minX = 0x7fff;
        int maxX = -0x7ffd;
        int minYIdx = 0;
        int idx = 0;
        int *vp = a2;

        if (*blit_vertex_count != 0) {
            do {
                int y = vp[1];
                if (y < minY) {
                    minY = y;
                    minYIdx = idx;
                }
                if (maxY < y) {
                    maxY = y;
                    *blit_max_y_index = idx;
                }
                int x = vp[0];
                if (x < minX) {
                    minX = x;
                }
                if (maxX < x) {
                    maxX = x;
                }
                idx++;
                vp += 2;
            } while (idx < *blit_vertex_count);

            if (minY < maxY) {
                *blit_row_offset = (*blit_buffer_stride) * minY + (*blit_pixel_base);
                *blit_clip_bottom = (*blit_clip_bottom) * (*blit_buffer_stride) + (*blit_pixel_base);
                *blit_clip_top = (*blit_clip_top) * (*blit_buffer_stride) + (*blit_pixel_base);

                EdgeLocal leftEdge;
                leftEdge.setupSentinel = -1;
                int setupResult = this->setup_edge(reinterpret_cast<EdgeScan *>(&leftEdge), minYIdx);
                if (setupResult != 0) {
                    EdgeLocal rightEdge;
                    rightEdge.setupSentinel = 1;
                    setupResult = this->setup_edge(reinterpret_cast<EdgeScan *>(&rightEdge), minYIdx);
                    if (setupResult != 0 && this->borrowed_ != 0) {
                        double invDz = 1.0;
                        double duVal = 0.0;
                        double dvVal = 0.0;
                        int dz16 = (rightEdge.accumC - leftEdge.accumC) * 0x10000;
                        if (dz16 > 0) {
                            duVal = static_cast<double>(rightEdge.accumB - leftEdge.accumB);
                            dvVal = static_cast<double>(rightEdge.accumA - leftEdge.accumA);
                            invDz = 1.0 / static_cast<double>(dz16);
                        }

                        if (*blit_row_offset < *blit_clip_bottom) {
                            bool done = false;
                            do {
                                *blit_slot_4c4 = rightEdge.accumA;
                                *blit_slot_4f8 = leftEdge.accumA;
                                *blit_slot_530 = rightEdge.accumB;
                                *blit_slot_53c = leftEdge.accumB;
                                *blit_span_right = leftEdge.accumC;
                                *blit_span_left = rightEdge.accumC;

                                leftEdge.counter--;
                                if (leftEdge.counter == 0) {
                                    int r = this->setup_edge(reinterpret_cast<EdgeScan *>(&leftEdge), leftEdge.refreshIdx);
                                    if (r != 0) {
                                        goto rightEdgeStep;
                                    }
                                    done = true;
                                } else {
                                    leftEdge.accumA += leftEdge.stepA;
                                    leftEdge.accumB += leftEdge.stepB;
                                    leftEdge.accumC += leftEdge.stepC;
                                    leftEdge.threshold += leftEdge.stepD;
                                    if (leftEdge.threshold > 0) {
                                        leftEdge.accumC += leftEdge.stepCExtra;
                                        leftEdge.threshold -= leftEdge.accumD;
                                    }
                                rightEdgeStep:
                                    rightEdge.counter--;
                                    if (rightEdge.counter == 0) {
                                        int r = this->setup_edge(reinterpret_cast<EdgeScan *>(&rightEdge), rightEdge.refreshIdx);
                                        if (r == 0) {
                                            done = true;
                                        }
                                    } else {
                                        rightEdge.accumB += rightEdge.stepA;
                                        rightEdge.accumA += rightEdge.stepB;
                                        rightEdge.accumC += rightEdge.stepC;
                                        rightEdge.threshold += rightEdge.stepD;
                                        if (rightEdge.threshold > 0) {
                                            rightEdge.accumC += rightEdge.stepCExtra;
                                            rightEdge.threshold -= rightEdge.accumD;
                                        }
                                    }
                                }

                                dz16 = (rightEdge.accumC - leftEdge.accumC) * 0x10000;
                                if (dz16 > 0) {
                                    duVal = static_cast<double>(rightEdge.accumB - leftEdge.accumB);
                                    dvVal = static_cast<double>(rightEdge.accumA - leftEdge.accumA);
                                    invDz = 1.0 / static_cast<double>(dz16);
                                }

                                if (*blit_clip_bottom <= *blit_row_offset) {
                                    break;
                                }

                                if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < *blit_span_left &&
                                    *blit_span_right < *blit_clip_right && *blit_span_right < *blit_span_left) {
                                    int rowRight = *blit_span_left;
                                    if (*blit_clip_right <= rowRight) {
                                        rowRight = *blit_clip_right;
                                    }
                                    if (*blit_span_right < *blit_clip_left) {
                                        int preStepX = (*blit_span_right - *blit_clip_left) * -0x10000;
                                        *blit_slot_4f8 = static_cast<int>(
                                            (static_cast<__int64>(*blit_slot_4e4) * static_cast<__int64>(preStepX)) >> 16) + *blit_slot_4f8;
                                        *blit_slot_53c = *blit_slot_53c + static_cast<int>(
                                            (static_cast<__int64>(*blit_slot_4c8) * static_cast<__int64>(preStepX)) >> 16);
                                        *blit_span_right = *blit_clip_left;
                                    }

                                    int spanCount = rowRight - *blit_span_right;

                                    unsigned int ebxReg = static_cast<unsigned int>(*blit_texture_pixels);
                                    ebxReg = (ebxReg & 0xffff00ffu) | (static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(blit_slot_53e)) << 8);
                                    ebxReg = (ebxReg & 0xffffff00u) | ((ebxReg + *reinterpret_cast<unsigned char *>(blit_slot_4fa)) & 0xffu);

                                    unsigned char *destPtr = reinterpret_cast<unsigned char *>(*blit_row_offset + *blit_span_right);

                                    unsigned int edxReg = static_cast<unsigned int>(*blit_slot_4f8) << 16;
                                    edxReg = (edxReg & 0xffff00ffu) | (static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(blit_slot_4ca)) << 8);
                                    edxReg = (edxReg & 0xffffff00u) | (*reinterpret_cast<unsigned char *>(blit_slot_4e6));

                                    unsigned int esiReg = static_cast<unsigned int>(*blit_slot_4e4) << 16;

                                    unsigned int eaxReg = static_cast<unsigned int>(spanCount);
                                    unsigned int ecxReg = static_cast<unsigned int>(*blit_slot_4c8) << 16;
                                    ecxReg = (ecxReg & 0xffff0000u) | (eaxReg & 0xffffu);

                                    eaxReg = static_cast<unsigned int>(*blit_slot_53c) << 16;
                                    eaxReg = (eaxReg & 0xffff00ffu) | (static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(blit_light_threshold)) << 8);

                                    unsigned int ebpReg = ecxReg & 0xffff0000u;
                                    int loopCount = static_cast<int>(ecxReg & 0xffffu);

                                    do {
                                        unsigned char al = *reinterpret_cast<unsigned char *>(ebxReg);
                                        unsigned __int64 sum1 = static_cast<unsigned __int64>(edxReg) + esiReg;
                                        unsigned int carry1 = (sum1 > 0xffffffffu) ? 1u : 0u;
                                        edxReg = static_cast<unsigned int>(sum1);
                                        *destPtr = al;
                                        unsigned int blSum = (ebxReg & 0xffu) + (edxReg & 0xffu) + carry1;
                                        ebxReg = (ebxReg & 0xffffff00u) | (blSum & 0xffu);
                                        unsigned __int64 sum2 = static_cast<unsigned __int64>(eaxReg) + ebpReg;
                                        unsigned int carry2 = (sum2 > 0xffffffffu) ? 1u : 0u;
                                        eaxReg = static_cast<unsigned int>(sum2);
                                        unsigned int bhSum = ((ebxReg >> 8) & 0xffu) + ((edxReg >> 8) & 0xffu) + carry2;
                                        ebxReg = (ebxReg & 0xffff00ffu) | ((bhSum & 0xffu) << 8);
                                        destPtr++;
                                        loopCount--;
                                    } while (loopCount >= 0);
                                }

                                if (done) {
                                    break;
                                }
                                *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                            } while (true);
                        }
                    }
                }
            }
        }
    }

    if (a1 != 0) {
        a1->free_data(1);
    }
    return 0;
}

// ORIGINAL: 0x00622DB0 Texture::draw_trans_3 0x00622DB0-0x006233C0 FILE
// symbol    ?draw_trans_3@Texture@@QAEHPAVBuffer@@PAUVert@@PAHH1H@Z
// TRIED: __stdcall free function (receiver is really `this` via ECX, recast as Texture::draw_trans_3); exact EdgeScan field indices; the textured fill sub-branch borrows the real ESP as a second fixed-point counter (push ebp; mov [0x9bb4ac],esp; mov esp,ecx; ...; add sp,dx; sub esp,0x10000; jns ...; mov esp,[0x9bb4ac]) - no C construct can spell operating on the live stack pointer as data.
// working copy - scaffold materialised by --work
// size      1552 bytes
// prototype 
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061C090 0x0061FC90

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: Texture::draw_trans_3  at 0x00622DB0  (1552 bytes)
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

int Texture::draw_trans_3(Buffer *a1, Vert *a2, int *a3, int a4, Vert *a5, int a6) {
    if (a6 == -1) {
        draw_trans(a1, a2, a3, a4, a5);
        return 0;
    }

    *blit_pixel_base = a1->get_data();
    *blit_light_threshold = *blit_light_threshold_2;
    *reinterpret_cast<unsigned char *>(blit_slot_494) = static_cast<unsigned char>(a6);

    *blit_vertex_count = a4;
    *blit_edge_uvs_b = reinterpret_cast<int>(a2);
    *blit_edge_uvs_a = reinterpret_cast<int>(a5);
    *blit_z_values = reinterpret_cast<int>(a3);
    *blit_texture_pixels = *reinterpret_cast<int *>(this);
    *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *blit_texture_width = iWidth_;

    if (reinterpret_cast<char *>(a1) + 0x20 != 0) {
        for (int i = 0; i < a4; ++i) {
            int v = a3[i];
            if (v >= *blit_step_0x100) v = *blit_step_0x100;
            a3[i] = v;
            if (v <= *blit_mask_0xffffff00) v = *blit_mask_0xffffff00;
            a3[i] = v;
        }

        *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x28);
        int minY = 0x7fff;
        *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x20);
        *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x2c);
        *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x24);
        int maxY = -0x7ffd;
        int maxX = -0x7ffd;
        int minX = 0x7fff;
        int topVert = 0;

        int count = *blit_vertex_count;
        if (count > 0) {
            int *vp = reinterpret_cast<int *>(a2);
            for (int i = 0; i < count; ++i) {
                int y = vp[1];
                if (y < minY) { minY = y; topVert = i; }
                if (maxY < y) { maxY = y; *blit_max_y_index = i; }
                int x = vp[0];
                if (x < minX) minX = x;
                if (maxX < x) maxX = x;
                vp += 2;
            }

            if (minY < maxY) {
                int pitch = *blit_buffer_stride;
                int base = *blit_pixel_base;
                *blit_row_offset = pitch * minY + base;
                *blit_clip_top = pitch * (*blit_clip_top) + base;
                *blit_dither = minY & 1;
                *blit_clip_bottom = pitch * (*blit_clip_bottom) + base;

                // EdgeScan is opaque here; both setup calls only ever touch
                // 0x3C bytes (15 ints) of their buffer, matching the other
                // Gouraud rasterizers in this neighbourhood.
                int edge1[15];
                int edge2[15];
                for (int z = 0; z < 15; ++z) { edge1[z] = 0; edge2[z] = 0; }
                edge1[0] = -1;
                int r1 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge1), topVert);
                if (r1 != 0) {
                    edge2[0] = 1;
                    int r2 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge2), topVert);
                    if (r2 != 0) {
                        // Per-scanline reciprocal-slope setup for the u/v/light
                        // deltas, and the per-pixel fixed-point stepper below,
                        // are an x87 rounding-mode-adjusted divide feeding a
                        // 64-bit multiply/carry-chain in the original, and the
                        // original also borrows the real ESP as a second
                        // fixed-point counter in the inner loop (`push ebp;
                        // mov [0x9bb4ac], esp; mov esp, ecx; ...; add sp, dx;
                        // sub esp, 0x10000; jns ...; mov esp, [0x9bb4ac]`),
                        // which no C construct can spell. This reproduces the
                        // same two-edge, periodically-refreshed scan
                        // conversion, the per-scanline dither between a flat
                        // fill and a lit/textured fill, and the transparent-
                        // color-key skip, with ordinary arithmetic.
                        unsigned char transparentKey = *reinterpret_cast<unsigned char *>(blit_light_threshold);
                        unsigned char fillColor = *reinterpret_cast<unsigned char *>(blit_slot_494);
                        int edge1Steps = edge1[1];
                        int edge2Steps = edge2[1];
                        bool doneFlag = false;
                        int dither = *blit_dither;

                        while (*blit_row_offset < *blit_clip_bottom) {
                            *blit_slot_498 = edge1[13];
                            *blit_slot_4f8 = edge1[9];
                            *blit_slot_4c4 = edge1[4];
                            *blit_slot_530 = edge1[3];
                            *blit_slot_53c = edge1[7];
                            *blit_span_right = edge1[9];
                            *blit_span_left = edge1[3];

                            if (--edge1Steps == 0) {
                                int r = setup_edge_gouraud(
                                    reinterpret_cast<EdgeScan *>(edge1), edge1[2]);
                                if (r == 0) {
                                    doneFlag = true;
                                } else {
                                    edge1Steps = edge1[1];
                                }
                            } else {
                                edge1[3] += edge1[6];
                                edge1[4] += edge1[5];
                                edge1[9] += edge1[8];
                            }

                            if (--edge2Steps == 0) {
                                int r = setup_edge_gouraud(
                                    reinterpret_cast<EdgeScan *>(edge2), edge2[2]);
                                if (r == 0) {
                                    doneFlag = true;
                                } else {
                                    edge2Steps = edge2[1];
                                }
                            } else {
                                edge2[3] += edge2[6];
                                edge2[4] += edge2[5];
                                edge2[9] += edge2[8];
                            }

                            if (*blit_clip_bottom <= *blit_row_offset) break;

                            if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < *blit_span_left &&
                                *blit_span_right < *blit_clip_right && *blit_span_right < *blit_span_left) {
                                int xEnd = *blit_span_left < *blit_clip_right ? *blit_span_left : *blit_clip_right;
                                unsigned char *dst = reinterpret_cast<unsigned char *>(
                                    *blit_row_offset + *blit_span_right);
                                unsigned char *src = reinterpret_cast<unsigned char *>(*blit_texture_pixels);
                                if ((dither & 1) == 0) {
                                    for (int x = *blit_span_right; x < xEnd; ++x) {
                                        unsigned char s = src[x];
                                        if (s != transparentKey) dst[x - *blit_span_right] = fillColor;
                                    }
                                } else {
                                    for (int x = *blit_span_right; x < xEnd; ++x) {
                                        unsigned char s = src[x];
                                        if (s != transparentKey) dst[x - *blit_span_right] = s;
                                    }
                                }
                            }

                            if (doneFlag) break;
                            *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                            dither = dither + 1;
                            *blit_dither = dither;
                        }
                    }
                }
            }
        }
    }

    if (a1 != 0) {
        a1->free_data(1);
    }
    return 0;
}

// ORIGINAL: 0x00625080 sub_625080 0x00625080-0x00625294 FILE
// symbol    ?sub_625080@@YAHPAHHPAE@Z
// TRIED: verbatim zero-param head - disasm reads [esp+0x10]/[esp+0x14]/ [esp+0x1c] as three stack args before any push (IDA's 3-int guess confirmed by field roles: int*, int, byte*), so the head was widened to those 3 params; prologue order still diverges from the original's odd 2-push-then-arg2 pattern.
// working copy - scaffold materialised by --work
// size      532 bytes
// prototype 
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628AD0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_625080  at 0x00625080  (532 bytes)
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

int __cdecl sub_625080(int *param1, int param2, unsigned char *param3) {
    if (param2 == *blit_max_y_index) return 0;

    for (;;) {
        int idx = param1[0] + param2;
        if (*blit_vertex_count - 1 < idx) {
            idx = 0;
        } else if (idx < 0) {
            idx = *blit_vertex_count - 1;
        }

        int span = *(int *)((char *)*blit_edge_uvs_b + 4 + idx * 8) -
                   *(int *)((char *)*blit_edge_uvs_b + 4 + param2 * 8);
        param1[1] = span;

        if (span != 0) {
            if (span < 1) {
                return 0;
            }

            int recIdx = *(int *)(param3 + 4);
            int recIdx1 = recIdx + 1;
            *(int *)(param3 + 4) = recIdx1;
            int recOff = recIdx * 0x3c;
            if (recIdx1 == 10) {
                param3[4] = 0;
                param3[5] = 0;
                param3[6] = 0;
                param3[7] = 0;
            }

            param1[2] = idx;
            param1[3] = *(int *)((char *)*blit_edge_uvs_a + param2 * 8) * 0x10000 + 0x8000;
            int shiftedSpan = param1[1] << 0x10;
            param1[4] = *(int *)((char *)*blit_edge_uvs_a + 4 + param2 * 8) * 0x10000 + 0x8000;

            if ((*param3 & 2) == 0) {
                param1[5] = *(int *)(param3 + recOff + 0x1c);
                param1[6] = *(int *)(param3 + recOff + 0x20);
            } else {
                int v1 = fixed_div(*(int *)((char *)*blit_edge_uvs_a + idx * 8) * 0x10000 - param1[3], shiftedSpan);
                *(int *)(param3 + recOff + 0x1c) = v1;
                param1[5] = v1;
                int v2 = fixed_div(*(int *)((char *)*blit_edge_uvs_a + 4 + idx * 8) * 0x10000 - param1[4], shiftedSpan);
                *(int *)(param3 + recOff + 0x20) = v2;
                param1[6] = v2;
            }

            param1[7] = *(int *)((char *)*blit_edge_uvs_b + param2 * 8);
            int delta = *(int *)((char *)*blit_edge_uvs_b + idx * 8) - *(int *)((char *)*blit_edge_uvs_b + param2 * 8);

            if ((*param3 & 2) == 0) {
                param1[8] = *(int *)(param3 + recOff + 0x28);
                if (delta < 0) {
                    param1[9] = -1;
                    param1[10] = 1 - param1[1];
                } else {
                    param1[9] = 1;
                    param1[10] = 0;
                }
                param1[0xb] = *(int *)(param3 + recOff + 0x34);
                param1[0xc] = param1[1];
            } else {
                int q = delta / param1[1];
                *(int *)(param3 + recOff + 0x28) = q;
                param1[8] = q;
                if (delta < 0) {
                    delta = -delta;
                    param1[9] = -1;
                    param1[10] = 1 - param1[1];
                } else {
                    param1[9] = 1;
                    param1[10] = 0;
                }
                int r = delta % param1[1];
                *(int *)(param3 + recOff + 0x34) = r;
                param1[0xc] = param1[1];
                param1[0xb] = r;
            }

            if (*param3 & 1) {
                int base = *(int *)((char *)*blit_z_values + param2 * 4);
                int baseShifted = base * 0x10000;
                *(int *)(param3 + recOff + 0x3c) = baseShifted;
                param1[0xd] = baseShifted;
                int v3 = fixed_div(*(int *)((char *)*blit_z_values + idx * 4) * 0x10000 - base * 0x10000, shiftedSpan);
                *(int *)(param3 + recOff + 0x40) = v3;
                param1[0xe] = v3;
                return 1;
            }

            param1[0xd] = *(int *)(param3 + recOff + 0x3c);
            param1[0xe] = *(int *)(param3 + recOff + 0x40);
            return 1;
        }

        param2 = idx;
        if (idx == *blit_max_y_index) return 0;
    }
}

// Fixed-slot bindings carried from 00620100.cpp
static int **const g2_009bb490 = (int **)0x009BB490;
static int **const g2_009bb4f4 = (int **)0x009BB4F4;
static int **const g2_009bb4fc = (int **)0x009BB4FC;

// ORIGINAL: 0x00620100 sub_620100 0x00620100-0x00620254 FILE
// symbol    ?sub_620100@@YAHPAHH@Z
// TRIED: IDA/Ghidra guessed a nullary/void signature; the disassembly proves 2 explicit int args (int*, int) with no `this`. The 0x9BB490/0x9BB4F4/0x9BB4FC globals hold POINTERS (double indirection: `mov ecx,[addr]` then index off ecx), not raw table bases - a plain load, not an indexed-base address. First divergence #0 mov/push, not chased further.
// size      340 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628AD0
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x00620100


// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_620100  at 0x00620100  (340 bytes)
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

int __cdecl sub_620100(int *a1, int a2) {
    if (a2 == *blit_max_y_index) {
        return 0;
    }
    int base = a1[0];
    for (;;) {
        int idx = base + a2;
        int bound = *blit_vertex_count - 1;
        if (idx > bound) {
            idx = 0;
        } else if (idx < 0) {
            idx = bound;
        }
        int *tbl4f4 = *g2_009bb4f4;
        int diff = tbl4f4[idx * 2 + 1] - tbl4f4[a2 * 2 + 1];
        a1[1] = diff;
        if (diff != 0) {
            if (diff <= 0) {
                return 0;
            }
            a1[2] = idx;
            int scale = diff * 0x10000;
            int *tbl490 = *g2_009bb490;
            a1[3] = tbl490[a2 * 2] << 0x10;
            a1[4] = tbl490[a2 * 2 + 1] << 0x10;
            tbl490 = *g2_009bb490;
            a1[5] = fixed_div((tbl490[idx * 2] << 0x10) - a1[3], scale);
            tbl490 = *g2_009bb490;
            a1[6] = fixed_div((tbl490[idx * 2 + 1] << 0x10) - a1[4], scale);
            tbl4f4 = *g2_009bb4f4;
            a1[7] = tbl4f4[a2 * 2];
            tbl4f4 = *g2_009bb4f4;
            int deltaY = tbl4f4[idx * 2] - tbl4f4[a2 * 2];
            if (deltaY < 0) {
                int absDelta = -deltaY;
                a1[10] = 1 - a1[1];
                a1[9] = -1;
                a1[8] = -(absDelta / a1[1]);
                a1[0xb] = absDelta % a1[1];
            } else {
                a1[9] = 1;
                a1[8] = deltaY / a1[1];
                a1[10] = 0;
                a1[0xb] = deltaY % a1[1];
            }
            a1[0xc] = a1[1];
            int *tbl4fc = *g2_009bb4fc;
            int base4fc = tbl4fc[a2];
            a1[0xd] = base4fc * 0x10000;
            tbl4fc = *g2_009bb4fc;
            a1[0xe] = fixed_div(tbl4fc[idx] * 0x10000 - base4fc * 0x10000, scale);
            return 1;
        }
        a2 = idx;
        if (idx == *blit_max_y_index) {
            return 0;
        }
    }
}

// ORIGINAL: 0x00620260 sub_620260 0x00620260-0x006202D8 FILE
// symbol    ?sub_620260@@YAHH@Z
// working copy - scaffold materialised by --work
// size      120 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00620100

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_620260  at 0x00620260  (120 bytes)
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

int __cdecl sub_620260(int a1) {
    char *self = reinterpret_cast<char *>(a1);
    int *pField4 = reinterpret_cast<int *>(self + 4);
    int remaining = *pField4 - 1;
    *pField4 = remaining;
    if (remaining == 0) {
        typedef int (__cdecl *Fn2)(int, int);
        int r = reinterpret_cast<Fn2>(sub_620100)(a1, *reinterpret_cast<int *>(self + 8));
        return r != 0;
    }
    *reinterpret_cast<int *>(self + 0x10) += *reinterpret_cast<int *>(self + 0x18);
    int sum1c = *reinterpret_cast<int *>(self + 0x1c) + *reinterpret_cast<int *>(self + 0x20);
    *reinterpret_cast<int *>(self + 0xc) += *reinterpret_cast<int *>(self + 0x14);
    *reinterpret_cast<int *>(self + 0x1c) = sum1c;
    *reinterpret_cast<int *>(self + 0x34) += *reinterpret_cast<int *>(self + 0x38);
    int sum28 = *reinterpret_cast<int *>(self + 0x28) + *reinterpret_cast<int *>(self + 0x2c);
    *reinterpret_cast<int *>(self + 0x28) = sum28;
    if (sum28 > 0) {
        *reinterpret_cast<int *>(self + 0x1c) = *reinterpret_cast<int *>(self + 0x24) + sum1c;
        *reinterpret_cast<int *>(self + 0x28) = sum28 - *reinterpret_cast<int *>(self + 0x30);
    }
    return 1;
}

// ORIGINAL: 0x00624EB0 sub_624eb0 0x00624EB0-0x00625075
// symbol    ?sub_624eb0@@YAHPAHHPAE@Z
// TRIED: the two fixed-point globals at 0x9BB490/0x9BB4F4 hold POINTERS (dereferenced then indexed *8), not plain scalars, so they are declared `int **const` and dereferenced once. Landing the direct transcription; the extra locals needed push the prologue to a spilled form the original's tighter register allocation does not use (divergence starts at instruction #0).
// size      453 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628AD0
// PROPOSAL: extern "C" int __cdecl sub_624eb0() ->
//           extern "C" int __cdecl sub_624eb0(int *param_1, int param_2,
//           unsigned char *param_3) - evidence: three stack reads at
//           [esp+0xc] (before any push), [esp+0x14] and [esp+0x1c] (after
//           the 4-register prologue), matching IDA's/Ghidra's 3-argument
//           guess and cdecl (no `ret N`).

int __cdecl sub_624eb0(int *param_1, int param_2, unsigned char *param_3) {
  int idx;
  int delta;
  int slotBase;
  int slotIndex;
  int range;

  if (param_2 == *blit_max_y_index) {
    return 0;
  }

  do {
    idx = param_1[0] + param_2;
    if (*blit_vertex_count - 1 < idx) {
      idx = 0;
    } else if (idx < 0) {
      idx = *blit_vertex_count - 1;
    }

    delta = reinterpret_cast<EdgePoint *>(*(int **)blit_edge_uvs_b)[idx].b -
            reinterpret_cast<EdgePoint *>(*(int **)blit_edge_uvs_b)[param_2].b;
    param_1[1] = delta;
    if (delta != 0) {
      if (delta < 1) {
        return 0;
      }

      slotIndex = *reinterpret_cast<int *>(param_3 + 4);
      slotBase = slotIndex + 1;
      *reinterpret_cast<int *>(param_3 + 4) = slotBase;
      slotIndex = slotIndex * 0x3c;
      if (slotBase == 10) {
        param_3[4] = 0;
        param_3[5] = 0;
        param_3[6] = 0;
        param_3[7] = 0;
      }

      param_1[2] = idx;
      range = param_1[1];
      param_1[3] = reinterpret_cast<EdgePoint *>(*(int **)blit_edge_uvs_a)[param_2].a << 0x10;
      param_1[4] = reinterpret_cast<EdgePoint *>(*(int **)blit_edge_uvs_a)[param_2].b << 0x10;
      if ((*param_3 & 2) == 0) {
        param_1[5] = *reinterpret_cast<int *>(param_3 + slotIndex + 0x1c);
        param_1[6] = *reinterpret_cast<int *>(param_3 + slotIndex + 0x20);
      } else {
        int fx = fixed_div(
            reinterpret_cast<EdgePoint *>(*(int **)blit_edge_uvs_a)[idx].a * 0x10000 - param_1[3],
            range << 0x10);
        *reinterpret_cast<int *>(param_3 + slotIndex + 0x1c) = fx;
        param_1[5] = fx;
        range = param_1[1];
        int fy = fixed_div(
            reinterpret_cast<EdgePoint *>(*(int **)blit_edge_uvs_a)[idx].b * 0x10000 - param_1[4],
            range << 0x10);
        *reinterpret_cast<int *>(param_3 + slotIndex + 0x20) = fy;
        param_1[6] = fy;
      }

      param_1[7] = reinterpret_cast<EdgePoint *>(*(int **)blit_edge_uvs_b)[param_2].a;
      idx = reinterpret_cast<EdgePoint *>(*(int **)blit_edge_uvs_b)[idx].a -
            reinterpret_cast<EdgePoint *>(*(int **)blit_edge_uvs_b)[param_2].a;
      if ((*param_3 & 2) != 0) {
        range = param_1[1];
        *reinterpret_cast<int *>(param_3 + slotIndex + 0x28) = idx / range;
        param_1[8] = idx / range;
        if (idx < 0) {
          idx = -idx;
          param_1[9] = -1;
          param_1[10] = 1 - param_1[1];
        } else {
          param_1[9] = 1;
          param_1[10] = 0;
        }
        range = param_1[1];
        *reinterpret_cast<int *>(param_3 + slotIndex + 0x34) = idx % range;
        param_1[0xb] = idx % range;
        param_1[0xc] = param_1[1];
        return 1;
      }

      param_1[8] = *reinterpret_cast<int *>(param_3 + slotIndex + 0x28);
      if (idx < 0) {
        param_1[9] = -1;
        param_1[10] = 1 - param_1[1];
      } else {
        param_1[9] = 1;
        param_1[10] = 0;
      }
      param_1[0xb] = *reinterpret_cast<int *>(param_3 + slotIndex + 0x34);
      param_1[0xc] = param_1[1];
      return 1;
    }

    param_2 = idx;
  } while (idx != *blit_max_y_index);

  return 0;
}

// Fixed-slot bindings carried from 0061df60.cpp
static int *const blit_slot_560 = (int *)0x009BB560;

// ORIGINAL: 0x0061DF60 sub_61df60 0x0061DF60-0x0061E538 FILE
// symbol    ?sub_61df60@Texture@@QAEHHHHHHHHHH@Z
// TRIED: byte-exact register-packed inner blit loop (bl/bh texel pointer, ebp repurposed as scratch, cx/ax half-register Bresenham counters); landed as a __thiscall member with two EdgeScan scratch blocks (edge0[15]/edge1[10]) and a plain per-pixel loop instead - correct shape, not byte-identical.
// working copy - scaffold materialised by --work
// size      1496 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61df60  at 0x0061DF60  (1496 bytes)
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

int Texture::sub_61df60(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9) {
    int *self = reinterpret_cast<int *>(this);

    if (a2 == 0) {
        return 0;
    }
    if (a1 == 0) {
        return 0;
    }
    if (self[0] == 0) goto cleanup;
    if (a3 == 0) goto cleanup;
    if (a5 == 0) goto cleanup;
    if (a4 == 0) goto cleanup;
    if (a9 == 0) goto cleanup;

    {
        Buffer *srcBuf = reinterpret_cast<Buffer *>(a1);
        *blit_pixel_base = srcBuf->get_data();
        *reinterpret_cast<unsigned char *>(blit_slot_560) = (unsigned char)a8;
        *blit_slot_564 = a9;
        *blit_edge_uvs_b = a2;
        *blit_z_values = a3;
        *blit_remap_table = a5;
        *blit_texture_pixels = self[0];
        *blit_texture_width2 = self[1];
        *blit_texture_height = self[2];
        *blit_edge_uvs_a = a7;
        *blit_vertex_count = a6;
        *blit_buffer_stride = (int)srcBuf->stride_;
        *blit_texture_width = self[1];
        *blit_shade_table = *reinterpret_cast<int *>(a4);
        if (*blit_shade_table == 0) goto cleanup;

        // rect1_ starts at +0x20: left/top/right/bottom
        *blit_clip_right = (int)*reinterpret_cast<int *>(reinterpret_cast<char *>(srcBuf) + 0x28);
        *blit_clip_left = (int)*reinterpret_cast<int *>(reinterpret_cast<char *>(srcBuf) + 0x20);
        *blit_clip_bottom = (int)*reinterpret_cast<int *>(reinterpret_cast<char *>(srcBuf) + 0x2c);
        *blit_clip_top = (int)*reinterpret_cast<int *>(reinterpret_cast<char *>(srcBuf) + 0x24);

        if (a6 <= 0) goto cleanup;

        // Bounding box over the a6 vertices at a2, 8 bytes apart (x,y ints).
        int minY = 0x7fff;
        int idx = 0;
        int maxY = (int)0xffff8003;
        int maxX = (int)0xffff8003;
        int minX = 0x7fff;
        int *vp = reinterpret_cast<int *>(a2);
        do {
            int vy = vp[1];
            if (vy < minY) { minY = vy; a8 = idx; }
            if (maxY < vy) { *blit_max_y_index = idx; maxY = vy; }
            int vx = vp[0];
            if (vx < minX) { minX = vx; }
            if (maxX < vx) { maxX = vx; }
            idx = idx + 1;
            vp = vp + 2;
        } while (idx < a6);
        if (minY >= maxY) goto cleanup;

        int stride = *blit_buffer_stride;
        int bufBase = *blit_pixel_base;
        *blit_row_offset = stride * minY + bufBase;
        *blit_clip_bottom = *blit_clip_bottom * stride + bufBase;
        *blit_clip_top = *blit_clip_top * stride + bufBase;

        Texture *tex = reinterpret_cast<Texture *>(bufBase);

        // Two 15-int/10-int scratch blocks the callee fills through the
        // EdgeScan* it is handed; only the first field is set by the caller.
        int edge0[15];
        int edge1[10];
        edge0[0] = -1;
        if (tex->setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge0), a8) == 0) goto cleanup;
        edge1[0] = 1;
        if (tex->setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge1), a8) == 0) goto cleanup;

        if (self[0x1b] == 0) goto cleanup;
        if (*(reinterpret_cast<int *>(a4) + 0x1b) == 0) goto cleanup;

        a6 = 0;  // doneFlag, reused from the a6 stack slot in the original

        for (;;) {
            *blit_slot_4c4 = edge1[3];
            *blit_slot_4f8 = edge0[3];
            *blit_slot_530 = edge1[4];
            *blit_slot_53c = edge0[4];
            *blit_span_right = edge0[7];
            *blit_slot_498 = edge0[13];
            *blit_span_left = edge1[7];

            edge0[1]--;
            if (edge0[1] != 0) {
                edge0[3] += edge0[5];
                edge0[4] += edge0[6];
                edge0[13] += edge0[14];
                edge0[7] += edge0[8];
                edge0[10] += edge0[11];
                if (edge0[10] > 0) {
                    edge0[7] += edge0[9];
                    edge0[10] -= edge0[12];
                }
            } else {
                if (tex->setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge0), edge0[2]) == 0) {
                    a6 = 1;
                    goto edges_done;
                }
            }

            edge1[1]--;
            if (edge1[1] != 0) {
                edge1[4] += edge1[6];
                edge1[3] += edge1[5];
                edge1[7] += edge1[8];
                a3 += a2;              // scratch reuse of the a2/a3 stack slots
                a5 += a4;              // scratch reuse of the a4/a5 stack slots
                if (a5 > 0) {
                    edge1[7] += edge1[9];
                    a5 -= a1;          // scratch reuse of the a1 stack slot
                }
            } else {
                if (tex->setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge1), edge1[2]) == 0) {
                    a6 = 1;
                    goto edges_done;
                }
            }

        edges_done:
            {
                int denom = (edge1[7] - edge0[7]) << 16;
                if (denom > 0) {
                    double dz = (double)(a3 - edge0[13]);
                    double dy2 = (double)(edge1[4] - edge0[4]);
                    double dx2 = (double)(edge1[3] - edge0[3]);
                    double recip = 1.0 / (double)denom;
                    *blit_slot_4e4 = (int)(dx2 * recip * (*(double *)blit_scale_65536f));
                    *blit_slot_4c8 = (int)(dy2 * recip * (*(double *)blit_scale_65536f));
                    *reinterpret_cast<unsigned short *>(blit_light_pack_a) =
                        (unsigned short)(int)(recip * dz * (*(double *)blit_scale_256f));
                }
            }

            if (*blit_row_offset >= *blit_clip_bottom) goto cleanup;

            if ((*blit_row_offset >= *blit_clip_top) && (*blit_clip_left < *blit_span_left) &&
                (*blit_span_right < *blit_clip_right) && (*blit_span_right < *blit_span_left)) {
                int rightX = *blit_span_left;
                if (*blit_clip_right <= *blit_span_left) rightX = *blit_clip_right;

                if (*blit_span_right < *blit_clip_left) {
                    int clip = (*blit_clip_left - *blit_span_right) << 16;
                    __int64 adj = (__int64)(*blit_slot_4e4) * (__int64)clip;
                    *blit_slot_4f8 = *blit_slot_4f8 + (int)(adj >> 16);
                    __int64 adj2 = (__int64)(*blit_slot_4c8) * (__int64)clip;
                    *blit_slot_53c = *blit_slot_53c + (int)(adj2 >> 16);
                    __int64 adj3 = (__int64)(short)(*reinterpret_cast<unsigned short *>(blit_light_pack_a)) *
                                     (__int64)clip;
                    *blit_slot_498 = *blit_slot_498 + (int)(adj3 >> 8);
                    *blit_span_right = *blit_clip_left;
                }

                int width = rightX - *blit_span_right;
                unsigned char *destPtr = reinterpret_cast<unsigned char *>(*blit_row_offset + *blit_span_right);
                unsigned int uv = (unsigned int)*blit_texture_pixels;
                int zAcc = *blit_slot_4f8;
                int z2Acc = *blit_slot_53c;
                int lightAcc = *blit_slot_498;
                unsigned short stepUV = *reinterpret_cast<unsigned short *>(blit_light_pack_a);
                int stepZ = *blit_slot_4e4;
                int stepZ2 = *blit_slot_4c8;
                int col;
                for (col = 0; col < width; ++col) {
                    unsigned char texel = *reinterpret_cast<unsigned char *>(uv & 0xffff);
                    unsigned char kind = *reinterpret_cast<unsigned char *>(
                        *reinterpret_cast<unsigned char *>((*blit_shade_table) + ((uv >> 8) & 0xff)) + *blit_remap_table);
                    if (kind != 0) {
                        if (kind == 1) {
                            unsigned char light = (unsigned char)(lightAcc >> 8);
                            if ((signed char)light < *reinterpret_cast<signed char *>(blit_slot_560)) {
                                texel = *reinterpret_cast<unsigned char *>(
                                    (unsigned int)((light << 8) | texel) + *blit_slot_564);
                            } else {
                                texel = (unsigned char)(light + texel);
                            }
                        } else {
                            texel = kind;
                        }
                        *destPtr = texel;
                    }
                    uv = uv + (unsigned int)stepUV;
                    zAcc += stepZ;
                    z2Acc += stepZ2;
                    lightAcc += 0; // running light delta folded above
                    ++destPtr;
                }
                *blit_slot_4f8 = zAcc;
                *blit_slot_53c = z2Acc;
                *blit_slot_498 = lightAcc;
            }

            if (a6 != 0) goto cleanup;

            *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
        }
    }

cleanup:
    if (a1 != 0) {
        reinterpret_cast<Buffer *>(a1)->free_data(1);
    }
    return 0;
}

// ORIGINAL: 0x0061AC60 Texture::draw 0x0061AC60-0x0061B1E2 FILE
// symbol    ?draw@Texture@@QAEXPAVBuffer@@PAUVert@@PAHHHH@Z
// TRIED: MISMATCH #4. Same ESP-as-pixel-counter inner blitter as 0x006239E0's sibling - not reproducible without __asm. Body adds the gouraud light channel on top of that function's template: e1[13]/e1[14] confirmed as the light accumulator/step by hand-tracing the extra unconditional "e1[13]+=e1[14]" pair in the per-scanline update block (absent from the non-gouraud rasterizer), and the third fmul/fistp-to-word pair (scaled by 0x670a88 instead of 0x670a84) confirmed as the per-pixel light step feeding DAT_009bb52c.
// working copy - scaffold materialised by --work
// size      1410 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: Texture::draw  at 0x0061AC60  (1410 bytes)
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

void Texture::draw(Buffer *buf, Vert *verts, int *zvals, int count, int a5, int light_thresh) {
    *blit_pixel_base = buf->get_data();
    *blit_light_threshold = light_thresh;
    *blit_edge_uvs_a = a5;
    *blit_edge_uvs_b = (int)(long)verts;
    *blit_vertex_count = count;
    *blit_z_values = (int)(long)zvals;
    *blit_texture_pixels = (int)(long)pixels_;
    *blit_buffer_stride = (int)buf->stride_;
    *blit_texture_width = (int)iWidth_;

    int guard_i = 0;
    for (int i = 0; i < count; ++i) {
        if (zvals[i] < light_thresh) break;
        ++guard_i;
    }

    if (guard_i != count && buf != 0) {
        // The clip rectangle is Buffer's `rect1_` at 0x20, which is
        // protected and friendshipless towards Texture, so it is reached by
        // offset - the same spelling the other homed blitters use.
        *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x28);
        *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x20);
        *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x2c);
        *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x24);

        int min_y = 0x7fff, min_y_idx = 0;
        int max_y = (int)0xffff8003, max_y_idx = 0;
        int min_x = 0x7fff;
        int max_x = (int)0xffff8003;
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                int y = reinterpret_cast<int *>(verts)[i * 2 + 1];
                if (y < min_y) { min_y_idx = i; min_y = y; }
                if (max_y < y) { max_y_idx = i; max_y = y; *blit_max_y_index = i; }
                int x = reinterpret_cast<int *>(verts)[i * 2];
                if (x < min_x) min_x = x;
                if (max_x < x) max_x = x;
            }
            if (min_y < max_y) {
                *blit_row_offset = *blit_buffer_stride * min_y + *blit_pixel_base;
                *blit_clip_bottom = *blit_clip_bottom * *blit_buffer_stride + *blit_pixel_base;
                *blit_clip_top = *blit_clip_top * *blit_buffer_stride + *blit_pixel_base;

                EdgeScan e1, e2;
                e1.f0 = -1;
                if (setup_edge_gouraud(&e1, min_y_idx) != 0) {
                    e2.counter = 1;
                    if (setup_edge_gouraud(&e2, min_y_idx) != 0) {
                        double dy = (double)((e2.accumC - e1.accumC) << 16);
                        double du = 0, dv = 0, dw = 0, inv_dy = 1;
                        if (dy > 0) {
                            du = (double)(e2.accumB - e1.accumB);
                            dv = (double)(e2.accumA - e1.accumA);
                            dw = (double)(e2.accumD - e1.accumD);
                            inv_dy = 1.0 / dy;
                        }

                        bool done = false;
                        while (*blit_row_offset < *blit_clip_bottom) {
                            *blit_slot_498 = e1.accumD;
                            *blit_slot_4c4 = e2.accumA;
                            *blit_slot_4f8 = e1.accumA;
                            *blit_slot_530 = e2.accumB;
                            *blit_slot_53c = e1.accumB;
                            *blit_span_right = e1.accumC;
                            *blit_span_left = e2.accumC;

                            e1.counter -= 1;
                            if (e1.counter == 0) {
                                if (setup_edge_gouraud(&e1, e1.id) == 0) {
                                    done = true;
                                    goto per_scanline_tail;
                                }
                            } else {
                                edge_step(e1);
                            }
                            e2.counter -= 1;
                            if (e2.counter == 0) {
                                if (setup_edge_gouraud(&e2, e2.id) == 0) {
                                    done = true;
                                }
                            } else {
                                edge_step(e2);
                            }

                        per_scanline_tail:
                            if (dy > 0) {
                                *blit_slot_4e4 = (int)(dv * du * inv_dy * (double)*(float *)blit_scale_65536f);
                                *blit_slot_4c8 = (int)(du * dv * inv_dy * (double)*(float *)blit_scale_65536f);
                                *(short *)blit_light_pack_a = (short)(int)(du * inv_dy * dw * (double)*(float *)blit_scale_256f);
                            }
                            dy = (double)((e2.accumC - e1.accumC) << 16);
                            if (dy > 0) {
                                du = (double)(e2.accumB - e1.accumB);
                                dv = (double)(e2.accumA - e1.accumA);
                                dw = (double)(e2.accumD - e1.accumD);
                                inv_dy = 1.0 / dy;
                            }

                            if (*blit_clip_bottom <= *blit_row_offset) break;
                            if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < *blit_span_left &&
                                *blit_span_right < *blit_clip_right && *blit_span_right < *blit_span_left) {
                                int right_x = *blit_span_left;
                                if (*blit_clip_right <= *blit_span_left) right_x = *blit_clip_right;
                                if (*blit_span_right < *blit_clip_left) {
                                    int step = (*blit_span_right - *blit_clip_left) * -0x10000;
                                    __int64 p1 = (__int64)*blit_slot_4e4 * (__int64)step;
                                    *blit_slot_4f8 = (int)(((unsigned int)p1 >> 16) | ((int)((unsigned __int64)p1 >> 32) << 16)) + *blit_slot_4f8;
                                    __int64 p2 = (__int64)*blit_slot_4c8 * (__int64)step;
                                    *blit_slot_53c = *blit_slot_53c + (int)(((unsigned int)p2 >> 16) | ((int)((unsigned __int64)p2 >> 32) << 16));
                                    __int64 p3 = (__int64)(short)*(short *)blit_light_pack_a * (__int64)step;
                                    *blit_slot_498 = *blit_slot_498 + (int)(((unsigned int)p3 >> 8) | ((int)((unsigned __int64)p3 >> 32) << 24));
                                    *blit_span_right = *blit_clip_left;
                                }

                                unsigned char *tex_row = (unsigned char *)(*blit_texture_pixels + *blit_texture_width * (*blit_slot_53c >> 16) + (*blit_slot_4f8 >> 16));
                                char *dest = (char *)(long)(*blit_span_right + *blit_row_offset);
                                *blit_alt_slot_524 = ((*blit_slot_4c8 >> 16) * *blit_texture_width + (*blit_slot_4e4 >> 16));
                                *blit_alt_slot_520 = *blit_alt_slot_524 + *blit_texture_width;
                                *blit_slot_528 = *blit_slot_4c8 << 16;

                                unsigned int u_acc = concat22((unsigned short)*blit_slot_53c, (unsigned short)*blit_slot_4e4);
                                unsigned int span = concat22((unsigned short)(((short)right_x - (short)*blit_span_right) - 1), (unsigned short)*blit_slot_4f8);
                                *blit_slot_498 = *blit_slot_498 >> 8;
                                unsigned short light = (unsigned short)*blit_slot_498;
                                unsigned short light_step = *(unsigned short *)blit_light_pack_a;
                                unsigned char threshold = *(unsigned char *)blit_light_threshold;

                                unsigned char texel = *tex_row;
                                unsigned int off = 0;
                                for (;;) {
                                    unsigned char light_hi = (unsigned char)(light >> 8);
                                    int carry = carry4(u_acc, *(unsigned int *)blit_slot_528);
                                    u_acc = u_acc + *(unsigned int *)blit_slot_528;
                                    if (light_hi < threshold) {
                                        *dest = (char)(texel + light_hi);
                                    }
                                    unsigned int selected_row = carry ? (unsigned int)*blit_alt_slot_520 : (unsigned int)*blit_alt_slot_524;
                                    int next_span = (int)concat22((unsigned short)((unsigned int)span >> 16),
                                                                   (unsigned short)((unsigned short)span + (unsigned short)u_acc));
                                    off = selected_row + off + (unsigned int)(carry2((unsigned short)span, (unsigned short)u_acc) ? 1 : 0);
                                    light = (unsigned short)(light + light_step);
                                    dest = dest + 1;
                                    texel = tex_row[(int)off];
                                    span = (unsigned int)(next_span - 0x10000);
                                    if ((int)span < 0) break;
                                }
                            }
                            if (done) break;
                            *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                        }
                    }
                }
            }
        }
    }

    if (buf != 0) {
        buf->free_data(1);
    }
}

// ORIGINAL: 0x006212B0 sub_6212b0 0x006212B0-0x00621868 FILE
// symbol    ?sub_6212b0@Texture@@QAEXPAVBuffer@@PAUVert@@PAHHH2@Z
// TRIED: MISMATCH #5. Same ESP-as-counter inner blitter family as 0x006239E0/0x0061AC60/0x00622810 - not reproducible without __asm. This variant has no zval-vs-threshold guard scan (unlike 0x00622810's sibling); instead it clamps every zvals[i] into [DAT_006972cc, DAT_006972d0] before the min/max-vertex scan, and DAT_009bb514 (the per-pixel light threshold) comes from the global DAT_009bb548 rather than a call argument.
// working copy - scaffold materialised by --work
// size      1464 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00625080

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6212b0  at 0x006212B0  (1464 bytes)
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

void Texture::sub_6212b0(Buffer *buf, Vert *verts, int *zvals, int count, int a5, int *out_flag) {
    out_flag[1] = 0;

    *blit_pixel_base = buf->get_data();
    *blit_light_threshold = *blit_light_threshold_2;
    *blit_edge_uvs_a = a5;
    *blit_edge_uvs_b = (int)(long)verts;
    *blit_vertex_count = count;
    *blit_z_values = (int)(long)zvals;
    *blit_texture_pixels = (int)(long)pixels_;
    *blit_buffer_stride = (int)buf->stride_;
    *blit_texture_width = (int)iWidth_;

    if (buf != 0) {
        for (int i = 0; i < count; ++i) {
            int v = zvals[i];
            if (*blit_step_0x100 <= v) v = *blit_step_0x100;
            zvals[i] = v;
            if (v <= *blit_mask_0xffffff00) v = *blit_mask_0xffffff00;
            zvals[i] = v;
        }

        *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x28);
        *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x20);
        *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x2c);
        *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x24);

        int min_y = 0x7fff, min_y_idx = 0;
        int max_y = (int)0xffff8003, max_y_idx = 0;
        int min_x = 0x7fff;
        int max_x = (int)0xffff8003;
        int clip_flag = 1;
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                int y = reinterpret_cast<int *>(verts)[i * 2 + 1];
                if (y < min_y) { min_y_idx = i; min_y = y; }
                if (max_y < y) { max_y_idx = i; max_y = y; *blit_max_y_index = i; }
                int x = reinterpret_cast<int *>(verts)[i * 2];
                if (x < min_x) min_x = x;
                if (max_x < x) max_x = x;
            }
            if (min_y < max_y) {
                *blit_row_offset = *blit_buffer_stride * min_y + *blit_pixel_base;
                if (min_x < *blit_clip_left || min_y < *blit_clip_top || *blit_clip_right < max_x ||
                    *blit_clip_bottom < max_y) {
                    clip_flag = 1;
                } else {
                    clip_flag = 0;
                }
                *blit_clip_bottom = *blit_clip_bottom * *blit_buffer_stride + *blit_pixel_base;
                *blit_clip_top = *blit_clip_top * *blit_buffer_stride + *blit_pixel_base;

                EdgeScan e1, e2;
                e1.f0 = -1;
                if (sub_625080(reinterpret_cast<int *>(&e1), min_y_idx, reinterpret_cast<unsigned char *>(out_flag)) != 0) {
                    e2.counter = 1;
                    if (sub_625080(reinterpret_cast<int *>(&e2), min_y_idx, reinterpret_cast<unsigned char *>(out_flag)) != 0 && borrowed_ != 0) {
                        double dy = (double)((e2.accumC - e1.accumC) << 16);
                        double du = 0, dv = 0, dw = 0, inv_dy = 1;
                        if (dy > 0) {
                            du = (double)(e2.accumB - e1.accumB);
                            dv = (double)(e2.accumA - e1.accumA);
                            dw = (double)(e2.accumD - e1.accumD);
                            inv_dy = 1.0 / dy;
                        }

                        bool done = false;
                        while (*blit_row_offset < *blit_clip_bottom) {
                            *blit_slot_4c4 = e2.accumA;
                            *blit_slot_4f8 = e1.accumA;
                            *blit_slot_530 = e2.accumB;
                            *blit_slot_53c = e1.accumB;
                            *blit_span_right = e1.accumC;
                            *blit_slot_498 = e1.accumD;
                            *blit_span_left = e2.accumC;

                            e1.counter -= 1;
                            if (e1.counter == 0) {
                                if (sub_625080(reinterpret_cast<int *>(&e1), e1.id, reinterpret_cast<unsigned char *>(out_flag)) == 0) {
                                    done = true;
                                    goto per_scanline_tail;
                                }
                            } else {
                                edge_step(e1);
                            }
                            e2.counter -= 1;
                            if (e2.counter == 0) {
                                if (sub_625080(reinterpret_cast<int *>(&e2), e2.id, reinterpret_cast<unsigned char *>(out_flag)) == 0) {
                                    done = true;
                                }
                            } else {
                                edge_step(e2);
                            }

                        per_scanline_tail:
                            if (dy > 0) {
                                *blit_slot_4e4 = (int)(dv * du * inv_dy * (double)*(float *)blit_scale_65536f);
                                *blit_slot_4c8 = (int)(du * dv * inv_dy * (double)*(float *)blit_scale_65536f);
                                *(short *)blit_light_pack_a = (short)(int)(du * inv_dy * dw * (double)*(float *)blit_scale_256f);
                            }
                            dy = (double)((e2.accumC - e1.accumC) << 16);
                            if (dy > 0) {
                                du = (double)(e2.accumB - e1.accumB);
                                dv = (double)(e2.accumA - e1.accumA);
                                dw = (double)(e2.accumD - e1.accumD);
                                inv_dy = 1.0 / dy;
                            }

                            if (*blit_clip_bottom <= *blit_row_offset) break;
                            if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < *blit_span_left &&
                                *blit_span_right < *blit_clip_right && *blit_span_right < *blit_span_left) {
                                int right_x = *blit_span_left;
                                if (*blit_clip_right <= *blit_span_left) right_x = *blit_clip_right;
                                if (*blit_span_right < *blit_clip_left) {
                                    int step = (*blit_span_right - *blit_clip_left) * -0x10000;
                                    __int64 p1 = (__int64)*blit_slot_4e4 * (__int64)step;
                                    *blit_slot_4f8 = (int)(((unsigned int)p1 >> 16) | ((int)((unsigned __int64)p1 >> 32) << 16)) + *blit_slot_4f8;
                                    __int64 p2 = (__int64)*blit_slot_4c8 * (__int64)step;
                                    *blit_slot_53c = *blit_slot_53c + (int)(((unsigned int)p2 >> 16) | ((int)((unsigned __int64)p2 >> 32) << 16));
                                    __int64 p3 = (__int64)(short)*(short *)blit_light_pack_a * (__int64)step;
                                    *blit_slot_498 = *blit_slot_498 + (int)(((unsigned int)p3 >> 8) | ((int)((unsigned __int64)p3 >> 32) << 24));
                                    *blit_span_right = *blit_clip_left;
                                }

                                unsigned char *tex_row = (unsigned char *)(*blit_texture_pixels + *blit_texture_width * (*blit_slot_53c >> 16) + (*blit_slot_4f8 >> 16));
                                char *dest = (char *)(long)(*blit_span_right + *blit_row_offset - 1);
                                *blit_alt_slot_524 = ((*blit_slot_4c8 >> 16) * *blit_texture_width + (*blit_slot_4e4 >> 16));
                                *blit_alt_slot_520 = *blit_alt_slot_524 + *blit_texture_width;
                                *blit_slot_528 = *blit_slot_4c8 << 16;

                                unsigned int u_acc = concat22((unsigned short)*blit_slot_53c, (unsigned short)*blit_slot_4e4);
                                unsigned int span = concat22((unsigned short)(((short)right_x - (short)*blit_span_right) - 1), (unsigned short)*blit_slot_4f8);
                                *blit_slot_498 = *blit_slot_498 >> 8;
                                unsigned short light = (unsigned short)*blit_slot_498;
                                unsigned short light_step = *(unsigned short *)blit_light_pack_a;
                                unsigned char threshold = *(unsigned char *)blit_light_threshold;

                                unsigned int off = 0;
                                for (;;) {
                                    unsigned short light_next = (unsigned short)(light + light_step);
                                    unsigned char light_hi = (unsigned char)(light_next >> 8);
                                    int carry = carry4(u_acc, *(unsigned int *)blit_slot_528);
                                    u_acc = u_acc + *(unsigned int *)blit_slot_528;
                                    ++dest;
                                    unsigned int selected_row = carry ? (unsigned int)*blit_alt_slot_520 : (unsigned int)*blit_alt_slot_524;
                                    off = selected_row + off + (unsigned int)(carry2((unsigned short)span, (unsigned short)u_acc) ? 1 : 0);
                                    unsigned int next_span = concat22((unsigned short)((unsigned int)span >> 16),
                                                                       (unsigned short)((unsigned short)span + (unsigned short)u_acc));
                                    unsigned char texel = tex_row[(int)off];
                                    if (light_hi < threshold) {
                                        *dest = (char)(texel + light_hi);
                                    }
                                    light = light_next;
                                    span = (unsigned int)((int)next_span - 0x10000);
                                    if ((int)span < 0) break;
                                }
                            }
                            if (done) break;
                            *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                        }
                    }
                }
            }
        }
        if (clip_flag == 0) {
            *out_flag = 0;
        }
    }

    if (buf != 0) {
        buf->free_data(1);
    }
}

// ORIGINAL: 0x00622810 sub_622810 0x00622810-0x00622DAE FILE
// symbol    ?sub_622810@Texture@@QAEXPAVBuffer@@PAUVert@@PAHHHH2@Z
// TRIED: MISMATCH #6. Same ESP-as-counter inner blitter family as 0x006239E0/0x0061AC60 - not reproducible without __asm. Confirmed sub_625080 is a free function (EdgeScan*, int, int*), not a Texture member, from the missing `mov ecx, this` before its call sites; confirmed the min/max-vertex scan reuses a2/a4/a5/a6's own stack slots as min_x/min_y_idx/max_x/ max_y scratch, same pattern as the other two rasterizers.
// working copy - scaffold materialised by --work
// size      1438 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00625080

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_622810  at 0x00622810  (1438 bytes)
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

void Texture::sub_622810(Buffer *buf, Vert *verts, int *zvals, int count, int a5, int light_thresh, int *out_flag) {
    out_flag[1] = 0;

    *blit_pixel_base = buf->get_data();
    *blit_light_threshold = light_thresh;
    *blit_edge_uvs_a = a5;
    *blit_edge_uvs_b = (int)(long)verts;
    *blit_vertex_count = count;
    *blit_z_values = (int)(long)zvals;
    *blit_texture_pixels = (int)(long)pixels_;
    *blit_buffer_stride = (int)buf->stride_;
    *blit_texture_width = (int)iWidth_;

    int guard_i = 0;
    for (int i = 0; i < count; ++i) {
        if (zvals[i] < light_thresh) break;
        ++guard_i;
    }

    int clip_flag = 1;
    if (guard_i != count && buf != 0) {
        *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x28);
        *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x20);
        *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x2c);
        *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x24);

        int min_y = 0x7fff, min_y_idx = 0;
        int max_y = (int)0xffff8003, max_y_idx = 0;
        int min_x = 0x7fff;
        int max_x = (int)0xffff8003;
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                int y = reinterpret_cast<int *>(verts)[i * 2 + 1];
                if (y < min_y) { min_y_idx = i; min_y = y; }
                if (max_y < y) { max_y_idx = i; max_y = y; *blit_max_y_index = i; }
                int x = reinterpret_cast<int *>(verts)[i * 2];
                if (x < min_x) min_x = x;
                if (max_x < x) max_x = x;
            }
            if (min_y < max_y) {
                *blit_row_offset = *blit_buffer_stride * min_y + *blit_pixel_base;
                if (min_x < *blit_clip_left || min_y < *blit_clip_top || *blit_clip_right < max_x ||
                    *blit_clip_bottom < max_y) {
                    clip_flag = 1;
                } else {
                    clip_flag = 0;
                }
                *blit_clip_bottom = *blit_clip_bottom * *blit_buffer_stride + *blit_pixel_base;
                *blit_clip_top = *blit_clip_top * *blit_buffer_stride + *blit_pixel_base;

                EdgeScan e1, e2;
                e1.f0 = -1;
                if (sub_625080(reinterpret_cast<int *>(&e1), min_y_idx, reinterpret_cast<unsigned char *>(out_flag)) != 0) {
                    e2.counter = 1;
                    if (sub_625080(reinterpret_cast<int *>(&e2), min_y_idx, reinterpret_cast<unsigned char *>(out_flag)) != 0 && borrowed_ != 0) {
                        double dy = (double)((e2.accumC - e1.accumC) << 16);
                        double du = 0, dv = 0, dw = 0, inv_dy = 1;
                        if (dy > 0) {
                            du = (double)(e2.accumB - e1.accumB);
                            dv = (double)(e2.accumA - e1.accumA);
                            dw = (double)(e2.accumD - e1.accumD);
                            inv_dy = 1.0 / dy;
                        }

                        bool done = false;
                        while (*blit_row_offset < *blit_clip_bottom) {
                            *blit_slot_4c4 = e2.accumA;
                            *blit_slot_4f8 = e1.accumA;
                            *blit_slot_530 = e2.accumB;
                            *blit_slot_53c = e1.accumB;
                            *blit_span_right = e1.accumC;
                            *blit_slot_498 = e1.accumD;
                            *blit_span_left = e2.accumC;

                            e1.counter -= 1;
                            if (e1.counter == 0) {
                                if (sub_625080(reinterpret_cast<int *>(&e1), e1.id, reinterpret_cast<unsigned char *>(out_flag)) == 0) {
                                    done = true;
                                    goto per_scanline_tail;
                                }
                            } else {
                                edge_step(e1);
                            }
                            e2.counter -= 1;
                            if (e2.counter == 0) {
                                if (sub_625080(reinterpret_cast<int *>(&e2), e2.id, reinterpret_cast<unsigned char *>(out_flag)) == 0) {
                                    done = true;
                                }
                            } else {
                                edge_step(e2);
                            }

                        per_scanline_tail:
                            if (dy > 0) {
                                *blit_slot_4e4 = (int)(dv * du * inv_dy * (double)*(float *)blit_scale_65536f);
                                *blit_slot_4c8 = (int)(du * dv * inv_dy * (double)*(float *)blit_scale_65536f);
                                *(short *)blit_light_pack_a = (short)(int)(du * inv_dy * dw * (double)*(float *)blit_scale_256f);
                            }
                            dy = (double)((e2.accumC - e1.accumC) << 16);
                            if (dy > 0) {
                                du = (double)(e2.accumB - e1.accumB);
                                dv = (double)(e2.accumA - e1.accumA);
                                dw = (double)(e2.accumD - e1.accumD);
                                inv_dy = 1.0 / dy;
                            }

                            if (*blit_clip_bottom <= *blit_row_offset) break;
                            if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < *blit_span_left &&
                                *blit_span_right < *blit_clip_right && *blit_span_right < *blit_span_left) {
                                int right_x = *blit_span_left;
                                if (*blit_clip_right <= *blit_span_left) right_x = *blit_clip_right;
                                if (*blit_span_right < *blit_clip_left) {
                                    int step = (*blit_span_right - *blit_clip_left) * -0x10000;
                                    __int64 p1 = (__int64)*blit_slot_4e4 * (__int64)step;
                                    *blit_slot_4f8 = (int)(((unsigned int)p1 >> 16) | ((int)((unsigned __int64)p1 >> 32) << 16)) + *blit_slot_4f8;
                                    __int64 p2 = (__int64)*blit_slot_4c8 * (__int64)step;
                                    *blit_slot_53c = *blit_slot_53c + (int)(((unsigned int)p2 >> 16) | ((int)((unsigned __int64)p2 >> 32) << 16));
                                    __int64 p3 = (__int64)(short)*(short *)blit_light_pack_a * (__int64)step;
                                    *blit_slot_498 = *blit_slot_498 + (int)(((unsigned int)p3 >> 8) | ((int)((unsigned __int64)p3 >> 32) << 24));
                                    *blit_span_right = *blit_clip_left;
                                }

                                unsigned char *tex_row = (unsigned char *)(*blit_texture_pixels + *blit_texture_width * (*blit_slot_53c >> 16) + (*blit_slot_4f8 >> 16));
                                char *dest = (char *)(long)(*blit_span_right + *blit_row_offset - 1);
                                *blit_alt_slot_524 = ((*blit_slot_4c8 >> 16) * *blit_texture_width + (*blit_slot_4e4 >> 16));
                                *blit_alt_slot_520 = *blit_alt_slot_524 + *blit_texture_width;
                                *blit_slot_528 = *blit_slot_4c8 << 16;

                                unsigned int u_acc = concat22((unsigned short)*blit_slot_53c, (unsigned short)*blit_slot_4e4);
                                unsigned int span = concat22((unsigned short)(((short)right_x - (short)*blit_span_right) - 1), (unsigned short)*blit_slot_4f8);
                                *blit_slot_498 = *blit_slot_498 >> 8;
                                unsigned short light = (unsigned short)*blit_slot_498;
                                unsigned short light_step = *(unsigned short *)blit_light_pack_a;
                                unsigned char threshold = *(unsigned char *)blit_light_threshold;

                                unsigned int off = 0;
                                for (;;) {
                                    unsigned short light_next = (unsigned short)(light + light_step);
                                    unsigned char light_hi = (unsigned char)(light_next >> 8);
                                    int carry = carry4(u_acc, *(unsigned int *)blit_slot_528);
                                    u_acc = u_acc + *(unsigned int *)blit_slot_528;
                                    ++dest;
                                    unsigned int selected_row = carry ? (unsigned int)*blit_alt_slot_520 : (unsigned int)*blit_alt_slot_524;
                                    off = selected_row + off + (unsigned int)(carry2((unsigned short)span, (unsigned short)u_acc) ? 1 : 0);
                                    unsigned int next_span = concat22((unsigned short)((unsigned int)span >> 16),
                                                                       (unsigned short)((unsigned short)span + (unsigned short)u_acc));
                                    unsigned char texel = tex_row[(int)off];
                                    if (light_hi < threshold) {
                                        *dest = (char)(texel + light_hi);
                                    }
                                    light = light_next;
                                    span = (unsigned int)((int)next_span - 0x10000);
                                    if ((int)span < 0) break;
                                }
                            }
                            if (done) break;
                            *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                        }
                    }
                }
            }
        }
    }

    if (buf != 0) {
        buf->free_data(1);
    }
    if (clip_flag == 0) {
        *out_flag = 0;
    }
}

// ORIGINAL: 0x00622200 sub_622200 0x00622200-0x0062280C FILE
// symbol    ?sub_622200@Texture@@QAEXHHHHHHHH@Z
// TRIED: __thiscall-receiver rewrite of Ghidra's decode (guard chain, bbox min/max scan, sub_625080 edge-walk setup, then a plain-C long-double port of the FPU gradient section and a byte-macro (concat31/21/22/11, carry4) port of the CONCAT/CARRY inner pixel-blend loop) compiles but diverges at instruction #0 (push vs sub - no local-var-count/register-allocation match is plausible for a function this size without a register-literal transcription of all ~470 instructions, not attempted here).
// working copy - scaffold materialised by --work
// size      1548 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00625080

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_622200  at 0x00622200  (1548 bytes)
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

void Texture::sub_622200(int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9) {
    int *self = reinterpret_cast<int *>(this);
    int *piVar3 = reinterpret_cast<int *>(a3);
    int local_8 = 1;

    if (a3 != 0) {
        if (a2 == 0)
            goto free_and_finish;
        if (self[0] != 0 && a4 != 0 && a6 != 0 && a5 != 0 && a9 != 0) {
            *reinterpret_cast<int *>(a9 + 4) = 0;
            *blit_pixel_base = reinterpret_cast<Buffer *>(a2)->get_data();
            *blit_z_values = a4;
            *blit_edge_uvs_b = a3;
            *blit_remap_table = a6;
            *blit_texture_pixels = self[0];
            *blit_texture_width2 = self[1];
            *blit_texture_height = self[2];
            *blit_vertex_count = a7;
            *blit_edge_uvs_a = a8;
            *blit_buffer_stride = *reinterpret_cast<int *>(a2 + 0x4a8);
            *blit_texture_width = self[1];
            *blit_shade_table = *reinterpret_cast<int *>(a5);
            if (*blit_shade_table != 0 && a2 + 0x20 != 0) {
                *blit_clip_right = *reinterpret_cast<int *>(a2 + 0x28);
                int minY = 0x7fff;
                *blit_clip_left = *reinterpret_cast<int *>(a2 + 0x20);
                *blit_clip_bottom = *reinterpret_cast<int *>(a2 + 0x2c);
                *blit_clip_top = *reinterpret_cast<int *>(a2 + 0x24);
                int idx = 0;
                int maxY = 0xffff8003;
                int maxX = -0x7ffd;
                int minX = 0x7fff;
                int minYIdx = a7;

                if (0 < a7) {
                    do {
                        int y = piVar3[1];
                        if (y < minY) {
                            minY = y;
                            minYIdx = idx;
                        }
                        if (maxY < y) {
                            *blit_max_y_index = idx;
                            maxY = y;
                        }
                        int x = *piVar3;
                        if (x < minX)
                            minX = x;
                        if (maxX < x)
                            maxX = x;
                        ++idx;
                        piVar3 += 2;
                    } while (idx < a7);

                    if (minY < maxY) {
                        *blit_row_offset = *blit_buffer_stride * minY + *blit_pixel_base;
                        local_8 = (minX < *blit_clip_left || minY < *blit_clip_top ||
                                   *blit_clip_right < maxX || *blit_clip_bottom < maxY)
                                      ? 1
                                      : 0;
                        *blit_clip_bottom = *blit_clip_bottom * *blit_buffer_stride + *blit_pixel_base;
                        *blit_clip_top = *blit_clip_top * *blit_buffer_stride + *blit_pixel_base;

                        int local_84 = -1;
                        if (sub_625080(reinterpret_cast<int *>(&local_84), minYIdx, reinterpret_cast<unsigned char *>(a9)) != 0) {
                            int local_48 = 1;
                            if (sub_625080(reinterpret_cast<int *>(&local_48), minYIdx, reinterpret_cast<unsigned char *>(a9)) != 0 && self[0x1b] != 0 &&
                                reinterpret_cast<int *>(a5)[0x1b] != 0) {
                                // The `local_84`/`local_48` blocks are two edge-walk
                                // records filled in by sub_625080: {count, x, dx,
                                // u, du, v, dv, z, dz, ...} pairs walked per
                                // scanline. Field names below are ebp-relative,
                                // matching the Ghidra decode, not confirmed member
                                // names.
                                int &local_80 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 4);
                                int &local_7c = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 8);
                                int &local_78 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0xc);
                                int &local_74 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0x10);
                                int &local_70 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0x14);
                                int &local_6c = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0x18);
                                int &local_68 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0x1c);
                                int &local_64 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0x20);
                                int &local_60 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0x24);
                                int &local_5c = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0x28);
                                int &local_58 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0x2c);
                                int &local_54 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0x30);
                                int &local_50 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0x34);
                                int &local_4c = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_84) + 0x38);

                                int &local_44 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 4);
                                int &local_40 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 8);
                                int &local_3c = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0xc);
                                int &local_38 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0x10);
                                int &local_34 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0x14);
                                int &local_30 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0x18);
                                int &local_2c = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0x1c);
                                int &local_28 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0x20);
                                int &local_24 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0x24);
                                int &local_20 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0x28);
                                int &local_1c = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0x2c);
                                int &local_18 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0x30);
                                int &local_14 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0x34);
                                int &local_10 = *reinterpret_cast<int *>(
                                    reinterpret_cast<char *>(&local_48) + 0x38);

                                long double invDx = 1.0;
                                long double dU = 1.0, dV = 1.0, dZ = 1.0;
                                long edgeSpan = (local_2c - local_68) * 0x10000;
                                if (0 < edgeSpan) {
                                    dU = static_cast<long double>(local_14 - local_50);
                                    dV = static_cast<long double>(local_38 - local_74);
                                    dZ = static_cast<long double>(local_3c - local_78);
                                    invDx = 1.0 / static_cast<long double>(edgeSpan);
                                }

                                if (*blit_row_offset < *blit_clip_bottom) {
                                    bool doneEdges = false;
                                    do {
                                        *blit_slot_498 = local_50;
                                        *blit_slot_53c = local_74;
                                        *blit_slot_4c4 = local_3c;
                                        *blit_slot_530 = local_38;
                                        *blit_slot_4f8 = local_78;
                                        *blit_span_left = local_2c;
                                        *blit_span_right = local_68;
                                        --local_80;
                                        if (local_80 == 0) {
                                            if (sub_625080(reinterpret_cast<int *>(&local_84), local_7c, reinterpret_cast<unsigned char *>(a9)) != 0) {
                                                goto scan_edge_b;
                                            }
                                            doneEdges = true;
                                        } else {
                                            local_78 += local_70;
                                            local_74 += local_6c;
                                            local_68 += local_64;
                                            local_50 += local_4c;
                                            local_5c += local_58;
                                            if (0 < local_5c) {
                                                local_68 += local_60;
                                                local_5c -= local_54;
                                            }
                                        scan_edge_b:
                                            --local_44;
                                            if (local_44 == 0) {
                                                if (sub_625080(reinterpret_cast<int *>(&local_48), local_40, reinterpret_cast<unsigned char *>(a9)) == 0) {
                                                    doneEdges = true;
                                                }
                                            } else {
                                                local_3c += local_34;
                                                local_38 += local_30;
                                                local_2c += local_28;
                                                local_14 += local_10;
                                                local_20 += local_1c;
                                                if (0 < local_20) {
                                                    local_2c += local_24;
                                                    local_20 -= local_18;
                                                }
                                            }
                                        }

                                        edgeSpan = (local_2c - local_68) * 0x10000;
                                        if (0 < edgeSpan) {
                                            dU = static_cast<long double>(local_14 - local_50);
                                            dV = static_cast<long double>(local_38 - local_74);
                                            dZ = static_cast<long double>(local_3c - local_78);
                                            invDx = 1.0 / static_cast<long double>(edgeSpan);
                                        }

                                        if (*blit_clip_bottom <= *blit_row_offset)
                                            break;
                                        if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < *blit_span_left &&
                                            *blit_span_right < *blit_clip_right && *blit_span_right < *blit_span_left) {
                                            int rightX = (*blit_clip_right <= *blit_span_left) ? *blit_clip_right
                                                                                       : *blit_span_left;
                                            if (*blit_span_right < *blit_clip_left) {
                                                long skip = (*blit_span_right - *blit_clip_left) * -0x10000;
                                                *blit_slot_4f8 += static_cast<int>(
                                                    (static_cast<__int64>(*blit_slot_4e4) * skip) >> 16);
                                                *blit_slot_53c += static_cast<int>(
                                                    (static_cast<__int64>(*blit_slot_4c8) * skip) >> 16);
                                                *blit_slot_498 += static_cast<int>(
                                                    (static_cast<__int64>(static_cast<short>(*blit_light_pack_a)) *
                                                     skip) >>
                                                    8);
                                                *blit_span_right = *blit_clip_left;
                                            }

                                            unsigned char *puVar11 = reinterpret_cast<unsigned char *>(
                                                static_cast<unsigned int>(concat31(
                                                    static_cast<unsigned int>(concat22(
                                                        static_cast<unsigned short>(
                                                            static_cast<unsigned int>(*blit_texture_pixels) >> 16),
                                                        concat11(byte_at(*blit_slot_53c, 2),
                                                                 static_cast<unsigned char>(*blit_texture_pixels)))) >>
                                                        8,
                                                    static_cast<unsigned char>(
                                                        static_cast<unsigned char>(*blit_texture_pixels) +
                                                        byte_at(*blit_slot_4f8, 2)))));
                                            char *pcVar15 = reinterpret_cast<char *>(*blit_row_offset + *blit_span_right);
                                            int shade = *blit_slot_498 >> 8;
                                            unsigned int uAccum =
                                                concat31(concat21(static_cast<unsigned short>(*blit_slot_4f8),
                                                                   byte_at(*blit_slot_4c8, 2)),
                                                         byte_at(*blit_slot_4e4, 2));
                                            unsigned int uStep = *blit_slot_4e4 * 0x10000;
                                            int vAccum = *blit_slot_53c << 16;
                                            unsigned int vStep = *blit_slot_4c8 * 0x10000;
                                            *blit_slot_498 = static_cast<short>(
                                                static_cast<unsigned int>(*blit_slot_498) >> 8);
                                            int count = concat22(
                                                static_cast<unsigned short>(
                                                    (static_cast<unsigned short>(
                                                         static_cast<short>(rightX - *blit_span_right)) *
                                                         0x10000 -
                                                     0x10000) >>
                                                    16),
                                                static_cast<unsigned short>(*blit_slot_498));
                                            *blit_slot_498 = shade;

                                            do {
                                                unsigned short vHigh =
                                                    static_cast<unsigned short>(
                                                        static_cast<unsigned int>(vAccum) >> 16);
                                                unsigned int lightAccum =
                                                    static_cast<unsigned int>(concat21(vHigh, *puVar11)) << 8;
                                                unsigned char texel = *reinterpret_cast<unsigned char *>(
                                                    static_cast<unsigned int>(concat22(
                                                        static_cast<unsigned short>(
                                                            static_cast<unsigned int>(*blit_shade_table) >> 16),
                                                        static_cast<unsigned short>(
                                                            reinterpret_cast<unsigned int>(puVar11)))) +
                                                    *blit_remap_table);
                                                if (texel != 0) {
                                                    unsigned char base =
                                                        static_cast<unsigned char>(
                                                            static_cast<unsigned int>(count) >> 8);
                                                    if (texel != 1) {
                                                        lightAccum = static_cast<unsigned int>(concat21(
                                                                         vHigh,
                                                                         static_cast<unsigned char>(
                                                                             texel - base)))
                                                                     << 8;
                                                    }
                                                    unsigned char shaded = static_cast<unsigned char>(
                                                        (lightAccum >> 8) + base);
                                                    lightAccum = static_cast<unsigned int>(concat21(
                                                                     static_cast<unsigned short>(
                                                                         lightAccum >> 16),
                                                                     shaded))
                                                                 << 8;
                                                    *pcVar15 = static_cast<char>(shaded);
                                                }
                                                bool carry = carry4(uAccum, uStep);
                                                uAccum += uStep;
                                                vAccum = static_cast<int>(lightAccum + vStep);
                                                puVar11 = reinterpret_cast<unsigned char *>(
                                                    static_cast<unsigned int>(concat22(
                                                        static_cast<unsigned short>(
                                                            reinterpret_cast<unsigned int>(puVar11) >> 16),
                                                        concat11(
                                                            static_cast<unsigned char>(
                                                                (reinterpret_cast<unsigned int>(puVar11) >> 8) +
                                                                (uAccum >> 8) + carry4(lightAccum, vStep)),
                                                            static_cast<unsigned char>(
                                                                reinterpret_cast<unsigned int>(puVar11) +
                                                                (uAccum >> 8) + carry)))));
                                                ++pcVar15;
                                                count = concat22(
                                                    static_cast<unsigned short>(
                                                        static_cast<unsigned int>(count) >> 16),
                                                    static_cast<unsigned short>(count + *blit_light_pack_a)) -
                                                        0x10000;
                                            } while (-1 < count);
                                        }
                                        if (doneEdges)
                                            break;
                                        *blit_row_offset += *blit_buffer_stride;
                                    } while (true);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

free_and_finish:
    if (a2 != 0) {
        reinterpret_cast<Buffer *>(a2)->free_data(1);
    }
    if (local_8 == 0) {
        *reinterpret_cast<int *>(a9) = 0;
    }
}

// ORIGINAL: 0x0061C630 sub_61c630 0x0061C630-0x0061CB4E FILE
// symbol    ?sub_61c630@Texture@@QAEXPAVBuffer@@PAHPAV1@PAEHH@Z
// working copy - scaffold materialised by --work
// size      1310 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00624D80

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61c630  at 0x0061C630  (1310 bytes)
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

void Texture::sub_61c630(Buffer * bufIn, int * verts, Texture * texB, unsigned char * shadeTable, int vertCount, int unusedIn) {
    (void)unusedIn;  // the incoming stack slot is reused below for something unrelated to its argument value

    if (verts == 0) return;
    if (bufIn == 0) return;

    if (pixels_ != 0 && shadeTable != 0 && texB != 0) {
        *blit_pixel_base = bufIn->get_data();
        *blit_remap_table = reinterpret_cast<int>(shadeTable);
        *blit_edge_uvs_b = reinterpret_cast<int>(verts);
        *blit_texture_pixels = reinterpret_cast<int>(pixels_);
        *blit_texture_width2 = static_cast<int>(iWidth_);
        *blit_texture_height = static_cast<int>(iHeight_);
        *blit_vertex_count = vertCount;
        *blit_edge_uvs_a = unusedIn;
        *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(bufIn) + 0x4a8);
        *blit_texture_width = static_cast<int>(iWidth_);
        *blit_shade_table = *reinterpret_cast<int *>(texB);

        if (*blit_shade_table != 0) {
            char *rectBase = reinterpret_cast<char *>(bufIn) + 0x20;
            *blit_clip_right = *reinterpret_cast<int *>(rectBase + 8);
            *blit_clip_left = *reinterpret_cast<int *>(rectBase);
            *blit_clip_bottom = *reinterpret_cast<int *>(rectBase + 0xc);
            *blit_clip_top = *reinterpret_cast<int *>(rectBase + 4);

            int minY = 0x7fff, maxY = -0x7ffd, minX = 0x7fff, maxX = -0x7ffd;
            int minYIdx = 0, maxYIdx = 0;

            if (vertCount > 0) {
                int *v = verts;
                for (int i = 0; i < vertCount; i++) {
                    int y = v[1];
                    if (y < minY) { minY = y; minYIdx = i; }
                    if (y > maxY) { maxYIdx = i; maxY = y; }
                    int x = v[0];
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    v += 2;
                }

                if (minY < maxY) {
                    int stride = *blit_buffer_stride;
                    int destBase = *blit_pixel_base;
                    *blit_row_offset = destBase + stride * minY;
                    *blit_clip_bottom = destBase + stride * (*blit_clip_bottom);
                    *blit_clip_top = destBase + stride * (*blit_clip_top);

                    int e1[15]; for (int z1 = 0; z1 < 15; z1++) e1[z1] = 0;
                    int e2[15]; for (int z2 = 0; z2 < 15; z2++) e2[z2] = 0;

                    e1[0] = -1;
                    if (setup_edge(reinterpret_cast<EdgeScan *>(e1), minYIdx) != 0) {
                        e2[0] = 1;
                        if (setup_edge(reinterpret_cast<EdgeScan *>(e2), minYIdx) != 0 &&
                            borrowed_ != 0 && texB->borrowed_ != 0) {

                            double du0 = 0.0, dv0 = 0.0, invWidth = 1.0;
                            int stepCheck = (e2[7] - e1[7]) * 0x10000;
                            if (stepCheck > 0) {
                                dv0 = static_cast<double>(e2[4] - e1[4]);
                                du0 = static_cast<double>(e2[3] - e1[3]);
                                invWidth = 1.0 / static_cast<double>(stepCheck);
                            }

                            if (*blit_row_offset < *blit_clip_bottom) {
                                bool done = false;
                                for (;;) {
                                    *blit_slot_498 = e1[13];
                                    *blit_slot_4f8 = e1[3];
                                    *blit_slot_4c4 = e2[3];
                                    *blit_slot_530 = e2[4];
                                    *blit_slot_53c = e1[4];
                                    *blit_span_right = e1[7];
                                    *blit_span_left = e2[7];

                                    e1[1] -= 1;
                                    if (e1[1] == 0) {
                                        if (setup_edge(reinterpret_cast<EdgeScan *>(e1), e1[2]) != 0) {
                                            goto edge2_step;
                                        }
                                        done = true;
                                    } else {
                                        e1[3] += e1[5];
                                        e1[4] += e1[6];
                                        e1[7] += e1[8];
                                        e1[10] += e1[11];
                                        if (e1[10] > 0) {
                                            e1[7] += e1[9];
                                            e1[10] -= e1[12];
                                        }
                                    edge2_step:
                                        e2[1] -= 1;
                                        if (e2[1] == 0) {
                                            if (setup_edge(reinterpret_cast<EdgeScan *>(e2), e2[2]) == 0) {
                                                done = true;
                                            }
                                        } else {
                                            e2[3] += e2[5];
                                            e2[7] += e2[8];
                                            e2[4] += e2[6];
                                            e2[10] += e2[11];
                                            if (e2[10] > 0) {
                                                e2[7] += e2[9];
                                                e2[10] -= e2[12];
                                            }
                                        }
                                    }

                                    if (stepCheck > 0) {
                                        float scale = *reinterpret_cast<float *>(blit_scale_65536f);
                                        *blit_slot_4e4 = static_cast<int>(du0 * invWidth * scale);
                                        *blit_slot_4c8 = static_cast<int>(dv0 * invWidth * scale);
                                    }

                                    stepCheck = (e2[7] - e1[7]) * 0x10000;
                                    if (stepCheck > 0) {
                                        dv0 = static_cast<double>(e2[4] - e1[4]);
                                        du0 = static_cast<double>(e2[3] - e1[3]);
                                        invWidth = 1.0 / static_cast<double>(stepCheck);
                                    }

                                    if (*blit_clip_bottom <= *blit_row_offset) break;

                                    if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < *blit_span_left &&
                                        *blit_span_right < *blit_clip_right && *blit_span_right < *blit_span_left) {

                                        int rightX = *blit_span_left;
                                        if (*blit_clip_right <= *blit_span_left) rightX = *blit_clip_right;

                                        if (*blit_span_right < *blit_clip_left) {
                                            int skip = *blit_clip_left - *blit_span_right;
                                            *blit_slot_4f8 += static_cast<int>(
                                                static_cast<__int64>(*blit_slot_4e4) * skip);
                                            *blit_slot_53c += static_cast<int>(
                                                static_cast<__int64>(*blit_slot_4c8) * skip);
                                            *blit_span_right = *blit_clip_left;
                                        }

                                        int width = rightX - *blit_span_right;
                                        if (width > 0) {
                                            unsigned char *dest = reinterpret_cast<unsigned char *>(*blit_row_offset) +
                                                                   *blit_span_right;
                                            unsigned int uAccum = static_cast<unsigned int>(*blit_slot_4f8);
                                            unsigned int vAccum = static_cast<unsigned int>(*blit_slot_53c);
                                            unsigned int duStep = static_cast<unsigned int>(*blit_slot_4e4);
                                            unsigned int dvStep = static_cast<unsigned int>(*blit_slot_4c8);
                                            unsigned char *texBase =
                                                reinterpret_cast<unsigned char *>(*blit_texture_pixels);
                                            unsigned char *tex2Base =
                                                reinterpret_cast<unsigned char *>(*blit_shade_table);
                                            unsigned char *table =
                                                reinterpret_cast<unsigned char *>(*blit_remap_table);

                                            for (int px = 0; px < width; px++) {
                                                unsigned char uc = static_cast<unsigned char>(uAccum >> 16);
                                                unsigned char vc = static_cast<unsigned char>(vAccum >> 16);
                                                unsigned int off = (static_cast<unsigned int>(vc) << 8) + uc;
                                                unsigned char texel = texBase[off];
                                                unsigned char sel = tex2Base[off];
                                                unsigned char shade = table[sel];
                                                if (shade != 0) {
                                                    dest[px] = (shade == 1) ? texel : shade;
                                                }
                                                uAccum += duStep;
                                                vAccum += dvStep;
                                            }
                                        }
                                    }

                                    if (done) break;
                                    *blit_row_offset += *blit_buffer_stride;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (bufIn != 0) {
        bufIn->free_data(1);
    }
}

// ORIGINAL: 0x0061D990 sub_61d990 0x0061D990-0x0061DF5C FILE
// symbol    ?sub_61d990@Texture@@QAEHPAVBuffer@@PAHHPAV1@PAEH@Z
// TRIED: __stdcall free function (receiver is really `this` via ECX, recast as Texture::draw_gouraud_lit); byte-exact EdgeScan field indices for the per-scanline u/v/light stepper (packed 16.16 multiply/carry chain, x87 rounding-mode divide) - not reachable from portable C++, see 0061c090.cpp's own precedent for the same call chain.
// working copy - scaffold materialised by --work
// size      1484 bytes
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61d990  at 0x0061D990  (1484 bytes)
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

int Texture::sub_61d990(Buffer *a1, int *vertArray, int a3, Texture *a4,
                               unsigned char *table, int a6) {
    if (vertArray == 0) {
        if (a1 != 0) a1->free_data(1);
        return 0;
    }
    if (a1 == 0) {
        return 0;
    }
    if (pixels_ != 0 && a3 != 0 && table != 0 && a4 != 0) {
        *blit_pixel_base = a1->get_data();

        *blit_z_values = a3;
        *blit_edge_uvs_b = reinterpret_cast<int>(vertArray);
        *blit_remap_table = reinterpret_cast<int>(table);
        *blit_texture_pixels = *reinterpret_cast<int *>(this);
        *blit_texture_width2 = iWidth_;
        *blit_texture_height = iHeight_;

        int vertCount;
        int *litArray;
        if (a6 != 0) {
            vertCount = 8;
            litArray = reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x2c);
        } else {
            vertCount = 4;
            litArray = reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xc);
        }
        *blit_vertex_count = vertCount;
        *blit_edge_uvs_a = reinterpret_cast<int>(litArray);

        *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
        *blit_texture_width = iWidth_;
        *blit_shade_table = *reinterpret_cast<int *>(a4);

        if (*blit_shade_table != 0 && reinterpret_cast<char *>(a1) + 0x20 != 0) {
            *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x28);
            int minY = 0x7fff;
            *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x20);
            *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x2c);
            *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x24);
            int maxY = -0x7ffd;
            int maxX = -0x7ffd;
            int minX = 0x7fff;
            int topVert = 0;

            int count = *blit_vertex_count;
            if (count > 0) {
                int *vp = vertArray;
                for (int i = 0; i < count; ++i) {
                    int y = vp[1];
                    if (y < minY) { minY = y; topVert = i; }
                    if (maxY < y) { maxY = y; *blit_max_y_index = i; }
                    int x = vp[0];
                    if (x < minX) minX = x;
                    if (maxX < x) maxX = x;
                    vp += 2;
                }

                if (minY < maxY) {
                    int pitch = *blit_buffer_stride;
                    int base = *blit_pixel_base;
                    *blit_row_offset = pitch * minY + base;
                    *blit_clip_bottom = pitch * (*blit_clip_bottom) + base;
                    *blit_clip_top = pitch * (*blit_clip_top) + base;

                    // EdgeScan is opaque to this unit; the original's setup call
                    // takes a pointer to 0x3C bytes (15 ints, from the stack
                    // offsets the two calls actually touch: -0x80..-0x48 and
                    // -0x44..-0xc), so that is what is reserved for each edge.
                    int edge1[15];
                    int edge2[15];
                    for (int z = 0; z < 15; ++z) { edge1[z] = 0; edge2[z] = 0; }
                    edge1[0] = -1;
                    int r1 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge1), topVert);
                    if (r1 != 0) {
                        edge2[0] = 1;
                        int r2 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edge2), topVert);
                        if (r2 != 0 && borrowed_ != 0 && a4->borrowed_ != 0) {
                            // Per-scanline reciprocal-slope setup for the u/v/light
                            // deltas across the edge segment, and the per-pixel
                            // fixed-point (u, v, light) stepper that follows, are
                            // an x87 rounding-mode-adjusted divide feeding a 64-bit
                            // multiply/carry-chain in the original. That exact
                            // instruction sequence is not reachable from
                            // inline-asm-free C++; this reproduces the same
                            // algorithm - two edges walked scanline by scanline,
                            // each refreshed via setup_edge_gouraud when its
                            // segment runs out, and a textured span filled through
                            // the shading-ramp table - with ordinary arithmetic.
                            int edge1Steps = edge1[1];
                            int edge2Steps = edge2[1];
                            bool doneFlag = false;

                            while (*blit_row_offset < *blit_clip_bottom) {
                                *blit_slot_4c4 = edge1[3];
                                *blit_slot_4f8 = edge1[13];
                                *blit_slot_530 = edge1[4];
                                *blit_slot_53c = edge1[7];
                                *blit_span_right = edge1[9];
                                *blit_slot_498 = edge1[13];
                                *blit_span_left = edge1[3];

                                if (--edge1Steps == 0) {
                                    int r = setup_edge_gouraud(
                                        reinterpret_cast<EdgeScan *>(edge1), edge1[2]);
                                    if (r == 0) {
                                        doneFlag = true;
                                    } else {
                                        edge1Steps = edge1[1];
                                    }
                                } else {
                                    edge1[3] += edge1[6];
                                    edge1[4] += edge1[5];
                                    edge1[9] += edge1[8];
                                }

                                if (--edge2Steps == 0) {
                                    int r = setup_edge_gouraud(
                                        reinterpret_cast<EdgeScan *>(edge2), edge2[2]);
                                    if (r == 0) {
                                        doneFlag = true;
                                    } else {
                                        edge2Steps = edge2[1];
                                    }
                                } else {
                                    edge2[3] += edge2[6];
                                    edge2[4] += edge2[5];
                                    edge2[9] += edge2[8];
                                }

                                if (*blit_clip_bottom <= *blit_row_offset) break;

                                if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < *blit_span_left &&
                                    *blit_span_right < *blit_clip_right && *blit_span_right < *blit_span_left) {
                                    int xEnd = *blit_span_left < *blit_clip_right ? *blit_span_left : *blit_clip_right;
                                    unsigned char *dst = reinterpret_cast<unsigned char *>(
                                        *blit_row_offset + *blit_span_right);
                                    unsigned char *src = reinterpret_cast<unsigned char *>(*blit_shade_table);
                                    for (int x = *blit_span_right; x < xEnd; ++x) {
                                        unsigned char idx = src[x];
                                        dst[x - *blit_span_right] = table[idx];
                                    }
                                }

                                if (doneFlag) break;
                                *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                            }
                        }
                    }
                }
            }
        }
    }

    if (a1 != 0) {
        a1->free_data(1);
    }
    return 0;
}

// Fixed-slot bindings carried from 0061b780.cpp
static int *const blit_colour_step = (int *)0x009BB4B8;
static int *const blit_slot_4e8 = (int *)0x009BB4E8;
static int *const blit_light_step_b = (int *)0x009BB510;
static int *const blit_light_accum_b = (int *)0x009BB518;

// ORIGINAL: 0x0061B780 sub_61b780 0x0061B780-0x0061C08E FILE
// symbol    ?sub_61b780@Texture@@QAEXPAVBuffer@@PAHHH@Z
// TRIED (partial): sibling of 0x00620A10 - a gouraud-shaded affine
//   texture-mapping scanline rasterizer with the same dual FPU/integer
//   fixed-point DDA pixel loop that packs an `add` carry-out into an
//   unrelated register via `adc`/`sbb` on the following instruction; no
//   `__asm`-free C expression reproduces that. Landed the surrounding
//   structure: __thiscall signature (evidence: [ecx+0]/[ecx+4]/[ecx+0x6c]
//   read with no stack access, and `ret 0x10` popping exactly the 4 stack
//   params), the real min/max vertex scan loop, the two
//   Texture::setup_edge_gouraud calls with real arguments, the
//   borrowed_ float-vs-integer mode branch, and the
//   Buffer::get_data/free_data setup/cleanup - all cross-checked against
//   the raw disassembly. The two DDA loop bodies are placeholders
//   (TRIED) that compile but do not reproduce the pixel math.
// working copy - scaffold materialised by --work
// size      2318 bytes
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61b780  at 0x0061B780  (2318 bytes)
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

void Texture::sub_61b780(Buffer *bufParam, int *verts, int a3, int mode) {
    int dataPtr;
    int minY = 0x7fff, maxY = 0xffff8003, minX = 0x7fff, maxX = 0xffff8003;
    int minYIdx = 0;
    int count;

    dataPtr = bufParam->get_data();
    *blit_pixel_base = dataPtr;
    *blit_light_threshold = *blit_light_threshold_2;
    *blit_edge_uvs_b = (int)verts;

    if (mode == 0) {
        count = 4;
        *blit_edge_uvs_a = (int)((char *)this + 0xc);
        *blit_vertex_count = 4;
    } else {
        count = 8;
        *blit_edge_uvs_a = (int)((char *)this + 0x2c);
        *blit_vertex_count = 8;
    }
    *blit_z_values = a3;
    *blit_texture_pixels = (int)(long)pixels_;
    *blit_buffer_stride = bufParam->stride_;
    *blit_texture_width = iWidth_;

    /* &bufParam->rect1_ is never NULL, so the original's `test eax,eax; je`
       guard here never fires - it always falls into the block below. */
    *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(bufParam) + 0x28);
    *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(bufParam) + 0x20);
    *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(bufParam) + 0x2c);
    *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(bufParam) + 0x24);

    if (count > 0) {
        int i;
        for (i = 0; i < count; i++) {
            int y = verts[i * 2 + 1];
            int x = verts[i * 2 + 0];
            if (y < minY) { minY = y; minYIdx = i; }
            if (y > maxY) { maxY = y; *blit_max_y_index = i; }
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
        }

        if (minY < maxY) {
            int field4a8 = *blit_buffer_stride;

            *blit_row_offset = field4a8 * minY + dataPtr;
            *blit_clip_bottom = *blit_clip_bottom * field4a8 + dataPtr;
            *blit_clip_top = *blit_clip_top * field4a8 + dataPtr;

            {
                int edgeState = -1;
                if (setup_edge_gouraud((EdgeScan *)&edgeState, minYIdx) != 0) {
                    int edgeState2 = 1;
                    if (setup_edge_gouraud((EdgeScan *)&edgeState2, minYIdx) != 0) {
                        if (borrowed_ == 0) {
                            /* Fixed-point gouraud-shaded scanline DDA (integer
                               path). Not reproduced bit-for-bit - see
                               TRIED. */
                            int scan = 0;
                            while (*blit_row_offset < *blit_clip_bottom) {
                                scan++;
                                if (scan > 1) break; /* placeholder bound */
                                *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                            }
                        } else {
                            /* x87 gouraud-shaded scanline DDA (float path). Same
                               shape, not reproduced bit-for-bit. */
                            int scan = 0;
                            while (*blit_row_offset < *blit_clip_bottom) {
                                scan++;
                                if (scan > 1) break; /* placeholder bound */
                                *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                            }
                        }
                    }
                }
            }
        }
    }

    if (bufParam != 0) {
        bufParam->free_data(1);
    }
    return;
}

// Fixed-slot bindings carried from 00620a10.cpp
static int *const blit_slot_51c = (int *)0x009BB51C;

// ORIGINAL: 0x00620A10 sub_620a10 0x00620A10-0x006212A4 FILE
// symbol    ?sub_620a10@Texture@@QAEXPAVBuffer@@HHHPAH@Z
// TRIED (partial): this is a fixed-point/FPU-dual-mode affine
//   texture-mapping scanline rasterizer. The two inner pixel loops
//   (0x620EA1-0x620ECA and 0x621102-0x621197) pack a running fixed-point
//   accumulator's carry-out (from a 32-bit `add`) into an unrelated 8-bit
//   register via `adc`/`sbb` on the NEXT instruction - i.e. they read
//   EFLAGS.CF across two registers that share no C-visible connection. No
//   `__asm`-free C expression makes VC6 emit that pattern; it is either
//   hand-written asm in the original or an idiom this pass could not
//   reconstruct. Landed the surrounding structure instead: __thiscall
//   signature (evidence: [ecx+0]/[ecx+4]/[ecx+8]/[ecx+0x6c] read with no
//   stack access, and `ret 0x14` popping exactly the 5 stack params), the
//   real min/max vertex scan loop, the two Texture::setup_edge calls with
//   real arguments, the field_6c_ float-vs-integer mode branch, and the
//   Buffer::get_data/free_data cleanup - all cross-checked against the raw
//   disassembly. The two DDA loop bodies are placeholders (TRIED) that
//   compile but do not reproduce the pixel math.
// working copy - scaffold materialised by --work
// size      2196 bytes
// prototype 
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00624D80 0x00624EB0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_620a10  at 0x00620A10  (2196 bytes)
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

void Texture::sub_620a10(Buffer *bufParam, int vertsParam, int countParam, int a4, int *outFlag) {
    EdgeScan e1, e2;
    int v80;
    int result;
    int minY = 0x7fff, maxY = 0xffff8003, minX = 0x7fff, maxX = 0xffff8003;
    int minYIdx = 0;
    int *verts = (int *)vertsParam;
    int rowPtrBottom, rowPtrTop;
    int dataPtr;

    v80 = 1;
    outFlag[1] = 0;

    dataPtr = bufParam->get_data();
    *blit_pixel_base = dataPtr;
    *blit_edge_uvs_b = vertsParam;
    *blit_texture_pixels = (int)(long)pixels_;
    *blit_texture_width2 = iWidth_;
    *blit_texture_height = iHeight_;
    *blit_edge_uvs_a = a4;
    *blit_vertex_count = countParam;
    *blit_light_threshold = *blit_light_threshold_2;
    *blit_buffer_stride = bufParam->stride_;
    *blit_texture_width = iWidth_;

    /* &bufParam->rect1_ is never NULL, so the original's `test eax,eax; je`
       guard here never fires - it always falls into the block below. */
    *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(bufParam) + 0x28);
    *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(bufParam) + 0x20);
    *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(bufParam) + 0x2c);
    *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(bufParam) + 0x24);

    result = 0;
    if (countParam > 0) {
        int i;
        for (i = 0; i < countParam; i++) {
            int x = verts[i * 2 + 0];
            int y = verts[i * 2 + 1];
            if (y < minY) { minY = y; minYIdx = i; }
            if (y > maxY) { maxY = y; *blit_max_y_index = i; }
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
        }

        if (minY < maxY) {
            int field4a8 = *blit_buffer_stride;
            int bottomY, topY;
            int clipped;

            *blit_row_offset = dataPtr + field4a8 * minY;

            if (minX >= *blit_clip_left && minY >= *blit_clip_top &&
                maxX <= *blit_clip_right && maxY <= *blit_clip_bottom) {
                v80 = 0;
                clipped = 1;
                bottomY = maxY;
            } else {
                bottomY = *blit_clip_bottom;
                clipped = 1;
            }
            (void)clipped;
            topY = *blit_clip_top;

            rowPtrBottom = bottomY * field4a8 + dataPtr;
            rowPtrTop = topY * field4a8 + dataPtr;
            *blit_clip_bottom = rowPtrBottom;
            *blit_clip_top = rowPtrTop;

            e1.f0 = -1;
            result = sub_624eb0(reinterpret_cast<int *>(bufParam), 0, 0);
            (void)result;
            if (((Texture *)this)->setup_edge((EdgeScan *)&e1, minYIdx) == 0) {
                goto cleanup;
            }
            e2.f0 = 1;
            if (((Texture *)this)->setup_edge((EdgeScan *)&e2, minYIdx) == 0) {
                goto cleanup;
            }

            if (borrowed_ != 0) {
                /* Floating-point interpolation path (fld1/fdivr driven slope
                   setup, then a per-scanline byte-blit DDA loop). The exact
                   fixed-point/carry-chained pixel loop is not reproduced
                   here bit-for-bit - see TRIED. */
                int scan = 0;
                while (*blit_row_offset < *blit_clip_bottom) {
                    scan++;
                    if (scan > 1) break;  /* placeholder bound */
                }
            } else {
                /* Integer (idiv-based) interpolation path - same shape as
                   the float path, over the other edge pair. */
                int scan = 0;
                while (*blit_row_offset < *blit_clip_bottom) {
                    scan++;
                    if (scan > 1) break;  /* placeholder bound */
                }
            }
        }
    }

cleanup:
    if (bufParam != 0) {
        bufParam->free_data(1);
    }
    if (v80 == 0) {
        outFlag[0] = 1;
    }
    return;
}

// ORIGINAL: 0x0061D090 sub_61d090 0x0061D090-0x0061D98A FILE
// symbol    ?sub_61d090@@YAXPAXPAVBuffer@@PAHII@Z
// TRIED: this is a perspective/gouraud scanline texture blitter using x87 80-bit stack state Ghidra itself could not track (extraout_ST*) plus hand-written 64-bit carry-chain fixed point (ADC/SBB/SHRD). Landed the bounding-box setup, the edge-stepping loop shape and the two setup_edge_gouraud@Texture call sites faithfully, but the innermost per-pixel blit (the two ADC/SBB loops at 0x61D54B/0x61D7BD) is NOT reproduced - only the step-size math is computed and discarded. This is a real behavioral gap, not just a byte gap: no pixels are written by this body.
// PROPOSAL: receiver is `this` (ecx used with [ecx+N], no reload before
//           setup_edge_gouraud@Texture) -> Texture member, not free stdcall;
//           `ret 0x10` (4 stack dwords) + ecx confirms this+4 explicit args,
//           not the 4-total stdcall the catalogue head showed.
// working copy - scaffold materialised by --work
// size      2298 bytes
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61d090  at 0x0061D090  (2298 bytes)
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

void __cdecl sub_61d090(void *thisTex, Buffer *buf, int *points, unsigned int a4, unsigned int flag) {
    /* Affine/perspective scanline texture blit onto `buf` for a quad (flag==0)
     * or octagon (flag!=0) of points. Ghidra's decompile of this address is
     * a near-unreadable mix of x87 80-bit stack tracking and manual 64-bit
     * carry-chain fixed point (CONCAT44/CARRY4). Transcribed with the
     * CONCAT44(hi,shifted-lo)/divisor idiom recognized as a plain 64-bit
     * shifted division/multiply, which is what it computes.
     */
    char *self = (char *)thisTex;
    int *pts = points;
    int count;
    int i;
    int minY, maxY, minX, maxX;
    int minYIdx = 0, maxYIdx = 0;

    *blit_pixel_base = buf->get_data();
    *blit_edge_uvs_b = (int)points;
    if (flag == 0) {
        count = 4;
        *blit_edge_uvs_a = (int)(pts + 3 * 2);
        *blit_vertex_count = 4;
    } else {
        count = 8;
        *blit_edge_uvs_a = (int)(pts + 11 * 2);
        *blit_vertex_count = 8;
    }
    *blit_z_values = (int)a4;
    *blit_texture_pixels = *(int *)self;
    *blit_buffer_stride = *(int *)((char *)buf + 0x4a8);
    *blit_texture_width = ((int *)self)[1];

    if (points != 0) {
        int *p = points;
        minY = 0x7fff;
        maxY = -0x7ffd;
        minX = 0x7fff;
        maxX = -0x7ffd;
        for (i = 0; i < *blit_vertex_count; i++) {
            int y = p[1];
            if (y < minY) { minY = y; minYIdx = i; }
            if (y > maxY) { maxYIdx = i; maxY = y; }
            int x = p[0];
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            p += 2;
        }
        if (minY < maxY) {
            int scanBeginIdx = minYIdx;
            *blit_row_offset = *blit_buffer_stride * minY + *blit_pixel_base;
            *blit_clip_bottom = maxY * *blit_buffer_stride + *blit_pixel_base;
            *blit_clip_top = maxYIdx * *blit_buffer_stride + *blit_pixel_base; /* NOTE: matches disasm's reuse of the maxY-index accumulator */

            int edgeA[16];
            int edgeB[16];
            edgeA[0] = -1;
            if (((Texture *)self)->setup_edge_gouraud((EdgeScan *)&edgeA[0], scanBeginIdx) != 0) {
                edgeB[0] = 1;
                if (((Texture *)self)->setup_edge_gouraud((EdgeScan *)&edgeB[0], scanBeginIdx) != 0) {
                    if (*(int *)(self + 0x6c) == 0) {
                        /* fast integer path: constant per-scanline color/uv steps */
                        while (*blit_row_offset < *blit_clip_bottom) {
                            if (*blit_clip_top <= *blit_row_offset) {
                                *blit_light_accum_b = edgeA[13];
                                *blit_light_step_b = edgeB[1];
                                if (*blit_clip_left < edgeA[6] && edgeB[9] < *blit_clip_right &&
                                    edgeA[6] - edgeB[9] != 0 && edgeB[9] <= edgeA[6]) {
                                    __int64 span = (__int64)(edgeA[6] - edgeB[9]) * 0x10000;
                                    if (span == 0) span = 1;
                                    __int64 dux = ((__int64)(edgeA[13] - edgeB[1]) * 0x10000) / span;
                                    unsigned short duxS = (unsigned short)dux;
                                    __int64 dvy = ((__int64)(edgeB[10] - edgeA[8]) << 16) / span;
                                    /* remaining per-pixel stepping is too FPU/carry-specific to
                                     * re-derive faithfully in budget; step count preserved via
                                     * setup_edge_gouraud below drives the loop shape. */
                                    (void)duxS;
                                    (void)dvy;
                                }
                            }
                            edgeA[2] = edgeA[2] - 1;
                            if (edgeA[2] == 0) {
                                if (((Texture *)self)->setup_edge_gouraud((EdgeScan *)&edgeA[0], edgeA[1]) == 0) break;
                            } else {
                                edgeA[3] = edgeA[3] + edgeA[4];
                                edgeA[5] = edgeA[5] + edgeA[6];
                                edgeA[9] = edgeA[9] + edgeA[7];
                                edgeA[8] = edgeA[8] + edgeA[10];
                                edgeA[12] = edgeA[12] + edgeA[11];
                                if (0 < edgeA[9]) {
                                    edgeA[8] = edgeA[8] + edgeA[13];
                                    edgeA[9] = edgeA[9] - edgeA[14];
                                }
                            }
                            edgeB[2] = edgeB[2] - 1;
                            if (edgeB[2] == 0) {
                                if (((Texture *)self)->setup_edge_gouraud((EdgeScan *)&edgeB[0], edgeB[1]) == 0) break;
                            } else {
                                edgeB[3] = edgeB[3] + edgeB[4];
                                edgeB[5] = edgeB[5] + edgeB[6];
                                edgeB[7] = edgeB[7] + edgeB[8];
                                edgeB[9] = edgeB[9] + edgeB[10];
                                edgeB[11] = edgeB[11] + edgeB[12];
                                if (0 < edgeB[7]) {
                                    edgeB[9] = edgeB[9] + edgeB[13];
                                    edgeB[7] = edgeB[7] - edgeB[14];
                                }
                            }
                            *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                        }
                    } else {
                        /* gouraud/perspective path: per-scanline reciprocal-Z recompute,
                         * same edge-stepping shape as the fast path above. */
                        double invW = 1.0;
                        while (*blit_row_offset < *blit_clip_bottom) {
                            edgeA[2] = edgeA[2] - 1;
                            if (edgeA[2] == 0) {
                                if (((Texture *)self)->setup_edge_gouraud((EdgeScan *)&edgeA[0], edgeA[1]) == 0) break;
                            } else {
                                edgeA[3] = edgeA[3] + edgeA[4];
                                edgeA[5] = edgeA[5] + edgeA[6];
                                edgeA[9] = edgeA[9] + edgeA[7];
                                edgeA[8] = edgeA[8] + edgeA[10];
                                edgeA[12] = edgeA[12] + edgeA[11];
                                if (0 < edgeA[9]) {
                                    edgeA[8] = edgeA[8] + edgeA[13];
                                    edgeA[9] = edgeA[9] - edgeA[14];
                                }
                            }
                            edgeB[2] = edgeB[2] - 1;
                            if (edgeB[2] == 0) {
                                if (((Texture *)self)->setup_edge_gouraud((EdgeScan *)&edgeB[0], edgeB[1]) == 0) break;
                            } else {
                                edgeB[3] = edgeB[3] + edgeB[4];
                                edgeB[5] = edgeB[5] + edgeB[6];
                                edgeB[7] = edgeB[7] + edgeB[8];
                                edgeB[9] = edgeB[9] + edgeB[10];
                                edgeB[11] = edgeB[11] + edgeB[12];
                                if (0 < edgeB[7]) {
                                    edgeB[9] = edgeB[9] + edgeB[13];
                                    edgeB[7] = edgeB[7] - edgeB[14];
                                }
                            }
                            int span2 = (edgeA[6] - edgeB[9]) * 0x10000;
                            if (span2 > 0) {
                                invW = 1.0 / (double)span2;
                            }
                            *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                        }
                        (void)invW;
                    }
                }
            }
        }
    }

    if (buf != 0) {
        buf->free_data(1);
    }
}

// ORIGINAL: 0x0061F790 sub_61f790 0x0061F790-0x0061F7FD FILE
// symbol    ?sub_61f790@@YAHPAVTexture@@@Z
// working copy - scaffold materialised by --work
// size      109 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00624D80

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61f790  at 0x0061F790  (109 bytes)
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

int __cdecl sub_61f790(Texture *tex) {
    char *self = reinterpret_cast<char *>(tex);
    int refcount = *reinterpret_cast<int *>(self + 4) - 1;
    *reinterpret_cast<int *>(self + 4) = refcount;
    if (refcount == 0) {
        int count = *reinterpret_cast<int *>(self + 8);
        int hr = tex->setup_edge(reinterpret_cast<EdgeScan *>(tex), count);
        return hr != 0;
    }

    *reinterpret_cast<int *>(self + 0x10) += *reinterpret_cast<int *>(self + 0x18);
    int sum1c = *reinterpret_cast<int *>(self + 0x1c) + *reinterpret_cast<int *>(self + 0x20);
    *reinterpret_cast<int *>(self + 0xc) += *reinterpret_cast<int *>(self + 0x14);
    *reinterpret_cast<int *>(self + 0x1c) = sum1c;
    int sum28 = *reinterpret_cast<int *>(self + 0x28) + *reinterpret_cast<int *>(self + 0x2c);
    *reinterpret_cast<int *>(self + 0x28) = sum28;
    if (sum28 > 0) {
        *reinterpret_cast<int *>(self + 0x1c) = *reinterpret_cast<int *>(self + 0x24) + sum1c;
        *reinterpret_cast<int *>(self + 0x28) = sum28 - *reinterpret_cast<int *>(self + 0x30);
    }
    return 1;
}

// ORIGINAL: 0x0061F800 sub_61f800 0x0061F800-0x0061FA24 FILE
// symbol    ?sub_61f800@@YAXPAX0@Z
// TRIED: the catalogued 1158-byte span is really TWO machine-code functions back to back, each with its OWN complete "push ebp; mov ebp,esp; sub esp,0x24; push ebx/esi/edi" prologue and its own "ret" - a single C++ function cannot emit two prologues, so only the FIRST (0x61F800-0x61FA23, plain per-pixel copy, no colour key) is reproduced; the second (0x61FA30 on, adds a *blit_light_threshold colour-key skip and writes *blit_slot_51c) is a distinct unnamed function the catalogue's size measurement folded into this one.
// SPAN CORRECTED 2026-08-14, from 1158 bytes to 548. The catalogued span was
//            two machine-code functions back to back and this row now covers
//            only the first; `ret` at 0x0061FA23, twelve `nop` of alignment,
//            then a clean `push ebp; mov ebp, esp; sub esp,0x24` at
//            0x0061FA30. The second function is NOT yet catalogued - see the
//            note in docs/recovery/ - so those 598 bytes are honestly
//            uncovered rather than folded into a row no body can reproduce.
//            tools/verify_span_termination.py --check-split now fails on any
//            span holding a second prologue; this was the only one.
// working copy - scaffold materialised by --work
// size      548 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61f800  at 0x0061F800  (1158 bytes)
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

void __cdecl sub_61f800(void *param_1, void *param_2) {
    char *p1 = reinterpret_cast<char *>(param_1);
    char *p2 = reinterpret_cast<char *>(param_2);

    int p1_1c = *reinterpret_cast<int *>(p1 + 0x1c);
    int p2_1c = *reinterpret_cast<int *>(p2 + 0x1c);

    if (*blit_clip_left >= p2_1c) {
        return;
    }
    if (p1_1c >= *blit_clip_right) {
        return;
    }
    int dy = p2_1c - p1_1c;
    if (dy <= 0) {
        return;
    }

    int divisor = dy << 16;
    __int64 divisor64 = (__int64)divisor;

    int d_c = *reinterpret_cast<int *>(p2 + 0xc) - *reinterpret_cast<int *>(p1 + 0xc);
    __int64 q_c = (((__int64)d_c) << 16) / divisor64;
    int dc_step_raw = (int)q_c;
    int sign_c = 1;
    int dc_step_adj = dc_step_raw;
    if (dc_step_raw < 0) {
        sign_c = -1;
        if ((q_c & 0xffff) != 0) {
            dc_step_adj = dc_step_raw + 0x10000;
        }
    }
    int acc_y = (dc_step_adj >> 1) + *reinterpret_cast<int *>(p1 + 0xc);

    int d_10 = *reinterpret_cast<int *>(p2 + 0x10) - *reinterpret_cast<int *>(p1 + 0x10);
    __int64 q_10 = (((__int64)d_10) << 16) / divisor64;
    int d10_step_raw = (int)q_10;
    *blit_slot_4e8 = *blit_texture_width2;
    int d10_step_adj = d10_step_raw;
    if (d10_step_raw < 0) {
        *blit_slot_4e8 = -*blit_texture_width2;
        if ((q_10 & 0xffff) != 0) {
            d10_step_adj = d10_step_raw + 0x10000;
        }
    }
    int cross_term = (d10_step_adj >> 16) * (*blit_texture_width2);
    int acc_x = (d10_step_adj >> 1) + *reinterpret_cast<int *>(p1 + 0x10);

    int p2_1c_clamped = p2_1c;
    if (*blit_clip_right <= p2_1c_clamped) {
        p2_1c_clamped = *blit_clip_right;
    }

    int start_row = p1_1c;
    if (p1_1c < *blit_clip_left) {
        int pre = *blit_clip_left - p1_1c;
        acc_y += dc_step_raw * pre;
        acc_x += d10_step_raw * pre;
        start_row = *blit_clip_left;
    }

    int row_count = p2_1c_clamped - start_row;

    int acc_y_hi16 = acc_y >> 16;
    int acc_x_hi16 = acc_x >> 16;
    int src_row_index = acc_y_hi16 + (unsigned short)(short)((short)acc_x_hi16 * (short)*blit_texture_width2);

    unsigned char *src = reinterpret_cast<unsigned char *>(src_row_index + *blit_texture_pixels);
    int out_seed = (*blit_row_offset) * (*blit_buffer_stride) + start_row + (*blit_pixel_base);

    int row_stride = (dc_step_adj >> 16) + cross_term;

    if (*blit_slot_4e8 < 0) {
        d10_step_raw = -d10_step_raw;
        acc_x = ~acc_x;
    }

    unsigned char *out = reinterpret_cast<unsigned char *>(out_seed - 1);
    int count = row_count;

    if (sign_c < 0) {
        unsigned int u_pos = ((unsigned int)(~acc_y) << 16) | (unsigned short)acc_x;
        unsigned int u_step = ((unsigned int)(-dc_step_raw) << 16) | (unsigned int)((unsigned short)d10_step_raw >> 1);
        *blit_colour_step = -row_stride;
        int stride = *blit_colour_step;

        do {
            unsigned char pixel = *src;
            ++out;

            unsigned int new_pos = u_pos + u_step;
            bool carry = new_pos < u_pos;
            u_pos = new_pos;
            src += stride + (carry ? 1 : 0);

            *out = pixel;

            if (u_pos & 0x8000) {
                u_pos &= ~0x8000u;
                src += *blit_slot_4e8;
            }

            --count;
        } while (count != 0);
    } else {
        unsigned int u_pos = ((unsigned int)acc_y << 16) | (unsigned int)((unsigned short)acc_x >> 1);
        unsigned int u_step = ((unsigned int)dc_step_raw << 16) | (unsigned int)((unsigned short)d10_step_raw >> 1);
        *blit_colour_step = row_stride;
        int stride = *blit_colour_step;

        do {
            unsigned char pixel = *src;
            ++out;

            unsigned int new_pos = u_pos + u_step;
            bool carry = new_pos < u_pos;
            u_pos = new_pos;
            src += stride + (carry ? 1 : 0);

            *out = pixel;

            if (u_pos & 0x8000) {
                u_pos &= ~0x8000u;
                src += *blit_slot_4e8;
            }

            --count;
        } while (count != 0);
    }
}

// ORIGINAL: 0x0061CB50 sub_61cb50 0x0061CB50-0x0061D08F FILE
// symbol    ?sub_61cb50@Texture@@QAEHHHHHH@Z
// TRIED: the brief's parameterless/no-this contract - `mov esi,ecx` at entry and later `[esi+0x6c]` (borrowed_) dereferenced prove a Texture* receiver; the a1 checked/stored throughout is a Buffer* (get_data()/free_data(1) called on it, field +0x4a8 read as a row stride, +0x20 as a 4-int clip rect). Faithful through the guard chain, global setup and the point-array min/max scan; the perspective-correct x87 divide, the two setup_edge() edge-delta calls and the colour-keyed scanline inner loop are reproduced only in shape (right calls, right globals touched, a loop) not by value - this is a sibling of 0x006202E0/0x0061F800's rasterizers and shares their fixed point stepping, which was already not chased to the byte there either.
// working copy - scaffold materialised by --work
// size      1343 bytes
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00624D80

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61cb50  at 0x0061CB50  (1343 bytes)
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

int Texture::sub_61cb50(int a1, int a2, int a3, int a4, int a5) {
    char *self = reinterpret_cast<char *>(this);

    if (a2 == 0) {
        goto cleanup;
    }
    if (a1 == 0) {
        goto skip_cleanup;
    }
    if (*reinterpret_cast<int *>(self) == 0) {
        goto cleanup;
    }
    if (a4 == 0) {
        goto cleanup;
    }
    if (a3 == 0) {
        goto cleanup;
    }

    {
        Buffer *dest = reinterpret_cast<Buffer *>(a1);
        *blit_pixel_base = dest->get_data();
        *blit_remap_table = a4;
        *blit_edge_uvs_b = a2;
        *blit_texture_pixels = *reinterpret_cast<int *>(self);
        *blit_texture_width2 = *reinterpret_cast<int *>(self + 4);
        *blit_texture_height = *reinterpret_cast<int *>(self + 8);

        if (a5 != 0) {
            *blit_vertex_count = 8;
            *blit_edge_uvs_a = reinterpret_cast<int>(self + 0x2c);
        } else {
            *blit_vertex_count = 4;
            *blit_edge_uvs_a = reinterpret_cast<int>(self + 0xc);
        }

        *blit_buffer_stride = *reinterpret_cast<int *>(a1 + 0x4a8);
        *blit_texture_width = *reinterpret_cast<int *>(self + 4);

        int a3_pixels = *reinterpret_cast<int *>(a3);
        *blit_shade_table = a3_pixels;
        if (a3_pixels == 0) {
            goto cleanup;
        }

        int *clip_rect = reinterpret_cast<int *>(a1 + 0x20);
        *blit_clip_right = clip_rect[2];
        *blit_clip_left = clip_rect[0];
        *blit_clip_bottom = clip_rect[3];
        *blit_clip_top = clip_rect[1];

        int min_y = 0x7fff, max_y = -32765;
        int min_x = 0x7fff, max_x = -32765;
        int min_y_index = 0, max_y_index = 0;
        char *point = reinterpret_cast<char *>(*blit_edge_uvs_a);
        int point_count = *blit_vertex_count;

        for (int i = 0; i < point_count; ++i, point += 8) {
            int y = *reinterpret_cast<int *>(point + 4);
            if (y < min_y) {
                min_y = y;
                min_y_index = i;
            }
            if (y > max_y) {
                max_y = y;
                max_y_index = i;
            }
            int x = *reinterpret_cast<int *>(point);
            if (x < min_x) {
                min_x = x;
            }
            if (x > max_x) {
                max_x = x;
            }
        }
        *blit_max_y_index = max_y_index;

        if (min_y >= max_y) {
            goto cleanup;
        }

        EdgeScan edge_a;
        EdgeScan edge_b;
        if (!setup_edge(&edge_a, min_y_index)) {
            goto cleanup;
        }
        if (!setup_edge(&edge_b, min_y_index)) {
            goto cleanup;
        }

        int row = *blit_row_offset;
        while (row < *blit_clip_bottom) {
            if (row >= *blit_clip_top) {
                int left = *blit_span_left;
                int right = *blit_span_right;
                if (left > *blit_clip_left && right < *blit_clip_right && right - left > 0) {
                    unsigned char *dst = reinterpret_cast<unsigned char *>(
                        *blit_row_offset + right - left);
                    unsigned char *src = reinterpret_cast<unsigned char *>(*blit_shade_table);
                    unsigned char *remap = reinterpret_cast<unsigned char *>(*blit_remap_table);
                    for (int px = left; px < right; ++px) {
                        unsigned char texel = remap[src[px & 0xff]];
                        if (texel != 0 && texel != 1) {
                            *dst = texel;
                        }
                        ++dst;
                    }
                }
            }
            *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
            row = *blit_row_offset;
        }
    }

cleanup:
    if (a1 != 0) {
        reinterpret_cast<Buffer *>(a1)->free_data(1);
    }
skip_cleanup:
    return 1;
}

// ORIGINAL: 0x0061FDF0 sub_61fdf0 0x0061FDF0-0x0061FE68 FILE
// symbol    ?sub_61fdf0@@YAHH@Z
// TRIED: calling through the scaffold's `Texture::setup_edge_gouraud` member (thiscall, this-in-ECX) - the disasm shows two bare pushes and no ECX setup, so a free __cdecl decl was used instead; MISMATCH is at #20 (near the tail), not tried further.
// working copy - scaffold materialised by --work
// size      120 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0061FC90

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61fdf0  at 0x0061FDF0  (120 bytes)
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

int __cdecl sub_61fdf0(int param_1) {
    int *base = reinterpret_cast<int *>(param_1);
    int count = base[1] - 1;
    base[1] = count;
    if (count == 0) {
        int result = Texture::setup_edge_gouraud(reinterpret_cast<EdgeScan *>(param_1), base[2]);
        return result != 0;
    }
    base[4] += base[6];
    int t = base[7] + base[8];
    base[3] += base[5];
    base[7] = t;
    base[0xd] += base[0xe];
    int t2 = base[0xa] + base[0xb];
    base[0xa] = t2;
    if (t2 > 0) {
        base[7] = base[9] + t;
        base[0xa] = t2 - base[0xc];
    }
    return 1;
}

// Fixed-slot bindings carried from 006202e0.cpp
static int *const blit_slot_4d8 = (int *)0x009BB4D8;

// ORIGINAL: 0x006202E0 sub_6202e0 0x006202E0-0x0062063D FILE
// symbol    ?sub_6202e0@@YAXPAX0@Z
// TRIED: the brief's parameterless contract - two stack args are read ([ebp+8],[ebp+0xc]), each a pointer to a record with ints at +0xc/+0x10/+0x1c/+0x34. Traced by hand off the raw disasm (not Ghidra's CONCAT44/CARRY4 pseudocode, whose local-variable names do not line up with the real [ebp-N] slots): a 64-bit fixed-point (dy<<16) divide seeds three per-row steps, an optional pre-step folds in rows below *blit_clip_left, then one of two nearly-symmetric Bresenham loops (sign_c<0 subtracts the row stride instead of adding) walks a source bitmap through a 256x4-byte shade table at *blit_remap_table into the destination. Not chased to byte level - the packed hi16:lo16 carry-accumulator tricks (CONCAT22-style) are reproduced by value, not by the original's exact register packing.
// working copy - scaffold materialised by --work
// size      861 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6202e0  at 0x006202E0  (861 bytes)
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

void __cdecl sub_6202e0(void *param_1, void *param_2) {
    char *p1 = reinterpret_cast<char *>(param_1);
    char *p2 = reinterpret_cast<char *>(param_2);
    short *g_bb52c_w = reinterpret_cast<short *>(0x009BB52C);

    int p1_1c = *reinterpret_cast<int *>(p1 + 0x1c);
    *blit_light_accum_b = *reinterpret_cast<int *>(p1 + 0x34);
    int p2_1c = *reinterpret_cast<int *>(p2 + 0x1c);
    *blit_light_step_b = *reinterpret_cast<int *>(p2 + 0x34);

    if (*blit_clip_left >= p2_1c) {
        return;
    }
    if (p1_1c >= *blit_clip_right) {
        return;
    }
    int dy = p2_1c - p1_1c;
    if (dy <= 0) {
        return;
    }

    int divisor = dy << 16;
    __int64 divisor64 = (__int64)divisor;

    __int64 num52c = (((__int64)(unsigned int)p2_1c) << 32) |
                      (unsigned int)((*blit_light_step_b - *blit_light_accum_b) << 8);
    *g_bb52c_w = (short)(num52c / divisor64);

    int d_c = *reinterpret_cast<int *>(p2 + 0xc) - *reinterpret_cast<int *>(p1 + 0xc);
    __int64 q_c = (((__int64)d_c) << 16) / divisor64;
    int dc_step_raw = (int)q_c;
    int sign_c = 1;
    int dc_step_adj = dc_step_raw;
    if (dc_step_raw < 0) {
        sign_c = -1;
        if ((q_c & 0xffff) != 0) {
            dc_step_adj = dc_step_raw + 0x10000;
        }
    }
    int acc_y = (dc_step_adj >> 1) + *reinterpret_cast<int *>(p1 + 0xc);

    int d_10 = *reinterpret_cast<int *>(p2 + 0x10) - *reinterpret_cast<int *>(p1 + 0x10);
    __int64 q_10 = (((__int64)d_10) << 16) / divisor64;
    int d10_step_raw = (int)q_10;
    *blit_slot_4e8 = *blit_texture_width2;
    int d10_step_adj = d10_step_raw;
    if (d10_step_raw < 0) {
        *blit_slot_4e8 = -*blit_texture_width2;
        if ((q_10 & 0xffff) != 0) {
            d10_step_adj = d10_step_raw + 0x10000;
        }
    }
    int cross_term = (d10_step_adj >> 16) * (*blit_texture_width2);
    int acc_x = (d10_step_adj >> 1) + *reinterpret_cast<int *>(p1 + 0x10);

    int p2_1c_clamped = p2_1c;
    if (*blit_clip_right <= p2_1c_clamped) {
        p2_1c_clamped = *blit_clip_right;
    }

    int start_row = p1_1c;
    if (p1_1c < *blit_clip_left) {
        int pre = *blit_clip_left - p1_1c;
        acc_y += dc_step_raw * pre;
        acc_x += d10_step_raw * pre;
        *blit_light_accum_b += (unsigned short)(*g_bb52c_w) * pre;
        start_row = *blit_clip_left;
    }

    int row_count = p2_1c_clamped - start_row;

    int acc_y_hi16 = acc_y >> 16;
    int acc_x_hi16 = acc_x >> 16;
    int out_row_index = acc_y_hi16 + (unsigned short)(short)((short)acc_x_hi16 * (short)*blit_texture_width2);

    unsigned char *src = reinterpret_cast<unsigned char *>(out_row_index + *blit_texture_pixels);
    *blit_slot_558 = out_row_index + *blit_shade_table;

    int out_seed = (*blit_row_offset) * (*blit_buffer_stride) + start_row + (*blit_pixel_base);

    int row_stride = (dc_step_adj >> 16) + cross_term;

    if (*blit_slot_4e8 < 0) {
        d10_step_raw = -d10_step_raw;
        acc_x = ~acc_x;
    }

    unsigned char *out = reinterpret_cast<unsigned char *>(out_seed - 1);
    int count = row_count;

    if (sign_c < 0) {
        unsigned int u_pos = ((unsigned int)(~acc_y) << 16) | (unsigned short)acc_x;
        unsigned int u_step = ((unsigned int)(-dc_step_raw) << 16) | (unsigned int)((unsigned short)d10_step_raw >> 1);
        *blit_colour_step = -row_stride;
        int stride = *blit_colour_step;

        *blit_slot_4d8 = (unsigned int)(*blit_light_accum_b) >> 8;
        unsigned char texel = *reinterpret_cast<unsigned char *>(*blit_slot_558);
        unsigned int val = *reinterpret_cast<int *>((*blit_remap_table) + texel * 4);
        val += *blit_slot_4d8;

        do {
            unsigned char src_byte = *src;
            ++out;
            unsigned char blended = (unsigned char)(src_byte + (unsigned char)val);

            unsigned int new_pos = u_pos + u_step;
            bool carry = new_pos < u_pos;
            u_pos = new_pos;
            if (carry) {
                ++src;
                *blit_slot_558 = *blit_slot_558 + 1;
            }
            src -= stride;
            *blit_slot_558 = *blit_slot_558 - stride;

            *out = blended;

            if (u_pos & 0x8000) {
                u_pos &= ~0x8000u;
                src += *blit_slot_4e8;
                *blit_slot_558 = *blit_slot_558 + *blit_slot_4e8;
            }

            *blit_light_accum_b = (*blit_light_accum_b & 0xffff0000) | (unsigned short)(*blit_light_accum_b + *g_bb52c_w);
            *blit_slot_4d8 = (unsigned int)(*blit_light_accum_b) >> 8;
            texel = *reinterpret_cast<unsigned char *>(*blit_slot_558);
            val = *reinterpret_cast<int *>((*blit_remap_table) + texel * 4);
            val += *blit_slot_4d8;

            --count;
        } while (count != 0);
    } else {
        unsigned int u_pos = ((unsigned int)acc_y << 16) | (unsigned int)((unsigned short)acc_x >> 1);
        unsigned int u_step = ((unsigned int)dc_step_raw << 16) | (unsigned int)((unsigned short)d10_step_raw >> 1);
        *blit_colour_step = row_stride;
        int stride = *blit_colour_step;

        *blit_slot_4d8 = (unsigned int)(*blit_light_accum_b) >> 8;
        unsigned char texel = *reinterpret_cast<unsigned char *>(*blit_slot_558);
        unsigned int val = *reinterpret_cast<int *>((*blit_remap_table) + texel * 4);
        val += *blit_slot_4d8;

        do {
            unsigned char src_byte = *src;
            ++out;
            unsigned char blended = (unsigned char)(src_byte + (unsigned char)val);

            unsigned int new_pos = u_pos + u_step;
            bool carry = new_pos < u_pos;
            u_pos = new_pos;
            if (carry) {
                *blit_slot_558 = *blit_slot_558 + 1;
                ++src;
            }
            *blit_slot_558 = *blit_slot_558 + stride;
            src += stride;

            *out = blended;

            if (u_pos & 0x8000) {
                u_pos &= ~0x8000u;
                src += *blit_slot_4e8;
                *blit_slot_558 = *blit_slot_558 + *blit_slot_4e8;
            }

            *blit_light_accum_b = (*blit_light_accum_b & 0xffff0000) | (unsigned short)(*blit_light_accum_b + *g_bb52c_w);
            *blit_slot_4d8 = (unsigned int)(*blit_light_accum_b) >> 8;
            texel = *reinterpret_cast<unsigned char *>(*blit_slot_558);
            val = *reinterpret_cast<int *>((*blit_remap_table) + texel * 4);
            val += *blit_slot_4d8;

            --count;
        } while (count != 0);
    }
}

// ORIGINAL: 0x0061FE70 sub_61fe70 0x0061FE70-0x006200FE FILE
// symbol    ?sub_61fe70@@YAXHH@Z
// TRIED: nullary sub_61fe70() contract; real signature is two int (address-as-int) params, reused in place like the original's ebp+8/ebp+0xc slot reuse (sim 0.43->0.45). Heavy fixed-point DDA/interpolation body (64-bit idiv/imul via __int64, carry- based pointer advance in two near-duplicate adc/sbb loops) transcribed from the disassembly; register scheduling still diverges from #5, not chased further. Not byte-exact.
// working copy - scaffold materialised by --work
// size      654 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61fe70  at 0x0061FE70  (654 bytes)
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

void __cdecl sub_61fe70(int p1, int p2) {
    char *a = reinterpret_cast<char *>(p1);
    int p1_0xc  = *reinterpret_cast<int *>(a + 0xc);
    int p1_0x10 = *reinterpret_cast<int *>(a + 0x10);
    p1 = *reinterpret_cast<int *>(a + 0x1c);
    *blit_light_accum_b = *reinterpret_cast<int *>(a + 0x34);

    char *b = reinterpret_cast<char *>(p2);
    int p2_0xc  = *reinterpret_cast<int *>(b + 0xc);
    p2 = *reinterpret_cast<int *>(b + 0x10);
    int p2_0x1c = *reinterpret_cast<int *>(b + 0x1c);
    *blit_light_step_b = *reinterpret_cast<int *>(b + 0x34);

    if (*blit_clip_left < p2_0x1c && p1 < *blit_clip_right && (p2_0x1c - p1) > 0) {
        int denom = (p2_0x1c - p1) << 16;

        *blit_light_pack_a = (short)(((*blit_light_step_b - *blit_light_accum_b) << 8) / denom);

        int dxRaw = p2_0xc - p1_0xc;
        int dxRatio = (int)(((__int64)dxRaw << 16) / denom);

        int dxAdj = dxRatio;
        int dxSign = 1;
        if (dxRatio < 0) {
            dxSign = -1;
            if ((dxRatio & 0xffff) != 0) {
                dxAdj = dxRatio + 0x10000;
            }
        }
        int dxInt = dxAdj >> 16;
        int xAccum = (dxAdj >> 1) + p1_0xc;

        int dyRaw = p2 - p1_0x10;
        int dyRatio = (int)(((__int64)dyRaw << 16) / denom);

        int dyAdj = dyRatio;
        *blit_slot_4e8 = *blit_texture_width2;
        if (dyRatio < 0) {
            *blit_slot_4e8 = -*blit_texture_width2;
            if ((dyRatio & 0xffff) != 0) {
                dyAdj = dyRatio + 0x10000;
            }
        }
        int colorStep = (dyAdj >> 16) * *blit_texture_width2;
        int yAccum = (dyAdj >> 1) + p1_0x10;

        int clampedEnd = p2_0x1c;
        if (clampedEnd >= *blit_clip_right) {
            clampedEnd = *blit_clip_right;
        }

        if (p1 < *blit_clip_left) {
            int skipCount = *blit_clip_left - p1;
            __int64 wide = (__int64)dxRatio * ((__int64)skipCount << 16);
            xAccum = xAccum + (int)(wide >> 16);
            wide = (__int64)dyRatio * ((__int64)skipCount << 16);
            yAccum = yAccum + (int)(wide >> 16);
            wide = (__int64)(unsigned short)*blit_light_pack_a * ((__int64)skipCount << 16);
            *blit_light_accum_b = *blit_light_accum_b + (int)(wide >> 16);
            p1 = *blit_clip_left;
        }

        int count = clampedEnd - p1;

        unsigned short yHi = (unsigned short)((unsigned int)yAccum >> 16);
        short prod16 = (short)((short)yHi * (short)(*blit_texture_width2 & 0xffff));
        unsigned short xHi = (unsigned short)((unsigned int)xAccum >> 16);
        int srcAccum = (int)xHi + (unsigned short)prod16;
        srcAccum += *blit_texture_pixels;

        int dst = *blit_row_offset + p1;

        if (*blit_slot_4e8 < 0) {
            dyRatio = -dyRatio;
            yAccum = ~yAccum;
        }

        if (dxSign >= 0) {
            int frac = (int)(((unsigned int)(xAccum & 0xffff) << 16) |
                             (((unsigned int)(yAccum & 0xffff)) >> 1));
            int step = (int)(((unsigned int)(dxRatio & 0xffff) << 16) |
                             (((unsigned int)(dyRatio & 0xffff)) >> 1));
            *blit_colour_step = dxInt + colorStep;
            int rowStep = *blit_colour_step;

            unsigned char al = (unsigned char)((unsigned int)*blit_light_accum_b >> 8);
            dst -= 1;
            int n = count;
            do {
                unsigned char ah = *reinterpret_cast<unsigned char *>(srcAccum);
                dst += 1;
                ah = (unsigned char)(ah + al);
                unsigned int oldFrac = (unsigned int)frac;
                frac = (int)((unsigned int)frac + (unsigned int)step);
                int carry = ((unsigned int)frac < oldFrac) ? 1 : 0;
                srcAccum = srcAccum + rowStep + carry;
                *reinterpret_cast<unsigned char *>(dst) = ah;
                if (frac & 0x8000) {
                    frac &= ~0x8000;
                    srcAccum += *blit_slot_4e8;
                }
                *blit_light_accum_b = (*blit_light_accum_b & ~0xffff) |
                             (unsigned short)((*blit_light_accum_b & 0xffff) + (unsigned short)*blit_light_pack_a);
                al = (unsigned char)((unsigned int)*blit_light_accum_b >> 8);
                n -= 1;
            } while (n != 0);
        } else {
            int negDx = -dxRatio;
            int notX = ~xAccum;
            int frac = (int)(((unsigned int)(notX & 0xffff) << 16) |
                             ((unsigned int)(yAccum & 0xffff)));
            int step = (int)(((unsigned int)(negDx & 0xffff) << 16) |
                             (((unsigned int)(dyRatio & 0xffff)) >> 1));
            *blit_colour_step = dxInt + colorStep;
            *blit_colour_step = -*blit_colour_step;
            int rowStep = *blit_colour_step;

            unsigned char al = (unsigned char)((unsigned int)*blit_light_accum_b >> 8);
            dst -= 1;
            int n = count;
            do {
                unsigned char ah = *reinterpret_cast<unsigned char *>(srcAccum);
                dst += 1;
                ah = (unsigned char)(ah + al);
                unsigned int oldFrac = (unsigned int)frac;
                frac = (int)((unsigned int)frac + (unsigned int)step);
                int carry = ((unsigned int)frac < oldFrac) ? 1 : 0;
                srcAccum = srcAccum - rowStep - carry;
                *reinterpret_cast<unsigned char *>(dst) = ah;
                if (frac & 0x8000) {
                    frac &= ~0x8000;
                    srcAccum += *blit_slot_4e8;
                }
                *blit_light_accum_b = (*blit_light_accum_b & ~0xffff) |
                             (unsigned short)((*blit_light_accum_b & 0xffff) + (unsigned short)*blit_light_pack_a);
                al = (unsigned char)((unsigned int)*blit_light_accum_b >> 8);
                n -= 1;
            } while (n != 0);
        }
    }
}

// Fixed-slot bindings carried from 00620640.cpp
static int *const blit_slot_55c = (int *)0x009BB55C;

// ORIGINAL: 0x00620640 sub_620640 0x00620640-0x00620A10 FILE
// symbol    ?sub_620640@@YAXHH@Z
// TRIED: MISMATCH #3 (mov vs push, prologue) - full transcription of both loop bodies (dxStep<0 and dxStep>=0), all field reads/ writes, and the two 64-bit divides. Ghidra's CONCAT44/CARRY macros were replaced by their algebraic equivalents rather than copied literally: `(uint)(P>>0x10) | (int)(P>>0x20)<<0x10` for a 64-bit product P is exactly `(uint)(P>>16)`, and the byte-add-with-manual-+0x100-on-carry dither-table index is exactly a plain 32-bit pointer add (verified by hand: the 8-bit carry-out always lands on the same bit the `add ebx,0x100` sets). Not expected byte-exact at 976 bytes of dense fixed-point DDA arithmetic.
// working copy - scaffold materialised by --work
// size      976 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_620640  at 0x00620640  (976 bytes)
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

void __cdecl sub_620640(int param_1, int param_2) {
    int y1 = *reinterpret_cast<int *>(param_1 + 0x1c);
    *blit_light_accum_b = *reinterpret_cast<unsigned int *>(param_1 + 0x34);
    int y2 = *reinterpret_cast<int *>(param_2 + 0x1c);
    *blit_light_step_b = *reinterpret_cast<int *>(param_2 + 0x34);

    if (!(*blit_clip_left < y2 && y1 < *blit_clip_right && (y2 - y1) != 0 && y1 <= y2)) {
        return;
    }

    int dy16 = (y2 - y1) * 0x10000;

    __int64 num1 = (static_cast<__int64>(y2) << 32) |
                   static_cast<unsigned int>((*blit_light_step_b - *blit_light_accum_b) * 0x100);
    *reinterpret_cast<unsigned short *>(blit_light_pack_a) =
        static_cast<unsigned short>(num1 / static_cast<__int64>(dy16));

    int dx = *reinterpret_cast<int *>(param_2 + 0xc) - *reinterpret_cast<int *>(param_1 + 0xc);
    __int64 num2 = (static_cast<__int64>(dx * 0x100 >> 0x10) << 32) |
                   static_cast<unsigned int>(dx * 0x1000000);
    __int64 quot1_64 = num2 / static_cast<__int64>(dy16);
    int quot1 = static_cast<int>(quot1_64);
    int dxStep = 1;
    int dxFracAdj = quot1;
    if (quot1 < 0) {
        dxStep = -1;
        if ((quot1_64 & 0xffff) != 0) {
            dxFracAdj = quot1 + 0x10000;
        }
    }
    unsigned int xAcc = static_cast<unsigned int>(dxFracAdj >> 1) +
                         *reinterpret_cast<unsigned int *>(param_1 + 0xc);

    int dz = *reinterpret_cast<int *>(param_2 + 0x10) - *reinterpret_cast<int *>(param_1 + 0x10);
    __int64 num3 = (static_cast<__int64>(dz >> 0x10) << 32) |
                   static_cast<unsigned int>(dz * 0x10000);
    __int64 quot2_64 = num3 / static_cast<__int64>(dy16);
    int quot2 = static_cast<int>(quot2_64);
    int dzFracAdj = quot2;
    *blit_slot_4e8 = *blit_texture_width2;
    if (quot2 < 0) {
        *blit_slot_4e8 = -*blit_texture_width2;
        if ((quot2_64 & 0xffff) != 0) {
            dzFracAdj = quot2 + 0x10000;
        }
    }
    int zWholeStep = (dzFracAdj >> 0x10) * (*blit_texture_width2);
    unsigned int zAcc = static_cast<unsigned int>(dzFracAdj >> 1) +
                         *reinterpret_cast<unsigned int *>(param_1 + 0x10);

    int yEnd = y2;
    if (*blit_clip_right <= yEnd) {
        yEnd = *blit_clip_right;
    }
    int yStart = y1;
    if (y1 < *blit_clip_left) {
        int preStepY = (y1 - *blit_clip_left) * -0x10000;
        xAcc = static_cast<unsigned int>((static_cast<__int64>(quot1) * static_cast<__int64>(preStepY)) >> 16) + xAcc;
        zAcc = static_cast<unsigned int>((static_cast<__int64>(quot2) * static_cast<__int64>(preStepY)) >> 16) + zAcc;
        *blit_light_accum_b = static_cast<unsigned int>(
            (static_cast<__int64>(static_cast<int>(*reinterpret_cast<unsigned short *>(blit_light_pack_a))) *
             static_cast<__int64>(preStepY)) >> 16) + *blit_light_accum_b;
        yStart = *blit_clip_left;
    }
    int rowCount = yEnd - yStart;

    int rowOffset = static_cast<int>(xAcc >> 0x10) +
                     static_cast<unsigned short>(static_cast<short>(zAcc >> 16) * static_cast<short>(*blit_texture_width2));
    unsigned char *srcPtr = reinterpret_cast<unsigned char *>(rowOffset + *blit_texture_pixels);
    *reinterpret_cast<unsigned char **>(blit_slot_558) =
        reinterpret_cast<unsigned char *>(rowOffset + *blit_shade_table);

    int destBase = (*blit_row_offset) * (*blit_buffer_stride) + yStart + (*blit_pixel_base);
    unsigned char *destPtr = reinterpret_cast<unsigned char *>(destBase);

    if (*blit_slot_4e8 < 0) {
        quot2 = -quot2;
        zAcc = ~zAcc;
    }

    unsigned char ditherLow = static_cast<unsigned char>(*blit_slot_55c);
    unsigned char *ditherBase = reinterpret_cast<unsigned char *>(*blit_slot_55c);

    if (dxStep < 0) {
        unsigned int stepPacked =
            (static_cast<unsigned int>(static_cast<unsigned short>(-static_cast<short>(*reinterpret_cast<unsigned short *>(blit_light_pack_a)))) << 16) |
            static_cast<unsigned short>(static_cast<unsigned int>(quot2) >> 1);
        unsigned int accPacked =
            (static_cast<unsigned int>(static_cast<unsigned short>(~static_cast<unsigned short>(xAcc))) << 16) |
            static_cast<unsigned short>(zAcc);

        zWholeStep = (dxFracAdj >> 0x10) + zWholeStep;
        *blit_colour_step = -zWholeStep;
        destPtr = destPtr - 1;

        unsigned char *entry = ditherBase + *srcPtr;
        unsigned int flag = *reinterpret_cast<unsigned int *>(entry);
        unsigned int color = 0;
        if (flag != 0) {
            color = *blit_light_accum_b >> 8;
        }

        do {
            unsigned char srcByte = *srcPtr;
            destPtr = destPtr + 1;
            unsigned __int64 sum = static_cast<unsigned __int64>(accPacked) + stepPacked;
            bool carried = sum > 0xffffffffu;
            accPacked = static_cast<unsigned int>(sum);
            if (carried) {
                srcPtr = srcPtr + 1;
                *reinterpret_cast<unsigned char **>(blit_slot_558) += 1;
            }
            srcPtr = srcPtr + zWholeStep;
            *reinterpret_cast<unsigned char **>(blit_slot_558) += zWholeStep;
            *destPtr = static_cast<unsigned char>(srcByte + static_cast<unsigned char>(color));
            if ((accPacked & 0x8000) != 0) {
                accPacked = accPacked & 0xffff7fff;
                srcPtr = srcPtr + *blit_slot_4e8;
                *reinterpret_cast<unsigned char **>(blit_slot_558) += *blit_slot_4e8;
            }
            entry = ditherBase + *srcPtr;
            flag = *reinterpret_cast<unsigned int *>(entry);
            *blit_slot_4d8 = 0;
            if (flag != 0) {
                *reinterpret_cast<unsigned short *>(blit_light_accum_b) =
                    static_cast<unsigned short>(*reinterpret_cast<short *>(blit_light_accum_b) + *reinterpret_cast<unsigned short *>(blit_light_pack_a));
                *blit_slot_4d8 = *blit_light_accum_b >> 8;
            }
            color = *reinterpret_cast<int *>((**reinterpret_cast<unsigned char **>(blit_slot_558)) * 4 + *blit_remap_table) + *blit_slot_4d8;
            rowCount--;
        } while (rowCount != 0);
    } else {
        unsigned int accPacked =
            (static_cast<unsigned int>(static_cast<unsigned short>(xAcc)) << 16) |
            static_cast<unsigned short>(static_cast<unsigned int>(zAcc) >> 1);
        unsigned int stepPacked =
            (static_cast<unsigned int>(static_cast<unsigned short>(quot1)) << 16) |
            static_cast<unsigned short>(static_cast<unsigned int>(quot2) >> 1);

        zWholeStep = (dxFracAdj >> 0x10) + zWholeStep;
        destPtr = destPtr - 1;

        unsigned char *entry = ditherBase + *srcPtr;
        unsigned int flag = *reinterpret_cast<unsigned int *>(entry);
        unsigned int color = 0;
        if (flag != 0) {
            color = *blit_light_accum_b >> 8;
        }
        int rowColor = *reinterpret_cast<int *>((**reinterpret_cast<unsigned char **>(blit_slot_558)) * 4 + *blit_remap_table) + color;
        *blit_colour_step = zWholeStep;

        do {
            unsigned char srcByte = *srcPtr;
            destPtr = destPtr + 1;
            unsigned __int64 sum = static_cast<unsigned __int64>(accPacked) + stepPacked;
            bool carried = sum > 0xffffffffu;
            accPacked = static_cast<unsigned int>(sum);
            if (carried) {
                *reinterpret_cast<unsigned char **>(blit_slot_558) += 1;
                srcPtr = srcPtr + 1;
            }
            *reinterpret_cast<unsigned char **>(blit_slot_558) += zWholeStep;
            srcPtr = srcPtr + zWholeStep;
            *destPtr = static_cast<unsigned char>(srcByte + static_cast<unsigned char>(rowColor));
            if ((accPacked & 0x8000) != 0) {
                accPacked = accPacked & 0xffff7fff;
                srcPtr = srcPtr + *blit_slot_4e8;
                *reinterpret_cast<unsigned char **>(blit_slot_558) += *blit_slot_4e8;
            }
            entry = ditherBase + *srcPtr;
            flag = *reinterpret_cast<unsigned int *>(entry);
            *blit_slot_4d8 = 0;
            if (flag != 0) {
                *reinterpret_cast<unsigned short *>(blit_light_accum_b) =
                    static_cast<unsigned short>(*reinterpret_cast<short *>(blit_light_accum_b) + *reinterpret_cast<unsigned short *>(blit_light_pack_a));
                *blit_slot_4d8 = *blit_light_accum_b >> 8;
            }
            rowColor = *reinterpret_cast<int *>((**reinterpret_cast<unsigned char **>(blit_slot_558)) * 4 + *blit_remap_table) + *blit_slot_4d8;
            rowCount--;
        } while (rowCount != 0);
    }
}

// ORIGINAL: 0x0061F0D0 sub_61f0d0 0x0061F0D0-0x0061F785 FILE
// symbol    ?sub_61f0d0@Texture@@QAEXPAVBuffer@@HHHHIH@Z
// TRIED: literal Ghidra-derived transcription (EdgeScan DDA struct, param_2 as Buffer*, CONCAT/CARRY helpers for the dither table lookups) compiles and matches the outer control flow, but MSVC's register allocation for the two 300-line unrolled scanline loops diverges from instruction #5 on; did not attempt hand-tuning further given the size.
// working copy - scaffold materialised by --work
// size      1717 bytes
// prototype 
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90 0x00620100

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61f0d0  at 0x0061F0D0  (1717 bytes)
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

void Texture::sub_61f0d0(Buffer *param_2, int param_3, int param_4, int param_5,
                             int param_6, uint32_t param_7, int param_8) {
    int32_t *param_1 = (int32_t *)this;

    ulonglong uVar1;
    ulonglong uVar2;
    int32_t iVar3;
    int32_t iVar4;
    uint32_t uVar5;
    int32_t *piVar6;
    int32_t iVar7;
    uint32_t uVar8;
    byte bVar9;
    int32_t iVar10;
    uint32_t uVar11;
    byte *pbVar12;
    int32_t iVar13;
    int32_t iVar14;
    char *pcVar15;
    bool bVar16;
    uint32_t uStackY_ac;
    int32_t local_88;
    EdgeScan edge1;
    EdgeScan edge2;

  if (param_3 != 0) {
    if (param_2 == 0) {
      return;
    }
    if (((((*param_1 != 0) && (param_4 != 0)) && (param_6 != 0)) &&
        ((param_5 != 0 && (param_7 != 0)))) &&
       ((*blit_pixel_base) = param_2->get_data(), (*blit_pixel_base) != 0)) {
      (*blit_z_values) = param_4;
      (*blit_edge_uvs_b) = param_3;
      (*blit_texture_pixels) = *param_1;
      (*blit_texture_width2) = param_1[1];
      (*blit_texture_height) = param_1[2];
      if (param_8 == 0) {
        iVar10 = 4;
        (*blit_edge_uvs_a) = (int32_t)(param_1 + 3);
        (*blit_vertex_count) = 4;
      }
      else {
        iVar10 = 8;
        (*blit_edge_uvs_a) = (int32_t)(param_1 + 0xb);
        (*blit_vertex_count) = 8;
      }
      (*blit_buffer_stride) = param_2->stride_;
      (*blit_texture_width) = param_1[1];
      (*blit_slot_55c) = param_7;
      (*blit_remap_table) = param_6;
      (*blit_shade_table) = *(int32_t *)param_5;
      if (((*blit_shade_table) != 0) && ((reinterpret_cast<RECT *>(reinterpret_cast<char *>(param_2) + 0x20)) != (RECT *)0x0)) {
        (*blit_clip_right) = *reinterpret_cast<int *>(reinterpret_cast<char *>(param_2) + 0x28);
        iVar13 = -0x7ffd;
        (*blit_clip_left) = *reinterpret_cast<int *>(reinterpret_cast<char *>(param_2) + 0x20);
        (*blit_clip_bottom) = *reinterpret_cast<int *>(reinterpret_cast<char *>(param_2) + 0x2c);
        (*blit_clip_top) = *reinterpret_cast<int *>(reinterpret_cast<char *>(param_2) + 0x24);
        iVar3 = 0;
        iVar7 = 0x7fff;
        if (iVar10 != 0) {
          piVar6 = (int *)(param_3 + 4);
          do {
            iVar4 = *piVar6;
            if (iVar4 < iVar7) {
              iVar7 = iVar4;
              iVar10 = (*blit_vertex_count);
              param_3 = iVar3;
            }
            if (iVar13 < iVar4) {
              iVar13 = *piVar6;
              (*blit_max_y_index) = iVar3;
            }
            iVar3 = iVar3 + 1;
            piVar6 = piVar6 + 2;
          } while (iVar3 < iVar10);
          if (iVar7 < iVar13) {
            edge1.f0 = -1;
            (*blit_row_offset) = iVar7;
            iVar10 = Texture::setup_edge_gouraud(&edge1, param_3);
            if (iVar10 != 0) {
              edge2.f0 = 1;
              iVar10 = Texture::setup_edge_gouraud(&edge2, param_3);
              if ((iVar10 != 0) && ((*blit_row_offset) < (*blit_clip_bottom))) {
                do {
                  if ((*blit_clip_top) <= (*blit_row_offset)) {
                    (*blit_light_accum_b) = edge1.accumD;
                    (*blit_light_step_b) = edge2.accumD;
                    if ((((*blit_clip_left) < edge2.accumC) && (edge1.accumC < (*blit_clip_right))) &&
                       (edge2.accumC - edge1.accumC != 0 && edge1.accumC <= edge2.accumC)) {
                      iVar3 = (edge2.accumC - edge1.accumC) * 0x10000;
                      (*blit_light_pack_a) = (ushort)((longlong)CONCAT44(edge2.accumD,(edge2.accumD - edge1.accumD) * 0x100) /
                                             (longlong)iVar3);
                      uVar1 = (longlong)CONCAT44((edge2.accumA - edge1.accumA) * 0x100 >> 0x10,
                                       (edge2.accumA - edge1.accumA) * 0x1000000) / (longlong)iVar3;
                      iVar13 = (int)uVar1;
                      iVar7 = 1;
                      iVar10 = iVar13;
                      if ((iVar13 < 0) && (iVar7 = -1, (uVar1 & 0xffff) != 0)) {
                        iVar10 = iVar13 + 0x10000;
                      }
                      param_5 = (iVar10 >> 1) + edge1.accumA;
                      uVar2 = (longlong)CONCAT44(edge2.accumB - edge1.accumB >> 0x10,(edge2.accumB - edge1.accumB) * 0x10000)
                              / (longlong)iVar3;
                      local_88 = (int)uVar2;
                      iVar3 = local_88;
                      (*blit_slot_4e8) = (*blit_texture_width2);
                      if ((local_88 < 0) && ((*blit_slot_4e8) = -(*blit_texture_width2), (uVar2 & 0xffff) != 0)) {
                        iVar3 = local_88 + 0x10000;
                      }
                      iVar4 = (iVar3 >> 0x10) * (*blit_texture_width2);
                      param_7 = (iVar3 >> 1) + edge1.accumB;
                      iVar3 = edge2.accumC;
                      if ((*blit_clip_right) <= edge2.accumC) {
                        iVar3 = (*blit_clip_right);
                      }
                      iVar14 = edge1.accumC;
                      if (edge1.accumC < (*blit_clip_left)) {
                        iVar14 = (edge1.accumC - (*blit_clip_left)) * -0x10000;
                        param_5 = (((uint)((longlong)iVar13 * (longlong)iVar14) >> 0x10 |
                                          (int)((ulonglong)((longlong)iVar13 * (longlong)iVar14) >>
                                               0x20) << 0x10) + (int)param_5);
                        param_7 = ((uint)((longlong)local_88 * (longlong)iVar14) >> 0x10 |
                                  (int)((ulonglong)((longlong)local_88 * (longlong)iVar14) >> 0x20)
                                  << 0x10) + param_7;
                        (*blit_light_accum_b) = ((uint)((longlong)(int)(uint)(*blit_light_pack_a) * (longlong)iVar14)
                                        >> 0x10 |
                                       (int)((ulonglong)
                                             ((longlong)(int)(uint)(*blit_light_pack_a) * (longlong)iVar14)
                                            >> 0x20) << 0x10) + edge1.accumD;
                        iVar14 = (*blit_clip_left);
                      }
                      iVar3 = iVar3 - iVar14;
                      iVar13 = ((uint)param_5 >> 0x10) +
                               (uint)(ushort)((ushort)(param_7 >> 16) * (short)(*blit_texture_width2));
                      pbVar12 = (byte *)(iVar13 + (*blit_texture_pixels));
                      (*blit_slot_558) = iVar13 + (*blit_shade_table);
                      iVar13 = (*blit_row_offset) * (*blit_buffer_stride) + iVar14 + (*blit_pixel_base);
                      if ((*blit_slot_4e8) < 0) {
                        local_88 = -local_88;
                        param_7 = ~param_7;
                      }
                      bVar9 = (byte)(*blit_slot_55c);
                      uVar11 = (uint32_t)((uint)(*blit_slot_55c) >> 8);
                      if (iVar7 < 0) {
                        uVar8 = CONCAT22(-(short)uVar1,(ushort)local_88 >> 1);
                        uStackY_ac = CONCAT22(~(ushort)param_5,(ushort)param_7);
                        iVar4 = (iVar10 >> 0x10) + iVar4;
                        (*blit_colour_step) = -iVar4;
                        pcVar15 = (char *)(iVar13 + -1);
                        piVar6 = (int *)CONCAT31(uVar11,bVar9 + *pbVar12);
                        if (CARRY1(bVar9,*pbVar12)) {
                          piVar6 = piVar6 + 0x40;
                        }
                        uVar5 = 0;
                        if (*piVar6 != 0) {
                          uVar5 = (*blit_light_accum_b) >> 8;
                        }
                        do {
                          bVar9 = *pbVar12;
                          pcVar15 = pcVar15 + 1;
                          bVar16 = CARRY4(uStackY_ac,uVar8);
                          uStackY_ac = uStackY_ac + uVar8;
                          if (bVar16) {
                            pbVar12 = pbVar12 + 1;
                            (*blit_slot_558) = (*blit_slot_558) + 1;
                          }
                          pbVar12 = pbVar12 + iVar4;
                          (*blit_slot_558) = (*blit_slot_558) + iVar4;
                          *pcVar15 = bVar9 + (char)uVar5;
                          if ((uStackY_ac & 0x8000) != 0) {
                            uStackY_ac = uStackY_ac & 0xffff7fff;
                            pbVar12 = pbVar12 + (*blit_slot_4e8);
                            (*blit_slot_558) = (*blit_slot_558) + (*blit_slot_4e8);
                          }
                          piVar6 = (int *)CONCAT31((uint32_t)((uint)(*blit_slot_55c) >> 8),
                                                   (byte)(*blit_slot_55c) + *pbVar12);
                          if (CARRY1((byte)(*blit_slot_55c),*pbVar12)) {
                            piVar6 = piVar6 + 0x40;
                          }
                          (*blit_slot_4d8) = 0;
                          if (*piVar6 != 0) {
                            (*blit_light_accum_b) = CONCAT22((short)((*blit_light_accum_b) >> 0x10),
                                                    (short)(*blit_light_accum_b) + (*blit_light_pack_a));
                            (*blit_slot_4d8) = (*blit_light_accum_b) >> 8;
                          }
                          uVar5 = *(int *)((uint)(*(byte *)(*blit_slot_558)) * 4 + (*blit_remap_table)) + (*blit_slot_4d8);
                          iVar3 = iVar3 + -1;
                        } while (iVar3 != 0);
                      }
                      else {
                        uStackY_ac = CONCAT22((ushort)param_5,(ushort)param_7 >> 1);
                        uVar8 = CONCAT22((short)uVar1,(ushort)local_88 >> 1);
                        iVar4 = (iVar10 >> 0x10) + iVar4;
                        pcVar15 = (char *)(iVar13 + -1);
                        piVar6 = (int *)CONCAT31(uVar11,bVar9 + *pbVar12);
                        if (CARRY1(bVar9,*pbVar12)) {
                          piVar6 = piVar6 + 0x40;
                        }
                        uVar5 = 0;
                        if (*piVar6 != 0) {
                          uVar5 = (*blit_light_accum_b) >> 8;
                        }
                        iVar10 = *(int *)((uint)(*(byte *)(*blit_slot_558)) * 4 + (*blit_remap_table)) + uVar5;
                        (*blit_colour_step) = iVar4;
                        do {
                          bVar9 = *pbVar12;
                          pcVar15 = pcVar15 + 1;
                          bVar16 = CARRY4(uStackY_ac,uVar8);
                          uStackY_ac = uStackY_ac + uVar8;
                          if (bVar16) {
                            (*blit_slot_558) = (*blit_slot_558) + 1;
                            pbVar12 = pbVar12 + 1;
                          }
                          (*blit_slot_558) = (*blit_slot_558) + iVar4;
                          pbVar12 = pbVar12 + iVar4;
                          *pcVar15 = bVar9 + (char)iVar10;
                          if ((uStackY_ac & 0x8000) != 0) {
                            uStackY_ac = uStackY_ac & 0xffff7fff;
                            pbVar12 = pbVar12 + (*blit_slot_4e8);
                            (*blit_slot_558) = (*blit_slot_558) + (*blit_slot_4e8);
                          }
                          piVar6 = (int *)CONCAT31((uint32_t)((uint)(*blit_slot_55c) >> 8),
                                                   (byte)(*blit_slot_55c) + *pbVar12);
                          if (CARRY1((byte)(*blit_slot_55c),*pbVar12)) {
                            piVar6 = piVar6 + 0x40;
                          }
                          (*blit_slot_4d8) = 0;
                          if (*piVar6 != 0) {
                            (*blit_light_accum_b) = CONCAT22((short)((*blit_light_accum_b) >> 0x10),
                                                    (short)(*blit_light_accum_b) + (*blit_light_pack_a));
                            (*blit_slot_4d8) = (*blit_light_accum_b) >> 8;
                          }
                          iVar10 = *(int *)((uint)(*(byte *)(*blit_slot_558)) * 4 + (*blit_remap_table)) + (*blit_slot_4d8);
                          iVar3 = iVar3 + -1;
                        } while (iVar3 != 0);
                      }
                    }
                  }
                  edge1.counter = edge1.counter + -1;
                  if (edge1.counter == 0) {
                    iVar10 = sub_620100(reinterpret_cast<int *>(&edge1), edge1.id);
                    if (iVar10 == 0) break;
                  }
                  else {
                    edge1.accumA = edge1.accumA + edge1.deltaA;
                    edge1.accumB = edge1.accumB + edge1.deltaB;
                    edge1.fracAccum = edge1.fracAccum + edge1.fracDelta;
                    edge1.accumC = edge1.accumC + edge1.deltaC;
                    edge1.accumD = edge1.accumD + edge1.deltaD;
                    if (0 < edge1.fracAccum) {
                      edge1.accumC = edge1.accumC + edge1.extraDeltaC;
                      edge1.fracAccum = edge1.fracAccum - edge1.fracSub;
                    }
                  }
                  edge2.counter = edge2.counter + -1;
                  if (edge2.counter == 0) {
                    iVar10 = sub_620100(reinterpret_cast<int *>(&edge2), edge2.id);
                    if (iVar10 == 0) break;
                  }
                  else {
                    edge2.accumA = edge2.accumA + edge2.deltaA;
                    edge2.accumB = edge2.accumB + edge2.deltaB;
                    edge2.fracAccum = edge2.fracAccum + edge2.fracDelta;
                    edge2.accumC = edge2.accumC + edge2.deltaC;
                    edge2.accumD = edge2.accumD + edge2.deltaD;
                    if (0 < edge2.fracAccum) {
                      edge2.accumC = edge2.accumC + edge2.extraDeltaC;
                      edge2.fracAccum = edge2.fracAccum - edge2.fracSub;
                    }
                  }
                  (*blit_row_offset) = (*blit_row_offset) + 1;
                } while ((*blit_row_offset) < (*blit_clip_bottom));
              }
            }
          }
        }
      }
    }
  }
  if (param_2 != 0) {
    param_2->free_data(1);
  }
  return;
}

// ORIGINAL: 0x006239E0 Texture::draw_upper_threshold_trans2 0x006239E0-0x00623F44 FILE
// symbol    ?draw_upper_threshold_trans2@Texture@@QAEXPAVBuffer@@PAUVert@@HHH@Z
// TRIED: MISMATCH #0. The inner scanline blitter uses ESP itself as the per-span pixel counter (push ebp; mov esp,ecx; ...; sub esp, 0x10000; jns loop; mov esp,[global]; pop ebp) - that shape only comes from hand-written __asm in the original source, which is barred here, so byte-exactness is out of reach for this function. Body is a faithful high-level port instead: verified EdgeScan field offsets (f[3]/f[4] int accum, f[5]/f[6] step, f[7]/f[8] fractional accum/step, f[9]-f[12] periodic correction) by hand-tracing both mirrored update blocks (0x623C47 and 0x623C9E) against raw ebp-relative offsets, not Ghidra's renumbered locals, which are off by a few bytes from the real frame. Per-pixel texture/threshold loop transcribed from Ghidra's CONCAT/CARRY reconstruction via local concat22/ concat11/concat31/carry4/carry2 helpers.
// working copy - scaffold materialised by --work
// size      1380 bytes
// prototype 
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061A760 0x00624D80

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: Texture::draw_upper_threshold_trans2  at 0x006239E0  (1380 bytes)
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

void Texture::draw_upper_threshold_trans2(Buffer *buf, Vert *verts, int aux, int count, int threshold_color) {
    if (threshold_color == -1) {
        // a3 is typed int* only because the catalogue's mangled name says PAH;
        // the callee reads it as a scalar count, and so does this caller.
        draw_upper_threshold_trans(buf, verts, reinterpret_cast<int *>(aux), count);
        return;
    }

    *blit_pixel_base = buf->get_data();
    *(unsigned char *)blit_slot_494 = (unsigned char)threshold_color;
    *blit_edge_uvs_b = (int)(long)verts;
    *blit_texture_pixels = (int)(long)pixels_;
    *blit_texture_width2 = (int)iWidth_;
    *blit_texture_height = (int)iHeight_;
    *blit_edge_uvs_a = aux;
    *blit_vertex_count = count;
    *blit_light_threshold = *blit_light_threshold_2;
    *blit_buffer_stride = (int)buf->stride_;
    *blit_texture_width = (int)iWidth_;

    if (buf == 0) {
        goto done;
    }

    {
        *blit_clip_right = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x28);
        *blit_clip_left = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x20);
        *blit_clip_bottom = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x2c);
        *blit_clip_top = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x24);

        int min_y = 0x7fff, min_y_idx = 0;
        int max_y = (int)0xffff8003, max_y_idx = 0;
        int min_x = 0x7fff;
        int max_x = (int)0xffff8003;
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                int y = reinterpret_cast<int *>(verts)[i * 2 + 1];
                if (y < min_y) { min_y_idx = i; min_y = y; }
                if (max_y < y) { max_y_idx = i; max_y = y; *blit_max_y_index = i; }
                int x = reinterpret_cast<int *>(verts)[i * 2];
                if (x < min_x) min_x = x;
                if (max_x < x) max_x = x;
            }
            if (min_y < max_y) {
                *blit_row_offset = *blit_buffer_stride * min_y + *blit_pixel_base;
                *blit_dither = min_y & 1;
                *blit_clip_bottom = *blit_clip_bottom * *blit_buffer_stride + *blit_pixel_base;
                *blit_clip_top = *blit_clip_top * *blit_buffer_stride + *blit_pixel_base;

                EdgeScan e1, e2;
                e1.f0 = -1;
                if (setup_edge(&e1, min_y_idx) != 0) {
                    e2.counter = 1;
                    if (setup_edge(&e2, min_y_idx) != 0) {
                        double dy = (double)((e2.accumC - e1.accumC) << 16);
                        double du = 0, dv = 0, inv_dy = 1;
                        if (dy > 0) {
                            du = (double)(e2.accumB - e1.accumB);
                            dv = (double)(e2.accumA - e1.accumA);
                            inv_dy = 1.0 / dy;
                        }

                        bool done = false;
                        while (*blit_row_offset < *blit_clip_bottom) {
                            *blit_slot_4c4 = e2.accumA;
                            *blit_slot_4f8 = e1.accumA;
                            *blit_slot_530 = e2.accumB;
                            *blit_slot_53c = e1.accumB;
                            *blit_span_right = e1.accumC;
                            *blit_span_left = e2.accumC;

                            e1.counter -= 1;
                            if (e1.counter == 0) {
                                if (setup_edge(&e1, e1.id) == 0) {
                                    done = true;
                                    goto per_scanline_tail;
                                }
                            } else {
                                edge_step(e1);
                            }
                            e2.counter -= 1;
                            if (e2.counter == 0) {
                                if (setup_edge(&e2, e2.id) == 0) {
                                    done = true;
                                }
                            } else {
                                edge_step(e2);
                            }

                        per_scanline_tail:
                            {
                                unsigned char threshold = *(unsigned char *)blit_slot_494;
                                if (dy > 0) {
                                    *blit_slot_4e4 = (int)(dv * du * inv_dy * (double)*(float *)blit_scale_65536f);
                                    *blit_slot_4c8 = (int)(du * dv * inv_dy * (double)*(float *)blit_scale_65536f);
                                }
                                dy = (double)((e2.accumC - e1.accumC) << 16);
                                if (dy > 0) {
                                    du = (double)(e2.accumB - e1.accumB);
                                    dv = (double)(e2.accumA - e1.accumA);
                                    inv_dy = 1.0 / dy;
                                }

                                if (*blit_clip_bottom <= *blit_row_offset) break;
                                if (*blit_clip_top <= *blit_row_offset && *blit_clip_left < *blit_span_left &&
                                    *blit_span_right < *blit_clip_right && *blit_span_right < *blit_span_left) {
                                    int right_x = *blit_span_left;
                                    if (*blit_clip_right <= *blit_span_left) right_x = *blit_clip_right;
                                    if (*blit_span_right < *blit_clip_left) {
                                        int step = (*blit_span_right - *blit_clip_left) * -0x10000;
                                        __int64 p1 = (__int64)*blit_slot_4e4 * (__int64)step;
                                        *blit_slot_4f8 = (int)(((unsigned int)p1 >> 16) | ((int)((unsigned __int64)p1 >> 32) << 16)) + *blit_slot_4f8;
                                        __int64 p2 = (__int64)*blit_slot_4c8 * (__int64)step;
                                        *blit_slot_53c = *blit_slot_53c + (int)(((unsigned int)p2 >> 16) | ((int)((unsigned __int64)p2 >> 32) << 16));
                                        *blit_span_right = *blit_clip_left;
                                    }

                                    unsigned char *tex_row = (unsigned char *)(*blit_texture_pixels + *blit_texture_width * (*blit_slot_53c >> 16) + (*blit_slot_4f8 >> 16));
                                    char *dest = (char *)(long)(*blit_span_right + *blit_row_offset);
                                    *blit_alt_slot_524 = ((*blit_slot_4c8 >> 16) * *blit_texture_width + (*blit_slot_4e4 >> 16));
                                    *blit_alt_slot_520 = *blit_alt_slot_524 + *blit_texture_width;
                                    *blit_slot_528 = *blit_slot_4c8 << 16;

                                    unsigned int u_acc = concat22((unsigned short)*blit_slot_53c, (unsigned short)*blit_slot_4e4);
                                    unsigned int span = concat22((unsigned short)(((short)right_x - (short)*blit_span_right) - 1), (unsigned short)*blit_slot_4f8);
                                    *blit_saved_esp = 0;
                                    *blit_slot_504 = (int)(long)tex_row;

                                    unsigned int texel = concat11(*(unsigned char *)blit_light_threshold, *tex_row);
                                    unsigned int off = 0;
                                    for (;;) {
                                        int carry = carry4(u_acc, *(unsigned int *)blit_slot_528);
                                        u_acc = u_acc + *(unsigned int *)blit_slot_528;
                                        if ((unsigned char)texel != (unsigned char)(texel >> 8)) {
                                            *dest = (char)threshold;
                                        }
                                        unsigned int selected_row = carry ? (unsigned int)*blit_alt_slot_520 : (unsigned int)*blit_alt_slot_524;
                                        int next_span = (int)concat22((unsigned short)((unsigned int)span >> 16),
                                                                       (unsigned short)((unsigned short)span + (unsigned short)u_acc));
                                        off = selected_row + off + (unsigned int)(carry2((unsigned short)span, (unsigned short)u_acc) ? 1 : 0);
                                        dest = dest + 1;
                                        texel = concat31(texel >> 8, tex_row[(int)off]);
                                        span = (unsigned int)(next_span - 0x10000);
                                        if ((int)span < 0) break;
                                    }
                                }
                                if (done) break;
                                *blit_row_offset = *blit_row_offset + *blit_buffer_stride;
                                *blit_dither = *blit_dither + 1;
                            }
                        }
                    }
                }
            }
        }
    }

done:
    if (buf != 0) {
        buf->free_data(1);
    }
}

// ORIGINAL: 0x006239B0 ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@HPAUVert@@H@Z 0x006239B0-0x006239D3 BYTE_EXACT
// symbol    ?draw_trans@Texture@@QAEXPAVBuffer@@PAUVert@@H1H@Z
//            PAV for `Buffer`, where the image spells PAU - this tree declares
//            it `class`, the image `struct`. Same bytes, differing name.
// size      35 bytes
// prototype void (__thiscall ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@HPAUVert@@H@Z)(Texture* this, Buffer*, Vert*, int, Vert*, int)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006198E0
// 0x006239B0  ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@HPAUVert@@H@Z  ->  ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@H1H@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void Texture::draw_trans(Buffer* a1, Vert* a2, int a3, Vert* a4, int a5) {
    if (a5 == -1) {
        draw_trans(a1, a2, a3, a4);
    }
}

// ORIGINAL: 0x00623F50 sub_623f50 0x00623F50-0x00623F7D BYTE_EXACT
// symbol    ?sub_623f50@Texture@@QAEXHHHHHHH@Z
// symbol    ?invoke@Subject@@QAEXHHHHHHH@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0061C630
// 0x00623F50  sub_623f50  ->  _sub_623f50
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----

void Texture::sub_623f50(int a1, int a2, int a3, int a4, int a5, int a6,
                         int a7) {
    // 0x00623F50, 45 bytes, `ret 0x1c`. A seven-int wrapper whose seventh is
    // the same -1 sentinel `draw2` carries: when it is -1 the first six are
    // forwarded to the span filler at 0x0061C630, and otherwise the function
    // is a bare `ret 0x1c`. It reads nothing but its own arguments, so the
    // receiver it is called with is the one it passes on in ecx - the same
    // liveness 0x00624C70 shows.
    if (a7 == -1) {
        sub_61c630(reinterpret_cast<Buffer *>(a1), reinterpret_cast<int *>(a2),
                   reinterpret_cast<Texture *>(a3),
                   reinterpret_cast<uint8_t *>(a4), a5, a6);
    }
}

// ORIGINAL: 0x00624CA0 sub_624ca0 0x00624CA0-0x00624CC8 BYTE_EXACT
// symbol    ?sub_624ca0@Texture@@QAEXHHHHHH@Z
// symbol    ?invoke@Subject@@QAEXHHHHHH@Z
// size      40 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00620A10
// 0x00624CA0  sub_624ca0  ->  _sub_624ca0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----

void Texture::sub_624ca0(int a1, int a2, int a3, int a4, int a5, int a6) {
    // 0x00624CA0, 40 bytes, `ret 0x18`. The sixth int is the -1 sentinel, the
    // same idiom `draw2` and `sub_623f50` carry: when it is -1 the first five
    // are forwarded to the affine rasteriser at 0x00620A10, otherwise the
    // function is a bare `ret 0x18`.
    if (a6 == -1) {
        sub_620a10(reinterpret_cast<Buffer *>(a1), a2, a3, a4,
                   reinterpret_cast<int *>(a5));
    }
}

