#include "caviar_cpu.h"

// Original starts are supported by the detection stores and renderer reads;
// neighbouring addresses do not establish complete original allocation bounds.
// RECOVERY_OBJECT: v1 0x009C0830 ?CaviarVoxelAlgoFlag@@3EA
unsigned char CaviarVoxelAlgoFlag = 0;
// RECOVERY_OBJECT: v1 0x009C0B70 ?vox_cpu_vendor@@3EA
unsigned char vox_cpu_vendor = 0;

// CPUID vendor words in EBX/EDX/ECX order: "GenuineIntel", "AuthenticAMD".
// The two 12-byte strings are adjacent in .data with no terminators.
// RECOVERY_OBJECT: v1 0x00698DD8 ?vox_vendor_intel@@3PAKA
unsigned long vox_vendor_intel[3] = {0x756e6547, 0x49656e69, 0x6c65746e};
// RECOVERY_OBJECT: v1 0x00698DE4 ?vox_vendor_amd@@3PAKA
unsigned long vox_vendor_amd[3] = {0x68747541, 0x69746e65, 0x444d4163};

// ORIGINAL: 0x0063E860 sub_63e860 0x0063E860-0x0063E94E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned0x63E860-0x63E94E preserves EAX/EBX/ECX/EDX/ESI/EDI; AC and ID full-word comparison gates, ordered output stores, signed leaf0 limit, vendor words, base-family/MMX ranks and no final flags restoration.", "Pinned0x639330 ignores incoming EAX after the call and reads rank9C0830. Full indirect caller and final condition-flag effects remain obligations; no new matching or acceptance claim.", "task:caviar-cpu-probe-oct11; preserve original identity/spans and historical VC6 limitation as seed evidence, not a reason to retain rank1 fallback."], "reason": "Recover the original CPU flags and CPUID classification in its register-preserving intrinsic owner, sharing genuine byte storage with preferences and renderer consumers."}
// symbol    ?vox_detect_cpu@@YAXXZ
// Hand-written asm: pushfd/popfd AC and ID toggling plus cpuid, saving
// eax..edi around a manual ebp frame; flags are left with ID toggled.
__declspec(naked) void vox_detect_cpu() {
    __asm {
        push ebp
        mov ebp, esp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        pushfd
        pop eax
        mov ecx, eax
        xor eax, 0x40000
        push eax
        popfd
        pushfd
        pop eax
        xor eax, ecx
        mov byte ptr [CaviarVoxelAlgoFlag], 0
        mov byte ptr [vox_cpu_vendor], 0
        je cpu_63e946
        push ecx
        popfd
        mov byte ptr [CaviarVoxelAlgoFlag], 1
        mov eax, ecx
        xor eax, 0x200000
        push eax
        popfd
        pushfd
        pop eax
        xor eax, ecx
        je cpu_63e946
        mov eax, 0
        cpuid
        cmp dword ptr [vox_vendor_intel], ebx
        jne cpu_63e8d0
        cmp dword ptr [vox_vendor_intel+4], edx
        jne cpu_63e8d0
        cmp dword ptr [vox_vendor_intel+8], ecx
        jne cpu_63e8d0
        mov byte ptr [vox_cpu_vendor], 1
        jmp cpu_63e8ef
cpu_63e8d0:
        cmp dword ptr [vox_vendor_amd], ebx
        jne cpu_63e8ef
        cmp dword ptr [vox_vendor_amd+4], edx
        jne cpu_63e8ef
        cmp dword ptr [vox_vendor_amd+8], ecx
        jne cpu_63e8ef
        mov byte ptr [vox_cpu_vendor], 2
cpu_63e8ef:
        cmp eax, 1
        jl cpu_63e946
        mov eax, 1
        cpuid
        shr eax, 8
        and eax, 0xf
        cmp eax, 5
        je cpu_63e914
        cmp eax, 6
        je cpu_63e92e
        mov byte ptr [CaviarVoxelAlgoFlag], 0
        jmp cpu_63e946
cpu_63e914:
        test edx, 0x800000
        jne cpu_63e925
        mov byte ptr [CaviarVoxelAlgoFlag], 2
        jmp cpu_63e946
cpu_63e925:
        mov byte ptr [CaviarVoxelAlgoFlag], 3
        jmp cpu_63e946
cpu_63e92e:
        test edx, 0x800000
        jne cpu_63e93f
        mov byte ptr [CaviarVoxelAlgoFlag], 4
        jmp cpu_63e946
cpu_63e93f:
        mov byte ptr [CaviarVoxelAlgoFlag], 5
cpu_63e946:
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        ret
    }
}
