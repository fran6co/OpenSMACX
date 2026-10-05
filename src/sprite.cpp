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
#include "original_seam.h"
#include "sprite.h"
#include "strings.h" // source-owned shared filename buffer
#include "buffer.h"
#include "graphicwin.h"
#include "texture.h"
// Independent DWORD geometry outputs from load_interface (0x455610).
// At 455Bxx the original sums frame sprite bounds for the caption/border
// and subtracts bottom sprite bounds. popups_normal copies these into the
// corresponding Win defaults. Their initial image storage is zero; the
// recovered producer supplies measured frame geometry, not fallback values.
// RECOVERY_OBJECT: v1 0x0075ADE8 ?InterfaceCaptionHeight@@3HA
int InterfaceCaptionHeight;         // 0x0075ADE8
// RECOVERY_OBJECT: v1 0x00759E20 ?InterfaceBorderThickness@@3HA
int InterfaceBorderThickness;       // 0x00759E20
// RECOVERY_OBJECT: v1 0x00779644 ?InterfaceBottomBorderThickness@@3HA
int InterfaceBottomBorderThickness; // 0x00779644
// Other independent DWORD outputs published by 455B1F..455C55. These
// formulas are frame measurements; no containing allocation is asserted.
// RECOVERY_OBJECT: v1 0x0079A5E4 ?InterfaceAltCaptionHeight@@3HA
int InterfaceAltCaptionHeight;          // 0x0079A5E4
// RECOVERY_OBJECT: v1 0x007ACD2C ?InterfaceAltBorderThickness@@3HA
int InterfaceAltBorderThickness;        // 0x007ACD2C
// RECOVERY_OBJECT: v1 0x0076DC90 ?InterfaceAltBottomBorderThickness@@3HA
int InterfaceAltBottomBorderThickness;  // 0x0076DC90
// RECOVERY_OBJECT: v1 0x00791D60 ?InterfaceThirdCaptionHeight@@3HA
int InterfaceThirdCaptionHeight;        // 0x00791D60
// RECOVERY_OBJECT: v1 0x007ACF04 ?InterfaceThirdBottomBorderThickness@@3HA
int InterfaceThirdBottomBorderThickness;// 0x007ACF04
// RECOVERY_OBJECT: v1 0x0077970C ?InterfaceFourthBorderThickness@@3HA
int InterfaceFourthBorderThickness;     // 0x0077970C
// RECOVERY_OBJECT: v1 0x007ACE78 ?InterfaceFourthBottomBorderThickness@@3HA
int InterfaceFourthBottomBorderThickness;// 0x007ACE78
// RECOVERY_OBJECT: v1 0x0076DC8C ?InterfaceDefaultMetric@@3HA
int InterfaceDefaultMetric;             // 0x0076DC8C
// RECOVERY_OBJECT: v1 0x0078DD7C ?InterfaceTopLeftMetric@@3HA
int InterfaceTopLeftMetric;             // 0x0078DD7C
// RECOVERY_OBJECT: v1 0x0078A30C ?InterfaceMidRightMetric@@3HA
int InterfaceMidRightMetric;            // 0x0078A30C

#include <new>
#include "general.h"   // mem_get, for the placeholder pixel
#include "heap.h"      // Heap::get, behind TexHeap::get_mem below
#include "filemap.h"   // Filemap, behind Sprite::save/load below
#include "filewin.h"
#include "spritebox.h"
#include <stdlib.h>

