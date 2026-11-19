#include "stdafx.h"
// WORKAROUND(decl): wants_to_attack 0x0055BC80 ties on how many declarations precede
// faction.h's (pads here or before stdafx.h move it; pads after the last
// include never do). fe sweep 2026-11-19, 0-63 pads: exact at 14-45, no
// other function of the unit changes. The original header declaration
// set is unknown; re-tune after header batches.
// HEADER-PADS: function FactionTreatyHeaderPad
void FactionTreatyHeaderPad0();
void FactionTreatyHeaderPad1();
void FactionTreatyHeaderPad2();
void FactionTreatyHeaderPad3();
void FactionTreatyHeaderPad4();
void FactionTreatyHeaderPad5();
void FactionTreatyHeaderPad6();
void FactionTreatyHeaderPad7();
void FactionTreatyHeaderPad8();
void FactionTreatyHeaderPad9();
void FactionTreatyHeaderPad10();
void FactionTreatyHeaderPad11();
void FactionTreatyHeaderPad12();
void FactionTreatyHeaderPad13();
void FactionTreatyHeaderPad14();
void FactionTreatyHeaderPad15();
void FactionTreatyHeaderPad16();
void FactionTreatyHeaderPad17();
void FactionTreatyHeaderPad18();
void FactionTreatyHeaderPad19();
void FactionTreatyHeaderPad20();
void FactionTreatyHeaderPad21();
void FactionTreatyHeaderPad22();
void FactionTreatyHeaderPad23();
void FactionTreatyHeaderPad24();
void FactionTreatyHeaderPad25();
void FactionTreatyHeaderPad26();
void FactionTreatyHeaderPad27();
void FactionTreatyHeaderPad28();
void FactionTreatyHeaderPad29();
#include "faction.h"
#include "map.h"
#include "base.h"
#include "crt_random_inline.h"
#include "tutwin.h"
#include "council.h"
#include "technology.h"
#include "veh.h"
#include "netmsg.h"
#include "netdaemon.h"
#include "multiwin.h"
#include "xpops.h"
#include "random.h"
#include "faction_inline.h"

// Treaty/agenda writers and the AI diplomacy run, 0x0055B760-0x00560ACD
// (treaty_off .. enemy_diplomacy). This boundary is reconstructed, and the
// historical source filename is unknown. The run starts right after the
// FontQueue unit's ~Vector (0x0055B750) and ends where the AI movement unit's
// go_to (0x00560AD0) begins. The evidence that it is a separate unit:
// wants_to_attack calls great_beelzebub (0x00539B70) out of line, as only
// communicate (another unit) also does; has_treaty and has_agenda are expanded as in
// faction.cpp (faction_inline.h); and the treaty/agenda writers are called out
// of line from faction.cpp with no auto_inline pragmas needed.

void __cdecl interlude(int id, char *filename, int arg3, int arg4);


// The diplomacy flag word at 0x0074DA9C.
// RECOVERY_OBJECT: v2 0x0074DA9C 0x0074DAA0 ?DiploRedrawFlag@@3HA 0x0055B870
int DiploRedrawFlag;  // 0x0074DA9C


/*
Purpose: Clear `treaty` in the diplomatic status word between two factions, both
         directions of the pair, and take the side effects the clear implies:
         clearing any of the three low bits also clears 0x1100000, clearing
         VENDETTA also clears 0x8040, and clearing WANT_REVENGE also clears
         SHALL_BETRAY in the +8 view of the same table.
// ORIGINAL: 0x0055B760 ?treaty_off@@YAXHHH@Z 0x0055B760-0x0055B818 BYTE_EXACT
// LEVER: PROMOTED out of src/unrecovered/0055b760.cpp, whose claim proved the
//         ARTIFACT reproduced while the shipped program contained nothing - the
//         same promotion set_state_id made beside CheckBox::get_state_id. The
//         artifact's two raw table bindings are SpyingStatusTable and the same
//         table eight uint32_t elements further in, which is
//         SpyingStatusTable[index + 8] - the view do_check reads for its
//         0x80000000-id boxes, and the fold lands on the same displacement.
// size      184 bytes
// prototype void (__cdecl ?treaty_off@@YAXHHH@Z)(int faction_id, int faction_id_with, int treaty)
// callers   14   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl treaty_off(int faction_id, int faction_id_with, int treaty) {
    if (treaty & 7) {
        treaty |= 0x1100000;
    }
    if (treaty & DTREATY_VENDETTA) {
        treaty |= 0x8040;
    }
    if (treaty & DTREATY_WANT_REVENGE) {
        // 0xfffff3ff == ~(SHALL_BETRAY | 0x800), and in the +8 view: dropping
        // WANT_REVENGE drops that pair as well.
        int index = faction_id_with;
        PlayersData[faction_id].diplo_agenda[index] &= 0xfffff3ff;
        index = faction_id;
        PlayersData[faction_id_with].diplo_agenda[index] &= 0xfffff3ff;
    }
    int index = faction_id_with;
    PlayersData[faction_id].diplo_treaties[index] &= ~treaty;
    index = faction_id;
    PlayersData[faction_id_with].diplo_treaties[index] &= ~treaty;
}


/*
Purpose: Clear `agenda` in the diplomatic agenda word between two factions,
         both directions of the pair. agenda_on's mirror, without its 0x20/0x40
         interaction - the caller has already decided which bit it wants gone.
// ORIGINAL: 0x0055B820 ?agenda_off@@YAXHHH@Z 0x0055B820-0x0055B870 BYTE_EXACT
// LEVER: `int mask = ~agenda;` FIRST - naming the complement up front is what
//         makes VC6 emit the image's `not edx` before the index arithmetic and
//         keeps the mask in a register for both directions. Writing
//         `SpyingStatusTable[index + 8] &= ~agenda` twice deferred the `not`
//         past the first `push edi` and diverged at instruction 5 (18/30).
// size      80 bytes
// prototype void (__cdecl ?agenda_off@@YAXHHH@Z)(int faction_id, int faction_id_with, int agenda)
// callers   5   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl agenda_off(int faction_id, int faction_id_with, int agenda) {
    int mask = ~agenda;
    int index = faction_id_with;
    PlayersData[faction_id].diplo_agenda[index] &= mask;
    index = faction_id;
    PlayersData[faction_id_with].diplo_agenda[index] &= mask;
}


/*
Purpose: Set `treaty` in the diplomatic status word between two factions, both
         directions of the pair, and take the side effects the set implies:
         a PACT carries TREATY with it, a vendetta declared on top of a live
         pact/treaty/truce zeroes both directions of the third status view and
         clears a block of status bits in both directions, and either faction
         being the local one asks for the multiplayer window to be redrawn.
// ORIGINAL: 0x0055B870 ?treaty_on@@YAXHHH@Z 0x0055B870-0x0055BA76 BYTE_EXACT
// LEVER: the local faction is a CACHED LOCAL, not a global read at each use -
// `int local_faction = LocalFaction;` at entry plus two re-reads (after the
// vendetta-table clearing, after the ambience call) is what puts the image's
// `mov ecx,[0x939284]` at instruction 2 and keeps 94/152 instructions
// agreeing. Reading LocalFaction at each use dropped it to 15/152, because
// VC6 then loads the global lazily inside the first block.
// TRIED: naming `index` at the TOP of the `treaty & 7` block (the treaty_off
//            spelling) so the stores could use scaled addressing - VC6 then
//            hoists the whole lea chain ABOVE the two LocalFaction compares
//            and the match falls to 12/152. The image computes the index
//            between the compares and the clear, so the first block spells
//            the expression inline in the `if` and names `index` only for the
//            two clears.
// RESIDUAL: 13 divergent instructions. Ten are one cluster the source cannot
//            reach from here: the vendetta block wants its index scaled ONCE -
//            `shl eax,2` then `[eax + 0x96c9f8]` and `[eax + 0x96cb8c]` for
//            the byte test, the vendetta-table store and the status load - and
//            one `index` local reaching two different table bases compiles as
//            a SIB `[eax*4 + base]` per access instead, plus a
//            `lea eax,[eax*4+..]; mov [eax],edx` for the last
//            `&= 0xfaeef378`. The three after that are register choice
//            (eax vs edx) in the two final lea chains. The archived unit bound
//            both tables as `extern int g_0096c9f8_tbl[]` arrays rather than
//            folded `uint32_t *const` pointers, which may be what buys the
//            shared scaling - untested, since binding those addresses as real
//            arrays means defining storage the tree does not own.
// TRIED: three further flag sets under --all-flags: /O2 without /Oy- scores
//            26/152, /O1 7/152 - the default /O2 /Gy /GR- /Oy- /GX at 0.993
//            similar is the winner.
// size      518 bytes
// prototype void (__cdecl ?treaty_on@@YAXHHH@Z)(int faction_id, int faction_id_with, int treaty)
// callers   33   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00446B40 0x005D5A70
Return Value: n/a
Status: Complete
*/
void __cdecl treaty_on(int faction_id, int faction_id_with, int treaty) {
    // The local faction, cached for the pair tests and re-read after each
    // store or call that could alias it - the image keeps it in ecx and does
    // exactly two reloads, one after the vendetta-table clearing and one
    // after the ambience call.
    int local_faction = ConsoleGlobal.faction_id_;
    if (treaty & 7) {
        if ((faction_id == local_faction || faction_id_with == local_faction)
            && (PlayersData[faction_id].diplo_treaties[faction_id_with]
                & DTREATY_VENDETTA)) {
            // ~1 on a bit-0 flag: the request goes away here.
            DiploRedrawFlag &= ~1;
        }
        // ~0x118050: VENDETTA, both UNK_40/UNK_8000 and the 0x1100000 pair -
        // the same bits treaty_off adds to its mask for the same three low
        // bits, cleared here rather than or'ed on.
        int index = faction_id_with;
        PlayersData[faction_id].diplo_treaties[index] &= 0xffee7faf;
        index = faction_id;
        PlayersData[faction_id_with].diplo_treaties[index] &= 0xffee7faf;
    }
    if (treaty & DTREATY_PACT) {
        treaty |= DTREATY_TREATY;
    }
    if (treaty & DTREATY_VENDETTA) {
        int index = faction_id_with;
        if (PlayersData[faction_id].diplo_treaties[index] & 7) {
            PlayersData[faction_id_with].diplo_unk4[faction_id] = 0;
            PlayersData[faction_id].diplo_unk4[index] = 0;
            local_faction = ConsoleGlobal.faction_id_;
        }
        treaty |= 0x80000108;
        PlayersData[faction_id].diplo_treaties[index] &= 0xfaeef378;
        PlayersData[faction_id_with].diplo_treaties[faction_id]
            &= 0xfaeef378;
        if (faction_id == local_faction
            || faction_id_with == local_faction) {
            DiploRedrawFlag |= 1;
            ambience(0xcc);
            local_faction = ConsoleGlobal.faction_id_;
        }
    }
    if (treaty & 0x17) {
        int index = faction_id_with;
        PlayersData[faction_id].diplo_treaties[index] &= 0xff7fffff;
        index = faction_id;
        PlayersData[faction_id_with].diplo_treaties[index] &= 0xff7fffff;
    }
    int index = faction_id_with;
    PlayersData[faction_id].diplo_treaties[index] |= treaty;
    index = faction_id;
    PlayersData[faction_id_with].diplo_treaties[index] |= treaty;
    if ((faction_id == local_faction || faction_id_with == local_faction)
        && (treaty & 0x1f) && MultiWindow.panel_open()) {
        MultiWindow.GraphicWin::redraw();
    }
}


