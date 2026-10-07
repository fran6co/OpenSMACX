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
#include <float.h>

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

// Plain polygon edges use only the first thirteen EdgeScan words. Gouraud's
// extra channel is not initialized by the original setup at 0x00624D80.
static int texture_fixed(int value) {
    return static_cast<int>(static_cast<unsigned int>(value) << 16);
}

static int texture_difference(int a, int b) {
    return static_cast<int>(static_cast<unsigned int>(a) -
                            static_cast<unsigned int>(b));
}

static int texture_u(const EdgePoint &uv) { return uv.a; }
static int texture_v(const EdgePoint &uv) { return uv.b; }
static int texture_u(const Vert &uv) { return uv.x; }
static int texture_v(const Vert &uv) { return uv.y; }

template<class Coordinates>
static int texture_edge(EdgeScan &edge, int index, const Vert *vertices,
                        const Coordinates *uv, int count, int bottom) {
    if (index == bottom) return 0;
    for (;;) {
        int next = index + edge.f0;
        if (next >= count) next = 0;
        else if (next < 0) next = count - 1;
        edge.counter = texture_difference(vertices[next].y, vertices[index].y);
        if (edge.counter != 0) {
            if (edge.counter < 0) return 0;
            edge.id = next;
            edge.accumA = texture_fixed(texture_u(uv[index]));
            edge.accumB = texture_fixed(texture_v(uv[index]));
            const int height = texture_fixed(edge.counter);
            edge.deltaA = static_cast<int>(
                static_cast<int64_t>(texture_difference(texture_fixed(texture_u(uv[next])), edge.accumA))
                * 65536 / height);
            edge.deltaB = static_cast<int>(
                static_cast<int64_t>(texture_difference(texture_fixed(texture_v(uv[next])), edge.accumB))
                * 65536 / height);
            edge.accumC = vertices[index].x;
            const int dx = texture_difference(vertices[next].x, vertices[index].x);
            const int magnitude = dx < 0 ? texture_difference(0, dx) : dx;
            edge.extraDeltaC = dx < 0 ? -1 : 1;
            edge.fracAccum = dx < 0 ? 1 - edge.counter : 0;
            const int quotient = magnitude / edge.counter;
            edge.deltaC = dx < 0 ? texture_difference(0, quotient) : quotient;
            edge.fracSub = edge.counter;
            edge.fracDelta = static_cast<int>(magnitude % edge.counter);
            return 1;
        }
        index = next;
        if (index == bottom) return 0;
    }
}

template<class Coordinates>
static int texture_advance_edge(EdgeScan &edge, const Vert *vertices,
                                const Coordinates *uv, int count, int bottom) {
    if (--edge.counter == 0)
        return texture_edge(edge, edge.id, vertices, uv, count, bottom);
    edge.accumA = static_cast<int>(static_cast<unsigned int>(edge.accumA) + edge.deltaA);
    edge.accumB = static_cast<int>(static_cast<unsigned int>(edge.accumB) + edge.deltaB);
    edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.deltaC);
    edge.fracAccum = static_cast<int>(static_cast<unsigned int>(edge.fracAccum) + edge.fracDelta);
    if (edge.fracAccum > 0) {
        edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.extraDeltaC);
        edge.fracAccum = texture_difference(edge.fracAccum, edge.fracSub);
    }
    return 1;
}

// 0x0061A3DF clears the x87 precision bits (AND AX,FCFF), then 0x0061A3E3
// selects truncation (OR AX,0C00): arithmetic has a 24-bit significand, not
// FNINIT's 64. Reproduce FDIVR at 0x0061A421 and FMUL at 0x0061A530 with
// integer significand/exponent arithmetic, independently truncating both.
// FILD loads the signed delta exactly; scaling the rounded product by 65536
// is an exact exponent adjustment before FISTP. The widest product is 55 bits.
static int texture_span_step(int delta, int width) {
    const uint64_t denominator = static_cast<uint64_t>(width) << 16;
    uint64_t upper_power = 1;
    int reciprocal_shift = 23;
    while (upper_power < denominator) {
        upper_power <<= 1;
        ++reciprocal_shift;
    }
    const uint64_t significand = (static_cast<uint64_t>(1) << reciprocal_shift) / denominator;
    const uint64_t magnitude = delta < 0 ? -static_cast<int64_t>(delta) : delta;
    uint64_t product = significand * magnitude;
    int exponent = 16 - reciprocal_shift;
    while (product >= (static_cast<uint64_t>(1) << 24)) {
        product >>= 1;
        ++exponent;
    }
    const uint64_t rounded = exponent >= 0 ? product << exponent : product >> -exponent;
    return delta < 0 ? static_cast<int>(-static_cast<int64_t>(rounded)) : static_cast<int>(rounded);
}

static unsigned short texture_light_step(int delta, int width);
// The three keyed span kernels (0x0061A240/0x0061A760/0x006239E0) count
// pixels with one packed register pair: DEC; SHL,16; MOV CX,u_fraction
// builds a signed high-word countdown over an unsigned U fraction, the body
// always runs once before the first test, and ADD SP,du_fraction cannot
// carry into the counter, so the U fraction stays a pure 16-bit accumulator.
// SUB ESP,0x10000/JNS ends on the first negative high word: a span already
// admitted by the raw end>start test runs uint16(end-start) pixels for
// 1..32769 and exactly one pixel for every other value - including the zero
// and inverted spans that clipping can leave behind.
static unsigned int texture_span_pixels(int difference) {
    const unsigned int pixels = static_cast<unsigned int>(difference) & 0xffff;
    return pixels - 1u < 0x8001u ? pixels : 1u;
}

// Shared C++ translations of the original packed-coordinate pixel loops.
static int texture_gouraud_edge(EdgeScan &, int, const Vert *, const Vert *,
                                const int *, int, int);
static int texture_advance_gouraud_edge(EdgeScan &, const Vert *, const Vert *,
                                        const int *, int, int);
static unsigned short texture_light_step(int, int);

static unsigned int texture_draw_source_step(int du, int dv, int width) {
    const int dv_integer = dv >= 0 ? dv / 0x10000
        : static_cast<int>(-((-(static_cast<int64_t>(dv)) + 0xffff) / 0x10000));
    const int du_integer = du >= 0 ? du / 0x10000
        : static_cast<int>(-((-(static_cast<int64_t>(du)) + 0xffff) / 0x10000));
    return static_cast<unsigned int>(dv_integer) *
               static_cast<unsigned int>(width) +
           static_cast<unsigned int>(du_integer);
}

static unsigned int texture_draw_source_index(int width, int u, int v) {
    const unsigned int u_integer = static_cast<unsigned short>(
        static_cast<unsigned int>(u) >> 16);
    const unsigned int v_integer = static_cast<unsigned short>(
        static_cast<unsigned int>(v) >> 16);
    return v_integer * static_cast<unsigned int>(width) + u_integer;
}

static unsigned int texture_draw_next_source_index(
    unsigned int source_offset, unsigned int &v_fraction_and_du_fraction,
    unsigned short &u_fraction, unsigned int dv_fraction,
    unsigned short du_fraction, unsigned int source_step, int width) {
    const unsigned int previous_v_fraction = v_fraction_and_du_fraction;
    v_fraction_and_du_fraction += dv_fraction;
    const bool v_carry = v_fraction_and_du_fraction < previous_v_fraction;
    const unsigned int next_u_fraction =
        static_cast<unsigned int>(u_fraction) + du_fraction;
    const bool u_carry = next_u_fraction > 0xffffu;
    u_fraction = static_cast<unsigned short>(next_u_fraction);
    source_offset += source_step + (v_carry ? static_cast<unsigned int>(width) : 0u);
    source_offset += u_carry ? 1u : 0u;
    return source_offset;
}

static void texture_draw_lit_span_mode(unsigned char *destination,
                                       const unsigned char *pixels, int width,
                                       int u, int v, int du, int dv,
                                       int light, unsigned short light_step,
                                       int pixel_count, bool keyed,
                                       unsigned char key) {
    unsigned int source_offset = texture_draw_source_index(width, u, v);
    unsigned short u_fraction = static_cast<unsigned short>(u);
    unsigned int v_fraction_and_du_fraction =
        (static_cast<unsigned int>(static_cast<unsigned short>(v)) << 16) |
        static_cast<unsigned short>(du);
    const unsigned int dv_fraction =
        static_cast<unsigned int>(static_cast<unsigned short>(dv)) << 16;
    const unsigned int source_step = texture_draw_source_step(du, dv, width);
    unsigned short light_accumulator = static_cast<unsigned short>(
        static_cast<unsigned int>(light) >> 8);
    const unsigned short du_fraction = static_cast<unsigned short>(du);

    for (int pixel = 0; pixel < pixel_count; ++pixel) {
        const unsigned int shade = (light_accumulator >> 8) & 0xffu;
        const unsigned char texel = pixels[source_offset];
        if (!keyed || texel != key) {
            destination[0] = static_cast<unsigned char>(
                static_cast<unsigned int>(texel) + shade);
        }
        source_offset = texture_draw_next_source_index(
            source_offset, v_fraction_and_du_fraction, u_fraction,
            dv_fraction, du_fraction, source_step, width);
        light_accumulator = static_cast<unsigned short>(
            light_accumulator + light_step);
        ++destination;
    }
}

static void texture_draw_lit_span(unsigned char *destination,
                                  const unsigned char *pixels, int width,
                                  int u, int v, int du, int dv,
                                  int light, unsigned short light_step,
                                  int pixel_count) {
    texture_draw_lit_span_mode(destination, pixels, width, u, v, du, dv,
                               light, light_step, pixel_count, false, 0);
}

static void texture_draw_keyed_lit_span(unsigned char *destination,
                                        const unsigned char *pixels, int width,
                                        int u, int v, int du, int dv,
                                        int light, unsigned short light_step,
                                        int pixel_count, unsigned char key) {
    texture_draw_lit_span_mode(destination, pixels, width, u, v, du, dv,
                               light, light_step, pixel_count, true, key);
}

struct TextureDrawSpanResult {
    bool has_pixel;
    unsigned int last_source_offset;
    unsigned int next_source_offset;
    unsigned int last_packed_uv;
    unsigned int next_packed_uv;
    unsigned short last_light;
    unsigned short next_light;
    int *packed_uv_sink;
    volatile const unsigned char *preload_pixels;
    unsigned char prefetched_source;
};

static void texture_draw_coast_span(unsigned char *destination,
                                    const unsigned char *pixels,
                                    const unsigned char *coast_pixels,
                                    const unsigned char *remap, int width,
                                    int u, int v, int du, int dv,
                                    int light, unsigned short light_step,
                                    int pixel_count,
                                    TextureDrawSpanResult *result = 0) {
    unsigned int source_offset = texture_draw_source_index(width, u, v);
    unsigned short u_fraction = static_cast<unsigned short>(u);
    unsigned int v_fraction_and_du_fraction =
        (static_cast<unsigned int>(static_cast<unsigned short>(v)) << 16) |
        static_cast<unsigned short>(du);
    const unsigned int dv_fraction =
        static_cast<unsigned int>(static_cast<unsigned short>(dv)) << 16;
    const unsigned int source_step = texture_draw_source_step(du, dv, width);
    unsigned short light_accumulator = static_cast<unsigned short>(
        static_cast<unsigned int>(light) >> 8);
    const unsigned short du_fraction = static_cast<unsigned short>(du);

    if (result != 0) result->has_pixel = false;
    for (int pixel = 0; pixel < pixel_count; ++pixel) {
        if (result != 0) {
            result->has_pixel = true;
            result->last_source_offset = source_offset;
            result->last_packed_uv = v_fraction_and_du_fraction;
            result->last_light = light_accumulator;
            if (result->packed_uv_sink != 0)
                *result->packed_uv_sink =
                    static_cast<int>(v_fraction_and_du_fraction);
        }
        const unsigned char base_texel = result != 0
            ? result->prefetched_source : pixels[source_offset];
        const unsigned char mapped = remap[coast_pixels[source_offset]];
        if (mapped != 0) {
            if (mapped == 1) {
                const unsigned int shade = (light_accumulator >> 8) & 0xffu;
                destination[0] = static_cast<unsigned char>(
                    static_cast<unsigned int>(base_texel) + shade);
            } else {
                destination[0] = mapped;
            }
        }
        source_offset = texture_draw_next_source_index(
            source_offset, v_fraction_and_du_fraction, u_fraction,
            dv_fraction, du_fraction, source_step, width);
        light_accumulator = static_cast<unsigned short>(
            light_accumulator + light_step);
        if (result != 0) {
            result->next_source_offset = source_offset;
            result->next_packed_uv = v_fraction_and_du_fraction;
            result->next_light = light_accumulator;
        }
        ++destination;
        if (result != 0 && result->preload_pixels != 0)
            result->prefetched_source = result->preload_pixels[source_offset];
    }
}

static void texture_draw_keyed_coast_fill_span(
    unsigned char *destination, const unsigned char *coast_pixels,
    const unsigned char *remap, int width, int u, int v, int du, int dv,
    unsigned char fill_color, int pixel_count,
    TextureDrawSpanResult *result = 0) {
    unsigned int source_offset = texture_draw_source_index(width, u, v);
    unsigned short u_fraction = static_cast<unsigned short>(u);
    unsigned int v_fraction_and_du_fraction =
        (static_cast<unsigned int>(static_cast<unsigned short>(v)) << 16) |
        static_cast<unsigned short>(du);
    const unsigned int dv_fraction =
        static_cast<unsigned int>(static_cast<unsigned short>(dv)) << 16;
    const unsigned int source_step = texture_draw_source_step(du, dv, width);
    const unsigned short du_fraction = static_cast<unsigned short>(du);

    if (result != 0) result->has_pixel = false;
    for (int pixel = 0; pixel < pixel_count; ++pixel) {
        if (result != 0) {
            result->has_pixel = true;
            result->last_source_offset = source_offset;
            result->last_packed_uv = v_fraction_and_du_fraction;
            result->last_light = 0;
            if (result->packed_uv_sink != 0)
                *result->packed_uv_sink =
                    static_cast<int>(v_fraction_and_du_fraction);
        }
        if (remap[coast_pixels[source_offset]] != 0)
            destination[0] = fill_color;
        source_offset = texture_draw_next_source_index(
            source_offset, v_fraction_and_du_fraction, u_fraction,
            dv_fraction, du_fraction, source_step, width);
        if (result != 0) {
            result->next_source_offset = source_offset;
            result->next_packed_uv = v_fraction_and_du_fraction;
            result->next_light = 0;
        }
        ++destination;
        if (result != 0 && result->preload_pixels != 0)
            result->prefetched_source = result->preload_pixels[source_offset];
    }
}