/*
Purpose: Initialize an empty sprite and charge its own size to the sprite
         memory total.
// ORIGINAL: 0x005E37E0 ??0Sprite@@QAE@XZ 0x005E37E0-0x005E3815 BYTE_EXACT
// size      53 bytes
// prototype void (__thiscall ??0Sprite@@QAE@XZ)(Sprite* this)
// callers   154   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
Sprite::Sprite() {
    // The legacy body interleaves the accounting update between the first two
    // field stores and writes the type byte late, so the observable store
    // order is pinned rather than left to the compiler.
    // THE MEMBERS, still volatile: the store ORDER is what is pinned here,
    // not the punning. Each `&member` keeps the exact width and the exact
    // sequence the image writes, and the class stops walking itself.
    *reinterpret_cast<volatile uint32_t *>(&ppszFileName_) = 0;
    SpriteMemoryUsed = static_cast<int>(
        static_cast<uint32_t>(SpriteMemoryUsed) + sizeof(Sprite));
    *reinterpret_cast<volatile uint32_t *>(&pcBits_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iSpriteWidth2_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iSpriteWidth_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iSpriteHeight_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iWidth_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iHeight_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iLeftOffset_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iTopOffset_) = 0;
    // THE MEMBER, kept volatile: 0x08 is cTransparentIndex_, and the
    // volatile is what holds this store between the two `ordered[]` writes
    // either side of it rather than letting VC6 sink it.
    *reinterpret_cast<volatile uint8_t *>(&cTransparentIndex_) = 9;
    *reinterpret_cast<volatile uint32_t *>(&fObj1Exists_) = 0;
}


/*
Purpose: Release a sprite's allocations, discount its pixel memory, and clear
         every field except the type byte.
// ORIGINAL: 0x005E3820 ?close@Sprite@@QAEXXZ 0x005E3820-0x005E3884 BYTE_EXACT
// size      100 bytes
// prototype void (__thiscall ?close@Sprite@@QAEXXZ)(Sprite* this)
// callers   111   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Sprite::close() {
    // REAL MEMBERS, NOT A `volatile uint32_t *` ALIAS. Reading the object
    // through a volatile handle blocks VC6's memory-operand folding: the image
    // emits `cmp dword ptr [esi+0x28], edi` and `imul eax, dword ptr [esi+0x10]`
    // where an aliased read forces `mov` into a register first, which was four
    // extra instructions across this body.
    //
    // The pixel buffer is only owned, accounted, and released while the
    // borrowed flag at 0x28 is clear.
    if (fObj1Exists_ == 0 && pcBits_ != 0) {
        SpriteMemoryUsed = static_cast<int>(
            static_cast<uint32_t>(SpriteMemoryUsed)
            - static_cast<uint32_t>(iSpriteHeight_)
                  * static_cast<uint32_t>(iSpriteWidth_));
        // A SECOND TEST OF THE SAME POINTER, which the image really does: it
        // re-tests `[esi+4]` at 0x005E3845 before the free, even though the
        // outer condition already proved it non-zero, and the `je` target is
        // the zero-store - so that store sits OUTSIDE the inner guard.
        if (pcBits_ != 0) {
            free(pcBits_);
        }
        pcBits_ = 0;
    }
    if (ppszFileName_ != 0) {
        free(ppszFileName_);
        ppszFileName_ = 0;
    }
    // The type byte at 0x08 is deliberately preserved.
    iSpriteWidth2_ = 0;
    iSpriteWidth_ = 0;
    iSpriteHeight_ = 0;
    iWidth_ = 0;
    iHeight_ = 0;
    iLeftOffset_ = 0;
    iTopOffset_ = 0;
    fObj1Exists_ = 0;
}


// ORIGINAL: 0x005EAD9F ?change_color@Sprite@@QAEHHH@Z 0x005EAD9F-0x005EAE4E
// RECOVERY_BINDING: v1 {"evidence": ["Original5EADA5 tests the pixel pointer; null returns7.5EADBC multiplies height by stride, not source or display width.5EADFA skips all work when the four-byte group count is zero, including non-null one-to-three-byte buffers; preserve that asymmetry rather than the seed full-scan loop.", "For valid nonnegative allocated sprite sizes, original unrolled byte comparisons and tail replace only matches after masking both colors to8bits. No transparency/metadata change or service call occurs; borrowed pixels are modified too. Malformed dimensions/overflowed allocation domains are not established as safe. Genuine C++ loop remains unclaimed; no assembly or copied machine code."], "reason": "Recover the sprite pixel recoloring service using the genuine owned/borrowed pixel pointer and stride."}
int Sprite::change_color(int old_color, int new_color) {
    if (pcBits_ == 0) {
        return 7;
    }
    const unsigned int size = static_cast<unsigned int>(iSpriteHeight_)
                           * static_cast<unsigned int>(iSpriteWidth2_);
    // Original 5EADFA skips the tail as well when there is no four-byte
    // group: buffers of one, two or three pixels are deliberately untouched.
    if (size < 4) {
        return 0;
    }
    const unsigned char from = static_cast<unsigned char>(old_color);
    const unsigned char to = static_cast<unsigned char>(new_color);
    unsigned char *pixels = reinterpret_cast<unsigned char *>(pcBits_);
    for (unsigned int index = 0; index < size; ++index) {
        if (pixels[index] == from) {
            pixels[index] = to;
        }
    }
    return 0;
}

// ORIGINAL: 0x005EAE4F ?copy@Sprite@@QAEHPAUSprite@@@Z 0x005EAE4F-0x005EAF34
// RECOVERY_BINDING: v1 {"evidence": ["5EAE57 checks source bits before touching destination.5EAE6C..5EAEC4 performs the same owned/borrowed cleanup and field clearing as Sprite::close, preserving the transparent key.5EAEC7 captures wrapped32bit stride*height only after cleanup; allocation pointer publishes before null failure4.", "5EAEEA rereads source bits after allocation;5EAEF5..5EAF29 reads metadata after memcpy, writes dimensions/offsets/key, clears borrowed status and copies stride last. Original has no pixel-allocation increment to SpriteMemoryUsed and no self/null destination guard. Valid pixel allocations and ordinary C++ memcpy domains remain required; no behavioral repair is invented."], "reason": "Recover source.copy(destination) through Sprite ownership and close semantics, retaining original allocation failure and metadata observation order."}
// symbol    ?copy@Sprite@@QAEHPAV1@@Z
int Sprite::copy(Sprite *destination) {
    if (pcBits_ == 0) {
        return 7;
    }
    // Original 5EAE6C..5EAEC4 inlines this cleanup. In particular it does
    // not free borrowed pixels or clear the destination's transparent key.
    destination->close();
    const unsigned int size = static_cast<unsigned int>(iSpriteHeight_)
                           * static_cast<unsigned int>(iSpriteWidth2_);
    char *pixels = static_cast<char *>(mem_get(size));
    destination->pcBits_ = pixels;
    if (pixels == 0) {
        return 4;
    }
    memcpy(pixels, pcBits_, size);
    // These are fresh source reads AFTER copying, not a saved descriptor.
    // The original does not charge this allocation to SpriteMemoryUsed.
    destination->iSpriteWidth_ = iSpriteWidth_;
    destination->iSpriteHeight_ = iSpriteHeight_;
    destination->iWidth_ = iWidth_;
    destination->iHeight_ = iHeight_;
    destination->iLeftOffset_ = iLeftOffset_;
    destination->iTopOffset_ = iTopOffset_;
    destination->cTransparentIndex_ = cTransparentIndex_;
    destination->fObj1Exists_ = 0;
    destination->iSpriteWidth2_ = iSpriteWidth2_;
    return 0;
}

// OBJECTS, NOT POINTERS: the image addresses 0x00696D18 and 0x00696D1C
// directly - `mov esi, dword ptr [0x696d18]` - where a pointer variable
// loads the pointer first.
// RECOVERY_OBJECT: v2 0x00696D18 0x00696D1C ?SpriteScaleNumerator@@3HA 0x005EB20F
int SpriteScaleNumerator = 1;  // 0x00696D18
// RECOVERY_OBJECT: v2 0x00696D1C 0x00696D20 ?SpriteScaleDenominator@@3HA 0x005EB20F
int SpriteScaleDenominator = 1;  // 0x00696D1C

// Blitters update the cache only after fill_scaling_table returns. Equal
// numerator/denominator bypasses this cache and uses the unscaled path.
// RECOVERY_OBJECT: v2 0x00696D20 0x00696D24 ?SpriteCachedScaleNumerator@@3HA 0x005E3E50
int SpriteCachedScaleNumerator = -1;
// RECOVERY_OBJECT: v2 0x00696D24 0x00696D28 ?SpriteCachedScaleDenominator@@3HA 0x005E3E50
int SpriteCachedScaleDenominator = -1;
// RECOVERY_OBJECT: v2 0x009B6624 0x009B6628 ?SpriteScalingRatio@@3MA 0x005E3E50
float SpriteScalingRatio;
// RECOVERY_OBJECT: v2 0x009B661C 0x009B6620 ?SpriteMinimumScaledWidth@@3HA 0x005E3E50
int SpriteMinimumScaledWidth;
// RECOVERY_OBJECT: v2 0x009B6620 0x009B6624 ?SpriteMinimumScaledHeight@@3HA 0x005E3E50
int SpriteMinimumScaledHeight;

// The fill ends at 0x9B4758; that address is a separate blitter scalar,
// not another table. The clipping buffers end at independently used colour
// and pointer storage (0x9B53E0 and 0x9B606C), respectively.
// RECOVERY_OBJECT: v2 0x009B3AD8 0x009B4758 ?SpriteScalingSteps@@3PAHA 0x005EB20F
int SpriteScalingSteps[SpriteScalingTableCount];
// RECOVERY_OBJECT: v2 0x009B4760 0x009B53E0 ?SpriteClippedYSteps@@3PAHA 0x005EABBF
int SpriteClippedYSteps[SpriteScalingTableCount];
// RECOVERY_OBJECT: v2 0x009B53EC 0x009B606C ?SpriteClippedXSteps@@3PAHA 0x005EAB0F
int SpriteClippedXSteps[SpriteScalingTableCount];

// ORIGINAL: 0x005EB20F ?fill_scaling_table@Sprite@@QAEXXZ 0x005EB20F-0x005EB294
// RECOVERY_BINDING: v1 {"evidence": ["Pinned5EB20F..5EB294 divides larger by smaller scale operands, biases error by quotient parity, and fills9B3AD8 until9B4758. Integer error operations wrap32bits before signed branches; cache publication belongs to callers.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover the shared800-entry scale-step table generator with its original quotient-parity phase."}
void Sprite::fill_scaling_table() {
    const int larger = SpriteScaleNumerator > SpriteScaleDenominator
        ? SpriteScaleNumerator : SpriteScaleDenominator;
    const int smaller = SpriteScaleNumerator < SpriteScaleDenominator
        ? SpriteScaleNumerator : SpriteScaleDenominator;
    int quotient, twice_remainder, twice_divisor, error;
    if (smaller > 1) {
        quotient = larger / smaller;
        twice_remainder = static_cast<int>(static_cast<uint32_t>(larger % smaller) * 2u);
        twice_divisor = static_cast<int>(static_cast<uint32_t>(smaller) * 2u);
        error = static_cast<int>(static_cast<uint32_t>(twice_remainder >> 1) -
                                 static_cast<uint32_t>(twice_divisor));
    } else {
        quotient = larger;
        twice_remainder = twice_divisor = error = 0;
    }
    // Original integer error accumulation wraps at 32 bits before its signed
    // branch. Positive scale operands alone do not guarantee no overflow.
    if (quotient & 1)
        error = static_cast<int>(static_cast<uint32_t>(error) + static_cast<uint32_t>(larger));
    for (int i = 0; i < SpriteScalingTableCount; ++i) {
        if (smaller == 1) {
            SpriteScalingSteps[i] = quotient;
        } else {
            error = static_cast<int>(static_cast<uint32_t>(error) + static_cast<uint32_t>(twice_remainder));
            SpriteScalingSteps[i] = quotient;
            if (error > 0) {
                error = static_cast<int>(static_cast<uint32_t>(error) - static_cast<uint32_t>(twice_divisor));
                SpriteScalingSteps[i] = static_cast<int>(static_cast<uint32_t>(quotient) + 1u);
            }
        }
    }
}

// Intersected, nonempty rectangles and accesses within the 800-step bank are
// caller preconditions. The original helpers do not clamp oversized requests.
// Expansion maps each input pixel/row to a run of output pixels/rows; the
// clipped head and tail may shorten those runs. Output stores retain order
// even when a caller supplies aliased output pointers.
static int *sprite_clip_expand(int skip, int extent, int *source_skip,
    int *destination_skip, int *source_count, int *destination_count, int *table) {
    if (extent == 0) return 0;
    *destination_count = extent;
    *destination_skip = skip;
    int index = 0;
    while (skip > 0) {
        skip -= SpriteScalingSteps[index];
        ++index;
    }
    const int *input = SpriteScalingSteps + index;
    int count;
    int *output;
    if (skip < 0) {
        const int remaining = -skip;
        extent -= remaining;
        --index;
        table[0] = remaining;
        *source_skip = index;
        output = table + 1;
        count = 1;
    } else {
        *source_skip = index;
        output = table;
        count = 0;
    }
    while (extent > 0) {
        const int step = *input++;
        *output++ = step;
        extent -= step;
        ++count;
    }
    if (extent < 0) output[-1] += extent;
    *source_count = count;
    return table;
}

// Shrink entries are source advances per output pixel/row. Unlike expansion,
// the first and third output arguments are in destination coordinates.
static int *sprite_clip_shrink(int skip, int extent, int *destination_skip,
    int *source_skip, int *destination_count, int *source_count, int *table) {
    if (extent == 0) return 0;
    *destination_skip = skip;
    *destination_count = extent;
    *source_skip = 0;
    int index = 0;
    while (index < skip) {
        *source_skip += SpriteScalingSteps[index];
        ++index;
    }
    *source_count = 0;
    for (int i = 0; i < extent; ++i) {
        const int step = SpriteScalingSteps[index + i];
        table[i] = step;
        *source_count += step;
    }
    return table;
}

// ORIGINAL: 0x005EAB0F ?get_clipped_x_table_expand@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z 0x005EAB0F-0x005EABB7
// RECOVERY_BINDING: v1 {"evidence": ["Original returns9B53EC and caller5E5244 consumes EAX as an int-table pointer. It clips partial first/last runs and reports source and destination extents, retaining ordered alias-visible output stores.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover expansion clipping as a typed pointer to the actual X run table."}
// symbol    ?get_clipped_x_table_expand@Sprite@@QAEPAHPAUtagRECT@@0PAH111@Z
int *Sprite::get_clipped_x_table_expand(RECT *full, RECT *clipped, int *source_skip,
    int *destination_skip, int *source_count, int *destination_count) {
    return sprite_clip_expand(clipped->left - full->left, clipped->right - clipped->left,
        source_skip, destination_skip, source_count, destination_count, SpriteClippedXSteps);
}

// ORIGINAL: 0x005EABBF ?get_clipped_y_table_expand@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z 0x005EABBF-0x005EAC69
// RECOVERY_BINDING: v1 {"evidence": ["Original returns9B4760 and caller5E5285 consumes EAX as an int-table pointer. Top/bottom coordinate counterpart of5EAB0F with its own scratch table.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover expansion clipping as a typed pointer to the independent Y run table."}
// symbol    ?get_clipped_y_table_expand@Sprite@@QAEPAHPAUtagRECT@@0PAH111@Z
int *Sprite::get_clipped_y_table_expand(RECT *full, RECT *clipped, int *source_skip,
    int *destination_skip, int *source_count, int *destination_count) {
    return sprite_clip_expand(clipped->top - full->top, clipped->bottom - clipped->top,
        source_skip, destination_skip, source_count, destination_count, SpriteClippedYSteps);
}

// ORIGINAL: 0x005EAC6F ?get_clipped_x_table_shrink@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z 0x005EAC6F-0x005EACFB
// RECOVERY_BINDING: v1 {"evidence": ["Original computes source prefix skip and total source advances from9B3AD8; returned9B53EC is consumed as a pointer by5E503C. Output roles differ from expansion and retain alias-visible writes.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover shrinking X clipping using actual source-advance entries."}
// symbol    ?get_clipped_x_table_shrink@Sprite@@QAEPAHPAUtagRECT@@0PAH111@Z
int *Sprite::get_clipped_x_table_shrink(RECT *full, RECT *clipped, int *destination_skip,
    int *source_skip, int *destination_count, int *source_count) {
    return sprite_clip_shrink(clipped->left - full->left, clipped->right - clipped->left,
        destination_skip, source_skip, destination_count, source_count, SpriteClippedXSteps);
}

// ORIGINAL: 0x005EACFF ?get_clipped_y_table_shrink@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z 0x005EACFF-0x005EAD8D
// RECOVERY_BINDING: v1 {"evidence": ["Original top/bottom counterpart of5EAC6F returns9B4760. Callers read one int entry per row but pixel kernels explicitly consume its low byte.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover shrinking Y clipping with an independent source-advance table."}
// symbol    ?get_clipped_y_table_shrink@Sprite@@QAEPAHPAUtagRECT@@0PAH111@Z
int *Sprite::get_clipped_y_table_shrink(RECT *full, RECT *clipped, int *destination_skip,
    int *source_skip, int *destination_count, int *source_count) {
    return sprite_clip_shrink(clipped->top - full->top, clipped->bottom - clipped->top,
        destination_skip, source_skip, destination_count, source_count, SpriteClippedYSteps);
}

// The blitters own per-draw cursors and pitches locally; only scale decisions
// and their reusable lookup tables are shared. Pixel loops preserve source
// order, including overlapping source/destination and expansion replication.
// ORIGINAL: 0x005E3E00 ?draw_mono@Sprite@@QAEHPAUBuffer@@HHHHHH@Z 0x005E3E00-0x005E3E50 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned5E3E00 saves696D18/1C, substitutes final two parameters, calls5E3E50 with destination/key/x/y/color, restores denominator then numerator and returns result. Retain the claimed same-span seed and its restoration obligation; this binding is not a new claim.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover the genuine mono scaling overload rather than changing global layout or copying an opaque seed class."}
// symbol    ?draw_mono@Sprite@@QAEHPAVBuffer@@HHHHHH@Z
int Sprite::draw_mono(Buffer *destination, int transparent, int x, int y,
                      int color, int numerator, int denominator) {
    const int saved_numerator = SpriteScaleNumerator;
    const int saved_denominator = SpriteScaleDenominator;
    SpriteScaleNumerator = numerator;
    SpriteScaleDenominator = denominator;
    const int result = draw_mono(destination, transparent, x, y, color);
    SpriteScaleDenominator = saved_denominator;
    SpriteScaleNumerator = saved_numerator;
    return result;
}

// ORIGINAL: 0x005E3E50 ?draw_mono@Sprite@@QAEHPAUBuffer@@HHHH@Z 0x005E3E50-0x005E4ADA
// RECOVERY_BINDING: v1 {"evidence": ["Pinned3210-byte worker loads file-backed sprites through9B6080; software paths cover1:1, shrinking and expansion. Initial unscaled top skip uses logical width while subsequent rows use pitch. Scaled origins truncate coordinate+offset*ratio, not scaled coordinates. Preserve final free_data and source-byte/run ordering; local loop scratch replaces assembler scratch, not shared scale owners.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover the monochrome sprite blitter through actual Sprite storage, Buffer locking and scale tables."}
// symbol    ?draw_mono@Sprite@@QAEHPAVBuffer@@HHHH@Z
int Sprite::draw_mono(Buffer *destination, int transparent, int x, int y,
                      int color) {
    if (ppszFileName_ != 0) {
        const int loaded = g_BUFFER.load_pcx(ppszFileName_, 0, 10, 236);
        if (loaded != 0) return loaded;
        const int result = g_BUFFER.draw_mono(destination, transparent, x, y,
            color, SpriteScaleNumerator, SpriteScaleDenominator);
        g_BUFFER.close();
        return result;
    }
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0)
        return 3;

    if (SpriteScaleNumerator != SpriteScaleDenominator &&
        (SpriteCachedScaleNumerator != SpriteScaleNumerator ||
         SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        // FILD/FIDIV followed by the one float32 publication at5E3F2F.
        SpriteScalingRatio = static_cast<float>(
            static_cast<double>(SpriteScaleNumerator) / SpriteScaleDenominator);
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }

    const unsigned char fill = static_cast<unsigned char>(color);
    const unsigned char key = static_cast<unsigned char>(transparent);
    const int source_pitch = iSpriteWidth2_;
    int destination_pitch = static_cast<int>(destination->stride_);
    const unsigned char *const scaled_source =
        reinterpret_cast<const unsigned char *>(pcBits_);

    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        RECT original;
        original.top = y + iTopOffset_;
        original.left = x + iLeftOffset_;
        original.right = original.left + iSpriteWidth_;
        original.bottom = original.top + iSpriteHeight_;
        RECT clipped = original;
        if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        // Deliberate original asymmetry: the initial clipped top skip uses
        // logical width (+10), while subsequent rows advance by pitch (+C).
        const unsigned char *source_row =
            reinterpret_cast<const unsigned char *>(pcBits_) +
            (clipped.left - original.left) +
            (clipped.top - original.top) * iSpriteWidth_;
        unsigned char *destination_row =
            static_cast<unsigned char *>(destination->locked_bits_) +
            clipped.left + clipped.top * destination_pitch;
        const int columns = clipped.right - clipped.left;
        const int rows = clipped.bottom - clipped.top;
        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                if (source_row[column] != key)
                    destination_row[column] = fill;
            }
            if (row + 1 < rows) {
                source_row += source_pitch;
                destination_row += destination_pitch;
            }
        }
        destination->free_data(1);
        return 0;
    }

    // The integer argument coordinates are not themselves scaled. Original
    // x87 keeps the multiply/add live through _ftol; avoid float32 products.
    const int scaled_height = static_cast<int>(
        static_cast<double>(iSpriteHeight_) * SpriteScalingRatio);
    const int scaled_width = static_cast<int>(
        static_cast<double>(iSpriteWidth_) * SpriteScalingRatio);
    const int top = static_cast<int>(
        static_cast<double>(y) + static_cast<double>(iTopOffset_) * SpriteScalingRatio);
    const int left = static_cast<int>(
        static_cast<double>(x) + static_cast<double>(iLeftOffset_) * SpriteScalingRatio);
    RECT original = {left, top, left + scaled_width, top + scaled_height};
    RECT clipped = original;
    if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
        destination->free_data(1);
        return 0;
    }
    // The first clip is an early rejection only. The second call below
    // really recomputes from the original bounds; it must not be removed.
    unsigned char *const destination_bits =
        static_cast<unsigned char *>(destination->locked_bits_);
    const int base_offset = original.left + original.top * destination_pitch;
    destination_pitch = static_cast<int>(destination->stride_);
    int source_x, source_y, destination_x, destination_y;
    int source_columns, source_rows, destination_columns, destination_rows;

    if (SpriteScaleNumerator < SpriteScaleDenominator) {
        if (original.right - original.left < SpriteMinimumScaledWidth ||
            original.bottom - original.top < SpriteMinimumScaledHeight) {
            destination->free_data(1);
            return 0;
        }
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_shrink(&original, &clipped,
            &destination_x, &source_x, &destination_columns, &source_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_shrink(&original, &clipped,
            &destination_y, &source_y, &destination_rows, &source_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < destination_rows; ++row) {
            const unsigned char *source = source_row;
            for (int column = 0; column < destination_columns; ++column) {
                if (*source != key) destination_row[column] = fill;
                if (column + 1 < destination_columns) source += columns[column];
            }
            // MOV DL,[row_table]; ADD row_table,4 at5E4477..447C.
            // Nonzero low-byte row skips are the finite original domain.
            if (row + 1 < destination_rows) {
                source_row += source_pitch * static_cast<unsigned char>(rows[row]);
                destination_row += destination_pitch;
            }
        }
    } else {
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_expand(&original, &clipped,
            &source_x, &destination_x, &source_columns, &destination_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_expand(&original, &clipped,
            &source_y, &destination_y, &source_rows, &destination_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < source_rows; ++row) {
            const unsigned char selector = static_cast<unsigned char>(rows[row]);
            const int repeats = selector == 1 ? 1 : selector == 2 ? 2 : selector == 3 ? 3 : 4;
            int output_column = 0;
            for (int column = 0; column < source_columns; ++column) {
                if (source_row[column] != key) {
                    for (int repeat = 0; repeat < repeats; ++repeat)
                        for (int pixel = 0; pixel < columns[column]; ++pixel)
                            destination_row[repeat * destination_pitch + output_column + pixel] = fill;
                }
                output_column += columns[column];
            }
            if (row + 1 < source_rows) {
                source_row += source_pitch;
                destination_row += repeats * destination_pitch;
            }
        }
    }
    destination->free_data(1);
    return 0;
}

// ORIGINAL: 0x005E4B9A ?draw@Sprite@@QAEHPAUBuffer@@HHH@Z 0x005E4B9A-0x005E5833
// RECOVERY_BINDING: v1 {"evidence": ["Pinned worker loads file-backed sprites through Buffer9B6080, otherwise uses real destination lock/clip/pitch and the scale-table helpers. Initial top clipping uses pitch. Each expanded source byte is sampled once and replicated in the original per-source-pixel row order.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Replace the original-address Sprite draw forwarder with genuine clipped transparent pixel copying."}
// symbol    ?draw@Sprite@@QAEHPAVBuffer@@HHH@Z
int Sprite::draw(Buffer *destination, int transparent, int x, int y) {
    if (ppszFileName_ != 0) {
        const int loaded = g_BUFFER.load_pcx(ppszFileName_, 0, 10, 236);
        if (loaded != 0) return loaded;
        const int result = g_BUFFER.draw(destination, transparent, x, y,
            SpriteScaleNumerator, SpriteScaleDenominator);
        g_BUFFER.close();
        return result;
    }
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0)
        return 3;

    if (SpriteScaleNumerator != SpriteScaleDenominator &&
        (SpriteCachedScaleNumerator != SpriteScaleNumerator ||
         SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        // FILD/FIDIV followed by the one float32 publication at5E3F2F.
        SpriteScalingRatio = static_cast<float>(
            static_cast<double>(SpriteScaleNumerator) / SpriteScaleDenominator);
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }

    const unsigned char key = static_cast<unsigned char>(transparent);
    const int source_pitch = iSpriteWidth2_;
    int destination_pitch = static_cast<int>(destination->stride_);
    const unsigned char *const scaled_source =
        reinterpret_cast<const unsigned char *>(pcBits_);

    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        RECT original;
        original.top = y + iTopOffset_;
        original.left = x + iLeftOffset_;
        original.right = original.left + iSpriteWidth_;
        original.bottom = original.top + iSpriteHeight_;
        RECT clipped = original;
        if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        // The plain-color worker uses source pitch for the initial top clip.
        const unsigned char *source_row =
            reinterpret_cast<const unsigned char *>(pcBits_) +
            (clipped.left - original.left) +
            (clipped.top - original.top) * source_pitch;
        unsigned char *destination_row =
            static_cast<unsigned char *>(destination->locked_bits_) +
            clipped.left + clipped.top * destination_pitch;
        const int columns = clipped.right - clipped.left;
        const int rows = clipped.bottom - clipped.top;
        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                if (source_row[column] != key)
                    destination_row[column] = source_row[column];
            }
            if (row + 1 < rows) {
                source_row += source_pitch;
                destination_row += destination_pitch;
            }
        }
        destination->free_data(1);
        return 0;
    }

    // The integer argument coordinates are not themselves scaled. Original
    // x87 keeps the multiply/add live through _ftol; avoid float32 products.
    const int scaled_height = static_cast<int>(
        static_cast<double>(iSpriteHeight_) * SpriteScalingRatio);
    const int scaled_width = static_cast<int>(
        static_cast<double>(iSpriteWidth_) * SpriteScalingRatio);
    const int top = static_cast<int>(
        static_cast<double>(y) + static_cast<double>(iTopOffset_) * SpriteScalingRatio);
    const int left = static_cast<int>(
        static_cast<double>(x) + static_cast<double>(iLeftOffset_) * SpriteScalingRatio);
    RECT original = {left, top, left + scaled_width, top + scaled_height};
    RECT clipped = original;
    if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
        destination->free_data(1);
        return 0;
    }
    // The first clip is an early rejection only. The second call below
    // really recomputes from the original bounds; it must not be removed.
    unsigned char *const destination_bits =
        static_cast<unsigned char *>(destination->locked_bits_);
    const int base_offset = original.left + original.top * destination_pitch;
    destination_pitch = static_cast<int>(destination->stride_);
    int source_x, source_y, destination_x, destination_y;
    int source_columns, source_rows, destination_columns, destination_rows;

    if (SpriteScaleNumerator < SpriteScaleDenominator) {
        if (original.right - original.left < SpriteMinimumScaledWidth ||
            original.bottom - original.top < SpriteMinimumScaledHeight) {
            destination->free_data(1);
            return 0;
        }
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_shrink(&original, &clipped,
            &destination_x, &source_x, &destination_columns, &source_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_shrink(&original, &clipped,
            &destination_y, &source_y, &destination_rows, &source_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < destination_rows; ++row) {
            const unsigned char *source = source_row;
            for (int column = 0; column < destination_columns; ++column) {
                if (*source != key) destination_row[column] = *source;
                if (column + 1 < destination_columns) source += columns[column];
            }
            // MOV DL,[row_table]; ADD row_table,4 at5E4477..447C.
            // Nonzero low-byte row skips are the finite original domain.
            if (row + 1 < destination_rows) {
                source_row += source_pitch * static_cast<unsigned char>(rows[row]);
                destination_row += destination_pitch;
            }
        }
    } else {
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_expand(&original, &clipped,
            &source_x, &destination_x, &source_columns, &destination_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_expand(&original, &clipped,
            &source_y, &destination_y, &source_rows, &destination_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < source_rows; ++row) {
            const unsigned char selector = static_cast<unsigned char>(rows[row]);
            const int repeats = selector == 1 ? 1 : selector == 2 ? 2 : selector == 3 ? 3 : 4;
            int output_column = 0;
            for (int column = 0; column < source_columns; ++column) {
                const unsigned char pixel_value = source_row[column];
                if (pixel_value != key) {
                    for (int repeat = 0; repeat < repeats; ++repeat)
                        for (int pixel = 0; pixel < columns[column]; ++pixel)
                            destination_row[repeat * destination_pitch + output_column + pixel] = pixel_value;
                }
                output_column += columns[column];
            }
            if (row + 1 < source_rows) {
                source_row += source_pitch;
                destination_row += repeats * destination_pitch;
            }
        }
    }
    destination->free_data(1);
    return 0;
}

// ORIGINAL: 0x005E9D44 ?draw@Sprite@@QAEHPAUBuffer@@HHHPAE@Z 0x005E9D44-0x005EAAFF
// RECOVERY_BINDING: v1 {"evidence": ["Terrain loader calls4522AF/45238B select this five-argument worker. Unlike plain Sprite draw it has no filename-load branch. Unit/shrink compare the low-byte key then map every other source value, including zero; expansion also skips zero.", "Expansion consumes the low byte of each Y selector:1/2/3 repeats that many rows, all other values repeat4. Four-row lookups5EA54D/5EA5A0/5EA5F3/5EA646 alone read table[first destination pixel], while other modes read table[source]. Map once per source pixel before row-major replication, preserving live source/destination/table overlap.", "Unit clipping reloads pcBits afterward with the earlier pitch. Scaled clipping retains preclip source bits and post-first-clip destination bits; original-position base uses the old pitch, traversal the reloaded pitch. Retain shared float32 ratio/cache and genuine helper owners.", "This structured kernel assumes valid buffers/tables, positive bounded scaling and ordinary fitting geometry. Zero low-byte shrink steps have a pathological original wrap loop. Extreme x87 cancellation, altered FP environment and asynchronous aliases of historical assembly scratch are not established by the portable spelling. No original bug fix is authorized. Preserve the full seed in src/unrecovered/005e9d44.cpp."], "reason": "Recover the complete source-remap Sprite worker used by terrain-sheet conversion, using genuine members, clipped-table helpers and typed Buffer access."}
// symbol    ?draw@Sprite@@QAEHPAVBuffer@@HHHPAE@Z
int Sprite::draw(Buffer *destination, int transparent, int x, int y,
                 unsigned char *table) {
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0)
        return 3;

    if (SpriteScaleNumerator != SpriteScaleDenominator &&
        (SpriteCachedScaleNumerator != SpriteScaleNumerator ||
         SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        SpriteScalingRatio = static_cast<float>(
            static_cast<double>(SpriteScaleNumerator) / SpriteScaleDenominator);
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }

    const unsigned char key = static_cast<unsigned char>(transparent);
    const int source_pitch = iSpriteWidth2_;
    int destination_pitch = static_cast<int>(destination->stride_);
    const unsigned char *const scaled_source =
        reinterpret_cast<const unsigned char *>(pcBits_);

    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        const int height = iSpriteHeight_;
        const int width = iSpriteWidth_;
        RECT original;
        original.top = y + iTopOffset_;
        original.left = x + iLeftOffset_;
        original.right = original.left + width;
        original.bottom = original.top + height;
        RECT clipped = original;
        if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        // Original rereads the member after IntersectRect in the unit path.
        const unsigned char *source_row =
            reinterpret_cast<const unsigned char *>(pcBits_) +
            (clipped.left - original.left) +
            (clipped.top - original.top) * source_pitch;
        unsigned char *destination_row =
            static_cast<unsigned char *>(destination->locked_bits_) +
            clipped.left + clipped.top * destination_pitch;
        const int columns = clipped.right - clipped.left;
        const int rows = clipped.bottom - clipped.top;
        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                if (source_row[column] != key)
                    destination_row[column] = table[source_row[column]];
            }
            if (row + 1 < rows) {
                source_row += source_pitch;
                destination_row += destination_pitch;
            }
        }
        destination->free_data(1);
        return 0;
    }

    // Original FILD/FMUL/FADD remains live through _ftol: only the shared
    // scale ratio is float32, not each product. Coordinates aren't scaled.
    const int scaled_height = static_cast<int>(
        static_cast<double>(iSpriteHeight_) * SpriteScalingRatio);
    const int scaled_width = static_cast<int>(
        static_cast<double>(iSpriteWidth_) * SpriteScalingRatio);
    const int top = static_cast<int>(
        static_cast<double>(y) + static_cast<double>(iTopOffset_) * SpriteScalingRatio);
    const int left = static_cast<int>(
        static_cast<double>(x) + static_cast<double>(iLeftOffset_) * SpriteScalingRatio);
    RECT original = {left, top, left + scaled_width, top + scaled_height};
    RECT clipped = original;
    if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
        destination->free_data(1);
        return 0;
    }
    // Early clipping is repeated. First pixel uses the previously captured
    // pitch, then traversal uses the member reread after the first clip.
    unsigned char *const destination_bits =
        static_cast<unsigned char *>(destination->locked_bits_);
    const int base_offset = original.left + original.top * destination_pitch;
    destination_pitch = static_cast<int>(destination->stride_);
    int source_x, source_y, destination_x, destination_y;
    int source_columns, source_rows, destination_columns, destination_rows;

    if (SpriteScaleNumerator < SpriteScaleDenominator) {
        if (original.right - original.left < SpriteMinimumScaledWidth ||
            original.bottom - original.top < SpriteMinimumScaledHeight) {
            destination->free_data(1);
            return 0;
        }
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_shrink(&original, &clipped,
            &destination_x, &source_x, &destination_columns, &source_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_shrink(&original, &clipped,
            &destination_y, &source_y, &destination_rows, &source_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source +
            (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits +
            (base_offset + destination_x + destination_pitch * destination_y);
        for (int row = 0; row < destination_rows; ++row) {
            const unsigned char *source = source_row;
            for (int column = 0; column < destination_columns; ++column) {
                if (*source != key) destination_row[column] = table[*source];
                if (column + 1 < destination_columns) source += columns[column];
            }
            if (row + 1 < destination_rows) {
                source_row += source_pitch * static_cast<unsigned char>(rows[row]);
                destination_row += destination_pitch;
            }
        }
    } else {
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_expand(&original, &clipped,
            &source_x, &destination_x, &source_columns, &destination_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_expand(&original, &clipped,
            &source_y, &destination_y, &source_rows, &destination_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source +
            (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits +
            (base_offset + destination_x + destination_pitch * destination_y);
        for (int row = 0; row < source_rows; ++row) {
            const unsigned char selector = static_cast<unsigned char>(rows[row]);
            const int repeats = selector == 1 ? 1 : selector == 2 ? 2 : selector == 3 ? 3 : 4;
            int output_column = 0;
            for (int column = 0; column < source_columns; ++column) {
                const unsigned char pixel_value = source_row[column];
                // Unlike unit/shrink, every expansion branch also skips zero.
                if (pixel_value != key && pixel_value != 0) {
                    // Original four-row branch alone indexes with destination
                    // (5EA54D/5EA5A0/5EA5F3/5EA646). Preserve this quirk.
                    const unsigned char mapped = table[repeats == 4
                        ? destination_row[output_column] : pixel_value];
                    for (int repeat = 0; repeat < repeats; ++repeat)
                        for (int pixel = 0; pixel < columns[column]; ++pixel)
                            destination_row[repeat * destination_pitch + output_column + pixel] = mapped;
                }
                output_column += columns[column];
            }
            if (row + 1 < source_rows) {
                source_row += source_pitch;
                destination_row += repeats * destination_pitch;
            }
        }
    }
    destination->free_data(1);
    return 0;
}

// ORIGINAL: 0x005E5833 ?draw_dest@Sprite@@QAEHPAUBuffer@@HHHPAE@Z 0x005E5833-0x005E6652
// RECOVERY_BINDING: v1 {"evidence": ["Pinned worker tests transparency before source zero. Unit and shrink paths remap individual destination pixels for zero sources. Expansion5E6097/6268/63F3/6546 samples table[first destination] once per source pixel and replicates that cached color over its block. No file-backed load path exists in this worker.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Replace the destination-remap Sprite forwarder with actual clipped pixel operations."}
// symbol    ?draw_dest@Sprite@@QAEHPAVBuffer@@HHHPAE@Z
int Sprite::draw_dest(Buffer *destination, int transparent, int x, int y,
                      unsigned char *table) {
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0)
        return 3;

    if (SpriteScaleNumerator != SpriteScaleDenominator &&
        (SpriteCachedScaleNumerator != SpriteScaleNumerator ||
         SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        // FILD/FIDIV followed by the one float32 publication at5E3F2F.
        SpriteScalingRatio = static_cast<float>(
            static_cast<double>(SpriteScaleNumerator) / SpriteScaleDenominator);
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }

    const unsigned char key = static_cast<unsigned char>(transparent);
    const int source_pitch = iSpriteWidth2_;
    int destination_pitch = static_cast<int>(destination->stride_);
    const unsigned char *const scaled_source =
        reinterpret_cast<const unsigned char *>(pcBits_);

    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        RECT original;
        original.top = y + iTopOffset_;
        original.left = x + iLeftOffset_;
        original.right = original.left + iSpriteWidth_;
        original.bottom = original.top + iSpriteHeight_;
        RECT clipped = original;
        if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        // The plain-color worker uses source pitch for the initial top clip.
        const unsigned char *source_row =
            reinterpret_cast<const unsigned char *>(pcBits_) +
            (clipped.left - original.left) +
            (clipped.top - original.top) * source_pitch;
        unsigned char *destination_row =
            static_cast<unsigned char *>(destination->locked_bits_) +
            clipped.left + clipped.top * destination_pitch;
        const int columns = clipped.right - clipped.left;
        const int rows = clipped.bottom - clipped.top;
        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                if (source_row[column] != key)
                    destination_row[column] = source_row[column] == 0
                        ? table[destination_row[column]] : source_row[column];
            }
            if (row + 1 < rows) {
                source_row += source_pitch;
                destination_row += destination_pitch;
            }
        }
        destination->free_data(1);
        return 0;
    }

    // The integer argument coordinates are not themselves scaled. Original
    // x87 keeps the multiply/add live through _ftol; avoid float32 products.
    const int scaled_height = static_cast<int>(
        static_cast<double>(iSpriteHeight_) * SpriteScalingRatio);
    const int scaled_width = static_cast<int>(
        static_cast<double>(iSpriteWidth_) * SpriteScalingRatio);
    const int top = static_cast<int>(
        static_cast<double>(y) + static_cast<double>(iTopOffset_) * SpriteScalingRatio);
    const int left = static_cast<int>(
        static_cast<double>(x) + static_cast<double>(iLeftOffset_) * SpriteScalingRatio);
    RECT original = {left, top, left + scaled_width, top + scaled_height};
    RECT clipped = original;
    if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
        destination->free_data(1);
        return 0;
    }
    // The first clip is an early rejection only. The second call below
    // really recomputes from the original bounds; it must not be removed.
    unsigned char *const destination_bits =
        static_cast<unsigned char *>(destination->locked_bits_);
    const int base_offset = original.left + original.top * destination_pitch;
    destination_pitch = static_cast<int>(destination->stride_);
    int source_x, source_y, destination_x, destination_y;
    int source_columns, source_rows, destination_columns, destination_rows;

    if (SpriteScaleNumerator < SpriteScaleDenominator) {
        if (original.right - original.left < SpriteMinimumScaledWidth ||
            original.bottom - original.top < SpriteMinimumScaledHeight) {
            destination->free_data(1);
            return 0;
        }
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_shrink(&original, &clipped,
            &destination_x, &source_x, &destination_columns, &source_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_shrink(&original, &clipped,
            &destination_y, &source_y, &destination_rows, &source_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < destination_rows; ++row) {
            const unsigned char *source = source_row;
            for (int column = 0; column < destination_columns; ++column) {
                if (*source != key)
                    destination_row[column] = *source == 0
                        ? table[destination_row[column]] : *source;
                if (column + 1 < destination_columns) source += columns[column];
            }
            // MOV DL,[row_table]; ADD row_table,4 at5E4477..447C.
            // Nonzero low-byte row skips are the finite original domain.
            if (row + 1 < destination_rows) {
                source_row += source_pitch * static_cast<unsigned char>(rows[row]);
                destination_row += destination_pitch;
            }
        }
    } else {
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_expand(&original, &clipped,
            &source_x, &destination_x, &source_columns, &destination_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_expand(&original, &clipped,
            &source_y, &destination_y, &source_rows, &destination_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < source_rows; ++row) {
            const unsigned char selector = static_cast<unsigned char>(rows[row]);
            const int repeats = selector == 1 ? 1 : selector == 2 ? 2 : selector == 3 ? 3 : 4;
            int output_column = 0;
            for (int column = 0; column < source_columns; ++column) {
                const unsigned char pixel_value = source_row[column];
                if (pixel_value != key) {
                    // Expanded zero pixels sample the first destination once,
                    // then replicate that result over the whole output block.
                    const unsigned char mapped = pixel_value == 0
                        ? table[destination_row[output_column]] : pixel_value;
                    for (int repeat = 0; repeat < repeats; ++repeat)
                        for (int pixel = 0; pixel < columns[column]; ++pixel)
                            {
                            unsigned char &target = destination_row[
                                repeat * destination_pitch + output_column + pixel];
                            target = mapped;
                        }
                }
                output_column += columns[column];
            }
            if (row + 1 < source_rows) {
                source_row += source_pitch;
                destination_row += repeats * destination_pitch;
            }
        }
    }
    destination->free_data(1);
    return 0;
}

// ORIGINAL: 0x005E66A2 ?draw_dest_unk1@Sprite@@QAEHPAUBuffer@@HHPAE@Z 0x005E66A2-0x005E73CC
// draw_dest's sibling with no transparency key of its own: the stencil is
// `source byte == 0 -> destination = table[destination]` (cmp byte ptr [esi],0
// at 0x005E68F1/0x005E6C29), so borrowed, keyed and zero destinations all
// remap wherever the source is zero. Everything else is the draw_dest family
// skeleton: the same scale preamble (the one float32 publication at 0x005E66FC,
// the 4.0 ceiling over the qword at 0x0066FDC8), source pitch for the initial
// 1:1 top clip, shrink/expand table helpers, and the final free_data.
// symbol    ?draw_dest_unk1@Sprite@@QAEHPAVBuffer@@HHPAE@Z
int Sprite::draw_dest_unk1(Buffer *destination, int x, int y,
                           unsigned char *table) {
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0)
        return 3;

    if (SpriteScaleNumerator != SpriteScaleDenominator &&
        (SpriteCachedScaleNumerator != SpriteScaleNumerator ||
         SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        // FILD/FIDIV followed by the one float32 publication at5E66FC.
        SpriteScalingRatio = static_cast<float>(
            static_cast<double>(SpriteScaleNumerator) / SpriteScaleDenominator);
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }

    const int source_pitch = iSpriteWidth2_;
    int destination_pitch = static_cast<int>(destination->stride_);
    const unsigned char *const scaled_source =
        reinterpret_cast<const unsigned char *>(pcBits_);

    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        RECT original;
        original.top = y + iTopOffset_;
        original.left = x + iLeftOffset_;
        original.right = original.left + iSpriteWidth_;
        original.bottom = original.top + iSpriteHeight_;
        RECT clipped = original;
        if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        // The plain worker uses source pitch for the initial top clip.
        const unsigned char *source_row =
            reinterpret_cast<const unsigned char *>(pcBits_) +
            (clipped.left - original.left) +
            (clipped.top - original.top) * source_pitch;
        unsigned char *destination_row =
            static_cast<unsigned char *>(destination->locked_bits_) +
            clipped.left + clipped.top * destination_pitch;
        const int columns = clipped.right - clipped.left;
        const int rows = clipped.bottom - clipped.top;
        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                if (source_row[column] == 0)
                    destination_row[column] =
                        table[destination_row[column]];
            }
            if (row + 1 < rows) {
                source_row += source_pitch;
                destination_row += destination_pitch;
            }
        }
        destination->free_data(1);
        return 0;
    }

    // The integer argument coordinates are not themselves scaled. Original
    // x87 keeps the multiply/add live through _ftol; avoid float32 products.
    const int scaled_height = static_cast<int>(
        static_cast<double>(iSpriteHeight_) * SpriteScalingRatio);
    const int scaled_width = static_cast<int>(
        static_cast<double>(iSpriteWidth_) * SpriteScalingRatio);
    const int top = static_cast<int>(
        static_cast<double>(y) + static_cast<double>(iTopOffset_) * SpriteScalingRatio);
    const int left = static_cast<int>(
        static_cast<double>(x) + static_cast<double>(iLeftOffset_) * SpriteScalingRatio);
    RECT original = {left, top, left + scaled_width, top + scaled_height};
    RECT clipped = original;
    if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
        destination->free_data(1);
        return 0;
    }
    // The first clip is an early rejection only. The second call below
    // really recomputes from the original bounds; it must not be removed.
    unsigned char *const destination_bits =
        static_cast<unsigned char *>(destination->locked_bits_);
    const int base_offset = original.left + original.top * destination_pitch;
    destination_pitch = static_cast<int>(destination->stride_);
    int source_x, source_y, destination_x, destination_y;
    int source_columns, source_rows, destination_columns, destination_rows;

    if (SpriteScaleNumerator < SpriteScaleDenominator) {
        if (original.right - original.left < SpriteMinimumScaledWidth ||
            original.bottom - original.top < SpriteMinimumScaledHeight) {
            destination->free_data(1);
            return 0;
        }
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_shrink(&original, &clipped,
            &destination_x, &source_x, &destination_columns, &source_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_shrink(&original, &clipped,
            &destination_y, &source_y, &destination_rows, &source_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < destination_rows; ++row) {
            const unsigned char *source = source_row;
            unsigned char *destination = destination_row;
            for (int column = 0; column < destination_columns; ++column) {
                if (*source == 0)
                    *destination = table[*destination];
                // The source advance runs on every column, the last one
                // included; the row step below subtracts the consumed row.
                source += columns[column];
                ++destination;
            }
            // MOV AL,[row_table]; ADD row_table,4 at5E6CA9..5E6CB4, then the
            // (count-1) extra source pitches and the pitch - source_columns
            // residual: pitch * selector - source_columns is what the two
            // image steps add up to.
            // Nonzero low-byte row skips are the finite original domain.
            if (row + 1 < destination_rows) {
                source_row += static_cast<int>(
                    static_cast<unsigned char>(rows[row])) * source_pitch
                    - source_columns;
                destination_row += destination_pitch;
            }
        }
    } else {
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_expand(&original, &clipped,
            &source_x, &destination_x, &source_columns, &destination_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_expand(&original, &clipped,
            &source_y, &destination_y, &source_rows, &destination_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < source_rows; ++row) {
            const unsigned char selector = static_cast<unsigned char>(rows[row]);
            const int repeats = selector == 1 ? 1 : selector == 2 ? 2 : selector == 3 ? 3 : 4;
            int output_column = 0;
            for (int column = 0; column < source_columns; ++column) {
                if (source_row[column] == 0) {
                    // Zero source pixels sample the first destination once,
                    // then replicate that result over the whole output block.
                    const unsigned char mapped =
                        table[destination_row[output_column]];
                    for (int repeat = 0; repeat < repeats; ++repeat)
                        for (int pixel = 0; pixel < columns[column]; ++pixel)
                            destination_row[
                                repeat * destination_pitch + output_column +
                                pixel] = mapped;
                }
                output_column += columns[column];
            }
            if (row + 1 < source_rows) {
                source_row += source_pitch;
                destination_row += repeats * destination_pitch;
            }
        }
    }
    destination->free_data(1);
    return 0;
}

// ORIGINAL: 0x005E73CC ?draw_dest_unk1@Sprite@@QAEHPAVBuffer@@HHHHPAE@Z 0x005E73CC-0x005E741C BYTE_EXACT
// The wider draw overloads' scale substitution: saves 696D18/696D1C, installs
// the last two int arguments, calls the four-argument worker and restores
// denominator then numerator (5E73CC..5E741B). The BYTE_EXACT above was
// banked by `recover record` from the tree-style body as landed (fresh
// measure 2026-09-26, 22/22 instructions) with the seed's historical bytes
// intact; the seed then left with this sub-batch like its siblings.
// symbol    ?draw_dest_unk1@Sprite@@QAEHPAVBuffer@@HHHHPAE@Z
int Sprite::draw_dest_unk1(Buffer *destination, int x, int y, int numerator,
                           int denominator, unsigned char *table) {
    const int saved_numerator = SpriteScaleNumerator;
    const int saved_denominator = SpriteScaleDenominator;
    SpriteScaleNumerator = numerator;
    SpriteScaleDenominator = denominator;
    const int result = draw_dest_unk1(destination, x, y, table);
    SpriteScaleDenominator = saved_denominator;
    SpriteScaleNumerator = saved_numerator;
    return result;
}

// ORIGINAL: 0x005E741C ?draw_dest_source@Sprite@@QAEHPAUBuffer@@HHHPAE@Z 0x005E741C-0x005E811C
// The name is the archive's, not the image's: the worker REMAPS DESTINATIONS.
// Where a source byte is not the transparency key the destination pixel goes
// through the table (`mov bl,[edi]; mov bl,[ebp+ebx]` at 0x005E7672/0x005E79A2
// /0x005E7C07) - unlike draw_dest there is no `source == 0` fallback writing
// the source through, and unlike draw_dest_unk1 the key byte is live. Seven-
// argument 0x005E811C forwards here after substituting its scale pair.
// symbol    ?draw_dest_source@Sprite@@QAEHPAVBuffer@@HHHPAE@Z
int Sprite::draw_dest_source(Buffer *destination, int transparent, int x, int y,
                             unsigned char *table) {
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0)
        return 3;

    if (SpriteScaleNumerator != SpriteScaleDenominator &&
        (SpriteCachedScaleNumerator != SpriteScaleNumerator ||
         SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        // FILD/FIDIV followed by the one float32 publication at5E7476.
        SpriteScalingRatio = static_cast<float>(
            static_cast<double>(SpriteScaleNumerator) / SpriteScaleDenominator);
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }

    const unsigned char key = static_cast<unsigned char>(transparent);
    const int source_pitch = iSpriteWidth2_;
    int destination_pitch = static_cast<int>(destination->stride_);
    const unsigned char *const scaled_source =
        reinterpret_cast<const unsigned char *>(pcBits_);

    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        RECT original;
        original.top = y + iTopOffset_;
        original.left = x + iLeftOffset_;
        original.right = original.left + iSpriteWidth_;
        original.bottom = original.top + iSpriteHeight_;
        RECT clipped = original;
        if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        // The plain worker uses source pitch for the initial top clip.
        const unsigned char *source_row =
            reinterpret_cast<const unsigned char *>(pcBits_) +
            (clipped.left - original.left) +
            (clipped.top - original.top) * source_pitch;
        unsigned char *destination_row =
            static_cast<unsigned char *>(destination->locked_bits_) +
            clipped.left + clipped.top * destination_pitch;
        const int columns = clipped.right - clipped.left;
        const int rows = clipped.bottom - clipped.top;
        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                if (source_row[column] != key)
                    destination_row[column] =
                        table[destination_row[column]];
            }
            if (row + 1 < rows) {
                source_row += source_pitch;
                destination_row += destination_pitch;
            }
        }
        destination->free_data(1);
        return 0;
    }

    // The integer argument coordinates are not themselves scaled. Original
    // x87 keeps the multiply/add live through _ftol; avoid float32 products.
    const int scaled_height = static_cast<int>(
        static_cast<double>(iSpriteHeight_) * SpriteScalingRatio);
    const int scaled_width = static_cast<int>(
        static_cast<double>(iSpriteWidth_) * SpriteScalingRatio);
    const int top = static_cast<int>(
        static_cast<double>(y) + static_cast<double>(iTopOffset_) * SpriteScalingRatio);
    const int left = static_cast<int>(
        static_cast<double>(x) + static_cast<double>(iLeftOffset_) * SpriteScalingRatio);
    RECT original = {left, top, left + scaled_width, top + scaled_height};
    RECT clipped = original;
    if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
        destination->free_data(1);
        return 0;
    }
    // The first clip is an early rejection only. The second call below
    // really recomputes from the original bounds; it must not be removed.
    unsigned char *const destination_bits =
        static_cast<unsigned char *>(destination->locked_bits_);
    const int base_offset = original.left + original.top * destination_pitch;
    destination_pitch = static_cast<int>(destination->stride_);
    int source_x, source_y, destination_x, destination_y;
    int source_columns, source_rows, destination_columns, destination_rows;

    if (SpriteScaleNumerator < SpriteScaleDenominator) {
        if (original.right - original.left < SpriteMinimumScaledWidth ||
            original.bottom - original.top < SpriteMinimumScaledHeight) {
            destination->free_data(1);
            return 0;
        }
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_shrink(&original, &clipped,
            &destination_x, &source_x, &destination_columns, &source_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_shrink(&original, &clipped,
            &destination_y, &source_y, &destination_rows, &source_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < destination_rows; ++row) {
            const unsigned char *source = source_row;
            unsigned char *destination = destination_row;
            for (int column = 0; column < destination_columns; ++column) {
                if (*source != key)
                    *destination = table[*destination];
                // The source advance runs on every column, the last one
                // included; the row step below subtracts the consumed row.
                source += columns[column];
                ++destination;
            }
            // MOV AL,[row_table]; ADD row_table,4 at5E7A16..5E7A21: the image
            // steps pitch*(selector-1) plus the (pitch - source_columns)
            // residual, which is pitch * selector - source_columns together.
            // Nonzero low-byte row skips are the finite original domain.
            if (row + 1 < destination_rows) {
                source_row += static_cast<int>(
                    static_cast<unsigned char>(rows[row])) * source_pitch
                    - source_columns;
                destination_row += destination_pitch;
            }
        }
    } else {
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_expand(&original, &clipped,
            &source_x, &destination_x, &source_columns, &destination_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_expand(&original, &clipped,
            &source_y, &destination_y, &source_rows, &destination_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < source_rows; ++row) {
            const unsigned char selector = static_cast<unsigned char>(rows[row]);
            const int repeats = selector == 1 ? 1 : selector == 2 ? 2 : selector == 3 ? 3 : 4;
            int output_column = 0;
            for (int column = 0; column < source_columns; ++column) {
                const unsigned char pixel_value = source_row[column];
                if (pixel_value != key) {
                    // Non-key source pixels sample the first destination
                    // once, then replicate that result over the whole block.
                    const unsigned char mapped =
                        table[destination_row[output_column]];
                    for (int repeat = 0; repeat < repeats; ++repeat)
                        for (int pixel = 0; pixel < columns[column]; ++pixel)
                            destination_row[
                                repeat * destination_pitch + output_column +
                                pixel] = mapped;
                }
                output_column += columns[column];
            }
            if (row + 1 < source_rows) {
                source_row += source_pitch;
                destination_row += repeats * destination_pitch;
            }
        }
    }
    destination->free_data(1);
    return 0;
}

/*
Purpose: Draw with destination-source remapping under a temporary scale.
// ORIGINAL: 0x005E811C ?draw_dest_source@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z 0x005E811C-0x005E816C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The seven-argument seed substitutes the scale pair around the already source-owned destination-source remapping overload.","evidence":["Original 0x005E811C-0x005E816C saves 0x00696D18/1C, calls the five-argument overload at 0x005E741C and restores the two globals in reverse order.","src/sprite.cpp owns the callee and typed scale globals; git:d2e09e91:src/recovered/units/005e811c.cpp retains the historical exact seed."]}
// symbol    ?draw_dest_source@Sprite@@QAEHPAVBuffer@@HHHHHPAE@Z
*/
int Sprite::draw_dest_source(Buffer *destination, int transparent, int x, int y,
                             int numerator, int denominator,
                             unsigned char *table) {
    const int saved_numerator = SpriteScaleNumerator;
    const int saved_denominator = SpriteScaleDenominator;
    SpriteScaleNumerator = numerator;
    SpriteScaleDenominator = denominator;
    const int result = draw_dest_source(destination, transparent, x, y, table);
    SpriteScaleDenominator = saved_denominator;
    SpriteScaleNumerator = saved_numerator;
    return result;
}

