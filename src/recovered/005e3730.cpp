// ORIGINAL: 0x005E3730 sub_5e3730 0x005E3730-0x005E376B BYTE_EXACT
// size      59 bytes
// prototype int (__stdcall sub_5E3730)(LPCSTR lpString, int, int y, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DD020 0x006453E0
// 0x005E3730  sub_5e3730  ->  _sub_5e3730
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl _strlen(char*);

class BufferProxy { public:
    int sub_5e3730(char *, int, int, int);
};

int BufferProxy::sub_5e3730(char *a1, int a2, int a3, int a4) {
    if (a1 == 0) {
        return a2;
    }
    return reinterpret_cast<Buffer *>(this)->write_cent_l(a1, a2, a3, a4, _strlen(a1));
}
