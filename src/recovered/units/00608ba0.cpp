// ORIGINAL: 0x00608BA0 FILE
// name      ?add_special@StringList@@QAEHPAD@Z
// size      92 bytes
// spans     0x00608BA0-0x00608BFC
// prototype int (__thiscall ?add_special@StringList@@QAEHPAD@Z)(StringList* this, int8*)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00608BA0
// measured tier  NO_COMPILE
// refusal        u00608ba0.cpp(30) : error C2653: 'StringList' : is not a class or namespace name u00608ba0.cpp(49) : error C2065: 'field_1C_' : undeclared identifier u00608ba0.
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00608ba0/unit.cpp
// and score it with tools/agent_brief.py.
// 0x00608BA0  ?add_special@StringList@@QAEHPAD@Z  (92 bytes)
//
// Fresh brief updates the on-disk scaffold: return type is `int` (mangled
// `H`, not the stale `X`/void the scaffold used to show), the parameter is
// `char *` (not `int8*`), and StringList/StringStruct already carry real
// field layouts from earlier recoveries (field_1C_/field_20_/field_24_;
// StringStruct::add(int)). None of that is redeclared here - it is already
// in the emitted scaffold; this is only the out-of-line definition.
//
// `*a1 == '^'` alone matches, but the a1[1] three-way split (nul / '^' /
// anything else) needs the "default" arm (kind=1) written as the innermost
// branch of a negated, nested if - `if (a1[1] != 0) { if (a1[1] != '^') {
// kind=1 } else {...} } else { kind=3 }` - not a flat if/else-if/else on the
// positive conditions. The flat form places kind=1 and kind=3 in source
// order (je/cmp/je); the original places kind=3's block LAST, past the ^^
// handling, which only the negated-nested form reproduces. Measured: flat
// form scored mnemonic_similarity 0.8254 (edit_count 6); negating just the
// outer test (a1[1] != 0) got to 0.9206 (edit_count 5) with no other change.
//
// kind=2 has to be assigned separately in BOTH arms of the innermost
// `a1[2]=='^'` split, not hoisted once above it - hoisting produced an extra
// reordered `mov edx,2` (edit_count 5, one insert + one delete at adjacent
// indices); duplicating it in each arm let the compiler place it exactly
// where the original does, adjacent to the `add eax,N` in the same arm.
//
// `this->add(0)` is called through `reinterpret_cast<StringStruct *>(this)`:
// the disassembly reuses `ecx` unmodified for the call (no adjustor), and
// the two classes are declared as unrelated opaque shells in this unit, so
// the cast is what reproduces "same address, different static type".
int StringList::add_special(char *a1) {
    int kind = 0;
    if (*a1 == '^') {
        if (a1[1] != 0) {
            if (a1[1] != '^') {
                kind = 1;
                a1++;
            } else if (a1[2] == '^') {
                kind = 2;
                a1 += 3;
            } else {
                kind = 2;
                a1 += 2;
            }
        } else {
            kind = 3;
            a1++;
        }
    }
    field_1C_ = (uint32_t)a1;
    field_20_ = kind;
    field_24_ = 0;
    return reinterpret_cast<StringStruct *>(this)->add(0) != 0;
}