// Shared four-byte sprite-memory counter: the constructor at 0x005E37E6/
// 0x005E37EF reads/writes it, and close at 0x005E3833/0x005E383F updates
// the same scalar. Its original storage is in .data's loader-zero-filled
// tail, not a pointer into the executable. The constructor adds sizeof(Sprite);
// pixel-release paths subtract pixel area, so full accounting is still unclear.
// RECOVERY_OBJECT: v2 0x009B6618 0x009B661C ?SpriteMemoryUsed@@3HA 0x005E37E0
int SpriteMemoryUsed = 0;

/*
Purpose: Draw with a temporary scale and a destination remap table.
// ORIGINAL: 0x005E4AFA ?draw@Sprite@@QAEHPAUBuffer@@HHHHHH@Z 0x005E4AFA-0x005E4B4A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The seven-argument seed is a scale-substitution wrapper around the already source-owned table-remapping Sprite::draw overload; its final catalogued int is a pointer to the remap table.","evidence":["Original 0x005E4AFA-0x005E4B4A saves the dwords at 0x00696D18/1C, substitutes numerator/denominator, calls 0x005E9D44 and restores in reverse order.","src/sprite.cpp owns 0x005E9D44 and binds both scale globals; git:d2e09e91:src/recovered/units/005e4afa.cpp retains the historical exact seed."]}
// symbol    ?draw@Sprite@@QAEHPAVBuffer@@HHHHHPAE@Z
*/
int Sprite::draw(Buffer *destination, int transparent, int x, int y,
                 int numerator, int denominator, unsigned char *table) {
    const int saved_numerator = SpriteScaleNumerator;
    const int saved_denominator = SpriteScaleDenominator;
    SpriteScaleNumerator = numerator;
    SpriteScaleDenominator = denominator;
    const int result = draw(destination, transparent, x, y, table);
    SpriteScaleDenominator = saved_denominator;
    SpriteScaleNumerator = saved_numerator;
    return result;
}