/*
Purpose: Set `agenda` in the diplomatic agenda word between two factions, both
         directions of the pair; setting the 0x20 agenda bit also clears the
         0x40 bit beside it.
// ORIGINAL: 0x0055BA80 ?agenda_on@@YAXHHH@Z 0x0055BA80-0x0055BB29 BYTE_EXACT
// LEVER: the agenda view is `SpyingStatusTable[index + 8]`, exactly
//         treaty_off's spelling for the +8 view, and `index` is declared
//         INSIDE each block the way treaty_off declares it - one `int index`
//         per block, reassigned for the second direction. Declaring it before
//         the `agenda & 0x20` test hoisted the index chain above the test and
//         cost 49 of 56 instructions; the image computes it inside the branch.
// size      169 bytes
// prototype void (__cdecl ?agenda_on@@YAXHHH@Z)(int faction_id, int faction_id_with, int agenda)
// callers   8   call targets   0
// kind      game
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl agenda_on(int faction_id, int faction_id_with, int agenda) {
    // The agenda view is the same table eight dwords in - 0x96CA18, which is
    // PlayerData::diplo_agenda[0].
    if (agenda & 0x20) {
        int index = faction_id_with;
        PlayersData[faction_id].diplo_agenda[index] &= ~0x40;
        index = faction_id;
        PlayersData[faction_id_with].diplo_agenda[index] &= ~0x40;
    }
    int index = faction_id_with;
    PlayersData[faction_id].diplo_agenda[index] |= agenda;
    index = faction_id;
    PlayersData[faction_id_with].diplo_agenda[index] |= agenda;
}


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


/*
Purpose: Set or unset the diplomatic agenda for the specified faction with another faction.
// ORIGINAL: 0x0055BBA0 ?set_agenda@@YAXHHHH@Z 0x0055BBA0-0x0055BBF1 BYTE_EXACT
// size      81 bytes
// prototype void (__cdecl ?set_agenda@@YAXHHHH@Z)(int factionID, int factionIDWith, int agenda, int set)
// callers   10   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl set_agenda(int faction_id, int faction_id_with, int agenda, BOOL set) {
    if (set) {
        PlayersData[faction_id].diplo_agenda[faction_id_with] |= agenda;
    } else {
        PlayersData[faction_id].diplo_agenda[faction_id_with] &= ~agenda;
    }
}


// Historical SpyingBaseIndex interpreted 0x009A6524 as an independent
// scalar. It is SecretProject.empath_guild (+0x10 in the table at
// 0x009A6514); project construction/removal and setup_game own its writes.
/*
Purpose: Determine whether the current faction has intelligence visibility of
         the given subject faction.
// ORIGINAL: 0x0055BC00 ?spying@@YAHH@Z 0x0055BC00-0x0055BC7C BYTE_EXACT
// LEVER: ONE `||` CHAIN IN A `return`, not three early-return `if`s. The image reaches its `mov eax, 1` epilogue from three different jumps and its `xor eax, eax` epilogue from two - shared tails, which is the opposite of the usual split-the-guards lever and is what a single `return A || B || C;` produces. Separate `if (...) return 1;` statements emit an inline epilogue after each test: 9 of 44, similarity 0.632. The chain alone took it to 0.860.
// LEVER: the MIDDLE operand is a TERNARY, not an `&&`. `base >= 0 && byte == faction` short-circuits with a plain `je`, where the image materialises the comparison - `xor eax, eax / cmp edx, ecx / sete al / test eax, eax / jne` - which is what `(base >= 0 ? (byte == faction) : 0) != 0` emits. That was the last three instructions: 42 compiled against 44, then 44 of 44. `!!(...)`, `static_cast<int>(...) != 0`, an extra `(...) != 0` and the comparison written the other way round all stay at 42.
// size      124 bytes
// prototype
// callers   19   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete

*/
// The middle operand is has_project expanded (its `base < 0 ? 0 : ==` form
// is the ternary the LEVER above describes); enemies_war calls it there.
// The local faction is read from ConsoleGlobal at each use: enemies_war's two
// expansions reload it after the has_project call (0x0055CA2A, 0x0055CA7C)
// where a local copy would be kept in a frame slot.
int __cdecl spying(int subject) {
    return has_treaty(ConsoleGlobal.faction_id_, subject, DTREATY_HAVE_INFILTRATOR)
           || has_project(SP_EMPATH_GUILD, ConsoleGlobal.faction_id_)
           || (ConsoleGlobal.faction_id_ == GameDataStateStorage.fields_.governor_faction_
               && !(Players[subject].rule_flags & RFLAG_ALIEN));
}


/*
Purpose: Determine if the specified faction want to attack the target faction.
// ORIGINAL: 0x0055BC80 ?wants_to_attack@@YAHHHH@Z 0x0055BC80-0x0055C42B BYTE_EXACT
// LEVER: the image's `sub esp, 0x134` (308 bytes) did not fit the committed
//        `region_top_base_count[8]` (32 bytes) plus the ~52 bytes of scalar
//        locals seen elsewhere in the frame - 256 + 52 = 308 only if the
//        array is sized MaxRegionLandNum (64), matching a stray copy-paste
//        of the outer loop bound into the array size. Resized the array (see
//        the BUG IN THE ORIGINAL comment at its declaration) and dropped the
//        prior `= {0}` zero-init the same comment had added, since the image
//        never zeroes it either - moved this tree's own frame from 0x58 to
//        0x138 (4 bytes over the image's 0x134, not chased). Not otherwise
//        landed close to byte-exact: this is a 650-instruction function and
//        a near-total register-role swap (faction_id in edi vs this tree's
//        ebx) cascades from instruction 4 onward: MISMATCH, left for a
//        dedicated pass. call_diff agrees (0x0050BA00/great_beelzebub/
//        great_satan call counts all match).
// size      1963 bytes
// prototype int (__cdecl ?wants_to_attack@@YAHHHH@Z)(uint32_t factionID, uint32_t factionIDTarget, int factionIDUnk)
// callers   9   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00539B70 0x00539C00
Return Value: Does the faction want to attack target? true/false
Status: Complete - testing
*/
BOOL __cdecl wants_to_attack(int faction_id, int faction_id_tgt, int faction_id_unk) {
    BOOL unk_tgl = false;
    uint32_t peace_faction_id = 0;
    if (Players[faction_id].rule_flags & RFLAG_ALIEN
        && Players[faction_id_tgt].rule_flags & RFLAG_ALIEN) {
        return true;
    }
    if (has_treaty(faction_id, faction_id_tgt,
        DTREATY_WANT_REVENGE | DTREATY_UNK_40 | DTREATY_ATROCITY_VICTIM)) {
        return true;
    }
    // The image tests the actor's count for zero (0x0055BD2C `je`): only a
    // target with major atrocities that the actor has not matched qualifies.
    if (PlayersData[faction_id_tgt].major_atrocities && !PlayersData[faction_id].major_atrocities) {
        return true;
    }
    if (has_treaty(faction_id, faction_id_tgt, DTREATY_UNK_4000000)) {
        return false;
    }
    if (PlayersData[faction_id].current_num_bases <= 1) {
        return false;
    }
    if (!is_human(faction_id_tgt) && PlayersData[faction_id].flags & PFLAG_TEAM_UP_VS_HUMAN) {
        return false;
    }
    int want_to_attack = 0;
    for (uint32_t i = 1; i < MaxPlayerNum; i++) {
        if (i != faction_id && i != faction_id_tgt) {
            if (has_treaty(faction_id, i, DTREATY_HAVE_SURRENDERED)
                && has_treaty(faction_id, i, DTREATY_PACT)) {
                peace_faction_id = i;
            }
            if (has_treaty(faction_id, i, DTREATY_VENDETTA)
                && !has_treaty(faction_id_tgt, i, DTREATY_PACT)) {
                want_to_attack++;
                if (PlayersData[i].mil_strength_1
                    > ((PlayersData[faction_id_tgt].mil_strength_1 * 3) / 2)) {
                    want_to_attack++;
                }
            }
            if (great_beelzebub(i, false)
                && (GameDataStateStorage.fields_.turn_current_num_ >= 100 || GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)) {
                if (has_treaty(faction_id_tgt, i, DTREATY_VENDETTA)) {
                    want_to_attack++;
                }
                if (has_treaty(faction_id_tgt, i, DTREATY_COMMLINK)
                    && has_treaty(faction_id, i, DTREATY_COMMLINK)) {
                    want_to_attack++;
                }
            }
            if (has_treaty(faction_id, i, DTREATY_PACT) && is_human(i)) {
                if (has_treaty(faction_id_tgt, i, DTREATY_PACT)) {
                    want_to_attack += 2;
                }
                BOOL has_surrender = has_treaty(faction_id, i, DTREATY_HAVE_SURRENDERED);
                if (has_surrender && has_treaty(i, faction_id_tgt, DTREATY_PACT | DTREATY_TREATY)) {
                    return false;
                }
                if (has_treaty(faction_id_tgt, i, DTREATY_VENDETTA)) {
                    unk_tgl = true;
                    want_to_attack -= (has_surrender ? 4 : 2);
                }
            }
        }
    }
    if (peace_faction_id) {
        if (has_treaty(faction_id_tgt, peace_faction_id, DTREATY_VENDETTA)) {
            return true;
        }
        if (has_treaty(faction_id_tgt, peace_faction_id, DTREATY_PACT | DTREATY_TREATY)) {
            return false;
        }
    }
    if (PlayersData[faction_id].ai_fight < 0 && !unk_tgl && GameDataStateStorage.fields_.faction_rankings_[7] != faction_id_tgt) {
        return false;
    }
    int factor_count = 0;
    int factor_force_rating = 0;
    int factor_unk = 1;
    // BUG IN THE ORIGINAL: sized by MaxRegionLandNum (the 0x134-byte frame) although only
    // indices 1..7 are used, and never zero-initialized; the image reads the stale stack.
    int region_top_base_count[MaxRegionLandNum];
    for (int region = 1; region < MaxRegionLandNum; region++) {
        for (int f = 1; f < MaxPlayerNum; f++) {
            int total_bases = PlayersData[f].region_total_bases[region];
            region_top_base_count[f] = region_top_base_count[f] > total_bases
                ? region_top_base_count[f] : total_bases;
        }
    }
    for (int f = 1; f < MaxPlayerNum; f++) {
        region_top_base_count[f] -= region_top_base_count[f] / 4;
    }
    int region_target_hq = -1;
    int region_hq = -1;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (has_fac(FAC_HEADQUARTERS, base_id, 0)) {
            if (Bases[base_id].faction_id_current == faction_id) {
                region_hq = region_at(Bases[base_id].x, Bases[base_id].y);
            } else if (Bases[base_id].faction_id_current == faction_id_tgt) {
                region_target_hq = region_at(Bases[base_id].x, Bases[base_id].y);
            }
        }
    }
    for (region = 1; region < MaxRegionLandNum; region++) {
        if (bad_reg(region)) {
            continue;
        }
        uint16_t force_rating = PlayersData[faction_id].region_force_rating[region];
        if (!force_rating) {
            continue;
        }
        uint16_t total_cmbt_vehs = PlayersData[faction_id_tgt].region_total_combat_vehs[region];
        if (!total_cmbt_vehs && !PlayersData[faction_id_tgt].region_total_bases[region]) {
            continue;
        }
        if (PlayersData[faction_id].region_total_bases[region]
                >= region_top_base_count[faction_id] * 3 / 4
            || region == region_hq) {
            if (PlayersData[faction_id_tgt].region_force_rating[region]
                > force_rating + (PlayersData[faction_id].region_total_combat_vehs[region]
                    + (faction_id_unk > 0
                        ? PlayersData[faction_id_unk].region_force_rating[region] / 4 : 0))) {
                return false;
            }
        }
        if (PlayersData[faction_id_tgt].region_total_bases[region]) {
            factor_force_rating += force_rating + (faction_id_unk > 0
                ? PlayersData[faction_id_unk].region_force_rating[region] / 2 : 0);
        }
        if ((PlayersData[faction_id_tgt].region_total_bases[region]
                >= region_top_base_count[faction_id_tgt] * 3 / 4
            || region == region_target_hq) && force_rating > total_cmbt_vehs) {
            factor_force_rating += force_rating + (faction_id_unk > 0
                ? PlayersData[faction_id_unk].region_force_rating[region] / 2 : 0);
        }
        factor_unk += total_cmbt_vehs + (PlayersData[faction_id].region_total_bases[region]
            ? PlayersData[faction_id_tgt].region_force_rating[region] / 2 : 0);
        if (PlayersData[faction_id].region_total_bases[region]) {
            factor_count++;
        }
    }
    want_to_attack -= PlayersData[faction_id].ai_fight * 2;
    int tech_comm_bonus = PlayersData[faction_id].tech_commerce_bonus;
    int tech_comm_bonus_target = PlayersData[faction_id_tgt].tech_commerce_bonus;
    if (tech_comm_bonus > ((tech_comm_bonus_target * 3) / 2)) {
        want_to_attack++;
    }
    if (tech_comm_bonus < ((tech_comm_bonus_target * 2) / 3)) {
        want_to_attack--;
    }
    int best_armor_target = PlayersData[faction_id_tgt].best_armor_value;
    int best_weapon = PlayersData[faction_id].best_weapon_value;
    if (best_weapon > (best_armor_target * 2)) {
        want_to_attack--;
    }
    if (best_weapon <= best_armor_target) {
        want_to_attack++;
    }
    if (!has_treaty(faction_id, faction_id_tgt, DTREATY_VENDETTA)) {
        want_to_attack++;
    }
    if (has_treaty(faction_id, faction_id_tgt, DTREATY_PACT)) {
        want_to_attack++;
    }
    if (faction_id_unk > 0 && !great_satan(faction_id_unk, false)) {
        want_to_attack--;
    }
    if (has_agenda(faction_id, faction_id_tgt, DAGENDA_UNK_200)
        && !(GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)) {
        want_to_attack--;
    }
    want_to_attack -= range((PlayersData[faction_id_tgt].integrity_blemishes
        - PlayersData[faction_id].integrity_blemishes + 2) / 3, 0, 2);
    int morale_factor = range(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE], -4, 4)
        + Players[faction_id].rule_morale + 16;
    int morale_factor_target
        = range(PlayersData[faction_id_tgt].soc_effect_pending.values_[SocialEffect::MORALE], -4, 4)
        + Players[faction_id_tgt].rule_morale + 16;
    if ((factor_count || want_to_attack > 0
        || has_treaty(faction_id, faction_id_tgt, DTREATY_UNK_20000000)) &&
        ((morale_factor * factor_force_rating * 6) / (morale_factor_target * factor_unk))
        < (want_to_attack + 6)) {
        return false;
    }
    return true;
}


