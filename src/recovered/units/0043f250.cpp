// ORIGINAL: 0x0043F250 FILE
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0043F250
// name           ?timer_callback_daemon2@DiploPop@@QAAXHH@Z
// size           56 bytes
// measured tier  NO_COMPILE
// refusal        u0043f250.cpp(1) : error C2653: 'DiploPop' : is not a class or namespace name u0043f250.cpp(2) : error C2065: 'DiploPop' : undeclared identifier u0043f250.cpp(2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043f250/unit.cpp
// and score it with tools/agent_brief.py.
void __cdecl DiploPop::timer_callback_daemon2(int a1, int a2) {
    DiploPop *obj = reinterpret_cast<DiploPop *>(a1);
    if (obj != 0) {
        char *base = reinterpret_cast<char *>(obj);
        int frame2 = *reinterpret_cast<int *>(base + 0x1098);
        int frame1 = *reinterpret_cast<int *>(base + 0x1094);
        Flic *flic = reinterpret_cast<Flic *>(base + 0xAE8);
        flic->decode_frame(frame1, frame2);
        reinterpret_cast<GraphicWin *>(g_007ae820)->soft_update(
            reinterpret_cast<RECT *>(&obj->field_1074_));
    }
}
