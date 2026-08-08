// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0041A7A0
// name           ?check_base@BaseWin@@QAEXH@Z
// size           63 bytes
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
