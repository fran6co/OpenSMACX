#ifndef OPENSMACX_LIFTED_MULDIV_H
#define OPENSMACX_LIFTED_MULDIV_H

// One-operand MUL, DIV and IDIV for the whole-image static recompilation.
//
// This header is the companion to `lifted_x86.h` and deliberately does NOT
// include it: the signed one-operand multiply already lives there as
// `opensmacx_imul1_8/16/32`, and duplicating it - or reaching into its
// templates - would make the include order load-bearing. Everything below is
// self-contained on top of `lifted_runtime.h`, so the integrator's patch is a
// single `#include` that can sit anywhere.
//
// Three things distinguish these instructions from the rest of the ALU, and
// each one is a decision recorded here rather than a detail left implicit.
//
// **The register pair is part of the instruction, not the operand.** The
// lowering passes only the explicit source; AX / DX:AX / EDX:EAX are read and
// written here. At width 8 the destination is AX as a whole (AL and AH), so
// the upper 16 bits of EAX survive; at width 16 both AX and DX are 16-bit
// merges; at width 32 EAX and EDX are replaced outright.
//
// **A bad divide is a fault, not a value.** DIV and IDIV raise #DE when the
// divisor is zero AND when the quotient does not fit the destination. Both go
// through `opensmacx_trap`, which names the instruction's own address. The
// alternative - letting C++ divide by zero, or evaluating INT64_MIN / -1 - is
// undefined behaviour whose symptom would surface arbitrarily far from the
// instruction that caused it, which at phase 5 is the difference between a
// one-line fix and a week.
//
// **Undefined flags are left unchanged.** DIV and IDIV leave CF, OF, SF, ZF,
// AF and PF architecturally undefined; MUL defines only CF and OF. Where the
// architecture says nothing, this header writes nothing: the divides touch no
// flag at all, and MUL touches only CF and OF. That is deterministic, it is
// the same policy `opensmacx_imul_wide_flags` already applies to the signed
// multiply in `lifted_x86.h`, and it means a lifted body that reads a flag
// after a divide reads whatever the previous flag-setting instruction left -
// which is at least reproducible, unlike the hardware it models.

#include <cstdint>

#include "lifted_runtime.h"

// ---------------------------------------------------------------------------
// Unsigned multiply
//
// CF and OF both mean "the high half is not zero", i.e. the full product does
// not fit in the low half. For the unsigned form that is literally a nonzero
// high half; the signed form in lifted_x86.h needs a sign-extension test
// instead, which is why the two cannot share a predicate.
// ---------------------------------------------------------------------------

inline void opensmacx_muldiv_carry_flags(OpensmacxStaticRecompileState &s,
                                         bool overflowed) {
    s.eflags = (s.eflags & ~(uint32_t(OpensmacxCarryFlag)
                             | uint32_t(OpensmacxOverflowFlag)))
             | (overflowed ? uint32_t(OpensmacxCarryFlag)
                             | uint32_t(OpensmacxOverflowFlag)
                           : 0U);
}

// AL * src -> AX.
inline void opensmacx_mul1_8(OpensmacxStaticRecompileState &s, uint32_t v) {
    const uint32_t product = (s.eax & 0xFFU) * (v & 0xFFU);
    s.eax = (s.eax & 0xFFFF0000U) | (product & 0xFFFFU);
    opensmacx_muldiv_carry_flags(s, (product & 0xFF00U) != 0U);
}

// AX * src -> DX:AX.
inline void opensmacx_mul1_16(OpensmacxStaticRecompileState &s, uint32_t v) {
    const uint32_t product = (s.eax & 0xFFFFU) * (v & 0xFFFFU);
    s.eax = (s.eax & 0xFFFF0000U) | (product & 0xFFFFU);
    s.edx = (s.edx & 0xFFFF0000U) | ((product >> 16) & 0xFFFFU);
    opensmacx_muldiv_carry_flags(s, (product >> 16) != 0U);
}

// EAX * src -> EDX:EAX.
inline void opensmacx_mul1_32(OpensmacxStaticRecompileState &s, uint32_t v) {
    const uint64_t product = static_cast<uint64_t>(s.eax) * static_cast<uint64_t>(v);
    s.eax = static_cast<uint32_t>(product);
    s.edx = static_cast<uint32_t>(product >> 32);
    opensmacx_muldiv_carry_flags(s, (product >> 32) != 0U);
}

// ---------------------------------------------------------------------------
// Divide
//
// Two cores, one unsigned and one signed, because the overflow condition is
// genuinely different: unsigned asks whether the quotient exceeds an upper
// bound, signed asks whether it left a two-sided range. Neither core touches
// flags or registers - the wrappers own the register pair, the cores own the
// arithmetic and the faults.
// ---------------------------------------------------------------------------

// A 2W-bit dividend over a W-bit divisor, quotient required to fit in W bits.
// `limit` is that W-bit maximum, passed in rather than templated so there is
// one body and one place for the fault to be raised.
inline void opensmacx_div_core(uint64_t dividend, uint32_t divisor,
                               uint64_t limit, uint32_t address,
                               uint32_t &quotient, uint32_t &remainder) {
    if (divisor == 0U) {
        opensmacx_trap(address, "divide by zero");
    }
    const uint64_t whole = dividend / divisor;
    if (whole > limit) {
        // #DE for "quotient too large", the same vector as divide by zero on
        // hardware but a different cause, so it gets its own message.
        opensmacx_trap(address, "divide overflow: quotient does not fit");
    }
    quotient = static_cast<uint32_t>(whole);
    remainder = static_cast<uint32_t>(dividend % divisor);
}

