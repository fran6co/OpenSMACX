// ORIGINAL: 0x004C2360 ?vehdraw_construct_vehicle@@YAHHHHHHHPAUCaviar@@@Z 0x004C2360-0x004C246D
// TRIED: `extern signed char g_0094f280[];` (array decl, not a fixed pointer const) kept the shift/index a genuine `lea`/scaled address instead of the compiler folding the base into an `add` before the `shl` - the indexed-table-base case the brief warns about.
// TRIED: every exit after an `add_object` call falls straight to `pop/ret` with no `xor eax,eax` in the original, and the final fallthrough leaves whatever the last call happened to leave in eax - so the original return type is `void`, not `int`; MNEMONIC_ONLY (109/109, 269/269 bytes) once the two early exits became bare `return;`.
// size      269 bytes
// prototype int (__cdecl ?vehdraw_construct_vehicle@@YAHHHHHHHPAUCaviar@@@Z)(int factionID, int, int, int, int, int, Caviar*)
// callers   0   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C2030 0x004C2470 0x004C2CF0 0x004C3090 0x00617920 0x00617F20
// PROPOSAL: int -> void, since the disassembly never zeroes eax before any
//           `ret` and one path returns with eax = the last-loaded a6.

extern signed char g_0094f280[];
extern unsigned char g_0094ae6a[];

void __cdecl vehdraw_construct_vehicle(int a1, int a2, int a3, int a4, int a5, int a6, Caviar * a7) {
    a7->field_13CC_ = 0;
    if (a3 == 0x15) {
        int one = 1;
        if (a2 == one && g_0094f280[a4 * 0x10] <= one) {
            a2 = 0;
        }
    }
    vehdraw_construct_chassis(a1, a2, a3, a4, a5, a7);
    vehdraw_construct_weapon(a1, a2, a3, a7);
    vehdraw_construct_abil(a1, a7, a6);
    vehdraw_construct_armor(a1, a2, a3, a4, 0, a7);
    if (a5 != 0) {
        if (a2 == 0) {
            unsigned char shape = g_0094ae6a[a3 * 0x10];
            if (shape < 3 || shape == 0xb) {
                a7->add_object((CaviarData *)(g_0079961c + a5 * 3), (VOX_Vect *)g_00687ae8, 0);
                return;
            }
        } else if (a2 == 8) {
            a7->add_object((CaviarData *)(g_0076e9a4 + a5 * 3), 0);
            return;
        }
        a7->add_object((CaviarData *)(g_0076e554 + a5 * 3), 0);
        if (a6 & 0x8000) {
            a7->add_object((CaviarData *)g_00779560, 0);
        }
    }
}
