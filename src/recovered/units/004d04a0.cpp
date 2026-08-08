// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004D04A0
// name           ?wait@Console@@QAEXH@Z
// size           64 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004d04a0/unit.cpp
// and score it with tools/agent_brief.py.
extern int g_table_95282c[];

void Console::wait(int a1) {
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23D18) = 0;
    g_table_95282c[a1 * 13] |= 2;
    ready();
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23BE4) = 0;
}
