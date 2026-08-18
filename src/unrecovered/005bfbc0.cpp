// ORIGINAL: 0x005BFBC0 ?X_pop_ask@@YAHPADPBDHPADP6AHXZH@Z 0x005BFBC0-0x005BFD02 BYTE_EXACT
// LEVER: crt-by-pointer same shape as 0x005BF480 (X_pop): fopen/fclose by name is C2660 (nullary via declfix, not in CRT_SIGNATURES) and a local redeclaration is C2733, so both go through function-pointer casts. Branch structure follows this address's own Ghidra decompilation directly, which was available here and not for 0x005BF480: per-mod flag bit clear takes the range-checked alienuscript path (0x691d1c/0x691d20, pop_ask via 0x691b24); bit set takes the simple path (0x691d04/0x691d08, pop_ask via 0x691b20).
// LEVER: string-routines-are-calls it sat at MISMATCH, 398 bytes against 322, similarity ~0.83, while the scaffold inlined its `strcmp` as an `/Oi` expansion the image does not have. The scaffold now emits `#pragma function(...)` below the declarations it names, and the body was already right.
// size      322 bytes
// prototype
// callers   5   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00627910 0x00645598 0x00645646 0x00645660
int __cdecl X_pop_ask(char * a1, const char * a2, int a3, char * a4, int (__cdecl *a5)(), int a6) {
    typedef void *(__cdecl *FopenFn)(const char *, const char *);
    typedef int (__cdecl *FcloseFn)(void *);
    FopenFn real_fopen = (FopenFn)fopen;
    FcloseFn real_fclose = (FcloseFn)fclose;
    void *fp;
    int result;

    if (strcmp(a1, *(char **)g_00691b0c) == 0 || strcmp(a1, (char *)g_00691cf8) == 0) {
        if ((((char *)g_00946f58)[*g_00939284 * 0x59c] & 0x80) != 0) {
            if (*g_009a6488 != 0) {
                fp = real_fopen((char *)g_00691d08, (char *)g_00691d04);
                if (fp != 0) {
                    real_fclose(fp);
                    result = pop_ask(*(char **)g_00691b20, (char *)a2, a3, a4, a5, a6);
                    if (result != -1) {
                        return result;
                    }
                }
            }
        } else if (*g_009a6488 != 0 && 0 < *g_0093f7cc && *g_0093f7cc < 8 &&
                   (((char *)g_00946f58)[*g_0093f7cc * 0x59c] & 0x80) != 0) {
            fp = real_fopen((char *)g_00691d20, (char *)g_00691d1c);
            if (fp != 0) {
                real_fclose(fp);
                result = pop_ask(*(char **)g_00691b24, (char *)a2, a3, a4, a5, a6);
                if (result != -1) {
                    return result;
                }
            }
        }
        return pop_ask(a1, (char *)a2, a3, a4, a5, a6);
    }
    return pop_ask(a1, (char *)a2, a3, a4, a5, a6);
}