// The signed core. `low`/`high` bound the destination register's range.
//
// C++ integer division truncates toward zero (that is [expr.mul]/4, not a
// platform habit), and so does IDIV: the remainder takes the sign of the
// DIVIDEND. -7 / 2 is -3 remainder -1 in both, where a floor division would
// give -4 remainder 1. That correspondence is what lets `/` and `%` model the
// instruction directly, and the differential test beside the real IDIV is
// what confirms it rather than assuming it.
inline void opensmacx_idiv_core(int64_t dividend, int32_t divisor,
                                int64_t low, int64_t high, uint32_t address,
                                int64_t &quotient, int64_t &remainder) {
    if (divisor == 0) {
        opensmacx_trap(address, "divide by zero");
    }
    // INT64_MIN / -1 is the one division C++ leaves undefined, and it is also
    // a #DE on hardware. The check has to happen BEFORE the divide, because
    // afterwards there is nothing left to check: the program has already
    // executed undefined behaviour. At width 32 this is reachable - a dividend
    // of 0x8000000000000000 in EDX:EAX with a divisor of -1 - so it is not a
    // theoretical guard.
    if (divisor == -1 && dividend == INT64_MIN) {
        opensmacx_trap(address, "divide overflow: quotient does not fit");
    }
    const int64_t whole = dividend / divisor;
    if (whole < low || whole > high) {
        // The famous INT_MIN / -1 case arrives here at widths 8 and 16, and
        // so does every ordinary "quotient too wide" divide.
        opensmacx_trap(address, "divide overflow: quotient does not fit");
    }
    quotient = whole;
    remainder = dividend % divisor;
}

// AX / src -> AL quotient, AH remainder.
inline void opensmacx_div1_8(OpensmacxStaticRecompileState &s, uint32_t v,
                             uint32_t address) {
    uint32_t quotient = 0, remainder = 0;
    opensmacx_div_core(s.eax & 0xFFFFU, v & 0xFFU, 0xFFU, address,
                       quotient, remainder);
    s.eax = (s.eax & 0xFFFF0000U) | (quotient & 0xFFU) | ((remainder & 0xFFU) << 8);
}

// DX:AX / src -> AX quotient, DX remainder.
inline void opensmacx_div1_16(OpensmacxStaticRecompileState &s, uint32_t v,
                              uint32_t address) {
    const uint32_t dividend = ((s.edx & 0xFFFFU) << 16) | (s.eax & 0xFFFFU);
    uint32_t quotient = 0, remainder = 0;
    opensmacx_div_core(dividend, v & 0xFFFFU, 0xFFFFU, address,
                       quotient, remainder);
    s.eax = (s.eax & 0xFFFF0000U) | (quotient & 0xFFFFU);
    s.edx = (s.edx & 0xFFFF0000U) | (remainder & 0xFFFFU);
}

// EDX:EAX / src -> EAX quotient, EDX remainder.
inline void opensmacx_div1_32(OpensmacxStaticRecompileState &s, uint32_t v,
                              uint32_t address) {
    const uint64_t dividend =
        (static_cast<uint64_t>(s.edx) << 32) | static_cast<uint64_t>(s.eax);
    uint32_t quotient = 0, remainder = 0;
    opensmacx_div_core(dividend, v, 0xFFFFFFFFU, address, quotient, remainder);
    s.eax = quotient;
    s.edx = remainder;
}

// AX / src -> AL quotient, AH remainder, both signed.
inline void opensmacx_idiv1_8(OpensmacxStaticRecompileState &s, uint32_t v,
                              uint32_t address) {
    const int64_t dividend = static_cast<int16_t>(s.eax & 0xFFFFU);
    const int32_t divisor = static_cast<int8_t>(v & 0xFFU);
    int64_t quotient = 0, remainder = 0;
    opensmacx_idiv_core(dividend, divisor, -128, 127, address,
                        quotient, remainder);
    s.eax = (s.eax & 0xFFFF0000U)
          | (static_cast<uint32_t>(quotient) & 0xFFU)
          | ((static_cast<uint32_t>(remainder) & 0xFFU) << 8);
}

// DX:AX / src -> AX quotient, DX remainder, both signed.
inline void opensmacx_idiv1_16(OpensmacxStaticRecompileState &s, uint32_t v,
                               uint32_t address) {
    const uint32_t packed = ((s.edx & 0xFFFFU) << 16) | (s.eax & 0xFFFFU);
    const int64_t dividend = static_cast<int32_t>(packed);
    const int32_t divisor = static_cast<int16_t>(v & 0xFFFFU);
    int64_t quotient = 0, remainder = 0;
    opensmacx_idiv_core(dividend, divisor, -32768, 32767, address,
                        quotient, remainder);
    s.eax = (s.eax & 0xFFFF0000U) | (static_cast<uint32_t>(quotient) & 0xFFFFU);
    s.edx = (s.edx & 0xFFFF0000U) | (static_cast<uint32_t>(remainder) & 0xFFFFU);
}

// EDX:EAX / src -> EAX quotient, EDX remainder, both signed. This is the form
// the image actually uses: 932 of the 970 refusals this header closes are
// 32-bit IDIV.
inline void opensmacx_idiv1_32(OpensmacxStaticRecompileState &s, uint32_t v,
                               uint32_t address) {
    const int64_t dividend = static_cast<int64_t>(
        (static_cast<uint64_t>(s.edx) << 32) | static_cast<uint64_t>(s.eax));
    const int32_t divisor = static_cast<int32_t>(v);
    int64_t quotient = 0, remainder = 0;
    opensmacx_idiv_core(dividend, divisor, INT32_MIN, INT32_MAX, address,
                        quotient, remainder);
    s.eax = static_cast<uint32_t>(quotient);
    s.edx = static_cast<uint32_t>(remainder);
}

#endif
