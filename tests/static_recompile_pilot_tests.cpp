#include "static_recompile_runtime.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" uint32_t __attribute__((thiscall))
opensmacx_static_recompiled_result_004C5460(const uint8_t *, uint32_t)
    asm("_opensmacx_static_recompiled_result_004C5460");

extern "C" void opensmacx_static_recompiled_core_004C5460(
    OpensmacxStaticRecompileState *, const uint8_t *, uint32_t);

namespace {

constexpr size_t CanarySize = 16;
constexpr size_t ObjectSize = 0x24 + 16 * 0x18;
constexpr uint32_t CompareFlagMask =
    OpensmacxCarryFlag | OpensmacxParityFlag | OpensmacxAuxiliaryCarryFlag
    | OpensmacxZeroFlag | OpensmacxSignFlag | OpensmacxOverflowFlag;
constexpr uint32_t TestFlagMask = CompareFlagMask & ~OpensmacxAuxiliaryCarryFlag;

struct DeviceFixture {
    uint8_t storage[CanarySize + ObjectSize + CanarySize];

    uint8_t *object_bytes() {
        return storage + CanarySize;
    }

};

int Failures = 0;

void expect(bool condition, const char *message) {
    if (!condition) {
        std::printf("FAIL: %s\n", message);
        ++Failures;
    }
}

void initialize(DeviceFixture &fixture) {
    for (size_t index = 0; index < sizeof(fixture.storage); ++index) {
        fixture.storage[index] = static_cast<uint8_t>(0x3DU + index * 29U);
    }
}

uint32_t expected_result(uint32_t index, uint8_t disabled) {
    if (index > 15U) {
        return (index & 0xFFFFFF00U) | 1U;
    }
    return disabled == 0 ? 1U : 0U;
}

void verify_core_preservation(
        const OpensmacxStaticRecompileState &before,
        const OpensmacxStaticRecompileState &after) {
    expect(after.ebx == before.ebx, "core preserves EBX");
    expect(after.ecx == before.ecx, "core preserves ECX");
    expect(after.esi == before.esi, "core preserves ESI");
    expect(after.edi == before.edi, "core preserves EDI");
    expect(after.ebp == before.ebp, "core restores EBP");
    expect(after.esp == before.esp + 8U, "core models RET 4 stack movement");
}

#if !defined(OPENSMACX_STATIC_RECOMPILE_STANDALONE)
struct OriginalCapture {
    uint32_t eax;
    uint32_t edx;
    uint32_t eflags;
    uint32_t stack_before;
    uint32_t stack_after;
};

OriginalCapture capture_original(
        const uint8_t *object, uint32_t argument, uint32_t initial_edx) {
    OriginalCapture capture = {};
    capture.edx = initial_edx;
    __asm__ volatile("movl %%esp, %0" : "=r"(capture.stack_before));
    __asm__ volatile(
        "movl %[object], %%ecx\n\t"
        "pushl %[argument]\n\t"
        "call _opensmacx_legacy_004C5460\n\t"
        "pushfl\n\t"
        "popl %[flags]"
        : "=a"(capture.eax), "+d"(capture.edx), [flags] "=r"(capture.eflags)
        : [argument] "m"(argument), [object] "m"(object)
        : "ecx", "memory", "cc");
    __asm__ volatile("movl %%esp, %0" : "=r"(capture.stack_after));
    return capture;
}
#endif

void test_valid_indices() {
    const uint8_t disabled_values[] = {0U, 1U, 0x80U, 0xFFU};
    const uint32_t edx_values[] = {0U, 0x12345678U, 0xFFFFFF00U};
    for (uint32_t index = 0; index < 16U; ++index) {
        for (uint8_t disabled : disabled_values) {
            DeviceFixture generated_fixture;
            initialize(generated_fixture);
            generated_fixture.object_bytes()[0x24 + index * 0x18] = disabled;
            DeviceFixture original_fixture;
            std::memcpy(&original_fixture, &generated_fixture, sizeof(original_fixture));
            uint8_t before[sizeof(generated_fixture.storage)];
            std::memcpy(before, generated_fixture.storage, sizeof(before));

            const uint32_t generated = opensmacx_static_recompiled_result_004C5460(
                generated_fixture.object_bytes(), index);
            expect(generated == expected_result(index, disabled),
                   "generated wrapper returns expected valid-index value");
            expect(std::memcmp(
                       before, generated_fixture.storage, sizeof(before)) == 0,
                   "generated wrapper leaves valid object and canaries unchanged");

            for (uint32_t initial_edx : edx_values) {
                OpensmacxStaticRecompileState state = {
                    0xA1A2A3A4U, 0xB1B2B3B4U,
                    reinterpret_cast<uintptr_t>(generated_fixture.object_bytes()), initial_edx,
                    0x51525354U, 0x61626364U, 0x71727374U, 0x81828384U,
                    0x00000202U,
                };
                const OpensmacxStaticRecompileState initial = state;
                opensmacx_static_recompiled_core_004C5460(
                    &state, generated_fixture.object_bytes(), index);
                expect(state.eax == expected_result(index, disabled),
                       "generated core returns expected valid-index value");
                expect(state.edx == ((initial_edx & 0xFFFFFF00U) | disabled),
                       "generated core preserves upper EDX around DL load");
                verify_core_preservation(initial, state);

#if !defined(OPENSMACX_STATIC_RECOMPILE_STANDALONE)
                const OriginalCapture original = capture_original(
                    original_fixture.object_bytes(), index, initial_edx);
                expect(original.eax == state.eax,
                       "generated core EAX matches original valid path");
                expect(original.edx == state.edx,
                       "generated core EDX matches original valid path");
                expect((original.eflags & TestFlagMask)
                           == (state.eflags & TestFlagMask),
                       "generated core flags match original TEST path");
                expect(original.stack_before == original.stack_after,
                       "original thiscall balances its stack argument");
                expect(std::memcmp(
                           original_fixture.storage, generated_fixture.storage,
                           sizeof(original_fixture.storage)) == 0,
                       "original and generated calls preserve identical complete storage");
#endif
            }
        }
    }
}

void test_invalid_indices() {
    const uint32_t indices[] = {
        16U, 0x100U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU,
    };
    for (uint32_t index : indices) {
        expect(opensmacx_static_recompiled_result_004C5460(
                   reinterpret_cast<const uint8_t *>(1U), index)
                   == expected_result(index, 0),
               "generated wrapper does not dereference invalid object on invalid index");

        OpensmacxStaticRecompileState state = {
            0xA1A2A3A4U, 0xB1B2B3B4U, 1U, 0xC1C2C3C4U,
            0x51525354U, 0x61626364U, 0x71727374U, 0x81828384U,
            0x00000202U,
        };
        const OpensmacxStaticRecompileState initial = state;
        opensmacx_static_recompiled_core_004C5460(
            &state, reinterpret_cast<const uint8_t *>(1U), index);
        expect(state.eax == expected_result(index, 0),
               "generated core preserves invalid-index upper EAX bits");
        expect(state.edx == initial.edx,
               "generated core preserves EDX on invalid path");
        verify_core_preservation(initial, state);

#if !defined(OPENSMACX_STATIC_RECOMPILE_STANDALONE)
        const OriginalCapture original = capture_original(
            reinterpret_cast<const uint8_t *>(1U), index, initial.edx);
        expect(original.eax == state.eax,
               "generated core EAX matches original invalid path");
        expect(original.edx == state.edx,
               "generated core EDX matches original invalid path");
        expect((original.eflags & CompareFlagMask)
                   == (state.eflags & CompareFlagMask),
               "generated core flags match original CMP path");
        expect(original.stack_before == original.stack_after,
               "original invalid path balances its stack argument");
#endif
    }
}

}  // namespace

int main() {
    static_assert(sizeof(uintptr_t) == sizeof(uint32_t),
                  "static recompilation pilot requires i386");
    test_valid_indices();
    test_invalid_indices();
    if (Failures != 0) {
        std::printf("%d static recompilation pilot checks failed\n", Failures);
        return 1;
    }
    std::puts("Static recompilation pilot checks passed");
    return 0;
}
