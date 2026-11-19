/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// ORIGINAL: 0x00492410 ??_GPrefWin@@WEEE@AEPAXI@Z 0x00492410-0x0049241B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original ??_GPrefWin@@WEEE@AEPAXI@Z at 0x00492410 is an 11-byte secondary scalar deleting adjustor; it subtracts 0x444 from ECX and tail-jumps to the complete-object PrefWin scalar deleting entry without touching stack or flags.", "Historical seed correction: the earlier `??3` deallocation interpretation was refuted; the original is a `??_G` scalar-deleting adjustor. `WEEE` demangles to adjustor{1092}, equal to 0x444.", "Pinned PrefWin constructor at 0x004923c2 stores Buffer-view vftable 0x0066d41c at this+0x444; the source owner is the typed PrefWin GraphicWin/Buffer multiple-inheritance view.", "Current src/prefwin.cpp COFF Buffer-view vftable first slot selects generated ??_EPrefWin@@WEEE@AEPAXI@Z; its body performs the same 0x444 adjustment and preserves the flag argument.", "Current PrefWin COFF weak external ??_EPrefWin@@UAEPAXI@Z records search mode 2 (library search) and fallback target ??_GPrefWin@@UAEPAXI@Z; this linked image resolves both primary labels to the same physical class deleting body. The object fallback and linked address are per-build evidence, not a general E/G equivalence or owner-selection rule.", "That measured current primary body calls PrefWin::~PrefWin, tests flags bit 0, calls operator delete only when bit 0 is set, returns the saved object pointer, and uses ret 4; this is the per-class measured E-to-G scalar path, not a general vector/scalar equivalence.", "The removed transitional C++ names were ?adjust_this_pref_win@@YIPAXPAX0I@Z and ?scalar_delete_pref_win@@YIPAXPAX0I@Z; no callers exist outside deleting_thunks.cpp/.h, and the current generated Buffer vftable uses the class-owned E adjustor instead.", "Historical seed metadata noted flags hidden;sp_ready;purged_ok, zero direct callers, and no direct call targets. These remain historical observations only; they do not establish virtual-dispatch completeness or acceptance.", "The old secondary record had no BYTE_EXACT or SEMANTIC claim. Its original identity, full span, kind and parsed history are moved unchanged; no matching claim is granted."], "reason": "Bind the pinned secondary scalar deleting identity for PrefWin to its measured genuine Buffer-view generated adjustor. Keep the original G identity and point its current symbol fact at the owner-emitted E adjustor; the per-object weak fallback and bit-0 path above establish why this exact current implementation preserves the original operation."}
// symbol    ??_EPrefWin@@WEEE@AEPAXI@Z
// kind      game
// ORIGINAL: 0x004923E0 ??_GPrefWin@@UAEPAXI@Z 0x004923E0-0x00492401 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004923E0-0x00492401", "Pinned class constructor ??0PrefWin@@QAE@XZ at 0x004921E0 stores original class/view table VA 0x66d424 at 0x004923BC (mov dword ptr [esi], 0x66d424); the current genuine owner vftable ??_7PrefWin@@6BWin@@@ selects generated ??_GPrefWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EPrefWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GPrefWin@@UAEPAXI@Z at 0x004923E0 calls the source-mapped ??1PrefWin@@QAE@XZ at 0x00491DF0, then tests byte ptr [ebp + 8], 1; je 0x4923fa bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/prefwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original PrefWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GPrefWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x0048D4F0 _$E21 0x0048D4F0-0x0048D506 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682574 selects original initializer0x48d4f0..0x48d506, registering fini0x48d510..0x48d51a; complete independent decode establishes exclusive boundaries.", "Source-owned object?ConsolePrefWin@@3VPrefWin@@A identifies original storage0x8578d8. Startup arguments/calls: call 0x4921e0; push 0x48d510. Cleanup receiver/calls: mov ecx, 0x8578d8; jmp 0x491df0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/prefwin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?ConsolePrefWin@@3VPrefWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E16
// ORIGINAL: 0x0048D510 _$E19 0x0048D510-0x0048D51A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682574 selects original initializer0x48d4f0..0x48d506, registering fini0x48d510..0x48d51a; complete independent decode establishes exclusive boundaries.", "Source-owned object?ConsolePrefWin@@3VPrefWin@@A identifies original storage0x8578d8. Startup arguments/calls: call 0x4921e0; push 0x48d510. Cleanup receiver/calls: mov ecx, 0x8578d8; jmp 0x491df0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/prefwin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?ConsolePrefWin@@3VPrefWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E14
// ORIGINAL: 0x0048D520 _$E26 0x0048D520-0x0048D536 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682578 selects original initializer0x48d520..0x48d536, registering fini0x48d540..0x48d54a; complete independent decode establishes exclusive boundaries.", "Source-owned object?PrefWinButtonGroup@@3VButtonGroup@@A identifies original storage0x8577f0. Startup arguments/calls: call 0x62b7c0; push 0x48d540. Cleanup receiver/calls: mov ecx, 0x8577f0; jmp 0x62b7f0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/prefwin.cpp COFF selects _$E26 through .CRT$XCU and registers _$E24, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?PrefWinButtonGroup@@3VButtonGroup@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E21
// ORIGINAL: 0x0048D540 _$E24 0x0048D540-0x0048D54A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682578 selects original initializer0x48d520..0x48d536, registering fini0x48d540..0x48d54a; complete independent decode establishes exclusive boundaries.", "Source-owned object?PrefWinButtonGroup@@3VButtonGroup@@A identifies original storage0x8577f0. Startup arguments/calls: call 0x62b7c0; push 0x48d540. Cleanup receiver/calls: mov ecx, 0x8577f0; jmp 0x62b7f0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/prefwin.cpp COFF selects _$E26 through .CRT$XCU and registers _$E24, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?PrefWinButtonGroup@@3VButtonGroup@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E19
#include "stdafx.h"
#include "interface_fonts.h"
#include "prefwin.h"

#include <string.h>

#include "alpha.h"
#include "ambience.h"
#include "buttongroup.h"
#include "buffer.h"
#include "console.h"
#include "faction.h"
#include "game.h"
#include "interface_palette.h"
#include "gamma.h"
#include "prefwin_geometry.h"
#include "main.h"
#include "mapwin.h"
#include "palette.h"
#include "popup.h"
#include "text.h"

#include "tutwin.h"
#include "rectangle_operations.h"
#include "wave_device.h"
#include "worldwin.h"
// Counter canary: PrefWin::init 0x0048D840 flips with the front-end symbol
// counter (guide 2.5; fe sweep 2026-11-19: exact at 0-7, 10, 12, 14-15, 17-18,
// 20-41, 46, 49-50, 52-53, 55-56, 59, 62 pads). After the dead-declaration
// batch: exact at 2-4, 6-9, 12, 14, 16-17, 19-20, 22-43, ... (0 and 1 flip it), so 2.
// After the par-3 literal sweep (eight volume keys as literals): exact at 1, 4-7, 9, 13, 15-18, 21-26, ..., so 1.
// After the faction.h label externs left (2026-11-19): exact at 0, 3, 5, 7, 8, 10-12, 15, 18-21, ..., so no pads.
// After the dead-helper batch (d64959c815), whole-unit fe sweep: exact at 2-5, 8-9, 12, 14, 16-17, 19-21, 24, 27-30,
// 32, 36, 38-41, 44-49 and 51-63 (0 and 1 flip it), so 57, the middle of the widest run.
// CANARY-PADS: function PrefWinDeclFnPad
void PrefWinDeclFnPad0();
void PrefWinDeclFnPad1();
void PrefWinDeclFnPad2();
void PrefWinDeclFnPad3();
void PrefWinDeclFnPad4();
void PrefWinDeclFnPad5();
void PrefWinDeclFnPad6();
void PrefWinDeclFnPad7();
void PrefWinDeclFnPad8();
void PrefWinDeclFnPad9();
void PrefWinDeclFnPad10();
void PrefWinDeclFnPad11();
void PrefWinDeclFnPad12();
void PrefWinDeclFnPad13();
void PrefWinDeclFnPad14();
void PrefWinDeclFnPad15();
void PrefWinDeclFnPad16();
void PrefWinDeclFnPad17();
void PrefWinDeclFnPad18();
void PrefWinDeclFnPad19();
void PrefWinDeclFnPad20();
void PrefWinDeclFnPad21();
void PrefWinDeclFnPad22();
void PrefWinDeclFnPad23();
void PrefWinDeclFnPad24();
void PrefWinDeclFnPad25();
void PrefWinDeclFnPad26();
void PrefWinDeclFnPad27();
void PrefWinDeclFnPad28();
void PrefWinDeclFnPad29();
void PrefWinDeclFnPad30();
void PrefWinDeclFnPad31();
void PrefWinDeclFnPad32();
void PrefWinDeclFnPad33();
void PrefWinDeclFnPad34();
void PrefWinDeclFnPad35();
void PrefWinDeclFnPad36();
void PrefWinDeclFnPad37();
void PrefWinDeclFnPad38();
void PrefWinDeclFnPad39();
void PrefWinDeclFnPad40();
void PrefWinDeclFnPad41();
void PrefWinDeclFnPad42();
void PrefWinDeclFnPad43();
void PrefWinDeclFnPad44();
void PrefWinDeclFnPad45();
void PrefWinDeclFnPad46();
void PrefWinDeclFnPad47();
void PrefWinDeclFnPad48();
void PrefWinDeclFnPad49();
void PrefWinDeclFnPad50();
void PrefWinDeclFnPad51();
void PrefWinDeclFnPad52();
void PrefWinDeclFnPad53();
void PrefWinDeclFnPad54();
void PrefWinDeclFnPad55();
void PrefWinDeclFnPad56();



























int __cdecl X_text_open(char *src_file_id, const char *section_id);
void __cdecl slide_show(GraphicWin *window, int mode);
void __cdecl slide_hide(GraphicWin *window, int mode);
// The process-wide preference window is a genuine PrefWin object. Its
// original CRT initializer is 0x48D4F0, before ButtonGroup at 0x48D520.
// RECOVERY_OBJECT: v2 0x008578D8 0x00872CA8 ?ConsolePrefWin@@3VPrefWin@@A 0x004921E0
PrefWin ConsolePrefWin;   // 0x008578D8
// The six preference-page tabs have their own ButtonGroup at 0x8577F0,
// immediately before the initialized geometry words beginning at 0x857884.
// RECOVERY_OBJECT: v2 0x008577F0 0x00857884 ?PrefWinButtonGroup@@3VButtonGroup@@A 0x0048D840
ButtonGroup PrefWinButtonGroup;
// PrefWin geometry constants observed in the pinned image. Roles are named
// only where the PrefWin/GraphicWin initialization call arguments establish
// them; the remaining four retain address-bearing provisional names.
// The original CRT slots 0x0068257C..0x006825E0 select these 26 arithmetic
// initializers directly, in declaration order. Bind the production /Ob2
// CRT-selected entries, not the separate helper names removed by inlining.
// The explicit measurement context retains that selection with /Gy as well.
// These bindings do not establish whole-program startup order, linked
// retention or historical TU boundaries.

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
// symbol    _$E24
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
// symbol    _$E27
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
// symbol    _$E30
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
// symbol    _$E33
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
// symbol    _$E36
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
// symbol    _$E39
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
// symbol    _$E42
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
// ORIGINAL: 0x0048D610 sub_48d610 0x0048D610-0x0048D62B FILE BYTE_EXACT
// symbol    _$E45
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
int32_t PrefWinGeometryValue_85789C = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_857888) + static_cast<uint32_t>(PrefWinButtonWidth) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74));