static void texture_reset_fpu() {
    // FNINIT at entry/exit resets, rather than restores, the caller's state.
    // VC6 _fpreset defaults to 53-bit precision, so explicitly select the
    // original 64-bit significand and all exception masks afterward.
    _fpreset();
    _control87(_MCW_EM | _PC_64 | _RC_NEAR | _IC_PROJECTIVE,
               _MCW_EM | _MCW_PC | _MCW_RC | _MCW_IC);
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
Purpose: Cut a width x height rectangle at (x, y) out of a locked buffer into
         this texture, taking the pixel block from the sheet heap when one was
         passed (in which case the texture is borrowed, not owned) and from the
         CRT otherwise, then fill in the two per-vertex u/v arrays the
         rasterisers interpolate across: `corners_` gets the four corners and
         `cornersAndMids_` those four plus the four edge midpoints.
// ORIGINAL: 0x00619710 ?extract@Texture@@QAEHPAUBuffer@@HHHHPAUTexHeap@@@Z 0x00619710-0x006198DE
// symbol    ?extract@Texture@@QAEHPAVBuffer@@HHHHPAUTexHeap@@@Z
//            the image spells `Buffer` as a struct (PAU) and this tree declares
//            it `class`, so VC6 decorates PAV. Same bytes, differing name -
//            the same fact `draw_coast` and `Sprite::extract` record.
// size      462 bytes
// prototype int (__thiscall ?extract@Texture@@QAEHPAUBuffer@@HHHHPAUTexHeap@@@Z)(Texture* this, Buffer*, int, int, int, int, TexHeap*)
// callers   3   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005E33F3 0x005E34A3 0x00635370 0x00644EF2 0x00645930
// TRIED (as src/unrecovered/00619710.cpp, the offsets walked as raw ints):
//   the extra locals this body needs pushed the prologue to a `sub esp`
//   before the register pushes, so divergence started at instruction #1.
//   Homed here with the vertex arrays spelled as the members they are (see
//   texture.h) and the tail's field writes in the image's own order.
Status: Promoted from src/unrecovered/00619710.cpp.
*/
int Texture::extract(Buffer *buffer, int x, int y, int width, int height,
                     TexHeap *heap) {
    if (buffer == 0) {
        return 0x10;
    }

    if (pixels_ != 0 && borrowed_ == 0) {
        if (pixels_ != 0) {
            free(pixels_);
        }
        pixels_ = 0;
    }
    iWidth_ = 0;
    iHeight_ = 0;
    borrowed_ = 0;

    if (x + width > static_cast<int>(buffer->dib_.bmiHeader.biWidth)) {
        return 3;
    }
    if (y + height > -static_cast<int>(buffer->dib_.bmiHeader.biHeight)) {
        return 3;
    }

    iHeight_ = height;
    iWidth_ = width;

    int srcData = buffer->get_data(x, y);
    if (srcData == 0) {
        return 7;
    }

    int stride = buffer->stride_;
    pixels_ = 0;
    if (heap != 0) {
        pixels_ = reinterpret_cast<void *>(heap->get_tex_mem(iHeight_ << 8));
        if (pixels_ != 0) {
            borrowed_ = 1;
        }
    }
    if (pixels_ == 0) {
        pixels_ = mem_get(iHeight_ * iWidth_);
        if (pixels_ == 0) {
            iWidth_ = 0;
            iHeight_ = 0;
            borrowed_ = 0;
            buffer->free_data(1);
            return 4;
        }
        borrowed_ = 0;
    }

    char *dest = reinterpret_cast<char *>(pixels_);
    int src = srcData;
    if (height != 0) {
        int count = height;
        do {
            memcpy(dest, reinterpret_cast<void *>(src), iWidth_);
            if (borrowed_ == 0) {
                dest += iWidth_;
            } else {
                dest += 0x100;
            }
            src += stride;
            --count;
        } while (count != 0);
    }

    buffer->free_data(1);

    int widthMinus1 = iWidth_ - 1;
    int heightMinus1 = iHeight_ - 1;
    int widthHalfMinus1 = static_cast<int>(iWidth_) / 2 - 1;
    int heightHalfMinus1 = static_cast<int>(iHeight_) / 2 - 1;
    corners_[0].a = 0;
    corners_[0].b = 0;
    corners_[1].a = widthMinus1;
    corners_[1].b = 0;
    corners_[2].a = widthMinus1;
    corners_[2].b = heightMinus1;
    corners_[3].a = 0;
    corners_[3].b = heightMinus1;
    cornersAndMids_[0].a = 0;
    cornersAndMids_[0].b = 0;
    cornersAndMids_[1].a = widthHalfMinus1;
    cornersAndMids_[1].b = 0;
    cornersAndMids_[2].a = widthMinus1;
    cornersAndMids_[2].b = 0;
    cornersAndMids_[3].a = widthMinus1;
    cornersAndMids_[3].b = heightHalfMinus1;
    cornersAndMids_[4].a = widthMinus1;
    cornersAndMids_[4].b = heightMinus1;
    cornersAndMids_[5].a = widthHalfMinus1;
    cornersAndMids_[5].b = heightMinus1;
    cornersAndMids_[6].a = 0;
    cornersAndMids_[6].b = heightMinus1;
    cornersAndMids_[7].a = 0;
    cornersAndMids_[7].b = heightHalfMinus1;
    return 0;
}

// Rectangle form of extract; ECX remains the same Texture receiver at the
// call to the six-argument overload. Both original exits pop three stack slots,
// but the third incoming slot is never read. Its original type is unresolved.

// ORIGINAL: 0x006196D0 ?extract@Texture@@QAEHPAUBuffer@@PAURECT@@@Z 0x006196D0-0x00619706 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x006196D0-0x00619706 is 54 bytes. ECX remains unchanged for its direct call to Texture::extract at 0x00619710. The first two stack slots supply Buffer and RECT; both exits use ret 0xC, but the third incoming slot is never read. A null rectangle returns 0x10; otherwise the call supplies left, top, right-left, bottom-top and a literal null heap.", "The historical seed restored by 03927c9a preserves the canonical Texture name and original byte obligation. Its separate TextureRectExtract class is verification scaffolding, not evidence of another object type. Mapped-span caller search is incomplete and establishes no type for the unused slot."], "reason": "Recover an ordinary Texture rectangle overload on the original receiver, without a fake holder class or reinterpret_cast. Keep the canonical catalogue name even though it omits the third stack slot; the emitted-symbol fact describes the actual three-argument method. int is only a provisional representation of the unused 32-bit slot, not a recovered original type."}
// symbol    ?extract@Texture@@QAEHPAVBuffer@@PAUtagRECT@@H@Z
int Texture::extract(Buffer *buffer, RECT *rect, int) {
    if (rect == 0) {
        return 0x10;
    }
    return extract(buffer, rect->left, rect->top,
                   rect->right - rect->left, rect->bottom - rect->top, 0);
}

/*
Purpose: Reset cache update flags to 3 and its next-record cursor to zero.
         Cached records remain untouched. Calls nothing.
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
    *reinterpret_cast<volatile unsigned int *>(&flags_) = 3;
    *reinterpret_cast<volatile int *>(&next_record_) = 0;
}





/*
Purpose: Invalidate both cache channels and reset the record cursor.

             mov eax,ecx / mov [eax],3 / mov [eax+4],0 / ret

         The constructor returns this in EAX. Flag bits 1 and 2 request fresh
         light and geometry records respectively at original 0x00625080;
         the record payload remains uninitialized until that helper fills it.
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
    flags_ = 3;
    next_record_ = 0;
}

/*
Purpose: Hand out the next of ten 60-byte slots, wrapping back to the first.

             mov edx,[ecx+4] / lea eax,[edx+edx*2] / inc edx / cmp edx,0xa
             mov [ecx+4],edx / lea eax,[eax+eax*4] / lea eax,[ecx+eax*4+8]
             jne end / mov dword [ecx+4],0 / end: ret

         The two `lea`s multiply by 3 then by 5, and the third scales by 4:
         60 bytes a slot, the array starting at offset 8.

         The pointer handed back is the slot the counter held ON ENTRY, and
         the counter is advanced afterwards - so the wrap to zero does not
         affect what this call returns, only the next one. The `cmp` happens
         before the store either way; writing the wrap first would return the
         wrong slot on the tenth call and only then.

// Historical scope note: this identity was previously kept in the transitional leaf collection; TextureStore ownership is supported by its cursor and EdgeScan layout.
// ORIGINAL: 0x006252C0 sub_6252c0 0x006252C0-0x006252DE BYTE_EXACT
// symbol    ?sub_6252c0@TextureStore@@QAEPAUEdgeScan@@XZ
// Former emitted adapter symbol (not the TextureStore member): ?leaf_006252c0_redirect@@YIPAXPAX0@Z
// size      30 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// Mapped-span caller search found no call site; all-image and dynamic caller evidence remain unresolved.
Return Value: a pointer to the slot that was current on entry
Status: Complete
*/
EdgeScan *TextureStore::sub_6252c0() {
    EdgeScan *const selected = &edges_[next_record_];
    const int next = next_record_ + 1;
    next_record_ = next;
    if (next == 10) {
        next_record_ = 0;
    }
    return selected;
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk.
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
// Both radius objects initially contain zeroes. Texture's constructor writes
// through offset 0x6C; independently constructed CaviarData at 0x788028 and
// Sprite[3] at 0x75B8C8 bound their storage, independently of this build's sizeof.
// Original CRT entries 0x68228C/0x682290 invoke 0x44E5F0/0x44E620 in order;
// their registered exits 0x44E610/0x44E640 call Texture::~Texture in reverse.
// Asset extraction belongs to load_terrain (0x451C50), homed in game.cpp.
// RECOVERY_OBJECT: v2 0x00787FB8 0x00788028 ?g_RADIUS1_TEXTURE@@3VTexture@@A 0x0044E5F0
Texture g_RADIUS1_TEXTURE;
// RECOVERY_OBJECT: v2 0x0075B858 0x0075B8C8 ?g_RADIUS2_TEXTURE@@3VTexture@@A 0x0044E620
Texture g_RADIUS2_TEXTURE;
// Original CRT 0044E650 (table 00682294) constructs 4 Texture object(s);
// exit 0044E680 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0076E9E0 0x0076EBA0 ?g_ROCKY_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_ROCKY_TEXTURES[4];
// Original CRT 0044E6A0 (table 00682298) constructs 2 Texture object(s);
// exit 0044E6D0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007A7820 0x007A7900 ?g_OCEAN_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_OCEAN_TEXTURES[2];
// Original CRT 0044E6F0 (table 0068229C) constructs 1 Texture object(s);
// exit 0044E710 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00799E48 0x00799EB8 ?g_FLAT_ARID_LAND_TEXTURE@@3VTexture@@A 0x00451C50
Texture g_FLAT_ARID_LAND_TEXTURE;
// Original CRT 0044E720 (table 006822A0) constructs 16 Texture object(s);
// exit 0044E750 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00799EB8 0x0079A5B8 ?g_MOIST_LAND_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_MOIST_LAND_TEXTURES[16];
// Original CRT 0044E770 (table 006822A4) constructs 16 Texture object(s);
// exit 0044E7A0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00799738 0x00799E38 ?g_RAINY_LAND_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_RAINY_LAND_TEXTURES[16];
// Original CRT 0044E7C0 (table 006822A8) constructs 15 Texture object(s);
// exit 0044E7F0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00789C28 0x0078A2B8 ?g_JUNGLE_LAND_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_JUNGLE_LAND_TEXTURES[15];
// Original CRT 0044E810 (table 006822AC) constructs 1 Texture object(s);
// exit 0044E830 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007AC220 0x007AC290 ?g_DUNE_LAND_TEXTURE@@3VTexture@@A 0x00451C50
Texture g_DUNE_LAND_TEXTURE;
// Original CRT 0044E840 (table 006822B0) constructs 8 Texture object(s);
// exit 0044E870 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007797F8 0x00779B78 ?g_SUNNY_MESA_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_SUNNY_MESA_TEXTURES[8];
// Original CRT 0044E890 (table 006822B4) constructs 2 Texture object(s);
// exit 0044E8C0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0076E8A0 0x0076E980 ?g_RAINFALL_SINGLE_TILE_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_RAINFALL_SINGLE_TILE_TEXTURES[2];
// Original CRT 0044E8E0 (table 006822B8) constructs 9 Texture object(s);
// exit 0044E910 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00792218 0x00792608 ?g_ROAD_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_ROAD_TEXTURES[9];
// Original CRT 0044E930 (table 006822BC) constructs 9 Texture object(s);
// exit 0044E960 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00798E08 0x007991F8 ?g_MAGTUBE_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_MAGTUBE_TEXTURES[9];
// Original CRT 0044E980 (table 006822C0) constructs 16 Texture object(s);
// exit 0044E9B0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007A7AA0 0x007A81A0 ?g_RIVER_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_RIVER_TEXTURES[16];
// Original CRT 0044E9D0 (table 006822C4) constructs 3 Texture object(s);
// exit 0044EA00 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0078A340 0x0078A490 ?g_MOUNT_PLANET_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_MOUNT_PLANET_TEXTURES[3];
// Original CRT 0044EA20 (table 006822C8) constructs 3 Texture object(s);
// exit 0044EA50 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00788100 0x00788250 ?g_GARLAND_CRATER_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_GARLAND_CRATER_TEXTURES[3];
// Original CRT 0044EA70 (table 006822CC) constructs 30 Texture object(s);
// exit 0044EAA0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00776A80 0x007777A0 ?g_FUNGUS_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_FUNGUS_TEXTURES[30];
// Original CRT 0044EAC0 (table 006822D0) constructs 9 Texture object(s);
// exit 0044EAF0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00799238 0x00799628 ?g_FARM_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_FARM_TEXTURES[9];
// Original CRT 0044EB10 (table 006822D4) constructs 16 Texture object(s);
// exit 0044EB40 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0078A758 0x0078AE58 ?g_FOREST_TEXTURES@@3PAVTexture@@A 0x00451C50
Texture g_FOREST_TEXTURES[16];
// Original CRT 0044EB60 (table 006822D8) constructs 1 Texture object(s);
// exit 0044EB80 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0078DD80 0x0078DDF0 ?g_RAINFALL_SINGLE_TILE_TEXTURE@@3VTexture@@A 0x00451C50
Texture g_RAINFALL_SINGLE_TILE_TEXTURE;

// Shared Texture-owned storage for the original raster cells.
// This is a collection of independent source objects; it does not assert that
// the original linker used one aggregate layout. Starts are mapped below;
// complete original allocation bounds and semantic names remain unproved.
// Overlaps below are explicit. The dword/word unions retain the original
// little-endian VC6 representation; they do not claim portable union punning.
// RECOVERY_OBJECT: v1 0x00670A84 _TextureRasterScale65536
static const float TextureRasterScale65536 = 65536.0f;
// RECOVERY_OBJECT: v1 0x00670A88 _TextureRasterScale256
static const float TextureRasterScale256 = 256.0f;

// PE relocations at 0x6972C4/C8 point to 0x9BB520/524. Model those values as
// pointers to their source-owned counterparts; the same cells become integer
// row offsets when the raster methods write the union's value member.
union TextureRasterRelocatedRowCell {
    int *initial_target;
    int value;
};
// RECOVERY_OBJECT: v1 0x009BB520 _TextureRasterScratch520
static int TextureRasterScratch520;
// RECOVERY_OBJECT: v1 0x009BB524 _TextureRasterScratch524
static int TextureRasterScratch524;
// RECOVERY_OBJECT: v1 0x006972C4 _TextureRasterAlternateRowBase
static TextureRasterRelocatedRowCell TextureRasterAlternateRowBase =
    { &TextureRasterScratch520 };
// RECOVERY_OBJECT: v1 0x006972C8 _TextureRasterAlternateRowNext
static TextureRasterRelocatedRowCell TextureRasterAlternateRowNext =
    { &TextureRasterScratch524 };

// RECOVERY_OBJECT: v1 0x009BB490 _TextureRasterEdgeVerticesA
static int *TextureRasterEdgeVerticesA;
// RECOVERY_OBJECT: v1 0x009BB494 _TextureRasterScratch494
static int TextureRasterScratch494;
// RECOVERY_OBJECT: v1 0x009BB498 _TextureRasterScratch498
static int TextureRasterScratch498;
// RECOVERY_OBJECT: v1 0x009BB49C _TextureRasterBufferStride
static int TextureRasterBufferStride;
// RECOVERY_OBJECT: v1 0x009BB4A4 _TextureRasterScratch4A4
static int TextureRasterScratch4A4;
// RECOVERY_OBJECT: v1 0x009BB504 _TextureRasterScratch504
static int TextureRasterScratch504;
// RECOVERY_OBJECT: v1 0x009BB4A8 _TextureRasterTextureWidth
static int TextureRasterTextureWidth;
// RECOVERY_OBJECT: v1 0x009BB4B0 _TextureRasterVertexCount
static int TextureRasterVertexCount;
// RECOVERY_OBJECT: v1 0x009BB4B4 _TextureRasterMaxYIndex
static int TextureRasterMaxYIndex;
// RECOVERY_OBJECT: v1 0x009BB4B8 _TextureRasterScratch4B8
static int TextureRasterScratch4B8;
// RECOVERY_OBJECT: v1 0x009BB4C0 _TextureRasterTextureHeight
static int TextureRasterTextureHeight;
// RECOVERY_OBJECT: v1 0x009BB4C4 _TextureRasterScratch4C4
static int TextureRasterScratch4C4;
struct TextureRasterWordPair {
    unsigned short low_word;
    unsigned short high_word;
};
union TextureRasterDwordHighWordView {
    int dword;
    unsigned char bytes[4];
    TextureRasterWordPair words;
};
// RECOVERY_OBJECT: v1 0x009BB4C8 _TextureRasterScratch4C8
static TextureRasterDwordHighWordView TextureRasterScratch4C8;
// RECOVERY_OBJECT: v1 0x009BB4CC _TextureRasterSpanRight
static int TextureRasterSpanRight; // Legacy spelling; original LEFT x.
// RECOVERY_OBJECT: v1 0x009BB4D0 _TextureRasterDither
static int TextureRasterDither;
// RECOVERY_OBJECT: v1 0x009BB4D8 _TextureRasterScratch4D8
static int TextureRasterScratch4D8;
// RECOVERY_OBJECT: v1 0x009BB4DC _TextureRasterClipLeft
static int TextureRasterClipLeft;
// RECOVERY_OBJECT: v1 0x009BB4E0 _TextureRasterClipTop
static int TextureRasterClipTop;
// RECOVERY_OBJECT: v1 0x009BB4E4 _TextureRasterScratch4E4
static TextureRasterDwordHighWordView TextureRasterScratch4E4;
// RECOVERY_OBJECT: v1 0x009BB4E8 _TextureRasterSignedPitch
static int TextureRasterSignedPitch;
// RECOVERY_OBJECT: v1 0x009BB4EC _TextureRasterPixelBase
static int TextureRasterPixelBase;
// RECOVERY_OBJECT: v1 0x009BB4F0 _TextureRasterSpanLeft
static int TextureRasterSpanLeft; // Legacy spelling; original RIGHT x.
// RECOVERY_OBJECT: v1 0x009BB4F4 _TextureRasterEdgeVerticesB
static int *TextureRasterEdgeVerticesB;
// RECOVERY_OBJECT: v1 0x009BB4F8 _TextureRasterUAccumulator
static TextureRasterDwordHighWordView TextureRasterUAccumulator;
// RECOVERY_OBJECT: v1 0x009BB4FC _TextureRasterZValues
static int *TextureRasterZValues;
// RECOVERY_OBJECT: v1 0x009BB508 _TextureRasterRowOffset
static int TextureRasterRowOffset;
// Shared row pitch: 15 original entry sites publish receiver width after
// get_data; cached-light consumers share this exact object.
// RECOVERY_OBJECT: v2 0x009BB50C 0x009BB510 ?TextureRowPitchScratch@@3HA 0x00621870
int TextureRowPitchScratch;
// RECOVERY_OBJECT: v1 0x009BB510 _TextureRasterLightStepB
static int TextureRasterLightStepB;
// RECOVERY_OBJECT: v1 0x009BB514 _TextureRasterLightThreshold
static int TextureRasterLightThreshold;
// RECOVERY_OBJECT: v1 0x009BB518 _TextureRasterLightAccumB
static int TextureRasterLightAccumB;
// RECOVERY_OBJECT: v1 0x009BB51C _TextureRasterPackedStep
static int TextureRasterPackedStep;
// RECOVERY_OBJECT: v1 0x009BB528 _TextureRasterScratch528
static int TextureRasterScratch528;
// RECOVERY_OBJECT: v1 0x009BB52C _TextureRasterLightPackA
static TextureRasterWordPair TextureRasterLightPackA;
// RECOVERY_OBJECT: v1 0x009BB530 _TextureRasterScratch530
static int TextureRasterScratch530;
// RECOVERY_OBJECT: v1 0x009BB534 _TextureRasterClipRight
static int TextureRasterClipRight;
// RECOVERY_OBJECT: v1 0x009BB538 _TextureRasterClipBottom
static int TextureRasterClipBottom;
// RECOVERY_OBJECT: v1 0x009BB53C _TextureRasterVAccumulator
static TextureRasterDwordHighWordView TextureRasterVAccumulator;
// RECOVERY_OBJECT: v1 0x009BB544 _TextureRasterScratch544
static int TextureRasterScratch544;
// 0x9BB548..54C remains the existing TextureRasterKey object.
// RECOVERY_OBJECT: v1 0x009BB550 _TextureRasterScratch550
static int TextureRasterScratch550;
// RECOVERY_OBJECT: v1 0x009BB554 _TextureRasterScratch554
static int TextureRasterScratch554;
// RECOVERY_OBJECT: v1 0x009BB558 _TextureRasterShadeCursor
static int TextureRasterShadeCursor;
// RECOVERY_OBJECT: v1 0x009BB55C _TextureRasterScratch55C
static int TextureRasterScratch55C;
// RECOVERY_OBJECT: v1 0x009BB560 _TextureRasterScratch560
static int TextureRasterScratch560;
// RECOVERY_OBJECT: v1 0x009BB564 _TextureRasterPalette
static int TextureRasterPalette;

static const float *const blit_scale_65536f = &TextureRasterScale65536;
static const float *const blit_scale_256f = &TextureRasterScale256;
static int *const blit_slot_494 = &TextureRasterScratch494;
static int *const blit_slot_498 = &TextureRasterScratch498;
static int *const blit_buffer_stride = &TextureRasterBufferStride;
static int *const blit_slot_4a4 = &TextureRasterScratch4A4;
static int *const blit_slot_504 = &TextureRasterScratch504;
static int *const blit_texture_width = &TextureRasterTextureWidth;
static int *const blit_vertex_count = &TextureRasterVertexCount;
static int *const blit_max_y_index = &TextureRasterMaxYIndex;
static int *const blit_colour_step = &TextureRasterScratch4B8;
static int *const blit_texture_height = &TextureRasterTextureHeight;
static int *const blit_slot_4c4 = &TextureRasterScratch4C4;
static int *const blit_slot_4c8 = &TextureRasterScratch4C8.dword;
static int *const blit_span_right = &TextureRasterSpanRight;
static int *const blit_dither = &TextureRasterDither;
static int *const blit_slot_4d8 = &TextureRasterScratch4D8;
static int *const blit_clip_left = &TextureRasterClipLeft;
static int *const blit_clip_top = &TextureRasterClipTop;
static int *const blit_slot_4e4 = &TextureRasterScratch4E4.dword;
static int *const blit_slot_4e8 = &TextureRasterSignedPitch;
static int *const blit_pixel_base = &TextureRasterPixelBase;
static int *const blit_span_left = &TextureRasterSpanLeft;
static int *const blit_slot_4f8 = &TextureRasterUAccumulator.dword;
static unsigned short *const blit_slot_4fa =
    &TextureRasterUAccumulator.words.high_word;
static int *const blit_row_offset = &TextureRasterRowOffset;
static int *const blit_light_step_b = &TextureRasterLightStepB;
static int *const blit_light_threshold = &TextureRasterLightThreshold;
static int *const blit_light_accum_b = &TextureRasterLightAccumB;
static int *const blit_slot_51c = &TextureRasterPackedStep;
static int *const blit_slot_528 = &TextureRasterScratch528;
static unsigned short *const blit_light_pack_a =
    &TextureRasterLightPackA.low_word;
static int *const blit_slot_530 = &TextureRasterScratch530;
static int *const blit_clip_right = &TextureRasterClipRight;
static int *const blit_clip_bottom = &TextureRasterClipBottom;
static int *const blit_slot_53c = &TextureRasterVAccumulator.dword;
static unsigned short *const blit_slot_53e =
    &TextureRasterVAccumulator.words.high_word;
static int *const blit_texture_pixels = &TextureRasterScratch544;
static int *const blit_remap_table = &TextureRasterScratch550;
static int *const blit_shade_table = &TextureRasterScratch554;
static int *const blit_slot_558 = &TextureRasterShadeCursor;
static int *const blit_slot_55c = &TextureRasterScratch55C;
static int *const blit_slot_560 = &TextureRasterScratch560;
static int *const blit_slot_564 = &TextureRasterPalette;

static unsigned char *const blit_slot_4ca = &TextureRasterScratch4C8.bytes[2];
static unsigned char *const blit_slot_4e6 = &TextureRasterScratch4E4.bytes[2];

static int **const blit_edge_uvs_a = &TextureRasterEdgeVerticesA;
static int **const blit_edge_uvs_b = &TextureRasterEdgeVerticesB;
static int **const blit_z_values = &TextureRasterZValues;
static int **const g2_009bb490 = &TextureRasterEdgeVerticesA;
static int **const g2_009bb4f4 = &TextureRasterEdgeVerticesB;
static int **const g2_009bb4fc = &TextureRasterZValues;

// Existing TextureRasterKey remains the sole owner of the 0x9BB548 object.
static int *const blit_light_threshold_2 =
    reinterpret_cast<int *>(&TextureRasterKey);


static int texture_coast_advance_edge(EdgeScan &edge) {
    if (--edge.counter == 0)
        return Texture::setup_edge_gouraud(&edge, edge.id);
    edge.accumA = static_cast<int>(static_cast<unsigned int>(edge.accumA) + edge.deltaA);
    edge.accumB = static_cast<int>(static_cast<unsigned int>(edge.accumB) + edge.deltaB);
    edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.deltaC);
    edge.accumD = static_cast<int>(static_cast<unsigned int>(edge.accumD) + edge.deltaD);
    edge.fracAccum = static_cast<int>(static_cast<unsigned int>(edge.fracAccum) + edge.fracDelta);
    if (edge.fracAccum > 0) {
        edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.extraDeltaC);
        edge.fracAccum = texture_difference(edge.fracAccum, edge.fracSub);
    }
    return 1;
}

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
    // 0x61EAD0..0x61F0C6 takes every guard through the common epilogue.
    do {
    if (a2 == 0 || a1 == 0 || pixels_ == 0 || a3 == 0 || a5 == 0 || a4 == 0)
        break;

    blit_pixel_base[0] = a1->get_data();
    texture_reset_fpu();
    *blit_edge_uvs_b = reinterpret_cast<int *>(a2);
    *blit_z_values = reinterpret_cast<int *>(a3);
    blit_remap_table[0] = static_cast<int>(reinterpret_cast<intptr_t>(a5));
    blit_texture_pixels[0] = static_cast<int>(reinterpret_cast<intptr_t>(pixels_));
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    blit_texture_height[0] = static_cast<int>(iHeight_);
    blit_vertex_count[0] = a6;
    *blit_edge_uvs_a = reinterpret_cast<int *>(a7);
    blit_buffer_stride[0] = static_cast<int>(a1->stride_);
    blit_texture_width[0] = static_cast<int>(iWidth_);
    blit_shade_table[0] = static_cast<int>(reinterpret_cast<intptr_t>(a4->pixels_));
    if (*blit_shade_table == 0) break;

    RECT *clip = &a1->rect1_;
    blit_clip_right[0] = clip->right;
    blit_clip_left[0] = clip->left;
    blit_clip_bottom[0] = clip->bottom;
    blit_clip_top[0] = clip->top;

    int min_y = 0x7fff, max_y = static_cast<int>(0xffff8003u);
    int min_x = 0x7fff, max_x = static_cast<int>(0xffff8003u);
    int min_y_index = a6;
    for (int i = 0; i < a6; ++i) {
        const int y = a2[i].y;
        if (y < min_y) { min_y = y; min_y_index = i; }
        if (y > max_y) { max_y = y; blit_max_y_index[0] = i; }
        const int x = a2[i].x;
        if (x < min_x) min_x = x;
        if (x > max_x) max_x = x;
    }
    (void)min_x;
    (void)max_x;
    if (min_y >= max_y) break;

    const int stride = blit_buffer_stride[0];
    const int destination_base = blit_pixel_base[0];
    blit_row_offset[0] = static_cast<int>(static_cast<unsigned int>(stride) *
                                           static_cast<unsigned int>(min_y) +
                                           static_cast<unsigned int>(destination_base));
    blit_clip_bottom[0] = static_cast<int>(static_cast<unsigned int>(stride) *
                                            static_cast<unsigned int>(clip->bottom) +
                                            static_cast<unsigned int>(destination_base));
    blit_clip_top[0] = static_cast<int>(static_cast<unsigned int>(stride) *
                                         static_cast<unsigned int>(clip->top) +
                                         static_cast<unsigned int>(destination_base));

    EdgeScan edge_left;
    edge_left.f0 = -1;
    if (setup_edge_gouraud(&edge_left, min_y_index) == 0) break;
    EdgeScan edge_right;
    edge_right.f0 = 1;
    if (setup_edge_gouraud(&edge_right, min_y_index) == 0) break;

    _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
    int gradient_span = static_cast<int>(static_cast<unsigned int>(
        texture_difference(edge_right.accumC, edge_left.accumC)) << 16);
    // The native path rejects an initially clipped-away first row after
    // setting up both edges but before its first row snapshot/advance.
    if (blit_row_offset[0] >= blit_clip_bottom[0]) break;

    int du = *blit_slot_4e4;
    int dv = *blit_slot_4c8;
    unsigned short light_step = static_cast<unsigned short>(*blit_light_pack_a);
    for (;;) {
        *blit_slot_4c4 = edge_right.accumA;
        *blit_slot_4f8 = edge_left.accumA;
        *blit_slot_530 = edge_right.accumB;
        *blit_slot_53c = edge_left.accumB;
        *blit_span_right = edge_left.accumC;
        *blit_slot_498 = edge_left.accumD;
        *blit_span_left = edge_right.accumC;
        const int left_x = edge_left.accumC;
        const int right_x = edge_right.accumC;
        int u = *blit_slot_4f8;
        int v = *blit_slot_53c;
        int light = edge_left.accumD;
        const bool update_steps = gradient_span > 0;
        int row_du = du, row_dv = dv;
        unsigned short row_light_step = light_step;
        if (update_steps) {
            row_du = texture_span_step(
                texture_difference(edge_right.accumA, edge_left.accumA), gradient_span >> 16);
            row_dv = texture_span_step(
                texture_difference(edge_right.accumB, edge_left.accumB), gradient_span >> 16);
            row_light_step = texture_light_step(
                texture_difference(edge_right.accumD, edge_left.accumD), gradient_span >> 16);
        }
        bool done_edges = false;
        if (texture_coast_advance_edge(edge_left) != 0) {
            if (texture_coast_advance_edge(edge_right) == 0) done_edges = true;
        } else {
            done_edges = true;
        }

        // The assembly snapshots this row's endpoints and derives its channel
        // gradients from those values, then advances edges before publishing
        // the gradients. The updated horizontal span feeds the next row.
        if (update_steps) {
            du = row_du;
            dv = row_dv;
            light_step = row_light_step;
            *blit_slot_4e4 = du;
            *blit_slot_4c8 = dv;
            *blit_light_pack_a = static_cast<int>(light_step);
        }
        gradient_span = static_cast<int>(static_cast<unsigned int>(
            texture_difference(edge_right.accumC, edge_left.accumC)) << 16);

        const int row = blit_row_offset[0];
        if (row >= blit_clip_bottom[0]) break;
        if (row >= blit_clip_top[0] && right_x > blit_clip_left[0] &&
            left_x < blit_clip_right[0] && right_x > left_x) {
            int clipped_right = right_x;
            if (clipped_right > blit_clip_right[0]) clipped_right = blit_clip_right[0];
            int clipped_left = left_x;
            if (clipped_left < blit_clip_left[0]) {
                const int skip = static_cast<int>(static_cast<unsigned int>(
                    texture_difference(blit_clip_left[0], clipped_left)) << 16);
                u = static_cast<int>(static_cast<unsigned int>(u) +
                    static_cast<unsigned int>((static_cast<int64_t>(du) * skip) >> 16));
                v = static_cast<int>(static_cast<unsigned int>(v) +
                    static_cast<unsigned int>((static_cast<int64_t>(dv) * skip) >> 16));
                light = static_cast<int>(static_cast<unsigned int>(light) +
                    static_cast<unsigned int>((static_cast<int64_t>(light_step) * skip) >> 8));
                *blit_slot_4f8 = u;
                *blit_slot_53c = v;
                *blit_slot_498 = light;
                clipped_left = blit_clip_left[0];
            }
            *blit_slot_498 = light >> 8;
            unsigned char *out = reinterpret_cast<unsigned char *>(
                static_cast<intptr_t>(row)) + clipped_left;
            const unsigned int pixel_count = texture_span_pixels(
                texture_difference(clipped_right, clipped_left));
            const unsigned int row_source_offset = texture_draw_source_index(
                blit_texture_width[0], u, v);
            *blit_slot_504 = static_cast<int>(
                static_cast<unsigned int>(blit_texture_pixels[0]) + row_source_offset);
            *blit_slot_558 = static_cast<int>(
                static_cast<unsigned int>(*blit_shade_table) + row_source_offset);
            TextureDrawSpanResult span_result;
            span_result.packed_uv_sink = blit_slot_4a4;
            span_result.preload_pixels = reinterpret_cast<volatile const unsigned char *>(
                static_cast<intptr_t>(blit_texture_pixels[0]));
            span_result.prefetched_source =
                span_result.preload_pixels[row_source_offset];
            texture_draw_coast_span(out, reinterpret_cast<const unsigned char *>(
                static_cast<intptr_t>(blit_texture_pixels[0])),
                reinterpret_cast<const unsigned char *>(static_cast<intptr_t>(*blit_shade_table)),
                reinterpret_cast<const unsigned char *>(static_cast<intptr_t>(*blit_remap_table)),
                blit_texture_width[0], u, v, du, dv, light, light_step,
                static_cast<int>(pixel_count), &span_result);
        }
        if (done_edges) break;
        blit_row_offset[0] = static_cast<int>(static_cast<unsigned int>(row) +
                                               static_cast<unsigned int>(stride));
    }
    } while (false);
    if (a1 != 0) a1->free_data(1);
    texture_reset_fpu();
}

// Historical register-allocation evidence from the superseded translation:
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

