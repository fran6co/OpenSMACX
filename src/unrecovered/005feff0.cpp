// ORIGINAL: 0x005FEFF0 ?create_table_from_color@Palette@@QAEHHPAEHHHH@Z 0x005FEFF0-0x005FF19E
// RULED-OUT: MISMATCH #3 test/push - Palette has no named per-field layout (all generic field_XX_), so `this` is treated as a raw unsigned char* (the 256-entry colour table, then the 5-record "special colours" table at +0x40c) rather than named members.
// size      430 bytes
// prototype int (__thiscall ?create_table_from_color@Palette@@QAEHHPAEHHHH@Z)(Palette* this, int, unsigned int8*, int, int, int, int)
// callers   2   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FF470
// To start: tools/decomp_status.py --work 0x005FEFF0

int Palette::create_table_from_color(int a1, unsigned char * a2, int a3, int a4, int a5, int a6) {
    unsigned char *const colors = reinterpret_cast<unsigned char *>(this);

    if (*g_009b8178 == 0) {
        return 7;
    }
    if (a2 == 0) {
        return 0x10;
    }

    int i = 0;
    if (0 < a3) {
        do {
            a2[i] = static_cast<unsigned char>(i);
            ++i;
        } while (i < a3);
    }
    int range_end = a3 + a4;
    for (i = range_end; i < 0x100; ++i) {
        a2[i] = static_cast<unsigned char>(i);
    }

    int index = a3;
    uint32_t target = *reinterpret_cast<uint32_t *>(colors + (a1 & 0xff) * 4);

    if (a3 < range_end) {
        int divisor = a5 + a6;
        unsigned char blend[0x400];
        unsigned char *dest = blend + a3 * 4;
        unsigned char *src = colors + 2 + a3 * 4;
        do {
            dest[0] = static_cast<unsigned char>(
                (static_cast<int>(src[-2]) * a5 + static_cast<int>(target & 0xff) * a6) / divisor);
            dest[1] = static_cast<unsigned char>(
                (static_cast<int>(src[-1]) * a5 + static_cast<int>((target >> 8) & 0xff) * a6) / divisor);
            int sum = static_cast<int>(src[0]) * a5 + static_cast<int>((target >> 0x10) & 0xff) * a6;
            int blue = sum / divisor;
            dest[2] = static_cast<unsigned char>(blue);
            unsigned char nearest = static_cast<unsigned char>(
                get_nearest_palette_index(dest[0], dest[1], dest[2], 1));
            src += 4;
            dest += 4;
            a2[index] = nearest;
            ++index;
        } while (index < range_end);
    }

    unsigned char *record = colors + 0x40c;
    int remaining = 5;
    do {
        if (*reinterpret_cast<int32_t *>(record - 8) != -1) {
            unsigned int start = record[0];
            if (start < static_cast<unsigned int>(record[1] + start)) {
                do {
                    a2[start] = static_cast<unsigned char>(start);
                    ++start;
                } while (static_cast<int>(start) < static_cast<int>(record[0] + record[1]));
            }
        }
        record += 0x10;
        --remaining;
    } while (remaining != 0);

    return 0;
}
