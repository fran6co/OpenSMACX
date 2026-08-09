// ORIGINAL: 0x00633980
// 0x00633980  ?on_left_double_click@CheckButton@@QAEXHH@Z  ->  ?on_left_double_click@CheckButton@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Invoke the registered click callback with the stored argument.
Original Offset: 00633980
Return Value: n/a
Status: Complete
*/
void CheckButton::on_left_double_click(int a1, int a2) {
    // `call eax`, not `call [eax+N]`: a raw __cdecl function-pointer FIELD,
    // not a vtable slot. Both parameters are unused - the body never touches
    // the stack.
    typedef void(__cdecl * Callback)(int);
    Callback const fn = *reinterpret_cast<Callback *>(
        reinterpret_cast<char *>(this) + 0xa20);
    if (fn) {
        const int argument = *reinterpret_cast<int *>(
            reinterpret_cast<char *>(this) + 0xa18);
        fn(argument);
    }
}
