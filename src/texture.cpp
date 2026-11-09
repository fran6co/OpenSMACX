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

// The three plain textured scan converters advance only U, V, X and the
// fractional X accumulator. Their pinned update blocks do not touch the
// Gouraud-only D channel, which setup_edge does not initialize.
static void texture_uv_edge_step(EdgeScan &edge) {
    edge.accumA = static_cast<int>(
        static_cast<uint32_t>(edge.accumA) + static_cast<uint32_t>(edge.deltaA));
    edge.accumB = static_cast<int>(
        static_cast<uint32_t>(edge.accumB) + static_cast<uint32_t>(edge.deltaB));
    edge.accumC = static_cast<int>(
        static_cast<uint32_t>(edge.accumC) + static_cast<uint32_t>(edge.deltaC));
    edge.fracAccum = static_cast<int>(
        static_cast<uint32_t>(edge.fracAccum) + static_cast<uint32_t>(edge.fracDelta));
    if (edge.fracAccum > 0) {
        edge.accumC = static_cast<int>(
            static_cast<uint32_t>(edge.accumC) + static_cast<uint32_t>(edge.extraDeltaC));
        edge.fracAccum = static_cast<int>(
            static_cast<uint32_t>(edge.fracAccum) - static_cast<uint32_t>(edge.fracSub));
    }
}

// Pinned 61C304-341/366-3AC keyed rows use DWORD ADDs in this field order,
// including the conditional C correction. Retain their 32-bit wrap locally
// instead of using rasterizers' signed-add helper.
static void texture_draw_gouraud_edge_step(EdgeScan &edge) {
    edge.accumA = static_cast<int>(
        static_cast<unsigned int>(edge.accumA) +
        static_cast<unsigned int>(edge.deltaA));
    edge.accumB = static_cast<int>(
        static_cast<unsigned int>(edge.accumB) +
        static_cast<unsigned int>(edge.deltaB));
    edge.accumC = static_cast<int>(
        static_cast<unsigned int>(edge.accumC) +
        static_cast<unsigned int>(edge.deltaC));
    edge.fracAccum = static_cast<int>(
        static_cast<unsigned int>(edge.fracAccum) +
        static_cast<unsigned int>(edge.fracDelta));
    if (edge.fracAccum > 0) {
        edge.accumC = static_cast<int>(
            static_cast<unsigned int>(edge.accumC) +
            static_cast<unsigned int>(edge.extraDeltaC));
        edge.fracAccum = static_cast<int>(
            static_cast<unsigned int>(edge.fracAccum) -
            static_cast<unsigned int>(edge.fracSub));
    }
    edge.accumD = static_cast<int>(
        static_cast<unsigned int>(edge.accumD) +
        static_cast<unsigned int>(edge.deltaD));
}

// The lit spans' row blocks (0x0061AEBD, 0x0061AF1F) step D before the
// fractional X accumulator.
static void texture_lit_edge_step(EdgeScan &edge) {
    edge.accumA += edge.deltaA;
    edge.accumB += edge.deltaB;
    edge.accumC += edge.deltaC;
    edge.accumD += edge.deltaD;
    edge.fracAccum += edge.fracDelta;
    if (edge.fracAccum > 0) {
        edge.accumC += edge.extraDeltaC;
        edge.fracAccum -= edge.fracSub;
    }
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
    const volatile unsigned char *source = pixels;
    unsigned char source_texel = source[source_offset];

    for (int pixel = 0; pixel < pixel_count; ++pixel) {
        const unsigned int shade = (light_accumulator >> 8) & 0xffu;
        if (!keyed || source_texel != key) {
            destination[0] = static_cast<unsigned char>(
                static_cast<unsigned int>(source_texel) + shade);
        }
        light_accumulator = static_cast<unsigned short>(
            light_accumulator + light_step);
        source_offset = texture_draw_next_source_index(
            source_offset, v_fraction_and_du_fraction, u_fraction,
            dv_fraction, du_fraction, source_step, width);
        ++destination;
        source_texel = source[source_offset];
    }
}

static void texture_draw_lit_span(unsigned char *destination,
                                  const unsigned char *pixels, int width,
                                  int u, int v, int du, int dv,
                                  int light, unsigned short light_step,
                                  int pixel_count) {
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
    const volatile unsigned char *source = pixels;
    unsigned char source_texel = source[source_offset];

    // 623921 outputs the initial texel, then 62393E rereads the current
    // source before 623941/944 advances the address. Later outputs therefore
    // use the preceding iteration's source byte (S0, S0, S1, ...).
    for (int pixel = 0; pixel < pixel_count; ++pixel) {
        const unsigned int shade = (light_accumulator >> 8) & 0xffu;
        destination[0] = static_cast<unsigned char>(
            static_cast<unsigned int>(source_texel) + shade);
        light_accumulator = static_cast<unsigned short>(
            light_accumulator + light_step);
        source_texel = source[source_offset];
        source_offset = texture_draw_next_source_index(
            source_offset, v_fraction_and_du_fraction, u_fraction,
            dv_fraction, du_fraction, source_step, width);
        ++destination;
    }
}

// The opaque 61B1F0 loop preloads its first texel, then loads the next source
// byte after every destination store, including after the final output. Keep
// that trailing read local to this owner: it can fault and can observe a
// source/destination alias after the preceding store.
static void texture_draw_opaque_lit_span(unsigned char *destination,
                                         const unsigned char *pixels, int width,
                                         int u, int v, int du, int dv,
                                         int light, unsigned short light_step,
                                         int pixel_count) {
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
    const volatile unsigned char *source = pixels;
    unsigned char source_texel = source[source_offset];

    for (int pixel = 0; pixel < pixel_count; ++pixel) {
        const unsigned int shade = (light_accumulator >> 8) & 0xffu;
        destination[0] = static_cast<unsigned char>(
            static_cast<unsigned int>(source_texel) + shade);
        light_accumulator = static_cast<unsigned short>(
            light_accumulator + light_step);
        source_offset = texture_draw_next_source_index(
            source_offset, v_fraction_and_du_fraction, u_fraction,
            dv_fraction, du_fraction, source_step, width);
        ++destination;
        source_texel = source[source_offset];
    }
}

static void texture_draw_keyed_lit_span(unsigned char *destination,
                                        const unsigned char *pixels, int width,
                                        int u, int v, int du, int dv,
                                        int light, unsigned short light_step,
                                        int pixel_count, unsigned char key) {
    texture_draw_lit_span_mode(destination, pixels, width, u, v, du, dv,
                               light, light_step, pixel_count, true, key);
}

