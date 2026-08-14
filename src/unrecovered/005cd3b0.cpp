// ORIGINAL: 0x005CD3B0
// RULED-OUT: MISMATCH #3 push/mov - Ghidra's own decode (6 char params + int*
//            + uint, matching the disassembly's 8 stack slots exactly) used
//            over IDA's 4-char/4-int guess; the four fixed-point multiplies
//            use `__int64` (VC6 has no `long long`) mirroring
//            src/unrecovered/005cd2d0.cpp's established idiom; register
//            allocation for the four fill loops still differs from the
//            original past instruction 3.
// name      sub_5cd3b0
// size      422 bytes
// spans     0x005CD3B0-0x005CD556
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// To start: tools/decomp_status.py --work 0x005CD3B0

extern "C" void __cdecl sub_5cd3b0(char param_1, char param_2, char param_3, char param_4,
                                    char param_5, char param_6, int *param_7,
                                    unsigned int param_8) {
    int idx6 = (static_cast<int>(static_cast<__int64>(*g_009c5820) *
                                  static_cast<__int64>(static_cast<int>(param_6)) >> 16) >> 1) & 0x7f;
    int idx5 = (static_cast<int>(static_cast<__int64>(*g_009c5820) *
                                  static_cast<__int64>(static_cast<int>(param_5)) >> 16) >> 1) & 0x7f;
    int iVar7 = reinterpret_cast<int *>(g_009c5b2c)[idx6] + reinterpret_cast<int *>(g_009c5d2c)[idx5];

    unsigned int stride = param_8 & 0xfffffffcu;

    int idx1 = (static_cast<int>(static_cast<__int64>(*g_009c5820) *
                                  static_cast<__int64>(static_cast<int>(param_1)) >> 16) >> 1) & 0x7f;
    int iVar1 = reinterpret_cast<int *>(g_009c592c)[idx1];
    {
        int *piVar6 = param_7;
        int outer = 8;
        do {
            int *piVar8 = piVar6;
            for (int inner = 8; inner != 0; --inner) {
                *piVar8 = iVar1 + iVar7;
                ++piVar8;
            }
            piVar6 = reinterpret_cast<int *>(reinterpret_cast<char *>(piVar6) + stride);
            --outer;
        } while (outer != 0);
    }

    int idx2 = (static_cast<int>(static_cast<__int64>(*g_009c5820) *
                                  static_cast<__int64>(static_cast<int>(param_2)) >> 16) >> 1) & 0x7f;
    int iVar1b = reinterpret_cast<int *>(g_009c592c)[idx2];
    {
        int *piVar6 = param_7 + 8;
        int outer = 8;
        do {
            int *piVar8 = piVar6;
            for (int inner = 8; inner != 0; --inner) {
                *piVar8 = iVar1b + iVar7;
                ++piVar8;
            }
            piVar6 = reinterpret_cast<int *>(reinterpret_cast<char *>(piVar6) + stride);
            --outer;
        } while (outer != 0);
    }

    int idx3 = (static_cast<int>(static_cast<__int64>(*g_009c5820) *
                                  static_cast<__int64>(static_cast<int>(param_3)) >> 16) >> 1) & 0x7f;
    int iVar1c = reinterpret_cast<int *>(g_009c592c)[idx3];
    {
        int *piVar6 = param_7 + param_8 * 2;
        int outer = 8;
        do {
            int *piVar8 = piVar6;
            for (int inner = 8; inner != 0; --inner) {
                *piVar8 = iVar1c + iVar7;
                ++piVar8;
            }
            piVar6 = reinterpret_cast<int *>(reinterpret_cast<char *>(piVar6) + stride);
            --outer;
        } while (outer != 0);
    }

    int idx4 = (static_cast<int>(static_cast<__int64>(*g_009c5820) *
                                  static_cast<__int64>(static_cast<int>(param_4)) >> 16) >> 1) & 0x7f;
    int iVar1d = reinterpret_cast<int *>(g_009c592c)[idx4];
    {
        int *piVar6 = param_7 + param_8 * 2 + 8;
        int outer = 8;
        do {
            int *piVar8 = piVar6;
            for (int inner = 8; inner != 0; --inner) {
                *piVar8 = iVar1d + iVar7;
                ++piVar8;
            }
            piVar6 = reinterpret_cast<int *>(reinterpret_cast<char *>(piVar6) + stride);
            --outer;
        } while (outer != 0);
    }
}
