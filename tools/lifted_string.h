#ifndef OPENSMACX_LIFTED_STRING_H
#define OPENSMACX_LIFTED_STRING_H

// The x86 string operations - MOVS, STOS, LODS, SCAS, CMPS - and the REP
// prefixes that turn each of them into a loop.
//
// These are the last instructions in the image that are not "an operation over
// values": they read their operands from ESI and EDI, write their counter to
// ECX, and take their DIRECTION from a flag rather than from the encoding. The
// generic operand model in `tools/x86_lower.py` therefore has nothing to say
// about them - there are no operands to lower - so each one is a single call
// naming the operation and its width, and everything else is here.
//
// Three things are easy to get wrong, and each has its own comment below:
// where the direction flag lives, when ECX is tested, and what REP does to ZF.
//
// ---------------------------------------------------------------------------
// WHERE DF LIVES, AND WHY
// ---------------------------------------------------------------------------
//
// `OpensmacxX86Flag` in `tools/static_recompile_runtime.h` names six flags -
// CF, PF, AF, ZF, SF, OF - and that file cannot change, because
// `static_recompile_pilot.py` is written against it.
//
// DF does not need it to change. `eflags` is a full `uint32_t` and the enum
// uses the ARCHITECTURAL bit positions, so bit 10 - DF's real position on the
// hardware - is simply unoccupied. The decision made here is to store DF THERE,
// at its true bit, rather than inventing a spare field or a shadow bool:
//
//   * the state word stays bit-compatible with a real EFLAGS image, which is
//     what makes the differential test below a straight comparison - the same
//     word goes into `popfl` and comes out of `pushfl`;
//   * PUSHF/POPF, when phase 4 reaches them, will carry DF for free;
//   * nothing existing clobbers it. Every flag write in `lifted_x86.h` is of
//     the form `(s.eflags & ~MASK) | ...`, and every mask used - the six-flag
//     `OpensmacxAluFlags` (0x8D5), `OpensmacxAluFlagsKeepingCarry`, and the
//     CF|OF pair the rotates and the multiplies write - excludes bit 10. An
//     arithmetic instruction between a `std` and a `rep movsd` cannot lose the
//     direction, which is exactly the property the real machine has.
//
// The one obligation this creates is on the caller side of the world: a lifted
// body that hands `s.eflags` to real hardware must clear DF afterwards, because
// the platform ABI requires DF clear at a call boundary. Nothing in this header
// leaves DF changed - only `cld` and `std` write it, which is also true of the
// original.

#include "lifted_x86.h"

constexpr uint32_t OpensmacxDirectionFlag = 1U << 10;

inline bool opensmacx_df(const OpensmacxStaticRecompileState &s) {
    return (s.eflags & OpensmacxDirectionFlag) != 0;
}

// CLD and STD write DF and nothing else - not the ALU flags, not ZF.
inline void opensmacx_cld(OpensmacxStaticRecompileState &s) {
    s.eflags &= ~OpensmacxDirectionFlag;
}

inline void opensmacx_std(OpensmacxStaticRecompileState &s) {
    s.eflags |= OpensmacxDirectionFlag;
}

// ---------------------------------------------------------------------------
// Width
//
// Memory goes through the same accessors as every other lifted load and store;
// the comparison goes through `opensmacx_sub`, because SCAS and CMPS set flags
// exactly as CMP does and there is no reason for a second implementation of
// six flags to exist and drift.
// ---------------------------------------------------------------------------

template <unsigned W>
struct OpensmacxStringWidth;

template <>
struct OpensmacxStringWidth<8> {
    static constexpr uint32_t kBytes = 1U;
    static uint32_t load(uint32_t address) { return opensmacx_mem8(address); }
    static void store(uint32_t address, uint32_t value) {
        opensmacx_store8(address, value);
    }
    static void compare(OpensmacxStaticRecompileState &s, uint32_t a, uint32_t b) {
        (void)opensmacx_sub8(s, a, b);
    }
};

template <>
struct OpensmacxStringWidth<16> {
    static constexpr uint32_t kBytes = 2U;
    static uint32_t load(uint32_t address) { return opensmacx_mem16(address); }
    static void store(uint32_t address, uint32_t value) {
        opensmacx_store16(address, value);
    }
    static void compare(OpensmacxStaticRecompileState &s, uint32_t a, uint32_t b) {
        (void)opensmacx_sub16(s, a, b);
    }
};