/*
Purpose: Draw the sprite with a temporarily substituted uniform scale ratio.
// ORIGINAL: 0x005E4B4A ?draw@Sprite@@QAEHPAUBuffer@@HHHHH@Z 0x005E4B4A-0x005E4B95 BYTE_EXACT
// symbol    ?draw@Sprite@@QAEHPAVBuffer@@HHHHH@Z
// size      75 bytes
// prototype int (__thiscall ?draw@Sprite@@QAEHPAUBuffer@@HHHHH@Z)(Sprite* this, Buffer*, int, int, int, int, int)
// callers   48   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E4B9A
Return Value: Whatever the four-argument overload returns
Status: Complete with a temporary dependency on the four-argument overload
*/
int Sprite::draw(Buffer *buffer, int a, int b, int c, int x, int y) {
    // The ratio is swapped for the duration of the call and restored in the
    // reverse order the legacy body uses.
    const int saved_numerator = SpriteScaleNumerator;
    const int saved_denominator = SpriteScaleDenominator;
    SpriteScaleNumerator = x;
    SpriteScaleDenominator = y;
    const int result = draw(buffer, a, b, c);
    SpriteScaleDenominator = saved_denominator;
    SpriteScaleNumerator = saved_numerator;
    return result;
}


/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005E4ADA ?UNK1@Sprite@@QAEHHHHHHHH@Z 0x005E4ADA-0x005E4ADF BYTE_EXACT
// symbol    ?draw_noop@Sprite@@QAEHPAVBuffer@@HHHHHH@Z
// size      5 bytes
// prototype int (__thiscall ?UNK1@Sprite@@QAEHHHHHHHH@Z)(Sprite* this, int, int, int, int, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Sprite::draw_noop(Buffer *, int, int, int, int, int, int) {
    return 0;
}


/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005E4AEA ?UNK2@Sprite@@QAEHHHHHH@Z 0x005E4AEA-0x005E4AEF BYTE_EXACT
// symbol    ?draw_noop@Sprite@@QAEHPAVBuffer@@HHHH@Z
// size      5 bytes
// prototype int (__thiscall ?UNK2@Sprite@@QAEHHHHHH@Z)(Sprite* this, int, int, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Sprite::draw_noop(Buffer *, int, int, int, int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x005EAAFF ?UNK3@Sprite@@QAEXHH@Z 0x005EAAFF-0x005EAB02 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK3@Sprite@@QAEXHH@Z)(Sprite* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Sprite::UNK3(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x005EAD8F ?UNK4@Sprite@@QAEXHH@Z 0x005EAD8F-0x005EAD92 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK4@Sprite@@QAEXHH@Z)(Sprite* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Sprite::UNK4(int, int) {
}


/*
// ORIGINAL: 0x0063CE20 sub_63ce20 0x0063CE20-0x0063CEE7;0x00663998-0x006639AD BYTE_EXACT
// symbol    ?init_class@Sprite@@SAHXZ
// LEVER 2026-09-03: two things, and the first one is worth stating because
//   this body carried a TRIED note for two passes claiming otherwise. The /GX
//   SEH frame over the local Buffer IS reproduced by the plain /O2 /Ob0 /GX
//   build - `sub esp, 0x588`, the fs:[0] swap and the `mov dword ptr
//   [esp+0x594], -1` state stores are all there - so the frame was never the
//   wall. What was missing was the return shape: two SEPARATE epilogues,
//   `if (result != 0) return result;` then `return 0;`, which is the
//   split-epilogue lever. 34/53 -> 53/53 BYTE_EXACT. The SEH span at 0x663998
//   stays cold code the image lays elsewhere.
// size      220 bytes
// prototype
// callers   1   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7DE0 0x005E39A0 0x005EAF3F
//
// NAMED Sprite::init_class 2026-08-30: jackal_init_real runs it in the
// X::init_class() chain between FileWin::init_class and the font bring-up,
// and it seeds the class-shared sprite (g_BLANK_SPRITE, 0x009BEAE8) from
// jackal.pcx - or blank when the PCX is missing. The catalogue carries no
// name for it, so the address keeps the marker line. The teardown twin,
// 0x0063CEF0 in general.cpp, is close_class by the same reading.
//
// Promoted 2026-08-15 from src/unrecovered/0063ce20.cpp to retire its
// pending_bodies forwarder. Loads the 16x16 sprite at 0x9BEAE8 from a PCX, or
// creates it blank if the PCX is missing.
Status: Complete
*/
// NAMED FROM THE IMAGE AND FROM THIS BODY'S OWN COMMENT: 0x00698CAC holds
// the string 'jackal.pcx' (image_data --string), and 0x009BEAE8 is the
// 16x16 sprite this loads into - which guarded_teardowns.h ALREADY declares
// as TeardownObject009BEAE8. Two bindings at one address is the shape
// duplicate_globals flags, so this file uses the existing declaration
// rather than minting a second name for the same storage.
static const char *const JackalPcxPath = (const char *)0x00698CAC;

Sprite g_BLANK_SPRITE;  // 0x009BEAE8 - the create_blank scratch sprite

int Sprite::init_class() {
    Buffer buf;
    int result;
    if (buf.load_pcx(JackalPcxPath, 0, 10, 0xec) != 0) {
        result = g_BLANK_SPRITE.create_blank(0x10, 0x10, 9);
    } else {
        result = g_BLANK_SPRITE.extract(&buf, 9, 1, 0x12, 0x10, 0x10, 0);
    }
    // TWO RETURNS, not one: the image does not share the Buffer destructor
    // between the two exits, so a single `return result` costs nine
    // instructions (34/53) - a guard clause for the failure value puts the
    // `mov eax,esi` epilogue first and the `xor eax,eax` one second, exactly
    // the image's order, and the state store lands between `mov esi,eax` and
    // the destructor receiver's `lea` where the image has it.
    if (result != 0) {
        return result;
    }
    return 0;
}

/*
Purpose: Take `size` bytes of sheet memory: walk the heap's block table and
         allocate from the first block whose free tail is large enough.
// ORIGINAL: 0x006353C0 ?get_mem@TexHeap@@QAEHH@Z 0x006353C0-0x00635402 FILE SEMANTIC
// symbol    ?get_mem@TexHeap@@QAEPAXH@Z
// TRIED 2026-09-03, two batches of four: count declared before index, both
//   for-loop forms (initialiser inside the `for`, index declared outside it),
//   a named `Heap *block` local instead of the double `blocks[index]` access,
//   `count_ + 1` without the static_cast, `1 + static_cast<int>(count_)`, an
//   empty-initialiser `for` over the same pair, and `blocks` hoisted above
//   the count guard. All eight: 26 of 33 or worse. `blocks` above the guard
//   is 18 of 33 and the four restructurings are 5 of 33 with 35 instructions
//   - two MORE than the image, because moving index's initialiser inside the
//   guard makes VC6 emit a second `xor`.
// LEVER (not enough): the committed spelling is the only one at the image's
//   33 instructions, and it is 26 of 33 with MNEMONIC_ONLY 1.000 - the four
//   divergences are ALL the same register swap, the loop index in eax and
//   `count_ + 1` in edx where this tree has them the other way round
//   (`blocks`, `size`, the walking pointer and the block load all agree).
//   Also swept --all-flags: /O2 /Ob0 /GX and /O2 /Gy /GR- all tie at 26 of
//   33; /G6 is 2 of 33. Plateau: a pure allocator coin flip on a 66-byte
//   body, no source relationship left to name.
// size      66 bytes
// prototype int (__thiscall ?get_mem@TexHeap@@QAEHH@Z)(#120* this, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4680
Status: Promoted 2026-08-29 from src/recovered/units/006353c0.cpp, which
measured MNEMONIC_ONLY against the opaque scaffold and carried no claim. The
`+0x10` read is the block Heap's `free_size_`; the allocation itself is
`Heap::get` (0x005D4680). `Sprite::extract` below is the one caller, and it
only reaches here when a caller passed a heap at all.
*/
void *TexHeap::get_mem(int size) {
    if (base_ != 0) {
        int index = 0;
        int count = static_cast<int>(count_) + 1;
        if (count > 0) {
            Heap **blocks = blocks_;
            while (index < count) {
                if (static_cast<int>(blocks[index]->free_size_) >= size) {
                    return blocks[index]->get(size);
                }
                ++index;
            }
        }
    }
    return 0;
}

/*
Purpose: Hand out the first sheet-heap block whose free tail fits `size` and
         whose `current_` cursor is 64-KiB aligned (low 16 bits zero), returning
         the payload from Heap::get. The walk is get_mem's, with
         one more condition in the filter, and `Texture::extract` is its only
         caller - it takes the returned block as a borrowed texture rather
         than allocating fresh pixels.
// ORIGINAL: 0x00635370 ?get_tex_mem@TexHeap@@QAEHH@Z 0x00635370-0x006353BD
// symbol    ?get_tex_mem@TexHeap@@QAEPAXH@Z
// size      77 bytes
// prototype int (__thiscall ?get_tex_mem@TexHeap@@QAEHH@Z)(TexHeap* this, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4680
// TRIED (as src/recovered/units/00635370.cpp, raw pointer walks): the
//   artifact reached MISMATCH at instruction 3 with a push-ebp frame and two
//   callee-saved pushes where the image has three and no frame, and its note
//   ruled out a source-form cause. Homed here on the member spelling get_mem
//   already measures 26/33 with, the low-16 test written through the Heap
//   members rather than raw offsets; the frame shape is unchanged by that.
Status: Promoted from src/recovered/units/00635370.cpp.
*/
void *TexHeap::get_tex_mem(int size) {
    if (base_ != 0) {
        int index = 0;
        int count = static_cast<int>(count_) + 1;
        if (count > 0) {
            Heap **blocks = blocks_;
            do {
                Heap *block = blocks[index];
                if ((reinterpret_cast<uintptr_t>(block->current_) & 0xffff) == 0 &&
                    static_cast<int>(block->free_size_) >= size) {
                    return block->get(size);
                }
                ++index;
            } while (index < count);
        }
    }
    return 0;
}

/*
Purpose: Extract a sprite from a rectangle, rejecting a null region.
// ORIGINAL: 0x005E3960 ?extract@Sprite@@QAEHPAUBuffer@@HPAURECT@@PAUTexHeap@@@Z 0x005E3960-0x005E399E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The rectangle overload forwards its bounds to the source-owned coordinate extractor, returning 3 for a null rectangle.","evidence":["Original 0x005E3960-0x005E399E loads RECT.left/top/right/bottom, computes right-left and bottom-top, and calls 0x005E39A0.","src/sprite.cpp owns the coordinate overload at 0x005E39A0; git:d2e09e91:src/recovered/units/005e3960.cpp retains the historical exact seed."]}
// symbol    ?extract@Sprite@@QAEHPAVBuffer@@HPAUtagRECT@@PAUTexHeap@@@Z
*/
int Sprite::extract(Buffer *buffer, int transparent, RECT *region, TexHeap *heap) {
    if (region == 0) {
        return 3;
    }
    return extract(buffer, transparent, region->left, region->top,
                   region->right - region->left, region->bottom - region->top,
                   heap);
}

