#include "stdafx.h"

#include "time.h"
#include "win.h"         // do_all_draws/do_non_input, clock_wait's pump

// Pump redraws before non-input work at least once. The original captures the
// WINMM timeGetTime import once and compares unsigned elapsed time, including
// wraparound and negative arguments interpreted as unsigned durations.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050DD90 ?clock_wait@@YAXH@Z 0x0050DD90-0x0050DDBA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/0050dd90.cpp retained exact seed and original extent; preserved without copying historical claim", "Pinned PE import669368 resolves WINMM.dll timeGetTime; full disassembly50DD90..50DDBA captures import once, calls5FCBB0 then5FCA30 and uses unsigned jb elapsed compare"], "reason": "Home cooperative unsigned elapsed-time wait to timer integration; preserve import capture, redraw/non-input ordering, mandatory first iteration and modular DWORD subtraction, without fixed import address."}
void __cdecl clock_wait(int milliseconds) {
    DWORD (WINAPI *const read_clock)() = timeGetTime;
    const DWORD started = read_clock();
    do {
        do_all_draws();
        do_non_input();
    } while (read_clock() - started < static_cast<DWORD>(milliseconds));
}
