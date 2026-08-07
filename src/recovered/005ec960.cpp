// 0x005EC960  ?get_lbutton_state@Win@@QAEHXZ  ->  ?get_lbutton_state@Win@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Report the left mouse button state, honouring a swapped-buttons
         system setting.
Original Offset: 005EC960
Return Value: nonzero while the logical left button is down
Status: Complete
*/
// ONE expression. Naming either result as a local is still correct C++, but
// VC6 then defers the `movsx` that widens the SHORT until after the second
// call instead of folding it straight after the first.
extern "C" __declspec(dllimport) short __stdcall GetAsyncKeyState(int key);
extern "C" __declspec(dllimport) int __stdcall GetSystemMetrics(int index);

int Win::get_lbutton_state() {
    return GetSystemMetrics(0x17) ^ (GetAsyncKeyState(1) >> 15);
}
