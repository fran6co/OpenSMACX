// ORIGINAL: 0x004C8F00 ?stop@VoiceTx@@QAEHXZ 0x004C8F00-0x004C8F3C BYTE_EXACT
// size      60 bytes
// prototype int (__thiscall ?stop@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C51C0 0x004C5B70
// indirect  0x004C8F33
// 0x004C8F00  ?stop@VoiceTx@@QAEHXZ  ->  ?stop@VoiceTx@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class VCall2 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual int slot008();
};

int VoiceTx::stop() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<uint8_t *>(self + 0x54) & 1) {
        int result = reinterpret_cast<Wave_In_Device *>(g_0090db50)->end_record();
        if (result != 0) {
            return result;
        }
        *reinterpret_cast<uint32_t *>(self + 0x54) &= 0xfffffffe;
        reinterpret_cast<Wave_Device *>(g_0090d978)->enable();
        if (field_3C_) {
            return reinterpret_cast<VCall2 *>(field_3C_)->slot008();
        }
    }
    return 0;
}
