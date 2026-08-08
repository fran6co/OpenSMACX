// 0x00445650  ?ok_callback@@YAXXZ  ->  ?ok_callback@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl ok_callback() {
    Wave *wave = reinterpret_cast<Wave *>(g_0074d8f0);
    wave->set_pitch(*g_0074daa8);
    wave->play();
    int pitch = *g_0074daa8;
    if (pitch >= 0) {
        pitch -= 300;
    } else {
        pitch += 200;
    }
    *g_0074daa8 = pitch;
    if (pitch <= -1200 || pitch >= 1200) {
        *g_0074daa8 = 0;
    }
}
