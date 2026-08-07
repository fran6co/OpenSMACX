// 0x0045D2E0  ?delete_iface_mode@SubInterface@@QAEXXZ  ->  ?delete_iface_mode@SubInterface@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Drop this interface from the registry.
Original Offset: 0045D2E0
Return Value: n/a
Status: Complete
*/
void SubInterface::delete_iface_mode() {
    // A TOP-TESTED while, not a do-while. VC6 rotates a do-while search loop,
    // duplicating the compare before the back edge, under every measured flag
    // set; with `while` it proves the bound non-empty and drops the redundant
    // first test.
    //
    // Both sides of the bound compare are cast to int because a bare pointer
    // comparison is UNSIGNED - `jae`/`jb` - where the original has a signed
    // `jl`. The cast goes on the real address, not a derived index: a small
    // immediate shortens the encoding by two bytes.
    int index = 0;
    int *entry = g_007ae7f0;
    while (reinterpret_cast<int>(entry) < reinterpret_cast<int>(g_007ae820)) {
        if (*entry == reinterpret_cast<int>(this)) {
            g_007ae7f0[index] = 0;
            return;
        }
        ++entry;
        ++index;
    }
}
