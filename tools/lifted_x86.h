#ifndef OPENSMACX_LIFTED_X86_H
#define OPENSMACX_LIFTED_X86_H

// x86 semantics runtime for the whole-image static recompilation.
//
// `tools/x86_lower.py` is the contract: every name here exists because the
// lowering emits a call to it. Nothing here is generic for its own sake - the
// operand model already handled addressing modes, so what remains is
// operations over VALUES, and there are about thirty of them.
//
// Three conventions hold everywhere below, and every subtle bug in an emulated
// ALU is a violation of one of them.
//
// **Every value is a uint32_t, every operation has a width.** An 8- or 16-bit
// operation masks its result to its own width before returning, so the caller's
// sub-register merge writes the same bits the hardware would.
//
// **Flags come from the width's own sign bit and carry position**, never from
// bit 31. `add8(0x7F, 1)` overflows; the same numbers at width 32 do not.
//
// **Flags are computed from the inputs, not reconstructed from the result.**
// Overflow needs both operands' signs and the two expressions - add and sub -
// are genuinely different, not one with a negated argument.

#include <cstdint>
#include <cstring>

#include "lifted_runtime.h"

// ---------------------------------------------------------------------------
// Memory
//
// `opensmacx_mem32` / `opensmacx_store32` already live in lifted_runtime.h; the
// narrow widths complete the set. memcpy because a lifted access is only as
// aligned as the original program made it, and the host is not obliged to
// tolerate what x86 tolerated.
// ---------------------------------------------------------------------------

inline uint32_t opensmacx_mem8(uint32_t address) {
    uint8_t value;
    __builtin_memcpy(&value, opensmacx_at(address), sizeof(value));
    return value;
}

inline uint32_t opensmacx_mem16(uint32_t address) {
    uint16_t value;
    __builtin_memcpy(&value, opensmacx_at(address), sizeof(value));
    return value;
}

inline void opensmacx_store8(uint32_t address, uint32_t value) {
    const uint8_t narrow = static_cast<uint8_t>(value);
    __builtin_memcpy(opensmacx_at(address), &narrow, sizeof(narrow));
}

inline void opensmacx_store16(uint32_t address, uint32_t value) {
    const uint16_t narrow = static_cast<uint16_t>(value);
    __builtin_memcpy(opensmacx_at(address), &narrow, sizeof(narrow));
}

// ---------------------------------------------------------------------------
// Flags
// ---------------------------------------------------------------------------

inline bool opensmacx_cf(const OpensmacxStaticRecompileState &s) {
    return (s.eflags & OpensmacxCarryFlag) != 0;
}
inline bool opensmacx_pf(const OpensmacxStaticRecompileState &s) {
    return (s.eflags & OpensmacxParityFlag) != 0;
}
inline bool opensmacx_af(const OpensmacxStaticRecompileState &s) {
    return (s.eflags & OpensmacxAuxiliaryCarryFlag) != 0;
}
inline bool opensmacx_zf(const OpensmacxStaticRecompileState &s) {
    return (s.eflags & OpensmacxZeroFlag) != 0;
}
inline bool opensmacx_sf(const OpensmacxStaticRecompileState &s) {
    return (s.eflags & OpensmacxSignFlag) != 0;
}
inline bool opensmacx_of(const OpensmacxStaticRecompileState &s) {
    return (s.eflags & OpensmacxOverflowFlag) != 0;
}

// The six flags an ALU operation defines. INC and DEC deliberately write the
// second set: preserving CF is the whole reason they are not `add 1`/`sub 1`.
constexpr uint32_t OpensmacxAluFlags =
    OpensmacxCarryFlag | OpensmacxParityFlag | OpensmacxAuxiliaryCarryFlag |
    OpensmacxZeroFlag | OpensmacxSignFlag | OpensmacxOverflowFlag;
constexpr uint32_t OpensmacxAluFlagsKeepingCarry =
    OpensmacxAluFlags & ~static_cast<uint32_t>(OpensmacxCarryFlag);

