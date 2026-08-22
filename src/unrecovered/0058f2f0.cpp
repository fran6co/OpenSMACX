// ORIGINAL: 0x0058F2F0 ?game_init@@YAHHH@Z 0x0058F2F0-0x0058F421
// TRIED: original establishes a real EBP frame (flags: frame); this body compiles FPO (esp-relative) instead - same mnemonics through the first branch, diverging only in WHEN the `add esp,4` stack cleanup after `log_set_state` happens (original defers it to the shared join point at 0x58f3d9, this body does it immediately before the `jmp`). Landed at MISMATCH #18.
// size      305 bytes
// prototype BOOL (__cdecl ?game_init@@YAHHH@Z)(BOOL tglParseStrings, BOOL tglAllRules)
// callers   4   call targets   12
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00455EB0 0x0050F3D0 0x00538FB0 0x005873C0 0x006018A0 0x00601910 0x006168F0 0x006169A0 0x00616A00 0x006257E0 0x00626450 0x00645470
// indirect  0x0058F304 0x0058F30F 0x0058F38D 0x0058F3CD

// timeGetTime and CharUpperA come from the scaffold
// (emit_translation_unit.WIN32_IMPORTS), `__declspec(dllimport)` included
// so the calls keep the original's IAT-indirect shape.

extern "C" int __cdecl set_def_cancel_text(char *);
extern "C" int __cdecl set_def_ok_text(char *);

int __cdecl game_init(int a1, int a2) {
    *g_0068f21c = 1;
    my_srand((int)timeGetTime());
    random_reseed(timeGetTime());
    if (a1 == 0) {
        log_set_state(0);
    } else {
        Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
        strings->init(0xc320);
        if (labels_init() != 0) {
            return 1;
        }
        char *buf = reinterpret_cast<char *>(g_009b86a0);
        *buf = 0;
        char *pop = *reinterpret_cast<char **>(g_009b90f8);
        strcat(buf, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(pop + 0x198))));
        if (*g_009bc054 != 2) {
            CharUpperA(buf);
        }
        set_def_cancel_text(buf);

        *buf = 0;
        char *pop2 = *reinterpret_cast<char **>(g_009b90f8);
        strcat(buf, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(pop2 + 0x194))));
        CharUpperA(buf);
        set_def_ok_text(buf);
    }
    load_sprites();
    if (read_rules(a2) != 0) {
        return 1;
    }
    start_timers();
    *g_009b7b00 = 0x0075ae4c;
    *g_009b7b04 = 0x0075ae78;
    *g_009b7b0c = 0x0075ae20;
    return 0;
}