/*
Purpose: Cut the width x height rectangle at (left, top) out of a locked
         buffer's pixels into this sprite, trimming the fully transparent
         border on all four sides, then allocate the trimmed pixel block from
         the sheet heap or the CRT and copy it across.
// ORIGINAL: 0x005E39A0 ?extract@Sprite@@QAEHPAUBuffer@@HHHHHPAUTexHeap@@@Z 0x005E39A0-0x005E3DF1 FILE
// symbol    ?extract@Sprite@@QAEHPAVBuffer@@HHHHHPAUTexHeap@@@Z
// The image spells the buffer pointer `PAU` - Buffer as a STRUCT - where this
// tree declares `class Buffer`, and MSVC decorates the first-seen class-key.
// `Sprite::draw` carries the same fact for the same reason.
// TRIED: direct field-name transcription of Sprite::extract mirroring Ghidra's param_1[N]; frame-pointer/register allocation shape diverges at insn #0
// TRIED: first guard spelled `buffer == 0 || buffer->get_data() == 0`, as two
//   nested `if (x != 0)` blocks, and as one `x != 0 && y != 0` chain - all
//   three compile to the same 44 of 397 agreeing instructions (0.392 similar).
//   Flag sets scored by measure --all-flags: /c /O2 /Ob0 /Gy /GR- /GX is the
//   best at 44/397; every non-/Ob0 set is 7/397 or worse because the in-class
//   `get_data()` then inlines and its E8 disappears. /Ob0 is load-bearing.
// TRIED 2026-09-03, re-swept after sprite.h retyped cTransparentIndex_ to
//   unsigned char: the committed body is 17 of 397 (0.415 similar) at /O2
//   /Ob0 /GX, the same best set as before. The fold
//   `cTransparentIndex_ = transparent & 0xff` scores 12 of 397; the nested
//   two-`if` guard scores the same 17; the volatile-punned byte store scores
//   18 and was refused as source. The retype neither helped nor hurt here.
// TRIED: the receiver-spill fingerprint, not reached. The image never
//   enregisters `buffer`: it loads it fresh into volatile ecx at each of its
//   ten uses (`mov ecx, [esp+0x1c]`) and spends the callee-saved registers on
//   this=esi, width=ebx, height=edi, zero=ebp - `xor ebp,ebp` sits above the
//   first null test. Every spelling tried instead enregisters `buffer` into
//   ebx at the function's first statement (scheduled before the remaining
//   callee-saved pushes) and shifts every other role one register over
//   (this=ebp, width=esi, zero=edi), renaming every field access and call
//   receiver in the body at once. Same wall as the Palette receiver-spill
//   bodies: guard reordering and condition shape do not move it.
// size      1105 bytes
// prototype int (__thiscall ?extract@Sprite@@QAEHPAUBuffer@@HHHHHPAUTexHeap@@@Z)(Sprite* this, Buffer*, int, int, int, int, int, #120*)
// callers   29   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005E3373 0x005E33F3 0x005E34A3 0x005FD2B0 0x006353C0 0x00644EF2 0x00645930
Return Value: 0 extracted; 3 bad arguments or empty rectangle; 4 the CRT
              allocation failed; 7 the buffer would not lock; 9 the rectangle
              is entirely transparent
Status: Semantics transcribed from the image

The four scans walk the requested rectangle four ways: top down for the first
row with a visible pixel, bottom up for the last, and across the kept rows for
the first and last visible columns. `iTopOffset_` counts the rows the top scan
spent; `iSpriteHeight_` opens as the untrimmed height and each bottom-scan row
spends one; `iLeftOffset_` and the right-trim local mirror them on the columns.
The bounds come off `dib_.bmiHeader`, whose `biHeight` is NEGATIVE for a
top-down DIB - so the bottom bound really is `-biHeight`, and the
two-coordinate `get_data` rejects the same way.
*/
int Sprite::extract(Buffer *buffer, int transparent, int left, int top,
                    int width, int height, TexHeap *heap) {
    int stride;
    int col;
    int rows;
    int right_trim;
    int row_index;
    const uint8_t *pixels;

    // ONE LOAD, ONE EXIT: the image tests `buffer` against zero with the same
    // ecx load that then serves as the call's receiver, and both tests share
    // the `return 3` tail with the four guards further down. NESTED, not `||`:
    // the nesting is what ends `buffer`'s live range at the test.
    if (buffer != 0 && buffer->get_data() != 0) {
        // Free the previous contents exactly as Sprite::close opens: the pixel
        // block is only owned, accounted, and released while the borrowed flag is
        // clear, and the image re-tests `pcBits_` before the free.
        if (fObj1Exists_ == 0 && pcBits_ != 0) {
            SpriteMemoryUsed = static_cast<int>(
                static_cast<uint32_t>(SpriteMemoryUsed)
                - static_cast<uint32_t>(iSpriteHeight_)
                      * static_cast<uint32_t>(iSpriteWidth_));
            if (pcBits_ != 0) {
                free(pcBits_);
            }
            pcBits_ = 0;
        }
        if (ppszFileName_ != 0) {
            free(ppszFileName_);
            ppszFileName_ = 0;
        }
        iSpriteWidth2_ = 0;
        iSpriteWidth_ = 0;
        iSpriteHeight_ = 0;
        iWidth_ = 0;
        iHeight_ = 0;
        iLeftOffset_ = 0;
        iTopOffset_ = 0;
        fObj1Exists_ = 0;

        if (width == 0) {
            return 3;
        }
        if (height == 0) {
            return 3;
        }
        if (left + width > static_cast<int>(buffer->dib_.bmiHeader.biWidth)) {
            return 3;
        }
        if (top + height > -static_cast<int>(buffer->dib_.bmiHeader.biHeight)) {
            return 3;
        }

        transparent &= 0xff;
        cTransparentIndex_ = static_cast<unsigned char>(transparent);
        iWidth_ = width;
        iHeight_ = height;
        pixels = reinterpret_cast<const uint8_t *>(buffer->get_data(left, top));
        if (pixels == 0) {
            buffer->free_data(1);
            return 7;
        }
        stride = static_cast<int>(buffer->stride_);
        iTopOffset_ = 0;
        // TOP TRIM: walk down until a row carries a visible pixel.
        for (row_index = 0; row_index < height; row_index++) {
            for (col = 0; col < width; col++) {
                if (pixels[col] != transparent) {
                    goto top_done;
                }
            }
            pixels += stride;
            iTopOffset_++;
        }
    top_done:
        do_sound();
        if (iTopOffset_ == height) {
            buffer->free_data(2);
            return 9;
        }
        buffer->free_data(1);
        // BOTTOM TRIM: walk up from the last row, right to left, until a row
        // carries a visible pixel. The pointer arrives pre-decremented - the
        // image indexes `row[col]` off `pixels - 1`, not `pixels[col - 1]`.
        pixels = reinterpret_cast<const uint8_t *>(
            buffer->get_data(left, top + height - 1));
        iSpriteHeight_ = height - iTopOffset_;
        for (row_index = 0; row_index < height; row_index++) {
            const uint8_t *from_right = pixels - 1;
            for (col = width; col > 0; col--) {
                if (from_right[col] != transparent) {
                    goto bottom_done;
                }
            }
            pixels -= stride;
            iSpriteHeight_--;
        }
    bottom_done:
        if (iSpriteHeight_ == 0) {
            buffer->free_data(2);
            return 9;
        }
        do_sound();
        buffer->free_data(1);
        // LEFT TRIM: on every kept row, find the first visible column.
        pixels = reinterpret_cast<const uint8_t *>(
            buffer->get_data(left, top + iTopOffset_));
        iLeftOffset_ = width;
        for (rows = iSpriteHeight_; rows > 0; rows--) {
            for (col = 0; col < width; col++) {
                if (pixels[col] != transparent) {
                    break;
                }
            }
            if (col < iLeftOffset_) {
                iLeftOffset_ = col;
            }
            pixels += stride;
        }
        do_sound();
        buffer->free_data(1);
        // RIGHT TRIM: on every kept row, count the transparent pixels at the end.
        // THIS scan indexes `pixels[col - 1]` - the image folds the -1 into the
        // addressing here, unlike the bottom trim above.
        pixels = reinterpret_cast<const uint8_t *>(
            buffer->get_data(left, top + iTopOffset_));
        right_trim = width;
        for (rows = iSpriteHeight_; rows > 0; rows--) {
            int counted = 0;
            for (col = width; col > 0; col--) {
                if (pixels[col - 1] != transparent) {
                    break;
                }
                counted++;
            }
            if (counted < right_trim) {
                right_trim = counted;
            }
            pixels += stride;
        }
        buffer->free_data(1);
        iSpriteWidth_ = width - iLeftOffset_ - right_trim;
        if (iSpriteWidth_ == 0) {
            buffer->free_data(1);
            return 9;
        }
        // The sheet heap is tried first and only when the caller named one. The
        // pixels then draw through the heap and `fObj1Exists_` says so; anything
        // else comes off the CRT as plain owned memory.
        pcBits_ = 0;
        if (heap != 0) {
            pcBits_ = reinterpret_cast<char *>(heap->get_mem(iSpriteHeight_ * iSpriteWidth_));
            if (pcBits_ != 0) {
                fObj1Exists_ = 1;
            }
        }
        do_sound();
        if (pcBits_ == 0) {
            pcBits_ = static_cast<char *>(mem_get(iSpriteWidth_ * iSpriteHeight_));
            if (pcBits_ == 0) {
                if (ppszFileName_ != 0) {
                    free(ppszFileName_);
                    ppszFileName_ = 0;
                }
                iSpriteWidth2_ = 0;
                iSpriteWidth_ = 0;
                iSpriteHeight_ = 0;
                iWidth_ = 0;
                iHeight_ = 0;
                iLeftOffset_ = 0;
                iTopOffset_ = 0;
                fObj1Exists_ = 0;
                buffer->free_data(1);
                return 4;
            }
            fObj1Exists_ = 0;
        }
        SpriteMemoryUsed += iSpriteWidth_ * iSpriteHeight_;
        pixels = reinterpret_cast<const uint8_t *>(
            buffer->get_data(left + iLeftOffset_, top + iTopOffset_));
        uint8_t *out = reinterpret_cast<uint8_t *>(pcBits_);
        do_sound();
        for (rows = iSpriteHeight_; rows > 0; rows--) {
            memcpy(out, pixels, iSpriteWidth_);
            out += iSpriteWidth_;
            pixels += stride;
        }
        iSpriteWidth2_ = iSpriteWidth_;
        buffer->free_data(2);
        return 0;

    }
    return 3;
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
// ORIGINAL: 0x00406850 ??1Sprite@@QAE@XZ 0x00406850-0x00406855 BYTE_EXACT
// body      src/sprite.h
// size      5 bytes
// prototype 
// callers   1   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)
*/

/*
Purpose: Turn the sprite into a one-pixel placeholder of the given extent.
// ORIGINAL: 0x005EAF3F ?create_blank@Sprite@@QAEHHHH@Z 0x005EAF3F-0x005EAF86 BYTE_EXACT
// LEVER: `iHeight_ = height` sits BETWEEN the two zero stores, not after
//   them. The image loads height into edx at the function's second
//   instruction and keeps it live across the whole body, so height's first
//   source use has to be early; but its STORE is the second-to-last. Slotting
//   the statement between `iLeftOffset_ = 0` and `iTopOffset_ = 0` does both
//   at once - the load hoists to the top and the store schedules down to
//   0x005EAF68, immediately before the depth byte store. Statement order
//   alone, no temps: 23/25 -> 25/25.
// size      71 bytes
// prototype int (__thiscall ?create_blank@Sprite@@QAEHHHH@Z)(Sprite* this, int, int, int)
// kind      game
// calls     0x005D4510
Return Value: No errors (0); allocation failed (4)
Status: Complete

PROMOTED FROM src/recovered/units/005eaf3f.cpp, which wrote every field as
`iWidth_`.

ONE BYTE OF PIXELS. The sheet is 1x1 and the extent is whatever the caller
asked for, so this is a placeholder that draws a single colour - and
`iSpriteWidth2_` takes the width while `iSpriteWidth_` takes 1, which is the
pair the drawing code reads as "sheet stride" and "frame width".
*/
int Sprite::create_blank(int width, int height, int depth) {
    iWidth_ = width;
    iSpriteWidth2_ = width;
    iSpriteWidth_ = 1;
    iSpriteHeight_ = 1;
    iLeftOffset_ = 0;
    iHeight_ = height;
    iTopOffset_ = 0;
    cTransparentIndex_ = static_cast<unsigned char>(depth);
    pcBits_ = static_cast<char *>(mem_get(1));
    return pcBits_ ? 0 : 4;
}

// ORIGINAL: 0x005EAF8F ?save@Sprite@@QAEHH@Z 0x005EAF8F-0x005EB0B2;0x00662C30-0x00662C42
// Writes the sprite to a Filemap-mapped file: six metadata ints, the
// transparency byte, the stride int - deliberately UNALIGNED, at offset
// 0x19 (the image stores it with mov after the byte, no alignment padding) -
// then the pixel block. The `0x2c` in the map size is the header the body
// actually ends at, not the on-disk field order. The stack-local Filemap is
// the image's SEH try/finally (0x00662C30/0x00662C38, __CxxFrameHandler):
// every early return runs its destructor without an explicit close.
// Calls 0x00628380 (ctor) 0x00628650 (create) 0x006287C0 (close)
// 0x006283E0 (dtor) 0x00645930 (memcpy).
int Sprite::save(int filename) {
    Filemap fm;
    if (pcBits_ == 0) {
        return 7;
    }
    char *map = static_cast<char *>(fm.create(
        reinterpret_cast<LPCSTR>(filename),
        static_cast<uint32_t>(iSpriteWidth_ * iSpriteHeight_ + 0x2c), 1));
    if (map == 0) {
        return 4;
    }
    char *cursor = map;
    *reinterpret_cast<int *>(cursor) = iSpriteWidth_;
    cursor += 4;
    *reinterpret_cast<int *>(cursor) = iSpriteHeight_;
    cursor += 4;
    *reinterpret_cast<int *>(cursor) = iWidth_;
    cursor += 4;
    *reinterpret_cast<int *>(cursor) = iHeight_;
    cursor += 4;
    *reinterpret_cast<int *>(cursor) = iLeftOffset_;
    cursor += 4;
    *reinterpret_cast<int *>(cursor) = iTopOffset_;
    cursor += 4;
    *cursor = static_cast<char>(cTransparentIndex_);
    cursor += 1;
    // The unaligned stride store the byte at 0x18 sets up.
    *reinterpret_cast<int *>(cursor) = iSpriteWidth2_;
    cursor += 4;
    memcpy(cursor, pcBits_,
           static_cast<uint32_t>(iSpriteHeight_) * iSpriteWidth2_);
    fm.close();
    return 0;
}

// ORIGINAL: 0x005EB0BF ?load@Sprite@@QAEHH@Z 0x005EB0BF-0x005EB205;0x00662C42-0x00662C54
// close()'s cleanup as the head (borrowed pixels survive it, owned pixels
// are released and charged back through 0x009B6618, the name is freed, and
// like close() the transparency byte at 0x08 is not among the zero stores),
// then the save() header parse in reverse into the EXISTING pcBits_. The image
// never allocates: memcpy targets whatever pcBits_ still holds, so a sprite
// without borrowed pixels hands memcpy a null destination - no guard exists
// in the original and none is invented here. The unaligned stride read at
// 0x19 mirrors save()'s store. Calls 0x00628380 (ctor) 0x00628540 (open)
// 0x006283E0 (dtor) 0x006287C0 (close) 0x00644EF2 (free) 0x00645930 (memcpy).
int Sprite::load(int filename) {
    Filemap fm;
    if (fObj1Exists_ == 0 && pcBits_ != 0) {
        SpriteMemoryUsed = static_cast<int>(
            static_cast<uint32_t>(SpriteMemoryUsed)
            - static_cast<uint32_t>(iSpriteHeight_)
                  * static_cast<uint32_t>(iSpriteWidth_));
        // close()'s second test of the same pointer, kept: the zero store
        // sits outside the inner guard in the image as well.
        if (pcBits_ != 0) {
            free(pcBits_);
        }
        pcBits_ = 0;
    }
    if (ppszFileName_ != 0) {
        free(ppszFileName_);
        ppszFileName_ = 0;
    }
    iSpriteWidth2_ = 0;
    iSpriteWidth_ = 0;
    iSpriteHeight_ = 0;
    iWidth_ = 0;
    iHeight_ = 0;
    iLeftOffset_ = 0;
    iTopOffset_ = 0;
    fObj1Exists_ = 0;
    const unsigned char *map = static_cast<const unsigned char *>(
        fm.open(reinterpret_cast<LPCSTR>(filename), 1));
    if (map == 0) {
        return 6;
    }
    iSpriteWidth_ = *reinterpret_cast<const int *>(map);
    map += 4;
    iSpriteHeight_ = *reinterpret_cast<const int *>(map);
    map += 4;
    iWidth_ = *reinterpret_cast<const int *>(map);
    map += 4;
    iHeight_ = *reinterpret_cast<const int *>(map);
    map += 4;
    iLeftOffset_ = *reinterpret_cast<const int *>(map);
    map += 4;
    iTopOffset_ = *reinterpret_cast<const int *>(map);
    map += 4;
    cTransparentIndex_ = *map;
    map += 1;
    iSpriteWidth2_ = *reinterpret_cast<const int *>(map);
    map += 4;
    memcpy(pcBits_, map,
           static_cast<uint32_t>(iSpriteWidth2_) * iSpriteHeight_);
    fm.close();
    return 0;
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
Sprite g_UNUSED_SPRITE_VAR02;  // 0x006A7130
Sprite g_UNUSED_SPRITE_VAR11;  // 0x006A72E0
Sprite g_UNUSED_SPRITE_VAR06;  // 0x006A71F0
Sprite g_UNUSED_SPRITE_VAR09;  // 0x006A7280
Sprite g_UNUSED_SPRITE_VAR21;  // 0x006A74C0
Sprite g_UNUSED_SPRITE_VAR05;  // 0x006A71C0
Sprite g_UNUSED_SPRITE_VAR08;  // 0x006A7250
Sprite g_UNUSED_SPRITE_VAR04;  // 0x006A7190
Sprite g_UNUSED_SPRITE_VAR01;  // 0x006A7100
Sprite g_UNUSED_SPRITE_VAR18;  // 0x006A7430
Sprite g_UNUSED_SPRITE_VAR03;  // 0x006A7160
Sprite g_UNUSED_SPRITE_VAR20;  // 0x006A7490
Sprite g_UNUSED_SPRITE_VAR16;  // 0x006A73D0
Sprite g_UNUSED_SPRITE_VAR14;  // 0x006A7370
Sprite g_UNUSED_SPRITE_VAR22;  // 0x006A74F0
Sprite g_UNUSED_SPRITE_VAR10;  // 0x006A72B0
Sprite g_UNUSED_SPRITE_VAR15;  // 0x006A73A0
Sprite g_UNUSED_SPRITE_VAR13;  // 0x006A7340
Sprite g_UNUSED_SPRITE_VAR17;  // 0x006A7400
Sprite g_UNUSED_SPRITE_VAR19;  // 0x006A7460
Sprite g_UNUSED_SPRITE_VAR12;  // 0x006A7310
Sprite g_UNUSED_SPRITE_VAR07;  // 0x006A7220
// Original CRT initializer 0x0044CCA0 constructs three 0x2C-byte Sprites;
// 0x0044CCD0 destroys the same array. DipEdit::init references elements 1
// and 2 at 0x00779504 / 0x00779530. Initial storage is separate from lifetime.
// RECOVERY_OBJECT: v2 0x007794D8 0x0077955C ?g_IFACE_CLOSE_X_SPRITES@@3PAVSprite@@A 0x004DB080
Sprite g_IFACE_CLOSE_X_SPRITES[3];
// Original CRT vector initializers 0x44CCF0,0x44CD40,...,0x44CF20 each
// construct 51 Sprites at stride 0x2C; their exits use the same extents.
// Each range is initially zero. Construction and asset extraction are separate.
// RECOVERY_OBJECT: v2 0x007AC290 0x007ACB54 ?g_IFACE_BOX_SPRITES1@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_BOX_SPRITES1[51];
// RECOVERY_OBJECT: v2 0x0076DC98 0x0076E55C ?g_IFACE_BOX_SPRITES2@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_BOX_SPRITES2[51];
// RECOVERY_OBJECT: v2 0x0078AE58 0x0078B71C ?g_IFACE_BOX_SPRITES3@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_BOX_SPRITES3[51];
// RECOVERY_OBJECT: v2 0x0078CC60 0x0078D524 ?g_IFACE_BOX_SPRITES4@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_BOX_SPRITES4[51];
// RECOVERY_OBJECT: v2 0x00779CB0 0x0077A574 ?g_IFACE_BOX_SPRITES5@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_BOX_SPRITES5[51];
// RECOVERY_OBJECT: v2 0x007A6978 0x007A723C ?g_IFACE_BOX_SPRITES6@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_BOX_SPRITES6[51];
// RECOVERY_OBJECT: v2 0x0078E0A0 0x0078E964 ?g_IFACE_BOX_SPRITES7@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_BOX_SPRITES7[51];
// RECOVERY_OBJECT: v2 0x0075B950 0x0075C214 ?g_IFACE_BOX_SPRITES8@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_BOX_SPRITES8[51];
// The eight original single-object initializers at 0x44CFC0..0x44D110
// call Sprite's constructor, whose final DWORD store is at +0x28.
// These 0x2C-byte ranges are initially zero; the loader supplies pixels.
// RECOVERY_OBJECT: v2 0x007921E8 0x00792214 ?g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE@@3VSprite@@A 0x00453A70
Sprite g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE;
// RECOVERY_OBJECT: v2 0x00779478 0x007794A4 ?g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE@@3VSprite@@A 0x00453A70
Sprite g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE;
// RECOVERY_OBJECT: v2 0x007AC1C0 0x007AC1EC ?g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE@@3VSprite@@A 0x00453A70
Sprite g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE;
// RECOVERY_OBJECT: v2 0x007AB400 0x007AB42C ?g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE@@3VSprite@@A 0x00453A70
Sprite g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE;
// RECOVERY_OBJECT: v2 0x007793F0 0x0077941C ?g_IFACE_STD_POPUPS_TOP_MID_SPRITE@@3VSprite@@A 0x00453A70
Sprite g_IFACE_STD_POPUPS_TOP_MID_SPRITE;
// RECOVERY_OBJECT: v2 0x007871D0 0x007871FC ?g_IFACE_STD_POPUPS_BOT_MID_SPRITE@@3VSprite@@A 0x00453A70
Sprite g_IFACE_STD_POPUPS_BOT_MID_SPRITE;
// RECOVERY_OBJECT: v2 0x007ACFB0 0x007ACFDC ?g_IFACE_STD_POPUPS_MID_LEFT_SPRITE@@3VSprite@@A 0x00453A70
Sprite g_IFACE_STD_POPUPS_MID_LEFT_SPRITE;
// RECOVERY_OBJECT: v2 0x0075B058 0x0075B084 ?g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE@@3VSprite@@A 0x00453A70
Sprite g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE;
// Original CRT 0044EB90 (table 006822DC) constructs 6 Sprite object(s);
// exit 0044EBC0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00791C58 0x00791D60 ?g_TER1_WHITE_ORG_YEL_TILE_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_TER1_WHITE_ORG_YEL_TILE_SPRITES[6];
// Original CRT 0044EBE0 (table 006822E0) constructs 9 Sprite object(s);
// exit 0044EC10 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007A99A0 0x007A9B2C ?g_TER1_BOTTOM_LEFT_TILE_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_TER1_BOTTOM_LEFT_TILE_SPRITES[9];
// Original CRT 0044EC30 (table 006822E4) constructs 2 Sprite object(s);
// exit 0044EC60 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00779420 0x00779478 ?g_TER1_UNUSED_SPRITES2@@3PAVSprite@@A 0x00451C50
Sprite g_TER1_UNUSED_SPRITES2[2];
// Original CRT 0044EC80 (table 006822E8) constructs 1 Sprite object(s);
// exit 0044ECA0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00776A50 0x00776A7C ?g_TER1_MINE_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_MINE_SPRITE;
// Original CRT 0044ECB0 (table 006822EC) constructs 1 Sprite object(s);
// exit 0044ECD0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007991F8 0x00799224 ?g_TER1_SOLAR_COLLECTOR_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_SOLAR_COLLECTOR_SPRITE;
// Original CRT 0044ECE0 (table 006822F0) constructs 1 Sprite object(s);
// exit 0044ED00 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0078A5B0 0x0078A5DC ?g_TER1_TIDAL_HARNESS_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_TIDAL_HARNESS_SPRITE;
// Original CRT 0044ED10 (table 006822F4) constructs 1 Sprite object(s);
// exit 0044ED30 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00789B68 0x00789B94 ?g_TER1_MINING_PLATFORM_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_MINING_PLATFORM_SPRITE;
// Original CRT 0044ED40 (table 006822F8) constructs 1 Sprite object(s);
// exit 0044ED60 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00798C40 0x00798C6C ?g_TER1_TUT_BLANK_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_TUT_BLANK_SPRITE;
// Original CRT 0044ED70 (table 006822FC) constructs 1 Sprite object(s);
// exit 0044ED90 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007A6860 0x007A688C ?g_TER1_KELP_FARM_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_KELP_FARM_SPRITE;
// Original CRT 0044EDA0 (table 00682300) constructs 1 Sprite object(s);
// exit 0044EDC0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00779390 0x007793BC ?g_TER1_CONDENSER_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_CONDENSER_SPRITE;
// Original CRT 0044EDD0 (table 00682304) constructs 1 Sprite object(s);
// exit 0044EDF0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007ABF60 0x007ABF8C ?g_TER1_ECHELON_MIRROR_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_ECHELON_MIRROR_SPRITE;
// Original CRT 0044EE00 (table 00682308) constructs 1 Sprite object(s);
// exit 0044EE20 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0078DD20 0x0078DD4C ?g_TER1_BOREHOLE_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_BOREHOLE_SPRITE;
// Original CRT 0044EE30 (table 0068230C) constructs 1 Sprite object(s);
// exit 0044EE50 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007ACB88 0x007ACBB4 ?g_TER1_BOREHOLE_CLUSTER_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_BOREHOLE_CLUSTER_SPRITE;
// Original CRT 0044EE60 (table 00682310) constructs 6 Sprite object(s);
// exit 0044EE90 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007AC098 0x007AC1A0 ?g_TER1_MANIFOLD_NEXUS_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_TER1_MANIFOLD_NEXUS_SPRITES[6];
// Original CRT 0044EEB0 (table 00682314) constructs 15 Sprite object(s);
// exit 0044EEE0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00776758 0x007769EC ?g_TER1WRECK_UNITY_WRECKAGE_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_TER1WRECK_UNITY_WRECKAGE_SPRITES[15];
// Original CRT 0044EF00 (table 00682318) constructs 4 Sprite object(s);
// exit 0044EF30 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00799658 0x00799708 ?g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES[4];
// Original CRT 0044EF50 (table 0068231C) constructs 6 Sprite object(s);
// exit 0044EF80 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007ACD70 0x007ACE78 ?g_FOSSIL_FIELD_RIDGE_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_FOSSIL_FIELD_RIDGE_SPRITES[6];
// Original CRT 0044EFA0 (table 00682320) constructs 5 Sprite object(s);
// exit 0044EFD0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007792B0 0x0077938C ?g_TER1_UNUSED_SPRITES1@@3PAVSprite@@A 0x00451C50
Sprite g_TER1_UNUSED_SPRITES1[5];
// Original CRT 0044EFF0 (table 00682324) constructs 5 Sprite object(s);
// exit 0044F020 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00791FC8 0x007920A4 ?g_TER1_FARM_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_TER1_FARM_SPRITES[5];
// Original CRT 0044F040 (table 00682328) constructs 5 Sprite object(s);
// exit 0044F070 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0079A710 0x0079A7EC ?g_TER1_SOIL_ENRICHER_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_TER1_SOIL_ENRICHER_SPRITES[5];
// Original CRT 0044F090 (table 0068232C) constructs 12 Sprite object(s);
// exit 0044F0C0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0075B230 0x0075B440 ?g_TER1_SEA_LAND_RESOURCE_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_TER1_SEA_LAND_RESOURCE_SPRITES[12];
// Original CRT 0044F0E0 (table 00682330) constructs 6 Sprite object(s);
// exit 0044F110 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0078A650 0x0078A758 ?g_TER1_LANDMARK_RESOURCE_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_TER1_LANDMARK_RESOURCE_SPRITES[6];
// Original CRT 0044F130 (table 00682334) constructs 2 Sprite object(s);
// exit 0044F160 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x00779770 0x007797C8 ?g_GLOW_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_GLOW_SPRITES[2];
// Original CRT 0044F180 (table 00682338) constructs 6 Sprite object(s);
// exit 0044F1B0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0077AFF8 0x0077B100 ?g_TER1_UNITY_POD_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_TER1_UNITY_POD_SPRITES[6];
// Original CRT 0044F1D0 (table 0068233C) constructs 1 Sprite object(s);
// exit 0044F1F0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x0075B098 0x0075B0C4 ?g_TER1_MONOLITH_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_MONOLITH_SPRITE;
// Original CRT 0044F200 (table 00682340) constructs 1 Sprite object(s);
// exit 0044F220 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007AD010 0x007AD03C ?g_TER1_BUNKER_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_BUNKER_SPRITE;
// Original CRT 0044F230 (table 00682344) constructs 1 Sprite object(s);
// exit 0044F250 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007ACC60 0x007ACC8C ?g_TER1_AIRBASE_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_AIRBASE_SPRITE;
// Original CRT 0044F260 (table 00682348) constructs 1 Sprite object(s);
// exit 0044F280 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007A6830 0x007A685C ?g_TER1_SENSOR_ARRAY_SPRITE@@3VSprite@@A 0x00451C50
Sprite g_TER1_SENSOR_ARRAY_SPRITE;
// Original CRT 0044F290 (table 0068234C) constructs 2 Sprite object(s);
// exit 0044F2C0 uses the same extent. Initial zero storage is not lifetime proof.
// RECOVERY_OBJECT: v2 0x007ACBB8 0x007ACC10 ?g_RAINFALL_DOUBLE_TILE_SPRITES@@3PAVSprite@@A 0x00451C50
Sprite g_RAINFALL_DOUBLE_TILE_SPRITES[2];
// CRT 44F2E0 constructs152 Sprites; 44F310 destroys the same 0x2C-stride bank.
// RECOVERY_OBJECT: v2 0x007777A0 0x007791C0 ?g_VEH_SPRITES@@3PAVSprite@@A 0x00454270
Sprite g_VEH_SPRITES[152];
// CRT 44F330/44F360 construct/destroy112 Sprites: fourteen for each faction.
// RECOVERY_OBJECT: v2 0x0078B778 0x0078CAB8 ?g_FLAGS_VEH_SPRITES@@3PAVSprite@@A 0x00453F70
Sprite g_FLAGS_VEH_SPRITES[112];
// Original CRT 0x682358 constructs 16 Sprite(s) via 0x44F380
// and registers 0x44F3B0 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x0075B450 0x0075B710 ?g_ICONS_GENERAL_SPRITES@@3PAVSprite@@A 0x004547E0
Sprite g_ICONS_GENERAL_SPRITES[16];  // 0x0075B450, 0x2c stride
// Original CRT 0x68235C constructs 32 Sprite(s) via 0x44F3D0
// and registers 0x44F400 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x007A72A0 0x007A7820 ?g_RESOURCE_ICON_SPRITES@@3PAVSprite@@A 0x004547E0
Sprite g_RESOURCE_ICON_SPRITES[32];  // 0x007A72A0, 0x2c stride
// RECOVERY_OBJECT: v2 0x0078CAD0 0x0078CC30 ?g_CITIZEN_LG_CURSOR_SPRITES@@3PAVSprite@@A 0x00454FC0
Sprite g_CITIZEN_LG_CURSOR_SPRITES[8];  // 0x0078CAD0, 0x2c stride
// RECOVERY_OBJECT: v2 0x007765C0 0x007766F4 ?g_SPECIALIST_LG_CURSOR_SPRITES@@3PAVSprite@@A 0x00454FC0
Sprite g_SPECIALIST_LG_CURSOR_SPRITES[7];  // 0x007765C0, 0x2c stride
// RECOVERY_OBJECT: v2 0x00776420 0x00776580 ?g_CITIZEN_SM_CURSOR_SPRITES@@3PAVSprite@@A 0x00454FC0
Sprite g_CITIZEN_SM_CURSOR_SPRITES[8];  // 0x00776420, 0x2c stride
// RECOVERY_OBJECT: v2 0x00779B78 0x00779CAC ?g_SPECIALIST_SM_CURSOR_SPRITES@@3PAVSprite@@A 0x00454FC0
Sprite g_SPECIALIST_SM_CURSOR_SPRITES[7];  // 0x00779B78, 0x2c stride
// RECOVERY_OBJECT: v2 0x007AD1B0 0x007AD260 ?g_AL_CITIZEN_LG_CURSOR_SPRITES@@3PAVSprite@@A 0x00454FC0
Sprite g_AL_CITIZEN_LG_CURSOR_SPRITES[4];  // 0x007AD1B0, 0x2c stride
// RECOVERY_OBJECT: v2 0x00798CD0 0x00798E04 ?g_AL_SPECIALIST_LG_CURSOR_SPRITES@@3PAVSprite@@A 0x00454FC0
Sprite g_AL_SPECIALIST_LG_CURSOR_SPRITES[7];  // 0x00798CD0, 0x2c stride
// RECOVERY_OBJECT: v2 0x0075B180 0x0075B230 ?g_AL_CITIZEN_SM_CURSOR_SPRITES@@3PAVSprite@@A 0x00454FC0
Sprite g_AL_CITIZEN_SM_CURSOR_SPRITES[4];  // 0x0075B180, 0x2c stride
// RECOVERY_OBJECT: v2 0x0075B710 0x0075B844 ?g_AL_SPECIALIST_SM_CURSOR_SPRITES@@3PAVSprite@@A 0x00454FC0
Sprite g_AL_SPECIALIST_SM_CURSOR_SPRITES[7];  // 0x0075B710, 0x2c stride
// Original CRT 0x682380 constructs 1 Sprite(s) via 0x44F6A0
// and registers 0x44F6C0 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x00791F98 0x00791FC4 ?g_RED_ALIEN_HEAD_ICON_SPRITE@@3VSprite@@A 0x004547E0
Sprite g_RED_ALIEN_HEAD_ICON_SPRITE;  // 0x00791F98
// Original CRT 0x682384 constructs 4 Sprite(s) via 0x44F6D0
// and registers 0x44F700 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x0075AE20 0x0075AED0 ?g_SILVER_MENU_ICON_SPRITES@@3PAVSprite@@A 0x004547E0
Sprite g_SILVER_MENU_ICON_SPRITES[4];  // 0x0075AE20, 0x2c stride
// Original CRT 0x682388 constructs 2 Sprite(s) via 0x44F720
// and registers 0x44F750 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x0075AF68 0x0075AFC0 ?g_SILVER_CHECKBOX_ICON_SPRITES@@3PAVSprite@@A 0x004547E0
Sprite g_SILVER_CHECKBOX_ICON_SPRITES[2];  // 0x0075AF68, 0x2c stride
// Original CRT 0x68238C constructs 1 Sprite(s) via 0x44F770
// and registers 0x44F790 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x0075AD88 0x0075ADB4 ?g_RED_MALE_HEAD_ICON_SPRITE@@3VSprite@@A 0x004547E0
Sprite g_RED_MALE_HEAD_ICON_SPRITE;  // 0x0075AD88
// Original CRT 0x682390 constructs 1 Sprite(s) via 0x44F7A0
// and registers 0x44F7C0 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x0078A520 0x0078A54C ?g_NULL_RESOURCE_ICON_SPRITE@@3VSprite@@A 0x004547E0
Sprite g_NULL_RESOURCE_ICON_SPRITE;  // 0x0078A520
// Original CRT 0x682394 constructs 2 Sprite(s) via 0x44F7D0
// and registers 0x44F800 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x0075AF10 0x0075AF68 ?g_PEACE_SIGN_SPRITES@@3PAVSprite@@A 0x004547E0
Sprite g_PEACE_SIGN_SPRITES[2];  // 0x0075AF10, 0x2c stride
// Original CRT 0x682398 constructs 1 Sprite(s) via 0x44F820
// and registers 0x44F840 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x00776728 0x00776754 ?g_ICON_TILE_SQUARE_SPRITE@@3VSprite@@A 0x004547E0
Sprite g_ICON_TILE_SQUARE_SPRITE;  // 0x00776728
// CRT 44F850/44F880 construct/destroy144 Sprites at stride0x2C.
// RECOVERY_OBJECT: v2 0x007A9B30 0x007AB3F0 ?g_XI_BOOM_VEH_SPRITES@@3PAVSprite@@A 0x00454270
Sprite g_XI_BOOM_VEH_SPRITES[144];
// CRT 44F8A0/44F8D0 construct/destroy64 Sprites. The loader's moving
// bottom-row sentinel7AC0C0 is not the end of this object's storage.
// RECOVERY_OBJECT: v2 0x007AB460 0x007ABF60 ?g_XF_BOOM_VEH_SPRITES@@3PAVSprite@@A 0x00454270
Sprite g_XF_BOOM_VEH_SPRITES[64];
// Loader454D10 identifies these owners; the pinned CRT table6823A4..6823C8
// selects their constructor wrappers in this order. Each uses Sprite2C;
// array wrappers pass the stated count/stride to the CRT vector helpers.
// Constructor/destructor pairs below prove original bounds/lifetime intent,
// not current linked initializer ordering or successful artwork loading.
// Native ctor44F8F0 / dtor44F910, one Sprite.
// RECOVERY_OBJECT: v2 0x00779200 0x0077922C ?g_BATTLE_MIND_WORM_SPRITE@@3VSprite@@A 0x00454D10
Sprite g_BATTLE_MIND_WORM_SPRITE;  // 0x00779200
// Native ctor44F920 / dtor44F940, one Sprite.
// RECOVERY_OBJECT: v2 0x0076DC30 0x0076DC5C ?g_BATTLE_ISLE_DEEP_SPRITE@@3VSprite@@A 0x00454D10
Sprite g_BATTLE_ISLE_DEEP_SPRITE;  // 0x0076DC30
// Native ctor44F950 / dtor44F970, one Sprite.
// RECOVERY_OBJECT: v2 0x0078A4A0 0x0078A4CC ?g_BATTLE_LOCUSTS_CHIRON_SPRITE@@3VSprite@@A 0x00454D10
Sprite g_BATTLE_LOCUSTS_CHIRON_SPRITE;  // 0x0078A4A0
// Native ctor44F980 / dtor44F9A0, one Sprite.
// RECOVERY_OBJECT: v2 0x00787E70 0x00787E9C ?g_BATTLE_FUNGAL_TOWER_SPRITE@@3VSprite@@A 0x00454D10
Sprite g_BATTLE_FUNGAL_TOWER_SPRITE;  // 0x00787E70
// Native ctor44F9B0 / dtor44F9D0, one Sprite.
// RECOVERY_OBJECT: v2 0x007AC000 0x007AC02C ?g_BATTLE_SPORE_LAUNCHER_SPRITE@@3VSprite@@A 0x00454D10
Sprite g_BATTLE_SPORE_LAUNCHER_SPRITE;  // 0x007AC000
// Native ctor44F9E0 / dtor44FA00, one Sprite.
// RECOVERY_OBJECT: v2 0x00779570 0x0077959C ?g_BATTLE_SEALURK_SPRITE@@3VSprite@@A 0x00454D10
Sprite g_BATTLE_SEALURK_SPRITE;  // 0x00779570
// Native ctor44FA10 / dtor44FA40, 89 Sprites at stride2C.
// RECOVERY_OBJECT: v2 0x00759E28 0x0075AD74 ?g_TECH_ICON_SPRITES@@3PAVSprite@@A 0x00454D10
Sprite g_TECH_ICON_SPRITES[89];  // 0x00759E28, 0x2c stride
// Native ctor44FA60 / dtor44FA90, 70 Sprites at stride2C.
// RECOVERY_OBJECT: v2 0x00787200 0x00787E08 ?g_FACILITY_ICON_SPRITES@@3PAVSprite@@A 0x00454D10
Sprite g_FACILITY_ICON_SPRITES[70];  // 0x00787200, 0x2c stride
// Native ctor44FAB0 / dtor44FAE0, 64 Sprites at stride2C.
// RECOVERY_OBJECT: v2 0x00759320 0x00759E20 ?g_SECRET_PROJECT_ICON_SPRITES@@3PAVSprite@@A 0x00454D10
Sprite g_SECRET_PROJECT_ICON_SPRITES[64];  // 0x00759320, 0x2c stride
// Original 0x44FB00/0x44FB30 construct/destroy 3 elements at stride 0x2C.
// RECOVERY_OBJECT: v2 0x00788038 0x007880BC ?g_IFACE_MP_COMBO_ARROW_SPRITES@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_MP_COMBO_ARROW_SPRITES[3];
// Original CRT 0x6823CC constructs 12 Sprite(s) via 0x44FB50
// and registers 0x44FB80 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x00791D88 0x00791F98 ?g_SCROLL_BAR_ARROW_ICON_SPRITES@@3PAVSprite@@A 0x004547E0
Sprite g_SCROLL_BAR_ARROW_ICON_SPRITES[12];  // 0x00791D88, 0x2c stride
// Original CRT 0x6823D0 constructs 12 Sprite(s) via 0x44FBA0
// and registers 0x44FBD0 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x0078DE30 0x0078E040 ?g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES@@3PAVSprite@@A 0x004547E0
Sprite g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES[12];  // 0x0078DE30, 0x2c stride
// Original CRT 0x6823D4 constructs 1 Sprite(s) via 0x44FBF0
// and registers 0x44FC10 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x0079A5E8 0x0079A614 ?g_SCROLL_BAR_FILLER_ICON_SPRITES@@3VSprite@@A 0x004547E0
Sprite g_SCROLL_BAR_FILLER_ICON_SPRITES;  // 0x0079A5E8
// Original CRT 0x6823D8 constructs 1 Sprite(s) via 0x44FC20
// and registers 0x44FC40 for teardown; element extent is 0x2C.
// RECOVERY_OBJECT: v2 0x007796B0 0x007796DC ?g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE@@3VSprite@@A 0x004547E0
Sprite g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE;  // 0x007796B0
// Original 0x44FC50/0x44FC80 construct/destroy 2 elements at stride 0x2C.
// RECOVERY_OBJECT: v2 0x007AC040 0x007AC098 ?g_IFACE_LOCK_SPRITES@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_LOCK_SPRITES[2];
Sprite g_UNUSED_SPRITES_VAR01[8];  // 0x007A7940, 0x2c stride
Sprite g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE;  // 0x0078DD50
Sprite g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE;  // 0x00789BC8
Sprite g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE;  // 0x00776580
Sprite g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE;  // 0x007ABFD0
Sprite g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE;  // 0x00787F88
Sprite g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE;  // 0x0078A2D0
Sprite g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE;  // 0x0078E070
Sprite g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE;  // 0x0078A550
Sprite g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE;  // 0x00789BF8
Sprite g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE;  // 0x0075B150
Sprite g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE;  // 0x007AD180
Sprite g_UNUSED_SPRITE_VAR23;  // 0x007880D0
Sprite g_UNUSED_SPRITE_VAR24;  // 0x0079A5B8
Sprite g_UNUSED_SPRITE_VAR25;  // 0x007797C8
Sprite g_UNUSED_SPRITE_VAR26;  // 0x00792638
Sprite g_UNUSED_SPRITE_VAR27;  // 0x0079A650
Sprite g_UNUSED_SPRITE_VAR28;  // 0x00799708
Sprite g_UNUSED_SPRITE_VAR29;  // 0x007ACFE0
Sprite g_UNUSED_SPRITE_VAR30;  // 0x007A7910
Sprite g_UNUSED_SPRITE_VAR31;  // 0x00779240
Sprite g_UNUSED_SPRITE_VAR32;  // 0x007ACD00
Sprite g_UNUSED_SPRITE_VAR33;  // 0x00776A20
Sprite g_UNUSED_SPRITE_VAR34;  // 0x007769F0
Sprite g_UNUSED_SPRITE_VAR35;  // 0x007796E0
Sprite g_UNUSED_SPRITE_VAR36;  // 0x007AC1F0
Sprite g_UNUSED_SPRITE_VAR37;  // 0x00789A60
Sprite g_UNUSED_SPRITE_VAR38;  // 0x0078CC30
Sprite g_UNUSED_SPRITE_VAR39;  // 0x00787F58
Sprite g_UNUSED_SPRITE_VAR40;  // 0x00798C00
Sprite g_UNUSED_SPRITE_VAR41;  // 0x0078A580
Sprite g_UNUSED_SPRITE_VAR42;  // 0x007AD0F0
Sprite g_UNUSED_SPRITE_VAR43;  // 0x007AD080
Sprite g_UNUSED_SPRITE_VAR44;  // 0x00792608
Sprite g_UNUSED_SPRITE_VAR45;  // 0x00787EA0
Sprite g_UNUSED_SPRITE_VAR46;  // 0x007ACCD0
// Original 0x450380/0x4503B0 construct/destroy 3 elements at stride 0x2C.
// RECOVERY_OBJECT: v2 0x0075AFD0 0x0075B054 ?g_IFACE_TECH_TREE_ARROW_SPRITES@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_TECH_TREE_ARROW_SPRITES[3];
Sprite g_UNUSED_SPRITE_VAR47;  // 0x00779740
Sprite g_UNUSED_SPRITE_VAR48;  // 0x0078A4D0
Sprite g_UNUSED_SPRITE_VAR49;  // 0x007AD140
Sprite g_UNUSED_SPRITE_VAR50;  // 0x007793C0
Sprite g_UNUSED_SPRITE_VAR51;  // 0x00789B98
Sprite g_UNUSED_SPRITE_VAR52;  // 0x007766F8
Sprite g_UNUSED_SPRITE_VAR53;  // 0x00798C70
Sprite g_UNUSED_SPRITE_VAR54;  // 0x0076EBA0
Sprite g_UNUSED_SPRITE_VAR55;  // 0x007592E0
Sprite g_UNUSED_SPRITE_VAR56;  // 0x007795C0
Sprite g_UNUSED_SPRITE_VAR57;  // 0x00792130
Sprite g_UNUSED_SPRITE_VAR58;  // 0x0075ADF0
Sprite g_UNUSED_SPRITE_VAR59;  // 0x0076E980
Sprite g_UNUSED_SPRITE_VAR60;  // 0x0078A310
Sprite g_UNUSED_SPRITE_VAR61;  // 0x0076DC60
Sprite g_UNUSED_SPRITE_VAR62;  // 0x00789B18
Sprite g_UNUSED_SPRITE_VAR63;  // 0x007A68C0
Sprite g_UNUSED_SPRITE_VAR64;  // 0x0077A588
Sprite g_UNUSED_SPRITE_VAR65;  // 0x00779710
Sprite g_UNUSED_SPRITE_VAR66;  // 0x0079A6C0
Sprite g_UNUSED_SPRITE_VAR67;  // 0x0078DDF0
Sprite g_UNUSED_SPRITE_VAR68;  // 0x007A7250
Sprite g_UNUSED_SPRITE_VAR69;  // 0x007AD0B0
Sprite g_UNUSED_SPRITE_VAR70;  // 0x007AD050
Sprite g_UNUSED_SPRITE_VAR71;  // 0x007A6890
Sprite g_UNUSED_SPRITE_VAR72;  // 0x007763F0
Sprite g_UNUSED_SPRITE_VAR73;  // 0x007ACD30
Sprite g_UNUSED_SPRITE_VAR74;  // 0x007ABF90
Sprite g_UNUSED_SPRITES_VAR02[3];  // 0x007A68F0, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR03[3];  // 0x00789A90, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR04[3];  // 0x00792160, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR05[3];  // 0x0075B0C8, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR06[3];  // 0x0075B8C8, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR07[3];  // 0x00787ED0, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR08[3];  // 0x007ACE80, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR09[3];  // 0x007920A8, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR10[3];  // 0x007ACF08, 0x2c stride
Sprite g_UNUSED_SPRITE_VAR75;  // 0x007794A8
Sprite g_UNUSED_SPRITE_VAR76;  // 0x00779670
Sprite g_UNUSED_SPRITE_VAR77;  // 0x007AB430
Sprite g_UNUSED_SPRITE_VAR78;  // 0x00798CA0
Sprite g_UNUSED_SPRITE_VAR79;  // 0x0078E040
Sprite g_UNUSED_SPRITE_VAR80;  // 0x007A67F0
Sprite g_UNUSED_SPRITE_VAR81;  // 0x00779618
Sprite g_UNUSED_SPRITE_VAR82;  // 0x007ACB58
Sprite g_UNUSED_SPRITE_VAR83;  // 0x007ACC20
Sprite g_BASEWIN_SPRITES[27];  // 0x0077A5C8, 0x2c stride
// Original 0x471350/0x471380 use a one-element vector at stride 0x2C.
// RECOVERY_OBJECT: v2 0x007F67C8 0x007F67F4 ?g_IFACE_GREEN_RIGHT_ARROW_SPRITE@@3PAVSprite@@A 0x00453A70
Sprite g_IFACE_GREEN_RIGHT_ARROW_SPRITE[1];
// RECOVERY_OBJECT: v2 0x0093AA70 0x0093AC80 ?g_CURSOR_SPRITES@@3PAVSprite@@A 0x00454FC0
Sprite g_CURSOR_SPRITES[12];  // 0x0093AA70, 0x2c stride
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

// ORIGINAL: 0x00634FE0 ?shutdown@TexHeap@@QAEXXZ 0x00634FE0-0x00635046 BYTE_EXACT
// size      102 bytes
// prototype void (__thiscall ?shutdown@TexHeap@@QAEXXZ)(TexHeap* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2
// 0x00634FE0  ?shutdown@TexHeap@@QAEXXZ  ->  ?shutdown@TexHeap@@QAEXXZ
//
// Purpose: cut the whole sheet heap down - the base block, then the block
// TABLE (count_ + 1 Heap entries, each released before the table itself),
// then the three counters.


void TexHeap::shutdown() {
    if (base_ != 0) {
        free(base_);
        base_ = 0;
    }

    if (blocks_ != 0) {
        // The image re-reads count_ for the loop test at every iteration, so
        // the bound stays an expression over the field rather than a local.
        for (int index = 0; index < static_cast<int>(count_) + 1; ++index) {
            Heap *block = blocks_[index];
            if (block != 0) {
                free(block);
                blocks_[index] = 0;
            }
        }
        if (blocks_ != 0) {
            free(blocks_);
        }
        blocks_ = 0;
    }

    count_ = 0;
    capacity_ = 0;
}

/*
Purpose: Draw into a destination buffer with scale ratio x/y for this call, then
         restoring what was there. The five-argument overload does the work.
// ORIGINAL: 0x005E6652 ?draw_dest@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z 0x005E6652-0x005E66A2 BYTE_EXACT
// symbol    ?draw_dest@Sprite@@QAEHPAVBuffer@@HHHHHPAE@Z
// size      80 bytes
// prototype int (__thiscall ?draw_dest@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z)(Sprite* this, Buffer*, int, int, int, int, int, unsigned int8*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E5833
Status: Complete
*/
int Sprite::draw_dest(Buffer *buffer, int a, int b, int c, int x, int y,
                      unsigned char *bits) {
    const int saved_numerator = SpriteScaleNumerator;
    const int saved_denominator = SpriteScaleDenominator;
    SpriteScaleNumerator = x;
    SpriteScaleDenominator = y;
    const int result = draw_dest(buffer, a, b, c, bits);
    SpriteScaleDenominator = saved_denominator;
    SpriteScaleNumerator = saved_numerator;
    return result;
}

/*
Purpose: Draw into a destination buffer through the unk2 worker with ratio x/y,
         substituting the pair into the class scale for the duration of
         the call. Byte for byte the same wrapper as draw_dest above.
// ORIGINAL: 0x005E816C ?draw_dest_unk2@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z 0x005E816C-0x005E81BC BYTE_EXACT
// symbol    ?draw_dest_unk2@Sprite@@QAEHPAVBuffer@@HHHHHPAE@Z
// size      80 bytes
// prototype int (__thiscall ?draw_dest_unk2@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z)(Sprite* this, Buffer*, int, int, int, int, int, unsigned int8*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E81BC
Status: Complete
*/
int Sprite::draw_dest_unk2(Buffer *buffer, int a, int b, int c, int x, int y,
                           unsigned char *bits) {
    const int saved_numerator = SpriteScaleNumerator;
    const int saved_denominator = SpriteScaleDenominator;
    SpriteScaleNumerator = x;
    SpriteScaleDenominator = y;
    const int result = draw_dest_unk2(buffer, a, b, c, bits);
    SpriteScaleDenominator = saved_denominator;
    SpriteScaleNumerator = saved_numerator;
    return result;
}

/*
Purpose: Draw into a destination buffer through the multi-table worker with
         ratio x/y, substituting the pair into the class scale for the
         duration of the call. Byte for byte the same wrapper as draw_dest
         above.
// ORIGINAL: 0x005E8FA5 ?draw_multi_table_dest@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z 0x005E8FA5-0x005E8FF5 BYTE_EXACT
// symbol    ?draw_multi_table_dest@Sprite@@QAEHPAVBuffer@@HHHHHPAE@Z
// size      80 bytes
// prototype int (__thiscall ?draw_multi_table_dest@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z)(Sprite* this, Buffer*, int, int, int, int, int, unsigned int8*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E8FF5
Status: Complete
*/
int Sprite::draw_multi_table_dest(Buffer *buffer, int a, int b, int c, int x,
                                  int y, unsigned char *bits) {
    const int saved_numerator = SpriteScaleNumerator;
    const int saved_denominator = SpriteScaleDenominator;
    SpriteScaleNumerator = x;
    SpriteScaleDenominator = y;
    const int result = draw_multi_table_dest(buffer, a, b, c, bits);
    SpriteScaleDenominator = saved_denominator;
    SpriteScaleNumerator = saved_numerator;
    return result;
}

// The icon and cursor loaders reference distinct original NEWICONS.PCX
// storage. Keep this loader beside its assets, separate from game.cpp
// literal pooling; this does not assert a historical TU boundary.
// Populate the existing resource/control icon objects from three shared
// sheets. Startup through load_sprites remains a separate recovery dependency.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004547E0 ?load_icons@@YAXXZ 0x004547E0-0x00454D0C
// RECOVERY_BINDING: v1 {"evidence": ["Full original4547E0..454D0C and retained seed src/recovered/units/004547e0.cpp establish the single no-argument loader, three sheet loads, recolor pairs before every extraction, all receivers/rectangles and unconditional terminal close.", "All service results are ignored. Original454B04 resets the arrow-column coordinate to104 before filler extraction. Menu and checkbox receivers belong to existing arrays, including checkbox element1 rather than separate75AF94 storage.", "Original per-asset CRT constructors/teardowns independently establish element counts and44-byte stride. Existing load_sprites startup and raw caption-icon publisher are separate unfinished ownership dependencies, not proof supplied by this binding."], "reason": "Complete original icon-sheet loader uses the existing shared PCX Buffer and thirteen existing Sprite groups. Recover ordinary C++ object/array accesses without original-address aliases or replacement assets."}
void __cdecl load_icons() {
    Buffer &buffer = g_PCX_PARSE_TEMP_BUFFER1;
    Sprite *icon;
    int source_x;
    int source_y;
    int columns_left;

    // Pinned literals: 685064 NEWICONS.PCX; 685074 TER1WRECK.PCX;
    // 685084 ICONS.PCX. All loads use palette 0, base 10, count 236.
    // There is no DirectDraw guard here, unlike load_cursors.
    // RECOVERY_LITERAL: v1 0x00685064 0x00685071 ??_C@_0N@NBMC@NEWICONS?4PCX?$AA@ 0x004547E0
    buffer.load_pcx("NEWICONS.PCX", 0, 10, 236);
    icon = g_ICONS_GENERAL_SPRITES;
    source_y = 13;
    do {
        source_x = 1;
        columns_left = 4;
        do {
            buffer.change_color(263, 265);
            buffer.change_color(262, 265);
            icon->extract(&buffer, 265, source_x, source_y, 22, 22, 0);
            source_x += 23;
            ++icon;
        } while (--columns_left);
        source_y += 23;
    } while (icon < g_ICONS_GENERAL_SPRITES + 16);

    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_NULL_RESOURCE_ICON_SPRITE.extract(&buffer, 265, 1, 174, 22, 22, 0);

    icon = g_PEACE_SIGN_SPRITES;
    for (source_y = 220; icon < g_PEACE_SIGN_SPRITES + 2; source_y += 23) {
        buffer.change_color(263, 265);
        buffer.change_color(262, 265);
        icon->extract(&buffer, 265, 1, source_y, 22, 22, 0);
        ++icon;
    }

    icon = g_RESOURCE_ICON_SPRITES;
    source_y = 304;
    do {
        source_x = 174;
        columns_left = 8;
        do {
            buffer.change_color(263, 265);
            buffer.change_color(262, 265);
            icon->extract(&buffer, 265, source_x, source_y, 40, 40, 0);
            source_x += 41;
            ++icon;
        } while (--columns_left);
        source_y += 41;
    } while (icon < g_RESOURCE_ICON_SPRITES + 32);

    // RECOVERY_LITERAL: v1 0x00685074 0x00685082 ??_C@_0O@CAND@TER1WRECK?4PCX?$AA@ 0x004547E0
    buffer.load_pcx("TER1WRECK.PCX", 0, 10, 236);
    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_RED_ALIEN_HEAD_ICON_SPRITE.extract(&buffer, 265, 768, 148, 28, 42, 0);

    // RECOVERY_LITERAL: v1 0x00685084 0x0068508E ??_C@_09OIPD@ICONS?4PCX?$AA@ 0x004547E0
    buffer.load_pcx("ICONS.PCX", 0, 10, 236);
    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_ICON_TILE_SQUARE_SPRITE.extract(&buffer, 265, 1, 1, 100, 50, 0);

    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_RED_MALE_HEAD_ICON_SPRITE.extract(&buffer, 265, 46, 303, 33, 30, 0);

    icon = g_SCROLL_BAR_ARROW_ICON_SPRITES;
    source_y = 1;
    do {
        source_x = 104;
        columns_left = 3;
        do {
            buffer.change_color(263, 265);
            buffer.change_color(262, 265);
            icon->extract(&buffer, 265, source_x, source_y, 24, 24, 0);
            source_x += 25;
            ++icon;
        } while (--columns_left);
        source_y += 25;
    } while (icon < g_SCROLL_BAR_ARROW_ICON_SPRITES + 12);

    // Original EDI is reset to 104 after the final arrow row; the filler
    // source X is not the previous row's final icon or its one-past position.
    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_SCROLL_BAR_FILLER_ICON_SPRITES.extract(&buffer, 265, 104, 101, 18, 18, 0);

    icon = g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES;
    source_y = 1;
    do {
        source_x = 192;
        columns_left = 3;
        do {
            buffer.change_color(263, 265);
            buffer.change_color(262, 265);
            icon->extract(&buffer, 265, source_x, source_y, 10, 10, 0);
            source_x += 11;
            ++icon;
        } while (--columns_left);
        source_y += 11;
    } while (icon < g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES + 12);

    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE.extract(&buffer, 265, 225, 1, 10, 10, 0);

    icon = g_SILVER_MENU_ICON_SPRITES;
    for (source_x = 320; icon < g_SILVER_MENU_ICON_SPRITES + 4; source_x += 16) {
        buffer.change_color(263, 265);
        buffer.change_color(262, 265);
        icon->extract(&buffer, 265, source_x, 1, 15, 15, 0);
        ++icon;
    }

    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_SILVER_CHECKBOX_ICON_SPRITES[0].extract(&buffer, 265, 320, 20, 16, 16, 0);

    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_SILVER_CHECKBOX_ICON_SPRITES[1].extract(&buffer, 265, 337, 20, 16, 16, 0);

    // Every service result above is deliberately ignored, including failed
    // loads and extractions. Close occurs only after all 86 extraction attempts.
    buffer.close();
}

// ORIGINAL: 0x005E8FF5 ?draw_multi_table_dest@Sprite@@QAEHPAUBuffer@@HHHPAE@Z 0x005E8FF5-0x005E9D44 FILE
// draw_dest's 16-bit-table sibling: every drawn destination pixel samples the
// 64K combined table at (source << 8) | destination, and the transparency key
// compares SIGNED (the image's `char` loads). The original's inner copy loop
// is a hand-unrolled Duff's device that computes its own re-entry address at
// runtime; this body keeps the guard clauses, the scale-ratio gate and the
// rect intersection and performs the same per-pixel operation with plain
// nested loops. The seed's scaled walkers dropped the y-table row repeats and
// the per-run replication; the family's verified walk (see draw_dest above)
// supplies both, and the disassembly at 0x005E929A..0x005E92AC (the seed's
// four re-entry labels) is the same sampler.
// TRIED (forwarder): the remaining specialized Sprite workers were reached
// through this forwarder until the body landed here beside its family.
// HAND-WRITTEN ASSEMBLY IN THE ORIGINAL: pushf/popf around the unrolled copy
// - semantic equivalence is the honest ceiling; the answer is not `__asm`.
int Sprite::draw_multi_table_dest(Buffer *destination, int transparent, int x,
                                  int y, unsigned char *multi_table) {
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0)
        return 3;

    if (SpriteScaleNumerator != SpriteScaleDenominator &&
        (SpriteCachedScaleNumerator != SpriteScaleNumerator ||
         SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        SpriteScalingRatio = static_cast<float>(
            static_cast<double>(SpriteScaleNumerator) / SpriteScaleDenominator);
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }

    const int source_pitch = iSpriteWidth2_;
    int destination_pitch = static_cast<int>(destination->stride_);
    const unsigned char *const scaled_source =
        reinterpret_cast<const unsigned char *>(pcBits_);

    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        RECT original;
        original.top = y + iTopOffset_;
        original.left = x + iLeftOffset_;
        original.right = original.left + iSpriteWidth_;
        original.bottom = original.top + iSpriteHeight_;
        RECT clipped = original;
        if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row =
            reinterpret_cast<const unsigned char *>(pcBits_) +
            (clipped.left - original.left) +
            (clipped.top - original.top) * source_pitch;
        unsigned char *destination_row =
            static_cast<unsigned char *>(destination->locked_bits_) +
            clipped.left + clipped.top * destination_pitch;
        const int columns = clipped.right - clipped.left;
        const int rows = clipped.bottom - clipped.top;
        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                if (static_cast<char>(source_row[column]) !=
                    static_cast<char>(transparent)) {
                    destination_row[column] = multi_table[
                        (source_row[column] << 8) | destination_row[column]];
                }
            }
            if (row + 1 < rows) {
                source_row += source_pitch;
                destination_row += destination_pitch;
            }
        }
        destination->free_data(1);
        return 1;
    }

    const int scaled_height = static_cast<int>(
        static_cast<double>(iSpriteHeight_) * SpriteScalingRatio);
    const int scaled_width = static_cast<int>(
        static_cast<double>(iSpriteWidth_) * SpriteScalingRatio);
    const int top = static_cast<int>(
        static_cast<double>(y) + static_cast<double>(iTopOffset_) * SpriteScalingRatio);
    const int left = static_cast<int>(
        static_cast<double>(x) + static_cast<double>(iLeftOffset_) * SpriteScalingRatio);
    RECT original = {left, top, left + scaled_width, top + scaled_height};
    RECT clipped = original;
    if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
        destination->free_data(1);
        return 0;
    }
    unsigned char *const destination_bits =
        static_cast<unsigned char *>(destination->locked_bits_);
    const int base_offset = original.left + original.top * destination_pitch;
    destination_pitch = static_cast<int>(destination->stride_);
    int source_x, source_y, destination_x, destination_y;
    int source_columns, source_rows, destination_columns, destination_rows;

    if (SpriteScaleNumerator < SpriteScaleDenominator) {
        if (original.right - original.left < SpriteMinimumScaledWidth ||
            original.bottom - original.top < SpriteMinimumScaledHeight) {
            destination->free_data(1);
            return 0;
        }
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_shrink(&original, &clipped,
            &destination_x, &source_x, &destination_columns, &source_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_shrink(&original, &clipped,
            &destination_y, &source_y, &destination_rows, &source_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < destination_rows; ++row) {
            const unsigned char *source = source_row;
            for (int column = 0; column < destination_columns; ++column) {
                if (static_cast<char>(*source) != static_cast<char>(transparent)) {
                    destination_row[column] =
                        multi_table[(*source << 8) | destination_row[column]];
                }
                if (column + 1 < destination_columns) source += columns[column];
            }
            if (row + 1 < destination_rows) {
                source_row += source_pitch * static_cast<unsigned char>(rows[row]);
                destination_row += destination_pitch;
            }
        }
    } else {
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_expand(&original, &clipped,
            &source_x, &destination_x, &source_columns, &destination_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_expand(&original, &clipped,
            &source_y, &destination_y, &source_rows, &destination_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < source_rows; ++row) {
            const unsigned char selector = static_cast<unsigned char>(rows[row]);
            const int repeats = selector == 1 ? 1 : selector == 2 ? 2 : selector == 3 ? 3 : 4;
            int output_column = 0;
            for (int column = 0; column < source_columns; ++column) {
                const unsigned char pixel_value = source_row[column];
                if (static_cast<char>(pixel_value) !=
                    static_cast<char>(transparent)) {
                    for (int repeat = 0; repeat < repeats; ++repeat)
                        for (int pixel = 0; pixel < columns[column]; ++pixel) {
                            unsigned char &target = destination_row[
                                repeat * destination_pitch + output_column + pixel];
                            target = multi_table[(pixel_value << 8) | target];
                        }
                }
                output_column += columns[column];
            }
            if (row + 1 < source_rows) {
                source_row += source_pitch;
                destination_row += repeats * destination_pitch;
            }
        }
    }
    destination->free_data(1);
    return 1;
}

// ORIGINAL: 0x005E81BC ?draw_dest_unk2@Sprite@@QAEHPAUBuffer@@HHHPAE@Z 0x005E81BC-0x005E8FA5 FILE
// symbol    ?draw_dest_unk2@Sprite@@QAEHPAVBuffer@@HHHPAE@Z
// draw_dest's stencil sibling: the transparency key passes untouched, a zero
// source byte passes untouched, and everything else is remapped - the uniform
// and shrink walks remap through the DESTINATION pixel (table[dst]) while the
// expanded runs replicate table[source] over the run. The original's inner
// loops are a hand-unrolled Duff's device (pushf/popf protected); this body
// keeps the guards, the scale gate and the clipping and performs the same
// per-pixel operations with plain nested loops, on the family's verified walk.
// HAND-WRITTEN ASSEMBLY IN THE ORIGINAL: the shipped bytes save and restore
// the FLAGS around the unrolled block - byte-exactness is not reachable from
// C++ here; the honest ceiling is semantic equivalence.
// TRIED (seed): two indirect back-edges (jmp edx @0x5E8420, jmp [0x9B3AC4]
//         @0x5E8766) are Duff's-device dispatch; rewritten as a
//         switch(remainder) on the loop-invariant state. The row-repeat count
//         loop @0x5E87F8-0x5E8817 (esi += stride, N-1 times from a y-table
//         byte) is a case Ghidra silently dropped; reconstructed from raw
//         bytes.
int Sprite::draw_dest_unk2(Buffer *destination, int transparent, int x, int y,
                           unsigned char *table) {
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0)
        return 3;

    if (SpriteScaleNumerator != SpriteScaleDenominator &&
        (SpriteCachedScaleNumerator != SpriteScaleNumerator ||
         SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        SpriteScalingRatio = static_cast<float>(
            static_cast<double>(SpriteScaleNumerator) / SpriteScaleDenominator);
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }

    const unsigned char key = static_cast<unsigned char>(transparent);
    const int source_pitch = iSpriteWidth2_;
    int destination_pitch = static_cast<int>(destination->stride_);
    const unsigned char *const scaled_source =
        reinterpret_cast<const unsigned char *>(pcBits_);

    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        RECT original;
        original.top = y + iTopOffset_;
        original.left = x + iLeftOffset_;
        original.right = original.left + iSpriteWidth_;
        original.bottom = original.top + iSpriteHeight_;
        RECT clipped = original;
        if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row =
            reinterpret_cast<const unsigned char *>(pcBits_) +
            (clipped.left - original.left) +
            (clipped.top - original.top) * source_pitch;
        unsigned char *destination_row =
            static_cast<unsigned char *>(destination->locked_bits_) +
            clipped.left + clipped.top * destination_pitch;
        const int columns = clipped.right - clipped.left;
        const int rows = clipped.bottom - clipped.top;
        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                const unsigned char source = source_row[column];
                if (source != key && source != 0)
                    destination_row[column] = table[destination_row[column]];
            }
            if (row + 1 < rows) {
                source_row += source_pitch;
                destination_row += destination_pitch;
            }
        }
        destination->free_data(1);
        return 0;
    }

    const int scaled_height = static_cast<int>(
        static_cast<double>(iSpriteHeight_) * SpriteScalingRatio);
    const int scaled_width = static_cast<int>(
        static_cast<double>(iSpriteWidth_) * SpriteScalingRatio);
    const int top = static_cast<int>(
        static_cast<double>(y) + static_cast<double>(iTopOffset_) * SpriteScalingRatio);
    const int left = static_cast<int>(
        static_cast<double>(x) + static_cast<double>(iLeftOffset_) * SpriteScalingRatio);
    RECT original = {left, top, left + scaled_width, top + scaled_height};
    RECT clipped = original;
    if (!IntersectRect(&clipped, &clipped, &destination->rect1_)) {
        destination->free_data(1);
        return 0;
    }
    unsigned char *const destination_bits =
        static_cast<unsigned char *>(destination->locked_bits_);
    const int base_offset = original.left + original.top * destination_pitch;
    destination_pitch = static_cast<int>(destination->stride_);
    int source_x, source_y, destination_x, destination_y;
    int source_columns, source_rows, destination_columns, destination_rows;

    if (SpriteScaleNumerator < SpriteScaleDenominator) {
        if (original.right - original.left < SpriteMinimumScaledWidth ||
            original.bottom - original.top < SpriteMinimumScaledHeight) {
            destination->free_data(1);
            return 0;
        }
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_shrink(&original, &clipped,
            &destination_x, &source_x, &destination_columns, &source_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_shrink(&original, &clipped,
            &destination_y, &source_y, &destination_rows, &source_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < destination_rows; ++row) {
            const unsigned char *source = source_row;
            for (int column = 0; column < destination_columns; ++column) {
                if (*source != key && *source != 0)
                    destination_row[column] = table[*source];
                if (column + 1 < destination_columns) source += columns[column];
            }
            if (row + 1 < destination_rows) {
                source_row += source_pitch * static_cast<unsigned char>(rows[row]);
                destination_row += destination_pitch;
            }
        }
    } else {
        IntersectRect(&clipped, &original, &destination->rect1_);
        int *const columns = get_clipped_x_table_expand(&original, &clipped,
            &source_x, &destination_x, &source_columns, &destination_columns);
        if (columns == 0) {
            destination->free_data(1);
            return 0;
        }
        int *const rows = get_clipped_y_table_expand(&original, &clipped,
            &source_y, &destination_y, &source_rows, &destination_rows);
        if (rows == 0) {
            destination->free_data(1);
            return 0;
        }
        const unsigned char *source_row = scaled_source + (source_x + source_pitch * source_y);
        unsigned char *destination_row = destination_bits + (base_offset +
            destination_x + destination_pitch * destination_y);
        for (int row = 0; row < source_rows; ++row) {
            const unsigned char selector = static_cast<unsigned char>(rows[row]);
            const int repeats = selector == 1 ? 1 : selector == 2 ? 2 : selector == 3 ? 3 : 4;
            int output_column = 0;
            for (int column = 0; column < source_columns; ++column) {
                const unsigned char pixel_value = source_row[column];
                if (pixel_value != key && pixel_value != 0) {
                    const unsigned char mapped = table[pixel_value];
                    for (int repeat = 0; repeat < repeats; ++repeat)
                        for (int pixel = 0; pixel < columns[column]; ++pixel) {
                            destination_row[
                                repeat * destination_pitch + output_column +
                                pixel] = mapped;
                        }
                }
                output_column += columns[column];
            }
            if (row + 1 < source_rows) {
                source_row += source_pitch;
                destination_row += repeats * destination_pitch;
            }
        }
    }
    destination->free_data(1);
    return 0;
}

