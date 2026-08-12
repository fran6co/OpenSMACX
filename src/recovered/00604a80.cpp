// ORIGINAL: 0x00604A80 BYTE_EXACT
// 0x00604A80  ?on_modal@BasePop@@QAAHXZ  ->  ?on_modal@BasePop@@QAAHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Report whether the popup is modal - true when the guard is clear.
Original Offset: 00604A80
Return Value: nonzero when modal
Status: Complete
*/
int __cdecl BasePop::on_modal() {
    return *g_009b8cfc == 0;
}
