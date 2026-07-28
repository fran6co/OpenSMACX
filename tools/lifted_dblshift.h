#ifndef OPENSMACX_LIFTED_DBLSHIFT_H
#define OPENSMACX_LIFTED_DBLSHIFT_H

// SHLD and SHRD - the double-precision shifts.
//
// These are the last substantial mnemonic gap in the lift: 129 instructions in
// terranx.exe - 100 SHRD and 29 SHLD, 58% of the 221 traps left - reduced to
// four distinct encodings, every one of them 32-bit with a register
// destination:
//
//    77x  0f ac d0 10   shrd eax, edx, 0x10
//    29x  0f a5 d0      shld eax, edx, cl
//    18x  0f ac d0 08   shrd eax, edx, 8
//     5x  0f ad c0      shrd eax, eax, cl
//
// The header covers the whole instruction pair anyway - both widths, both
// count forms - because the lowering routes any destination through
// `write_operand`, and a memory destination is then the same helper between a
// load and a store.
//
// A double shift is one shift of a value that is TWICE the operand width. SHLD
// shifts the destination left and fills the vacated low bits from the HIGH end
// of the source; SHRD shifts the destination right and fills the vacated high
// bits from the LOW end of the source. The source is never written. That
// asymmetry - left takes from the top of src, right takes from the bottom - is
// the single easiest thing here to get backwards, and getting it backwards
// still produces a plausible-looking number at every count.
//
// Five rules, each of which is a separate way to be wrong:
//
// **The count is masked to 5 bits FIRST**, at BOTH widths. A 16-bit SHRD by 32
// is a shift by zero, not a shift by 32 and not a shift by 16.
//
// **A count of zero is a whole no-operation.** The destination keeps its value
// and NOT ONE FLAG IS WRITTEN - not even the ones the count-1 case would
// define. Recomputing SF/ZF/PF "from the unchanged result" is the natural
// mistake and it silently destroys a flag set some earlier compare produced.
//
// **CF is the last bit shifted OUT of the destination** - bit (W - count) for
// SHLD, bit (count - 1) for SHRD, both read from the ORIGINAL destination,
// never from the result and never from the source.
//
// **OF is architecturally defined only for a count of 1**, where it means "the
// sign bit changed". It is computed here at every count, uniformly, so that
// two runs of the same lifted code agree; only the count-1 value is claimed to
// match hardware, and only that one is compared in the differential.
//
// **AF is architecturally undefined** and is cleared, the same choice
// `lifted_x86.h` makes for the logic operations.
//
// ---------------------------------------------------------------------------
// The undefined region, stated rather than papered over
// ---------------------------------------------------------------------------
//
// After the 5-bit mask a 32-bit double shift can only ask for 0..31, so it is
// always inside its own width. A 16-BIT one can ask for up to 31, and the SDM
// pseudo-code sends `count > SIZE` to "DEST is undefined; CF, OF, SF, ZF, AF,
// PF are undefined". That is counts 17..31 at width 16.
//
// This header's deterministic choice for that region is TO DO NOTHING: the
// destination keeps its value and no flag is written, exactly as for a count
// of zero. It is a choice, not a match, and the difference was MEASURED rather
// than assumed: over 180,004 cases in that region the machine under Wine
// disagreed with this header 174,512 times - it keeps double-shifting, so
// `shldw $17` on 1234/ABCD yields 579A where this yields 1234. The
// differential therefore EXCLUDES counts 17..31 at width 16 and prints that
// disagreement as a separate line instead of folding it into a pass. Nothing
// in this image reaches it: every SHLD/SHRD in terranx.exe is 32-bit, where
// the 5-bit mask cannot produce a count above the width at all.
//
// Count 16 at width 16 is a different case and is NOT undefined: `count >
// SIZE` is false, so the SDM pseudo-code defines it, and what it defines is
// "the destination becomes the source, CF is bit 0 (SHLD) or bit 15 (SHRD) of
// the old destination". The general expressions below produce exactly that
// with no special case, the differential compares it, and the machine agrees:
// `shldw`/`shrdw` by 16 on 1234/ABCD both give ABCD.

