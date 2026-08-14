// ORIGINAL: 0x004E7F80
// name      sub_4e7f80
// size      276 bytes
// spans     0x004E7F80-0x004E8094
// prototype
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E6E50 0x004E7310 0x004E7750
// RULED-OUT: same register-allocation divergence at the prologue as
//            0x004E7E50 (its near-identical sibling, subtracting the yield
//            instead of adding it); landing the same closest form.

extern int g_0066efbc[];
extern int g_0066f440[];

extern "C" int __cdecl sub_4e7f80(int a1) {
    char *baseRec = *reinterpret_cast<char **>(0x90ea30);
    int x = *reinterpret_cast<short *>(baseRec) + g_0066efbc[a1];

    if ((*reinterpret_cast<unsigned char *>(0x94988c) & 1) == 0) {
        int mapWidth = *reinterpret_cast<int *>(0x949870);
        if (x < 0) {
            x += mapWidth;
        }
        else if (x >= mapWidth) {
            x -= mapWidth;
        }
    }

    int y = *reinterpret_cast<short *>(baseRec + 2) + g_0066f440[a1];

    if (y >= 0 && y < *reinterpret_cast<int *>(0x949874) &&
        x >= 0 && x < *reinterpret_cast<int *>(0x949870)) {
        unsigned char *terrainBase = *reinterpret_cast<unsigned char **>(0x94a30c);
        int tileIdx = *reinterpret_cast<int *>(0x68faf0) * y + (x >> 1);
        if ((terrainBase[tileIdx * 0x2c] & 0xe0) < 0x60) {
            *reinterpret_cast<int *>(baseRec + 0x30) |= 0x8000;
        }

        int sum = 0;
        for (int i = 0; i < 4; i++) {
            baseRec = *reinterpret_cast<char **>(0x90ea30);
            int faction = *reinterpret_cast<int *>(0x689370);
            unsigned char owner = *reinterpret_cast<unsigned char *>(baseRec + 4);
            int rv;
            switch (i) {
            case 0:
                rv = crop_yield(owner, faction, x, y, 0);
                break;
            case 1:
                rv = mine_yield(owner, faction, x, y, 0);
                break;
            case 2:
                rv = energy_yield(owner, faction, x, y, 0);
                break;
            case 3:
                rv = 0;
                *reinterpret_cast<int *>(0x90e998) = 0;
                break;
            default:
                rv = 0;
            }
            baseRec = *reinterpret_cast<char **>(0x90ea30);
            *reinterpret_cast<int *>(baseRec + 0xc0 + i * 4) -= rv;
            sum += rv;
        }
        return sum;
    }
    return 0;
}