/*
Purpose: sub_48d630 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D630 sub_48d630 0x0048D630-0x0048D644 FILE BYTE_EXACT
// symbol    _$E48
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
// symbol    _$E51
// former symbol _sub_48d650
// RECOVERY_OBJECT: v2 0x008578A8 0x008578AC ?PrefWinGeometryValue_8578A8@@3HA 0x0048D650
int32_t PrefWinGeometryValue_8578A8 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578AC) - 3u * static_cast<uint32_t>(PrefWinGeometryConstant_686B74) - static_cast<uint32_t>(PrefWinButtonWidth));

/*
Purpose: sub_48d670 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D670 sub_48d670 0x0048D670-0x0048D687 FILE BYTE_EXACT
// symbol    _$E54
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
// symbol    _$E57
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
// symbol    _$E60
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
// symbol    _$E63
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
// symbol    _$E66
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
// symbol    _$E69
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
// symbol    _$E72
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
// symbol    _$E75
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
// symbol    _$E78
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
// symbol    _$E81
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
// symbol    _$E84
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
int32_t PrefWinGeometryValue_8577E0 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578B0) - static_cast<uint32_t>(PrefWinGeometryValue_8577DC) - static_cast<uint32_t>(PrefWinGeometryConstant_686B78));

/*
Purpose: sub_48d7b0 - arithmetic over 3 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D7B0 sub_48d7b0 0x0048D7B0-0x0048D7C4 FILE BYTE_EXACT
// symbol    _$E87
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
// symbol    _$E90
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
// ORIGINAL: 0x0048D7F0 sub_48d7f0 0x0048D7F0-0x0048D80B FILE BYTE_EXACT
// symbol    _$E93
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
int32_t PrefWinGeometryValue_8578CC = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8578B4) + static_cast<uint32_t>(PrefWinToggleWidth) + static_cast<uint32_t>(PrefWinGeometryConstant_686B74));

/*
Purpose: sub_48d810 - arithmetic over 2 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0048D810 sub_48d810 0x0048D810-0x0048D81E FILE BYTE_EXACT
// symbol    _$E96
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
// symbol    _$E99
// former symbol _sub_48d820
// RECOVERY_OBJECT: v2 0x008578B8 0x008578BC ?PrefWinGeometryValue_8578B8@@3HA 0x0048D820
int32_t PrefWinGeometryValue_8578B8 = static_cast<int32_t>(static_cast<uint32_t>(PrefWinGeometryValue_8577E4) - 3u * static_cast<uint32_t>(PrefWinGeometryConstant_686B74) - static_cast<uint32_t>(PrefWinToggleWidth));


// Original caption storage begins NUL; the writable extent and indirect
// writers at its image location 0x872CAC remain unproved.
static char PrefWinCaption[] = "";

// The geometry object is recovered in the adjacent global-arithmetic owner.
// Keep these as ordinary object declarations here; they are not executable
// address aliases and their startup initializers remain compiler-owned.
extern int32_t PrefWinDisplayWidth, PrefWinDisplayHeight, PrefWinButtonWidth;
extern int32_t PrefWinTitleButtonHeight, PrefWinToggleWidth, PrefWinToggleHeight;
extern int32_t PrefWinGeometryConstant_686B78;
// Original 0x48D884 writes 50 to this separate DWORD on the 800-pixel path.
// RECOVERY_OBJECT: v2 0x00872CA8 0x00872CAC ?PrefWinNarrowDisplayVerticalOffset@@3HA 0x0048D840
int PrefWinNarrowDisplayVerticalOffset;
// MessageWin's two cells of the iface.pcx colour table (game.h declares it).
// WORKAROUND(decl): the former local declaration stays, shadowed by the macros.
extern int MessageWinTextColor, MessageWinTextBackground;
#define MessageWinTextColor      (IfaceColorTable[1])
#define MessageWinTextBackground (IfaceColorTable[4])
// Mutable preference state begins at 1.0; the 100.0 factor converts the
// stored gamma scalar to the integer value written to the preferences file.
// RECOVERY_OBJECT: v2 0x006846A8 0x006846AC ?PrefWinGammaCorrection@@3MA 0x00491400
float PrefWinGammaCorrection = 1.0f;
// The 100.0 factor at 0x0066D58C is the compiler's float literal for the
// `* 100.0f` in on_button_clicked (it is fmul's memory operand, loaded after
// the correction), not a named object.


static Win *console_window() {
  return static_cast<Win *>(static_cast<GraphicWin *>(&ConsoleGlobal));
}

static Win *preference_window() {
  // Embedded controls belong to the canonical preferences window (8578D8),
  // not its Console parent (9156B0).
  return static_cast<Win *>(static_cast<GraphicWin *>(&ConsolePrefWin));
}

static int label_index(int byte_offset) { return byte_offset / 4; }


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004921E0 ??0PrefWin@@QAE@XZ 0x004921E0-0x004923DA;0x00658930-0x00658ABC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004921e0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
PrefWin::PrefWin() : GraphicWin() {}

// ORIGINAL: 0x00491DF0 ??1PrefWin@@QAE@XZ 0x00491DF0-0x004921DF;0x006586C0-0x00658924 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00491df0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// compile_context src/prefwin.cpp
// TRIED: real nested member declarations (arrays/Scroll/CheckBox/NetWin in true layout order) letting the compiler auto-cascade the teardown and build the SEH frame - instead used explicit reinterpret_cast<T*>(self+offset)->close()/~T() calls matching the disasm's two FlatButton arrays (stride 0xB4C), 5 Scrolls, one CheckBox+Dialog+GraphicWin triple, and 6-element NetWin array (stride 0xB28, closed via UNK4()) in program order.
// symbol    ??1PrefWin@@UAE@XZ
// The compiler owns this implicit destructor. The pinned complete stage has
// no PrefWin table stores before the embedded controls and GraphicWin teardown.

// ORIGINAL: 0x0048D840 ?init@PrefWin@@QAEXXZ 0x0048D840-0x0048F9F5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0048d840.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// The first button of each group is reached through a held control pointer:
// set_font addresses the Buffer base as [edi+0x444] (0x48DA0A, 0x48F6E9,
// 0x48F8F3) where the member spelling folds the offset into `this`; a
// FlatButton* local assigned at those three sites reproduces it. The check
// boxes take their sprites through CheckBox::set_sprites.
void PrefWin::init() {
  int frame_adjustment = 0;                                   // 48D84D [ebp-4]=ebx
  field_A14_ = 0;
  field_A18_ = 0;
  field_A1C_ = 0;
  field_A5C_ = 0;

  int top;                                                    // edi
  if (WinScreenWidth == 800) {                                // 48D868 [9B7B1C]
    // ? 48D876: image loads 0x32 into eax once, subtracts it from [8578D0]
    // and stores it to [872CA8]; either two constant statements or
    // `PrefWinGeometryValue_8578D0 -= (PrefWinNarrowDisplayVerticalOffset = 50);`
    PrefWinNarrowDisplayVerticalOffset = 50;                  // 48D883 [872CA8]
    PrefWinGeometryValue_8578D0 -= 50;                        // 48D888 [8578D0]
    top = 0;                                                  // 48D88E
    frame_adjustment = 0x50;                                  // 48D890
  } else {
    top = WinScreenHeight - 0x26A;                            // 48D899 [9B7B20]
  }
  // 48D8A4: x = WinScreenWidth/2 - PrefWinDisplayWidth/2 (both signed /2).
  // Parent = Console (0x9156B0) converted to Win* through its virtual base,
  // with the compiler's null test (48D8C0).
  int left = WinScreenWidth / 2 - PrefWinDisplayWidth / 2;
  GraphicWin::init(left, top,
                   PrefWinDisplayWidth + 0x4E, PrefWinDisplayHeight,
                   PrefWinCaption /* 0x872CAC */, 0x1000000,
                   console_window(), 0, 0);                   // 48D8F9
  GraphicWin::fill(9);                                        // 48D902

  font1_ = find_font(10, 0);                                  // 48D90A  (current src: 16)
  font2_ = find_font(12, 0);                                  // 48D918
  font3_ = find_font(12, 2);                                  // 48D927
  font4_ = find_font(12, 1);                                  // 48D936
  font5_ = find_font(14, 0);                                  // 48D944
  font6_ = find_font(14, 2);                                  // 48D953
  font7_ = find_font(14, 1);                                  // 48D962
  // ? 48D970: the font arguments are read from the absolute addresses
  // 0x872C90/0x872C9C (= ConsolePrefWin+0x1B3B8 / +0x1B3C4), not through esi,
  // although the receiver is this->Buffer (esi+0x444). Source likely spells
  // them ConsolePrefWin.font2_ / ConsolePrefWin.font5_.
  Buffer::set_font(ConsolePrefWin.font2_, ConsolePrefWin.font2_,
                   ConsolePrefWin.font5_, 0);                 // 48D987
  Buffer::set_text_color(MessageWinTextColor /* [68A5A4] */,
                         MessageWinTextBackground /* [68A5B0] */, 1, 1); // 48D99F

  // ---- page tabs: label copied into StringTemp, then BaseButton::init ----
  StringTemp[0] = 0;                                          // 48D9AA
  strcat(StringTemp, label_get(750));   /* 0xBB8 */           // 48D9C7
  FlatButton *button = &bscPrefButton_;
  button->init(StringTemp, 0, PrefWinGeometryValue_85788C,
                      PrefWinGeometryValue_857890,
                      PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
                      preference_window() /* 0x8578D8 */, 0); // 48D9FD
  button->Buffer::set_font(&TutWinBaseWindow.font1_ /* 0x6E84B4 */, 0, 0, 0); // 48DA10
  button->redraw();              // ? 48DA19 virtual slot +0xF8 // 48DA19

  StringTemp[0] = 0;                                          // 48DA24
  strcat(StringTemp, label_get(751));   /* 0xBBC */           // 48DA41
  advPrefButton_.init(StringTemp, 1, PrefWinGeometryValue_85788C,
                      PrefWinGeometryValue_857890 + 0x16,
                      PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
                      preference_window(), 0);                // 48DA79
  advPrefButton_.set_font_redraw(&TutWinBaseWindow.font1_);

  StringTemp[0] = 0;                                          // 48DAA8
  strcat(StringTemp, label_get(752));   /* 0xBC0 */           // 48DAC5
  warnPrefButton_.init(StringTemp, 2, PrefWinGeometryValue_85788C,
                       PrefWinGeometryValue_857890 + 0x2C,
                       PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
                       preference_window(), 0);               // 48DAFD
  warnPrefButton_.set_font_redraw(&TutWinBaseWindow.font1_);

  StringTemp[0] = 0;                                          // 48DB2C
  strcat(StringTemp, label_get(753));   /* 0xBC4 */           // 48DB49
  autoPrefButton_.init(StringTemp, 3, PrefWinGeometryValue_85788C,
                       PrefWinGeometryValue_857890 + 0x42,
                       PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
                       preference_window(), 0);               // 48DB81
  autoPrefButton_.set_font_redraw(&TutWinBaseWindow.font1_);

  StringTemp[0] = 0;                                          // 48DBB0
  strcat(StringTemp, label_get(754));   /* 0xBC8 */           // 48DBCD
  audioVisPrefButton_.init(StringTemp, 4, PrefWinGeometryValue_85788C,
                           PrefWinGeometryValue_857890 + 0x58,
                           PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
                           preference_window(), 0);           // 48DC05
  audioVisPrefButton_.set_font_redraw(&TutWinBaseWindow.font1_);

  StringTemp[0] = 0;                                          // 48DC34
  strcat(StringTemp, label_get(1077));  /* 0x10D4 */          // 48DC51
  mapPrefButton_.init(StringTemp, 5, PrefWinGeometryValue_85788C,
                      PrefWinGeometryValue_857890 + 0x6E,
                      PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
                      preference_window(), 0);                // 48DC89
  mapPrefButton_.set_font_redraw(&TutWinBaseWindow.font1_);

  StringTemp[0] = 0;                                          // 48DCB8
  strcat(StringTemp, label_get(1078));  /* 0x10D8 */          // 48DCD5
  gammaButton_.init(StringTemp, 6, PrefWinGeometryValue_85788C,
                    PrefWinGeometryValue_857890 + 0x9A,
                    PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
                    preference_window(), 0);                  // 48DD10
  gammaButton_.set_font_redraw(&TutWinBaseWindow.font1_);

  PrefWinButtonGroup.init(0x5D, 0);                           // 48DD42
  PrefWinButtonGroup.add(&bscPrefButton_);   // edi still = this+0xA60  48DD4D
  PrefWinButtonGroup.add(&advPrefButton_);                    // 48DD5E
  PrefWinButtonGroup.add(&warnPrefButton_);                   // 48DD6F
  PrefWinButtonGroup.add(&autoPrefButton_);                   // 48DD80
  PrefWinButtonGroup.add(&audioVisPrefButton_);               // 48DD91
  PrefWinButtonGroup.add(&mapPrefButton_);                    // 48DDA2
  PrefWinButtonGroup.set(0, 1);                               // 48DDAF

  // ---- BASICPREFS ----
  if (X_text_open(ScriptPopupFile, "BASICPREFS" /* 0x686B98 */)) return; // 48DDBF
  text_get();                                                 // 48DDCF
  text_get();                                                 // 48DDD4
  text_get();                                                 // 48DDD9
  // Header inline CheckBox::init(l,t,r,b,heap_size): close(); Dialog::init(...)
  // through the virtual Dialog base (vbtable [edi]+8).
  bscPrefCheckBox_.init(PrefWinGeometryValue_8578C8, PrefWinGeometryValue_8578C4,
                        PrefWinGeometryValue_8578A0, PrefWinGeometryValue_8578A4,
                        0);                                   // 48DE09 / 48DE26
  bscPrefCheckBox_.set_sprites(&g_IFACE_CLOSE_X_SPRITES[1], &g_IFACE_CLOSE_X_SPRITES[2]);
  StringTemp[0] = 0;  text_get();                             // 48DE39
  strcat(StringTemp, " " /* 0x686BA4 */);
  strcat(StringTemp, TextBufferGetPtr /* [9B7D00] */);
  bscPrefCheckBox_.item(StringTemp, 7, 0);                    // 48DE71
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BA8 */);
  strcat(StringTemp, TextBufferGetPtr);
  bscPrefCheckBox_.item(StringTemp, 8, 0);                    // 48DEAE
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BAC */);
  strcat(StringTemp, TextBufferGetPtr);
  bscPrefCheckBox_.item(StringTemp, 9, 0);                    // 48DEEA
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BB0 */);
  strcat(StringTemp, TextBufferGetPtr);
  bscPrefCheckBox_.item(StringTemp, 10, 0);                   // 48DF27
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BB4 */);
  strcat(StringTemp, TextBufferGetPtr);
  bscPrefCheckBox_.item(StringTemp, 11, 0);                   // 48DF64
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BB8 */);
  strcat(StringTemp, TextBufferGetPtr);
  bscPrefCheckBox_.item(StringTemp, 12, 0);                   // 48DFA0
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BBC */);
  strcat(StringTemp, TextBufferGetPtr);
  bscPrefCheckBox_.item(StringTemp, 13, 0);                   // 48DFDD
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BC0 */);
  strcat(StringTemp, TextBufferGetPtr);
  bscPrefCheckBox_.item(StringTemp, 14, 0);                   // 48E01A
  // Virtual attach via the Dialog vbase vtable slot 1 (+4).
  bscPrefCheckBox_.attach(&ConsolePrefWin, PrefWinGeometryValue_8578C8,
                          PrefWinGeometryValue_8578C4, 0x1000120); // 48E048
  text_close();                                               // 48E04B

  // ---- ADVANCEDPREFS ----
  if (X_text_open(ScriptPopupFile, "ADVANCEDPREFS" /* 0x686BC4 */)) return; // 48E05C
  text_get();
  text_get();
  text_get();                                                 // 48E06C..48E076
  advPrefCheckBox_.init(PrefWinGeometryValue_8578C8, PrefWinGeometryValue_8578C4,
                        PrefWinGeometryValue_8578A0, PrefWinGeometryValue_8578A4,
                        0);                                   // 48E0A6 / 48E0C3
  advPrefCheckBox_.set_sprites(&g_IFACE_CLOSE_X_SPRITES[1], &g_IFACE_CLOSE_X_SPRITES[2]);
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BD4 */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 15, 0);                   // 48E10E
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BD8 */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 16, 0);                   // 48E14A
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BDC */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 17, 0);                   // 48E187
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BE0 */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 18, 0);                   // 48E1C4
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BE4 */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 19, 0);                   // 48E200
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BE8 */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 28, 0);                   // 48E23D
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BEC */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 20, 0);                   // 48E27A
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BF0 */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 21, 0);                   // 48E2B6
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BF4 */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 23, 0);  // ? 48E2E9 pushes before mov ecx,edi (scheduling only) // 48E2F3
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BF8 */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 24, 0);                   // 48E330
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686BFC */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 25, 0);                   // 48E36C
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C00 */);
  strcat(StringTemp, TextBufferGetPtr);
  advPrefCheckBox_.item(StringTemp, 26, 0);                   // 48E3A9
  advPrefCheckBox_.attach(&ConsolePrefWin, PrefWinGeometryValue_8578C8,
                          PrefWinGeometryValue_8578C4, 0x1000120); // 48E3D7
  text_close();                                               // 48E3DA

  // ---- AUTOMATIONPREFS ----
  if (X_text_open(ScriptPopupFile, "AUTOMATIONPREFS" /* 0x686C04 */)) return; // 48E3EB
  text_get();
  text_get();
  text_get();                                                 // 48E3FB..48E405
  autoPrefCheckBox_.init(PrefWinGeometryValue_8578C8, PrefWinGeometryValue_8578C4,
                         PrefWinGeometryValue_8578A0, PrefWinGeometryValue_8578A4,
                         0);                                  // 48E435 / 48E452
  autoPrefCheckBox_.set_sprites(&g_IFACE_CLOSE_X_SPRITES[1], &g_IFACE_CLOSE_X_SPRITES[2]);
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C14 */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x1D, 0);                // 48E49D
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C18 */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x1E, 0);                // 48E4D9
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C1C */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x1F, 0);                // 48E516
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C20 */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x20, 0);                // 48E553
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C24 */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x21, 0);                // 48E58F
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C28 */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x22, 0);                // 48E5CC
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C2C */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x2A, 0);                // 48E609
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C30 */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x23, 0);                // 48E645
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C34 */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x24, 0);  // ? 48E678 pushes before mov ecx,edi (scheduling only) // 48E682
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C38 */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x25, 0);                // 48E6BF
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C3C */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x26, 0);                // 48E6FB
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C40 */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x27, 0);                // 48E738
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C44 */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x28, 0);                // 48E775
  StringTemp[0] = 0;  text_get();
  strcat(StringTemp, " " /* 0x686C48 */);
  strcat(StringTemp, TextBufferGetPtr);
  autoPrefCheckBox_.item(StringTemp, 0x29, 0);                // 48E7B1
  autoPrefCheckBox_.attach(&ConsolePrefWin, PrefWinGeometryValue_8578C8,
                           PrefWinGeometryValue_8578C4, 0x1000120); // 48E7E1
  text_close();                                               // 48E7E4

  // ---- part 2 continues at 0x48E7E9 (4th X_text_open) ----
  // 0x48E7E9
  if (X_text_open(ScriptPopupFile, "AUDIOVISUALPREFS")) return;  // 0x686C4C
  text_get();
  text_get();
  text_get();
  text_get();
  text_get();
  text_get();
  audioVisPrefCheckBox_.init(PrefWinGeometryValue_8578C8, PrefWinGeometryValue_8578C4, PrefWinGeometryValue_8578A0,
      PrefWinGeometryValue_8578A4 - PrefWinGeometryValue_8577E0 - PrefWinGeometryConstant_686B78 - frame_adjustment, 0);
  audioVisPrefCheckBox_.set_sprites(&g_IFACE_CLOSE_X_SPRITES[1], &g_IFACE_CLOSE_X_SPRITES[2]);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C60
  strcat(StringTemp, TextBufferGetPtr);
  audioVisPrefCheckBox_.item(StringTemp, 0x40, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C64
  strcat(StringTemp, TextBufferGetPtr);
  audioVisPrefCheckBox_.item(StringTemp, 0x41, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C68
  strcat(StringTemp, TextBufferGetPtr);
  audioVisPrefCheckBox_.item(StringTemp, 0x42, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C6C
  strcat(StringTemp, TextBufferGetPtr);
  audioVisPrefCheckBox_.item(StringTemp, 0x47, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C70
  strcat(StringTemp, TextBufferGetPtr);
  audioVisPrefCheckBox_.item(StringTemp, 0x43, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C74
  strcat(StringTemp, TextBufferGetPtr);
  audioVisPrefCheckBox_.item(StringTemp, 0x44, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C78
  strcat(StringTemp, TextBufferGetPtr);
  audioVisPrefCheckBox_.item(StringTemp, 0x45, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C7C
  strcat(StringTemp, TextBufferGetPtr);
  audioVisPrefCheckBox_.item(StringTemp, 0x16, 0);
  audioVisPrefCheckBox_.attach(&ConsolePrefWin, PrefWinGeometryValue_8578C8, PrefWinGeometryValue_8578C4, 0x1000120);
  text_close();

  // 0x48EAA7
  if (X_text_open(ScriptPopupFile, "BASEWARNINGS")) return;  // 0x686C80
  text_get();
  text_get();
  text_get();
  text_get();
  warnPrefCheckBox_.init(PrefWinGeometryValue_8578C8, PrefWinGeometryValue_8578C4, PrefWinGeometryValue_8578A0,
      PrefWinGeometryValue_8578A4 - PrefWinGeometryValue_8577E0 - PrefWinGeometryConstant_686B78 + 0x28, 0);
  warnPrefCheckBox_.set_sprites(&g_IFACE_CLOSE_X_SPRITES[1], &g_IFACE_CLOSE_X_SPRITES[2]);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C90
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x2B, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C94
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x2C, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C98
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x2D, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686C9C
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x2E, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CA0
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x2F, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CA4
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x30, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CA8
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x31, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CAC
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x32, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CB0
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x33, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CB4
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x34, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CB8
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x36, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CBC
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x37, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CC0
  strcat(StringTemp, TextBufferGetPtr);
  warnPrefCheckBox_.item(StringTemp, 0x38, 0);
  warnPrefCheckBox_.attach(&ConsolePrefWin, PrefWinGeometryValue_8578C8, PrefWinGeometryValue_8578C4, 0x1000120);
  // No text_close/X_text_open: the continuation reads on in BASEWARNINGS, and
  // takes no leading text_get() before its init (0x48EE85).
  warnContPrefCheckBox_.init(PrefWinGeometryValue_8578C8, PrefWinGeometryValue_8578C4, PrefWinGeometryValue_8578A0,
      PrefWinGeometryValue_8578A4 - PrefWinGeometryValue_8577E0 - PrefWinGeometryConstant_686B78, 0);
  warnContPrefCheckBox_.set_sprites(&g_IFACE_CLOSE_X_SPRITES[1], &g_IFACE_CLOSE_X_SPRITES[2]);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CC4
  strcat(StringTemp, TextBufferGetPtr);
  warnContPrefCheckBox_.item(StringTemp, 0x39, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CC8
  strcat(StringTemp, TextBufferGetPtr);
  warnContPrefCheckBox_.item(StringTemp, 0x3A, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CCC
  strcat(StringTemp, TextBufferGetPtr);
  warnContPrefCheckBox_.item(StringTemp, 0x3B, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CD0
  strcat(StringTemp, TextBufferGetPtr);
  warnContPrefCheckBox_.item(StringTemp, 0x3C, 0);
  warnContPrefCheckBox_.attach(&ConsolePrefWin, PrefWinGeometryValue_8578C8, PrefWinGeometryValue_8578C4, 0x1000120);
  text_close();

  // 0x48F036
  if (X_text_open(ScriptPopupFile, "MAPPREFS")) return;  // 0x686CD4
  text_get();
  text_get();
  text_get();
  text_get();
  mapPrefCheckBox_.init(PrefWinGeometryValue_8578C8, PrefWinGeometryValue_8578C4, PrefWinGeometryValue_8578A0, PrefWinGeometryValue_8578A4, 0);
  mapPrefCheckBox_.set_sprites(&g_IFACE_CLOSE_X_SPRITES[1], &g_IFACE_CLOSE_X_SPRITES[2]);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CE0
  strcat(StringTemp, TextBufferGetPtr);
  mapPrefCheckBox_.item(StringTemp, 0x48, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CE4
  strcat(StringTemp, TextBufferGetPtr);
  mapPrefCheckBox_.item(StringTemp, 0x49, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CE8
  strcat(StringTemp, TextBufferGetPtr);
  mapPrefCheckBox_.item(StringTemp, 0x4E, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CEC
  strcat(StringTemp, TextBufferGetPtr);
  mapPrefCheckBox_.item(StringTemp, 0x4A, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CF0
  strcat(StringTemp, TextBufferGetPtr);
  mapPrefCheckBox_.item(StringTemp, 0x4B, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CF4
  strcat(StringTemp, TextBufferGetPtr);
  mapPrefCheckBox_.item(StringTemp, 0x4C, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CF8
  strcat(StringTemp, TextBufferGetPtr);
  mapPrefCheckBox_.item(StringTemp, 0x4D, 0);
  StringTemp[0] = 0;
  text_get();
  strcat(StringTemp, " ");  // 0x686CFC
  strcat(StringTemp, TextBufferGetPtr);
  mapPrefCheckBox_.item(StringTemp, 0x4F, 0);
  mapPrefCheckBox_.attach(&ConsolePrefWin, PrefWinGeometryValue_8578C8, PrefWinGeometryValue_8578C4, 0x1000120);
  text_close();

  // 0x48F306: all four init_horz first, then all four set_range.
  // Parent pushed as immediate 0x8578D8 (= preference_window() / &ConsolePrefWin).
  volumeMainScroll_.init_horz(PrefWinGeometryValue_8578CC, PrefWinGeometryValue_8578D0, PrefWinGeometryValue_8578B8, preference_window(), 0x50);
  volumeVoiceScroll_.init_horz(PrefWinGeometryValue_8578CC, PrefWinGeometryValue_8578D0 + 0x16, PrefWinGeometryValue_8578B8, preference_window(), 0x51);
  volumeSFXScroll_.init_horz(PrefWinGeometryValue_8578CC, PrefWinGeometryValue_8578D0 + 0x2C, PrefWinGeometryValue_8578B8, preference_window(), 0x52);
  volumeMusicScroll_.init_horz(PrefWinGeometryValue_8578CC, PrefWinGeometryValue_8578D0 + 0x42, PrefWinGeometryValue_8578B8, preference_window(), 0x53);
  volumeMainScroll_.set_range(0, 0x7F);
  volumeVoiceScroll_.set_range(0, 0x7F);
  volumeSFXScroll_.set_range(0, 0x7F);
  volumeMusicScroll_.set_range(0, 0x7F);

  // 0x48F3DD: set_pos and its on_scrolling (virtual, vtable +0xC4) interleaved.
// RECOVERY_LITERAL: v1 0x00686D00 0x00686D0C $SG55978 0x0048D840
  volumeMainScroll_.set_pos(prefs_get("Main Volume", 0x7F, 0));    // 0x686D00 "Main Volume"
  on_scrolling(0x50, volumeMainScroll_.position());                     // [esi+0xB1B8]
// RECOVERY_LITERAL: v1 0x00686D0C 0x00686D19 $SG55979 0x0048D840
  volumeVoiceScroll_.set_pos(prefs_get("Voice Volume", 0x55, 0));  // 0x686D0C "Voice Volume"
  on_scrolling(0x51, volumeVoiceScroll_.position());
// RECOVERY_LITERAL: v1 0x00686D1C 0x00686D27 $SG55980 0x0048D840
  volumeSFXScroll_.set_pos(prefs_get("SFX Volume", 0x5F, 0));      // 0x686D1C "SFX Volume"
  on_scrolling(0x52, volumeSFXScroll_.position());
  if (FactionMusic) {  // 0x74DAA0; the shared `push 0` precedes the cmp
// RECOVERY_LITERAL: v1 0x00686D28 0x00686D35 $SG55982 0x0048D840
    volumeMusicScroll_.set_pos(prefs_get("Music Volume", 0x3F, 0));  // 0x686D28 "Music Volume"
    on_scrolling(0x53, volumeMusicScroll_.position());
  } else {
    volumeMusicScroll_.set_pos(0);
  }

  // 0x48F4AF
  volumeMainScroll_.hide();
  volumeVoiceScroll_.hide();
  volumeSFXScroll_.hide();
  volumeMusicScroll_.hide();
  // The basic page's box is SHOWN (GraphicWin-vbase slot 1 with 0, as in
  // display()'s page 0); the other six are hidden (slot 2).
  bscPrefCheckBox_.show(0);  // 0x48F4EB  // ? spelling: show(0) vs a GraphicWin:: qualified call
  advPrefCheckBox_.hide();
  autoPrefCheckBox_.hide();
  audioVisPrefCheckBox_.hide();
  warnPrefCheckBox_.hide();   // through edi
  warnContPrefCheckBox_.hide();
  mapPrefCheckBox_.hide();

  // 0x48F59E: label string pointer passed straight from StringTable.get (no
  // StringTemp copy, no null substitution).
  volumeMainToggle_.init(StringTable.get(Labels.strings_ptr[0x10E0 / 4]), 0x55, PrefWinGeometryValue_8578B4,
      PrefWinGeometryValue_8578BC - PrefWinNarrowDisplayVerticalOffset,
      PrefWinToggleWidth, PrefWinToggleHeight, preference_window(), 0);
  volumeVoiceToggle_.init(StringTable.get(Labels.strings_ptr[0x10E8 / 4]), 0x57, PrefWinGeometryValue_8578B4,
      PrefWinGeometryValue_8578BC - PrefWinNarrowDisplayVerticalOffset + 0x16,
      PrefWinToggleWidth, PrefWinToggleHeight, preference_window(), 0);
  volumeSFXToggle_.init(StringTable.get(Labels.strings_ptr[0x10F0 / 4]), 0x56, PrefWinGeometryValue_8578B4,
      PrefWinGeometryValue_8578BC - PrefWinNarrowDisplayVerticalOffset + 0x2C,
      PrefWinToggleWidth, PrefWinToggleHeight, preference_window(), 0);
  volumeMusicToggle_.init(StringTable.get(Labels.strings_ptr[0x10F8 / 4]), 0x58, PrefWinGeometryValue_8578B4,
      PrefWinGeometryValue_8578BC - PrefWinNarrowDisplayVerticalOffset + 0x42,
      PrefWinToggleWidth, PrefWinToggleHeight, preference_window(), 0);
  volumeMainToggle_.set_font_redraw(&TutWinBaseWindow.font1_);
  volumeVoiceToggle_.set_font_redraw(&TutWinBaseWindow.font1_);
  volumeSFXToggle_.set_font_redraw(&TutWinBaseWindow.font1_);
  volumeMusicToggle_.set_font_redraw(&TutWinBaseWindow.font1_);
  // Stores at 0x48F771: main, voice, sfx, music (+0x15820,+0x16EB8,+0x1636C,+0x17A04).
  volumeMainToggle_.field_A18_ = 0;
  volumeVoiceToggle_.field_A18_ = 0;
  volumeSFXToggle_.field_A18_ = 0;
  volumeMusicToggle_.field_A18_ = 0;
  volumeMainToggle_.hide();
  volumeVoiceToggle_.hide();
  volumeSFXToggle_.hide();
  volumeMusicToggle_.hide();

  // 0x48F7B9: restore takes the raw string; OK and Cancel go through StringTemp.
  restoreDefaultButton_.init(StringTable.get(Labels.strings_ptr[0x10DC / 4]), 0x5A, PrefWinGeometryValue_85788C,
      PrefWinGeometryValue_857884 - PrefWinNarrowDisplayVerticalOffset + PrefWinGeometryValue_8578C0 - 0x45,  // ? 0x48F7E6 lea [eax+ecx-0x45]
      PrefWinGeometryValue_857894, PrefWinTitleButtonHeight, preference_window(), 0);
  StringTemp[0] = 0;
  strcat(StringTemp, label_get(0x194 / 4));
  okButton_.init(StringTemp, 0x5C, PrefWinGeometryValue_85788C,
      PrefWinGeometryValue_857884 - PrefWinNarrowDisplayVerticalOffset + PrefWinGeometryValue_8578C0 - 0x2E,
      PrefWinGeometryValue_857894, PrefWinTitleButtonHeight, preference_window(), 0);
  StringTemp[0] = 0;
  strcat(StringTemp, label_get(0x198 / 4));
  cancelButton_.init(StringTemp, 0x5B, PrefWinGeometryValue_85788C,
      PrefWinGeometryValue_857884 - PrefWinNarrowDisplayVerticalOffset + PrefWinGeometryValue_8578C0 - 0x17,
      PrefWinGeometryValue_857894, PrefWinTitleButtonHeight, preference_window(), 0);
  restoreDefaultButton_.set_font_redraw(&TutWinBaseWindow.font1_);
  okButton_.set_font_redraw(&TutWinBaseWindow.font1_);
  cancelButton_.set_font_redraw(&TutWinBaseWindow.font1_);
  // Stores at 0x48F956: restore, ok, cancel (+0x1909C,+0x1A734,+0x19BE8).
  restoreDefaultButton_.field_A18_ = 0;
  okButton_.field_A18_ = 0;
  cancelButton_.field_A18_ = 0;
  restoreDefaultButton_.hide();
  okButton_.hide();
  cancelButton_.hide();

  // 0x48F989
  moreButton_.init(StringTable.get(Labels.strings_ptr[0x1100 / 4]), 0x5E, PrefWinGeometryValue_8578C8,
      PrefWinGeometryValue_857898 + PrefWinGeometryValue_8578A4 - 0x17,  // ? 0x48F9AE lea [eax+ecx-0x17], eax=857898, ecx=8578A4
      PrefWinGeometryValue_857894, PrefWinTitleButtonHeight, preference_window(), 0);
  moreButton_.field_A18_ = 0;  // +0x1B280
  moreButton_.hide();
  hide();  // 0x48F9E7: VIRTUAL dispatch through this's vtable slot 2, not Win::hide()
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048FA00 ?display@PrefWin@@QAEXH@Z 0x0048FA00-0x0048FCC5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0048fa00.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::display(int page) {
  ConsoleGlobal.alpha_menu_.hide();
  prefsFinal_ = 0;
  morePrefsFinal_ = 0;
  warningsFinal_ = 0;
  prefsBasicPending_ = GameDataStateStorage.fields_.game_preferences_;
  morePrefsBasicPending_ = GameDataStateStorage.fields_.game_more_preferences_;
  prefsAdvPending_ = GameDataStateStorage.fields_.game_preferences_;
  morePrefsAdvPending_ = GameDataStateStorage.fields_.game_more_preferences_;
  prefsAutoPending_ = GameDataStateStorage.fields_.game_preferences_;
  morePrefsAutoPending_ = GameDataStateStorage.fields_.game_more_preferences_;
  prefsAVPending_ = GameDataStateStorage.fields_.game_preferences_;
  morePrefsAVPending_ = GameDataStateStorage.fields_.game_more_preferences_;
  warningsPending_ = GameDataStateStorage.fields_.game_warnings_;
  prefsMapPending_ = GameDataStateStorage.fields_.game_preferences_;
  morePrefsMapPending_ = GameDataStateStorage.fields_.game_more_preferences_;
  RECT outer;
  set_rectangle_dimensions(&outer, 0x17, 0, PrefWinDisplayWidth, PrefWinDisplayHeight);
  RECT inner;
  set_rectangle_dimensions(&inner, 0x1B, 4, PrefWinDisplayWidth - 8, PrefWinDisplayHeight - 8);
  RECT buttons;
  set_rectangle_dimensions(&buttons, PrefWinGeometryValue_857888, PrefWinGeometryValue_857884,
      PrefWinButtonWidth, PrefWinGeometryValue_8578C0);
  Buffer::box_sprite(&outer, &InterfaceBoxStyles[9]);
  Buffer::box_sprite(&inner, &InterfaceBoxStyles[9]);
  Buffer::box_sprite(&buttons, &InterfaceBoxStyles[18]);
  get_preferences();
  volumeMainToggle_.field_A18_ = 0;
  if (volumeMusicToggle_.field_A18_) volumeMainToggle_.field_A18_ = 1;
  if (volumeVoiceToggle_.field_A18_) volumeMainToggle_.field_A18_ = 1;
  if (volumeSFXToggle_.field_A18_) volumeMainToggle_.field_A18_ = 1;
  change_volume_button_text(0x55);
  redraw_option();
  redraw_volume();
  restoreDefaultButton_.show(0);
  okButton_.show(0);
  cancelButton_.show(0);
  if (page >= 0) {
    field_A18_ = field_A14_;
    field_A14_ = page;
    PrefWinButtonGroup.button_click(page);
  }
  redraw();
  slide_show(this, 0);
  show(3);
  Win::set_def_focus(preference_window());
  set_modal(0, pop_wait, nullptr);
  if (~GameDataStateStorage.fields_.game_preferences_ & 0x20000000)
    DialogDefaults[36] |= 4u;
  else
    DialogDefaults[36] &= ~4u;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048FDD0 ?destroy@PrefWin@@QAEXXZ 0x0048FDD0-0x0048FF51 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0048fdd0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::destroy() {
  if (field_A5C_) {
    set_preferences();
    warningsFinal_ |= warningsPending_;
    prefsFinal_ |= prefsBasicPending_ | prefsAdvPending_ | prefsAutoPending_ |
                  prefsAVPending_ | prefsMapPending_;
    morePrefsFinal_ |= morePrefsBasicPending_ | morePrefsAdvPending_ |
                      morePrefsAutoPending_ | morePrefsAVPending_ | morePrefsMapPending_;
    AlphaIniPrefs.preferences = prefsFinal_;
    AlphaIniPrefs.more_preferences = morePrefsFinal_;
    AlphaIniPrefs.announce = warningsFinal_;
    prefs_use();
    prefs_save(0);
// RECOVERY_LITERAL: v1 0x00686D38 0x00686D44 $SG56001 0x0048FDD0
    prefs_put("Main Volume", volumeMainScroll_.position(), 0);
// RECOVERY_LITERAL: v1 0x00686D44 0x00686D51 $SG56002 0x0048FDD0
    prefs_put("Voice Volume", volumeVoiceScroll_.position(), 0);
// RECOVERY_LITERAL: v1 0x00686D54 0x00686D61 $SG56003 0x0048FDD0
    prefs_put("Music Volume", volumeMusicScroll_.position(), 0);
// RECOVERY_LITERAL: v1 0x00686D64 0x00686D6F $SG56004 0x0048FDD0
    prefs_put("SFX Volume", volumeSFXScroll_.position(), 0);
    field_A5C_ = 0;
  }
  draw_map(1);
  WorldClimateWorldWin.field_239BC_ |= 4;
  okButton_.set_pressed_visual(0);
  cancelButton_.set_pressed_visual(0);
  slide_hide(this, 0);
  hide();
  Win::set_def_focus(console_window());
  release_modal();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0048FCD0 0x0048FD98 0x0048FDAC 0x0048FCEA
// RECOVERY_ASSOCIATED: v1 switch_selector 0x0048FCD0 0x0048FDAC 0x0048FDC8 0x0048FCE4
// ORIGINAL: 0x0048FCD0 ?on_key_click@PrefWin@@QAEHHH@Z 0x0048FCD0-0x0048FD98 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0048fcd0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// symbol    ?on_key_click@PrefWin@@UAEHHH@Z
int PrefWin::on_key_click(int repeat, int key) {
  (void)repeat;
  switch (key) {
  case VK_RETURN:
    field_A5C_ = 1;
    // Enter confirms the preferences before taking the Escape close path.
  case VK_ESCAPE:
    destroy();
    return 1;
  case VK_UP:
    field_A18_ = field_A14_;
    if (field_A14_ <= 0)
      field_A14_ = 5;
    else
      --field_A14_;
    field_A1C_ = 1;
    PrefWinButtonGroup.button_click(field_A14_);
    return 1;
  case VK_DOWN:
    field_A18_ = field_A14_;
    if (field_A14_ >= 5)
      field_A14_ = 0;
    else
      ++field_A14_;
    field_A1C_ = 1;
    PrefWinButtonGroup.button_click(field_A14_);
    break;
  }
  return 1;
}

// ORIGINAL: 0x0048FF60 ?on_group_clicked@PrefWin@@QAEXHHH@Z 0x0048FF60-0x0048FFA1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/0048ff60.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// symbol    ?on_group_clicked@PrefWin@@UAEXHHH@Z
// kind      game
void PrefWin::on_group_clicked(int group, int previous, int selected) {
  (void)group;
  if (!field_A1C_) {
    if (previous <= 5 && previous >= 0) field_A18_ = previous;
  } else {
    field_A1C_ = 0;
  }
  field_A14_ = selected;
  on_button_clicked(selected);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048FFB0 ?hide_option_buttons@PrefWin@@QAEXXZ 0x0048FFB0-0x00490123 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0048ffb0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0048FFB0 0x00490124 0x00490140 0x0048FFC2
void PrefWin::hide_option_buttons() {
  switch (field_A18_) {
    case 0: bscPrefCheckBox_.hide(); break;
    case 1: advPrefCheckBox_.hide(); break;
    case 3: autoPrefCheckBox_.hide(); break;
    case 4:
      audioVisPrefCheckBox_.hide();
      volumeMainScroll_.hide();
      volumeVoiceScroll_.hide();
      volumeSFXScroll_.hide();
      volumeMusicScroll_.hide();
      volumeMainToggle_.hide();
      volumeVoiceToggle_.hide();
      volumeSFXToggle_.hide();
      volumeMusicToggle_.hide();
      break;
    case 2:
      warnPrefCheckBox_.hide();
      moreButton_.hide();
      break;
    case 6:
      warnContPrefCheckBox_.hide();
      moreButton_.hide();
      break;
    case 5: mapPrefCheckBox_.hide(); break;
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00490140 0x00490328 0x00490344 0x00490152
// ORIGINAL: 0x00490140 ?update_pref_buttons@PrefWin@@QAEXXZ 0x00490140-0x00490326 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00490140.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::update_pref_buttons() {
  switch (field_A14_) {
    case 0: bscPrefCheckBox_.show(0); break;
    case 1: advPrefCheckBox_.show(0); break;
    case 2: {
      warnPrefCheckBox_.show(0);
      FlatButton &more_button = moreButton_;
      more_button.set_name(StringTable.get(Labels.strings_ptr[label_index(0x1100)]));
      more_button.show(0);
      break;
    }
    case 6: {
      warnContPrefCheckBox_.show(0);
      StringTemp[0] = '\0';
      LPSTR label_value = Labels.strings_ptr[label_index(0xA60)];
      strcat(StringTemp, StringTable.get(label_value));
      moreButton_.set_name(StringTemp);
      moreButton_.show(0);
      break;
    }
    case 3: autoPrefCheckBox_.show(0); break;
    case 4:
      audioVisPrefCheckBox_.show(0);
      volumeMainScroll_.show(0);
      volumeVoiceScroll_.show(0);
      volumeSFXScroll_.show(0);
      volumeMusicScroll_.show(0);
      volumeMainToggle_.show(0);
      volumeVoiceToggle_.show(0);
      volumeSFXToggle_.show(0);
      volumeMusicToggle_.show(0);
      break;
    case 5: mapPrefCheckBox_.show(0); break;
  }
}

// ORIGINAL: 0x00490350 ?set_preferences@PrefWin@@QAEXXZ 0x00490350-0x00490AFA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00490350.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// TRIED: declared each CheckBox* group pointer lazily, right before its first get_state_id() call (matching the original's `lea edi, [esi+OFFSET]` sitting immediately before that group's first push), instead of all six upfront. Moved divergence #1(sim0.98)->#63 on the frameless build (original prologue is `push esi; mov esi,ecx; push edi` with no `push ebp`, so that is the row that matters, not the framed one the ranking line shows). Not chased further given size (1962 bytes / ~71 mechanical get_state_id calls).
void PrefWin::set_preferences() {
  prefsBasicPending_ = 0;
  morePrefsBasicPending_ = 0;
  prefsAdvPending_ = 0;
  morePrefsAdvPending_ = 0;
  prefsAutoPending_ = 0;
  morePrefsAutoPending_ = 0;
  prefsAVPending_ = 0;
  morePrefsAVPending_ = 0;
  warningsPending_ = 0;
  prefsMapPending_ = 0;
  morePrefsMapPending_ = 0;
  if (bscPrefCheckBox_.get_state_id(7)) prefsBasicPending_ |= PREF_BSC_TUTORIAL_MSGS;
  if (bscPrefCheckBox_.get_state_id(8)) prefsBasicPending_ |= PREF_BSC_AUTO_DESIGN_VEH;
  if (bscPrefCheckBox_.get_state_id(9)) morePrefsBasicPending_ |= MPREF_BSC_AUTO_PRUNE_OBS_VEH;
  if (bscPrefCheckBox_.get_state_id(10)) prefsBasicPending_ |= PREF_BSC_MOUSE_EDGE_SCROLL_VIEW;
  if (bscPrefCheckBox_.get_state_id(11)) prefsBasicPending_ |= PREF_BSC_AUTOSAVE_EACH_TURN;
  if (bscPrefCheckBox_.get_state_id(12)) prefsBasicPending_ |= PREF_BSC_PAUSE_END_TURN;
  if (!bscPrefCheckBox_.get_state_id(13)) prefsBasicPending_ |= PREF_BSC_DONT_QUICK_MOVE_ENEMY_VEH;
  if (!bscPrefCheckBox_.get_state_id(14)) prefsBasicPending_ |= PREF_BSC_DONT_QUICK_MOVE_ALLY_VEH;
  if (!advPrefCheckBox_.get_state_id(15)) prefsAdvPending_ |= PREF_ADV_RADIO_BTN_NOT_SEL_SING_CLK;
  if (advPrefCheckBox_.get_state_id(0x10)) morePrefsAdvPending_ |= MPREF_ADV_CLICK_VEH_CANCELS_ORDERS;
  if (advPrefCheckBox_.get_state_id(0x11)) morePrefsAdvPending_ |= MPREF_ADV_CONFIRM_ODDS_BF_ATTACKING;
  if (advPrefCheckBox_.get_state_id(0x12)) morePrefsAdvPending_ |= MPREF_ADV_PAUSE_AFTER_BATTLES;
  if (advPrefCheckBox_.get_state_id(0x13)) prefsAdvPending_ |= PREF_ADV_FAST_BATTLE_RESOLUTION;
  if (advPrefCheckBox_.get_state_id(0x1C)) morePrefsAdvPending_ |= MPREF_ADV_QUICK_MOVE_VEH_ORDERS;
  if (advPrefCheckBox_.get_state_id(0x14)) morePrefsAdvPending_ |= MPREF_ADV_QUICK_MOVE_ALL_VEH;
  if (advPrefCheckBox_.get_state_id(0x15)) prefsAdvPending_ |= PREF_ADV_NO_CENTER_VEH_ORDERS;
  if (advPrefCheckBox_.get_state_id(0x17)) morePrefsAdvPending_ |= MPREF_ADV_ZOOM_BASE_NO_RECENTER_MAP;
  if (advPrefCheckBox_.get_state_id(0x18)) morePrefsAdvPending_ |= MPREF_ADV_DETAIL_MAIN_MENUS;
  if (advPrefCheckBox_.get_state_id(0x19)) morePrefsAdvPending_ |= MPREF_ADV_DETAIL_RIGHT_CLICK_MENUS;
  if (advPrefCheckBox_.get_state_id(0x1A)) morePrefsAdvPending_ |= MPREF_ADV_RIGHT_CLICK_POPS_UP_MENU;
  if (autoPrefCheckBox_.get_state_id(0x1D)) prefsAutoPending_ |= PREF_AUTO_AIR_VEH_RET_HOME_FUEL_RNG;
  if (autoPrefCheckBox_.get_state_id(0x1E)) prefsAutoPending_ |= PREF_AUTO_FORMER_PLANT_FORESTS;
  if (autoPrefCheckBox_.get_state_id(0x1F)) prefsAutoPending_ |= PREF_AUTO_FORMER_RAISE_LWR_TERRAIN;
  if (autoPrefCheckBox_.get_state_id(0x20)) prefsAutoPending_ |= PREF_AUTO_FORMER_BUILD_ADV;
  if (!autoPrefCheckBox_.get_state_id(0x2A)) morePrefsAutoPending_ |= MPREF_AUTO_FORMER_CANT_BUILD_ROADS;
  if (autoPrefCheckBox_.get_state_id(0x21)) morePrefsAutoPending_ |= MPREF_AUTO_FORMER_REMOVE_FUNGUS;
  if (autoPrefCheckBox_.get_state_id(0x22)) morePrefsAutoPending_ |= MPREF_AUTO_FORMER_BUILD_SENSORS;
  if (autoPrefCheckBox_.get_state_id(0x23)) prefsAutoPending_ |= PREF_AUTO_END_MOVE_SPOT_VEH_PACT;
  if (autoPrefCheckBox_.get_state_id(0x24)) prefsAutoPending_ |= PREF_AUTO_END_MOVE_SPOT_VEH_TREATY;
  if (autoPrefCheckBox_.get_state_id(0x25)) prefsAutoPending_ |= PREF_AUTO_END_MOVE_SPOT_VEH_TRUCE;
  if (autoPrefCheckBox_.get_state_id(0x26)) prefsAutoPending_ |= PREF_AUTO_END_MOVE_SPOT_VEH_WAR;
  if (autoPrefCheckBox_.get_state_id(0x27)) prefsAutoPending_ |= PREF_AUTO_DONT_END_MOVE_DIFF_TRIAD;
  if (autoPrefCheckBox_.get_state_id(0x28)) prefsAutoPending_ |= PREF_AUTO_WAKE_VEH_TRANS_REACH_LAND;
  if (autoPrefCheckBox_.get_state_id(0x29)) morePrefsAutoPending_ |= MPREF_AUTO_ALWAYS_INSPECT_MONOLITH;
  if (audioVisPrefCheckBox_.get_state_id(0x3D)) prefsAVPending_ |= PREF_AV_VOLUME_MUSIC_TOGGLE;
  if (audioVisPrefCheckBox_.get_state_id(0x3E)) prefsAVPending_ |= PREF_AV_VOLUME_SFX_TOGGLE;
  if (audioVisPrefCheckBox_.get_state_id(0x3F)) morePrefsAVPending_ |= MPREF_AV_VOLUME_VOICE_TOGGLE;
  if (audioVisPrefCheckBox_.get_state_id(0x40)) morePrefsAVPending_ |= MPREF_AV_VOICEOVER_STOP_CLOSE_POPUP;
  if (audioVisPrefCheckBox_.get_state_id(0x41)) prefsAVPending_ |= PREF_AV_SECRET_PROJECT_MOVIES;
  if (!audioVisPrefCheckBox_.get_state_id(0x42)) prefsAVPending_ |= PREF_AV_INTERLUDES_DISABLED;
  if (!audioVisPrefCheckBox_.get_state_id(0x47)) morePrefsAVPending_ |= MPREF_AV_MONUMENTS_DISABLED;
  if (audioVisPrefCheckBox_.get_state_id(0x43)) prefsAVPending_ |= PREF_AV_MAP_ANIMATIONS;
  if (audioVisPrefCheckBox_.get_state_id(0x44)) prefsAVPending_ |= PREF_AV_SLIDING_WINDOWS;
  if (audioVisPrefCheckBox_.get_state_id(0x45)) morePrefsAVPending_ |= MPREF_AV_SLIDING_SCROLLBARS;
  if (audioVisPrefCheckBox_.get_state_id(0x16)) morePrefsAVPending_ |= MPREF_AV_WHOLE_VEH_BLINKS;
  if (volumeVoiceToggle_.field_A18_) morePrefsAVPending_ |= MPREF_AV_VOLUME_VOICE_TOGGLE;
  if (volumeSFXToggle_.field_A18_) prefsAVPending_ |= PREF_AV_VOLUME_SFX_TOGGLE;
  if (volumeMusicToggle_.field_A18_) prefsAVPending_ |= PREF_AV_VOLUME_MUSIC_TOGGLE;
  if (warnPrefCheckBox_.get_state_id(0x2B)) warningsPending_ |= WARN_STOP_NEW_FAC_BUILT;
  if (warnPrefCheckBox_.get_state_id(0x2C)) warningsPending_ |= WARN_STOP_NON_COMBAT_VEH_BUILT;
  if (warnPrefCheckBox_.get_state_id(0x2D)) warningsPending_ |= WARN_STOP_COMBAT_VEH_BUILT;
  if (warnPrefCheckBox_.get_state_id(0x2E)) warningsPending_ |= WARN_STOP_PROTOTYPE_COMPLETE;
  if (warnPrefCheckBox_.get_state_id(0x2F)) warningsPending_ |= WARN_STOP_BUILT_VIA_GOV_QUEUE;
  if (warnPrefCheckBox_.get_state_id(0x30)) warningsPending_ |= WARN_STOP_DRONE_RIOTS;
  if (warnPrefCheckBox_.get_state_id(0x31)) warningsPending_ |= WARN_STOP_DRONE_RIOTS_END;
  if (warnPrefCheckBox_.get_state_id(0x32)) warningsPending_ |= WARN_STOP_GOLDEN_AGE;
  if (warnPrefCheckBox_.get_state_id(0x33)) warningsPending_ |= WARN_STOP_GOLDEN_AGE_END;
  if (warnPrefCheckBox_.get_state_id(0x34)) warningsPending_ |= WARN_STOP_NUTRIENT_SHORTAGE;
  if (warnPrefCheckBox_.get_state_id(0x35)) warningsPending_ |= WARN_STOP_UNK_100;
  if (warnPrefCheckBox_.get_state_id(0x36)) warningsPending_ |= WARN_STOP_BUILD_OUT_OF_DATE;
  if (warnPrefCheckBox_.get_state_id(0x37)) warningsPending_ |= WARN_STOP_POP_LIMIT_REACHED;
  if (warnPrefCheckBox_.get_state_id(0x38)) warningsPending_ |= WARN_STOP_DELAY_IN_TRANSCEND;
  if (warnContPrefCheckBox_.get_state_id(0x39)) warningsPending_ |= WARN_STOP_STARVATION;
  if (warnContPrefCheckBox_.get_state_id(0x3A)) warningsPending_ |= WARN_STOP_MINERAL_SHORTAGE;
  if (warnContPrefCheckBox_.get_state_id(0x3B)) warningsPending_ |= WARN_STOP_ENERGY_SHORTAGE;
  if (warnContPrefCheckBox_.get_state_id(0x3C)) warningsPending_ |= WARN_STOP_RANDOM_EVENT;
  if (mapPrefCheckBox_.get_state_id(0x48)) morePrefsMapPending_ |= MPREF_MAP_SHOW_FOG_WAR;
  if (mapPrefCheckBox_.get_state_id(0x49)) prefsMapPending_ |= PREF_MAP_SHOW_GRID;
  if (mapPrefCheckBox_.get_state_id(0x4A)) prefsMapPending_ |= PREF_MAP_SHOW_BASE_GRID;
  if (mapPrefCheckBox_.get_state_id(0x4B)) morePrefsMapPending_ |= MPREF_MAP_SHOW_BASE_NAMES;
  if (mapPrefCheckBox_.get_state_id(0x4C)) morePrefsMapPending_ |= MPREF_MAP_SHOW_PROD_WITH_BASE_NAMES;
  if (mapPrefCheckBox_.get_state_id(0x4D)) morePrefsMapPending_ |= MPREF_MAP_SHOW_FLAT_TERRAIN;
  if (mapPrefCheckBox_.get_state_id(0x4E)) morePrefsMapPending_ |= MPREF_MAP_SHOW_GRID_OCEAN_SQ;
  if (!mapPrefCheckBox_.get_state_id(0x4F)) morePrefsMapPending_ |= MPREF_MAP_HIDE_ACTIVE_VEH_GOTO_PATH;
}

// ORIGINAL: 0x00490B00 ?get_preferences@PrefWin@@QAEXXZ 0x00490B00-0x004911FD FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00490b00.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// TRIED (2026-11-13): with its nine in-text tables declared the body matches
// the image block for block except `test bit, flags` operand order and three
// AV-loop `mov eax, [morePrefsAVPending_]` loads; operand swap, int/uint
// types, a local `more`, `!= 0` forms and a helper inline all compile alike.
void PrefWin::get_preferences() {
  int id = 0;
  uint32_t bit = 0;
  int i;
  for (i = 0; i < 8; i++) {
    uint32_t prefs = prefsBasicPending_;
    uint32_t more = morePrefsBasicPending_;
    switch (i) {
      case 0: id = 7; bit = PREF_BSC_TUTORIAL_MSGS; break;
      case 1: id = 8; bit = PREF_BSC_AUTO_DESIGN_VEH; break;
      case 2: id = 10; bit = PREF_BSC_MOUSE_EDGE_SCROLL_VIEW; break;
      case 3: id = 11; bit = PREF_BSC_AUTOSAVE_EACH_TURN; break;
      case 4: id = 12; bit = PREF_BSC_PAUSE_END_TURN; break;
      case 5: id = 13; bit = PREF_BSC_DONT_QUICK_MOVE_ENEMY_VEH; break;
      case 6: id = 14; bit = PREF_BSC_DONT_QUICK_MOVE_ALLY_VEH; break;
      case 7: id = 9; bit = MPREF_BSC_AUTO_PRUNE_OBS_VEH; break;
    }
    switch (id) {
      case 9:
        if (more & bit) {
          bscPrefCheckBox_.set_state_id(id, 1);
        } else {
          bscPrefCheckBox_.set_state_id(id, 0);
        }
        break;
      case 13:
      case 14:
        if (prefs & bit) {
          bscPrefCheckBox_.set_state_id(id, 0);
        } else {
          bscPrefCheckBox_.set_state_id(id, 1);
        }
        break;
      default:
        if (prefs & bit) {
          bscPrefCheckBox_.set_state_id(id, 1);
        } else {
          bscPrefCheckBox_.set_state_id(id, 0);
        }
        break;
    }
  }
  for (i = 0; i < 12; i++) {
    uint32_t prefs = prefsAdvPending_;
    uint32_t more = morePrefsAdvPending_;
    switch (i) {
      case 0: id = 15; bit = PREF_ADV_RADIO_BTN_NOT_SEL_SING_CLK; break;
      case 1: id = 0x10; bit = MPREF_ADV_CLICK_VEH_CANCELS_ORDERS; break;
      case 2: id = 0x11; bit = MPREF_ADV_CONFIRM_ODDS_BF_ATTACKING; break;
      case 3: id = 0x12; bit = MPREF_ADV_PAUSE_AFTER_BATTLES; break;
      case 4: id = 0x13; bit = PREF_ADV_FAST_BATTLE_RESOLUTION; break;
      case 5: id = 0x1C; bit = MPREF_ADV_QUICK_MOVE_VEH_ORDERS; break;
      case 6: id = 0x14; bit = MPREF_ADV_QUICK_MOVE_ALL_VEH; break;
      case 7: id = 0x15; bit = PREF_ADV_NO_CENTER_VEH_ORDERS; break;
      case 8: id = 0x18; bit = MPREF_ADV_DETAIL_MAIN_MENUS; break;
      case 9: id = 0x17; bit = MPREF_ADV_ZOOM_BASE_NO_RECENTER_MAP; break;
      case 10: id = 0x19; bit = MPREF_ADV_DETAIL_RIGHT_CLICK_MENUS; break;
      case 11: id = 0x1A; bit = MPREF_ADV_RIGHT_CLICK_POPS_UP_MENU; break;
    }
    switch (id) {
      default:
        if (more & bit) {
          advPrefCheckBox_.set_state_id(id, 1);
        } else {
          advPrefCheckBox_.set_state_id(id, 0);
        }
        break;
      case 15:
      case 0x13:
      case 0x15:
        if (prefs & bit) {
          advPrefCheckBox_.set_state_id(id, 1);
        } else {
          advPrefCheckBox_.set_state_id(id, 0);
        }
        if (id == 15) {
          advPrefCheckBox_.flip_state_id(id);
        }
        break;
    }
  }
  for (i = 0; i < 14; i++) {
    uint32_t prefs = prefsAutoPending_;
    uint32_t more = morePrefsAutoPending_;
    switch (i) {
      case 0: id = 0x1D; bit = PREF_AUTO_AIR_VEH_RET_HOME_FUEL_RNG; break;
      case 1: id = 0x1E; bit = PREF_AUTO_FORMER_PLANT_FORESTS; break;
      case 2: id = 0x1F; bit = PREF_AUTO_FORMER_RAISE_LWR_TERRAIN; break;
      case 3: id = 0x20; bit = PREF_AUTO_FORMER_BUILD_ADV; break;
      case 4: id = 0x21; bit = MPREF_AUTO_FORMER_REMOVE_FUNGUS; break;
      case 5: id = 0x22; bit = MPREF_AUTO_FORMER_BUILD_SENSORS; break;
      case 6: id = 0x23; bit = PREF_AUTO_END_MOVE_SPOT_VEH_PACT; break;
      case 7: id = 0x24; bit = PREF_AUTO_END_MOVE_SPOT_VEH_TREATY; break;
      case 8: id = 0x25; bit = PREF_AUTO_END_MOVE_SPOT_VEH_TRUCE; break;
      case 9: id = 0x26; bit = PREF_AUTO_END_MOVE_SPOT_VEH_WAR; break;
      case 10: id = 0x27; bit = PREF_AUTO_DONT_END_MOVE_DIFF_TRIAD; break;
      case 11: id = 0x28; bit = PREF_AUTO_WAKE_VEH_TRANS_REACH_LAND; break;
      case 12: id = 0x29; bit = MPREF_AUTO_ALWAYS_INSPECT_MONOLITH; break;
      case 13: id = 0x2A; bit = MPREF_AUTO_FORMER_CANT_BUILD_ROADS; break;
    }
    switch (id) {
      case 0x21:
      case 0x22:
      case 0x29:
        if (more & bit) {
          autoPrefCheckBox_.set_state_id(id, 1);
        } else {
          autoPrefCheckBox_.set_state_id(id, 0);
        }
        break;
      case 0x2A:
        if (more & bit) {
          autoPrefCheckBox_.set_state_id(id, 0);
        } else {
          autoPrefCheckBox_.set_state_id(id, 1);
        }
        break;
      default:
        if (prefs & bit) {
          autoPrefCheckBox_.set_state_id(id, 1);
        } else {
          autoPrefCheckBox_.set_state_id(id, 0);
        }
        break;
    }
  }
  for (i = 0; i < 11; i++) {
    uint32_t prefs = prefsAVPending_;
    switch (i) {
      case 0: id = 0x3D; bit = PREF_AV_VOLUME_MUSIC_TOGGLE; break;
      case 1: id = 0x3E; bit = PREF_AV_VOLUME_SFX_TOGGLE; break;
      case 2: id = 0x3F; bit = MPREF_AV_VOLUME_VOICE_TOGGLE; break;
      case 3: id = 0x40; bit = MPREF_AV_VOICEOVER_STOP_CLOSE_POPUP; break;
      case 4: id = 0x41; bit = PREF_AV_SECRET_PROJECT_MOVIES; break;
      case 5: id = 0x42; bit = PREF_AV_INTERLUDES_DISABLED; break;
      case 6: id = 0x43; bit = PREF_AV_MAP_ANIMATIONS; break;
      case 7: id = 0x44; bit = PREF_AV_SLIDING_WINDOWS; break;
      case 8: id = 0x45; bit = MPREF_AV_SLIDING_SCROLLBARS; break;
      case 9: id = 0x16; bit = MPREF_AV_WHOLE_VEH_BLINKS; break;
      case 10: id = 0x47; bit = MPREF_AV_MONUMENTS_DISABLED; break;
    }
    switch (id) {
      case 0x3F:
        if (morePrefsAVPending_ & bit) {
          volumeVoiceToggle_.field_A18_ = 1;
        } else {
          volumeVoiceToggle_.field_A18_ = 0;
        }
        change_volume_button_text(0x57);
        break;
      case 0x3E:
        if (prefs & bit) {
          volumeSFXToggle_.field_A18_ = 1;
        } else {
          volumeSFXToggle_.field_A18_ = 0;
        }
        change_volume_button_text(0x56);
        break;
      case 0x3D:
        if (prefs & bit) {
          volumeMusicToggle_.field_A18_ = 1;
        } else {
          volumeMusicToggle_.field_A18_ = 0;
        }
        change_volume_button_text(0x58);
        break;
      case 0x42:
        if (prefs & bit) {
          audioVisPrefCheckBox_.set_state_id(id, 0);
        } else {
          audioVisPrefCheckBox_.set_state_id(id, 1);
        }
        break;
      case 0x47:
        if (morePrefsAVPending_ & bit) {
          audioVisPrefCheckBox_.set_state_id(id, 0);
        } else {
          audioVisPrefCheckBox_.set_state_id(id, 1);
        }
        break;
      case 0x16:
      case 0x40:
      case 0x45:
        if (morePrefsAVPending_ & bit) {
          audioVisPrefCheckBox_.set_state_id(id, 1);
        } else {
          audioVisPrefCheckBox_.set_state_id(id, 0);
        }
        break;
      default:
        if (prefs & bit) {
          audioVisPrefCheckBox_.set_state_id(id, 1);
        } else {
          audioVisPrefCheckBox_.set_state_id(id, 0);
        }
        break;
    }
  }
  for (i = 0; i < 13; i++) {
    uint32_t prefs = warningsPending_;
    switch (i) {
      case 0: id = 0x2B; bit = WARN_STOP_NEW_FAC_BUILT; break;
      case 1: id = 0x2C; bit = WARN_STOP_NON_COMBAT_VEH_BUILT; break;
      case 2: id = 0x2D; bit = WARN_STOP_COMBAT_VEH_BUILT; break;
      case 3: id = 0x2E; bit = WARN_STOP_PROTOTYPE_COMPLETE; break;
      case 4: id = 0x2F; bit = WARN_STOP_BUILT_VIA_GOV_QUEUE; break;
      case 5: id = 0x30; bit = WARN_STOP_DRONE_RIOTS; break;
      case 6: id = 0x31; bit = WARN_STOP_DRONE_RIOTS_END; break;
      case 7: id = 0x32; bit = WARN_STOP_GOLDEN_AGE; break;
      case 8: id = 0x33; bit = WARN_STOP_GOLDEN_AGE_END; break;
      case 9: id = 0x34; bit = WARN_STOP_NUTRIENT_SHORTAGE; break;
      case 10: id = 0x36; bit = WARN_STOP_BUILD_OUT_OF_DATE; break;
      case 11: id = 0x37; bit = WARN_STOP_POP_LIMIT_REACHED; break;
      case 12: id = 0x38; bit = WARN_STOP_DELAY_IN_TRANSCEND; break;
    }
    if (prefs & bit) {
      warnPrefCheckBox_.set_state_id(id, 1);
    } else {
      warnPrefCheckBox_.set_state_id(id, 0);
    }
  }
  for (i = 0; i < 4; i++) {
    // The pinned table pairs index 0 with 0x3A and index 3 with 0x39.
    switch (i) {
      case 0: id = 0x3A; bit = WARN_STOP_MINERAL_SHORTAGE; break;
      case 1: id = 0x3B; bit = WARN_STOP_ENERGY_SHORTAGE; break;
      case 2: id = 0x3C; bit = WARN_STOP_RANDOM_EVENT; break;
      case 3: id = 0x39; bit = WARN_STOP_STARVATION; break;
    }
    if (warningsPending_ & bit) {
      warnContPrefCheckBox_.set_state_id(id, 1);
    } else {
      warnContPrefCheckBox_.set_state_id(id, 0);
    }
  }
  for (i = 0; i < 8; i++) {
    uint32_t prefs = prefsMapPending_;
    uint32_t more = morePrefsMapPending_;
    switch (i) {
      case 0: id = 0x48; bit = MPREF_MAP_SHOW_FOG_WAR; break;
      case 1: id = 0x49; bit = PREF_MAP_SHOW_GRID; break;
      case 2: id = 0x4A; bit = PREF_MAP_SHOW_BASE_GRID; break;
      case 3: id = 0x4B; bit = MPREF_MAP_SHOW_BASE_NAMES; break;
      case 4: id = 0x4C; bit = MPREF_MAP_SHOW_PROD_WITH_BASE_NAMES; break;
      case 5: id = 0x4D; bit = MPREF_MAP_SHOW_FLAT_TERRAIN; break;
      case 6: id = 0x4E; bit = MPREF_MAP_SHOW_GRID_OCEAN_SQ; break;
      case 7: id = 0x4F; bit = MPREF_MAP_HIDE_ACTIVE_VEH_GOTO_PATH; break;
    }
    switch (id) {
      case 0x49:
      case 0x4A:
        if (prefs & bit) {
          mapPrefCheckBox_.set_state_id(id, 1);
        } else {
          mapPrefCheckBox_.set_state_id(id, 0);
        }
        break;
      case 0x4F:
        if (more & bit) {
          mapPrefCheckBox_.set_state_id(id, 0);
        } else {
          mapPrefCheckBox_.set_state_id(id, 1);
        }
        break;
      default:
        if (more & bit) {
          mapPrefCheckBox_.set_state_id(id, 1);
        } else {
          mapPrefCheckBox_.set_state_id(id, 0);
        }
        break;
    }
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00490B00 0x00491200 0x00491220 0x00490B28
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00490B00 0x00491220 0x00491250 0x00490BF5
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00490B00 0x00491250 0x00491288 0x00490D20
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00490B00 0x00491288 0x00491294 0x00490DDE
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00490B00 0x00491294 0x0049129E 0x00490DD8
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00490B00 0x004912A0 0x004912CC 0x00490E66
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00490B00 0x004912CC 0x004912E8 0x00490F03
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00490B00 0x004912E8 0x0049131A 0x00490EFD
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00490B00 0x0049131C 0x00491350 0x00490FFE
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00490B00 0x00491350 0x00491360 0x004910D5
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00490B00 0x00491360 0x00491380 0x00491146
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00491380 0x004913E8 0x004913F8 0x0049138E
// ORIGINAL: 0x00491380 ?on_scrolling@PrefWin@@QAEXHH@Z 0x00491380-0x004913E6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00491380.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// symbol    ?on_scrolling@PrefWin@@UAEXHH@Z
void PrefWin::on_scrolling(int control_id, int position) {
  switch (control_id) {
    case 0x50: WaveDeviceGlobal.set_volume(position); break;
    case 0x51: WaveDeviceGlobal.set_group_volume(0, position); break;
    case 0x52: WaveDeviceGlobal.set_group_volume(1, position); break;
    case 0x53: if (FactionMusic) FactionMusic->set_volume(position); break;
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00491400 ?on_button_clicked@PrefWin@@QAEXH@Z 0x00491400-0x00491794;0x00658AC0-0x00658B6C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00491400.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// symbol    ?on_button_clicked@PrefWin@@UAEXH@Z
void PrefWin::on_button_clicked(int button_id) {
  Gamma gamma;
  switch (button_id) {
    case 6:
      g_PALETTE1.copy_from(&g_PALETTE2);
      PrefWinGammaCorrection = static_cast<float>(
          gamma.exec(&g_PALETTE1, PrefWinGammaCorrection, 0));
      prefs_put("Gamma Correction",
          static_cast<int>(PrefWinGammaCorrection * 100.0f), 0);
      break;
    case 0x5E:
      if (field_A14_ == 2) {
        field_A18_ = 2;
        field_A14_ = 6;
      } else {
        field_A18_ = 6;
        field_A14_ = 2;
      }
      break;
    case 0x5C:
      field_A5C_ = 1;
      // Accept falls through to the shared close.
    case 0x5B:
      destroy();
      break;
    case 0x5A: // Restore the rule and warning defaults; retain live UI controls.
      prefsBasicPending_ = default_prefs();
      morePrefsBasicPending_ = default_prefs2();
      prefsAdvPending_ = default_prefs();
      morePrefsAdvPending_ = default_prefs2();
      prefsAutoPending_ = default_prefs();
      morePrefsAutoPending_ = default_prefs2();
      prefsAVPending_ = default_prefs();
      morePrefsAVPending_ = default_prefs2();
      warningsPending_ = default_warn();
      prefsMapPending_ = default_prefs();
      morePrefsMapPending_ = default_prefs2();
      volumeMainToggle_.field_A18_ = 1;
      volumeVoiceToggle_.field_A18_ = 1;
      volumeSFXToggle_.field_A18_ = 1;
      volumeMusicToggle_.field_A18_ = 1;
      volumeMainScroll_.set_pos(0x7F);
      WaveDeviceGlobal.set_volume(0x7F);
      volumeVoiceScroll_.set_pos(0x7F);
      WaveDeviceGlobal.set_group_volume(0, 0x7F);
      volumeSFXScroll_.set_pos(0x5F);
      WaveDeviceGlobal.set_group_volume(1, 0x5F);
      volumeMusicScroll_.set_pos(0x3F);
      if (FactionMusic) FactionMusic->set_volume(0x3F);
      get_preferences();
      break;
  }
  change_volume_button_state(button_id);
  change_volume_button_text(button_id);
  redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00491810 ?redraw_option@PrefWin@@QAEXXZ 0x00491810-0x00491857 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00491810.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::redraw_option() {
  const long top = PrefWinGeometryValue_857898;
  const long left = PrefWinGeometryValue_85789C;
  RECT rect = {left, top,
      PrefWinGeometryValue_8578A8 + left,
      PrefWinGeometryValue_8578A4 + top};
  Buffer::box_sprite(&rect, &InterfaceBoxStyles[18]);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00491860 ?redraw_volume@PrefWin@@QAEXXZ 0x00491860-0x004918BA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00491860.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::redraw_volume() {
  const int top = PrefWinGeometryValue_8577DC - PrefWinNarrowDisplayVerticalOffset;
  RECT rect = {PrefWinGeometryValue_8577D8, top, PrefWinGeometryValue_8577E4 + PrefWinGeometryValue_8577D8, PrefWinGeometryValue_8577E0 + top};
  if (field_A14_ == 4)
    Buffer::box_sprite(&rect, &InterfaceBoxStyles[9]);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004918C0 0x004919E0 0x004919F0 0x004918D5
// ORIGINAL: 0x004918C0 ?change_volume_button_text@PrefWin@@QAEXH@Z 0x004918C0-0x004919DD FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004918c0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::change_volume_button_text(int button_id) {
  switch (button_id) {
    case 0x57:
      if (!volumeVoiceToggle_.field_A18_)
        volumeVoiceToggle_.set_name(StringTable.get(Labels.strings_ptr[0x43A]));
      else
        volumeVoiceToggle_.set_name(StringTable.get(Labels.strings_ptr[0x43B]));
      return;
    case 0x56:
      if (!volumeSFXToggle_.field_A18_)
        volumeSFXToggle_.set_name(StringTable.get(Labels.strings_ptr[0x43C]));
      else
        volumeSFXToggle_.set_name(StringTable.get(Labels.strings_ptr[0x43D]));
      return;
    case 0x58:
      if (!volumeMusicToggle_.field_A18_)
        volumeMusicToggle_.set_name(StringTable.get(Labels.strings_ptr[0x43E]));
      else
        volumeMusicToggle_.set_name(StringTable.get(Labels.strings_ptr[0x43F]));
      return;
    case 0x55:
      if (!volumeMainToggle_.field_A18_)
        volumeMainToggle_.set_name(StringTable.get(Labels.strings_ptr[0x438]));
      else
        volumeMainToggle_.set_name(StringTable.get(Labels.strings_ptr[0x439]));
      return;
    default:
      return;
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004919F0 0x00491BEC 0x00491BFC 0x00491A07
// ORIGINAL: 0x004919F0 ?change_volume_button_state@PrefWin@@QAEXH@Z 0x004919F0-0x00491BE9 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004919f0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::change_volume_button_state(int button_id) {
  switch (button_id) {
    case 0x57: {
      if (volumeVoiceToggle_.field_A18_) {
        volumeVoiceToggle_.field_A18_ = 0;
        WaveDeviceGlobal.disable_group(0);
        return;
      }
      const int main_selected = volumeMainToggle_.field_A18_;
      volumeVoiceToggle_.field_A18_ = 1;
      if (!main_selected) {
        volumeMainToggle_.field_A18_ = 1;
        change_volume_button_text(0x55);
      }
      WaveDeviceGlobal.enable_group(0);
      return;
    }
    case 0x56: {
      if (volumeSFXToggle_.field_A18_) {
        WaveDeviceGlobal.disable_group(1);
        volumeSFXToggle_.field_A18_ = 0;
        return;
      }
      const int main_selected = volumeMainToggle_.field_A18_;
      volumeSFXToggle_.field_A18_ = 1;
      if (!main_selected) {
        volumeMainToggle_.field_A18_ = 1;
        change_volume_button_text(0x55);
      }
      WaveDeviceGlobal.enable_group(1);
      return;
    }
    case 0x58: {
      if (volumeMusicToggle_.field_A18_) {
        if (FactionMusic) {
          delete FactionMusic;
          FactionMusic = nullptr;
        }
        volumeMusicToggle_.field_A18_ = 0;
        return;
      }
      const int main_selected = volumeMainToggle_.field_A18_;
      volumeMusicToggle_.field_A18_ = 1;
      if (!main_selected) {
        volumeMainToggle_.field_A18_ = 1;
        change_volume_button_text(0x55);
      }
      GameDataStateStorage.fields_.game_preferences_ |= PREF_AV_VOLUME_MUSIC_TOGGLE;
      load_music(ConsoleGlobal.faction_id_);
      if (FactionMusic) FactionMusic->begin();
      return;
    }
    case 0x55:
      if (volumeMainToggle_.field_A18_) {
        volumeMainToggle_.field_A18_ = 0;
        volumeMusicToggle_.field_A18_ = 0;
        volumeVoiceToggle_.field_A18_ = 0;
        volumeSFXToggle_.field_A18_ = 0;
        WaveDeviceGlobal.disable();
        GameDataStateStorage.fields_.game_preferences_ &= ~0xC00u;
        GameDataStateStorage.fields_.game_more_preferences_ &= ~0x4000u;
      } else {
        volumeMainToggle_.field_A18_ = 1;
        volumeMusicToggle_.field_A18_ = 1;
        volumeVoiceToggle_.field_A18_ = 1;
        volumeSFXToggle_.field_A18_ = 1;
        WaveDeviceGlobal.enable();
      }
      change_volume_button_text(0x58);
      change_volume_button_text(0x57);
      change_volume_button_text(0x56);
      return;
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00491C00 ?on_redraw@PrefWin@@QAEXXZ 0x00491C00-0x00491DEE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00491c00.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// symbol    ?on_redraw@PrefWin@@UAEXXZ
void PrefWin::on_redraw() {
  redraw_option();
  redraw_volume();
  hide_option_buttons();
  update_pref_buttons();
  bscPrefCheckBox_.redraw();
  advPrefCheckBox_.redraw();
  warnPrefCheckBox_.redraw();
  warnContPrefCheckBox_.redraw();
  autoPrefCheckBox_.redraw();
  audioVisPrefCheckBox_.redraw();
  mapPrefCheckBox_.redraw();
  volumeMainToggle_.redraw();
  volumeVoiceToggle_.redraw();
  volumeSFXToggle_.redraw();
  volumeMusicToggle_.redraw();
  okButton_.redraw();
  cancelButton_.redraw();
  restoreDefaultButton_.redraw();
}
// RECOVERY_GENERATED: v1 vtable 0x0066D424 ? ??_7PrefWin@@6BWin@@@ 0x004921E0
// RECOVERY_GENERATED: v1 vtable 0x0066D41C ? ??_7PrefWin@@6BBuffer@@@ 0x004921E0
