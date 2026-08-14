// ORIGINAL: 0x00548DD0
// RULED-OUT: MISMATCH #10 push/mov - the per-(player,base) flag array and
//            the per-base coordinate/owner tables used as plain
//            static-address casts with hand computed byte strides rather
//            than typed extern arrays; register scheduling in the base-scan
//            loop still differs.
// name      ?dont_withdrawal@@YAHHH@Z
// size      429 bytes
// spans     0x00548DD0-0x00548F7D
// prototype
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3C60 0x004E3EF0 0x00644F3A
// To start: tools/decomp_status.py --work 0x00548DD0

int __cdecl dont_withdrawal(int a1, int a2) {
    *g_0093f7cc = a2;

    if ((*reinterpret_cast<unsigned int *>(
             reinterpret_cast<char *>(g_0096c9f8) + (a1 * 0x833 + a2) * 4) & 0x800000) == 0) {
        return 0;
    }
    if (*g_009a64c8 <= 0) {
        return 0;
    }

    char *record = reinterpret_cast<char *>(g_00952832);
    for (int index = 0; index < *g_009a64c8; ++index, record += 0x34) {
        short *const rec16 = reinterpret_cast<short *>(record);
        if (*reinterpret_cast<unsigned char *>(record + 4) == static_cast<unsigned char>(a2) &&
            reinterpret_cast<char *>(g_009ab892)[rec16[0] * 0x34] != 0x0b) {
            int base_y = rec16[-4];
            int base_x = rec16[-5];
            if ((*reinterpret_cast<unsigned char *>(
                     reinterpret_cast<char *>(g_0094a30c) +
                     (*g_0068faf0 * base_y + (base_x >> 1)) * 0x2c) & 0xe0) > 0x5f) {
                int base_idx = base_find(base_x, base_y, a1);
                if (base_idx >= 0) {
                    short target_y = *reinterpret_cast<short *>(
                        reinterpret_cast<char *>(g_0097d042) + base_idx * 0x9a);
                    short target_x = *reinterpret_cast<short *>(
                        reinterpret_cast<char *>(g_0097d040) + base_idx * 0x9a);
                    abs(target_x - base_x);
                    abs(target_y - base_y);
                    int territory = whose_territory(a1, base_x, base_y, 0, 0);
                    if (territory == a2 &&
                        *reinterpret_cast<unsigned char *>(record + 7) == 0x18) {
                        int base2 = base_at(rec16[5], rec16[9]);
                        if (base2 >= 0) {
                            unsigned char owner = *reinterpret_cast<unsigned char *>(
                                reinterpret_cast<char *>(g_0097d044) + base2 * 0x134);
                            if (owner != a1 && owner != a2 && owner != 0 &&
                                (*reinterpret_cast<unsigned int *>(
                                     reinterpret_cast<char *>(g_0096c9f8) +
                                     a2 * 0x20cc + owner * 4) & 0x10) != 0) {
                                unsigned int flags = *reinterpret_cast<unsigned int *>(
                                    reinterpret_cast<char *>(g_0096c9f8) +
                                    (a1 * 0x833 + owner) * 4);
                                if ((flags & 1) != 0) {
                                    return static_cast<int>(owner);
                                }
                                if ((flags & 2) != 0 && *g_0093f7d0 > 0) {
                                    return static_cast<int>(owner);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}