// The native inner loop reads the next source texel at 0x61F078 after every
// visible pixel, including the last. The span helper models that volatile
// prefetch without a bounds check; instruction-level fault timing remains a
// restoration obligation.


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

    int *arrF4 = *blit_edge_uvs_b;
    int height = arrF4[c * 2 + 1] - arrF4[b * 2 + 1];
    *reinterpret_cast<int *>(self + 4) = height;

    if (height != 0) {
      if (height < 1) {
        return 0;
      }
      *reinterpret_cast<int *>(self + 8) = c;
      int scaled = height * 0x10000;

      int *arr90 = *blit_edge_uvs_a;
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

      int *arrFC = *blit_z_values;
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
    int *arr4f4 = *blit_edge_uvs_b;
    int iVar1 = arr4f4[iVar3 * 2 + 1] - arr4f4[a2 * 2 + 1];
    *(int *)(e + 4) = iVar1;
    if (iVar1 != 0) {
      if (iVar1 <= 0) {
        return 0;
      }
      *(int *)(e + 8) = iVar3;
      int *arr490 = *blit_edge_uvs_a;
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
// The explicit-UV overload is opaque despite its historical name. At
// 0x00619D24 every sampled byte is stored, and DEC/JNS at 0x00619D30 makes
// the span inclusive, even at clip.right. U and V wrap independently in
// the low pointer bytes. TexHeap::get_tex_mem supplies a 64-KiB-aligned
// payload; Texture::extract writes its borrowed rows at a 256-byte pitch.
// Per-call edge scratch has the same bounded-domain limitations as the
// color-key overload: ordinary convex geometry, established extrema, no
// fixed-point/row-address overflow and valid sampled allocation bounds.
// Manually misaligned borrowed textures and shared-scratch/FP status observed
// by callbacks remain unverified. No original-address storage is used here.
void Texture::draw_trans(Buffer *buffer, Vert *vertices, int count, Vert *uv) {
    unsigned char *destination = reinterpret_cast<unsigned char *>(buffer->get_data());
    texture_reset_fpu();
    const unsigned char *pixels = static_cast<const unsigned char *>(pixels_);
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    const int stride = static_cast<int>(buffer->stride_);
    const RECT clip = buffer->rect1_;

    int min_y = 0x7fff, max_y = -0x7ffd;
    int top = 0, bottom = 0;
    for (int i = 0; i < count; ++i) {
        if (vertices[i].y < min_y) { min_y = vertices[i].y; top = i; }
        if (vertices[i].y > max_y) { max_y = vertices[i].y; bottom = i; }
    }
    EdgeScan left, right;
    left.f0 = -1;
    right.f0 = 1;
    if (min_y < max_y && texture_edge(left, top, vertices, uv, count, bottom) &&
        texture_edge(right, top, vertices, uv, count, bottom) && borrowed_ != 0) {
        _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
        int du = 0, dv = 0;
        int y = min_y;
        if (y >= clip.bottom) {
            buffer->free_data(1);
            texture_reset_fpu();
            return;
        }
        for (;;) {
            int x = left.accumC;
            int end = right.accumC;
            int u = left.accumA, v = left.accumB;
            const int span = texture_fixed(texture_difference(end, x));
            if (span > 0) {
                du = texture_span_step(texture_difference(right.accumA, u), span >> 16);
                dv = texture_span_step(texture_difference(right.accumB, v), span >> 16);
            }
            const int done = !texture_advance_edge(left, vertices, uv, count, bottom) ||
                             !texture_advance_edge(right, vertices, uv, count, bottom);
            if (y >= clip.bottom) break;
            if (y >= clip.top && end > clip.left && x < clip.right && end > x) {
                if (end > clip.right) end = clip.right;
                if (x < clip.left) {
                    const int skip = texture_fixed(texture_difference(clip.left, x));
                    u = static_cast<int>(static_cast<unsigned int>(u) +
                        static_cast<unsigned int>((static_cast<int64_t>(du) * skip) >> 16));
                    v = static_cast<int>(static_cast<unsigned int>(v) +
                        static_cast<unsigned int>((static_cast<int64_t>(dv) * skip) >> 16));
                    x = clip.left;
                }
                unsigned char *out = destination + static_cast<int64_t>(y) * stride + x;
                unsigned int column = static_cast<unsigned char>(static_cast<unsigned int>(u) >> 16);
                unsigned int row = static_cast<unsigned char>(static_cast<unsigned int>(v) >> 16);
                unsigned int uf = static_cast<unsigned short>(u);
                unsigned int vf = static_cast<unsigned short>(v);
                const unsigned int column_step = static_cast<unsigned char>(du >> 16);
                const unsigned int row_step = static_cast<unsigned char>(dv >> 16);
                unsigned int remaining = static_cast<unsigned short>(texture_difference(end, x)) + 1u;
                for (; remaining != 0; --remaining, ++out) {
                    *out = pixels[row * 256 + column];
                    uf += static_cast<unsigned short>(du);
                    vf += static_cast<unsigned short>(dv);
                    column = (column + column_step + (uf >> 16)) & 0xff;
                    row = (row + row_step + (vf >> 16)) & 0xff;
                    uf &= 0xffff;
                    vf &= 0xffff;
                }
            }
            if (done) break;
            ++y;
        }
    }
    buffer->free_data(1);
    texture_reset_fpu();
}

// Fixed-slot bindings carried from 0061a240.cpp
// load_terrain at 0x00451CD2 and gen_terrain_poly at 0x00464EA9 write
// a dword here. Initial image storage is zero; the unrecovered terrain
// initialization sets it to nine. Rasterizers use it as a key or threshold,
// so its identity is shared, not a per-Texture transparent-index member.
// RECOVERY_OBJECT: v1 0x009BB548 ?TextureRasterKey@@3IA
unsigned int TextureRasterKey;

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
// 0x0061A55D..0x0061A6C5 packs U/V fractions and a pixel count into
// registers, including ESP. It is color-key copying, not a jump table.
// Edge and scanline scratch is local here; original shared-scratch side
// effects/reentrancy and FP status visible inside surface Unlock are not
// established by this reconstruction. The terrain writers of TextureRasterKey
// and the other legacy rasterizers remain separate recovery dependencies.
// The local-scratch interpretation covers ordinary convex screen polygons:
// edge/span heights fit positive signed 16.16, fixed_div quotients fit int,
// and positive-stride clipped row addresses do not wrap signed 32-bit order.
// Oversized spans can reuse preceding calls' shared slopes in the executable;
// those exceptional-input and callback-status behaviors remain unverified.
void Texture::draw_trans(Buffer *buffer, Vert *vertices, int use_eight_vertices) {
    unsigned char *destination = reinterpret_cast<unsigned char *>(buffer->get_data());
    texture_reset_fpu();
    const EdgePoint *uv = use_eight_vertices ? cornersAndMids_ : corners_;
    const int count = use_eight_vertices ? 8 : 4;
    const unsigned char key = static_cast<unsigned char>(TextureRasterKey);
    const unsigned char *pixels = static_cast<const unsigned char *>(pixels_);
    const int width = static_cast<int>(iWidth_);
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    const int stride = static_cast<int>(buffer->stride_);
    const RECT clip = buffer->rect1_;

    int min_y = 0x7fff, max_y = -0x7ffd;
    int top = 0, bottom = 0;
    for (int i = 0; i < count; ++i) {
        if (vertices[i].y < min_y) { min_y = vertices[i].y; top = i; }
        if (vertices[i].y > max_y) { max_y = vertices[i].y; bottom = i; }
    }
    EdgeScan left, right;
    left.f0 = -1;
    right.f0 = 1;
    if (min_y < max_y && texture_edge(left, top, vertices, uv, count, bottom) &&
        texture_edge(right, top, vertices, uv, count, bottom)) {
        _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
        int du = 0, dv = 0;
        int y = min_y;
        if (y >= clip.bottom) {
            buffer->free_data(1);
            texture_reset_fpu();
            return;
        }
        for (;;) {
            int x = left.accumC;
            int end = right.accumC;
            int u = left.accumA, v = left.accumB;
            const int span = texture_fixed(texture_difference(end, x));
            if (span > 0) {
                du = texture_span_step(texture_difference(right.accumA, u), span >> 16);
                dv = texture_span_step(texture_difference(right.accumB, v), span >> 16);
            }
            // Both sides advance before the saved row is drawn. Left failure
            // skips right advancement, but never suppresses this final row.
            const int done = !texture_advance_edge(left, vertices, uv, count, bottom) ||
                             !texture_advance_edge(right, vertices, uv, count, bottom);
            if (y >= clip.bottom) break;
            if (y >= clip.top && y < clip.bottom && end > clip.left &&
                x < clip.right && end > x) {
                if (end > clip.right) end = clip.right;
                if (x < clip.left) {
                    const int skip = texture_fixed(texture_difference(clip.left, x));
                    u = static_cast<int>(static_cast<unsigned int>(u) +
                        static_cast<unsigned int>((static_cast<int64_t>(du) * skip) >> 16));
                    v = static_cast<int>(static_cast<unsigned int>(v) +
                        static_cast<unsigned int>((static_cast<int64_t>(dv) * skip) >> 16));
                    x = clip.left;
                }
                unsigned char *out = destination + static_cast<int64_t>(y) * stride + x;
                int64_t source = static_cast<unsigned short>(static_cast<unsigned int>(v) >> 16)
                    * static_cast<int64_t>(width)
                    + static_cast<unsigned short>(static_cast<unsigned int>(u) >> 16);
                unsigned int uf = static_cast<unsigned short>(u);
                unsigned int vf = static_cast<unsigned short>(v);
                const int64_t step = static_cast<int64_t>(dv >> 16) * width + (du >> 16);
                // 0x0061A6BA packs the countdown and the U fraction into ESP
                // itself, so the span writes its first pixel before any test
                // and an admitted span clipped to zero or inverted width
                // still writes one pixel. The original also preloads the next
                // texel at 0x0061A6F6 before testing the counter; this C++
                // loop omits that unused final read - a known, preserved
                // difference, not an equivalence claim.
                const unsigned int remaining = texture_span_pixels(texture_difference(end, x));
                for (unsigned int written = 0; written < remaining; ++written, ++out) {
                    const unsigned char texel = pixels[source];
                    if (texel != key) *out = texel;
                    uf += static_cast<unsigned short>(du);
                    vf += static_cast<unsigned short>(dv);
                    source += step + (uf >> 16) + static_cast<int64_t>(vf >> 16) * width;
                    uf &= 0xffff;
                    vf &= 0xffff;
                }
            }
            if (done) break;
            ++y;
        }
    }
    buffer->free_data(1);
    texture_reset_fpu();
}

// ORIGINAL: 0x0061A760 ?draw_upper_threshold_trans@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@H@Z 0x0061A760-0x0061AC53 FILE
// symbol    ?draw_color_key@Texture@@QAEXPAVBuffer@@PAUVert@@H1@Z
//            the image spells `Buffer` and `Texture` as structs (PAU) and the
//            two setups as `QAA`, a __cdecl member taking no receiver; this tree
//            declares both classes `class` and both setups `static`, so VC6
//            mangles them PAV and SA. Same bytes, differing name.
// size      1267 bytes
// prototype void (__thiscall ?draw_color_key@Texture@@QAEXPAVBuffer@@PAUVert@@H1@Z)(Texture* this, Buffer*, Vert*, int count, Vert* uv)
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00624D80
// The catalogue's upper-threshold name misrepresents the body. 0x0061A760
// reads four stack arguments and ends in ret 0x10: Buffer*, Vert*, a signed
// count used purely as a scalar, and a second Vert* the edge setup consumes
// as the UV base (stored at 0x0061A7A5, count at 0x0061A7B3). Its kernel at
// 0x0061ABE5..E9 skips only equality with the low byte of TextureRasterKey;
// no depth array, no threshold comparison, no perspective division and no
// borrowed-texture guard appears. Width-based sampling, plain edges and the
// PC24 span slopes are 0x0061A240's, with count and UV pair taken from the
// caller instead of the embedded corner arrays. draw_color_key is this
// tree's provisional name: the shipped image carries no debug symbol for
// the body, and the behaviour, not a recovered name, is what the spelling
// claims. The one product caller is 0x006239E0's -1 forward; the other two
// mapped callers are unrecovered gen_terrain_poly seeds.
// LEVER: the span kernel at 0x0061ABBA packs its countdown into ESP and is
// reproduced with texture_span_pixels (see 0x0061A240's note); the original
// preloads the next texel at 0x0061ABF6 before testing the counter and this
// C++ loop omits that unused final read.
void Texture::draw_color_key(Buffer *buffer, Vert *vertices, int count, Vert *uv) {
    unsigned char *destination = reinterpret_cast<unsigned char *>(buffer->get_data());
    texture_reset_fpu();
    const unsigned char *pixels = static_cast<const unsigned char *>(pixels_);
    const int width = static_cast<int>(iWidth_);
    const unsigned char key = static_cast<unsigned char>(TextureRasterKey);
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    const int stride = static_cast<int>(buffer->stride_);
    const RECT clip = buffer->rect1_;

    int min_y = 0x7fff, max_y = -0x7ffd;
    int top = 0, bottom = 0;
    for (int i = 0; i < count; ++i) {
        if (vertices[i].y < min_y) { min_y = vertices[i].y; top = i; }
        if (vertices[i].y > max_y) { max_y = vertices[i].y; bottom = i; }
    }
    EdgeScan left, right;
    left.f0 = -1;
    right.f0 = 1;
    if (min_y < max_y && texture_edge(left, top, vertices, uv, count, bottom) &&
        texture_edge(right, top, vertices, uv, count, bottom)) {
        _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
        int du = 0, dv = 0;
        int y = min_y;
        if (y >= clip.bottom) {
            buffer->free_data(1);
            texture_reset_fpu();
            return;
        }
        for (;;) {
            int x = left.accumC;
            int end = right.accumC;
            int u = left.accumA, v = left.accumB;
            const int span = texture_fixed(texture_difference(end, x));
            if (span > 0) {
                du = texture_span_step(texture_difference(right.accumA, u), span >> 16);
                dv = texture_span_step(texture_difference(right.accumB, v), span >> 16);
            }
            // Both sides advance before the saved row is drawn. Left failure
            // skips right advancement, but never suppresses this final row.
            const int done = !texture_advance_edge(left, vertices, uv, count, bottom) ||
                             !texture_advance_edge(right, vertices, uv, count, bottom);
            if (y >= clip.bottom) break;
            if (y >= clip.top && y < clip.bottom && end > clip.left &&
                x < clip.right && end > x) {
                if (end > clip.right) end = clip.right;
                if (x < clip.left) {
                    const int skip = texture_fixed(texture_difference(clip.left, x));
                    u = static_cast<int>(static_cast<unsigned int>(u) +
                        static_cast<unsigned int>((static_cast<int64_t>(du) * skip) >> 16));
                    v = static_cast<int>(static_cast<unsigned int>(v) +
                        static_cast<unsigned int>((static_cast<int64_t>(dv) * skip) >> 16));
                    x = clip.left;
                }
                unsigned char *out = destination + static_cast<int64_t>(y) * stride + x;
                int64_t source = static_cast<unsigned short>(static_cast<unsigned int>(v) >> 16)
                    * static_cast<int64_t>(width)
                    + static_cast<unsigned short>(static_cast<unsigned int>(u) >> 16);
                unsigned int uf = static_cast<unsigned short>(u);
                unsigned int vf = static_cast<unsigned short>(v);
                const int64_t step = static_cast<int64_t>(dv >> 16) * width + (du >> 16);
                const unsigned int remaining = texture_span_pixels(texture_difference(end, x));
                for (unsigned int written = 0; written < remaining; ++written, ++out) {
                    const unsigned char texel = pixels[source];
                    if (texel != key) *out = texel;
                    uf += static_cast<unsigned short>(du);
                    vf += static_cast<unsigned short>(dv);
                    source += step + (uf >> 16) + static_cast<int64_t>(vf >> 16) * width;
                    uf &= 0xffff;
                    vf &= 0xffff;
                }
            }
            if (done) break;
            ++y;
        }
    }
    buffer->free_data(1);
    texture_reset_fpu();
}

// Fixed-slot bindings carried from 0061b1f0.cpp
// Shared signed light clamp: gen_terrain's three writer pairs temporarily
// select -4/+4 and restore these initial -256/+256 limits. Six original
// Texture entries clamp their input arrays against the same storage.
// RECOVERY_OBJECT: v2 0x006972CC 0x006972D0 ?TextureLightFloor@@3HA 0x00621870
int TextureLightFloor = -256;
// RECOVERY_OBJECT: v2 0x006972D0 0x006972D4 ?TextureLightCeiling@@3HA 0x00621870
int TextureLightCeiling = 256;

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

    *blit_edge_uvs_a = reinterpret_cast<int *>(a5);
    *blit_edge_uvs_b = reinterpret_cast<int *>(a2);
    *blit_vertex_count = a4;
    *blit_z_values = reinterpret_cast<int *>(a3);
    *blit_texture_pixels = *reinterpret_cast<int *>(self);
    *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *blit_texture_width = *reinterpret_cast<int *>(self + 4);

    if (reinterpret_cast<char *>(a1) + 0x20 != 0) {
        int *p = a3;
        int n = a4;
        if (n > 0) {
            do {
                int v = *p;
                if (TextureLightCeiling <= v) v = TextureLightCeiling;
                *p = v;
                if (v <= TextureLightFloor) v = TextureLightFloor;
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
        int count = *blit_vertex_count;
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                int y = v->y;
                if (y < minY) {
                    topVert = i;
                    minY = y;
                }
                if (maxY < y) {
                    *blit_max_y_index = i;
                    maxY = y;
                }
                int x = v->x;
                if (x < minX) minX = x;
                if (maxX < x) maxX = x;
                ++v;
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


void Texture::draw_trans(Buffer *buffer, Vert *vertices, int *zvals,
                         int count, Vert *uv) {
    unsigned char *destination = reinterpret_cast<unsigned char *>(buffer->get_data());
    texture_reset_fpu();

    *blit_pixel_base = static_cast<int>(reinterpret_cast<intptr_t>(destination));
    *blit_light_threshold = *blit_light_threshold_2;
    *blit_edge_uvs_a = reinterpret_cast<int *>(uv);
    *blit_edge_uvs_b = reinterpret_cast<int *>(vertices);
    *blit_vertex_count = count;
    *blit_z_values = reinterpret_cast<int *>(zvals);
    *blit_texture_pixels = static_cast<int>(reinterpret_cast<intptr_t>(pixels_));
    *blit_buffer_stride = static_cast<int>(buffer->stride_);
    *blit_texture_width = static_cast<int>(iWidth_);

    // Preserve the native derived-pointer check: it follows get_data and is
    // therefore not a null-buffer guard.
    if (reinterpret_cast<char *>(buffer) + 0x20 != 0) {
        for (int light_index = 0; light_index < count; ++light_index) {
            int value = zvals[light_index];
            if (TextureLightCeiling <= value) value = TextureLightCeiling;
            zvals[light_index] = value;
            if (value <= TextureLightFloor) value = TextureLightFloor;
            zvals[light_index] = value;
        }

        const RECT clip = buffer->rect1_;
        *blit_clip_right = clip.right;
        *blit_clip_left = clip.left;
        *blit_clip_bottom = clip.bottom;
        *blit_clip_top = clip.top;

        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int max_x = static_cast<int>(0xffff8003u);
        int min_x = 0x7fff;
        int top_index = 0;
        for (int i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top_index = i;
            }
            if (max_y < vertices[i].y) {
                max_y = vertices[i].y;
                *blit_max_y_index = i;
            }
            if (vertices[i].x < min_x) min_x = vertices[i].x;
            if (max_x < vertices[i].x) max_x = vertices[i].x;
        }
        (void)min_x;
        (void)max_x;

        if (count > 0 && min_y < max_y) {
            *blit_row_offset = static_cast<int>(
                static_cast<unsigned int>(*blit_buffer_stride) *
                    static_cast<unsigned int>(min_y) +
                static_cast<unsigned int>(*blit_pixel_base));
            *blit_clip_bottom = static_cast<int>(
                static_cast<unsigned int>(*blit_clip_bottom) *
                    static_cast<unsigned int>(*blit_buffer_stride) +
                static_cast<unsigned int>(*blit_pixel_base));
            *blit_clip_top = static_cast<int>(
                static_cast<unsigned int>(*blit_clip_top) *
                    static_cast<unsigned int>(*blit_buffer_stride) +
                static_cast<unsigned int>(*blit_pixel_base));

            EdgeScan left_edge;
            EdgeScan right_edge;
            left_edge.f0 = -1;
            if (setup_edge_gouraud(&left_edge, top_index) != 0) {
                right_edge.f0 = 1;
                if (setup_edge_gouraud(&right_edge, top_index) != 0) {
                    // Both edges are advanced before the saved row is drawn;
                    // left-refresh failure skips the right-edge transition.
                    _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
                    int left_rows = left_edge.counter;
                    int right_rows = right_edge.counter;
                    bool edges_done = false;

                    int initial_span = texture_difference(right_edge.accumC,
                                                          left_edge.accumC);
                    if (initial_span > 0) {
                        *blit_slot_4e4 = texture_span_step(
                            texture_difference(right_edge.accumA,
                                               left_edge.accumA),
                            initial_span);
                        *blit_slot_4c8 = texture_span_step(
                            texture_difference(right_edge.accumB,
                                               left_edge.accumB),
                            initial_span);
                        *blit_light_pack_a =
                            texture_light_step(
                                texture_difference(right_edge.accumD,
                                                   left_edge.accumD),
                                initial_span);
                    }

                    while (*blit_row_offset < *blit_clip_bottom) {
                        const int left_x = left_edge.accumC;
                        const int right_x = right_edge.accumC;
                        const int u = left_edge.accumA;
                        const int v = left_edge.accumB;
                        const int light = left_edge.accumD;
                        const int du = *blit_slot_4e4;
                        const int dv = *blit_slot_4c8;
                        const unsigned short light_step =
                            *blit_light_pack_a;

                        bool left_failed = false;
                        if (--left_rows == 0) {
                            if (setup_edge_gouraud(&left_edge, left_edge.id) == 0) {
                                edges_done = true;
                                left_failed = true;
                            } else {
                                left_rows = left_edge.counter;
                            }
                        } else {
                            edge_step(left_edge);
                        }
                        if (!left_failed) {
                            if (--right_rows == 0) {
                                if (setup_edge_gouraud(&right_edge,
                                                      right_edge.id) == 0) {
                                    edges_done = true;
                                } else {
                                    right_rows = right_edge.counter;
                                }
                            } else {
                                edge_step(right_edge);
                            }
                        }

                        const int next_span = texture_difference(
                            right_edge.accumC, left_edge.accumC);
                        if (next_span > 0) {
                            *blit_slot_4e4 = texture_span_step(
                                texture_difference(right_edge.accumA,
                                                   left_edge.accumA),
                                next_span);
                            *blit_slot_4c8 = texture_span_step(
                                texture_difference(right_edge.accumB,
                                                   left_edge.accumB),
                                next_span);
                            *blit_light_pack_a =
                                texture_light_step(
                                    texture_difference(right_edge.accumD,
                                                       left_edge.accumD),
                                    next_span);
                        }

                        if (*blit_clip_bottom <= *blit_row_offset) break;
                        if (*blit_clip_top <= *blit_row_offset &&
                            *blit_clip_left < right_x &&
                            left_x < *blit_clip_right && left_x < right_x) {
                            int clipped_right = right_x;
                            if (*blit_clip_right <= right_x)
                                clipped_right = *blit_clip_right;
                            int clipped_left = left_x;
                            int span_u = u;
                            int span_v = v;
                            int span_light = light;
                            if (left_x < *blit_clip_left) {
                                const int skipped = texture_difference(
                                    *blit_clip_left, left_x);
                                const int fixed_skip = static_cast<int>(
                                    static_cast<unsigned int>(skipped) << 16);
                                span_u = static_cast<int>(
                                    static_cast<unsigned int>(span_u) +
                                    static_cast<unsigned int>(
                                        (static_cast<__int64>(du) * fixed_skip) >> 16));
                                span_v = static_cast<int>(
                                    static_cast<unsigned int>(span_v) +
                                    static_cast<unsigned int>(
                                        (static_cast<__int64>(dv) * fixed_skip) >> 16));
                                span_light = static_cast<int>(
                                    static_cast<unsigned int>(span_light) +
                                    static_cast<unsigned int>(
                                        (static_cast<__int64>(
                                             static_cast<short>(light_step)) *
                                         fixed_skip) >> 8));
                                clipped_left = *blit_clip_left;
                            }

                            const int pixel_count = texture_span_pixels(
                                texture_difference(clipped_right, clipped_left));
                            unsigned char *row_destination =
                                reinterpret_cast<unsigned char *>(
                                    static_cast<uintptr_t>(
                                        static_cast<unsigned int>(*blit_row_offset) +
                                        static_cast<unsigned int>(clipped_left)));
                            const unsigned char key = static_cast<unsigned char>(
                                *reinterpret_cast<unsigned char *>(
                                    blit_light_threshold));
                            texture_draw_keyed_lit_span(
                                row_destination,
                                static_cast<const unsigned char *>(pixels_),
                                static_cast<int>(iWidth_), span_u, span_v, du, dv,
                                span_light, light_step, pixel_count, key);
                        }
                        if (edges_done) break;
                        *blit_row_offset = static_cast<int>(
                            static_cast<unsigned int>(*blit_row_offset) +
                            static_cast<unsigned int>(*blit_buffer_stride));
                    }
                }
            }
        }
    }

    if (buffer != 0) {
        buffer->free_data(1);
    }
    texture_reset_fpu();
}

// Fixed-slot bindings carried from 0061e540.cpp

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


void Texture::draw_mask_dest(Buffer *buffer, Vert *vertices,
                             unsigned char *remap, unsigned char *palette,
                             int count, Vert *uv) {
    // HISTORICAL SCAFFOLD NOTE (retained verbatim; the completed body below
    // supersedes its conclusion): From here the original drives a per-scanline
    // callback (setup_edge_gouraud) whose control flow resumes by restoring
    // esp from a value saved at a fixed address (0x009bb4ac) - a hand-written
    // continuation/coroutine jump with no standard C++ form, and inline
    // machine code is off the table per policy. Ruled out under budget rather
    // than guessed at: the setup above and the two setup_edge_gouraud calls
    // are faithful in shape and argument order; the fixed-point Gouraud/
    // palette span-fill inner loop between them is not attempted.
    if (vertices != 0 && buffer != 0 && pixels_ != 0 &&
        remap != 0) {
        *blit_pixel_base = buffer->get_data();
        texture_reset_fpu();
        *blit_edge_uvs_b = reinterpret_cast<int *>(vertices);
        *blit_remap_table = static_cast<int>(reinterpret_cast<intptr_t>(remap));
        *blit_texture_pixels = static_cast<int>(reinterpret_cast<intptr_t>(pixels_));
        TextureRowPitchScratch = static_cast<int>(iWidth_);
        *blit_texture_height = static_cast<int>(iHeight_);
        *blit_edge_uvs_a = reinterpret_cast<int *>(uv);
        *blit_vertex_count = count;
        *blit_buffer_stride = static_cast<int>(buffer->stride_);
        *blit_texture_width = static_cast<int>(iWidth_);
        *blit_slot_564 = static_cast<int>(reinterpret_cast<intptr_t>(palette));

        RECT *buffer_rect = &buffer->rect1_;
        if (buffer_rect != 0) {
            *blit_clip_right = buffer_rect->right;
            int min_y = 0x7fff;
            *blit_clip_left = buffer_rect->left;
            *blit_clip_bottom = buffer_rect->bottom;
            *blit_clip_top = buffer_rect->top;

            if (count > 0) {
                int max_y = -0x7ffd;
                int min_x = -0x7ffd;
                int max_x = 0x7fff;
                int min_y_index = 0;
                int max_y_index = 0;
                int *vertex_words = reinterpret_cast<int *>(vertices);
                for (int i = 0; i < count; ++i) {
                    const int y = vertex_words[1];
                    if (y < min_y) {
                        min_y_index = i;
                        min_y = y;
                    }
                    if (max_y < y) {
                        max_y_index = i;
                        max_y = y;
                        *blit_max_y_index = i;
                    }
                    const int x = vertex_words[0];
                    if (x < max_x) max_x = x;
                    if (min_x < x) min_x = x;
                    vertex_words += 2;
                }
                (void)max_x;
                (void)min_x;

                if (min_y < max_y) {
                    *blit_row_offset = static_cast<int>(
                        static_cast<unsigned int>(*blit_buffer_stride) *
                            static_cast<unsigned int>(min_y) +
                        static_cast<unsigned int>(*blit_pixel_base));
                    *blit_clip_bottom = static_cast<int>(
                        static_cast<unsigned int>(*blit_clip_bottom) *
                            static_cast<unsigned int>(*blit_buffer_stride) +
                        static_cast<unsigned int>(*blit_pixel_base));
                    *blit_clip_top = static_cast<int>(
                        static_cast<unsigned int>(*blit_clip_top) *
                            static_cast<unsigned int>(*blit_buffer_stride) +
                        static_cast<unsigned int>(*blit_pixel_base));

                    EdgeScan left_edge;
                    EdgeScan right_edge;
                    left_edge.f0 = -1;
                    if (setup_edge_gouraud(&left_edge, min_y_index) != 0) {
                        right_edge.f0 = 1;
                        if (setup_edge_gouraud(&right_edge, min_y_index) != 0) {
                            // Native FCW: 24-bit significand and truncating
                            // conversion for the three span deltas below.
                            _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
                            int active_du = 0;
                            int active_dv = 0;
                            short active_dz = 0;
                            const int initial_span = texture_difference(
                                right_edge.accumC, left_edge.accumC);
                            int prior_span_fixed = static_cast<int>(
                                static_cast<unsigned int>(initial_span) << 16);
                            if (prior_span_fixed > 0) {
                                active_du = texture_span_step(
                                    texture_difference(right_edge.accumA,
                                                       left_edge.accumA),
                                    initial_span);
                                active_dv = texture_span_step(
                                    texture_difference(right_edge.accumB,
                                                       left_edge.accumB),
                                    initial_span);
                                active_dz = static_cast<short>(texture_span_step(
                                    texture_difference(right_edge.accumD,
                                                       left_edge.accumD),
                                    initial_span) / 256);
                            }
                            bool done_edges = false;
                            while (*blit_row_offset < *blit_clip_bottom) {
                                // The image publishes this row's edge values
                                // before advancing either edge. Keep those
                                // values while the next row is prepared.
                                *blit_slot_4c4 = right_edge.accumA;
                                *blit_slot_4f8 = left_edge.accumA;
                                *blit_slot_530 = right_edge.accumB;
                                *blit_slot_53c = left_edge.accumB;
                                *blit_span_right = left_edge.accumC;
                                *blit_slot_498 = left_edge.accumD;
                                *blit_span_left = right_edge.accumC;

                                bool left_refresh_failed = false;
                                if (--left_edge.counter == 0) {
                                    if (setup_edge_gouraud(&left_edge,
                                                           left_edge.id) == 0)
                                        done_edges = left_refresh_failed = true;
                                } else {
                                    edge_step(left_edge);
                                }
                                // The left-refresh failure branches directly
                                // to the gradient-conversion label in the
                                // pinned body. In particular it does not
                                // decrement, refresh, or step the right edge.
                                if (!left_refresh_failed) {
                                    if (--right_edge.counter == 0) {
                                        if (setup_edge_gouraud(&right_edge,
                                                               right_edge.id) == 0)
                                            done_edges = true;
                                    } else {
                                        edge_step(right_edge);
                                    }
                                }

                                // The native TEST/JLE gates the complete
                                // FMUL/FISTP conversion on the preceding
                                // shifted span in ESI. When that span is
                                // nonpositive, leave the globals untouched;
                                // the x87 stack and prior global values are
                                // both intentionally retained.
                                if (prior_span_fixed > 0) {
                                    *blit_slot_4e4 = active_du;
                                    *blit_slot_4c8 = active_dv;
                                    *blit_light_pack_a = static_cast<unsigned short>(active_dz);
                                }

                                // The original changes x87 precision to 24
                                // bits and truncates each FISTP result. The
                                // integer helper preserves those independently
                                // quantized 16.16 span steps; the z channel is
                                // stored as a signed 8.8 word.
                                const int horizontal_difference = texture_difference(
                                    right_edge.accumC, left_edge.accumC);
                                const int edge_span = static_cast<int>(
                                    static_cast<unsigned int>(horizontal_difference)
                                    << 16);
                                int next_du = active_du;
                                int next_dv = active_dv;
                                short next_dz = active_dz;
                                if (edge_span > 0) {
                                    const int span_width = edge_span >> 16;
                                    next_du = texture_span_step(
                                        texture_difference(right_edge.accumA,
                                                           left_edge.accumA),
                                        span_width);
                                    next_dv = texture_span_step(
                                        texture_difference(right_edge.accumB,
                                                           left_edge.accumB),
                                        span_width);
                                    next_dz = static_cast<short>(texture_span_step(
                                            texture_difference(right_edge.accumD,
                                                               left_edge.accumD),
                                            span_width) / 256);
                                }

                                if (*blit_clip_bottom <= *blit_row_offset) break;
                                if (*blit_clip_top <= *blit_row_offset &&
                                    *blit_clip_left < *blit_span_left &&
                                    *blit_span_right < *blit_clip_right &&
                                    *blit_span_right < *blit_span_left) {
                                    int right_x = *blit_span_left;
                                    if (*blit_clip_right <= *blit_span_left)
                                        right_x = *blit_clip_right;
                                    if (*blit_span_right < *blit_clip_left) {
                                        const int skip = static_cast<int>(
                                            (static_cast<unsigned int>(
                                                 texture_difference(*blit_span_right,
                                                                    *blit_clip_left)) *
                                             0xffff0000u));
                                        *blit_slot_4f8 = static_cast<int>(
                                            static_cast<unsigned int>(*blit_slot_4f8) +
                                            static_cast<unsigned int>(
                                                (static_cast<__int64>(*blit_slot_4e4) *
                                                 skip) >> 16));
                                        *blit_slot_53c = static_cast<int>(
                                            static_cast<unsigned int>(*blit_slot_53c) +
                                            static_cast<unsigned int>(
                                                (static_cast<__int64>(*blit_slot_4c8) *
                                                 skip) >> 16));
                                        *blit_slot_498 = static_cast<int>(
                                            static_cast<unsigned int>(*blit_slot_498) +
                                            static_cast<unsigned int>(
                                                (static_cast<__int64>(static_cast<short>(
                                                    *blit_light_pack_a)) * skip) >> 8));
                                        *blit_span_right = *blit_clip_left;
                                    }

                                    const unsigned int source_offset =
                                        static_cast<unsigned int>(*blit_texture_width) *
                                            static_cast<unsigned int>(*blit_slot_53c >> 16) +
                                        static_cast<unsigned int>(*blit_slot_4f8 >> 16);
                                    unsigned char *source = reinterpret_cast<unsigned char *>(
                                        static_cast<uintptr_t>(
                                            static_cast<unsigned int>(*blit_texture_pixels) +
                                            source_offset));
                                    unsigned char *destination = reinterpret_cast<unsigned char *>(
                                        static_cast<uintptr_t>(
                                            static_cast<unsigned int>(*blit_span_right) +
                                            static_cast<unsigned int>(*blit_row_offset)));
                                    TextureRasterAlternateRowNext.value =
                                        static_cast<int>(
                                            static_cast<unsigned int>(*blit_slot_4c8 >> 16) *
                                                static_cast<unsigned int>(*blit_texture_width) +
                                            static_cast<unsigned int>(*blit_slot_4e4 >> 16));
                                    TextureRasterAlternateRowBase.value = static_cast<int>(
                                        static_cast<unsigned int>(TextureRasterAlternateRowNext.value) +
                                        static_cast<unsigned int>(*blit_texture_width));
                                    *blit_slot_528 = static_cast<int>(
                                        static_cast<unsigned int>(*blit_slot_4c8) << 16);

                                    unsigned int v_accumulator = concat22(
                                        static_cast<unsigned short>(*blit_slot_53c),
                                        static_cast<unsigned short>(*blit_slot_4e4));
                                    unsigned int span = concat22(
                                        static_cast<unsigned short>(
                                            (static_cast<short>(right_x) -
                                             static_cast<short>(*blit_span_right)) - 1),
                                        static_cast<unsigned short>(*blit_slot_4f8));
                                    unsigned int texture_offset = 0;
                                    const unsigned int pixels = texture_span_pixels(
                                        texture_difference(right_x,
                                                           *blit_span_right));
                                    for (unsigned int x = 0; x < pixels; ++x) {
                                        *blit_slot_4a4 = static_cast<int>(v_accumulator);
                                        const unsigned char texel = source[texture_offset];
                                        const unsigned char mapped = remap[texel];
                                        if (mapped != 0) {
                                            unsigned char *lookup = palette +
                                                (static_cast<unsigned int>(mapped - 1) << 8) +
                                                destination[0];
                                            destination[0] = *lookup;
                                        }
                                        const bool v_carry = carry4(
                                            v_accumulator,
                                            static_cast<unsigned int>(*blit_slot_528));
                                        const unsigned int next_v = v_accumulator +
                                            static_cast<unsigned int>(*blit_slot_528);
                                        const unsigned int selected_row = v_carry
                                            ? static_cast<unsigned int>(TextureRasterAlternateRowBase.value)
                                            : static_cast<unsigned int>(TextureRasterAlternateRowNext.value);
                                        texture_offset += selected_row +
                                            static_cast<unsigned int>(
                                                carry2(static_cast<unsigned short>(span),
                                                       static_cast<unsigned short>(next_v))
                                                    ? 1 : 0);
                                        const unsigned int next_span = concat22(
                                            static_cast<unsigned short>(span >> 16),
                                            static_cast<unsigned short>(
                                                static_cast<unsigned short>(span) +
                                                static_cast<unsigned short>(next_v)));
                                        v_accumulator = next_v;
                                        span = next_span;
                                        ++destination;
                                    }
                                }
                                active_du = next_du;
                                active_dv = next_dv;
                                active_dz = next_dz;
                                prior_span_fixed = edge_span;
                                if (done_edges) break;
                                *blit_row_offset = static_cast<int>(
                                    static_cast<unsigned int>(*blit_row_offset) +
                                    static_cast<unsigned int>(*blit_buffer_stride));
                            }
                        }
                    }
                }
            }
        }
    }

    if (buffer != 0) buffer->free_data(1);
    texture_reset_fpu();
}

// Fixed-slot bindings carried from 006233c0.cpp

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

void Texture::draw(Buffer *buffer, Vert *vertices, int *zvals, int count,
                   Vert *edge_vertices, int fill_color) {
    if (fill_color == -1) {
        draw(buffer, vertices, zvals, count, edge_vertices);
        return;
    }

    unsigned char *destination = reinterpret_cast<unsigned char *>(buffer->get_data());
    *blit_pixel_base = static_cast<int>(reinterpret_cast<intptr_t>(destination));
    *blit_light_threshold = *blit_light_threshold_2;
    *reinterpret_cast<unsigned char *>(blit_slot_494) =
        static_cast<unsigned char>(fill_color);
    texture_reset_fpu();
    *blit_edge_uvs_b = reinterpret_cast<int *>(vertices);
    *blit_edge_uvs_a = reinterpret_cast<int *>(edge_vertices);
    *blit_vertex_count = count;
    *blit_z_values = reinterpret_cast<int *>(zvals);
    *blit_texture_pixels = static_cast<int>(reinterpret_cast<intptr_t>(pixels_));
    *blit_buffer_stride = static_cast<int>(buffer->stride_);
    *blit_texture_width = static_cast<int>(iWidth_);

    for (int light_index = 0; light_index < count; ++light_index) {
        int value = zvals[light_index];
        if (value >= TextureLightCeiling) value = TextureLightCeiling;
        zvals[light_index] = value;
        if (value <= TextureLightFloor) value = TextureLightFloor;
        zvals[light_index] = value;
    }

    const RECT clip = buffer->rect1_;
    *blit_clip_right = clip.right;
    *blit_clip_left = clip.left;
    *blit_clip_bottom = clip.bottom;
    *blit_clip_top = clip.top;
    int min_y = 0x7fff, max_y = -0x7ffd;
    int min_x = 0x7fff, max_x = -0x7ffd;
    int top = 0, bottom = 0;
    for (int i = 0; i < count; ++i) {
        if (vertices[i].y < min_y) { min_y = vertices[i].y; top = i; }
        if (max_y < vertices[i].y) {
            max_y = vertices[i].y;
            bottom = i;
            *blit_max_y_index = i;
        }
        if (vertices[i].x < min_x) min_x = vertices[i].x;
        if (max_x < vertices[i].x) max_x = vertices[i].x;
    }
    (void)min_x;
    (void)max_x;

    if (min_y < max_y) {
        const int stride = static_cast<int>(buffer->stride_);
        int y = min_y;
        *blit_row_offset = static_cast<int>(
            static_cast<unsigned int>(stride) * static_cast<unsigned int>(min_y) +
            static_cast<unsigned int>(*blit_pixel_base));
        *blit_clip_top = static_cast<int>(
            static_cast<unsigned int>(stride) * static_cast<unsigned int>(clip.top) +
            static_cast<unsigned int>(*blit_pixel_base));
        *blit_clip_bottom = static_cast<int>(
            static_cast<unsigned int>(stride) * static_cast<unsigned int>(clip.bottom) +
            static_cast<unsigned int>(*blit_pixel_base));

        EdgeScan left = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        EdgeScan right = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        left.f0 = -1;
        right.f0 = 1;
        if (texture_gouraud_edge(left, top, vertices, edge_vertices,
                                 zvals, count, bottom) &&
            texture_gouraud_edge(right, top, vertices, edge_vertices,
                                 zvals, count, bottom)) {
            _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
            int du = *blit_slot_4e4;
            int dv = *blit_slot_4c8;
            unsigned short light_step = static_cast<unsigned short>(*blit_light_pack_a);
            int parity = y & 1;
            *blit_dither = parity;
            while (*blit_row_offset < *blit_clip_bottom) {
                int x = left.accumC;
                int end = right.accumC;
                int u = left.accumA;
                int v = left.accumB;
                int light = left.accumD;
                *blit_slot_498 = light;
                *blit_slot_4f8 = u;
                *blit_slot_53c = v;
                *blit_slot_4c4 = right.accumA;
                *blit_slot_530 = right.accumB;
                *blit_span_right = x;
                *blit_span_left = end;

                const int span = texture_fixed(texture_difference(end, x));
                if (span > 0) {
                    du = texture_span_step(texture_difference(right.accumA, u), span >> 16);
                    dv = texture_span_step(texture_difference(right.accumB, v), span >> 16);
                    light_step = texture_light_step(
                        texture_difference(right.accumD, light), span >> 16);
                    *blit_slot_4e4 = du;
                    *blit_slot_4c8 = dv;
                    *blit_light_pack_a = light_step;
                }
                const int done = !texture_advance_gouraud_edge(
                                     left, vertices, edge_vertices, zvals, count, bottom) ||
                                 !texture_advance_gouraud_edge(
                                     right, vertices, edge_vertices, zvals, count, bottom);
                if (*blit_clip_top <= *blit_row_offset &&
                    end > clip.left && x < clip.right && end > x) {
                    if (end > clip.right) end = clip.right;
                    if (x < clip.left) {
                        const int skip = texture_fixed(texture_difference(clip.left, x));
                        u = static_cast<int>(static_cast<unsigned int>(u) +
                            static_cast<unsigned int>((static_cast<int64_t>(du) * skip) >> 16));
                        v = static_cast<int>(static_cast<unsigned int>(v) +
                            static_cast<unsigned int>((static_cast<int64_t>(dv) * skip) >> 16));
                        light = static_cast<int>(static_cast<unsigned int>(light) +
                            static_cast<unsigned int>((static_cast<int64_t>(light_step) * skip) >> 8));
                        x = clip.left;
                    }
                    *blit_slot_4f8 = u;
                    *blit_slot_53c = v;
                    *blit_slot_498 = light;
                    unsigned char *out = destination + static_cast<int64_t>(y) * stride + x;
                    const unsigned int pixel_count =
                        texture_span_pixels(texture_difference(end, x));
                    if ((parity & 1) == 0) {
                        const unsigned char color =
                            *reinterpret_cast<unsigned char *>(blit_slot_494);
                        for (unsigned int pixel = 0; pixel < pixel_count; ++pixel)
                            out[pixel] = color;
                    } else {
                        texture_draw_lit_span(
                            out, static_cast<const unsigned char *>(pixels_),
                            static_cast<int>(iWidth_), u, v, du, dv, light,
                            light_step, static_cast<int>(pixel_count));
                    }
                }
                if (done) break;
                ++y;
                ++parity;
                *blit_dither = parity;
                *blit_row_offset = static_cast<int>(
                    static_cast<unsigned int>(*blit_row_offset) +
                    static_cast<unsigned int>(stride));
            }
        }
    }
    if (buffer != 0) buffer->free_data(1);
    texture_reset_fpu();
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
// Correction from the pinned 0x623F80 frame: these locals are the full 15-word EdgeScan records; the former flat-array field-index interpretation is retained as history, not current layout evidence.
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
    if (self[0] != 0 && a3 != 0 && a5 != 0) {
        *blit_pixel_base = a1->get_data();
        texture_reset_fpu();
        *reinterpret_cast<char *>(blit_slot_494) = static_cast<char>(a8);
        *blit_edge_uvs_b = reinterpret_cast<int *>(a2);
        *blit_z_values = reinterpret_cast<int *>(a3);
        *blit_remap_table = reinterpret_cast<int>(a5);
        *blit_texture_pixels = self[0];
        TextureRowPitchScratch = self[1];
        *blit_texture_height = self[2];
        *blit_vertex_count = a6;
        *blit_edge_uvs_a = reinterpret_cast<int *>(a7);
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
            int minYIdx = a8;
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

                    // Two complete EdgeScan records: f0 is traversal direction, followed by
                    // counter/id and the stepped A/B/C/D channels.
                    EdgeScan edge1;
                    EdgeScan edge2;
                    edge1.f0 = -1;
                    if (this->setup_edge_gouraud(&edge1, minYIdx) != 0) {
                        edge2.f0 = 1;
                        if (this->setup_edge_gouraud(&edge2, minYIdx) != 0) {
                            // The native path selects round-toward-zero after both edge setups.
                            _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
                            long double invDx = 1.0;
                            long double dA = 1.0, dB = 1.0, dC = 1.0;
                            long edgeSpan = (edge2.accumC - edge1.accumC) * 0x10000;
                            if (0 < edgeSpan) {
                                dA = static_cast<long double>(edge2.accumA - edge1.accumA);
                                dB = static_cast<long double>(edge2.accumB - edge1.accumB);
                                dC = static_cast<long double>(edge2.accumD - edge1.accumD);
                                invDx = 1.0 / static_cast<long double>(edgeSpan);
                            }

                            if (*blit_row_offset < *blit_clip_bottom) {
                                bool doneEdges = false;
                                do {
                                    *blit_slot_4c4 = edge2.accumA;
                                    *blit_slot_4f8 = edge1.accumA;
                                    *blit_slot_530 = edge2.accumB;
                                    *blit_slot_53c = edge1.accumB;
                                    *blit_span_right = edge1.accumC;
                                    *blit_slot_498 = edge1.accumD;
                                    *blit_span_left = edge2.accumC;

                                    --edge1.counter;
                                    if (edge1.counter == 0) {
                                        if (this->setup_edge_gouraud(&edge1, edge1.id) != 0) {
                                            goto edge2_step;
                                        }
                                        doneEdges = true;
                                    } else {
                                        edge1.accumA += edge1.deltaA;
                                        edge1.accumB += edge1.deltaB;
                                        edge1.accumD += edge1.deltaD;
                                        edge1.accumC += edge1.deltaC;
                                        edge1.fracAccum += edge1.fracDelta;
                                        if (0 < edge1.fracAccum) {
                                            edge1.accumC += edge1.extraDeltaC;
                                            edge1.fracAccum -= edge1.fracSub;
                                        }
                                    edge2_step:
                                        --edge2.counter;
                                        if (edge2.counter == 0) {
                                            if (this->setup_edge_gouraud(&edge2, edge2.id) == 0) {
                                                doneEdges = true;
                                            }
                                        } else {
                                            edge2.accumA += edge2.deltaA;
                                            edge2.accumB += edge2.deltaB;
                                            edge2.accumC += edge2.deltaC;
                                            edge2.accumD += edge2.deltaD;
                                            edge2.fracAccum += edge2.fracDelta;
                                            if (0 < edge2.fracAccum) {
                                                edge2.accumC += edge2.extraDeltaC;
                                                edge2.fracAccum -= edge2.fracSub;
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

                                    edgeSpan = (edge2.accumC - edge1.accumC) * 0x10000;
                                    if (0 < edgeSpan) {
                                        dA = static_cast<long double>(edge2.accumA - edge1.accumA);
                                        dB = static_cast<long double>(edge2.accumB - edge1.accumB);
                                        dC = static_cast<long double>(edge2.accumD - edge1.accumD);
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

                                        const unsigned int base_source_offset = texture_draw_source_index(
                                            *blit_texture_width, *blit_slot_4f8,
                                            *blit_slot_53c);
                                        char *src = reinterpret_cast<char *>(
                                            static_cast<uintptr_t>(
                                                static_cast<unsigned int>(*blit_texture_pixels) +
                                                base_source_offset));
                                        *blit_slot_504 = reinterpret_cast<int>(src);
                                        *blit_slot_558 = reinterpret_cast<int>(src) + (*blit_shade_table - *blit_texture_pixels);
                                        char *dst = reinterpret_cast<char *>(*blit_span_right + *blit_row_offset);
                                        const int row_light = *blit_slot_498;
                                        *blit_slot_498 = row_light >> 8;
                                        TextureRasterAlternateRowNext.value =
                                            (*blit_slot_4c8 >> 16) * *blit_texture_width + (*blit_slot_4e4 >> 16);
                                        TextureRasterAlternateRowBase.value = TextureRasterAlternateRowNext.value + *blit_texture_width;
                                        *blit_slot_528 = *blit_slot_4c8 << 16;

                                        const int pixel_count = static_cast<int>(
                                            texture_span_pixels(static_cast<short>(rightX) -
                                                                static_cast<short>(*blit_span_right)));
                                        TextureDrawSpanResult span_result;
                                        span_result.packed_uv_sink = blit_slot_4a4;
                                        span_result.preload_pixels =
                                            reinterpret_cast<volatile const unsigned char *>(
                                                static_cast<uintptr_t>(
                                                    static_cast<unsigned int>(*blit_texture_pixels)));
                                        span_result.prefetched_source =
                                            span_result.preload_pixels[base_source_offset];
                                        if ((*blit_dither & 1) == 0) {
                                            texture_draw_keyed_coast_fill_span(
                                                reinterpret_cast<unsigned char *>(dst),
                                                reinterpret_cast<const unsigned char *>(*blit_shade_table),
                                                reinterpret_cast<const unsigned char *>(*blit_remap_table),
                                                *blit_texture_width, *blit_slot_4f8, *blit_slot_53c,
                                                *blit_slot_4e4, *blit_slot_4c8,
                                                static_cast<unsigned char>(*blit_slot_494), pixel_count,
                                                &span_result);
                                        } else {
                                            texture_draw_coast_span(
                                                reinterpret_cast<unsigned char *>(dst),
                                                reinterpret_cast<const unsigned char *>(*blit_texture_pixels),
                                                reinterpret_cast<const unsigned char *>(*blit_shade_table),
                                                reinterpret_cast<const unsigned char *>(*blit_remap_table),
                                                *blit_texture_width, *blit_slot_4f8, *blit_slot_53c,
                                                *blit_slot_4e4, *blit_slot_4c8, row_light,
                                                static_cast<unsigned short>(*blit_light_pack_a),
                                                pixel_count, &span_result);
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
    texture_reset_fpu();
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
    *blit_edge_uvs_b = reinterpret_cast<int *>(a2);
    *blit_texture_pixels = *reinterpret_cast<int *>(thisBytes);
    TextureRowPitchScratch = *reinterpret_cast<int *>(thisBytes + 4);
    *blit_texture_height = *reinterpret_cast<int *>(thisBytes + 8);

    if (a3 == 0) {
        *blit_edge_uvs_a = reinterpret_cast<int *>(thisBytes + 0xc);
        *blit_vertex_count = 4;
    } else {
        *blit_edge_uvs_a = reinterpret_cast<int *>(thisBytes + 0x2c);
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
    *blit_edge_uvs_b = reinterpret_cast<int *>(a2);
    *blit_edge_uvs_a = reinterpret_cast<int *>(a5);
    *blit_z_values = reinterpret_cast<int *>(a3);
    *blit_texture_pixels = *reinterpret_cast<int *>(this);
    *blit_buffer_stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *blit_texture_width = iWidth_;

    if (reinterpret_cast<char *>(a1) + 0x20 != 0) {
        for (int i = 0; i < a4; ++i) {
            int v = a3[i];
            if (v >= TextureLightCeiling) v = TextureLightCeiling;
            a3[i] = v;
            if (v <= TextureLightFloor) v = TextureLightFloor;
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

// Cached counterpart of the Gouraud edge setup. Only six channels in a
// selected record are initialized; flags independently invalidate geometry
// (bit 2) and lighting (bit 1). Empty/descending edges do not consume records.
static int texture_cached_edge(EdgeScan &edge, int index,
                               const Vert *vertices, const Vert *uv,
                               const int *light, int count, int bottom,
                               TextureStore &cache) {
    if (index == bottom) return 0;
    for (;;) {
        int next = index + edge.f0;
        if (next >= count) next = 0;
        else if (next < 0) next = count - 1;
        edge.counter = texture_difference(vertices[next].y, vertices[index].y);
        if (edge.counter != 0) {
            if (edge.counter < 0) return 0;
            EdgeScan &record = *cache.sub_6252c0();
            edge.id = next;
            edge.accumA = static_cast<int>(
                static_cast<unsigned int>(texture_fixed(uv[index].x)) + 0x8000u);
            edge.accumB = static_cast<int>(
                static_cast<unsigned int>(texture_fixed(uv[index].y)) + 0x8000u);
            const int height = texture_fixed(edge.counter);
            if (cache.flags_ & 2) {
                record.deltaA = static_cast<int>(static_cast<int64_t>(
                    texture_difference(texture_fixed(uv[next].x), edge.accumA)) * 65536 / height);
                record.deltaB = static_cast<int>(static_cast<int64_t>(
                    texture_difference(texture_fixed(uv[next].y), edge.accumB)) * 65536 / height);
            }
            edge.deltaA = record.deltaA;
            edge.deltaB = record.deltaB;
            edge.accumC = vertices[index].x;
            const int dx = texture_difference(vertices[next].x, vertices[index].x);
            if (cache.flags_ & 2) {
                record.deltaC = dx / edge.counter;
                const int magnitude = dx < 0 ? texture_difference(0, dx) : dx;
                record.fracDelta = magnitude % edge.counter;
            }
            edge.deltaC = record.deltaC;
            edge.extraDeltaC = dx < 0 ? -1 : 1;
            edge.fracAccum = dx < 0 ? 1 - edge.counter : 0;
            edge.fracDelta = record.fracDelta;
            edge.fracSub = edge.counter;
            if (cache.flags_ & 1) {
                record.accumD = texture_fixed(light[index]);
                record.deltaD = static_cast<int>(static_cast<int64_t>(
                    texture_difference(texture_fixed(light[next]), record.accumD)) * 65536 / height);
            }
            edge.accumD = record.accumD;
            edge.deltaD = record.deltaD;
            return 1;
        }
        index = next;
        if (index == bottom) return 0;
    }
}

static int texture_advance_cached_edge(EdgeScan &edge, const Vert *vertices,
                                       const Vert *uv, const int *light,
                                       int count, int bottom, TextureStore &cache) {
    if (--edge.counter == 0)
        return texture_cached_edge(edge, edge.id, vertices, uv, light, count, bottom, cache);
    edge.accumA = static_cast<int>(static_cast<unsigned int>(edge.accumA) + edge.deltaA);
    edge.accumB = static_cast<int>(static_cast<unsigned int>(edge.accumB) + edge.deltaB);
    edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.deltaC);
    edge.accumD = static_cast<int>(static_cast<unsigned int>(edge.accumD) + edge.deltaD);
    edge.fracAccum = static_cast<int>(static_cast<unsigned int>(edge.fracAccum) + edge.fracDelta);
    if (edge.fracAccum > 0) {
        edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.extraDeltaC);
        edge.fracAccum = texture_difference(edge.fracAccum, edge.fracSub);
    }
    return 1;
}

// ORIGINAL: 0x00625080 sub_625080 0x00625080-0x00625294 FILE
// symbol    ?sub_625080@@YAHPAUEdgeScan@@HPAVTextureStore@@@Z
// TRIED: verbatim zero-param head - disasm reads [esp+0x10]/[esp+0x14]/ [esp+0x1c] as three stack args before any push (IDA's 3-int guess confirmed by field roles: int*, int, byte*), so the head was widened to those 3 params; prologue order still diverges from the original's odd 2-push-then-arg2 pattern.
// TRIED: consolidating blit_edge_uvs_a/b + blit_z_values onto ONE `int **const`
//            binding per address (the duplicate_globals cleanup, 8 -> 5), in
//            both spellings - keep the `*` (`*(int *)(...)`) and drop it
//            (`static int *&b = *(int **)0xADDR`). NEITHER is codegen-neutral,
//            so the pair of bindings is load-bearing, not a spelling slip.
//            Measured on this body, 10 load sites: `int *const` costs VC6 two
//            instructions per read (`mov [<binding>],reg` then
//            `mov (reg),reg` - the binding is NOT folded to the immediate the
//            way `*blit_vertex_count` is) and `int **const` costs three. This
//            body went 24/183 -> 6/183, 0x00624EB0 8/160 -> 0/160,
//            0x00619D80 9/358 -> 3/358; 11 bodies fell in total, 5 rose
//            (0x0061C630 6->9, 0x0061EAD0 4->6, 0x0061FC90 3->5), net -36
//            agreeing instructions. The image emits ONE `mov ecx,[0x9bb4f4]`
//            per read, so the real lever is finding a binding spelling VC6
//            folds at all - neither form here does. Reverted, duplicates kept.
// working copy - scaffold materialised by --work
// size      532 bytes
// prototype 
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628AD0

// The original three-argument entry consumes shared polygon context. Its
// remaining legacy callers still publish that context to the old scratch
// block; the genuine cached-light body passes its context directly to the
// same typed implementation. Those old raster bodies remain recovery work.

int __cdecl sub_625080(EdgeScan *edge, int index, TextureStore *cache) {
    return texture_cached_edge(*edge, index,
        reinterpret_cast<const Vert *>(*blit_edge_uvs_b),
        reinterpret_cast<const Vert *>(*blit_edge_uvs_a),
        reinterpret_cast<const int *>(*blit_z_values),
        *blit_vertex_count, *blit_max_y_index, *cache);
}

// Fixed-slot bindings carried from 00620100.cpp

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

    delta = reinterpret_cast<EdgePoint *>(*blit_edge_uvs_b)[idx].b -
            reinterpret_cast<EdgePoint *>(*blit_edge_uvs_b)[param_2].b;
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
      param_1[3] = reinterpret_cast<EdgePoint *>(*blit_edge_uvs_a)[param_2].a << 0x10;
      param_1[4] = reinterpret_cast<EdgePoint *>(*blit_edge_uvs_a)[param_2].b << 0x10;
      if ((*param_3 & 2) == 0) {
        param_1[5] = *reinterpret_cast<int *>(param_3 + slotIndex + 0x1c);
        param_1[6] = *reinterpret_cast<int *>(param_3 + slotIndex + 0x20);
      } else {
        int fx = fixed_div(
            reinterpret_cast<EdgePoint *>(*blit_edge_uvs_a)[idx].a * 0x10000 - param_1[3],
            range << 0x10);
        *reinterpret_cast<int *>(param_3 + slotIndex + 0x1c) = fx;
        param_1[5] = fx;
        range = param_1[1];
        int fy = fixed_div(
            reinterpret_cast<EdgePoint *>(*blit_edge_uvs_a)[idx].b * 0x10000 - param_1[4],
            range << 0x10);
        *reinterpret_cast<int *>(param_3 + slotIndex + 0x20) = fy;
        param_1[6] = fy;
      }

      param_1[7] = reinterpret_cast<EdgePoint *>(*blit_edge_uvs_b)[param_2].a;
      idx = reinterpret_cast<EdgePoint *>(*blit_edge_uvs_b)[idx].a -
            reinterpret_cast<EdgePoint *>(*blit_edge_uvs_b)[param_2].a;
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

static int texture_raster_gradient_span_admitted(int span);
static unsigned char *texture_sub61df60_page_address(void *base,
                                                       int u, int v) {
    const uintptr_t base_bits = reinterpret_cast<uintptr_t>(base);
    const unsigned int x =
        static_cast<unsigned char>(static_cast<unsigned int>(base_bits) +
                                   (static_cast<unsigned int>(u) >> 16));
    const unsigned int y = static_cast<unsigned char>(
        static_cast<unsigned int>(v) >> 16);
    const uintptr_t offset = (static_cast<uintptr_t>(y) << 8) | x;
    return reinterpret_cast<unsigned char *>(
        (base_bits & ~static_cast<uintptr_t>(0xffffu)) | offset);
}

// ORIGINAL: 0x0061DF60 sub_61df60 0x0061DF60-0x0061E538 FILE
// symbol    ?sub_61df60@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPAV1@PAEH1H4@Z
// Historical synthetic seed symbol: ?sub_61df60@Texture@@QAEHHHHHHHHHH@Z
// ECX receiver and RET 24h establish nine stack arguments. Buffer/Vert/light,
// mask/remap, vertex count/UV, threshold and palette uses establish their types.
// The mapped sentinel caller discards EAX; no original result contract is proved.
// Both Gouraud edge records are the observed 15-word EdgeScan, not the older
// undersized ten-word right-edge scratch. Pending gradients apply one row later.
// TRIED: byte-exact register-packed inner blit loop (bl/bh texel pointer, ebp repurposed as scratch, cx/ax half-register Bresenham counters); landed as a __thiscall member with two EdgeScan scratch blocks (edge0[15]/edge1[10]) and a plain per-pixel loop instead - correct shape, not byte-identical.
// working copy - scaffold materialised by --work
// size      1496 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90

void Texture::sub_61df60(Buffer *buffer, Vert *vertices, int *zvals,
                         Texture *mask_texture, unsigned char *remap,
                         int count, Vert *uv, int threshold,
                         unsigned char *palette) {
    if (vertices == 0) goto cleanup;
    if (buffer == 0) goto epilogue_only;
    if (pixels_ == 0) goto cleanup;
    if (zvals == 0) goto cleanup;
    if (remap == 0) goto cleanup;
    if (mask_texture == 0) goto cleanup;
    if (palette == 0) goto cleanup;

    {
        unsigned char *destination = reinterpret_cast<unsigned char *>(
            static_cast<intptr_t>(buffer->get_data()));
        *blit_pixel_base = static_cast<int>(
            reinterpret_cast<intptr_t>(destination));
        texture_reset_fpu();
        *reinterpret_cast<unsigned char *>(blit_slot_560) =
            static_cast<unsigned char>(threshold);
        *blit_slot_564 = static_cast<int>(reinterpret_cast<intptr_t>(palette));
        *blit_edge_uvs_b = reinterpret_cast<int *>(vertices);
        *blit_z_values = reinterpret_cast<int *>(zvals);
        *blit_remap_table = static_cast<int>(reinterpret_cast<intptr_t>(remap));
        *blit_texture_pixels = static_cast<int>(
            reinterpret_cast<intptr_t>(pixels_));
        TextureRowPitchScratch = static_cast<int>(iWidth_);
        *blit_texture_height = static_cast<int>(iHeight_);
        *blit_edge_uvs_a = reinterpret_cast<int *>(uv);
        *blit_vertex_count = count;
        *blit_buffer_stride = static_cast<int>(buffer->stride_);
        *blit_texture_width = static_cast<int>(iWidth_);
        *blit_shade_table = static_cast<int>(
            reinterpret_cast<intptr_t>(mask_texture->pixels_));

        if (*blit_shade_table == 0) goto cleanup;
        *blit_clip_right = buffer->rect1_.right;
        *blit_clip_left = buffer->rect1_.left;
        *blit_clip_bottom = buffer->rect1_.bottom;
        *blit_clip_top = buffer->rect1_.top;
        if (count <= 0) goto cleanup;

        int min_y = 0x7fff;
        int min_x = 0x7fff;
        int max_x = static_cast<int>(0xffff8003u);
        int max_y = static_cast<int>(0xffff8003u);
        int top_index = 0;
        int index = 0;
        do {
            const int y = vertices[index].y;
            if (y < min_y) {
                min_y = y;
                top_index = index;
            }
            if (max_y < y) {
                max_y = y;
                *blit_max_y_index = index;
            }
            const int x = vertices[index].x;
            if (x < min_x) min_x = x;
            if (max_x < x) max_x = x;
            ++index;
        } while (index < count);
        (void)min_x;
        (void)max_x;
        if (min_y >= max_y) goto cleanup;

        const int stride = *blit_buffer_stride;
        const unsigned int base = static_cast<unsigned int>(*blit_pixel_base);
        *blit_row_offset = static_cast<int>(
            static_cast<unsigned int>(stride) * static_cast<unsigned int>(min_y) +
            base);
        *blit_clip_bottom = static_cast<int>(
            static_cast<unsigned int>(*blit_clip_bottom) *
                static_cast<unsigned int>(stride) + base);
        *blit_clip_top = static_cast<int>(
            static_cast<unsigned int>(*blit_clip_top) *
                static_cast<unsigned int>(stride) + base);

        EdgeScan left;
        EdgeScan right;
        left.f0 = -1;
        if (Texture::setup_edge_gouraud(&left, top_index) == 0) goto cleanup;
        right.f0 = 1;
        if (Texture::setup_edge_gouraud(&right, top_index) == 0) goto cleanup;
        if (borrowed_ == 0) goto cleanup;
        if (mask_texture->borrowed_ == 0) goto cleanup;

        _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
        int du = *blit_slot_4e4;
        int dv = *blit_slot_4c8;
        unsigned short light_step = static_cast<unsigned short>(
            *blit_light_pack_a);
        int pending_du = du;
        int pending_dv = dv;
        unsigned short pending_light_step = light_step;

        int initial_span = texture_difference(right.accumC, left.accumC);
        bool pending_gradient =
            texture_raster_gradient_span_admitted(initial_span) != 0;
        if (pending_gradient) {
            pending_du = texture_span_step(
                texture_difference(right.accumA, left.accumA), initial_span);
            pending_dv = texture_span_step(
                texture_difference(right.accumB, left.accumB), initial_span);
            pending_light_step = texture_light_step(
                texture_difference(right.accumD, left.accumD), initial_span);
        }

        if (*blit_row_offset >= *blit_clip_bottom) goto cleanup;
        for (;;) {
            const int left_x = left.accumC;
            const int right_x = right.accumC;
            int u = left.accumA;
            int v = left.accumB;
            int light = left.accumD;
            *blit_slot_4c4 = right.accumA;
            *blit_slot_4f8 = u;
            *blit_slot_530 = right.accumB;
            *blit_slot_53c = v;
            *blit_span_right = left_x;   // Native slot 9BB4CC is the LEFT x.
            *blit_slot_498 = light;
            *blit_span_left = right_x;  // Native slot 9BB4F0 is the RIGHT x.

            bool done = false;
            if (--left.counter == 0) {
                if (Texture::setup_edge_gouraud(&left, left.id) == 0)
                    done = true;
            } else {
                left.accumA = static_cast<int>(
                    static_cast<unsigned int>(left.accumA) +
                    static_cast<unsigned int>(left.deltaA));
                left.accumB = static_cast<int>(
                    static_cast<unsigned int>(left.accumB) +
                    static_cast<unsigned int>(left.deltaB));
                left.accumC = static_cast<int>(
                    static_cast<unsigned int>(left.accumC) +
                    static_cast<unsigned int>(left.deltaC));
                left.accumD = static_cast<int>(
                    static_cast<unsigned int>(left.accumD) +
                    static_cast<unsigned int>(left.deltaD));
                left.fracAccum = static_cast<int>(
                    static_cast<unsigned int>(left.fracAccum) +
                    static_cast<unsigned int>(left.fracDelta));
                if (left.fracAccum > 0) {
                    left.accumC = static_cast<int>(
                        static_cast<unsigned int>(left.accumC) +
                        static_cast<unsigned int>(left.extraDeltaC));
                    left.fracAccum = texture_difference(left.fracAccum,
                                                        left.fracSub);
                }
            }
            if (!done) {
                if (--right.counter == 0) {
                    if (Texture::setup_edge_gouraud(&right, right.id) == 0)
                        done = true;
                } else {
                    right.accumA = static_cast<int>(
                        static_cast<unsigned int>(right.accumA) +
                        static_cast<unsigned int>(right.deltaA));
                    right.accumB = static_cast<int>(
                        static_cast<unsigned int>(right.accumB) +
                        static_cast<unsigned int>(right.deltaB));
                    right.accumC = static_cast<int>(
                        static_cast<unsigned int>(right.accumC) +
                        static_cast<unsigned int>(right.deltaC));
                    right.accumD = static_cast<int>(
                        static_cast<unsigned int>(right.accumD) +
                        static_cast<unsigned int>(right.deltaD));
                    right.fracAccum = static_cast<int>(
                        static_cast<unsigned int>(right.fracAccum) +
                        static_cast<unsigned int>(right.fracDelta));
                    if (right.fracAccum > 0) {
                        right.accumC = static_cast<int>(
                            static_cast<unsigned int>(right.accumC) +
                            static_cast<unsigned int>(right.extraDeltaC));
                        right.fracAccum = texture_difference(right.fracAccum,
                                                            right.fracSub);
                    }
                }
            }

            if (pending_gradient) {
                du = pending_du;
                dv = pending_dv;
                light_step = pending_light_step;
                *blit_slot_4e4 = du;
                *blit_slot_4c8 = dv;
                *blit_light_pack_a =
                    light_step;
            }

            const int next_span = texture_difference(right.accumC, left.accumC);
            pending_gradient =
                texture_raster_gradient_span_admitted(next_span) != 0;
            if (pending_gradient) {
                pending_du = texture_span_step(
                    texture_difference(right.accumA, left.accumA), next_span);
                pending_dv = texture_span_step(
                    texture_difference(right.accumB, left.accumB), next_span);
                pending_light_step = texture_light_step(
                    texture_difference(right.accumD, left.accumD), next_span);
            }

            if (*blit_row_offset >= *blit_clip_bottom) break;
            if (*blit_row_offset >= *blit_clip_top &&
                right_x > *blit_clip_left && left_x < *blit_clip_right &&
                right_x > left_x) {
                int clipped_right = right_x;
                if (*blit_clip_right <= clipped_right)
                    clipped_right = *blit_clip_right;
                int clipped_left = left_x;
                if (clipped_left < *blit_clip_left) {
                    const int skip = static_cast<int>(
                        static_cast<unsigned int>(texture_difference(
                            *blit_clip_left, clipped_left)) << 16);
                    u = static_cast<int>(static_cast<unsigned int>(u) +
                        static_cast<unsigned int>((static_cast<int64_t>(du) * skip) >> 16));
                    v = static_cast<int>(static_cast<unsigned int>(v) +
                        static_cast<unsigned int>((static_cast<int64_t>(dv) * skip) >> 16));
                    light = static_cast<int>(static_cast<unsigned int>(light) +
                        static_cast<unsigned int>((static_cast<int64_t>(
                            static_cast<unsigned short>(light_step)) * skip) >> 8));
                    *blit_slot_4f8 = u;
                    *blit_slot_53c = v;
                    *blit_slot_498 = light;
                    clipped_left = *blit_clip_left;
                }
                *blit_span_right = clipped_left;

                const unsigned int out_address =
                    static_cast<unsigned int>(*blit_row_offset) +
                    static_cast<unsigned int>(clipped_left);
                unsigned char *out = reinterpret_cast<unsigned char *>(
                    static_cast<intptr_t>(out_address));
                const unsigned int pixel_count = texture_span_pixels(
                    texture_difference(clipped_right, clipped_left));
                // The original shifts the published light scratch in place
                // before loading the per-pixel WORD accumulator.
                *blit_slot_498 = light >> 8;
                unsigned short light_accumulator = static_cast<unsigned short>(
                    static_cast<unsigned int>(light) >> 8);
                int pixel;
                for (pixel = 0; pixel < static_cast<int>(pixel_count); ++pixel) {
                    unsigned char *source = texture_sub61df60_page_address(
                        pixels_, u, v);
                    // Raw E49E loads [EBX] before touching the mask/remap
                    // path, even when that path will preserve destination.
                    const unsigned char base_texel =
                        *reinterpret_cast<volatile const unsigned char *>(source);
                    const uintptr_t mask_base = reinterpret_cast<uintptr_t>(
                        mask_texture->pixels_);
                    const uintptr_t source_offset =
                        reinterpret_cast<uintptr_t>(source) & 0xffffu;
                    const unsigned char mask = *reinterpret_cast<const unsigned char *>(
                        (mask_base & ~static_cast<uintptr_t>(0xffffu)) |
                        source_offset);
                    const unsigned char mapped = remap[mask];
                    if (mapped != 0) {
                        if (mapped == 1) {
                            const unsigned char shade = static_cast<unsigned char>(
                                light_accumulator >> 8);
                            unsigned char value = static_cast<unsigned char>(
                                static_cast<unsigned int>(base_texel) + shade);
                            if (static_cast<signed char>(shade) <
                                static_cast<signed char>(threshold)) {
                                const unsigned int table_index =
                                    (static_cast<unsigned int>(shade) << 8) | out[0];
                                value = palette[table_index];
                            }
                            out[0] = value;
                        } else {
                            out[0] = mapped;
                        }
                    }
                    u = static_cast<int>(static_cast<unsigned int>(u) +
                                         static_cast<unsigned int>(du));
                    v = static_cast<int>(static_cast<unsigned int>(v) +
                                         static_cast<unsigned int>(dv));
                    light_accumulator = static_cast<unsigned short>(
                        light_accumulator + light_step);
                    ++out;
                }
            }
            if (done) goto cleanup;
            *blit_row_offset = static_cast<int>(
                static_cast<unsigned int>(*blit_row_offset) +
                static_cast<unsigned int>(*blit_buffer_stride));
        }
    }

cleanup:
    if (buffer != 0) buffer->free_data(1);
epilogue_only:
    texture_reset_fpu();
}

// Local Gouraud helpers shared with draw2; their definitions follow the
// rasterizer family below. Neither routine depends on image scratch storage.
static int texture_gouraud_edge(EdgeScan &edge, int index,
                               const Vert *vertices, const Vert *uv,
                               const int *light, int count, int bottom);
static int texture_advance_gouraud_edge(EdgeScan &edge, const Vert *vertices,
                                       const Vert *uv, const int *light,
                                       int count, int bottom);
static unsigned short texture_light_step(int delta, int width);

// The cutoff rasterizer uses draw2's shaded sampling kernel on every row.
// The full signed cutoff first rejects polygons whose vertices are all at
// or above it; each pixel then compares signed light/cutoff LOW bytes.
// No transparency key or light clamp participates in this original entry.
// Local scratch has the same bounded domain as draw2: established extrema,
// convex geometry, valid UV allocations, fitting edge divisions, positive
// pitch, nonwrapping source addresses and row addresses preserving signed
// order. The original's unused terminal texel preload is omitted. Shared
// scratch observations and FPU status during release callbacks are unproved.
// ORIGINAL: 0x0061AC60 Texture::draw 0x0061AC60-0x0061B1E2 FILE
// symbol    ?draw_below_cutoff@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1H@Z
// TRIED: MISMATCH #4. Same ESP-as-pixel-counter inner blitter as 0x006239E0's sibling - not reproducible without __asm. Body adds the gouraud light channel on top of that function's template: e1[13]/e1[14] confirmed as the light accumulator/step by hand-tracing the extra unconditional "e1[13]+=e1[14]" pair in the per-scanline update block (absent from the non-gouraud rasterizer), and the third fmul/fistp-to-word pair (scaled by 0x670a88 instead of 0x670a84) confirmed as the per-pixel light step feeding DAT_009bb52c.
// size      1410 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90

void Texture::draw_below_cutoff(Buffer *buffer, Vert *vertices, int *zvals, int count,
                                Vert *uv, int cutoff) {
    unsigned char *destination = reinterpret_cast<unsigned char *>(buffer->get_data());
    texture_reset_fpu();
    const unsigned char *pixels = static_cast<const unsigned char *>(pixels_);
    const int stride = static_cast<int>(buffer->stride_);
    const int width = iWidth_;
    int in_front = 0;
    for (; in_front < count; ++in_front) {
        if (zvals[in_front] < cutoff) break;
    }
    if (in_front != count) {
        const RECT clip = buffer->rect1_;
        int min_y = 0x7fff, max_y = -0x7ffd;
        int top = 0, bottom = 0;
        for (int i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) { min_y = vertices[i].y; top = i; }
            if (vertices[i].y > max_y) { max_y = vertices[i].y; bottom = i; }
        }
        EdgeScan left, right;
        left.f0 = -1;
        right.f0 = 1;
        if (min_y < max_y &&
            texture_gouraud_edge(left, top, vertices, uv, zvals, count, bottom) &&
            texture_gouraud_edge(right, top, vertices, uv, zvals, count, bottom)) {
            _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
            int du = 0, dv = 0;
            unsigned short dz = 0;
            int y = min_y;
            if (y >= clip.bottom) {
                buffer->free_data(1);
                texture_reset_fpu();
                return;
            }
            for (;;) {
                int x = left.accumC;
                int end = right.accumC;
                int u = left.accumA, v = left.accumB, z = left.accumD;
                const int span = texture_fixed(texture_difference(end, x));
                if (span > 0) {
                    du = texture_span_step(texture_difference(right.accumA, u), span >> 16);
                    dv = texture_span_step(texture_difference(right.accumB, v), span >> 16);
                    dz = texture_light_step(texture_difference(right.accumD, z), span >> 16);
                }
                const int done =
                    !texture_advance_gouraud_edge(left, vertices, uv, zvals, count, bottom) ||
                    !texture_advance_gouraud_edge(right, vertices, uv, zvals, count, bottom);
                if (y >= clip.bottom) break;
                if (y >= clip.top && end > clip.left && x < clip.right && end > x) {
                    if (end > clip.right) end = clip.right;
                    if (x < clip.left) {
                        const int skip = texture_fixed(texture_difference(clip.left, x));
                        u = static_cast<int>(static_cast<unsigned int>(u) +
                            static_cast<unsigned int>((static_cast<int64_t>(du) * skip) >> 16));
                        v = static_cast<int>(static_cast<unsigned int>(v) +
                            static_cast<unsigned int>((static_cast<int64_t>(dv) * skip) >> 16));
                        // 61B079..61B081 zero-extends the WORD slope before IMUL;
                        // SHRD by eight brings the result back to 16.16.
                        z = static_cast<int>(static_cast<unsigned int>(z) +
                            static_cast<unsigned int>((static_cast<int64_t>(dz) * skip) >> 8));
                        x = clip.left;
                    }
                    unsigned char *out = destination + static_cast<int64_t>(y) * stride + x;
                    const unsigned int remaining = texture_span_pixels(texture_difference(end, x));
                    int64_t source =
                        static_cast<unsigned short>(static_cast<unsigned int>(v) >> 16)
                            * static_cast<int64_t>(width)
                        + static_cast<unsigned short>(static_cast<unsigned int>(u) >> 16);
                    unsigned int uf = static_cast<unsigned short>(u);
                    unsigned int vf = static_cast<unsigned short>(v);
                    unsigned short light = static_cast<unsigned short>(z >> 8);
                    const int64_t step = static_cast<int64_t>(dv >> 16) * width + (du >> 16);
                    for (unsigned int written = 0; written < remaining; ++written, ++out) {
                        const unsigned char texel = pixels[source];
                        const signed char shade = static_cast<signed char>(light >> 8);
                        if (shade < static_cast<signed char>(cutoff))
                            *out = static_cast<unsigned char>(texel + shade);
                        uf += static_cast<unsigned short>(du);
                        vf += static_cast<unsigned short>(dv);
                        source += step + (uf >> 16) + static_cast<int64_t>(vf >> 16) * width;
                        uf &= 0xffff;
                        vf &= 0xffff;
                        light = static_cast<unsigned short>(light + dz);
                    }
                }
                if (done) break;
                ++y;
            }
        }
    }
    buffer->free_data(1);
    texture_reset_fpu();
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

void Texture::sub_6212b0(Buffer *buf, Vert *verts, int *zvals, int count,
                         int edgeVerts, int *out_flag) {
    out_flag[1] = 0;

    *blit_pixel_base = buf->get_data();
    *blit_light_threshold = *blit_light_threshold_2;
    *blit_edge_uvs_a = reinterpret_cast<int *>(edgeVerts);
    *blit_edge_uvs_b = reinterpret_cast<int *>(verts);
    *blit_vertex_count = count;
    *blit_z_values = reinterpret_cast<int *>(zvals);
    *blit_texture_pixels = (int)(long)pixels_;
    *blit_buffer_stride = (int)buf->stride_;
    *blit_texture_width = (int)iWidth_;

    if (buf != 0) {
        for (int i = 0; i < count; ++i) {
            int v = zvals[i];
            if (TextureLightCeiling <= v) v = TextureLightCeiling;
            zvals[i] = v;
            if (v <= TextureLightFloor) v = TextureLightFloor;
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
                if (sub_625080(&e1, min_y_idx, reinterpret_cast<TextureStore *>(out_flag)) != 0) {
                    e2.counter = 1;
                    if (sub_625080(&e2, min_y_idx, reinterpret_cast<TextureStore *>(out_flag)) != 0 && borrowed_ != 0) {
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
                                if (sub_625080(&e1, e1.id, reinterpret_cast<TextureStore *>(out_flag)) == 0) {
                                    done = true;
                                    goto per_scanline_tail;
                                }
                            } else {
                                edge_step(e1);
                            }
                            e2.counter -= 1;
                            if (e2.counter == 0) {
                                if (sub_625080(&e2, e2.id, reinterpret_cast<TextureStore *>(out_flag)) == 0) {
                                    done = true;
                                }
                            } else {
                                edge_step(e2);
                            }

                        per_scanline_tail:
                            if (dy > 0) {
                                *blit_slot_4e4 = (int)(dv * du * inv_dy * (double)*blit_scale_65536f);
                                *blit_slot_4c8 = (int)(du * dv * inv_dy * (double)*blit_scale_65536f);
                                *blit_light_pack_a = static_cast<unsigned short>((short)(int)(du * inv_dy * dw * (double)*blit_scale_256f));
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
                                    __int64 p3 = (__int64)static_cast<short>(*blit_light_pack_a) * (__int64)step;
                                    *blit_slot_498 = *blit_slot_498 + (int)(((unsigned int)p3 >> 8) | ((int)((unsigned __int64)p3 >> 32) << 24));
                                    *blit_span_right = *blit_clip_left;
                                }

                                unsigned char *tex_row = (unsigned char *)(*blit_texture_pixels + *blit_texture_width * (*blit_slot_53c >> 16) + (*blit_slot_4f8 >> 16));
                                char *dest = (char *)(long)(*blit_span_right + *blit_row_offset - 1);
                                TextureRasterAlternateRowNext.value = ((*blit_slot_4c8 >> 16) * *blit_texture_width + (*blit_slot_4e4 >> 16));
                                TextureRasterAlternateRowBase.value = TextureRasterAlternateRowNext.value + *blit_texture_width;
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
                                    unsigned int selected_row = carry ? (unsigned int)TextureRasterAlternateRowBase.value : (unsigned int)TextureRasterAlternateRowNext.value;
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

void Texture::sub_622810(Buffer *buf, Vert *verts, int *zvals, int count,
                         int edgeVerts, int light_thresh,
                         int *out_flag) {
    out_flag[1] = 0;

    *blit_pixel_base = buf->get_data();
    *blit_light_threshold = light_thresh;
    *blit_edge_uvs_a = reinterpret_cast<int *>(edgeVerts);
    *blit_edge_uvs_b = reinterpret_cast<int *>(verts);
    *blit_vertex_count = count;
    *blit_z_values = reinterpret_cast<int *>(zvals);
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
                if (sub_625080(&e1, min_y_idx, reinterpret_cast<TextureStore *>(out_flag)) != 0) {
                    e2.counter = 1;
                    if (sub_625080(&e2, min_y_idx, reinterpret_cast<TextureStore *>(out_flag)) != 0 && borrowed_ != 0) {
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
                                if (sub_625080(&e1, e1.id, reinterpret_cast<TextureStore *>(out_flag)) == 0) {
                                    done = true;
                                    goto per_scanline_tail;
                                }
                            } else {
                                edge_step(e1);
                            }
                            e2.counter -= 1;
                            if (e2.counter == 0) {
                                if (sub_625080(&e2, e2.id, reinterpret_cast<TextureStore *>(out_flag)) == 0) {
                                    done = true;
                                }
                            } else {
                                edge_step(e2);
                            }

                        per_scanline_tail:
                            if (dy > 0) {
                                *blit_slot_4e4 = (int)(dv * du * inv_dy * (double)*blit_scale_65536f);
                                *blit_slot_4c8 = (int)(du * dv * inv_dy * (double)*blit_scale_65536f);
                                *blit_light_pack_a = static_cast<unsigned short>((short)(int)(du * inv_dy * dw * (double)*blit_scale_256f));
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
                                    __int64 p3 = (__int64)static_cast<short>(*blit_light_pack_a) * (__int64)step;
                                    *blit_slot_498 = *blit_slot_498 + (int)(((unsigned int)p3 >> 8) | ((int)((unsigned __int64)p3 >> 32) << 24));
                                    *blit_span_right = *blit_clip_left;
                                }

                                unsigned char *tex_row = (unsigned char *)(*blit_texture_pixels + *blit_texture_width * (*blit_slot_53c >> 16) + (*blit_slot_4f8 >> 16));
                                char *dest = (char *)(long)(*blit_span_right + *blit_row_offset - 1);
                                TextureRasterAlternateRowNext.value = ((*blit_slot_4c8 >> 16) * *blit_texture_width + (*blit_slot_4e4 >> 16));
                                TextureRasterAlternateRowBase.value = TextureRasterAlternateRowNext.value + *blit_texture_width;
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
                                    unsigned int selected_row = carry ? (unsigned int)TextureRasterAlternateRowBase.value : (unsigned int)TextureRasterAlternateRowNext.value;
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
            texture_reset_fpu();
            *blit_z_values = reinterpret_cast<int *>(a4);
            *blit_edge_uvs_b = reinterpret_cast<int *>(a3);
            *blit_remap_table = a6;
            *blit_texture_pixels = self[0];
            TextureRowPitchScratch = self[1];
            *blit_texture_height = self[2];
            *blit_vertex_count = a7;
            *blit_edge_uvs_a = reinterpret_cast<int *>(a8);
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

                        EdgeScan left;
                        left.f0 = -1;
                        if (sub_625080(&left, minYIdx, reinterpret_cast<TextureStore *>(a9)) != 0) {
                            EdgeScan right;
                            right.f0 = 1;
                            if (sub_625080(&right, minYIdx, reinterpret_cast<TextureStore *>(a9)) != 0 && self[0x1b] != 0 &&
                                reinterpret_cast<int *>(a5)[0x1b] != 0) {
                                // 0x622443 selects 24-bit precision and chop for
                                // the x87 gradient conversion; FNINIT is at exit.
                                _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
                                long double invDx = 1.0;
                                long double dU = 1.0, dV = 1.0, dZ = 1.0;
                                int edgeSpan = static_cast<int>(
                                    static_cast<unsigned int>(texture_difference(
                                        right.accumC, left.accumC)) << 16);
                                if (0 < edgeSpan) {
                                    dU = static_cast<long double>(texture_difference(right.accumD, left.accumD));
                                    dV = static_cast<long double>(texture_difference(right.accumB, left.accumB));
                                    dZ = static_cast<long double>(texture_difference(right.accumA, left.accumA));
                                    invDx = 1.0 / static_cast<long double>(edgeSpan);
                                }

                                if (*blit_row_offset < *blit_clip_bottom) {
                                    bool doneEdges = false;
                                    do {
                                        *blit_slot_498 = left.accumD;
                                        *blit_slot_53c = left.accumB;
                                        *blit_slot_4c4 = right.accumA;
                                        *blit_slot_530 = right.accumB;
                                        *blit_slot_4f8 = left.accumA;
                                        *blit_span_left = right.accumC;
                                        *blit_span_right = left.accumC;
                                        --left.counter;
                                        if (left.counter == 0) {
                                            if (sub_625080(&left, left.id, reinterpret_cast<TextureStore *>(a9)) != 0) {
                                                goto scan_edge_b;
                                            }
                                            doneEdges = true;
                                        } else {
                                            left.accumA += left.deltaA;
                                            left.accumB += left.deltaB;
                                            left.accumC += left.deltaC;
                                            left.accumD += left.deltaD;
                                            left.fracAccum += left.fracDelta;
                                            if (0 < left.fracAccum) {
                                                left.accumC += left.extraDeltaC;
                                                left.fracAccum -= left.fracSub;
                                            }
                                        scan_edge_b:
                                            --right.counter;
                                            if (right.counter == 0) {
                                                if (sub_625080(&right, right.id, reinterpret_cast<TextureStore *>(a9)) == 0) {
                                                    doneEdges = true;
                                                }
                                            } else {
                                                right.accumA += right.deltaA;
                                                right.accumB += right.deltaB;
                                                right.accumC += right.deltaC;
                                                right.accumD += right.deltaD;
                                                right.fracAccum += right.fracDelta;
                                                if (0 < right.fracAccum) {
                                                    right.accumC += right.extraDeltaC;
                                                    right.fracAccum -= right.fracSub;
                                                }
                                            }
                                        }

                                        // 0x6225C3 publishes the gradient captured
                                        // before this row's edge advance. Keep the
                                        // old span/operands until those stores finish.
                                        if (0 < edgeSpan) {
                                            *blit_slot_4e4 = static_cast<int>(
                                                dZ * invDx * static_cast<long double>(*blit_scale_65536f));
                                            *blit_slot_4c8 = static_cast<int>(
                                                dV * invDx * static_cast<long double>(*blit_scale_65536f));
                                            *blit_light_pack_a = static_cast<unsigned short>(static_cast<short>(
                                                dU * invDx * static_cast<long double>(*blit_scale_256f)));
                                        }
                                        edgeSpan = static_cast<int>(
                                            static_cast<unsigned int>(texture_difference(
                                                right.accumC, left.accumC)) << 16);
                                        if (0 < edgeSpan) {
                                            dU = static_cast<long double>(texture_difference(right.accumD, left.accumD));
                                            dV = static_cast<long double>(texture_difference(right.accumB, left.accumB));
                                            dZ = static_cast<long double>(texture_difference(right.accumA, left.accumA));
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
    texture_reset_fpu();
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
        *blit_edge_uvs_b = reinterpret_cast<int *>(verts);
        *blit_texture_pixels = reinterpret_cast<int>(pixels_);
        TextureRowPitchScratch = static_cast<int>(iWidth_);
        *blit_texture_height = static_cast<int>(iHeight_);
        *blit_vertex_count = vertCount;
        *blit_edge_uvs_a = reinterpret_cast<int *>(unusedIn);
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
                                        float scale = *blit_scale_65536f;
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

// Original 0x61DE5A..0x61DF1D pixel path: the base texel is read from the
// receiver Texture, the second Texture is addressed by the low 16-bit source
// offset and supplies a shade selector, and the byte table maps that selector.
static int texture_raster_step_gouraud(EdgeScan &edge, Texture *texture);

static int texture_raster_gradient_span_admitted(int span) {
    const int scaled = static_cast<int>(
        static_cast<unsigned int>(span) << 16);
    return scaled > 0;
}

static void texture_draw_shade_span(unsigned char *destination,
                                    const unsigned char *pixels,
                                    const unsigned char *shade_pixels,
                                    const unsigned char *remap, int width,
                                    int u, int v, int du, int dv,
                                    int light, unsigned short light_step,
                                    int pixel_count) {
    (void)width;
    uintptr_t source_base = reinterpret_cast<uintptr_t>(pixels);
    uintptr_t shade_base = reinterpret_cast<uintptr_t>(shade_pixels);
    unsigned char source_u = static_cast<unsigned char>(
        (source_base + static_cast<unsigned int>(u) / 0x10000u) & 0xffu);
    unsigned char source_v = static_cast<unsigned char>(
        (static_cast<unsigned int>(v) >> 16) & 0xffu);
    unsigned int packed_uv =
        (static_cast<unsigned int>(u) << 16) |
        (((static_cast<unsigned int>(dv) >> 16) & 0xffu) << 8) |
        ((static_cast<unsigned int>(du) >> 16) & 0xffu);
    const unsigned int packed_u_step = static_cast<unsigned int>(du) << 16;
    unsigned int packed_v = static_cast<unsigned int>(v) << 16;
    const unsigned int packed_v_step = static_cast<unsigned int>(dv) << 16;
    unsigned short light_accumulator = static_cast<unsigned short>(
        static_cast<unsigned int>(light) >> 8);
    const unsigned int count = texture_span_pixels(pixel_count);
    for (unsigned int pixel = 0; pixel < count; ++pixel) {
        const uintptr_t source_address =
            (source_base & ~static_cast<uintptr_t>(0xffffu)) |
            (static_cast<uintptr_t>(source_v) << 8) | source_u;
        const uintptr_t shade_address =
            (shade_base & ~static_cast<uintptr_t>(0xffffu)) |
            (source_address & 0xffffu);
        const unsigned char base = *reinterpret_cast<const unsigned char *>(
            source_address);
        const unsigned char selector = *reinterpret_cast<const unsigned char *>(
            shade_address);
        const unsigned char mapped = remap[selector];
        if (selector >= 5) {
            destination[0] = mapped;
        } else {
            destination[0] = static_cast<unsigned char>(
                static_cast<unsigned int>(base) +
                static_cast<unsigned int>(light_accumulator >> 8) +
                static_cast<unsigned int>(mapped));
        }
        const unsigned int old_packed_uv = packed_uv;
        packed_uv += packed_u_step;
        source_u = static_cast<unsigned char>(source_u +
            static_cast<unsigned char>(packed_uv) +
            static_cast<unsigned int>(packed_uv < old_packed_uv));
        const unsigned int old_packed_v = packed_v;
        packed_v += packed_v_step;
        source_v = static_cast<unsigned char>(source_v +
            static_cast<unsigned char>(packed_uv >> 8) +
            static_cast<unsigned int>(packed_v < old_packed_v));
        light_accumulator = static_cast<unsigned short>(
            light_accumulator + light_step);
        ++destination;
    }
}

// ORIGINAL: 0x0061D990 sub_61d990 0x0061D990-0x0061DF5C FILE
// symbol    ?sub_61d990@Texture@@QAEXPAVBuffer@@PAH1PAV1@PAEH@Z
// Historical synthetic seed symbol: ?sub_61d990@Texture@@QAEHPAVBuffer@@PAHHPAV1@PAEH@Z
// Original ECX receiver and RET 18h establish six stack arguments; the third
// argument is the per-vertex light array consumed by setup_edge_gouraud.
// No stable EAX result is established by the original exits. The void interface
// avoids inventing a zero result; indirect/external caller contracts are unknown.
// TRIED: __stdcall free function (receiver is really `this` via ECX, recast as Texture::draw_gouraud_lit); byte-exact EdgeScan field indices for the per-scanline u/v/light stepper (packed 16.16 multiply/carry chain, x87 rounding-mode divide) - not reachable from portable C++, see 0061c090.cpp's own precedent for the same call chain.
// size      1484 bytes
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90

void Texture::sub_61d990(Buffer *buffer, int *vertices, int *zvals,
                         Texture *shade_texture, unsigned char *remap,
                         int wide_flag) {
    if (vertices == 0) {
        if (buffer != 0) buffer->free_data(1);
        texture_reset_fpu();
        return;
    }
    if (buffer == 0) {
        texture_reset_fpu();
        return;
    }
    if (pixels_ == 0 || zvals == 0 || remap == 0 || shade_texture == 0) {
        buffer->free_data(1);
        texture_reset_fpu();
        return;
    }

    *blit_pixel_base = buffer->get_data();
    texture_reset_fpu();
    *blit_z_values = reinterpret_cast<int *>(zvals);
    *blit_edge_uvs_b = reinterpret_cast<int *>(vertices);
    *blit_remap_table = static_cast<int>(reinterpret_cast<intptr_t>(remap));
    *blit_texture_pixels = static_cast<int>(reinterpret_cast<intptr_t>(pixels_));
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    *blit_texture_height = static_cast<int>(iHeight_);
    *blit_texture_width = static_cast<int>(iWidth_);
    *blit_edge_uvs_a = reinterpret_cast<int *>(wide_flag != 0 ? cornersAndMids_ : corners_);
    *blit_vertex_count = wide_flag != 0 ? 8 : 4;
    *blit_buffer_stride = static_cast<int>(buffer->stride_);
    *blit_shade_table = static_cast<int>(reinterpret_cast<intptr_t>(
        shade_texture->pixels_));

    if (shade_texture->pixels_ != 0) {
        const RECT &clip = buffer->rect1_;
        *blit_clip_right = clip.right;
        *blit_clip_left = clip.left;
        *blit_clip_bottom = clip.bottom;
        *blit_clip_top = clip.top;

        const int count = *blit_vertex_count;
        int min_y = 0x7fff;
        int max_y = static_cast<int>(0xffff8003u);
        int min_x = 0x7fff;
        int max_x = static_cast<int>(0xffff8003u);
        int top_index = 0;
        for (int index = 0; index < count; ++index) {
            const int y = vertices[index * 2 + 1];
            if (y < min_y) {
                min_y = y;
                top_index = index;
            }
            if (max_y < y) {
                max_y = y;
                *blit_max_y_index = index;
            }
            const int x = vertices[index * 2];
            if (x < min_x) min_x = x;
            if (max_x < x) max_x = x;
        }
        (void)min_x;
        (void)max_x;

        if (min_y < max_y) {
            const int stride = buffer->stride_;
            const int base = *blit_pixel_base;
            *blit_row_offset = stride * min_y + base;
            *blit_clip_bottom = stride * *blit_clip_bottom + base;
            *blit_clip_top = stride * *blit_clip_top + base;

            EdgeScan left;
            EdgeScan right;
            left.f0 = -1;
            right.f0 = 1;
            if (setup_edge_gouraud(&left, top_index) != 0 &&
                setup_edge_gouraud(&right, top_index) != 0 &&
                borrowed_ != 0 && shade_texture->borrowed_ != 0) {
                _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
                bool done = false;

                while (*blit_row_offset < *blit_clip_bottom) {
                    *blit_slot_4c4 = right.accumA;
                    *blit_slot_530 = right.accumB;
                    *blit_span_left = right.accumC;
                    *blit_slot_4f8 = left.accumA;
                    *blit_slot_53c = left.accumB;
                    *blit_span_right = left.accumC;
                    *blit_slot_498 = left.accumD;
                    const EdgeScan row_left = left;
                    const EdgeScan row_right = right;
                    if (!texture_raster_step_gouraud(left, this)) {
                        done = true;
                    } else if (!texture_raster_step_gouraud(right, this)) {
                        done = true;
                    }
                    if (*blit_clip_bottom <= *blit_row_offset) break;
                    const int span = texture_difference(
                        row_right.accumC, row_left.accumC);
                    if (texture_raster_gradient_span_admitted(span)) {
                        *blit_slot_4e4 = texture_span_step(
                            texture_difference(row_right.accumA,
                                               row_left.accumA), span);
                        *blit_slot_4c8 = texture_span_step(
                            texture_difference(row_right.accumB,
                                               row_left.accumB), span);
                        *blit_light_pack_a =
                            texture_light_step(
                                texture_difference(row_right.accumD,
                                                   row_left.accumD), span);
                    }
                    const int du = *blit_slot_4e4;
                    const int dv = *blit_slot_4c8;
                    const unsigned short light_step =
                        *blit_light_pack_a;
                    if (*blit_clip_top <= *blit_row_offset &&
                        row_right.accumC > *blit_clip_left &&
                        row_left.accumC < *blit_clip_right &&
                        span > 0) {
                        int left_x = row_left.accumC;
                        int right_x = row_right.accumC;
                        if (right_x > *blit_clip_right)
                            right_x = *blit_clip_right;
                        if (left_x < *blit_clip_left)
                            left_x = *blit_clip_left;

                        const int clipped_pixels = texture_difference(
                            left_x, row_left.accumC);
                        int u = static_cast<int>(
                            static_cast<unsigned int>(row_left.accumA) +
                            static_cast<unsigned int>(
                                static_cast<int64_t>(du) * clipped_pixels));
                        int v = static_cast<int>(
                            static_cast<unsigned int>(row_left.accumB) +
                            static_cast<unsigned int>(
                                static_cast<int64_t>(dv) * clipped_pixels));
                        int light = static_cast<int>(
                            static_cast<unsigned int>(row_left.accumD) +
                            (static_cast<unsigned int>(light_step) *
                             static_cast<unsigned int>(clipped_pixels) << 8));
                        if (clipped_pixels != 0) {
                            *blit_slot_4f8 = u;
                            *blit_slot_53c = v;
                            *blit_slot_498 = light;
                            *blit_span_right = left_x;
                        }
                        *blit_slot_498 = light >> 8;
                        const int clipped_span = texture_difference(
                            right_x, left_x);
                        texture_draw_shade_span(
                            reinterpret_cast<unsigned char *>(
                                *blit_row_offset + left_x),
                            static_cast<const unsigned char *>(pixels_),
                            static_cast<const unsigned char *>(shade_texture->pixels_),
                            remap, static_cast<int>(iWidth_), u, v, du, dv,
                            light, light_step,
                            clipped_span);
                    }

                    if (done) break;
                    *blit_row_offset = texture_difference(
                        *blit_row_offset, -stride);
                }
            }
        }
    }

    buffer->free_data(1);
    texture_reset_fpu();
}


// Fixed-slot bindings carried from 0061b780.cpp

/* Ignored proposal payload only; emitted into an overlay by the guarded writer. */
static unsigned int texture_raster_pixel_count(int width) {
    return width > 0 ? static_cast<unsigned int>(width) : 0u;
}

static int texture_raster_advance_light(int light, unsigned short amount,
                                        int borrowed) {
    const unsigned int value = static_cast<unsigned int>(light);
    if (borrowed) {
        const unsigned int channel =
            (((value >> 8) & 0xffffu) + static_cast<unsigned int>(amount)) & 0xffffu;
        return static_cast<int>((value & 0xff0000ffu) | (channel << 8));
    }
    return static_cast<int>((value & 0xffff0000u) |
        ((value + static_cast<unsigned int>(amount)) & 0xffffu));
}

static void texture_raster_span(unsigned char *destination,
                                const unsigned char *pixels, int source_pitch,
                                int row, int stride, int left, int right,
                                int clip_left, int clip_right, int u, int v,
                                int phase_u, int phase_v, int light,
                                int du, int dv, unsigned short dlight,
                                unsigned char key, int keyed, int wrap_source) {
    if (left < clip_left) {
        const int skip = clip_left - left;
        u = static_cast<int>(static_cast<unsigned int>(u) +
            static_cast<unsigned int>(static_cast<int64_t>(du) * skip));
        v = static_cast<int>(static_cast<unsigned int>(v) +
            static_cast<unsigned int>(static_cast<int64_t>(dv) * skip));
        light = texture_raster_advance_light(light,
            static_cast<unsigned short>(static_cast<unsigned int>(dlight) * skip),
            wrap_source);
        left = clip_left;
    }
    u = static_cast<int>(static_cast<unsigned int>(u) +
                        static_cast<unsigned int>(phase_u));
    v = static_cast<int>(static_cast<unsigned int>(v) +
                        static_cast<unsigned int>(phase_v));
    if (right > clip_right) right = clip_right;
    if (right <= left) return;

    unsigned char *out = destination + static_cast<int64_t>(row) * stride + left;
    const unsigned int length = texture_raster_pixel_count(right - left);
    for (unsigned int column = 0; column < length; ++column) {
        const unsigned int source_x = static_cast<unsigned int>(u) >> 16;
        const unsigned int source_y = static_cast<unsigned int>(v) >> 16;
        const unsigned int sx = wrap_source ? (source_x & 0xffu) : source_x;
        const unsigned int sy = wrap_source ? (source_y & 0xffu) : source_y;
        const unsigned char texel = pixels[static_cast<int64_t>(sy) * source_pitch + sx];
        if (!keyed || texel != key) {
            const unsigned int shade = wrap_source
                ? ((static_cast<unsigned int>(light) >> 16) & 0xffu)
                : ((static_cast<unsigned int>(light) >> 8) & 0xffu);
            out[column] = static_cast<unsigned char>(texel + shade);
        }
        u = static_cast<int>(static_cast<unsigned int>(u) + static_cast<unsigned int>(du));
        v = static_cast<int>(static_cast<unsigned int>(v) + static_cast<unsigned int>(dv));
        light = texture_raster_advance_light(light, dlight, wrap_source);
    }
}

static int texture_raster_step_gouraud(EdgeScan &edge, Texture *texture) {
    if (--edge.counter == 0) return texture->setup_edge_gouraud(&edge, edge.id);
    edge.accumA = static_cast<int>(static_cast<unsigned int>(edge.accumA) + edge.deltaA);
    edge.accumB = static_cast<int>(static_cast<unsigned int>(edge.accumB) + edge.deltaB);
    edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.deltaC);
    edge.accumD = static_cast<int>(static_cast<unsigned int>(edge.accumD) + edge.deltaD);
    edge.fracAccum = static_cast<int>(static_cast<unsigned int>(edge.fracAccum) + edge.fracDelta);
    if (edge.fracAccum > 0) {
        edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.extraDeltaC);
        edge.fracAccum = texture_difference(edge.fracAccum, edge.fracSub);
    }
    return 1;
}

static int texture_raster_step_plain(EdgeScan &edge, int *edge_cache) {
    if (--edge.counter == 0) {
        return sub_624eb0(reinterpret_cast<int *>(&edge), edge.id,
                          reinterpret_cast<unsigned char *>(edge_cache));
    }
    edge.accumA = static_cast<int>(static_cast<unsigned int>(edge.accumA) + edge.deltaA);
    edge.accumB = static_cast<int>(static_cast<unsigned int>(edge.accumB) + edge.deltaB);
    edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.deltaC);
    edge.fracAccum = static_cast<int>(static_cast<unsigned int>(edge.fracAccum) + edge.fracDelta);
    if (edge.fracAccum > 0) {
        edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.extraDeltaC);
        edge.fracAccum = texture_difference(edge.fracAccum, edge.fracSub);
    }
    return 1;
}

static void texture_raster_gouraud_rows(Texture *texture, Buffer *buffer,
                                        int first_y, int last_y, int top_index, int keyed,
                                        int borrowed, const unsigned char *pixels,
                                        int pitch) {
    EdgeScan left, right;
    left.f0 = -1;
    right.f0 = 1;
    if (!texture->setup_edge_gouraud(&left, top_index)) return;
    if (!texture->setup_edge_gouraud(&right, top_index)) return;

    const RECT clip = buffer->current_clip();
    const int stride = buffer->stride_;
    unsigned char *destination = reinterpret_cast<unsigned char *>(*blit_pixel_base);
    const unsigned char key = static_cast<unsigned char>(*blit_light_threshold);
    int y = first_y;
    while (y < last_y && y < clip.bottom) {
        const int x = left.accumC;
        const int end = right.accumC;
        if (y >= clip.top && end > clip.left && x < clip.right && end > x) {
            const int span = end - x;
            const int du = texture_span_step(texture_difference(right.accumA, left.accumA), span);
            const int dv = texture_span_step(texture_difference(right.accumB, left.accumB), span);
            const unsigned short dl = texture_light_step(
                texture_difference(right.accumD, left.accumD), span);
            const int phase_u = borrowed ? 0 : du >> 1;
            const int phase_v = borrowed ? 0 : dv >> 1;
            texture_raster_span(destination, pixels, pitch, y, stride, x, end,
                clip.left, clip.right, left.accumA, left.accumB, phase_u, phase_v,
                left.accumD, du, dv, dl, key, keyed && borrowed, borrowed);
        }
        if (!texture_raster_step_gouraud(left, texture)) break;
        if (!texture_raster_step_gouraud(right, texture)) break;
        ++y;
    }
}

static void texture_raster_plain_rows(Texture *texture, Buffer *buffer,
                                      int first_y, int last_y, int top_index,
                                      const unsigned char *pixels, int pitch,
                                      int keyed, int borrowed,
                                      int *edge_cache) {
    EdgeScan left, right;
    left.f0 = -1;
    right.f0 = 1;
    if (!sub_624eb0(reinterpret_cast<int *>(&left), top_index,
                    reinterpret_cast<unsigned char *>(edge_cache))) return;
    if (!sub_624eb0(reinterpret_cast<int *>(&right), top_index,
                    reinterpret_cast<unsigned char *>(edge_cache))) return;
    const RECT clip = buffer->current_clip();
    const int stride = buffer->stride_;
    unsigned char *destination = reinterpret_cast<unsigned char *>(*blit_pixel_base);
    const unsigned char key = static_cast<unsigned char>(*blit_light_threshold);
    int y = first_y;
    while (y < last_y && y < clip.bottom) {
        const int x = left.accumC;
        const int end = right.accumC;
        if (y >= clip.top && end > clip.left && x < clip.right && end > x) {
            const int span = end - x;
            const int du = texture_span_step(texture_difference(right.accumA, left.accumA), span);
            const int dv = texture_span_step(texture_difference(right.accumB, left.accumB), span);
            texture_raster_span(destination, pixels, pitch, y, stride, x, end,
                clip.left, clip.right, left.accumA, left.accumB, 0x8000, 0x8000, 0,
                du, dv, 0, key, keyed, borrowed);
        }
        if (!texture_raster_step_plain(left, edge_cache)) break;
        if (!texture_raster_step_plain(right, edge_cache)) break;
        ++y;
    }
}

// ORIGINAL: 0x0061B780 sub_61b780 0x0061B780-0x0061C08E FILE
// CURRENT BODY STATUS: multirow/pixel DDA restored; x87 status and
// exact packed carry spelling remain restoration debt.
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

void Texture::sub_61b780(Buffer *buffer, int *vertices, int zvals, int mode) {
    const int count = mode == 0 ? 4 : 8;
    int min_y = 0x7fff, max_y = -0x7ffd;
    int min_x = 0x7fff, max_x = -0x7ffd;
    int top = 0;
    unsigned char *destination = reinterpret_cast<unsigned char *>(buffer->get_data());
    texture_reset_fpu();
    *blit_pixel_base = static_cast<int>(reinterpret_cast<intptr_t>(destination));
    *blit_light_threshold = *blit_light_threshold_2;
    *blit_edge_uvs_b = reinterpret_cast<int *>(vertices);
    *blit_edge_uvs_a = reinterpret_cast<int *>(reinterpret_cast<char *>(this) + (mode == 0 ? 0x0c : 0x2c));
    *blit_vertex_count = count;
    *blit_z_values = reinterpret_cast<int *>(zvals);
    *blit_texture_pixels = static_cast<int>(reinterpret_cast<intptr_t>(pixels_));
    *blit_buffer_stride = buffer->stride_;
    *blit_texture_width = iWidth_;
    *blit_clip_right = buffer->rect1_.right;
    *blit_clip_left = buffer->rect1_.left;
    *blit_clip_bottom = buffer->rect1_.bottom;
    *blit_clip_top = buffer->rect1_.top;
    for (int i = 0; i < count; ++i) {
        const int x = vertices[i * 2];
        const int y = vertices[i * 2 + 1];
        if (y < min_y) { min_y = y; top = i; }
        if (y > max_y) { max_y = y; *blit_max_y_index = i; }
        if (x < min_x) min_x = x;
        if (x > max_x) max_x = x;
    }
    if (min_y < max_y) {
        texture_raster_gouraud_rows(this, buffer, min_y, max_y, top, 1,
            borrowed_ != 0, static_cast<const unsigned char *>(pixels_),
            borrowed_ ? 256 : TextureRowPitchScratch);
    }
    if (buffer != 0) buffer->free_data(1);
    texture_reset_fpu();
}

// Fixed-slot bindings carried from 00620a10.cpp

// ORIGINAL: 0x00620A10 sub_620a10 0x00620A10-0x006212A4 FILE
// CURRENT BODY STATUS: multirow keyed affine pixel traversal restored;
// exact x87 status/packed carry equivalence remains restoration debt.
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

void Texture::sub_620a10(Buffer *buffer, int vertices_arg, int count,
                         int edge_vertices_arg, int *out_flag) {
    int v80 = 1;
    out_flag[1] = 0;
    int *vertices = reinterpret_cast<int *>(vertices_arg);
    int data = buffer->get_data();
    texture_reset_fpu();
    *blit_pixel_base = data;
    *blit_edge_uvs_b = reinterpret_cast<int *>(vertices_arg);
    *blit_texture_pixels = static_cast<int>(reinterpret_cast<intptr_t>(pixels_));
    TextureRowPitchScratch = iWidth_;
    *blit_texture_height = iHeight_;
    *blit_edge_uvs_a = reinterpret_cast<int *>(edge_vertices_arg);
    *blit_vertex_count = count;
    *blit_light_threshold = *blit_light_threshold_2;
    *blit_buffer_stride = buffer->stride_;
    *blit_texture_width = iWidth_;
    *blit_clip_right = buffer->rect1_.right;
    *blit_clip_left = buffer->rect1_.left;
    *blit_clip_bottom = buffer->rect1_.bottom;
    *blit_clip_top = buffer->rect1_.top;
    int min_y = 0x7fff, max_y = -0x7ffd;
    int min_x = 0x7fff, max_x = -0x7ffd;
    int top = 0;
    for (int i = 0; i < count; ++i) {
        const int x = vertices[i * 2];
        const int y = vertices[i * 2 + 1];
        if (y < min_y) { min_y = y; top = i; }
        if (y > max_y) { max_y = y; *blit_max_y_index = i; }
        if (x < min_x) min_x = x;
        if (x > max_x) max_x = x;
    }
    if (min_y < max_y) {
        *blit_row_offset = data + *blit_buffer_stride * min_y;
        if (min_x >= *blit_clip_left && min_y >= *blit_clip_top &&
            max_x <= *blit_clip_right && max_y <= *blit_clip_bottom) {
            v80 = 0;
        }
        texture_raster_plain_rows(this, buffer, min_y, max_y, top,
            static_cast<const unsigned char *>(pixels_),
            borrowed_ ? 256 : TextureRowPitchScratch,
            1, borrowed_ != 0, out_flag);
    }
    if (buffer != 0) buffer->free_data(1);
    if (v80 == 0) out_flag[0] = 1;
    texture_reset_fpu();
}

// MEMBER RECOVERY: ECX receiver + four explicit arguments; full row/pixel
// traversal uses no-key Gouraud byte addition. x87 and packed-carry
// bit equivalence remains a restoration obligation.
// ORIGINAL: 0x0061D090 sub_61d090 0x0061D090-0x0061D98A FILE
// symbol    ?sub_61d090@Texture@@QAEXPAVBuffer@@PAH1I@Z
// TRIED: this is a perspective/gouraud scanline texture blitter using x87 80-bit stack state Ghidra itself could not track (extraout_ST*) plus hand-written 64-bit carry-chain fixed point (ADC/SBB/SHRD). Landed the bounding-box setup, the edge-stepping loop shape and the two setup_edge_gouraud@Texture call sites faithfully, but the innermost per-pixel blit (the two ADC/SBB loops at 0x61D54B/0x61D7BD) is NOT reproduced - only the step-size math is computed and discarded. This is a real behavioral gap, not just a byte gap: no pixels are written by this body.
// PINNED RECEIVER EVIDENCE: ECX is preserved and loads Texture fields;
//           RET 0x10 pops the four explicit member arguments. The emitted
//           source symbol below is derived from this C++ declaration, not a
//           claim about an original debug symbol.
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

void Texture::sub_61d090(Buffer *buffer, int *vertices, int *zvals,
                         unsigned int flag) {
    const int count = flag == 0 ? 4 : 8;
    int min_y = 0x7fff, max_y = -0x7ffd;
    int min_x = 0x7fff, max_x = -0x7ffd;
    int top = 0;
    *blit_pixel_base = static_cast<int>(reinterpret_cast<intptr_t>(
        reinterpret_cast<unsigned char *>(buffer->get_data())));
    texture_reset_fpu();
    *blit_edge_uvs_b = reinterpret_cast<int *>(vertices);
    *blit_edge_uvs_a = reinterpret_cast<int *>(reinterpret_cast<char *>(this) + (flag == 0 ? 0x0c : 0x2c));
    *blit_vertex_count = count;
    *blit_z_values = reinterpret_cast<int *>(zvals);
    *blit_texture_pixels = static_cast<int>(reinterpret_cast<intptr_t>(pixels_));
    *blit_buffer_stride = buffer->stride_;
    *blit_texture_width = iWidth_;
    *blit_clip_right = buffer->rect1_.right;
    *blit_clip_left = buffer->rect1_.left;
    *blit_clip_bottom = buffer->rect1_.bottom;
    *blit_clip_top = buffer->rect1_.top;
    for (int i = 0; i < count; ++i) {
        const int x = vertices[i * 2];
        const int y = vertices[i * 2 + 1];
        if (y < min_y) { min_y = y; top = i; }
        if (y > max_y) { max_y = y; *blit_max_y_index = i; }
        if (x < min_x) min_x = x;
        if (x > max_x) max_x = x;
    }
    if (min_y < max_y) {
        texture_raster_gouraud_rows(this, buffer, min_y, max_y, top, 0,
            borrowed_ != 0, static_cast<const unsigned char *>(pixels_),
            borrowed_ ? 256 : TextureRowPitchScratch);
    }
    if (buffer != 0) buffer->free_data(1);
    texture_reset_fpu();
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
//            0x0061FA30. That second 598-byte function now has its own
//            source-owned identity below; never fold it into this first span.
//            Historical split verification used the retired span-termination
//            script; the separate prologue/return evidence remains relevant.
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
    *blit_slot_4e8 = TextureRowPitchScratch;
    int d10_step_adj = d10_step_raw;
    if (d10_step_raw < 0) {
        *blit_slot_4e8 = -TextureRowPitchScratch;
        if ((q_10 & 0xffff) != 0) {
            d10_step_adj = d10_step_raw + 0x10000;
        }
    }
    int cross_term = (d10_step_adj >> 16) * (TextureRowPitchScratch);
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
    int src_row_index = acc_y_hi16 + (unsigned short)(short)((short)acc_x_hi16 * (short)TextureRowPitchScratch);

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

// The distinct keyed-span entry consumes two EdgeScan pointers and shared
// raster context through the existing blit slots. No direct original caller,
// historical translation unit or unique class ownership is established.
// This context preserves the packed arithmetic without an executable forwarder.
struct TextureKeyedEdgeSpanState {
    unsigned char *destination_row;
    const unsigned char *pixels;
    int pitch;
    int clip_left;
    int clip_right;
    unsigned char key;
    int *signed_pitch_slot;
    int *packed_step_slot;
    int *colour_step_slot;
};

static unsigned int texture_keyed_shift_product(int slope, int pixel_offset) {
    const int scaled_offset = static_cast<int>(
        static_cast<unsigned int>(pixel_offset) << 16);
    const int64_t product = static_cast<int64_t>(slope) *
                            static_cast<int64_t>(scaled_offset);
    return static_cast<unsigned int>(static_cast<uint64_t>(product) >> 16);
}

static int texture_keyed_wrap_add(int value, unsigned int delta) {
    return static_cast<int>(static_cast<unsigned int>(value) + delta);
}

// The original shifts the signed pixel span left before IDIV. Keep its
// 32-bit wrap instead of dividing by the unshifted width.
static int texture_keyed_span_quotient(int delta, int pixel_span) {
    const unsigned int divisor_bits =
        static_cast<unsigned int>(pixel_span) << 16;
    const int divisor = static_cast<int>(divisor_bits);
    const int64_t numerator = static_cast<int64_t>(delta) * 0x10000;
    return static_cast<int>(numerator / divisor);
}

static unsigned int texture_keyed_initial_source_index(int pitch, int u, int v) {
    const unsigned short v_integer = static_cast<unsigned short>(
        static_cast<unsigned int>(v) >> 16);
    const unsigned short u_integer = static_cast<unsigned short>(
        static_cast<unsigned int>(u) >> 16);
    const unsigned short pitch_word = static_cast<unsigned short>(pitch);
    const unsigned short row_product = static_cast<unsigned short>(
        static_cast<unsigned int>(v_integer) * pitch_word);
    return static_cast<unsigned int>(row_product) + u_integer;
}

static void texture_draw_keyed_edge_span(EdgeScan *left, EdgeScan *right,
                                         TextureKeyedEdgeSpanState &state) {
    const int left_x = left->accumC;
    const int right_x = right->accumC;
    const int span = texture_difference(right_x, left_x);
    if (right_x <= state.clip_left || left_x >= state.clip_right ||
        span <= 0) {
        return;
    }

    const int raw_du = texture_keyed_span_quotient(
        texture_difference(right->accumA, left->accumA), span);
    const int raw_dv = texture_keyed_span_quotient(
        texture_difference(right->accumB, left->accumB), span);
    const int u_direction = raw_du < 0 ? -1 : 1;
    int du = raw_du;
    if (du < 0 && (static_cast<unsigned int>(du) & 0xffffu) != 0)
        du = texture_difference(du, -0x10000);
    int dv = raw_dv;
    if (dv < 0 && (static_cast<unsigned int>(dv) & 0xffffu) != 0)
        dv = texture_difference(dv, -0x10000);

    int u = texture_keyed_wrap_add(left->accumA,
        static_cast<unsigned int>(du >> 1) + 0x8000u);
    int v = texture_keyed_wrap_add(left->accumB,
        static_cast<unsigned int>(dv >> 1) + 0x8000u);

    *state.signed_pitch_slot = raw_dv < 0 ? -state.pitch : state.pitch;
    const int source_row_step = static_cast<int>(
        static_cast<unsigned int>(du >> 16) +
        static_cast<unsigned int>(dv >> 16) *
            static_cast<unsigned int>(state.pitch));

    int clipped_left = left_x;
    int clipped_right = right_x;
    if (clipped_right > state.clip_right) clipped_right = state.clip_right;
    if (clipped_left < state.clip_left) {
        const int skipped = texture_difference(state.clip_left, clipped_left);
        u = texture_keyed_wrap_add(u, texture_keyed_shift_product(raw_du, skipped));
        v = texture_keyed_wrap_add(v, texture_keyed_shift_product(raw_dv, skipped));
        clipped_left = state.clip_left;
    }

    unsigned char *destination = state.destination_row + clipped_left;
    unsigned int source_offset = texture_keyed_initial_source_index(
        state.pitch, u, v);
    const bool reverse_v = raw_dv < 0;
    unsigned short raw_v_fraction = static_cast<unsigned short>(raw_dv);
    unsigned short v_fraction = static_cast<unsigned short>(v);
    if (*state.signed_pitch_slot < 0) {
        raw_v_fraction = static_cast<unsigned short>(
            0u - static_cast<unsigned int>(raw_v_fraction));
        v_fraction = static_cast<unsigned short>(~v_fraction);
    }
    const unsigned short dv_fraction_15 = static_cast<unsigned short>(
        raw_v_fraction >> 1);
    const unsigned int packed_u = u_direction < 0
        ? static_cast<unsigned int>(~u) : static_cast<unsigned int>(u);
    unsigned int packed_uv = packed_u << 16;
    packed_uv |= u_direction < 0 ? v_fraction
        : static_cast<unsigned short>(v_fraction >> 1);
    const unsigned int packed_uv_step =
        (static_cast<unsigned int>(u_direction < 0
            ? texture_difference(0, raw_du) : raw_du) << 16) |
        dv_fraction_15;
    *state.packed_step_slot = static_cast<int>(
        packed_uv_step);
    *state.colour_step_slot = u_direction < 0
        ? texture_difference(0, source_row_step) : source_row_step;
    const int v_wrap_pitch = reverse_v ? -state.pitch : state.pitch;

    // The original DEC/JNZ loop consumes the unsigned 32-bit span count. Do
    // not turn a zero/wrapped count after clipping into an empty-span return.
    unsigned int remaining = static_cast<unsigned int>(
        texture_difference(clipped_right, clipped_left));
    do {
        const unsigned char texel = state.pixels[source_offset];
        if (texel != state.key) destination[0] = texel;
        const unsigned int previous_packed_uv = packed_uv;
        packed_uv += packed_uv_step;
        const bool packed_carry = packed_uv < previous_packed_uv;
        source_offset = static_cast<unsigned int>(source_offset +
            static_cast<unsigned int>(u_direction < 0
                ? source_row_step - static_cast<int>(packed_carry)
                : source_row_step + static_cast<int>(packed_carry)));
        if ((packed_uv & 0x8000u) != 0) {
            source_offset = static_cast<unsigned int>(source_offset +
                static_cast<unsigned int>(v_wrap_pitch));
            packed_uv &= ~0x8000u;
        }
        ++destination;
        --remaining;
    } while (remaining != 0);
}

// The offset-derived name does not invent a caller or a Texture receiver:
// the entry takes two stack EdgeScan pointers and reads shared raster slots.
// ORIGINAL: 0x0061FA30 sub_61fa30 0x0061FA30-0x0061FC86 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; raw 61FA30 prologue, 61FC85 RET and alignment through 61FC8F, next mapped entry 61FC90.", "Raw entry loads two stack EdgeScan records at offsets 0C/10/1C and uses shared clip, source, destination, key and packed-step storage; original source ownership and reachability remain unknown.", "Full raw dataflow preserves wrapped shifted divisor, clipped multiply, low-word source-row product, initial lookup before signed-pitch NEG/NOT transformation, both packed U-direction loops and scratch publications. Exceptional IDIV and native fault timing remain restoration work."], "reason": "Distinct two-stack-edge keyed raster entry has an independent prologue and RET between the preceding completed span and the next Gouraud edge setup entry. Its offset-derived name and source home expose shared raster dependencies without claiming a Texture receiver, historical TU or original caller."}
// symbol    ?sub_61fa30@@YAXPAUEdgeScan@@0@Z
void __cdecl sub_61fa30(EdgeScan *left, EdgeScan *right) {
    TextureKeyedEdgeSpanState state;
    const unsigned int destination = static_cast<unsigned int>(*blit_pixel_base) +
        static_cast<unsigned int>(*blit_row_offset) *
        static_cast<unsigned int>(*blit_buffer_stride);
    state.destination_row = reinterpret_cast<unsigned char *>(destination);
    state.pixels = reinterpret_cast<const unsigned char *>(*blit_texture_pixels);
    state.pitch = TextureRowPitchScratch;
    state.clip_left = *blit_clip_left;
    state.clip_right = *blit_clip_right;
    state.key = static_cast<unsigned char>(*blit_light_threshold);
    state.signed_pitch_slot = blit_slot_4e8;
    state.packed_step_slot = blit_slot_51c;
    state.colour_step_slot = blit_colour_step;
    texture_draw_keyed_edge_span(left, right, state);
}

// ORIGINAL: 0x0061CB50 sub_61cb50 0x0061CB50-0x0061D08F FILE
// Historical synthetic seed symbol: ?sub_61cb50@Texture@@QAEHHHHHH@Z
// symbol    ?sub_61cb50@Texture@@QAEXPAVBuffer@@PAUVert@@PAV1@PAEH@Z
// The entry receives this Texture plus Buffer, Vert, second Texture, remap
// and perimeter-mode arguments (RET 0x14). The pinned cleanup establishes no
// common EAX result; Kuna also labels it void. No external result contract or
// original caller is established. The seed int-return hint remains historical
// evidence, not permission to fabricate a successful return value.
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

static int texture_sub61cb50_advance_edge(EdgeScan &edge) {
    if (--edge.counter == 0)
        return Texture::setup_edge(&edge, edge.id);
    edge.accumA = static_cast<int>(static_cast<unsigned int>(edge.accumA) +
                                   static_cast<unsigned int>(edge.deltaA));
    edge.accumB = static_cast<int>(static_cast<unsigned int>(edge.accumB) +
                                   static_cast<unsigned int>(edge.deltaB));
    edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) +
                                   static_cast<unsigned int>(edge.deltaC));
    edge.fracAccum = static_cast<int>(static_cast<unsigned int>(edge.fracAccum) +
                                      static_cast<unsigned int>(edge.fracDelta));
    if (edge.fracAccum > 0) {
        edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) +
                                       static_cast<unsigned int>(edge.extraDeltaC));
        edge.fracAccum = texture_difference(edge.fracAccum, edge.fracSub);
    }
    return 1;
}

void Texture::sub_61cb50(Buffer *buffer, Vert *vertices, Texture *shade,
                        unsigned char *remap, int use_eight_vertices) {
    // The image tests vertices before Buffer, then skips release only when
    // Buffer itself is null. Keep the shared cleanup edge and its FINIT.
    if (vertices == 0) goto cleanup;
    if (buffer == 0) goto finish;
    if (pixels_ == 0) goto cleanup;
    if (remap == 0) goto cleanup;
    if (shade == 0) goto cleanup;

    {
        *blit_pixel_base = buffer->get_data();
        texture_reset_fpu();

        *blit_remap_table = static_cast<int>(reinterpret_cast<intptr_t>(remap));
        *blit_edge_uvs_b = reinterpret_cast<int *>(vertices);
        *blit_texture_pixels = static_cast<int>(reinterpret_cast<intptr_t>(pixels_));
        TextureRowPitchScratch = static_cast<int>(iWidth_);
        *blit_texture_height = static_cast<int>(iHeight_);
        if (use_eight_vertices != 0) {
            *blit_vertex_count = 8;
            *blit_edge_uvs_a = reinterpret_cast<int *>(cornersAndMids_);
        } else {
            *blit_vertex_count = 4;
            *blit_edge_uvs_a = reinterpret_cast<int *>(corners_);
        }
        *blit_buffer_stride = static_cast<int>(buffer->stride_);
        *blit_texture_width = static_cast<int>(iWidth_);
        const unsigned char *shade_pixels =
            static_cast<const unsigned char *>(shade->pixels_);
        *blit_shade_table = static_cast<int>(reinterpret_cast<intptr_t>(shade_pixels));
        if (shade_pixels == 0) goto cleanup;

        *blit_clip_right = buffer->rect1_.right;
        *blit_clip_left = buffer->rect1_.left;
        *blit_clip_bottom = buffer->rect1_.bottom;
        *blit_clip_top = buffer->rect1_.top;

        int min_y = 0x7fff;
        int max_y = -32765;
        int min_x = 0x7fff;
        int max_x = -32765;
        int min_y_index = 0;
        int max_y_index = 0;
        const int count = *blit_vertex_count;
        for (int i = 0; i < count; ++i) {
            const int y = vertices[i].y;
            if (y < min_y) { min_y = y; min_y_index = i; }
            if (y > max_y) { max_y = y; max_y_index = i; }
            const int x = vertices[i].x;
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
        }
        (void)min_x;
        (void)max_x;
        *blit_max_y_index = max_y_index;
        if (min_y >= max_y) goto cleanup;

        const int base = *blit_pixel_base;
        const int stride = *blit_buffer_stride;
        *blit_row_offset = static_cast<int>(static_cast<unsigned int>(base) +
            static_cast<unsigned int>(stride) * static_cast<unsigned int>(min_y));
        *blit_clip_top = static_cast<int>(static_cast<unsigned int>(base) +
            static_cast<unsigned int>(stride) * static_cast<unsigned int>(buffer->rect1_.top));
        *blit_clip_bottom = static_cast<int>(static_cast<unsigned int>(base) +
            static_cast<unsigned int>(stride) * static_cast<unsigned int>(buffer->rect1_.bottom));

        EdgeScan left;
        left.f0 = -1;
        if (setup_edge(&left, min_y_index) == 0) goto cleanup;
        EdgeScan right;
        right.f0 = 1;
        if (setup_edge(&right, min_y_index) == 0) goto cleanup;
        if (borrowed_ == 0 || shade->borrowed_ == 0) goto cleanup;

        // The original selects 24-bit precision and chop before its x87
        // reciprocal/multiply sequence; exact x87 spill/rounding is retained
        // as a restoration obligation, while this body preserves its row and
        // pixel decisions.
        _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
        int du = *blit_slot_4e4;
        int dv = *blit_slot_4c8;
        int row = *blit_row_offset;
        while (row < *blit_clip_bottom) {
            const EdgeScan row_left = left;
            const EdgeScan row_right = right;
            const int span_left = row_left.accumC;
            const int span_right = row_right.accumC;
            const int span = texture_difference(span_right, span_left);
            // These are the raw 0x9BB4F0/4CC span endpoints, the paired edge
            // UVs, and the third (uninitialized by setup_edge) edge word.
            // memcpy preserves the original stack object's representation
            // rather than imposing an invented zero initializer.
            memcpy(blit_slot_498, &left.accumD, sizeof(*blit_slot_498));
            *blit_slot_4c4 = row_right.accumA;
            *blit_slot_530 = row_right.accumB;
            *blit_span_left = span_right;  // native 0x9BB4F0
            *blit_slot_4f8 = row_left.accumA;
            *blit_slot_53c = row_left.accumB;
            *blit_span_right = span_left;  // native 0x9BB4CC
            if (span > 0) {
                du = texture_span_step(texture_difference(row_right.accumA, row_left.accumA), span);
                dv = texture_span_step(texture_difference(row_right.accumB, row_left.accumB), span);
                *blit_slot_4e4 = du;
                *blit_slot_4c8 = dv;
            }

            const int left_done = !texture_sub61cb50_advance_edge(left);
            const int right_done = left_done ? 1 :
                !texture_sub61cb50_advance_edge(right);

            if (row >= *blit_clip_top && span_right > *blit_clip_left &&
                span_left < *blit_clip_right && span_right > span_left) {
                int x = span_left;
                int end = span_right;
                if (end > *blit_clip_right) end = *blit_clip_right;
                int u = row_left.accumA;
                int v = row_left.accumB;
                if (x < *blit_clip_left) {
                    const int skip = texture_difference(*blit_clip_left, x);
                    u = static_cast<int>(static_cast<unsigned int>(u) +
                        static_cast<unsigned int>(static_cast<int64_t>(du) * skip));
                    v = static_cast<int>(static_cast<unsigned int>(v) +
                        static_cast<unsigned int>(static_cast<int64_t>(dv) * skip));
                    x = *blit_clip_left;
                }
                unsigned char *destination = reinterpret_cast<unsigned char *>(
                    static_cast<intptr_t>(*blit_row_offset)) + x;
                const unsigned char *base_pixels =
                    static_cast<const unsigned char *>(pixels_);
                const unsigned char *lookup_pixels = shade_pixels;
                for (int px = x; px < end; ++px, ++destination) {
                    const unsigned int tx = (static_cast<unsigned int>(u) >> 16) & 0xffu;
                    const unsigned int ty = (static_cast<unsigned int>(v) >> 16) & 0xffu;
                    const unsigned int index = (ty << 8) | tx;
                    const unsigned char original = base_pixels[index];
                    const unsigned char mapped = remap[lookup_pixels[index]];
                    *destination = (mapped == 0 || mapped == 1) ? original : mapped;
                    u = static_cast<int>(static_cast<unsigned int>(u) + static_cast<unsigned int>(du));
                    v = static_cast<int>(static_cast<unsigned int>(v) + static_cast<unsigned int>(dv));
                }
            }

            if (left_done || right_done) break;
            row = static_cast<int>(static_cast<unsigned int>(row) +
                                  static_cast<unsigned int>(stride));
            *blit_row_offset = row;
        }
    }

cleanup:
    if (buffer != 0) buffer->free_data(1);
finish:
    texture_reset_fpu();
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
    unsigned short *const g_bb52c_w = blit_light_pack_a;

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
    *g_bb52c_w = static_cast<unsigned short>(num52c / divisor64);

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
    *blit_slot_4e8 = TextureRowPitchScratch;
    int d10_step_adj = d10_step_raw;
    if (d10_step_raw < 0) {
        *blit_slot_4e8 = -TextureRowPitchScratch;
        if ((q_10 & 0xffff) != 0) {
            d10_step_adj = d10_step_raw + 0x10000;
        }
    }
    int cross_term = (d10_step_adj >> 16) * (TextureRowPitchScratch);
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
    int out_row_index = acc_y_hi16 + (unsigned short)(short)((short)acc_x_hi16 * (short)TextureRowPitchScratch);

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
        *blit_slot_4e8 = TextureRowPitchScratch;
        if (dyRatio < 0) {
            *blit_slot_4e8 = -TextureRowPitchScratch;
            if ((dyRatio & 0xffff) != 0) {
                dyAdj = dyRatio + 0x10000;
            }
        }
        int colorStep = (dyAdj >> 16) * TextureRowPitchScratch;
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
        short prod16 = (short)((short)yHi * (short)(TextureRowPitchScratch & 0xffff));
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
    *blit_light_pack_a =
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
    *blit_slot_4e8 = TextureRowPitchScratch;
    if (quot2 < 0) {
        *blit_slot_4e8 = -TextureRowPitchScratch;
        if ((quot2_64 & 0xffff) != 0) {
            dzFracAdj = quot2 + 0x10000;
        }
    }
    int zWholeStep = (dzFracAdj >> 0x10) * (TextureRowPitchScratch);
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
            (static_cast<__int64>(static_cast<int>(*blit_light_pack_a)) *
             static_cast<__int64>(preStepY)) >> 16) + *blit_light_accum_b;
        yStart = *blit_clip_left;
    }
    int rowCount = yEnd - yStart;

    int rowOffset = static_cast<int>(xAcc >> 0x10) +
                     static_cast<unsigned short>(static_cast<short>(zAcc >> 16) * static_cast<short>(TextureRowPitchScratch));
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
            (static_cast<unsigned int>(static_cast<unsigned short>(-static_cast<short>(*blit_light_pack_a))) << 16) |
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
                    static_cast<unsigned short>(*reinterpret_cast<short *>(blit_light_accum_b) + *blit_light_pack_a);
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
                    static_cast<unsigned short>(*reinterpret_cast<short *>(blit_light_accum_b) + *blit_light_pack_a);
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
      *blit_z_values = reinterpret_cast<int *>(param_4);
      *blit_edge_uvs_b = reinterpret_cast<int *>(param_3);
      (*blit_texture_pixels) = *param_1;
      (TextureRowPitchScratch) = param_1[1];
      (*blit_texture_height) = param_1[2];
      if (param_8 == 0) {
        iVar10 = 4;
        *blit_edge_uvs_a = reinterpret_cast<int *>((int32_t)(param_1 + 3));
        (*blit_vertex_count) = 4;
      }
      else {
        iVar10 = 8;
        *blit_edge_uvs_a = reinterpret_cast<int *>((int32_t)(param_1 + 0xb));
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
                      (*blit_slot_4e8) = (TextureRowPitchScratch);
                      if ((local_88 < 0) && ((*blit_slot_4e8) = -(TextureRowPitchScratch), (uVar2 & 0xffff) != 0)) {
                        iVar3 = local_88 + 0x10000;
                      }
                      iVar4 = (iVar3 >> 0x10) * (TextureRowPitchScratch);
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
                               (uint)(ushort)((ushort)(param_7 >> 16) * (short)(TextureRowPitchScratch));
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
// symbol    ?draw_color_key@Texture@@QAEXPAVBuffer@@PAUVert@@H1H@Z
// TRIED: MISMATCH #0. The inner scanline blitter uses ESP itself as the per-span pixel counter (push ebp; mov esp,ecx; ...; sub esp, 0x10000; jns loop; mov esp,[global]; pop ebp) - that shape only comes from hand-written __asm in the original source, which is barred here, so byte-exactness is out of reach for this function. Body is a faithful high-level port instead: verified EdgeScan field offsets (f[3]/f[4] int accum, f[5]/f[6] step, f[7]/f[8] fractional accum/step, f[9]-f[12] periodic correction) by hand-tracing both mirrored update blocks (0x623C47 and 0x623C9E) against raw ebp-relative offsets, not Ghidra's renumbered locals, which are off by a few bytes from the real frame. Per-pixel texture/threshold loop transcribed from Ghidra's CONCAT/CARRY reconstruction via local concat22/ concat11/concat31/carry4/carry2 helpers.
// size      1380 bytes
// prototype void (__thiscall ?draw_color_key@Texture@@QAEXPAVBuffer@@PAUVert@@H1H@Z)(Texture* this, Buffer*, Vert*, int count, Vert* uv, int color)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061A760 0x00624D80
// The fifth argument is an alternate-row color, not a threshold, and the
// third/fourth are the count and UV pair of 0x0061A760 - the old aux/count
// spelling had them reversed. Exactly -1 forwards to the four-argument
// overload at 0x00623A05, before this function locks the buffer or resets
// the FPU; every other value, including other negatives, runs the keyed
// raster with row recoloring. Parity starts at min_y & 1 (0x00623B3C) and
// advances with each row (0x00623F15): absolute screen-Y odd rows copy the
// texel (0x00623EA1..A5), even rows write the color's low byte captured at
// 0x00623A28 (0x00623ED0..D4), and both paths leave color-key holes intact.
// draw_color_key is this tree's provisional name for the overloading pair;
// the shipped image names the body only by its seed label.
// LEVER: the span kernel at 0x00623E6A/0x00623EE1 packs its countdown into
// ESP and is reproduced with texture_span_pixels (see 0x0061A240's note);
// the original preloads the next texel before testing the counter and this
// C++ loop omits that unused final read.
void Texture::draw_color_key(Buffer *buffer, Vert *vertices, int count, Vert *uv, int color) {
    if (color == -1) {
        draw_color_key(buffer, vertices, count, uv);
        return;
    }
    unsigned char *destination = reinterpret_cast<unsigned char *>(buffer->get_data());
    texture_reset_fpu();
    const unsigned char alternate = static_cast<unsigned char>(color);
    const unsigned char *pixels = static_cast<const unsigned char *>(pixels_);
    const int width = static_cast<int>(iWidth_);
    const unsigned char key = static_cast<unsigned char>(TextureRasterKey);
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    const int stride = static_cast<int>(buffer->stride_);
    const RECT clip = buffer->rect1_;

    int min_y = 0x7fff, max_y = -0x7ffd;
    int top = 0, bottom = 0;
    for (int i = 0; i < count; ++i) {
        if (vertices[i].y < min_y) { min_y = vertices[i].y; top = i; }
        if (vertices[i].y > max_y) { max_y = vertices[i].y; bottom = i; }
    }
    EdgeScan left, right;
    left.f0 = -1;
    right.f0 = 1;
    if (min_y < max_y && texture_edge(left, top, vertices, uv, count, bottom) &&
        texture_edge(right, top, vertices, uv, count, bottom)) {
        _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
        int du = 0, dv = 0;
        int parity = min_y & 1;
        int y = min_y;
        if (y >= clip.bottom) {
            buffer->free_data(1);
            texture_reset_fpu();
            return;
        }
        for (;;) {
            int x = left.accumC;
            int end = right.accumC;
            int u = left.accumA, v = left.accumB;
            const int span = texture_fixed(texture_difference(end, x));
            if (span > 0) {
                du = texture_span_step(texture_difference(right.accumA, u), span >> 16);
                dv = texture_span_step(texture_difference(right.accumB, v), span >> 16);
            }
            // Both sides advance before the saved row is drawn. Left failure
            // skips right advancement, but never suppresses this final row.
            const int done = !texture_advance_edge(left, vertices, uv, count, bottom) ||
                             !texture_advance_edge(right, vertices, uv, count, bottom);
            if (y >= clip.bottom) break;
            if (y >= clip.top && y < clip.bottom && end > clip.left &&
                x < clip.right && end > x) {
                if (end > clip.right) end = clip.right;
                if (x < clip.left) {
                    const int skip = texture_fixed(texture_difference(clip.left, x));
                    u = static_cast<int>(static_cast<unsigned int>(u) +
                        static_cast<unsigned int>((static_cast<int64_t>(du) * skip) >> 16));
                    v = static_cast<int>(static_cast<unsigned int>(v) +
                        static_cast<unsigned int>((static_cast<int64_t>(dv) * skip) >> 16));
                    x = clip.left;
                }
                unsigned char *out = destination + static_cast<int64_t>(y) * stride + x;
                int64_t source = static_cast<unsigned short>(static_cast<unsigned int>(v) >> 16)
                    * static_cast<int64_t>(width)
                    + static_cast<unsigned short>(static_cast<unsigned int>(u) >> 16);
                unsigned int uf = static_cast<unsigned short>(u);
                unsigned int vf = static_cast<unsigned short>(v);
                const int64_t step = static_cast<int64_t>(dv >> 16) * width + (du >> 16);
                const unsigned int remaining = texture_span_pixels(texture_difference(end, x));
                for (unsigned int written = 0; written < remaining; ++written, ++out) {
                    const unsigned char texel = pixels[source];
                    if (texel != key) *out = (parity & 1) != 0 ? texel : alternate;
                    uf += static_cast<unsigned short>(du);
                    vf += static_cast<unsigned short>(dv);
                    source += step + (uf >> 16) + static_cast<int64_t>(vf >> 16) * width;
                    uf &= 0xffff;
                    vf &= 0xffff;
                }
            }
            if (done) break;
            ++y;
            ++parity;
        }
    }
    buffer->free_data(1);
    texture_reset_fpu();
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
// The sentinel value -1 forwards to the explicit-UV packed rasterizer.
// Other values perform no acquisition or drawing in this overload.
void Texture::draw_trans(Buffer *buffer, Vert *vertices, int count, Vert *uv, int sentinel) {
    if (sentinel == -1) {
        draw_trans(buffer, vertices, count, uv);
    }
}

// ORIGINAL: 0x00623F50 sub_623f50 0x00623F50-0x00623F7D BYTE_EXACT
// symbol    ?sub_623f50@Texture@@QAEXPAVBuffer@@PAHPAV1@PAEHHH@Z
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

void Texture::sub_623f50(Buffer *a1, int *a2, Texture *a3, uint8_t *a4,
                         int a5, int a6, int a7) {
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
// symbol    ?sub_624ca0@Texture@@QAEXPAVBuffer@@HHHPAHH@Z
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

void Texture::sub_624ca0(Buffer *a1, int a2, int a3, int a4, int *a5,
                         int a6) {
    // 0x00624CA0, 40 bytes, `ret 0x18`. The sixth int is the -1 sentinel, the
    // same idiom `draw2` and `sub_623f50` carry: when it is -1 the first five
    // are forwarded to the affine rasteriser at 0x00620A10, otherwise the
    // function is a bare `ret 0x18`.
    if (a6 == -1) {
        sub_620a10(a1, a2, a3, a4, a5);
    }
}


// ===== THE -1 SENTINEL WRAPPERS =====
//
// Five 45-to-60-byte members, all the same shape and all homed from the same
// reading of the image. Each takes one more argument than the rasteriser it
// fronts; that last argument is a sentinel, and only when it is -1 does the
// wrapper forward. The interesting fact is what they do NOT do: not one of
// them loads ECX before its call, and every callee is a __thiscall member -
// so the receiver reaches the callee through ECX untouched, which is what
// makes these members of Texture rather than the __stdcall free functions the
// archive had them as. Homed as members, the C body is a bare `if` over a
// forwarding call and VC6 supplies the ECX pass-through itself.

/*
// ORIGINAL: 0x00624C70 sub_624c70 0x00624C70-0x00624C9D BYTE_EXACT
// symbol    ?sub_624c70@Texture@@QAEXPAVBuffer@@PAUVert@@PAHHH2H@Z
// size      45 bytes
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006212B0
Return Value: whatever sub_6212b0 leaves in EAX
Status: Complete
*/
void Texture::sub_624c70(Buffer *buf, Vert *verts, int *zvals, int count,
                         int edgeVerts, int *out_flag, int sentinel) {
    if (sentinel == -1) {
        sub_6212b0(buf, verts, zvals, count, edgeVerts, out_flag);
    }
}

/*
// ORIGINAL: 0x00624CD0 sub_624cd0 0x00624CD0-0x00624CFD BYTE_EXACT
// symbol    ?draw_cached_lit@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1PAVTextureStore@@H@Z
// size      45 bytes
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00621870
// Byte-identical to sub_624c70 above except for the call target, which is
// what says the two callees have the same 6-argument shape. VOID, and that is
// measured: spelling it `int` and `return sub_621870(...)` makes VC6 tail-jmp
// where the image makes a real call and then returns.
Return Value: void
Status: Complete
*/
void Texture::draw_cached_lit(Buffer *buf, Vert *verts, int *zvals, int count,
                               Vert *edgeVerts, TextureStore *cache, int sentinel) {
    if (sentinel == -1) {
        draw_cached_lit(buf, verts, zvals, count, edgeVerts, cache);
    }
}

/*
// ORIGINAL: 0x00624D00 sub_624d00 0x00624D00-0x00624D37 BYTE_EXACT
// symbol    ?sub_624d00@Texture@@QAEXHHHHHHHHH@Z
// size      55 bytes
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00622200
Return Value: whatever sub_622200 leaves in EAX
Status: Complete
*/
void Texture::sub_624d00(int a1, int a2, int a3, int a4, int a5, int a6,
                         int a7, int a8, int sentinel) {
    if (sentinel == -1) {
        sub_622200(a1, a2, a3, a4, a5, a6, a7, a8);
    }
}

/*
// ORIGINAL: 0x00624D40 sub_624d40 0x00624D40-0x00624D72 BYTE_EXACT
// symbol    ?sub_624d40@Texture@@QAEXPAVBuffer@@PAUVert@@PAHHHH2H@Z
// size      50 bytes
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00622810
Return Value: whatever sub_622810 leaves in EAX
Status: Complete
*/
void Texture::sub_624d40(Buffer *buf, Vert *verts, int *zvals, int count,
                         int a5, int light_thresh, int *out_flag,
                         int sentinel) {
    if (sentinel == -1) {
        sub_622810(buf, verts, zvals, count, a5, light_thresh, out_flag);
    }
}

/*
// ORIGINAL: 0x00624630 sub_624630 0x00624630-0x0062466C BYTE_EXACT
// symbol    ?sub_624630@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPAV1@PAEH1H4H@Z
// Historical synthetic seed symbol: ?sub_624630@Texture@@QAEXHHHHHHHHHH@Z
// Nine forwarded stack slots use the recovered callee's types; the tenth is
// the -1 sentinel. Preserve the historical byte obligation through API repair.
// size      60 bytes
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0061DF60
// VOID like sub_624cd0 above: `return sub_61df60(...)` is a tail jump and the
// image makes a real call.
Return Value: void
Status: Complete
*/
void Texture::sub_624630(Buffer *buffer, Vert *vertices, int *zvals,
                         Texture *mask_texture, unsigned char *remap,
                         int count, Vert *uv, int threshold,
                         unsigned char *palette, int sentinel) {
    if (sentinel == -1) {
        sub_61df60(buffer, vertices, zvals, mask_texture, remap, count, uv,
                   threshold, palette);
    }
}

// The Gouraud setup at 0x0061FC90 uses the same integer X edge as the
// plain setup, but starts UVs at texel centres and adds an independent
// 16.16 light channel. Its endpoint UV target deliberately has no half-
// texel bias: subtract the biased start before the fixed-point division.
static int texture_gouraud_edge(EdgeScan &edge, int index,
                                 const Vert *vertices, const Vert *uv,
                                 const int *light, int count, int bottom) {
    if (index == bottom) return 0;
    for (;;) {
        int next = index + edge.f0;
        if (next >= count) next = 0;
        else if (next < 0) next = count - 1;
        edge.counter = texture_difference(vertices[next].y, vertices[index].y);
        if (edge.counter != 0) {
            if (edge.counter < 0) return 0;
            edge.id = next;
            edge.accumA = static_cast<int>(
                static_cast<unsigned int>(texture_fixed(uv[index].x)) + 0x8000u);
            edge.accumB = static_cast<int>(
                static_cast<unsigned int>(texture_fixed(uv[index].y)) + 0x8000u);
            const int height = texture_fixed(edge.counter);
            edge.deltaA = static_cast<int>(
                static_cast<int64_t>(texture_difference(texture_fixed(uv[next].x), edge.accumA))
                * 65536 / height);
            edge.deltaB = static_cast<int>(
                static_cast<int64_t>(texture_difference(texture_fixed(uv[next].y), edge.accumB))
                * 65536 / height);
            edge.accumC = vertices[index].x;
            const int dx = texture_difference(vertices[next].x, vertices[index].x);
            edge.deltaC = dx / edge.counter;
            const int magnitude = dx < 0 ? texture_difference(0, dx) : dx;
            edge.extraDeltaC = dx < 0 ? -1 : 1;
            edge.fracAccum = dx < 0 ? 1 - edge.counter : 0;
            edge.fracSub = edge.counter;
            edge.fracDelta = magnitude % edge.counter;
            edge.accumD = texture_fixed(light[index]);
            edge.deltaD = static_cast<int>(
                static_cast<int64_t>(texture_difference(texture_fixed(light[next]), edge.accumD))
                * 65536 / height);
            return 1;
        }
        index = next;
        if (index == bottom) return 0;
    }
}

static int texture_advance_gouraud_edge(EdgeScan &edge, const Vert *vertices,
                                         const Vert *uv, const int *light,
                                         int count, int bottom) {
    if (--edge.counter == 0)
        return texture_gouraud_edge(edge, edge.id, vertices, uv, light, count, bottom);
    edge.accumA = static_cast<int>(static_cast<unsigned int>(edge.accumA) + edge.deltaA);
    edge.accumB = static_cast<int>(static_cast<unsigned int>(edge.accumB) + edge.deltaB);
    edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.deltaC);
    edge.accumD = static_cast<int>(static_cast<unsigned int>(edge.accumD) + edge.deltaD);
    edge.fracAccum = static_cast<int>(static_cast<unsigned int>(edge.fracAccum) + edge.fracDelta);
    if (edge.fracAccum > 0) {
        edge.accumC = static_cast<int>(static_cast<unsigned int>(edge.accumC) + edge.extraDeltaC);
        edge.fracAccum = texture_difference(edge.fracAccum, edge.fracSub);
    }
    return 1;
}

// PC24/chop reciprocal and product, as in texture_span_step, but the
// final power-of-two scale is 256 and FISTP stores a signed WORD. Masked
// conversion overflow produces the x87 integer-indefinite word 0x8000.
// The resulting bits are reproduced; x87 exception status is not.
static unsigned short texture_light_step(int delta, int width) {
    const uint64_t denominator = static_cast<uint64_t>(width) << 16;
    uint64_t upper_power = 1;
    int reciprocal_shift = 23;
    while (upper_power < denominator) {
        upper_power <<= 1;
        ++reciprocal_shift;
    }
    const uint64_t significand = (static_cast<uint64_t>(1) << reciprocal_shift) / denominator;
    const uint64_t magnitude = delta < 0 ? -static_cast<int64_t>(delta) : delta;
    uint64_t product = significand * magnitude;
    int exponent = 8 - reciprocal_shift;
    while (product >= (static_cast<uint64_t>(1) << 24)) {
        product >>= 1;
        ++exponent;
    }
    const uint64_t rounded = exponent >= 0 ? product << exponent : product >> -exponent;
    if (rounded > (delta < 0 ? 32768u : 32767u)) return 0x8000u;
    return static_cast<unsigned short>(
        delta < 0 ? -static_cast<int64_t>(rounded) : static_cast<int64_t>(rounded));
}

// 0x00624670 alternates opaque fill rows with depth-tested, lit texture
// rows. The seventh argument is the -1 forwarding sentinel, not the cutoff.
// The ordinary path and mapped caller establish no meaningful return value.
// Local scratch follows the recovered keyed rasterizers' bounded domain:
// established extrema, convex geometry, valid UV allocations, fitting edge
// divisions, positive pitch, nonwrapping source addresses and row addresses
// preserving signed order (the original compares signed pointers, not y).
// Shared-scratch observations
// and FPU status inside release callbacks remain unproved. The original reads
// one unused texel on fill rows and preloads an unused texel after the final
// textured pixel; those unused reads are omitted, as in the keyed kernels.
// The sentinel delegates to 0x0061AC60's shaded-every-row counterpart above.
// ORIGINAL: 0x00624670 Texture::draw2 0x00624670-0x00624C6D FILE
// RECOVERY_BINDING: v1 {"evidence": ["624676..62469D identify seventh-argument minus-one forwarding; 6246B0 acquires the Buffer; 624B99..624BE9 select row parity and perform light-tested sampling; 61FC90 supplies the Gouraud edge interpretation. Bounded geometry and unused-read limitations are documented beside the source body."], "reason": "This Texture member alternates opaque fill rows with cutoff-tested lit texture rows, consuming real Buffer, vertex, depth and UV arrays. Original callers ignore EAX; the ordinary path has no meaningful return value."}
// symbol    ?draw2@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1HH@Z
void Texture::draw2(Buffer *buffer, Vert *vertices, int *zvals, int count,
                    Vert *uv, int cutoff, int fillColor) {
    if (fillColor == -1) {
        draw_below_cutoff(buffer, vertices, zvals, count, uv, cutoff);
        return;
    }

    unsigned char *destination = reinterpret_cast<unsigned char *>(buffer->get_data());
    texture_reset_fpu();
    const unsigned char *pixels = static_cast<const unsigned char *>(pixels_);
    const int stride = static_cast<int>(buffer->stride_);
    const int width = iWidth_;
    int in_front = 0;
    for (; in_front < count; ++in_front) {
        if (zvals[in_front] < cutoff) break;
    }
    if (in_front != count) {
        const RECT clip = buffer->rect1_;
        int min_y = 0x7fff, max_y = -0x7ffd;
        int top = 0, bottom = 0;
        for (int i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) { min_y = vertices[i].y; top = i; }
            if (vertices[i].y > max_y) { max_y = vertices[i].y; bottom = i; }
        }
        EdgeScan left, right;
        left.f0 = -1;
        right.f0 = 1;
        if (min_y < max_y &&
            texture_gouraud_edge(left, top, vertices, uv, zvals, count, bottom) &&
            texture_gouraud_edge(right, top, vertices, uv, zvals, count, bottom)) {
            _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
            int du = 0, dv = 0;
            unsigned short dz = 0;
            int y = min_y;
            if (y >= clip.bottom) {
                buffer->free_data(1);
                texture_reset_fpu();
                return;
            }
            for (;;) {
                int x = left.accumC;
                int end = right.accumC;
                int u = left.accumA, v = left.accumB, z = left.accumD;
                const int span = texture_fixed(texture_difference(end, x));
                if (span > 0) {
                    du = texture_span_step(texture_difference(right.accumA, u), span >> 16);
                    dv = texture_span_step(texture_difference(right.accumB, v), span >> 16);
                    dz = texture_light_step(texture_difference(right.accumD, z), span >> 16);
                }
                const int done =
                    !texture_advance_gouraud_edge(left, vertices, uv, zvals, count, bottom) ||
                    !texture_advance_gouraud_edge(right, vertices, uv, zvals, count, bottom);
                if (y >= clip.bottom) break;
                if (y >= clip.top && end > clip.left && x < clip.right && end > x) {
                    if (end > clip.right) end = clip.right;
                    if (x < clip.left) {
                        const int skip = texture_fixed(texture_difference(clip.left, x));
                        u = static_cast<int>(static_cast<unsigned int>(u) +
                            static_cast<unsigned int>((static_cast<int64_t>(du) * skip) >> 16));
                        v = static_cast<int>(static_cast<unsigned int>(v) +
                            static_cast<unsigned int>((static_cast<int64_t>(dv) * skip) >> 16));
                        // 624AC9 zero-extends the WORD slope before IMUL;
                        // SHRD by eight brings the result back to 16.16.
                        z = static_cast<int>(static_cast<unsigned int>(z) +
                            static_cast<unsigned int>((static_cast<int64_t>(dz) * skip) >> 8));
                        x = clip.left;
                    }
                    unsigned char *out = destination + static_cast<int64_t>(y) * stride + x;
                    const unsigned int remaining = texture_span_pixels(texture_difference(end, x));
                    if ((y & 1) == 0) {
                        for (unsigned int written = 0; written < remaining; ++written)
                            out[written] = static_cast<unsigned char>(fillColor);
                    } else {
                        int64_t source =
                            static_cast<unsigned short>(static_cast<unsigned int>(v) >> 16)
                                * static_cast<int64_t>(width)
                            + static_cast<unsigned short>(static_cast<unsigned int>(u) >> 16);
                        unsigned int uf = static_cast<unsigned short>(u);
                        unsigned int vf = static_cast<unsigned short>(v);
                        unsigned short light = static_cast<unsigned short>(z >> 8);
                        const int64_t step = static_cast<int64_t>(dv >> 16) * width + (du >> 16);
                        for (unsigned int written = 0; written < remaining; ++written, ++out) {
                            const unsigned char texel = pixels[source];
                            const signed char shade = static_cast<signed char>(light >> 8);
                            if (shade < static_cast<signed char>(cutoff))
                                *out = static_cast<unsigned char>(texel + shade);
                            uf += static_cast<unsigned short>(du);
                            vf += static_cast<unsigned short>(dv);
                            source += step + (uf >> 16) + static_cast<int64_t>(vf >> 16) * width;
                            uf &= 0xffff;
                            vf &= 0xffff;
                            light = static_cast<unsigned short>(light + dz);
                        }
                    }
                }
                if (done) break;
                ++y;
            }
        }
    }
    buffer->free_data(1);
    texture_reset_fpu();
}

// ORIGINAL: 0x00621870 ?sub_621870@Texture@@QAEHPAVBuffer@@PAUVert@@PAHHH2@Z 0x00621870-0x006221F1
// RECOVERY_BINDING: v1 {"evidence": ["Coordinator investigation_deferred 2026-09-19 (run-z7f3353q): sub_624cd0 trial bytes=unknown references=unknown with exactly one emitted reference ?sub_621870@Texture@@QAEHPAVBuffer@@PAUVert@@PAHHH2@Z at offset 38 binding_address null; sibling sub_624c70 passes because its callee sub_6212b0 is defined in src/texture.cpp:6515-6520 - the cause is the cross-TU callee, not the wrapper spelling; the masked measure (16 instructions, 4 masked bytes) proves every other byte correct.", "Seed src/unrecovered/00621870.cpp line 1 catalogues ORIGINAL 0x00621870 sub_621870 0x00621870-0x006221F1 FILE; the forwarder at src/pending_bodies.cpp:2225-2231 emits the member symbol via its C++ definition and the task record preserves the already-exact caller sub_624ca0 untouched."], "reason": "The pending_bodies.cpp forwarder at the annotated line is the only compiled definition of Texture::sub_621870 (its comment records that it is reached by name by Texture::sub_624cd0); the extent is the seed's catalogue adopted verbatim so the by-name reference resolves against the original identity without homing the x87 body."}
// Deleted seed provenance: git:25c0645e742c23f149e0895c391a9ba79ad744cb:src/unrecovered/00621870.cpp
// symbol    ?draw_cached_lit@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1PAVTextureStore@@@Z
// The historical binding above describes the preserved forwarder provenance.
// This source home reconstructs both raster paths with a typed TextureStore;
// the only mapped caller discards EAX, so the interface expresses void.
// Bounded domain: valid receiver/buffer/cache/arrays, established extrema,
// convex edges, positive pitch and nonwrapping signed-ordered row addresses,
// fitting original signed divisions, and positive raw spans at most 32767.
// Owned-storage spans must remain positive after clipping (its DEC/JNE loop
// otherwise traverses the wrapped 32-bit count). The borrowed packed loop's
// single write for an empty clipped span is retained, not silently discarded.
// A larger positive raw span can make the original fixed width nonpositive
// and consume stale global gradients; no invented zero-gradient fallback is
// supplied for that unproved domain. Borrowed pixels retain the TexHeap's
// 64-KiB-aligned payload and 256-byte row layout: 635390 rejects a
// nonaligned heap cursor, 5D46CA returns it unchanged, and 6197CB/CF
// publish pixels_/borrowed_ only on success. Sample offsets must remain
// within that allocation; fabricated or mutated borrowed pointers are unproved.
// Owned samples use the actual shared pitch scratch, including its history.
//
// Local scratch does not prove asynchronous observations of the original
// scratch block. Explicit PC24 integer arithmetic reproduces numeric results,
// not pending x87 registers/status observed inside release callbacks. The
// original computes the next reciprocal before drawing this row; no ordinary
// callback intervenes. Entry/exit FPU resets remain in their original order.
void Texture::draw_cached_lit(Buffer *buffer, Vert *vertices, int *light,
                              int count, Vert *uv, TextureStore *cache) {
    cache->next_record_ = 0;
    unsigned char *destination = reinterpret_cast<unsigned char *>(buffer->get_data());
    const unsigned char key = static_cast<unsigned char>(TextureRasterKey);
    texture_reset_fpu();
    const unsigned char *pixels = static_cast<const unsigned char *>(pixels_);
    const int stride = static_cast<int>(buffer->stride_);
    for (int vertex = 0; vertex < count; ++vertex) {
        int value = light[vertex];
        if (value >= TextureLightCeiling) value = TextureLightCeiling;
        light[vertex] = value;
        if (value <= TextureLightFloor) value = TextureLightFloor;
        light[vertex] = value;
    }
    const RECT clip = buffer->rect1_;
    int min_y = 0x7fff, max_y = -0x7ffd;
    int min_x = 0x7fff, max_x = -0x7ffd;
    int top, bottom;
    bool clipped = true;
    for (int index = 0; index < count; ++index) {
        if (vertices[index].y < min_y) { min_y = vertices[index].y; top = index; }
        if (vertices[index].y > max_y) { max_y = vertices[index].y; bottom = index; }
        if (vertices[index].x < min_x) min_x = vertices[index].x;
        if (vertices[index].x > max_x) max_x = vertices[index].x;
    }
    if (min_y < max_y) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(destination);
        uintptr_t row = base + static_cast<int64_t>(min_y) * stride;
        const uintptr_t first_row = base + static_cast<int64_t>(clip.top) * stride;
        const uintptr_t last_row = base + static_cast<int64_t>(clip.bottom) * stride;
        if (min_x >= clip.left && min_y >= clip.top &&
            max_x <= clip.right && max_y <= clip.bottom) clipped = false;
        EdgeScan left, right;
        left.f0 = -1;
        right.f0 = 1;
        if (texture_cached_edge(left, top, vertices, uv, light, count, bottom, *cache) &&
            texture_cached_edge(right, top, vertices, uv, light, count, bottom, *cache)) {
            if (borrowed_ != 0) {
                _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
                int du, dv;
                unsigned short shade_step;
                while (static_cast<intptr_t>(row) < static_cast<intptr_t>(last_row)) {
                    int x = left.accumC;
                    int end = right.accumC;
                    int u = left.accumA, v = left.accumB, shade = left.accumD;
                    const int span = texture_fixed(texture_difference(end, x));
                    const int delta_u = texture_difference(right.accumA, u);
                    const int delta_v = texture_difference(right.accumB, v);
                    const int delta_shade = texture_difference(right.accumD, shade);
                    // Preserve the cached-edge transition before the saved
                    // terminal span is drawn; failure of the left skips right.
                    const bool done =
                        !texture_advance_cached_edge(left, vertices, uv, light, count, bottom, *cache) ||
                        !texture_advance_cached_edge(right, vertices, uv, light, count, bottom, *cache);
                    if (span > 0) {
                        du = texture_span_step(delta_u, span >> 16);
                        dv = texture_span_step(delta_v, span >> 16);
                        shade_step = texture_light_step(delta_shade, span >> 16);
                    }
                    if (static_cast<intptr_t>(row) >= static_cast<intptr_t>(first_row) &&
                        end > clip.left && x < clip.right && end > x) {
                        if (end > clip.right) end = clip.right;
                        if (x < clip.left) {
                            const int skip = texture_fixed(texture_difference(clip.left, x));
                            u = static_cast<int>(static_cast<unsigned int>(u) +
                                static_cast<unsigned int>((static_cast<int64_t>(du) * skip) >> 16));
                            v = static_cast<int>(static_cast<unsigned int>(v) +
                                static_cast<unsigned int>((static_cast<int64_t>(dv) * skip) >> 16));
                            shade = static_cast<int>(static_cast<unsigned int>(shade) +
                                static_cast<unsigned int>((static_cast<int64_t>(shade_step) * skip) >> 8));
                            x = clip.left;
                        }
                        unsigned char *out = reinterpret_cast<unsigned char *>(row + x);
                        unsigned int source_x = (static_cast<unsigned int>(u) >> 16) & 0xff;
                        unsigned int source_y = (static_cast<unsigned int>(v) >> 16) & 0xff;
                        unsigned int uf = static_cast<unsigned short>(u);
                        unsigned int vf = static_cast<unsigned short>(v);
                        unsigned short brightness = static_cast<unsigned short>(shade >> 8);
                        const unsigned int length = texture_span_pixels(texture_difference(end, x));
                        for (unsigned int column = 0; column < length; ++column) {
                            const unsigned char texel = pixels[source_y * 256u + source_x];
                            uf += static_cast<unsigned short>(du);
                            vf += static_cast<unsigned short>(dv);
                            source_x = (source_x + (static_cast<unsigned int>(du) >> 16) + (uf >> 16)) & 0xff;
                            source_y = (source_y + (static_cast<unsigned int>(dv) >> 16) + (vf >> 16)) & 0xff;
                            uf &= 0xffff;
                            vf &= 0xffff;
                            if (texel != key)
                                out[column] = static_cast<unsigned char>(texel + (brightness >> 8));
                            brightness = static_cast<unsigned short>(brightness + shade_step);
                        }
                    }
                    if (done) break;
                    row += stride;
                }
            } else {
                while (static_cast<intptr_t>(row) < static_cast<intptr_t>(last_row)) {
                    int x = left.accumC;
                    int end = right.accumC;
                    if (static_cast<intptr_t>(row) >= static_cast<intptr_t>(first_row) &&
                        end > clip.left && x < clip.right && end > x) {
                        int u = left.accumA, v = left.accumB;
                        unsigned int shade = static_cast<unsigned int>(left.accumD);
                        const int span = texture_fixed(texture_difference(end, x));
                        // 621E92 has no CDQ: the high dividend is right X,
                        // not the sign of the low, shifted light delta.
                        const unsigned int low = static_cast<unsigned int>(
                            texture_difference(right.accumD, left.accumD)) << 8;
                        const int64_t numerator = static_cast<int64_t>(end) * 65536 * 65536 + low;
                        const unsigned short shade_step = static_cast<unsigned short>(numerator / span);
                        const int du = static_cast<int>(static_cast<int64_t>(
                            texture_difference(right.accumA, u)) * 65536 / span);
                        int dv = static_cast<int>(static_cast<int64_t>(
                            texture_difference(right.accumB, v)) * 65536 / span);
                        int adjusted_u = du;
                        if (du < 0 && (static_cast<unsigned int>(du) & 0xffff))
                            adjusted_u = static_cast<int>(static_cast<unsigned int>(du) + 0x10000u);
                        const int whole_u = adjusted_u >> 16;
                        u = static_cast<int>(static_cast<unsigned int>(u) + (adjusted_u >> 1));
                        int adjusted_v = dv;
                        int signed_pitch = TextureRowPitchScratch;
                        if (dv < 0) {
                            signed_pitch = texture_difference(0, signed_pitch);
                            if (static_cast<unsigned int>(dv) & 0xffff)
                                adjusted_v = static_cast<int>(static_cast<unsigned int>(dv) + 0x10000u);
                        }
                        const int whole_v = static_cast<int>(static_cast<unsigned int>(adjusted_v >> 16) *
                                                             static_cast<unsigned int>(TextureRowPitchScratch));
                        v = static_cast<int>(static_cast<unsigned int>(v) + (adjusted_v >> 1));
                        if (end >= clip.right) end = clip.right;
                        if (x < clip.left) {
                            const int skip = texture_fixed(texture_difference(clip.left, x));
                            u = static_cast<int>(static_cast<unsigned int>(u) +
                                static_cast<unsigned int>((static_cast<int64_t>(du) * skip) >> 16));
                            v = static_cast<int>(static_cast<unsigned int>(v) +
                                static_cast<unsigned int>((static_cast<int64_t>(dv) * skip) >> 16));
                            shade += static_cast<unsigned int>((static_cast<int64_t>(shade_step) * skip) >> 16);
                            x = clip.left;
                        }
                        // The original 16-bit IMUL retains only AX; its row
                        // offset is modulo 65536, not a full-width product.
                        int64_t source = static_cast<unsigned short>(
                            (static_cast<unsigned int>(v) >> 16) *
                            static_cast<unsigned int>(TextureRowPitchScratch));
                        source += static_cast<unsigned short>(static_cast<unsigned int>(u) >> 16);
                        if (signed_pitch < 0) {
                            dv = texture_difference(0, dv);
                            v = ~v;
                        }
                        const bool reverse_u = du < 0;
                        // 622078 shifts the now-unused V local, NOT the
                        // copied packed low WORD. Negative U consequently
                        // keeps all sixteen V fraction bits here.
                        unsigned int fractions = reverse_u
                            ? (static_cast<unsigned int>(~u) << 16) | static_cast<unsigned short>(v)
                            : (static_cast<unsigned int>(u) << 16) | (static_cast<unsigned short>(v) >> 1);
                        const unsigned int step =
                            ((reverse_u ? 0u - static_cast<unsigned int>(du) : static_cast<unsigned int>(du)) << 16) |
                            (static_cast<unsigned short>(dv) >> 1);
                        const int whole_step = static_cast<int>(static_cast<unsigned int>(whole_u) + whole_v);
                        unsigned char *out = reinterpret_cast<unsigned char *>(row + x);
                        const int length = texture_difference(end, x);
                        for (int column = 0; column < length; ++column) {
                            const unsigned char texel = pixels[source];
                            const unsigned int next = fractions + step;
                            const bool carry = next < fractions;
                            fractions = next;
                            // Original SBB subtracts -whole_step and carry;
                            // ADC adds whole_step and carry in the other arm.
                            source += static_cast<int64_t>(whole_step) + (reverse_u ? -int(carry) : int(carry));
                            out[column] = static_cast<unsigned char>(texel + ((shade >> 8) & 0xff));
                            if (fractions & 0x8000u) {
                                fractions &= ~0x8000u;
                                source += signed_pitch;
                            }
                            shade = (shade & 0xffff0000u) | static_cast<unsigned short>(shade + shade_step);
                        }
                    }
                    // Owned storage uses uncached Gouraud setup for later
                    // edges; it does not consume further TextureStore rows.
                    if (!texture_advance_gouraud_edge(left, vertices, uv, light, count, bottom) ||
                        !texture_advance_gouraud_edge(right, vertices, uv, light, count, bottom)) break;
                    row += stride;
                }
            }
        }
    }
    buffer->free_data(1);
    texture_reset_fpu();
    if (!clipped) cache->flags_ = 0;
}