// 62333F..623367 uses UV stepping and a keyed constant fill on even rows.
// Its next-source preload occurs after every output, including the last.
static void texture_draw_keyed_fill_span(unsigned char *destination,
                                         const unsigned char *pixels, int width,
                                         int u, int v, int du, int dv,
                                         int pixel_count, unsigned char key,
                                         unsigned char fill) {
    unsigned int source_offset = texture_draw_source_index(width, u, v);
    unsigned short u_fraction = static_cast<unsigned short>(u);
    unsigned int v_fraction_and_du_fraction =
        (static_cast<unsigned int>(static_cast<unsigned short>(v)) << 16) |
        static_cast<unsigned short>(du);
    const unsigned int dv_fraction =
        static_cast<unsigned int>(static_cast<unsigned short>(dv)) << 16;
    const unsigned int source_step = texture_draw_source_step(du, dv, width);
    const unsigned short du_fraction = static_cast<unsigned short>(du);
    const volatile unsigned char *source = pixels;
    unsigned char source_texel = source[source_offset];
    for (int pixel = 0; pixel < pixel_count; ++pixel) {
        if (source_texel != key) destination[0] = fill;
        source_offset = texture_draw_next_source_index(
            source_offset, v_fraction_and_du_fraction, u_fraction,
            dv_fraction, du_fraction, source_step, width);
        ++destination;
        source_texel = source[source_offset];
    }
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

namespace {
inline void release_texture_pixels(unsigned char *pixels) {
    if (pixels != 0) {
        free(pixels);
    }
}
}

/*
Purpose: Destroy the texture. Unlike close, which resets the descriptor fields
         and borrowed flag, the destructor only settles an owned pixel pointer:
         free it and clear it; borrowed pixels remain unchanged. Descriptor
         fields and the borrowed flag keep whatever they held.
// ORIGINAL: 0x00619660 ??1Texture@@QAE@XZ 0x00619660-0x00619685 BYTE_EXACT
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
        release_texture_pixels(pixels_);
        pixels_ = nullptr;
    }
}

/*
Purpose: Release the texture. The pixels are freed only when they are ours -
         that is, when there are pixels at all and the borrowed flag is clear;
         a borrowed texture keeps its block and is merely forgotten. Either way
         the same four fields the constructor set are reset, except that a
         borrowed texture leaves its pixel pointer alone.
// ORIGINAL: 0x00619690 ?close@Texture@@QAEXXZ 0x00619690-0x006196CA BYTE_EXACT
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
        release_texture_pixels(pixels_);
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
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
int Texture::extract(Buffer *buffer, int x, int y, int width, int height,
                     TexHeap *heap) {
    if (buffer == 0) {
        return 0x10;
    }

    close();

    if (x + width <= static_cast<int>(buffer->dib_.bmiHeader.biWidth) &&
        y + height <= -static_cast<int>(buffer->dib_.bmiHeader.biHeight)) {
        iWidth_ = width;
        iHeight_ = height;

        unsigned char *srcData = buffer->get_data(x, y);
        if (srcData == 0) {
            return 7;
        }

        int stride = buffer->stride_;
        pixels_ = 0;
        if (heap != 0) {
            pixels_ = static_cast<unsigned char *>(heap->get_tex_mem(iHeight_ << 8));
            if (pixels_ != 0) {
                borrowed_ = 1;
            }
        }
        if (pixels_ == 0) {
            pixels_ = static_cast<unsigned char *>(mem_get(iHeight_ * iWidth_));
            if (pixels_ == 0) {
                iWidth_ = 0;
                iHeight_ = 0;
                borrowed_ = 0;
                buffer->free_data(1);
                return 4;
            }
            borrowed_ = 0;
        }

        unsigned char *dest = pixels_;
        const unsigned char *src = srcData;
        while (height--) {
            memcpy(dest, src, iWidth_);
            if (borrowed_ != 0) {
                dest += 0x100;
            } else {
                dest += iWidth_;
            }
            src += stride;
        }

        buffer->free_data(1);

        int w = static_cast<int>(iWidth_);
        int h = static_cast<int>(iHeight_);
        int widthMinus1 = w - 1;
        int heightMinus1 = h - 1;
        int widthHalfMinus1 = w / 2 - 1;
        int heightHalfMinus1 = h / 2 - 1;
        corners_[0].x = 0;
        corners_[0].y = 0;
        corners_[1].x = widthMinus1;
        corners_[2].x = widthMinus1;
        cornersAndMids_[1].x = widthHalfMinus1;
        cornersAndMids_[2].x = widthMinus1;
        cornersAndMids_[3].x = widthMinus1;
        cornersAndMids_[4].x = widthMinus1;
        corners_[2].y = heightMinus1;
        corners_[3].y = heightMinus1;
        cornersAndMids_[4].y = heightMinus1;
        cornersAndMids_[5].y = heightMinus1;
        cornersAndMids_[6].y = heightMinus1;
        corners_[1].y = 0;
        corners_[3].x = 0;
        cornersAndMids_[0].x = 0;
        cornersAndMids_[0].y = 0;
        cornersAndMids_[1].y = 0;
        cornersAndMids_[2].y = 0;
        cornersAndMids_[3].y = heightHalfMinus1;
        cornersAndMids_[5].x = widthHalfMinus1;
        cornersAndMids_[6].x = 0;
        cornersAndMids_[7].x = 0;
        cornersAndMids_[7].y = heightHalfMinus1;
        return 0;
    }

    return 3;
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif

// Rectangle form of extract; ECX remains the same Texture receiver at the
// call to the six-argument overload. Both original exits pop three stack slots,
// but the third incoming slot is never read. Its original type is unresolved.

// ORIGINAL: 0x006196D0 ?extract@Texture@@QAEHPAUBuffer@@PAURECT@@@Z 0x006196D0-0x00619706 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x006196D0-0x00619706 is 54 bytes. ECX remains unchanged for its direct call to Texture::extract at 0x00619710. The first two stack slots supply Buffer and RECT; both exits use ret 0xC, but the third incoming slot is never read. A null rectangle returns 0x10; otherwise the call supplies left, top, right-left, bottom-top and a literal null heap.", "The historical seed restored by 76ffacd8 preserves the canonical Texture name and original byte obligation. Its separate TextureRectExtract class is verification scaffolding, not evidence of another object type. Mapped-span caller search is incomplete and establishes no type for the unused slot."], "reason": "Recover an ordinary Texture rectangle overload on the original receiver, without a fake holder class or reinterpret_cast. Keep the canonical catalogue name even though it omits the third stack slot; the emitted-symbol fact describes the actual three-argument method. int is only a provisional representation of the unused 32-bit slot, not a recovered original type."}
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
    flags_ = 3;
    next_record_ = 0;
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
static Vert *TextureRasterEdgeVerticesA;
// RECOVERY_OBJECT: v1 0x009BB494 _TextureRasterScratch494
static unsigned char TextureRasterScratch494;
// RECOVERY_OBJECT: v1 0x009BB498 _TextureRasterScratch498
static int TextureRasterScratch498;
// RECOVERY_OBJECT: v1 0x009BB49C _TextureRasterBufferStride
static int TextureRasterBufferStride;
// RECOVERY_OBJECT: v1 0x009BB4A4 _TextureRasterScratch4A4
static int TextureRasterScratch4A4;
// The pinned renderers reload this cell as the source base for byte loads.
// RECOVERY_OBJECT: v1 0x009BB504 _TextureRasterScratch504
static unsigned char *TextureRasterScratch504;
// The color-key spans park the real ESP here while ESP counts pixels.
// RECOVERY_OBJECT: v1 0x009BB4AC _TextureRasterSavedStack
static int TextureRasterSavedStack;
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
static void *TextureRasterPixelBase;
// RECOVERY_OBJECT: v1 0x009BB4F0 _TextureRasterSpanLeft
static int TextureRasterSpanLeft; // Legacy spelling; original RIGHT x.
// RECOVERY_OBJECT: v1 0x009BB4F4 _TextureRasterEdgeVerticesB
static Vert *TextureRasterEdgeVerticesB;
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
static TextureRasterDwordHighWordView TextureRasterLightAccumB;
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
static void *TextureRasterScratch544;
// 0x9BB548..54C remains the existing TextureRasterKey object.
// Callers publish a table pointer here. The pinned 0x61EB05/0x61EB39 path
// writes the remap pointer, 0x61F03E/0x61F044 consumes byte entries, and
// 0x6204F9/0x6204FC/0x620502 reads four-byte entries from the same cell. The
// pointer is generic because these consumers use different table element
// widths; this does not assert a capacity for every producer.
// RECOVERY_OBJECT: v1 0x009BB550 _TextureRasterScratch550
static const void *TextureRasterScratch550;
// RECOVERY_OBJECT: v1 0x009BB554 _TextureRasterScratch554
static void *TextureRasterScratch554;
// RECOVERY_OBJECT: v1 0x009BB558 _TextureRasterShadeCursor
// Byte-address cursor in the second texture plane. Original dword adds wrap
// on the 32-bit target; address-domain storage avoids aliasing an int cell
// through unsigned char** and does not truncate host control pointers.
static uintptr_t TextureRasterShadeCursor;
// RECOVERY_OBJECT: v1 0x009BB55C _TextureRasterScratch55C
static int TextureRasterScratch55C;
// RECOVERY_OBJECT: v1 0x009BB560 _TextureRasterScratch560
static unsigned char TextureRasterScratch560;
// RECOVERY_OBJECT: v1 0x009BB564 _TextureRasterPalette
static unsigned char *TextureRasterPalette;

// These mapped cells are pointer-sized on the original 32-bit target. Keep
// address arithmetic in an integer domain so raster offsets do not perform
// undefined subtraction between unrelated allocations.
static uintptr_t texture_pointer_address(const void *pointer) {
    return reinterpret_cast<uintptr_t>(pointer);
}

static uintptr_t texture_pointer_offset(const void *base, uint32_t offset) {
    // Preserve the target's 32-bit ADD wrap while allowing host fixtures to
    // use real allocations above 4 GiB. A wrapped high-bit offset is the same
    // signed displacement the x86 pointer ADD applies.
    return texture_pointer_address(base) +
        static_cast<intptr_t>(static_cast<int32_t>(offset));
}

static void *texture_pointer_from_address(uintptr_t address) {
    return reinterpret_cast<void *>(address);
}

// Direct raster-global accesses remove private alias cells; preserve the key's
// former signed-int view as representation bits here.
static int texture_raster_key_signed_bits() {
    int value;
    memcpy(&value, &TextureRasterKey, sizeof(value));
    return value;
}


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

// ORIGINAL: 0x0061EAD0 ?draw_coast@Texture@@QAEXPAUBuffer@@PAUVert@@PAHPAUTexture@@PAEH1@Z 0x0061EAD0-0x0061F0C6 FILE BYTE_EXACT
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

void Texture::draw_coast(Buffer *buffer, Vert *vertices, int *zvals, Texture *coast,
                         unsigned char *remap, int count, Vert *uv) {
    if (vertices != 0 && buffer != 0 && pixels_ != 0 && zvals != 0 && remap != 0 &&
        coast != 0) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterZValues = zvals;
    TextureRasterScratch550 = remap;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    TextureRasterVertexCount = count;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    TextureRasterScratch554 = coast->pixels_;
    if (TextureRasterScratch554 != 0) {
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int max_y = -0x7ffd;
        int min_y = 0x7fff;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (int i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: ESP-counter span that
                                // remaps the coast texel and adds the light.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_eef1
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_eef1:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_ef56
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_ef56:
                                sub ebx, edi
                                mov ecx, ebx
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                sub esi, dword ptr [TextureRasterScratch544]
                                mov edi, dword ptr [TextureRasterScratch554]
                                add edi, esi
                                mov dword ptr [TextureRasterShadeCursor], edi
                                xor esi, esi
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                mov al, byte ptr [ebx + esi]
                                mov cx, word ptr [TextureRasterScratch498]
                            K_f028:
                                mov dword ptr [TextureRasterScratch4A4], edx
                                mov edx, dword ptr [TextureRasterShadeCursor]
                                add edx, esi
                                mov dl, byte ptr [edx]
                                and edx, 0xff
                                add edx, dword ptr [TextureRasterScratch550]
                                mov dl, byte ptr [edx]
                                cmp dl, 0x0
                                je K_f058
                                cmp dl, 0x1
                                je K_f054
                                mov al, dl
                                sub al, ch
                            K_f054:
                                add al, ch
                                mov byte ptr [edi], al
                            K_f058:
                                mov edx, dword ptr [TextureRasterScratch4A4]
                                add cx, word ptr [TextureRasterLightPackA]
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_f028
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
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


// ORIGINAL: 0x0061FC90 ?setup_edge_gouraud@Texture@@QAAHPAUEdgeScan@@H@Z 0x0061FC90-0x0061FDEA FILE BYTE_EXACT
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


int __cdecl Texture::setup_edge_gouraud(EdgeScan *edge, int vertex_index) {
    int endpoint;
    for (;;) {
        if (vertex_index == TextureRasterMaxYIndex) return 0;
        endpoint = edge->f0 + vertex_index;
        const int last = TextureRasterVertexCount - 1;
        if (endpoint > last) endpoint = 0;
        else if (endpoint < 0) endpoint = last;

        edge->counter = TextureRasterEdgeVerticesB[endpoint].y -
                        TextureRasterEdgeVerticesB[vertex_index].y;
        if (edge->counter != 0) break;
        vertex_index = endpoint;
    }
    if (edge->counter <= 0) return 0;

    const int scale = edge->counter << 16;
    edge->id = endpoint;
    edge->accumA = (TextureRasterEdgeVerticesA[vertex_index].x << 16) + 0x8000;
    edge->accumB = (TextureRasterEdgeVerticesA[vertex_index].y << 16) + 0x8000;
    edge->deltaA = fixed_div(
        (TextureRasterEdgeVerticesA[endpoint].x << 16) - edge->accumA, scale);
    edge->deltaB = fixed_div(
        (TextureRasterEdgeVerticesA[endpoint].y << 16) - edge->accumB, scale);

    edge->accumC = TextureRasterEdgeVerticesB[vertex_index].x;
    int dx = TextureRasterEdgeVerticesB[endpoint].x -
             TextureRasterEdgeVerticesB[vertex_index].x;
    edge->deltaC = dx / edge->counter;
    if (dx < 0) {
        dx = -dx;
        edge->extraDeltaC = -1;
        edge->fracAccum = 1 - edge->counter;
    } else {
        edge->extraDeltaC = 1;
        edge->fracAccum = 0;
    }
    edge->fracDelta = dx % edge->counter;
    edge->fracSub = edge->counter;

    const int accum_d = TextureRasterZValues[vertex_index] << 16;
    edge->accumD = accum_d;
    edge->deltaD = fixed_div((TextureRasterZValues[endpoint] << 16) - accum_d, scale);
    return 1;
}

// ORIGINAL: 0x00624D80 ?setup_edge@Texture@@QAAHPAUEdgeScan@@H@Z 0x00624D80-0x00624EA9 BYTE_EXACT
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


int __cdecl Texture::setup_edge(EdgeScan *edge, int vertex_index) {
    int endpoint;
    for (;;) {
        if (vertex_index == TextureRasterMaxYIndex) return 0;
        endpoint = edge->f0 + vertex_index;
        const int last = TextureRasterVertexCount - 1;
        if (endpoint > last) endpoint = 0;
        else if (endpoint < 0) endpoint = last;

        edge->counter = TextureRasterEdgeVerticesB[endpoint].y -
                        TextureRasterEdgeVerticesB[vertex_index].y;
        if (edge->counter != 0) break;
        vertex_index = endpoint;
    }
    if (edge->counter <= 0) return 0;

    const int scale = edge->counter << 16;
    edge->id = endpoint;
    edge->accumA = TextureRasterEdgeVerticesA[vertex_index].x << 16;
    edge->accumB = TextureRasterEdgeVerticesA[vertex_index].y << 16;
    edge->deltaA = fixed_div(
        (TextureRasterEdgeVerticesA[endpoint].x << 16) - edge->accumA, scale);
    edge->deltaB = fixed_div(
        (TextureRasterEdgeVerticesA[endpoint].y << 16) - edge->accumB, scale);

    edge->accumC = TextureRasterEdgeVerticesB[vertex_index].x;
    int dx = TextureRasterEdgeVerticesB[endpoint].x -
             TextureRasterEdgeVerticesB[vertex_index].x;
    int delta_c;
    if (dx < 0) {
        dx = -dx;
        edge->fracAccum = 1 - edge->counter;
        edge->extraDeltaC = -1;
        delta_c = -(dx / edge->counter);
    } else {
        edge->extraDeltaC = 1;
        delta_c = dx / edge->counter;
        edge->fracAccum = 0;
    }
    edge->deltaC = delta_c;
    edge->fracDelta = dx % edge->counter;
    edge->fracSub = edge->counter;
    return 1;
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
// the span inclusive, even at clip.right. U and V wrap independently in the
// low pointer bytes. TexHeap::get_tex_mem supplies a 64-KiB-aligned payload;
// Texture::extract writes its borrowed rows at a 256-byte pitch.
//
// 0x6198FE..0x61997A publishes the destination, texture/UV/screen-vertex
// pointers, count, dimensions, stride and clip bounds into the existing
// TextureRaster cells. After the vertex scan, the bottom/top clip words are
// rebased to low-32-bit destination row addresses before setup_edge consumes
// the vertex/count/UV/max-Y cells. Each row republishes edge accumulators
// before rollover setup; positive-span slopes publish after those edge
// advances and before row clipping. Left clipping also writes its adjusted X
// back to the shared span cell. Keep these effects even where locals could
// calculate the same pixels. 0x9BB508 is a target-width scratch word: this
// body stores low 32-bit destination-row address bits there, but the shared
// cell has other scalar uses and its original type is not established. Host
// pixel stores retain a full pointer through texture_pointer_offset rather
// than converting those truncated bits back to a host pointer.
//
// texture_span_step remains the current integer model of original x87
// arithmetic; exact FISTP results/status under every caller control-word
// state, invalid geometry, aliasing and runtime effects remain unproved.
void Texture::draw_trans(Buffer *buffer, Vert *vertices, int count, Vert *uv) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    TextureRasterVertexCount = count;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int min_x = 0x7fff;
        int max_x = -0x7ffd;
        int top;
        EdgeScan left;
        EdgeScan right;
        Vert *v = vertices;
        for (int i = 0; i < count; ++i, ++v) {
            if (v->y < min_y) {
                min_y = v->y;
                top = i;
            }
            if (v->y > max_y) {
                // volatile: stand-in for the image's reload of v->y in the max branch; no plain spelling found
                int y = *(volatile int *)&v->y;
                TextureRasterMaxYIndex = i;
                max_y = y;
            }
            if (v->x < min_x) {
                min_x = v->x;
            }
            if (v->x > max_x) {
                // volatile: stand-in for the image's reload of v->x in the max branch; no plain spelling found
                max_x = *(volatile int *)&v->x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge(&right, top) && borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int delta;
                    if (span > 0) {
                        delta = right.accumB - left.accumB;
                        __asm fild delta
                        delta = right.accumA - left.accumA;
                        __asm {
                            fild delta
                            fild span
                            fdivr st, st(3)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_uv_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_uv_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmulp st(2), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                delta = right.accumB - left.accumB;
                                __asm fild delta
                                delta = right.accumA - left.accumA;
                                __asm {
                                    fild delta
                                    fild span
                                    fdivr st, st(3)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: clipped span with
                                // adc carry stepping and EBP as the V step.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl S_43
                                mov ebx, dword ptr [TextureRasterClipRight]
                            S_43:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge S_8f
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            S_8f:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                and ecx, 0xffff
                            S_f7:
                                mov al, byte ptr [ebx]
                                add edx, esi
                                mov byte ptr [edi], al
                                lea ecx, [ecx]
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                inc edi
                                dec ecx
                                jns S_f7
                                pop ebp
                                span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
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
// The scanline edge protocol shares the mapped TextureRaster cells, including
// the original address-word clipping and rollover snapshots. 0x0061A55D..0x0061A6C5
// packs U/V fractions and a pixel count into registers, including ESP. It is
// color-key copying, not a jump table.
// The FPU blocks and the span loop are original hand-written assembly. With
// asm present VC6 keeps the dead x-extent scan, as the image does. Remaining
// gap: the image's vertex loop keeps the walked pointer in ECX and re-reads
// the vertex in the max branches.
void Texture::draw_trans(Buffer *buffer, Vert *vertices, int use_eight_vertices) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    if (use_eight_vertices) {
        TextureRasterVertexCount = 8;
        TextureRasterEdgeVerticesA = cornersAndMids_;
    } else {
        TextureRasterVertexCount = 4;
        TextureRasterEdgeVerticesA = corners_;
    }
    TextureRasterLightThreshold = static_cast<int>(TextureRasterKey);
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int min_x = 0x7fff;
        int max_x = -0x7ffd;
        int top;
        EdgeScan left;
        EdgeScan right;
        Vert *v = vertices;
        for (int i = 0; i < TextureRasterVertexCount; ++i, ++v) {
            if (v->y < min_y) {
                min_y = v->y;
                top = i;
            }
            if (v->y > max_y) {
                // volatile: stand-in for the image's reload of v->y in the max branch; no plain spelling found
                int y = *(volatile int *)&v->y;
                TextureRasterMaxYIndex = i;
                max_y = y;
            }
            if (v->x < min_x) {
                min_x = v->x;
            }
            if (v->x > max_x) {
                // volatile: stand-in for the image's reload of v->x in the max branch; no plain spelling found
                max_x = *(volatile int *)&v->x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dv;
                    int du;
                    if (span > 0) {
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(3)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_uv_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_uv_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmulp st(2), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(3)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: color-keyed span with
                                // ESP as the packed pixel counter/U fraction,
                                // sbb/adc carry stepping, and EBP as the carry.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_right
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_right:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_left
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_left:
                                sub ebx, edi
                                mov ecx, ebx
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                xor esi, esi
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                mov al, byte ptr [ebx+esi]
                            K_pixel:
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                cmp al, ah
                                je K_keyed
                                mov byte ptr [edi], al
                            K_keyed:
                                add sp, dx
                                adc esi, dword ptr [TextureRasterAlternateRowNext+ebp*4]
                                inc edi
                                mov al, byte ptr [ebx+esi]
                                sub esp, 0x10000
                                jns K_pixel
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
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
// The body is 0x0061A240's with count and UV base from the caller: the same
// hand-written FPU blocks and ESP-counter span loop, kept as __asm, and the
// same remaining vertex-scan register gap.
void Texture::draw_color_key(Buffer *buffer, Vert *vertices, int count, Vert *uv) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterEdgeVerticesA = uv;
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    TextureRasterVertexCount = count;
    TextureRasterLightThreshold = static_cast<int>(TextureRasterKey);
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int min_x = 0x7fff;
        int max_x = -0x7ffd;
        int top;
        EdgeScan left;
        EdgeScan right;
        Vert *v = vertices;
        for (int i = 0; i < count; ++i, ++v) {
            if (v->y < min_y) {
                min_y = v->y;
                top = i;
            }
            if (v->y > max_y) {
                // volatile: stand-in for the image's reload of v->y in the max branch; no plain spelling found
                int y = *(volatile int *)&v->y;
                TextureRasterMaxYIndex = i;
                max_y = y;
            }
            if (v->x < min_x) {
                min_x = v->x;
            }
            if (v->x > max_x) {
                // volatile: stand-in for the image's reload of v->x in the max branch; no plain spelling found
                max_x = *(volatile int *)&v->x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dv;
                    int du;
                    if (span > 0) {
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(3)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_uv_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_uv_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmulp st(2), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(3)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: color-keyed span with
                                // ESP as the packed pixel counter/U fraction,
                                // sbb/adc carry stepping, and EBP as the carry.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_right
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_right:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_left
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_left:
                                sub ebx, edi
                                mov ecx, ebx
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                xor esi, esi
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                mov al, byte ptr [ebx+esi]
                            K_pixel:
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                cmp al, ah
                                je K_keyed
                                mov byte ptr [edi], al
                            K_keyed:
                                add sp, dx
                                adc esi, dword ptr [TextureRasterAlternateRowNext+ebp*4]
                                inc edi
                                mov al, byte ptr [ebx+esi]
                                sub esp, 0x10000
                                jns K_pixel
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}

// Fixed-slot bindings carried from 0061b1f0.cpp
// Shared signed light clamp: gen_terrain's three writer pairs temporarily
// select -4/+4 and restore these initial -256/+256 limits. Six original
// Texture entries clamp their input arrays against the same storage.
// RECOVERY_OBJECT: v2 0x006972CC 0x006972D0 ?TextureLightFloor@@3HA 0x00621870
int TextureLightFloor = -256;
// RECOVERY_OBJECT: v2 0x006972D0 0x006972D4 ?TextureLightCeiling@@3HA 0x00621870
int TextureLightCeiling = 256;

// ORIGINAL: 0x0061B1F0 ?draw@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@@Z 0x0061B1F0-0x0061B778 FILE BYTE_EXACT
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


void Texture::draw(Buffer *buffer, Vert *vertices, int *zvals, int count, Vert *uv) {
    TextureRasterPixelBase = buffer->get_data();
    TextureRasterLightThreshold = static_cast<int>(TextureRasterKey);
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterVertexCount = count;
    TextureRasterZValues = zvals;
    TextureRasterScratch544 = pixels_;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        for (int i = 0; i < count; ++i) {
            zvals[i] = min(zvals[i], TextureLightCeiling);
            zvals[i] = max(zvals[i], TextureLightFloor);
        }
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int min_x = 0x7fff;
        int max_x = -0x7ffd;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (i = 0; i < TextureRasterVertexCount; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: lit span with ESP as
                                // the packed pixel counter and sbb/adc stepping.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_b5e0
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_b5e0:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_b645
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_b645:
                                sub ebx, edi
                                mov ecx, ebx
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                xor esi, esi
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                mov al, byte ptr [ebx + esi]
                                mov cx, word ptr [TextureRasterScratch498]
                            K_b703:
                                add al, ch
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                mov byte ptr [edi], al
                                add cx, word ptr [TextureRasterLightPackA]
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_b703
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}

// ORIGINAL: 0x0061C090 ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@@Z 0x0061C090-0x0061C622 FILE BYTE_EXACT
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
    TextureRasterPixelBase = buffer->get_data();
    TextureRasterLightThreshold = static_cast<int>(TextureRasterKey);
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterVertexCount = count;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterZValues = zvals;
    TextureRasterScratch544 = pixels_;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        for (int i = 0; i < count; ++i) {
            zvals[i] = min(zvals[i], TextureLightCeiling);
            zvals[i] = max(zvals[i], TextureLightFloor);
        }
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int min_x = 0x7fff;
        int max_x = -0x7ffd;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (i = 0; i < TextureRasterVertexCount; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: color-keyed lit span
                                // with ESP as the packed pixel counter.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_c480
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_c480:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_c4e5
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_c4e5:
                                sub ebx, edi
                                mov ecx, ebx
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                xor esi, esi
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                mov al, byte ptr [ebx + esi]
                                mov cx, word ptr [TextureRasterScratch498]
                            K_c5a9:
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                cmp al, ah
                                je K_c5b9
                                add al, ch
                                mov byte ptr [edi], al
                            K_c5b9:
                                add cx, word ptr [TextureRasterLightPackA]
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_c5a9
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}

// Fixed-slot bindings carried from 0061e540.cpp

struct EdgeScanBuf {
    int marker;
};

// ORIGINAL: 0x0061E540 ?draw_mask_dest@Texture@@QAEXPAUBuffer@@PAUVert@@PAEPAEHPAUVert@@@Z 0x0061E540-0x0061EACE FILE BYTE_EXACT
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
    if (vertices != 0 && buffer != 0 && pixels_ != 0 && remap != 0) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterScratch550 = remap;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    TextureRasterEdgeVerticesA = uv;
    TextureRasterVertexCount = count;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    TextureRasterPalette = palette;
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (int i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: ESP-counter span that
                                // maps each texel through the remap table and
                                // blends it with the destination via the palette.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_e938
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_e938:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_e984
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_e984:
                                sub ebx, edi
                                mov ecx, ebx
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                xor esi, esi
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                mov al, byte ptr [ebx + esi]
                            K_ea2e:
                                mov dword ptr [TextureRasterScratch4A4], edx
                                mov dl, al
                                and edx, 0xff
                                add edx, dword ptr [TextureRasterScratch550]
                                mov dl, byte ptr [edx]
                                cmp dl, 0x0
                                je K_ea67
                                dec dl
                                and edx, 0xff
                                shl edx, 0x8
                                add edx, dword ptr [TextureRasterPalette]
                                mov al, byte ptr [edi]
                                and eax, 0xff
                                add edx, eax
                                mov dl, byte ptr [edx]
                                mov byte ptr [edi], dl
                            K_ea67:
                                mov edx, dword ptr [TextureRasterScratch4A4]
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_ea2e
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}

// Fixed-slot bindings carried from 006233c0.cpp

// ORIGINAL: 0x006233C0 ?draw@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@H@Z 0x006233C0-0x006239AB FILE BYTE_EXACT
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
                   Vert *uv, int fill_color) {
    if (fill_color == -1) {
        draw(buffer, vertices, zvals, count, uv);
        return;
    }
    TextureRasterPixelBase = buffer->get_data();
    TextureRasterLightThreshold = static_cast<int>(TextureRasterKey);
    TextureRasterScratch494 = static_cast<unsigned char>(fill_color);
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterVertexCount = count;
    TextureRasterZValues = zvals;
    TextureRasterScratch544 = pixels_;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        for (int i = 0; i < count; ++i) {
            zvals[i] = min(zvals[i], TextureLightCeiling);
            zvals[i] = max(zvals[i], TextureLightFloor);
        }
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int min_x = 0x7fff;
        int max_x = -0x7ffd;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (i = 0; i < TextureRasterVertexCount; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterDither = min_y & 1;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: ESP-counter span; odd
                                // Dither rows draw lit texels, even rows write
                                // the fill color.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_37eb
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_37eb:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_3850
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_3850:
                                sub ebx, edi
                                mov ecx, ebx
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                test dword ptr [TextureRasterDither], 0x1
                                jne K_387d
                                mov ah, byte ptr [TextureRasterScratch494]
                            K_3872:
                                mov byte ptr [edi], ah
                                inc edi
                                dec ecx
                                jne K_3872
                                jmp span_done
                            K_387d:
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                xor esi, esi
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                mov al, byte ptr [ebx + esi]
                                mov cx, word ptr [TextureRasterScratch498]
                            K_392b:
                                add al, ch
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                mov byte ptr [edi], al
                                add cx, word ptr [TextureRasterLightPackA]
                                mov al, byte ptr [ebx + esi]
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                sub esp, 0x10000
                                jns K_392b
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                            ++TextureRasterDither;
                        }
                    }
                }
            }
        }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}

// Helpers mirroring the CONCAT/CARRY-style byte packing the widest blitter
// (draw_coast at 0x00623F80) does on its register pairs. They live with the
// rest of the family's helpers at the top of this unit.

// ORIGINAL: 0x00623F80 ?draw_coast@Texture@@QAEXPAUBuffer@@PAUVert@@PAHPAUTexture@@PAEHPAUVert@@H@Z 0x00623F80-0x00624626 FILE BYTE_EXACT
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


void Texture::draw_coast(Buffer *buffer, Vert *vertices, int *zvals, Texture *coast,
                         unsigned char *remap, int count, Vert *uv, int fill_color) {
    if (fill_color == -1) {
        draw_coast(buffer, vertices, zvals, coast, remap, count, uv);
        return;
    }
    if (vertices != 0 && buffer != 0 && pixels_ != 0 && zvals != 0 && remap != 0 &&
        coast != 0) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterScratch494 = static_cast<unsigned char>(fill_color);
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterZValues = zvals;
    TextureRasterScratch550 = remap;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    TextureRasterVertexCount = count;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    TextureRasterScratch554 = coast->pixels_;
    if (TextureRasterScratch554 != 0) {
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int max_y = -0x7ffd;
        int min_y = 0x7fff;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (int i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterDither = min_y & 1;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: ESP-counter span; odd
                                // Dither rows remap and light the coast texel,
                                // even rows write the fill color.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_43e8
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_43e8:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_444d
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_444d:
                                sub ebx, edi
                                mov ecx, ebx
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                sub esi, dword ptr [TextureRasterScratch544]
                                mov edi, dword ptr [TextureRasterScratch554]
                                add edi, esi
                                mov dword ptr [TextureRasterShadeCursor], edi
                                xor esi, esi
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                mov al, byte ptr [ebx + esi]
                                test dword ptr [TextureRasterDither], 0x1
                                je K_4588
                                mov cx, word ptr [TextureRasterScratch498]
                            K_452b:
                                mov dword ptr [TextureRasterScratch4A4], edx
                                mov edx, dword ptr [TextureRasterShadeCursor]
                                add edx, esi
                                mov dl, byte ptr [edx]
                                and edx, 0xff
                                add edx, dword ptr [TextureRasterScratch550]
                                mov dl, byte ptr [edx]
                                cmp dl, 0x0
                                je K_455b
                                cmp dl, 0x1
                                je K_4557
                                mov al, dl
                                sub al, ch
                            K_4557:
                                add al, ch
                                mov byte ptr [edi], al
                            K_455b:
                                mov edx, dword ptr [TextureRasterScratch4A4]
                                add cx, word ptr [TextureRasterLightPackA]
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_452b
                                jmp K_45d7
                            K_4588:
                                mov cl, byte ptr [TextureRasterScratch494]
                            K_458e:
                                mov dword ptr [TextureRasterScratch4A4], edx
                                mov edx, dword ptr [TextureRasterShadeCursor]
                                add edx, esi
                                mov dl, byte ptr [edx]
                                and edx, 0xff
                                add edx, dword ptr [TextureRasterScratch550]
                                mov dl, byte ptr [edx]
                                cmp dl, 0x0
                                je K_45b3
                                mov byte ptr [edi], cl
                            K_45b3:
                                mov edx, dword ptr [TextureRasterScratch4A4]
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_458e
                            K_45d7:
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                            ++TextureRasterDither;
                        }
                    }
                }
            }
        }
    }
    }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}

// ORIGINAL: 0x00619D80 Texture::draw_trans2 0x00619D80-0x0061A23F FILE
// RECOVERY_BINDING: v1 {"reason":"Recover the transparent rasterizer's geometry argument as the shared Vert coordinate-pair type.","evidence":["The pinned original publishes the argument into geometry pointer cell 0x9BB4F4, reads signed x/y at offsets0/4 and advances by8. Current typed edge setup consumes that same shared Vert pointer.","The historical decorated name was generated seed metadata, not original debug type evidence. No incoming producer is established; this binds the consumer contract, not original source spelling."]}
// symbol    ?draw_trans2@Texture@@QAEHPAVBuffer@@PAUVert@@H@Z
// TRIED: MISMATCH #4 (push vs mov, prologue) on a 1215-byte function whose own Ghidra decompilation admits it lost the x87 stack (float10/extraout_ST0/in_ST2 markers). Transcribed structure: get_data, min/max vertex scan, two setup_edge calls, an outer per-scanline loop with periodic edge-refresh (mirrors the counter/threshold DDA shape already used at 0x005D9FB0's sibling), and a packed dual-accumulator inner blend loop transcribed instruction-by-instruction from the raw asm (0x61A17C-0x61A1F2) rather than Ghidra's CONCAT/CARRY text. The perspective slope (1/dz, du, dv) is modelled as plain persistent `double`s refreshed only when dz>0 - matching WHEN the original recomputes, not its literal FPU choreography. EdgeScan is opaque (setup_edge is itself unrecovered); the 13-field local struct order is inferred from read/write order, not a confirmed layout.
// working copy - scaffold materialised by --work
// size      1215 bytes
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00624D80

// STRUCTURAL EVIDENCE: Pinned setup_edge 0x624D80 writes EdgeScan offsets
// +0x28/+0x2c/+0x30 as fracAccum/fracDelta/fracSub; draw_trans2 row blocks
// 0x619FE2-0x61A000 and 0x61A039-0x61A074 confirm the matching DDA updates.
// This supersedes the former local-overlay interpretation above.

// 0x0061A240's body with the borrowed_ guard and an unkeyed copy span. The
// image never sets EAX before ret, so the original was likely void; the
// declared int return is kept and returns 0.
int Texture::draw_trans2(Buffer *buffer, Vert *vertices, int use_eight_vertices) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    if (use_eight_vertices) {
        TextureRasterVertexCount = 8;
        TextureRasterEdgeVerticesA = cornersAndMids_;
    } else {
        TextureRasterVertexCount = 4;
        TextureRasterEdgeVerticesA = corners_;
    }
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int min_x = 0x7fff;
        int max_x = -0x7ffd;
        int top;
        EdgeScan left;
        EdgeScan right;
        Vert *v = vertices;
        for (int i = 0; i < TextureRasterVertexCount; ++i, ++v) {
            if (v->y < min_y) {
                min_y = v->y;
                top = i;
            }
            if (v->y > max_y) {
                // volatile: stand-in for the image's reload of v->y in the max branch; no plain spelling found
                int y = *(volatile int *)&v->y;
                TextureRasterMaxYIndex = i;
                max_y = y;
            }
            if (v->x < min_x) {
                min_x = v->x;
            }
            if (v->x > max_x) {
                // volatile: stand-in for the image's reload of v->x in the max branch; no plain spelling found
                max_x = *(volatile int *)&v->x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge(&right, top)) {
                if (borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dv;
                    int du;
                    if (span > 0) {
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(3)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_uv_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_uv_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmulp st(2), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(3)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: unkeyed copy span with adc carry stepping, EBP as the V step and a lea filler.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_a12c
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_a12c:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_a178
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_a178:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                and ecx, 0xffff
                            K_a1e0:
                                mov al, byte ptr [ebx]
                                add edx, esi
                                mov byte ptr [edi], al
                                lea ecx, [ecx]
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                inc edi
                                dec ecx
                                jns K_a1e0
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
                }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
    return 0;
}

// ORIGINAL: 0x00622DB0 Texture::draw_trans_3 0x00622DB0-0x006233C0 FILE BYTE_EXACT
// symbol    ?draw_trans_3@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1H@Z
// TRIED: __stdcall free function (receiver is really `this` via ECX, recast as Texture::draw_trans_3); exact EdgeScan field indices; the textured fill sub-branch borrows the real ESP as a second fixed-point counter (push ebp; mov [0x9bb4ac],esp; mov esp,ecx; ...; add sp,dx; sub esp,0x10000; jns ...; mov esp,[0x9bb4ac]) - no C construct can spell operating on the live stack pointer as data.
// working copy - scaffold materialised by --work
// size      1552 bytes
// prototype 
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061C090 0x0061FC90


void Texture::draw_trans_3(Buffer *buffer, Vert *vertices, int *zvals, int count,
                          Vert *uv, int fill_color) {
    if (fill_color == -1) {
        draw_trans(buffer, vertices, zvals, count, uv);
        return;
    }
    TextureRasterPixelBase = buffer->get_data();
    TextureRasterLightThreshold = static_cast<int>(TextureRasterKey);
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterScratch494 = static_cast<unsigned char>(fill_color);
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterVertexCount = count;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterZValues = zvals;
    TextureRasterScratch544 = pixels_;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        for (int i = 0; i < count; ++i) {
            zvals[i] = min(zvals[i], TextureLightCeiling);
            zvals[i] = max(zvals[i], TextureLightFloor);
        }
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int min_x = 0x7fff;
        int max_x = -0x7ffd;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (i = 0; i < TextureRasterVertexCount; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterDither = min_y & 1;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: color-keyed ESP-counter
                                // span; odd Dither rows draw lit texels, even
                                // rows write the fill color.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_31db
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_31db:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_3240
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_3240:
                                sub ebx, edi
                                mov ecx, ebx
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                xor esi, esi
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                mov al, byte ptr [ebx + esi]
                                test dword ptr [TextureRasterDither], 0x1
                                je K_333f
                                mov cx, word ptr [TextureRasterScratch498]
                            K_3310:
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                cmp al, ah
                                je K_3320
                                add al, ch
                                mov byte ptr [edi], al
                            K_3320:
                                add cx, word ptr [TextureRasterLightPackA]
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_3310
                                jmp K_3369
                            K_333f:
                                mov cl, byte ptr [TextureRasterScratch494]
                            K_3345:
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                cmp al, ah
                                je K_3353
                                mov byte ptr [edi], cl
                            K_3353:
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_3345
                            K_3369:
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                            ++TextureRasterDither;
                        }
                    }
                }
            }
        }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
    return;
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

// ORIGINAL: 0x00625080 sub_625080 0x00625080-0x00625294 FILE BYTE_EXACT
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

// The mapped entry performs this cache setup from the shared polygon context.
// Keep its cache selection and channel writes in the body: the original calls
// fixed_div between observable record/edge stores and rereads shared tables.

int __cdecl sub_625080(EdgeScan *edge, int index, TextureStore *cache) {
    int endpoint;
    for (;;) {
        if (index == TextureRasterMaxYIndex) return 0;
        endpoint = edge->f0 + index;
        const int last = TextureRasterVertexCount - 1;
        if (endpoint > last) endpoint = 0;
        else if (endpoint < 0) endpoint = last;

        edge->counter = TextureRasterEdgeVerticesB[endpoint].y -
                        TextureRasterEdgeVerticesB[index].y;
        if (edge->counter != 0) break;
        index = endpoint;
    }
    if (edge->counter <= 0) return 0;

    EdgeScan *record = &cache->edges_[cache->next_record_];
    if (++cache->next_record_ == 10) cache->next_record_ = 0;

    edge->id = endpoint;
    const int scale = edge->counter << 16;
    edge->accumA = (TextureRasterEdgeVerticesA[index].x << 16) + 0x8000;
    edge->accumB = (TextureRasterEdgeVerticesA[index].y << 16) + 0x8000;

    if (cache->flags_ & 2u) {
        const int delta_a = fixed_div(
            (TextureRasterEdgeVerticesA[endpoint].x << 16) - edge->accumA,
            scale);
        record->deltaA = delta_a;
        edge->deltaA = delta_a;
        const int delta_b = fixed_div(
            (TextureRasterEdgeVerticesA[endpoint].y << 16) - edge->accumB,
            scale);
        record->deltaB = delta_b;
        edge->deltaB = delta_b;
    } else {
        edge->deltaA = record->deltaA;
        edge->deltaB = record->deltaB;
    }

    edge->accumC = TextureRasterEdgeVerticesB[index].x;
    int dx = TextureRasterEdgeVerticesB[endpoint].x -
             TextureRasterEdgeVerticesB[index].x;
    if (cache->flags_ & 2u) {
        const int delta_c = dx / edge->counter;
        record->deltaC = delta_c;
        edge->deltaC = delta_c;
        if (dx < 0) {
            dx = -dx;
            edge->extraDeltaC = -1;
            edge->fracAccum = 1 - edge->counter;
        } else {
            edge->extraDeltaC = 1;
            edge->fracAccum = 0;
        }
        const int frac_delta = dx % edge->counter;
        record->fracDelta = frac_delta;
        edge->fracSub = edge->counter;
        edge->fracDelta = frac_delta;
    } else {
        edge->deltaC = record->deltaC;
        if (dx < 0) {
            edge->extraDeltaC = -1;
            edge->fracAccum = 1 - edge->counter;
        } else {
            edge->extraDeltaC = 1;
            edge->fracAccum = 0;
        }
        edge->fracDelta = record->fracDelta;
        edge->fracSub = edge->counter;
    }

    if (cache->flags_ & 1u) {
        const int accum_d = TextureRasterZValues[index] << 16;
        record->accumD = accum_d;
        edge->accumD = accum_d;
        const int delta_d = fixed_div(
            (TextureRasterZValues[endpoint] << 16) - accum_d, scale);
        record->deltaD = delta_d;
        edge->deltaD = delta_d;
    } else {
        edge->accumD = record->accumD;
        edge->deltaD = record->deltaD;
    }
    return 1;
}

// Fixed-slot bindings carried from 00620100.cpp

// ORIGINAL: 0x00620100 sub_620100 0x00620100-0x00620254 FILE BYTE_EXACT
// symbol    ?sub_620100@@YAHPAUEdgeScan@@H@Z
// TRIED: IDA/Ghidra guessed a nullary/void signature; the disassembly proves 2 explicit int args (int*, int) with no `this`. The 0x9BB490/0x9BB4F4/0x9BB4FC globals hold POINTERS (double indirection: `mov ecx,[addr]` then index off ecx), not raw table bases - a plain load, not an indexed-base address. First divergence #0 mov/push, not chased further.
// size      340 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628AD0
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x00620100

// The fifteen indexed DWORDs are the established EdgeScan fields in order.


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

int __cdecl sub_620100(EdgeScan *edge, int vertex_index) {
    int endpoint;
    for (;;) {
        if (vertex_index == TextureRasterMaxYIndex) return 0;
        endpoint = edge->f0 + vertex_index;
        const int last = TextureRasterVertexCount - 1;
        if (endpoint > last) endpoint = 0;
        else if (endpoint < 0) endpoint = last;

        edge->counter = TextureRasterEdgeVerticesB[endpoint].y -
                        TextureRasterEdgeVerticesB[vertex_index].y;
        if (edge->counter != 0) break;
        vertex_index = endpoint;
    }
    if (edge->counter <= 0) return 0;

    const int scale = edge->counter << 16;
    edge->id = endpoint;
    edge->accumA = TextureRasterEdgeVerticesA[vertex_index].x << 16;
    edge->accumB = TextureRasterEdgeVerticesA[vertex_index].y << 16;
    edge->deltaA = fixed_div(
        (TextureRasterEdgeVerticesA[endpoint].x << 16) - edge->accumA, scale);
    edge->deltaB = fixed_div(
        (TextureRasterEdgeVerticesA[endpoint].y << 16) - edge->accumB, scale);

    edge->accumC = TextureRasterEdgeVerticesB[vertex_index].x;
    int dx = TextureRasterEdgeVerticesB[endpoint].x -
             TextureRasterEdgeVerticesB[vertex_index].x;
    int delta_c;
    if (dx < 0) {
        dx = -dx;
        edge->fracAccum = 1 - edge->counter;
        edge->extraDeltaC = -1;
        delta_c = -(dx / edge->counter);
    } else {
        edge->extraDeltaC = 1;
        delta_c = dx / edge->counter;
        edge->fracAccum = 0;
    }
    edge->deltaC = delta_c;
    edge->fracDelta = dx % edge->counter;
    edge->fracSub = edge->counter;

    const int accum_d = TextureRasterZValues[vertex_index] << 16;
    edge->accumD = accum_d;
    edge->deltaD = fixed_div((TextureRasterZValues[endpoint] << 16) - accum_d, scale);
    return 1;
}

// ORIGINAL: 0x00620260 sub_620260 0x00620260-0x006202D8 FILE BYTE_EXACT
// symbol    ?sub_620260@@YAHPAUEdgeScan@@@Z
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

int __cdecl sub_620260(EdgeScan *edge) {
    edge->counter = static_cast<int>(
        static_cast<unsigned int>(edge->counter) - 1u);
    if (edge->counter == 0) {
        return sub_620100(edge, edge->id) != 0;
    }
    edge->accumA = static_cast<int>(
        static_cast<unsigned int>(edge->accumA) +
        static_cast<unsigned int>(edge->deltaA));
    edge->accumB = static_cast<int>(
        static_cast<unsigned int>(edge->accumB) +
        static_cast<unsigned int>(edge->deltaB));
    edge->accumC = static_cast<int>(
        static_cast<unsigned int>(edge->accumC) +
        static_cast<unsigned int>(edge->deltaC));
    edge->accumD = static_cast<int>(
        static_cast<unsigned int>(edge->accumD) +
        static_cast<unsigned int>(edge->deltaD));
    edge->fracAccum = static_cast<int>(
        static_cast<unsigned int>(edge->fracAccum) +
        static_cast<unsigned int>(edge->fracDelta));
    if (edge->fracAccum > 0) {
        edge->accumC = static_cast<int>(
            static_cast<unsigned int>(edge->accumC) +
            static_cast<unsigned int>(edge->extraDeltaC));
        edge->fracAccum = static_cast<int>(
            static_cast<unsigned int>(edge->fracAccum) -
            static_cast<unsigned int>(edge->fracSub));
    }
    return 1;
}

// ORIGINAL: 0x00624EB0 sub_624eb0 0x00624EB0-0x00625075 BYTE_EXACT
// symbol    ?sub_624eb0@@YAHPAUEdgeScan@@HPAVTextureStore@@@Z
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

// Historical prototype notes above retain the old guessed pointer spellings.
// The typed records below are established by EdgeScan's 0x3c layout and the
// TextureStore cursor/flags plus its ten embedded EdgeScan records.

int __cdecl sub_624eb0(EdgeScan *edge, int index, TextureStore *cache) {
    int endpoint;
    for (;;) {
        if (index == TextureRasterMaxYIndex) return 0;
        endpoint = edge->f0 + index;
        const int last = TextureRasterVertexCount - 1;
        if (endpoint > last) endpoint = 0;
        else if (endpoint < 0) endpoint = last;

        edge->counter = TextureRasterEdgeVerticesB[endpoint].y -
                        TextureRasterEdgeVerticesB[index].y;
        if (edge->counter != 0) break;
        index = endpoint;
    }
    if (edge->counter <= 0) return 0;

    EdgeScan *record = &cache->edges_[cache->next_record_];
    if (++cache->next_record_ == 10) cache->next_record_ = 0;

    edge->id = endpoint;
    const int scale = edge->counter << 16;
    edge->accumA = TextureRasterEdgeVerticesA[index].x << 16;
    edge->accumB = TextureRasterEdgeVerticesA[index].y << 16;

    if (cache->flags_ & 2u) {
        const int delta_a = fixed_div(
            (TextureRasterEdgeVerticesA[endpoint].x << 16) - edge->accumA,
            scale);
        record->deltaA = delta_a;
        edge->deltaA = delta_a;
        const int delta_b = fixed_div(
            (TextureRasterEdgeVerticesA[endpoint].y << 16) - edge->accumB,
            scale);
        record->deltaB = delta_b;
        edge->deltaB = delta_b;
    } else {
        edge->deltaA = record->deltaA;
        edge->deltaB = record->deltaB;
    }

    edge->accumC = TextureRasterEdgeVerticesB[index].x;
    int dx = TextureRasterEdgeVerticesB[endpoint].x -
             TextureRasterEdgeVerticesB[index].x;
    if (cache->flags_ & 2u) {
        const int delta_c = dx / edge->counter;
        record->deltaC = delta_c;
        edge->deltaC = delta_c;
        if (dx < 0) {
            dx = -dx;
            edge->extraDeltaC = -1;
            edge->fracAccum = 1 - edge->counter;
        } else {
            edge->extraDeltaC = 1;
            edge->fracAccum = 0;
        }
        const int frac_delta = dx % edge->counter;
        record->fracDelta = frac_delta;
        edge->fracDelta = frac_delta;
        edge->fracSub = edge->counter;
        return 1;
    }

    edge->deltaC = record->deltaC;
    if (dx < 0) {
        edge->extraDeltaC = -1;
        edge->fracAccum = 1 - edge->counter;
    } else {
        edge->extraDeltaC = 1;
        edge->fracAccum = 0;
    }
    edge->fracDelta = record->fracDelta;
    edge->fracSub = edge->counter;
    return 1;
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

// ORIGINAL: 0x0061DF60 sub_61df60 0x0061DF60-0x0061E538 FILE BYTE_EXACT
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
                         Texture *mask_texture, unsigned char *remap, int count,
                         Vert *uv, int threshold, unsigned char *palette) {
    if (vertices != 0 && buffer != 0 && pixels_ != 0 && zvals != 0 && remap != 0 &&
        mask_texture != 0 && palette != 0) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterScratch560 = static_cast<unsigned char>(threshold);
    TextureRasterPalette = palette;
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterZValues = zvals;
    TextureRasterScratch550 = remap;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    TextureRasterEdgeVerticesA = uv;
    TextureRasterVertexCount = count;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    TextureRasterScratch554 = mask_texture->pixels_;
    if (TextureRasterScratch554 != 0) {
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int max_y = -0x7ffd;
        int min_y = 0x7fff;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (int i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top) && borrowed_ != 0 &&
                    mask_texture->borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: masked span with adc
                                // carry stepping; mask byte 1 lights the texel and
                                // blends it via the palette below the threshold,
                                // other nonzero bytes are written as is.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_e3c0
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_e3c0:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_e425
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_e425:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                shl ecx, 0x10
                                sub ecx, 0x10000
                                mov cx, word ptr [TextureRasterScratch498]
                            K_e49e:
                                mov ah, byte ptr [ebx]
                                push edx
                                push ecx
                                mov ecx, dword ptr [TextureRasterScratch554]
                                mov cx, bx
                                xor edx, edx
                                mov dl, byte ptr [ecx]
                                pop ecx
                                add edx, dword ptr [TextureRasterScratch550]
                                mov dl, byte ptr [edx]
                                cmp dl, 0x0
                                je K_e4e2
                                cmp dl, 0x1
                                jne K_e4de
                                add ah, ch
                                cmp ch, byte ptr [TextureRasterScratch560]
                                jge K_e4e0
                                push edx
                                xor edx, edx
                                mov dh, ch
                                mov dl, byte ptr [edi]
                                add edx, dword ptr [TextureRasterPalette]
                                mov ah, byte ptr [edx]
                                pop edx
                                jmp K_e4e0
                            K_e4de:
                                mov ah, dl
                            K_e4e0:
                                mov byte ptr [edi], ah
                            K_e4e2:
                                pop edx
                                add cx, word ptr [TextureRasterLightPackA]
                                add edx, esi
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                inc edi
                                sub ecx, 0x10000
                                jns K_e49e
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
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
// order. Each span retains the original next-texel load after source advance,
// including the final load even when the key/cutoff suppresses a store. Shared
// scratch observations and FPU status during release callbacks are unproved.
// ORIGINAL: 0x0061AC60 Texture::draw 0x0061AC60-0x0061B1E2 FILE BYTE_EXACT
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
    TextureRasterPixelBase = buffer->get_data();
    TextureRasterLightThreshold = cutoff;
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterVertexCount = count;
    TextureRasterZValues = zvals;
    TextureRasterScratch544 = pixels_;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    int i;
    for (i = 0; i < count; ++i) {
        if (zvals[i] < cutoff) {
            break;
        }
    }
    if (i == count) {
        goto finish;
    }
    {
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: lit span with ESP as
                                // the packed pixel counter, sbb/adc stepping
                                // and a cutoff test on the light byte.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_b039
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_b039:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_b09e
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_b09e:
                                sub ebx, edi
                                mov ecx, ebx
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                xor esi, esi
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                mov al, byte ptr [ebx + esi]
                                mov cx, word ptr [TextureRasterScratch498]
                            K_b162:
                                add al, ch
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                cmp ch, ah
                                jge K_b172
                                mov byte ptr [edi], al
                            K_b172:
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                add cx, word ptr [TextureRasterLightPackA]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_b162
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    }
finish:
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}

// ORIGINAL: 0x006212B0 sub_6212b0 0x006212B0-0x00621868 FILE BYTE_EXACT
// symbol    ?sub_6212b0@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1PAVTextureStore@@@Z
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

void Texture::sub_6212b0(Buffer *buffer, Vert *vertices, int *zvals, int count,
                         Vert *uv, TextureStore *cache) {
    int needs_clip = 1;
    cache->next_record_ = 0;
    TextureRasterPixelBase = buffer->get_data();
    TextureRasterLightThreshold = static_cast<int>(TextureRasterKey);
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterVertexCount = count;
    TextureRasterZValues = zvals;
    TextureRasterScratch544 = pixels_;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        for (int i = 0; i < count; ++i) {
            zvals[i] = min(zvals[i], TextureLightCeiling);
            zvals[i] = max(zvals[i], TextureLightFloor);
        }
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (i = 0; i < TextureRasterVertexCount; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            if (min_x < TextureRasterClipLeft || min_y < TextureRasterClipTop ||
                max_x > TextureRasterClipRight || max_y > TextureRasterClipBottom) {
                needs_clip = 1;
            } else {
                needs_clip = 0;
            }
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (sub_625080(&left, top, cache)) {
                right.f0 = 1;
                if (sub_625080(&right, top, cache) && borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!sub_625080(&left, left.id, cache)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!sub_625080(&right, right.id, cache)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: color-keyed lit span
                                // with adc carry stepping.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_170a
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_170a:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_176f
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_176f:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                shl ecx, 0x10
                                sub ecx, 0x10000
                                mov cx, word ptr [TextureRasterScratch498]
                                sub cx, word ptr [TextureRasterLightPackA]
                                dec edi
                            K_17f6:
                                add cx, word ptr [TextureRasterLightPackA]
                                mov al, byte ptr [ebx]
                                inc edi
                                add edx, esi
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                cmp al, ah
                                je K_1810
                                add al, ch
                                mov byte ptr [edi], al
                            K_1810:
                                sub ecx, 0x10000
                                jns K_17f6
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
    if (needs_clip == 0) {
        cache->flags_ = 0;
    }
}

// ORIGINAL: 0x00622810 sub_622810 0x00622810-0x00622DAE FILE BYTE_EXACT
// symbol    ?sub_622810@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1HPAVTextureStore@@@Z
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

void Texture::sub_622810(Buffer *buffer, Vert *vertices, int *zvals, int count,
                         Vert *uv, int threshold, TextureStore *cache) {
    int needs_clip = 1;
    cache->next_record_ = 0;
    TextureRasterPixelBase = buffer->get_data();
    TextureRasterLightThreshold = threshold;
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterVertexCount = count;
    TextureRasterZValues = zvals;
    TextureRasterScratch544 = pixels_;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    int i;
    for (i = 0; i < count; ++i) {
        if (zvals[i] < threshold) {
            break;
        }
    }
    if (i == count) {
        goto finish;
    }
    {
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            if (min_x < TextureRasterClipLeft || min_y < TextureRasterClipTop ||
                max_x > TextureRasterClipRight || max_y > TextureRasterClipBottom) {
                needs_clip = 1;
            } else {
                needs_clip = 0;
            }
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (sub_625080(&left, top, cache)) {
                right.f0 = 1;
                if (sub_625080(&right, top, cache) && borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!sub_625080(&left, left.id, cache)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!sub_625080(&right, right.id, cache)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: lit span with adc carry
                                // stepping that draws only below the threshold.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_2c53
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_2c53:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_2cb8
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_2cb8:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                shl ecx, 0x10
                                sub ecx, 0x10000
                                mov cx, word ptr [TextureRasterScratch498]
                                sub cx, word ptr [TextureRasterLightPackA]
                                dec edi
                            K_2d3f:
                                add cx, word ptr [TextureRasterLightPackA]
                                mov al, byte ptr [ebx]
                                inc edi
                                add edx, esi
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                cmp ch, ah
                                jge K_2d59
                                add al, ch
                                mov byte ptr [edi], al
                            K_2d59:
                                sub ecx, 0x10000
                                jns K_2d3f
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    }
finish:
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
    if (needs_clip == 0) {
        cache->flags_ = 0;
    }
}

// ORIGINAL: 0x00622200 sub_622200 0x00622200-0x0062280C FILE BYTE_EXACT
// symbol    ?sub_622200@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPAV1@PAEH1PAVTextureStore@@@Z
// TRIED: __thiscall-receiver rewrite of Ghidra's decode (guard chain, bbox min/max scan, sub_625080 edge-walk setup, then a plain-C long-double port of the FPU gradient section and a byte-macro (concat31/21/22/11, carry4) port of the CONCAT/CARRY inner pixel-blend loop) compiles but diverges at instruction #0 (push vs sub - no local-var-count/register-allocation match is plausible for a function this size without a register-literal transcription of all ~470 instructions, not attempted here).
// working copy - scaffold materialised by --work
// size      1548 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00625080


void Texture::sub_622200(Buffer *buffer, Vert *vertices, int *zvals,
                         Texture *shade_texture, unsigned char *remap, int count,
                         Vert *uv, TextureStore *cache) {
    int needs_clip = 1;
    if (vertices != 0 && buffer != 0 && pixels_ != 0 && zvals != 0 && remap != 0 &&
        shade_texture != 0 && cache != 0) {
    cache->next_record_ = 0;
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterZValues = zvals;
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterScratch550 = remap;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    TextureRasterVertexCount = count;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    TextureRasterScratch554 = shade_texture->pixels_;
    if (TextureRasterScratch554 != 0) {
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int max_y = -0x7ffd;
        int min_y = 0x7fff;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (int i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            if (min_x < TextureRasterClipLeft || min_y < TextureRasterClipTop ||
                max_x > TextureRasterClipRight || max_y > TextureRasterClipBottom) {
                needs_clip = 1;
            } else {
                needs_clip = 0;
            }
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (sub_625080(&left, top, cache)) {
                right.f0 = 1;
                if (sub_625080(&right, top, cache) && borrowed_ != 0 &&
                    shade_texture->borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!sub_625080(&left, left.id, cache)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!sub_625080(&right, right.id, cache)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: span with adc carry
                                // stepping that remaps the shade texel and adds
                                // the light.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_2694
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_2694:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_26f9
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_26f9:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                shl ecx, 0x10
                                sub ecx, 0x10000
                                mov cx, word ptr [TextureRasterScratch498]
                            K_2772:
                                mov ah, byte ptr [ebx]
                                push edx
                                push ecx
                                mov ecx, dword ptr [TextureRasterScratch554]
                                mov cx, bx
                                xor edx, edx
                                mov dl, byte ptr [ecx]
                                pop ecx
                                add edx, dword ptr [TextureRasterScratch550]
                                mov dl, byte ptr [edx]
                                cmp dl, 0x0
                                je K_279e
                                cmp dl, 0x1
                                je K_279a
                                mov ah, dl
                                sub ah, ch
                            K_279a:
                                add ah, ch
                                mov byte ptr [edi], ah
                            K_279e:
                                pop edx
                                add cx, word ptr [TextureRasterLightPackA]
                                add edx, esi
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                inc edi
                                sub ecx, 0x10000
                                jns K_2772
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
    if (needs_clip == 0) {
        cache->flags_ = 0;
    }
}

// ORIGINAL: 0x0061C630 sub_61c630 0x0061C630-0x0061CB4E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recover geometry and UV arrays as Vert pointers; the final argument is consumed, not unused.","evidence":["Pinned61C630 publishes its second/final arguments into geometry/UV pointer cells9BB4F4/9BB490 and walks signed coordinates at eight-byte stride. Typed edge setup consumes both channels.","The real623F50sentinel wrapper forwards both values unchanged. Seed-generated integer signatures do not establish original types; producer-side callers outside this closure remain unobserved."]}
// symbol    ?sub_61c630@Texture@@QAEXPAVBuffer@@PAUVert@@PAV1@PAEH1@Z
// working copy - scaffold materialised by --work
// size      1310 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00624D80

// STRUCTURAL EVIDENCE: setup_edge 0x624D80 writes through EdgeScan+0x30;
// sub_61c630 reads untouched firstEdge+0x34 into scratch 0x9BB498 at 0x61C875/87B.
// Preserve that indeterminate stack effect; its original value/pixel impact is unproved.

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

void Texture::sub_61c630(Buffer *buffer, Vert *vertices, Texture *shade,
                         unsigned char *remap, int count, Vert *uv) {
    EdgeScan left;
    EdgeScan right;
    if (vertices != 0 && buffer != 0 && pixels_ != 0 && remap != 0 && shade != 0) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterScratch550 = remap;
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    TextureRasterVertexCount = count;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    TextureRasterScratch554 = shade->pixels_;
    if (TextureRasterScratch554 != 0) {
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int max_y = -0x7ffd;
        int min_y = 0x7fff;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        for (int i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge(&right, top) && borrowed_ != 0 &&
                    shade->borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dv;
                    int du;
                    if (span > 0) {
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(3)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_uv_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_uv_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmulp st(2), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(3)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: span with adc carry
                                // stepping that writes texels or table bytes
                                // as the second texture's shade byte selects.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_ca22
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_ca22:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_ca6e
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_ca6e:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                shl ecx, 0x10
                                sub ecx, 0x10000
                            K_cad3:
                                mov ah, byte ptr [ebx]
                                push edx
                                push ecx
                                mov ecx, dword ptr [TextureRasterScratch554]
                                mov cx, bx
                                xor edx, edx
                                mov dl, byte ptr [ecx]
                                add edx, dword ptr [TextureRasterScratch550]
                                mov dl, byte ptr [edx]
                                cmp dl, 0x0
                                je K_cafa
                                cmp dl, 0x1
                                je K_caf8
                                mov ah, dl
                            K_caf8:
                                mov byte ptr [edi], ah
                            K_cafa:
                                pop ecx
                                pop edx
                                add edx, esi
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                inc edi
                                sub ecx, 0x10000
                                jns K_cad3
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
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

// ORIGINAL: 0x0061D990 sub_61d990 0x0061D990-0x0061DF5C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recover the shaded rasterizer's geometry as Vert pointers while keeping its separate per-vertex int light array.","evidence":["Pinned61D990 publishes geometry to9BB4F4, scans x/y at offsets0/+4 with eight-byte stride, and separately publishes the light pointer to9BB4FC. Existing typed edge consumers distinguish these channels.","Seed-generated decorated symbols are not original type evidence; guards, release order and unknown external producer contracts remain unchanged."]}
// symbol    ?sub_61d990@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPAV1@PAEH@Z
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

void Texture::sub_61d990(Buffer *buffer, Vert *vertices, int *zvals,
                         Texture *shade_texture, unsigned char *remap,
                         int wide_flag) {
    if (vertices != 0 && buffer != 0 && pixels_ != 0 && zvals != 0 && remap != 0 &&
        shade_texture != 0) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterZValues = zvals;
    TextureRasterScratch550 = remap;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    if (wide_flag) {
        TextureRasterVertexCount = 8;
        TextureRasterEdgeVerticesA = cornersAndMids_;
    } else {
        TextureRasterVertexCount = 4;
        TextureRasterEdgeVerticesA = corners_;
    }
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    TextureRasterScratch554 = shade_texture->pixels_;
    if (TextureRasterScratch554 != 0) {
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int max_y = -0x7ffd;
        int min_y = 0x7fff;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (int i = 0; i < TextureRasterVertexCount; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top) && borrowed_ != 0 &&
                    shade_texture->borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: shaded span with adc
                                // carry stepping; shade bytes below 5 add the
                                // light to the remapped texel, others replace it.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_ddf5
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_ddf5:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_de5a
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_de5a:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                shl ecx, 0x10
                                sub ecx, 0x10000
                                mov cx, word ptr [TextureRasterScratch498]
                            K_ded3:
                                mov ah, byte ptr [ebx]
                                push edx
                                push ecx
                                mov ecx, dword ptr [TextureRasterScratch554]
                                mov cx, bx
                                xor edx, edx
                                mov dl, byte ptr [ecx]
                                pop ecx
                                cmp dl, 0x5
                                jb K_def5
                                add edx, dword ptr [TextureRasterScratch550]
                                mov ah, byte ptr [edx]
                                pop edx
                                jmp K_df05
                            K_def5:
                                add ah, ch
                                add edx, dword ptr [TextureRasterScratch550]
                                mov al, byte ptr [edx]
                                cmp dl, 0x5
                                pop edx
                                add ah, al
                            K_df05:
                                mov byte ptr [edi], ah
                                add cx, word ptr [TextureRasterLightPackA]
                                add edx, esi
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                inc edi
                                sub ecx, 0x10000
                                jns K_ded3
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}


// Fixed-slot bindings carried from 0061b780.cpp

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

// ORIGINAL: 0x0061B780 sub_61b780 0x0061B780-0x0061C08E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recover separate Vert geometry and int light-array pointer contracts in the Gouraud rasterizer.","evidence":["Pinned61B780 publishes arguments to geometry cell9BB4F4 and light cell9BB4FC; coordinate traversal has eight-byte x/y stride, whereas the light edge consumer indexes independent int values.","No upstream source producer is established. These are use-derived contracts, not original source spelling; existing row/pixel traversal and restoration obligations are retained."]}
// CURRENT BODY STATUS: multirow/pixel DDA restored; x87 status and
// exact packed carry spelling remain restoration debt.
// symbol    ?sub_61b780@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH@Z
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


void Texture::sub_61b780(Buffer *buffer, Vert *vertices, int *zvals, int mode) {
    TextureRasterPixelBase = buffer->get_data();
    TextureRasterLightThreshold = static_cast<int>(TextureRasterKey);
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    if (mode) {
        TextureRasterVertexCount = 8;
        TextureRasterEdgeVerticesA = cornersAndMids_;
    } else {
        TextureRasterVertexCount = 4;
        TextureRasterEdgeVerticesA = corners_;
    }
    TextureRasterZValues = zvals;
    TextureRasterScratch544 = pixels_;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int max_y = -0x7ffd;
        int min_y = 0x7fff;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (int i = 0; i < TextureRasterVertexCount; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top)) {
                if (borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: color-keyed lit span
                                // with adc carry stepping.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_bb7d
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_bb7d:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_bbe2
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_bbe2:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                shl ecx, 0x10
                                sub ecx, 0x10000
                                mov cx, word ptr [TextureRasterScratch498]
                                sub cx, word ptr [TextureRasterLightPackA]
                                dec edi
                            K_bc69:
                                add cx, word ptr [TextureRasterLightPackA]
                                mov al, byte ptr [ebx]
                                inc edi
                                add edx, esi
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                cmp al, ah
                                je K_bc83
                                add al, ch
                                mov byte ptr [edi], al
                            K_bc83:
                                sub ecx, 0x10000
                                jns K_bc69
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                } else {
                    // Without a borrowed 256-pitch texture the rows step U/V
                    // in integer fixed point.
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        do {
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                int u = left.accumA;
                                int v = left.accumB;
                                int x = left.accumC;
                                TextureRasterLightAccumB.dword = left.accumD;
                                int ru = right.accumA;
                                int rv = right.accumB;
                                int rx = right.accumC;
                                TextureRasterLightStepB = right.accumD;
                                int du;
                                int dv;
                                int dv_step;
                                int u_dir;
                                int du_whole;
                                // Hand-written assembly: idiv slopes, signed
                                // texture pitch stepping and adc/sbb row carry
                                // in two mirrored span loops.
                                __asm {
                        mov ebx, dword ptr [rx]
                        cmp ebx, dword ptr [TextureRasterClipLeft]
                        jle row_done
                        mov ecx, dword ptr [x]
                        cmp ecx, dword ptr [TextureRasterClipRight]
                        jge row_done
                        sub ebx, ecx
                        jle row_done
                        shl ebx, 0x10
                        mov eax, dword ptr [TextureRasterLightStepB]
                        sub eax, dword ptr [TextureRasterLightAccumB]
                        shl eax, 0x8
                        idiv ebx
                        mov word ptr [TextureRasterLightPackA], ax
                        mov eax, dword ptr [ru]
                        sub eax, dword ptr [u]
                        mov edx, eax
                        sar edx, 0x10
                        shl eax, 0x10
                        idiv ebx
                        mov dword ptr [du], eax
                        mov ecx, 0x1
                        and eax, eax
                        jns K_bd77
                        neg ecx
                        test eax, 0xffff
                        je K_bd77
                        add eax, 0x10000
                    K_bd77:
                        mov dword ptr [u_dir], ecx
                        mov ecx, eax
                        sar ecx, 0x10
                        mov dword ptr [du_whole], ecx
                        sar eax, 1
                        add eax, dword ptr [u]
                        mov dword ptr [u], eax
                        mov eax, dword ptr [rv]
                        sub eax, dword ptr [v]
                        mov edx, eax
                        sar edx, 0x10
                        shl eax, 0x10
                        idiv ebx
                        mov dword ptr [dv], eax
                        mov ecx, dword ptr [TextureRowPitchScratch]
                        and eax, eax
                        jns K_bdbe
                        neg ecx
                        test eax, 0xffff
                        je K_bdbe
                        add eax, 0x10000
                    K_bdbe:
                        mov dword ptr [TextureRasterSignedPitch], ecx
                        mov ecx, eax
                        sar eax, 0x10
                        imul dword ptr [TextureRowPitchScratch]
                        mov dword ptr [dv_step], eax
                        sar ecx, 1
                        add ecx, dword ptr [v]
                        mov dword ptr [v], ecx
                        mov ebx, dword ptr [rx]
                        cmp ebx, dword ptr [TextureRasterClipRight]
                        jl K_bdee
                        mov ebx, dword ptr [TextureRasterClipRight]
                    K_bdee:
                        mov edi, dword ptr [x]
                        cmp edi, dword ptr [TextureRasterClipLeft]
                        jge K_be4a
                        sub edi, dword ptr [TextureRasterClipLeft]
                        neg edi
                        mov eax, dword ptr [du]
                        shl edi, 0x10
                        imul edi
                        shrd eax, edx, 0x10
                        add eax, dword ptr [u]
                        mov dword ptr [u], eax
                        mov eax, dword ptr [dv]
                        imul edi
                        shrd eax, edx, 0x10
                        add eax, dword ptr [v]
                        mov dword ptr [v], eax
                        xor eax, eax
                        mov ax, word ptr [TextureRasterLightPackA]
                        imul edi
                        shrd eax, edx, 0x10
                        add eax, dword ptr [TextureRasterLightAccumB]
                        mov dword ptr [TextureRasterLightAccumB], eax
                        mov edi, dword ptr [TextureRasterClipLeft]
                        mov dword ptr [x], edi
                    K_be4a:
                        sub ebx, edi
                        mov ecx, ebx
                        xor eax, eax
                        mov ax, word ptr [v+2]
                        mov dx, word ptr [TextureRowPitchScratch]
                        imul dx
                        xor esi, esi
                        mov si, word ptr [u+2]
                        add esi, eax
                        add esi, dword ptr [TextureRasterScratch544]
                        mov edi, dword ptr [TextureRasterRowOffset]
                        add edi, dword ptr [x]
                        cmp dword ptr [TextureRasterSignedPitch], 0x0
                        jge K_be87
                        neg dword ptr [dv]
                        not dword ptr [v]
                    K_be87:
                        cmp dword ptr [u_dir], 0x0
                        jl K_bf04
                        mov ebx, dword ptr [u]
                        shl ebx, 0x10
                        mov bx, word ptr [v]
                        shr bx, 1
                        mov edx, dword ptr [du]
                        shl edx, 0x10
                        mov dx, word ptr [dv]
                        shr dx, 1
                        mov eax, dword ptr [du_whole]
                        add eax, dword ptr [dv_step]
                        mov dword ptr [TextureRasterScratch4B8], eax
                        push ebp
                        mov ebp, dword ptr [TextureRasterScratch4B8]
                        dec edi
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        shr eax, 0x8
                    K_bed1:
                        mov ah, byte ptr [esi]
                        inc edi
                        add ah, al
                        add ebx, edx
                        adc esi, ebp
                        mov byte ptr [edi], ah
                        test bh, 0x80
                        je K_beea
                        and bh, 0x7f
                        add esi, dword ptr [TextureRasterSignedPitch]
                    K_beea:
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        add ax, word ptr [TextureRasterLightPackA]
                        mov dword ptr [TextureRasterLightAccumB], eax
                        shr eax, 0x8
                        dec ecx
                        jne K_bed1
                        pop ebp
                        jmp row_done
                    K_bf04:
                        mov edx, dword ptr [du]
                        neg edx
                        shl edx, 0x10
                        mov dx, word ptr [dv]
                        shr dx, 1
                        mov ebx, dword ptr [u]
                        not ebx
                        shl ebx, 0x10
                        mov bx, word ptr [v]
                        shr dword ptr [v], 1
                        mov eax, dword ptr [du_whole]
                        add eax, dword ptr [dv_step]
                        mov dword ptr [TextureRasterScratch4B8], eax
                        neg dword ptr [TextureRasterScratch4B8]
                        push ebp
                        mov ebp, dword ptr [TextureRasterScratch4B8]
                        dec edi
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        shr eax, 0x8
                    K_bf4f:
                        mov ah, byte ptr [esi]
                        inc edi
                        add ah, al
                        add ebx, edx
                        sbb esi, ebp
                        mov byte ptr [edi], ah
                        test bh, 0x80
                        je K_bf68
                        and bh, 0x7f
                        add esi, dword ptr [TextureRasterSignedPitch]
                    K_bf68:
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        add ax, word ptr [TextureRasterLightPackA]
                        mov dword ptr [TextureRasterLightAccumB], eax
                        shr eax, 0x8
                        dec ecx
                        jne K_bf4f
                        pop ebp
                                row_done:
                                }
                            }
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    break;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    break;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        } while (TextureRasterRowOffset < TextureRasterClipBottom);
                    }
                }
            }
        }
    }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}

// Fixed-slot bindings carried from 00620a10.cpp

// ORIGINAL: 0x00620A10 sub_620a10 0x00620A10-0x006212A4 FILE
// RECOVERY_BINDING: v1 {"reason":"Recover the affine rasterizer's two vertex-array arguments as the existing Vert type instead of passing pointers through int.","evidence":["Pinned 0x620A10 stores stack arguments at EBP+0x0C/+0x14 unchanged into shared vertex-pointer cells 0x9BB4F4/0x9BB490; 0x620AE1..0x620B1C indexes the first at eight-byte stride and reads signed x/y at offsets 0/+4.","The complete mapped 0x624CA0 wrapper forwards both pointer values unchanged when its final argument is -1; both shared cells are consumed as Vert arrays by the existing typed edge setup.","Historical decorated seed symbols and IDA int4 guesses were generated hypotheses, not independent original C++ type evidence. The recovered pointer contract follows actual uses; unobserved upstream producers and original type spelling remain unknown."]}
// CURRENT BODY STATUS: multirow keyed affine pixel traversal restored;
// exact x87 status/packed carry equivalence remains restoration debt.
// symbol    ?sub_620a10@Texture@@QAEXPAVBuffer@@PAUVert@@H1PAVTextureStore@@@Z
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

void Texture::sub_620a10(Buffer *buffer, Vert *vertices, int count, Vert *uv,
                         TextureStore *cache) {
    int needs_clip = 1;
    cache->next_record_ = 0;
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    TextureRasterEdgeVerticesA = uv;
    TextureRasterVertexCount = count;
    TextureRasterLightThreshold = static_cast<int>(TextureRasterKey);
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int min_x = 0x7fff;
        int max_x = -0x7ffd;
        int top;
        EdgeScan left;
        EdgeScan right;
        Vert *v = vertices;
        for (int i = 0; i < count; ++i, ++v) {
            if (v->y < min_y) {
                min_y = v->y;
                top = i;
            }
            if (v->y > max_y) {
                // volatile: stand-in for the image's reload of v->y in the max branch; no plain spelling found
                int y = *(volatile int *)&v->y;
                TextureRasterMaxYIndex = i;
                max_y = y;
            }
            if (v->x < min_x) {
                min_x = v->x;
            }
            if (v->x > max_x) {
                // volatile: stand-in for the image's reload of v->x in the max branch; no plain spelling found
                max_x = *(volatile int *)&v->x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            if (min_x < TextureRasterClipLeft || min_y < TextureRasterClipTop ||
                max_x > TextureRasterClipRight || max_y > TextureRasterClipBottom) {
                needs_clip = 1;
            } else {
                needs_clip = 0;
            }
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (sub_624eb0(&left, top, cache)) {
                right.f0 = 1;
                if (sub_624eb0(&right, top, cache)) {
                if (borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dv;
                    int du;
                    if (span > 0) {
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(3)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!sub_624eb0(&left, left.id, cache)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_uv_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!sub_624eb0(&right, right.id, cache)) {
                                    done = 1;
                                }
                            } else {
                                texture_uv_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmulp st(2), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(3)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: color-keyed span with
                                // adc carry stepping.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_df9
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_df9:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_e45
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_e45:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                shl ecx, 0x10
                                sub ecx, 0x10000
                                dec edi
                            K_eb1:
                                mov al, byte ptr [ebx]
                                inc edi
                                cmp al, ah
                                je K_eba
                                mov byte ptr [edi], al
                            K_eba:
                                add edx, esi
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                sub ecx, 0x10000
                                jns K_eb1
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                } else {
                    // Without a borrowed 256-pitch texture the rows step U/V
                    // in integer fixed point.
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        do {
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                int u = left.accumA;
                                int v = left.accumB;
                                int ru = right.accumA;
                                int rv = right.accumB;
                                int x = left.accumC;
                                int rx = right.accumC;
                                int du;
                                int dv;
                                int dv_step;
                                int u_dir;
                                int du_whole;
                                // Hand-written assembly: idiv slopes, signed
                                // pitch stepping, adc/sbb row carry and its
                                // own ESI/EDI saves; the destination scales the
                                // row address by the stride again, as shipped.
                                __asm {
                                    push esi
                                    push edi
                                    mov ebx, dword ptr [rx]
                                    cmp ebx, dword ptr [TextureRasterClipLeft]
                                    jle K_1198
                                    mov ecx, dword ptr [x]
                                    cmp ecx, dword ptr [TextureRasterClipRight]
                                    jge K_1198
                                    sub ebx, ecx
                                    jle K_1198
                                    shl ebx, 0x10
                                    mov eax, dword ptr [ru]
                                    sub eax, dword ptr [u]
                                    mov edx, eax
                                    sar edx, 0x10
                                    shl eax, 0x10
                                    idiv ebx
                                    mov dword ptr [du], eax
                                    mov ecx, 0x1
                                    and eax, eax
                                    jns K_fa0
                                    neg ecx
                                    test eax, 0xffff
                                    je K_fa0
                                    add eax, 0x10000
                                K_fa0:
                                    mov dword ptr [u_dir], ecx
                                    mov ecx, eax
                                    sar ecx, 0x10
                                    mov dword ptr [du_whole], ecx
                                    sar eax, 1
                                    add eax, dword ptr [u]
                                    mov dword ptr [u], eax
                                    mov eax, dword ptr [rv]
                                    sub eax, dword ptr [v]
                                    mov edx, eax
                                    sar edx, 0x10
                                    shl eax, 0x10
                                    idiv ebx
                                    mov dword ptr [dv], eax
                                    mov ecx, dword ptr [TextureRowPitchScratch]
                                    and eax, eax
                                    jns K_fe7
                                    neg ecx
                                    test eax, 0xffff
                                    je K_fe7
                                    add eax, 0x10000
                                K_fe7:
                                    mov dword ptr [TextureRasterSignedPitch], ecx
                                    mov ecx, eax
                                    sar eax, 0x10
                                    imul dword ptr [TextureRowPitchScratch]
                                    mov dword ptr [dv_step], eax
                                    sar ecx, 1
                                    add ecx, dword ptr [v]
                                    mov dword ptr [v], ecx
                                    mov ebx, dword ptr [rx]
                                    cmp ebx, dword ptr [TextureRasterClipRight]
                                    jl K_101a
                                    mov ebx, dword ptr [TextureRasterClipRight]
                                K_101a:
                                    mov edi, dword ptr [x]
                                    cmp edi, dword ptr [TextureRasterClipLeft]
                                    jge K_105d
                                    sub edi, dword ptr [TextureRasterClipLeft]
                                    neg edi
                                    mov eax, dword ptr [du]
                                    shl edi, 0x10
                                    imul edi
                                    shrd eax, edx, 0x10
                                    add eax, dword ptr [u]
                                    mov dword ptr [u], eax
                                    mov eax, dword ptr [dv]
                                    imul edi
                                    shrd eax, edx, 0x10
                                    add eax, dword ptr [v]
                                    mov dword ptr [v], eax
                                    mov edi, dword ptr [TextureRasterClipLeft]
                                    mov dword ptr [x], edi
                                K_105d:
                                    sub ebx, edi
                                    mov ecx, ebx
                                    add dword ptr [v], 0x8000
                                    xor eax, eax
                                    mov ax, word ptr [v+2]
                                    mov dx, word ptr [TextureRowPitchScratch]
                                    imul dx
                                    add dword ptr [u], 0x8000
                                    xor esi, esi
                                    mov si, word ptr [u+2]
                                    add esi, eax
                                    add esi, dword ptr [TextureRasterScratch544]
                                    mov eax, dword ptr [TextureRasterRowOffset]
                                    imul dword ptr [TextureRasterBufferStride]
                                    add eax, dword ptr [x]
                                    add eax, dword ptr [TextureRasterPixelBase]
                                    mov edi, eax
                                    cmp dword ptr [TextureRasterSignedPitch], 0x0
                                    jge K_10b5
                                    neg dword ptr [dv]
                                    not dword ptr [v]
                                K_10b5:
                                    cmp dword ptr [u_dir], 0x0
                                    jl K_1127
                                    mov ebx, dword ptr [u]
                                    shl ebx, 0x10
                                    mov bx, word ptr [v]
                                    shr bx, 1
                                    mov edx, dword ptr [du]
                                    shl edx, 0x10
                                    mov dx, word ptr [dv]
                                    shr dx, 1
                                    mov dword ptr [TextureRasterPackedStep], edx
                                    mov eax, dword ptr [du_whole]
                                    add eax, dword ptr [dv_step]
                                    mov dword ptr [TextureRasterScratch4B8], eax
                                    mov ah, byte ptr [TextureRasterLightThreshold]
                                    push ebp
                                    mov ebp, dword ptr [TextureRasterScratch4B8]
                                K_1102:
                                    mov al, byte ptr [esi]
                                    cmp al, ah
                                    je K_110a
                                    mov byte ptr [edi], al
                                K_110a:
                                    add ebx, dword ptr [TextureRasterPackedStep]
                                    adc esi, ebp
                                    test bh, 0x80
                                    je K_1120
                                    add esi, dword ptr [TextureRasterSignedPitch]
                                    and bh, 0x7f
                                K_1120:
                                    inc edi
                                    dec ecx
                                    jne K_1102
                                    pop ebp
                                    jmp K_1198
                                K_1127:
                                    mov edx, dword ptr [du]
                                    neg edx
                                    shl edx, 0x10
                                    mov dx, word ptr [dv]
                                    shr dx, 1
                                    mov dword ptr [TextureRasterPackedStep], edx
                                    mov ebx, dword ptr [u]
                                    not ebx
                                    shl ebx, 0x10
                                    mov bx, word ptr [v]
                                    shr dword ptr [v], 1
                                    mov eax, dword ptr [du_whole]
                                    add eax, dword ptr [dv_step]
                                    mov dword ptr [TextureRasterScratch4B8], eax
                                    neg dword ptr [TextureRasterScratch4B8]
                                    mov ah, byte ptr [TextureRasterLightThreshold]
                                    push ebp
                                    mov ebp, dword ptr [TextureRasterScratch4B8]
                                K_1175:
                                    mov al, byte ptr [esi]
                                    cmp al, ah
                                    je K_117d
                                    mov byte ptr [edi], al
                                K_117d:
                                    add ebx, dword ptr [TextureRasterPackedStep]
                                    sbb esi, ebp
                                    test bh, 0x80
                                    je K_1193
                                    add esi, dword ptr [TextureRasterSignedPitch]
                                    and bh, 0x7f
                                K_1193:
                                    inc edi
                                    dec ecx
                                    jne K_1175
                                    pop ebp
                                K_1198:
                                    pop edi
                                    pop esi
                                row_done:
                                }
                            }
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge(&left, left.id)) {
                                    break;
                                }
                            } else {
                                texture_uv_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge(&right, right.id)) {
                                    break;
                                }
                            } else {
                                texture_uv_edge_step(right);
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        } while (TextureRasterRowOffset < TextureRasterClipBottom);
                    }
                }
                }
            }
        }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    if (needs_clip == 0) {
        cache->flags_ = 1;
    }
    __asm finit
}


// MEMBER RECOVERY: ECX receiver + four explicit arguments; full row/pixel
// traversal uses no-key Gouraud byte addition. x87 and packed-carry
// bit equivalence remains a restoration obligation.
// ORIGINAL: 0x0061D090 sub_61d090 0x0061D090-0x0061D98A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recover the no-key Gouraud rasterizer's geometry pointer as Vert rather than reinterpreting int pairs.","evidence":["Pinned61D090 and its existing typed row helper use geometry pointer9BB4F4 with eight-byte coordinate stride and the separate int light pointer9BB4FC. The original receiver and four stack arguments remain unchanged.","Historical seed symbols were inferred; producer contracts and exact x87/packed-carry restoration remain separate obligations."]}
// symbol    ?sub_61d090@Texture@@QAEXPAVBuffer@@PAUVert@@PAHI@Z
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


void Texture::sub_61d090(Buffer *buffer, Vert *vertices, int *zvals, unsigned int mode) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    if (mode) {
        TextureRasterVertexCount = 8;
        TextureRasterEdgeVerticesA = cornersAndMids_;
    } else {
        TextureRasterVertexCount = 4;
        TextureRasterEdgeVerticesA = corners_;
    }
    TextureRasterZValues = zvals;
    TextureRasterScratch544 = pixels_;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int max_y = -0x7ffd;
        int min_y = 0x7fff;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (int i = 0; i < TextureRasterVertexCount; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top)) {
                if (borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: lit span with adc
                                // carry stepping.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_d483
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_d483:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_d4e8
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_d4e8:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                shl ecx, 0x10
                                sub ecx, 0x10000
                                mov cx, word ptr [TextureRasterScratch498]
                                sub cx, word ptr [TextureRasterLightPackA]
                                dec edi
                            K_d569:
                                add cx, word ptr [TextureRasterLightPackA]
                                mov al, byte ptr [ebx]
                                inc edi
                                add al, ch
                                add edx, esi
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                sub ecx, 0x10000
                                mov byte ptr [edi], al
                                jns K_d569
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                } else {
                    // Without a borrowed 256-pitch texture the rows step U/V
                    // in integer fixed point.
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        do {
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                int u = left.accumA;
                                int v = left.accumB;
                                int x = left.accumC;
                                TextureRasterLightAccumB.dword = left.accumD;
                                int ru = right.accumA;
                                int rv = right.accumB;
                                int rx = right.accumC;
                                TextureRasterLightStepB = right.accumD;
                                int du;
                                int dv;
                                int dv_step;
                                int u_dir;
                                int du_whole;
                                // Hand-written assembly: idiv slopes, signed
                                // texture pitch stepping and adc/sbb row carry
                                // in two mirrored span loops.
                                __asm {
                        mov ebx, dword ptr [rx]
                        cmp ebx, dword ptr [TextureRasterClipLeft]
                        jle row_done
                        mov ecx, dword ptr [x]
                        cmp ecx, dword ptr [TextureRasterClipRight]
                        jge row_done
                        sub ebx, ecx
                        jle row_done
                        shl ebx, 0x10
                        mov eax, dword ptr [TextureRasterLightStepB]
                        sub eax, dword ptr [TextureRasterLightAccumB]
                        shl eax, 0x8
                        idiv ebx
                        mov word ptr [TextureRasterLightPackA], ax
                        mov eax, dword ptr [ru]
                        sub eax, dword ptr [u]
                        mov edx, eax
                        sar edx, 0x10
                        shl eax, 0x10
                        idiv ebx
                        mov dword ptr [du], eax
                        mov ecx, 0x1
                        and eax, eax
                        jns K_bd77
                        neg ecx
                        test eax, 0xffff
                        je K_bd77
                        add eax, 0x10000
                    K_bd77:
                        mov dword ptr [u_dir], ecx
                        mov ecx, eax
                        sar ecx, 0x10
                        mov dword ptr [du_whole], ecx
                        sar eax, 1
                        add eax, dword ptr [u]
                        mov dword ptr [u], eax
                        mov eax, dword ptr [rv]
                        sub eax, dword ptr [v]
                        mov edx, eax
                        sar edx, 0x10
                        shl eax, 0x10
                        idiv ebx
                        mov dword ptr [dv], eax
                        mov ecx, dword ptr [TextureRowPitchScratch]
                        and eax, eax
                        jns K_bdbe
                        neg ecx
                        test eax, 0xffff
                        je K_bdbe
                        add eax, 0x10000
                    K_bdbe:
                        mov dword ptr [TextureRasterSignedPitch], ecx
                        mov ecx, eax
                        sar eax, 0x10
                        imul dword ptr [TextureRowPitchScratch]
                        mov dword ptr [dv_step], eax
                        sar ecx, 1
                        add ecx, dword ptr [v]
                        mov dword ptr [v], ecx
                        mov ebx, dword ptr [rx]
                        cmp ebx, dword ptr [TextureRasterClipRight]
                        jl K_bdee
                        mov ebx, dword ptr [TextureRasterClipRight]
                    K_bdee:
                        mov edi, dword ptr [x]
                        cmp edi, dword ptr [TextureRasterClipLeft]
                        jge K_be4a
                        sub edi, dword ptr [TextureRasterClipLeft]
                        neg edi
                        mov eax, dword ptr [du]
                        shl edi, 0x10
                        imul edi
                        shrd eax, edx, 0x10
                        add eax, dword ptr [u]
                        mov dword ptr [u], eax
                        mov eax, dword ptr [dv]
                        imul edi
                        shrd eax, edx, 0x10
                        add eax, dword ptr [v]
                        mov dword ptr [v], eax
                        xor eax, eax
                        mov ax, word ptr [TextureRasterLightPackA]
                        imul edi
                        shrd eax, edx, 0x10
                        add eax, dword ptr [TextureRasterLightAccumB]
                        mov dword ptr [TextureRasterLightAccumB], eax
                        mov edi, dword ptr [TextureRasterClipLeft]
                        mov dword ptr [x], edi
                    K_be4a:
                        sub ebx, edi
                        mov ecx, ebx
                        xor eax, eax
                        mov ax, word ptr [v+2]
                        mov dx, word ptr [TextureRowPitchScratch]
                        imul dx
                        xor esi, esi
                        mov si, word ptr [u+2]
                        add esi, eax
                        add esi, dword ptr [TextureRasterScratch544]
                        mov edi, dword ptr [TextureRasterRowOffset]
                        add edi, dword ptr [x]
                        cmp dword ptr [TextureRasterSignedPitch], 0x0
                        jge K_be87
                        neg dword ptr [dv]
                        not dword ptr [v]
                    K_be87:
                        cmp dword ptr [u_dir], 0x0
                        jl K_bf04
                        mov ebx, dword ptr [u]
                        shl ebx, 0x10
                        mov bx, word ptr [v]
                        shr bx, 1
                        mov edx, dword ptr [du]
                        shl edx, 0x10
                        mov dx, word ptr [dv]
                        shr dx, 1
                        mov eax, dword ptr [du_whole]
                        add eax, dword ptr [dv_step]
                        mov dword ptr [TextureRasterScratch4B8], eax
                        push ebp
                        mov ebp, dword ptr [TextureRasterScratch4B8]
                        dec edi
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        shr eax, 0x8
                    K_bed1:
                        mov ah, byte ptr [esi]
                        inc edi
                        add ah, al
                        add ebx, edx
                        adc esi, ebp
                        mov byte ptr [edi], ah
                        test bh, 0x80
                        je K_beea
                        and bh, 0x7f
                        add esi, dword ptr [TextureRasterSignedPitch]
                    K_beea:
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        add ax, word ptr [TextureRasterLightPackA]
                        mov dword ptr [TextureRasterLightAccumB], eax
                        shr eax, 0x8
                        dec ecx
                        jne K_bed1
                        pop ebp
                        jmp row_done
                    K_bf04:
                        mov edx, dword ptr [du]
                        neg edx
                        shl edx, 0x10
                        mov dx, word ptr [dv]
                        shr dx, 1
                        mov ebx, dword ptr [u]
                        not ebx
                        shl ebx, 0x10
                        mov bx, word ptr [v]
                        shr dword ptr [v], 1
                        mov eax, dword ptr [du_whole]
                        add eax, dword ptr [dv_step]
                        mov dword ptr [TextureRasterScratch4B8], eax
                        neg dword ptr [TextureRasterScratch4B8]
                        push ebp
                        mov ebp, dword ptr [TextureRasterScratch4B8]
                        dec edi
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        shr eax, 0x8
                    K_bf4f:
                        mov ah, byte ptr [esi]
                        inc edi
                        add ah, al
                        add ebx, edx
                        sbb esi, ebp
                        mov byte ptr [edi], ah
                        test bh, 0x80
                        je K_bf68
                        and bh, 0x7f
                        add esi, dword ptr [TextureRasterSignedPitch]
                    K_bf68:
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        add ax, word ptr [TextureRasterLightPackA]
                        mov dword ptr [TextureRasterLightAccumB], eax
                        shr eax, 0x8
                        dec ecx
                        jne K_bf4f
                        pop ebp
                                row_done:
                                }
                            }
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    break;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    break;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        } while (TextureRasterRowOffset < TextureRasterClipBottom);
                    }
                }
            }
        }
    }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}

// ORIGINAL: 0x0061F790 sub_61f790 0x0061F790-0x0061F7FD FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x0061f790: sole stack argument supplies counter+4, endpoint+8 and the interpolant words; zero after decrement calls static setup with that same pointer", "src/texture.h: EdgeScan fields and 0x3C stride; pinned setup 0x00624D80/0x0061FC90 writes those exact words", "Original signed branch and 32-bit wrapping arithmetic retained; unsigned-to-int conversion is target-specific, not a portability claim"], "reason": "The free cdecl edge stepper consumes the same EdgeScan record as its static setup callback, not a Texture receiver or integer address."}
// symbol    ?sub_61f790@@YAHPAUEdgeScan@@@Z
// size      109 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00624D80

int __cdecl sub_61f790(EdgeScan *edge) {
    edge->counter = static_cast<int>(
        static_cast<unsigned int>(edge->counter) - 1u);
    if (edge->counter == 0) {
        return Texture::setup_edge(edge, edge->id) != 0;
    }

    edge->accumA = static_cast<int>(
        static_cast<unsigned int>(edge->accumA) +
        static_cast<unsigned int>(edge->deltaA));
    edge->accumB = static_cast<int>(
        static_cast<unsigned int>(edge->accumB) +
        static_cast<unsigned int>(edge->deltaB));
    edge->accumC = static_cast<int>(
        static_cast<unsigned int>(edge->accumC) +
        static_cast<unsigned int>(edge->deltaC));
    edge->fracAccum = static_cast<int>(
        static_cast<unsigned int>(edge->fracAccum) +
        static_cast<unsigned int>(edge->fracDelta));
    if (edge->fracAccum > 0) {
        edge->accumC = static_cast<int>(
            static_cast<unsigned int>(edge->accumC) +
            static_cast<unsigned int>(edge->extraDeltaC));
        edge->fracAccum = static_cast<int>(
            static_cast<unsigned int>(edge->fracAccum) -
            static_cast<unsigned int>(edge->fracSub));
    }
    return 1;
}

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

// ORIGINAL: 0x0061F800 sub_61f800 0x0061F800-0x0061FA24 FILE BYTE_EXACT
// symbol    ?sub_61f800@@YAXPAUEdgeScan@@0@Z
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

// The reconstructed C++ interface uses EdgeScan pointers because each pinned
// entry dereferences both argument addresses at the existing EdgeScan fields
// (+0x0c, +0x10, +0x1c, and +0x34 in Gouraud paths). Incoming producers and the
// original nominal parameter type remain unknown; this is not recovered
// original-prototype evidence.
void __cdecl sub_61f800(EdgeScan *left, EdgeScan *right) {
    int acc_y = left->accumA;
    int acc_x = left->accumB;
    int end_y = right->accumA;
    int end_x = right->accumB;
    int start_row = left->accumC;
    int end_row = right->accumC;
    int y_step;
    int y_sign;
    int y_whole;
    int x_step;
    int x_whole;
    // Hand-written assembly: and/jns sign tests, shrd fixed-point products, adc/sbb
    // carry stepping and EBP borrowed as the row-step register.
    __asm {
        push esi
        push edi
        mov ebx, end_row
        cmp ebx, TextureRasterClipLeft
        jle span_done
        mov ecx, start_row
        cmp ecx, TextureRasterClipRight
        jge span_done
        sub ebx, ecx
        jle span_done
        shl ebx, 0x10
        mov eax, end_y
        sub eax, acc_y
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov y_step, eax
        mov ecx, 1
        and eax, eax
        jns y_rounded
        neg ecx
        test eax, 0xffff
        je y_rounded
        add eax, 0x10000
    y_rounded:
        mov y_sign, ecx
        mov ecx, eax
        sar ecx, 0x10
        mov y_whole, ecx
        sar eax, 1
        add eax, acc_y
        mov acc_y, eax
        mov eax, end_x
        sub eax, acc_x
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov x_step, eax
        mov ecx, TextureRowPitchScratch
        and eax, eax
        jns x_rounded
        neg ecx
        test eax, 0xffff
        je x_rounded
        add eax, 0x10000
    x_rounded:
        mov TextureRasterSignedPitch, ecx
        mov ecx, eax
        sar eax, 0x10
        imul TextureRowPitchScratch
        mov x_whole, eax
        sar ecx, 1
        add ecx, acc_x
        mov acc_x, ecx
        mov ebx, end_row
        cmp ebx, TextureRasterClipRight
        jl right_clipped
        mov ebx, TextureRasterClipRight
    right_clipped:
        mov edi, start_row
        cmp edi, TextureRasterClipLeft
        jge left_clipped
        sub edi, TextureRasterClipLeft
        neg edi
        mov eax, y_step
        shl edi, 0x10
        imul edi
        shrd eax, edx, 0x10
        add eax, acc_y
        mov acc_y, eax
        mov eax, x_step
        imul edi
        shrd eax, edx, 0x10
        add eax, acc_x
        mov acc_x, eax
        mov edi, TextureRasterClipLeft
        mov start_row, edi
    left_clipped:
        sub ebx, edi
        mov ecx, ebx
        xor eax, eax
        mov ax, word ptr acc_x + 2
        mov dx, word ptr TextureRowPitchScratch
        imul dx
        xor esi, esi
        mov si, word ptr acc_y + 2
        add esi, eax
        add esi, TextureRasterScratch544
        mov eax, TextureRasterRowOffset
        imul TextureRasterBufferStride
        add eax, start_row
        add eax, TextureRasterPixelBase
        mov edi, eax
        cmp TextureRasterSignedPitch, 0
        jge pitch_ready
        neg x_step
        not acc_x
    pitch_ready:
        cmp y_sign, 0
        jl reverse_span
        mov ebx, acc_y
        shl ebx, 0x10
        mov bx, word ptr acc_x
        shr bx, 1
        mov edx, y_step
        shl edx, 0x10
        mov dx, word ptr x_step
        shr dx, 1
        mov eax, y_whole
        add eax, x_whole
        mov TextureRasterScratch4B8, eax
        push ebp
        mov ebp, TextureRasterScratch4B8
        dec edi
    forward_pixel:
        mov al, [esi]
        inc edi
        add ebx, edx
        adc esi, ebp
        mov [edi], al
        test bh, 0x80
        je forward_next
        and bh, 0x7f
        add esi, TextureRasterSignedPitch
    forward_next:
        dec ecx
        jne forward_pixel
        pop ebp
        jmp span_done
    reverse_span:
        mov edx, y_step
        neg edx
        shl edx, 0x10
        mov dx, word ptr x_step
        shr dx, 1
        mov ebx, acc_y
        not ebx
        shl ebx, 0x10
        mov bx, word ptr acc_x
        shr acc_x, 1
        mov eax, y_whole
        add eax, x_whole
        mov TextureRasterScratch4B8, eax
        neg TextureRasterScratch4B8
        push ebp
        mov ebp, TextureRasterScratch4B8
        dec edi
    reverse_pixel:
        mov al, [esi]
        inc edi
        add al, ah
        add ebx, edx
        sbb esi, ebp
        mov [edi], al
        test bh, 0x80
        je reverse_next
        and bh, 0x7f
        add esi, TextureRasterSignedPitch
    reverse_next:
        dec ecx
        jne reverse_pixel
        pop ebp
    span_done:
        pop edi
        pop esi
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
// ORIGINAL: 0x0061FA30 sub_61fa30 0x0061FA30-0x0061FC86 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; raw 61FA30 prologue, 61FC85 RET and alignment through 61FC8F, next mapped entry 61FC90.", "Raw entry loads two stack EdgeScan records at offsets 0C/10/1C and uses shared clip, source, destination, key and packed-step storage; original source ownership and reachability remain unknown.", "Full raw dataflow preserves wrapped shifted divisor, clipped multiply, low-word source-row product, initial lookup before signed-pitch NEG/NOT transformation, both packed U-direction loops and scratch publications. Exceptional IDIV and native fault timing remain restoration work."], "reason": "Distinct two-stack-edge keyed raster entry has an independent prologue and RET between the preceding completed span and the next Gouraud edge setup entry. Its offset-derived name and source home expose shared raster dependencies without claiming a Texture receiver, historical TU or original caller."}
// symbol    ?sub_61fa30@@YAXPAUEdgeScan@@0@Z
void __cdecl sub_61fa30(EdgeScan *left, EdgeScan *right) {
    int acc_y = left->accumA;
    int acc_x = left->accumB;
    int end_y = right->accumA;
    int end_x = right->accumB;
    int start_row = left->accumC;
    int end_row = right->accumC;
    int y_step;
    int y_sign;
    int y_whole;
    int x_step;
    int x_whole;
    // Hand-written assembly: and/jns sign tests, shrd fixed-point products, adc/sbb
    // carry stepping and EBP borrowed as the row-step register; skips the threshold colour.
    __asm {
        push esi
        push edi
        mov ebx, end_row
        cmp ebx, TextureRasterClipLeft
        jle span_done
        mov ecx, start_row
        cmp ecx, TextureRasterClipRight
        jge span_done
        sub ebx, ecx
        jle span_done
        shl ebx, 0x10
        mov eax, end_y
        sub eax, acc_y
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov y_step, eax
        mov ecx, 1
        and eax, eax
        jns y_rounded
        neg ecx
        test eax, 0xffff
        je y_rounded
        add eax, 0x10000
    y_rounded:
        mov y_sign, ecx
        mov ecx, eax
        sar ecx, 0x10
        mov y_whole, ecx
        sar eax, 1
        add eax, acc_y
        mov acc_y, eax
        mov eax, end_x
        sub eax, acc_x
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov x_step, eax
        mov ecx, TextureRowPitchScratch
        and eax, eax
        jns x_rounded
        neg ecx
        test eax, 0xffff
        je x_rounded
        add eax, 0x10000
    x_rounded:
        mov TextureRasterSignedPitch, ecx
        mov ecx, eax
        sar eax, 0x10
        imul TextureRowPitchScratch
        mov x_whole, eax
        sar ecx, 1
        add ecx, acc_x
        mov acc_x, ecx
        mov ebx, end_row
        cmp ebx, TextureRasterClipRight
        jl right_clipped
        mov ebx, TextureRasterClipRight
    right_clipped:
        mov edi, start_row
        cmp edi, TextureRasterClipLeft
        jge left_clipped
        sub edi, TextureRasterClipLeft
        neg edi
        mov eax, y_step
        shl edi, 0x10
        imul edi
        shrd eax, edx, 0x10
        add eax, acc_y
        mov acc_y, eax
        mov eax, x_step
        imul edi
        shrd eax, edx, 0x10
        add eax, acc_x
        mov acc_x, eax
        mov edi, TextureRasterClipLeft
        mov start_row, edi
    left_clipped:
        sub ebx, edi
        mov ecx, ebx
        add acc_x, 0x8000
        xor eax, eax
        mov ax, word ptr acc_x + 2
        mov dx, word ptr TextureRowPitchScratch
        imul dx
        add acc_y, 0x8000
        xor esi, esi
        mov si, word ptr acc_y + 2
        add esi, eax
        add esi, TextureRasterScratch544
        mov eax, TextureRasterRowOffset
        imul TextureRasterBufferStride
        add eax, start_row
        add eax, TextureRasterPixelBase
        mov edi, eax
        cmp TextureRasterSignedPitch, 0
        jge pitch_ready
        neg x_step
        not acc_x
    pitch_ready:
        cmp y_sign, 0
        jl reverse_span
        mov ebx, acc_y
        shl ebx, 0x10
        mov bx, word ptr acc_x
        shr bx, 1
        mov edx, y_step
        shl edx, 0x10
        mov dx, word ptr x_step
        shr dx, 1
        mov TextureRasterPackedStep, edx
        mov eax, y_whole
        add eax, x_whole
        mov TextureRasterScratch4B8, eax
        mov ah, byte ptr TextureRasterLightThreshold
        push ebp
        mov ebp, TextureRasterScratch4B8
    forward_pixel:
        mov al, [esi]
        cmp al, ah
        je forward_skip
        mov [edi], al
    forward_skip:
        add ebx, TextureRasterPackedStep
        adc esi, ebp
        test bh, 0x80
        je forward_next
        add esi, TextureRasterSignedPitch
        and bh, 0x7f
    forward_next:
        inc edi
        dec ecx
        jne forward_pixel
        pop ebp
        jmp span_done
    reverse_span:
        mov edx, y_step
        neg edx
        shl edx, 0x10
        mov dx, word ptr x_step
        shr dx, 1
        mov TextureRasterPackedStep, edx
        mov ebx, acc_y
        not ebx
        shl ebx, 0x10
        mov bx, word ptr acc_x
        shr acc_x, 1
        mov eax, y_whole
        add eax, x_whole
        mov TextureRasterScratch4B8, eax
        neg TextureRasterScratch4B8
        mov ah, byte ptr TextureRasterLightThreshold
        push ebp
        mov ebp, TextureRasterScratch4B8
    reverse_pixel:
        mov al, [esi]
        cmp al, ah
        je reverse_skip
        mov [edi], al
    reverse_skip:
        add ebx, TextureRasterPackedStep
        sbb esi, ebp
        test bh, 0x80
        je reverse_next
        add esi, TextureRasterSignedPitch
        and bh, 0x7f
    reverse_next:
        inc edi
        dec ecx
        jne reverse_pixel
        pop ebp
    span_done:
        pop edi
        pop esi
    }
}

// ORIGINAL: 0x0061CB50 sub_61cb50 0x0061CB50-0x0061D08F FILE BYTE_EXACT
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
    EdgeScan left;
    EdgeScan right;
    if (vertices != 0 && buffer != 0 && pixels_ != 0 && remap != 0 && shade != 0) {
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterScratch550 = remap;
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    if (use_eight_vertices) {
        TextureRasterVertexCount = 8;
        TextureRasterEdgeVerticesA = cornersAndMids_;
    } else {
        TextureRasterVertexCount = 4;
        TextureRasterEdgeVerticesA = corners_;
    }
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    TextureRasterScratch554 = shade->pixels_;
    if (TextureRasterScratch554 != 0) {
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int max_y = -0x7ffd;
        int min_y = 0x7fff;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        for (int i = 0; i < TextureRasterVertexCount; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge(&right, top) && borrowed_ != 0 &&
                    shade->borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dv;
                    int du;
                    if (span > 0) {
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(3)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_uv_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_uv_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmulp st(2), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(3)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: span with adc carry
                                // stepping; shade byte 0 skips, 1 copies the
                                // texel, others write the remap byte.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_cf63
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_cf63:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_cfaf
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_cfaf:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                shl ecx, 0x10
                                sub ecx, 0x10000
                            K_d014:
                                mov ah, byte ptr [ebx]
                                push edx
                                push ecx
                                mov ecx, dword ptr [TextureRasterScratch554]
                                mov cx, bx
                                xor edx, edx
                                mov dl, byte ptr [ecx]
                                add edx, dword ptr [TextureRasterScratch550]
                                mov dl, byte ptr [edx]
                                cmp dl, 0x0
                                je K_d03b
                                cmp dl, 0x1
                                je K_d039
                                mov ah, dl
                            K_d039:
                                mov byte ptr [edi], ah
                            K_d03b:
                                pop ecx
                                pop edx
                                add edx, esi
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                inc edi
                                sub ecx, 0x10000
                                jns K_d014
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                }
            }
        }
    }
    }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}

// ORIGINAL: 0x0061FDF0 sub_61fdf0 0x0061FDF0-0x0061FE68 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x0061fdf0: sole stack argument supplies counter+4, endpoint+8 and the interpolant words; zero after decrement calls static setup with that same pointer", "src/texture.h: EdgeScan fields and 0x3C stride; pinned setup 0x00624D80/0x0061FC90 writes those exact words", "Original signed branch and 32-bit wrapping arithmetic retained; unsigned-to-int conversion is target-specific, not a portability claim"], "reason": "The free cdecl edge stepper consumes the same EdgeScan record as its static setup callback, not a Texture receiver or integer address."}
// symbol    ?sub_61fdf0@@YAHPAUEdgeScan@@@Z
// TRIED: calling through the scaffold's `Texture::setup_edge_gouraud` member (thiscall, this-in-ECX) - the disasm shows two bare pushes and no ECX setup, so a free __cdecl decl was used instead; MISMATCH is at #20 (near the tail), not tried further.
// size      120 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0061FC90

int __cdecl sub_61fdf0(EdgeScan *edge) {
    edge->counter = static_cast<int>(
        static_cast<unsigned int>(edge->counter) - 1u);
    if (edge->counter == 0) {
        return Texture::setup_edge_gouraud(edge, edge->id) != 0;
    }

    edge->accumA = static_cast<int>(
        static_cast<unsigned int>(edge->accumA) +
        static_cast<unsigned int>(edge->deltaA));
    edge->accumB = static_cast<int>(
        static_cast<unsigned int>(edge->accumB) +
        static_cast<unsigned int>(edge->deltaB));
    edge->accumC = static_cast<int>(
        static_cast<unsigned int>(edge->accumC) +
        static_cast<unsigned int>(edge->deltaC));
    edge->accumD = static_cast<int>(
        static_cast<unsigned int>(edge->accumD) +
        static_cast<unsigned int>(edge->deltaD));
    edge->fracAccum = static_cast<int>(
        static_cast<unsigned int>(edge->fracAccum) +
        static_cast<unsigned int>(edge->fracDelta));
    if (edge->fracAccum > 0) {
        edge->accumC = static_cast<int>(
            static_cast<unsigned int>(edge->accumC) +
            static_cast<unsigned int>(edge->extraDeltaC));
        edge->fracAccum = static_cast<int>(
            static_cast<unsigned int>(edge->fracAccum) -
            static_cast<unsigned int>(edge->fracSub));
    }
    return 1;
}

// Fixed-slot bindings carried from 006202e0.cpp

// ORIGINAL: 0x006202E0 sub_6202e0 0x006202E0-0x0062063D FILE BYTE_EXACT
// symbol    ?sub_6202e0@@YAXPAUEdgeScan@@0@Z
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

void __cdecl sub_6202e0(EdgeScan *left, EdgeScan *right) {
    int acc_y = left->accumA;
    int acc_x = left->accumB;
    int end_y = right->accumA;
    int end_x = right->accumB;
    int start_row = left->accumC;
    int end_row = right->accumC;
    TextureRasterLightAccumB.dword = left->accumD;
    TextureRasterLightStepB = right->accumD;
    int y_step;
    int y_sign;
    int y_whole;
    int x_step;
    int x_whole;
    // Hand-written assembly: and/jns sign tests, shrd products, jae carry stepping, EBP as the row step.
    __asm {
        push esi
        push edi
        mov ebx, dword ptr [end_row]
        cmp ebx, dword ptr [TextureRasterClipLeft]
        jle L_310
        mov ecx, dword ptr [start_row]
        cmp ecx, dword ptr [TextureRasterClipRight]
        jge L_310
        sub ebx, ecx
        jle L_310
        shl ebx, 0x10
        mov eax, dword ptr [TextureRasterLightStepB]
        sub eax, dword ptr [TextureRasterLightAccumB]
        shl eax, 0x8
        idiv ebx
        mov word ptr [TextureRasterLightPackA], ax
        mov eax, dword ptr [end_y]
        sub eax, dword ptr [acc_y]
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov dword ptr [y_step], eax
        mov ecx, 0x1
        and eax, eax
        jns L_6b
        neg ecx
        test eax, 0xffff
        je L_6b
        add eax, 0x10000
    L_6b:
        mov dword ptr [y_sign], ecx
        mov ecx, eax
        sar ecx, 0x10
        mov dword ptr [y_whole], ecx
        sar eax, 1
        add eax, dword ptr [acc_y]
        mov dword ptr [acc_y], eax
        mov eax, dword ptr [end_x]
        sub eax, dword ptr [acc_x]
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov dword ptr [x_step], eax
        mov ecx, dword ptr [TextureRowPitchScratch]
        and eax, eax
        jns L_a9
        neg ecx
        test eax, 0xffff
        je L_a9
        add eax, 0x10000
    L_a9:
        mov dword ptr [TextureRasterSignedPitch], ecx
        mov ecx, eax
        sar eax, 0x10
        imul dword ptr [TextureRowPitchScratch]
        mov dword ptr [x_whole], eax
        sar ecx, 1
        add ecx, dword ptr [acc_x]
        mov dword ptr [acc_x], ecx
        mov ebx, dword ptr [end_row]
        cmp ebx, dword ptr [TextureRasterClipRight]
        jl L_d6
        mov ebx, dword ptr [TextureRasterClipRight]
    L_d6:
        mov edi, dword ptr [start_row]
        cmp edi, dword ptr [TextureRasterClipLeft]
        jge L_12c
        sub edi, dword ptr [TextureRasterClipLeft]
        neg edi
        mov eax, dword ptr [y_step]
        shl edi, 0x10
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [acc_y]
        mov dword ptr [acc_y], eax
        mov eax, dword ptr [x_step]
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [acc_x]
        mov dword ptr [acc_x], eax
        xor eax, eax
        mov ax, word ptr [TextureRasterLightPackA]
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [TextureRasterLightAccumB]
        mov dword ptr [TextureRasterLightAccumB], eax
        mov edi, dword ptr [TextureRasterClipLeft]
        mov dword ptr [start_row], edi
    L_12c:
        sub ebx, edi
        mov ecx, ebx
        xor eax, eax
        mov ax, word ptr [acc_x+2]
        mov dx, word ptr [TextureRowPitchScratch]
        imul dx
        xor esi, esi
        mov si, word ptr [acc_y+2]
        add esi, eax
        mov dword ptr [TextureRasterShadeCursor], esi
        add esi, dword ptr [TextureRasterScratch544]
        mov eax, dword ptr [TextureRasterShadeCursor]
        add eax, dword ptr [TextureRasterScratch554]
        mov dword ptr [TextureRasterShadeCursor], eax
        mov eax, dword ptr [TextureRasterRowOffset]
        imul dword ptr [TextureRasterBufferStride]
        add eax, dword ptr [start_row]
        add eax, dword ptr [TextureRasterPixelBase]
        mov edi, eax
        cmp dword ptr [TextureRasterSignedPitch], 0x0
        jge L_189
        neg dword ptr [x_step]
        not dword ptr [acc_x]
    L_189:
        cmp dword ptr [y_sign], 0x0
        jl L_25e
        mov ebx, dword ptr [acc_y]
        shl ebx, 0x10
        mov bx, word ptr [acc_x]
        shr bx, 1
        mov edx, dword ptr [y_step]
        shl edx, 0x10
        mov dx, word ptr [x_step]
        shr dx, 1
        mov eax, dword ptr [y_whole]
        add eax, dword ptr [x_whole]
        mov dword ptr [TextureRasterScratch4B8], eax
        push ebp
        mov ebp, dword ptr [TextureRasterScratch4B8]
        dec edi
        mov eax, dword ptr [TextureRasterLightAccumB]
        shr eax, 0x8
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterShadeCursor]
        movzx eax, byte ptr [eax]
        shl eax, 0x2
        add eax, dword ptr [TextureRasterScratch550]
        mov eax, dword ptr [eax]
        add eax, dword ptr [TextureRasterScratch4D8]
    L_1e6:
        mov ah, byte ptr [esi]
        inc edi
        add ah, al
        add ebx, edx
        jae L_1f6
        inc dword ptr [TextureRasterShadeCursor]
        inc esi
    L_1f6:
        add dword ptr [TextureRasterShadeCursor], ebp
        add esi, ebp
        mov byte ptr [edi], ah
        test bh, 0x80
        je L_223
        and bh, 0x7f
        add esi, dword ptr [TextureRasterSignedPitch]
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterSignedPitch]
        add dword ptr [TextureRasterShadeCursor], eax
        mov eax, dword ptr [TextureRasterScratch4D8]
    L_223:
        mov eax, dword ptr [TextureRasterLightAccumB]
        add ax, word ptr [TextureRasterLightPackA]
        mov dword ptr [TextureRasterLightAccumB], eax
        shr eax, 0x8
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterShadeCursor]
        movzx eax, byte ptr [eax]
        shl eax, 0x2
        add eax, dword ptr [TextureRasterScratch550]
        mov eax, dword ptr [eax]
        add eax, dword ptr [TextureRasterScratch4D8]
        dec ecx
        jne L_1e6
        pop ebp
        jmp L_310
    L_25e:
        mov edx, dword ptr [y_step]
        neg edx
        shl edx, 0x10
        mov dx, word ptr [x_step]
        shr dx, 1
        mov ebx, dword ptr [acc_y]
        not ebx
        shl ebx, 0x10
        mov bx, word ptr [acc_x]
        shr dword ptr [acc_x], 1
        mov eax, dword ptr [y_whole]
        add eax, dword ptr [x_whole]
        mov dword ptr [TextureRasterScratch4B8], eax
        neg dword ptr [TextureRasterScratch4B8]
        push ebp
        mov ebp, dword ptr [TextureRasterScratch4B8]
        dec edi
        mov eax, dword ptr [TextureRasterLightAccumB]
        shr eax, 0x8
    L_29d:
        mov ah, byte ptr [esi]
        inc edi
        add ah, al
        add ebx, edx
        jae L_2ad
        inc esi
        inc dword ptr [TextureRasterShadeCursor]
    L_2ad:
        sub esi, ebp
        sub dword ptr [TextureRasterShadeCursor], ebp
        mov byte ptr [edi], ah
        test bh, 0x80
        je L_2da
        and bh, 0x7f
        add esi, dword ptr [TextureRasterSignedPitch]
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterSignedPitch]
        add dword ptr [TextureRasterShadeCursor], eax
        mov eax, dword ptr [TextureRasterScratch4D8]
    L_2da:
        mov eax, dword ptr [TextureRasterLightAccumB]
        add ax, word ptr [TextureRasterLightPackA]
        mov dword ptr [TextureRasterLightAccumB], eax
        shr eax, 0x8
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterShadeCursor]
        movzx eax, byte ptr [eax]
        shl eax, 0x2
        add eax, dword ptr [TextureRasterScratch550]
        mov eax, dword ptr [eax]
        add eax, dword ptr [TextureRasterScratch4D8]
        dec ecx
        jne L_29d
        pop ebp
    L_310:
        pop edi
        pop esi
    }
}

// ORIGINAL: 0x0061FE70 sub_61fe70 0x0061FE70-0x006200FE FILE BYTE_EXACT
// symbol    ?sub_61fe70@@YAXPAUEdgeScan@@0@Z
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

void __cdecl sub_61fe70(EdgeScan *left, EdgeScan *right) {
    int acc_y = left->accumA;
    int acc_x = left->accumB;
    int start_row = left->accumC;
    TextureRasterLightAccumB.dword = left->accumD;
    int end_y = right->accumA;
    int end_x = right->accumB;
    int end_row = right->accumC;
    TextureRasterLightStepB = right->accumD;
    int y_step;
    int y_sign;
    int y_whole;
    int x_step;
    int x_whole;
    // Hand-written assembly: and/jns sign tests, shrd products, adc/sbb carry stepping, EBP as the row step.
    __asm {
        mov ebx, dword ptr [end_row]
        cmp ebx, dword ptr [TextureRasterClipLeft]
        jle span_done
        mov ecx, dword ptr [start_row]
        cmp ecx, dword ptr [TextureRasterClipRight]
        jge span_done
        sub ebx, ecx
        jle span_done
        shl ebx, 0x10
        mov eax, dword ptr [TextureRasterLightStepB]
        sub eax, dword ptr [TextureRasterLightAccumB]
        shl eax, 0x8
        idiv ebx
        mov word ptr [TextureRasterLightPackA], ax
        mov eax, dword ptr [end_y]
        sub eax, dword ptr [acc_y]
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov dword ptr [y_step], eax
        mov ecx, 0x1
        and eax, eax
        jns L_69
        neg ecx
        test eax, 0xffff
        je L_69
        add eax, 0x10000
    L_69:
        mov dword ptr [y_sign], ecx
        mov ecx, eax
        sar ecx, 0x10
        mov dword ptr [y_whole], ecx
        sar eax, 1
        add eax, dword ptr [acc_y]
        mov dword ptr [acc_y], eax
        mov eax, dword ptr [end_x]
        sub eax, dword ptr [acc_x]
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov dword ptr [x_step], eax
        mov ecx, dword ptr [TextureRowPitchScratch]
        and eax, eax
        jns L_a7
        neg ecx
        test eax, 0xffff
        je L_a7
        add eax, 0x10000
    L_a7:
        mov dword ptr [TextureRasterSignedPitch], ecx
        mov ecx, eax
        sar eax, 0x10
        imul dword ptr [TextureRowPitchScratch]
        mov dword ptr [x_whole], eax
        sar ecx, 1
        add ecx, dword ptr [acc_x]
        mov dword ptr [acc_x], ecx
        mov ebx, dword ptr [end_row]
        cmp ebx, dword ptr [TextureRasterClipRight]
        jl L_d4
        mov ebx, dword ptr [TextureRasterClipRight]
    L_d4:
        mov edi, dword ptr [start_row]
        cmp edi, dword ptr [TextureRasterClipLeft]
        jge L_12a
        sub edi, dword ptr [TextureRasterClipLeft]
        neg edi
        mov eax, dword ptr [y_step]
        shl edi, 0x10
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [acc_y]
        mov dword ptr [acc_y], eax
        mov eax, dword ptr [x_step]
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [acc_x]
        mov dword ptr [acc_x], eax
        xor eax, eax
        mov ax, word ptr [TextureRasterLightPackA]
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [TextureRasterLightAccumB]
        mov dword ptr [TextureRasterLightAccumB], eax
        mov edi, dword ptr [TextureRasterClipLeft]
        mov dword ptr [start_row], edi
    L_12a:
        sub ebx, edi
        mov ecx, ebx
        xor eax, eax
        mov ax, word ptr [acc_x+2]
        mov dx, word ptr [TextureRowPitchScratch]
        imul dx
        xor esi, esi
        mov si, word ptr [acc_y+2]
        add esi, eax
        add esi, dword ptr [TextureRasterScratch544]
        mov edi, dword ptr [TextureRasterRowOffset]
        add edi, dword ptr [start_row]
        cmp dword ptr [TextureRasterSignedPitch], 0x0
        jge L_164
        neg dword ptr [x_step]
        not dword ptr [acc_x]
    L_164:
        cmp dword ptr [y_sign], 0x0
        jl L_1d2
        mov ebx, dword ptr [acc_y]
        shl ebx, 0x10
        mov bx, word ptr [acc_x]
        shr bx, 1
        mov edx, dword ptr [y_step]
        shl edx, 0x10
        mov dx, word ptr [x_step]
        shr dx, 1
        mov eax, dword ptr [y_whole]
        add eax, dword ptr [x_whole]
        mov dword ptr [TextureRasterScratch4B8], eax
        push ebp
        mov ebp, dword ptr [TextureRasterScratch4B8]
        dec edi
        mov eax, dword ptr [TextureRasterLightAccumB]
        shr eax, 0x8
    L_19f:
        mov ah, byte ptr [esi]
        inc edi
        add ah, al
        add ebx, edx
        adc esi, ebp
        mov byte ptr [edi], ah
        test bh, 0x80
        je L_1b8
        and bh, 0x7f
        add esi, dword ptr [TextureRasterSignedPitch]
    L_1b8:
        mov eax, dword ptr [TextureRasterLightAccumB]
        add ax, word ptr [TextureRasterLightPackA]
        mov dword ptr [TextureRasterLightAccumB], eax
        shr eax, 0x8
        dec ecx
        jne L_19f
        pop ebp
        jmp span_done
    L_1d2:
        mov edx, dword ptr [y_step]
        neg edx
        shl edx, 0x10
        mov dx, word ptr [x_step]
        shr dx, 1
        mov ebx, dword ptr [acc_y]
        not ebx
        shl ebx, 0x10
        mov bx, word ptr [acc_x]
        shr dword ptr [acc_x], 1
        mov eax, dword ptr [y_whole]
        add eax, dword ptr [x_whole]
        mov dword ptr [TextureRasterScratch4B8], eax
        neg dword ptr [TextureRasterScratch4B8]
        push ebp
        mov ebp, dword ptr [TextureRasterScratch4B8]
        dec edi
        mov eax, dword ptr [TextureRasterLightAccumB]
        shr eax, 0x8
    L_211:
        mov ah, byte ptr [esi]
        inc edi
        add ah, al
        add ebx, edx
        sbb esi, ebp
        mov byte ptr [edi], ah
        test bh, 0x80
        je L_22a
        and bh, 0x7f
        add esi, dword ptr [TextureRasterSignedPitch]
    L_22a:
        mov eax, dword ptr [TextureRasterLightAccumB]
        add ax, word ptr [TextureRasterLightPackA]
        mov dword ptr [TextureRasterLightAccumB], eax
        shr eax, 0x8
        dec ecx
        jne L_211
        pop ebp
    span_done:
    }
}

// Fixed-slot bindings carried from 00620640.cpp

// ORIGINAL: 0x00620640 sub_620640 0x00620640-0x00620A10 FILE BYTE_EXACT
// symbol    ?sub_620640@@YAXPAUEdgeScan@@0@Z
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

void __cdecl sub_620640(EdgeScan *left, EdgeScan *right) {
    int acc_y = left->accumA;
    int acc_x = left->accumB;
    int end_y = right->accumA;
    int end_x = right->accumB;
    int start_row = left->accumC;
    int end_row = right->accumC;
    TextureRasterLightAccumB.dword = left->accumD;
    TextureRasterLightStepB = right->accumD;
    int y_step;
    int y_sign;
    int y_whole;
    int x_step;
    int x_whole;
    // Hand-written assembly: and/jns sign tests, shrd products, jae carry stepping, EBP as the row step.
    __asm {
        push esi
        push edi
        mov ebx, dword ptr [end_row]
        cmp ebx, dword ptr [TextureRasterClipLeft]
        jle L_383
        mov ecx, dword ptr [start_row]
        cmp ecx, dword ptr [TextureRasterClipRight]
        jge L_383
        sub ebx, ecx
        jle L_383
        shl ebx, 0x10
        mov eax, dword ptr [TextureRasterLightStepB]
        sub eax, dword ptr [TextureRasterLightAccumB]
        shl eax, 0x8
        idiv ebx
        mov word ptr [TextureRasterLightPackA], ax
        mov eax, dword ptr [end_y]
        sub eax, dword ptr [acc_y]
        shl eax, 0x8
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov dword ptr [y_step], eax
        mov ecx, 0x1
        and eax, eax
        jns L_6e
        neg ecx
        test eax, 0xffff
        je L_6e
        add eax, 0x10000
    L_6e:
        mov dword ptr [y_sign], ecx
        mov ecx, eax
        sar ecx, 0x10
        mov dword ptr [y_whole], ecx
        sar eax, 1
        add eax, dword ptr [acc_y]
        mov dword ptr [acc_y], eax
        mov eax, dword ptr [end_x]
        sub eax, dword ptr [acc_x]
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov dword ptr [x_step], eax
        mov ecx, dword ptr [TextureRowPitchScratch]
        and eax, eax
        jns L_ac
        neg ecx
        test eax, 0xffff
        je L_ac
        add eax, 0x10000
    L_ac:
        mov dword ptr [TextureRasterSignedPitch], ecx
        mov ecx, eax
        sar eax, 0x10
        imul dword ptr [TextureRowPitchScratch]
        mov dword ptr [x_whole], eax
        sar ecx, 1
        add ecx, dword ptr [acc_x]
        mov dword ptr [acc_x], ecx
        mov ebx, dword ptr [end_row]
        cmp ebx, dword ptr [TextureRasterClipRight]
        jl L_d9
        mov ebx, dword ptr [TextureRasterClipRight]
    L_d9:
        mov edi, dword ptr [start_row]
        cmp edi, dword ptr [TextureRasterClipLeft]
        jge L_12f
        sub edi, dword ptr [TextureRasterClipLeft]
        neg edi
        mov eax, dword ptr [y_step]
        shl edi, 0x10
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [acc_y]
        mov dword ptr [acc_y], eax
        mov eax, dword ptr [x_step]
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [acc_x]
        mov dword ptr [acc_x], eax
        xor eax, eax
        mov ax, word ptr [TextureRasterLightPackA]
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [TextureRasterLightAccumB]
        mov dword ptr [TextureRasterLightAccumB], eax
        mov edi, dword ptr [TextureRasterClipLeft]
        mov dword ptr [start_row], edi
    L_12f:
        sub ebx, edi
        mov ecx, ebx
        xor eax, eax
        mov ax, word ptr [acc_x+2]
        mov dx, word ptr [TextureRowPitchScratch]
        imul dx
        xor esi, esi
        mov si, word ptr [acc_y+2]
        add esi, eax
        mov dword ptr [TextureRasterShadeCursor], esi
        add esi, dword ptr [TextureRasterScratch544]
        mov eax, dword ptr [TextureRasterShadeCursor]
        add eax, dword ptr [TextureRasterScratch554]
        mov dword ptr [TextureRasterShadeCursor], eax
        mov eax, dword ptr [TextureRasterRowOffset]
        imul dword ptr [TextureRasterBufferStride]
        add eax, dword ptr [start_row]
        add eax, dword ptr [TextureRasterPixelBase]
        mov edi, eax
        cmp dword ptr [TextureRasterSignedPitch], 0x0
        jge L_18c
        neg dword ptr [x_step]
        not dword ptr [acc_x]
    L_18c:
        cmp dword ptr [y_sign], 0x0
        jl L_299
        mov ebx, dword ptr [acc_y]
        shl ebx, 0x10
        mov bx, word ptr [acc_x]
        shr bx, 1
        mov edx, dword ptr [y_step]
        shl edx, 0x10
        mov dx, word ptr [x_step]
        shr dx, 1
        mov eax, dword ptr [y_whole]
        add eax, dword ptr [x_whole]
        mov dword ptr [TextureRasterScratch4B8], eax
        push ebp
        mov ebp, dword ptr [TextureRasterScratch4B8]
        dec edi
        push ebx
        mov ebx, dword ptr [TextureRasterScratch55C]
        add bl, byte ptr [esi]
        jae L_1d4
        add ebx, 0x100
    L_1d4:
        mov ebx, dword ptr [ebx]
        xor eax, eax
        and ebx, ebx
        je L_1e4
        mov eax, dword ptr [TextureRasterLightAccumB]
        shr eax, 0x8
    L_1e4:
        pop ebx
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterShadeCursor]
        movzx eax, byte ptr [eax]
        shl eax, 0x2
        add eax, dword ptr [TextureRasterScratch550]
        mov eax, dword ptr [eax]
        add eax, dword ptr [TextureRasterScratch4D8]
    L_203:
        mov ah, byte ptr [esi]
        inc edi
        add ah, al
        add ebx, edx
        jae L_213
        inc dword ptr [TextureRasterShadeCursor]
        inc esi
    L_213:
        add dword ptr [TextureRasterShadeCursor], ebp
        add esi, ebp
        mov byte ptr [edi], ah
        test bh, 0x80
        je L_240
        and bh, 0x7f
        add esi, dword ptr [TextureRasterSignedPitch]
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterSignedPitch]
        add dword ptr [TextureRasterShadeCursor], eax
        mov eax, dword ptr [TextureRasterScratch4D8]
    L_240:
        push ebx
        mov ebx, dword ptr [TextureRasterScratch55C]
        add bl, byte ptr [esi]
        jae L_251
        add ebx, 0x100
    L_251:
        mov ebx, dword ptr [ebx]
        xor eax, eax
        and ebx, ebx
        je L_26d
        mov eax, dword ptr [TextureRasterLightAccumB]
        add ax, word ptr [TextureRasterLightPackA]
        mov dword ptr [TextureRasterLightAccumB], eax
        shr eax, 0x8
    L_26d:
        pop ebx
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterShadeCursor]
        movzx eax, byte ptr [eax]
        shl eax, 0x2
        add eax, dword ptr [TextureRasterScratch550]
        mov eax, dword ptr [eax]
        add eax, dword ptr [TextureRasterScratch4D8]
        dec ecx
        jne L_203
        pop ebp
        jmp L_383
    L_299:
        mov edx, dword ptr [y_step]
        neg edx
        shl edx, 0x10
        mov dx, word ptr [x_step]
        shr dx, 1
        mov ebx, dword ptr [acc_y]
        not ebx
        shl ebx, 0x10
        mov bx, word ptr [acc_x]
        shr dword ptr [acc_x], 1
        mov eax, dword ptr [y_whole]
        add eax, dword ptr [x_whole]
        mov dword ptr [TextureRasterScratch4B8], eax
        neg dword ptr [TextureRasterScratch4B8]
        push ebp
        mov ebp, dword ptr [TextureRasterScratch4B8]
        dec edi
        push ebx
        mov ebx, dword ptr [TextureRasterScratch55C]
        add bl, byte ptr [esi]
        jae L_2e1
        add ebx, 0x100
    L_2e1:
        mov ebx, dword ptr [ebx]
        xor eax, eax
        and ebx, ebx
        je L_2f1
        mov eax, dword ptr [TextureRasterLightAccumB]
        shr eax, 0x8
    L_2f1:
        pop ebx
    L_2f2:
        mov ah, byte ptr [esi]
        inc edi
        add ah, al
        add ebx, edx
        jae L_302
        inc esi
        inc dword ptr [TextureRasterShadeCursor]
    L_302:
        sub esi, ebp
        sub dword ptr [TextureRasterShadeCursor], ebp
        mov byte ptr [edi], ah
        test bh, 0x80
        je L_32f
        and bh, 0x7f
        add esi, dword ptr [TextureRasterSignedPitch]
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterSignedPitch]
        add dword ptr [TextureRasterShadeCursor], eax
        mov eax, dword ptr [TextureRasterScratch4D8]
    L_32f:
        push ebx
        mov ebx, dword ptr [TextureRasterScratch55C]
        add bl, byte ptr [esi]
        jae L_340
        add ebx, 0x100
    L_340:
        mov ebx, dword ptr [ebx]
        xor eax, eax
        and ebx, ebx
        je L_35c
        mov eax, dword ptr [TextureRasterLightAccumB]
        add ax, word ptr [TextureRasterLightPackA]
        mov dword ptr [TextureRasterLightAccumB], eax
        shr eax, 0x8
    L_35c:
        pop ebx
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterShadeCursor]
        movzx eax, byte ptr [eax]
        shl eax, 0x2
        add eax, dword ptr [TextureRasterScratch550]
        mov eax, dword ptr [eax]
        add eax, dword ptr [TextureRasterScratch4D8]
        dec ecx
        jne L_2f2
        pop ebp
    L_383:
        pop edi
        pop esi
    }
}

static void texture_keyed_shade_edge_step(EdgeScan &e) {
    e.accumA += e.deltaA;
    e.accumB += e.deltaB;
    e.accumC += e.deltaC;
    e.accumD += e.deltaD;
    e.fracAccum += e.fracDelta;
    if (e.fracAccum > 0) {
        e.accumC += e.extraDeltaC;
        e.fracAccum -= e.fracSub;
    }
}

// The keyed, shade-plane Gouraud span the 0x0061F0D0 rasterizer expands for
// each row. It has no out-of-line copy in the image; written `inline` so
// VC6 expands its assembly block.
inline void __cdecl texture_keyed_shade_span(EdgeScan *left, EdgeScan *right) {
    int acc_y = left->accumA;
    int acc_x = left->accumB;
    int end_y = right->accumA;
    int end_x = right->accumB;
    int start_row = left->accumC;
    int end_row = right->accumC;
    TextureRasterLightAccumB.dword = left->accumD;
    TextureRasterLightStepB = right->accumD;
    int y_step;
    int y_sign;
    int y_whole;
    int x_step;
    int x_whole;
    // Hand-written assembly: and/jns sign tests, shrd products, jae carry
    // stepping, add-bl/jae key lookup, EBP as the row step.
    __asm {
        push esi
        push edi
        mov ebx, dword ptr [end_row]
        cmp ebx, dword ptr [TextureRasterClipLeft]
        jle K_3a4
        mov ecx, dword ptr [start_row]
        cmp ecx, dword ptr [TextureRasterClipRight]
        jge K_3a4
        sub ebx, ecx
        jle K_3a4
        shl ebx, 0x10
        mov eax, dword ptr [TextureRasterLightStepB]
        sub eax, dword ptr [TextureRasterLightAccumB]
        shl eax, 0x8
        idiv ebx
        mov word ptr [TextureRasterLightPackA], ax
        mov eax, dword ptr [end_y]
        sub eax, dword ptr [acc_y]
        shl eax, 0x8
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov dword ptr [y_step], eax
        mov ecx, 0x1
        and eax, eax
        jns K_6e
        neg ecx
        test eax, 0xffff
        je K_6e
        add eax, 0x10000
    K_6e:
        mov dword ptr [y_sign], ecx
        mov ecx, eax
        sar ecx, 0x10
        mov dword ptr [y_whole], ecx
        sar eax, 1
        add eax, dword ptr [acc_y]
        mov dword ptr [acc_y], eax
        mov eax, dword ptr [end_x]
        sub eax, dword ptr [acc_x]
        mov edx, eax
        sar edx, 0x10
        shl eax, 0x10
        idiv ebx
        mov dword ptr [x_step], eax
        mov ecx, dword ptr [TextureRowPitchScratch]
        and eax, eax
        jns K_b2
        neg ecx
        test eax, 0xffff
        je K_b2
        add eax, 0x10000
    K_b2:
        mov dword ptr [TextureRasterSignedPitch], ecx
        mov ecx, eax
        sar eax, 0x10
        imul dword ptr [TextureRowPitchScratch]
        mov dword ptr [x_whole], eax
        sar ecx, 1
        add ecx, dword ptr [acc_x]
        mov dword ptr [acc_x], ecx
        mov ebx, dword ptr [end_row]
        cmp ebx, dword ptr [TextureRasterClipRight]
        jl K_e2
        mov ebx, dword ptr [TextureRasterClipRight]
    K_e2:
        mov edi, dword ptr [start_row]
        cmp edi, dword ptr [TextureRasterClipLeft]
        jge K_13b
        sub edi, dword ptr [TextureRasterClipLeft]
        neg edi
        mov eax, dword ptr [y_step]
        shl edi, 0x10
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [acc_y]
        mov dword ptr [acc_y], eax
        mov eax, dword ptr [x_step]
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [acc_x]
        mov dword ptr [acc_x], eax
        xor eax, eax
        mov ax, word ptr [TextureRasterLightPackA]
        imul edi
        shrd eax, edx, 0x10
        add eax, dword ptr [TextureRasterLightAccumB]
        mov dword ptr [TextureRasterLightAccumB], eax
        mov edi, dword ptr [TextureRasterClipLeft]
        mov dword ptr [start_row], edi
    K_13b:
        sub ebx, edi
        mov ecx, ebx
        xor eax, eax
        mov ax, word ptr [acc_x+2]
        mov dx, word ptr [TextureRowPitchScratch]
        imul dx
        xor esi, esi
        mov si, word ptr [acc_y+2]
        add esi, eax
        mov dword ptr [TextureRasterShadeCursor], esi
        add esi, dword ptr [TextureRasterScratch544]
        mov eax, dword ptr [TextureRasterShadeCursor]
        add eax, dword ptr [TextureRasterScratch554]
        mov dword ptr [TextureRasterShadeCursor], eax
        mov eax, dword ptr [TextureRasterRowOffset]
        imul dword ptr [TextureRasterBufferStride]
        add eax, dword ptr [start_row]
        add eax, dword ptr [TextureRasterPixelBase]
        mov edi, eax
        cmp dword ptr [TextureRasterSignedPitch], 0x0
        jge K_19b
        neg dword ptr [x_step]
        not dword ptr [acc_x]
    K_19b:
        cmp dword ptr [y_sign], 0x0
        jl K_2b1
        mov ebx, dword ptr [acc_y]
        shl ebx, 0x10
        mov bx, word ptr [acc_x]
        shr bx, 1
        mov edx, dword ptr [y_step]
        shl edx, 0x10
        mov dx, word ptr [x_step]
        shr dx, 1
        mov eax, dword ptr [y_whole]
        add eax, dword ptr [x_whole]
        mov dword ptr [TextureRasterScratch4B8], eax
        push ebp
        mov ebp, dword ptr [TextureRasterScratch4B8]
        dec edi
        push ebx
        mov ebx, dword ptr [TextureRasterScratch55C]
        add bl, byte ptr [esi]
        jae K_1ec
        add ebx, 0x100
    K_1ec:
        mov ebx, dword ptr [ebx]
        xor eax, eax
        and ebx, ebx
        je K_1fc
        mov eax, dword ptr [TextureRasterLightAccumB]
        shr eax, 0x8
    K_1fc:
        pop ebx
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterShadeCursor]
        movzx eax, byte ptr [eax]
        shl eax, 0x2
        add eax, dword ptr [TextureRasterScratch550]
        mov eax, dword ptr [eax]
        add eax, dword ptr [TextureRasterScratch4D8]
    K_21b:
        mov ah, byte ptr [esi]
        inc edi
        add ah, al
        add ebx, edx
        jae K_22b
        inc dword ptr [TextureRasterShadeCursor]
        inc esi
    K_22b:
        add dword ptr [TextureRasterShadeCursor], ebp
        add esi, ebp
        mov byte ptr [edi], ah
        test bh, 0x80
        je K_258
        and bh, 0x7f
        add esi, dword ptr [TextureRasterSignedPitch]
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterSignedPitch]
        add dword ptr [TextureRasterShadeCursor], eax
        mov eax, dword ptr [TextureRasterScratch4D8]
    K_258:
        push ebx
        mov ebx, dword ptr [TextureRasterScratch55C]
        add bl, byte ptr [esi]
        jae K_269
        add ebx, 0x100
    K_269:
        mov ebx, dword ptr [ebx]
        xor eax, eax
        and ebx, ebx
        je K_285
        mov eax, dword ptr [TextureRasterLightAccumB]
        add ax, word ptr [TextureRasterLightPackA]
        mov dword ptr [TextureRasterLightAccumB], eax
        shr eax, 0x8
    K_285:
        pop ebx
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterShadeCursor]
        movzx eax, byte ptr [eax]
        shl eax, 0x2
        add eax, dword ptr [TextureRasterScratch550]
        mov eax, dword ptr [eax]
        add eax, dword ptr [TextureRasterScratch4D8]
        dec ecx
        jne K_21b
        pop ebp
        jmp K_3a4
    K_2b1:
        mov edx, dword ptr [y_step]
        neg edx
        shl edx, 0x10
        mov dx, word ptr [x_step]
        shr dx, 1
        mov ebx, dword ptr [acc_y]
        not ebx
        shl ebx, 0x10
        mov bx, word ptr [acc_x]
        shr dword ptr [acc_x], 1
        mov eax, dword ptr [y_whole]
        add eax, dword ptr [x_whole]
        mov dword ptr [TextureRasterScratch4B8], eax
        neg dword ptr [TextureRasterScratch4B8]
        push ebp
        mov ebp, dword ptr [TextureRasterScratch4B8]
        dec edi
        push ebx
        mov ebx, dword ptr [TextureRasterScratch55C]
        add bl, byte ptr [esi]
        jae K_302
        add ebx, 0x100
    K_302:
        mov ebx, dword ptr [ebx]
        xor eax, eax
        and ebx, ebx
        je K_312
        mov eax, dword ptr [TextureRasterLightAccumB]
        shr eax, 0x8
    K_312:
        pop ebx
    K_313:
        mov ah, byte ptr [esi]
        inc edi
        add ah, al
        add ebx, edx
        jae K_323
        inc esi
        inc dword ptr [TextureRasterShadeCursor]
    K_323:
        sub esi, ebp
        sub dword ptr [TextureRasterShadeCursor], ebp
        mov byte ptr [edi], ah
        test bh, 0x80
        je K_350
        and bh, 0x7f
        add esi, dword ptr [TextureRasterSignedPitch]
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterSignedPitch]
        add dword ptr [TextureRasterShadeCursor], eax
        mov eax, dword ptr [TextureRasterScratch4D8]
    K_350:
        push ebx
        mov ebx, dword ptr [TextureRasterScratch55C]
        add bl, byte ptr [esi]
        jae K_361
        add ebx, 0x100
    K_361:
        mov ebx, dword ptr [ebx]
        xor eax, eax
        and ebx, ebx
        je K_37d
        mov eax, dword ptr [TextureRasterLightAccumB]
        add ax, word ptr [TextureRasterLightPackA]
        mov dword ptr [TextureRasterLightAccumB], eax
        shr eax, 0x8
    K_37d:
        pop ebx
        mov dword ptr [TextureRasterScratch4D8], eax
        mov eax, dword ptr [TextureRasterShadeCursor]
        movzx eax, byte ptr [eax]
        shl eax, 0x2
        add eax, dword ptr [TextureRasterScratch550]
        mov eax, dword ptr [eax]
        add eax, dword ptr [TextureRasterScratch4D8]
        dec ecx
        jne K_313
        pop ebp
    K_3a4:
        pop edi
        pop esi
    }
}

// ORIGINAL: 0x0061F0D0 sub_61f0d0 0x0061F0D0-0x0061F785 FILE
// RECOVERY_BINDING: v1 {"reason":"Recover separate vertex/light pointer inputs without changing the original argument-slot reuse edge case.","evidence":["Pinned61F0D0 publishes vertex/light arguments into9BB4F4/9BB4FC.61F216..61F23Ewalks y-coordinates at eight-byte stride;61F22Aoverwrites the original vertex argument slot with an index only on strict y<0x7FFF.","61F248reloads that slot for both edge setup calls. When no y passes the strict test, the original pointer bits remain the setup index; the typed source retains this residue explicitly instead of initializing index0.","The terminal61F785fragment belongs to this method and retains its identity/span/claim. Source-generated old signatures were hypotheses; original source spelling and unobserved upstream producers remain unknown."]}
// symbol    ?sub_61f0d0@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPBXPBIPBEH@Z
// TRIED: literal Ghidra-derived transcription (EdgeScan DDA struct, param_2 as Buffer*, CONCAT/CARRY helpers for the dither table lookups) compiles and matches the outer control flow, but MSVC's register allocation for the two 300-line unrolled scanline loops diverges from instruction #5 on; did not attempt hand-tuning further given the size.
// size      1717 bytes
// prototype 
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x0061FC90 0x00620100

// The pinned entry reads Texture's pixels/width/height at +0/+4/+8,
// selects the embedded UV arrays at +0x0C/+0x2C, and reads Buffer's
// current rectangle at +0x20..+0x2C. Use those existing members; the
// raster scratch cells below still retain their separately mapped storage.

// The retained nullsub_259 identity is the terminal ret 0x1C reached by fallthrough from Texture::sub_61f0d0's common epilogue. The source-owned relation below preserves its distinct span and historical claim without inventing a callable null stub or counting another recovered function. This does not exclude an interior-entry path; restoration must establish the complete parent extent as well as the fragment, rather than locating a convenient RET in the compiled body.
// ORIGINAL: 0x0061F785 nullsub_259 0x0061F785-0x0061F788 FILE BYTE_EXACT
// RECOVERY_FRAGMENT: v1 {"evidence":["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x0061F771-0x0061F788","source:src/texture.cpp:Texture::sub_61f0d0"],"parent_address":"0x0061F0D0","parent_symbol":"?sub_61f0d0@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPBXPBIPBEH@Z","reason":"terminal return reached by fallthrough from the mapped Texture method epilogue"}
// kind      game
void Texture::sub_61f0d0(Buffer *param_2, Vert *param_3, int *param_4,
                             const void *shade_source, const uint32_t *remap,
                             const unsigned char *lookup_bytes, int param_8) {
    if (param_3 != 0) {
        if (param_2 == 0) {
            return;
        }
        if (pixels_ != 0 && param_4 != 0 && remap != 0 && shade_source != 0
            && lookup_bytes != 0) {
            TextureRasterPixelBase = param_2->get_data();
            if (TextureRasterPixelBase != 0) {
                TextureRasterZValues = param_4;
                TextureRasterEdgeVerticesB = param_3;
                TextureRasterScratch550 = remap;
                TextureRasterScratch544 = pixels_;
                TextureRowPitchScratch = static_cast<int>(iWidth_);
                TextureRasterTextureHeight = static_cast<int>(iHeight_);
                if (param_8 != 0) {
                    TextureRasterVertexCount = 8;
                    TextureRasterEdgeVerticesA = cornersAndMids_;
                } else {
                    TextureRasterVertexCount = 4;
                    TextureRasterEdgeVerticesA = corners_;
                }
                TextureRasterBufferStride = param_2->stride_;
                TextureRasterTextureWidth = static_cast<int>(iWidth_);
                TextureRasterScratch55C = static_cast<int>(texture_pointer_address(lookup_bytes));
                TextureRasterScratch550 = remap;
                TextureRasterScratch554 = *static_cast<void *const *>(shade_source);
                if (TextureRasterScratch554 != 0) {
                    RECT *clip = &param_2->rect1_;
                    if (clip != 0) {
                        TextureRasterClipRight = clip->right;
                        TextureRasterClipLeft = clip->left;
                        TextureRasterClipBottom = clip->bottom;
                        TextureRasterClipTop = clip->top;
                        int max_y = -0x7ffd;
                        int min_y = 0x7fff;
                        int top;
                        for (int i = 0; i < TextureRasterVertexCount; ++i) {
                            if (param_3[i].y < min_y) {
                                min_y = param_3[i].y;
                                top = i;
                            }
                            if (param_3[i].y > max_y) {
                                // volatile: stand-in for the image's reload of param_3[i].y at 0x0061F231; no plain spelling found
                                max_y = *(volatile int *)&param_3[i].y;
                                TextureRasterMaxYIndex = i;
                            }
                        }
                        if (min_y < max_y) {
                            EdgeScan left;
                            EdgeScan right;
                            TextureRasterRowOffset = min_y;
                            left.f0 = -1;
                            if (Texture::setup_edge_gouraud(&left, top)) {
                                right.f0 = 1;
                                if (Texture::setup_edge_gouraud(&right, top)) {
                                    while (TextureRasterRowOffset < TextureRasterClipBottom) {
                                        if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                            texture_keyed_shade_span(&left, &right);
                                        }
                                        if (--left.counter == 0) {
                                            if (!sub_620100(&left, left.id)) {
                                                break;
                                            }
                                        } else {
                                            texture_keyed_shade_edge_step(left);
                                        }
                                        if (--right.counter == 0) {
                                            if (!sub_620100(&right, right.id)) {
                                                break;
                                            }
                                        } else {
                                            texture_keyed_shade_edge_step(right);
                                        }
                                        ++TextureRasterRowOffset;
                                    }
                                }
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
// Otherwise 0x0061A760's body; the span loop is the original hand-written
// assembly (ESP as the packed pixel counter), kept as __asm.
void Texture::draw_color_key(Buffer *buffer, Vert *vertices, int count, Vert *uv, int color) {
    if (color == -1) {
        draw_color_key(buffer, vertices, count, uv);
        return;
    }
    TextureRasterPixelBase = buffer->get_data();
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterScratch494 = static_cast<unsigned char>(color);
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterScratch544 = pixels_;
    TextureRowPitchScratch = static_cast<int>(iWidth_);
    TextureRasterEdgeVerticesA = uv;
    TextureRasterTextureHeight = static_cast<int>(iHeight_);
    TextureRasterVertexCount = count;
    TextureRasterLightThreshold = static_cast<int>(TextureRasterKey);
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int min_x = 0x7fff;
        int max_x = -0x7ffd;
        int top;
        EdgeScan left;
        EdgeScan right;
        Vert *v = vertices;
        for (int i = 0; i < count; ++i, ++v) {
            if (v->y < min_y) {
                min_y = v->y;
                top = i;
            }
            if (v->y > max_y) {
                // volatile: stand-in for the image's reload of v->y in the max branch; no plain spelling found
                int y = *(volatile int *)&v->y;
                TextureRasterMaxYIndex = i;
                max_y = y;
            }
            if (v->x < min_x) {
                min_x = v->x;
            }
            if (v->x > max_x) {
                // volatile: stand-in for the image's reload of v->x in the max branch; no plain spelling found
                max_x = *(volatile int *)&v->x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterDither = min_y & 1;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dv;
                    int du;
                    if (span > 0) {
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(3)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_uv_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_uv_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmulp st(2), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(3)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: color-keyed span with ESP as the packed counter; Dither parity picks texel copy or the fill color.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_right
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_right:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_left
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_left:
                                sub ebx, edi
                                mov ecx, ebx
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                xor esi, esi
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                test dword ptr [TextureRasterDither], 0x1
                                je K_3ebf
                                mov al, byte ptr [ebx + esi]
                            K_3e99:
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                cmp al, ah
                                je K_3ea7
                                mov byte ptr [edi], al
                            K_3ea7:
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_3e99
                                jmp K_3eec
                            K_3ebf:
                                mov cl, byte ptr [TextureRasterScratch494]
                                mov al, byte ptr [ebx + esi]
                            K_3ec8:
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                cmp al, ah
                                je K_3ed6
                                mov byte ptr [edi], cl
                            K_3ed6:
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_3ec8
                            K_3eec:
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                            ++TextureRasterDither;
                        }
                    }
                }
            }
        }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
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
// RECOVERY_BINDING: v1 {"reason":"Propagate the evidenced geometry/UV pointer contracts through the genuine shaded-raster sentinel wrapper.","evidence":["Pinned623F50forwards geometry and UV arguments unchanged to61C630only when its seventh argument equals-1.61C630's eight-byte coordinate scan and typed shared edge consumers establish the contracts.","The historical claim and full span are retained. Seed-generated int signatures are not original type evidence; no original-address forwarder or new matching claim is introduced."]}
// symbol    ?sub_623f50@Texture@@QAEXPAVBuffer@@PAUVert@@PAV1@PAEH1H@Z
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

void Texture::sub_623f50(Buffer *a1, Vert *a2, Texture *a3, uint8_t *a4,
                         int a5, Vert *a6, int a7) {
    // 0x00623F50, 45 bytes, `ret 0x1c`. A seven-argument wrapper whose seventh is
    // the same -1 sentinel `draw2` carries: when it is -1 the first six are
    // forwarded to the span filler at 0x0061C630, and otherwise the function
    // is a bare `ret 0x1c`. It reads nothing but its own arguments, so the
    // receiver it is called with is the one it passes on in ecx - the same
    // liveness 0x00624C70 shows.
    if (a7 == -1) {
        sub_61c630(a1, a2, a3, a4, a5, a6);
    }
}

// ORIGINAL: 0x00624CA0 sub_624ca0 0x00624CA0-0x00624CC8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Carry the affine rasterizer's evidenced Vert pointer contract through its genuine sentinel wrapper.","evidence":["Pinned 0x624CA0 forwards stack arguments two/four unchanged to 0x620A10 when argument six equals -1; otherwise it returns without invoking the rasterizer.","The callee's eight-byte coordinate accesses and publication into typed vertex-pointer globals establish the pointer interpretation. Historical source-generated integer signatures were hypotheses, not original debug symbols.","The historical matching obligation and complete original span are retained; current symbols follow the typed C++ declarations."]}
// symbol    ?sub_624ca0@Texture@@QAEXPAVBuffer@@PAUVert@@H1PAVTextureStore@@H@Z
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

void Texture::sub_624ca0(Buffer *a1, Vert *a2, int a3, Vert *a4, TextureStore *a5,
                         int a6) {
    // 0x00624CA0, 40 bytes, `ret 0x18`. The sixth argument is the -1 sentinel, the
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
// symbol    ?sub_624c70@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1PAVTextureStore@@H@Z
// size      45 bytes
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006212B0
Return Value: whatever sub_6212b0 leaves in EAX
Status: Complete
*/
void Texture::sub_624c70(Buffer *buf, Vert *verts, int *zvals, int count,
                         Vert *edgeVerts, TextureStore *cache, int sentinel) {
    if (sentinel == -1) {
        sub_6212b0(buf, verts, zvals, count, edgeVerts, cache);
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
// symbol    ?sub_624d00@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPAV1@PAEH1PAVTextureStore@@H@Z
// size      55 bytes
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00622200
Return Value: whatever sub_622200 leaves in EAX
Status: Complete
*/
void Texture::sub_624d00(Buffer *buffer, Vert *vertices, int *zvals,
                         Texture *shade_texture, unsigned char *remap,
                         int count, Vert *edge_vertices, TextureStore *cache,
                         int sentinel) {
    if (sentinel == -1) {
        sub_622200(buffer, vertices, zvals, shade_texture, remap, count,
                   edge_vertices, cache);
    }
}

/*
// ORIGINAL: 0x00624D40 sub_624d40 0x00624D40-0x00624D72 BYTE_EXACT
// symbol    ?sub_624d40@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1HPAVTextureStore@@H@Z
// size      50 bytes
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00622810
Return Value: whatever sub_622810 leaves in EAX
Status: Complete
*/
void Texture::sub_624d40(Buffer *buf, Vert *verts, int *zvals, int count,
                         Vert *edgeVerts, int light_thresh,
                         TextureStore *cache,
                         int sentinel) {
    if (sentinel == -1) {
        sub_622810(buf, verts, zvals, count, edgeVerts, light_thresh, cache);
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
// and FPU status inside release callbacks remain unproved. The original
// reads one texture byte before selecting fill versus lit rows, including on
// fill rows; lit rows then preload after each source advance, including after
// the final textured pixel. These volatile reads retain their fault/alias
// order even when a fill row never otherwise consumes the initial texel.
// The sentinel delegates to 0x0061AC60's shaded-every-row counterpart above.
// ORIGINAL: 0x00624670 Texture::draw2 0x00624670-0x00624C6D FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["624676..62469D identify seventh-argument minus-one forwarding; 6246B0 acquires the Buffer; 624BA8 reads the current texture byte before 624BAB selects lit versus fill; 624B99..624BE9 select row parity and perform light-tested sampling; 61FC90 supplies the Gouraud edge interpretation. Bounded geometry and ordered source-read obligations are documented beside the source body."], "reason": "This Texture member alternates opaque fill rows with cutoff-tested lit texture rows, consuming real Buffer, vertex, depth and UV arrays. Original callers ignore EAX; the ordinary path has no meaningful return value."}
// symbol    ?draw2@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH1HH@Z
void Texture::draw2(Buffer *buffer, Vert *vertices, int *zvals, int count,
                    Vert *uv, int cutoff, int fillColor) {
    if (fillColor == -1) {
        draw_below_cutoff(buffer, vertices, zvals, count, uv, cutoff);
        return;
    }
    TextureRasterPixelBase = buffer->get_data();
    TextureRasterLightThreshold = cutoff;
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterScratch494 = static_cast<unsigned char>(fillColor);
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterVertexCount = count;
    TextureRasterZValues = zvals;
    TextureRasterScratch544 = pixels_;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    int i;
    for (i = 0; i < count; ++i) {
        if (zvals[i] < cutoff) {
            break;
        }
    }
    if (i == count) {
        goto finish;
    }
    {
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int max_y = -0x7ffd;
        int min_y = 0x7fff;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (i = 0; i < count; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            TextureRasterDither = min_y & 1;
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (Texture::setup_edge_gouraud(&left, top)) {
                right.f0 = 1;
                if (Texture::setup_edge_gouraud(&right, top)) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: ESP-counter span; odd
                                // Dither rows draw lit texels below the cutoff,
                                // even rows write the fill color.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_4a89
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_4a89:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_4aee
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_4aee:
                                sub ebx, edi
                                mov ecx, ebx
                                mov esi, dword ptr [TextureRasterScratch544]
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator+2]
                                mov eax, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                add esi, eax
                                xor edx, edx
                                add dx, word ptr [TextureRasterUAccumulator+2]
                                add esi, edx
                                mov dword ptr [TextureRasterScratch504], esi
                                xor esi, esi
                                mov edi, dword ptr [TextureRasterSpanRight]
                                add edi, dword ptr [TextureRasterRowOffset]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                sar eax, 0x10
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                imul edx
                                mov edx, dword ptr [TextureRasterScratch4E4]
                                sar edx, 0x10
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowNext], eax
                                mov edx, dword ptr [TextureRasterTextureWidth]
                                add eax, edx
                                mov dword ptr [TextureRasterAlternateRowBase], eax
                                xor edx, edx
                                mov dx, word ptr [TextureRasterScratch4C8]
                                shl edx, 0x10
                                mov dword ptr [TextureRasterScratch528], edx
                                xor edx, edx
                                mov dx, word ptr [TextureRasterVAccumulator]
                                shl edx, 0x10
                                mov dx, word ptr [TextureRasterScratch4E4]
                                dec ecx
                                shl ecx, 0x10
                                mov cx, word ptr [TextureRasterUAccumulator]
                                mov ebx, dword ptr [TextureRasterScratch504]
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov dword ptr [TextureRasterSavedStack], esp
                                mov esp, ecx
                                mov al, byte ptr [ebx + esi]
                                test dword ptr [TextureRasterDither], 0x1
                                je K_4bed
                                mov cx, word ptr [TextureRasterScratch498]
                            K_4bbe:
                                add al, ch
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                cmp ch, ah
                                jge K_4bce
                                mov byte ptr [edi], al
                            K_4bce:
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                add cx, word ptr [TextureRasterLightPackA]
                                inc edi
                                mov al, byte ptr [ebx + esi]
                                sub esp, 0x10000
                                jns K_4bbe
                                jmp K_4c0f
                            K_4bed:
                                mov al, byte ptr [TextureRasterScratch494]
                            K_4bf2:
                                add edx, dword ptr [TextureRasterScratch528]
                                sbb ebp, ebp
                                mov byte ptr [edi], al
                                add sp, dx
                                adc esi, dword ptr [4*ebp + TextureRasterAlternateRowNext]
                                inc edi
                                sub esp, 0x10000
                                jns K_4bf2
                            K_4c0f:
                                mov esp, dword ptr [TextureRasterSavedStack]
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                            ++TextureRasterDither;
                        }
                    }
                }
            }
        }
    }
    }
