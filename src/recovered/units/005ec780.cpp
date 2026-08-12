// ORIGINAL: 0x005EC780 FILE
// name      ?set_cursor@Win@@QAEHPAUHCURSOR@@@Z
// size      60 bytes
// spans     0x005EC780-0x005EC7BC
// prototype int (__thiscall ?set_cursor@Win@@QAEHPAUHCURSOR@@@Z)(Win* this, HCURSOR* phCursor)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F1820
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005EC780
// measured tier  NO_COMPILE
// refusal        u005ec780.cpp(7) : error C2065: 'Win' : undeclared identifier u005ec780.cpp(7) : error C2059: syntax error : ',' u005ec780.cpp(9) : error C2653: 'Win' : is not 
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ec780/unit.cpp
// and score it with tools/agent_brief.py.
// `update_cursor` is one of the `QAA`/`QAG` Win window-procedure family: the
// mangled name reads as a non-static member, but the real ABI omits the
// receiver entirely (see tools/recovery_symbols.py:is_nonstatic_member). The
// scaffolding's class-member declaration therefore adds an implicit `this`
// no real caller passes; a plain free `__cdecl` declaration with the same
// two explicit parameters reproduces the direct call the original makes.
int __cdecl WinUpdateCursor(Win *, int);

int Win::set_cursor(HCURSOR *a1) {
    if (a1 != 0 && *reinterpret_cast<int *>(a1) == 0) {
        return 7;
    }
    cursor_sprite_ = 0;
    cursor_handle_ = a1;
    cursor_name_ = 0;
    WinUpdateCursor(0, 1);
    return 0;
}
