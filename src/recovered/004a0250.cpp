// 0x004A0250  ?sat_expl_daemon@@YAXH@Z  ->  ?sat_expl_daemon@@YAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl sat_expl_daemon(int a1) {
    reinterpret_cast<ReportWin *>(a1)->exp_anim();
}
