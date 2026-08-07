// 0x006051C0  sub_6051c0  ->  _sub_6051c0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Original Offset: 006051C0
Return Value: n/a
Status: Complete
*/
// `__declspec(dllimport)` is load-bearing: without it VC6 emits a direct
// `call rel32` (5 bytes) where the original calls indirectly through the
// import slot at 0x00669334 with `call dword ptr` (6 bytes).
extern "C" __declspec(dllimport) int __stdcall GetSystemMetrics(int index);

extern "C" void __cdecl sub_6051c0() {
    *reinterpret_cast<int *>(0x009B8DD4) = GetSystemMetrics(3);
}
