// ORIGINAL: 0x0050E890 ?auto_play_callback@@YAXH@Z 0x0050E890-0x0050E97E
// size      238 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005169F0 0x00625810
// indirect  0x0050E8FE 0x0050E90F 0x0050E939 0x0050E968 0x0050E978
// FRAME SHAPE. This body used to diverge at mnemonic #0 - `push ecx`, a
// stack slot the original does not have - and now diverges at #64 of 80,
// with 79 mnemonics in common. Three source-form facts got it there, all
// measured here on cl 12.00.8168 rather than reasoned about:
//
//   1. THE PostMessageA POINTER IS A LOCAL IN THE KEYSTROKE BRANCH. Spelled
//      inline, VC6 emits `call dword ptr [0x669314]` twice and needs no
//      callee-saved register for it; the original's `mov edi, [0x669314]`
//      plus two `call edi` is a value held in a register ACROSS a call,
//      which only a local gets. That is what makes EDI live and puts
//      `push esi; push edi` at 0x0050E8C6 instead of at the entry.
//   2. THE PACKED WORD IS AN EXPRESSION, NOT A LOCAL, written out at both
//      call sites. Naming it `int packed = ...` gives it a whole-branch
//      register home and costs a stack slot - `push ecx` at byte 0, the
//      old divergence - in every one of the twelve pack spellings tried.
//   3. THE MASK SPELLING DECIDES THE REGISTER ASSIGNMENT, which is the
//      non-obvious part. `(y & 0xff) << 8 | x & 0xff` lowers to two byte
//      inserts, `mov ch, <y>` and `mov cl, <x>`, and a byte insert needs a
//      byte-addressable register. EBX is the only callee-saved one, so x
//      takes it and y is spilled to the stack. Casting only the y half to
//      `unsigned short` makes VC6 lower x's half as a 32-bit `and esi, 0xff`
//      followed by `or`, which frees x to live in ESI and y to take EBX -
//      the original's allocation - and reproduces
//      `xor ecx,ecx / mov ch,bl / and esi,0xff / and ecx,0xffff /
//      mov edi,ecx / or edi,esi` instruction for instruction. The
//      expression is MAKEWORD(x, y); the redundant-looking `and ecx,0xffff`
//      is that macro's `(WORD)` cast surviving into the output.
//
// RULED-OUT, all re-measured rather than inherited: signed against unsigned
// x and y (no effect); thirteen pack spellings, of which only the one below
// reaches #64 and the same cast placed outside the OR falls back to #16;
// declaring the locals at the top of the function; sharing one set of locals
// between the two branches; computing the pack before SetCursorPos (that
// also loses the frame, but reorders the pack ahead of the call and stops at
// #52); and, for the pointer, `register`, `const`, a nested scope, a
// function-scope variable assigned in both branches, assignment inside the
// first call's callee expression, a `void **` read, a const-qualified or
// `extern`-declared global, and overwriting x's own storage with it.
//
// WHAT IS LEFT is one instruction: the original loads the pointer into ESI
// AFTER the pack - `mov esi, [0x669314]`, reusing the register x vacated -
// and calls `call esi` twice. Every spelling that gives that load a named
// local makes VC6 schedule it BEFORE the mask, where ESI is still busy, so
// it claims a fourth callee-saved register: EDI (divergence #60, the pack
// then landing in ESI) or, for this pack spelling, EBP (#46). Reproduced
// here: 0x0050E890 through 0x0050E959.

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
        int key = (signed char)random(0, 0xff);
        PostMessageAFn post = *reinterpret_cast<PostMessageAFn *>(g_00669314);
        post(reinterpret_cast<void *>(*g_009b7b28), 0x100, key, 0);
        post(reinterpret_cast<void *>(*g_009b7b28), 0x101, key, 0);
        return;
    }

    int x = random(0, *g_009b7b1c);
    int y = random(0, *g_009b7b20);
    (*reinterpret_cast<SetCursorPosFn *>(g_00669310))(x, y);

    (*reinterpret_cast<PostMessageAFn *>(g_00669314))(
        reinterpret_cast<void *>(*g_009b7b28), 0x201, 0,
        (unsigned short)((y & 0xff) << 8) | (x & 0xff));
    (*reinterpret_cast<PostMessageAFn *>(g_00669314))(
        reinterpret_cast<void *>(*g_009b7b28), 0x202, 0,
        (unsigned short)((y & 0xff) << 8) | (x & 0xff));
}
