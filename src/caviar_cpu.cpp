#include "caviar_cpu.h"

// Original starts are supported by the detection stores and renderer reads;
// neighbouring addresses do not establish complete original allocation bounds.
// RECOVERY_OBJECT: v1 0x009C0830 ?CaviarVoxelAlgoFlag@@3EA
unsigned char CaviarVoxelAlgoFlag = 0;
// RECOVERY_OBJECT: v1 0x009C0B70 ?vox_cpu_vendor@@3EA
unsigned char vox_cpu_vendor = 0;

#ifndef OPENSMACX_CPU_TEST_HARDWARE
#if !defined(__clang__) || !defined(_M_IX86)
#error Caviar CPU recovery requires the explicitly registered i386 intrinsic frontend
#endif
extern "C" void __cpuidex(int *, int, int);
#pragma intrinsic(__cpuidex)

struct CaviarCpuHardware {
    unsigned int read_flags() { return __builtin_ia32_readeflags_u32(); }
    void write_flags(unsigned int value) { __builtin_ia32_writeeflags_u32(value); }
    void identify(int *words, int leaf, int incoming_ecx) {
        __cpuidex(words, leaf, incoming_ecx);
    }
};
#endif

// ORIGINAL: 0x0063E860 sub_63e860 0x0063E860-0x0063E94E FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned0x63E860-0x63E94E preserves EAX/EBX/ECX/EDX/ESI/EDI; AC and ID full-word comparison gates, ordered output stores, signed leaf0 limit, vendor words, base-family/MMX ranks and no final flags restoration.", "Pinned0x639330 ignores incoming EAX after the call and reads rank9C0830. Full indirect caller and final condition-flag effects remain obligations; no new matching or acceptance claim.", "task:caviar-cpu-probe-oct11; preserve original identity/spans and historical VC6 limitation as seed evidence, not a reason to retain rank1 fallback."], "reason": "Recover the original CPU flags and CPUID classification in its register-preserving intrinsic owner, sharing genuine byte storage with preferences and renderer consumers."}
// TRIED: byte-exact is unreachable under the inline-assembly rule - the body
//        is the flags push/pop pair toggling EFLAGS bit 0x40000 (and
//        0x200000) plus CPUID itself, and this address calls nothing (from
//        the promoted artifact).
// Native source writer transfers the existing original identity here.
// symbol    ?vox_detect_cpu@@YAXXZ
// Historical seed finding: VC6 could not express the flags/CPUID instructions
// without assembly, so its former body selected rank1/vendor0 unconditionally.
// This intrinsic owner restores detection rather than preserving that fallback.
// The known caller ignores the result; original EAX is restored, not an int0.
// Full indirect caller coverage and final condition-flag fidelity remain open.
void vox_detect_cpu() {
    CaviarCpuHardware hardware;
    caviar_cpu_detail::classify(hardware, CaviarVoxelAlgoFlag, vox_cpu_vendor);
}
