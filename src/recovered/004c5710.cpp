// ORIGINAL: 0x004C5710 BYTE_EXACT
// name      ?delete_device@Midi_Device@@QAEHXZ
// size      39 bytes
// spans     0x004C5710-0x004C5737
// prototype int (__thiscall ?delete_device@Midi_Device@@QAEHXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x004C5710  ?delete_device@Midi_Device@@QAEHXZ  ->  ?delete_device@Midi_Device@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Release the MIDI device handle through the driver, if both the
         handle and the driver entry point exist.
Original Offset: 004C5710
Return Value: zero on success, 0x14 when there is nothing to release
Status: Complete
*/
int Midi_Device::delete_device() {
    typedef void(*MidiFn)();
    if (field_14_ && *g_0090db40) {
        (reinterpret_cast<MidiFn>(*g_0090db40))();
        field_14_ = 0;
        return 0;
    }
    return 0x14;
}
