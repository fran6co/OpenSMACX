// ORIGINAL: 0x00666989 sub_666989 0x00666989-0x00666A7D EXCLUDED S2d
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
//       ANY body (confirmed directly against this exact candidate via
//       byte_match.match_function, not just inferred). Landed anyway for
//       coverage: it compiles and is a faithful transcription of the
//       disassembly/Ghidra decompilation.

extern "C" {
    extern int g_9be6bc;
    extern int g_9c0a18;
    extern int g_9c09f8;
    extern int g_9be6b0;
    extern int g_9be6a4;
    extern int g_9be6b4;
    extern int g_9be6a8;

    extern int g_666620;
    extern int g_666630;
    extern int g_666810;
    extern int g_666894;
    extern int g_66694f;
    extern int g_666808;
    extern int g_6668ae;
    extern int g_666966;
    extern int g_66682a;
    extern int g_6668bf;
    extern int g_666977;
    extern int g_6668d0;
    extern int g_6668d7;
    extern int g_6668e0;
    extern int g_6668ee;
    extern int g_6668f7;
    extern int g_6668fe;
    extern int g_6668e7;
    extern int g_66660e;
    extern int g_666618;
    extern int g_66663b;
    extern int g_666645;
    extern int g_66666f;
    extern int g_666679;
    extern int g_666655;
    extern int g_66665f;
    extern int g_666628;
    extern int g_66664d;
    extern int g_666681;
    extern int g_666667;
    extern int g_6665ee;
}

extern "C" int __cdecl sub_666989() {
    int value = g_9be6bc;
    g_666620 = value;
    g_666630 = value;
    g_666810 = value;
    g_666894 = value;
    g_66694f = value;
    value = g_9c0a18;
    g_666808 = value;
    g_6668ae = value;
    g_666966 = value;
    value = g_9c09f8;
    g_66682a = value;
    g_6668bf = value;
    g_666977 = value;
    value = value + 2;
    g_6668d0 = value;
    value = g_9be6b0;
    value = value * 2;
    g_6668d7 = value;
    g_6668e0 = value;
    value = value + 2;
    g_6668ee = value;
    g_6668f7 = value;
    value = value + g_9c09f8;
    g_6668fe = value;
    value = value - 2;
    g_6668e7 = value;
    value = g_9be6a4;
    g_66660e = value;
    g_666618 = value;
    value = value + 2;
    g_66663b = value;
    g_666645 = value;
    value = value + g_9be6b4;
    value = value + g_9be6b4;
    g_66666f = value;
    g_666679 = value;
    value = value - 2;
    g_666655 = value;
    g_66665f = value;
    value = g_9be6a8;
    g_666628 = value;
    value = value + 2;
    g_66664d = value;
    value = value + g_9be6b0;
    value = value + g_9be6b0;
    g_666681 = value;
    value = value - 2;
    g_666667 = value;
    value = g_9be6b4;
    g_6665ee = value;
    return value;
}
