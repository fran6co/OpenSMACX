#include "stdafx.h"
#include "prefwin_geometry.h"

// PrefWin geometry constants observed in the pinned image. Roles are named
// only where the PrefWin/GraphicWin initialization call arguments establish
// them; the remaining four retain address-bearing provisional names.
// The /Gy measurement recipes emit an arithmetic helper followed by a
// tail-jump wrapper registered in .CRT$XCU. Bind the original arithmetic body
// to the helper, not the wrapper. The ordinary non-/Gy application recipe
// numbers its direct initializer differently; these bindings alone do not
// establish linked startup correspondence or historical TU boundaries.

// RECOVERY_OBJECT: v2 0x00686B6C 0x00686B70 ?PrefWinGeometryConstant_686B6C@@3HA 0x0048D550
int32_t PrefWinGeometryConstant_686B6C = 4;
// RECOVERY_OBJECT: v2 0x00686B70 0x00686B74 ?PrefWinGeometryConstant_686B70@@3HA 0x0048D560
int32_t PrefWinGeometryConstant_686B70 = 4;
// RECOVERY_OBJECT: v2 0x00686B74 0x00686B78 ?PrefWinGeometryConstant_686B74@@3HA 0x0048D5B0
int32_t PrefWinGeometryConstant_686B74 = 3;
// RECOVERY_OBJECT: v2 0x00686B78 0x00686B7C ?PrefWinGeometryConstant_686B78@@3HA 0x0048D5D0
int32_t PrefWinGeometryConstant_686B78 = 3;
// RECOVERY_OBJECT: v2 0x00686B7C 0x00686B80 ?PrefWinDisplayWidth@@3HA 0x0048D570
int32_t PrefWinDisplayWidth = 700;
// RECOVERY_OBJECT: v2 0x00686B80 0x00686B84 ?PrefWinDisplayHeight@@3HA 0x0048D590
int32_t PrefWinDisplayHeight = 390;
// RECOVERY_OBJECT: v2 0x00686B84 0x00686B88 ?PrefWinButtonWidth@@3HA 0x0048D610
int32_t PrefWinButtonWidth = 220;
// RECOVERY_OBJECT: v2 0x00686B88 0x00686B8C ?PrefWinTitleButtonHeight@@3HA 0x0048D840
int32_t PrefWinTitleButtonHeight = 20;
// RECOVERY_OBJECT: v2 0x00686B8C 0x00686B90 ?PrefWinToggleWidth@@3HA 0x0048D820
int32_t PrefWinToggleWidth = 120;
// RECOVERY_OBJECT: v2 0x00686B90 0x00686B94 ?PrefWinToggleHeight@@3HA 0x0048D840
int32_t PrefWinToggleHeight = 20;