template <unsigned W>
struct OpensmacxWidth {
    // `~0 >> (32 - W)` rather than `(1 << W) - 1`: the latter is undefined at
    // W == 32, which is the width that matters most.
    static constexpr uint32_t kMask = 0xFFFFFFFFU >> (32U - W);
    static constexpr uint32_t kSign = 1U << (W - 1U);
};

// Sign-extend a width-W value to a full signed 32-bit one.
template <unsigned W>
inline int32_t opensmacx_sext(uint32_t value) {
    return static_cast<int32_t>(value << (32U - W)) >> (32U - W);
}

// PF is the parity of the LOW 8 BITS of the result - at every width. The wider
// bits never participate.
inline uint32_t opensmacx_parity_flag(uint32_t result) {
    return __builtin_parity(result & 0xFFU) ? 0U : uint32_t(OpensmacxParityFlag);
}

// SF, ZF and PF, which every flag-writing operation except NOT and the rotates
// derives the same way.
template <unsigned W>
inline uint32_t opensmacx_result_flags(uint32_t result) {
    return (result == 0U ? uint32_t(OpensmacxZeroFlag) : 0U)
         | ((result & OpensmacxWidth<W>::kSign) ? uint32_t(OpensmacxSignFlag) : 0U)
         | opensmacx_parity_flag(result);
}

// ---------------------------------------------------------------------------
// Addition and subtraction
//
// ADD/ADC and SUB/SBB/CMP/NEG are each one core with a carry-in, which is what
// makes ADC and SBB correct rather than nearly correct: the incoming CF enters
// both the result and the carry-out.
// ---------------------------------------------------------------------------

template <unsigned W>
inline uint32_t opensmacx_add_core(OpensmacxStaticRecompileState &s,
                                   uint32_t a, uint32_t b, uint32_t carry_in) {
    constexpr uint32_t mask = OpensmacxWidth<W>::kMask;
    constexpr uint32_t sign = OpensmacxWidth<W>::kSign;
    a &= mask;
    b &= mask;
    const uint64_t wide = static_cast<uint64_t>(a) + b + carry_in;
    const uint32_t result = static_cast<uint32_t>(wide) & mask;
    s.eflags = (s.eflags & ~OpensmacxAluFlags)
             | opensmacx_result_flags<W>(result)
             | ((wide >> W) & 1U ? uint32_t(OpensmacxCarryFlag) : 0U)
             | ((a ^ b ^ result) & 0x10U ? uint32_t(OpensmacxAuxiliaryCarryFlag) : 0U)
             // Overflow on an add: the operands agreed in sign and the result
             // disagrees with them.
             | (~(a ^ b) & (a ^ result) & sign ? uint32_t(OpensmacxOverflowFlag) : 0U);
    return result;
}

template <unsigned W>
inline uint32_t opensmacx_sub_core(OpensmacxStaticRecompileState &s,
                                   uint32_t a, uint32_t b, uint32_t borrow_in) {
    constexpr uint32_t mask = OpensmacxWidth<W>::kMask;
    constexpr uint32_t sign = OpensmacxWidth<W>::kSign;
    a &= mask;
    b &= mask;
    const uint32_t result = (a - b - borrow_in) & mask;
    s.eflags = (s.eflags & ~OpensmacxAluFlags)
             | opensmacx_result_flags<W>(result)
             // Borrow, widened so `b + 1` cannot wrap back to zero.
             | (static_cast<uint64_t>(a) < static_cast<uint64_t>(b) + borrow_in
                    ? uint32_t(OpensmacxCarryFlag) : 0U)
             | ((a ^ b ^ result) & 0x10U ? uint32_t(OpensmacxAuxiliaryCarryFlag) : 0U)
             // Overflow on a subtract: the operands DISAGREED in sign and the
             // result disagrees with the minuend. Not the add expression.
             | ((a ^ b) & (a ^ result) & sign ? uint32_t(OpensmacxOverflowFlag) : 0U);
    return result;
}

template <unsigned W>
inline uint32_t opensmacx_add_w(OpensmacxStaticRecompileState &s,
                                uint32_t a, uint32_t b) {
    return opensmacx_add_core<W>(s, a, b, 0U);
}

