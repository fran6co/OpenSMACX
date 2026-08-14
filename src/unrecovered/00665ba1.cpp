// ORIGINAL: 0x00665BA1 EXCLUDED S2d
// name      sub_665ba1
// size      242 bytes
// spans     0x00665BA1-0x00665C93
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: was `EXCLUDED S2a`; same shape as 0x00664D51's sibling
//            (see that file's note in full) - the disassembly is a normal
//            `ret`, no tail-jmp, self-contained store sequence, not an EH
//            funclet by docs/EXCLUSIONS.md 2a's own test. Un-excluded.
// NOTE: still structurally capped, but for a different reason than S2a
//       claimed: tools/byte_match.py SELFMOD_RANGE (0x00664000,0x00669000)
//       covers this address too, so classify_body reports
//       REFUSED/"body has a span in _SELFMOD" for ANY body, content-
//       independent - same ceiling as 0x004C86D0's SHARED_TAIL. No
//       docs/EXCLUSIONS.md section cites it. Landed anyway for coverage.
// RULED-OUT: combining `value + g_9be6b4 + g_9be6b4` into one expression vs.
//            splitting into two statements - identical rebuilt bytes either
//            way (VC6 folds both into one `lea`/`mov` pair; original is two
//            separate `add`). Kept split as the more faithful source form.
//            First divergence #18 add/mov, mnemonic similarity 0.92, 4 edit
//            blocks, unrelated to this expression's semantics.

extern "C" {
    extern int g_9be6bc;
    extern int g_9c082c;
    extern int g_9be6a4;
    extern int g_9be6b4;
    extern int g_9be6a8;
    extern int g_9be6b0;

    extern int g_66581e;
    extern int g_66582a;
    extern int g_665a0b;
    extern int g_665a96;
    extern int g_665b61;
    extern int g_665ab0;
    extern int g_665b75;
    extern int g_66580e;
    extern int g_665818;
    extern int g_665a1e;
    extern int g_665a28;
    extern int g_665aba;
    extern int g_665ac4;
    extern int g_665b7f;
    extern int g_665b89;
    extern int g_665835;
    extern int g_66583f;
    extern int g_665ad2;
    extern int g_665adc;
    extern int g_665865;
    extern int g_66586f;
    extern int g_665b02;
    extern int g_665b0c;
    extern int g_66584d;
    extern int g_665857;
    extern int g_665aea;
    extern int g_665af4;
    extern int g_665824;
    extern int g_665a2e;
    extern int g_665aca;
    extern int g_665b8f;
    extern int g_665845;
    extern int g_665ae2;
    extern int g_665875;
    extern int g_665b12;
    extern int g_66585d;
    extern int g_665afa;
    extern int g_6657ee;
}

extern "C" int __cdecl sub_665ba1() {
    int value = g_9be6bc;
    g_66581e = value;
    g_66582a = value;
    g_665a0b = value;
    g_665a96 = value;
    g_665b61 = value;
    value = g_9c082c;
    g_665ab0 = value;
    g_665b75 = value;
    value = g_9be6a4;
    g_66580e = value;
    g_665818 = value;
    g_665a1e = value;
    g_665a28 = value;
    g_665aba = value;
    g_665ac4 = value;
    g_665b7f = value;
    g_665b89 = value;
    value = value + 2;
    g_665835 = value;
    g_66583f = value;
    g_665ad2 = value;
    g_665adc = value;
    value = value + g_9be6b4;
    value = value + g_9be6b4;
    g_665865 = value;
    g_66586f = value;
    g_665b02 = value;
    g_665b0c = value;
    value = value - 2;
    g_66584d = value;
    g_665857 = value;
    g_665aea = value;
    g_665af4 = value;
    value = g_9be6a8;
    g_665824 = value;
    g_665a2e = value;
    g_665aca = value;
    g_665b8f = value;
    value = value + 1;
    g_665845 = value;
    g_665ae2 = value;
    value = value + g_9be6b0;
    g_665875 = value;
    g_665b12 = value;
    value = value - 1;
    g_66585d = value;
    g_665afa = value;
    value = g_9be6b4;
    g_6657ee = value;
    return value;
}