/*
Purpose: sub_48d550 - arithmetic over 2 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D550 sub_48d550 0x0048D550-0x0048D55E FILE BYTE_EXACT
// symbol    _$E14
// former symbol ?global_arith_0048d550_redirect@@YIXPAX0@Z
// size      14 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008577EC 0x008577F0 ?PrefWinGeometryValue_8577EC@@3HA 0x0048D550
int32_t PrefWinGeometryValue_8577EC = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryConstant_686B6C) + 23u);

/*
Purpose: sub_48d560 - arithmetic over 2 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D560 sub_48d560 0x0048D560-0x0048D56B FILE BYTE_EXACT
// symbol    _$E17
// former symbol ?global_arith_0048d560_redirect@@YIXPAX0@Z
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008577E8 0x008577EC ?PrefWinGeometryValue_8577E8@@3HA 0x0048D560
int32_t PrefWinGeometryValue_8577E8 = PrefWinGeometryConstant_686B70;

/*
Purpose: sub_48d570 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D570 sub_48d570 0x0048D570-0x0048D587 FILE BYTE_EXACT
// symbol    _$E20
// LEVER: the first global's register must stay live across the doubling. Committed as `void`, VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second load ecx; the image has `lea ecx, [eax + eax]` with the second load in edx, which VC6 emits ONLY when a is still live - four spellings of the statement, `register` storage and every /O flag set produce the identical shl form. Returning the first global is the one shape that keeps it live, and it costs no instruction: the value is already in eax at the ret. No caller in the image, so nothing observes the choice either way. PROOF OF THE MECHANISM: keeping the first global's value alive with a conditional second store compiles the image's exact `mov edx / lea ecx,[eax+eax] / sub edx,ecx` plus that store - liveness is the only thing the `void` spelling was missing.
// former symbol ?global_arith_0048d570_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008578AC 0x008578B0 ?PrefWinGeometryValue_8578AC@@3HA 0x0048D570
int32_t PrefWinGeometryValue_8578AC = static_cast<int32_t>(static_cast<uint32_t>(PrefWinDisplayWidth) - 2u * static_cast<uint32_t>(PrefWinGeometryConstant_686B6C));

/*
Purpose: sub_48d590 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D590 sub_48d590 0x0048D590-0x0048D5A7 FILE BYTE_EXACT
// symbol    _$E23
// LEVER: the first global's register must stay live across the doubling. VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second load ecx; the image has `lea ecx, [eax + eax]` with the second load in edx, which VC6 emits ONLY when the first global is still live - four statement spellings, `register` storage and every /O flag produce the identical shl form. Returning the first global is the one shape that keeps it live, and it costs no instruction: the value is already in eax at the ret. No caller in the image, so nothing observes the choice.
// former symbol ?global_arith_0048d590_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008578B0 0x008578B4 ?PrefWinGeometryValue_8578B0@@3HA 0x0048D590
int32_t PrefWinGeometryValue_8578B0 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinDisplayHeight) - 2u * static_cast<uint32_t>(PrefWinGeometryConstant_686B70));

/*
Purpose: sub_48d5b0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D5B0 sub_48d5b0 0x0048D5B0-0x0048D5C4 FILE BYTE_EXACT
// symbol    _$E26
// LEVER: the first global's register must stay live across the add. As `void`, VC6 computes the sum into the FIRST global's register (`add eax, ecx`, so the image's `add ecx, eax` reads as a different program). Six statement spellings scored identically - the destination is chosen by liveness, not by which operand is written first. Returning the first global keeps its register live, VC6 has to add into the second one's register, and the bytes match with no extra instruction: the value is already in eax at the ret. No caller in the image observes the choice.
// former symbol ?global_arith_0048d5b0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x00857888 0x0085788C ?PrefWinGeometryValue_857888@@3HA 0x0048D5B0
int32_t PrefWinGeometryValue_857888 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8577EC) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74));

/*
Purpose: sub_48d5d0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D5D0 sub_48d5d0 0x0048D5D0-0x0048D5E4 FILE BYTE_EXACT
// symbol    _$E29
// LEVER: the first global's register must stay live across the add. As `void`, VC6 computes the sum into the FIRST global's register (`add eax, ecx`, so the image's `add ecx, eax` reads as a different program). Six statement spellings scored identically - the destination is chosen by liveness, not by which operand is written first. Returning the first global keeps its register live, VC6 has to add into the second one's register, and the bytes match with no extra instruction: the value is already in eax at the ret. No caller in the image observes the choice.
// former symbol ?global_arith_0048d5d0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x00857884 0x00857888 ?PrefWinGeometryValue_857884@@3HA 0x0048D5D0
int32_t PrefWinGeometryValue_857884 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8577E8) + static_cast<uint32_t>(PrefWinGeometryConstant_686B78));

/*
Purpose: sub_48d5f0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D5F0 sub_48d5f0 0x0048D5F0-0x0048D607 FILE BYTE_EXACT
// symbol    _$E32
// LEVER: the first global's register must stay live across the doubling. VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second load ecx; the image has `lea ecx, [eax + eax]` with the second load in edx, which VC6 emits ONLY when the first global is still live - four statement spellings, `register` storage and every /O flag produce the identical shl form. Returning the first global is the one shape that keeps it live, and it costs no instruction: the value is already in eax at the ret. No caller in the image, so nothing observes the choice.
// former symbol ?global_arith_0048d5f0_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008578C0 0x008578C4 ?PrefWinGeometryValue_8578C0@@3HA 0x0048D5F0
int32_t PrefWinGeometryValue_8578C0 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578B0) - 2u * static_cast<uint32_t>(PrefWinGeometryConstant_686B78));

/*
Purpose: sub_48d610 - arithmetic over 4 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D610 sub_48d610 0x0048D610-0x0048D62B FILE SEMANTIC
// symbol    _$E35
// TRIED 2026-09-03: six shapes for the four-global chain, including the
//   register-liveness lever that landed the other sixteen bodies in this
//   file. Closest is declaring the accumulator before the first global,
//   which puts all three loads in the image's order and agrees 4 of 6 - but
//   the first add still lands in eax and the reload of the third global in
//   ecx, the exact mirror of the image's `add ecx,eax; mov eax,[<3rd>]`.
//   The first add only moves into ecx when the first global is still live,
//   and a live first global cannot share its register with the reload. The
//   image has both at once, so this pair is as far as source form goes.
// former symbol ?global_arith_0048d610_redirect@@YIXPAX0@Z
// size      27 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x0085789C 0x008578A0 ?PrefWinGeometryValue_85789C@@3HA 0x0048D610
int32_t PrefWinGeometryValue_85789C = static_cast<int32_t>(static_cast<uint32_t>(PrefWinButtonWidth) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74) + static_cast<uint32_t>(PrefWinGeometryValue_857888));

/*
Purpose: sub_48d630 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D630 sub_48d630 0x0048D630-0x0048D644 FILE BYTE_EXACT
// symbol    _$E38
// LEVER: the first global's register must stay live across the add. As `void`, VC6 computes the sum into the FIRST global's register (`add eax, ecx`, so the image's `add ecx, eax` reads as a different program). Six statement spellings scored identically - the destination is chosen by liveness, not by which operand is written first. Returning the first global keeps its register live, VC6 has to add into the second one's register, and the bytes match with no extra instruction: the value is already in eax at the ret. No caller in the image observes the choice.
// former symbol ?global_arith_0048d630_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x00857898 0x0085789C ?PrefWinGeometryValue_857898@@3HA 0x0048D630
int32_t PrefWinGeometryValue_857898 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8577E8) + static_cast<uint32_t>(PrefWinGeometryConstant_686B78));

// Original ordered initializer at 0x0048D650, between the 0x48D630 and
// 0x48D670 entries in the pinned CRT initializer array.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048D650 sub_48d650 0x0048D650-0x0048D66E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT table entry selects 0048D650, whose typed arithmetic and output store were independently decoded from the pinned executable.", "recovery/tasks/prefwin-geometry-initializers.json", "src/prefwin_geometry.h", "pinned .CRT$XCU slot 0x0068257C + 4*9 points to original 0x0048D650; full original span writes output object 0x8578A8", "prefwin_geometry.cpp namespace-scope initializer for PrefWinGeometryValue_8578A8; this is the candidate source owner, not a generated-symbol qualification", "recovery/tasks/prefwin-geometry-initializers.json"], "reason": "Bind original initializer identity 0x0048D650 to the natural C++ dynamic initializer of PrefWinGeometryValue_8578A8. The source-owned variable declaration is the owner; generated symbol and exact table occurrence remain subject to fresh native COFF/link qualification."}
// symbol    _$E41
// former symbol _sub_48d650
// RECOVERY_OBJECT: v2 0x008578A8 0x008578AC ?PrefWinGeometryValue_8578A8@@3HA 0x0048D650
int32_t PrefWinGeometryValue_8578A8 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578AC) - 3u * static_cast<uint32_t>(PrefWinGeometryConstant_686B74) - static_cast<uint32_t>(PrefWinButtonWidth));

/*
Purpose: sub_48d670 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D670 sub_48d670 0x0048D670-0x0048D687 FILE BYTE_EXACT
// symbol    _$E44
// LEVER: the first global's register must stay live across the doubling. VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second load ecx; the image has `lea ecx, [eax + eax]` with the second load in edx, which VC6 emits ONLY when the first global is still live - four statement spellings, `register` storage and every /O flag produce the identical shl form. Returning the first global is the one shape that keeps it live, and it costs no instruction: the value is already in eax at the ret. No caller in the image, so nothing observes the choice.
// former symbol ?global_arith_0048d670_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008578A4 0x008578A8 ?PrefWinGeometryValue_8578A4@@3HA 0x0048D670
int32_t PrefWinGeometryValue_8578A4 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578B0) - 2u * static_cast<uint32_t>(PrefWinGeometryConstant_686B78));

/*
Purpose: sub_48d690 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D690 sub_48d690 0x0048D690-0x0048D6A4 FILE BYTE_EXACT
// symbol    _$E47
// LEVER: the first global's register must stay live across the add. As `void`, VC6 computes the sum into the FIRST global's register (`add eax, ecx`, so the image's `add ecx, eax` reads as a different program). Six statement spellings scored identically - the destination is chosen by liveness, not by which operand is written first. Returning the first global keeps its register live, VC6 has to add into the second one's register, and the bytes match with no extra instruction: the value is already in eax at the ret. No caller in the image observes the choice.
// former symbol ?global_arith_0048d690_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008578C8 0x008578CC ?PrefWinGeometryValue_8578C8@@3HA 0x0048D690
int32_t PrefWinGeometryValue_8578C8 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_85789C) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74));

/*
Purpose: sub_48d6b0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D6B0 sub_48d6b0 0x0048D6B0-0x0048D6C4 FILE BYTE_EXACT
// symbol    _$E50
// LEVER: the first global's register must stay live across the add. As `void`, VC6 computes the sum into the FIRST global's register (`add eax, ecx`, so the image's `add ecx, eax` reads as a different program). Six statement spellings scored identically - the destination is chosen by liveness, not by which operand is written first. Returning the first global keeps its register live, VC6 has to add into the second one's register, and the bytes match with no extra instruction: the value is already in eax at the ret. No caller in the image observes the choice.
// former symbol ?global_arith_0048d6b0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008578C4 0x008578C8 ?PrefWinGeometryValue_8578C4@@3HA 0x0048D6B0
int32_t PrefWinGeometryValue_8578C4 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_857898) + static_cast<uint32_t>(PrefWinGeometryConstant_686B78));

/*
Purpose: sub_48d6d0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D6D0 sub_48d6d0 0x0048D6D0-0x0048D6E7 FILE BYTE_EXACT
// symbol    _$E53
// LEVER: the first global's register must stay live across the doubling. VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second load ecx; the image has `lea ecx, [eax + eax]` with the second load in edx, which VC6 emits ONLY when the first global is still live - four statement spellings, `register` storage and every /O flag produce the identical shl form. Returning the first global is the one shape that keeps it live, and it costs no instruction: the value is already in eax at the ret. No caller in the image, so nothing observes the choice.
// former symbol ?global_arith_0048d6d0_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008578A0 0x008578A4 ?PrefWinGeometryValue_8578A0@@3HA 0x0048D6D0
int32_t PrefWinGeometryValue_8578A0 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578A8) - 2u * static_cast<uint32_t>(PrefWinGeometryConstant_686B74));

/*
Purpose: sub_48d6f0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D6F0 sub_48d6f0 0x0048D6F0-0x0048D704 FILE BYTE_EXACT
// symbol    _$E56
// LEVER: the first global's register must stay live across the add. As `void`, VC6 computes the sum into the FIRST global's register (`add eax, ecx`, so the image's `add ecx, eax` reads as a different program). Six statement spellings scored identically - the destination is chosen by liveness, not by which operand is written first. Returning the first global keeps its register live, VC6 has to add into the second one's register, and the bytes match with no extra instruction: the value is already in eax at the ret. No caller in the image observes the choice.
// former symbol ?global_arith_0048d6f0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x0085788C 0x00857890 ?PrefWinGeometryValue_85788C@@3HA 0x0048D6F0
int32_t PrefWinGeometryValue_85788C = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_857888) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74));

/*
Purpose: sub_48d710 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D710 sub_48d710 0x0048D710-0x0048D724 FILE BYTE_EXACT
// symbol    _$E59
// LEVER: the first global's register must stay live across the add. As `void`, VC6 computes the sum into the FIRST global's register (`add eax, ecx`, so the image's `add ecx, eax` reads as a different program). Six statement spellings scored identically - the destination is chosen by liveness, not by which operand is written first. Returning the first global keeps its register live, VC6 has to add into the second one's register, and the bytes match with no extra instruction: the value is already in eax at the ret. No caller in the image observes the choice.
// former symbol ?global_arith_0048d710_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x00857890 0x00857894 ?PrefWinGeometryValue_857890@@3HA 0x0048D710
int32_t PrefWinGeometryValue_857890 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_857884) + static_cast<uint32_t>(PrefWinGeometryConstant_686B78));

/*
Purpose: sub_48d730 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D730 sub_48d730 0x0048D730-0x0048D747 FILE BYTE_EXACT
// symbol    _$E62
// LEVER: the first global's register must stay live across the doubling. VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second load ecx; the image has `lea ecx, [eax + eax]` with the second load in edx, which VC6 emits ONLY when the first global is still live - four statement spellings, `register` storage and every /O flag produce the identical shl form. Returning the first global is the one shape that keeps it live, and it costs no instruction: the value is already in eax at the ret. No caller in the image, so nothing observes the choice.
// former symbol ?global_arith_0048d730_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x00857894 0x00857898 ?PrefWinGeometryValue_857894@@3HA 0x0048D730
int32_t PrefWinGeometryValue_857894 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinButtonWidth) - 2u * static_cast<uint32_t>(PrefWinGeometryConstant_686B74));

/*
Purpose: sub_48d750 - arithmetic over 2 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D750 sub_48d750 0x0048D750-0x0048D75B FILE BYTE_EXACT
// symbol    _$E65
// former symbol ?global_arith_0048d750_redirect@@YIXPAX0@Z
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008577D8 0x008577DC ?PrefWinGeometryValue_8577D8@@3HA 0x0048D750
int32_t PrefWinGeometryValue_8577D8 = PrefWinGeometryValue_8578C8;

/*
Purpose: sub_48d760 - arithmetic over 2 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D760 sub_48d760 0x0048D760-0x0048D770 FILE BYTE_EXACT
// symbol    _$E68
// former symbol ?global_arith_0048d760_redirect@@YIXPAX0@Z
// size      16 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008577DC 0x008577E0 ?PrefWinGeometryValue_8577DC@@3HA 0x0048D760
int32_t PrefWinGeometryValue_8577DC = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8577E8) + 0x11Bu);

/*
Purpose: sub_48d770 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D770 sub_48d770 0x0048D770-0x0048D787 FILE BYTE_EXACT
// symbol    _$E71
// LEVER: the first global's register must stay live across the doubling. VC6 folds `2*a` into a's own register (`shl eax, 1`) and gives the second load ecx; the image has `lea ecx, [eax + eax]` with the second load in edx, which VC6 emits ONLY when the first global is still live - four statement spellings, `register` storage and every /O flag produce the identical shl form. Returning the first global is the one shape that keeps it live, and it costs no instruction: the value is already in eax at the ret. No caller in the image, so nothing observes the choice.
// former symbol ?global_arith_0048d770_redirect@@YIHPAX0@Z
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008577E4 0x008577E8 ?PrefWinGeometryValue_8577E4@@3HA 0x0048D770
int32_t PrefWinGeometryValue_8577E4 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578A8) - 2u * static_cast<uint32_t>(PrefWinGeometryConstant_686B74));

/*
Purpose: sub_48d790 - arithmetic over 4 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D790 sub_48d790 0x0048D790-0x0048D7AB FILE BYTE_EXACT
// symbol    _$E74
// former symbol ?global_arith_0048d790_redirect@@YIXPAX0@Z
// size      27 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008577E0 0x008577E4 ?PrefWinGeometryValue_8577E0@@3HA 0x0048D790
int32_t PrefWinGeometryValue_8577E0 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578B0) - static_cast<uint32_t>(PrefWinGeometryConstant_686B78) - static_cast<uint32_t>(PrefWinGeometryValue_8577DC));

/*
Purpose: sub_48d7b0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D7B0 sub_48d7b0 0x0048D7B0-0x0048D7C4 FILE BYTE_EXACT
// symbol    _$E77
// LEVER: the first global's register must stay live across the add. As `void`, VC6 computes the sum into the FIRST global's register (`add eax, ecx`, so the image's `add ecx, eax` reads as a different program). Six statement spellings scored identically - the destination is chosen by liveness, not by which operand is written first. Returning the first global keeps its register live, VC6 has to add into the second one's register, and the bytes match with no extra instruction: the value is already in eax at the ret. No caller in the image observes the choice.
// former symbol ?global_arith_0048d7b0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008578B4 0x008578B8 ?PrefWinGeometryValue_8578B4@@3HA 0x0048D7B0
int32_t PrefWinGeometryValue_8578B4 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8577D8) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74));

/*
Purpose: sub_48d7d0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D7D0 sub_48d7d0 0x0048D7D0-0x0048D7E4 FILE BYTE_EXACT
// symbol    _$E80
// LEVER: the first global's register must stay live across the add. As `void`, VC6 computes the sum into the FIRST global's register (`add eax, ecx`, so the image's `add ecx, eax` reads as a different program). Six statement spellings scored identically - the destination is chosen by liveness, not by which operand is written first. Returning the first global keeps its register live, VC6 has to add into the second one's register, and the bytes match with no extra instruction: the value is already in eax at the ret. No caller in the image observes the choice.
// former symbol ?global_arith_0048d7d0_redirect@@YIHPAX0@Z
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008578BC 0x008578C0 ?PrefWinGeometryValue_8578BC@@3HA 0x0048D7D0
int32_t PrefWinGeometryValue_8578BC = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8577DC) + static_cast<uint32_t>(PrefWinGeometryConstant_686B78));

/*
Purpose: sub_48d7f0 - arithmetic over 4 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D7F0 sub_48d7f0 0x0048D7F0-0x0048D80B FILE SEMANTIC
// symbol    _$E83
// TRIED 2026-09-03: same six shapes as 0x0048D610, same wall - see that
//   marker. The image's `add ecx,eax` needs the first global live past the
//   first add, and its register is the one the third global's reload wants.
// former symbol ?global_arith_0048d7f0_redirect@@YIXPAX0@Z
// size      27 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x008578CC 0x008578D0 ?PrefWinGeometryValue_8578CC@@3HA 0x0048D7F0
int32_t PrefWinGeometryValue_8578CC = static_cast<int32_t>(static_cast<uint32_t>(PrefWinToggleWidth) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74) + static_cast<uint32_t>(PrefWinGeometryValue_8578B4));

/*
Purpose: sub_48d810 - arithmetic over 2 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D810 sub_48d810 0x0048D810-0x0048D81E FILE
// symbol    _$E86
// TRIED: the last of four instructions. The image has `add eax, -2` (83 C0 FE); VC6 emits `sub eax, 2` (83 E8 02) and will not be talked out of it. Six spellings measured with `tools/try_spellings.py` - `eax + -2`, `eax += -2`, unsigned `+ 0xFFFFFFFEu`, an int cast around the unsigned add, and both one-liner forms - all score 3 of 4, identically. All ten flag sets score 3 of 4 or worse. The two are semantically the same instruction; the encoding is not reachable from C++ with this compiler.
// TRIED: the one divergence is an ENCODING choice VC6 will not make - the image has `add eax, -2` (83 C0 FE), this tree emits `sub eax, 2` (83 E8 02), same semantics and same length. `eax = eax + -2` is already the source form and VC6 canonicalises it to a subtract; so does the unsigned spelling `eax + 0xFFFFFFFEu`, measured. A literal negative addend cannot reach that encoding, so whatever the original wrote, the -2 did not arrive as a constant here.
// former symbol ?global_arith_0048d810_redirect@@YIXPAX0@Z
// size      14 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
// TRIED 2026-09-03, second pass, still 3 of 4: `eax -= 2`, `eax = eax - 2`,
//   `eax + ~1` (~1 is -2), `eax - 1 - 1`, `eax + 0xFFFFFFFE` and the unsigned
//   wrap `uint32_t eax + 0xFFFFFFFEu` all compile the identical
//   `mov/sub/store/ret`; `eax - (-2)` compiles `add eax, 2`, which is the
//   proof that the sign of the immediate is what VC6 canonicalises away.
//   /G6 measured too, in the tree's own ladder and directly: identical.
//   The register-liveness lever that landed the other 24 bodies in this file
//   does not apply - there is one variable and it is live to the store either
//   way, so no spelling of the statement changes the encoder's choice.
*/
// RECOVERY_OBJECT: v2 0x008578D0 0x008578D4 ?PrefWinGeometryValue_8578D0@@3HA 0x0048D810
int32_t PrefWinGeometryValue_8578D0 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578BC) - 2u);

