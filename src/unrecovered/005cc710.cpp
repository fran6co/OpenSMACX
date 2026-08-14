// ORIGINAL: 0x005CC710
// name      sub_5cc710
// size      326 bytes
// spans     0x005CC710-0x005CC856
// prototype
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005CBAE0 0x00644EF2
// indirect  0x005CC7F2 0x005CC80E 0x005CC82A 0x005CC842
// PROPOSAL: extern "C" int __cdecl sub_5cc710() -> a __thiscall member, no
//           free params - evidence: ecx used immediately at entry for
//           field reads (this + 0x3c, this + 0x648, ...). Expressed via an
//           introduced `Sub5cc710Shim` class, class-close and out-of-line
//           definition sharing one line for the same reason as 0x005D50F0.
// RULED-OUT: the four vtable-slot-2 calls read `push piVar1; mov ecx,
//            [piVar1]; call dword ptr [ecx+8]` - a plain function-pointer
//            call taking one explicit pointer argument, NOT a thiscall
//            dispatch (the receiver never lands in ecx before the call),
//            so these are cdecl function-pointer casts, not virtual calls
//            through an introduced class. `sub_5cbae0` similarly cannot be
//            called by name (catalogued unmangled with no CRT_SIGNATURES
//            entry, so declfix leaves it nullary - C2660 direct, C2733
//            redeclared) and the real call passes `this` in ecx and a
//            literal 0 in edx with no stack push, i.e. __fastcall(void*,
//            int); routed through a __fastcall function-pointer cast.
//            Best reached: MISMATCH, edit_count 4, 338 bytes vs 326,
//            mnemonic similarity ~0.98.
class Sub5cc710Shim {
public:
    void run();
}; void Sub5cc710Shim::run() {
    typedef void (__cdecl *Slot2Fn)(void *);
    char *base = (char *)this;
    char *obj;
    void *piVar1;
    int vt;

    if (*(int *)(base + 0x3c) != 0) {
        if (*(int *)(base + 0x648) != 0) { free(*(void **)(base + 0x648)); *(int *)(base + 0x648) = 0; }
        if (*(int *)(base + 0x64c) != 0) { free(*(void **)(base + 0x64c)); *(int *)(base + 0x64c) = 0; }
        if (*(int *)(base + 0x650) != 0) { free(*(void **)(base + 0x650)); *(int *)(base + 0x650) = 0; }
        if (*(int *)(base + 0x638) != 0) { free(*(void **)(base + 0x638)); *(int *)(base + 0x638) = 0; }
        if (*(int *)(base + 0x63c) != 0) { free(*(void **)(base + 0x63c)); *(int *)(base + 0x63c) = 0; }
        if (*(int *)(base + 0x644) != 0) { free(*(void **)(base + 0x644)); *(int *)(base + 0x644) = 0; }
        if (*(int *)(base + 0x640) != 0) { free(*(void **)(base + 0x640)); *(int *)(base + 0x640) = 0; }
        if (*(int *)(base + 0x4c) == 2) {
            typedef int (__fastcall *Fn5cbae0)(void *, int);
            ((Fn5cbae0)sub_5cbae0)(this, 0);
        }
        obj = *(char **)base;
        piVar1 = *(void **)(obj + 0x24);
        if (piVar1 != 0 && (*(unsigned int *)(obj + 4) & 0x4000000) == 0) {
            vt = *(int *)piVar1;
            ((Slot2Fn)(*(int *)(vt + 8)))(piVar1);
            obj = *(char **)base;
            *(int *)(obj + 0x24) = 0;
        }
        obj = *(char **)base;
        piVar1 = *(void **)(obj + 0x20);
        if (piVar1 != 0 && (*(unsigned int *)(obj + 4) & 0x800) == 0) {
            vt = *(int *)piVar1;
            ((Slot2Fn)(*(int *)(vt + 8)))(piVar1);
            obj = *(char **)base;
            *(int *)(obj + 0x20) = 0;
        }
        obj = *(char **)base;
        piVar1 = *(void **)(obj + 0x1c);
        if (piVar1 != 0 && (*(unsigned int *)(obj + 4) & 0x800) == 0) {
            vt = *(int *)piVar1;
            ((Slot2Fn)(*(int *)(vt + 8)))(piVar1);
            obj = *(char **)base;
            *(int *)(obj + 0x1c) = 0;
        }
        if (*(int *)(base + 0x3d4) != 0) {
            obj = *(char **)base;
            piVar1 = *(void **)(obj + 0x10);
            vt = *(int *)piVar1;
            ((Slot2Fn)(*(int *)(vt + 8)))(piVar1);
            obj = *(char **)base;
            *(int *)(obj + 0x10) = 0;
            *(int *)(base + 0x3d4) = 0;
        }
        *(int *)(base + 0x3c) = 0;
    }
}