// ORIGINAL: 0x005E3890 ?init@Sprite@@QAEHPADHH@Z 0x005E3890-0x005E3953 FILE
// Re-target a live sprite at a new PCX file: release the previous pixel block
// (charging its size back to SpriteMemoryUsed) and file name, clear the
// geometry, copy the new name into heap storage, and store the requested
// width/height into both the source and the displayed dimension pairs.
int Sprite::init(char *filename, int width, int height) {
    if (filename == 0) {
        return 0x10;
    }
    if (fObj1Exists_ == 0 && pcBits_ != 0) {
        SpriteMemoryUsed -= iSpriteHeight_ * iSpriteWidth_;
        if (pcBits_ != 0) {
            free(pcBits_);
        }
        pcBits_ = 0;
    }
    if (ppszFileName_ != 0) {
        free(ppszFileName_);
        ppszFileName_ = 0;
    }
    iSpriteWidth2_ = 0;
    iSpriteWidth_ = 0;
    iSpriteHeight_ = 0;
    iWidth_ = 0;
    iHeight_ = 0;
    iLeftOffset_ = 0;
    iTopOffset_ = 0;
    fObj1Exists_ = 0;
    ppszFileName_ = static_cast<char *>(mem_get(strlen(filename) + 1));
    if (ppszFileName_ == 0) {
        return 4;
    }
    ppszFileName_[0] = 0;
    strcat(ppszFileName_, filename);
    iSpriteWidth2_ = width;
    iSpriteWidth_ = width;
    iSpriteHeight_ = height;
    iWidth_ = width;
    iHeight_ = height;
    return 0;
}

