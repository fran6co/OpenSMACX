// ORIGINAL: 0x00638DE0
// name      sub_638de0
// size      330 bytes
// spans     0x00638DE0-0x00638F2A
// prototype
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00638620 0x00638650 0x00639390
// indirect  0x00638E3E
// PROPOSAL: extern "C" int __cdecl sub_638de0() -> takes 4 stack ints
//           (int a1, int *a2, int a3, int a4), matching both this
//           address's own Ghidra decompilation and IDA's arity guess -
//           every read is at a fixed [esp+N] before any push, never ecx,
//           so it is a plain cdecl free function, just with an empty
//           parameter list in the given head.
// RULED-OUT: calling sub_638620/sub_638650/sub_639390 by name - all three
//            are catalogued unmangled with no CRT_SIGNATURES entry, so
//            declfix leaves them nullary (C2660 direct, C2733 if
//            redeclared); routed through function-pointer casts. The
//            fallback branch's indirect call goes through
//            `*(int*)0x9c0b90`, a stored function pointer value (not a
//            vtable), called the same way. Sharing ONE `sub_638650` call
//            site between the 0x4040302/0x4040303 cases via goto (matching
//            the disassembly's single shared call at 0x638e99) was not
//            attempted - the two branches' locals would cross the jump
//            into each other's scope, which C++ refuses; each case is
//            expanded with its own call instead.
//            Best reached: MISMATCH, edit_count 17, 315 bytes vs 330,
//            mnemonic similarity ~0.67.
extern "C" int __cdecl sub_638de0(int a1, int *a2, int a3, int a4) {
    typedef int (__cdecl *Fn638650)(int, void *, int);
    typedef int (__cdecl *Fn638620)(int, void *);
    typedef void (__cdecl *Fn639390)(void *);
    typedef int (__cdecl *FnUnk)(int, int, int);
    Fn638650 real_638650 = (Fn638650)sub_638650;
    Fn638620 real_638620 = (Fn638620)sub_638620;
    Fn639390 real_639390 = (Fn639390)sub_639390;

    int local_8, local_4;
    int remaining = a3 - 8;
    int result;

    if (remaining == 0) {
        return 0;
    }
    do {
        result = real_638650(a1, &local_8, 8);
        if (result != 0) {
            return -1;
        }
        remaining = remaining - local_4;
        if (remaining < 0) {
            real_639390((void *)*g_006989d0);
            return -1;
        }
        if (local_8 == 0x4040301) {
            result = real_638620(a4, a2);
            if (result != 0) {
                return -1;
            }
            result = real_638650(a1, (void *)a2[0], a4);
            if (result != 0) {
                return -1;
            }
        } else if (local_8 == 0x4040302) {
            int size = a4 * 0xc;
            result = real_638620(size, a2 + 1);
            if (result != 0) {
                return -1;
            }
            result = real_638650(a1, (void *)a2[1], size);
            if (result != 0) {
                return -1;
            }
        } else if (local_8 == 0x4040303) {
            int size = a4 * 0x24;
            result = real_638620(size, a2 + 2);
            if (result != 0) {
                return -1;
            }
            result = real_638650(a1, (void *)a2[2], size);
            if (result != 0) {
                return -1;
            }
        } else {
            result = ((FnUnk)(*g_009c0b90))(a1, local_4 - 8, 1);
            if (result < 0) {
                real_639390((void *)*g_006989d0);
                return -1;
            }
        }
        if (remaining == 0) {
            return 0;
        }
    } while (true);
}