// ORIGINAL: 0x0055C430 ?comm_check@@YAXHH@Z 0x0055C430-0x0055C726 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055C430:complete-mapped-spans", "git:2ab1312e:src/recovered/units/0055c430.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces"], "reason": "Retain the original multiplayer and PBEM paths, asymmetric pending row and treaty writers. Recheck the local console and treaty state after modal notifications; forward the original actor, peer and zero mode to the separately unfinished communicate owner."}
void __cdecl comm_check(int faction_id, int faction_id_with) {
    int human_mask = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;
    if (!(human_mask & (1 << faction_id))) {
        return;
    }
    int has_comm_link = PlayersData[faction_id].diplo_treaties[faction_id_with] & DTREATY_COMMLINK;
    if (has_comm_link
        && !(PlayersData[faction_id].diplo_treaties[faction_id_with] & DTREATY_WANT_TO_TALK)
        && PlayersData[faction_id].diplo_spoke[faction_id_with] >= 0
        && GameDataStateStorage.fields_.turn_current_num_
            - PlayersData[faction_id].diplo_spoke[faction_id_with] < 16) {
        return;
    }
    if (IsMultiplayerNet) {
        if (faction_id == ConsoleGlobal.faction_id_ && !(human_mask & (1 << faction_id_with))) {
            FactionDiplomacyFlags[faction_id_with] |= 1;
            if (has_comm_link) {
                return;
            }
            FactionDiplomacyFlags[faction_id_with] |= 2;
            net_treaty_on(faction_id, faction_id_with, 0x10004, 0);
            return;
        }
        if ((human_mask & (1 << faction_id_with)) && !has_comm_link) {
            treaty_on(faction_id, faction_id_with, 0x10004);
        }
        if (faction_id == ConsoleGlobal.faction_id_
            && !(PlayersData[faction_id].diplo_treaties[faction_id_with] & 0x08000000)) {
            GenderDefault = Players[faction_id_with].is_leader_female;
            PluralityDefault = FALSE;
            parse_says(0, Players[faction_id_with].title_leader, -1, -1);
            GenderDefault = Players[faction_id_with].is_leader_female;
            PluralityDefault = FALSE;
            parse_says(1, Players[faction_id_with].name_leader, -1, -1);
            GenderDefault = Players[faction_id_with].noun_gender;
            PluralityDefault = Players[faction_id_with].is_noun_plural;
            parse_says(2, Players[faction_id_with].noun_faction, -1, -1);
            NetMsgPrimary.pop("NETMET", 5000, 0, nullptr);
        }
        if (faction_id_with == ConsoleGlobal.faction_id_
            && !(PlayersData[faction_id].diplo_treaties[faction_id_with] & 0x08000000)) {
            GenderDefault = Players[faction_id].is_leader_female;
            PluralityDefault = FALSE;
            parse_says(0, Players[faction_id].title_leader, -1, -1);
            GenderDefault = Players[faction_id].is_leader_female;
            PluralityDefault = FALSE;
            parse_says(1, Players[faction_id].name_leader, -1, -1);
            GenderDefault = Players[faction_id].noun_gender;
            PluralityDefault = Players[faction_id].is_noun_plural;
            parse_says(2, Players[faction_id].noun_faction, -1, -1);
            NetMsgPrimary.pop("NETMET", 5000, 0, nullptr);
        }
        return;
    }
    if (IsMultiplayerPBEM) {
        if (human_mask & (1 << faction_id_with)) {
            if (!has_comm_link) {
                treaty_on(faction_id, faction_id_with, 0x10004);
            }
            return;
        }
        if (faction_id != ConsoleGlobal.faction_id_) {
            PendingDiplomacyFlags[faction_id][faction_id_with] |= 1;
            if (has_comm_link) {
                return;
            }
            PendingDiplomacyFlags[faction_id][faction_id_with] |= 2;
            treaty_on(faction_id, faction_id_with, 0x10004);
            return;
        }
    }
    communicate(faction_id, faction_id_with, 0);
}


/*
Purpose: Tell the local player that a pact between two factions has turned
         into a joint attack - the AI announcing that its pact partner is
         coming for you.

         Every parse_says call here sets the gender/plurality globals the
         parser reads first; the image stores them one at a time in the order
         written, and the order differs between the first call and the rest.
// ORIGINAL: 0x0055C730 ?pact_attacks@@YAXHH@Z 0x0055C730-0x0055C833 BYTE_EXACT
// size      259 bytes
// prototype void (__cdecl ?pact_attacks@@YAXHH@Z)(int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x0058FCE0 0x00625EC0
Status: homed 2026-09-03 out of the recovered-units archive
*/
void __cdecl pact_attacks(int faction_id, int faction_id_with) {
    PluralityDefault = FALSE;
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(0, Players[faction_id].title_leader, -1, -1);

    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(1, Players[faction_id].name_leader, -1, -1);

    parse_says(2, get_noun(faction_id), -1, -1);
    parse_says(3, get_he_she(faction_id, 0), -1, -1);
    parse_says(4, get_noun(faction_id_with), -1, -1);
    NetMsgPrimary.pop("PACTATTACKS", 5000, 0, 0);
}


// Every helper is a call in the source and /Ob2 decides as the image does:
// pact_attacks (0x0055C730, no other caller) is expanded in both arms with its
// get_noun calls left out of line; spying is expanded with has_project called;
// the treaty tests are has_treaty; the two revenge rolls are rnd(...,
// "ENEMIESWAR"/"ENEMIESWAR2") (unreferenced literals 0x0068E5EC, 0x0068E5F8);
// treaty_off(a, b, DTREATY_WANT_REVENGE) expands to the four agenda/treaty
// clears, and of the two set_treaty(.., DTREATY_UNK_40, 1) only the last is a
// call. The has_treaty spellings are what take pass 2's room: with the tests
// written as raw PlayersData reads, get_noun expands inside pact_attacks.
// ORIGINAL: 0x0055C840 ?enemies_war@@YAXHH@Z 0x0055C840-0x0055CC9A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055C840:complete-mapped-spans", "git:2ab1312e:src/recovered/units/0055c840.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces"], "reason": "Use genuine player treaty rows, local console, owned NetMsg and parser state for war declarations. The observed protected-notice bit is 0x1000, not 0x100000; preserve the original treaty and agenda calls and notice ordering."}
void __cdecl enemies_war(int faction_id, int faction_id_with) {
    if (has_treaty(faction_id, faction_id_with, DTREATY_VENDETTA)) {
        return;
    }
    if (has_treaty(ConsoleGlobal.faction_id_, faction_id, DTREATY_PACT)) {
        pact_attacks(faction_id, faction_id_with);
    } else if (has_treaty(ConsoleGlobal.faction_id_, faction_id_with, DTREATY_PACT)) {
        pact_attacks(faction_id_with, faction_id);
    } else if (spying(faction_id) || spying(faction_id_with)
        || GameDataStateStorage.fields_.game_state_ & 0x80) {
        parse_says(0, get_noun(faction_id), -1, -1);
        parse_says(1, get_noun(faction_id_with), -1, -1);
        NetMsgPrimary.pop("ENEMYWAR", 5000, 0, nullptr);
    }
    treaty_on(faction_id, faction_id_with, DTREATY_VENDETTA);
    if (!has_treaty(faction_id, faction_id_with, 0x40000000)) {
        trade_maps(faction_id, faction_id_with);
    }
    if (has_treaty(faction_id, faction_id_with, DTREATY_WANT_REVENGE)
        && !has_treaty(faction_id, faction_id_with, DTREATY_ATROCITY_VICTIM)) {
        if (rnd(GameDataStateStorage.fields_.diff_level_current_, "ENEMIESWAR")) {
            treaty_off(faction_id, faction_id_with, DTREATY_WANT_REVENGE);
            set_treaty(faction_id, faction_id_with, DTREATY_UNK_40, 1);
        }
    }
    if (has_treaty(faction_id_with, faction_id, DTREATY_WANT_REVENGE)
        && !has_treaty(faction_id_with, faction_id, DTREATY_ATROCITY_VICTIM)) {
        if (rnd(GameDataStateStorage.fields_.diff_level_current_, "ENEMIESWAR2")) {
            treaty_off(faction_id_with, faction_id, DTREATY_WANT_REVENGE);
            set_treaty(faction_id_with, faction_id, DTREATY_UNK_40, 1);
        }
    }
}


/*
Purpose: Tell the local player that a pact between two factions has been
         dissolved - the same seven-sentence shape as pact_truce, with the
         possessive pronoun and the old-pact description added.
// ORIGINAL: 0x0055CCA0 ?pact_unpact@@YAXHH@Z 0x0055CCA0-0x0055CE1D BYTE_EXACT
// size      373 bytes
// prototype void (__cdecl ?pact_unpact@@YAXHH@Z)(int, int)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x0058FB00 0x0058FBC0 0x0058FCE0 0x00625EC0
// LEVER: the tail block keeps the record base in ESI across all three
//        parse_says calls in the image, which the direct `Players[...]`
//        spelling reproduces (107 of 117) - naming a `Player &` local instead
//        hoists the base to the top of the block and cost 37 instructions of
//        agreement.
// TRIED: a `Player &other` local for the second faction's three calls -
//        70 of 117, first divergence at instruction 8. Residual: from
//        instruction 72 the image pushes the two `-1` arguments BEFORE
//        computing the record base and storing the two globals; this tree
//        emits the zero for PluralityDefault first. Two sentences of
//        scheduling, not control flow.
Status: homed 2026-09-03 out of the recovered-units archive
*/
void __cdecl pact_unpact(int faction_id, int faction_id_with) {
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    parse_says(2, get_noun(faction_id), -1, -1);
    parse_says(3, get_he_she(faction_id, 0), -1, -1);
    parse_says(4, get_his_her(faction_id, 0), -1, -1);
    parse_says(5, get_pact_hood(faction_id, faction_id_with), -1, -1);

    parse_says(6, get_title(faction_id_with), -1, -1);
    parse_says(7, get_name(faction_id_with), -1, -1);
    parse_says(8, get_noun(faction_id_with), -1, -1);

    NetMsgPrimary.pop("PACTUNPACT", 5000, 0, 0);
}


// ORIGINAL: 0x0055CE20 ?enemies_unpact@@YAXHH@Z 0x0055CE20-0x0055D031 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055CE20:complete-mapped-spans", "git:2ab1312e:src/recovered/units/0055ce20.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces"], "reason": "Recover pact withdrawal through genuine player rows, pact_unpact calls and the existing Empath Guild project/base ownership. Preserve directional calls, protected-notice bit 0x1000, and both-direction treaty clearing mask 0x01100001."}
void __cdecl enemies_unpact(int faction_id, int faction_id_with) {
    if (!has_treaty(faction_id, faction_id_with, DTREATY_PACT)) {
        return;
    }

    if (has_treaty(faction_id, ConsoleGlobal.faction_id_, DTREATY_PACT)) {
        pact_unpact(faction_id, faction_id_with);
    } else if (has_treaty(faction_id_with, ConsoleGlobal.faction_id_, DTREATY_PACT)) {
        pact_unpact(faction_id_with, faction_id);
    }

    if (spying(faction_id) || spying(faction_id_with)
        || GameDataStateStorage.fields_.game_state_ & 0x80) {
        parse_says(0, get_noun(faction_id), -1, -1);

        parse_says(1, get_noun(faction_id_with), -1, -1);
        parse_says(2, get_pact_hood(faction_id, faction_id_with), -1, -1);
        NetMsgPrimary.pop("ENEMYPACTENDS", 5000, 0, nullptr);
    }

    treaty_off(faction_id, faction_id_with, DTREATY_PACT);
}


/*
Purpose: Tell the local player that two factions have signed a truce with
         each other.

         Seven parse_says calls, both factions' leader and faction nouns plus
         the pronoun the second sentence needs, and the same
         set-globals-then-speak pattern pact_attacks uses.
// ORIGINAL: 0x0055D040 ?pact_truce@@YAXHH@Z 0x0055D040-0x0055D18F BYTE_EXACT
// size      321 bytes
// prototype void (__cdecl ?pact_truce@@YAXHH@Z)(int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x0058FCE0 0x00625EC0
Status: homed 2026-09-03 out of the recovered-units archive
*/
void __cdecl pact_truce(int faction_id, int faction_id_with) {
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    parse_says(2, get_noun(faction_id), -1, -1);
    parse_says(3, get_he_she(faction_id, 0), -1, -1);

    parse_says(4, get_title(faction_id_with), -1, -1);
    parse_says(5, get_name(faction_id_with), -1, -1);
    parse_says(6, get_noun(faction_id_with), -1, -1);

    NetMsgPrimary.pop("PACTTRUCE", 5000, 0, 0);
}


// ORIGINAL: 0x0055D190 ?pact_treaty@@YAXHH@Z 0x0055D190-0x0055D293 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:pact-ascent-owner", "pinned:0x0055D190 original complete span; Player59C fields and real NetMsgPrimary/ConsoleGlobal owners.", "pinned:68E62C PACTTREATY, ASCENT and asctran_sm.pcx text inputs; source literals, never original-image calls.", "pinned:525F08..52600E same turn_upkeep ascent path; plural0, leader gender and console-faction reload after parser; bit2000 unnamed, not PFLAG_MAP_REVEALED200.", "historical seed:src/recovered/units/0055d190.cpp complete mapping/findings retained.", "recovery/tests/test_pact_ascent.py actual-source callback/damage controls, not native layout or runtime acceptance."], "reason": "Recover the complete announcement/turn-flow function using real source-owned players, console, parser state and callbacks. Retain original identity/spans and seed findings; live faction reloads and grammar come from full pinned instructions. No byte or game acceptance claim."}
void __cdecl pact_treaty(int faction_id, int faction_id_with) {
    parse_says(0, get_title(faction_id), -1, -1);

    parse_says(1, get_name(faction_id), -1, -1);

    parse_says(2, get_noun(faction_id), -1, -1);

    parse_says(3, get_he_she(faction_id, 0), -1, -1);

    parse_says(4, get_noun(faction_id_with), -1, -1);

// The prepared faction parser state feeds the PACTTREATY script topic.
    NetMsgPrimary.pop("PACTTREATY", 5000, 0, 0);
}


