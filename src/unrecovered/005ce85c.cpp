// ORIGINAL: 0x005CE85C sub_5ce85c 0x005CE85C-0x005CEA09
// TRIED: MISMATCH #1 push/mov - the bit-reader state (accumulator, bits-available count, word cursor) that the original keeps in esi/edi/ebp for the whole function is modelled as ordinary locals written back once at the shared epilogue and at the single early-return branch; entry-frame shape differs first.
// size      429 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// To start: tools/decomp_status.py --work 0x005CE85C

extern "C" int __cdecl sub_5ce85c() {
    unsigned short *cursor = *reinterpret_cast<unsigned short **>(g_009c464c);
    unsigned int acc = static_cast<unsigned int>(*g_009c4650) << 8;
    int count = *g_009c4654 - 8;

    *g_009c314c = (*g_009c4650 >> 0x18) * (*g_009c454c);

    if (count < 0x10) {
        unsigned short word = *cursor;
        ++cursor;
        acc = acc | (static_cast<unsigned int>(word)
                      << ((0x10 - static_cast<char>(count)) & 0x1f));
        count = count + 8;
    }

    for (int i = 0; i < 63; ++i) {
        g_009c3150[i] = 0;
    }

    int iVar2 = 1;
    while (true) {
        unsigned int uVar4 = static_cast<unsigned int>(g_009c344c[acc >> 0x17]);
        unsigned char bVar3 = static_cast<unsigned char>(uVar4);
        int new_count = count;

        if (static_cast<char>(bVar3) > 9) {
            if (bVar3 < 0x20) {
                if (bVar3 < 0x10) {
                    acc = acc << 9;
                    new_count = count - 9;
                    if (new_count < 0x10) {
                        unsigned short word = *cursor;
                        ++cursor;
                        acc = acc | (static_cast<unsigned int>(word)
                                      << ((0x10 - static_cast<char>(new_count)) & 0x1f));
                        new_count = count + 7;
                    }
                    uVar4 = static_cast<unsigned int>(g_009c3c4c[acc >> 0x18]);
                } else {
                    acc = acc << 6;
                    new_count = count - 6;
                    if (new_count < 0x10) {
                        unsigned short word = *cursor;
                        ++cursor;
                        acc = acc | (static_cast<unsigned int>(word)
                                      << ((0x10 - static_cast<char>(new_count)) & 0x1f));
                        new_count = count + 10;
                    }
                    uVar4 = static_cast<unsigned int>(g_009c404c[acc >> 0x18]);
                }
            } else {
                if (bVar3 > 0x2f) {
                    acc = acc << 2;
                    int final_count = count - 2;
                    if (final_count < 0x10) {
                        unsigned short word = *cursor;
                        ++cursor;
                        acc = acc | (static_cast<unsigned int>(word)
                                      << ((0x10 - static_cast<char>(final_count)) & 0x1f));
                        final_count = count + 0xe;
                    }
                    *g_009c4650 = static_cast<int>(acc);
                    *g_009c4654 = final_count;
                    *g_009c464c = reinterpret_cast<int>(cursor);
                    return iVar2;
                }
                acc = acc << 6;
                new_count = count - 6;
                if (new_count < 0x10) {
                    unsigned short word = *cursor;
                    ++cursor;
                    acc = acc | (static_cast<unsigned int>(word)
                                  << ((0x10 - static_cast<char>(new_count)) & 0x1f));
                    new_count = count + 10;
                }
                uVar4 = (acc & 0xffff0000) | 0x10;
            }
        }

        count = new_count - static_cast<int>(uVar4 & 0xff);
        int next_iVar2 = iVar2 + static_cast<int>((uVar4 >> 0x10) & 0x3f);
        acc = acc << (uVar4 & 0x1f);
        if (count < 0x10) {
            unsigned short word = *cursor;
            ++cursor;
            acc = acc | (static_cast<unsigned int>(word)
                          << ((0x10 - static_cast<char>(count)) & 0x1f));
            count = count + 0x10;
        }
        int slot = g_009c334c[next_iVar2];
        iVar2 = next_iVar2 + 1;
        *reinterpret_cast<int *>(reinterpret_cast<char *>(g_009c314c) + slot) =
            (static_cast<int>(uVar4) >> 0x16) *
            (*reinterpret_cast<int *>(reinterpret_cast<char *>(g_009c454c) + slot));
    }
}
