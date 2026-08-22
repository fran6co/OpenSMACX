// ORIGINAL: 0x00617DB0 ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUCaviarData@@H@Z 0x00617DB0-0x00617F1F
// TRIED: field offsets (0x74C/0x754/0x10AC/0x10C/0x110/0x13CC) match src/caviar.h's carved-out `Caviar` layout exactly, so the scaffold's real class (from the catalogued mangled name) is used as-is; only `CaviarData` is redefined here (opaque, three raw fields matching src/caviar.h's private layout) to avoid its `close()` method and static_assert. Three parallel per-entry arrays get written each loop iteration (a single-byte table at +0x74C stride 0xC, a 3-dword record table at +0x9C0 stride 0xC via a struct copied out of `a3`, and a 9-dword table at +0x754 stride 0xC via the `rep movsd` in the original, written here as a plain element loop), plus a pointer table at +0x10AC and an 8-byte-record table at +0x10C/+0x110 sourced from `a1`. 0.56 mnemonic similarity, first divergence at #0 - not chased further.
// size      367 bytes
// prototype int (__thiscall ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUCaviarData@@H@Z)(Caviar* this, CaviarData*, LPBYTE, CaviarData*, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

class CaviarData {
public:
    int field0;
    int field4;
    char *field8_ptr;
};

int Caviar::add_object(CaviarData *a1, unsigned char *a2, CaviarData *a3, int a4) {
    char *self = reinterpret_cast<char *>(this);

    if (a1 == 0 || a3 == 0) {
        return 0x10;
    }

    char *a1sub = *reinterpret_cast<char **>(reinterpret_cast<char *>(a1) + 8);
    if (a1sub == 0) {
        return 7;
    }
    char *a3sub = *reinterpret_cast<char **>(reinterpret_cast<char *>(a3) + 8);
    if (a3sub == 0) {
        return 7;
    }

    int count1 = *reinterpret_cast<int *>(a1sub + 0x24);
    int rem = a4 % *reinterpret_cast<int *>(a3sub + 0x20);

    int byteOffset = 0;
    if (count1 > 0) {
        int outer = 0;
        do {
            if (*reinterpret_cast<int *>(self + 0x13cc) == 200) {
                return 1;
            }

            char *a3base = *reinterpret_cast<char **>(a3sub + 0x30);
            int n = *reinterpret_cast<int *>(self + 0x13cc);

            *reinterpret_cast<char *>(*reinterpret_cast<int *>(self + 0x74c + n * 0xc)) =
                *reinterpret_cast<char *>(*reinterpret_cast<int *>(a3base + byteOffset) + rem);

            {
                int *src = reinterpret_cast<int *>(
                    *reinterpret_cast<int *>(a3base + byteOffset + 4) + rem * 0xc);
                int *dst = *reinterpret_cast<int **>(self + (n + 0x9c) * 0xc);
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
            }

            {
                int *src = reinterpret_cast<int *>(
                    *reinterpret_cast<int *>(a3base + byteOffset + 8) + rem * 0x24);
                int *dst = *reinterpret_cast<int **>(self + 0x754 + n * 0xc);
                for (int k = 0; k < 9; ++k) {
                    dst[k] = src[k];
                }
            }

            *reinterpret_cast<unsigned char **>(self + 0x10ac + n * 4) = a2;

            {
                int *rec = reinterpret_cast<int *>(*reinterpret_cast<int *>(a1sub + 0x2c));
                *reinterpret_cast<int *>(self + 0x10c + n * 8) = rec[outer * 2];
                *reinterpret_cast<int *>(self + 0x110 + n * 8) = rec[outer * 2 + 1];
            }

            *reinterpret_cast<int *>(self + 0x13cc) = n + 1;
            ++outer;
            byteOffset += 0xc;
        } while (outer < count1);
    }

    return 0;
}