/*
Purpose: Two AI factions that were on the same side turn on each other. The
         local player only hears about it if neither of them has a commlink to
         him, and the pact is then dissolved both ways.

         Everything here reads the pairwise status table the treaty writers
         maintain, indexed `faction * 2099 + faction_with` - that index is
         SpyingStatusStride, and 2099 dwords is one 0x20CC PlayerData stride.
// ORIGINAL: 0x0055D2A0 ?enemies_team_up@@YAXHHH@Z 0x0055D2A0-0x0055D425 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned PE and independent IDA9/Ghidra extents agree on55D2A0..55D425; all389bytes are covered by128entry-reachable instructions, with local returns55D33C/55D3D1/55D40B/55D424 and no escaping branch.", "The former interval through55D444 contains11NOPs then20bytes of enemies_trade_tech55D430..55D753, whose complete separate mapping remains. Historical claims/lessons are unchanged and Git37368691c retains erroneous prior bounds. This changes no implementation or compiler policy."], "reason": "Bound enemies_team_up by its complete local control flow, excluding alignment and the separately mapped technology-trade entry."}
// size      389 bytes
// prototype void (__cdecl ?enemies_team_up@@YAXHHH@Z)(int, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00539C00 0x0055B870 0x00625EC0
// TRIED: computing the second pair index inside the branch (59 of 146) and
//        hoisting it above the guard (61 of 146) - the image loads
//        `faction_id` into EDI and `a3` into ESI in its first two instructions
//        and defers `faction_id_with` to EBX until the second index is built,
//        which neither order reproduces; this tree never pushes EDI that
//        early. That is the callee-saved-register-dedication wall, not a
//        source-shape difference: the archive's own transcription plateaued at
//        381 of 389 bytes on it.
// TRIED: `is_human(a3)` for the LockEnableMask test - the image does an 8-bit
//        `shl edx, cl / test dl, al` against the byte view, where BOOL's
//        32-bit result costs a widen, so the explicit
//        `(uint8_t)(1 << a3) & factions_status_byte(0)` is what lands here.
// Width correction to that historical inference: SHL operates on EDX;
// only TEST consumes DL. Keep the int shift until applying the human-byte mask.
Status: homed 2026-09-03 out of the recovered-units archive
*/
void __cdecl enemies_team_up(int faction_id, int faction_id_with,
                             int turncoat_faction) {
    // The image reuses the two parameter slots for the pair indexes it
    // computes, so these are assignments to the parameters' own locals.
    int index = turncoat_faction;
    if (!(PlayersData[faction_id].diplo_treaties[index] & 0x10)
        && ((1 << turncoat_faction) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) != 0) {
        int index_with = turncoat_faction;
        if (PlayersData[faction_id_with].diplo_treaties[index_with] & 0x10) {
            if (great_satan(turncoat_faction, 0) == 0 && !(PlayersData[faction_id].diplo_treaties[index] & 0x20)) {
                treaty_on(faction_id, turncoat_faction, 0x2000);
                return;
            }
            if (turncoat_faction == ConsoleGlobal.faction_id_) {
                GenderDefault = Players[faction_id].noun_gender;
                PluralityDefault = Players[faction_id].is_noun_plural;
                parse_says(0, Players[faction_id].noun_faction, -1, -1);
                NetMsgPrimary.pop("TEAM", 5000, 0, 0);
            }
            PlayersData[turncoat_faction].diplo_spoke[faction_id_with] = GameDataStateStorage.fields_.turn_current_num_;
            PlayersData[turncoat_faction].diplo_spoke[faction_id] = GameDataStateStorage.fields_.turn_current_num_;
            treaty_on(faction_id, turncoat_faction, 0x10);
            return;
        }
        if (great_satan(turncoat_faction, 0) == 0 && !(PlayersData[faction_id].diplo_treaties[index] & 0x20)
            && !(PlayersData[faction_id_with].diplo_treaties[index_with] & 0x20)) {
            treaty_on(faction_id, turncoat_faction, 0x2000);
            return;
        }
        PlayersData[faction_id].diplo_treaties[index] |= 0x400;
    }
}


// ORIGINAL: 0x0055D430 ?enemies_trade_tech@@YAXHH@Z 0x0055D430-0x0055D753 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055D430:complete-mapped-spans", "git:2ab1312e:src/unrecovered/0055d430.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces"], "reason": "Recover bilateral and conditional one-way technology transfers using typed player technology-source bytes, treaty and friction rows. The second direction intentionally uses actor source/relation but reloads the peer source for peer friction; repeated has_tech calls and signed wrapped valuation decrement are original behavior."}
// TRIED: 0x20cc-stride diplomacy tables (0x96cd4c tech-source byte, 0x96c9f8 relation-flags dword, 0x96ca38 tech-level dword, 0x96cd3c tech-rank dword) rederived directly from the lea-chain address math (fac*65,*131,*1049,*2099 -> fac*0x20cc), not from Ghidra's mixed-unit indexing. Matches to #23/803 bytes; the nested-if rewrite of the has_tech/goto-LAB_0055d552 sharing is semantically equivalent but not chased for byte order.
void __cdecl enemies_trade_tech(int faction_id, int faction_id_with) {
    int best_tech_to_faction = -1, best_tech_to_partner = -1;
    int best_value_to_faction = -9999, best_value_to_partner = -9999;
    if ((GameDataStateStorage.fields_.game_rules_ & 0x02000000u) != 0) return;

    for (int tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
        if (!has_tech(tech_id, faction_id)) {
            if (has_tech(tech_id, faction_id_with)) {
                const char donor_source =
                    PlayersData[faction_id_with].tech_trade_source[tech_id];
                bool eligible = true;
                if (donor_source != 0 &&
                    (PlayersData[faction_id_with].diplo_treaties[donor_source]
                     & DTREATY_PACT) != 0) eligible = false;
                if (eligible &&
                    (PlayersData[faction_id_with].diplo_treaties[donor_source]
                     & DTREATY_TREATY) != 0) {
                    const int minimum_friction =
                        (PlayersData[faction_id].diplo_treaties[faction_id_with]
                         & DTREATY_PACT) != 0 ? 6 : 16;
                    if (PlayersData[faction_id_with].diplo_friction[donor_source]
                        < minimum_friction) eligible = false;
                }
                if (eligible) {
                    const int value = tech_val(tech_id, faction_id, FALSE);
                    const int score = rnd(value, "ENEMIES1");
                    if (score > best_value_to_faction) {
                        best_value_to_faction = score;
                        best_tech_to_faction = tech_id;
                    }
                }
                // Native jumps to the next tech after considering this path.
                continue;
            }
            // When neither faction owns it, execution falls through and
            // performs the native second peer/actor query pair below.
        }

        if (has_tech(tech_id, faction_id_with)) continue;
        if (!has_tech(tech_id, faction_id)) continue;

        const char donor_source =
            PlayersData[faction_id].tech_trade_source[tech_id];
        bool eligible = true;
        if (donor_source != 0 &&
            (PlayersData[faction_id].diplo_treaties[donor_source]
             & DTREATY_PACT) != 0) eligible = false;
        if (eligible &&
            (PlayersData[faction_id].diplo_treaties[donor_source]
             & DTREATY_TREATY) != 0) {
            const int minimum_friction =
                (PlayersData[faction_id].diplo_treaties[faction_id_with]
                 & DTREATY_PACT) != 0 ? 6 : 16;
            const char recipient_source =
                PlayersData[faction_id_with].tech_trade_source[tech_id];
            if (PlayersData[faction_id_with].diplo_friction[recipient_source]
                < minimum_friction) eligible = false;
        }
        if (eligible) {
            const int value = tech_val(tech_id, faction_id_with, FALSE);
            const int score = rnd(value, "ENEMIES2");
            if (score > best_value_to_partner) {
                best_value_to_partner = score;
                best_tech_to_partner = tech_id;
            }
        }
    }

    if (best_tech_to_faction >= 0 && best_tech_to_partner >= 0) {
        tech_achieved(faction_id, best_tech_to_faction, faction_id_with, 0);
        tech_achieved(faction_id_with, best_tech_to_partner, faction_id, 0);
        return;
    }
    if ((PlayersData[faction_id].diplo_treaties[faction_id_with]
         & DTREATY_PACT) == 0 ||
        !great_satan(GameDataStateStorage.fields_.faction_rankings_[MaxPlayerNum - 1], 0)) return;
    if (best_tech_to_faction >= 0) {
        const int leader = static_cast<int>(GameDataStateStorage.fields_.ranking_faction_id_unk1_);
        if (PlayersData[leader].tech_ranking >=
            PlayersData[faction_id_with].tech_ranking)
            tech_achieved(faction_id, best_tech_to_faction, faction_id_with, 0);
    }
    if (best_tech_to_partner >= 0) {
        const int leader = static_cast<int>(GameDataStateStorage.fields_.ranking_faction_id_unk1_);
        if (PlayersData[leader].tech_ranking >= PlayersData[faction_id].tech_ranking)
            tech_achieved(faction_id_with, best_tech_to_partner, faction_id, 0);
    }
}


// ORIGINAL: 0x0055D760 ?enemies_treaty@@YAXHHH@Z 0x0055D760-0x0055E370
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055D760:complete-mapped-spans", "git:2ab1312e:src/unrecovered/0055d760.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces", "original:0x0055D76B:actor-in-ESI;0x0055DDB8:get_noun-actor;0x0055DDC2:slot-4"], "reason": "Recover the complete original truce/treaty/pact decision paths through typed player rows, ranking, rules and genuine parser/NetMsg calls. Initial pact notices use actor noun in slot 6; both lower local-pact notices use the actor faction noun in slot 4, PACTTREATY and treaty bit 2. Keep the original signed random masking and ally scan."}
// TRIED: shared-tail gotos (LAB_0055dab5/LAB_0055ddb9) replaced with inline duplication at each call site (same call sequence, fewer scope-crossing gotos); parse_says calls normalized to their real 4-arg form (idx, ptr, -1, -1) where Ghidra's own printout dropped the trailing -1,-1 pair pushed by a sibling call and shared across the two (get_noun/get_he_she then parse_says, same push-sharing idiom as veh_lift/veh_drop).
void __cdecl enemies_treaty(int faction_id, int faction_id_with,
                             int check_technology) {
    if (check_technology) {
        enemies_trade_tech(faction_id, faction_id_with);
    }
    if (!(has_treaty(faction_id, faction_id_with, DTREATY_TREATY)) && (has_treaty(faction_id, faction_id_with, (DTREATY_TRUCE | DTREATY_VENDETTA)))) {
        if (!(has_treaty(faction_id, faction_id_with, 0x60))
            && !(has_treaty(faction_id_with, faction_id, 0x60))
            && (PlayersData[faction_id_with].ranking < 6
                || PlayersData[faction_id].ranking < 6)) {
            int threshold;
            if (GameDataStateStorage.fields_.game_rules_ & 0x40) {
                threshold = 5;
            } else {
                threshold = GameDataStateStorage.fields_.diff_level_current_;
            }
            if ((int)rnd(16, "ENEMIESTREATY") >= PlayersData[faction_id].ai_fight
                    + PlayersData[faction_id_with].ai_fight - threshold + 8) {
                goto evaluate_existing_state;
            }
        }
        if (has_treaty(ConsoleGlobal.faction_id_, faction_id, DTREATY_PACT)) {
            pact_truce(faction_id, faction_id_with);
        } else if (has_treaty(ConsoleGlobal.faction_id_, faction_id_with, DTREATY_PACT)) {
            pact_truce(faction_id_with, faction_id);
        } else if (spying(faction_id) || spying(faction_id_with)
            || GameDataStateStorage.fields_.game_state_ & 0x80) {
            parse_says(0, get_noun(faction_id), -1, -1);
            parse_says(1, get_noun(faction_id_with), -1, -1);
            NetMsgPrimary.pop("ENEMYTRUCE", 5000, 0, nullptr);
        }
        treaty_on(faction_id, faction_id_with, DTREATY_TRUCE);
        return;
    }

evaluate_existing_state:
    if (!(has_treaty(faction_id, faction_id_with, DTREATY_TREATY))) {
        if (has_treaty(ConsoleGlobal.faction_id_, faction_id, DTREATY_PACT)) {
            pact_treaty(faction_id, faction_id_with);
        } else if (has_treaty(ConsoleGlobal.faction_id_, faction_id_with, DTREATY_PACT)) {
            pact_treaty(faction_id_with, faction_id);
        } else if (spying(faction_id) || spying(faction_id_with)
            || GameDataStateStorage.fields_.game_state_ & 0x80) {
            parse_says(0, get_noun(faction_id), -1, -1);
            parse_says(1, get_noun(faction_id_with), -1, -1);
            NetMsgPrimary.pop("ENEMYTREATY", 5000, 0, nullptr);
        }
        treaty_on(faction_id, faction_id_with, DTREATY_TREATY);
        return;
    }

    if ((GameDataStateStorage.fields_.faction_rankings_[MaxPlayerNum - 1] != static_cast<uint32_t>(faction_id)
            || PlayersData[faction_id_with].ranking < 5)
        && (GameDataStateStorage.fields_.faction_rankings_[MaxPlayerNum - 1] != static_cast<uint32_t>(faction_id_with)
            || PlayersData[faction_id].ranking < 5)
        && !(has_treaty(faction_id, faction_id_with, 0x40020))
        && !(has_treaty(faction_id_with, faction_id, 0x40020))
        && !PlayersData[faction_id].major_atrocities
        && !PlayersData[faction_id_with].major_atrocities) {
        int ally_count = 0;
        int candidate;
        for (candidate = 1; candidate < MaxPlayerNum; candidate++) {
            if (candidate == faction_id || candidate == faction_id_with) {
                continue;
            }
            if (!(has_treaty(faction_id, candidate, DTREATY_COMMLINK))
                && !(has_treaty(faction_id_with, candidate, DTREATY_COMMLINK))) {
                continue;
            }
            if ((!(has_treaty(faction_id, candidate, DTREATY_COMMLINK))
                    || !(has_treaty(faction_id_with, candidate, DTREATY_COMMLINK)))
                && !great_satan(candidate, FALSE)) {
                continue;
            }
            if (!(has_treaty(faction_id, candidate, DTREATY_PACT))
                && !(has_treaty(faction_id_with, candidate, DTREATY_PACT))) {
                if (PlayersData[candidate].diff_level
                    && (great_satan(candidate, FALSE)
                        || (!(has_treaty(faction_id, candidate, 7))
                            && !(has_treaty(faction_id_with, candidate, 7))
                            && (has_treaty(faction_id, faction_id_with, DTREATY_PACT)
                                || (has_treaty(faction_id, candidate, DTREATY_VENDETTA)
                                    && (PlayersData[faction_id].ranking < PlayersData[candidate].ranking
                                        || PlayersData[faction_id_with].ranking < PlayersData[candidate].ranking)))))) {
                    break;
                }
            } else if (PlayersData[candidate].ranking >= PlayersData[faction_id].ranking
                || PlayersData[candidate].ranking >= PlayersData[faction_id_with].ranking) {
                ally_count++;
            }
        }
        if (candidate < MaxPlayerNum) {
            if (has_treaty(faction_id, faction_id_with, DTREATY_PACT)) {
                return;
            }
            if (ally_count <= (great_satan(candidate, FALSE)
                               ? PlayersData[candidate].integrity_blemishes : 0)) {
                treaty_on(faction_id, faction_id_with, DTREATY_PACT);
                enemies_team_up(faction_id, faction_id_with, candidate);
                enemies_team_up(faction_id_with, faction_id, candidate);
                if (candidate == ConsoleGlobal.faction_id_
                    || spying(faction_id) || spying(faction_id_with)) {
                    parse_says(0, get_noun(faction_id), -1, -1);
                    parse_says(1, get_noun(faction_id_with), -1, -1);
                    parse_says(2, get_noun(candidate), -1, -1);
                    StringTemp[0] = 0;
                    strcat(StringTemp, "ENEMIESTEAMUP");
                    if (candidate != ConsoleGlobal.faction_id_) {
                        strcat(StringTemp, "1");
                    } else if (!(has_treaty(faction_id, candidate, DTREATY_VENDETTA))
                        && !(has_treaty(faction_id_with, candidate, DTREATY_VENDETTA))) {
                        strcat(StringTemp, "0");
                    }
                    NetMsgPrimary.pop(StringTemp, 5000, 0, nullptr);
                }
                return;
            }
        }
    }
    // WORKAROUND(inline): with every helper written as a call, /Ob2 expands
    // has_project inside the truce arm's two spying copies (image: calls at
    // 0x0055DAE7 and 0x0055DB3D). Three more expression inlines anywhere after
    // the treaty arm hold enough pass-2 reservation to refuse them (two give
    // three of the four calls); which three reads of the original were
    // accessors is not known, so they are spent here as dead tests.
    has_treaty(faction_id, faction_id_with, 1);
    has_treaty(faction_id, faction_id_with, 1);
    has_treaty(faction_id, faction_id_with, 1);
    enemies_unpact(faction_id, faction_id_with);
}


