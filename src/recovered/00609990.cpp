// ORIGINAL: 0x00609990 BYTE_EXACT
// name      ?item@Dialog@@QAEHPADH@Z
// size      52 bytes
// spans     0x00609990-0x006099C4
// prototype int (__thiscall ?item@Dialog@@QAEHPADH@Z)(Dialog* this, int8*, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100
// 0x00609990  ?item@Dialog@@QAEHPADH@Z  ->  ?item@Dialog@@QAEHPADH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int Dialog::item(char* a1, int a2) {
    if (!a1) {
        return 3;
    }
    char *list = reinterpret_cast<char *>(this) + 0xbc;
    *reinterpret_cast<char **>(list + 0x1c) = a1;
    *reinterpret_cast<int *>(list + 0x20) = 0;
    *reinterpret_cast<int *>(list + 0x24) = 1;
    return reinterpret_cast<StringStruct *>(list)->add(a2);
}
