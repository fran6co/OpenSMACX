// ORIGINAL: 0x0046FCE0 FILE
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0046FCE0
// name           ?UNK7@MapWin@@QAEXXZ
// size           58 bytes
// measured tier  NO_COMPILE
// refusal        u0046fce0.cpp(1) : error C2653: 'MapWin' : is not a class or namespace name u0046fce0.cpp(2) : error C2065: 'owned_' : undeclared identifier u0046fce0.cpp(3) : 
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0046fce0/unit.cpp
// and score it with tools/agent_brief.py.
void MapWin::UNK7() {
    if (owned_ != 0) {
        for (int i = 0; i < *g_00949884; ++i) {
            for (int j = 0; j < 4; ++j) {
                reinterpret_cast<char *>(owned_)[i * 0xC + j] = static_cast<char>(0xFF);
            }
        }
    }
}
