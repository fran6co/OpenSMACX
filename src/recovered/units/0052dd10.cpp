// ORIGINAL: 0x0052DD10 FILE
// name      ?UNK2@DeletionList@@QAEHH@Z
// size      59 bytes
// spans     0x0052DD10-0x0052DD4B
// prototype int (__thiscall ?UNK2@DeletionList@@QAEHH@Z)(DeletionList* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0052DD10
// measured tier  NO_COMPILE
// refusal        u0052dd10.cpp(1) : error C2653: 'DeletionList' : is not a class or namespace name u0052dd10.cpp(2) : error C2065: 'g_009a64e8' : undeclared identifier u0052dd10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052dd10/unit.cpp
// and score it with tools/agent_brief.py.
int DeletionList::UNK2(int a1) {
    unsigned char mask = *reinterpret_cast<unsigned char *>(g_009a64e8);
    unsigned char *rec = reinterpret_cast<unsigned char *>(this);
    for (int i = 0; i < 0x18; ++i, rec += 0x3C) {
        unsigned char flags = *rec;
        unsigned char masked = mask;
        masked &= flags;
        if (masked != mask) {
            if (*reinterpret_cast<int *>(rec + 4) == a1) {
                return i;
            }
        }
    }
    return -1;
}
