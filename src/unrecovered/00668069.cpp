// ORIGINAL: 0x00668069 sub_668069 0x00668069-0x0066815D EXCLUDED S2d
// TRIED: was `EXCLUDED S2a`; disassembly is a normal `ret`, no tail-jmp, self-contained store sequence - not an EH funclet by docs/EXCLUSIONS.md 2a's own test (see 0x00664D51's note in full). Un-excluded.
// size      244 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// NOTE: content-independent REFUSED either way: tools/byte_match.py
//       SELFMOD_RANGE (0x00664000,0x00669000) covers this address, so
//       `classify_body` reports REFUSED/"body has a span in _SELFMOD" for
//       ANY body here (same population as its sibling 0x00666989). Landed
//       anyway for coverage: it compiles and is a faithful transcription of
//       the disassembly/Ghidra decompilation.

extern "C" {
    extern int g_9be6bc;
    extern int g_9c0a18;
    extern int g_9c09f8;
    extern int g_9be6b0;
    extern int g_9be6a4;
    extern int g_9be6b4;
    extern int g_9be6a8;

    extern int g_667d00;
    extern int g_667d10;
    extern int g_667ef0;
    extern int g_667f74;
    extern int g_66802f;
    extern int g_667ee8;
    extern int g_667f8e;
    extern int g_668046;
    extern int g_667f0a;
    extern int g_667f9f;
    extern int g_668057;
    extern int g_667fb0;
    extern int g_667fb7;
    extern int g_667fc0;
    extern int g_667fce;
    extern int g_667fd7;
    extern int g_667fde;
    extern int g_667fc7;
    extern int g_667cee;
    extern int g_667cf8;
    extern int g_667d1b;
    extern int g_667d25;
    extern int g_667d4f;
    extern int g_667d59;
    extern int g_667d35;
    extern int g_667d3f;
    extern int g_667d08;
    extern int g_667d2d;
    extern int g_667d61;
    extern int g_667d47;
    extern int g_667cce;
}

extern "C" int __cdecl sub_668069() {
    int value = g_9be6bc;
    g_667d00 = value;
    g_667d10 = value;
    g_667ef0 = value;
    g_667f74 = value;
    g_66802f = value;
    value = g_9c0a18;
    g_667ee8 = value;
    g_667f8e = value;
    g_668046 = value;
    value = g_9c09f8;
    g_667f0a = value;
    g_667f9f = value;
    g_668057 = value;
    value = value + 2;
    g_667fb0 = value;
    value = g_9be6b0;
    value = value * 2;
    g_667fb7 = value;
    g_667fc0 = value;
    value = value + 2;
    g_667fce = value;
    g_667fd7 = value;
    value = value + g_9c09f8;
    g_667fde = value;
    value = value - 2;
    g_667fc7 = value;
    value = g_9be6a4;
    g_667cee = value;
    g_667cf8 = value;
    value = value + 2;
    g_667d1b = value;
    g_667d25 = value;
    value = value + g_9be6b4;
    value = value + g_9be6b4;
    g_667d4f = value;
    g_667d59 = value;
    value = value - 2;
    g_667d35 = value;
    g_667d3f = value;
    value = g_9be6a8;
    g_667d08 = value;
    value = value + 2;
    g_667d2d = value;
    value = value + g_9be6b0;
    value = value + g_9be6b0;
    g_667d61 = value;
    value = value - 2;
    g_667d47 = value;
    value = g_9be6b4;
    g_667cce = value;
    return value;
}
