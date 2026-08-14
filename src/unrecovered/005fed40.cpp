// ORIGINAL: 0x005FED40
// name      ?create_table@Palette@@QAEHPAEHHH@Z
// size      312 bytes
// spans     0x005FED40-0x005FEE78
// prototype int (__thiscall ?create_table@Palette@@QAEHPAEHHH@Z)(Palette* this, unsigned int8*, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FF630 0x00628DB0 0x00645550
// RULED-OUT: manual dword-by-dword copy loop for the HSV struct (compiler
//            reordered vs. the `rep movsd` struct-assignment form used
//            below); computing `&hsv[0]` at the call site instead of before
//            the loop. Best reached: MISMATCH, edit_count 4/~110 mnemonics,
//            313 bytes vs 312 - one `fmul qword ptr [addr]` compiled as
//            `mov eax,addr; fmul qword ptr [eax]` instead of the direct
//            absolute form, apparently because `&hsv[0]`'s address happened
//            to occupy eax at that point in the original and not here.
int Palette::create_table(unsigned char * a1, int a2, int a3, int a4) {
    struct HSVLocal { double h; double s; double v; };
    HSVLocal hsv[256];
    HSVLocal local1;
    HSVLocal *ref0;
    int i;
    int upper;

    if (*g_009b8178 == 0) {
        return 7;
    }
    if (a1 == 0) {
        return 0x10;
    }
    if (a4 < -100 || a4 > 100) {
        return 3;
    }
    for (i = 0; i < a2; i++) {
        a1[i] = (unsigned char)i;
    }
    upper = a2 + a3;
    for (i = upper; i < 0x100; i++) {
        a1[i] = (unsigned char)i;
    }
    for (i = 0; i < 0x100; i++) {
        RGB_to_HSV((PALETTEENTRY *)((unsigned char *)this + i * 4), (HSV *)&hsv[i]);
    }
    ref0 = &hsv[0];
    for (i = a2; i < upper; i++) {
        local1 = hsv[i];
        local1.v = (double)(a4 + 100) * local1.v * *(double *)g_0066eb38;
        a1[i] = (unsigned char)get_nearest_palette_index((HSV *)&local1, (HSV *)ref0, 1);
    }
    return 0;
}
