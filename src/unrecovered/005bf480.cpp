// ORIGINAL: 0x005BF480
// name      ?X_pop@@YAHPADPBDHPADHP6AHXZ@Z
// size      322 bytes
// spans     0x005BF480-0x005BF5C2
// prototype int (__cdecl ?X_pop@@YAHPADPBDHPADHP6AHXZ@Z)(int8*, int8*, int length, int8*, int, int (__cdecl *)())
// callers   9   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006272C0 0x00645598 0x00645646 0x00645660
// RULED-OUT: calling fopen/fclose directly by name - the catalogue records
//            them unmangled with no CRT_SIGNATURES entry, so declfix leaves
//            them nullary (C2660) and a local redeclaration is C2733
//            (second C linkage); routed through function-pointer casts
//            instead, mirroring 0x005BFBC0 (X_pop_ask)'s Ghidra shape for
//            the branch structure - bit clear on the per-mod flag table
//            takes the 0x93f7cc-range-checked alienuscript path
//            (0x691ca4/0x691ca8, pop via 0x691b24); bit set takes the
//            simple path (0x691c8c/0x691c90, pop via 0x691b20).
//            Best reached: MISMATCH, edit_count 5, 398 bytes vs 322,
//            mnemonic similarity ~0.83.
int __cdecl X_pop(char * a1, const char * a2, int a3, char * a4, int a5, int (__cdecl *a6)()) {
    typedef void *(__cdecl *FopenFn)(const char *, const char *);
    typedef int (__cdecl *FcloseFn)(void *);
    FopenFn real_fopen = (FopenFn)fopen;
    FcloseFn real_fclose = (FcloseFn)fclose;
    void *fp;
    int result;

    if (strcmp(a1, *(char **)g_00691b0c) == 0 || strcmp(a1, (char *)g_00691c80) == 0) {
        if ((((char *)g_00946f58)[*g_00939284 * 0x59c] & 0x80) != 0) {
            if (*g_009a6488 != 0) {
                fp = real_fopen((char *)g_00691c90, (char *)g_00691c8c);
                if (fp != 0) {
                    real_fclose(fp);
                    result = pop(*(char **)g_00691b20, (char *)a2, a3, a4, a5, a6);
                    if (result != -1) {
                        return result;
                    }
                }
            }
        } else if (*g_009a6488 != 0 && 0 < *g_0093f7cc && *g_0093f7cc < 8 &&
                   (((char *)g_00946f58)[*g_0093f7cc * 0x59c] & 0x80) != 0) {
            fp = real_fopen((char *)g_00691ca8, (char *)g_00691ca4);
            if (fp != 0) {
                real_fclose(fp);
                result = pop(*(char **)g_00691b24, (char *)a2, a3, a4, a5, a6);
                if (result != -1) {
                    return result;
                }
            }
        }
        return pop(a1, (char *)a2, a3, a4, a5, a6);
    }
    return pop(a1, (char *)a2, a3, a4, a5, a6);
}
