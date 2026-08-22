// ORIGINAL: 0x006141C0 ?set_def_name@FileWin@@QAEHPAD@Z 0x006141C0-0x006142C9
// TRIED: plain `strlen`/`strcat` declarations get inlined by /O2 (repnz scasb / rep movs), which the original does not do at this call site; the `_strlen`/`_strcat` no-arg extern + function-pointer cast idiom (see src/unrecovered/00614d90.cpp) forces a real call and got the prologue and the backslash-search loop byte-exact (a `while` loop, not `do/while`, avoided a loop rotation the original does not have). Still MISMATCH: the drive-letter append block (`dirEnd[1]==':'`) converges back to the trailing strlen/strcat with a duplicated block reached via `jmp` in the original, but this source form falls straight through instead - a block-layout choice, not an arithmetic one.
// size      265 bytes
// prototype int (__thiscall ?set_def_name@FileWin@@QAEHPAD@Z)(FileWin* this, int8*)
// callers   5   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645470

extern "C" int __cdecl _strlen();
typedef unsigned int (__cdecl *StrlenFn)(const char *);
extern "C" int __cdecl _strcat();
typedef char *(__cdecl *StrcatFn)(char *, const char *);

int FileWin::set_def_name(char * a1) {
    char local_104[0x104];
    char *pcVar2;
    char *pcVar3;
    char *dirEnd;
    if (a1 == 0) {
        field_410_ = 0;
        return 0;
    }
    if (reinterpret_cast<StrlenFn>(_strlen)(a1) > 0x104) {
        return 3;
    }
    local_104[0] = '\0';
    reinterpret_cast<StrcatFn>(_strcat)(local_104, a1);
    pcVar2 = local_104 + reinterpret_cast<StrlenFn>(_strlen)(local_104);
    while (pcVar2 != local_104) {
        if (*pcVar2 == '\\') {
            if (pcVar2 == local_104) break;
            pcVar3 = pcVar2 + 1;
            *pcVar2 = '\0';
            dirEnd = local_104;
            goto have_filename;
        }
        pcVar2 = pcVar2 - 1;
    }
    pcVar3 = pcVar2;
    dirEnd = 0;
have_filename:
    field_410_ = 0;
    reinterpret_cast<StrcatFn>(_strcat)((char *)&field_410_, pcVar3);
    if (dirEnd != 0) {
        if (dirEnd[1] == ':') {
            reinterpret_cast<StrcatFn>(_strcat)(dirEnd, (char *)g_00697134);
        }
        if (reinterpret_cast<StrlenFn>(_strlen)(dirEnd) <= 0x104) {
            field_531_ = 0;
            reinterpret_cast<StrcatFn>(_strcat)((char *)&field_531_, dirEnd);
        }
        return 0;
    }
    field_531_ = 0;
    return 0;
}
