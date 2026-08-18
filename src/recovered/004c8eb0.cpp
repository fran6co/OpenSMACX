// ORIGINAL: 0x004C8EB0 ?start@VoiceTx@@QAEHXZ 0x004C8EB0-0x004C8EF4 BYTE_EXACT
// size      68 bytes
// prototype int (__thiscall ?start@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C51D0 0x004C5B60
// indirect  0x004C8EDE
// 0x004C8EB0  ?start@VoiceTx@@QAEHXZ  ->  ?start@VoiceTx@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Wave_Device::disable() and the vtable slot both feed a checked
// return value in the original (test eax,eax after the call), so
// they need a return type the catalogue's void attribution doesn't
// give them; shadowed here under a different name so Wave_Device's
// own declaration is untouched.
class WaveDeviceEx { public:
    int disable();
};

class VCall37 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual int slot036();
};

int VoiceTx::start() {
    char *self = reinterpret_cast<char *>(this);
    if (!(*reinterpret_cast<uint8_t *>(self + 0x54) & 1)) {
        int result = reinterpret_cast<WaveDeviceEx *>(g_0090d978)->disable();
        if (result != 0) {
            return result;
        }
        result = reinterpret_cast<Wave_In_Device *>(g_0090db50)->start_record();
        if (result != 0) {
            return result;
        }
        int *field3c = *reinterpret_cast<int **>(self + 0x3c);
        if (field3c != 0) {
            result = reinterpret_cast<VCall37 *>(field3c)->slot036();
            if (result != 0) {
                return result;
            }
        }
        *reinterpret_cast<int *>(self + 0x54) |= 1;
    }
    return 0;
}
