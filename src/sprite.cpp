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
#include "sprite.h"
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
    // Use the actual pointer and signed member types. Volatile receiver
    // access preserves the original store sequence without aliasing pointer
    // storage through a dword lvalue. Target pointers are four bytes.
    volatile Sprite *const receiver = this;
    receiver->ppszFileName_ = 0;
    SpriteMemoryUsed = static_cast<int>(
        static_cast<uint32_t>(SpriteMemoryUsed) + sizeof(Sprite));
    receiver->pcBits_ = 0;
    receiver->iSpriteWidth2_ = 0;
    receiver->iSpriteWidth_ = 0;
    receiver->iSpriteHeight_ = 0;
    receiver->iWidth_ = 0;
    receiver->iHeight_ = 0;
    receiver->iLeftOffset_ = 0;
    receiver->iTopOffset_ = 0;
    receiver->cTransparentIndex_ = 9;
    receiver->fObj1Exists_ = 0;
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
    unsigned char *pixels = pcBits_;
    const int pixel_count = iSpriteHeight_ * iSpriteWidth2_;
    const int group_count = pixel_count >> 2;
    old_color = static_cast<unsigned char>(old_color);
    const int tail_count = pixel_count - (group_count << 2);
    new_color = static_cast<unsigned char>(new_color);
    // Hand-written assembly: unrolled compare chains with `loop`, `and ecx,ecx` tests, mov ah writes.
    // A zero group count skips the tail too; a negative product wraps the LOOP count.
    __asm {
        push esi
        mov esi, pixels
        mov ecx, group_count
        mov al, byte ptr old_color
        mov ah, byte ptr new_color
        and ecx, ecx
        je recolor_done
    scan_0:
        cmp [esi], al
        je hit_0
    scan_1:
        inc esi
        cmp [esi], al
        je hit_1
    scan_2:
        inc esi
        cmp [esi], al
        je hit_2
    scan_3:
        inc esi
        cmp [esi], al
        je hit_3
    scan_next:
        inc esi
        loop scan_0
        dec esi
        jmp tail_start
    hit_loop:
        cmp [esi], al
        jne scan_1
    hit_0:
        mov [esi], ah
        inc esi
        cmp [esi], al
        jne scan_2
    hit_1:
        mov [esi], ah
        inc esi
        cmp [esi], al
        jne scan_3
    hit_2:
        mov [esi], ah
        inc esi
        cmp [esi], al
        jne scan_next
    hit_3:
        mov [esi], ah
        inc esi
        loop hit_loop
        dec esi
    tail_start:
        mov ecx, tail_count
        and ecx, ecx
        je recolor_done
    tail_loop:
        inc esi
        cmp [esi], al
        jne tail_next
        mov [esi], ah
    tail_next:
        loop tail_loop
    recolor_done:
        pop esi
    }
    return 0;
}

// ORIGINAL: 0x005EAE4F ?copy@Sprite@@QAEHPAUSprite@@@Z 0x005EAE4F-0x005EAF34 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["5EAE57 checks source bits before touching destination.5EAE6C..5EAEC4 performs the same owned/borrowed cleanup and field clearing as Sprite::close, preserving the transparent key.5EAEC7 captures wrapped32bit stride*height only after cleanup; allocation pointer publishes before null failure4.", "5EAEEA rereads source bits after allocation;5EAEF5..5EAF29 reads metadata after memcpy, writes dimensions/offsets/key, clears borrowed status and copies stride last. Original has no pixel-allocation increment to SpriteMemoryUsed and no self/null destination guard. Valid pixel allocations and ordinary C++ memcpy domains remain required; no behavioral repair is invented."], "reason": "Recover source.copy(destination) through Sprite ownership and close semantics, retaining original allocation failure and metadata observation order."}
// symbol    ?copy@Sprite@@QAEHPAV1@@Z
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
int Sprite::copy(Sprite *destination) {
    if (pcBits_ == 0) {
        return 7;
    }
    if (destination->fObj1Exists_ == 0 && destination->pcBits_ != 0) {
        SpriteMemoryUsed = static_cast<int>(
            static_cast<uint32_t>(SpriteMemoryUsed)
            - static_cast<uint32_t>(destination->iSpriteHeight_)
                  * static_cast<uint32_t>(destination->iSpriteWidth_));
        if (destination->pcBits_ != 0) {
            free(destination->pcBits_);
        }
        destination->pcBits_ = 0;
    }
    if (destination->ppszFileName_ != 0) {
        free(destination->ppszFileName_);
        destination->ppszFileName_ = 0;
    }
    destination->iSpriteWidth2_ = 0;
    destination->iSpriteWidth_ = 0;
    destination->iSpriteHeight_ = 0;
    destination->iWidth_ = 0;
    destination->iHeight_ = 0;
    destination->iLeftOffset_ = 0;
    destination->iTopOffset_ = 0;
    destination->fObj1Exists_ = 0;
    unsigned int size = static_cast<unsigned int>(iSpriteWidth2_);
    size *= static_cast<unsigned int>(iSpriteHeight_);
    unsigned char *pixels = static_cast<unsigned char *>(mem_get(size));
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
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif

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

// Scaling ratios, clipping tables, and renderer clip state are distinct shared
// cells. The eight renderers share these counts, returned-table cursors and
// pitches. For example, 5E4DC6/4DD2 publish unit-clipped dimensions; 5E503C
// stores the X table before its null test, 5E51C5 advances the Y cursor, and
// 5E580D consumes the expansion source-row count. Early returns do not clear
// prior state. These cells start in the image's zero-filled data tail, not
// per-call zeroed storage. Other row-tail/dispatch scratch remains unresolved.
// RECOVERY_OBJECT: v1 0x009B3AB8 ?SpriteRasterYTable@@3PAHA
int *SpriteRasterYTable;
// RECOVERY_OBJECT: v1 0x009B3AD4 ?SpriteRasterDestinationColumnCount@@3HA
int SpriteRasterDestinationColumnCount;
// RECOVERY_OBJECT: v2 0x009B3AD8 0x009B4758 ?SpriteScalingSteps@@3PAHA 0x005EB20F
int SpriteScalingSteps[SpriteScalingTableCount];
// RECOVERY_OBJECT: v1 0x009B475C ?SpriteRasterSourceColumnCount@@3HA
int SpriteRasterSourceColumnCount;
// RECOVERY_OBJECT: v2 0x009B4760 0x009B53E0 ?SpriteClippedYSteps@@3PAHA 0x005EABBF
int SpriteClippedYSteps[SpriteScalingTableCount];
// RECOVERY_OBJECT: v1 0x009B53E8 ?SpriteRasterDestinationRowCount@@3HA
int SpriteRasterDestinationRowCount;
// RECOVERY_OBJECT: v2 0x009B53EC 0x009B606C ?SpriteClippedXSteps@@3PAHA 0x005EAB0F
int SpriteClippedXSteps[SpriteScalingTableCount];
// RECOVERY_OBJECT: v1 0x009B606C ?SpriteRasterXTable@@3PAHA
int *SpriteRasterXTable;
// RECOVERY_OBJECT: v1 0x009B6074 ?SpriteRasterSourceRowCount@@3HA
int SpriteRasterSourceRowCount;
// RECOVERY_OBJECT: v1 0x009B3AC8 ?SpriteRasterDestinationPitch@@3HA
int SpriteRasterDestinationPitch;
// RECOVERY_OBJECT: v1 0x009B6070 ?SpriteRasterSourcePitch@@3HA
int SpriteRasterSourcePitch;
// Successful unit clipping publishes source then destination row tails
// (5E40AD/40BF); successful scaled helpers publish destination then source
// (5E436E/437F). Rejected paths retain prior values. Scaled source tails
// subtract the full source span, not destination width; every horizontal
// step, including the final X-table entry, contributes before the tail.
// Byte-offset arithmetic below preserves the 32-bit row displacement, not
// the original shrink selector-zero loop's 2^32-1 repeated pitch additions.
// RECOVERY_OBJECT: v1 0x009B3ABC ?SpriteRasterSourceRowTail@@3HA
int SpriteRasterSourceRowTail;
// RECOVERY_OBJECT: v1 0x009B3ACC ?SpriteRasterDestinationRowTail@@3HA
int SpriteRasterDestinationRowTail;
// Unrolled-group count, expansion remainder and mono fill colour cells.
// RECOVERY_OBJECT: v1 0x009B6078 ?SpriteRasterGroupCount@@3HA
int SpriteRasterGroupCount;
// RECOVERY_OBJECT: v1 0x009B4758 ?SpriteRasterGroupRemainder@@3HA
int SpriteRasterGroupRemainder;
// RECOVERY_OBJECT: v1 0x009B53E0 ?SpriteRasterMonoColor@@3HA
int SpriteRasterMonoColor;
// Saved computed-jump entry of the shrinking remap worker and the
// expansion workers' column countdown.
// RECOVERY_OBJECT: v1 0x009B3AC4 ?SpriteRasterShrinkEntry@@3HA
int SpriteRasterShrinkEntry;
// RECOVERY_OBJECT: v1 0x009B53E4 ?SpriteRasterColumnCountdown@@3HA
int SpriteRasterColumnCountdown;
// The six later workers publish this cell only after both clipping helpers
// succeed (for example 5E5FEC and 5EA499); rejected, unit and shrink paths
// leave its prior value stale. The multi-table worker's [source][destination]
// lookup (5E9803/9805) needs a distinct typed row view. This spelling preserves
// both proven uses without claiming the original union declaration.
union SpriteRasterRemapPointer {
    unsigned char *single;
    unsigned char (*multi)[256];
};
// RECOVERY_OBJECT: v1 0x009B3AC0 ?SpriteRasterRemapTable@@3TSpriteRasterRemapPointer@@A
SpriteRasterRemapPointer SpriteRasterRemapTable;

// ORIGINAL: 0x005EB20F ?fill_scaling_table@Sprite@@QAEXXZ 0x005EB20F-0x005EB294 BYTE_EXACT
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
        error = twice_divisor = twice_remainder = 0;
    }
    // Original integer error accumulation wraps at 32 bits before its signed
    // branch. Positive scale operands alone do not guarantee no overflow.
    if (quotient & 1)
        error = static_cast<int>(static_cast<uint32_t>(error) + static_cast<uint32_t>(larger));
    int i = 0;
    if (smaller == 1) {
        for (i = 0; i < SpriteScalingTableCount; ++i) {
            SpriteScalingSteps[i] = quotient;
        }
    } else {
        for (i = 0; i < SpriteScalingTableCount; ++i) {
            error = static_cast<int>(static_cast<uint32_t>(error) + static_cast<uint32_t>(twice_remainder));
            SpriteScalingSteps[i] = quotient;
            if (error > 0) {
                error = static_cast<int>(static_cast<uint32_t>(error) - static_cast<uint32_t>(twice_divisor));
                SpriteScalingSteps[i] = static_cast<int>(static_cast<uint32_t>(quotient) + 1u);
            }
        }
    }
}

// ORIGINAL: 0x005EAB0F ?get_clipped_x_table_expand@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z 0x005EAB0F-0x005EABB7
// RECOVERY_BINDING: v1 {"evidence": ["Original returns9B53EC and caller5E5244 consumes EAX as an int-table pointer. It clips partial first/last runs and reports source and destination extents, retaining ordered alias-visible output stores.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover expansion clipping as a typed pointer to the actual X run table."}
// symbol    ?get_clipped_x_table_expand@Sprite@@QAEPAHPAUtagRECT@@0PAH111@Z
// Capture rectangle-derived values before writes; the caller can alias
// source_skip with a RECT coordinate.
int *Sprite::get_clipped_x_table_expand(RECT *full, RECT *clipped, int *source_skip,
    int *destination_skip, int *source_count, int *destination_count) {
    const int skip = clipped->left - full->left;
    int extent = clipped->right - clipped->left;
    if (extent == 0) return 0;
    *destination_count = extent;
    *destination_skip = skip;
    int index = 0;
    int adjusted_skip = skip;
    while (adjusted_skip > 0) { adjusted_skip -= SpriteScalingSteps[index]; ++index; }
    const int *input = SpriteScalingSteps + index;
    int count;
    int *output;
    if (adjusted_skip < 0) {
        const int remaining = -adjusted_skip;
        extent -= remaining;
        --index;
        SpriteClippedXSteps[0] = remaining;
        *source_skip = index;
        output = SpriteClippedXSteps + 1;
        count = 1;
    } else {
        *source_skip = index;
        output = SpriteClippedXSteps;
        count = 0;
    }
    while (extent > 0) {
        const int step = *input++;
        *output++ = step;
        extent -= step;
        ++count;
    }
    if (extent < 0) {
        --output;
        *output = *output + extent;
    }
    *source_count = count;
    return SpriteClippedXSteps;
}

// ORIGINAL: 0x005EABBF ?get_clipped_y_table_expand@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z 0x005EABBF-0x005EAC69
// RECOVERY_BINDING: v1 {"evidence": ["Original returns9B4760 and caller5E5285 consumes EAX as an int-table pointer. Top/bottom coordinate counterpart of5EAB0F with its own scratch table.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover expansion clipping as a typed pointer to the independent Y run table."}
// symbol    ?get_clipped_y_table_expand@Sprite@@QAEPAHPAUtagRECT@@0PAH111@Z
// Preserve the X routine's ordered stores and partial head/tail handling for
// the independent Y scratch table.
int *Sprite::get_clipped_y_table_expand(RECT *full, RECT *clipped, int *source_skip,
    int *destination_skip, int *source_count, int *destination_count) {
    const int skip = clipped->top - full->top;
    int extent = clipped->bottom - clipped->top;
    if (extent == 0) return 0;
    *destination_count = extent;
    *destination_skip = skip;
    int index = 0;
    int adjusted_skip = skip;
    while (adjusted_skip > 0) { adjusted_skip -= SpriteScalingSteps[index]; ++index; }
    const int *input = SpriteScalingSteps + index;
    int count;
    int *output;
    if (adjusted_skip < 0) {
        const int remaining = -adjusted_skip;
        extent -= remaining;
        --index;
        SpriteClippedYSteps[0] = remaining;
        *source_skip = index;
        output = SpriteClippedYSteps + 1;
        count = 1;
    } else {
        *source_skip = index;
        output = SpriteClippedYSteps;
        count = 0;
    }
    while (extent > 0) {
        const int step = *input++;
        *output++ = step;
        extent -= step;
        ++count;
    }
    if (extent < 0) {
        --output;
        *output = *output + extent;
    }
    *source_count = count;
    return SpriteClippedYSteps;
}

// ORIGINAL: 0x005EAC6F ?get_clipped_x_table_shrink@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z 0x005EAC6F-0x005EACFB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original computes source prefix skip and total source advances from9B3AD8; returned9B53EC is consumed as a pointer by5E503C. Output roles differ from expansion and retain alias-visible writes.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover shrinking X clipping using actual source-advance entries."}
// symbol    ?get_clipped_x_table_shrink@Sprite@@QAEPAHPAUtagRECT@@0PAH111@Z
// The zero-extent return precedes output writes; nonpositive extents retain
// the original signed loop behavior.
int *Sprite::get_clipped_x_table_shrink(RECT *full, RECT *clipped, int *destination_skip,
    int *source_skip, int *destination_count, int *source_count) {
    const int skip = clipped->left - full->left;
    const int extent = clipped->right - clipped->left;
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
        SpriteClippedXSteps[i] = SpriteScalingSteps[i + index];
        *source_count += SpriteScalingSteps[i + index];
    }
    return SpriteClippedXSteps;
}

// ORIGINAL: 0x005EACFF ?get_clipped_y_table_shrink@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z 0x005EACFF-0x005EAD8D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original top/bottom counterpart of5EAC6F returns9B4760. Callers read one int entry per row but pixel kernels explicitly consume its low byte.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover shrinking Y clipping with an independent source-advance table."}
// symbol    ?get_clipped_y_table_shrink@Sprite@@QAEPAHPAUtagRECT@@0PAH111@Z
// Keep the same no-write zero-extent return and signed iteration as X.
int *Sprite::get_clipped_y_table_shrink(RECT *full, RECT *clipped, int *destination_skip,
    int *source_skip, int *destination_count, int *source_count) {
    const int skip = clipped->top - full->top;
    const int extent = clipped->bottom - clipped->top;
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
        SpriteClippedYSteps[i] = SpriteScalingSteps[i + index];
        *source_count += SpriteScalingSteps[i + index];
    }
    return SpriteClippedYSteps;
}

