// ORIGINAL: 0x005CF150
// RULED-OUT: MISMATCH #0 mov/push - bit-reader state (accumulator/count/
//            cursor) modelled as plain re-read globals instead of the
//            register-resident values the original keeps across the whole
//            switch; entry-frame shape differs first.
// name      sub_5cf150
// size      416 bytes
// spans     0x005CF150-0x005CF2F0
// prototype
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// To start: tools/decomp_status.py --work 0x005CF150

extern "C" int __cdecl sub_5cf150() {
    unsigned char *pbVar6;
    int iVar7;
    int iVar8;
    int iVar9;
    unsigned char bVar1;
    unsigned int uVar4;
    __int64 lVar3;

    unsigned char *cursor = *reinterpret_cast<unsigned char **>(g_009c5920);
    unsigned char first_byte = *cursor;

    lVar3 = static_cast<__int64>(static_cast<int>(static_cast<signed char>(*g_009c5928))) *
            static_cast<__int64>(*g_009c5820);
    g_009c5600[0] = static_cast<int>(static_cast<unsigned int>(lVar3 >> 16));
    pbVar6 = cursor + 1;
    *g_009c5928 = static_cast<int>(
        (*reinterpret_cast<unsigned int *>(g_009c5928) >> 8) |
        (static_cast<unsigned int>(first_byte)
         << ((static_cast<unsigned int>(static_cast<char>(*g_009c5924)) - 8u) & 0x1fu)));
    iVar8 = 0x3f;

LAB_005cf199:
    iVar9 = g_009c95a8[iVar8];
    iVar7 = iVar8;
    switch (*reinterpret_cast<unsigned int *>(g_009c5928) & 7) {
    case 0:
        g_009c5600[iVar9] = 0;
        break;
    case 2:
        g_009c5600[iVar9] = g_009c5820[iVar9];
        break;
    default:
        if ((*reinterpret_cast<unsigned int *>(g_009c5928) & 0xfc) == 0xfc) {
            bVar1 = *pbVar6;
            pbVar6 = pbVar6 + 1;
            *g_009c5928 = static_cast<int>(
                (*reinterpret_cast<unsigned int *>(g_009c5928) >> 8) |
                (static_cast<unsigned int>(bVar1)
                 << ((static_cast<unsigned int>(static_cast<char>(*g_009c5924)) - 8u) & 0x1fu)));
            lVar3 = static_cast<__int64>(static_cast<int>(static_cast<signed char>(*g_009c5928))) *
                    static_cast<__int64>(g_009c5820[iVar9]);
            g_009c5600[iVar9] = static_cast<int>(static_cast<unsigned int>(lVar3 >> 16));
        } else {
            int masked = static_cast<int>(*reinterpret_cast<unsigned int *>(g_009c5928) & 0xfc);
            lVar3 = static_cast<__int64>((masked << 24) >> 10) *
                    static_cast<__int64>(g_009c5820[iVar9]);
            g_009c5600[iVar9] = static_cast<int>(static_cast<unsigned int>(lVar3 >> 16));
        }
LAB_005cf2c4:
        bVar1 = *pbVar6;
        pbVar6 = pbVar6 + 1;
        *g_009c5928 = static_cast<int>(
            (*reinterpret_cast<unsigned int *>(g_009c5928) >> 8) |
            (static_cast<unsigned int>(bVar1)
             << ((static_cast<unsigned int>(static_cast<char>(*g_009c5924)) - 8u) & 0x1fu)));
        iVar9 = iVar8 - 1;
        {
            bool below_one = iVar8 < 1;
            iVar8 = iVar9;
            if (iVar9 == 0 || below_one) {
                *g_009c5920 = reinterpret_cast<int>(pbVar6);
                return 0;
            }
        }
        goto LAB_005cf199;
    case 4:
        g_009c5600[iVar9] = 0;
        g_009c5600[g_009c95a4[iVar8]] = 0;
        iVar7 = iVar8 - 1;
        break;
    case 5:
        g_009c5600[iVar9] = 0;
        iVar9 = g_009c95a4[iVar8];
        iVar8 = iVar8 - 1;
        // fallthrough
    case 1:
        uVar4 = (*reinterpret_cast<unsigned int *>(g_009c5928) & 0xff) >> 3;
        do {
            iVar7 = iVar8;
            g_009c5600[iVar9] = 0;
            g_009c5600[g_009c95a4[iVar7]] = 0;
            iVar9 = g_009c95a0[iVar7];
            --uVar4;
            iVar8 = iVar7 - 2;
        } while (uVar4 != 0);
        iVar8 = iVar7 - 1;
        goto LAB_005cf2c4;
    case 6:
        g_009c5600[iVar9] = -g_009c5820[iVar9];
        break;
    }

    *g_009c5928 = static_cast<int>(*reinterpret_cast<unsigned int *>(g_009c5928) >> 3);
    iVar9 = *g_009c5924 - 3;
    if (iVar9 < 8) {
        bVar1 = *pbVar6;
        pbVar6 = pbVar6 + 1;
        unsigned char shift_amount = static_cast<unsigned char>(iVar9);
        iVar9 = *g_009c5924 + 5;
        *g_009c5928 = static_cast<int>(
            *reinterpret_cast<unsigned int *>(g_009c5928) |
            (static_cast<unsigned int>(bVar1) << (shift_amount & 0x1f)));
    }
    iVar8 = iVar7 - 1;
    *g_009c5924 = iVar9;
    if (iVar8 == 0 || iVar7 < 1) {
        *g_009c5920 = reinterpret_cast<int>(pbVar6);
        return 0;
    }
    goto LAB_005cf199;
}
