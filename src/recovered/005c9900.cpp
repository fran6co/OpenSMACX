// 0x005C9900  ?timer_callback_daemon@MultiDebug@@QAAXH@Z  ->  ?timer_callback_daemon@MultiDebug@@QAAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Forward the timer tick to the object's own virtual slot 62.
Original Offset: 005C9900
Return Value: n/a
Status: Complete
*/
void __cdecl MultiDebug::timer_callback_daemon(int a1) {
    // Dispatch on `this`, NOT on a1. For a __cdecl member VC6 puts `this` at
    // [ebp+8] and the explicit argument at [ebp+0xc]; dispatching on a1 is
    // SHAPE_EXACT and reads the wrong slot. a1 is unused by this body.
    if (this) {
        reinterpret_cast<VCall *>(this)->slot062();
    }
}
