// ORIGINAL: 0x005DBCB0 BYTE_EXACT
// name      ?wrap_right@Buffer@@QAEHPAVFont@@PADH@Z
// size      79 bytes
// spans     0x005DBCB0-0x005DBCFF
// prototype int (__thiscall ?wrap_right@Buffer@@QAEHPAVFont@@PADH@Z)(Buffer* this, Font*, int8*, int)
// callers   0   call targets   1
// 0x005DBCB0  ?wrap_right@Buffer@@QAEHPAVFont@@PADH@Z  ->  ?wrap_right@Buffer@@QAEHPAVFont@@PADH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct FontTriple { Font *a, *b, *c; };

int Buffer::wrap_right(Font * a1, char * a2, int a3) {
    FontTriple *triple = reinterpret_cast<FontTriple *>(reinterpret_cast<char *>(this) + 0x52c);
    FontTriple saved = *triple;
    if (a1 != 0 && a1->font_obj_ != 0) {
        triple->a = a1;
        font2_ = 0;
        font3_ = 0;
    }
    int result = wrap_right(a2, a3);
    *triple = saved;
    return result;
}