template <unsigned W>
inline uint32_t opensmacx_adc_w(OpensmacxStaticRecompileState &s,
                                uint32_t a, uint32_t b) {
    return opensmacx_add_core<W>(s, a, b, opensmacx_cf(s) ? 1U : 0U);
}

template <unsigned W>
inline uint32_t opensmacx_sub_w(OpensmacxStaticRecompileState &s,
                                uint32_t a, uint32_t b) {
    return opensmacx_sub_core<W>(s, a, b, 0U);
}

template <unsigned W>
inline uint32_t opensmacx_sbb_w(OpensmacxStaticRecompileState &s,
                                uint32_t a, uint32_t b) {
    return opensmacx_sub_core<W>(s, a, b, opensmacx_cf(s) ? 1U : 0U);
}

// ---------------------------------------------------------------------------
// Logic
//
// AND/OR/XOR/TEST clear CF and OF outright. AF is architecturally undefined;
// clearing it is a choice, made here so that two runs of the same lifted code
// always agree.
// ---------------------------------------------------------------------------

template <unsigned W>
inline uint32_t opensmacx_logic_result(OpensmacxStaticRecompileState &s,
                                       uint32_t result) {
    result &= OpensmacxWidth<W>::kMask;
    s.eflags = (s.eflags & ~OpensmacxAluFlags) | opensmacx_result_flags<W>(result);
    return result;
}

template <unsigned W>
inline uint32_t opensmacx_and_w(OpensmacxStaticRecompileState &s,
                                uint32_t a, uint32_t b) {
    return opensmacx_logic_result<W>(s, a & b);
}

template <unsigned W>
inline uint32_t opensmacx_or_w(OpensmacxStaticRecompileState &s,
                               uint32_t a, uint32_t b) {
    return opensmacx_logic_result<W>(s, a | b);
}

template <unsigned W>
inline uint32_t opensmacx_xor_w(OpensmacxStaticRecompileState &s,
                                uint32_t a, uint32_t b) {
    return opensmacx_logic_result<W>(s, a ^ b);
}

// ---------------------------------------------------------------------------
// Unary
// ---------------------------------------------------------------------------

// INC is `add 1` in every respect EXCEPT that it leaves CF alone. A loop that
// increments a counter inside a carry-propagating chain depends on this, and
// the damage from getting it wrong surfaces at some later branch rather than
// here.
template <unsigned W>
inline uint32_t opensmacx_inc_w(OpensmacxStaticRecompileState &s, uint32_t v) {
    constexpr uint32_t mask = OpensmacxWidth<W>::kMask;
    constexpr uint32_t sign = OpensmacxWidth<W>::kSign;
    v &= mask;
    const uint32_t result = (v + 1U) & mask;
    s.eflags = (s.eflags & ~OpensmacxAluFlagsKeepingCarry)
             | opensmacx_result_flags<W>(result)
             | ((v ^ 1U ^ result) & 0x10U ? uint32_t(OpensmacxAuxiliaryCarryFlag) : 0U)
             | (~(v ^ 1U) & (v ^ result) & sign ? uint32_t(OpensmacxOverflowFlag) : 0U);
    return result;
}

template <unsigned W>
inline uint32_t opensmacx_dec_w(OpensmacxStaticRecompileState &s, uint32_t v) {
    constexpr uint32_t mask = OpensmacxWidth<W>::kMask;
    constexpr uint32_t sign = OpensmacxWidth<W>::kSign;
    v &= mask;
    const uint32_t result = (v - 1U) & mask;
    s.eflags = (s.eflags & ~OpensmacxAluFlagsKeepingCarry)
             | opensmacx_result_flags<W>(result)
             | ((v ^ 1U ^ result) & 0x10U ? uint32_t(OpensmacxAuxiliaryCarryFlag) : 0U)
             | ((v ^ 1U) & (v ^ result) & sign ? uint32_t(OpensmacxOverflowFlag) : 0U);
    return result;
}

