// ORIGINAL: 0x005CEF8A sub_5cef8a 0x005CEF8A-0x005CF11D
// TRIED: each of the 15 calls is preceded by a `mov esi, <literal table address>` the sibling 0x005CED1D confirmed is a genuine hidden ESI input (Ghidra flags `unaff_ESI` there), not reproducible without inline asm; this transcribes every visible global store (DAT_009c3000/DAT_009c3004) and the call sequence and drops only the ESI loads, landing at 0.51 similarity.
// size      403 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005CED1D 0x005CEE80

extern "C" void __cdecl sub_5cef8a(int a1, int a2) {
    *g_009c3000 = a2 << 2;

    *g_009c3004 = reinterpret_cast<int>(g_009c302c);
    sub_5ced1d();
    *g_009c3004 = reinterpret_cast<int>(g_009c3030);
    sub_5ced1d();
    *g_009c3004 = reinterpret_cast<int>(g_009c3034);
    sub_5ced1d();
    *g_009c3004 = reinterpret_cast<int>(g_009c3038);
    sub_5ced1d();
    *g_009c3004 = reinterpret_cast<int>(g_009c303c);
    sub_5ced1d();
    *g_009c3004 = reinterpret_cast<int>(g_009c3040);
    sub_5ced1d();
    *g_009c3004 = reinterpret_cast<int>(g_009c3044);
    sub_5ced1d();
    *g_009c3004 = reinterpret_cast<int>(g_009c3048);
    sub_5ced1d();

    *g_009c3004 = a1;
    sub_5cee80();
    *g_009c3004 = *g_009c3004 + *g_009c3000;
    sub_5cee80();
    *g_009c3004 = *g_009c3004 + *g_009c3000;
    sub_5cee80();
    *g_009c3004 = *g_009c3004 + *g_009c3000;
    sub_5cee80();
    *g_009c3004 = *g_009c3004 + *g_009c3000;
    sub_5cee80();
    *g_009c3004 = *g_009c3004 + *g_009c3000;
    sub_5cee80();
    *g_009c3004 = *g_009c3004 + *g_009c3000;
    sub_5cee80();
    *g_009c3004 = *g_009c3004 + *g_009c3000;
    sub_5cee80();
}
