// ORIGINAL: 0x00632300 FILE
// name      ?set_polling@Net@@QAEXH@Z
// size      65 bytes
// spans     0x00632300-0x00632341
// prototype void (__thiscall ?set_polling@Net@@QAEXH@Z)(Net* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
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
// address        0x00632300
// measured tier  NO_COMPILE
// refusal        u00632300.cpp(1) : error C2653: 'Net' : is not a class or namespace name u00632300.cpp(2) : error C2673: 'set_polling' : global functions do not have 'this' poi
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00632300/unit.cpp
// and score it with tools/agent_brief.py.
void Net::set_polling(int a1) {
    char *rec = reinterpret_cast<char *>(this) + 0x164;
    for (int i = 0; i < 0x10; ++i, rec += 0x58) {
        *reinterpret_cast<int *>(rec) = 0;
    }
    if (a1 != 0) {
        *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(this) + 0xD8) |= 0x40000;
    } else {
        *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(this) + 0xD8) &= 0xFFFBFFFF;
    }
}
