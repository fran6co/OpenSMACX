// ORIGINAL: 0x004C57A0 BYTE_EXACT
// name      ?init@Midi_Device@@QAEHPAXK@Z
// size      68 bytes
// spans     0x004C57A0-0x004C57E4
// prototype int (__thiscall ?init@Midi_Device@@QAEHPAXK@Z)(Midi_Device* this, void*, unsigned int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x004C57A0  ?init@Midi_Device@@QAEHPAXK@Z  ->  ?init@Midi_Device@@QAEHPAXK@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Second VCall shim: slot000 takes one arg and slot003 takes two, unlike
// the emitter's nullary default for every slot.
class VCallShim { public:
    virtual int slot000(unsigned long);
    virtual void slot001();
    virtual void slot002();
    virtual int slot003(void *, unsigned long);
};

int Midi_Device::init(void *a1, unsigned long a2) {
    int result = reinterpret_cast<VCallShim *>(this)->slot000(a2);
    if (result != 0) {
        return result;
    }
    VCallShim *sub = *reinterpret_cast<VCallShim **>(
        reinterpret_cast<char *>(this) + 0x14);
    int result2 = sub->slot003(a1, a2);
    if (result2 != 0) {
        reinterpret_cast<VCallShim *>(this)->slot001();
        *reinterpret_cast<VCallShim **>(
            reinterpret_cast<char *>(this) + 0x14) = 0;
        return result2;
    }
    return 0;
}