// NEG is exactly `sub` from zero, flags included: CF becomes "the operand was
// non-zero" and OF fires only on the one value that has no negation.
template <unsigned W>
inline uint32_t opensmacx_neg_w(OpensmacxStaticRecompileState &s, uint32_t v) {
    return opensmacx_sub_core<W>(s, 0U, v, 0U);
}

// NOT affects NO flags. Not "sets them from the result" - none.
template <unsigned W>
inline uint32_t opensmacx_not_w(OpensmacxStaticRecompileState &, uint32_t v) {
    return ~v & OpensmacxWidth<W>::kMask;
}

// ---------------------------------------------------------------------------
// Shifts and rotates
//
// The count is masked to 5 bits FIRST, at every width - so an 8-bit shift by 32
// is a shift by zero, and a shift by zero touches no flag at all. That early
// return is not an optimisation; it is the semantics.
// ---------------------------------------------------------------------------

template <unsigned W>
inline uint32_t opensmacx_shl_w(OpensmacxStaticRecompileState &s,
                                uint32_t v, uint32_t count) {
    constexpr uint32_t mask = OpensmacxWidth<W>::kMask;
    constexpr uint32_t sign = OpensmacxWidth<W>::kSign;
    v &= mask;
    const uint32_t c = count & 31U;
    if (c == 0U) {
        return v;
    }
    const uint32_t result = (v << c) & mask;
    // CF is the last bit shifted out of the top. Past the operand width there
    // is nothing left to shift out.
    const uint32_t carry = (c <= W) ? ((v >> ((W - c) & 31U)) & 1U) : 0U;
    s.eflags = (s.eflags & ~OpensmacxAluFlags)
             | opensmacx_result_flags<W>(result)
             | (carry ? uint32_t(OpensmacxCarryFlag) : 0U)
             // Architecturally defined only for count 1, where it is
             // "the sign changed"; computed uniformly for determinism.
             | (((result & sign) != 0U) != (carry != 0U)
                    ? uint32_t(OpensmacxOverflowFlag) : 0U);
    return result;
}

template <unsigned W>
inline uint32_t opensmacx_shr_w(OpensmacxStaticRecompileState &s,
                                uint32_t v, uint32_t count) {
    constexpr uint32_t mask = OpensmacxWidth<W>::kMask;
    constexpr uint32_t sign = OpensmacxWidth<W>::kSign;
    v &= mask;
    const uint32_t c = count & 31U;
    if (c == 0U) {
        return v;
    }
    const uint32_t result = v >> c;
    const uint32_t carry = (v >> (c - 1U)) & 1U;
    s.eflags = (s.eflags & ~OpensmacxAluFlags)
             | opensmacx_result_flags<W>(result)
             | (carry ? uint32_t(OpensmacxCarryFlag) : 0U)
             // For SHR, OF is the sign of the ORIGINAL operand.
             | ((v & sign) ? uint32_t(OpensmacxOverflowFlag) : 0U);
    return result;
}

template <unsigned W>
inline uint32_t opensmacx_sar_w(OpensmacxStaticRecompileState &s,
                                uint32_t v, uint32_t count) {
    constexpr uint32_t mask = OpensmacxWidth<W>::kMask;
    const uint32_t c = count & 31U;
    const int32_t signed_value = opensmacx_sext<W>(v);
    if (c == 0U) {
        return v & mask;
    }
    const uint32_t result = static_cast<uint32_t>(signed_value >> c) & mask;
    const uint32_t carry =
        static_cast<uint32_t>(signed_value >> (c - 1U)) & 1U;
    s.eflags = (s.eflags & ~OpensmacxAluFlags)
             | opensmacx_result_flags<W>(result)
             | (carry ? uint32_t(OpensmacxCarryFlag) : 0U);
    // OF is cleared by SAR: an arithmetic right shift cannot change the sign.
    return result;
}

