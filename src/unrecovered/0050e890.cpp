// ORIGINAL: 0x0050E890
// name      ?auto_play_callback@@YAXH@Z
// size      238 bytes
// spans     0x0050E890-0x0050E97E
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005169F0 0x00625810
// indirect  0x0050E8FE 0x0050E90F 0x0050E939 0x0050E968 0x0050E978
// RULED-OUT: several source forms for the 0x9b7b1c/0x9b7b20 -> SetCursorPos
// -> packed-word PostMessage tail all compile to a framed function
// (`push ebp; mov ebp,esp; push ecx` and matching teardown) where the
// original is frameless throughout - tried: signed vs unsigned int locals
// for x/y; a named vs inlined PostMessageA/SetCursorPos function-pointer
// variable; nested `if`s instead of a chained `&&` guard; and an explicit
// byte-store union for the `(y&0xff)<<8 | x&0xff` pack (matches the
// original's `mov ch,bl` trick even less well, since &local forces a real
// stack slot). None reproduced the frameless three-register (ebx/esi/edi)
// allocation the original uses; landing the closest (arithmetic pack,
// inlined function-pointer casts) at mnemonic_similarity 0.96.

typedef int (__stdcall *PostMessageAFn)(void *, unsigned int, int, int);
typedef int (__stdcall *SetCursorPosFn)(int, int);

void __cdecl auto_play_callback(int a1) {
    *g_00939fd0 = *g_00939fd0 + 1;
    if (*g_00939fd0 > 0x64 && *g_009b7ae4 == 0 &&
        (*reinterpret_cast<unsigned char *>(g_009a64c0) & 2) != 0) {
        reinterpret_cast<Console *>(g_009156b0)->end_my_turn();
        *g_00939fd0 = 0;
    }

    if (random(0, 2) != 0) {
        unsigned int r = random(0, 0xff);
        int val = (signed char)(unsigned char)r;
        (*reinterpret_cast<PostMessageAFn *>(g_00669314))(
            reinterpret_cast<void *>(*g_009b7b28), 0x100, val, 0);
        (*reinterpret_cast<PostMessageAFn *>(g_00669314))(
            reinterpret_cast<void *>(*g_009b7b28), 0x101, val, 0);
        return;
    }

    unsigned int x = random(0, *g_009b7b1c);
    unsigned int y = random(0, *g_009b7b20);
    (*reinterpret_cast<SetCursorPosFn *>(g_00669310))(x, y);

    unsigned int packed = (y & 0xff) << 8 | x & 0xff;
    (*reinterpret_cast<PostMessageAFn *>(g_00669314))(
        reinterpret_cast<void *>(*g_009b7b28), 0x201, 0, packed);
    (*reinterpret_cast<PostMessageAFn *>(g_00669314))(
        reinterpret_cast<void *>(*g_009b7b28), 0x202, 0, packed);
}