// ORIGINAL: 0x00404860 ?sprite_draw_func@@YAXPAUSprite@@PAUGraphicWin@@HHH@Z 0x00404860-0x00404896 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/units/00404860.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x404860 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Draw callback using the real GraphicWin Buffer secondary base, Sprite transparency and native coordinate offsets."}
// symbol    ?sprite_draw_func@@YAXPAVSprite@@PAVGraphicWin@@HHH@Z
void __cdecl sprite_draw_func(Sprite *sprite, GraphicWin *owner,
                              int unused, int x, int y) {
    (void)unused;
    x -= 5;
    y -= 6;
    Buffer *buffer = owner ? static_cast<Buffer *>(owner) : 0;
    const unsigned char transparent = sprite->transparent_index();
    sprite->draw(buffer, transparent, x, y, 1, 1);
}

// ORIGINAL: 0x00450DE0 sub_450de0 0x00450DE0-0x00450E2D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/00450de0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450de0 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
extern "C" int __cdecl sub_450de0(Sprite *sprite, int x, int y) {
    Buffer *buffer = &g_PCX_PARSE_TEMP_BUFFER1;
    buffer->change_color(0x107, 0x109);
    buffer->change_color(0x106, 0x109);
    return sprite->extract(buffer, 0x109, x, y, 100, 50, 0);
}