// Original final initializer in the pinned geometry family.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048D820 sub_48d820 0x0048D820-0x0048D83E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT table entry selects 0048D820, whose typed arithmetic and output store were independently decoded from the pinned executable.", "recovery/tasks/prefwin-geometry-initializers.json", "src/prefwin_geometry.h", "pinned .CRT$XCU slot 0x0068257C + 4*25 points to original 0x0048D820; full original span writes output object 0x8578B8", "prefwin_geometry.cpp namespace-scope initializer for PrefWinGeometryValue_8578B8; this is the candidate source owner, not a generated-symbol qualification", "recovery/tasks/prefwin-geometry-initializers.json"], "reason": "Bind original initializer identity 0x0048D820 to the natural C++ dynamic initializer of PrefWinGeometryValue_8578B8. The source-owned variable declaration is the owner; generated symbol and exact table occurrence remain subject to fresh native COFF/link qualification."}
// symbol    _$E89
// former symbol _sub_48d820
// RECOVERY_OBJECT: v2 0x008578B8 0x008578BC ?PrefWinGeometryValue_8578B8@@3HA 0x0048D820
int32_t PrefWinGeometryValue_8578B8 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8577E4) - 3u * static_cast<uint32_t>(PrefWinGeometryConstant_686B74) - static_cast<uint32_t>(PrefWinToggleWidth));
