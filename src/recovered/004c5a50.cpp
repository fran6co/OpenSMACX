// ORIGINAL: 0x004C5A50 BYTE_EXACT
// name      ?release@Wave_In_Device@@QAEHXZ
// CORRECTED from ?release@Wave_In_Device@@QAEXXZ
//   a trailing `xor eax, eax` at 0x004C5A6D is reached from BOTH the
//   guard-taken and guard-skipped paths before `pop esi; ret`, so it is
//   a shared `return 0` and the function returns int. With the void head
//   the body matches through instruction 10 and is 2 bytes short
// size      33 bytes
// spans     0x004C5A50-0x004C5A71
// prototype int (__thiscall ?release@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5A5C 0x004C5A63
// 0x004C5A50  ?release@Wave_In_Device@@QAEHXZ  ->  ?release@Wave_In_Device@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Release the capture delegate and detach it.
Original Offset: 004C5A50
Return Value: zero
Status: Complete
*/
// Returns int, not the catalogued void. The trailing `xor eax,eax` sits at a
// MERGE POINT both paths reach before `pop esi; ret` - that is a `return 0`,
// not a side effect. Corrected in catalogue_corrections.py.
int Wave_In_Device::release() {
    if (field_14_) {
        reinterpret_cast<VCall *>(field_14_)->slot004();
        reinterpret_cast<VCall *>(this)->slot001();
        field_14_ = 0;
    }
    return 0;
}
