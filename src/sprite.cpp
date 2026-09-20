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
#include "buffer.h"

#include <new>
#include "general.h"   // mem_get, for the placeholder pixel
#include "heap.h"      // Heap::get, behind TexHeap::get_mem below
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

// Shared four-byte sprite-memory counter: the constructor at 0x005E37E6/
// 0x005E37EF reads/writes it, and close at 0x005E3833/0x005E383F updates
// the same scalar. Its original storage is in .data's loader-zero-filled
// tail, not a pointer into the executable. The constructor adds sizeof(Sprite);
// pixel-release paths subtract pixel area, so full accounting is still unclear.
// RECOVERY_OBJECT: v2 0x009B6618 0x009B661C ?SpriteMemoryUsed@@3HA 0x005E37E0
int SpriteMemoryUsed = 0;

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
// size      5 bytes
// prototype int (__thiscall ?UNK1@Sprite@@QAEHHHHHHHH@Z)(Sprite* this, int, int, int, int, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Sprite::UNK1(int, int, int, int, int, int, int) {
    return 0;
}


/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005E4AEA ?UNK2@Sprite@@QAEHHHHHH@Z 0x005E4AEA-0x005E4AEF BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK2@Sprite@@QAEHHHHHH@Z)(Sprite* this, int, int, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Sprite::UNK2(int, int, int, int, int) {
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
int TexHeap::get_mem(int size) {
    if (basePtr_ != 0) {
        int index = 0;
        int count = static_cast<int>(count_) + 1;
        if (count > 0) {
            Heap **blocks = reinterpret_cast<Heap **>(currentPtr_);
            while (index < count) {
                if (static_cast<int>(blocks[index]->free_size_) >= size) {
                    return reinterpret_cast<int>(blocks[index]->get(size));
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
// symbol    ?get_tex_mem@TexHeap@@QAEHH@Z
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
int TexHeap::get_tex_mem(int size) {
    if (basePtr_ != 0) {
        int index = 0;
        int count = static_cast<int>(count_) + 1;
        if (count > 0) {
            Heap **blocks = reinterpret_cast<Heap **>(currentPtr_);
            do {
                Heap *block = blocks[index];
                if ((reinterpret_cast<uint32_t>(block->current_) & 0xffff) == 0 &&
                    static_cast<int>(block->free_size_) >= size) {
                    return reinterpret_cast<int>(block->get(size));
                }
                ++index;
            } while (index < count);
        }
    }
    return 0;
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
Sprite g_IFACE_BOX_SPRITES1[51];  // 0x007AC290, 0x2c stride
Sprite g_IFACE_BOX_SPRITES2[51];  // 0x0076DC98, 0x2c stride
Sprite g_IFACE_BOX_SPRITES3[51];  // 0x0078AE58, 0x2c stride
Sprite g_IFACE_BOX_SPRITES4[51];  // 0x0078CC60, 0x2c stride
Sprite g_IFACE_BOX_SPRITES5[51];  // 0x00779CB0, 0x2c stride
Sprite g_IFACE_BOX_SPRITES6[51];  // 0x007A6978, 0x2c stride
Sprite g_IFACE_BOX_SPRITES7[51];  // 0x0078E0A0, 0x2c stride
Sprite g_IFACE_BOX_SPRITES8[51];  // 0x0075B950, 0x2c stride
Sprite g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE;  // 0x007921E8
Sprite g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE;  // 0x00779478
Sprite g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE;  // 0x007AC1C0
Sprite g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE;  // 0x007AB400
Sprite g_IFACE_STD_POPUPS_TOP_MID_SPRITE;  // 0x007793F0
Sprite g_IFACE_STD_POPUPS_BOT_MID_SPRITE;  // 0x007871D0
Sprite g_IFACE_STD_POPUPS_MID_LEFT_SPRITE;  // 0x007ACFB0
Sprite g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE;  // 0x0075B058
Sprite g_TER1_WHITE_ORG_YEL_TILE_SPRITES[6];  // 0x00791C58, 0x2c stride
Sprite g_TER1_BOTTOM_LEFT_TILE_SPRITES[9];  // 0x007A99A0, 0x2c stride
Sprite g_TER1_UNUSED_SPRITES2[2];  // 0x00779420, 0x2c stride
Sprite g_TER1_MINE_SPRITE;  // 0x00776A50
Sprite g_TER1_SOLAR_COLLECTOR_SPRITE;  // 0x007991F8
Sprite g_TER1_TIDAL_HARNESS_SPRITE;  // 0x0078A5B0
Sprite g_TER1_MINING_PLATFORM_SPRITE;  // 0x00789B68
Sprite g_TER1_TUT_BLANK_SPRITE;  // 0x00798C40
Sprite g_TER1_KELP_FARM_SPRITE;  // 0x007A6860
Sprite g_TER1_CONDENSER_SPRITE;  // 0x00779390
Sprite g_TER1_ECHELON_MIRROR_SPRITE;  // 0x007ABF60
Sprite g_TER1_BOREHOLE_SPRITE;  // 0x0078DD20
Sprite g_TER1_BOREHOLE_CLUSTER_SPRITE;  // 0x007ACB88
Sprite g_TER1_MANIFOLD_NEXUS_SPRITES[6];  // 0x007AC098, 0x2c stride
Sprite g_TER1WRECK_UNITY_WRECKAGE_SPRITES[15];  // 0x00776758, 0x2c stride
Sprite g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES[4];  // 0x00799658, 0x2c stride
Sprite g_FOSSIL_FIELD_RIDGE_SPRITES[6];  // 0x007ACD70, 0x2c stride
Sprite g_TER1_UNUSED_SPRITES1[5];  // 0x007792B0, 0x2c stride
Sprite g_TER1_FARM_SPRITES[5];  // 0x00791FC8, 0x2c stride
Sprite g_TER1_SOIL_ENRICHER_SPRITES[5];  // 0x0079A710, 0x2c stride
Sprite g_TER1_SEA_LAND_RESOURCE_SPRITES[12];  // 0x0075B230, 0x2c stride
Sprite g_TER1_LANDMARK_RESOURCE_SPRITES[6];  // 0x0078A650, 0x2c stride
Sprite g_GLOW_SPRITES[2];  // 0x00779770, 0x2c stride
Sprite g_TER1_UNITY_POD_SPRITES[6];  // 0x0077AFF8, 0x2c stride
Sprite g_TER1_MONOLITH_SPRITE;  // 0x0075B098
Sprite g_TER1_BUNKER_SPRITE;  // 0x007AD010
Sprite g_TER1_AIRBASE_SPRITE;  // 0x007ACC60
Sprite g_TER1_SENSOR_ARRAY_SPRITE;  // 0x007A6830
Sprite g_RAINFALL_DOUBLE_TILE_SPRITES[2];  // 0x007ACBB8, 0x2c stride
Sprite g_VEH_SPRITES[152];  // 0x007777A0, 0x2c stride
Sprite g_FLAGS_VEH_SPRITES[112];  // 0x0078B778, 0x2c stride
Sprite g_ICONS_GENERAL_SPRITES[16];  // 0x0075B450, 0x2c stride
Sprite g_RESOURCE_ICON_SPRITES[32];  // 0x007A72A0, 0x2c stride
Sprite g_CITIZEN_LG_CURSOR_SPRITES[8];  // 0x0078CAD0, 0x2c stride
Sprite g_SPECIALIST_LG_CURSOR_SPRITES[7];  // 0x007765C0, 0x2c stride
Sprite g_CITIZEN_SM_CURSOR_SPRITES[8];  // 0x00776420, 0x2c stride
Sprite g_SPECIALIST_SM_CURSOR_SPRITES[7];  // 0x00779B78, 0x2c stride
Sprite g_AL_CITIZEN_LG_CURSOR_SPRITES[4];  // 0x007AD1B0, 0x2c stride
Sprite g_AL_SPECIALIST_LG_CURSOR_SPRITES[7];  // 0x00798CD0, 0x2c stride
Sprite g_AL_CITIZEN_SM_CURSOR_SPRITES[4];  // 0x0075B180, 0x2c stride
Sprite g_AL_SPECIALIST_SM_CURSOR_SPRITES[7];  // 0x0075B710, 0x2c stride
Sprite g_RED_ALIEN_HEAD_ICON_SPRITE;  // 0x00791F98
Sprite g_SILVER_MENU_ICON_SPRITES[4];  // 0x0075AE20, 0x2c stride
Sprite g_SILVER_CHECKBOX_ICON_SPRITES[2];  // 0x0075AF68, 0x2c stride
Sprite g_RED_MALE_HEAD_ICON_SPRITE;  // 0x0075AD88
Sprite g_NULL_RESOURCE_ICON_SPRITE;  // 0x0078A520
Sprite g_PEACE_SIGN_SPRITES[2];  // 0x0075AF10, 0x2c stride
Sprite g_ICON_TILE_SQUARE_SPRITE;  // 0x00776728
Sprite g_XI_BOOM_VEH_SPRITES[144];  // 0x007A9B30, 0x2c stride
Sprite g_XF_BOOM_VEH_SPRITES[64];  // 0x007AB460, 0x2c stride
Sprite g_BATTLE_MIND_WORM_SPRITE;  // 0x00779200
Sprite g_BATTLE_ISLE_DEEP_SPRITE;  // 0x0076DC30
Sprite g_BATTLE_LOCUSTS_CHIRON_SPRITE;  // 0x0078A4A0
Sprite g_BATTLE_FUNGAL_TOWER_SPRITE;  // 0x00787E70
Sprite g_BATTLE_SPORE_LAUNCHER_SPRITE;  // 0x007AC000
Sprite g_BATTLE_SEALURK_SPRITE;  // 0x00779570
Sprite g_TECH_ICON_SPRITES[89];  // 0x00759E28, 0x2c stride
Sprite g_FACILITY_ICON_SPRITES[70];  // 0x00787200, 0x2c stride
Sprite g_SECRET_PROJECT_ICON_SPRITES[64];  // 0x00759320, 0x2c stride
Sprite g_IFACE_MP_COMBO_ARROW_SPRITES[3];  // 0x00788038, 0x2c stride
Sprite g_SCROLL_BAR_ARROW_ICON_SPRITES[12];  // 0x00791D88, 0x2c stride
Sprite g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES[12];  // 0x0078DE30, 0x2c stride
Sprite g_SCROLL_BAR_FILLER_ICON_SPRITES;  // 0x0079A5E8
Sprite g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE;  // 0x007796B0
Sprite g_IFACE_LOCK_SPRITES[2];  // 0x007AC040, 0x2c stride
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
Sprite g_IFACE_TECH_TREE_ARROW_SPRITES[3];  // 0x0075AFD0, 0x2c stride
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
Sprite g_IFACE_GREEN_RIGHT_ARROW_SPRITE[1];  // 0x007F67C8, 0x2c stride
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
    if (basePtr_ != 0) {
        free(reinterpret_cast<void *>(basePtr_));
        basePtr_ = 0;
    }

    if (currentPtr_ != 0) {
        // The image re-reads count_ for the loop test at every iteration, so
        // the bound stays an expression over the field rather than a local.
        for (int index = 0; index < static_cast<int>(count_) + 1; ++index) {
            Heap *block = reinterpret_cast<Heap **>(currentPtr_)[index];
            if (block != 0) {
                free(block);
                reinterpret_cast<Heap **>(currentPtr_)[index] = 0;
            }
        }
        if (currentPtr_ != 0) {
            free(reinterpret_cast<void *>(currentPtr_));
        }
        currentPtr_ = 0;
    }

    count_ = 0;
    baseSize_ = 0;
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