finish:
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
}

// ORIGINAL: 0x00621870 ?sub_621870@Texture@@QAEHPAVBuffer@@PAUVert@@PAHHH2@Z 0x00621870-0x006221F1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Coordinator investigation_deferred 2026-09-19 (run-z7f3353q): sub_624cd0 trial bytes=unknown references=unknown with exactly one emitted reference ?sub_621870@Texture@@QAEHPAVBuffer@@PAUVert@@PAHHH2@Z at offset 38 binding_address null; sibling sub_624c70 passes because its callee sub_6212b0 is defined in src/texture.cpp:6515-6520 - the cause is the cross-TU callee, not the wrapper spelling; the masked measure (16 instructions, 4 masked bytes) proves every other byte correct.", "Seed src/unrecovered/00621870.cpp line 1 catalogues ORIGINAL 0x00621870 sub_621870 0x00621870-0x006221F1 FILE; the forwarder at src/pending_bodies.cpp:2225-2231 emits the member symbol via its C++ definition and the task record preserves the already-exact caller sub_624ca0 untouched."], "reason": "The pending_bodies.cpp forwarder at the annotated line is the only compiled definition of Texture::sub_621870 (its comment records that it is reached by name by Texture::sub_624cd0); the extent is the seed's catalogue adopted verbatim so the by-name reference resolves against the original identity without homing the x87 body."}
// Deleted seed provenance: git:b5a8554e33a26faf238f51538994e24a254783a3:src/unrecovered/00621870.cpp
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
void Texture::draw_cached_lit(Buffer *buffer, Vert *vertices, int *zvals,
                              int count, Vert *uv, TextureStore *cache) {
    int needs_clip = 1;
    cache->next_record_ = 0;
    TextureRasterPixelBase = buffer->get_data();
    TextureRasterLightThreshold = static_cast<int>(TextureRasterKey);
    // Hand-written assembly: the blitters reset the FPU with FINIT.
    __asm finit
    TextureRasterEdgeVerticesB = vertices;
    TextureRasterVertexCount = count;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterZValues = zvals;
    TextureRasterScratch544 = pixels_;
    TextureRasterBufferStride = static_cast<int>(buffer->stride_);
    TextureRasterTextureWidth = static_cast<int>(iWidth_);
    RECT *clip = &buffer->rect1_;
    if (clip != 0) {
        for (int i = 0; i < count; ++i) {
            zvals[i] = min(zvals[i], TextureLightCeiling);
            zvals[i] = max(zvals[i], TextureLightFloor);
        }
        TextureRasterClipRight = clip->right;
        TextureRasterClipLeft = clip->left;
        TextureRasterClipBottom = clip->bottom;
        TextureRasterClipTop = clip->top;
        int min_y = 0x7fff;
        int max_y = -0x7ffd;
        int max_x = -0x7ffd;
        int min_x = 0x7fff;
        int top;
        EdgeScan left;
        EdgeScan right;
        for (i = 0; i < TextureRasterVertexCount; ++i) {
            if (vertices[i].y < min_y) {
                min_y = vertices[i].y;
                top = i;
            }
            if (vertices[i].y > max_y) {
                max_y = vertices[i].y;
                TextureRasterMaxYIndex = i;
            }
            if (vertices[i].x < min_x) {
                min_x = vertices[i].x;
            }
            if (vertices[i].x > max_x) {
                max_x = vertices[i].x;
            }
        }
        if (min_y < max_y) {
            const int stride = TextureRasterBufferStride;
            const int base = static_cast<int>(texture_pointer_address(TextureRasterPixelBase));
            TextureRasterRowOffset = stride * min_y + base;
            if (min_x < TextureRasterClipLeft || min_y < TextureRasterClipTop ||
                max_x > TextureRasterClipRight || max_y > TextureRasterClipBottom) {
                needs_clip = 1;
            } else {
                needs_clip = 0;
            }
            TextureRasterClipBottom = TextureRasterClipBottom * stride + base;
            TextureRasterClipTop = TextureRasterClipTop * stride + base;
            left.f0 = -1;
            if (sub_625080(&left, top, cache)) {
                right.f0 = 1;
                if (sub_625080(&right, top, cache)) {
                if (borrowed_ != 0) {
                    unsigned short control;
                    // Hand-written assembly: truncating 24-bit FPU control
                    // word, then 1.0 kept on the FPU stack for the rows.
                    __asm {
                        fstcw control
                        mov ax, control
                        and ax, 0xfcff
                        or ax, 0xc00
                        mov control, ax
                        fldcw control
                        fld1
                    }
                    int span = (right.accumC - left.accumC) << 16;
                    int dd;
                    int dv;
                    int du;
                    if (span > 0) {
                        dd = right.accumD - left.accumD;
                        __asm fild dd
                        dv = right.accumB - left.accumB;
                        __asm fild dv
                        du = right.accumA - left.accumA;
                        __asm {
                            fild du
                            fild span
                            fdivr st, st(4)
                        }
                    }
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        int done = 0;
                        for (;;) {
                            TextureRasterUAccumulator.dword = left.accumA;
                            TextureRasterVAccumulator.dword = left.accumB;
                            TextureRasterSpanRight = left.accumC;
                            TextureRasterScratch498 = left.accumD;
                            TextureRasterScratch4C4 = right.accumA;
                            TextureRasterScratch530 = right.accumB;
                            TextureRasterSpanLeft = right.accumC;
                            if (--left.counter == 0) {
                                if (!sub_625080(&left, left.id, cache)) {
                                    done = 1;
                                    goto edges_done;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!sub_625080(&right, right.id, cache)) {
                                    done = 1;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                        edges_done:
                            if (span > 0) {
                                // Hand-written assembly: scale the row slopes
                                // and store them with FISTP.
                                __asm {
                                    fmul st(1), st
                                    fmul st(2), st
                                    fmulp st(3), st
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4E4.dword
                                    fmul TextureRasterScale65536
                                    fistp TextureRasterScratch4C8.dword
                                    fmul TextureRasterScale256
                                    fistp TextureRasterLightPackA.low_word
                                }
                            }
                            span = (right.accumC - left.accumC) << 16;
                            if (span > 0) {
                                dd = right.accumD - left.accumD;
                                __asm fild dd
                                dv = right.accumB - left.accumB;
                                __asm fild dv
                                du = right.accumA - left.accumA;
                                __asm {
                                    fild du
                                    fild span
                                    fdivr st, st(4)
                                }
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipBottom) {
                                break;
                            }
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                // Hand-written assembly: color-keyed lit span
                                // with adc carry stepping.
                                __asm {
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                mov ecx, dword ptr [TextureRasterSpanRight]
                                cmp ebx, dword ptr [TextureRasterClipLeft]
                                jle span_done
                                cmp ecx, dword ptr [TextureRasterClipRight]
                                jge span_done
                                sub ebx, ecx
                                jle span_done
                                shl ebx, 0x10
                                mov ebx, dword ptr [TextureRasterSpanLeft]
                                cmp ebx, dword ptr [TextureRasterClipRight]
                                jl K_170a
                                mov ebx, dword ptr [TextureRasterClipRight]
                            K_170a:
                                mov edi, dword ptr [TextureRasterSpanRight]
                                cmp edi, dword ptr [TextureRasterClipLeft]
                                jge K_176f
                                sub edi, dword ptr [TextureRasterClipLeft]
                                neg edi
                                mov eax, dword ptr [TextureRasterScratch4E4]
                                shl edi, 0x10
                                imul edi
                                shrd eax, edx, 0x10
                                add eax, dword ptr [TextureRasterUAccumulator]
                                mov dword ptr [TextureRasterUAccumulator], eax
                                mov eax, dword ptr [TextureRasterScratch4C8]
                                imul edi
                                shrd eax, edx, 0x10
                                add dword ptr [TextureRasterVAccumulator], eax
                                xor eax, eax
                                mov ax, word ptr [TextureRasterLightPackA]
                                imul edi
                                shrd eax, edx, 0x8
                                add eax, dword ptr [TextureRasterScratch498]
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edi, dword ptr [TextureRasterClipLeft]
                                mov dword ptr [TextureRasterSpanRight], edi
                            K_176f:
                                sub ebx, edi
                                mov ecx, ebx
                                mov ebx, dword ptr [TextureRasterScratch544]
                                mov bh, byte ptr [TextureRasterVAccumulator+2]
                                add bl, byte ptr [TextureRasterUAccumulator+2]
                                mov edi, dword ptr [TextureRasterRowOffset]
                                add edi, dword ptr [TextureRasterSpanRight]
                                mov eax, dword ptr [TextureRasterScratch498]
                                sar eax, 0x8
                                mov dword ptr [TextureRasterScratch498], eax
                                mov edx, dword ptr [TextureRasterUAccumulator]
                                shl edx, 0x10
                                mov dh, byte ptr [TextureRasterScratch4C8+2]
                                mov dl, byte ptr [TextureRasterScratch4E4+2]
                                mov esi, dword ptr [TextureRasterScratch4E4]
                                shl esi, 0x10
                                mov eax, ecx
                                mov ecx, dword ptr [TextureRasterScratch4C8]
                                shl ecx, 0x10
                                mov cx, ax
                                mov eax, dword ptr [TextureRasterVAccumulator]
                                shl eax, 0x10
                                mov ah, byte ptr [TextureRasterLightThreshold]
                                push ebp
                                mov ebp, ecx
                                xor bp, bp
                                shl ecx, 0x10
                                sub ecx, 0x10000
                                mov cx, word ptr [TextureRasterScratch498]
                                sub cx, word ptr [TextureRasterLightPackA]
                                dec edi
                            K_17f6:
                                add cx, word ptr [TextureRasterLightPackA]
                                mov al, byte ptr [ebx]
                                inc edi
                                add edx, esi
                                adc bl, dl
                                add eax, ebp
                                adc bh, dh
                                cmp al, ah
                                je K_1810
                                add al, ch
                                mov byte ptr [edi], al
                            K_1810:
                                sub ecx, 0x10000
                                jns K_17f6
                                pop ebp
                            span_done:
                                }
                            }
                            if (done) {
                                break;
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        }
                    }
                } else {
                    // Without a borrowed 256-pitch texture the rows step U/V
                    // in integer fixed point.
                    if (TextureRasterRowOffset < TextureRasterClipBottom) {
                        do {
                            if (TextureRasterRowOffset >= TextureRasterClipTop) {
                                int u = left.accumA;
                                int v = left.accumB;
                                int x = left.accumC;
                                TextureRasterLightAccumB.dword = left.accumD;
                                int ru = right.accumA;
                                int rv = right.accumB;
                                int rx = right.accumC;
                                TextureRasterLightStepB = right.accumD;
                                int du;
                                int dv;
                                int dv_step;
                                int u_dir;
                                int du_whole;
                                // Hand-written assembly: idiv slopes, signed
                                // texture pitch stepping and adc/sbb row carry
                                // in two mirrored span loops.
                                __asm {
                        mov ebx, dword ptr [rx]
                        cmp ebx, dword ptr [TextureRasterClipLeft]
                        jle row_done
                        mov ecx, dword ptr [x]
                        cmp ecx, dword ptr [TextureRasterClipRight]
                        jge row_done
                        sub ebx, ecx
                        jle row_done
                        shl ebx, 0x10
                        mov eax, dword ptr [TextureRasterLightStepB]
                        sub eax, dword ptr [TextureRasterLightAccumB]
                        shl eax, 0x8
                        idiv ebx
                        mov word ptr [TextureRasterLightPackA], ax
                        mov eax, dword ptr [ru]
                        sub eax, dword ptr [u]
                        mov edx, eax
                        sar edx, 0x10
                        shl eax, 0x10
                        idiv ebx
                        mov dword ptr [du], eax
                        mov ecx, 0x1
                        and eax, eax
                        jns K_bd77
                        neg ecx
                        test eax, 0xffff
                        je K_bd77
                        add eax, 0x10000
                    K_bd77:
                        mov dword ptr [u_dir], ecx
                        mov ecx, eax
                        sar ecx, 0x10
                        mov dword ptr [du_whole], ecx
                        sar eax, 1
                        add eax, dword ptr [u]
                        mov dword ptr [u], eax
                        mov eax, dword ptr [rv]
                        sub eax, dword ptr [v]
                        mov edx, eax
                        sar edx, 0x10
                        shl eax, 0x10
                        idiv ebx
                        mov dword ptr [dv], eax
                        mov ecx, dword ptr [TextureRowPitchScratch]
                        and eax, eax
                        jns K_bdbe
                        neg ecx
                        test eax, 0xffff
                        je K_bdbe
                        add eax, 0x10000
                    K_bdbe:
                        mov dword ptr [TextureRasterSignedPitch], ecx
                        mov ecx, eax
                        sar eax, 0x10
                        imul dword ptr [TextureRowPitchScratch]
                        mov dword ptr [dv_step], eax
                        sar ecx, 1
                        add ecx, dword ptr [v]
                        mov dword ptr [v], ecx
                        mov ebx, dword ptr [rx]
                        cmp ebx, dword ptr [TextureRasterClipRight]
                        jl K_bdee
                        mov ebx, dword ptr [TextureRasterClipRight]
                    K_bdee:
                        mov edi, dword ptr [x]
                        cmp edi, dword ptr [TextureRasterClipLeft]
                        jge K_be4a
                        sub edi, dword ptr [TextureRasterClipLeft]
                        neg edi
                        mov eax, dword ptr [du]
                        shl edi, 0x10
                        imul edi
                        shrd eax, edx, 0x10
                        add eax, dword ptr [u]
                        mov dword ptr [u], eax
                        mov eax, dword ptr [dv]
                        imul edi
                        shrd eax, edx, 0x10
                        add eax, dword ptr [v]
                        mov dword ptr [v], eax
                        xor eax, eax
                        mov ax, word ptr [TextureRasterLightPackA]
                        imul edi
                        shrd eax, edx, 0x10
                        add eax, dword ptr [TextureRasterLightAccumB]
                        mov dword ptr [TextureRasterLightAccumB], eax
                        mov edi, dword ptr [TextureRasterClipLeft]
                        mov dword ptr [x], edi
                    K_be4a:
                        sub ebx, edi
                        mov ecx, ebx
                        xor eax, eax
                        mov ax, word ptr [v+2]
                        mov dx, word ptr [TextureRowPitchScratch]
                        imul dx
                        xor esi, esi
                        mov si, word ptr [u+2]
                        add esi, eax
                        add esi, dword ptr [TextureRasterScratch544]
                        mov edi, dword ptr [TextureRasterRowOffset]
                        add edi, dword ptr [x]
                        cmp dword ptr [TextureRasterSignedPitch], 0x0
                        jge K_be87
                        neg dword ptr [dv]
                        not dword ptr [v]
                    K_be87:
                        cmp dword ptr [u_dir], 0x0
                        jl K_bf04
                        mov ebx, dword ptr [u]
                        shl ebx, 0x10
                        mov bx, word ptr [v]
                        shr bx, 1
                        mov edx, dword ptr [du]
                        shl edx, 0x10
                        mov dx, word ptr [dv]
                        shr dx, 1
                        mov eax, dword ptr [du_whole]
                        add eax, dword ptr [dv_step]
                        mov dword ptr [TextureRasterScratch4B8], eax
                        push ebp
                        mov ebp, dword ptr [TextureRasterScratch4B8]
                        dec edi
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        shr eax, 0x8
                    K_bed1:
                        mov ah, byte ptr [esi]
                        inc edi
                        add ah, al
                        add ebx, edx
                        adc esi, ebp
                        mov byte ptr [edi], ah
                        test bh, 0x80
                        je K_beea
                        and bh, 0x7f
                        add esi, dword ptr [TextureRasterSignedPitch]
                    K_beea:
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        add ax, word ptr [TextureRasterLightPackA]
                        mov dword ptr [TextureRasterLightAccumB], eax
                        shr eax, 0x8
                        dec ecx
                        jne K_bed1
                        pop ebp
                        jmp row_done
                    K_bf04:
                        mov edx, dword ptr [du]
                        neg edx
                        shl edx, 0x10
                        mov dx, word ptr [dv]
                        shr dx, 1
                        mov ebx, dword ptr [u]
                        not ebx
                        shl ebx, 0x10
                        mov bx, word ptr [v]
                        shr dword ptr [v], 1
                        mov eax, dword ptr [du_whole]
                        add eax, dword ptr [dv_step]
                        mov dword ptr [TextureRasterScratch4B8], eax
                        neg dword ptr [TextureRasterScratch4B8]
                        push ebp
                        mov ebp, dword ptr [TextureRasterScratch4B8]
                        dec edi
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        shr eax, 0x8
                    K_bf4f:
                        mov ah, byte ptr [esi]
                        inc edi
                        add ah, al
                        add ebx, edx
                        sbb esi, ebp
                        mov byte ptr [edi], ah
                        test bh, 0x80
                        je K_bf68
                        and bh, 0x7f
                        add esi, dword ptr [TextureRasterSignedPitch]
                    K_bf68:
                        mov eax, dword ptr [TextureRasterLightAccumB]
                        add ax, word ptr [TextureRasterLightPackA]
                        mov dword ptr [TextureRasterLightAccumB], eax
                        shr eax, 0x8
                        dec ecx
                        jne K_bf4f
                        pop ebp
                                row_done:
                                }
                            }
                            if (--left.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&left, left.id)) {
                                    break;
                                }
                            } else {
                                texture_lit_edge_step(left);
                            }
                            if (--right.counter == 0) {
                                if (!Texture::setup_edge_gouraud(&right, right.id)) {
                                    break;
                                }
                            } else {
                                texture_lit_edge_step(right);
                            }
                            TextureRasterRowOffset += TextureRasterBufferStride;
                        } while (TextureRasterRowOffset < TextureRasterClipBottom);
                    }
                }
                }
            }
        }
    }
    if (buffer != 0) {
        buffer->free_data(1);
    }
    __asm finit
    if (needs_clip == 0) {
        cache->flags_ = 0;
    }
}
