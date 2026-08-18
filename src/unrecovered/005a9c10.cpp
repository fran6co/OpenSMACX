// ORIGINAL: 0x005A9C10 ?load_map_daemon@@YAHPAD@Z 0x005A9C10-0x005A9DAD
// RULED-OUT: extension check, env_open, header_check against the TERRAN/ TERRANMAP string depending on the extension, then (fread reuses the `a1` parameter's own stack slot as scratch, matching the disassembly) a MapWin close sweep, map_data, and the counter table reset/copy. Landed at 0.80 similarity; diverges at #3 on register vs stack scheduling for the extension check.
// size      413 bytes
// prototype
// callers   2   call targets   10
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00470F70 0x0057D1F0 0x0059A220 0x005A9280 0x00634BB0 0x00645598 0x00645660 0x00645DD0 0x00646178 0x0064FD20

typedef int (__cdecl *StrcmpiFn)(const char *, const char *);
typedef int (__cdecl *FcloseFn)(FILE *);

int __cdecl load_map_daemon(char *a1) {
    int isMap = 0;

    char *dot = strchr(a1, '.');
    if (dot != 0) {
        char *ext = *reinterpret_cast<char **>(g_00691b34);
        if ((reinterpret_cast<StrcmpiFn>(_strcmpi))(dot + 1, ext) == 0) {
            isMap = 1;
        }
    }

    FILE *fileHandle = reinterpret_cast<FILE *>(env_open(a1, reinterpret_cast<char *>(g_00690db8)));
    if (fileHandle == 0) {
        return 1;
    }

    int result = 2;
    FILE *savedHandle = fileHandle;

    char headerBuf[256];
    header_check(headerBuf, fileHandle);

    const char *expected = isMap ? reinterpret_cast<const char *>(g_00690dc4)
                                  : reinterpret_cast<const char *>(g_00690dbc);
    if (strcmp(headerBuf, expected) == 0) {
        result = 3;

        int freadResult = _fread(&a1, 4, 1, fileHandle);
        int mask = -(isMap != 0) & 0x51;
        int threshold = mask + 5;
        if (freadResult != 0 && reinterpret_cast<int>(a1) >= threshold) {
            for (int *p = g_007d3c40; p < g_007d3c5c; p++) {
                int val = *p;
                if (val != 0 && *reinterpret_cast<int *>(val + 0x1dd74) != 0) {
                    *reinterpret_cast<int *>(val + 0x1dd74) = 0;
                    reinterpret_cast<MapWin *>(val)->close();
                }
            }
            *g_00939294 = 0;
            result = 4;

            int mapResult = map_data(fileHandle, 0, isMap);
            if (mapResult == 0) {
                int count = *g_00949884;
                *g_009a64cc = 0;
                *g_009a64c8 = 0;
                *g_00939288 = 0;
                *g_0093928c = 0xffffffff;

                if (count > 0) {
                    char *rec = reinterpret_cast<char *>(*g_0094a30c) + 8;
                    int i = 0;
                    do {
                        int v = *reinterpret_cast<int *>(rec);
                        *reinterpret_cast<unsigned char *>(rec - 4) = 0;
                        v &= ~3;
                        i++;
                        *reinterpret_cast<int *>(rec) = v;
                        count = *g_00949884;
                        rec += 0x2c;
                    } while (i < count);
                }

                *g_00689374 = 0xffffffff;
                for (int j = 0; j < 11; j++) {
                    g_0068fb4c[j] = g_0068fb20[j];
                }

                result = 0;
                reinterpret_cast<Path *>(g_00945b00)->init();
                fileHandle = savedHandle;
            }
        }
    }

    (reinterpret_cast<FcloseFn>(fclose))(savedHandle);
    return result;
}
