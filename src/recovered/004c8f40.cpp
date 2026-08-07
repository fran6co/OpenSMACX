// 0x004C8F40  ?unload@VoiceTx@@QAEHXZ  ->  ?unload@VoiceTx@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef void (__cdecl *VoiceFreeFn)(int);

int VoiceTx::unload() {
    reinterpret_cast<VCall *>(this)->slot008();
    reinterpret_cast<VCall *>(this)->slot014();
    if (*g_0090db7c) {
        VoiceFreeFn fn = (VoiceFreeFn)(*g_0090db28);
        fn(*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x3c));
    }
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x3c) = 0;
    return 0;
}