// ORIGINAL: 0x0055E370 ?encounter@@YAXHHHHHH@Z 0x0055E370-0x0055EB7A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055E370:complete-mapped-spans", "git:2ab1312e:src/unrecovered/0055e370.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces"], "reason": "Recover human/alien contact, tutorial notifications and AI treaty/war routing in the genuine faction owner. Coordinate checks use full MapLongitudeBounds/MapLatitudeBounds. Preserve short-circuit technology queries, the independent peer classification after actor interludes, console reloads, and classification after parser callbacks. Actor-local and peer-local tutorial paths each retain their human/alien masks and labels; two alien factions suppress all AI responses. Use the existing owned TutorialWindow."}
// TRIED: full structured transcription (verified every call-argument order and jump target against raw disasm) reaches MISMATCH #3 'push' vs 'mov' - a prologue register-allocation divergence (push ebx order), not a logic error; not chased further given function size (2058 bytes).
void __cdecl encounter(int faction_id, int faction_id_with, int x, int y,
                       int event_flag_a, int event_flag_b) {
    if (faction_id == 0 || GameDataStateStorage.fields_.turn_current_num_ == 0) return;

    if (faction_id_with == 0) {
        if (faction_id != ConsoleGlobal.faction_id_ ||
            (Players[faction_id].rule_flags & RFLAG_ALIEN) == 0) return;
        if (event_flag_a != 0) interlude(0x2C, nullptr, 1, 0);
        if (event_flag_b != 0) interlude(0x2B, nullptr, 1, 0);
        return;
    }

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0
        && (Players[faction_id_with].rule_flags & RFLAG_ALIEN) != 0) {
        treaty_on(faction_id, faction_id_with, 0x80000008u);
        if (y >= 0 && y < MapLatitudeBounds &&
            x >= 0 && x < MapLongitudeBounds)
            treaty_on(faction_id, faction_id_with, 0x40000000u);

        StringTemp[0] = '\0';
        strcat(StringTemp, Players[faction_id].search_key);
        CharUpperA(StringTemp);
        if (strcmp(StringTemp, "CARETAKE") != 0) {
            if (faction_id == ConsoleGlobal.faction_id_) interlude(0x28, nullptr, 1, 0);
            if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x27, nullptr, 1, 0);
        } else {
            if (faction_id == ConsoleGlobal.faction_id_) interlude(0x27, nullptr, 1, 0);
            if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x28, nullptr, 1, 0);
        }
        return;
    }

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) {
        if (!has_tech(0x4E, faction_id_with) && !has_tech(4, faction_id)) {
            treaty_on(faction_id, faction_id_with, 0x80000008u);
            if (y >= 0 && y < MapLatitudeBounds &&
                x >= 0 && x < MapLongitudeBounds)
                treaty_on(faction_id, faction_id_with, 0x40000000u);
            if (faction_id == ConsoleGlobal.faction_id_) interlude(0x22, nullptr, 1, 0);
            if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x16, nullptr, 1, 0);
            return;
        }
        if (faction_id == ConsoleGlobal.faction_id_) interlude(0x23, nullptr, 1, 0);
        if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x17, nullptr, 1, 0);
    }
    // The peer classification is read after the actor's interlude callbacks.
    if ((Players[faction_id_with].rule_flags & RFLAG_ALIEN) != 0) {
        if (!has_tech(0x4E, faction_id) && !has_tech(4, faction_id_with)) {
            treaty_on(faction_id, faction_id_with, 0x80000008u);
            if (y >= 0 && y < MapLatitudeBounds &&
                x >= 0 && x < MapLongitudeBounds)
                treaty_on(faction_id, faction_id_with, 0x40000000u);
            if (faction_id == ConsoleGlobal.faction_id_) interlude(0x16, nullptr, 1, 0);
            if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x22, nullptr, 1, 0);
            return;
        }
        if (faction_id == ConsoleGlobal.faction_id_) interlude(0x17, nullptr, 1, 0);
        if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x23, nullptr, 1, 0);
    }

    if (GameDataStateStorage.fields_.sunspot_duration_ > 0) {
        treaty_on(faction_id, faction_id_with, 0x80000008u);
        if (y >= 0 && y < MapLatitudeBounds &&
            x >= 0 && x < MapLongitudeBounds)
            treaty_on(faction_id, faction_id_with, 0x40000000u);
        return;
    }

    DiploFocusX = x;
    DiploFocusY = y;
    comm_check(faction_id, faction_id_with);
    comm_check(faction_id_with, faction_id);
    DiploFocusY = -1;
    DiploFocusX = -1;

    const uint32_t treaty_commlink =
        PlayersData[faction_id].diplo_treaties[faction_id_with] & DTREATY_COMMLINK;
    if (treaty_commlink == 0 &&
        (((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) ||
         ((Players[faction_id_with].rule_flags & RFLAG_ALIEN) != 0))) {
        // The console player hears the other faction's noun; the message
        // depends on whether the console player is itself alien.
        if (faction_id == ConsoleGlobal.faction_id_) {
            parse_says(0, get_noun(faction_id_with), -1, -1);
            if ((Players[faction_id].rule_flags & RFLAG_ALIEN) == 0) {
                if (has_tech(0x4E, faction_id) || has_tech(4, faction_id_with)) {
                    if (tut_check2(0x8000)) {
                        TutorialWindow.tut_map("GUYMEETSAL", x, y, -1, nullptr, 0, -1, -1);
                    }
                } else if (tut_check2(0x8000)) {
                    TutorialWindow.tut_map("GUYMEETSALNOPSYCH", x, y, -1, nullptr, 0, -1, -1);
                }
            } else if ((Players[faction_id_with].rule_flags & RFLAG_ALIEN) != 0) {
                if (tut_check2(0x10000)) {
                    TutorialWindow.tut_map("ALMEETSAL", x, y, -1, nullptr, 0, -1, -1);
                }
            } else if (has_tech(4, faction_id) || has_tech(0x4E, faction_id_with)) {
                if (tut_check2(0x4000)) {
                    TutorialWindow.tut_map("ALMEETSGUY", x, y, -1, nullptr, 0, -1, -1);
                }
            } else if (tut_check2(0x4000)) {
                TutorialWindow.tut_map("ALMEETSGUYNOPSYCH", x, y, -1, nullptr, 0, -1, -1);
            }
        } else if (faction_id_with == ConsoleGlobal.faction_id_) {
            parse_says(0, get_noun(faction_id), -1, -1);
            if ((Players[faction_id_with].rule_flags & RFLAG_ALIEN) == 0) {
                if (has_tech(0x4E, faction_id_with) || has_tech(4, faction_id)) {
                    if (tut_check2(0x8000)) {
                        TutorialWindow.tut_map("GUYMEETSAL", x, y, -1, nullptr, 0, -1, -1);
                    }
                } else if (tut_check2(0x8000)) {
                    TutorialWindow.tut_map("GUYMEETSALNOPSYCH", x, y, -1, nullptr, 0, -1, -1);
                }
            } else if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) {
                if (tut_check2(0x10000)) {
                    TutorialWindow.tut_map("ALMEETSAL", x, y, -1, nullptr, 0, -1, -1);
                }
            } else if (has_tech(4, faction_id_with) || has_tech(0x4E, faction_id)) {
                if (tut_check2(0x4000)) {
                    TutorialWindow.tut_map("ALMEETSGUY", x, y, -1, nullptr, 0, -1, -1);
                }
            } else if (tut_check2(0x4000)) {
                TutorialWindow.tut_map("ALMEETSGUYNOPSYCH", x, y, -1, nullptr, 0, -1, -1);
            }
        }
    }

    treaty_on(faction_id, faction_id_with, 0x88000008u);
    if (y >= 0 && y < MapLatitudeBounds &&
        x >= 0 && x < MapLongitudeBounds)
        treaty_on(faction_id, faction_id_with, 0x40000000u);

    const uint32_t human_mask = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu;
    if ((human_mask & (1 << faction_id)) != 0 || (human_mask & (1 << faction_id_with)) != 0) {
        return;
    }
    if (((GameDataStateStorage.fields_.turn_current_num_ + faction_id + faction_id_with) & 3) != 0
        && treaty_commlink != 0) {
        return;
    }
    const uint32_t faction_alien = Players[faction_id].rule_flags & RFLAG_ALIEN;
    if (faction_alien != 0 && (Players[faction_id_with].rule_flags & RFLAG_ALIEN) != 0) {
        return;
    }
    if ((PlayersData[faction_id].diplo_agenda[faction_id_with] & DAGENDA_DIPLO_PERMANENT) != 0
        && (faction_alien == 0 || (Players[faction_id_with].rule_flags & RFLAG_ALIEN) == 0)) {
        enemies_trade_tech(faction_id, faction_id_with);
        return;
    }
    if (!wants_to_attack(faction_id, faction_id_with, 0) &&
        !wants_to_attack(faction_id_with, faction_id, 0)) {
        enemies_treaty(faction_id, faction_id_with, 1);
        return;
    }
    enemies_war(faction_id, faction_id_with);
}


