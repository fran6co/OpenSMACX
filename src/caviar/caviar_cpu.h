#pragma once

// Renderer rank and vendor are shared by CPU detection, preferences and
// renderer selection. They are bytes, not pointers or executable aliases.
extern unsigned char CaviarVoxelAlgoFlag;
extern unsigned char vox_cpu_vendor;

#if defined(__clang__) && defined(_M_IX86) && !defined(OPENSMACX_CPU_TEST_HARDWARE)
__attribute__((no_caller_saved_registers))
#endif
void vox_detect_cpu();

namespace caviar_cpu_detail {

// Hardware policy is separate only to test the original register operations
// without changing host flags. Production supplies real flags/CPUID intrinsics.
template<class Hardware>
void classify(Hardware &hardware, unsigned char &rank, unsigned char &vendor) {
    const unsigned int saved = hardware.read_flags();
    hardware.write_flags(saved ^ 0x40000u);
    const unsigned int observed = hardware.read_flags();
    rank = 0;
    vendor = 0;
    // The original compares the entire returned flags word, not just AC/ID.
    if (observed == saved) return;
    hardware.write_flags(saved);
    rank = 1;
    hardware.write_flags(saved ^ 0x200000u);
    if (hardware.read_flags() == saved) return;

    int words[4];
    // Preserve original incoming ECX even for leaves that ordinarily ignore it.
    hardware.identify(words, 0, static_cast<int>(saved));
    if (words[1] == 0x756e6547 && words[3] == 0x49656e69 && words[2] == 0x6c65746e) {
        vendor = 1;  // GenuineIntel, EBX/EDX/ECX order
    } else if (words[1] == 0x68747541 && words[3] == 0x69746e65 && words[2] == 0x444d4163) {
        vendor = 2;  // AuthenticAMD
    }
    if (words[0] < 1) return;  // Signed comparison in the original.
    hardware.identify(words, 1, words[2]);
    const unsigned int family = (static_cast<unsigned int>(words[0]) >> 8) & 15u;
    unsigned char selected = family == 5 ? 2 : family == 6 ? 4 : 0;
    if (selected != 0 && (words[3] & 0x800000) != 0) ++selected;
    rank = selected;
    // Deliberately no final flags restoration: the original leaves ID toggled.
}

}  // namespace caviar_cpu_detail
