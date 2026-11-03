#include "stdafx.h"
#include "faction.h"
#include "alpha.h"

// Keep the original out-of-line treaty-update calls visible to the linker.
// This boundary is reconstructed; the historical source filename is unknown.

/*
Purpose: Set or unset the diplomatic treaty for the specified faction with another faction.
// ORIGINAL: 0x0055BB30 ?set_treaty@@YAXHHHH@Z 0x0055BB30-0x0055BB98 BYTE_EXACT
// LEVER: naming the diplo_treaties element's address in a local
//            `uint32_t *treaty_slot` and reaching diplo_merc through it via
//            `(char *)treaty_slot + (offsetof(diplo_merc)-offsetof(diplo_treaties))`
//            moved 15/36 -> 27/36 agreeing (0.933 similar) over the un-named
//            `&PlayersData[...].diplo_treaties[...]` spelling, which ties the
//            original 15/36. Remaining divergence: the image's `set`-branch
//            materialises the index as a byte offset via one `shl eax,2` and
//            reuses plain `[eax+base]` addressing for the load/store/second
//            field; this tree's /O2 always folds `*4` into `[eax*4+base]`
//            scaled addressing instead, and the `else` branch RMW does not
//            fold to the image's single `and dword ptr [mem], reg`.
// TRIED: pointer to diplo_treaties field (shared or per-branch); local index var;
//            unsigned cast on index; explicit RMW temp; if-condition hoisted before the |=.
//            Set branch: image computes the index via explicit `shl eax,2` then reuses
//            the plain [eax+base] address for both the load and the store; this tree's
//            compile always folds the *4 into the addressing mode instead. Plateaus at
//            15/36 agreeing across all tried flag sets.
//            TRIED (2026-08-22): a `uint32_t *diplo_treaties` pointer bound to the
//            ROW (`PlayersData[faction_id].diplo_treaties`, no `[faction_id_with]` yet,
//            indexed at each of the three uses) - the lever that took cause_friction
//            13/33 -> 21/33. Here it is WORSE, not a tie: drops to 0/36. This is a
//            single read-modify-write per branch (not cause_friction's read-then-separate-
//            write of the same element), so there is no shared address to amortise.
//            TRIED (this pass) three spellings of the `shl eax,2`-once shape,
//            all measured and rejected: (1) the flat view - `int index` local
//            plus `SpyingStatusTable[index]` / `SpyingStatusTable[index + 32]`
//            for the merc store, the treaty_off/agenda_on idiom - falls to
//            15/36 because with three accesses at one base VC6 folds *4 into
//            every SIB and never scales eagerly; (2) a byte-granular slot
//            (`char *treaty_slot`) - ties the 32/36 baseline but the address
//            materialises as `lea eax,[eax*4+0x96c9f8]`, the full address,
//            not the image's base-less `shl eax,2`; (3) the byte offset added
//            inline at the use (`(char *)&SpyingStatusTable[index] + 0x80`)
//            - 15/36, the whole chain folds into two SIB accesses and the
//            compile is one instruction SHORTER (35). The image's register
//            holds 4*index with the table base folded into three separate
//            displacements, which is a scheduling choice VC6 6.0 does not
//            reach from any of these source forms; the 32/36 spelling kept
//            below is the closest of the four.
// size      104 bytes
// prototype
// callers   24   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl set_treaty(int faction_id, int faction_id_with, int treaty, BOOL set) {
    if (set) {
        uint32_t *treaty_slot = &PlayersData[faction_id].diplo_treaties[faction_id_with];
        *treaty_slot |= treaty;
        if (treaty & DTREATY_UNK_40) {
            PlayersData[faction_id].diplo_merc[faction_id_with] = 50;
        }
    } else {
        PlayersData[faction_id].diplo_treaties[faction_id_with] &= ~treaty;
    }
}

