#ifndef OPENSMACX_STATIC_RECOMPILE_RUNTIME_H
#define OPENSMACX_STATIC_RECOMPILE_RUNTIME_H

#include <cstdint>

struct OpensmacxStaticRecompileState {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eflags;
};

enum OpensmacxX86Flag : uint32_t {
    OpensmacxCarryFlag = 1U << 0,
    OpensmacxParityFlag = 1U << 2,
    OpensmacxAuxiliaryCarryFlag = 1U << 4,
    OpensmacxZeroFlag = 1U << 6,
    OpensmacxSignFlag = 1U << 7,
    OpensmacxOverflowFlag = 1U << 11,
};

inline uint8_t opensmacx_low8(uint32_t value) {
    return static_cast<uint8_t>(value);
}

inline void opensmacx_set_low8(uint32_t &value, uint8_t low) {
    value = (value & 0xFFFFFF00U) | low;
}

inline bool opensmacx_even_parity(uint8_t value) {
    unsigned int ones = 0;
    for (unsigned int bit = 0; bit < 8; ++bit) {
        ones += (value >> bit) & 1U;
    }
    return (ones & 1U) == 0;
}

inline void opensmacx_set_flag(uint32_t &eflags, uint32_t flag, bool set) {
    if (set) {
        eflags |= flag;
    } else {
        eflags &= ~flag;
    }
}

inline void opensmacx_cmp32(
        OpensmacxStaticRecompileState &state, uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    opensmacx_set_flag(state.eflags, OpensmacxCarryFlag, left < right);
    opensmacx_set_flag(
        state.eflags, OpensmacxParityFlag,
        opensmacx_even_parity(static_cast<uint8_t>(result)));
    opensmacx_set_flag(
        state.eflags, OpensmacxAuxiliaryCarryFlag,
        ((left ^ right ^ result) & 0x10U) != 0);
    opensmacx_set_flag(state.eflags, OpensmacxZeroFlag, result == 0);
    opensmacx_set_flag(
        state.eflags, OpensmacxSignFlag, (result & 0x80000000U) != 0);
    opensmacx_set_flag(
        state.eflags, OpensmacxOverflowFlag,
        ((left ^ right) & (left ^ result) & 0x80000000U) != 0);
}

inline void opensmacx_test8(
        OpensmacxStaticRecompileState &state, uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left & right);
    opensmacx_set_flag(state.eflags, OpensmacxCarryFlag, false);
    opensmacx_set_flag(
        state.eflags, OpensmacxParityFlag, opensmacx_even_parity(result));
    opensmacx_set_flag(state.eflags, OpensmacxZeroFlag, result == 0);
    opensmacx_set_flag(
        state.eflags, OpensmacxSignFlag, (result & 0x80U) != 0);
    opensmacx_set_flag(state.eflags, OpensmacxOverflowFlag, false);
}

inline bool opensmacx_below_or_equal(uint32_t eflags) {
    return (eflags & (OpensmacxCarryFlag | OpensmacxZeroFlag)) != 0;
}

#endif
