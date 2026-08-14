// ORIGINAL: 0x004DBB40 FILE
// RULED-OUT: nothing yet - MISMATCH #16 mov/push (94% mnemonic sim), a
//            push/store reordering around the `compute_faction_modifiers`
//            call; not chased further. `this` is unused throughout
//            (thiscall receiver never touched).
// name      ?editor_reset_faction@Console@@QAEXXZ
// size      242 bytes
// spans     0x004DBB40-0x004DBC32
// prototype void (__thiscall ?editor_reset_faction@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B0D70 0x005B44D0 0x005BF310
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x004DBB40

typedef int (__cdecl *NullCallback)();

void Console::editor_reset_faction() {
    if (X_pop((const char *)g_00688f98, (NullCallback)0) != 0) {
        unsigned int factionId = 1;
        int *local_8 = g_0094613c;
        int *puVar4 = g_0096edd0;
        do {
            puVar4[1] = 5;
            *puVar4 = 0;
            compute_faction_modifiers(factionId);

            int *puVar2 = puVar4 - 0x3f;
            int i = 4;
            do {
                puVar2[4] = 0;
                *puVar2 = 0;
                ++puVar2;
                --i;
            } while (i != 0);

            social_upkeep(factionId);
            int count = *g_009a64cc;
            puVar4[-0x53] = 0;
            puVar4[-0x54] = 0;
            *local_8 = 0;
            puVar4[-0x80] = 0;

            if (count > 0) {
                int *puVar3 = g_0097d078;
                int n = count;
                do {
                    if ((unsigned char)puVar3[-0xd] == factionId) {
                        *puVar3 = *puVar3 & 0x7fffffff;
                        puVar3[5] = 0;
                    }
                    puVar3 += 0x4d;
                    --n;
                } while (n != 0);
            }
            if (*g_009a64c8 > 0) {
                int *puVar3 = g_0095282c;
                int n = *g_009a64c8;
                do {
                    if (*((unsigned char *)puVar3 + 0xA) == factionId) {
                        *((unsigned char *)puVar3 + 0xD) = 0;
                        *puVar3 = *puVar3 & 0xfcffbdff;
                    }
                    puVar3 += 0xd;
                    --n;
                } while (n != 0);
            }
            ++factionId;
            ++local_8;
            puVar4 += 0x833;
        } while ((int)local_8 < 0x946158);
    }
}
