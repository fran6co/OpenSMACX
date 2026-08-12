// ORIGINAL: 0x0041A7A0 FILE
// name      ?check_base@BaseWin@@QAEXH@Z
// size      63 bytes
// spans     0x0041A7A0-0x0041A7DF
// prototype void (__thiscall ?check_base@BaseWin@@QAEXH@Z)(BaseWin* this, int baseID)
// callers   9   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004EC3B0 0x005D5A70 0x005F7E90
// indirect  0x0041A7CA
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0041A7A0
// measured tier  NO_COMPILE
// refusal        u0041a7a0.cpp(1) : error C2653: 'BaseWin' : is not a class or namespace name u0041a7a0.cpp(2) : error C2061: syntax error : identifier 'Win' u0041a7a0.cpp(3) : 
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041a7a0/unit.cpp
// and score it with tools/agent_brief.py.
void BaseWin::check_base(int a1) {
    if (reinterpret_cast<Win *>(this)->is_visible()
            && a1 == *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x40B0C)) {
        base_compute(1);
        reinterpret_cast<VCall *>(this)->slot062();
        reinterpret_cast<GraphicWin *>(g_007ae820)->redraw();
    }
}
