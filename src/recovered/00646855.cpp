// ORIGINAL: 0x00646855
// 0x00646855  _fgetc  ->  _fgetc
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
struct FileState {
    unsigned char *next;
    int count;
};
int __cdecl __filbuf(FileState *);
void __cdecl __lock_file(FileState *);
void __cdecl __unlock_file(FileState *);

int __cdecl _fgetc(FileState *file) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    __lock_file(file);
    int result;
    if (--file->count >= 0) {
        result = *file->next++;
    } else {
        result = __filbuf(file);
    }
    __unlock_file(file);
    return result;
}