#include <cstdint>

#include "lifted_x86.h"

// ---------------------------------------------------------------------------
// The core
//
// Both directions in one body: they share the count masking, the no-operation
// cases and the whole flag word, and differ only in which way the destination
// moves and which end of the source refills it. Writing them twice would mean
// maintaining the count rules twice.
// ---------------------------------------------------------------------------

template <unsigned W, bool Left>
inline uint32_t opensmacx_dblshift_w(OpensmacxStaticRecompileState &s,
                                     uint32_t dest, uint32_t src,
                                     uint32_t count) {
    constexpr uint32_t mask = OpensmacxWidth<W>::kMask;
    constexpr uint32_t sign = OpensmacxWidth<W>::kSign;
    dest &= mask;
    src &= mask;

    // Five bits, at both widths, before anything else looks at the count.
    const uint32_t c = count & 31U;

    // Count 0: no result, no flags. Both of those matter.
    if (c == 0U) {
        return dest;
    }
    // The undefined region - width 16 only. Documented above; deliberately
    // indistinguishable from count 0 here rather than invented.
    if (c > W) {
        return dest;
    }

    // c is now in 1..W, so `W - c` is in 0..W-1 and every shift below is in
    // range for the 32-bit intermediate. `dest << c` can push bits past bit 31
    // only at W == 32, where c <= 31 keeps it defined; the mask discards them.
    uint32_t result;
    uint32_t carry;
    if (Left) {
        // SHLD: the low bits come from the TOP of src.
        result = ((dest << c) | (src >> (W - c))) & mask;
        // The last bit to leave the top of dest.
        carry = (dest >> (W - c)) & 1U;
    } else {
        // SHRD: the high bits come from the BOTTOM of src.
        result = ((dest >> c) | (src << (W - c))) & mask;
        // The last bit to leave the bottom of dest.
        carry = (dest >> (c - 1U)) & 1U;
    }

    s.eflags = (s.eflags & ~OpensmacxAluFlags)
             | opensmacx_result_flags<W>(result)
             | (carry ? uint32_t(OpensmacxCarryFlag) : 0U)
             // "The sign bit changed", against the ORIGINAL destination.
             // Defined by the architecture for c == 1 only.
             | (((result ^ dest) & sign) ? uint32_t(OpensmacxOverflowFlag) : 0U);
    return result;
}

// ---------------------------------------------------------------------------
// The names the lowering emits
//
// There is no 8-bit SHLD or SHRD on x86 - the encoding has no byte form - so
// this deliberately does not mirror the 8/16/32 macro in `lifted_x86.h`. A
// generated `opensmacx_shld8` would be a helper for an instruction that does
// not exist, and would turn a mis-sized operand into a silent lowering instead
// of a compile error.
//
// Argument order is (destination, source, count), matching both the Intel
// operand order and the order capstone reports.
// ---------------------------------------------------------------------------

#define OPENSMACX_X86_DOUBLE_SHIFT(NAME, LEFT)                                \
    inline uint32_t opensmacx_##NAME##16(OpensmacxStaticRecompileState &s,    \
                                         uint32_t dest, uint32_t src,         \
                                         uint32_t count) {                    \
        return opensmacx_dblshift_w<16, LEFT>(s, dest, src, count);           \
    }                                                                         \
    inline uint32_t opensmacx_##NAME##32(OpensmacxStaticRecompileState &s,    \
                                         uint32_t dest, uint32_t src,         \
                                         uint32_t count) {                    \
        return opensmacx_dblshift_w<32, LEFT>(s, dest, src, count);           \
    }

OPENSMACX_X86_DOUBLE_SHIFT(shld, true)
OPENSMACX_X86_DOUBLE_SHIFT(shrd, false)

#undef OPENSMACX_X86_DOUBLE_SHIFT

#endif
