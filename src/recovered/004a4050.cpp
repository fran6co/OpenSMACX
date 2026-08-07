// 0x004A4050  ?bl_daemon@@YAXH@Z  ->  ?bl_daemon@@YAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl bl_daemon(int a1) {
    reinterpret_cast<ReportIf *>(a1)->bl_anim();
}
