// ORIGINAL: 0x00646886
// 0x00646886  _fputc  ->  _fputc
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// The auto-fixed declarations for `_flsbuf`/`_lock_file`/`_unlock_file` are
// nullary (`extern "C" int __cdecl _flsbuf();`) - an empty parameter list
// means "takes no arguments" in C++, not "unspecified" as in C, so they
// cannot be completed with a second, differently-typed extern "C"
// declaration (C2733). Calling through the real names is unreachable without
// redeclaring them; since the call sites are direct `E8` relocations either
// way (masked in the comparison regardless of target symbol), the callees
// are declared here under fresh names with the arity the disassembly needs
// instead.
struct FileHandle {
    char *ptr_;
    int cnt_;
};

extern "C" void __cdecl lock_stream(FileHandle *stream);
extern "C" void __cdecl unlock_stream(FileHandle *stream);
extern "C" int __cdecl flush_and_store(int ch, FileHandle *stream);

extern "C" int __cdecl fputc(int ch, FileHandle *stream) {
    int retval;
    lock_stream(stream);
    if (--stream->cnt_ >= 0) {
        *stream->ptr_ = (char)ch;
        stream->ptr_++;
        retval = (unsigned char)ch;
    } else {
        retval = flush_and_store(ch, stream);
    }
    unlock_stream(stream);
    return retval;
}
