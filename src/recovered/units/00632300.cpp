// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00632300
// name           ?set_polling@Net@@QAEXH@Z
// size           65 bytes
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