// Rotates touch CF and OF only - never SF, ZF, PF or AF. After the 5-bit mask
// the count is taken modulo the width, but a count that masks to a non-zero
// multiple of the width still writes CF, which is why the modulo happens after
// the zero test rather than instead of it.
template <unsigned W>
inline uint32_t opensmacx_rol_w(OpensmacxStaticRecompileState &s,
                                uint32_t v, uint32_t count) {
    constexpr uint32_t mask = OpensmacxWidth<W>::kMask;
    constexpr uint32_t sign = OpensmacxWidth<W>::kSign;
    v &= mask;
    const uint32_t masked = count & 31U;
    if (masked == 0U) {
        return v;
    }
    const uint32_t c = masked % W;
    const uint32_t result = c ? (((v << c) | (v >> (W - c))) & mask) : v;
    const uint32_t carry = result & 1U;
    s.eflags = (s.eflags & ~(uint32_t(OpensmacxCarryFlag) | uint32_t(OpensmacxOverflowFlag)))
             | (carry ? uint32_t(OpensmacxCarryFlag) : 0U)
             | (((result & sign) != 0U) != (carry != 0U)
                    ? uint32_t(OpensmacxOverflowFlag) : 0U);
    return result;
}

template <unsigned W>
inline uint32_t opensmacx_ror_w(OpensmacxStaticRecompileState &s,
                                uint32_t v, uint32_t count) {
    constexpr uint32_t mask = OpensmacxWidth<W>::kMask;
    constexpr uint32_t sign = OpensmacxWidth<W>::kSign;
    v &= mask;
    const uint32_t masked = count & 31U;
    if (masked == 0U) {
        return v;
    }
    const uint32_t c = masked % W;
    const uint32_t result = c ? (((v >> c) | (v << (W - c))) & mask) : v;
    const uint32_t carry = (result & sign) != 0U ? 1U : 0U;
    s.eflags = (s.eflags & ~(uint32_t(OpensmacxCarryFlag) | uint32_t(OpensmacxOverflowFlag)))
             | (carry ? uint32_t(OpensmacxCarryFlag) : 0U)
             // The top two bits of the result disagree.
             | (((result >> (W - 1U)) ^ (result >> (W - 2U))) & 1U
                    ? uint32_t(OpensmacxOverflowFlag) : 0U);
    return result;
}

// ---------------------------------------------------------------------------
// Signed multiply
//
// CF and OF mean the same thing in every form: the full product did not fit in
// the destination. SF/ZF/PF/AF are architecturally undefined; they are written
// from the truncated result so that the same lifted code always behaves the
// same way.
// ---------------------------------------------------------------------------

template <unsigned W>
inline uint32_t opensmacx_imul_w(OpensmacxStaticRecompileState &s,
                                 uint32_t a, uint32_t b) {
    const int64_t product = static_cast<int64_t>(opensmacx_sext<W>(a))
                          * static_cast<int64_t>(opensmacx_sext<W>(b));
    const uint32_t result = static_cast<uint32_t>(product) & OpensmacxWidth<W>::kMask;
    const bool truncated = product != static_cast<int64_t>(opensmacx_sext<W>(result));
    s.eflags = (s.eflags & ~OpensmacxAluFlags)
             | opensmacx_result_flags<W>(result)
             | (truncated ? uint32_t(OpensmacxCarryFlag) | uint32_t(OpensmacxOverflowFlag)
                          : 0U);
    return result;
}

inline void opensmacx_imul_wide_flags(OpensmacxStaticRecompileState &s,
                                      bool truncated) {
    s.eflags = (s.eflags & ~(uint32_t(OpensmacxCarryFlag) | uint32_t(OpensmacxOverflowFlag)))
             | (truncated ? uint32_t(OpensmacxCarryFlag) | uint32_t(OpensmacxOverflowFlag)
                          : 0U);
}

// AL * src -> AX. The high half is a result, not a discard, so the write is a
// 16-bit merge into EAX.
inline void opensmacx_imul1_8(OpensmacxStaticRecompileState &s, uint32_t v) {
    const int32_t product = opensmacx_sext<8>(s.eax) * opensmacx_sext<8>(v);
    const uint32_t wide = static_cast<uint32_t>(product) & 0xFFFFU;
    s.eax = (s.eax & 0xFFFF0000U) | wide;
    opensmacx_imul_wide_flags(s, opensmacx_sext<8>(wide) != product);
}

