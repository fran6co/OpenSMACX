// 0x006115E0  ?on_dialog_focus@SpriteBox@@QAEXH@Z  ->  ?on_dialog_focus@SpriteBox@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Repaint on dialog focus, through the enclosing object.
Original Offset: 006115E0
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_dialog_focus(int a1) {
    // `this - 0x8c` walks OUT of this subobject to the enclosing one, whose
    // own +4 holds a further this-adjustment delta - the MSVC virtual-base
    // shape. One expression: naming an intermediate changes the register
    // choice. `a1` is dead; `ret 4` still pops it.
    reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) - 0x8c
        + *reinterpret_cast<int *>(
            *reinterpret_cast<int *>(
                reinterpret_cast<char *>(this) - 0x8c) + 4))->slot062();
}