// ORIGINAL: 0x00450E30 sub_450e30 0x00450E30-0x00450E7D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/00450e30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450e30 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
extern "C" int __cdecl sub_450e30(Sprite *sprite, int x, int y) {
    Buffer *buffer = &g_PCX_PARSE_TEMP_BUFFER1;
    buffer->change_color(0x107, 0x109);
    buffer->change_color(0x106, 0x109);
    return sprite->extract(buffer, 0x109, x, y, 0x64, 0x3e, 0);
}

// ORIGINAL: 0x00450E80 sub_450e80 0x00450E80-0x00450ECD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/00450e80.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450e80 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
extern "C" int __cdecl sub_450e80(Sprite *sprite, int x, int y) {
    Buffer *buffer = &g_PCX_PARSE_TEMP_BUFFER1;
    buffer->change_color(0x107, 0x109);
    buffer->change_color(0x106, 0x109);
    return sprite->extract(buffer, 0x109, x, y, 0x64, 0x4b, 0);
}

// ORIGINAL: 0x00450ED0 sub_450ed0 0x00450ED0-0x00450F21 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/units/00450ed0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450ed0 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
extern "C" int __cdecl sub_450ed0(Sprite *sprite, int x, int y,
                       int width, int height) {
    Buffer *buffer = &g_PCX_PARSE_TEMP_BUFFER1;
    buffer->change_color(0x107, 0x109);
    buffer->change_color(0x106, 0x109);
    return sprite->extract(buffer, 0x109, x, y, width, height, 0);
}

// ORIGINAL: 0x00450F30 sub_450f30 0x00450F30-0x00450F81 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/units/00450f30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450f30 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
extern "C" void __cdecl sub_450f30(Sprite *sprite, int x, int y,
                        int width, int height) {
    Buffer *buffer = &g_PCX_PARSE_TEMP_BUFFER1;
    buffer->change_color(0x107, 0x109);
    buffer->change_color(0x106, 0x109);
    sprite->extract(buffer, 0x109, x, y, width, height, 0);
}

// ORIGINAL: 0x00450F90 sub_450f90 0x00450F90-0x00450FB0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/00450f90.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450f90 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
extern "C" int __cdecl sub_450f90(Texture *texture, int x, int y) {
    return texture->extract(&g_PCX_PARSE_TEMP_BUFFER1, x, y, 0x38, 0x38, 0);
}

// Heap metadata borrows the arena. Freeing its storage ends that lifetime;
// invoking Heap::~Heap here would incorrectly free each interior region.
void TexHeap::release_allocations() {
    if (base_ != 0) {
        free(base_);
        base_ = 0;
    }

    if (blocks_ != 0) {
        // The image re-reads count_ for the loop test at every iteration, so
        // the bound stays an expression over the field rather than a local.
        for (int index = 0; index < static_cast<int>(count_) + 1; ++index) {
            Heap *block = blocks_[index];
            if (block != 0) {
                free(block);
                blocks_[index] = 0;
            }
        }
        if (blocks_ != 0) {
            free(blocks_);
        }
        blocks_ = 0;
    }

}

// ORIGINAL: 0x00634FB0 ??0TexHeap@@QAE@XZ 0x00634FB0-0x00634FC0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:texheap-owner", "Native634FB0fourwordinitialization;634FC0ignores635050statusandreturnsthis;635050arena/table/20byteHeapallocations;635370/6353C0/634FE0sharedmemberconsumers.", "historical source:src/field_accessors.cpp"], "reason": "Recover source-owned typed arena and Heap-pointer table, genuine constructor and borrowed-region Heap lifetimes; initializer returns the native receiver. Preserve allocation order and native omissions, without invented global ownership."}
// symbol    ??0TexHeap@@QAE@XZ
TexHeap::TexHeap() {
    base_ = 0;
    blocks_ = 0;
    count_ = 0;
    capacity_ = 0;
}

// ORIGINAL: 0x00634FC0 ?UNK1@TexHeap@@QAEHH@Z 0x00634FC0-0x00634FD3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:texheap-owner", "Native634FB0fourwordinitialization;634FC0ignores635050statusandreturnsthis;635050arena/table/20byteHeapallocations;635370/6353C0/634FE0sharedmemberconsumers.", "historical source:src/recovered/00634fc0.cpp"], "reason": "Recover source-owned typed arena and Heap-pointer table, genuine constructor and borrowed-region Heap lifetimes; initializer returns the native receiver. Preserve allocation order and native omissions, without invented global ownership."}
// symbol    ?initialize@TexHeap@@QAEPAU1@H@Z
TexHeap *TexHeap::initialize(int count) {
    configure(count);
    return this;
}

// ORIGINAL: 0x00635050 ?UNK2@TexHeap@@QAEHH@Z 0x00635050-0x00635370
// RECOVERY_BINDING: v1 {"evidence": ["task:texheap-owner", "Native634FB0fourwordinitialization;634FC0ignores635050statusandreturnsthis;635050arena/table/20byteHeapallocations;635370/6353C0/634FE0sharedmemberconsumers.", "historical source:src/unrecovered/00635050.cpp"], "reason": "Recover source-owned typed arena and Heap-pointer table, genuine constructor and borrowed-region Heap lifetimes; initializer returns the native receiver. Preserve allocation order and native omissions, without invented global ownership."}
// symbol    ?configure@TexHeap@@QAEHH@Z
// TRIED: MISMATCH #3 mov/push - the four-register prologue (ebx/ebp/esi/edi) the original uses for this heavily-inlined free/loop/alloc body is not reproduced by writing the four inlined cleanup sites as a shared macro; register pressure differs from the start.
int TexHeap::configure(int count) {
    release_allocations();
    count_ = static_cast<uint32_t>(count);
    const uint32_t entries = count_ + 1u;
    capacity_ = entries << 16;
    base_ = static_cast<unsigned char *>(mem_get(capacity_));
    if (base_ == 0) {
        return 4;
    }
    blocks_ = static_cast<Heap **>(mem_get(entries * sizeof(*blocks_)));
    if (blocks_ == 0) {
        if (base_ != 0) {
            free(base_);
        }
        base_ = 0;
        return 4;
    }

    void *storage = mem_get(sizeof(Heap));
    blocks_[0] = static_cast<Heap *>(storage);
    if (storage == 0) {
        shutdown();
        return 4;
    }
    // Actual address alignment, not an executable address or self offset.
    const uintptr_t arena = reinterpret_cast<uintptr_t>(base_);
    uintptr_t cursor = (arena & ~static_cast<uintptr_t>(0xffff)) + 0x10000;
    blocks_[0] = new (storage) Heap(base_, static_cast<size_t>(cursor - arena));

    int index = 1;
    if (count > 1) {
        do {
            storage = mem_get(sizeof(Heap));
            blocks_[index] = static_cast<Heap *>(storage);
            if (storage == 0) {
                shutdown();
                return 4;
            }
            blocks_[index] = new (storage) Heap(
                reinterpret_cast<unsigned char *>(cursor), 0xffff);
            cursor += 0x10000;
            ++index;
        } while (index < count);
    }

    storage = mem_get(sizeof(Heap));
    blocks_[count_] = static_cast<Heap *>(storage);
    if (storage == 0) {
        shutdown();
        return 4;
    }
    // With count==0 the original overwrites (and leaks) descriptor zero.
    // Preserve that omission; the normal caller domain is not yet proved.
    blocks_[count_] = new (storage) Heap(
        reinterpret_cast<unsigned char *>(cursor),
        static_cast<size_t>(arena + capacity_ - cursor));
    return 0;
}

// ORIGINAL: 0x00454D10 ?load_misc@@YAXXZ 0x00454D10-0x00454FB6
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/00454d10.cpp", "task:asset-startup-loader-owner", "Native229 init receivers:89 tech,70 facility,64 project and6 battle Sprites;44-byte element stride; facility34..64 x prefix and >64 index-31; native files/dimensions/order. Existing Sprite storage is constructed in this owner, not original-address aliases."], "reason": "Recover the complete asset loader through source-owned C++ objects, arrays and normal lifecycle while retaining original mappings and historical findings; no asset/runtime acceptance claimed."}
void __cdecl load_misc() {
    int index;
    char number[80];

    for (index = 0; index < 89; ++index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "techs\\tech");
        if (index < 10) {
            strcat(StringTemp, "0");
        }
        _itoa(index, number, 10);
        strcat(StringTemp, number);
        g_TECH_ICON_SPRITES[index].init(StringTemp, 0, 0);
        g_TECH_ICON_SPRITES[index].iWidth_ = 0xD7;
        g_TECH_ICON_SPRITES[index].iHeight_ = 0xD7;
    }

    for (index = 0; index < 70; ++index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "facs\\");
        if (index >= 34 && index <= 64) {
            strcat(StringTemp, "x");
        }
        strcat(StringTemp, "fac0");
        if (index < 10) {
            strcat(StringTemp, "0");
        }
        if (index > 64) {
            _itoa(index - 0x1F, number, 10);
        } else {
            _itoa(index, number, 10);
        }
        strcat(StringTemp, number);
        g_FACILITY_ICON_SPRITES[index].init(StringTemp, 0, 0);
        g_FACILITY_ICON_SPRITES[index].iWidth_ = 0xD7;
        g_FACILITY_ICON_SPRITES[index].iHeight_ = 0xD7;
    }

    for (index = 0; index < 64; ++index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "projs\\proj0");
        if (index < 10) {
            strcat(StringTemp, "0");
        }
        _itoa(index, number, 10);
        strcat(StringTemp, number);
        g_SECRET_PROJECT_ICON_SPRITES[index].init(StringTemp, 0, 0);
        g_SECRET_PROJECT_ICON_SPRITES[index].iWidth_ = 0xD7;
        g_SECRET_PROJECT_ICON_SPRITES[index].iHeight_ = 0xD7;
    }

    g_BATTLE_MIND_WORM_SPRITE.init("btwrm.pcx", 0, 0);
    g_BATTLE_MIND_WORM_SPRITE.iWidth_ = 0x60;
    g_BATTLE_MIND_WORM_SPRITE.iHeight_ = 0x73;
    g_BATTLE_ISLE_DEEP_SPRITE.init("btisl.pcx", 0, 0);
    g_BATTLE_ISLE_DEEP_SPRITE.iWidth_ = 0x60;
    g_BATTLE_ISLE_DEEP_SPRITE.iHeight_ = 0x73;
    g_BATTLE_LOCUSTS_CHIRON_SPRITE.init("btchi.pcx", 0, 0);
    g_BATTLE_LOCUSTS_CHIRON_SPRITE.iWidth_ = 0x60;
    g_BATTLE_LOCUSTS_CHIRON_SPRITE.iHeight_ = 0x73;
    g_BATTLE_FUNGAL_TOWER_SPRITE.init("BTfungtow.pcx", 0, 0);
    g_BATTLE_FUNGAL_TOWER_SPRITE.iWidth_ = 0x60;
    g_BATTLE_FUNGAL_TOWER_SPRITE.iHeight_ = 0x73;
    g_BATTLE_SPORE_LAUNCHER_SPRITE.init("BTsporlnch.pcx", 0, 0);
    g_BATTLE_SPORE_LAUNCHER_SPRITE.iWidth_ = 0x60;
    g_BATTLE_SPORE_LAUNCHER_SPRITE.iHeight_ = 0x73;
    g_BATTLE_SEALURK_SPRITE.init("BTsealrk.pcx", 0, 0);
    g_BATTLE_SEALURK_SPRITE.iWidth_ = 0x60;
    g_BATTLE_SEALURK_SPRITE.iHeight_ = 0x73;
}