// AX * src -> DX:AX.
inline void opensmacx_imul1_16(OpensmacxStaticRecompileState &s, uint32_t v) {
    const int32_t product = opensmacx_sext<16>(s.eax) * opensmacx_sext<16>(v);
    const uint32_t wide = static_cast<uint32_t>(product);
    s.eax = (s.eax & 0xFFFF0000U) | (wide & 0xFFFFU);
    s.edx = (s.edx & 0xFFFF0000U) | ((wide >> 16) & 0xFFFFU);
    opensmacx_imul_wide_flags(s, opensmacx_sext<16>(wide) != product);
}

// EAX * src -> EDX:EAX. A 32-bit destination is a whole-register replacement.
inline void opensmacx_imul1_32(OpensmacxStaticRecompileState &s, uint32_t v) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(s.eax))
                          * static_cast<int64_t>(static_cast<int32_t>(v));
    const uint32_t low = static_cast<uint32_t>(product);
    s.eax = low;
    s.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    opensmacx_imul_wide_flags(
        s, static_cast<int64_t>(static_cast<int32_t>(low)) != product);
}

// ---------------------------------------------------------------------------
// The names the lowering actually emits
//
// `opensmacx_add32`, not `opensmacx_add<32>`: the generator pastes the width
// onto the mnemonic, so the width-suffixed name is the contract and the
// template is an implementation detail behind it.
// ---------------------------------------------------------------------------

#define OPENSMACX_X86_BINARY(NAME)                                            \
    inline uint32_t opensmacx_##NAME##8(OpensmacxStaticRecompileState &s,     \
                                        uint32_t a, uint32_t b) {             \
        return opensmacx_##NAME##_w<8>(s, a, b);                              \
    }                                                                         \
    inline uint32_t opensmacx_##NAME##16(OpensmacxStaticRecompileState &s,    \
                                         uint32_t a, uint32_t b) {            \
        return opensmacx_##NAME##_w<16>(s, a, b);                             \
    }                                                                         \
    inline uint32_t opensmacx_##NAME##32(OpensmacxStaticRecompileState &s,    \
                                         uint32_t a, uint32_t b) {            \
        return opensmacx_##NAME##_w<32>(s, a, b);                             \
    }

#define OPENSMACX_X86_UNARY(NAME)                                             \
    inline uint32_t opensmacx_##NAME##8(OpensmacxStaticRecompileState &s,     \
                                        uint32_t v) {                         \
        return opensmacx_##NAME##_w<8>(s, v);                                 \
    }                                                                         \
    inline uint32_t opensmacx_##NAME##16(OpensmacxStaticRecompileState &s,    \
                                         uint32_t v) {                        \
        return opensmacx_##NAME##_w<16>(s, v);                                \
    }                                                                         \
    inline uint32_t opensmacx_##NAME##32(OpensmacxStaticRecompileState &s,    \
                                         uint32_t v) {                        \
        return opensmacx_##NAME##_w<32>(s, v);                                \
    }

OPENSMACX_X86_BINARY(add)
OPENSMACX_X86_BINARY(adc)
OPENSMACX_X86_BINARY(sub)
OPENSMACX_X86_BINARY(sbb)
OPENSMACX_X86_BINARY(and)
OPENSMACX_X86_BINARY(or)
OPENSMACX_X86_BINARY(xor)
OPENSMACX_X86_BINARY(imul)

// Shifts share the binary shape - value, count - but the second argument is a
// count, not an operand, so they get their own pass rather than sharing a name.
OPENSMACX_X86_BINARY(shl)
OPENSMACX_X86_BINARY(shr)
OPENSMACX_X86_BINARY(sar)
OPENSMACX_X86_BINARY(rol)
OPENSMACX_X86_BINARY(ror)

OPENSMACX_X86_UNARY(inc)
OPENSMACX_X86_UNARY(dec)
OPENSMACX_X86_UNARY(neg)
OPENSMACX_X86_UNARY(not)

#undef OPENSMACX_X86_BINARY
#undef OPENSMACX_X86_UNARY

#endif
