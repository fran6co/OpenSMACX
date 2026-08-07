// 0x0060CE10  ?on_mouse_leave@ListBox@@QAEXHH@Z  ->  ?on_mouse_leave@ListBox@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Clear the hover index and repaint, through the enclosing object.
Original Offset: 0060CE10
Return Value: n/a
Status: Complete
*/
void ListBox::on_mouse_leave(int a1, int a2) {
    // `this - 0x48` reaches a vbtable-shaped descriptor with two deltas:
    // entry +8 locates the field, entry +4 the enclosing object's vtable.
    // Note 0x48 + 0xA8 == 0xF0 across this whole family, so the
    // field is at a FIXED +0xF0 in the enclosing object and these classes are
    // subobjects at differing offsets inside it.
    //
    // The arithmetic must go through `char *`. Through `int *` the compiler
    // scales the +8 and emits [eax+0x20].
    //
    // Both parameters are dead; `ret 8` still pops them.
    *reinterpret_cast<int *>(
        reinterpret_cast<char *>(this) + 0xA8 +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this) - 0x48)) + 8)) = -1;

    reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) - 0x48 +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this) - 0x48)) + 4))->slot062();
}