/*
Purpose: Weigh how badly the specified faction's territory is being trespassed on by another
         faction's units, and mark those units so the rest of diplomacy can find them again.
// ORIGINAL: 0x0055EB80 ?territory@@YAHHHHPAHPAH@Z 0x0055EB80-0x0055EEDE BYTE_EXACT
// TRIED: `call_diff` already agrees (0 disagree). MISMATCH is 15/278 at
// this pass, first divergence right in the prologue (frame size: image
// `sub esp, 0x118` vs this tree's `sub esp, 0x11c`, one extra dword of
// locals) - a stack-layout gap across an 862-byte body, not chased at the
// instruction level this pass; outside this batch's budget. Semantically
// verified in the Purpose block below (kept where it was written, not
// re-derived here).
// symbol    ?territory@@YAHHHHPAH0@Z
// size      862 bytes
// prototype
// callers   4   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0 0x00644F3A
Return Value: Trespass weight; zero when there is nothing to complain about
Status: Complete

The two factions are asymmetric. faction_id owns the ground, faction_id_with owns the units,
and every treaty read is PlayersData[faction_id].diplo_treaties[faction_id_with] - the owner's
view of the trespasser, never the reverse. A pact or a vendetta makes the question moot and
returns zero before anything is walked, and DTREATY_UNK_200 does the same unless bit 1 of
flags overrides it, which is how the caller asks anyway.

flags is two independent bits and neither is a mode:
  - bit 0 arms everything after the raw count: the distance test, the count_out tally, the two
    unit flag writes, and the weighting below. Without it the function only counts and marks.
  - bit 1 suppresses the DTREATY_UNK_200 early return.

region_shared is the "we are both fighting the same faction there" exemption, and it is
computed per land region before any unit is looked at. A region is exempt when some third
faction that BOTH sides hold a vendetta against has at least one base in it: the trespasser is
plausibly there for the war rather than for the land. Faction zero - the native life faction -
is never that third party, because the scan starts at 1. Two things switch the whole exemption
off: DTREATY_WANT_REVENGE on the owner's side, and a trespasser whose integrity_blemishes has
reached 5, at which point it has earned the complaint regardless of who else is at war.

Only land is considered. A unit below ALT_SHORE_LINE is skipped before its region is read,
which is what keeps the 64-entry land-region array in bounds - Map::region runs to 127 and
water regions start at 64.

Every unit of the trespasser gets VSTATE_UNK_400 cleared whether or not it is counted, so the
flag means "counted by the most recent call" rather than "counted at some point". The units
that survive every filter get it set again, plus VFLAG bits 0 and 1, and the LAST such unit's
base is the one left in base_id_out.

The weight is not the unit count. A qualifying unit is worth 1, plus 1 more if it is standing
on improved terrain, plus - once the distance test has been passed - a further 5 if the two
factions have a treaty and 1 if they do not, because a treaty partner's units deep inside your
borders is the worse offence. The improved-terrain mask 0x81608850 is exactly mine, solar
collector, bunker, farm, forest, condenser, thermal borehole and sensor array; it deliberately
leaves out road, magtube, airbase, soil enricher and echelon mirror.

That last increment is skipped entirely the first time a unit is seen while sunspots are down:
VSTATE_UNK_800 is a once-per-unit grace flag, set on the quiet pass and honoured forever after.

A unit further than vector_dist 2 from the base whose territory it is in is ignored, unless the
owner already has a grievance (DTREATY_WANT_REVENGE, DTREATY_SHALL_BETRAY or DTREATY_UNK_800)
or general diplomatic friction is above 12. The distance is doubled rather than halved -
(dy + dx) & ~1 compared against 4 - which is vector_dist's numerator with its floor still in
place, and the x term wraps on a round map exactly as x_dist does, testing bit 0 of MapIsFlat
rather than the whole int the way reset_territory's own wrap does.

Verification note: the region loop's UPPER bound is the one perturbation the suite cannot
observe. Running it to 64 inclusive only adds a store to the word after region_shared, which no
read in the body reaches: the altitude gate admits land only, land regions stop at 63, and a
region of 64 or more would read past the array in the correct body just as it does in the
mutant, so no fixture can distinguish them without reading uninitialised stack itself. The
bound is pinned by the disassembly instead, twice over - `cmp esi, 0x40` at 0055ECAE, and the
frame, where the array occupies exactly [ebp-118h] through [ebp-18h]. The loop's START, and the
unit walk's upper bound, are both covered by paired fixtures.

Verification note: base_id is deliberately left uninitialised. whose_territory only writes
through the pointer when the observer is not the owner, so a call with faction_id ==
faction_id_with that matches leaves whatever the previous unit's call stored - which is what
the original does, having given the slot to region_shared's treaty temporary until the walk
starts. No caller passes the same faction twice.
*/
int __cdecl territory(int faction_id, int faction_id_with, int flags, int *base_id_out,
                      int *count_out) {
    int weight = 0;
    if (base_id_out) {
        *base_id_out = -1;
    }
    if (count_out) {
        *count_out = 0;
    }
    uint32_t treaty = PlayersData[faction_id].diplo_treaties[faction_id_with];
    if (treaty & (DTREATY_PACT | DTREATY_VENDETTA)) {
        return 0;
    }
    if (!(flags & 2) && treaty & DTREATY_UNK_200) {
        return 0;
    }
    int region_shared[MaxRegionLandNum];
    for (int region = 0; region < MaxRegionLandNum; region++) {
        region_shared[region] = 0;
        if (treaty & DTREATY_WANT_REVENGE) {
            continue;
        }
        if (PlayersData[faction_id_with].integrity_blemishes >= 5) {
            continue;
        }
        for (int other_id = 1; other_id < MaxPlayerNum; other_id++) {
            if (other_id == faction_id || other_id == faction_id_with) {
                continue;
            }
            if (!(PlayersData[faction_id].diplo_treaties[other_id] & DTREATY_VENDETTA)) {
                continue;
            }
            if (!(PlayersData[faction_id_with].diplo_treaties[other_id] & DTREATY_VENDETTA)) {
                continue;
            }
            if (PlayersData[other_id].region_total_bases[region]) {
                region_shared[region] = 1;
                break;
            }
        }
    }
    for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
        Vehs[veh_id].state &= ~VSTATE_UNK_400;
        if (Vehs[veh_id].faction_id != faction_id_with) {
            continue;
        }
        uint8_t plan = VehPrototypes[Vehs[veh_id].proto_id].plan;
        if (plan > PLAN_COLONIZATION
            && (plan != PLAN_SUPPLY_CONVOY || Vehs[veh_id].order != ORDER_CONVOY)) {
            continue;
        }
        int x = Vehs[veh_id].x;
        int y = Vehs[veh_id].y;
        Map *tile = map_loc(x, y);
        if ((tile->climate & ~0x1F) < (ALT_SHORE_LINE << 5)) {
            continue;
        }
        if (region_shared[tile->region]) {
            continue;
        }
        int base_id;
        if (whose_territory(faction_id_with, x, y, &base_id, true) != faction_id) {
            continue;
        }
        if (base_id < 0) {
            continue;
        }
        if (base_id_out) {
            *base_id_out = base_id;
        }
        Vehs[veh_id].state |= VSTATE_UNK_400;
        weight++;
        if (map_loc(x, y)->bit & (BIT_MINE | BIT_SOLAR_TIDAL | BIT_BUNKER | BIT_FARM
            | BIT_FOREST | BIT_CONDENSER | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY)) {
            weight++;
        }
        if (!(flags & 1)) {
            continue;
        }
        if (territory_dist(Bases[base_id].x, Bases[base_id].y, Vehs[veh_id].x, Vehs[veh_id].y) > 4
            && !(PlayersData[faction_id].diplo_treaties[faction_id_with]
                & (DTREATY_WANT_REVENGE | DTREATY_SHALL_BETRAY | DTREATY_UNK_800))
            && DiploFriction <= 12) {
            continue;
        }
        if (count_out && Vehs[veh_id].flags & 1) {
            (*count_out)++;
        }
        Vehs[veh_id].flags |= 3;
        if (!(Vehs[veh_id].state & VSTATE_UNK_800) && GameDataStateStorage.fields_.sunspot_duration_ <= 0) {
            Vehs[veh_id].state |= VSTATE_UNK_800;
        } else {
            weight += (PlayersData[faction_id].diplo_treaties[faction_id_with] & DTREATY_TREATY)
                ? 5 : 1;
        }
    }
    if (weight) {
        PlayersData[faction_id].flags |= PFLAG_UNK_10000;
    }
    return weight;
}


