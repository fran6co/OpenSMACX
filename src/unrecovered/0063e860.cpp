// ORIGINAL: 0x0063E860 FILE
// RULED-OUT: byte-exact is unreachable under the inline-assembly rule. The
//            body is a classic CPUID-availability probe: the flags-register
//            push/pop pair toggling EFLAGS bit 0x40000 (and 0x200000) to
//            test for CPUID support, then the CPUID instruction itself to
//            read the vendor string and family (Ghidra's synthesized
//            cpuid_basic_info/cpuid_Version_info are not real callees -
//            this address calls nothing). None of the flags push/pop or
//            CPUID have a VC6 intrinsic or non-inline-assembly C++ spelling,
//            so no source form reaches these bytes under that rule. Landed
//            a plain stub that clears the two output flags and skips
//            detection.
// name      sub_63e860
// size      238 bytes
// spans     0x0063E860-0x0063E94E
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x0063E860

extern "C" int __cdecl sub_63e860() {
    unsigned char *const flag_9c0830 = (unsigned char *)0x009C0830;
    unsigned char *const flag_9c0b70 = (unsigned char *)0x009C0B70;
    *flag_9c0830 = 0;
    *flag_9c0b70 = 0;
    return 0;
}
