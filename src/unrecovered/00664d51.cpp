// ORIGINAL: 0x00664D51 sub_664d51 0x00664D51-0x00664E43 EXCLUDED S2d
// TRIED: this was carried as `EXCLUDED S2a` (C++ EH unwind funclet) by a prior bulk pass, but the disassembly contradicts that ground: docs/EXCLUSIONS.md 2a's own definition is "no frame of its own ... no prologue, no epilogue, no ret - a tail jmp into another symbol's entry", and this function has a normal `ret`, no tail-jmp into anyone else's entry point, and is fully self-contained (reads six globals, republishes them across table records, returns). It was swept into the funclet population by proximity - it sits in the byte gap between two much larger EH-funclet-bearing functions (0x006647F0 and 0x00664E50) - not by matching the population's own test. Un-excluded here so the body below can be measured.
// TRIED: combining the two `+= g_9be6b4` adds into one expression (`value + g_9be6b4 + g_9be6b4`) vs. splitting them into two statements - VC6 lowers both to the same `lea`/`mov` pair either way (original is two separate `add`); tried both, identical rebuilt bytes. Kept split as the more faithful source form. First divergence #18 add/mov (mnemonic similarity 0.92, 4 edit blocks, none of them affecting this expression's semantics).
// size      242 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// NOTE: the address IS structurally capped regardless, but for a DIFFERENT,
//       undocumented reason: tools/byte_match.py's SELFMOD_RANGE is the flat
//       interval (0x00664000, 0x00669000) - "copy protection" - and this
//       whole function sits inside it, so `classify_body` reports
//       `tier: REFUSED, "body has a span in _SELFMOD"` for ANY body here,
//       content-independent, the same way 0x004C86D0 is capped at
//       SHARED_TAIL regardless of content. No docs/EXCLUSIONS.md section
//       cites SELFMOD_RANGE at all (grepped, zero hits) - the real ground
//       for every address in 0x664000-0x669000 is undocumented, and S2a is
//       the wrong citation for it. Landed anyway per the coverage brief:
//       compiles and is a faithful transcription, which is what a REFUSED
//       ceiling still leaves worth banking.

extern "C" {
    extern int g_9be6bc;
    extern int g_9c082c;
    extern int g_9be6a4;
    extern int g_9be6b4;
    extern int g_9be6a8;
    extern int g_9be6b0;

    extern int g_6649ce;
    extern int g_6649da;
    extern int g_664bbb;
    extern int g_664c46;
    extern int g_664d11;
    extern int g_664c60;
    extern int g_664d25;
    extern int g_6649be;
    extern int g_6649c8;
    extern int g_664bce;
    extern int g_664bd8;
    extern int g_664c6a;
    extern int g_664c74;
    extern int g_664d2f;
    extern int g_664d39;
    extern int g_6649e5;
    extern int g_6649ef;
    extern int g_664c82;
    extern int g_664c8c;
    extern int g_664a15;
    extern int g_664a1f;
    extern int g_664cb2;
    extern int g_664cbc;
    extern int g_6649fd;
    extern int g_664a07;
    extern int g_664c9a;
    extern int g_664ca4;
    extern int g_6649d4;
    extern int g_664bde;
    extern int g_664c7a;
    extern int g_664d3f;
    extern int g_6649f5;
    extern int g_664c92;
    extern int g_664a25;
    extern int g_664cc2;
    extern int g_664a0d;
    extern int g_664caa;
    extern int g_66499e;
}

extern "C" int __cdecl sub_664d51() {
    int value = g_9be6bc;
    g_6649ce = value;
    g_6649da = value;
    g_664bbb = value;
    g_664c46 = value;
    g_664d11 = value;
    value = g_9c082c;
    g_664c60 = value;
    g_664d25 = value;
    value = g_9be6a4;
    g_6649be = value;
    g_6649c8 = value;
    g_664bce = value;
    g_664bd8 = value;
    g_664c6a = value;
    g_664c74 = value;
    g_664d2f = value;
    g_664d39 = value;
    value = value + 2;
    g_6649e5 = value;
    g_6649ef = value;
    g_664c82 = value;
    g_664c8c = value;
    value = value + g_9be6b4;
    value = value + g_9be6b4;
    g_664a15 = value;
    g_664a1f = value;
    g_664cb2 = value;
    g_664cbc = value;
    value = value - 2;
    g_6649fd = value;
    g_664a07 = value;
    g_664c9a = value;
    g_664ca4 = value;
    value = g_9be6a8;
    g_6649d4 = value;
    g_664bde = value;
    g_664c7a = value;
    g_664d3f = value;
    value = value + 1;
    g_6649f5 = value;
    g_664c92 = value;
    value = value + g_9be6b0;
    g_664a25 = value;
    g_664cc2 = value;
    value = value - 1;
    g_664a0d = value;
    g_664caa = value;
    value = g_9be6b4;
    g_66499e = value;
    return value;
}