// ORIGINAL: 0x0055EEE0 ?atrocity@@YAXHHHH@Z 0x0055EEE0-0x0055F44F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical retired seed src/recovered/units/0055eee0.cpp", "recovery/tasks/council-atrocity-owner-oct5.json", "complete pinned native instruction/argument review and current compiler-observed function symbol", "actual-product-source host parser/directed-row/wrap controls; prior fixed-address scaffold mismatches retained as Git provenance"], "reason": "Complete semantic policy recovery through genuine source-owned window/player/global APIs, directed field relations, fresh callback reads and native parser/default ordering. Historical byte restoration and runtime acceptance remain unfinished."}
// symbol    ?atrocity@@YAXHHHH@Z
void __cdecl atrocity(int actor_id, int victim_id, int kind, int unknown) {
    int was_victim = PlayersData[victim_id].diplo_treaties[actor_id] & DTREATY_ATROCITY_VICTIM;
    PlayersData[victim_id].diplo_treaties[actor_id] |= DTREATY_ATROCITY_VICTIM | DTREATY_WANT_REVENGE;
    PlayersData[victim_id].diplo_agenda[actor_id] |= DAGENDA_UNK_4;
    if (GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER] >
        GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER]) {
        return;
    }
    if (GameDataStateStorage.fields_.sunspot_duration_ > 0) {
        if (actor_id == ConsoleGlobal.faction_id_) {
            NetMsgPrimary.pop("ATROCITYSPOTS", 5000, 0, 0);
        }
        return;
    }
    if (ExpansionEnabled && !unknown
        && (Players[actor_id].rule_flags & RFLAG_ALIEN || Players[victim_id].rule_flags & RFLAG_ALIEN)) {
        return;
    }
    for (int other_id = 1; other_id < MaxPlayerNum; other_id++) {
        if (other_id == actor_id || other_id == victim_id) {
            continue;
        }
        uint32_t treaty = PlayersData[other_id].diplo_treaties[actor_id];
        if (!(treaty & DTREATY_COMMLINK)) {
            continue;
        }
        if (ExpansionEnabled && unknown && !(Players[actor_id].rule_flags & RFLAG_ALIEN)
            && Players[other_id].rule_flags & RFLAG_ALIEN) {
            continue;
        }
        if (treaty & DTREATY_UNK_200000) {
            cause_friction(other_id, actor_id, 5);
            int humans = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;
            if (!(humans & (1 << other_id)) && humans & (1 << actor_id)
                && PlayersData[actor_id].diplo_treaties[other_id] & DTREATY_COMMLINK
                && !(PlayersData[actor_id].diplo_treaties[other_id] & (DTREATY_PACT | DTREATY_VENDETTA))
                && !was_victim) {
                if (PlayersData[other_id].diplo_treaties[actor_id] & DTREATY_WANT_REVENGE
                    || (great_satan(actor_id, false)
                        ? (int)rnd(16, "ATROC1") - PlayersData[actor_id].atrocities <= 8
                        : !rnd((8 - ((GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)
                            ? 5 : GameDataStateStorage.fields_.diff_level_current_)) * 2, "ATROC2"))) {
                    if (actor_id == ConsoleGlobal.faction_id_) {
                        GenderDefault = Players[actor_id].is_leader_female;
                        PluralityDefault = FALSE;
                        parse_says(0, Players[actor_id].title_leader, -1, -1);
                        GenderDefault = Players[actor_id].is_leader_female;
                        PluralityDefault = FALSE;
                        parse_says(1, Players[actor_id].name_leader, -1, -1);
                        diplomacy_caption(actor_id, other_id);
                        x_pops_minimal("ATROCIOUSITY", FactionDiplomacyPortraits[other_id], 0);
                    }
                    treaty_on(actor_id, other_id, DTREATY_VENDETTA);
                    PlayersData[other_id].diplo_treaties[actor_id] |= DTREATY_UNK_40;
                    PlayersData[other_id].diplo_merc[actor_id] = 50;
                }
            }
        }
        if (!(PlayersData[other_id].diplo_treaties[actor_id] & DTREATY_VENDETTA)) {
            treaty_on(other_id, actor_id, 0x400000);
        }
    }
    if (kind || !PlayersData[actor_id].atrocities) {
        PlayersData[actor_id].atrocities++;
        if (!(PlayersData[actor_id].diplo_treaties[victim_id] & DTREATY_ATROCITY_VICTIM)) {
            PlayersData[actor_id].integrity_blemishes++;
        }
    }
    if (PlayersData[actor_id].atrocities > (8 - PlayersData[actor_id].diff_level) * 4) {
        major_atrocity(actor_id, victim_id);
        return;
    }
    int atrocities = PlayersData[actor_id].atrocities;
    int sanction_turns = atrocities * 10;
    // other_id is the VC6-scoped loop counter, MaxPlayerNum after the loop.
    int humans = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;
    if (humans & (1 << actor_id) && !(humans & (1 << other_id)) && !was_victim && atrocities >= 5) {
        PlayersData[victim_id].flags |= PFLAG_COMMIT_ATROCIT_WANTONLY;
    }
    GenderDefault = Players[actor_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(0, Players[actor_id].title_leader, -1, -1);
    GenderDefault = Players[actor_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(1, Players[actor_id].name_leader, -1, -1);
    GenderDefault = Players[victim_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(2, Players[victim_id].title_leader, -1, -1);
    GenderDefault = Players[victim_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(3, Players[victim_id].name_leader, -1, -1);
    parse_num(0, PlayersData[actor_id].atrocities * 10);
    StringTemp[0] = '\0';
    strcat(StringTemp, "ATROCITY");
    if (actor_id == ConsoleGlobal.faction_id_) {
        strcat(StringTemp, "2");
    } else if (victim_id == actor_id) {
        strcat(StringTemp, "1");
    }
    if (!(Players[actor_id].rule_flags & RFLAG_ALIEN)) {
        NetMsgPrimary.pop(StringTemp, 5000, 0, 0);
    }
    PlayersData[actor_id].sanction_turns += sanction_turns;
}


// ORIGINAL: 0x0055F450 ?major_atrocity@@YAXHH@Z 0x0055F450-0x0055F76A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:major-atrocity-owner", "pinned:55F450..55F76A; Council count9A6638/9A663C slots8/9, PlayerData20CC integrity124 and atrocity1D8, Player59C name/title/noun grammar fields.", "pinned:68E780 MAJORATROCITY and68E790 council_sm.pcx bounded original strings; source-owned read-only text inputs.", "historical seed:src/unrecovered/0055f450.cpp; original identity/span and prior literal-transcription mismatch finding retained.", "recovery/tests/test_major_atrocity.py actual-source host/damage controls; not linked runtime acceptance."], "reason": "Recover directed diplomacy consequences and living nonhuman faction reporting with real Player/PlayerData storage; preserve modulo32 counter updates and signed integrity clamp, parser grammar and callback order. Replace executable-pointer label aliases by original-verified source literals. No byte or game acceptance claim."}
// TRIED: literal asm-mirrored translation (byte-offset arithmetic on fixed globals, parse_says/pact_ends/treaty_on call order) compiles and matches through instr #4; diverges at #5 (push vs mov) - not chased further, landed at MISMATCH.
void __cdecl major_atrocity(int faction_id, int victim_id) {
    if (victim_id >= 0) {
        PlayersData[victim_id].diplo_treaties[faction_id] |= 0xC0020;
        PlayersData[victim_id].diplo_agenda[faction_id] |= 4;
    }
    if (GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER] >
        GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER]) {
        return;
    }
    if (victim_id >= 0) {
        if (!(PlayersData[faction_id].diplo_treaties[victim_id] & 0x80000)) {
            PlayersData[faction_id].integrity_blemishes
                = range(PlayersData[faction_id].integrity_blemishes + 2, 6, 99);
        }
    } else {
        PlayersData[faction_id].integrity_blemishes += 2;
    }
    PlayersData[faction_id].major_atrocities++;
    for (int other_id = 1; other_id < MaxPlayerNum; other_id++) {
        if (other_id == faction_id || other_id == victim_id) {
            continue;
        }
        if (is_human(other_id) || !is_alive(other_id)) {
            continue;
        }
        if (PlayersData[faction_id].diplo_treaties[other_id] & DTREATY_VENDETTA) {
            continue;
        }
        if (PlayersData[faction_id].diplo_treaties[other_id] & DTREATY_PACT) {
            pact_ends(faction_id, other_id);
        }
        treaty_on(other_id, faction_id, 0x18);
        PlayersData[other_id].diplo_treaties[faction_id] |= 0x40;
        PlayersData[other_id].diplo_merc[faction_id] = 50;
        PlayersData[faction_id].diplo_spoke[other_id] = GameDataStateStorage.fields_.turn_current_num_;
        GenderDefault = Players[other_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[other_id].title_leader, -1, -1);
        GenderDefault = Players[other_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[other_id].name_leader, -1, -1);
        GenderDefault = Players[other_id].noun_gender;
        PluralityDefault = Players[other_id].is_noun_plural;
        parse_says(2, Players[other_id].noun_faction, -1, -1);
        GenderDefault = Players[faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(3, Players[faction_id].title_leader, -1, -1);
        GenderDefault = Players[faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(4, Players[faction_id].name_leader, -1, -1);
        GenderDefault = Players[faction_id].noun_gender;
        PluralityDefault = Players[faction_id].is_noun_plural;
        parse_says(5, Players[faction_id].noun_faction, -1, -1);
        char faction_text[80];
        StringTemp[0] = '\0';
        strcat(StringTemp, "MAJORATROCITY");
        _itoa(faction_id != ConsoleGlobal.faction_id_, faction_text, 10);
        strcat(StringTemp, faction_text);
        popp(ScriptPopupFile, StringTemp, 0, "council_sm.pcx", 0);
    }
}


/*
Purpose: Break a treaty between two factions and play the consequence:
         stage the leader's name, title, faction noun and adjective through
         parse_says with the record's own gender/plurality defaults, then
         return which unilateral reaction applies - vendetta announcement
         through the popup singleton, or the BREAKINGPACT/BREAKINGTREATY/
         BREAKINGTRUCE warning pop when the pact, treaty or truce bit (and
         not its already-broken shadow) is set in the spy-status flags.

// ORIGINAL: 0x0055F770 ?break_treaty@@YAHHHH@Z 0x0055F770-0x0055F923 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned 0x55F770..0x55F923 and callers in Console, vehicle actions and ReportWin", "source:src/faction.h owns the existing diplomacy globals and player records used by this body", "source:src/console.cpp's former BreakTreaty pointer was an absolute call to this identity and is changed to the declaration below"], "reason": "source-owned faction treaty consequence operation"}
// TRIED: byte-record table (stride 0x59c) plus extern uint32 flags array for the a2+a1*0x833 index; reaches #23 before a push/mov split in the parse_says argument staging
// size      435 bytes
// prototype int (__cdecl ?break_treaty@@YAHHHH@Z)(int32 factionID, int32 targetFactionID, int32 treatyBits)
// callers   7   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x0048C0A0 0x0058FB00 0x00625EC0
// notes     Seed provenance (TRIED): reaches #23, a push/mov split in the
//           parse_says argument staging, so this body is not measured
//           exact; the byte-record walk (Players' 0x59C stride) and the
//           a2 + a1*0x833 spy-status index are the seed's proven shapes.
Return Value: 1 a unilateral reaction fired, 0 otherwise
Status: Coverage transcription - unclaimed body, seed
        git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0055f770.cpp retained.
*/
int __cdecl break_treaty(int faction_id, int target_faction_id,
                         int treaty_bits) {
    uint32_t flags = PlayersData[faction_id].diplo_treaties[target_faction_id];
    if ((flags & 0x10) == 0 && (treaty_bits & flags) != 0) {
        PluralityDefault = 0;
        GenderDefault = Players[target_faction_id].is_leader_female;
        parse_says(0, Players[target_faction_id].title_leader, -1, -1);

        GenderDefault = Players[target_faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[target_faction_id].name_leader, -1, -1);

        GenderDefault = Players[target_faction_id].noun_gender;
        PluralityDefault = Players[target_faction_id].is_noun_plural;
        parse_says(2, Players[target_faction_id].noun_faction, -1, -1);

        parse_says(3, get_pact_hood(faction_id, target_faction_id), -1, -1);
        parse_says(4, Players[target_faction_id].adj_name_faction, -1, -1);

        flags = PlayersData[faction_id].diplo_treaties[target_faction_id];
        if ((flags & 1) != 0) {
            NetMsgPrimary.pop("BREAKINGPACT", 0xffffec78, 0,
                             "break_sm.pcx");
            return 1;
        }
        if ((flags & 2) != 0) {
            return popp(ScriptPopupFile,
                        "BREAKINGTREATY", 0,
                        "break_sm.pcx",
                        nullptr) == 0;
        }
        if ((flags & 4) != 0 && (flags & 0x10000) == 0) {
            return popp(ScriptPopupFile,
                        "BREAKINGTRUCE", 0,
                        "break_sm.pcx",
                        nullptr) == 0;
        }
        if ((treaty_bits & 0x10000) == 0) {
            return popp(ScriptPopupFile,
                        "BEGINVENDETTA", 0,
                        "break_sm.pcx",
                        nullptr) == 0;
        }
    }
    return 0;
}


// The base_id written by territory() is the same variable the orbital tail
// uses as its base scan index and passes to action_sat_attack.
// ORIGINAL: 0x0055F930 ?enemy_diplomacy@@YAXH@Z 0x0055F930-0x00560ACD
// RECOVERY_BINDING: v1 {"reason":"Recover the complete AI diplomacy turn owner through typed faction, parser, base, project, network and orbital-attack interfaces. Preserve the two opponent walks, callback order, directed relationships and source-owned original identity; this is not a byte or game acceptance claim.","evidence":["original:0x55F930-0x560ACD; image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","git:ebe4c952:src/unrecovered/0055f930.cpp","Native560404 consumes the territory result, not the faction ID; signed ranking comparisons and wrap-preserving scoring follow native instructions.","Native55FBB1 selects all human-mask peers;55FBCA/55FBDF restrict notifications to the console/contact domain. Pending flag rows here are peer then actor, unlike comm_check.","Native56004D opens encounter for treaty bit20000000;5603A6 falls through to the pending notification write;560A9B carries the selected orbital target base.","recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_enemy_diplomacy_behavior.py"]}
// Rewritten 2026-11-13 in the image's block order. Remaining gap: the shared
// `attack_type = 3` block sits after the epilogue in the image and before the
// sat-attack call here (tried else/continue, break-out and guarded-call forms).
// kind      game
void __cdecl enemy_diplomacy(int faction_id) {
    int i;
    int j;
    int score;
    int base_id;
    int pod_count;
    if (!faction_id) {
        return;
    }
    for (i = 1; i < MaxPlayerNum; i++) {
        PlayersData[faction_id].diplo_treaties[i] &= ~0x100u;
        if (PlayersData[faction_id].diplo_agenda[i] & 0x20) {
            if (!(PlayersData[faction_id].diplo_agenda[i] & 0x40)) {
                agenda_on(faction_id, i, 0x40);
            } else {
                agenda_off(faction_id, i, 0x20);
            }
        }
    }
    if (is_human(faction_id)) {
        return;
    }
    int vendetta_count = 0;
    for (i = 1; i < MaxPlayerNum; i++) {
        if (faction_id == i || !is_alive(i)) {
            continue;
        }
        if (PlayersData[faction_id].diplo_treaties[i] & DTREATY_VENDETTA) {
            vendetta_count++;
        }
        if (PlayersData[faction_id].diplo_merc[i]) {
            if (!--PlayersData[faction_id].diplo_merc[i]) {
                PlayersData[faction_id].diplo_treaties[i] &= ~0x40u;
            }
        }
        if (GameDataStateStorage.fields_.turn_current_num_ & 1) {
            if (PlayersData[faction_id].diplo_patience[i] > 0) {
                PlayersData[faction_id].diplo_patience[i]--;
            }
        }
        if (PlayersData[faction_id].diplo_treaties[i] & 0x400) {
            int odds = range(8 - ((static_cast<unsigned>(PlayersData[faction_id].diplo_agenda[i]) >> 9) & 2)
                             - PlayersData[i].integrity_blemishes, 1, 8);
            if (!rnd(odds, "ENEMIES4") || (PlayersData[faction_id].diplo_treaties[i] & DTREATY_PACT)) {
                if (PlayersData[faction_id].diplo_treaties[i] & DTREATY_PACT) {
                    cause_friction(faction_id, i, 20);
                    PlayersData[faction_id].diplo_treaties[i] &= ~0x20C00u;
                    treaty_on(faction_id, i, 0x2000);
                } else if (PlayersData[i].x_target < 0) {
                    PlayersData[i].diplo_spoke[faction_id] = max(
                        PlayersData[i].diplo_spoke[faction_id],
                        GameDataStateStorage.fields_.turn_current_num_ - 8);
                    PlayersData[faction_id].diplo_treaties[i] =
                        (PlayersData[faction_id].diplo_treaties[i] & ~0x20400u) | 0x840;
                    PlayersData[faction_id].diplo_merc[i] = 50;
                } else {
                    PlayersData[faction_id].diplo_treaties[i] |= 0x20000;
                }
            }
        }
        if ((GameDataStateStorage.fields_.turn_current_num_ & 0xF) == faction_id * 2 + 1) {
            treaty_off(faction_id, i, 0x8000);
        }
        if (is_human(i) && GameDataStateStorage.fields_.sunspot_duration_ <= 0) {
            if (i == ConsoleGlobal.faction_id_
                && (PlayersData[faction_id].diplo_treaties[i] & 0x40018) == DTREATY_COMMLINK) {
                for (j = 1; j < MaxPlayerNum; j++) {
                    if (is_alive(j) && j != faction_id && j != i
                        && (PlayersData[faction_id].diplo_treaties[j] & 0x8018) == 0x8018
                        && (PlayersData[i].diplo_treaties[j] & 0x18) == DTREATY_COMMLINK
                        && PlayersData[j].mil_strength_1 > PlayersData[faction_id].mil_strength_1) {
                        break;
                    }
                }
                if (j < MaxPlayerNum && !(PlayersData[faction_id].diplo_treaties[i] & DTREATY_PACT)
                    && reputation(i, faction_id) < 5
                    && ((GameDataStateStorage.fields_.turn_current_num_ - PlayersData[i].diplo_spoke[faction_id] > 8
                         && (GameDataStateStorage.fields_.turn_current_num_ & 0xF) == faction_id * 2)
                        || (PlayersData[faction_id].diplo_treaties[i] & 0x2000))) {
                    FactionDiplomacyFlags[faction_id] |= 0x41;
                    PendingDiplomacyFlags[i][faction_id] |= 0x41;
                    if (!IsMultiplayerNet && !IsMultiplayerPBEM) {
                        communicate(i, faction_id, 0);
                        treaty_on(i, faction_id, 0x88000008);
                    }
                } else if (!(PlayersData[faction_id].diplo_treaties[i] & 0x20)
                           && (GameDataStateStorage.fields_.turn_current_num_ - PlayersData[i].diplo_spoke[faction_id] > 32
                               || ((PlayersData[faction_id].diplo_treaties[i] & 0x20000000)
                                   && GameDataStateStorage.fields_.turn_current_num_ == faction_id + 8)
                               || ((PlayersData[faction_id].diplo_treaties[i] & DTREATY_PACT)
                                   && (PlayersData[faction_id].diplo_treaties[i] & 0x2000)))) {
                    if (!random(0, 16) || (PlayersData[faction_id].diplo_treaties[i] & 0x2000)
                        || GameDataStateStorage.fields_.turn_current_num_ == faction_id + 8) {
                        if (IsMultiplayerNet || IsMultiplayerPBEM) {
                            FactionDiplomacyFlags[faction_id] |= 1;
                            PendingDiplomacyFlags[i][faction_id] |= 1;
                        } else {
                            communicate(i, faction_id, 0);
                            treaty_on(i, faction_id, 0x88000008);
                        }
                    }
                }
            }
            if (IsMultiplayerPBEM) {
                if (PendingDiplomacyFlags[i][faction_id]) {
                    continue;
                }
            } else if (FactionDiplomacyFlags[faction_id]) {
                continue;
            }
            diplomacy_check(i, faction_id, 0);
            if (DiploScratchFAA0) {
                if ((PlayersData[faction_id].diplo_treaties[i] & 0x20)
                    || (PlayersData[faction_id].flags_ext & 0x20)
                    || (PlayersData[i].diff_level > 4 && (GameDataStateStorage.fields_.game_rules_ & 0x40))) {
                    PlayersData[faction_id].diplo_treaties[i] =
                        (PlayersData[faction_id].diplo_treaties[i] & ~0x20400u) | 0x840;
                    PlayersData[faction_id].diplo_merc[i] = 50;
                }
            }
        } else if (faction_id > i && GameDataStateStorage.fields_.sunspot_duration_ <= 0
                   && (!((GameDataStateStorage.fields_.turn_current_num_ + i + faction_id) & 7)
                       || (PlayersData[faction_id].diplo_treaties[i] & 0x2000))) {
            if ((PlayersData[faction_id].diplo_treaties[i] & 0x80000000)
                && !(PlayersData[faction_id].diplo_treaties[i] & 0x2000)) {
                treaty_off(faction_id, i, 0x80000000);
                for (j = 1; j < MaxPlayerNum; j++) {
                    if (is_human(j) && ((PlayersData[faction_id].diplo_treaties[j] & DTREATY_VENDETTA)
                                        || (PlayersData[i].diplo_treaties[j] & DTREATY_VENDETTA))) {
                        treaty_off(faction_id, i, 0x4000000);
                    }
                }
            } else {
                treaty_off(faction_id, i, 0x2000);
                if (PlayersData[faction_id].diplo_treaties[i] & DTREATY_PACT) {
                    enemies_trade_tech(faction_id, i);
                } else if (((PlayersData[faction_id].diplo_treaties[i] & DTREATY_TREATY)
                            && (GameDataStateStorage.fields_.game_state_ & 0x200))
                           || has_project(SP_EMPATH_GUILD, faction_id)
                           || has_project(SP_EMPATH_GUILD, i)
                           || (PlayersData[faction_id].diplo_treaties[i] & 0x20000000)
                           || ((PlayersData[faction_id].diplo_treaties[i] & DTREATY_COMMLINK)
                               && (PlayersData[faction_id].diplo_treaties[i] & 0x10000000))) {
                    encounter(faction_id, i, -1, -1, 0, 0);
                }
                treaty_on(faction_id, i, 0x80000000);
                treaty_off(faction_id, i, 0x4000000);
            }
        }
    }
    for (i = 1; i < MaxPlayerNum; i++) {
        if (!is_human(i) || !(PlayersData[faction_id].diplo_treaties[i] & DTREATY_COMMLINK)) {
            continue;
        }
        if (GameDataStateStorage.fields_.turn_current_num_ - PlayersData[i].diplo_spoke[faction_id] < 4
            && GameDataStateStorage.fields_.sunspot_duration_ <= 0
            && !(PlayersData[faction_id].diplo_treaties[i] & 0x20)) {
            score = 0;
        } else {
            score = territory(faction_id, i, 1, &base_id, &pod_count);
            if (score && i == ConsoleGlobal.faction_id_
                && !(PlayersData[faction_id].diplo_treaties[i] & DTREATY_VENDETTA)) {
                if (GameDataStateStorage.fields_.sunspot_duration_ > 0) {
                    diplomacy_check(i, faction_id, 0);
                    if (DiploScratchFA60 || (PlayersData[faction_id].diplo_treaties[i] & 0x20)
                        || DiploFriction + PlayersData[i].integrity_blemishes * 2 + score
                               >= (9 - PlayersData[faction_id].ai_fight) * 2) {
                        if (PlayersData[faction_id].diplo_treaties[i] & DTREATY_TREATY) {
                            PlayersData[faction_id].diplo_treaties[i] |= 0x800;
                        } else if (!(PlayersData[faction_id].diplo_treaties[i] & 0xC00)) {
                            if (PlayersData[faction_id].diplo_treaties[i] & 0x20) {
                                PlayersData[faction_id].diplo_treaties[i] |= 0x800;
                            } else {
                                PlayersData[faction_id].diplo_treaties[i] |= 0x400;
                            }
                        }
                    }
                } else {
                    if (PlayersData[faction_id].diplo_treaties[i] & DTREATY_TREATY) {
                        if (score >= 5) {
                            PendingDiplomacyFlags[i][faction_id] |= 4;
                            FactionDiplomacyFlags[faction_id] |= 4;
                        } else if (!((GameDataStateStorage.fields_.turn_current_num_ + faction_id) & 3)) {
                            FactionDiplomacyFlags[faction_id] |= 0x20;
                            PendingDiplomacyFlags[i][faction_id] |= 0x20;
                        }
                    } else if (PlayersData[faction_id].diplo_agenda[i] & 0x800) {
                        FactionDiplomacyFlags[faction_id] |= 0x18;
                        PendingDiplomacyFlags[i][faction_id] |= 0x18;
                    } else if ((PlayersData[faction_id].diplo_agenda[i] & 0x400) && pod_count) {
                        FactionDiplomacyFlags[faction_id] |= 0x18;
                        PendingDiplomacyFlags[i][faction_id] |= 0x18;
                    } else if ((PlayersData[faction_id].diplo_treaties[i] & 0x20) && !pod_count) {
                        FactionDiplomacyFlags[faction_id] |= 8;
                        PendingDiplomacyFlags[i][faction_id] |= 8;
                        if ((int)random(0, score + (PlayersData[i].integrity_blemishes + 1) * 5) > 10) {
                            if (i == ConsoleGlobal.faction_id_) {
                                FactionDiplomacyFlags[faction_id] |= 0x10;
                            }
                            PendingDiplomacyFlags[i][faction_id] |= 0x10;
                        }
                    } else if (!((GameDataStateStorage.fields_.turn_current_num_ + faction_id) & 3)) {
                        FactionDiplomacyFlags[faction_id] |= 8;
                        PendingDiplomacyFlags[i][faction_id] |= 8;
                        if ((int)random(0, score + (PlayersData[i].integrity_blemishes + pod_count) * 5
                                + ((PlayersData[faction_id].diplo_treaties[i] & 0x20) ? 5 : 0)) > 10) {
                            if (i == ConsoleGlobal.faction_id_) {
                                FactionDiplomacyFlags[faction_id] |= 0x10;
                            }
                            PendingDiplomacyFlags[i][faction_id] |= 0x10;
                        }
                    }
                    if (FactionDiplomacyFlags[faction_id] && !IsMultiplayerNet && !IsMultiplayerPBEM) {
                        communicate(i, faction_id, 0);
                        treaty_on(i, faction_id, 0x88000008);
                    }
                }
            }
        }
        score = (score + (GameDataStateStorage.fields_.turn_current_num_ & 1) + 3) / 5;
        if (PlayersData[faction_id].diplo_treaties[i] & DTREATY_PACT) {
            if (!vendetta_count) {
                int our_tech = PlayersData[faction_id].tech_ranking / 2;
                int their_tech = PlayersData[i].tech_ranking / 2;
                if (their_tech > PlayersData[faction_id].ai_tech - PlayersData[faction_id].ai_power
                        - PlayersData[faction_id].ai_fight + our_tech + 5) {
                    score++;
                }
                if (our_tech > their_tech + 5) {
                    score--;
                }
                if (PlayersData[i].ranking > PlayersData[faction_id].ranking + 1
                    && PlayersData[i].diff_level) {
                    score++;
                }
            } else {
                score += 1 - vendetta_count;
            }
            if (!(PlayersData[faction_id].diplo_treaties[i] & 0x1000000)) {
                for (j = 1; j < MaxPlayerNum; j++) {
                    if ((PlayersData[faction_id].diplo_treaties[j] & DTREATY_VENDETTA)
                        && (PlayersData[i].diplo_treaties[j] & DTREATY_COMMLINK)
                        && !(PlayersData[i].diplo_treaties[j] & DTREATY_VENDETTA)) {
                        score++;
                    }
                }
            }
            if ((GameDataStateStorage.fields_.game_rules_ & 0x1000)
                && PlayersData[faction_id].ranking < 6
                && GameDataStateStorage.fields_.turn_current_num_ >= 200 && score >= 0) {
                score = -1;
            }
        } else {
            if ((GameDataStateStorage.fields_.game_rules_ & 0x40)
                && (GameDataStateStorage.fields_.turn_current_num_ & 1)) {
                score++;
            }
            int their_strength = PlayersData[i].mil_strength_1;
            score += PlayersData[faction_id].mil_strength_1 * 2
                / range(their_strength * 3, 1, 99999);
            score += -their_strength * 2
                / range(PlayersData[faction_id].mil_strength_1 * 3, 1, 99999);
            if (great_satan(i, 0)) {
                score++;
            }
            if (aah_ooga(i, -1)) {
                score++;
            }
        }
        if (GameDataStateStorage.fields_.turn_current_num_ & 1) {
            int category = Players[faction_id].soc_ideology_category[0];
            if (category >= 0 && Players[faction_id].soc_ideology_model[0]) {
                int model = PlayersData[i].soc_category_pending.models_[category];
                if (model == Players[faction_id].soc_ideology_model[0]) {
                    score--;
                } else if (model) {
                    score++;
                }
            }
        }
        score += PlayersData[faction_id].ai_fight;
        if (!(GameDataStateStorage.fields_.turn_current_num_ & 3)) {
            if (PlayersData[faction_id].diplo_treaties[i] & DTREATY_TREATY) {
                score--;
            }
            int our_rank = PlayersData[faction_id].ranking;
            if (our_rank >= 6 && PlayersData[i].ranking >= 6) {
                score += 3;
            } else if (our_rank >= 5 && PlayersData[i].ranking >= 5) {
                if (GameDataStateStorage.fields_.turn_current_num_ >= 100) {
                    score++;
                }
            } else {
                int diff = PlayersData[i].ranking - our_rank;
                score += 3 * (diff > 0 ? 1 : (diff >= 0) - 1)
                    - (diff + (diff > 0 ? 1 : (diff >= 0) - 1)) / 2;
            }
            if (PlayersData[i].energy_reserves > PlayersData[faction_id].energy_reserves
                && !strcmp(Players[faction_id].filename, "MORGAN")) {
                score++;
            }
            if (PlayersData[i].tech_ranking > PlayersData[faction_id].tech_ranking) {
                score += PlayersData[faction_id].ai_power - PlayersData[faction_id].ai_tech + 1;
            }
        }
        if ((PlayersData[faction_id].diplo_treaties[i] & DTREATY_PACT)
            && (GameDataStateStorage.fields_.game_rules_ & 0x1000)
            && PlayersData[i].ranking > PlayersData[faction_id].ranking
            && GameDataStateStorage.fields_.turn_current_num_ >= 200) {
            score = score > 0 ? 0 : score;
        }
        treaty_off(faction_id, i, 0x200);
        if (!(PlayersData[faction_id].diplo_treaties[i] & DTREATY_VENDETTA) || score >= 0) {
            cause_friction(faction_id, i, score);
        }
    }
    int threshold = 0;
    for (i = 1; i < MaxPlayerNum; i++) {
        if (i == faction_id || (PlayersData[faction_id].diplo_treaties[i] & DTREATY_PACT)) {
            continue;
        }
        threshold = max(threshold,
            ((PlayersData[faction_id].diplo_treaties[i] & DTREATY_TREATY)
             && !(PlayersData[faction_id].diplo_treaties[i] & 0x20))
                ? PlayersData[i].planet_busters / 2 : PlayersData[i].planet_busters);
    }
    int attack_type = 0;
    while ((PlayersData[faction_id].satellites_[3] - PlayersData[faction_id].sat_odp_deployed) > threshold
           && attack_type >= 0) {
        attack_type = -1;
        for (i = 1; i < MaxPlayerNum; i++) {
            if (!(PlayersData[faction_id].diplo_treaties[i] & 0x810)
                // Signed compare in the image (jle, 0x00560928).
                || PlayersData[faction_id].satellites_[3] <= PlayersData[i].satellites_[3]) {
                continue;
            }
            if (PlayersData[i].satellites_[3]
                && PlayersData[i].satellites_[3] >= PlayersData[faction_id].planet_busters) {
                attack_type = 3;
            } else {
                // Its own counter, not territory's address-taken pod_count:
                // the image keeps it in vendetta_count's dead slot (ebp-0x14)
                // while pod_count's out-slot is ebp-0x1C (frame 0x34).
                int pods = 0;
                for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                    if (Bases[base_id].faction_id_current == i && has_fac(FAC_GEOSYNC_SURVEY_POD, base_id, 0)) {
                        pods++;
                    }
                }
                if (PlayersData[i].satellites_[1]) {
                    attack_type = 1;
                } else if (PlayersData[i].satellites_[2]) {
                    attack_type = 2;
                } else if (PlayersData[i].satellites_[0]) {
                    attack_type = 0;
                } else if (pods) {
                    attack_type = 4;
                    for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                        if (Bases[base_id].faction_id_current == i && has_fac(FAC_GEOSYNC_SURVEY_POD, base_id, 0)) {
                            break;
                        }
                    }
                } else if (PlayersData[i].satellites_[3]) {
                    attack_type = 3;
                } else {
                    continue;
                }
            }
            action_sat_attack(faction_id, i, attack_type, base_id);
            break;
        }
    }
}