template <>
struct OpensmacxStringWidth<32> {
    static constexpr uint32_t kBytes = 4U;
    static uint32_t load(uint32_t address) { return opensmacx_mem32(address); }
    static void store(uint32_t address, uint32_t value) {
        opensmacx_store32(address, value);
    }
    static void compare(OpensmacxStaticRecompileState &s, uint32_t a, uint32_t b) {
        (void)opensmacx_sub32(s, a, b);
    }
};

// The signed stride, carried as the unsigned value that adds to the same thing.
// The step is ONE ELEMENT, not one byte: `rep movsd` advances by four. Getting
// this wrong copies the right number of bytes to a quarter of the destination.
template <unsigned W>
inline uint32_t opensmacx_string_step(const OpensmacxStaticRecompileState &s) {
    constexpr uint32_t bytes = OpensmacxStringWidth<W>::kBytes;
    return opensmacx_df(s) ? (0U - bytes) : bytes;
}

// AL / AX / EAX, the implicit accumulator. A narrow write MERGES, for the same
// reason every sub-register write in the lowering does: `lodsb` leaves the top
// 24 bits of EAX alone. At W == 32 the mask is all ones and the merge collapses
// to a replacement, so no special case is needed.
//
// The mask on the READ is redundant today and deliberate anyway: every consumer
// of it - `opensmacx_store8/16` and `opensmacx_sub8/16` - narrows its own
// arguments, so removing it here changes no behaviour and no test can see the
// difference. It states the width at the point the value is read rather than
// leaving that to two callees, which is the property that has to hold if either
// of them ever stops masking.
template <unsigned W>
inline uint32_t opensmacx_string_accumulator(const OpensmacxStaticRecompileState &s) {
    return s.eax & OpensmacxWidth<W>::kMask;
}

template <unsigned W>
inline void opensmacx_string_set_accumulator(OpensmacxStaticRecompileState &s,
                                             uint32_t value) {
    constexpr uint32_t mask = OpensmacxWidth<W>::kMask;
    s.eax = (s.eax & ~mask) | (value & mask);
}

// ---------------------------------------------------------------------------
// One element
//
// MOVS, STOS and LODS touch NO flags. Only SCAS and CMPS do, and they get them
// from the shared subtract.
// ---------------------------------------------------------------------------

template <unsigned W>
inline void opensmacx_movs_one(OpensmacxStaticRecompileState &s) {
    const uint32_t step = opensmacx_string_step<W>(s);
    OpensmacxStringWidth<W>::store(s.edi, OpensmacxStringWidth<W>::load(s.esi));
    s.esi += step;
    s.edi += step;
}

template <unsigned W>
inline void opensmacx_stos_one(OpensmacxStaticRecompileState &s) {
    const uint32_t step = opensmacx_string_step<W>(s);
    OpensmacxStringWidth<W>::store(s.edi, opensmacx_string_accumulator<W>(s));
    s.edi += step;
}

template <unsigned W>
inline void opensmacx_lods_one(OpensmacxStaticRecompileState &s) {
    const uint32_t step = opensmacx_string_step<W>(s);
    opensmacx_string_set_accumulator<W>(s, OpensmacxStringWidth<W>::load(s.esi));
    s.esi += step;
}

// SCAS is `cmp accumulator, [EDI]` - the accumulator is the LEFT operand, so a
// value below the one in memory sets CF. Swapping the two would leave ZF right
// and every other flag wrong, which is the shape of bug that survives a test
// that only looks at whether the search terminated.
template <unsigned W>
inline void opensmacx_scas_one(OpensmacxStaticRecompileState &s) {
    const uint32_t step = opensmacx_string_step<W>(s);
    OpensmacxStringWidth<W>::compare(s, opensmacx_string_accumulator<W>(s),
                                     OpensmacxStringWidth<W>::load(s.edi));
    s.edi += step;
}

// CMPS is `cmp [ESI], [EDI]`: the SOURCE is the left operand. This is the one
// string operation whose operand order reads backwards from its Intel-syntax
// destination-first neighbours, and Capstone prints it in that order too.
template <unsigned W>
inline void opensmacx_cmps_one(OpensmacxStaticRecompileState &s) {
    const uint32_t step = opensmacx_string_step<W>(s);
    OpensmacxStringWidth<W>::compare(s, OpensmacxStringWidth<W>::load(s.esi),
                                     OpensmacxStringWidth<W>::load(s.edi));
    s.esi += step;
    s.edi += step;
}