// Clip skips and transient pixel cursors are local, but counts, table cursors
// and pitches use the shared storage above. Pixel loops preserve source order,
// including overlapping source/destination and expansion replication.
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
// RECOVERY_BINDING: v1 {"evidence": ["Pinned3210-byte worker loads file-backed sprites through9B6080; software paths cover1:1, shrinking and expansion. Initial unscaled top skip uses logical width while subsequent rows use pitch. Scaled origins truncate coordinate+offset*ratio, not scaled coordinates. Preserve final free_data and source-byte/run ordering; shared clip counts, table cursors and pitches retain their original storage while transient pixel cursors remain local.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover the monochrome sprite blitter through actual Sprite storage, Buffer locking and scale tables."}
// symbol    ?draw_mono@Sprite@@QAEHPAVBuffer@@HHHH@Z
// The eight workers take code-label addresses in inline assembly, so VC6
// compiles them without optimization, as the image shows. Their frame order
// then follows the local names (VC6 orders /Od locals by a name hash), which
// is why these locals have the names they have.
int Sprite::draw_mono(Buffer *destination, int transparent, int x, int y,
                      int color) {
    int result;
    int source_y;
    int source_col;
    RECT rc_clip;
    int dest_col_ofs;
    unsigned char *dest_ptr;
    RECT original;
    int start_x;
    char *pcx_name;
    unsigned char *source;
    int dest_y;
    int start_y;
    int w;
    int scaled_height;
    int scaled_cx;
    int s_top;
    int sc_left;
    int h;
    if (ppszFileName_ != 0) {
        pcx_name = ppszFileName_;
        result = g_BUFFER.load_pcx(pcx_name, 0, 10, 236);
        if (result != 0) {
            return result;
        }
        result = g_BUFFER.draw_mono(destination, transparent, x, y, color,
                                    SpriteScaleNumerator, SpriteScaleDenominator);
        g_BUFFER.close();
        return result;
    }
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0) {
        return 3;
    }
    if (SpriteScaleNumerator != SpriteScaleDenominator
        && (SpriteCachedScaleNumerator != SpriteScaleNumerator
            || SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        SpriteScalingRatio = static_cast<float>(SpriteScaleNumerator) / SpriteScaleDenominator;
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }
    SpriteRasterMonoColor = static_cast<unsigned char>(color);
    SpriteRasterSourcePitch = iSpriteWidth2_;
    SpriteRasterDestinationPitch = destination->stride_;
    source = pcBits_;
    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        h = iSpriteHeight_;
        w = iSpriteWidth_;
        start_y = y + iTopOffset_;
        start_x = x + iLeftOffset_;
        rc_clip.left = start_x;
        rc_clip.top = start_y;
        rc_clip.right = start_x + w;
        rc_clip.bottom = start_y + h;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        // Deliberate original asymmetry: the clipped top skip uses the
        // logical width, while later rows advance by the pitch.
        source = pcBits_ + (rc_clip.left - original.left)
            + (rc_clip.top - original.top) * iSpriteWidth_;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationRowCount = rc_clip.bottom - rc_clip.top;
        SpriteRasterDestinationColumnCount = rc_clip.right - rc_clip.left;
        SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
        SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterDestinationColumnCount;
        SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
        // Hand-written assembly: computed `lea edx, label` / `jmp edx` entry
        // into an unrolled four-pixel keyed fill.
        __asm {
                push esi
                push edi
                mov esi, dword ptr [source]
                mov edi, dword ptr [dest_ptr]
                mov ecx, dword ptr [SpriteRasterGroupCount]
                mov ebx, dword ptr [SpriteRasterDestinationRowCount]
                mov al, byte ptr [transparent]
                mov ah, byte ptr [color]
                mov edx, dword ptr [SpriteRasterDestinationColumnCount]
                and edx, 0x3
                je A_2e
                dec edx
                je A_36
                dec edx
                je A_45
                dec edx
                je A_54
            A_2e:
                lea edx, A_8a
                jmp A_8a
            A_36:
                lea edx, A_9c
                inc ecx
                mov dword ptr [SpriteRasterGroupCount], ecx
                jmp A_9c
            A_45:
                lea edx, A_96
                inc ecx
                mov dword ptr [SpriteRasterGroupCount], ecx
                jmp A_96
            A_54:
                lea edx, A_90
                inc ecx
                mov dword ptr [SpriteRasterGroupCount], ecx
                jmp A_90
            A_63:
                jmp edx
            A_65:
                cmp byte ptr [esi], al
                je A_8e
            A_69:
                mov byte ptr [edi], ah
                inc esi
                inc edi
                cmp byte ptr [esi], al
                je A_94
            A_71:
                mov byte ptr [edi], ah
                inc esi
                inc edi
                cmp byte ptr [esi], al
                je A_9a
            A_79:
                mov byte ptr [edi], ah
                inc esi
                inc edi
                cmp byte ptr [esi], al
                je A_a0
            A_81:
                mov byte ptr [edi], ah
                inc esi
                inc edi
                dec ecx
                jne A_65
                jmp A_a5
            A_8a:
                cmp byte ptr [esi], al
                jne A_69
            A_8e:
                inc esi
                inc edi
            A_90:
                cmp byte ptr [esi], al
                jne A_71
            A_94:
                inc esi
                inc edi
            A_96:
                cmp byte ptr [esi], al
                jne A_79
            A_9a:
                inc esi
                inc edi
            A_9c:
                cmp byte ptr [esi], al
                jne A_81
            A_a0:
                inc esi
                inc edi
                dec ecx
                jne A_8a
            A_a5:
                mov ecx, dword ptr [SpriteRasterGroupCount]
                add esi, dword ptr [SpriteRasterSourceRowTail]
                add edi, dword ptr [SpriteRasterDestinationRowTail]
                dec ebx
                jne A_63
                pop edi
                pop esi
        }
    } else {
        scaled_height = static_cast<int>(iSpriteHeight_ * SpriteScalingRatio);
        scaled_cx = static_cast<int>(iSpriteWidth_ * SpriteScalingRatio);
        s_top = static_cast<int>(y + iTopOffset_ * SpriteScalingRatio);
        sc_left = static_cast<int>(x + iLeftOffset_ * SpriteScalingRatio);
        rc_clip.left = sc_left;
        rc_clip.top = s_top;
        rc_clip.right = sc_left + scaled_cx;
        rc_clip.bottom = s_top + scaled_height;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        rc_clip = original;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationPitch = destination->stride_;
        if (SpriteScaleNumerator < SpriteScaleDenominator) {
            if (original.right - original.left < SpriteMinimumScaledWidth
                || original.bottom - original.top < SpriteMinimumScaledHeight) {
                destination->free_data(1);
                return 0;
            }
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_shrink(&original, &rc_clip,
                &dest_col_ofs, &source_col, &SpriteRasterDestinationColumnCount,
                &SpriteRasterSourceColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_shrink(&original, &rc_clip,
                &dest_y, &source_y, &SpriteRasterDestinationRowCount,
                &SpriteRasterSourceRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            // Hand-written assembly: one-operand MUL row offset and computed
            // `jmp ebx` entry into an X-table stepped keyed fill.
            __asm {
                    push esi
                    push edi
                    mov eax, dword ptr [SpriteRasterSourcePitch]
                    mul dword ptr [source_y]
                    mov esi, eax
                    add esi, dword ptr [source_col]
                    add esi, dword ptr [source]
                    mov edi, dword ptr [dest_ptr]
                    mov edx, dword ptr [SpriteRasterXTable]
                    mov ecx, dword ptr [SpriteRasterGroupCount]
                    mov al, byte ptr [transparent]
                    mov ah, byte ptr [SpriteRasterMonoColor]
                    mov ebx, dword ptr [SpriteRasterDestinationColumnCount]
                    and ebx, 0x3
                    je B_3e
                    dec ebx
                    je B_46
                    dec ebx
                    je B_55
                    dec ebx
                    je B_64
                B_3e:
                    lea ebx, B_aa
                    jmp B_aa
                B_46:
                    lea ebx, B_c8
                    inc ecx
                    mov dword ptr [SpriteRasterGroupCount], ecx
                    jmp B_c8
                B_55:
                    lea ebx, B_be
                    inc ecx
                    mov dword ptr [SpriteRasterGroupCount], ecx
                    jmp B_be
                B_64:
                    lea ebx, B_b4
                    inc ecx
                    mov dword ptr [SpriteRasterGroupCount], ecx
                    jmp B_b4
                B_73:
                    jmp ebx
                B_75:
                    cmp byte ptr [esi], al
                    je B_ae
                B_79:
                    mov byte ptr [edi], ah
                    add esi, dword ptr [edx]
                    inc edi
                    add edx, 0x4
                    cmp byte ptr [esi], al
                    je B_b8
                B_85:
                    mov byte ptr [edi], ah
                    add esi, dword ptr [edx]
                    inc edi
                    add edx, 0x4
                    cmp byte ptr [esi], al
                    je B_c2
                B_91:
                    mov byte ptr [edi], ah
                    add esi, dword ptr [edx]
                    inc edi
                    add edx, 0x4
                    cmp byte ptr [esi], al
                    je B_cc
                B_9d:
                    mov byte ptr [edi], ah
                    add esi, dword ptr [edx]
                    inc edi
                    add edx, 0x4
                    dec ecx
                    jne B_75
                    jmp B_d5
                B_aa:
                    cmp byte ptr [esi], al
                    jne B_79
                B_ae:
                    inc edi
                    add esi, dword ptr [edx]
                    add edx, 0x4
                B_b4:
                    cmp byte ptr [esi], al
                    jne B_85
                B_b8:
                    inc edi
                    add esi, dword ptr [edx]
                    add edx, 0x4
                B_be:
                    cmp byte ptr [esi], al
                    jne B_91
                B_c2:
                    inc edi
                    add esi, dword ptr [edx]
                    add edx, 0x4
                B_c8:
                    cmp byte ptr [esi], al
                    jne B_9d
                B_cc:
                    inc edi
                    add esi, dword ptr [edx]
                    add edx, 0x4
                    dec ecx
                    jne B_aa
                B_d5:
                    mov ecx, dword ptr [SpriteRasterYTable]
                    mov dl, byte ptr [ecx]
                    add ecx, 0x4
                    mov dword ptr [SpriteRasterYTable], ecx
                    xor ecx, ecx
                    mov cl, dl
                    dec ecx
                    je B_f6
                B_ed:
                    add esi, dword ptr [SpriteRasterSourcePitch]
                    dec ecx
                    jne B_ed
                B_f6:
                    add esi, dword ptr [SpriteRasterSourceRowTail]
                    add edi, dword ptr [SpriteRasterDestinationRowTail]
                    mov edx, dword ptr [SpriteRasterXTable]
                    mov ecx, dword ptr [SpriteRasterDestinationRowCount]
                    dec ecx
                    mov dword ptr [SpriteRasterDestinationRowCount], ecx
                    mov ecx, dword ptr [SpriteRasterGroupCount]
                    jne B_73
                    pop edi
                    pop esi
            }
        } else {
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_expand(&original, &rc_clip,
                &source_col, &dest_col_ofs, &SpriteRasterSourceColumnCount,
                &SpriteRasterDestinationColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_expand(&original, &rc_clip,
                &source_y, &dest_y, &SpriteRasterSourceRowCount,
                &SpriteRasterDestinationRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterGroupCount = SpriteRasterSourceColumnCount >> 2;
            SpriteRasterGroupRemainder = SpriteRasterSourceColumnCount - (SpriteRasterGroupCount << 2);
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            // Hand-written assembly: 16-bit PUSHF/POPF around CLD, EBP as the
            // row counter, and replicated expansion rows.
            __asm {
                    push edi
                    push esi
                    pushf
                    cld
                    mov eax, dword ptr [SpriteRasterSourcePitch]
                    mul dword ptr [source_y]
                    mov esi, eax
                    add esi, dword ptr [source_col]
                    add esi, dword ptr [source]
                    mov edi, dword ptr [dest_ptr]
                    mov edx, dword ptr [SpriteRasterXTable]
                    mov ah, byte ptr [transparent]
                    push ebp
                    mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                    and ebx, 0x3
                    je C_3a
                    mov ecx, dword ptr [SpriteRasterGroupCount]
                    inc ecx
                    mov dword ptr [SpriteRasterGroupCount], ecx
                C_3a:
                    mov ecx, dword ptr [SpriteRasterYTable]
                    mov al, byte ptr [ecx]
                    add ecx, 0x4
                    mov dword ptr [SpriteRasterYTable], ecx
                    mov ebp, dword ptr [SpriteRasterGroupCount]
                    mov edx, dword ptr [SpriteRasterXTable]
                    cmp al, 0x1
                    je C_43f
                    cmp al, 0x2
                    je C_341
                    cmp al, 0x3
                    je C_1f7
                    mov al, byte ptr [SpriteRasterMonoColor]
                    mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                    and ebx, 0x3
                    je C_a9
                    dec ebx
                    je C_88
                    dec ebx
                    je C_93
                    dec ebx
                    je C_9e
                C_88:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_1df
                C_93:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_1d1
                C_9e:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_1c3
                C_a9:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                C_af:
                    cmp byte ptr [esi], ah
                    je C_1bd
                C_b7:
                    mov ecx, dword ptr [edx]
                C_b9:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_b9
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_c5:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_c5
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_d1:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_d1
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_dd:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_dd
                    sub edi, ebx
                    sub edi, ebx
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_1cb
                C_f5:
                    mov ecx, dword ptr [edx]
                C_f7:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_f7
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_103:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_103
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_10f:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_10f
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_11b:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_11b
                    sub edi, ebx
                    sub edi, ebx
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_1d9
                C_133:
                    mov ecx, dword ptr [edx]
                C_135:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_135
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_141:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_141
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_14d:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_14d
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_159:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_159
                    sub edi, ebx
                    sub edi, ebx
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_1e3
                C_16d:
                    mov ecx, dword ptr [edx]
                C_16f:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_16f
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_17b:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_17b
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_187:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_187
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_193:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_193
                    sub edi, ebx
                    sub edi, ebx
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    dec ebp
                    jne C_af
                    lea edi, [edi+2*ebx]
                    lea edi, [edi+ebx]
                    jmp C_4e6
                C_1b5:
                    cmp byte ptr [esi], ah
                    jne C_b7
                C_1bd:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_1c3:
                    cmp byte ptr [esi], ah
                    jne C_f5
                C_1cb:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_1d1:
                    cmp byte ptr [esi], ah
                    jne C_133
                C_1d9:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_1df:
                    cmp byte ptr [esi], ah
                    jne C_16d
                C_1e3:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                    dec ebp
                    jne C_1b5
                    lea edi, [edi+2*ebx]
                    lea edi, [edi+ebx]
                    jmp C_4e6
                C_1f7:
                    mov al, byte ptr [SpriteRasterMonoColor]
                    mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                    and ebx, 0x3
                    je C_231
                    dec ebx
                    je C_210
                    dec ebx
                    je C_21b
                    dec ebx
                    je C_226
                C_210:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_32c
                C_21b:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_31e
                C_226:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_310
                C_231:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                C_237:
                    cmp byte ptr [esi], ah
                    je C_30a
                C_23f:
                    mov ecx, dword ptr [edx]
                C_241:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_241
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_24d:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_24d
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_259:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_259
                    sub edi, ebx
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_318
                C_26f:
                    mov ecx, dword ptr [edx]
                C_271:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_271
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_27d:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_27d
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_289:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_289
                    sub edi, ebx
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_326
                C_29f:
                    mov ecx, dword ptr [edx]
                C_2a1:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_2a1
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_2ad:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_2ad
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_2b9:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_2b9
                    sub edi, ebx
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_330
                C_2cb:
                    mov ecx, dword ptr [edx]
                C_2cd:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_2cd
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_2d9:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_2d9
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_2e5:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_2e5
                    sub edi, ebx
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    dec ebp
                    jne C_237
                    lea edi, [edi+2*ebx]
                    jmp C_4e6
                C_302:
                    cmp byte ptr [esi], ah
                    jne C_23f
                C_30a:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_310:
                    cmp byte ptr [esi], ah
                    jne C_26f
                C_318:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_31e:
                    cmp byte ptr [esi], ah
                    jne C_29f
                C_326:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_32c:
                    cmp byte ptr [esi], ah
                    jne C_2cb
                C_330:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                    dec ebp
                    jne C_302
                    lea edi, [edi+2*ebx]
                    jmp C_4e6
                C_341:
                    mov al, byte ptr [SpriteRasterMonoColor]
                    mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                    and ebx, 0x3
                    je C_37b
                    dec ebx
                    je C_35a
                    dec ebx
                    je C_365
                    dec ebx
                    je C_370
                C_35a:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_42a
                C_365:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_420
                C_370:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_416
                C_37b:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                C_381:
                    cmp byte ptr [esi], ah
                    je C_410
                C_389:
                    mov ecx, dword ptr [edx]
                C_38b:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_38b
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_397:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_397
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_41a
                C_3a7:
                    mov ecx, dword ptr [edx]
                C_3a9:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_3a9
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_3b5:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_3b5
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_424
                C_3c5:
                    mov ecx, dword ptr [edx]
                C_3c7:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_3c7
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_3d3:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_3d3
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_42e
                C_3e3:
                    mov ecx, dword ptr [edx]
                C_3e5:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_3e5
                    add edi, ebx
                    sub edi, dword ptr [edx]
                    mov ecx, dword ptr [edx]
                C_3f1:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_3f1
                    sub edi, ebx
                    inc esi
                    add edx, 0x4
                    dec ebp
                    jne C_381
                    lea edi, [edi+ebx]
                    jmp C_4e6
                C_408:
                    cmp byte ptr [esi], ah
                    jne C_389
                C_410:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_416:
                    cmp byte ptr [esi], ah
                    jne C_3a7
                C_41a:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_420:
                    cmp byte ptr [esi], ah
                    jne C_3c5
                C_424:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_42a:
                    cmp byte ptr [esi], ah
                    jne C_3e3
                C_42e:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                    dec ebp
                    jne C_408
                    lea edi, [edi+ebx]
                    jmp C_4e6
                C_43f:
                    mov al, byte ptr [SpriteRasterMonoColor]
                    mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                    and ebx, 0x3
                    je C_470
                    dec ebx
                    je C_458
                    dec ebx
                    je C_460
                    dec ebx
                    je C_468
                C_458:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_4d9
                C_460:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_4cf
                C_468:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    jmp C_4c5
                C_470:
                    mov ebx, dword ptr [SpriteRasterDestinationPitch]
                C_476:
                    cmp byte ptr [esi], ah
                    je C_4bf
                C_47a:
                    mov ecx, dword ptr [edx]
                C_47c:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_47c
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_4c9
                C_48a:
                    mov ecx, dword ptr [edx]
                C_48c:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_48c
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_4d3
                C_49a:
                    mov ecx, dword ptr [edx]
                C_49c:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_49c
                    inc esi
                    add edx, 0x4
                    cmp byte ptr [esi], ah
                    je C_4dd
                C_4aa:
                    mov ecx, dword ptr [edx]
                C_4ac:
                    mov byte ptr [edi], al
                    inc edi
                    dec ecx
                    jne C_4ac
                    inc esi
                    add edx, 0x4
                    dec ebp
                    jne C_476
                    jmp C_4e6
                C_4bb:
                    cmp byte ptr [esi], ah
                    jne C_47a
                C_4bf:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_4c5:
                    cmp byte ptr [esi], ah
                    jne C_48a
                C_4c9:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_4cf:
                    cmp byte ptr [esi], ah
                    jne C_49a
                C_4d3:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                C_4d9:
                    cmp byte ptr [esi], ah
                    jne C_4aa
                C_4dd:
                    inc esi
                    add edi, dword ptr [edx]
                    add edx, 0x4
                    dec ebp
                    jne C_4bb
                C_4e6:
                    add esi, dword ptr [SpriteRasterSourceRowTail]
                    add edi, dword ptr [SpriteRasterDestinationRowTail]
                    mov ecx, dword ptr [SpriteRasterSourceRowCount]
                    dec ecx
                    mov dword ptr [SpriteRasterSourceRowCount], ecx
                    jne C_3a
                    pop ebp
                    popf
                    pop esi
                    pop edi
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
    int result;
    int source_y;
    int source_col;
    RECT rc_clip;
    int dest_col_ofs;
    unsigned char *dest_ptr;
    RECT original;
    int start_x;
    char *pcx_name;
    unsigned char *source;
    int dest_y;
    int start_y;
    int w;
    int scaled_height;
    int scaled_cx;
    int s_top;
    int sc_left;
    int h;
    if (ppszFileName_ != 0) {
        pcx_name = ppszFileName_;
        result = g_BUFFER.load_pcx(pcx_name, 0, 10, 236);
        if (result != 0) {
            return result;
        }
        result = g_BUFFER.draw(destination, transparent, x, y,
                               SpriteScaleNumerator, SpriteScaleDenominator);
        g_BUFFER.close();
        return result;
    }
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0) {
        return 3;
    }
    if (SpriteScaleNumerator != SpriteScaleDenominator
        && (SpriteCachedScaleNumerator != SpriteScaleNumerator
            || SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        SpriteScalingRatio = static_cast<float>(SpriteScaleNumerator) / SpriteScaleDenominator;
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }
    transparent = static_cast<unsigned char>(transparent);
    SpriteRasterSourcePitch = iSpriteWidth2_;
    SpriteRasterDestinationPitch = destination->stride_;
    source = pcBits_;
    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        h = iSpriteHeight_;
        w = iSpriteWidth_;
        start_y = y + iTopOffset_;
        start_x = x + iLeftOffset_;
        rc_clip.left = start_x;
        rc_clip.top = start_y;
        rc_clip.right = start_x + w;
        rc_clip.bottom = start_y + h;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        source = pcBits_ + (rc_clip.left - original.left)
            + (rc_clip.top - original.top) * SpriteRasterSourcePitch;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationRowCount = rc_clip.bottom - rc_clip.top;
        SpriteRasterDestinationColumnCount = rc_clip.right - rc_clip.left;
        SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
        SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterDestinationColumnCount;
        SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
        // Hand-written assembly: computed `lea edx, label` / `jmp edx` entry
        // into an unrolled four-pixel keyed fill.
        __asm {
                    push esi
                    push edi
                    mov esi, dword ptr [source]
                    mov edi, dword ptr [dest_ptr]
                    mov ecx, dword ptr [SpriteRasterGroupCount]
                    mov ebx, dword ptr [SpriteRasterDestinationRowCount]
                    mov al, byte ptr [transparent]
                    mov edx, dword ptr [SpriteRasterDestinationColumnCount]
                    and edx, 0x3
                    je D1_2b
                    dec edx
                    je D1_33
                    dec edx
                    je D1_42
                    dec edx
                    je D1_51
                D1_2b:
                    lea edx, D1_8f
                    jmp D1_8f
                D1_33:
                    lea edx, D1_a1
                    inc ecx
                    mov dword ptr [SpriteRasterGroupCount], ecx
                    jmp D1_a1
                D1_42:
                    lea edx, D1_9b
                    inc ecx
                    mov dword ptr [SpriteRasterGroupCount], ecx
                    jmp D1_9b
                D1_51:
                    lea edx, D1_95
                    inc ecx
                    mov dword ptr [SpriteRasterGroupCount], ecx
                    jmp D1_95
                D1_60:
                    jmp edx
                D1_62:
                    cmp byte ptr [esi], al
                    je D1_93
                D1_66:
                    mov ah, byte ptr [esi]
                    mov byte ptr [edi], ah
                    inc esi
                    inc edi
                    cmp byte ptr [esi], al
                    je D1_99
                D1_70:
                    mov ah, byte ptr [esi]
                    mov byte ptr [edi], ah
                    inc esi
                    inc edi
                    cmp byte ptr [esi], al
                    je D1_9f
                D1_7a:
                    mov ah, byte ptr [esi]
                    mov byte ptr [edi], ah
                    inc esi
                    inc edi
                    cmp byte ptr [esi], al
                    je D1_a5
                D1_84:
                    mov ah, byte ptr [esi]
                    mov byte ptr [edi], ah
                    inc esi
                    inc edi
                    dec ecx
                    jne D1_62
                    jmp D1_aa
                D1_8f:
                    cmp byte ptr [esi], al
                    jne D1_66
                D1_93:
                    inc esi
                    inc edi
                D1_95:
                    cmp byte ptr [esi], al
                    jne D1_70
                D1_99:
                    inc esi
                    inc edi
                D1_9b:
                    cmp byte ptr [esi], al
                    jne D1_7a
                D1_9f:
                    inc esi
                    inc edi
                D1_a1:
                    cmp byte ptr [esi], al
                    jne D1_84
                D1_a5:
                    inc esi
                    inc edi
                    dec ecx
                    jne D1_8f
                D1_aa:
                    mov ecx, dword ptr [SpriteRasterGroupCount]
                    add esi, dword ptr [SpriteRasterSourceRowTail]
                    add edi, dword ptr [SpriteRasterDestinationRowTail]
                    dec ebx
                    jne D1_60
                    pop edi
                    pop esi
        }
    } else {
        scaled_height = static_cast<int>(iSpriteHeight_ * SpriteScalingRatio);
        scaled_cx = static_cast<int>(iSpriteWidth_ * SpriteScalingRatio);
        s_top = static_cast<int>(y + iTopOffset_ * SpriteScalingRatio);
        sc_left = static_cast<int>(x + iLeftOffset_ * SpriteScalingRatio);
        rc_clip.left = sc_left;
        rc_clip.top = s_top;
        rc_clip.right = sc_left + scaled_cx;
        rc_clip.bottom = s_top + scaled_height;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        rc_clip = original;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationPitch = destination->stride_;
        if (SpriteScaleNumerator < SpriteScaleDenominator) {
            if (original.right - original.left < SpriteMinimumScaledWidth
                || original.bottom - original.top < SpriteMinimumScaledHeight) {
                destination->free_data(1);
                return 0;
            }
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_shrink(&original, &rc_clip,
                &dest_col_ofs, &source_col, &SpriteRasterDestinationColumnCount,
                &SpriteRasterSourceColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_shrink(&original, &rc_clip,
                &dest_y, &source_y, &SpriteRasterDestinationRowCount,
                &SpriteRasterSourceRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            // Hand-written assembly: one-operand MUL row offset and computed
            // `jmp ebx` entry into an X-table stepped keyed fill.
            __asm {
                        push esi
                        push edi
                        mov eax, dword ptr [SpriteRasterSourcePitch]
                        mul dword ptr [source_y]
                        mov esi, eax
                        add esi, dword ptr [source_col]
                        add esi, dword ptr [source]
                        mov edi, dword ptr [dest_ptr]
                        mov edx, dword ptr [SpriteRasterXTable]
                        mov ecx, dword ptr [SpriteRasterGroupCount]
                        mov ah, byte ptr [transparent]
                        mov ebx, dword ptr [SpriteRasterDestinationColumnCount]
                        and ebx, 0x3
                        je D2_38
                        dec ebx
                        je D2_40
                        dec ebx
                        je D2_4f
                        dec ebx
                        je D2_5e
                    D2_38:
                        lea ebx, D2_ac
                        jmp D2_ac
                    D2_40:
                        lea ebx, D2_ca
                        inc ecx
                        mov dword ptr [SpriteRasterGroupCount], ecx
                        jmp D2_ca
                    D2_4f:
                        lea ebx, D2_c0
                        inc ecx
                        mov dword ptr [SpriteRasterGroupCount], ecx
                        jmp D2_c0
                    D2_5e:
                        lea ebx, D2_b6
                        inc ecx
                        mov dword ptr [SpriteRasterGroupCount], ecx
                        jmp D2_b6
                    D2_6d:
                        jmp ebx
                    D2_6f:
                        cmp byte ptr [esi], ah
                        je D2_b0
                    D2_73:
                        mov al, byte ptr [esi]
                        mov byte ptr [edi], al
                        add esi, dword ptr [edx]
                        inc edi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D2_ba
                    D2_81:
                        mov al, byte ptr [esi]
                        mov byte ptr [edi], al
                        add esi, dword ptr [edx]
                        inc edi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D2_c4
                    D2_8f:
                        mov al, byte ptr [esi]
                        mov byte ptr [edi], al
                        add esi, dword ptr [edx]
                        inc edi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D2_ce
                    D2_9d:
                        mov al, byte ptr [esi]
                        mov byte ptr [edi], al
                        add esi, dword ptr [edx]
                        inc edi
                        add edx, 0x4
                        dec ecx
                        jne D2_6f
                        jmp D2_d7
                    D2_ac:
                        cmp byte ptr [esi], ah
                        jne D2_73
                    D2_b0:
                        inc edi
                        add esi, dword ptr [edx]
                        add edx, 0x4
                    D2_b6:
                        cmp byte ptr [esi], ah
                        jne D2_81
                    D2_ba:
                        inc edi
                        add esi, dword ptr [edx]
                        add edx, 0x4
                    D2_c0:
                        cmp byte ptr [esi], ah
                        jne D2_8f
                    D2_c4:
                        inc edi
                        add esi, dword ptr [edx]
                        add edx, 0x4
                    D2_ca:
                        cmp byte ptr [esi], ah
                        jne D2_9d
                    D2_ce:
                        inc edi
                        add esi, dword ptr [edx]
                        add edx, 0x4
                        dec ecx
                        jne D2_ac
                    D2_d7:
                        mov ecx, dword ptr [SpriteRasterYTable]
                        mov al, byte ptr [ecx]
                        add ecx, 0x4
                        mov dword ptr [SpriteRasterYTable], ecx
                        xor ecx, ecx
                        mov cl, al
                        dec ecx
                        je D2_f8
                    D2_ef:
                        add esi, dword ptr [SpriteRasterSourcePitch]
                        dec ecx
                        jne D2_ef
                    D2_f8:
                        add esi, dword ptr [SpriteRasterSourceRowTail]
                        add edi, dword ptr [SpriteRasterDestinationRowTail]
                        mov edx, dword ptr [SpriteRasterXTable]
                        mov ecx, dword ptr [SpriteRasterDestinationRowCount]
                        dec ecx
                        mov dword ptr [SpriteRasterDestinationRowCount], ecx
                        mov ecx, dword ptr [SpriteRasterGroupCount]
                        jne D2_6d
                        pop edi
                        pop esi
            }
        } else {
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_expand(&original, &rc_clip,
                &source_col, &dest_col_ofs, &SpriteRasterSourceColumnCount,
                &SpriteRasterDestinationColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_expand(&original, &rc_clip,
                &source_y, &dest_y, &SpriteRasterSourceRowCount,
                &SpriteRasterDestinationRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterGroupCount = SpriteRasterSourceColumnCount >> 2;
            SpriteRasterGroupRemainder = SpriteRasterSourceColumnCount - (SpriteRasterGroupCount << 2);
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            // Hand-written assembly: 16-bit PUSHF/POPF around CLD, EBP as the
            // row counter, and replicated expansion rows.
            __asm {
                        push edi
                        push esi
                        pushf
                        cld
                        mov eax, dword ptr [SpriteRasterSourcePitch]
                        mul dword ptr [source_y]
                        mov esi, eax
                        add esi, dword ptr [source_col]
                        add esi, dword ptr [source]
                        mov edi, dword ptr [dest_ptr]
                        mov edx, dword ptr [SpriteRasterXTable]
                        mov ah, byte ptr [transparent]
                        mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                        and ebx, 0x3
                        push ebp
                        je D3_3a
                        mov ecx, dword ptr [SpriteRasterGroupCount]
                        inc ecx
                        mov dword ptr [SpriteRasterGroupCount], ecx
                    D3_3a:
                        mov ecx, dword ptr [SpriteRasterYTable]
                        mov al, byte ptr [ecx]
                        add ecx, 0x4
                        mov dword ptr [SpriteRasterYTable], ecx
                        mov ebp, dword ptr [SpriteRasterGroupCount]
                        mov edx, dword ptr [SpriteRasterXTable]
                        cmp al, 0x1
                        je D3_44c
                        cmp al, 0x2
                        je D3_347
                        cmp al, 0x3
                        je D3_1fa
                        mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                        and ebx, 0x3
                        je D3_a4
                        dec ebx
                        je D3_83
                        dec ebx
                        je D3_8e
                        dec ebx
                        je D3_99
                    D3_83:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_1e2
                    D3_8e:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_1d4
                    D3_99:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_1c6
                    D3_a4:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    D3_aa:
                        cmp byte ptr [esi], ah
                        je D3_1c0
                    D3_b2:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_b6:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_b6
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_c2:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_c2
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_ce:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_ce
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_da:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_da
                        sub edi, ebx
                        sub edi, ebx
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_1ce
                    D3_f2:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_f6:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_f6
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_102:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_102
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_10e:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_10e
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_11a:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_11a
                        sub edi, ebx
                        sub edi, ebx
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_1dc
                    D3_132:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_136:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_136
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_142:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_142
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_14e:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_14e
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_15a:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_15a
                        sub edi, ebx
                        sub edi, ebx
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_1e6
                    D3_16e:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_172:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_172
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_17e:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_17e
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_18a:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_18a
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_196:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_196
                        sub edi, ebx
                        sub edi, ebx
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        dec ebp
                        jne D3_aa
                        lea edi, [edi+2*ebx]
                        lea edi, [edi+ebx]
                        jmp D3_4f9
                    D3_1b8:
                        cmp byte ptr [esi], ah
                        jne D3_b2
                    D3_1c0:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_1c6:
                        cmp byte ptr [esi], ah
                        jne D3_f2
                    D3_1ce:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_1d4:
                        cmp byte ptr [esi], ah
                        jne D3_132
                    D3_1dc:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_1e2:
                        cmp byte ptr [esi], ah
                        jne D3_16e
                    D3_1e6:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                        dec ebp
                        jne D3_1b8
                        lea edi, [edi+2*ebx]
                        lea edi, [edi+ebx]
                        jmp D3_4f9
                    D3_1fa:
                        mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                        and ebx, 0x3
                        je D3_22f
                        dec ebx
                        je D3_20e
                        dec ebx
                        je D3_219
                        dec ebx
                        je D3_224
                    D3_20e:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_332
                    D3_219:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_324
                    D3_224:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_316
                    D3_22f:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    D3_235:
                        cmp byte ptr [esi], ah
                        je D3_310
                    D3_23d:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_241:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_241
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_24d:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_24d
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_259:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_259
                        sub edi, ebx
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_31e
                    D3_26f:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_273:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_273
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_27f:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_27f
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_28b:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_28b
                        sub edi, ebx
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_32c
                    D3_2a1:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_2a5:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_2a5
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_2b1:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_2b1
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_2bd:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_2bd
                        sub edi, ebx
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_336
                    D3_2cf:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_2d3:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_2d3
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_2df:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_2df
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_2eb:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_2eb
                        sub edi, ebx
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        dec ebp
                        jne D3_235
                        lea edi, [edi+2*ebx]
                        jmp D3_4f9
                    D3_308:
                        cmp byte ptr [esi], ah
                        jne D3_23d
                    D3_310:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_316:
                        cmp byte ptr [esi], ah
                        jne D3_26f
                    D3_31e:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_324:
                        cmp byte ptr [esi], ah
                        jne D3_2a1
                    D3_32c:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_332:
                        cmp byte ptr [esi], ah
                        jne D3_2cf
                    D3_336:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                        dec ebp
                        jne D3_308
                        lea edi, [edi+2*ebx]
                        jmp D3_4f9
                    D3_347:
                        mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                        and ebx, 0x3
                        je D3_37c
                        dec ebx
                        je D3_35b
                        dec ebx
                        je D3_366
                        dec ebx
                        je D3_371
                    D3_35b:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_437
                    D3_366:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_42d
                    D3_371:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_423
                    D3_37c:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    D3_382:
                        cmp byte ptr [esi], ah
                        je D3_41d
                    D3_38a:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_38e:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_38e
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_39a:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_39a
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_427
                    D3_3aa:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_3ae:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_3ae
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_3ba:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_3ba
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_431
                    D3_3ca:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_3ce:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_3ce
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_3da:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_3da
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_43b
                    D3_3ea:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_3ee:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_3ee
                        add edi, ebx
                        sub edi, dword ptr [edx]
                        mov ecx, dword ptr [edx]
                    D3_3fa:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_3fa
                        sub edi, ebx
                        inc esi
                        add edx, 0x4
                        dec ebp
                        jne D3_382
                        lea edi, [edi+ebx]
                        jmp D3_4f9
                    D3_415:
                        cmp byte ptr [esi], ah
                        jne D3_38a
                    D3_41d:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_423:
                        cmp byte ptr [esi], ah
                        jne D3_3aa
                    D3_427:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_42d:
                        cmp byte ptr [esi], ah
                        jne D3_3ca
                    D3_431:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_437:
                        cmp byte ptr [esi], ah
                        jne D3_3ea
                    D3_43b:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                        dec ebp
                        jne D3_415
                        lea edi, [edi+ebx]
                        jmp D3_4f9
                    D3_44c:
                        mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                        and ebx, 0x3
                        je D3_47b
                        dec ebx
                        je D3_460
                        dec ebx
                        je D3_46b
                        dec ebx
                        je D3_473
                    D3_460:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_4ec
                    D3_46b:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_4e2
                    D3_473:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                        jmp D3_4d8
                    D3_47b:
                        mov ebx, dword ptr [SpriteRasterDestinationPitch]
                    D3_481:
                        cmp byte ptr [esi], ah
                        je D3_4d2
                    D3_485:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_489:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_489
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_4dc
                    D3_497:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_49b:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_49b
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_4e6
                    D3_4a9:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_4ad:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_4ad
                        inc esi
                        add edx, 0x4
                        cmp byte ptr [esi], ah
                        je D3_4f0
                    D3_4bb:
                        mov al, byte ptr [esi]
                        mov ecx, dword ptr [edx]
                    D3_4bf:
                        mov byte ptr [edi], al
                        inc edi
                        dec ecx
                        jne D3_4bf
                        inc esi
                        add edx, 0x4
                        dec ebp
                        jne D3_481
                        jmp D3_4f9
                    D3_4ce:
                        cmp byte ptr [esi], ah
                        jne D3_485
                    D3_4d2:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_4d8:
                        cmp byte ptr [esi], ah
                        jne D3_497
                    D3_4dc:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_4e2:
                        cmp byte ptr [esi], ah
                        jne D3_4a9
                    D3_4e6:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                    D3_4ec:
                        cmp byte ptr [esi], ah
                        jne D3_4bb
                    D3_4f0:
                        inc esi
                        add edi, dword ptr [edx]
                        add edx, 0x4
                        dec ebp
                        jne D3_4ce
                    D3_4f9:
                        add esi, dword ptr [SpriteRasterSourceRowTail]
                        add edi, dword ptr [SpriteRasterDestinationRowTail]
                        mov ecx, dword ptr [SpriteRasterSourceRowCount]
                        dec ecx
                        mov dword ptr [SpriteRasterSourceRowCount], ecx
                        jne D3_3a
                        pop ebp
                        popf
                        pop esi
                        pop edi
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
    int source_y;
    int source_col;
    RECT rc_clip;
    int dest_col_ofs;
    unsigned char *dest_ptr;
    RECT original;
    int start_x;
    unsigned char *source;
    int dest_y;
    int start_y;
    int w;
    int scaled_height;
    int scaled_cx;
    int s_top;
    int sc_left;
    int h;
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0) {
        return 3;
    }
    if (SpriteScaleNumerator != SpriteScaleDenominator
        && (SpriteCachedScaleNumerator != SpriteScaleNumerator
            || SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        SpriteScalingRatio = static_cast<float>(SpriteScaleNumerator) / SpriteScaleDenominator;
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }
    transparent = static_cast<unsigned char>(transparent);
    SpriteRasterSourcePitch = iSpriteWidth2_;
    SpriteRasterDestinationPitch = destination->stride_;
    source = pcBits_;
    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        h = iSpriteHeight_;
        w = iSpriteWidth_;
        start_y = y + iTopOffset_;
        start_x = x + iLeftOffset_;
        rc_clip.left = start_x;
        rc_clip.top = start_y;
        rc_clip.right = start_x + w;
        rc_clip.bottom = start_y + h;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        source = pcBits_ + (rc_clip.left - original.left)
            + (rc_clip.top - original.top) * SpriteRasterSourcePitch;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationRowCount = rc_clip.bottom - rc_clip.top;
        SpriteRasterDestinationColumnCount = rc_clip.right - rc_clip.left;
        SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
        SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterDestinationColumnCount;
        SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
        // Hand-written assembly: computed `lea edx, label` / `jmp edx` entry
        // into an unrolled four-pixel keyed fill.
        __asm {
                        push esi
                        push edi
                        mov esi, dword ptr [source]
                        mov edi, dword ptr [dest_ptr]
                        mov ecx, dword ptr [SpriteRasterGroupCount]
                        mov al, byte ptr [transparent]
                        mov edx, dword ptr [table]
                        xor ebx, ebx
                        push ebp
                        mov ebp, edx
                        mov edx, dword ptr [SpriteRasterDestinationColumnCount]
                        and edx, 0x3
                        je T1_2d
                        dec edx
                        je T1_35
                        dec edx
                        je T1_44
                        dec edx
                        je T1_53
                    T1_2d:
                        lea edx, T1_a1
                        jmp T1_a1
                    T1_35:
                        lea edx, T1_b3
                        inc ecx
                        mov dword ptr [SpriteRasterGroupCount], ecx
                        jmp T1_b3
                    T1_44:
                        lea edx, T1_ad
                        inc ecx
                        mov dword ptr [SpriteRasterGroupCount], ecx
                        jmp T1_ad
                    T1_53:
                        lea edx, T1_a7
                        inc ecx
                        mov dword ptr [SpriteRasterGroupCount], ecx
                        jmp T1_a7
                    T1_62:
                        jmp edx
                    T1_64:
                        cmp byte ptr [esi], al
                        je T1_a5
                    T1_68:
                        mov bl, byte ptr [esi]
                        mov bl, byte ptr [ebp+ebx]
                        mov byte ptr [edi], bl
                        inc esi
                        inc edi
                        cmp byte ptr [esi], al
                        je T1_ab
                    T1_76:
                        mov bl, byte ptr [esi]
                        mov bl, byte ptr [ebp+ebx]
                        mov byte ptr [edi], bl
                        inc esi
                        inc edi
                        cmp byte ptr [esi], al
                        je T1_b1
                    T1_84:
                        mov bl, byte ptr [esi]
                        mov bl, byte ptr [ebp+ebx]
                        mov byte ptr [edi], bl
                        inc esi
                        inc edi
                        cmp byte ptr [esi], al
                        je T1_b7
                    T1_92:
                        mov bl, byte ptr [esi]
                        mov bl, byte ptr [ebp+ebx]
                        mov byte ptr [edi], bl
                        inc esi
                        inc edi
                        dec ecx
                        jne T1_64
                        jmp T1_bc
                    T1_a1:
                        cmp byte ptr [esi], al
                        jne T1_68
                    T1_a5:
                        inc esi
                        inc edi
                    T1_a7:
                        cmp byte ptr [esi], al
                        jne T1_76
                    T1_ab:
                        inc esi
                        inc edi
                    T1_ad:
                        cmp byte ptr [esi], al
                        jne T1_84
                    T1_b1:
                        inc esi
                        inc edi
                    T1_b3:
                        cmp byte ptr [esi], al
                        jne T1_92
                    T1_b7:
                        inc esi
                        inc edi
                        dec ecx
                        jne T1_a1
                    T1_bc:
                        mov ecx, dword ptr [SpriteRasterGroupCount]
                        add esi, dword ptr [SpriteRasterSourceRowTail]
                        add edi, dword ptr [SpriteRasterDestinationRowTail]
                        dec dword ptr [SpriteRasterDestinationRowCount]
                        jne T1_62
                        pop ebp
                        pop edi
                        pop esi
        }
    } else {
        scaled_height = static_cast<int>(iSpriteHeight_ * SpriteScalingRatio);
        scaled_cx = static_cast<int>(iSpriteWidth_ * SpriteScalingRatio);
        s_top = static_cast<int>(y + iTopOffset_ * SpriteScalingRatio);
        sc_left = static_cast<int>(x + iLeftOffset_ * SpriteScalingRatio);
        rc_clip.left = sc_left;
        rc_clip.top = s_top;
        rc_clip.right = sc_left + scaled_cx;
        rc_clip.bottom = s_top + scaled_height;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        rc_clip = original;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationPitch = destination->stride_;
        if (SpriteScaleNumerator < SpriteScaleDenominator) {
            if (original.right - original.left < SpriteMinimumScaledWidth
                || original.bottom - original.top < SpriteMinimumScaledHeight) {
                destination->free_data(1);
                return 0;
            }
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_shrink(&original, &rc_clip,
                &dest_col_ofs, &source_col, &SpriteRasterDestinationColumnCount,
                &SpriteRasterSourceColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_shrink(&original, &rc_clip,
                &dest_y, &source_y, &SpriteRasterDestinationRowCount,
                &SpriteRasterSourceRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            // Hand-written assembly: one-operand MUL row offset and computed
            // `jmp ebx` entry into an X-table stepped keyed fill.
            __asm {
                            push esi
                            push edi
                            mov eax, dword ptr [SpriteRasterSourcePitch]
                            mul dword ptr [source_y]
                            mov esi, eax
                            add esi, dword ptr [source_col]
                            add esi, dword ptr [source]
                            mov edi, dword ptr [dest_ptr]
                            mov edx, dword ptr [SpriteRasterXTable]
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            mov ebx, dword ptr [table]
                            mov ah, byte ptr [transparent]
                            push ebp
                            mov ebp, ebx
                            mov ebx, dword ptr [SpriteRasterDestinationColumnCount]
                            and ebx, 0x3
                            je T2_3e
                            dec ebx
                            je T2_51
                            dec ebx
                            je T2_6b
                            dec ebx
                            je T2_82
                        T2_3e:
                            lea ebx, T2_ec
                            mov dword ptr [SpriteRasterShrinkEntry], ebx
                            xor ebx, ebx
                            jmp T2_ec
                        T2_51:
                            lea ebx, T2_10a
                            mov dword ptr [SpriteRasterShrinkEntry], ebx
                            xor ebx, ebx
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp T2_10a
                        T2_6b:
                            lea ebx, T2_100
                            mov dword ptr [SpriteRasterShrinkEntry], ebx
                            xor ebx, ebx
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp T2_100
                        T2_82:
                            lea ebx, T2_f6
                            mov dword ptr [SpriteRasterShrinkEntry], ebx
                            xor ebx, ebx
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp T2_f6
                        T2_99:
                            jmp dword ptr [SpriteRasterShrinkEntry]
                        T2_9f:
                            cmp byte ptr [esi], ah
                            je T2_f0
                        T2_a3:
                            mov bl, byte ptr [esi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            add esi, dword ptr [edx]
                            inc edi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T2_fa
                        T2_b5:
                            mov bl, byte ptr [esi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            add esi, dword ptr [edx]
                            inc edi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T2_104
                        T2_c7:
                            mov bl, byte ptr [esi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            add esi, dword ptr [edx]
                            inc edi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T2_10e
                        T2_d9:
                            mov bl, byte ptr [esi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            add esi, dword ptr [edx]
                            inc edi
                            add edx, 0x4
                            dec ecx
                            jne T2_9f
                            jmp T2_117
                        T2_ec:
                            cmp byte ptr [esi], ah
                            jne T2_a3
                        T2_f0:
                            inc edi
                            add esi, dword ptr [edx]
                            add edx, 0x4
                        T2_f6:
                            cmp byte ptr [esi], ah
                            jne T2_b5
                        T2_fa:
                            inc edi
                            add esi, dword ptr [edx]
                            add edx, 0x4
                        T2_100:
                            cmp byte ptr [esi], ah
                            jne T2_c7
                        T2_104:
                            inc edi
                            add esi, dword ptr [edx]
                            add edx, 0x4
                        T2_10a:
                            cmp byte ptr [esi], ah
                            jne T2_d9
                        T2_10e:
                            inc edi
                            add esi, dword ptr [edx]
                            add edx, 0x4
                            dec ecx
                            jne T2_ec
                        T2_117:
                            mov ecx, dword ptr [SpriteRasterYTable]
                            mov al, byte ptr [ecx]
                            add ecx, 0x4
                            mov dword ptr [SpriteRasterYTable], ecx
                            xor ecx, ecx
                            mov cl, al
                            dec ecx
                            je T2_138
                        T2_12f:
                            add esi, dword ptr [SpriteRasterSourcePitch]
                            dec ecx
                            jne T2_12f
                        T2_138:
                            add esi, dword ptr [SpriteRasterSourceRowTail]
                            add edi, dword ptr [SpriteRasterDestinationRowTail]
                            mov edx, dword ptr [SpriteRasterXTable]
                            mov ecx, dword ptr [SpriteRasterDestinationRowCount]
                            dec ecx
                            mov dword ptr [SpriteRasterDestinationRowCount], ecx
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            jne T2_99
                            pop ebp
                            pop edi
                            pop esi
            }
        } else {
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_expand(&original, &rc_clip,
                &source_col, &dest_col_ofs, &SpriteRasterSourceColumnCount,
                &SpriteRasterDestinationColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_expand(&original, &rc_clip,
                &source_y, &dest_y, &SpriteRasterSourceRowCount,
                &SpriteRasterDestinationRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterGroupCount = SpriteRasterSourceColumnCount >> 2;
            SpriteRasterGroupRemainder = SpriteRasterSourceColumnCount - (SpriteRasterGroupCount << 2);
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            SpriteRasterRemapTable.single = table;
            // Hand-written assembly: 16-bit PUSHF/POPF around CLD, EBP as the
            // row counter, and replicated expansion rows.
            __asm {
                            push edi
                            push esi
                            pushf
                            cld
                            mov eax, dword ptr [SpriteRasterSourcePitch]
                            mul dword ptr [source_y]
                            mov esi, eax
                            add esi, dword ptr [source_col]
                            add esi, dword ptr [source]
                            mov edi, dword ptr [dest_ptr]
                            mov edx, dword ptr [SpriteRasterXTable]
                            mov ah, byte ptr [transparent]
                            mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                            and ebx, 0x3
                            push ebp
                            mov ebp, dword ptr [SpriteRasterDestinationPitch]
                            je T3_40
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                        T3_40:
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            mov dword ptr [SpriteRasterColumnCountdown], ecx
                            mov ecx, dword ptr [SpriteRasterYTable]
                            mov al, byte ptr [ecx]
                            add ecx, 0x4
                            mov dword ptr [SpriteRasterYTable], ecx
                            mov edx, dword ptr [SpriteRasterXTable]
                            cmp al, 0x1
                            je T3_52a
                            cmp al, 0x2
                            je T3_3d7
                            cmp al, 0x3
                            je T3_24c
                            mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                            and ebx, 0x3
                            je T3_9e
                            dec ebx
                            je T3_8f
                            dec ebx
                            je T3_94
                            dec ebx
                            je T3_99
                        T3_8f:
                            jmp T3_22b
                        T3_94:
                            jmp T3_21d
                        T3_99:
                            jmp T3_20f
                        T3_9e:
                            cmp byte ptr [esi], ah
                            je T3_209
                        T3_a6:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_eb
                            xor ebx, ebx
                            mov bl, byte ptr [edi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_b9:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_b9
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_c5:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_c5
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_d1:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_d1
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_dd:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_dd
                            sub edi, ebp
                            sub edi, ebp
                            sub edi, ebp
                            jmp T3_ed
                        T3_eb:
                            add edi, ecx
                        T3_ed:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_217
                        T3_f9:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_13e
                            xor ebx, ebx
                            mov bl, byte ptr [edi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_10c:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_10c
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_118:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_118
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_124:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_124
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_130:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_130
                            sub edi, ebp
                            sub edi, ebp
                            sub edi, ebp
                            jmp T3_140
                        T3_13e:
                            add edi, ecx
                        T3_140:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_225
                        T3_14c:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_191
                            xor ebx, ebx
                            mov bl, byte ptr [edi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_15f:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_15f
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_16b:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_16b
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_177:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_177
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_183:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_183
                            sub edi, ebp
                            sub edi, ebp
                            sub edi, ebp
                            jmp T3_193
                        T3_191:
                            add edi, ecx
                        T3_193:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_233
                        T3_19f:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_1e4
                            xor ebx, ebx
                            mov bl, byte ptr [edi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_1b2:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_1b2
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_1be:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_1be
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_1ca:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_1ca
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_1d6:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_1d6
                            sub edi, ebp
                            sub edi, ebp
                            sub edi, ebp
                            jmp T3_1e6
                        T3_1e4:
                            add edi, ecx
                        T3_1e6:
                            inc esi
                            add edx, 0x4
                            dec dword ptr [SpriteRasterColumnCountdown]
                            jne T3_9e
                            lea edi, [edi+2*ebp]
                            lea edi, [edi+ebp]
                            jmp T3_62f
                        T3_201:
                            cmp byte ptr [esi], ah
                            jne T3_a6
                        T3_209:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_20f:
                            cmp byte ptr [esi], ah
                            jne T3_f9
                        T3_217:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_21d:
                            cmp byte ptr [esi], ah
                            jne T3_14c
                        T3_225:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_22b:
                            cmp byte ptr [esi], ah
                            jne T3_19f
                        T3_233:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                            dec dword ptr [SpriteRasterColumnCountdown]
                            jne T3_201
                            lea edi, [edi+2*ebp]
                            lea edi, [edi+ebp]
                            jmp T3_62f
                        T3_24c:
                            mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                            and ebx, 0x3
                            je T3_26f
                            dec ebx
                            je T3_260
                            dec ebx
                            je T3_265
                            dec ebx
                            je T3_26a
                        T3_260:
                            jmp T3_3bd
                        T3_265:
                            jmp T3_3af
                        T3_26a:
                            jmp T3_3a1
                        T3_26f:
                            cmp byte ptr [esi], ah
                            je T3_39b
                        T3_277:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_2ae
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_28a:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_28a
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_296:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_296
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_2a2:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_2a2
                            sub edi, ebp
                            sub edi, ebp
                            jmp T3_2b0
                        T3_2ae:
                            add edi, ecx
                        T3_2b0:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_3a9
                        T3_2bc:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_2f3
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_2cf:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_2cf
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_2db:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_2db
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_2e7:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_2e7
                            sub edi, ebp
                            sub edi, ebp
                            jmp T3_2f5
                        T3_2f3:
                            add edi, ecx
                        T3_2f5:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_3b7
                        T3_301:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_338
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_314:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_314
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_320:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_320
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_32c:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_32c
                            sub edi, ebp
                            sub edi, ebp
                            jmp T3_33a
                        T3_338:
                            add edi, ecx
                        T3_33a:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_3c1
                        T3_342:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_379
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_355:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_355
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_361:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_361
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_36d:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_36d
                            sub edi, ebp
                            sub edi, ebp
                            jmp T3_37b
                        T3_379:
                            add edi, ecx
                        T3_37b:
                            inc esi
                            add edx, 0x4
                            dec dword ptr [SpriteRasterColumnCountdown]
                            jne T3_26f
                            lea edi, [edi+2*ebp]
                            jmp T3_62f
                        T3_393:
                            cmp byte ptr [esi], ah
                            jne T3_277
                        T3_39b:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_3a1:
                            cmp byte ptr [esi], ah
                            jne T3_2bc
                        T3_3a9:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_3af:
                            cmp byte ptr [esi], ah
                            jne T3_301
                        T3_3b7:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_3bd:
                            cmp byte ptr [esi], ah
                            jne T3_342
                        T3_3c1:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                            dec dword ptr [SpriteRasterColumnCountdown]
                            jne T3_393
                            lea edi, [edi+2*ebp]
                            jmp T3_62f
                        T3_3d7:
                            mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                            and ebx, 0x3
                            je T3_3fa
                            dec ebx
                            je T3_3eb
                            dec ebx
                            je T3_3f0
                            dec ebx
                            je T3_3f5
                        T3_3eb:
                            jmp T3_510
                        T3_3f0:
                            jmp T3_502
                        T3_3f5:
                            jmp T3_4f4
                        T3_3fa:
                            cmp byte ptr [esi], ah
                            je T3_4ee
                        T3_402:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_42b
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_415:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_415
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_421:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_421
                            sub edi, ebp
                            jmp T3_42d
                        T3_42b:
                            add edi, ecx
                        T3_42d:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_4fc
                        T3_439:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_462
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_44c:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_44c
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_458:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_458
                            sub edi, ebp
                            jmp T3_464
                        T3_462:
                            add edi, ecx
                        T3_464:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_50a
                        T3_470:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_499
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_483:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_483
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_48f:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_48f
                            sub edi, ebp
                            jmp T3_49b
                        T3_499:
                            add edi, ecx
                        T3_49b:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_514
                        T3_4a3:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_4cc
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_4b6:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_4b6
                            add edi, ebp
                            sub edi, dword ptr [edx]
                            mov ecx, dword ptr [edx]
                        T3_4c2:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_4c2
                            sub edi, ebp
                            jmp T3_4ce
                        T3_4cc:
                            add edi, ecx
                        T3_4ce:
                            inc esi
                            add edx, 0x4
                            dec dword ptr [SpriteRasterColumnCountdown]
                            jne T3_3fa
                            lea edi, [edi+ebp]
                            jmp T3_62f
                        T3_4e6:
                            cmp byte ptr [esi], ah
                            jne T3_402
                        T3_4ee:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_4f4:
                            cmp byte ptr [esi], ah
                            jne T3_439
                        T3_4fc:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_502:
                            cmp byte ptr [esi], ah
                            jne T3_470
                        T3_50a:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_510:
                            cmp byte ptr [esi], ah
                            jne T3_4a3
                        T3_514:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                            dec dword ptr [SpriteRasterColumnCountdown]
                            jne T3_4e6
                            lea edi, [edi+ebp]
                            jmp T3_62f
                        T3_52a:
                            mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                            and ebx, 0x3
                            je T3_54d
                            dec ebx
                            je T3_53e
                            dec ebx
                            je T3_543
                            dec ebx
                            je T3_548
                        T3_53e:
                            jmp T3_61d
                        T3_543:
                            jmp T3_613
                        T3_548:
                            jmp T3_605
                        T3_54d:
                            cmp byte ptr [esi], ah
                            je T3_5ff
                        T3_555:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_570
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_568:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_568
                            jmp T3_572
                        T3_570:
                            add edi, ecx
                        T3_572:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_60d
                        T3_57e:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_599
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_591:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_591
                            jmp T3_59b
                        T3_599:
                            add edi, ecx
                        T3_59b:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_617
                        T3_5a3:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_5be
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_5b6:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_5b6
                            jmp T3_5c0
                        T3_5be:
                            add edi, ecx
                        T3_5c0:
                            inc esi
                            add edx, 0x4
                            cmp byte ptr [esi], ah
                            je T3_621
                        T3_5c8:
                            cmp byte ptr [esi], 0x0
                            mov ecx, dword ptr [edx]
                            je T3_5e3
                            xor ebx, ebx
                            mov bl, byte ptr [esi]
                            add ebx, dword ptr [SpriteRasterRemapTable]
                            mov al, byte ptr [ebx]
                        T3_5db:
                            mov byte ptr [edi], al
                            inc edi
                            dec ecx
                            jne T3_5db
                            jmp T3_5e5
                        T3_5e3:
                            add edi, ecx
                        T3_5e5:
                            inc esi
                            add edx, 0x4
                            dec dword ptr [SpriteRasterColumnCountdown]
                            jne T3_54d
                            jmp T3_62f
                        T3_5f7:
                            cmp byte ptr [esi], ah
                            jne T3_555
                        T3_5ff:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_605:
                            cmp byte ptr [esi], ah
                            jne T3_57e
                        T3_60d:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_613:
                            cmp byte ptr [esi], ah
                            jne T3_5a3
                        T3_617:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                        T3_61d:
                            cmp byte ptr [esi], ah
                            jne T3_5c8
                        T3_621:
                            inc esi
                            add edi, dword ptr [edx]
                            add edx, 0x4
                            dec dword ptr [SpriteRasterColumnCountdown]
                            jne T3_5f7
                        T3_62f:
                            add esi, dword ptr [SpriteRasterSourceRowTail]
                            add edi, dword ptr [SpriteRasterDestinationRowTail]
                            dec dword ptr [SpriteRasterSourceRowCount]
                            jne T3_40
                            pop ebp
                            popf
                            pop esi
                            pop edi
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
    int source_y;
    int source_col;
    RECT rc_clip;
    int dest_col_ofs;
    unsigned char *dest_ptr;
    RECT original;
    int start_x;
    unsigned char *source;
    int dest_y;
    int start_y;
    int w;
    int scaled_height;
    int scaled_cx;
    int s_top;
    int sc_left;
    int h;
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0) {
        return 3;
    }
    if (SpriteScaleNumerator != SpriteScaleDenominator
        && (SpriteCachedScaleNumerator != SpriteScaleNumerator
            || SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        SpriteScalingRatio = static_cast<float>(SpriteScaleNumerator) / SpriteScaleDenominator;
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }
    transparent = static_cast<unsigned char>(transparent);
    SpriteRasterSourcePitch = iSpriteWidth2_;
    SpriteRasterDestinationPitch = destination->stride_;
    source = pcBits_;
    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        h = iSpriteHeight_;
        w = iSpriteWidth_;
        start_y = y + iTopOffset_;
        start_x = x + iLeftOffset_;
        rc_clip.left = start_x;
        rc_clip.top = start_y;
        rc_clip.right = start_x + w;
        rc_clip.bottom = start_y + h;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        source = pcBits_ + (rc_clip.left - original.left)
            + (rc_clip.top - original.top) * SpriteRasterSourcePitch;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationRowCount = rc_clip.bottom - rc_clip.top;
        SpriteRasterDestinationColumnCount = rc_clip.right - rc_clip.left;
        SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
        SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterDestinationColumnCount;
        SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
        // Hand-written assembly: computed `lea edx, label` / `jmp edx` entry
        // into an unrolled four-pixel keyed fill.
        __asm {
                            push esi
                            push edi
                            mov esi, dword ptr [source]
                            mov edi, dword ptr [dest_ptr]
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            mov al, byte ptr [transparent]
                            mov edx, dword ptr [table]
                            xor ebx, ebx
                            push ebp
                            mov ebp, edx
                            mov edx, dword ptr [SpriteRasterDestinationColumnCount]
                            and edx, 0x3
                            je E1_2d
                            dec edx
                            je E1_38
                            dec edx
                            je E1_4a
                            dec edx
                            je E1_5c
                        E1_2d:
                            lea edx, E1_d6
                            jmp E1_d6
                        E1_38:
                            lea edx, E1_e8
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp E1_e8
                        E1_4a:
                            lea edx, E1_e2
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp E1_e2
                        E1_5c:
                            lea edx, E1_dc
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp E1_dc
                        E1_6b:
                            jmp edx
                        E1_6d:
                            cmp byte ptr [esi], al
                            je E1_da
                        E1_71:
                            cmp byte ptr [esi], 0x0
                            jne E1_80
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            jmp E1_84
                        E1_80:
                            mov ah, byte ptr [esi]
                            mov byte ptr [edi], ah
                        E1_84:
                            inc esi
                            inc edi
                            cmp byte ptr [esi], al
                            je E1_e0
                        E1_8a:
                            cmp byte ptr [esi], 0x0
                            jne E1_99
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            jmp E1_9d
                        E1_99:
                            mov ah, byte ptr [esi]
                            mov byte ptr [edi], ah
                        E1_9d:
                            inc esi
                            inc edi
                            cmp byte ptr [esi], al
                            je E1_e6
                        E1_a3:
                            cmp byte ptr [esi], 0x0
                            jne E1_b2
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            jmp E1_b6
                        E1_b2:
                            mov ah, byte ptr [esi]
                            mov byte ptr [edi], ah
                        E1_b6:
                            inc esi
                            inc edi
                            cmp byte ptr [esi], al
                            je E1_ec
                        E1_bc:
                            cmp byte ptr [esi], 0x0
                            jne E1_cb
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            jmp E1_cf
                        E1_cb:
                            mov ah, byte ptr [esi]
                            mov byte ptr [edi], ah
                        E1_cf:
                            inc esi
                            inc edi
                            dec ecx
                            jne E1_6d
                            jmp E1_f1
                        E1_d6:
                            cmp byte ptr [esi], al
                            jne E1_71
                        E1_da:
                            inc esi
                            inc edi
                        E1_dc:
                            cmp byte ptr [esi], al
                            jne E1_8a
                        E1_e0:
                            inc esi
                            inc edi
                        E1_e2:
                            cmp byte ptr [esi], al
                            jne E1_a3
                        E1_e6:
                            inc esi
                            inc edi
                        E1_e8:
                            cmp byte ptr [esi], al
                            jne E1_bc
                        E1_ec:
                            inc esi
                            inc edi
                            dec ecx
                            jne E1_d6
                        E1_f1:
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            add esi, dword ptr [SpriteRasterSourceRowTail]
                            add edi, dword ptr [SpriteRasterDestinationRowTail]
                            dec dword ptr [SpriteRasterDestinationRowCount]
                            jne E1_6b
                            pop ebp
                            pop edi
                            pop esi
        }
    } else {
        scaled_height = static_cast<int>(iSpriteHeight_ * SpriteScalingRatio);
        scaled_cx = static_cast<int>(iSpriteWidth_ * SpriteScalingRatio);
        s_top = static_cast<int>(y + iTopOffset_ * SpriteScalingRatio);
        sc_left = static_cast<int>(x + iLeftOffset_ * SpriteScalingRatio);
        rc_clip.left = sc_left;
        rc_clip.top = s_top;
        rc_clip.right = sc_left + scaled_cx;
        rc_clip.bottom = s_top + scaled_height;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        rc_clip = original;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationPitch = destination->stride_;
        if (SpriteScaleNumerator < SpriteScaleDenominator) {
            if (original.right - original.left < SpriteMinimumScaledWidth
                || original.bottom - original.top < SpriteMinimumScaledHeight) {
                destination->free_data(1);
                return 0;
            }
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_shrink(&original, &rc_clip,
                &dest_col_ofs, &source_col, &SpriteRasterDestinationColumnCount,
                &SpriteRasterSourceColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_shrink(&original, &rc_clip,
                &dest_y, &source_y, &SpriteRasterDestinationRowCount,
                &SpriteRasterSourceRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            // Hand-written assembly: one-operand MUL row offset and computed
            // `jmp ebx` entry into an X-table stepped keyed fill.
            __asm {
                                push esi
                                push edi
                                mov eax, dword ptr [SpriteRasterSourcePitch]
                                mul dword ptr [source_y]
                                mov esi, eax
                                add esi, dword ptr [source_col]
                                add esi, dword ptr [source]
                                mov edi, dword ptr [dest_ptr]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                mov ah, byte ptr [transparent]
                                mov ebx, dword ptr [table]
                                push ebp
                                mov ebp, ebx
                                mov ebx, dword ptr [SpriteRasterDestinationColumnCount]
                                and ebx, 0x3
                                je E2_3e
                                dec ebx
                                je E2_51
                                dec ebx
                                je E2_6b
                                dec ebx
                                je E2_85
                            E2_3e:
                                lea ebx, E2_11e
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                jmp E2_11e
                            E2_51:
                                lea ebx, E2_13c
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp E2_13c
                            E2_6b:
                                lea ebx, E2_132
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp E2_132
                            E2_85:
                                lea ebx, E2_128
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp E2_128
                            E2_9f:
                                jmp dword ptr [SpriteRasterShrinkEntry]
                            E2_a5:
                                cmp byte ptr [esi], ah
                                je E2_122
                            E2_a9:
                                cmp byte ptr [esi], 0x0
                                jne E2_b8
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                jmp E2_bc
                            E2_b8:
                                mov al, byte ptr [esi]
                                mov byte ptr [edi], al
                            E2_bc:
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E2_12c
                            E2_c6:
                                cmp byte ptr [esi], 0x0
                                jne E2_d5
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                jmp E2_d9
                            E2_d5:
                                mov al, byte ptr [esi]
                                mov byte ptr [edi], al
                            E2_d9:
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E2_136
                            E2_e3:
                                cmp byte ptr [esi], 0x0
                                jne E2_f2
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                jmp E2_f6
                            E2_f2:
                                mov al, byte ptr [esi]
                                mov byte ptr [edi], al
                            E2_f6:
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E2_140
                            E2_100:
                                cmp byte ptr [esi], 0x0
                                jne E2_10f
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                jmp E2_113
                            E2_10f:
                                mov al, byte ptr [esi]
                                mov byte ptr [edi], al
                            E2_113:
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                dec ecx
                                jne E2_a5
                                jmp E2_149
                            E2_11e:
                                cmp byte ptr [esi], ah
                                jne E2_a9
                            E2_122:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            E2_128:
                                cmp byte ptr [esi], ah
                                jne E2_c6
                            E2_12c:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            E2_132:
                                cmp byte ptr [esi], ah
                                jne E2_e3
                            E2_136:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            E2_13c:
                                cmp byte ptr [esi], ah
                                jne E2_100
                            E2_140:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                                dec ecx
                                jne E2_11e
                            E2_149:
                                mov ecx, dword ptr [SpriteRasterYTable]
                                mov al, byte ptr [ecx]
                                add ecx, 0x4
                                mov dword ptr [SpriteRasterYTable], ecx
                                xor ecx, ecx
                                mov cl, al
                                dec ecx
                                je E2_16a
                            E2_161:
                                add esi, dword ptr [SpriteRasterSourcePitch]
                                dec ecx
                                jne E2_161
                            E2_16a:
                                add esi, dword ptr [SpriteRasterSourceRowTail]
                                add edi, dword ptr [SpriteRasterDestinationRowTail]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                dec dword ptr [SpriteRasterDestinationRowCount]
                                jne E2_9f
                                pop ebp
                                pop edi
                                pop esi
            }
        } else {
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_expand(&original, &rc_clip,
                &source_col, &dest_col_ofs, &SpriteRasterSourceColumnCount,
                &SpriteRasterDestinationColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_expand(&original, &rc_clip,
                &source_y, &dest_y, &SpriteRasterSourceRowCount,
                &SpriteRasterDestinationRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterGroupCount = SpriteRasterSourceColumnCount >> 2;
            SpriteRasterGroupRemainder = SpriteRasterSourceColumnCount - (SpriteRasterGroupCount << 2);
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            SpriteRasterRemapTable.single = table;
            // Hand-written assembly: 16-bit PUSHF/POPF around CLD, EBP as the
            // row counter, and replicated expansion rows.
            __asm {
                                push edi
                                push esi
                                pushf
                                cld
                                mov eax, dword ptr [SpriteRasterSourcePitch]
                                mul dword ptr [source_y]
                                mov esi, eax
                                add esi, dword ptr [source_col]
                                add esi, dword ptr [source]
                                mov edi, dword ptr [dest_ptr]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ah, byte ptr [transparent]
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                push ebp
                                mov ebp, dword ptr [SpriteRasterDestinationPitch]
                                je E3_40
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                            E3_40:
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                mov dword ptr [SpriteRasterColumnCountdown], ecx
                                mov ecx, dword ptr [SpriteRasterYTable]
                                mov al, byte ptr [ecx]
                                add ecx, 0x4
                                mov dword ptr [SpriteRasterYTable], ecx
                                mov edx, dword ptr [SpriteRasterXTable]
                                cmp al, 0x1
                                je E3_52a
                                cmp al, 0x2
                                je E3_3d7
                                cmp al, 0x3
                                je E3_24c
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je E3_9e
                                dec ebx
                                je E3_8f
                                dec ebx
                                je E3_94
                                dec ebx
                                je E3_99
                            E3_8f:
                                jmp E3_22b
                            E3_94:
                                jmp E3_21d
                            E3_99:
                                jmp E3_20f
                            E3_9e:
                                cmp byte ptr [esi], ah
                                je E3_209
                            E3_a6:
                                cmp byte ptr [esi], 0x0
                                jne E3_b9
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_bb
                            E3_b9:
                                mov al, byte ptr [esi]
                            E3_bb:
                                mov ecx, dword ptr [edx]
                            E3_bd:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_bd
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_c9:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_c9
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_d5:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_d5
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_e1:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_e1
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_217
                            E3_f9:
                                cmp byte ptr [esi], 0x0
                                jne E3_10c
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_10e
                            E3_10c:
                                mov al, byte ptr [esi]
                            E3_10e:
                                mov ecx, dword ptr [edx]
                            E3_110:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_110
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_11c:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_11c
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_128:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_128
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_134:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_134
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_225
                            E3_14c:
                                cmp byte ptr [esi], 0x0
                                jne E3_15f
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_161
                            E3_15f:
                                mov al, byte ptr [esi]
                            E3_161:
                                mov ecx, dword ptr [edx]
                            E3_163:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_163
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_16f:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_16f
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_17b:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_17b
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_187:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_187
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_233
                            E3_19f:
                                cmp byte ptr [esi], 0x0
                                jne E3_1b2
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_1b4
                            E3_1b2:
                                mov al, byte ptr [esi]
                            E3_1b4:
                                mov ecx, dword ptr [edx]
                            E3_1b6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_1b6
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_1c2:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_1c2
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_1ce:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_1ce
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_1da:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_1da
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne E3_9e
                                lea edi, [edi+2*ebp]
                                lea edi, [edi+ebp]
                                jmp E3_62f
                            E3_201:
                                cmp byte ptr [esi], ah
                                jne E3_a6
                            E3_209:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_20f:
                                cmp byte ptr [esi], ah
                                jne E3_f9
                            E3_217:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_21d:
                                cmp byte ptr [esi], ah
                                jne E3_14c
                            E3_225:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_22b:
                                cmp byte ptr [esi], ah
                                jne E3_19f
                            E3_233:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne E3_201
                                lea edi, [edi+2*ebp]
                                lea edi, [edi+ebp]
                                jmp E3_62f
                            E3_24c:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je E3_26f
                                dec ebx
                                je E3_260
                                dec ebx
                                je E3_265
                                dec ebx
                                je E3_26a
                            E3_260:
                                jmp E3_3bd
                            E3_265:
                                jmp E3_3af
                            E3_26a:
                                jmp E3_3a1
                            E3_26f:
                                cmp byte ptr [esi], ah
                                je E3_39b
                            E3_277:
                                cmp byte ptr [esi], 0x0
                                jne E3_28a
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_28c
                            E3_28a:
                                mov al, byte ptr [esi]
                            E3_28c:
                                mov ecx, dword ptr [edx]
                            E3_28e:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_28e
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_29a:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_29a
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_2a6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_2a6
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_3a9
                            E3_2bc:
                                cmp byte ptr [esi], 0x0
                                jne E3_2cf
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_2d1
                            E3_2cf:
                                mov al, byte ptr [esi]
                            E3_2d1:
                                mov ecx, dword ptr [edx]
                            E3_2d3:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_2d3
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_2df:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_2df
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_2eb:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_2eb
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_3b7
                            E3_301:
                                cmp byte ptr [esi], 0x0
                                jne E3_314
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_316
                            E3_314:
                                mov al, byte ptr [esi]
                            E3_316:
                                mov ecx, dword ptr [edx]
                            E3_318:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_318
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_324:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_324
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_330:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_330
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_3c1
                            E3_342:
                                cmp byte ptr [esi], 0x0
                                jne E3_355
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_357
                            E3_355:
                                mov al, byte ptr [esi]
                            E3_357:
                                mov ecx, dword ptr [edx]
                            E3_359:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_359
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_365:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_365
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_371:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_371
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne E3_26f
                                lea edi, [edi+2*ebp]
                                jmp E3_62f
                            E3_393:
                                cmp byte ptr [esi], ah
                                jne E3_277
                            E3_39b:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_3a1:
                                cmp byte ptr [esi], ah
                                jne E3_2bc
                            E3_3a9:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_3af:
                                cmp byte ptr [esi], ah
                                jne E3_301
                            E3_3b7:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_3bd:
                                cmp byte ptr [esi], ah
                                jne E3_342
                            E3_3c1:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne E3_393
                                lea edi, [edi+2*ebp]
                                jmp E3_62f
                            E3_3d7:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je E3_3fa
                                dec ebx
                                je E3_3eb
                                dec ebx
                                je E3_3f0
                                dec ebx
                                je E3_3f5
                            E3_3eb:
                                jmp E3_510
                            E3_3f0:
                                jmp E3_502
                            E3_3f5:
                                jmp E3_4f4
                            E3_3fa:
                                cmp byte ptr [esi], ah
                                je E3_4ee
                            E3_402:
                                cmp byte ptr [esi], 0x0
                                jne E3_415
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_417
                            E3_415:
                                mov al, byte ptr [esi]
                            E3_417:
                                mov ecx, dword ptr [edx]
                            E3_419:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_419
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_425:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_425
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_4fc
                            E3_439:
                                cmp byte ptr [esi], 0x0
                                jne E3_44c
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_44e
                            E3_44c:
                                mov al, byte ptr [esi]
                            E3_44e:
                                mov ecx, dword ptr [edx]
                            E3_450:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_450
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_45c:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_45c
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_50a
                            E3_470:
                                cmp byte ptr [esi], 0x0
                                jne E3_483
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_485
                            E3_483:
                                mov al, byte ptr [esi]
                            E3_485:
                                mov ecx, dword ptr [edx]
                            E3_487:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_487
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_493:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_493
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_514
                            E3_4a3:
                                cmp byte ptr [esi], 0x0
                                jne E3_4b6
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_4b8
                            E3_4b6:
                                mov al, byte ptr [esi]
                            E3_4b8:
                                mov ecx, dword ptr [edx]
                            E3_4ba:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_4ba
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            E3_4c6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_4c6
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne E3_3fa
                                lea edi, [edi+ebp]
                                jmp E3_62f
                            E3_4e6:
                                cmp byte ptr [esi], ah
                                jne E3_402
                            E3_4ee:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_4f4:
                                cmp byte ptr [esi], ah
                                jne E3_439
                            E3_4fc:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_502:
                                cmp byte ptr [esi], ah
                                jne E3_470
                            E3_50a:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_510:
                                cmp byte ptr [esi], ah
                                jne E3_4a3
                            E3_514:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne E3_4e6
                                lea edi, [edi+ebp]
                                jmp E3_62f
                            E3_52a:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je E3_54d
                                dec ebx
                                je E3_53e
                                dec ebx
                                je E3_543
                                dec ebx
                                je E3_548
                            E3_53e:
                                jmp E3_61d
                            E3_543:
                                jmp E3_613
                            E3_548:
                                jmp E3_605
                            E3_54d:
                                cmp byte ptr [esi], ah
                                je E3_5ff
                            E3_555:
                                cmp byte ptr [esi], 0x0
                                jne E3_568
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_56a
                            E3_568:
                                mov al, byte ptr [esi]
                            E3_56a:
                                mov ecx, dword ptr [edx]
                            E3_56c:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_56c
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_60d
                            E3_57e:
                                cmp byte ptr [esi], 0x0
                                jne E3_591
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_593
                            E3_591:
                                mov al, byte ptr [esi]
                            E3_593:
                                mov ecx, dword ptr [edx]
                            E3_595:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_595
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_617
                            E3_5a3:
                                cmp byte ptr [esi], 0x0
                                jne E3_5b6
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_5b8
                            E3_5b6:
                                mov al, byte ptr [esi]
                            E3_5b8:
                                mov ecx, dword ptr [edx]
                            E3_5ba:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_5ba
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je E3_621
                            E3_5c8:
                                cmp byte ptr [esi], 0x0
                                jne E3_5db
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                jmp E3_5dd
                            E3_5db:
                                mov al, byte ptr [esi]
                            E3_5dd:
                                mov ecx, dword ptr [edx]
                            E3_5df:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne E3_5df
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne E3_54d
                                jmp E3_62f
                            E3_5f7:
                                cmp byte ptr [esi], ah
                                jne E3_555
                            E3_5ff:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_605:
                                cmp byte ptr [esi], ah
                                jne E3_57e
                            E3_60d:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_613:
                                cmp byte ptr [esi], ah
                                jne E3_5a3
                            E3_617:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            E3_61d:
                                cmp byte ptr [esi], ah
                                jne E3_5c8
                            E3_621:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne E3_5f7
                            E3_62f:
                                add esi, dword ptr [SpriteRasterSourceRowTail]
                                add edi, dword ptr [SpriteRasterDestinationRowTail]
                                dec dword ptr [SpriteRasterSourceRowCount]
                                jne E3_40
                                pop ebp
                                popf
                                pop esi
                                pop edi
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
    int source_y;
    int source_col;
    RECT rc_clip;
    int dest_col_ofs;
    unsigned char *dest_ptr;
    RECT original;
    int start_x;
    unsigned char *source;
    int dest_y;
    int start_y;
    int w;
    int scaled_height;
    int scaled_cx;
    int s_top;
    int sc_left;
    int h;
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0) {
        return 3;
    }
    if (SpriteScaleNumerator != SpriteScaleDenominator
        && (SpriteCachedScaleNumerator != SpriteScaleNumerator
            || SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        SpriteScalingRatio = static_cast<float>(SpriteScaleNumerator) / SpriteScaleDenominator;
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }
    SpriteRasterSourcePitch = iSpriteWidth2_;
    SpriteRasterDestinationPitch = destination->stride_;
    source = pcBits_;
    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        h = iSpriteHeight_;
        w = iSpriteWidth_;
        start_y = y + iTopOffset_;
        start_x = x + iLeftOffset_;
        rc_clip.left = start_x;
        rc_clip.top = start_y;
        rc_clip.right = start_x + w;
        rc_clip.bottom = start_y + h;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        source = pcBits_ + (rc_clip.left - original.left)
            + (rc_clip.top - original.top) * SpriteRasterSourcePitch;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationRowCount = rc_clip.bottom - rc_clip.top;
        SpriteRasterDestinationColumnCount = rc_clip.right - rc_clip.left;
        SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
        SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterDestinationColumnCount;
        SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
        // Hand-written assembly: computed `lea edx, label` / `jmp edx` entry
        // into an unrolled four-pixel keyed fill.
        __asm {
                            push esi
                            push edi
                            mov esi, dword ptr [source]
                            mov edi, dword ptr [dest_ptr]
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            mov edx, dword ptr [table]
                            xor ebx, ebx
                            push ebp
                            mov ebp, edx
                            mov edx, dword ptr [SpriteRasterDestinationColumnCount]
                            and edx, 0x3
                            je K1_2a
                            dec edx
                            je K1_32
                            dec edx
                            je K1_41
                            dec edx
                            je K1_50
                        K1_2a:
                            lea edx, K1_a2
                            jmp K1_a2
                        K1_32:
                            lea edx, K1_b7
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp K1_b7
                        K1_41:
                            lea edx, K1_b0
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp K1_b0
                        K1_50:
                            lea edx, K1_a9
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp K1_a9
                        K1_5f:
                            jmp edx
                        K1_61:
                            cmp byte ptr [esi], 0x0
                            jne K1_a7
                        K1_66:
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            cmp byte ptr [esi], 0x0
                            jne K1_ae
                        K1_75:
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            cmp byte ptr [esi], 0x0
                            jne K1_b5
                        K1_84:
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            cmp byte ptr [esi], 0x0
                            jne K1_bc
                        K1_93:
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            dec ecx
                            jne K1_61
                            jmp K1_c1
                        K1_a2:
                            cmp byte ptr [esi], 0x0
                            je K1_66
                        K1_a7:
                            inc esi
                            inc edi
                        K1_a9:
                            cmp byte ptr [esi], 0x0
                            je K1_75
                        K1_ae:
                            inc esi
                            inc edi
                        K1_b0:
                            cmp byte ptr [esi], 0x0
                            je K1_84
                        K1_b5:
                            inc esi
                            inc edi
                        K1_b7:
                            cmp byte ptr [esi], 0x0
                            je K1_93
                        K1_bc:
                            inc esi
                            inc edi
                            dec ecx
                            jne K1_a2
                        K1_c1:
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            add esi, dword ptr [SpriteRasterSourceRowTail]
                            add edi, dword ptr [SpriteRasterDestinationRowTail]
                            dec dword ptr [SpriteRasterDestinationRowCount]
                            jne K1_5f
                            pop ebp
                            pop edi
                            pop esi
        }
    } else {
        scaled_height = static_cast<int>(iSpriteHeight_ * SpriteScalingRatio);
        scaled_cx = static_cast<int>(iSpriteWidth_ * SpriteScalingRatio);
        s_top = static_cast<int>(y + iTopOffset_ * SpriteScalingRatio);
        sc_left = static_cast<int>(x + iLeftOffset_ * SpriteScalingRatio);
        rc_clip.left = sc_left;
        rc_clip.top = s_top;
        rc_clip.right = sc_left + scaled_cx;
        rc_clip.bottom = s_top + scaled_height;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        rc_clip = original;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationPitch = destination->stride_;
        if (SpriteScaleNumerator < SpriteScaleDenominator) {
            if (original.right - original.left < SpriteMinimumScaledWidth
                || original.bottom - original.top < SpriteMinimumScaledHeight) {
                destination->free_data(1);
                return 0;
            }
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_shrink(&original, &rc_clip,
                &dest_col_ofs, &source_col, &SpriteRasterDestinationColumnCount,
                &SpriteRasterSourceColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_shrink(&original, &rc_clip,
                &dest_y, &source_y, &SpriteRasterDestinationRowCount,
                &SpriteRasterSourceRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            // Hand-written assembly: one-operand MUL row offset and computed
            // `jmp ebx` entry into an X-table stepped keyed fill.
            __asm {
                                push esi
                                push edi
                                mov eax, dword ptr [SpriteRasterSourcePitch]
                                mul dword ptr [source_y]
                                mov esi, eax
                                add esi, dword ptr [source_col]
                                add esi, dword ptr [source]
                                mov edi, dword ptr [dest_ptr]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                mov ebx, dword ptr [table]
                                push ebp
                                mov ebp, ebx
                                mov ebx, dword ptr [SpriteRasterDestinationColumnCount]
                                and ebx, 0x3
                                je K2_3b
                                dec ebx
                                je K2_4e
                                dec ebx
                                je K2_68
                                dec ebx
                                je K2_82
                            K2_3b:
                                lea ebx, K2_f0
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                jmp K2_f0
                            K2_4e:
                                lea ebx, K2_111
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp K2_111
                            K2_68:
                                lea ebx, K2_106
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp K2_106
                            K2_82:
                                lea ebx, K2_fb
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp K2_fb
                            K2_99:
                                jmp dword ptr [SpriteRasterShrinkEntry]
                            K2_9f:
                                cmp byte ptr [esi], 0x0
                                jne K2_f5
                            K2_a4:
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K2_100
                            K2_b7:
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K2_10b
                            K2_ca:
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K2_116
                            K2_dd:
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                dec ecx
                                jne K2_9f
                                jmp K2_11f
                            K2_f0:
                                cmp byte ptr [esi], 0x0
                                je K2_a4
                            K2_f5:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            K2_fb:
                                cmp byte ptr [esi], 0x0
                                je K2_b7
                            K2_100:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            K2_106:
                                cmp byte ptr [esi], 0x0
                                je K2_ca
                            K2_10b:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            K2_111:
                                cmp byte ptr [esi], 0x0
                                je K2_dd
                            K2_116:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                                dec ecx
                                jne K2_f0
                            K2_11f:
                                mov ecx, dword ptr [SpriteRasterYTable]
                                mov al, byte ptr [ecx]
                                add ecx, 0x4
                                mov dword ptr [SpriteRasterYTable], ecx
                                xor ecx, ecx
                                mov cl, al
                                dec ecx
                                je K2_140
                            K2_137:
                                add esi, dword ptr [SpriteRasterSourcePitch]
                                dec ecx
                                jne K2_137
                            K2_140:
                                add esi, dword ptr [SpriteRasterSourceRowTail]
                                add edi, dword ptr [SpriteRasterDestinationRowTail]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                dec dword ptr [SpriteRasterDestinationRowCount]
                                jne K2_99
                                pop ebp
                                pop edi
                                pop esi
            }
        } else {
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_expand(&original, &rc_clip,
                &source_col, &dest_col_ofs, &SpriteRasterSourceColumnCount,
                &SpriteRasterDestinationColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_expand(&original, &rc_clip,
                &source_y, &dest_y, &SpriteRasterSourceRowCount,
                &SpriteRasterDestinationRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterGroupCount = SpriteRasterSourceColumnCount >> 2;
            SpriteRasterGroupRemainder = SpriteRasterSourceColumnCount - (SpriteRasterGroupCount << 2);
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            SpriteRasterRemapTable.single = table;
            // Hand-written assembly: 16-bit PUSHF/POPF around CLD, EBP as the
            // row counter, and replicated expansion rows.
            __asm {
                                push edi
                                push esi
                                pushf
                                cld
                                mov eax, dword ptr [SpriteRasterSourcePitch]
                                mul dword ptr [source_y]
                                mov esi, eax
                                add esi, dword ptr [source_col]
                                add esi, dword ptr [source]
                                mov edi, dword ptr [dest_ptr]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                push ebp
                                mov ebp, dword ptr [SpriteRasterDestinationPitch]
                                je K3_3d
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                            K3_3d:
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                mov dword ptr [SpriteRasterColumnCountdown], ecx
                                mov ecx, dword ptr [SpriteRasterYTable]
                                mov al, byte ptr [ecx]
                                add ecx, 0x4
                                mov dword ptr [SpriteRasterYTable], ecx
                                mov edx, dword ptr [SpriteRasterXTable]
                                cmp al, 0x1
                                je K3_4d3
                                cmp al, 0x2
                                je K3_39c
                                cmp al, 0x3
                                je K3_22d
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je K3_9b
                                dec ebx
                                je K3_8c
                                dec ebx
                                je K3_91
                                dec ebx
                                je K3_96
                            K3_8c:
                                jmp K3_20b
                            K3_91:
                                jmp K3_1fc
                            K3_96:
                                jmp K3_1ed
                            K3_9b:
                                cmp byte ptr [esi], 0x0
                                jne K3_1e7
                            K3_a4:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_b2:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_b2
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_be:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_be
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_ca:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_ca
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_d6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_d6
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_1f6
                            K3_ef:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_fd:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_fd
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_109:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_109
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_115:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_115
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_121:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_121
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_205
                            K3_13a:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_148:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_148
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_154:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_154
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_160:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_160
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_16c:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_16c
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_214
                            K3_185:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_193:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_193
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_19f:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_19f
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_1ab:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_1ab
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_1b7:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_1b7
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne K3_9b
                                lea edi, [edi+2*ebp]
                                lea edi, [edi+ebp]
                                jmp K3_5a8
                            K3_1de:
                                cmp byte ptr [esi], 0x0
                                je K3_a4
                            K3_1e7:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_1ed:
                                cmp byte ptr [esi], 0x0
                                je K3_ef
                            K3_1f6:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_1fc:
                                cmp byte ptr [esi], 0x0
                                je K3_13a
                            K3_205:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_20b:
                                cmp byte ptr [esi], 0x0
                                je K3_185
                            K3_214:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne K3_1de
                                lea edi, [edi+2*ebp]
                                lea edi, [edi+ebp]
                                jmp K3_5a8
                            K3_22d:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je K3_250
                                dec ebx
                                je K3_241
                                dec ebx
                                je K3_246
                                dec ebx
                                je K3_24b
                            K3_241:
                                jmp K3_381
                            K3_246:
                                jmp K3_372
                            K3_24b:
                                jmp K3_363
                            K3_250:
                                cmp byte ptr [esi], 0x0
                                jne K3_35d
                            K3_259:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_267:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_267
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_273:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_273
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_27f:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_27f
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_36c
                            K3_296:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_2a4:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_2a4
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_2b0:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_2b0
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_2bc:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_2bc
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_37b
                            K3_2d3:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_2e1:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_2e1
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_2ed:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_2ed
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_2f9:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_2f9
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_386
                            K3_30c:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_31a:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_31a
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_326:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_326
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_332:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_332
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne K3_250
                                lea edi, [edi+2*ebp]
                                jmp K3_5a8
                            K3_354:
                                cmp byte ptr [esi], 0x0
                                je K3_259
                            K3_35d:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_363:
                                cmp byte ptr [esi], 0x0
                                je K3_296
                            K3_36c:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_372:
                                cmp byte ptr [esi], 0x0
                                je K3_2d3
                            K3_37b:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_381:
                                cmp byte ptr [esi], 0x0
                                je K3_30c
                            K3_386:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne K3_354
                                lea edi, [edi+2*ebp]
                                jmp K3_5a8
                            K3_39c:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je K3_3bf
                                dec ebx
                                je K3_3b0
                                dec ebx
                                je K3_3b5
                                dec ebx
                                je K3_3ba
                            K3_3b0:
                                jmp K3_4b8
                            K3_3b5:
                                jmp K3_4a9
                            K3_3ba:
                                jmp K3_49a
                            K3_3bf:
                                cmp byte ptr [esi], 0x0
                                jne K3_494
                            K3_3c8:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_3d6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_3d6
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_3e2:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_3e2
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_4a3
                            K3_3f7:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_405:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_405
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_411:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_411
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_4b2
                            K3_426:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_434:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_434
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_440:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_440
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_4bd
                            K3_451:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_45f:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_45f
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            K3_46b:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_46b
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne K3_3bf
                                lea edi, [edi+ebp]
                                jmp K3_5a8
                            K3_48b:
                                cmp byte ptr [esi], 0x0
                                je K3_3c8
                            K3_494:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_49a:
                                cmp byte ptr [esi], 0x0
                                je K3_3f7
                            K3_4a3:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_4a9:
                                cmp byte ptr [esi], 0x0
                                je K3_426
                            K3_4b2:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_4b8:
                                cmp byte ptr [esi], 0x0
                                je K3_451
                            K3_4bd:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne K3_48b
                                lea edi, [edi+ebp]
                                jmp K3_5a8
                            K3_4d3:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je K3_4f6
                                dec ebx
                                je K3_4e7
                                dec ebx
                                je K3_4ec
                                dec ebx
                                je K3_4f1
                            K3_4e7:
                                jmp K3_595
                            K3_4ec:
                                jmp K3_58a
                            K3_4f1:
                                jmp K3_57f
                            K3_4f6:
                                cmp byte ptr [esi], 0x0
                                jne K3_579
                            K3_4fb:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_509:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_509
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_584
                            K3_518:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_526:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_526
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_58f
                            K3_535:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_543:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_543
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], 0x0
                                jne K3_59a
                            K3_552:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            K3_560:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne K3_560
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne K3_4f6
                                jmp K3_5a8
                            K3_574:
                                cmp byte ptr [esi], 0x0
                                je K3_4fb
                            K3_579:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_57f:
                                cmp byte ptr [esi], 0x0
                                je K3_518
                            K3_584:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_58a:
                                cmp byte ptr [esi], 0x0
                                je K3_535
                            K3_58f:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            K3_595:
                                cmp byte ptr [esi], 0x0
                                je K3_552
                            K3_59a:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne K3_574
                            K3_5a8:
                                add esi, dword ptr [SpriteRasterSourceRowTail]
                                add edi, dword ptr [SpriteRasterDestinationRowTail]
                                dec dword ptr [SpriteRasterSourceRowCount]
                                jne K3_3d
                                pop ebp
                                popf
                                pop esi
                                pop edi
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
    int source_y;
    int source_col;
    RECT rc_clip;
    int dest_col_ofs;
    unsigned char *dest_ptr;
    RECT original;
    int start_x;
    unsigned char *source;
    int dest_y;
    int start_y;
    int w;
    int scaled_height;
    int scaled_cx;
    int s_top;
    int sc_left;
    int h;
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0) {
        return 3;
    }
    if (SpriteScaleNumerator != SpriteScaleDenominator
        && (SpriteCachedScaleNumerator != SpriteScaleNumerator
            || SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        SpriteScalingRatio = static_cast<float>(SpriteScaleNumerator) / SpriteScaleDenominator;
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }
    SpriteRasterSourcePitch = iSpriteWidth2_;
    SpriteRasterDestinationPitch = destination->stride_;
    source = pcBits_;
    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        h = iSpriteHeight_;
        w = iSpriteWidth_;
        start_y = y + iTopOffset_;
        start_x = x + iLeftOffset_;
        rc_clip.left = start_x;
        rc_clip.top = start_y;
        rc_clip.right = start_x + w;
        rc_clip.bottom = start_y + h;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        source = pcBits_ + (rc_clip.left - original.left)
            + (rc_clip.top - original.top) * SpriteRasterSourcePitch;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationRowCount = rc_clip.bottom - rc_clip.top;
        SpriteRasterDestinationColumnCount = rc_clip.right - rc_clip.left;
        SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
        SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterDestinationColumnCount;
        SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
        // Hand-written assembly: computed `lea edx, label` / `jmp edx` entry
        // into an unrolled four-pixel keyed fill.
        __asm {
                            push esi
                            push edi
                            mov esi, dword ptr [source]
                            mov edi, dword ptr [dest_ptr]
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            mov edx, dword ptr [table]
                            xor ebx, ebx
                            mov ah, byte ptr [transparent]
                            push ebp
                            mov ebp, edx
                            mov edx, dword ptr [SpriteRasterDestinationColumnCount]
                            and edx, 0x3
                            je H1_2d
                            dec edx
                            je H1_35
                            dec edx
                            je H1_44
                            dec edx
                            je H1_53
                        H1_2d:
                            lea edx, H1_a1
                            jmp H1_a1
                        H1_35:
                            lea edx, H1_b3
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp H1_b3
                        H1_44:
                            lea edx, H1_ad
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp H1_ad
                        H1_53:
                            lea edx, H1_a7
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp H1_a7
                        H1_62:
                            jmp edx
                        H1_64:
                            cmp byte ptr [esi], ah
                            je H1_a5
                        H1_68:
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            cmp byte ptr [esi], ah
                            je H1_ab
                        H1_76:
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            cmp byte ptr [esi], ah
                            je H1_b1
                        H1_84:
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            cmp byte ptr [esi], ah
                            je H1_b7
                        H1_92:
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            dec ecx
                            jne H1_64
                            jmp H1_bc
                        H1_a1:
                            cmp byte ptr [esi], ah
                            jne H1_68
                        H1_a5:
                            inc esi
                            inc edi
                        H1_a7:
                            cmp byte ptr [esi], ah
                            jne H1_76
                        H1_ab:
                            inc esi
                            inc edi
                        H1_ad:
                            cmp byte ptr [esi], ah
                            jne H1_84
                        H1_b1:
                            inc esi
                            inc edi
                        H1_b3:
                            cmp byte ptr [esi], ah
                            jne H1_92
                        H1_b7:
                            inc esi
                            inc edi
                            dec ecx
                            jne H1_a1
                        H1_bc:
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            add esi, dword ptr [SpriteRasterSourceRowTail]
                            add edi, dword ptr [SpriteRasterDestinationRowTail]
                            dec dword ptr [SpriteRasterDestinationRowCount]
                            jne H1_62
                            pop ebp
                            pop edi
                            pop esi
        }
    } else {
        scaled_height = static_cast<int>(iSpriteHeight_ * SpriteScalingRatio);
        scaled_cx = static_cast<int>(iSpriteWidth_ * SpriteScalingRatio);
        s_top = static_cast<int>(y + iTopOffset_ * SpriteScalingRatio);
        sc_left = static_cast<int>(x + iLeftOffset_ * SpriteScalingRatio);
        rc_clip.left = sc_left;
        rc_clip.top = s_top;
        rc_clip.right = sc_left + scaled_cx;
        rc_clip.bottom = s_top + scaled_height;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        rc_clip = original;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationPitch = destination->stride_;
        if (SpriteScaleNumerator < SpriteScaleDenominator) {
            if (original.right - original.left < SpriteMinimumScaledWidth
                || original.bottom - original.top < SpriteMinimumScaledHeight) {
                destination->free_data(1);
                return 0;
            }
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_shrink(&original, &rc_clip,
                &dest_col_ofs, &source_col, &SpriteRasterDestinationColumnCount,
                &SpriteRasterSourceColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_shrink(&original, &rc_clip,
                &dest_y, &source_y, &SpriteRasterDestinationRowCount,
                &SpriteRasterSourceRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            // Hand-written assembly: one-operand MUL row offset and computed
            // `jmp ebx` entry into an X-table stepped keyed fill.
            __asm {
                                push esi
                                push edi
                                mov eax, dword ptr [SpriteRasterSourcePitch]
                                mul dword ptr [source_y]
                                mov esi, eax
                                add esi, dword ptr [source_col]
                                add esi, dword ptr [source]
                                mov edi, dword ptr [dest_ptr]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                mov ah, byte ptr [transparent]
                                mov ebx, dword ptr [table]
                                push ebp
                                mov ebp, ebx
                                mov ebx, dword ptr [SpriteRasterDestinationColumnCount]
                                and ebx, 0x3
                                je H2_3e
                                dec ebx
                                je H2_51
                                dec ebx
                                je H2_6b
                                dec ebx
                                je H2_82
                            H2_3e:
                                lea ebx, H2_ec
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                jmp H2_ec
                            H2_51:
                                lea ebx, H2_10a
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp H2_10a
                            H2_6b:
                                lea ebx, H2_100
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp H2_100
                            H2_82:
                                lea ebx, H2_f6
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp H2_f6
                            H2_99:
                                jmp dword ptr [SpriteRasterShrinkEntry]
                            H2_9f:
                                cmp byte ptr [esi], ah
                                je H2_f0
                            H2_a3:
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H2_fa
                            H2_b5:
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H2_104
                            H2_c7:
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H2_10e
                            H2_d9:
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                dec ecx
                                jne H2_9f
                                jmp H2_117
                            H2_ec:
                                cmp byte ptr [esi], ah
                                jne H2_a3
                            H2_f0:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            H2_f6:
                                cmp byte ptr [esi], ah
                                jne H2_b5
                            H2_fa:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            H2_100:
                                cmp byte ptr [esi], ah
                                jne H2_c7
                            H2_104:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            H2_10a:
                                cmp byte ptr [esi], ah
                                jne H2_d9
                            H2_10e:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                                dec ecx
                                jne H2_ec
                            H2_117:
                                mov ecx, dword ptr [SpriteRasterYTable]
                                mov al, byte ptr [ecx]
                                add ecx, 0x4
                                mov dword ptr [SpriteRasterYTable], ecx
                                xor ecx, ecx
                                mov cl, al
                                dec ecx
                                je H2_138
                            H2_12f:
                                add esi, dword ptr [SpriteRasterSourcePitch]
                                dec ecx
                                jne H2_12f
                            H2_138:
                                add esi, dword ptr [SpriteRasterSourceRowTail]
                                add edi, dword ptr [SpriteRasterDestinationRowTail]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                dec dword ptr [SpriteRasterDestinationRowCount]
                                jne H2_99
                                pop ebp
                                pop edi
                                pop esi
            }
        } else {
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_expand(&original, &rc_clip,
                &source_col, &dest_col_ofs, &SpriteRasterSourceColumnCount,
                &SpriteRasterDestinationColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_expand(&original, &rc_clip,
                &source_y, &dest_y, &SpriteRasterSourceRowCount,
                &SpriteRasterDestinationRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterGroupCount = SpriteRasterSourceColumnCount >> 2;
            SpriteRasterGroupRemainder = SpriteRasterSourceColumnCount - (SpriteRasterGroupCount << 2);
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            SpriteRasterRemapTable.single = table;
            // Hand-written assembly: 16-bit PUSHF/POPF around CLD, EBP as the
            // row counter, and replicated expansion rows.
            __asm {
                                push edi
                                push esi
                                pushf
                                cld
                                mov eax, dword ptr [SpriteRasterSourcePitch]
                                mul dword ptr [source_y]
                                mov esi, eax
                                add esi, dword ptr [source_col]
                                add esi, dword ptr [source]
                                mov edi, dword ptr [dest_ptr]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                mov ah, byte ptr [transparent]
                                push ebp
                                mov ebp, dword ptr [SpriteRasterDestinationPitch]
                                je H3_40
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                            H3_40:
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                mov dword ptr [SpriteRasterColumnCountdown], ecx
                                mov ecx, dword ptr [SpriteRasterYTable]
                                mov al, byte ptr [ecx]
                                add ecx, 0x4
                                mov dword ptr [SpriteRasterYTable], ecx
                                mov edx, dword ptr [SpriteRasterXTable]
                                cmp al, 0x1
                                je H3_4be
                                cmp al, 0x2
                                je H3_38f
                                cmp al, 0x3
                                je H3_228
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je H3_9e
                                dec ebx
                                je H3_8f
                                dec ebx
                                je H3_94
                                dec ebx
                                je H3_99
                            H3_8f:
                                jmp H3_207
                            H3_94:
                                jmp H3_1f9
                            H3_99:
                                jmp H3_1eb
                            H3_9e:
                                cmp byte ptr [esi], ah
                                je H3_1e5
                            H3_a6:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_b4:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_b4
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_c0:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_c0
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_cc:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_cc
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_d8:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_d8
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_1f3
                            H3_f0:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_fe:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_fe
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_10a:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_10a
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_116:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_116
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_122:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_122
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_201
                            H3_13a:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_148:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_148
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_154:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_154
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_160:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_160
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_16c:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_16c
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_20f
                            H3_184:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_192:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_192
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_19e:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_19e
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_1aa:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_1aa
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_1b6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_1b6
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne H3_9e
                                lea edi, [edi+2*ebp]
                                lea edi, [edi+ebp]
                                jmp H3_58b
                            H3_1dd:
                                cmp byte ptr [esi], ah
                                jne H3_a6
                            H3_1e5:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_1eb:
                                cmp byte ptr [esi], ah
                                jne H3_f0
                            H3_1f3:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_1f9:
                                cmp byte ptr [esi], ah
                                jne H3_13a
                            H3_201:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_207:
                                cmp byte ptr [esi], ah
                                jne H3_184
                            H3_20f:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne H3_1dd
                                lea edi, [edi+2*ebp]
                                lea edi, [edi+ebp]
                                jmp H3_58b
                            H3_228:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je H3_24b
                                dec ebx
                                je H3_23c
                                dec ebx
                                je H3_241
                                dec ebx
                                je H3_246
                            H3_23c:
                                jmp H3_375
                            H3_241:
                                jmp H3_367
                            H3_246:
                                jmp H3_359
                            H3_24b:
                                cmp byte ptr [esi], ah
                                je H3_353
                            H3_253:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_261:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_261
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_26d:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_26d
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_279:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_279
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_361
                            H3_28f:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_29d:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_29d
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_2a9:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_2a9
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_2b5:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_2b5
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_36f
                            H3_2cb:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_2d9:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_2d9
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_2e5:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_2e5
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_2f1:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_2f1
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_379
                            H3_303:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_311:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_311
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_31d:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_31d
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_329:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_329
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne H3_24b
                                lea edi, [edi+2*ebp]
                                jmp H3_58b
                            H3_34b:
                                cmp byte ptr [esi], ah
                                jne H3_253
                            H3_353:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_359:
                                cmp byte ptr [esi], ah
                                jne H3_28f
                            H3_361:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_367:
                                cmp byte ptr [esi], ah
                                jne H3_2cb
                            H3_36f:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_375:
                                cmp byte ptr [esi], ah
                                jne H3_303
                            H3_379:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne H3_34b
                                lea edi, [edi+2*ebp]
                                jmp H3_58b
                            H3_38f:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je H3_3b2
                                dec ebx
                                je H3_3a3
                                dec ebx
                                je H3_3a8
                                dec ebx
                                je H3_3ad
                            H3_3a3:
                                jmp H3_4a4
                            H3_3a8:
                                jmp H3_496
                            H3_3ad:
                                jmp H3_488
                            H3_3b2:
                                cmp byte ptr [esi], ah
                                je H3_482
                            H3_3ba:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_3c8:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_3c8
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_3d4:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_3d4
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_490
                            H3_3e8:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_3f6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_3f6
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_402:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_402
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_49e
                            H3_416:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_424:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_424
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_430:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_430
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_4a8
                            H3_440:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_44e:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_44e
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            H3_45a:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_45a
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne H3_3b2
                                lea edi, [edi+ebp]
                                jmp H3_58b
                            H3_47a:
                                cmp byte ptr [esi], ah
                                jne H3_3ba
                            H3_482:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_488:
                                cmp byte ptr [esi], ah
                                jne H3_3e8
                            H3_490:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_496:
                                cmp byte ptr [esi], ah
                                jne H3_416
                            H3_49e:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_4a4:
                                cmp byte ptr [esi], ah
                                jne H3_440
                            H3_4a8:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne H3_47a
                                lea edi, [edi+ebp]
                                jmp H3_58b
                            H3_4be:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je H3_4e1
                                dec ebx
                                je H3_4d2
                                dec ebx
                                je H3_4d7
                                dec ebx
                                je H3_4dc
                            H3_4d2:
                                jmp H3_579
                            H3_4d7:
                                jmp H3_56f
                            H3_4dc:
                                jmp H3_565
                            H3_4e1:
                                cmp byte ptr [esi], ah
                                je H3_55f
                            H3_4e5:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_4f3:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_4f3
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_569
                            H3_501:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_50f:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_50f
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_573
                            H3_51d:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_52b:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_52b
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je H3_57d
                            H3_539:
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            H3_547:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne H3_547
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne H3_4e1
                                jmp H3_58b
                            H3_55b:
                                cmp byte ptr [esi], ah
                                jne H3_4e5
                            H3_55f:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_565:
                                cmp byte ptr [esi], ah
                                jne H3_501
                            H3_569:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_56f:
                                cmp byte ptr [esi], ah
                                jne H3_51d
                            H3_573:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            H3_579:
                                cmp byte ptr [esi], ah
                                jne H3_539
                            H3_57d:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne H3_55b
                            H3_58b:
                                add esi, dword ptr [SpriteRasterSourceRowTail]
                                add edi, dword ptr [SpriteRasterDestinationRowTail]
                                dec dword ptr [SpriteRasterSourceRowCount]
                                jne H3_40
                                pop ebp
                                popf
                                pop esi
                                pop edi
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
// Historical note: the earlier recovery draft described 0x009BEAE8 using
// guarded_teardowns.h's TeardownObject009BEAE8 scaffold name. The current
// source owner is BlankSpriteStorageOwner and its blank_sprite() accessor;
// retain that older interpretation here as provenance, not as a current owner.
// The pinned short-string read at 0x00698CAC independently establishes this
// filename's spelling and terminator, not its original allocation extent.
// RECOVERY_OBJECT: v1 0x00698CAC _JackalPcxPath
static const char JackalPcxPath[] = "jackal.pcx";

// The exit byte is a real separate global at 0x009BEAE0. The original callback
// sets bit 0 before calling Sprite::~Sprite; do not fold this byte into an
// invented prefix allocation or let a second implicit destructor bypass it.
// RECOVERY_OBJECT: v1 0x009BEAE0 ?BlankSpriteCleanupGuard@@3EA
uint8_t BlankSpriteCleanupGuard;

// Raw, four-byte-aligned storage starts at the original Sprite address
// 0x009BEAE8. Placement construction starts one Sprite lifetime; the trivial
// byte union suppresses the compiler's unconditional second teardown.
// ORIGINAL: 0x0063BAE0 sub_63bae0 0x0063BAE0-0x0063BAF6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Historical seed src/recovered/units/0063bae0.cpp retains the complete original span and BYTE_EXACT obligation; its TRIED finding is retained by the source marker writer.","The seed finding’s explanation is retained: the earlier general.cpp prototype measured 5/6 because its atexit argument cleanup used add esp,4 where the pinned startup uses pop ecx.","Pinned startup 0x0063BAE0 constructs the Sprite at 0x009BEAE8 and registers cleanup callback 0x0063BB00 with _atexit.","The compiler-generated initializer references this owner, its constructor, the generated exit callback and _atexit, and is selected through .CRT$XCU. The owner embeds a 44-byte rawstorage_.bytes member at offset zero, qualified through the directly bound standalone CodeView definition; this is storage/lifetime evidence, not an exact linked allocation claim."],"reason":"Bind the original startup identity to the genuine compiler-generated initializer for the source-owned blank Sprite lifetime."}
// symbol    _$E17
// TRIED: same finding as 0x00589870 - homed into general.cpp it measures 5 of 6
// Historical TRIED continuation: the earlier atexit argument cleanup used add esp,4 where the pinned startup uses pop ecx.
// ORIGINAL: 0x0063BB00 sub_63bb00 0x0063BB00-0x0063BB1F FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned 0x0063BB00-0x0063BB1F tests and sets bit 0 at 0x009BEAE0 before calling Sprite::~Sprite on 0x009BEAE8; the set-bit branch returns without a second teardown.","The source-owned generated exit callback tests and sets the separate cleanup guard, then destroys the Sprite in rawstorage_.bytes. Its physical COFF references select this guard, this owner storage and Sprite::close; no unconditional second destruction is introduced.","docs/recovery/ida9-functions.csv retains the complete catalogued callback extent. This new identity has no matching claim."],"reason":"Bind the generated exit callback to the real owner’s guarded Sprite teardown; preserve bit-set-before-destruction and avoid an implicit second dtor."}
// symbol    _$E15
// RECOVERY_OBJECT: v3 0x009BEAE8 ?g_BLANK_SPRITE@@3VSprite@@A member ?g_BLANK_SPRITE_OWNER@@3UBlankSpriteStorageOwner@@A rawstorage_.bytes
BlankSpriteStorageOwner g_BLANK_SPRITE_OWNER;

// ORIGINAL: 0x0063CDC0 sub_63cdc0 0x0063CDC0-0x0063CE17 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image span 0x0063CDC0-0x0063CE17: early argument-2/null and dword-at-+8 checks; argument-1 selects null or +0x444 Buffer; final call targets Sprite::draw at 0x005E4B9A.", "Current GraphicWin multiply-inherits Buffer after Win; sizeof(Win)==0x444. Typed GraphicWin-to-Buffer conversion replaces the observed +0x444 destination adjustment.", "The absolute reads 0x009BEAF0 and 0x009BEB04 are the blank Sprite's cTransparentIndex_ (+8) and iHeight_ (+0x1c) in source-owned storage at 0x009BEAE8; access them through blank_sprite().", "The retired seed recorded the same identity/span and historical BYTE_EXACT claim; the claim remains an obligation, not fresh proof.", "Argument 2 remains opaque: only the original dword-at-+8 nonzero test is recovered; no caller edge or enclosing type is established in the current mapped-source closure."], "reason": "Draw the source-owned shared blank Sprite through blank_sprite() and a typed GraphicWin/Buffer destination while preserving the unknown gate context's original dword test, centering and ignored draw result."}
// symbol    _sub_63cdc0
// Purpose: conditionally draw the shared blank sprite centered in a window buffer.
// No caller is inferred: mapped-span inspection reports call target 0x005E4B9A,
// and no caller edge for this entry in the current mapped closure.
// Preserved seed facts: the measured span was 87 bytes; the source prototype
// was unknown; catalog metadata reported zero callers, one call target,
// game code, flags sp_ready/purged_ok, and direct call target 0x005E4B9A.
extern "C" void __cdecl sub_63cdc0(GraphicWin *window, void *gate_context, int unused,
                       int x, int top, int region_height) {
  if (gate_context != 0 &&
      *reinterpret_cast<const uint32_t *>(
          static_cast<const unsigned char *>(gate_context) + 8) != 0) {
    Buffer *destination;
    if (window != 0) {
      destination = static_cast<Buffer *>(window);
    } else {
      destination = 0;
    }
    const int y = top + (region_height - blank_sprite().height()) / 2;
    blank_sprite().draw(destination, blank_sprite().transparent_index(), x, y);
  }
}

BlankSpriteStorageOwner::BlankSpriteStorageOwner() {
    new (&rawstorage_.bytes[0]) Sprite();
}

BlankSpriteStorageOwner::~BlankSpriteStorageOwner() {
    if ((BlankSpriteCleanupGuard & 1) != 0) {
        return;
    }
    BlankSpriteCleanupGuard |= 1;
    sprite().~Sprite();
}

// ORIGINAL: 0x0063CE20 sub_63ce20 0x0063CE20-0x0063CEE7;0x00663998-0x006639AD BYTE_EXACT
// kind      game
// symbol    ?init_class@Sprite@@SAHXZ
int Sprite::init_class() {
    Buffer buf;
    int result;
    if (buf.load_pcx(JackalPcxPath, 0, 10, 0xec) != 0) {
        result = blank_sprite().create_blank(0x10, 0x10, 9);
    } else {
        result = blank_sprite().extract(&buf, 9, 1, 0x12, 0x10, 0x10, 0);
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
Purpose: close_class - close the blank sprite (0x009BEAE8 in the image, the
         real object g_BLANK_SPRITE here), unguarded; the caller
         (jackal_close) tail-calls it as the sprite's teardown stage. Named
         Sprite::close_class 2026-08-30 as the mirror of Sprite::init_class
         (0x0063CE20) - the catalogue carries no name for either.
// ORIGINAL: 0x0063CEF0 sub_63cef0 0x0063CEF0-0x0063CEFA BYTE_EXACT
// symbol    ?close_class@Sprite@@SAXXZ
// size      10 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Sprite::close_class() {
    blank_sprite().close();
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
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
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
        pixels = buffer->get_data(left, top);
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
        pixels = buffer->get_data(left, top + height - 1);
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
        pixels = buffer->get_data(left, top + iTopOffset_);
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
        pixels = buffer->get_data(left, top + iTopOffset_);
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
            pcBits_ = static_cast<unsigned char *>(heap->get_mem(iSpriteHeight_ * iSpriteWidth_));
            if (pcBits_ != 0) {
                fObj1Exists_ = 1;
            }
        }
        do_sound();
        if (pcBits_ == 0) {
            pcBits_ = static_cast<unsigned char *>(mem_get(iSpriteWidth_ * iSpriteHeight_));
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
        pixels = buffer->get_data(left + iLeftOffset_, top + iTopOffset_);
        uint8_t *out = pcBits_;
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
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif




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
#pragma auto_inline(off)
int Sprite::create_blank(int width, int height, int depth) {
    iWidth_ = width;
    iSpriteWidth2_ = width;
    iSpriteWidth_ = 1;
    iSpriteHeight_ = 1;
    iLeftOffset_ = 0;
    iHeight_ = height;
    iTopOffset_ = 0;
    cTransparentIndex_ = static_cast<unsigned char>(depth);
    pcBits_ = static_cast<unsigned char *>(mem_get(1));
    return pcBits_ ? 0 : 4;
}
#pragma auto_inline(on)

// File-format view, not a recovered runtime class. Original persistence
// places stride immediately after the transparency byte, without padding.
#pragma pack(push, 1)
// File scalar access must retain byte alignment, including the stride at 0x19.
struct SpriteFileInt {
    int value;
};
struct SpriteFileHeader {
    int sprite_width;
    int sprite_height;
    int width;
    int height;
    int left_offset;
    int top_offset;
    unsigned char transparent_index;
    int stride;
};
#pragma pack(pop)
static_assert(sizeof(SpriteFileHeader) == 0x1D, "packed Sprite file header");

// ORIGINAL: 0x005EAF8F ?save@Sprite@@QAEHH@Z 0x005EAF8F-0x005EB0B2;0x00662C30-0x00662C42 BYTE_EXACT
// symbol    ?save@Sprite@@QAEHPBD@Z
// Writes the sprite to a Filemap-mapped file: six metadata ints, the
// transparency byte, the stride int - deliberately UNALIGNED, at offset
// 0x19 (the image stores it with mov after the byte, no alignment padding) -
// then the pixel block at 0x1D. The allocation adds 0x2C, independently of
// this packed prefix and the height*stride copy count. The stack-local Filemap is
// the image's SEH try/finally (0x00662C30/0x00662C38, __CxxFrameHandler):
// every early return runs its destructor without an explicit close.
// Calls 0x00628380 (ctor) 0x00628650 (create) 0x006287C0 (close)
// 0x006283E0 (dtor) 0x00645930 (memcpy).
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
int Sprite::save(const char *filename) {
    Filemap fm;
    if (pcBits_ == 0) {
        return 7;
    }
    SpriteFileHeader *header = static_cast<SpriteFileHeader *>(fm.create(
        filename,
        static_cast<uint32_t>(iSpriteWidth_) *
            static_cast<uint32_t>(iSpriteHeight_) + 0x2cu, 1));
    if (header == 0) {
        return 4;
    }
    SpriteFileInt *cursor = reinterpret_cast<SpriteFileInt *>(header);
    cursor++->value = iSpriteWidth_;
    cursor++->value = iSpriteHeight_;
    cursor++->value = iWidth_;
    cursor++->value = iHeight_;
    cursor++->value = iLeftOffset_;
    cursor++->value = iTopOffset_;
    unsigned char *byte_cursor = reinterpret_cast<unsigned char *>(cursor);
    *byte_cursor++ = static_cast<unsigned char>(cTransparentIndex_);
    cursor = reinterpret_cast<SpriteFileInt *>(byte_cursor);
    cursor++->value = iSpriteWidth2_;
    memcpy(cursor, pcBits_,
           static_cast<uint32_t>(iSpriteHeight_) * iSpriteWidth2_);
    fm.close();
    return 0;
}

// ORIGINAL: 0x005EB0BF ?load@Sprite@@QAEHH@Z 0x005EB0BF-0x005EB205;0x00662C42-0x00662C54
// symbol    ?load@Sprite@@QAEHPBD@Z
// close()'s cleanup as the head (borrowed pixels survive it, owned pixels
// are released and charged back through 0x009B6618, the name is freed, and
// like close() the transparency byte at 0x08 is not among the zero stores),
// then the save() header parse in reverse into the EXISTING pcBits_. The image
// never allocates: memcpy targets whatever pcBits_ still holds, so a sprite
// without borrowed pixels hands memcpy a null destination - no guard exists
// in the original and none is invented here. The unaligned stride read at
// 0x19 mirrors save()'s store. Calls 0x00628380 (ctor) 0x00628540 (open)
// 0x006283E0 (dtor) 0x006287C0 (close) 0x00644EF2 (free) 0x00645930 (memcpy).
int Sprite::load(const char *filename) {
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
    const SpriteFileHeader *header = static_cast<const SpriteFileHeader *>(
        fm.open(filename, 1));
    if (header == 0) {
        return 6;
    }
    const SpriteFileInt *cursor = reinterpret_cast<const SpriteFileInt *>(header);
    iSpriteWidth_ = cursor++->value;
    iSpriteHeight_ = cursor++->value;
    iWidth_ = cursor++->value;
    iHeight_ = cursor++->value;
    iLeftOffset_ = cursor++->value;
    iTopOffset_ = cursor++->value;
    const unsigned char *byte_cursor = reinterpret_cast<const unsigned char *>(cursor);
    cTransparentIndex_ = *byte_cursor++;
    cursor = reinterpret_cast<const SpriteFileInt *>(byte_cursor);
    iSpriteWidth2_ = cursor++->value;
    memcpy(pcBits_, cursor,
           static_cast<uint32_t>(iSpriteWidth2_) * iSpriteHeight_);
    fm.close();
    return 0;
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

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
// symbol    ?draw_multi_table_dest@Sprite@@QAEHPAVBuffer@@HHHHHPAY0BAA@E@Z
// size      80 bytes
// prototype int (__thiscall ?draw_multi_table_dest@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z)(Sprite* this, Buffer*, int, int, int, int, int, unsigned int8*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E8FF5
Status: Complete
*/
int Sprite::draw_multi_table_dest(Buffer *buffer, int a, int b, int c, int x,
                                  int y, ColorTable *bits) {
    const int saved_numerator = SpriteScaleNumerator;
    const int saved_denominator = SpriteScaleDenominator;
    SpriteScaleNumerator = x;
    SpriteScaleDenominator = y;
    const int result = draw_multi_table_dest(buffer, a, b, c, bits);
    SpriteScaleDenominator = saved_denominator;
    SpriteScaleNumerator = saved_numerator;
    return result;
}


// ORIGINAL: 0x005E8FF5 ?draw_multi_table_dest@Sprite@@QAEHPAUBuffer@@HHHPAE@Z 0x005E8FF5-0x005E9D44 FILE
// symbol    ?draw_multi_table_dest@Sprite@@QAEHPAVBuffer@@HHHPAY0BAA@E@Z
// draw_dest's 16-bit-table sibling: every drawn destination pixel samples the
// 256-byte row selected by the source pixel at the destination pixel column,
// and the transparency key
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
                                  int y, ColorTable *multi_table) {
    int source_y;
    int source_col;
    RECT rc_clip;
    int dest_col_ofs;
    unsigned char *dest_ptr;
    RECT original;
    int start_x;
    unsigned char *source;
    int dest_y;
    int start_y;
    int w;
    int scaled_height;
    int scaled_cx;
    int s_top;
    int sc_left;
    int h;
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0) {
        return 3;
    }
    if (SpriteScaleNumerator != SpriteScaleDenominator
        && (SpriteCachedScaleNumerator != SpriteScaleNumerator
            || SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        SpriteScalingRatio = static_cast<float>(SpriteScaleNumerator) / SpriteScaleDenominator;
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }
    transparent = static_cast<unsigned char>(transparent);
    SpriteRasterSourcePitch = iSpriteWidth2_;
    SpriteRasterDestinationPitch = destination->stride_;
    source = pcBits_;
    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        h = iSpriteHeight_;
        w = iSpriteWidth_;
        start_y = y + iTopOffset_;
        start_x = x + iLeftOffset_;
        rc_clip.left = start_x;
        rc_clip.top = start_y;
        rc_clip.right = start_x + w;
        rc_clip.bottom = start_y + h;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        source = pcBits_ + (rc_clip.left - original.left)
            + (rc_clip.top - original.top) * SpriteRasterSourcePitch;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationRowCount = rc_clip.bottom - rc_clip.top;
        SpriteRasterDestinationColumnCount = rc_clip.right - rc_clip.left;
        SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
        SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterDestinationColumnCount;
        SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
        // Hand-written assembly: computed `lea edx, label` / `jmp edx` entry
        // into an unrolled four-pixel keyed fill.
        __asm {
                            push esi
                            push edi
                            mov esi, dword ptr [source]
                            mov edi, dword ptr [dest_ptr]
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            mov al, byte ptr [transparent]
                            mov edx, dword ptr [multi_table]
                            xor ebx, ebx
                            push ebp
                            mov ebp, edx
                            mov edx, dword ptr [SpriteRasterDestinationColumnCount]
                            and edx, 0x3
                            je G1_2d
                            dec edx
                            je G1_35
                            dec edx
                            je G1_44
                            dec edx
                            je G1_53
                        G1_2d:
                            lea edx, G1_a9
                            jmp G1_a9
                        G1_35:
                            lea edx, G1_bb
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp G1_bb
                        G1_44:
                            lea edx, G1_b5
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp G1_b5
                        G1_53:
                            lea edx, G1_af
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp G1_af
                        G1_62:
                            jmp edx
                        G1_64:
                            cmp byte ptr [esi], al
                            je G1_ad
                        G1_68:
                            mov bh, byte ptr [esi]
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            cmp byte ptr [esi], al
                            je G1_b3
                        G1_78:
                            mov bh, byte ptr [esi]
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            cmp byte ptr [esi], al
                            je G1_b9
                        G1_88:
                            mov bh, byte ptr [esi]
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            cmp byte ptr [esi], al
                            je G1_bf
                        G1_98:
                            mov bh, byte ptr [esi]
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                            inc esi
                            inc edi
                            dec ecx
                            jne G1_64
                            jmp G1_c4
                        G1_a9:
                            cmp byte ptr [esi], al
                            jne G1_68
                        G1_ad:
                            inc esi
                            inc edi
                        G1_af:
                            cmp byte ptr [esi], al
                            jne G1_78
                        G1_b3:
                            inc esi
                            inc edi
                        G1_b5:
                            cmp byte ptr [esi], al
                            jne G1_88
                        G1_b9:
                            inc esi
                            inc edi
                        G1_bb:
                            cmp byte ptr [esi], al
                            jne G1_98
                        G1_bf:
                            inc esi
                            inc edi
                            dec ecx
                            jne G1_a9
                        G1_c4:
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            add esi, dword ptr [SpriteRasterSourceRowTail]
                            add edi, dword ptr [SpriteRasterDestinationRowTail]
                            dec dword ptr [SpriteRasterDestinationRowCount]
                            jne G1_62
                            pop ebp
                            pop edi
                            pop esi
        }
    } else {
        scaled_height = static_cast<int>(iSpriteHeight_ * SpriteScalingRatio);
        scaled_cx = static_cast<int>(iSpriteWidth_ * SpriteScalingRatio);
        s_top = static_cast<int>(y + iTopOffset_ * SpriteScalingRatio);
        sc_left = static_cast<int>(x + iLeftOffset_ * SpriteScalingRatio);
        rc_clip.left = sc_left;
        rc_clip.top = s_top;
        rc_clip.right = sc_left + scaled_cx;
        rc_clip.bottom = s_top + scaled_height;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        rc_clip = original;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationPitch = destination->stride_;
        if (SpriteScaleNumerator < SpriteScaleDenominator) {
            if (original.right - original.left < SpriteMinimumScaledWidth
                || original.bottom - original.top < SpriteMinimumScaledHeight) {
                destination->free_data(1);
                return 0;
            }
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_shrink(&original, &rc_clip,
                &dest_col_ofs, &source_col, &SpriteRasterDestinationColumnCount,
                &SpriteRasterSourceColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_shrink(&original, &rc_clip,
                &dest_y, &source_y, &SpriteRasterDestinationRowCount,
                &SpriteRasterSourceRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            // Hand-written assembly: one-operand MUL row offset and computed
            // `jmp ebx` entry into an X-table stepped keyed fill.
            __asm {
                                push esi
                                push edi
                                mov eax, dword ptr [SpriteRasterSourcePitch]
                                mul dword ptr [source_y]
                                mov esi, eax
                                add esi, dword ptr [source_col]
                                add esi, dword ptr [source]
                                mov edi, dword ptr [dest_ptr]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                mov ah, byte ptr [transparent]
                                mov ebx, dword ptr [multi_table]
                                push ebp
                                mov ebp, ebx
                                mov ebx, dword ptr [SpriteRasterDestinationColumnCount]
                                and ebx, 0x3
                                je G2_3e
                                dec ebx
                                je G2_51
                                dec ebx
                                je G2_6b
                                dec ebx
                                je G2_85
                            G2_3e:
                                lea ebx, G2_f7
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                jmp G2_f7
                            G2_51:
                                lea ebx, G2_115
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp G2_115
                            G2_6b:
                                lea ebx, G2_10b
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp G2_10b
                            G2_85:
                                lea ebx, G2_101
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp G2_101
                            G2_9c:
                                jmp dword ptr [SpriteRasterShrinkEntry]
                            G2_a2:
                                cmp byte ptr [esi], ah
                                je G2_fb
                            G2_a6:
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G2_105
                            G2_ba:
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G2_10f
                            G2_ce:
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G2_119
                            G2_e2:
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                dec ecx
                                jne G2_a2
                                jmp G2_122
                            G2_f7:
                                cmp byte ptr [esi], ah
                                jne G2_a6
                            G2_fb:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            G2_101:
                                cmp byte ptr [esi], ah
                                jne G2_ba
                            G2_105:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            G2_10b:
                                cmp byte ptr [esi], ah
                                jne G2_ce
                            G2_10f:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            G2_115:
                                cmp byte ptr [esi], ah
                                jne G2_e2
                            G2_119:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                                dec ecx
                                jne G2_f7
                            G2_122:
                                mov ecx, dword ptr [SpriteRasterYTable]
                                mov al, byte ptr [ecx]
                                add ecx, 0x4
                                mov dword ptr [SpriteRasterYTable], ecx
                                xor ecx, ecx
                                mov cl, al
                                dec ecx
                                je G2_143
                            G2_13a:
                                add esi, dword ptr [SpriteRasterSourcePitch]
                                dec ecx
                                jne G2_13a
                            G2_143:
                                add esi, dword ptr [SpriteRasterSourceRowTail]
                                add edi, dword ptr [SpriteRasterDestinationRowTail]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                dec dword ptr [SpriteRasterDestinationRowCount]
                                jne G2_9c
                                pop ebp
                                pop edi
                                pop esi
            }
        } else {
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_expand(&original, &rc_clip,
                &source_col, &dest_col_ofs, &SpriteRasterSourceColumnCount,
                &SpriteRasterDestinationColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_expand(&original, &rc_clip,
                &source_y, &dest_y, &SpriteRasterSourceRowCount,
                &SpriteRasterDestinationRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterGroupCount = SpriteRasterSourceColumnCount >> 2;
            SpriteRasterGroupRemainder = SpriteRasterSourceColumnCount - (SpriteRasterGroupCount << 2);
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            SpriteRasterRemapTable.multi = multi_table;
            // Hand-written assembly: 16-bit PUSHF/POPF around CLD, EBP as the
            // row counter, and replicated expansion rows.
            __asm {
                                push edi
                                push esi
                                pushf
                                cld
                                mov eax, dword ptr [SpriteRasterSourcePitch]
                                mul dword ptr [source_y]
                                mov esi, eax
                                add esi, dword ptr [source_col]
                                add esi, dword ptr [source]
                                mov edi, dword ptr [dest_ptr]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ah, byte ptr [transparent]
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                push ebp
                                mov ebp, dword ptr [SpriteRasterDestinationPitch]
                                je G3_40
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                            G3_40:
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                mov dword ptr [SpriteRasterColumnCountdown], ecx
                                mov ecx, dword ptr [SpriteRasterYTable]
                                mov al, byte ptr [ecx]
                                add ecx, 0x4
                                mov dword ptr [SpriteRasterYTable], ecx
                                mov edx, dword ptr [SpriteRasterXTable]
                                cmp al, 0x1
                                je G3_4d6
                                cmp al, 0x2
                                je G3_39f
                                cmp al, 0x3
                                je G3_230
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je G3_9e
                                dec ebx
                                je G3_8f
                                dec ebx
                                je G3_94
                                dec ebx
                                je G3_99
                            G3_8f:
                                jmp G3_20f
                            G3_94:
                                jmp G3_201
                            G3_99:
                                jmp G3_1f3
                            G3_9e:
                                cmp byte ptr [esi], ah
                                je G3_1ed
                            G3_a6:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_b6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_b6
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_c2:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_c2
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_ce:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_ce
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_da:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_da
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_1fb
                            G3_f2:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_102:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_102
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_10e:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_10e
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_11a:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_11a
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_126:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_126
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_209
                            G3_13e:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_14e:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_14e
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_15a:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_15a
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_166:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_166
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_172:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_172
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_217
                            G3_18a:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_19a:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_19a
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_1a6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_1a6
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_1b2:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_1b2
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_1be:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_1be
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne G3_9e
                                lea edi, [edi+2*ebp]
                                lea edi, [edi+ebp]
                                jmp G3_5b7
                            G3_1e5:
                                cmp byte ptr [esi], ah
                                jne G3_a6
                            G3_1ed:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_1f3:
                                cmp byte ptr [esi], ah
                                jne G3_f2
                            G3_1fb:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_201:
                                cmp byte ptr [esi], ah
                                jne G3_13e
                            G3_209:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_20f:
                                cmp byte ptr [esi], ah
                                jne G3_18a
                            G3_217:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne G3_1e5
                                lea edi, [edi+2*ebp]
                                lea edi, [edi+ebp]
                                jmp G3_5b7
                            G3_230:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je G3_253
                                dec ebx
                                je G3_244
                                dec ebx
                                je G3_249
                                dec ebx
                                je G3_24e
                            G3_244:
                                jmp G3_385
                            G3_249:
                                jmp G3_377
                            G3_24e:
                                jmp G3_369
                            G3_253:
                                cmp byte ptr [esi], ah
                                je G3_363
                            G3_25b:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_26b:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_26b
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_277:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_277
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_283:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_283
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_371
                            G3_299:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_2a9:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_2a9
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_2b5:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_2b5
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_2c1:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_2c1
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_37f
                            G3_2d7:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_2e7:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_2e7
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_2f3:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_2f3
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_2ff:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_2ff
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_389
                            G3_311:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_321:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_321
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_32d:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_32d
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_339:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_339
                                sub edi, ebp
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne G3_253
                                lea edi, [edi+2*ebp]
                                jmp G3_5b7
                            G3_35b:
                                cmp byte ptr [esi], ah
                                jne G3_25b
                            G3_363:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_369:
                                cmp byte ptr [esi], ah
                                jne G3_299
                            G3_371:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_377:
                                cmp byte ptr [esi], ah
                                jne G3_2d7
                            G3_37f:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_385:
                                cmp byte ptr [esi], ah
                                jne G3_311
                            G3_389:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne G3_35b
                                lea edi, [edi+2*ebp]
                                jmp G3_5b7
                            G3_39f:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je G3_3c2
                                dec ebx
                                je G3_3b3
                                dec ebx
                                je G3_3b8
                                dec ebx
                                je G3_3bd
                            G3_3b3:
                                jmp G3_4bc
                            G3_3b8:
                                jmp G3_4ae
                            G3_3bd:
                                jmp G3_4a0
                            G3_3c2:
                                cmp byte ptr [esi], ah
                                je G3_49a
                            G3_3ca:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_3da:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_3da
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_3e6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_3e6
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_4a8
                            G3_3fa:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_40a:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_40a
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_416:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_416
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_4b6
                            G3_42a:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_43a:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_43a
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_446:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_446
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_4c0
                            G3_456:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_466:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_466
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            G3_472:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_472
                                sub edi, ebp
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne G3_3c2
                                lea edi, [edi+ebp]
                                jmp G3_5b7
                            G3_492:
                                cmp byte ptr [esi], ah
                                jne G3_3ca
                            G3_49a:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_4a0:
                                cmp byte ptr [esi], ah
                                jne G3_3fa
                            G3_4a8:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_4ae:
                                cmp byte ptr [esi], ah
                                jne G3_42a
                            G3_4b6:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_4bc:
                                cmp byte ptr [esi], ah
                                jne G3_456
                            G3_4c0:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne G3_492
                                lea edi, [edi+ebp]
                                jmp G3_5b7
                            G3_4d6:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je G3_4f9
                                dec ebx
                                je G3_4ea
                                dec ebx
                                je G3_4ef
                                dec ebx
                                je G3_4f4
                            G3_4ea:
                                jmp G3_5a5
                            G3_4ef:
                                jmp G3_59b
                            G3_4f4:
                                jmp G3_591
                            G3_4f9:
                                cmp byte ptr [esi], ah
                                je G3_58b
                            G3_501:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_511:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_511
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_595
                            G3_51f:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_52f:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_52f
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_59f
                            G3_53d:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_54d:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_54d
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je G3_5a9
                            G3_55b:
                                xor ebx, ebx
                                mov bh, byte ptr [esi]
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                                mov ecx, dword ptr [edx]
                            G3_56b:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne G3_56b
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne G3_4f9
                                jmp G3_5b7
                            G3_583:
                                cmp byte ptr [esi], ah
                                jne G3_501
                            G3_58b:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_591:
                                cmp byte ptr [esi], ah
                                jne G3_51f
                            G3_595:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_59b:
                                cmp byte ptr [esi], ah
                                jne G3_53d
                            G3_59f:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            G3_5a5:
                                cmp byte ptr [esi], ah
                                jne G3_55b
                            G3_5a9:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne G3_583
                            G3_5b7:
                                add esi, dword ptr [SpriteRasterSourceRowTail]
                                add edi, dword ptr [SpriteRasterDestinationRowTail]
                                dec dword ptr [SpriteRasterSourceRowCount]
                                jne G3_40
                                pop ebp
                                popf
                                pop esi
                                pop edi
            }
        }
    }
    destination->free_data(1);
    return 0;
}

// ORIGINAL: 0x005E81BC ?draw_dest_unk2@Sprite@@QAEHPAUBuffer@@HHHPAE@Z 0x005E81BC-0x005E8FA5 FILE
// symbol    ?draw_dest_unk2@Sprite@@QAEHPAVBuffer@@HHHPAE@Z
// Stencil remap: at unit scale, non-key/nonzero source pixels select table[dst].
// The shrink Duff walk instead selects table[src], with the original asymmetric
// zero test: lane zero maps nonzero, while the other three lanes map zero.
// A partial group enters at lane 4-(width%4); each row restarts that same lane.
// Expansion skips key/zero source pixels. Its one/two/three-row kernels map
// table[src], but the default four-row kernel samples table[first destination]
// once per source pixel and replicates that mapped value across its output run.
// The original saves/restores FLAGS around its unrolled assembly. Recovering
// that code generation and complete shared-scratch alias behavior remains
// matching debt; this C++ loop is not a claim of whole-span equivalence.
// TRIED (seed): two indirect back-edges (jmp edx @0x5E8420, jmp [0x9B3AC4]
//         @0x5E8766) are Duff's-device dispatch; rewritten as a
//         switch(remainder) on the loop-invariant state. The row-repeat count
//         loop @0x5E87F8-0x5E8817 (esi += stride, N-1 times from a y-table
//         byte) is a case Ghidra silently dropped; reconstructed from raw
//         bytes.
int Sprite::draw_dest_unk2(Buffer *destination, int transparent, int x, int y,
                           unsigned char *table) {
    int source_y;
    int source_col;
    RECT rc_clip;
    int dest_col_ofs;
    unsigned char *dest_ptr;
    RECT original;
    int start_x;
    unsigned char *source;
    int dest_y;
    int start_y;
    int w;
    int scaled_height;
    int scaled_cx;
    int s_top;
    int sc_left;
    int h;
    if (pcBits_ == 0 || destination == 0 || destination->get_data() == 0) {
        return 3;
    }
    if (SpriteScaleNumerator != SpriteScaleDenominator
        && (SpriteCachedScaleNumerator != SpriteScaleNumerator
            || SpriteCachedScaleDenominator != SpriteScaleDenominator)) {
        SpriteScalingRatio = static_cast<float>(SpriteScaleNumerator) / SpriteScaleDenominator;
        if (SpriteScalingRatio > 4.0) {
            destination->free_data(1);
            return 0;
        }
        fill_scaling_table();
        SpriteCachedScaleNumerator = SpriteScaleNumerator;
        SpriteCachedScaleDenominator = SpriteScaleDenominator;
    }
    transparent = static_cast<unsigned char>(transparent);
    SpriteRasterSourcePitch = iSpriteWidth2_;
    SpriteRasterDestinationPitch = destination->stride_;
    source = pcBits_;
    if (SpriteScaleNumerator == SpriteScaleDenominator) {
        h = iSpriteHeight_;
        w = iSpriteWidth_;
        start_y = y + iTopOffset_;
        start_x = x + iLeftOffset_;
        rc_clip.left = start_x;
        rc_clip.top = start_y;
        rc_clip.right = start_x + w;
        rc_clip.bottom = start_y + h;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        source = pcBits_ + (rc_clip.left - original.left)
            + (rc_clip.top - original.top) * SpriteRasterSourcePitch;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationRowCount = rc_clip.bottom - rc_clip.top;
        SpriteRasterDestinationColumnCount = rc_clip.right - rc_clip.left;
        SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
        SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterDestinationColumnCount;
        SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
        // Hand-written assembly: computed `lea edx, label` / `jmp edx` entry
        // into an unrolled four-pixel keyed fill.
        __asm {
                            push esi
                            push edi
                            mov esi, dword ptr [source]
                            mov edi, dword ptr [dest_ptr]
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            mov al, byte ptr [transparent]
                            mov edx, dword ptr [table]
                            xor ebx, ebx
                            push ebp
                            mov ebp, edx
                            mov edx, dword ptr [SpriteRasterDestinationColumnCount]
                            and edx, 0x3
                            je F1_2d
                            dec edx
                            je F1_38
                            dec edx
                            je F1_4a
                            dec edx
                            je F1_59
                        F1_2d:
                            lea edx, F1_bb
                            jmp F1_bb
                        F1_38:
                            lea edx, F1_cd
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp F1_cd
                        F1_4a:
                            lea edx, F1_c7
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp F1_c7
                        F1_59:
                            lea edx, F1_c1
                            inc ecx
                            mov dword ptr [SpriteRasterGroupCount], ecx
                            jmp F1_c1
                        F1_68:
                            jmp edx
                        F1_6a:
                            cmp byte ptr [esi], al
                            je F1_bf
                        F1_6e:
                            cmp byte ptr [esi], 0x0
                            je F1_7b
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                        F1_7b:
                            inc esi
                            inc edi
                            cmp byte ptr [esi], al
                            je F1_c5
                        F1_81:
                            cmp byte ptr [esi], 0x0
                            je F1_8e
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                        F1_8e:
                            inc esi
                            inc edi
                            cmp byte ptr [esi], al
                            je F1_cb
                        F1_94:
                            cmp byte ptr [esi], 0x0
                            je F1_a1
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                        F1_a1:
                            inc esi
                            inc edi
                            cmp byte ptr [esi], al
                            je F1_d1
                        F1_a7:
                            cmp byte ptr [esi], 0x0
                            je F1_b4
                            mov bl, byte ptr [edi]
                            mov bl, byte ptr [ebp+ebx]
                            mov byte ptr [edi], bl
                        F1_b4:
                            inc esi
                            inc edi
                            dec ecx
                            jne F1_6a
                            jmp F1_d6
                        F1_bb:
                            cmp byte ptr [esi], al
                            jne F1_6e
                        F1_bf:
                            inc esi
                            inc edi
                        F1_c1:
                            cmp byte ptr [esi], al
                            jne F1_81
                        F1_c5:
                            inc esi
                            inc edi
                        F1_c7:
                            cmp byte ptr [esi], al
                            jne F1_94
                        F1_cb:
                            inc esi
                            inc edi
                        F1_cd:
                            cmp byte ptr [esi], al
                            jne F1_a7
                        F1_d1:
                            inc esi
                            inc edi
                            dec ecx
                            jne F1_bb
                        F1_d6:
                            mov ecx, dword ptr [SpriteRasterGroupCount]
                            add esi, dword ptr [SpriteRasterSourceRowTail]
                            add edi, dword ptr [SpriteRasterDestinationRowTail]
                            dec dword ptr [SpriteRasterDestinationRowCount]
                            jne F1_68
                            pop ebp
                            pop edi
                            pop esi
        }
    } else {
        scaled_height = static_cast<int>(iSpriteHeight_ * SpriteScalingRatio);
        scaled_cx = static_cast<int>(iSpriteWidth_ * SpriteScalingRatio);
        s_top = static_cast<int>(y + iTopOffset_ * SpriteScalingRatio);
        sc_left = static_cast<int>(x + iLeftOffset_ * SpriteScalingRatio);
        rc_clip.left = sc_left;
        rc_clip.top = s_top;
        rc_clip.right = sc_left + scaled_cx;
        rc_clip.bottom = s_top + scaled_height;
        original = rc_clip;
        if (!IntersectRect(&rc_clip, &rc_clip, &destination->rect1_)) {
            destination->free_data(1);
            return 0;
        }
        rc_clip = original;
        dest_ptr = static_cast<unsigned char *>(destination->locked_bits_) + rc_clip.left
            + rc_clip.top * SpriteRasterDestinationPitch;
        SpriteRasterDestinationPitch = destination->stride_;
        if (SpriteScaleNumerator < SpriteScaleDenominator) {
            if (original.right - original.left < SpriteMinimumScaledWidth
                || original.bottom - original.top < SpriteMinimumScaledHeight) {
                destination->free_data(1);
                return 0;
            }
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_shrink(&original, &rc_clip,
                &dest_col_ofs, &source_col, &SpriteRasterDestinationColumnCount,
                &SpriteRasterSourceColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_shrink(&original, &rc_clip,
                &dest_y, &source_y, &SpriteRasterDestinationRowCount,
                &SpriteRasterSourceRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterGroupCount = SpriteRasterDestinationColumnCount >> 2;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            // Hand-written assembly: one-operand MUL row offset and computed
            // `jmp ebx` entry into an X-table stepped keyed fill.
            __asm {
                                push esi
                                push edi
                                mov eax, dword ptr [SpriteRasterSourcePitch]
                                mul dword ptr [source_y]
                                mov esi, eax
                                add esi, dword ptr [source_col]
                                add esi, dword ptr [source]
                                mov edi, dword ptr [dest_ptr]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                mov ah, byte ptr [transparent]
                                mov ebx, dword ptr [table]
                                push ebp
                                mov ebp, ebx
                                mov ebx, dword ptr [SpriteRasterDestinationColumnCount]
                                and ebx, 0x3
                                je F2_3e
                                dec ebx
                                je F2_51
                                dec ebx
                                je F2_6b
                                dec ebx
                                je F2_85
                            F2_3e:
                                lea ebx, F2_103
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                jmp F2_103
                            F2_51:
                                lea ebx, F2_121
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp F2_121
                            F2_6b:
                                lea ebx, F2_117
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp F2_117
                            F2_85:
                                lea ebx, F2_10d
                                mov dword ptr [SpriteRasterShrinkEntry], ebx
                                xor ebx, ebx
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                                jmp F2_10d
                            F2_9c:
                                jmp dword ptr [SpriteRasterShrinkEntry]
                            F2_a2:
                                cmp byte ptr [esi], ah
                                je F2_107
                            F2_a6:
                                cmp byte ptr [esi], 0x0
                                je F2_b3
                                mov bl, byte ptr [esi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                            F2_b3:
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F2_111
                            F2_bd:
                                cmp byte ptr [esi], 0x0
                                jne F2_ca
                                mov bl, byte ptr [esi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                            F2_ca:
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F2_11b
                            F2_d4:
                                cmp byte ptr [esi], 0x0
                                jne F2_e1
                                mov bl, byte ptr [esi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                            F2_e1:
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F2_125
                            F2_eb:
                                cmp byte ptr [esi], 0x0
                                jne F2_f8
                                mov bl, byte ptr [esi]
                                mov bl, byte ptr [ebp+ebx]
                                mov byte ptr [edi], bl
                            F2_f8:
                                add esi, dword ptr [edx]
                                inc edi
                                add edx, 0x4
                                dec ecx
                                jne F2_a2
                                jmp F2_12e
                            F2_103:
                                cmp byte ptr [esi], ah
                                jne F2_a6
                            F2_107:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            F2_10d:
                                cmp byte ptr [esi], ah
                                jne F2_bd
                            F2_111:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            F2_117:
                                cmp byte ptr [esi], ah
                                jne F2_d4
                            F2_11b:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                            F2_121:
                                cmp byte ptr [esi], ah
                                jne F2_eb
                            F2_125:
                                inc edi
                                add esi, dword ptr [edx]
                                add edx, 0x4
                                dec ecx
                                jne F2_103
                            F2_12e:
                                mov ecx, dword ptr [SpriteRasterYTable]
                                mov al, byte ptr [ecx]
                                add ecx, 0x4
                                mov dword ptr [SpriteRasterYTable], ecx
                                xor ecx, ecx
                                mov cl, al
                                dec ecx
                                je F2_14f
                            F2_146:
                                add esi, dword ptr [SpriteRasterSourcePitch]
                                dec ecx
                                jne F2_146
                            F2_14f:
                                add esi, dword ptr [SpriteRasterSourceRowTail]
                                add edi, dword ptr [SpriteRasterDestinationRowTail]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                dec dword ptr [SpriteRasterDestinationRowCount]
                                jne F2_9c
                                pop ebp
                                pop edi
                                pop esi
            }
        } else {
            IntersectRect(&rc_clip, &original, &destination->rect1_);
            SpriteRasterXTable = get_clipped_x_table_expand(&original, &rc_clip,
                &source_col, &dest_col_ofs, &SpriteRasterSourceColumnCount,
                &SpriteRasterDestinationColumnCount);
            if (SpriteRasterXTable == 0) {
                destination->free_data(1);
                return 0;
            }
            SpriteRasterYTable = get_clipped_y_table_expand(&original, &rc_clip,
                &source_y, &dest_y, &SpriteRasterSourceRowCount,
                &SpriteRasterDestinationRowCount);
            if (SpriteRasterYTable == 0) {
                destination->free_data(1);
                return 0;
            }
            dest_ptr += dest_col_ofs + SpriteRasterDestinationPitch * dest_y;
            SpriteRasterDestinationRowTail = SpriteRasterDestinationPitch - SpriteRasterDestinationColumnCount;
            SpriteRasterGroupCount = SpriteRasterSourceColumnCount >> 2;
            SpriteRasterGroupRemainder = SpriteRasterSourceColumnCount - (SpriteRasterGroupCount << 2);
            SpriteRasterSourceRowTail = SpriteRasterSourcePitch - SpriteRasterSourceColumnCount;
            SpriteRasterRemapTable.single = table;
            // Hand-written assembly: 16-bit PUSHF/POPF around CLD, EBP as the
            // row counter, and replicated expansion rows.
            __asm {
                                push edi
                                push esi
                                pushf
                                cld
                                mov eax, dword ptr [SpriteRasterSourcePitch]
                                mul dword ptr [source_y]
                                mov esi, eax
                                add esi, dword ptr [source_col]
                                add esi, dword ptr [source]
                                mov edi, dword ptr [dest_ptr]
                                mov edx, dword ptr [SpriteRasterXTable]
                                mov ah, byte ptr [transparent]
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                push ebp
                                mov ebp, dword ptr [SpriteRasterDestinationPitch]
                                je F3_40
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                inc ecx
                                mov dword ptr [SpriteRasterGroupCount], ecx
                            F3_40:
                                mov ecx, dword ptr [SpriteRasterGroupCount]
                                mov dword ptr [SpriteRasterColumnCountdown], ecx
                                mov ecx, dword ptr [SpriteRasterYTable]
                                mov al, byte ptr [ecx]
                                add ecx, 0x4
                                mov dword ptr [SpriteRasterYTable], ecx
                                mov edx, dword ptr [SpriteRasterXTable]
                                cmp al, 0x1
                                je F3_52a
                                cmp al, 0x2
                                je F3_3d7
                                cmp al, 0x3
                                je F3_24c
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je F3_9e
                                dec ebx
                                je F3_8f
                                dec ebx
                                je F3_94
                                dec ebx
                                je F3_99
                            F3_8f:
                                jmp F3_22b
                            F3_94:
                                jmp F3_21d
                            F3_99:
                                jmp F3_20f
                            F3_9e:
                                cmp byte ptr [esi], ah
                                je F3_209
                            F3_a6:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_eb
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_b9:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_b9
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_c5:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_c5
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_d1:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_d1
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_dd:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_dd
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                jmp F3_ed
                            F3_eb:
                                add edi, ecx
                            F3_ed:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_217
                            F3_f9:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_13e
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_10c:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_10c
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_118:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_118
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_124:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_124
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_130:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_130
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                jmp F3_140
                            F3_13e:
                                add edi, ecx
                            F3_140:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_225
                            F3_14c:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_191
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_15f:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_15f
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_16b:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_16b
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_177:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_177
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_183:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_183
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                jmp F3_193
                            F3_191:
                                add edi, ecx
                            F3_193:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_233
                            F3_19f:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_1e4
                                xor ebx, ebx
                                mov bl, byte ptr [edi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_1b2:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_1b2
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_1be:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_1be
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_1ca:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_1ca
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_1d6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_1d6
                                sub edi, ebp
                                sub edi, ebp
                                sub edi, ebp
                                jmp F3_1e6
                            F3_1e4:
                                add edi, ecx
                            F3_1e6:
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne F3_9e
                                lea edi, [edi+2*ebp]
                                lea edi, [edi+ebp]
                                jmp F3_62f
                            F3_201:
                                cmp byte ptr [esi], ah
                                jne F3_a6
                            F3_209:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_20f:
                                cmp byte ptr [esi], ah
                                jne F3_f9
                            F3_217:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_21d:
                                cmp byte ptr [esi], ah
                                jne F3_14c
                            F3_225:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_22b:
                                cmp byte ptr [esi], ah
                                jne F3_19f
                            F3_233:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne F3_201
                                lea edi, [edi+2*ebp]
                                lea edi, [edi+ebp]
                                jmp F3_62f
                            F3_24c:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je F3_26f
                                dec ebx
                                je F3_260
                                dec ebx
                                je F3_265
                                dec ebx
                                je F3_26a
                            F3_260:
                                jmp F3_3bd
                            F3_265:
                                jmp F3_3af
                            F3_26a:
                                jmp F3_3a1
                            F3_26f:
                                cmp byte ptr [esi], ah
                                je F3_39b
                            F3_277:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_2ae
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_28a:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_28a
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_296:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_296
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_2a2:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_2a2
                                sub edi, ebp
                                sub edi, ebp
                                jmp F3_2b0
                            F3_2ae:
                                add edi, ecx
                            F3_2b0:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_3a9
                            F3_2bc:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_2f3
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_2cf:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_2cf
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_2db:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_2db
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_2e7:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_2e7
                                sub edi, ebp
                                sub edi, ebp
                                jmp F3_2f5
                            F3_2f3:
                                add edi, ecx
                            F3_2f5:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_3b7
                            F3_301:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_338
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_314:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_314
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_320:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_320
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_32c:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_32c
                                sub edi, ebp
                                sub edi, ebp
                                jmp F3_33a
                            F3_338:
                                add edi, ecx
                            F3_33a:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_3c1
                            F3_342:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_379
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_355:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_355
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_361:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_361
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_36d:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_36d
                                sub edi, ebp
                                sub edi, ebp
                                jmp F3_37b
                            F3_379:
                                add edi, ecx
                            F3_37b:
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne F3_26f
                                lea edi, [edi+2*ebp]
                                jmp F3_62f
                            F3_393:
                                cmp byte ptr [esi], ah
                                jne F3_277
                            F3_39b:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_3a1:
                                cmp byte ptr [esi], ah
                                jne F3_2bc
                            F3_3a9:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_3af:
                                cmp byte ptr [esi], ah
                                jne F3_301
                            F3_3b7:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_3bd:
                                cmp byte ptr [esi], ah
                                jne F3_342
                            F3_3c1:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne F3_393
                                lea edi, [edi+2*ebp]
                                jmp F3_62f
                            F3_3d7:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je F3_3fa
                                dec ebx
                                je F3_3eb
                                dec ebx
                                je F3_3f0
                                dec ebx
                                je F3_3f5
                            F3_3eb:
                                jmp F3_510
                            F3_3f0:
                                jmp F3_502
                            F3_3f5:
                                jmp F3_4f4
                            F3_3fa:
                                cmp byte ptr [esi], ah
                                je F3_4ee
                            F3_402:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_42b
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_415:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_415
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_421:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_421
                                sub edi, ebp
                                jmp F3_42d
                            F3_42b:
                                add edi, ecx
                            F3_42d:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_4fc
                            F3_439:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_462
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_44c:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_44c
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_458:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_458
                                sub edi, ebp
                                jmp F3_464
                            F3_462:
                                add edi, ecx
                            F3_464:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_50a
                            F3_470:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_499
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_483:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_483
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_48f:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_48f
                                sub edi, ebp
                                jmp F3_49b
                            F3_499:
                                add edi, ecx
                            F3_49b:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_514
                            F3_4a3:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_4cc
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_4b6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_4b6
                                add edi, ebp
                                sub edi, dword ptr [edx]
                                mov ecx, dword ptr [edx]
                            F3_4c2:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_4c2
                                sub edi, ebp
                                jmp F3_4ce
                            F3_4cc:
                                add edi, ecx
                            F3_4ce:
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne F3_3fa
                                lea edi, [edi+ebp]
                                jmp F3_62f
                            F3_4e6:
                                cmp byte ptr [esi], ah
                                jne F3_402
                            F3_4ee:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_4f4:
                                cmp byte ptr [esi], ah
                                jne F3_439
                            F3_4fc:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_502:
                                cmp byte ptr [esi], ah
                                jne F3_470
                            F3_50a:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_510:
                                cmp byte ptr [esi], ah
                                jne F3_4a3
                            F3_514:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne F3_4e6
                                lea edi, [edi+ebp]
                                jmp F3_62f
                            F3_52a:
                                mov ebx, dword ptr [SpriteRasterSourceColumnCount]
                                and ebx, 0x3
                                je F3_54d
                                dec ebx
                                je F3_53e
                                dec ebx
                                je F3_543
                                dec ebx
                                je F3_548
                            F3_53e:
                                jmp F3_61d
                            F3_543:
                                jmp F3_613
                            F3_548:
                                jmp F3_605
                            F3_54d:
                                cmp byte ptr [esi], ah
                                je F3_5ff
                            F3_555:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_570
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_568:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_568
                                jmp F3_572
                            F3_570:
                                add edi, ecx
                            F3_572:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_60d
                            F3_57e:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_599
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_591:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_591
                                jmp F3_59b
                            F3_599:
                                add edi, ecx
                            F3_59b:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_617
                            F3_5a3:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_5be
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_5b6:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_5b6
                                jmp F3_5c0
                            F3_5be:
                                add edi, ecx
                            F3_5c0:
                                inc esi
                                add edx, 0x4
                                cmp byte ptr [esi], ah
                                je F3_621
                            F3_5c8:
                                cmp byte ptr [esi], 0x0
                                mov ecx, dword ptr [edx]
                                je F3_5e3
                                xor ebx, ebx
                                mov bl, byte ptr [esi]
                                add ebx, dword ptr [SpriteRasterRemapTable]
                                mov al, byte ptr [ebx]
                            F3_5db:
                                mov byte ptr [edi], al
                                inc edi
                                dec ecx
                                jne F3_5db
                                jmp F3_5e5
                            F3_5e3:
                                add edi, ecx
                            F3_5e5:
                                inc esi
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne F3_54d
                                jmp F3_62f
                            F3_5f7:
                                cmp byte ptr [esi], ah
                                jne F3_555
                            F3_5ff:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_605:
                                cmp byte ptr [esi], ah
                                jne F3_57e
                            F3_60d:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_613:
                                cmp byte ptr [esi], ah
                                jne F3_5a3
                            F3_617:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                            F3_61d:
                                cmp byte ptr [esi], ah
                                jne F3_5c8
                            F3_621:
                                inc esi
                                add edi, dword ptr [edx]
                                add edx, 0x4
                                dec dword ptr [SpriteRasterColumnCountdown]
                                jne F3_5f7
                            F3_62f:
                                add esi, dword ptr [SpriteRasterSourceRowTail]
                                add edi, dword ptr [SpriteRasterDestinationRowTail]
                                dec dword ptr [SpriteRasterSourceRowCount]
                                jne F3_40
                                pop ebp
                                popf
                                pop esi
                                pop edi
            }
        }
    }
    destination->free_data(1);
    return 0;
}

// ORIGINAL: 0x005E3890 ?init@Sprite@@QAEHPADHH@Z 0x005E3890-0x005E3953 FILE BYTE_EXACT
// Re-target a live sprite at a new PCX file: release the previous pixel block
// (charging its size back to SpriteMemoryUsed) and file name, clear the
// geometry, copy the new name into heap storage, and store the requested
// width/height into both the source and the displayed dimension pairs.
int Sprite::init(char *filename, int width, int height) {
    if (filename == 0) {
        return 0x10;
    }
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
