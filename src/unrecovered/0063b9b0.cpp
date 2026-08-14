// ORIGINAL: 0x0063B9B0
// name      ?sin@@YAHHH@Z
// size      267 bytes
// spans     0x0063B9B0-0x0063BABB
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: naming the masked angle (`a1 & 0x3fffffff`) once and reusing it
//            for both the table index and the interpolation fraction got the
//            `and`/`sar` sequence byte-identical to the original (88.0%
//            mnemonic agreement, 266 vs 267 bytes). The remaining divergence
//            is that the original only ever pushes/pops `esi` across the
//            table lookup, while this form makes the compiler want a second
//            saved register (`edi`) to hold both table values live at once;
//            not chased further into register-allocation order.

int __cdecl sin(int a1, int a2) {
    if (a1 & 0x80000000) {
        a2 = -a2;
    }
    unsigned int highBit = (unsigned int)a1 & 0x40000000;
    int frac = a1 & 0x3fffffff;
    int index = frac >> 22;
    if (a2 < 0xffff) {
        int lo = g_009be6dc[index];
        int hi = g_009be6e0[index];
        if (highBit != 0) {
            return (((((hi - lo) * (frac & 0x3fffff)) >> 22) - lo) + 0xffff) * a2 >> 16;
        }
        return (((((hi - lo) * (frac & 0x3fffff)) >> 22) + lo)) * a2 >> 16;
    }
    if (a2 < 0xffffff) {
        int lo = g_009be6dc[index];
        int hi = g_009be6e0[index];
        if (highBit != 0) {
            return (((((hi - lo) * (frac & 0x3fffff)) >> 22) - lo) + 0xffff) * (a2 >> 8) >> 8;
        }
        return (((((hi - lo) * (frac & 0x3fffff)) >> 22) + lo)) * (a2 >> 8) >> 8;
    }
    int lo = g_009be6dc[index];
    int hi = g_009be6e0[index];
    if (highBit != 0) {
        return (((((hi - lo) * (frac & 0x3fffff)) >> 22) - lo) + 0xffff) * (a2 >> 16);
    }
    return (((((hi - lo) * (frac & 0x3fffff)) >> 22) + lo)) * (a2 >> 16);
}