// ---------------------------------------------------------------------------
// The REP prefixes
//
// ECX is tested BEFORE the body, which is the whole of the entry case: `rep
// movsd` with ECX == 0 copies nothing, advances nothing and is architecturally
// a no-op. A do/while - or a decrement placed before the test - runs the body
// once and then loops 4,294,967,295 more times, and the two are indistinguish-
// able on every input except the one that matters.
//
// Plain REP does NOT consult ZF, at any width, for any of MOVS/STOS/LODS. It is
// only the comparing forms that have a second exit, and there ZF is tested
// AFTER the iteration: `repne scasb` over a buffer whose first byte matches
// must stop having consumed exactly one byte, with ECX one lower.
// ---------------------------------------------------------------------------

template <void (*STEP)(OpensmacxStaticRecompileState &)>
inline void opensmacx_rep_run(OpensmacxStaticRecompileState &s) {
    while (s.ecx != 0U) {
        STEP(s);
        --s.ecx;
    }
}

template <void (*STEP)(OpensmacxStaticRecompileState &), bool CONTINUE_WHILE_ZF>
inline void opensmacx_rep_while(OpensmacxStaticRecompileState &s) {
    while (s.ecx != 0U) {
        STEP(s);
        --s.ecx;
        if (opensmacx_zf(s) != CONTINUE_WHILE_ZF) {
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// The names the lowering emits
//
// `opensmacx_rep_movs32`, matching the width-suffixed convention of
// `lifted_x86.h`. The lowering pastes the Capstone mnemonic straight into the
// name - "rep movsd" becomes `rep_movs` plus the width the `d` denotes - so
// every prefix Capstone can print has a definition here, including the ones
// this image happens not to contain.
// ---------------------------------------------------------------------------

#define OPENSMACX_STRING_MOVING(NAME, W)                                       \
    inline void opensmacx_##NAME##W(OpensmacxStaticRecompileState &s) {        \
        opensmacx_##NAME##_one<W>(s);                                          \
    }                                                                          \
    inline void opensmacx_rep_##NAME##W(OpensmacxStaticRecompileState &s) {    \
        opensmacx_rep_run<opensmacx_##NAME##_one<W> >(s);                      \
    }

// SCAS and CMPS additionally take REPE and REPNE. Capstone prints F3 on a
// comparing op as "repe", but `rep` is the same encoding and some disassembly
// paths spell it that way, so it is defined as the alias it is rather than
// left to become a refusal.
#define OPENSMACX_STRING_COMPARING(NAME, W)                                    \
    inline void opensmacx_##NAME##W(OpensmacxStaticRecompileState &s) {        \
        opensmacx_##NAME##_one<W>(s);                                          \
    }                                                                          \
    inline void opensmacx_repe_##NAME##W(OpensmacxStaticRecompileState &s) {   \
        opensmacx_rep_while<opensmacx_##NAME##_one<W>, true>(s);               \
    }                                                                          \
    inline void opensmacx_repne_##NAME##W(OpensmacxStaticRecompileState &s) {  \
        opensmacx_rep_while<opensmacx_##NAME##_one<W>, false>(s);              \
    }                                                                          \
    inline void opensmacx_rep_##NAME##W(OpensmacxStaticRecompileState &s) {    \
        opensmacx_repe_##NAME##W(s);                                           \
    }

OPENSMACX_STRING_MOVING(movs, 8)
OPENSMACX_STRING_MOVING(movs, 16)
OPENSMACX_STRING_MOVING(movs, 32)
OPENSMACX_STRING_MOVING(stos, 8)
OPENSMACX_STRING_MOVING(stos, 16)
OPENSMACX_STRING_MOVING(stos, 32)
OPENSMACX_STRING_MOVING(lods, 8)
OPENSMACX_STRING_MOVING(lods, 16)
OPENSMACX_STRING_MOVING(lods, 32)

OPENSMACX_STRING_COMPARING(scas, 8)
OPENSMACX_STRING_COMPARING(scas, 16)
OPENSMACX_STRING_COMPARING(scas, 32)
OPENSMACX_STRING_COMPARING(cmps, 8)
OPENSMACX_STRING_COMPARING(cmps, 16)
OPENSMACX_STRING_COMPARING(cmps, 32)

#undef OPENSMACX_STRING_MOVING
#undef OPENSMACX_STRING_COMPARING

#endif
