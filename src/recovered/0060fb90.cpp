// 0x0060FB90  ?on_dialog_focus@CheckBox@@QAEXH@Z  ->  ?on_dialog_focus@CheckBox@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Repaint on dialog focus, dispatching through the enclosing object.
Original Offset: 0060FB90
Return Value: n/a
Status: Complete
*/
void CheckBox::on_dialog_focus(int a1) {
    // `this - 0x1c` is NOT arithmetic on a member: CheckBox is a SUBOBJECT of
    // a larger object, and this walks back to the enclosing one, then applies
    // a further delta read from its own +4. Written as one expression on
    // purpose - naming either intermediate makes VC6 pick eax for the delta
    // and edx for the vtable, which is mnemonic-identical and byte-different.
    // a1 is never read; `ret 4` still pops it.
    VCall *const target = reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) - 0x1c +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this) - 0x1c)) + 4));
    target->slot062();
}
