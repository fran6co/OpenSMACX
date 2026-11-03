#include "halloffame.h"

// Keep record cleanup separate from the FameWin UI caller: the pinned
// executable calls this helper out of line. Original filenames are unknown.

// ORIGINAL: 0x0044AF70 sub_44af70 0x0044AF70-0x0044AF8D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned native44AE50 reads/writes hallfame.jhh with rb/wb and mode759194;35records of8C bytes begin9A3840;44AF70 clears only their present bytes.", "Native44AEE6 owned ButtonGroup receiver753468 is FameWin74DAF8+5970;44AF04 virtual slot57 is Win::set_modal with callbackpop_wait5398E0.", "Root-only Console retirement dependency closure authorized in task history; retain git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/0044ae50.cpp and git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/0044af70.cpp seed interpretations.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/0044af70.cpp"], "reason": "Recover real FameWin entry and record-presence cleanup through existing typed HallOfFame records and owned button group, standard CRT I/O and inherited virtual modal API. Retain historical exact seed obligations and original spans; current binding grants no match or acceptance claim. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?clear_hall_of_fame@@YAXXZ
void __cdecl clear_hall_of_fame() {
    for (int group = 0; group < HallOfFameRecordCount / 5; ++group) {
        for (int rank = 0; rank < 5; ++rank) {
            HallOfFameRecords[group * 5 + rank].present = 0;
        }
    }
}

