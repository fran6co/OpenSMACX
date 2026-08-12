// ORIGINAL: 0x004C56D0 BYTE_EXACT
// 0x004C56D0  ?create_device@Midi_Device@@QAEHK@Z  ->  ?create_device@Midi_Device@@QAEHK@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int Midi_Device::create_device(unsigned long a1) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x14) == 0) {
        typedef int (__cdecl *MidiOpenFn)(void *, unsigned long);
        MidiOpenFn fn = reinterpret_cast<MidiOpenFn>(*g_0090db3c);
        if (fn != 0) {
            return fn(self + 0x14, a1);
        }
        return 0x14;
    }
    return 0xc;
}
