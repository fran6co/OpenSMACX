// ORIGINAL: 0x005D7AE0 BYTE_EXACT
// name      sub_5d7ae0
// size      61 bytes
// spans     0x005D7AE0-0x005D7B1D
// prototype 
// callers   1   call targets   1
// 0x005D7AE0  sub_5d7ae0  ->  _sub_5d7ae0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// No visible read ever touches `this` directly in this body - it is only
// ever forwarded, unchanged, as the receiver of the inner `load_pcx` call.
// That is only possible if THIS function is itself a thiscall member
// (this in ECX, never clobbered) rather than the plain __stdcall free
// function the catalogue's placeholder signature proposed; `ret 0x10`
// (4 stack dwords) is consistent with either reading, since a thiscall
// receiver costs no stack space, so it does not by itself distinguish them
// - the missing `mov ecx` before the `call load_pcx` does.
//
// Shadow struct for the first stack argument: a pointer to a small record
// holding a raw pcx byte buffer (0xc) and its length (0x10).
struct PcxSource {
    char pad_0[0xc];
    unsigned char *data;   // offset 0xc
    unsigned long size;    // offset 0x10
};

class PcxOwner { public:
    int validate_and_load_pcx(PcxSource *, Palette *, int, int);
};

int PcxOwner::validate_and_load_pcx(PcxSource *a1, Palette *a2, int a3, int a4) {
    if (a1 == 0) {
        return 3;
    }
    if (a1->data == 0) {
        return 7;
    }
    return reinterpret_cast<Buffer *>(this)->load_pcx(a1->data, a1->size, a2, a3, a4);
}
