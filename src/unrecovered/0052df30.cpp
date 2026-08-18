// ORIGINAL: 0x0052DF30 ?init@NetDaemon@@QAEHH@Z 0x0052DF30-0x0052E041
// RULED-OUT: `if (setup(a1)==0) return 0; return 1;` and its `!=`/local-var/ if-else variants all boolean-normalize (neg/sbb or xor/setcc) at #51 where the original keeps two distinct epilogues (fall-through returns the call's own eax, the taken branch explicitly sets eax=1). Landing the closest form.
// size      273 bytes
// prototype int (__thiscall ?init@NetDaemon@@QAEHH@Z)(NetDaemon* this, int)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E25B0 0x004E31E0 0x005900D0

int NetDaemon::init(int a1) {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<int *>(self + 0x1b30) = 0;
    *reinterpret_cast<int *>(self + 0x1b34) = 0;
    *reinterpret_cast<int *>(self + 0x1b38) = 0;
    *reinterpret_cast<int *>(self + 0x1b3c) = 0;
    *reinterpret_cast<int *>(self + 0x1b60) = 0;
    *reinterpret_cast<int *>(self + 0x1b5c) = 0;
    *reinterpret_cast<int *>(self + 0x1b6c) = 0;
    *reinterpret_cast<int *>(self + 0x1b68) = 0;
    *reinterpret_cast<int *>(self + 0x1b7c) = 0;
    *reinterpret_cast<int *>(self + 0x1ba8) = 0;
    *reinterpret_cast<int *>(self + 0x1ba0) = 0;
    *reinterpret_cast<int *>(self + 0x1ba4) = 0;
    *reinterpret_cast<int *>(self + 0x1bcc) = 0;
    *reinterpret_cast<int *>(self + 0x1bc8) = 0;
    *reinterpret_cast<int *>(self + 0x1b50) = 0;
    *reinterpret_cast<int *>(self + 0x1b54) = 0;
    *reinterpret_cast<int *>(self + 0x1b58) = 0;
    *reinterpret_cast<int *>(self + 0x1bc4) = 0;
    *reinterpret_cast<int *>(self + 0x1bc0) = 0;
    *reinterpret_cast<int *>(self + 0x1bb0) = 0;
    *reinterpret_cast<int *>(self + 0x1bac) = 0;
    *reinterpret_cast<int *>(self + 0x1bb4) = 0;
    *reinterpret_cast<int *>(self + 0x1b40) = 0;
    *reinterpret_cast<int *>(self + 0x1b44) = 0;
    *reinterpret_cast<int *>(self + 0x1bd8) = 0;
    *reinterpret_cast<int *>(self + 0x1bd4) = 0;
    *reinterpret_cast<int *>(self + 0x1b48) = 0;
    *reinterpret_cast<int *>(self + 0x1b4c) = 0;
    *reinterpret_cast<int *>(self + 0x1bd0) = 0xff;
    *reinterpret_cast<int *>(self + 0x1b78) = 0;

    int *p = reinterpret_cast<int *>(self + 0x1b80);
    for (int i = 8; i != 0; i--) {
        *p = 0;
        p++;
    }

    reinterpret_cast<Lock *>(self + 0x14a0)->clear();
    reinterpret_cast<AlphaNet *>(this)->close();

    *reinterpret_cast<int *>(self + 0x1b3c) = 0;
    *reinterpret_cast<int *>(self + 0x1b38) = 0;

    if (reinterpret_cast<AlphaNet *>(this)->setup(a1)) {
        return 1;
    }
    else {
        return 0;
    }
}
