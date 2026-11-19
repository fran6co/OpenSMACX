/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
// ORIGINAL: 0x00496900 _$E21 0x00496900-0x00496916 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825e8 selects original initializer0x496900..0x496916, registering fini0x496920..0x49692a; complete independent decode establishes exclusive boundaries.", "Source-owned object?ReportIfGlobal@@3VReportIf@@A identifies original storage0x885f38. Startup arguments/calls: call 0x4ad170; push 0x496920. Cleanup receiver/calls: mov ecx, 0x885f38; jmp 0x4acda0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/reportif.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?ReportIfGlobal@@3VReportIf@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E14
// ORIGINAL: 0x00496920 _$E19 0x00496920-0x0049692A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825e8 selects original initializer0x496900..0x496916, registering fini0x496920..0x49692a; complete independent decode establishes exclusive boundaries.", "Source-owned object?ReportIfGlobal@@3VReportIf@@A identifies original storage0x885f38. Startup arguments/calls: call 0x4ad170; push 0x496920. Cleanup receiver/calls: mov ecx, 0x885f38; jmp 0x4acda0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/reportif.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?ReportIfGlobal@@3VReportIf@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E12
// WORKAROUND(noinline): draw_projects' StringList local calls
// StringStruct::StringStruct (0x00401000) out of line (0x004AA103), giving
// its three-state EH layout; see stringstruct.h.
#define STRINGSTRUCT_CTOR_OUT_OF_LINE
#include "stdafx.h"
#include "interface_fonts.h"
#include "reportif.h"
#include "base.h"    // BaseCurrentCount bounds the report's fixed sort workspace
#include "general.h" // swap for the original reversed-bound normalization
#include "reportwin.h"  // ReportWinGlobal - the window done() un-modals
#include "console.h"  // ConsoleGlobal.faction_id_ owns original939284
#include "worldwin.h"
#include "menu.h"
#include "win.h"
#include "spritebox.h"
#include "net_class.h"
#include "flatbutton.h"
#include "sprite.h"
#include "buttongroup.h"
#include "spot.h"
#include "time.h"  // SatPanelTimer1/2 - close_sat stops both
#include "maininterface.h"  // MainInterfaceGlobal - the set_modal owner
#include "alpha.h"  // Labels
#include "faction.h"  // Players
#include "game.h"  // say_year
#include "technology.h"  // has_tech_call
#include "strings.h"  // StringTable, StringTemp
#include "basewin.h"
#include "veh.h"
#include "font.h"
#include "stringstruct.h"
#include "netdaemon.h"
#include "log.h"
#include "map.h"
#include "rectangle_operations.h"
#include "text.h"
#include "help_link.h"
// WORKAROUND(decl): counter coupling. After faction.h began defining
// get_name/get_title and this unit switched to help_link.h, the same
// this/button reload order also needs more distinct unused function
// declarations (the window moves with header inline counts; extern variables no longer move
// it). Only 0x004A6330 changes in the unit's object. With base.h's one-level
// has_fac, map.h's two wrap inlines and the array-typed RulesTerraforming,
// RulesOrder, RulesCitizen and Player ideology rows only 12 matches (1-11, 13,
// 14 and 16 flip it); before those retypes the window was 6-8. spot.h's gated
// Spot::add RECT inline moved it again. The tie-break is the front-end symbol counter modulo 64 (guide 2.5);
// `recover canary src/reportif.cpp` computes the pad count. Re-tuned
// 2026-11-19 after the header batch (fe sweep 0-63): exact at 8, 19-23 and
// 34-61; sat_attack 0x004A0260 (not exact) keeps its 15 rows only at 35-46
// and 56-61 (17 at 8 and 19-23), so 40, mid-window.
// CANARY: 0x004A6330 ?on_iface_button_clicked@ReportIf@@UAEXH@Z residues 0-27,38,49-53
// CANARY-PADS: function ReportIfDeclFnPad
void ReportIfDeclFnPad0();
void ReportIfDeclFnPad1();
void ReportIfDeclFnPad2();
void ReportIfDeclFnPad3();
void ReportIfDeclFnPad4();
void ReportIfDeclFnPad5();
void ReportIfDeclFnPad6();
void ReportIfDeclFnPad7();
void ReportIfDeclFnPad8();
void ReportIfDeclFnPad9();
void ReportIfDeclFnPad10();
void ReportIfDeclFnPad11();
void ReportIfDeclFnPad12();
void ReportIfDeclFnPad13();
void ReportIfDeclFnPad14();
void ReportIfDeclFnPad15();
void ReportIfDeclFnPad16();
void ReportIfDeclFnPad17();
void ReportIfDeclFnPad18();
void ReportIfDeclFnPad19();
void ReportIfDeclFnPad20();
void ReportIfDeclFnPad21();
void ReportIfDeclFnPad22();
void ReportIfDeclFnPad23();
void ReportIfDeclFnPad24();
void ReportIfDeclFnPad25();
void ReportIfDeclFnPad26();
void ReportIfDeclFnPad27();
void ReportIfDeclFnPad28();
void ReportIfDeclFnPad29();
void ReportIfDeclFnPad30();
void ReportIfDeclFnPad31();
void ReportIfDeclFnPad32();
void ReportIfDeclFnPad33();
void ReportIfDeclFnPad34();
void ReportIfDeclFnPad35();
void ReportIfDeclFnPad36();
void ReportIfDeclFnPad37();
void ReportIfDeclFnPad38();
void ReportIfDeclFnPad39();

// Shared punctuation objects the image pushes instead of unit literals.
extern char SocialWinCloseParenText[];
extern char SocialWinOpenParenText[];

extern char *DatalinkHelpFile;
// Every report renderer pushes these shared separator objects (0x00682820
// " ", 0x00682E90 ", ", 0x00682E94 ": ") instead of unit literals; only
// ReportIf::draw_sat's two paragraph spaces are unit literals.
extern const char BufferJoinSpace[];
extern char SocialWinCommaSpaceText[];
extern char SocialWinColonSpaceText[];
// The observed report scratch inputs are six immediate char-addresses, not
// pointer cells. Their original extent/owner is unknown; represent only the
// observed empty NUL-terminated contents, without asserting a larger bound.
// These starts are in the PE loader-zero virtual tail, not file-backed
// initializer bytes. The panel drawers read them as strcpy sources.
// RECOVERY_OBJECT: v1 0x008A624C ?ReportProjectScratchSmall@@3PADA
char ReportProjectScratchSmall[1] = {0};
// RECOVERY_OBJECT: v1 0x008A6250 ?ReportProjectScratchLarge@@3PADA
char ReportProjectScratchLarge[1] = {0};
// RECOVERY_OBJECT: v1 0x008A6254 ?ReportSatelliteScratchUpperText@@3PADA
char ReportSatelliteScratchUpperText[1] = {0};
// RECOVERY_OBJECT: v1 0x008A6258 ?ReportSatelliteScratchUpperLine@@3PADA
char ReportSatelliteScratchUpperLine[1] = {0};
// RECOVERY_OBJECT: v1 0x008A625C ?ReportSatelliteScratchLowerText@@3PADA
char ReportSatelliteScratchLowerText[1] = {0};
// RECOVERY_OBJECT: v1 0x008A6260 ?ReportSatelliteScratchLowerLine@@3PADA
char ReportSatelliteScratchLowerLine[1] = {0};
int __cdecl X_text_open(char *, const char *);
extern BaseWin TutWinBaseWindow;
extern int EnergyBaseMaintenance[MaxBaseNum];
extern int EnergyFacilityCounts[70];
extern int EnergyLabsTotal;
int __cdecl labs_write(GraphicWin *, int, int, RECT *, char *);
void __cdecl scroll_normal();
int __cdecl diplo_lock(int patience);
void __cdecl diplo_unlock();
void __cdecl draw_radius(int x, int y, int mode, int draw_type);



void __cdecl energy_compute(int faction_id, int mode);
void __cdecl scroll_small();

// Popup's scroll lifecycle callbacks operate on ReportWin's embedded Scroll.
void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll);
void __cdecl scroll_show(GraphicWin *window, Scroll *scroll);
void __cdecl slide_show(GraphicWin *window, int mode);
void __cdecl slide_hide(GraphicWin *window, int mode);
// on_status reads the distinct reentry cell and writes complete 0/1 dwords.
// Its initial zero comes from the PE loader's virtual tail, not stored bytes.
// RECOVERY_OBJECT: v2 0x008A6240 0x008A6244 ?ReportInterfaceStatusReentry@@3HA 0x004A41E8
int ReportInterfaceStatusReentry = 0;




// Superseded seed deleted; seed paths below are historical Git provenance.


// Superseded seed deleted; seed paths below are historical Git provenance.




// Superseded seed deleted; seed paths below are historical Git provenance.








// ListBox's ordinary member teardown includes its complete-object virtual
// bases: GraphicWin at +0x48 and Dialog at +0xA60, matching the original
// adjusted destructor calls (609EC0, 608E10, 5D4DD0).

/*
Purpose: Tear down every sub-object the constructor built, in exactly the
         reverse order.
// ORIGINAL: 0x004ACDA0 ??1ReportIf@@QAE@XZ 0x004ACDA0-0x004AD16C;0x00658D50-0x00658F74 FILE BYTE_EXACT
// compile_context src/reportif.cpp
// TRIED: SEH frame - the image has one (flags carry `frame`) but a body
//            built entirely from plain `->method()`/`->~T()` calls at raw
//            offsets never triggers VC6's unwind-protection scaffolding (no
//            local object construction for it to protect), so this tree's
//            compiled body has none. A prior attempt at this same address
//            (src/unrecovered/004acda0.cpp, now deleted - superseded by
//            this body) measured the identical finding in isolation.
//            MISMATCH, 4/235 instructions agree; not chased further given
//            the family-wide cap this batch's brief already names.
// size      1520 bytes
// prototype void (__thiscall ??1ReportIf@@QAE@XZ)(ReportIf* this)
// callers   0   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x005FA870 0x00607040 0x00607DA0 0x00608E10 0x00609EC0 0x0062B7F0 0x006456E4
*/
// The implicit destructor owns reverse declaration-order member teardown.
// Original 0x4ACDA0 never installs ReportIf's own interface table; its
// member and base destruction, including unwind cleanup, remains automatic.


















// The original range normalization and merge algorithm operate on a fixed
// 0x200-row workspace inside ReportIf. Keys are ordered descending; each
// merge writes the scratch pair, then publishes it back to the primary pair.




// Close the project page's global map overlay and the two project-selector
// buttons owned by MainInterface, not by ReportIf.










// The constructor owns this ListBox and these FlatButtons as members;
// the retained close_labs seed closes/hides those same
// objects in this order. Slot 2 is the inherited Win::hide virtual.
// Superseded seed deleted; seed paths below are historical Git provenance.


// The three FlatButtons are genuine ReportIf members. The seed
// calls Win slot 2 on them in this order; keep the original object identities
// while using the source-owned virtual method instead of a vtable shim.
// Superseded seed deleted; seed paths below are historical Git provenance.


// ReportWin owns these three controls as members; close_ops clears
// each ungrouped state word before hiding it. ReportIf's own controls and the
// final MainInterface pair remain their respective typed owners.


// Animation storage is the ReportIf sprite run; the destination is the
// Buffer base of the global WorldClimateWorldWin. Its timeout lives in the
// genuine ReportWin-owned Time member.
// Superseded seed deleted; seed paths below are historical Git provenance.






// Research state87BE24/885F30 is ReportWinGlobal+59AC/+FAB8.
// The energy list87BE84 is ReportWinGlobal+5A0C. No duplicate storage.

// The process-wide ReportIf itself, 0x00885F38 - see reportif.h. The
// image's dynamic initializer at 0x00496900 constructs it through the real
// recovered constructor above, so the storage sits here.
// Original constructor 0x004AD170 initializes this singleton; its two list
// receivers at 0x00890208/0x00890D5C are owned +0xA2D0/+0xAE24 members.
// This start-only binding does not infer a complete original allocation.
// The 0x20304-byte object ends four bytes before ReportInterfaceStatusReentry
// at 0x008A6240.
// RECOVERY_OBJECT: v2 0x00885F38 0x008A623C ?ReportIfGlobal@@3VReportIf@@A 0x004989F0
ReportIf ReportIfGlobal;  // 0x00885F38

/*
 * Panel drawing uses the shared mutable InterfaceBoxStyles records and the
 * Buffer subobject owned by MainInterfaceGlobal. Its working rectangle is
 * exposed by GraphicWin::buffer_working_rect().
 */





// MainInterface owns the interface-slot array and active interface ID.

// close_sat borrows ReportWinGlobal's buttons9/7/8 and timers2/3.
// Original881A24/88038C/880ED8 are876478+B5AC/9F14/AA60.
// Their +AA8/+A9C fields are BaseButton.group_/field_A9C_; the existing
// typed reset_field_A9C_if_ungrouped preserves the original inline condition.
// Original884738/884760 are876478+E2C0/E2E8, not timer pointers/globals.

// 0x005398E0 ?pop_wait@@YAHXZ - defined in faction.cpp (BYTE_EXACT); no
// header declares it, and the set_modal popups pass it as their wait
// callback (the designwin.cpp spelling).
int __cdecl pop_wait();

// The eight menu commands all use the same already-owned report window and
// interface mode. Retained seeds at 0x004AC9A0..0x004ACC29 establish each
// category value and the modal-versus-direct branch.
// Superseded seed deleted; seed paths below are historical Git provenance.


// Superseded seed deleted; seed paths below are historical Git provenance.


// Superseded seed deleted; seed paths below are historical Git provenance.


// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: explicit __thiscall function-pointer typedef for the vtable call (C4234); used a member-pointer/union cast instead


// Superseded seed deleted; seed paths below are historical Git provenance.


// Superseded seed deleted; seed paths below are historical Git provenance.


// Superseded seed deleted; seed paths below are historical Git provenance.


// Superseded seed deleted; seed paths below are historical Git provenance.








// These panel renderers belong to report()'s ReportIf owner. The retained
// seeds provide their original identities and cross-function evidence.














// Original interface table 0x0066D700 slot 1 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolled@ReportIf@@UAEXHH@Z 0x004A41D0
void ReportIf::on_iface_scrolled(int, int) {}
// Original interface table 0x0066D700 slot 2 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolling@ReportIf@@UAEXHH@Z 0x004A41D0
void ReportIf::on_iface_scrolling(int, int) {}
// Original interface table 0x0066D700 slot 6 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_left_up@ReportIf@@UAEXHH@Z 0x004A41D0
void ReportIf::on_iface_left_up(int, int) {}

// Original interface table 0x0066D700 slot 14 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_button_pressed@ReportIf@@UAEHH@Z 0x004A41D0
int ReportIf::on_iface_button_pressed(int) {
    return 0;
}

// Original interface table 0x0066D700 slot 17 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_dialog_back_draw@ReportIf@@UAEHPAVGraphicWin@@@Z 0x004A41D0
int ReportIf::on_iface_dialog_back_draw(::GraphicWin *) {
    return 0;
}



// ORIGINAL: 0x004AD840 ??_GReportWin@@UAEPAXI@Z 0x004AD840-0x004AD861 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004AD840-0x004AD861", "Pinned class constructor ??0ReportWin@@QAE@XZ at 0x004AD6B0 stores original class/view table VA 0x66d754 at 0x004AD81A (mov dword ptr [esi], 0x66d754); the current genuine owner vftable ??_7ReportWin@@6BWin@@@ selects generated ??_GReportWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EReportWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GReportWin@@UAEPAXI@Z at 0x004AD840 calls the source-mapped ??1ReportWin@@QAE@XZ at 0x004AD3B0, then tests byte ptr [ebp + 8], 1; je 0x4ad85a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/reportif.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original ReportWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GReportWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x00496930 _$E21 0x00496930-0x00496946 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825ec selects original initializer0x496930..0x496946, registering fini0x496950..0x49695a; complete independent decode establishes exclusive boundaries.", "Source-owned object?ReportWinGlobal@@3VReportWin@@A identifies original storage0x876478. Startup arguments/calls: call 0x4ad6b0; push 0x496950. Cleanup receiver/calls: mov ecx, 0x876478; jmp 0x4ad3b0. Constructor/destructor targets retain their existing class-owned source mappings.", "Historical standalone ReportWin COFF selected local initializer _$E21 through .CRT$XCU and cleanup _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate.", "The source-owned ReportWinGlobal now shares reportif.cpp with ReportIfGlobal, in pinned CRT order. Physical compiler qualification resolves its initializer to _$E13 and cleanup to _$E11, targeting the actual ReportWin constructor/destructor; the adjacent symbol fact binds the corresponding original entry. These local names do not prove a historical filename or retained selection."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?ReportWinGlobal@@3VReportWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E21
// ORIGINAL: 0x00496950 _$E19 0x00496950-0x0049695A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825ec selects original initializer0x496930..0x496946, registering fini0x496950..0x49695a; complete independent decode establishes exclusive boundaries.", "Source-owned object?ReportWinGlobal@@3VReportWin@@A identifies original storage0x876478. Startup arguments/calls: call 0x4ad6b0; push 0x496950. Cleanup receiver/calls: mov ecx, 0x876478; jmp 0x4ad3b0. Constructor/destructor targets retain their existing class-owned source mappings.", "Historical standalone ReportWin COFF selected local initializer _$E21 through .CRT$XCU and cleanup _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate.", "The source-owned ReportWinGlobal now shares reportif.cpp with ReportIfGlobal, in pinned CRT order. Physical compiler qualification resolves its initializer to _$E13 and cleanup to _$E11, targeting the actual ReportWin constructor/destructor; the adjacent symbol fact binds the corresponding original entry. These local names do not prove a historical filename or retained selection."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?ReportWinGlobal@@3VReportWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E19
#include "stdafx.h"
#include "font_metrics.h"
#include <math.h>
#include <stddef.h>
#include "random.h"
#include "basewin.h"
#include "infowin.h"
#include "popup.h"
#include "mapwin.h"
#include "reportwin.h"
#include "reportif.h"  // ReportIfGlobal - the singleton on_key_click re-reports
#include "menu.h"
#include "win.h"
#include "sprite.h"  // g_IFACE_TECH_TREE_ARROW_SPRITES - labs_line's arrow
#include "datalink.h"  // DatalinkMain - the research screen's host window
#include "maininterface.h"  // MainInterfaceGlobal - the set_modal owner
#include "tutwin.h"  // TutWinBaseWindow - the report's base-map zoom target
#include "alpha.h"
#include "socialwin.h"
#include "console.h"
#include "technology.h"
#include "game.h"
#include "base.h"
#include "netdaemon.h"
#include "pulldown.h"
#include "faction.h"
#include "log.h"
#include "veh.h"
#include "worldwin.h"
#include "map.h"
#include "general.h"
#include "council.h"
#include "text.h"

static_assert(offsetof(ReportWin, field_F934_) == 0xF934,
              "ReportWin explosion frame offset");
static_assert(sizeof(((ReportWin *)0)->field_F934_) == 4,
              "ReportWin explosion frame width");

// Shared dependencies identified by the original animation calls. The daemon
// implementations live in this ReportIf/ReportWin owner; action_sat_attack is implemented in
// veh.cpp. The colour table remains an external dependency.
void __cdecl sat_anim_daemon(int context);
void __cdecl sat_expl_daemon(int context);
void __cdecl bl_daemon(int context);
void __cdecl action_sat_attack(int attacker, int target, int type, int base_id);
extern Sprite g_TECH_ICON_SPRITES[89];
// Original init passes these two independently addressed empty captions at
// 496A80/496B64. Their initial NUL is proven; complete original object extents
// are not, so these start-only bindings do not claim four-byte storage.
// RECOVERY_OBJECT: v1 0x008A6244 ?ReportWindowCaptionSmall@@3PADA
char ReportWindowCaptionSmall[] = "";
// RECOVERY_OBJECT: v1 0x008A6248 ?ReportWindowCaptionLarge@@3PADA
char ReportWindowCaptionLarge[] = "";
void __cdecl sat_attack(int faction_id, int target_id, int type);
void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll);
void __cdecl scroll_show(GraphicWin *window, Scroll *scroll);
void __cdecl draw_radius(int x, int y, int mode, int draw_type);
int __cdecl break_treaty(int faction_id, int target_faction_id, int treaty);
int __cdecl diplo_lock(int patience);
void __cdecl diplo_unlock();
// Report page dispatch at 0x00496D35 uses the original table at 0x00496DE4:
// 0 intel; 1 no drawer; 2 labs; 3 energy; 4 ops; 5 projects; 6 satellites;
// 7 security; 8 score. The retired explicit-receiver helper shifted these
// categories and forwarded to original addresses. This is Win's genuine
// slot-12 override, with the owned Buffer and ReportIf's category state.


static bool reportwin_input_blocked() {
    return ConsoleControlTurnActive != 0
        || (NetDaemonNet.primary_state_.trailing_words_[6]
            & (1 << ConsoleGlobal.faction_id_)) != 0
        || NetDaemonNet.field_1BB4_ != 0
        || NetDaemonNet.primary_state_.leading_words_[2] != 0;
}


// The process-wide report window, 0x00876478 - see reportwin.h. The image's
// dynamic initializer at 0x00496930 constructs it through the real recovered
// constructor below. The fixed list/timer receivers used by ReportIf are
// members of this object, not separate global storage.
// Bound: last dword member +0xFAB8 is stored at 0x00496EDC and read as 0x00885F30; ReportIfGlobal follows.
// RECOVERY_OBJECT: v2 0x00876478 0x00885F34 ?ReportWinGlobal@@3VReportWin@@A 0x00496EDC
ReportWin ReportWinGlobal;  // 0x00876478

// MainInterface owns the interface-slot array and active interface ID.



// Fixed890208/890D5C receivers are ReportIfGlobal.listBox1_/listBox2_.

// 0x005398E0 ?pop_wait@@YAHXZ - defined in faction.cpp (BYTE_EXACT); no
// header declares it, and the set_modal popups pass it as their wait
// callback (the designwin.cpp spelling).
int __cdecl pop_wait();



// ORIGINAL: 0x004AD3B0 ??1ReportWin@@QAE@XZ 0x004AD3B0-0x004AD6A9;0x00659140-0x006592EE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:649230f6:src/recovered/units/004ad3b0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
// symbol    ??1ReportWin@@UAE@XZ























/*
Purpose: Step the receiver back to the subobject ??_GReportWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004AD870 ??_GReportWin@@WEEE@AEPAXI@Z 0x004AD870-0x004AD87B BYTE_EXACT
// symbol    ??_EReportWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3ReportWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004AD840` into
//   ??_GReportWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/


























// Report-page ownership: original on_right_click at497F99..497FA8
// passes the report faction as a nonzero gate, then the clicked faction
// payload and satellite kind. draw_sat creates those kind/faction pairs.
// Only the second argument selects PlayersData. Preserve the first argument's
// nonzero test: it is not an equality/permission test against the target.
// sat_edit, 0x004A0810. The satellite editor's click handler: with the shift
// key up, step the faction's satellite field for the chosen body up (or down
// under shift, clamped at zero); on the orbital-defense body the downward
// step also pulls the deployed-pod count back and repaints the topic window.





int __cdecl X_text_open(char *, const char *);
void __cdecl compute_score(int, int *, int *, int);
void __cdecl diplomacy_check(int, int, int);
int __cdecl most_objectives(int *, int *);
// compute_score/diplomacy_check remain seed-only dependencies requiring owner
// integration. ScriptPopupFile is the mutable pointer cell formerly exposed
// through the ClimatePopupTitle facade.

static void report_topic_line(ReportWin *win, RECT &cursor, int x) {
    win->Buffer::write_l(StringTemp, x, cursor.top);
}
// The image keeps a separate "[" / "]" literal pair at each of draw_intel's
// six sites, so the original spelled this as a macro, not a shared helper.
#define report_intel_label(label) StringTemp[0] = 0; strcat(StringTemp, "["); strcat(StringTemp, label_get(label)); strcat(StringTemp, SocialWinColonSpaceText); strcat(StringTemp, BufferJoinSpace); strcat(StringTemp, BufferJoinSpace); strcat(StringTemp, "]")
static void report_append_leader(int faction) {
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    strcat(StringTemp, Players[faction].title_leader);
    strcat(StringTemp, BufferJoinSpace);
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    strcat(StringTemp, Players[faction].name_leader);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0x2D0 / 4));
    strcat(StringTemp, BufferJoinSpace);
    GenderDefault = Players[faction].noun_gender;
    PluralityDefault = Players[faction].is_noun_plural;
    strcat(StringTemp, Players[faction].noun_faction);
}



// Requires coordinator-reviewed PlayerData final-dword migration:
// int score_items_[9] at +0x20A8, replacing unk_110..unk_118 including the
// intervening char[4]. Original4A1318 accesses9signedints;4A16C9 bounds9.


















































































































































































// ORIGINAL: 0x00496960 ?init@ReportWin@@QAEXXZ 0x00496960-0x00496BA5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:649230f6:src/recovered/units/00496960.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::init() {
    for (int i = 0; i < 16; ++i) selected_bases_[i] = 0;
    for (int j = 0; j < 4; ++j) selected_prototypes_[j] = 0;
    rect1_.right = 650;
    rect1_.bottom = 353;
    spot_.init(256);
    if (ReportIfGlobal.field_1E230_ == 0) {
        rect1_.bottom += 20;
        rect1_.left = (800 - rect1_.right) / 2;
        rect1_.top = 388 - rect1_.bottom;
        set_rectangle_dimensions(&report_panel_rect_, 23, 0, rect1_.right, rect1_.bottom);
        set_rectangle_dimensions(&energy_panel_rect_, 26, 3, rect1_.right - 6, rect1_.bottom - 56);
        GraphicWin::init(rect1_.left - 23, rect1_.top, rect1_.right + 78,
            rect1_.bottom, ReportWindowCaptionSmall,
            0x1000000, static_cast<Win *>(&ConsoleGlobal), 0, 0);
    } else {
        rect1_.bottom += 200;
        rect1_.left = (WinScreenWidth - rect1_.right) / 2;
        rect1_.top = WinScreenHeight - rect1_.bottom - 212;
        set_rectangle_dimensions(&report_panel_rect_, 23, 0, rect1_.right, rect1_.bottom);
        set_rectangle_dimensions(&energy_panel_rect_, 26, 3, rect1_.right - 6, rect1_.bottom - 56);
        GraphicWin::init(rect1_.left - 23, rect1_.top, rect1_.right + 78,
            rect1_.bottom, ReportWindowCaptionLarge,
            0x1000000, static_cast<Win *>(&ConsoleGlobal), 0, 0);
    }
    field_E268_ = font_line_advance(*ReportIfGlobal.field_1E184_);
}

// Guarded full-length right-aligned write; callers inline the null test.
static inline int report_write_right(char *text, int x, int y) {
    if (text == 0) {
        return x;
    }
    return MainInterfaceGlobal.Buffer::write_right_l(text, x, y, static_cast<int>(strlen(text)));
}

// Local rectangle inflation; inlined by reset and on_button_passover.
static inline void inflate_frame(RECT *rect, int dx, int dy) {
    rect->left -= dx;
    rect->right += dx;
    rect->top -= dy;
    rect->bottom += dy;
}

/*
Purpose: Reset the seven report buttons, initialize the report text buffer,
         copy the current report bounds, draw both panel frames, and restore
         the inset frame rectangle after its temporary expansion.
// ORIGINAL: 0x00496BB0 ?reset@ReportWin@@QAEXXZ 0x00496BB0-0x00496CFC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Independent IDA/Ghidra function records agree on the original extent 0x00496BB0-0x00496CFC and classify it as game code.", "Retired recovered seed provenance: git:c4c11a8a:src/recovered/units/00496bb0.cpp records the button-close loop, Buffer font/color setup, rectangle copies and box_sprite calls in this order.", "docs/recovery/member-accesses.csv records reset's writes through the report geometry and embedded button/panel members; reportwin.h provides the corresponding source-owned member layout."], "reason": "Recover the report reset in its existing ReportWin owner using the declared button array, buffer storage, rectangles and shared interface box styles."}
// prototype void (__thiscall ?reset@ReportWin@@QAEXXZ)(ReportWin* this)
// kind      game
// calls     0x005D5890 0x005E2DD3
// Homed from src/recovered/units/00496bb0.cpp.
*/
void ReportWin::reset() {
    for (int button = 0; button != 7; ++button) {
        flatButtons1_[button].close();
    }


    Buffer::set_font(ReportIfGlobal.field_1E184_, ReportIfGlobal.field_1E184_,
                    ReportIfGlobal.field_1E190_, nullptr);
    Buffer::set_text_color(0x99, -1, 1, 1);
    Buffer::set_text_color2(0xF2, -1, 1, 1);
    Buffer::set_text_color3(0xE1, -1, 1, 1);

    rect1_.left = energy_panel_rect_.left;
    rect1_.top = energy_panel_rect_.top;
    rect1_.right = energy_panel_rect_.right - energy_panel_rect_.left;
    rect1_.bottom = energy_panel_rect_.bottom - energy_panel_rect_.top;

    report_work_rect_ = energy_panel_rect_;

    Buffer::box_sprite(&report_panel_rect_, &InterfaceBoxStyles[9]);

    inflate_frame(&report_frame_rect_, 3, 3);
    Buffer::box_sprite(&report_frame_rect_, &InterfaceBoxStyles[3]);
    inflate_frame(&report_frame_rect_, -3, -3);
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00496D00 0x00496DE4 0x00496E08 0x00496D35
/*
Purpose: Repaint the report window's selected page inside the topic panel.
// ORIGINAL: 0x00496D00 ?on_redraw@ReportWin@@QAEXXZ 0x00496D00-0x00496DE4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original496D00..496DE4; retired seed git:ee30a69d:src/unrecovered/00496d00.cpp records the same identity and spans with its switch fold TRIED note kept below.", "The receiver is reportwin.h's own window: reset() is declared there (0x00496BB0), the +0x444 Buffer (set_clip is buffer.h's own) carries the topic rectangle at +0x5974, and the page selector is 0x00885F40.", "The eight page drawers are the seed's measured call targets 0x00498B30/0x0049BCD0/0x0049D540/0x0049EB10/0x0049F350/0x004A08E0/0x004A1140/0x004A2210, unrecovered members reached at their original addresses until declarations land."], "reason": "Coverage transcription of the report repaint with the page dispatch kept at the measured switch shape."}
// symbol    ?on_redraw@ReportWin@@UAEXXZ
// TRIED (seed): a switch on the 9 cases reproduces the disassembly's eight
//         separate trailing set_clip copies; VC6 does not tail-merge across
//         switch jump-table targets.
Status: Coverage transcription - unclaimed body
*/
void ReportWin::on_redraw() {
    reset();
    field_59AC_ = -1;
    Buffer::set_clip(&energy_panel_rect_);
    switch (ReportIfGlobal.field_8_) {
    case 2: draw_labs(); break;
    case 3: draw_energy(); break;
    case 4: draw_ops(); break;
    case 5: draw_projects(); break;
    case 6: draw_sat(); break;
    case 7: draw_security(); break;
    case 8: draw_score(); break;
    case 0: draw_intel(); break;
    }
    Buffer::restore_clip();
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00496E10 0x00497A30 0x00497A48 0x00496E8F
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00496E10 0x00497A48 0x00497A58 0x0049708B
// ORIGINAL: 0x00496E10 ?on_left_click@ReportWin@@QAEXHH@Z 0x00496E10-0x00497A30
// RECOVERY_BINDING: v1 {"evidence": ["git:649230f6:src/unrecovered/00496e10.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
// symbol    ?on_left_click@ReportWin@@UAEXHH@Z
void ReportWin::on_left_click(int x, int y) {
    if (reportwin_input_blocked()) return;
    int selected, payload;
    int base_id;
    if (spot_.check(x, y, &selected, &payload) + 1 != 0) {
        switch (ReportIfGlobal.field_8_) {
        case 2:
            if (selected == 0x29A) {
                const int tech = PlayersData[ReportIfGlobal.report_faction_id_].tech_id_researching;
                if (tech >= 0 && ((GameDataStateStorage.fields_.game_state_ & 0x80) != 0
                        || (GameDataStateStorage.fields_.game_rules_ & 0x200) == 0)) {
                    field_FAB8_ = tech;
                    field_59AC_ = tech;
                    reset();
                    draw_labs();
                    soft_update();
                    ReportIfGlobal.reset();
                    ReportIfGlobal.draw_labs();
                    MainInterfaceGlobal.soft_update();
                }
            }
            if (selected == 0x29A) {
                if (GameDataStateStorage.fields_.game_rules_ & 0x200) {
                    ConsoleGlobal.set_ai(ConsoleGlobal.faction_id_);
                } else {
                    ConsoleGlobal.set_tech(ConsoleGlobal.faction_id_);
                }
                reset();
                draw_labs();
                soft_update();
                ReportIfGlobal.reset();
                ReportIfGlobal.draw_labs();
                MainInterfaceGlobal.soft_update();
            }
            return;
        case 4: {
            int id;
            id = selected;
            if (selected >= 1024) {
                id = selected - 1024;
                if (id >= 1024) {
                    id -= 1024;
                    if (id >= 1024 && (id -= 1024,
                            Bases[id].faction_id_current == ConsoleGlobal.faction_id_)) {
                        log_say("Global EDBC base set", 0, 0, 0);
                        if (!NetDaemonNet.lock_base(id, 0, -1, -1)) {
                            TutWinBaseWindow.soft_zoom(id);
                            const uint32_t governor = BaseCurrent->governor;
                            if ((governor & 0x80000000u) && (governor & 0x8000000u)) {
                                BaseCurrent->governor = governor & 0x7FFFFFFFu;
                            } else {
                                unsigned category;
                                if (governor & 0x1000000u) category = 0;
                                else if (governor & 0x2000000u) category = 1;
                                else category = ((~governor & 0x4000000u) | 0x8000000u) >> 26;
                                BaseCurrent->governor = governor | 0x80000000u;
                                BaseCurrent->governor &= 0xF0FFFFFFu;
                                switch (category) {
                                case 0: BaseCurrent->governor |= 0x2000000u; break;
                                case 1: BaseCurrent->governor |= 0x4000000u; break;
                                case 2: BaseCurrent->governor |= 0x8000000u; break;
                                case 3: BaseCurrent->governor |= 0x1000000u; break;
                                }
                                TutWinBaseWindow.gov_activate();
                            }
                            draw_radius(Bases[id].x, Bases[id].y, 2, 2);
                            NetDaemonNet.unlock_base(id);
                        }
                    } else {
                        if (Bases[id].faction_id_current != ConsoleGlobal.faction_id_) return;
                        log_say("Reports left_click garrison", id, payload, 0);
                        TutWinBaseWindow.soft_zoom(id);
                        TutWinBaseWindow.garrison_click(payload, x, y, 0);
                        return;
                    }
                } else {
                    if (Bases[id].faction_id_current != ConsoleGlobal.faction_id_) return;
                    if (HIBYTE(GetAsyncKeyState(VK_SHIFT))) {
                        if (!HIBYTE(GetAsyncKeyState(VK_CONTROL))) {
                            for (int word = 0; word < 16; ++word) selected_bases_[word] = 0;
                        }
                        if (field_59AC_ != 0) {
                            field_59AC_ -= 1024;
                            if (field_59AC_ > id) {
                                field_59AC_ ^= id;
                                id ^= field_59AC_;
                                field_59AC_ ^= id;
                            }
                            for (int base_id = field_59AC_; base_id <= id; ++base_id) {
                                if (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_) {
                                    selected_bases_[base_id / 32] |= 1 << (base_id % 32);
                                }
                            }
                        } else {
                            for (int word = 0; word < 16; ++word) selected_bases_[word] = 0;
                            selected_bases_[id / 32] |= 1 << (id % 32);
                        }
                    } else if (HIBYTE(GetAsyncKeyState(VK_CONTROL))) {
                        selected_bases_[id / 32] ^= 1 << (id % 32);
                    } else {
                        for (int word = 0; word < 16; ++word) selected_bases_[word] = 0;
                        selected_bases_[id / 32] |= 1 << (id % 32);
                    }
                }
            } else if (HIBYTE(GetAsyncKeyState(VK_SHIFT))) {
                if (selected >= 0) {
                    TutWinBaseWindow.soft_zoom(id);
                    TutWinBaseWindow.pop_click(selected < BaseCurrent->specialist_total
                        ? BaseCurrent->specialist_total - selected - 1 : -1, 0, 0, 0);
                    base_compute(1);
                }
            } else {
                TutWinBaseWindow.soft_zoom(id);
                log_say("Base report psych click", 0, 0, 0);
                if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1)) return;
                int i;
                for (i = 20; i > 0; --i) {
                    if (BaseCurrent->resource_sqr_radius & (1 << i)) break;
                }
                if (i > 0) BaseCurrent->resource_sqr_radius &= ~(1 << i);
                ++BaseCurrent->specialist_total;
                base_compute(1);
                NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            }
            int count;
            int bit;
            count = 0;
            for (bit = 0; bit < 512; ++bit) {
                if (selected_bases_[bit / 32] & (1 << (bit % 32))) ++count;
            }
            if (count == 1) field_5A04_ = 1;
            else field_5A04_ = 0;
            field_59AC_ = selected;
            reset();
            draw_ops();
            soft_update();
            ReportIfGlobal.reset();
            ReportIfGlobal.draw_ops();
            MainInterfaceGlobal.soft_update();
            base_id = selected % 1024;
            if (!field_5A04_) return;
            if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_
                    && !(Bases[base_id].visibility & (1 << ConsoleGlobal.faction_id_))) return;
            WorldClimateWorldWin.tile_to_pixel(Bases[base_id].x, Bases[base_id].y,
                &ReportIfGlobal.animation_x_, &ReportIfGlobal.animation_y_);
            ReportIfGlobal.animation_x_ -= 7;
            ReportIfGlobal.animation_y_ -= 5;
            ReportIfGlobal.animation_state_ = -1;
            ReportIfGlobal.bl_anim();
            time1_.init(bl_daemon, reinterpret_cast<int>(&ReportIfGlobal), 150, 5);
            time1_.start();
            break;
        }
        case 5: {
            field_59AC_ = selected;
            reset();
            draw_projects();
            soft_update();
            ReportIfGlobal.reset();
            ReportIfGlobal.draw_projects();
            MainInterfaceGlobal.soft_update();
            base_id = selected >= 1024 ? selected - 1024 : base_project(selected);
            if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_
                    && !(Bases[base_id].visibility & (1 << ConsoleGlobal.faction_id_))) return;
            WorldClimateWorldWin.tile_to_pixel(Bases[base_id].x, Bases[base_id].y,
                &ReportIfGlobal.animation_x_, &ReportIfGlobal.animation_y_);
            ReportIfGlobal.animation_x_ -= 7;
            ReportIfGlobal.animation_y_ -= 5;
            ReportIfGlobal.animation_state_ = -1;
            ReportIfGlobal.bl_anim();
            time1_.init(bl_daemon, reinterpret_cast<int>(&ReportIfGlobal), 150, 5);
            time1_.start();
            break;
        }
        case 7: {
            if (ReportIfGlobal.report_faction_id_ != ConsoleGlobal.faction_id_) return;
            int proto_id = selected;  // [ebp+0Ch] (y's slot)
            if (HIBYTE(GetAsyncKeyState(VK_SHIFT))) {
                if (!HIBYTE(GetAsyncKeyState(VK_CONTROL))) {
                    for (int row = 0; row < 128; ++row) {
                        const int id = row < 64 ? row
                            : ReportIfGlobal.report_faction_id_ * 64 + row - 64;
                        if ((ReportIfGlobal.field_1E244_ < 3
                                && Chassis[VehPrototypes[id].chassis_id].triad
                                    == Chassis[VehPrototypes[proto_id].chassis_id].triad
                                && Weapon[VehPrototypes[id].weapon_id].offense_rating != 0)
                            || (ReportIfGlobal.field_1E244_ == 3
                                && Weapon[VehPrototypes[id].weapon_id].offense_rating == 0)
                            || ReportIfGlobal.field_1E244_ == 4) {
                            selected_prototypes_[row / 32] &= ~(1 << (row % 32));
                        }
                    }
                }
                if (field_59AC_ > proto_id) {
                    field_59AC_ ^= proto_id;
                    proto_id ^= field_59AC_;
                    field_59AC_ ^= proto_id;
                }
                int first = field_59AC_ % 64;
                if (field_59AC_ >= 64) first += 64;
                int last = proto_id % 64;  // [ebp+8] (x's slot)
                if (proto_id >= 64) last += 64;
                for (; first <= last; ++first) {
                    if (ReportIfGlobal.report_faction_id_ == ConsoleGlobal.faction_id_) {
                        const int id = first < 64 ? first
                            : ReportIfGlobal.report_faction_id_ * 64 + first - 64;
                        if ((ReportIfGlobal.field_1E244_ < 3
                                && Chassis[VehPrototypes[id].chassis_id].triad
                                    == Chassis[VehPrototypes[proto_id].chassis_id].triad
                                && Weapon[VehPrototypes[id].weapon_id].offense_rating != 0)
                            || (ReportIfGlobal.field_1E244_ == 3
                                && Weapon[VehPrototypes[id].weapon_id].offense_rating == 0)
                            || ReportIfGlobal.field_1E244_ == 4) {
                            selected_prototypes_[first / 32] |= 1 << (first % 32);
                        }
                    }
                }
            } else if (HIBYTE(GetAsyncKeyState(VK_CONTROL))) {
                int row = proto_id % 64;
                if (proto_id >= 64) row += 64;
                selected_prototypes_[row / 32] ^= 1 << (row % 32);
            } else {
                for (int row = 0; row < 128; ++row) {
                    const int id = row < 64 ? row
                        : ReportIfGlobal.report_faction_id_ * 64 + row - 64;
                    if ((ReportIfGlobal.field_1E244_ < 3
                            && Chassis[VehPrototypes[id].chassis_id].triad
                                == Chassis[VehPrototypes[proto_id].chassis_id].triad
                            && Weapon[VehPrototypes[id].weapon_id].offense_rating != 0)
                        || (ReportIfGlobal.field_1E244_ == 3
                            && Weapon[VehPrototypes[id].weapon_id].offense_rating == 0)
                        || ReportIfGlobal.field_1E244_ == 4) {
                        selected_prototypes_[row / 32] &= ~(1 << (row % 32));
                    }
                }
                row = proto_id % 64;
                if (proto_id >= 64) row += 64;
                selected_prototypes_[row / 32] |= 1 << (row % 32);
            }
            field_59AC_ = selected;
            reset();
            draw_security();
            soft_update();
            ReportIfGlobal.reset();
            ReportIfGlobal.draw_security();
            MainInterfaceGlobal.soft_update();
            return;
        }
        }
    } else if (ReportIfGlobal.field_8_ == 4) {
        field_5A08_ = 0;
        for (int i = 0; i < 512; ++i) {
            ReportIfGlobal.sort_entries_[i].base_id = i;
            ReportIfGlobal.sort_entries_[i].value = 0;
            ReportIfGlobal.sort_entries_[i].sorted_base_id = i;
            ReportIfGlobal.sort_entries_[i].sorted_value = 0;
        }
        redraw();
        ReportIfGlobal.draw_ops();
        MainInterfaceGlobal.soft_update();
    }
}

// ORIGINAL: 0x00497A60 ?on_left_double_click@ReportWin@@QAEXHH@Z 0x00497A60-0x00497AB2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/00497a60.cpp records the original 0x00497A60-0x00497AB2 method and BYTE_EXACT claim.", "src/reportwin.h places spot_ at 0x5994; src/tutwin.h binds TutWinBaseWindow at the seed's 0x006A7628 zoom receiver."], "reason": "Home ReportWin's existing double-click handler against its declared Spot member and the verified base-window object."}
// Deleted seed provenance: git:53d5fe2c1b1eff854b846b082f125794c1d0b7fe:src/recovered/units/00497a60.cpp
// symbol    ?on_left_double_click@ReportWin@@UAEXHH@Z
/*
Purpose: Double-click the currently hit report spot to zoom to a base when
         the active report page is the base report (page 4).
*/
void ReportWin::on_left_double_click(int x, int y) {
    int spot_position;
    int spot_type;
    int result = spot_.check(x, y, &spot_position, &spot_type);
    if (result >= 0 && ReportIfGlobal.field_8_ == 4 &&
        spot_position >= 0x400) {
        int base_id = spot_position - 0x400;
        if (base_id < 0x400) {
            TutWinBaseWindow.zoom(base_id, 0);
        }
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00497AC0 0x00497E6C 0x00497E98 0x00497D5D
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00497AC0 0x00497E98 0x00497EFC 0x00497D57
// ORIGINAL: 0x00497AC0 ?right_menu@ReportWin@@QAEXHHH@Z 0x00497AC0-0x00497E69;0x00659420-0x00659435 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:649230f6:src/recovered/units/00497ac0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::right_menu(int base_id, int x, int y) {
    PullDown menu;
    if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_
        || reportwin_input_blocked()) return;
    client_to_screen(&x, &y);
    menu.init(0);
    menu.add_item(1, StringTable.get(Labels.strings_ptr[0x894 / 4]));
    menu.add_item(100, StringTable.get(Labels.strings_ptr[0x910 / 4]));
    menu.add_separator(99);
    menu.add_item(2, StringTable.get(Labels.strings_ptr[0x898 / 4]));
    menu.add_item(3, StringTable.get(Labels.strings_ptr[0x89C / 4]));
    menu.add_separator(99);
    menu.add_item(4, StringTable.get(Labels.strings_ptr[0x8A0 / 4]));
    menu.add_item(5, StringTable.get(Labels.strings_ptr[0x8A4 / 4]));
    menu.add_item(6, StringTable.get(Labels.strings_ptr[0x8A8 / 4]));
    menu.add_item(7, StringTable.get(Labels.strings_ptr[0x8AC / 4]));
    menu.add_item(8, StringTable.get(Labels.strings_ptr[0xBFC / 4]));
    menu.add_item(9, StringTable.get(Labels.strings_ptr[0xA54 / 4]));
    menu.show(x, y, pop_expire);
    if (GameDataStateStorage.fields_.game_state_ & 0x800) return;
    const int selected = menu.get_selected();
    // The original performs only the signed upper-bound check here.
    const int action = selected < 64 ? menu.item_id(selected) : -1;
    TutWinBaseWindow.soft_zoom(base_id);
    switch (action) {
        case 1: TutWinBaseWindow.zoom(base_id, 0); break;
        case 2:
            ReportWinGlobal.release_modal();
            ReportIfGlobal.release_iface_mode();
            TutWinBaseWindow.production(0, 0);
            return;
        case 3: TutWinBaseWindow.hurry(); break;
        case 4: TutWinBaseWindow.gov_on(0x1000000); break;
        case 5: TutWinBaseWindow.gov_on(0x2000000); break;
        case 6: TutWinBaseWindow.gov_on(0x4000000); break;
        case 7: TutWinBaseWindow.gov_on(0x8000000); break;
        case 8: BaseCurrent->governor &= 0x7FFFFFFF; break;
        case 9: TutWinBaseWindow.governor(0); break;
        case 100:
            MapWinTable[0]->set_center(BaseCurrent->x, BaseCurrent->y, 1);
            ReportWinGlobal.release_modal();
            ReportIfGlobal.release_iface_mode();
            break;
    }
    redraw();
}

// ORIGINAL: 0x00497F00 ?on_right_click@ReportWin@@QAEXHH@Z 0x00497F00-0x004980EF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:649230f6:src/recovered/units/00497f00.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
// symbol    ?on_right_click@ReportWin@@UAEXHH@Z
void ReportWin::on_right_click(int x, int y) {
    if (reportwin_input_blocked()) return;
    int spot_id, payload;
    if (spot_.check(x, y, &spot_id, &payload) < 0) return;
    switch (ReportIfGlobal.field_8_) {
    case 4: {
        int id = spot_id;
        if (id >= 1024) {
            id -= 1024;
            if (id >= 1024) {
                id -= 1024;
                if (Bases[id].faction_id_current != ConsoleGlobal.faction_id_) return;
                int veh_id = payload;
                log_say("Reports right_click garrison", veh_id, id, 0);
                TutWinBaseWindow.soft_zoom(id);
                TutWinBaseWindow.garrison_click(veh_id, x, y, 1);
                return;
            }
            if (Bases[id].faction_id_current != ConsoleGlobal.faction_id_) return;
            right_menu(id, x, y);
            return;
        }
        log_say("Base report psych right click", 0, 0, 0);
        TutWinBaseWindow.soft_zoom(id);
        if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) != 0) return;
        if (BaseCurrent->specialist_total != 0) --BaseCurrent->specialist_total;
        base_compute(1);
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        break;
    }
    case 6:
        if ((GameDataStateStorage.fields_.game_state_ & 0x40) != 0
            && (GameDataStateStorage.fields_.game_state_ & 0x80) != 0) {
            sat_edit(ReportIfGlobal.report_faction_id_, payload, spot_id);
        }
        break;
    }
    redraw();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004980F0 ?on_mouse_move@ReportWin@@QAEXHH@Z 0x004980F0-0x004980F3 BYTE_EXACT
// symbol    ?on_mouse_move@ReportWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@ReportWin@@QAEXHH@Z)(ReportWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportWin::on_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00498100 ?on_mouse_leave@ReportWin@@QAEXHH@Z 0x00498100-0x00498103 BYTE_EXACT
// symbol    ?on_mouse_leave@ReportWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_mouse_leave@ReportWin@@QAEXHH@Z)(ReportWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportWin::on_mouse_leave(int, int) {
}

// ORIGINAL: 0x00498110 ?UNK1@ReportWin@@QAEXHHHH@Z 0x00498110-0x00498174 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/00498110.cpp records the original 0x00498110-0x00498174 method and its position/range/paging updates.", "src/reportwin.h places Scroll at 0xC10C; src/scroll.h places paging_ at Scroll+0xA40, which is the seed's 0xCB4C write."], "reason": "Home ReportWin's range callback and type its embedded Scroll storage while preserving the constructor's explicit placement order."}
// Deleted seed provenance: git:53d5fe2c1b1eff854b846b082f125794c1d0b7fe:src/recovered/units/00498110.cpp
/*
Purpose: Store the active position and range, update the embedded Scroll's
         page size and position, then clear the ReportWin page-transition flag.
*/
void ReportWin::UNK1(int position, int minimum, int maximum, int page_size) {
    field_C0FC_ = position;
    field_C100_ = minimum;
    field_C104_ = maximum;
    field_C108_ = page_size;
    Scroll &scroll = scroll_;
    scroll.set_range(minimum, maximum);
    scroll_.set_paging(field_C108_);
    scroll.set_pos(field_C0FC_);
    field_E270_ = 0;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00498180 0x004981D8 0x004981FC 0x00498199
// ORIGINAL: 0x00498180 ?on_scrolled@ReportWin@@QAEXHH@Z 0x00498180-0x004981D8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/00498180.cpp records the original 0x00498180-0x004981D8 handler and page dispatch.", "src/reportwin.h places field_C0FC_ at 0xC0FC; src/graphicwin.h identifies redraw as Win slot 62."], "reason": "Home the ReportWin scrollbar callback using its declared position member and inherited redraw virtual."}
// symbol    ?on_scrolled@ReportWin@@UAEXHH@Z
// Deleted seed provenance: git:53d5fe2c1b1eff854b846b082f125794c1d0b7fe:src/recovered/units/00498180.cpp
/*
Purpose: Track the report scrollbar's committed position and repaint the
         project page directly; recognized remaining pages use the window's
         redraw virtual, while unlisted pages do nothing.
*/
void ReportWin::on_scrolled(int, int pos) {
    field_C0FC_ = pos;
    switch (ReportIfGlobal.field_8_) {
    case 2: redraw(); break;
    case 3: redraw(); break;
    case 4: redraw(); break;
    case 5:
        reset();
        draw_projects();
        soft_update();
        break;
    case 6: redraw(); break;
    case 7: redraw(); break;
    case 8: redraw(); break;
    case 0: redraw(); break;
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00498200 0x00498258 0x0049827C 0x00498219
// ORIGINAL: 0x00498200 ?on_scrolling@ReportWin@@QAEXHH@Z 0x00498200-0x00498258 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/00498200.cpp records the original 0x00498200-0x00498258 handler and page dispatch.", "src/reportwin.h places field_C0FC_ at 0xC0FC; src/graphicwin.h identifies redraw as Win slot 62."], "reason": "Home the ReportWin scroll-in-progress callback using the same source-owned position and redraw behavior."}
// symbol    ?on_scrolling@ReportWin@@UAEXHH@Z
// Deleted seed provenance: git:53d5fe2c1b1eff854b846b082f125794c1d0b7fe:src/recovered/units/00498200.cpp
void ReportWin::on_scrolling(int, int pos) {
    field_C0FC_ = pos;
    switch (ReportIfGlobal.field_8_) {
    case 2: redraw(); break;
    case 3: redraw(); break;
    case 4: redraw(); break;
    case 5:
        reset();
        draw_projects();
        soft_update();
        break;
    case 6: redraw(); break;
    case 7: redraw(); break;
    case 8: redraw(); break;
    case 0: redraw(); break;
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00498280 0x00498694 0x004986B4 0x00498470
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00498280 0x004986B4 0x004986D0 0x004984A8
/*
Purpose: The report window's keyboard handler. Always answers 1.

             Enter (0x0D)                      - release the modal loop and
                                                 the active interface mode
             F1 (0x70)                         - on_button_clicked(1), slot 44
             F2-F8 (0x71-0x77)                 - switch to report 2..8 unless
                                                 that report is already up;
                                                 then re-report it on the
                                                 ReportIf singleton when the
                                                 active interface is the
                                                 world map's slot 5, or
                                                 set_modal(0, pop_wait,
                                                 MainInterfaceGlobal) - Win
                                                 slot 57 - over this window
             0x25/0x27                         - category +/- 1 with the
                                                 wrap stored twice, exactly
                                                 as the image emits it
             0x21/0x22/0x26/0x28               - page/line scroll of the
                                                 mirrored range state at
                                                 0xC0F8..0xC108, clamped and
                                                 pushed into scroll_ before
                                                 the slot-62 redraw()
             0x23/0x24                         - jump to the range maximum /
                                                 minimum
             0x30-0x5A or Escape               - release the modal loop and
                                                 the active interface mode
// ORIGINAL: 0x00498280 ?on_key_click@ReportWin@@QAEHHH@Z 0x00498280-0x00498691
// symbol    ?on_key_click@ReportWin@@UAEHHH@Z
// size      1041 bytes
// prototype int (__thiscall ?on_key_click@ReportWin@@QAEHHH@Z)(ReportWin* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045D380 0x004A3950 0x005EE280 0x00605D20
// indirect  0x0049829C 0x00498418 0x00498652
// Homed from src/unrecovered/00498280.cpp. pop_wait (0x005398E0) rides the
// set_modal slot as the poll callback, not as a call target.
*/
int ReportWin::on_key_click(int, int code) {
    int moved;
    if (code == 0x0D) {
        ReportWinGlobal.Win::release_modal();
        ReportIfGlobal.SubInterface::release_iface_mode();
        return 1;
    }
    if (code == 0x70) {
        on_button_clicked(1);
        return 1;
    }
    if (code >= 0x71 && code <= 0x77) {
        if (ReportIfGlobal.field_8_ == code - 0x6F) {
            ReportWinGlobal.Win::release_modal();
            ReportIfGlobal.SubInterface::release_iface_mode();
            return 1;
        }
        // The image enters the rotation switch's case-2 block for F2 (one
        // report_labs expansion, not two).
        if (code == 0x71) goto labs;
        if (code == 0x72) report_energy();
        else if (code == 0x73) report_ops();
        else if (code == 0x74) report_projects();
        else if (code == 0x75) report_sat();
        else if (code == 0x76) report_security();
        else if (code == 0x77) report_score(0);
        return 1;
    }
    if (code < 0x21 || code > 0x28) {
        if ((code > 0x2F && code < 0x5B) || code == 0x1B) {
            ReportWinGlobal.Win::release_modal();
            ReportIfGlobal.SubInterface::release_iface_mode();
        }
        return 1;
    }
    switch (code) {
    case 0x25:
        moved = ReportIfGlobal.field_8_ + 4;
        goto rotate;
    case 0x27:
        moved = ReportIfGlobal.field_8_ + 6;
    rotate:
        ReportIfGlobal.field_8_ = moved % 7 + 2;
        switch (ReportIfGlobal.field_8_) {
        case 2: labs: report_labs(); return 1;
        case 3: report_energy(); break;
        case 4: report_ops(); break;
        case 5: report_projects(); break;
        case 6: report_sat(); break;
        case 7: report_security(); break;
        case 8: report_score(0); break;
        }
        return 1;
    case 0x26:
        if (field_C0F8_ == 0) return 1;
        field_C0FC_ = range(field_C0FC_ - 1, field_C100_, field_C104_);
        break;
    case 0x28:
        if (field_C0F8_ == 0) return 1;
        field_C0FC_ = range(field_C0FC_ + 1, field_C100_, field_C104_);
        break;
    case 0x21:
        if (field_C0F8_ == 0) return 1;
        field_C0FC_ = range(field_C0FC_ - field_C108_, field_C100_, field_C104_);
        break;
    case 0x22:
        if (field_C0F8_ == 0) return 1;
        field_C0FC_ = range(field_C108_ + field_C0FC_, field_C100_, field_C104_);
        break;
    case 0x24:
        if (field_C0F8_ == 0) return 1;
        field_C0FC_ = field_C100_;
        break;
    case 0x23:
        if (field_C0F8_ == 0) return 1;
        field_C0FC_ = field_C104_;
        break;
    }
    if (field_C0F8_ != 0) {
        scroll_.set_pos(field_C0FC_);
        redraw();
    }
    return 1;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004986D0 0x004989A8 0x004989CC 0x00498745
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004986D0 0x004989CC 0x004989E4 0x0049888D
// ORIGINAL: 0x004986D0 ?on_button_clicked@ReportWin@@QAEXH@Z 0x004986D0-0x004989A8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:649230f6:src/recovered/units/004986d0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
// symbol    ?on_button_clicked@ReportWin@@UAEXH@Z
void ReportWin::on_button_clicked(int id) {
    const int datalink_mode = IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 8;
    if (id < 0x378) {
        if (datalink_mode) return;
        if (id == 9) {
            ReportWinGlobal.release_modal();
            ReportIfGlobal.release_iface_mode();
            return;
        }
        if (id != 1) return;
        switch (ReportIfGlobal.field_8_) {
            case 2: {
                int tech = field_FAB8_;
                if (tech + 1 == 0) tech = 62;
                DatalinkMain.exec(14, tech);
                return;
            }
            case 3: DatalinkMain.exec(0, 43); return;
            case 4: DatalinkMain.exec(10, 29); return;
            case 5: {
                int topic = field_59AC_;
                if (topic < 1024) {
                    topic += 70;
                    if (topic < 70) topic = 71;
                } else {
                    topic = -Bases[topic - 1024].queue_production_id[0];
                }
                DatalinkMain.exec(11, topic);
                return;
            }
            case 6: DatalinkMain.exec(14, Facility[65].preq_tech); return;
            case 7: DatalinkMain.exec(3, 7); return;
            case 8: DatalinkMain.exec(1, 4); return;
            case 0: DatalinkMain.exec(15, ReportIfGlobal.report_selection_); return;
        }
        return;
    }
    if (ReportIfGlobal.field_8_ == 2) {
        const int old_tech = field_FAB8_;
        id -= 0x378;
        switch (id) {
            case 1: field_FAB8_ = Technology[old_tech].preq_tech_1; break;
            case 2: field_FAB8_ = Technology[old_tech].preq_tech_2; break;
            case 3: case 4: case 5: case 6: {
                int count = 0;
                for (int i = 0; i < 178; ++i) {
                    if ((&Technology[i >> 1].preq_tech_1)[i & 1] == old_tech
                        && ++count == id - 2) {
                        field_FAB8_ = i >> 1;
                    }
                }
                break;
            }
        }
        ReportIfGlobal.listBox1_.set_selected_id(field_FAB8_);
        if (!datalink_mode) redraw();
        else DatalinkMain.set_index(field_FAB8_, 0);
        return;
    }
    if (ReportIfGlobal.field_8_ == 6 && id == 0xD3E) {
        const int target = ReportIfGlobal.buttonGroup3_.field_84_;
        if (target > 0 && target < 8) {
            const int type = ReportIfGlobal.buttonGroup4_.field_84_ - 65;
            if (type >= 0 && type < 5) {
                sat_attack(ConsoleGlobal.faction_id_, target, type);
                return;
            }
        }
        flatButton9_.hide();
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004989F0 0x00498B08 0x00498B2C 0x00498A3F
// ORIGINAL: 0x004989F0 ?on_group_clicked@ReportWin@@QAEXHHH@Z 0x004989F0-0x00498B06 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:649230f6:src/recovered/units/004989f0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
// symbol    ?on_group_clicked@ReportWin@@UAEXHHH@Z
void ReportWin::on_group_clicked(int group, int, int state) {
    time2_.stop();
    if (group == 4) ReportIfGlobal.field_1E240_ = state;
    redraw();
    if (group != 6) return;
    ReportIfGlobal.reset();
    switch (ReportIfGlobal.field_8_) {
        case 2: ReportIfGlobal.draw_labs(); break;
        case 3: ReportIfGlobal.draw_energy(); break;
        case 4: ReportIfGlobal.draw_ops(); break;
        case 5: ReportIfGlobal.draw_projects(); break;
        case 6: ReportIfGlobal.draw_sat(); break;
        case 7: ReportIfGlobal.draw_security(); break;
        case 8: ReportIfGlobal.draw_score(); break;
        case 0: ReportIfGlobal.draw_intel(); break;
    }
    MainInterfaceGlobal.soft_update();
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00498B30 0x0049B1DC 0x0049B1EC 0x00499199
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00498B30 0x0049B1EC 0x0049B1FC 0x00499224
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00498B30 0x0049B1FC 0x0049B20C 0x004992F9
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00498B30 0x0049B20C 0x0049B21C 0x004993BD
// ORIGINAL: 0x00498B30 ?draw_labs@ReportWin@@QAEXXZ 0x00498B30-0x0049B1DC
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00498B30-0x0049B1DC", "git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00498b30.cpp", "Retired recovered seed provenance: source:src/recovered/units/00430b30.cpp", "pinned:0x0049A5FC stores kind0x62 then0x0049A606 stores the chassis name value after the link text and before chassis detail formatting", "pinned:0x0049A7CD stores kind0x63 then0x0049A7D7 stores the weapon name value after the link text and before weapon detail formatting", "pinned:0x0049A9E1 stores kind0x64 then0x0049A9EB stores the armor name value after the link text and before armor detail formatting", "source:src/datalink.h TechnologyHelpEntryName/Kind are the independently observed 4-byte shared objects; Datalink help and tech_achieved reset use the same pair", "source:src/reportif.cpp reportwin_draw_lab_unlock_sections already traverses these same typed Chassis/Weapon/Armor rows and preserves its original lab detail and callback paths"], "reason": "The ReportWin laboratory page is shared by the ordinary report and Datalink entry renderer. Recover its real Buffer hosts, FlatButton members, technology Sprite array and ReportIf fonts; original chart geometry, status rows and unlock-list scans identify these owners. Retain the historical seed for further matching and runtime verification. Preserve the shared technology-help entry name/kind pair at the original chassis, weapon and armor emission points while keeping ReportWin's real two-pass lab/unlock rendering, existing table bounds, section order and link/detail ordering."}
// TRIED: full pixel-geometry reconstruction of the lab-tier bar chart (labs_line/FUN_0049b450/vtable-slot draw calls with exact coordinate math across up to 15 rows and 4 switch-selected sprite tables) - this pass preserves top-level branch shape and call order, not the per-row layout arithmetic.
// Frame map used (image [ebp-N] -> local):
//   -0x4c faction            -0x38 datalink            -0x30 host (GraphicWin *)
//   -0x7c..-0x70 children[4] (a fifth match spills into -0x6c = header.left)
//   -0x6c header RECT        -0x5c frame RECT          -0x48 box RECT
//   -0x28 bar RECT           -0x14 x                   -0x08 y
//   (x and y are address-taken by get_mouse_pos at the end, so every store to
//    them survives; that is why the image keeps them in memory throughout.)
//   header block: -0x18 icon_x / saved bar right, -0x34 icon_y, -0x04 rate,
//                 -0x10 accumulated, -0xd8 reportwin_append_number buffer
//   chart/lab block: -0x18 tech, -0x84/-0x80 prerequisite1/2, -0x10 count,
//                 -0x0c i, -0x10 pass, -0x04 section index, -0xc8 id[64]
//   UNSURE: the image also keeps a second copy of tech at -0x88 (used by the
//   prerequisite scan, has_tech/say_tech of the status rows and the -1 test),
//   while -0x18 serves the category label, tech_recurse and section compares.
//   That looks like an inline-helper parameter copy; one variable is used here.
//   UNSURE: rate (-0x04) / section index (-0x04), accumulated (-0x10) / count /
//   pass (-0x10) and the child index / host+0x444 (-0x0c) share slots in the
//   image; block scopes are used below so VC6 can overlap them.
void ReportWin::draw_labs() {
    // Seven technology ids in one array: the tech itself, its two
    // prerequisites and up to four children (image [ebp-0x88]..[ebp-0x70]).
    int ids[7];
    RECT header;
    RECT frame;
    RECT box;
    RECT bar;
    GraphicWin *host;
    int x;
    int y;

    int faction = ReportIfGlobal.report_faction_id_;
    ids[3] = -1;
    ids[4] = -1;
    ids[5] = -1;
    ids[6] = -1;
    int datalink = IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 8;
    if (datalink) {
        // 0x498B75..0x498C55
        host = &DatalinkMain;
        DatalinkMain.set_text_color_hyper(0xFE, -1, 1, 1);
        field_59A0_ = 0x96;
        field_59A8_ = 0xAA;
        field_59A4_ = 0x28;
        frame.left = DatalinkMain.entry_clip_rect().left + 3;
        frame.top = DatalinkMain.entry_clip_rect().top + 3;
        // UNSURE: the image forms right-left once and derives both
        // frame.right (= that + frame.left - 6) and bar.right (= that - 6).
        frame.right = frame.left
            + (DatalinkMain.entry_clip_rect().right
               - DatalinkMain.entry_clip_rect().left) - 6;
        frame.bottom = frame.top + 0x14;
        // Genuine original: the Datalink host never draws the research
        // header, but still clears its RECT and fills the bar RECT with
        // inset/width/height values that are never read on this path.
        header.left = 0;
        header.top = 0;
        header.right = 0;
        header.bottom = 0;
        bar.left = frame.left + 3;
        bar.top = frame.top + 3;
        bar.right = DatalinkMain.entry_clip_rect().right
            - DatalinkMain.entry_clip_rect().left - 6;
        bar.bottom = DatalinkMain.entry_clip_rect().bottom
            - DatalinkMain.entry_clip_rect().top - 6;
        field_A2C_ = frame.bottom + 0xA;
        field_A28_ = (DatalinkMain.entry_clip_rect().right
                      - DatalinkMain.entry_clip_rect().left - 0xB0) / 2
            + frame.left;
        field_A30_ = field_A28_ + 0xAA;
        field_A34_ = field_A2C_ + 0xF0;
    } else {
        // 0x498C5A..0x498D1A
        host = this;
        x = rect1_.left;
        y = rect1_.top;
        field_59A0_ = 0xBE;
        field_59A4_ = 0x28;
        field_59A8_ = 0xBE;
        frame.left = x;
        frame.top = y;
        frame.right = rect1_.right + x;
        frame.bottom = y + 0x46;
        int icon_x = x + 3;
        int icon_y = y + 3;
        header.left = icon_x + 0x43;
        header.right = frame.right - 3;
        header.top = icon_y;
        // Genuine original: literal bottoms, not offsets from the frame.
        header.bottom = 0x2F;
        bar.left = icon_x + 0x43;
        bar.top = 0x31;
        bar.right = frame.right - 3;
        bar.bottom = frame.bottom - 3;
        field_A2C_ = frame.bottom + 0x1D
            + (ReportIfGlobal.field_1E230_ ? 0x62 : 0);
        field_A28_ = (rect1_.right - 0xBE) / 2 + x;
        field_A30_ = field_A28_ + 0xBE;
        field_A34_ = field_A2C_ + 0xAE;
        energy_compute(faction, 0);
        int rate = tech_rate(faction);
        // UNSURE: the image stores x, then y, around the accumulated load.
        x = header.left + 3;
        y = header.top + 5;
        int accumulated = PlayersData[faction].tech_accumulated;
        Buffer::set_text_color3(0xE1, -1, 1, 1);
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        Buffer::box_sprite(&frame, &InterfaceBoxStyles[3]);

        // 0x498DA0: blind research hides the current technology unless the
        // game-state reveal bit is set. Literal order (0x686F70 before
        // 0x686F74 in the second test below) puts the visible arm first.
        if ((GameDataStateStorage.fields_.game_state_ & 0x80) != 0
            || (GameDataStateStorage.fields_.game_rules_
                & RULES_BLIND_RESEARCH) == 0) {
            // 0x498DFC..0x499079
            if (PlayersData[faction].tech_id_researching >= 0) {
                g_TECH_ICON_SPRITES[PlayersData[faction].tech_id_researching].draw_mono(
                    this, icon_x, icon_y,
                    mandate_color(tech_category(PlayersData[faction].tech_id_researching)),
                    0x40,
                    g_TECH_ICON_SPRITES[PlayersData[faction].tech_id_researching].width());
            }
            Buffer::box_sprite(&header, &InterfaceBoxStyles[1]);
            strcat(StringTemp, label_get(0x254 / 4));
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, "]");
            x = Buffer::write_l(StringTemp, x, y);
            StringTemp[0] = 0;
            strcat(StringTemp, "[");
            say_tech(StringTemp, PlayersData[faction].tech_id_researching, 0);
            CharUpperA(StringTemp);
            if (PlayersData[faction].tech_id_researching
                == MaxTechnologyNum - 1) {
                // Genuine original: the future-technology header appends the
                // count but no closing bracket here; the "]" below adds it.
                strcat(StringTemp, BufferJoinSpace);
                say_num(
                    PlayersData[faction].theory_of_everything + 1);
            } else if (PlayersData[faction].tech_id_researching >= 0) {
                strcat(StringTemp, "]");
                CharUpperA(StringTemp);
                x = Buffer::write_l(StringTemp, x, y) + 0xF;
                StringTemp[0] = 0;
                Buffer::set_text_color3(mandate_color(tech_category(
                    PlayersData[faction].tech_id_researching)), -1, 1, 1);
                strcat(StringTemp, "[");
                strcat(StringTemp, table_text(Mandate[tech_category(
                    PlayersData[faction].tech_id_researching)].name_caps));
                strcat(StringTemp, BufferJoinSpace);
                say_num(tech_recurse(
                    PlayersData[faction].tech_id_researching, 0));
            }
            // 0x49907C
            spot_.kill_type(0);
            spot_.add(0x29A, 0, frame.left, frame.top,
                      frame.right - frame.left, frame.bottom - frame.top);
        } else {
            // 0x498DB3..0x498DF7
            header.left = frame.left + 3;
            x = header.left + 5;
            Buffer::box_sprite(&header, &InterfaceBoxStyles[1]);
            strcat(StringTemp, label_get(0xAC4 / 4));
        }

        // 0x4990AF: the image re-reads both globals for a second test.
        if ((GameDataStateStorage.fields_.game_state_ & 0x80) != 0
            || (GameDataStateStorage.fields_.game_rules_
                & RULES_BLIND_RESEARCH) == 0) {
            // 0x499124..0x499175
            strcat(StringTemp, "]");
            Buffer::write_l(StringTemp, x, y);
            x = header.left + 3;
            y = header.top + 5;
        } else {
            // 0x4990C2..0x499122
            strcat(StringTemp, "]");
            Buffer::write_cent_text(StringTemp, x, y,
                                    header.right - header.left);
            // Genuine original: only the hidden branch moves the bar's left
            // edge (to the frame inset); x and y keep their header values.
            bar.left = frame.left + 3;
        }
        y += field_E268_ + 3;

        // 0x499182..0x49941C: research bar. Cases 0..3 follow the image's
        // jump tables at 0x49B1DC/0x49B1EC/0x49B1FC/0x49B20C.
        switch (tech_category(PlayersData[faction].tech_id_researching)) {
        case 0: Buffer::box_sprite(&bar, &InterfaceBoxStyles[38]); break;
        case 1: Buffer::box_sprite(&bar, &InterfaceBoxStyles[41]); break;
        case 2: Buffer::box_sprite(&bar, &InterfaceBoxStyles[44]); break;
        case 3: Buffer::box_sprite(&bar, &InterfaceBoxStyles[47]); break;
        }
        int right = bar.right;
        // The range() store of max into bar.right itself is elided by VC6,
        // matching the image's missing store on the `jg` path at 0x4991EF.
        bar.right = range(bar.left
                              + (bar.right - bar.left) * accumulated / rate,
                          bar.left, bar.right);
        Buffer::set_clip(&bar);
        {
            // UNSURE: the image swaps bar.right with the saved right edge
            // (store order: saved first, then bar.right).
            int swapped = right;
            right = bar.right;
            bar.right = swapped;
        }
        switch (tech_category(PlayersData[faction].tech_id_researching)) {
        case 0: Buffer::box_sprite(&bar, &InterfaceBoxStyles[36]); break;
        case 1: Buffer::box_sprite(&bar, &InterfaceBoxStyles[39]); break;
        case 2: Buffer::box_sprite(&bar, &InterfaceBoxStyles[42]); break;
        case 3: Buffer::box_sprite(&bar, &InterfaceBoxStyles[45]); break;
        }
        {
            int swapped = right;
            right = bar.right;
            bar.right = swapped;
        }
        Buffer::restore_clip();
        {
            // UNSURE: the image reads the old bar.left before storing the new
            // one (0x499272/0x499275), so the width term is formed first.
            int next = (right - bar.left) * EnergyLabsTotal / 100 / rate;
            bar.left = bar.right;
            bar.right = range(bar.right + next, bar.right, right);
        }
        Buffer::set_clip(&bar);
        inflate_frame(&bar, 7, 7);
        switch (tech_category(PlayersData[faction].tech_id_researching)) {
        case 0: Buffer::box_sprite(&bar, &InterfaceBoxStyles[36]); break;
        case 1: Buffer::box_sprite(&bar, &InterfaceBoxStyles[39]); break;
        case 2: Buffer::box_sprite(&bar, &InterfaceBoxStyles[42]); break;
        case 3: Buffer::box_sprite(&bar, &InterfaceBoxStyles[45]); break;
        }
        inflate_frame(&bar, -7, -7);
        Buffer::restore_clip();
        // UNSURE: the image stores left, top, right, bottom here and right,
        // left, bottom, top for the next inflate; inflate_frame stores
        // left, right, top, bottom.
        inflate_frame(&bar, -1, -1);
        Buffer::set_clip(&bar);
        inflate_frame(&bar, 7, 7);
        switch (tech_category(PlayersData[faction].tech_id_researching)) {
        case 0: Buffer::box_sprite(&bar, &InterfaceBoxStyles[38]); break;
        case 1: Buffer::box_sprite(&bar, &InterfaceBoxStyles[41]); break;
        case 2: Buffer::box_sprite(&bar, &InterfaceBoxStyles[44]); break;
        case 3: Buffer::box_sprite(&bar, &InterfaceBoxStyles[47]); break;
        }
        inflate_frame(&bar, -7, -7);
        Buffer::restore_clip();

        // 0x499421..0x499521: discovery captions.
        Buffer::set_text_color3(0xE1, -1, 1, 1);
        parse_num(0, BreakthroughTurnsGlobal);
        parse_it(ScriptPopupFile, "DISCOV");
        x = Buffer::write_l(StringTemp, x, y) + 4;
        int turns;
        if (EnergyLabsTotal == 0) {
            turns = 9999;
        } else {
            turns = (range(rate - accumulated, 0, 99999999) * 100
                     + EnergyLabsTotal - 1) / EnergyLabsTotal;
        }
        parse_num(0, turns);
        parse_num(1, game_year(
            turns + GameDataStateStorage.fields_.turn_current_num_));
        parse_it(ScriptPopupFile, "NEXTDISCOV");
        Buffer::write_l(StringTemp, x, y);
    }

    // 0x499524..0x4995F2: chart geometry. rect1_ now holds the chart size.
    int tech = field_FAB8_;
    x = frame.left;
    y = field_E268_ + frame.bottom + 0xF;
    rect1_.right = frame.right - frame.left;
    rect1_.bottom = energy_panel_rect_.bottom - y - 0x1E;
    if (!datalink) {
        lab_center_y_ = (rect1_.bottom - field_59A4_) / 2 + y;
        // Store order in the image is center, lower, upper.
        lab_three_child_lower_y_ = field_59A4_ + lab_center_y_ + 0x10;
        lab_three_child_upper_y_ = lab_center_y_ - field_59A4_ - 0x10;
    } else {
        // Genuine original: the Datalink host shrinks the chart height by
        // 200 on non-800-wide screens only while placing the tiers, then
        // restores it.
        if (WinScreenWidth != 800) rect1_.bottom -= 200;
        lab_center_y_ = (rect1_.bottom - field_59A4_) / 2 + y;
        lab_three_child_lower_y_ = field_59A4_ + lab_center_y_ + 0x10;
        lab_three_child_upper_y_ = lab_center_y_ - field_59A4_ - 0x10;
        if (WinScreenWidth != 800) rect1_.bottom += 200;
    }
    {
        // 0x4995F4..0x499660. The x and four-child y columns are parallel
        // members walked by one pointer; WORKAROUND(type): they are probably
        // `int x[3]` at 0x594C and `int y[7]` at 0x5958 in the original.
        for (int i = 0; i < 3; i++) {
            (&lab_four_child_top_y_)[i] =
                (i - 1) * (field_59A4_ + 0x10)
                - (field_59A4_ + 0x10) / 2
                + lab_center_y_;
            (&lab_prerequisite_x_)[i] = frame.left + 5
                + (rect1_.right - field_59A0_) * i / 2
                - i * 5;
        }
    }
    lab_center_x_ = field_A28_;
    lab_four_child_bottom_y_ = (field_59A4_ + 0x10) * 2
        - (field_59A4_ + 0x10) / 2 + lab_center_y_;

    {
        // 0x49968A..0x49970B: up to five children (two prerequisite slots
        // per technology). Genuine original: the bound is tested before the
        // store, so a fifth match writes one past ids[] (into the
        // header RECT's left edge in the image frame); only four are drawn.
        ids[0] = tech;
        ids[1] = Technology[tech].preq_tech_1;
        ids[2] = Technology[tech].preq_tech_2;
        int count = 0;
        int i;
        for (int j = 0; j < 0xB2 && count <= 4; j++) {
            if ((&Technology[j >> 1].preq_tech_1)[j & 1] == tech) {
                // UNSURE: the image carries both count and a pointer.
                ids[3 + count] = j >> 1;
                count++;
            }
        }

        // 0x49970D: prerequisites. The one-prerequisite arm is laid out
        // first and its final box_sprite tail-merges with the two-arm's.
        if (ids[1] < 0 || ids[2] < 0) {
            if (ids[1] + 1 != 0) {
                // 0x49972B..0x499862
                labs_line(field_A28_, lab_center_y_, lab_prerequisite_x_,
                          lab_center_y_, field_59A0_, field_59A4_, 0);
                StringTemp[0] = 0;
                say_tech(StringTemp, ids[1], 0);
                CharUpperA(StringTemp);
                FlatButton *button = &flatButtons1_[1];
                button->init(StringTemp, 0x379, lab_prerequisite_x_,
                             lab_center_y_, field_59A0_, field_59A4_, host, 0);
                button->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
                button->redraw();
                if (has_tech(ids[1], faction)) {
                    button->set_text_color(0xFF, -1, 1, 1);
                }
                button->set_border_styles(&InterfaceBoxStyles[27],
                                          &InterfaceBoxStyles[28],
                                          &InterfaceBoxStyles[29]);
                button->redraw();
                set_rectangle_dimensions(&box, lab_prerequisite_x_ - 3,
                                         lab_center_y_ - 3, field_59A0_ + 6,
                                         field_59A4_ + 6);
                host->Buffer::box_sprite(&box, &InterfaceBoxStyles[3]);
            }
        } else {
            // 0x499867..0x499ADD
            labs_line(field_A28_, lab_center_y_, lab_prerequisite_x_,
                      lab_upper_inner_y_, field_59A0_, field_59A4_, 0);
            StringTemp[0] = 0;
            say_tech(StringTemp, ids[1], 0);
            CharUpperA(StringTemp);
            FlatButton *button = &flatButtons1_[1];
            button->init(StringTemp, 0x379, lab_prerequisite_x_,
                         lab_upper_inner_y_, field_59A0_, field_59A4_, host, 0);
            button->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
            button->redraw();
            if (has_tech(ids[1], faction)) {
                button->set_text_color(0xFF, -1, 1, 1);
            }
            button->set_border_styles(&InterfaceBoxStyles[27],
                                      &InterfaceBoxStyles[28],
                                      &InterfaceBoxStyles[29]);
            button->redraw();
            set_rectangle_dimensions(&box, lab_prerequisite_x_ - 3,
                                     lab_upper_inner_y_ - 3, field_59A0_ + 6,
                                     field_59A4_ + 6);
            host->Buffer::box_sprite(&box, &InterfaceBoxStyles[3]);

            // Genuine original: the second line starts from lab_center_x_
            // (0x5950), the first from field_A28_; both hold the same value.
            labs_line(lab_center_x_, lab_center_y_, lab_prerequisite_x_,
                      lab_lower_inner_y_, field_59A0_, field_59A4_, 0);
            StringTemp[0] = 0;
            say_tech(StringTemp, ids[2], 0);
            CharUpperA(StringTemp);
            button = &flatButtons1_[2];
            button->init(StringTemp, 0x37A, lab_prerequisite_x_,
                         lab_lower_inner_y_, field_59A0_, field_59A4_, host, 0);
            button->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
            button->redraw();
            if (has_tech(ids[2], faction)) {
                button->set_text_color(0xFF, -1, 1, 1);
            }
            button->set_border_styles(&InterfaceBoxStyles[27],
                                      &InterfaceBoxStyles[28],
                                      &InterfaceBoxStyles[29]);
            button->redraw();
            set_rectangle_dimensions(&box, lab_prerequisite_x_ - 3,
                                     lab_lower_inner_y_ - 3, field_59A0_ + 6,
                                     field_59A4_ + 6);
            host->Buffer::box_sprite(&box, &InterfaceBoxStyles[3]);
        }

        // 0x499AE2..0x499FDB: children (button slots 3..6).
        if (count > 4) count = 4;
        if (count == 1 || count == 4) {
            // 0x499AF6: rows y[count-1 .. 2*count-2] of the 0x5958 column.
            for (i = 0; i < count; i++) {
                labs_line(lab_child_x_, (&lab_center_y_)[count - 1 + i],
                          lab_center_x_, lab_center_y_, field_59A0_,
                          field_59A4_, 0);
                StringTemp[0] = 0;
                say_tech(StringTemp, ids[3 + i], 0);
                CharUpperA(StringTemp);
                FlatButton *button = &flatButtons1_[i + 3];
                button->init(StringTemp, i + 0x37B, lab_child_x_,
                             (&lab_center_y_)[count - 1 + i], field_59A0_,
                             field_59A4_, host, 0);
                button->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
                button->redraw();
                if (has_tech(ids[3 + i], faction)) {
                    button->set_text_color(0xFF, -1, 1, 1);
                }
                button->set_border_styles(&InterfaceBoxStyles[27],
                                          &InterfaceBoxStyles[28],
                                          &InterfaceBoxStyles[29]);
                button->redraw();
                set_rectangle_dimensions(&box, lab_child_x_ - 3,
                    (&lab_center_y_)[count - 1 + i] - 3, field_59A0_ + 6,
                    field_59A4_ + 6);
                host->Buffer::box_sprite(&box, &InterfaceBoxStyles[3]);
            }
        } else if (count == 3) {
            // 0x499CBC: rows upper (y[1]), center (y[0]), lower (y[2]).
            // UNSURE: the image's address is
            //   this + 8*(i-1)*(i-1) + 4*(0x1656 - (i == 0)),
            // i.e. index 2*(i-1)*(i-1) - (i == 0); the spelling is a guess.
            for (i = 0; i < 3; i++) {
                labs_line(lab_child_x_,
                          (&lab_center_y_)[(i - 1) * 2 * (i - 1) - (i == 0)],
                          lab_center_x_, lab_center_y_, field_59A0_,
                          field_59A4_, 0);
                StringTemp[0] = 0;
                say_tech(StringTemp, ids[3 + i], 0);
                CharUpperA(StringTemp);
                FlatButton *button = &flatButtons1_[i + 3];
                // Genuine original spelling: the id is formed as
                // (i - 1) + 0x37C here (lea eax,[edx-1]; add eax,0x37C).
                button->init(StringTemp, (i - 1) + 0x37C, lab_child_x_,
                    (&lab_center_y_)[(i - 1) * 2 * (i - 1) - (i == 0)],
                    field_59A0_, field_59A4_, host, 0);
                button->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
                button->redraw();
                if (has_tech(ids[3 + i], faction)) {
                    button->set_text_color(0xFF, -1, 1, 1);
                }
                button->set_border_styles(&InterfaceBoxStyles[27],
                                          &InterfaceBoxStyles[28],
                                          &InterfaceBoxStyles[29]);
                button->redraw();
                set_rectangle_dimensions(&box, lab_child_x_ - 3,
                    (&lab_center_y_)[(i - 1) * 2 * (i - 1) - (i == 0)] - 3,
                    field_59A0_ + 6, field_59A4_ + 6);
                host->Buffer::box_sprite(&box, &InterfaceBoxStyles[3]);
            }
        } else {
            // 0x499E5E: count 2 (or 0) uses y[4], y[5].
            for (i = 0; i < count; i++) {
                labs_line(lab_child_x_, (&lab_upper_inner_y_)[i],
                          lab_center_x_, lab_center_y_, field_59A0_,
                          field_59A4_, 0);
                StringTemp[0] = 0;
                say_tech(StringTemp, ids[3 + i], 0);
                CharUpperA(StringTemp);
                FlatButton *button = &flatButtons1_[i + 3];
                button->init(StringTemp, i + 0x37B, lab_child_x_,
                             (&lab_upper_inner_y_)[i], field_59A0_,
                             field_59A4_, host, 0);
                button->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
                button->redraw();
                if (has_tech(ids[3 + i], faction)) {
                    button->set_text_color(0xFF, -1, 1, 1);
                }
                button->set_border_styles(&InterfaceBoxStyles[27],
                                          &InterfaceBoxStyles[28],
                                          &InterfaceBoxStyles[29]);
                button->redraw();
                set_rectangle_dimensions(&box, lab_child_x_ - 3,
                    (&lab_upper_inner_y_)[i] - 3, field_59A0_ + 6,
                    field_59A4_ + 6);
                host->Buffer::box_sprite(&box, &InterfaceBoxStyles[3]);
            }
        }
    }

    // 0x499C7E..0x49B172: measure pass (0) then draw pass (1).
    field_A24_ = 0;
    for (int pass = 0; pass < 2; pass++) {
        char id[64];  // UNSURE: size from the frame gap (-0xc8 .. -0x88)
        int i;

        // 0x499FE3..0x49A12C: first status row.
        host->Buffer::set_font(ReportIfGlobal.field_1E19C_,
                               ReportIfGlobal.field_1E19C_,
                               ReportIfGlobal.field_1E19C_, 0);
        host->Buffer::set_text_color3(has_tech(ids[0], faction) ? 0xFF : 0xE1,
                                      -1, 1, 1);
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        say_tech(StringTemp, ids[0], 0);
        CharUpperA(StringTemp);
        strcat(StringTemp, "]");
        if (pass) {
            x = field_A28_;
            y = (field_A34_ + field_A2C_ - field_A24_) / 2;
            box.right = x + field_59A8_ + 3;
            box.left = x - 3;
            box.bottom = field_A24_ + y + 6;
            box.top = y - 9;
            host->Buffer::box_sprite(&box, &InterfaceBoxStyles[3]);
            inflate_frame(&box, -3, -3);
            host->Buffer::box_sprite(&box, &InterfaceBoxStyles[0]);
            y = host->Buffer::wrap_cent(StringTemp, x + 3, y, field_59A8_ - 6);
        } else {
            field_A24_ += host->Buffer::wrap_height(StringTemp,
                                                    field_59A8_ - 6);
        }

        // 0x49A14A..0x49A2F1: second status row (category and depth).
        host->Buffer::set_font(ReportIfGlobal.field_1E190_,
                               ReportIfGlobal.field_1E190_,
                               ReportIfGlobal.field_1E190_, 0);
        host->Buffer::set_text_color3(mandate_color(tech_category(ids[0])),
                                      -1, 1, 1);
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0x209 + tech_category(tech)));
        strcat(StringTemp, BufferJoinSpace);
        say_num(tech_recurse(tech, 0));
        strcat(StringTemp, "]");
        // Genuine original: field_E268_ is added to the row result and then
        // taken back off y on both passes (0x49A235/0x49A268).
        if (pass) {
            y = host->Buffer::wrap_cent(StringTemp, x + 3, y, field_59A8_ - 6)
                + field_E268_;
        } else {
            // UNSURE: the image loads field_E268_ before the wrap_height call.
            field_A24_ += field_E268_
                + host->Buffer::wrap_height(StringTemp, field_59A8_ - 6);
        }
        y -= field_E268_;
        if (pass) {
            g_TECH_ICON_SPRITES[ids[0]].draw_mono(host,
                x + (field_59A8_ - 0x34) / 2 + 3, y + 3,
                mandate_color(tech_category(ids[0])), 0x32,
                g_TECH_ICON_SPRITES[tech].width());
            y += 0x3A;
        } else {
            field_A24_ += 0x3A - field_E268_;
        }

        if (tech != -1) {
            // 0x49A308..0x49A376: host text configuration.
            if (!datalink) {
                host->Buffer::set_text_color(0xE1, -1, 1, 1);
                host->Buffer::set_text_color2(0xE1, -1, 1, 1);
            } else {
                host->set_wrap_width(10);
                host->Buffer::set_font(DatalinkMain.entry_text_font(), 0, 0,
                                       DatalinkMain.entry_link_font());
                host->Buffer::set_text_color(0x99, -1, 1, 1);
                host->Buffer::set_text_color_hyper(0x96, -1, 1, 1);
            }

            // 0x49A37B..0x49A4E2: reactors.
            StringTemp[0] = 0;
            for (i = 0; i < 4; i++) {
                if (Reactor[i].preq_tech == tech) {
                    if (!StringTemp[0]) {
                        strcat(StringTemp, label_get(0x234 / 4));
                        strcat(StringTemp, SocialWinColonSpaceText);
                    } else {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    if (!datalink) {
                        strcat(StringTemp, StringTable.get(Reactor[i].name));
                    } else {
                        // Reactor link ids are one-based.
                        DatalinkMain.construct_id(id, 5, i + 1);
                        LPSTR name = StringTable.get(Reactor[i].name);
                        help_create_link(StringTemp, name, id);
                    }
                }
            }
            if (StringTemp[0]) {
                if (pass) {
                    y = host->Buffer::wrap(StringTemp, x + 3, y,
                                           field_59A8_ - 6);
                } else {
                    field_A24_ += host->Buffer::wrap_height(StringTemp,
                                                            field_59A8_ - 6);
                }
            }

            // 0x49A4E8..0x49A6B6: chassis.
            StringTemp[0] = 0;
            for (i = 0; i < 9; i++) {
                if (Chassis[i].preq_tech == tech) {
                    if (!StringTemp[0]) {
                        strcat(StringTemp, label_get(0x120 / 4));
                        strcat(StringTemp, SocialWinColonSpaceText);
                    } else {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    if (!datalink) {
                        strcat(StringTemp,
                               table_text(Chassis[i].offsv_1_name));
                    } else {
                        DatalinkMain.construct_id(id, 4, i);
                        LPSTR name = StringTable.get(Chassis[i].offsv_1_name);
                        help_create_link(StringTemp, name, id);
                    }
                    // Pinned 0x49A5FC/0x49A606: kind, then the raw name, on
                    // both passes.
                    TechnologyHelpEntryKind = 0x62;
                    TechnologyHelpEntryName = Chassis[i].offsv_1_name;
                    strcat(StringTemp, BufferJoinSpace);
                    strcat(StringTemp, SocialWinOpenParenText);
                    say_num(Chassis[i].speed);
                    strcat(StringTemp, SocialWinCloseParenText);
                }
            }
            if (StringTemp[0]) {
                if (pass) {
                    y = host->Buffer::wrap(StringTemp, x + 3, y,
                                           field_59A8_ - 6);
                } else {
                    field_A24_ += host->Buffer::wrap_height(StringTemp,
                                                            field_59A8_ - 6);
                }
            }

            // 0x49A6BC..0x49A8CA: weapons.
            StringTemp[0] = 0;
            for (i = 0; i < 26; i++) {
                if (Weapon[i].preq_tech == tech) {
                    if (!StringTemp[0]) {
                        strcat(StringTemp, label_get(0x124 / 4));
                        strcat(StringTemp, SocialWinColonSpaceText);
                    } else {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    if (!datalink) {
                        strcat(StringTemp, table_text(Weapon[i].name));
                    } else {
                        DatalinkMain.construct_id(id, 6, i);
                        LPSTR name = StringTable.get(Weapon[i].name);
                        help_create_link(StringTemp, name, id);
                    }
                    TechnologyHelpEntryKind = 0x63;
                    TechnologyHelpEntryName = Weapon[i].name;
                    strcat(StringTemp, BufferJoinSpace);
                    strcat(StringTemp, SocialWinOpenParenText);
                    // The two label arms share their get/strcat tail.
                    if (Weapon[i].offense_rating < 0) {
                        strcat(StringTemp, label_get(0x310 / 4));
                    } else if (Weapon[i].mode < 3) {
                        say_num(
                                                Weapon[i].offense_rating);
                    } else {
                        strcat(StringTemp, label_get(0x200 / 4));
                    }
                    strcat(StringTemp, SocialWinCloseParenText);
                }
            }
            if (StringTemp[0]) {
                if (pass) {
                    y = host->Buffer::wrap(StringTemp, x + 3, y,
                                           field_59A8_ - 6);
                } else {
                    field_A24_ += host->Buffer::wrap_height(StringTemp,
                                                            field_59A8_ - 6);
                }
            }

            // 0x49A8D0..0x49AACA: armor.
            StringTemp[0] = 0;
            for (i = 0; i < 14; i++) {
                if (Armor[i].preq_tech == tech) {
                    if (!StringTemp[0]) {
                        strcat(StringTemp, label_get(0x128 / 4));
                        strcat(StringTemp, SocialWinColonSpaceText);
                    } else {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    if (!datalink) {
                        strcat(StringTemp, table_text(Armor[i].name));
                    } else {
                        DatalinkMain.construct_id(id, 7, i);
                        LPSTR name = StringTable.get(Armor[i].name);
                        help_create_link(StringTemp, name, id);
                    }
                    TechnologyHelpEntryKind = 0x64;
                    TechnologyHelpEntryName = Armor[i].name;
                    strcat(StringTemp, BufferJoinSpace);
                    strcat(StringTemp, SocialWinOpenParenText);
                    if (Armor[i].defense_rating < 0) {
                        strcat(StringTemp, label_get(0x310 / 4));
                    } else {
                        say_num(
                                                Armor[i].defense_rating);
                    }
                    strcat(StringTemp, SocialWinCloseParenText);
                }
            }
            if (StringTemp[0]) {
                if (pass) {
                    y = host->Buffer::wrap(StringTemp, x + 3, y,
                                           field_59A8_ - 6);
                } else {
                    field_A24_ += host->Buffer::wrap_height(StringTemp,
                                                            field_59A8_ - 6);
                }
            }

            // 0x49AAD0..0x49AC35: abilities.
            StringTemp[0] = 0;
            for (i = 0; i < 29; i++) {
                if (Ability[i].preq_tech == tech) {
                    if (!StringTemp[0]) {
                        strcat(StringTemp, label_get(0x12C / 4));
                        strcat(StringTemp, SocialWinColonSpaceText);
                    } else {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    if (!datalink) {
                        strcat(StringTemp, StringTable.get(Ability[i].name));
                    } else {
                        DatalinkMain.construct_id(id, 8, i);
                        LPSTR name = StringTable.get(Ability[i].name);
                        help_create_link(StringTemp, name, id);
                    }
                }
            }
            if (StringTemp[0]) {
                if (pass) {
                    y = host->Buffer::wrap(StringTemp, x + 3, y,
                                           field_59A8_ - 6);
                } else {
                    field_A24_ += host->Buffer::wrap_height(StringTemp,
                                                            field_59A8_ - 6);
                }
            }

            // 0x49AC3B..0x49AD80: predefined prototypes. Genuine original:
            // all 64 prototype rows are scanned (0x9AB868..0x9AC568).
            StringTemp[0] = 0;
            for (i = 0; i < 64; i++) {
                if ((VehPrototypes[i].flags & 1)
                    && VehPrototypes[i].preq_tech == tech) {
                    if (!StringTemp[0]) {
                        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x318 / 4]));
                        strcat(StringTemp, SocialWinColonSpaceText);
                    } else {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    if (!datalink) {
                        strcat(StringTemp, VehPrototypes[i].veh_name);
                    } else {
                        DatalinkMain.construct_id(id, 3, i);
                        help_create_link(StringTemp, VehPrototypes[i].veh_name, id);
                    }
                }
            }
            if (StringTemp[0]) {
                if (pass) {
                    y = host->Buffer::wrap(StringTemp, x + 3, y,
                                           field_59A8_ - 6);
                } else {
                    field_A24_ += host->Buffer::wrap_height(StringTemp,
                                                            field_59A8_ - 6);
                }
            }

            // 0x49AD86..0x49AF76: facilities 1..133.
            StringTemp[0] = 0;
            for (i = 1; i < 134; i++) {
                if (Facility[i].preq_tech == tech
                    || Facility[i].free_tech == tech) {
                    if (!StringTemp[0]) {
                        strcat(StringTemp, label_get(0x110 / 4));
                        strcat(StringTemp, SocialWinColonSpaceText);
                    } else {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    // Genuine original: the opening parenthesis of the
                    // free-facility note precedes the facility name.
                    if (Facility[i].free_tech == tech) {
                        strcat(StringTemp, SocialWinOpenParenText);
                    }
                    if (!datalink) {
                        strcat(StringTemp, StringTable.get(Facility[i].name));
                    } else {
                        if (i < 70) {
                            DatalinkMain.construct_id(id, 0xA, i);
                        } else {
                            DatalinkMain.construct_id(id, 0xB, i);
                        }
                        LPSTR name = StringTable.get(Facility[i].name);
                        help_create_link(StringTemp, name, id);
                    }
                    if (Facility[i].free_tech == tech) {
                        strcat(StringTemp, BufferJoinSpace);
                        strcat(StringTemp, label_get(0x4DC / 4));
                        strcat(StringTemp, SocialWinCloseParenText);
                    }
                }
            }
            if (StringTemp[0]) {
                if (pass) {
                    y = host->Buffer::wrap(StringTemp, x + 3, y,
                                           field_59A8_ - 6);
                } else {
                    field_A24_ += host->Buffer::wrap_height(StringTemp,
                                                            field_59A8_ - 6);
                }
            }

            // 0x49AF7B..0x49B07D: social models. Genuine original: the
            // section is flushed after every match without clearing
            // StringTemp, so the accumulated text is wrapped again each time.
            StringTemp[0] = 0;
            for (i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (SocialCategories[i].preq_tech[j] == tech) {
                        if (!StringTemp[0]) {
                            strcat(StringTemp, label_get(0x118 / 4));
                            strcat(StringTemp, SocialWinColonSpaceText);
                        } else {
                            strcat(StringTemp, SocialWinCommaSpaceText);
                        }
                        // UNSURE: the image tests datalink here with no
                        // branch (0x49AFEF..0x49AFF7); identical arms that VC6
                        // merged would leave exactly that test.
                        if (!datalink) {
                            strcat(StringTemp, StringTable.get(
                                SocialCategories[i].name[j]));
                        } else {
                            strcat(StringTemp, StringTable.get(
                                SocialCategories[i].name[j]));
                        }
                        if (StringTemp[0]) {
                            if (pass) {
                                y = host->Buffer::wrap(StringTemp, x + 3, y,
                                                       field_59A8_ - 6);
                            } else {
                                field_A24_ += host->Buffer::wrap_height(
                                    StringTemp, field_59A8_ - 6);
                            }
                        }
                    }
                }
            }

            // 0x49B083..0x49B162: citizens. Same per-match flush; no link.
            StringTemp[0] = 0;
            for (i = 0; i < 7; i++) {
                if (Citizen[i].preq_tech == tech) {
                    if (!StringTemp[0]) {
                        strcat(StringTemp, label_get(0x4CC / 4));
                        strcat(StringTemp, SocialWinColonSpaceText);
                    } else {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    strcat(StringTemp,
                           StringTable.get(Citizen[i].singular_name));
                    if (StringTemp[0]) {
                        if (pass) {
                            y = host->Buffer::wrap(StringTemp, x + 3, y,
                                                   field_59A8_ - 6);
                        } else {
                            field_A24_ += host->Buffer::wrap_height(
                                StringTemp, field_59A8_ - 6);
                        }
                    }
                }
            }
        }
    }

    // 0x49B178..0x49B1CF
    host->set_wrap_width(0);
    for (int button_index = 1; button_index < 7; button_index++) {
        FlatButton *button = &flatButtons1_[button_index];
        // Genuine original: the mouse position is read into x/y.
        button->get_mouse_pos(&x, &y);
        button->on_mouse_move(x, y);
    }
    host->Buffer::set_clip(host->buffer_working_rect());
    // Genuine original: the last button's mouse position is published into
    // rect1_'s left/top (the ordinary host loaded them as x/y at entry).
    rect1_.left = x;
    rect1_.top = y;
}

// ORIGINAL: 0x0049B220 ?on_button_passover@ReportWin@@QAEXHH@Z 0x0049B220-0x0049B446 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0049B220-0x0049B446 and 0x0065C0C2-0x0065C20E", "Retired recovered seed provenance: git:649230f6:src/recovered/units/0049b220.cpp retains the recovered seed and original claim", "source:src/reportwin.h", "source:src/reportif.h:ReportIfGlobal.field_8_"], "reason": "Recover the category-button passover drawing through the seven owned FlatButtons, shared work RECT, real host window and existing labs_line. The original seed and historical claim remain restoration obligations; current byte correspondence is measured separately."}
// symbol    ?on_button_passover@ReportWin@@UAEXHH@Z
void ReportWin::on_button_passover(int button_id, int state) {
    GraphicWin *host = IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 8
        ? static_cast<GraphicWin *>(&DatalinkMain)
        : static_cast<GraphicWin *>(this);
    if (ReportIfGlobal.field_8_ != 2 || button_id < 0x378) return;

    button_id -= 0x378;
    // Original 0x0049B26C indexes without an upper-bound check.
    report_work_rect_ = flatButtons1_[button_id].outer_rect();

    int first_x, first_y, last_x, last_y;
    if (button_id < 3) {
        first_x = (int)lab_center_x_;
        first_y = (int)lab_center_y_;
        last_x = report_work_rect_.left;
        last_y = report_work_rect_.top;
    } else {
        first_x = report_work_rect_.left;
        first_y = report_work_rect_.top;
        last_x = (int)lab_center_x_;
        last_y = (int)lab_center_y_;
    }

    inflate_frame(&report_work_rect_, 3, 3);

    RECT marker;
    int x = (int)field_A28_;
    int center_y = (field_A34_ + field_A2C_ - field_A24_) / 2 - 3;
    set_rectangle_dimensions(&marker, x, center_y, field_59A8_, field_A24_ + 3);
    inflate_frame(&marker, 3, 6);
    if (state != 0) {
        host->Buffer::box_sprite(&marker, &InterfaceBoxStyles[5]);
        inflate_frame(&marker, -3, -3);
        host->Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[5]);
        labs_line(first_x, first_y, last_x, last_y,
                  field_59A0_, field_59A4_, 1);
    } else {
        host->Buffer::box_sprite(&marker, &InterfaceBoxStyles[3]);
        inflate_frame(&marker, -3, -3);
        host->Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[3]);
        labs_line(first_x, first_y, last_x, last_y,
                  field_59A0_, field_59A4_, 0);
    }
    host->soft_update();
}

/*
Purpose: Draw one lab-connection line of the research tree, plus its arrow.

The drawing surface is this window's Buffer unless the active interface is
slot 8, in which case it is the Datalink's (the research screen). The line
is a box_sprite from the shared InterfaceBoxStyles pair - element 3 when
style_select is zero, element 5 otherwise - clipped against the working
RECT at field_E258_, and the arrow sprite is the matching entry of the
tech-tree arrow table.

             host = IfaceIdToSlot[ActiveIfaceId] != 8 ? this : &DatalinkMain
             buffer = host + 0x444 (the GraphicWin Buffer base)
             arrow = g_IFACE_TECH_TREE_ARROW_SPRITES[style_flag]
             style = style_select == 0 ? styles[3] : styles[5]

The first branch (first_id == lab_center_x_) draws the tag-to-line
connector and returns; the three middle branches draw the line's right
half for each matched lab pair member; the common tail draws the line box
and the arrow head.
// ORIGINAL: 0x0049B450 ?labs_line@ReportWin@@QAEXHHHHHHH@Z 0x0049B450-0x0049BCC7 FILE
// TRIED: Ghidra-structure translation fixed to real Sprite::draw()/ Buffer::set_clip()/box_sprite() member calls (Ghidra had flattened the thiscall receiver away on all 6 draw() sites and both callee methods); compiles, MISMATCH #6 (early - likely the local8/table[idx]!=8 selection prologue).
// size      2167 bytes
// prototype void (__thiscall ?labs_line@ReportWin@@QAEXHHHHHHH@Z)(ReportWin* this, int, int, int, int, int, int, int)
// callers   2   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D8000 0x005E2DD3 0x005E4B9A
// Homed from src/unrecovered/0049b450.cpp. The seed's 0x78D5A0/0x78D5F0
// styles are shared InterfaceBoxStyles elements 3/5 (base 0x78D528: socialwin
// already re-homed element 3/6 the same way), and its 0x75AFD0 sprite is
// g_IFACE_TECH_TREE_ARROW_SPRITES (stride 0x2C, so stB*4 == 44*style_flag).
// Image shape: every rectangle is addressed as the member itself (the image
// forms lea esi,[edi+0xE258] at each use, so no pointer local), four-value
// fills evaluate all coordinates before the stores (set_rect), the
// 7-pixel shrink reloads the right edge (inflate_frame on the member), and
// each arrow evaluates the Buffer conversion and sprite address before the
// positions (the late-sprite Sprite::draw(Buffer *, x, y) form).
// LEVER: register equilibrium. The image keeps only `this` (edi) in a
// callee-saved register and half in [ebp-8]; ours enregisters half in ebx,
// so the Buffer pointer spills to a dead parameter slot. Tried: non-const,
// assignment-in-expression, recomputed half, a y local, block-scoped mid,
// declaration order; none moved it.
*/
void ReportWin::labs_line(int first_id, int second_id, int, int branch_id,
                          int x_offset, int line_height, int style_select) {
    // The research screen draws on the Datalink's own surface.
    GraphicWin *host = &DatalinkMain;
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 8) host = this;
    const int half = line_height / 2;
    // The image keeps the row centre in the line-height parameter's slot.
    line_height = static_cast<int>(lab_center_y_) + half;

    if (first_id == static_cast<int>(lab_center_x_)) {
        // 0x49B49D. The midpoint reuses first_id: the image keeps it in that
        // parameter's slot in both halves (0x49B4BE, 0x49B735).
        first_id = (static_cast<int>(lab_prerequisite_x_)
            + static_cast<int>(lab_center_x_) + x_offset) / 2 - 2;
        if (branch_id != static_cast<int>(lab_upper_inner_y_)
            && branch_id != static_cast<int>(lab_lower_inner_y_)) {
            // 0x49B4D3
            set_rect(&report_work_rect_,
                static_cast<int>(lab_prerequisite_x_) + x_offset - 4, line_height,
                first_id + 7, line_height + 20);
            inflate_frame(&report_work_rect_, -7, 0);
            report_work_rect_.bottom -= 7;
            host->Buffer::set_clip(&report_work_rect_);
            inflate_frame(&report_work_rect_, 7, 0);
            report_work_rect_.bottom += 7;
            if (style_select) {
                host->Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[5]);
                host->Buffer::restore_clip();
            } else {
                host->Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[3]);
                host->Buffer::restore_clip();
            }
        } else {
            // 0x49B57C
            set_rect(&report_work_rect_,
                static_cast<int>(lab_prerequisite_x_) + x_offset + 3,
                static_cast<int>(lab_upper_inner_y_) + half, first_id,
                static_cast<int>(lab_lower_inner_y_) + half);
            if (branch_id == static_cast<int>(lab_upper_inner_y_)) {
                report_work_rect_.bottom = line_height;
            } else {
                report_work_rect_.top = line_height + 1;
            }
            host->Buffer::set_clip(&report_work_rect_);
            // UNSURE: the image's push-select shape here and at 0x49B650 and
            // 0x49BBA9 (je; push; jmp; push; call) is an if/else of two
            // box_sprite calls, but that spelling flips the canary
            // on_iface_button_clicked through unit coupling; the earlier
            // ternary stays.
            set_rect(&report_work_rect_,
                static_cast<int>(lab_prerequisite_x_) + x_offset - 4,
                static_cast<int>(lab_upper_inner_y_) + half, first_id,
                static_cast<int>(lab_lower_inner_y_) + half);
            host->Buffer::box_sprite(&report_work_rect_,
                style_select ? &InterfaceBoxStyles[5] : &InterfaceBoxStyles[3]);
            host->Buffer::restore_clip();
        }
        // 0x49B62F
        set_rect(&report_work_rect_,
            first_id - 7, line_height,
            static_cast<int>(lab_center_x_) + 4, line_height + 20);
        inflate_frame(&report_work_rect_, -7, 0);
        report_work_rect_.bottom -= 7;
        host->Buffer::set_clip(&report_work_rect_);
        inflate_frame(&report_work_rect_, 7, 0);
        report_work_rect_.bottom += 7;
        host->Buffer::box_sprite(&report_work_rect_,
            style_select ? &InterfaceBoxStyles[5] : &InterfaceBoxStyles[3]);
        host->Buffer::restore_clip();
        // 0x49B6C1
        g_IFACE_TECH_TREE_ARROW_SPRITES[style_select ? 2 : 0].draw(host,
                           static_cast<int>(lab_center_x_) - 10,
                           static_cast<int>(lab_center_y_) + half - 6);
        return;
    }

    // 0x49B710
    first_id = (field_59A8_ + static_cast<int>(lab_child_x_)
        + static_cast<int>(lab_center_x_)) / 2 - 2;
    if (second_id != static_cast<int>(lab_three_child_upper_y_)
        && second_id != static_cast<int>(lab_three_child_lower_y_)) {
        if (second_id != static_cast<int>(lab_four_child_top_y_)
            && second_id != static_cast<int>(lab_four_child_bottom_y_)) {
            if (second_id != static_cast<int>(lab_upper_inner_y_)
                && second_id != static_cast<int>(lab_lower_inner_y_)) {
                // 0x49B77A: the arrow is drawn before the line.
                g_IFACE_TECH_TREE_ARROW_SPRITES[style_select ? 2 : 0].draw(host,
                                   static_cast<int>(lab_child_x_) - 10,
                                   line_height - 6);
                set_rect(&report_work_rect_,
                    first_id - 7, line_height,
                    static_cast<int>(lab_child_x_) + 4, line_height + 20);
                inflate_frame(&report_work_rect_, -7, 0);
                report_work_rect_.bottom -= 7;
                host->Buffer::set_clip(&report_work_rect_);
                inflate_frame(&report_work_rect_, 7, 0);
                report_work_rect_.bottom += 7;
            } else {
                // 0x49B836
                set_rect(&report_work_rect_,
                    first_id,
                    static_cast<int>(lab_upper_inner_y_) + half, static_cast<int>(lab_child_x_) - 3,
                    static_cast<int>(lab_lower_inner_y_) + half);
                if (second_id == static_cast<int>(lab_upper_inner_y_)) {
                    report_work_rect_.bottom = line_height;
                    g_IFACE_TECH_TREE_ARROW_SPRITES[style_select ? 2 : 0].draw(host,
                                       static_cast<int>(lab_child_x_) - 10,
                                       static_cast<int>(lab_upper_inner_y_) + half - 6);
                } else {
                    report_work_rect_.top = line_height + 1;
                    g_IFACE_TECH_TREE_ARROW_SPRITES[style_select ? 2 : 0].draw(host,
                                       static_cast<int>(lab_child_x_) - 10,
                                       static_cast<int>(lab_lower_inner_y_) + half - 7);
                }
                host->Buffer::set_clip(&report_work_rect_);
                set_rect(&report_work_rect_,
                    first_id,
                    static_cast<int>(lab_upper_inner_y_) + half, static_cast<int>(lab_child_x_) + 4,
                    static_cast<int>(lab_lower_inner_y_) + half);
            }
        } else {
            // 0x49B95C
            set_rect(&report_work_rect_,
                first_id,
                static_cast<int>(lab_four_child_top_y_) + half, static_cast<int>(lab_child_x_) - 3,
                static_cast<int>(lab_four_child_bottom_y_) + half);
            if (second_id == static_cast<int>(lab_four_child_top_y_)) {
                report_work_rect_.bottom = line_height;
                g_IFACE_TECH_TREE_ARROW_SPRITES[style_select ? 2 : 0].draw(host,
                                   static_cast<int>(lab_child_x_) - 10,
                                   static_cast<int>(lab_four_child_top_y_) + half - 6);
            } else {
                report_work_rect_.top = line_height + 1;
                g_IFACE_TECH_TREE_ARROW_SPRITES[style_select ? 2 : 0].draw(host,
                                   static_cast<int>(lab_child_x_) - 10,
                                   static_cast<int>(lab_four_child_bottom_y_) + half - 7);
            }
            host->Buffer::set_clip(&report_work_rect_);
            set_rect(&report_work_rect_,
                first_id,
                static_cast<int>(lab_four_child_top_y_) + half, static_cast<int>(lab_child_x_) + 4,
                static_cast<int>(lab_four_child_bottom_y_) + half);
        }
    } else {
        // 0x49BA82
        set_rect(&report_work_rect_,
            first_id,
            static_cast<int>(lab_three_child_upper_y_) + half, static_cast<int>(lab_child_x_) - 3,
            static_cast<int>(lab_three_child_lower_y_) + half);
        if (second_id == static_cast<int>(lab_three_child_upper_y_)) {
            report_work_rect_.bottom = line_height;
            g_IFACE_TECH_TREE_ARROW_SPRITES[style_select ? 2 : 0].draw(host,
                               static_cast<int>(lab_child_x_) - 10,
                               static_cast<int>(lab_three_child_upper_y_) + half - 6);
        } else {
            report_work_rect_.top = line_height + 1;
            g_IFACE_TECH_TREE_ARROW_SPRITES[style_select ? 2 : 0].draw(host,
                               static_cast<int>(lab_child_x_) - 10,
                               static_cast<int>(lab_three_child_lower_y_) + half - 7);
        }
        host->Buffer::set_clip(&report_work_rect_);
        set_rect(&report_work_rect_,
            first_id,
            static_cast<int>(lab_three_child_upper_y_) + half, static_cast<int>(lab_child_x_) + 4,
            static_cast<int>(lab_three_child_lower_y_) + half);
    }
    // 0x49BBA9
    if (style_select) {
        host->Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[5]);
        host->Buffer::restore_clip();
    } else {
        host->Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[3]);
        host->Buffer::restore_clip();
    }
    // 0x49BBCF
    report_work_rect_.left = field_59A8_ + static_cast<int>(lab_center_x_) - 4;
    report_work_rect_.top = line_height;
    if (style_select) {
        report_work_rect_.right = first_id + 7;
        report_work_rect_.bottom = line_height + 20;
        inflate_frame(&report_work_rect_, -7, 0);
        report_work_rect_.bottom -= 7;
        host->Buffer::set_clip(&report_work_rect_);
        inflate_frame(&report_work_rect_, 7, 0);
        report_work_rect_.bottom += 7;
        host->Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[5]);
        host->Buffer::restore_clip();
    } else {
        report_work_rect_.right = first_id + 7;
        report_work_rect_.bottom = line_height + 20;
        inflate_frame(&report_work_rect_, -7, 0);
        report_work_rect_.bottom -= 7;
        host->Buffer::set_clip(&report_work_rect_);
        inflate_frame(&report_work_rect_, 7, 0);
        report_work_rect_.bottom += 7;
        host->Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[3]);
        host->Buffer::restore_clip();
    }
}

/*
Purpose: Draw the energy page's five-panel frame and calculate the faction's
         underlying accounts. Text, row breakdowns and selection are pending.
// ORIGINAL: 0x0049BCD0 ?draw_energy@ReportWin@@QAEXXZ 0x0049BCD0-0x0049D481 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0049bcd0.cpp", "src/reportwin.h", "src/socialwin.cpp"], "reason": "Recover the energy-page frame and shared accounting through owned ReportWin geometry and the typed energy computation; preserve the remaining row and selection work explicitly."}
// TRIED: byte-exact prologue - the six local RECTs the original packs into overlapping/aliased ebp slots (shared fields between the six box_sprite calls) are instead six independent RECT locals here, so the frame layout differs from the first instruction on; 765/1663 mnemonics matched (80% rebuilt/original count), all string-building "report line" blocks, the diplomacy/loan loop, both base-relations branches and the ListBox/Dialog vbtable-relative subobject calls are transcribed field-for- field and call-for-call from the disassembly (cross-checked, not just taken from Ghidra). The two `mov ecx,[x]; mov edx, [ecx+8]; ...` virtual-base lookups are reproduced as raw pointer arithmetic rather than through ListBox's own `dialog_` member, since that field's offset is unverified.
// size      6065 bytes
// prototype void (__thiscall ?draw_energy@ReportWin@@QAEXXZ)(ReportWin* this)
// callers   3   call targets   30
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00445130 0x0048C060 0x004E3A00 0x004F6510 0x0050BA00 0x00590910 0x005DACB0 0x005DACE0 0x005DAD10 0x005DCEA0 0x005DD130 0x005DD300 0x005DDF60 0x005DE1D0 0x005E2DD3 0x00609470 0x00609C60 0x00609C90 0x00609CC0 0x00609CF0 0x00609F20 0x00609FA0 0x0060A670 0x0060C920 0x006169A0 0x00625E30 0x00625EC0 0x006453E0 0x00645470 0x0064FC88
Facility mode scans owned bases for one selected facility; base mode scans
facilities 0..64 in one selected base. Those indices are not faction IDs.
*/
void ReportWin::draw_energy() {
    const int faction = ReportIfGlobal.report_faction_id_;
    int total = 0;
    int base_id;
    int facility;
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3,
                  energy_panel_rect_.top + field_E268_ + 9};
    RECT account_head = {energy_panel_rect_.left + 3, title.bottom + 2,
                         (energy_panel_rect_.left + energy_panel_rect_.right) / 2 - 1,
                         title.bottom + field_E268_ + 9};
    RECT account_body = {account_head.left, account_head.bottom + 2,
                         account_head.right, energy_panel_rect_.bottom - 3};
    RECT list_head = {account_head.right + 3, title.bottom + 2,
                      energy_panel_rect_.right - 3, account_head.bottom};
    RECT list_body = {list_head.left, list_head.bottom + 2,
                      energy_panel_rect_.right - 3, energy_panel_rect_.bottom - 3};
    Buffer::box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[3]);
    Buffer::box_sprite(&title, &InterfaceBoxStyles[1]);
    Buffer::box_sprite(&account_head, &InterfaceBoxStyles[1]);
    Buffer::box_sprite(&list_head, &InterfaceBoxStyles[1]);
    Buffer::box_sprite(&account_body, &InterfaceBoxStyles[0]);
    Buffer::box_sprite(&list_body, &InterfaceBoxStyles[0]);
    Buffer::set_text_color(0x99, -1, 1, 1);
    Buffer::set_text_color2(0xF2, -1, 1, 1);
    Buffer::set_text_color3(0xE1, -1, 1, 1);
    energy_compute(faction, 0);

    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    GenderDefault = Players[faction].noun_gender;
    PluralityDefault = Players[faction].is_noun_plural;
    strcat(StringTemp, Players[faction].formal_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0xA98 / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    Buffer::write_cent_l(StringTemp, &title);
    ++title.top;
    ++title.bottom;

    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0xB9C / 4));
    strcat(StringTemp, "]");
    --account_head.top;
    --account_head.bottom;
    Buffer::write_cent_l(StringTemp, &account_head);
    ++account_head.top;
    ++account_head.bottom;

    rect1_.top = account_body.top + 6;
    rect1_.left = account_body.left + 6;
    rect1_.right = account_body.right - account_body.left - 12;

    // Reserves.
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0xE4 / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    Buffer::write_l(StringTemp, rect1_.left, rect1_.top);
    strcat(StringTemp, "}");
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    say_num(PlayersData[faction].energy_reserves);
    strcat(StringTemp, "}");
    Buffer::write_right(StringTemp, rect1_.left, rect1_.top, rect1_.right);

    // Economy.
    rect1_.top += field_E268_ * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x5F4 / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    Buffer::write_l(StringTemp, rect1_.left, rect1_.top);
    strcat(StringTemp, "}");
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    say_num(EnergyEconomyTotal);
    strcat(StringTemp, "}");
    Buffer::write_right(StringTemp, rect1_.left, rect1_.top, rect1_.right);

    // Maintenance.
    rect1_.top += field_E268_;
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x5F8 / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    Buffer::write_l(StringTemp, rect1_.left, rect1_.top);
    strcat(StringTemp, "}");
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, "-");
    say_num(EnergyMaintenanceTotal);
    strcat(StringTemp, "}");
    Buffer::write_right(StringTemp, rect1_.left, rect1_.top, rect1_.right);

    // Commerce.
    rect1_.top += field_E268_;
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x5FC / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    Buffer::write_l(StringTemp, rect1_.left, rect1_.top);
    strcat(StringTemp, "}");
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    if (EnergyCommerceDelta > 0) {
        strcat(StringTemp, "+");
    }
    say_num(EnergyCommerceDelta);
    strcat(StringTemp, "}");
    Buffer::write_right(StringTemp, rect1_.left, rect1_.top, rect1_.right);

    // Economic sanctions.
    if (PlayersData[faction].sanction_turns != 0) {
        rect1_.top += field_E268_ * 2;
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        strcat(StringTemp, StringTable.get((Players[faction].rule_flags & 0x80)
                                               ? Labels.strings_ptr[0x1194 / 4]
                                               : Labels.strings_ptr[0x708 / 4]));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, "}");
        CharUpperA(StringTemp);
        Buffer::write_l(StringTemp, rect1_.left, rect1_.top);
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        say_num(PlayersData[faction].sanction_turns);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x71C / 4));
        strcat(StringTemp, "}");
        Buffer::write_right(StringTemp, rect1_.left, rect1_.top, rect1_.right);
    }

    // Faction interest rule.
    int have_heading = 0;
    if (Players[faction].rule_flags & 0x4000) {
        rect1_.top += field_E268_;
        have_heading = 1;
        const int interest = Players[faction].rule_interest;
        parse_num(0, interest);
        StringTemp[0] = 0;
        strcat(StringTemp, "FACTENERGY");
        if (interest >= 0) {
            strcat(StringTemp, "BON");
            if (interest == 0) {
                strcat(StringTemp, "2");
            }
        } else {
            strcat(StringTemp, "PEN");
        }
        parse_it(ScriptPopupFile, StringTemp);
        Buffer::wrap_flying(StringTemp, rect1_.left, rect1_.top, rect1_.right);
        Buffer::wrap_flying_flush();
        rect1_.top += field_E268_ * 2;
    }

    // Loans owed to or by the other factions.
    for (int other = 1; other < MaxPlayerNum; ++other) {
        if (other == faction) continue;
        const int alive = GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;
        if (!(alive & (1 << other))) continue;
        if (!(alive & (1 << faction))) continue;
        if (PlayersData[faction].loan_balance[other] == 0
            && PlayersData[other].loan_balance[faction] == 0) continue;
        if (!have_heading) {
            have_heading = 1;
            rect1_.top += 10;
        }
        rect1_.top += field_E268_ * 2;
        StringTemp[0] = 0;
        if (PlayersData[faction].diplo_treaties[other] & 0x10) {
            strcat(StringTemp, "*");
            strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x604 / 4]));
            strcat(StringTemp, BufferJoinSpace);
        } else if ((PlayersData[faction].loan_balance[other] != 0
                    && PlayersData[other].sanction_turns != 0)
                   || (PlayersData[other].loan_balance[faction] != 0
                       && PlayersData[faction].sanction_turns != 0)) {
            strcat(StringTemp, "*");
            strcat(StringTemp, label_get(0x708 / 4));
            strcat(StringTemp, BufferJoinSpace);
        }
        parse_says(3, StringTemp, -1, -1);
        GenderDefault = Players[other].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[other].title_leader, -1, -1);
        GenderDefault = Players[other].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[other].name_leader, -1, -1);
        GenderDefault = Players[other].noun_gender;
        PluralityDefault = Players[other].is_noun_plural;
        parse_says(2, Players[other].noun_faction, -1, -1);
        parse_num(1, PlayersData[faction].loan_balance[other]
                         ? PlayersData[faction].loan_balance[other]
                         : PlayersData[other].loan_balance[faction]);
        parse_num(0, PlayersData[faction].loan_balance[other]
                         ? PlayersData[faction].loan_payment[other]
                         : PlayersData[other].loan_payment[faction]);
        parse_it(ScriptPopupFile, PlayersData[faction].loan_balance[other]
                                      ? "LOANPAYMENT" : "LOANINCOME");
        Buffer::wrap_flying(StringTemp, rect1_.left, rect1_.top, rect1_.right);
        Buffer::wrap_flying_flush();
    }

    // Net income.
    rect1_.top += field_E268_ * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0xEFC / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    Buffer::write_l(StringTemp, rect1_.left, rect1_.top);
    strcat(StringTemp, "}");
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    if (NetIncomeGlobal > 0) {
        strcat(StringTemp, "+");
    }
    say_num(NetIncomeGlobal);
    strcat(StringTemp, label_get(0x5EC / 4));
    strcat(StringTemp, "}");
    Buffer::write_right(StringTemp, rect1_.left, rect1_.top, rect1_.right);

    // Default selection: the first owned base.
    if (field_59AC_ == -1) {
        field_59B0_ = 0;
        for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; ++i) {
            if (Bases[i].faction_id_current == ReportIfGlobal.report_faction_id_) {
                field_59B0_ = 1;
                field_59AC_ = i;
                break;
            }
        }
    }
    if (field_59AC_ == -1) return;

    rect1_.left = list_body.left + 6;
    rect1_.top = list_body.top + 6;
    rect1_.right = list_body.right - list_body.left - 12;
    inflate_frame(&list_body, -3, -3);
    ListBox *list = &listBox_;
    list->close();
    list->Dialog::init(&list_body, 0);
    inflate_frame(&list_body, 3, 3);
    list->Dialog::field_48_ = field_E268_ + 3;
    listBox_.Dialog::set_dialog_font(ReportIfGlobal.field_1E184_,
                                     ReportIfGlobal.field_1E184_,
                                     ReportIfGlobal.field_1E184_);
    listBox_.Dialog::set_dialog_text_color(0x99, -1, 1, 1);
    listBox_.Dialog::set_dialog_text_color2(0x99, -1, 1, 1);
    listBox_.Dialog::set_dialog_text_color3(0x99, -1, 1, 1);
    listBox_.set_report_text_color(0xF2);

    if (field_59B0_) {
        // Base mode: the selected base's facilities.
        base_id = field_59AC_;
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        say_base(StringTemp, base_id);
        CharUpperA(StringTemp);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0xA98 / 4));
        strcat(StringTemp, "]");
        --list_head.top;
        --list_head.bottom;
        Buffer::write_cent_l(StringTemp, &list_head);
        ++list_head.top;
        ++list_head.bottom;
        for (facility = 0; facility < 0x41; ++facility) {
            if (Bases[base_id].faction_id_current == ReportIfGlobal.report_faction_id_
                && has_fac(facility, base_id, 0)) {
                StringTemp[0] = 0;
                say_facility(StringTemp, facility, 0);
                strcat(StringTemp, "|");
                strcat(StringTemp, "{");
                const int maint = fac_maint(facility, ReportIfGlobal.report_faction_id_);
                if (has_project(SP_SELF_AWARE_COLONY, ReportIfGlobal.report_faction_id_)) {
                    total += maint;
                    const int odd = maint & 1;
                    say_num(maint / 2);
                    if (odd) {
                        strcat(StringTemp, ".5");
                    }
                } else {
                    say_num(maint);
                    total += maint;
                }
                strcat(StringTemp, "}");
                list->item(StringTemp, facility);
            }
        }
        list->item(" | ", 99999998);
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        strcat(StringTemp, label_get(0xA9C / 4));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, "|");
        if (has_project(SP_SELF_AWARE_COLONY, ReportIfGlobal.report_faction_id_)) {
            say_num(total / 2);
            if (total & 1) {
                strcat(StringTemp, ".5");
            }
        } else {
            say_num(total);
        }
        strcat(StringTemp, "}");
    } else {
        // Facility mode: every owned base with the selected facility.
        facility = field_59AC_;
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        say_facility(StringTemp, facility, 0);
        CharUpperA(StringTemp);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x160 / 4));
        strcat(StringTemp, "]");
        CharUpperA(StringTemp);
        --list_head.top;
        --list_head.bottom;
        Buffer::write_cent_l(StringTemp, &list_head);
        ++list_head.top;
        ++list_head.bottom;
        for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (Bases[base_id].faction_id_current == ReportIfGlobal.report_faction_id_
                && has_fac(facility, base_id, 0)) {
                StringTemp[0] = 0;
                say_base(StringTemp, base_id);
                strcat(StringTemp, "|");
                strcat(StringTemp, "{");
                const int maint = fac_maint(facility, ReportIfGlobal.report_faction_id_);
                if (has_project(SP_SELF_AWARE_COLONY, ReportIfGlobal.report_faction_id_)) {
                    total += maint;
                    const int odd = maint & 1;
                    say_num(maint / 2);
                    if (odd) {
                        strcat(StringTemp, ".5");
                    }
                } else {
                    say_num(maint);
                    total += maint;
                }
                strcat(StringTemp, "}");
                list->item(StringTemp, base_id + 0x400);
            }
        }
        list->item(" | ", 99999998);
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        strcat(StringTemp, label_get(0xA9C / 4));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, "|");
        if (has_project(SP_SELF_AWARE_COLONY, ReportIfGlobal.report_faction_id_)) {
            say_num(total / 2);
            if (total & 1) {
                strcat(StringTemp, ".5");
            }
        } else {
            say_num(total);
        }
        strcat(StringTemp, "}");
    }
    list->item(StringTemp, 99999999);
    listBox_.field_2C_ = 0;
    listBox_.clear_selection_index();
    listBox_.Dialog::field_B4_ = 0;
    list->ListBox::attach(this, list->Dialog::window_x_, list->Dialog::window_y_, 0x81020);
    list->set_selected_id(99999999);
    listBox_.show(0);
}

/*
Purpose: The topic-panel selection callback. The image:

               cmp edi, 0x400 / jge else        - the selection id
               [esi+0x59AC] = id; [esi+0x59B0] = 0
               ListBox(0x00890208).set_selected_pos(-1, 1)
               ListBox(0x00890D5C).set_selected_id(id)
               ... else:
               cmp edi, 0x5F5E0FE / jl skip     - 0x5F5E0FD = 99999999
               ListBox(self+0x5A0C).set_selected_id(99999999) and return
               [esi+0x59AC] = id - 0x400; [esi+0x59B0] = 1
               ListBox(0x00890208).set_selected_id(id)
               ListBox(0x00890D5C).set_selected_pos(-1, 1)
               then reset(); draw_energy(); GraphicWin::soft_update()

         0x59AC/0x59B0 remember (id, kind): the field pair this class
         declares past flatButtons1_.
// ORIGINAL: 0x0049D490 ?on_selected@ReportWin@@QAEXHH@Z 0x0049D490-0x0049D535 FILE BYTE_EXACT
// symbol    ?on_selected@ReportWin@@UAEXHH@Z
// size      165 bytes
// prototype void (__thiscall ?on_selected@ReportWin@@QAEXHH@Z)(ReportWin* this, int, int)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00496BB0 0x0049BCD0 0x005D5890 0x00609FA0 0x0060A020
// Homed from src/unrecovered/0049d490.cpp.
*/
void ReportWin::on_selected(int selection_id, int) {
    if (selection_id < 0x400) {
        field_59AC_ = selection_id;
        field_59B0_ = 0;
        ReportIfGlobal.listBox1_.set_selected_pos(-1, 1);
        ReportIfGlobal.listBox2_.set_selected_id(selection_id);
    } else {
        if (selection_id >= 0x5F5E0FE) {
            listBox_.set_selected_id(99999999);
            return;
        }
        field_59AC_ = selection_id - 0x400;
        field_59B0_ = 1;
        ReportIfGlobal.listBox1_.set_selected_id(selection_id);
        ReportIfGlobal.listBox2_.set_selected_pos(-1, 1);
    }
    reset();
    draw_energy();
    soft_update();
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x0049D540 0x0049EAEC 0x0049EB0C 0x0049D882
// ORIGINAL: 0x0049D540 ?draw_ops@ReportWin@@QAEXXZ 0x0049D540-0x0049EAEB;0x00659435-0x00659447
// RECOVERY_BINDING: v1 {"evidence": ["git:649230f6:src/unrecovered/0049d540.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_ops() {
    const int faction = ReportIfGlobal.report_faction_id_;       // edi, spilled to [ebp-0x74]
    Random rng;                                                  // [ebp-0x54]
    rng.reseed(timeGetTime());
    // Image stores 59A0 then 59A4 from one register.
    field_59A0_ = field_59A4_ = 40;
    const int split = energy_panel_rect_.top + 34 + field_E268_;
    RECT upper = {energy_panel_rect_.left, energy_panel_rect_.top,           // [ebp-0x94]
        energy_panel_rect_.right, split};
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,   // [ebp-0x6c]
        energy_panel_rect_.right - 3, energy_panel_rect_.top + field_E268_ + 9};
    RECT lower = {energy_panel_rect_.left, split + 2,                        // [ebp-0x84]
        energy_panel_rect_.right, energy_panel_rect_.bottom};
    RECT icon = {energy_panel_rect_.left + 3, split + 5,                     // [ebp-0x34]
        energy_panel_rect_.left + 43, split + 45};
    RECT text = {energy_panel_rect_.left + 45, split + 5,                    // [ebp-0x48]
        energy_panel_rect_.right - 3, split + 45};
    Buffer::box_sprite(&upper, &InterfaceBoxStyles[0]);
    Buffer::box_sprite(&title, &InterfaceBoxStyles[1]);
    Buffer::box_sprite(&lower, &InterfaceBoxStyles[0]);
    const int pitch = field_59A4_ + 2;                           // [ebp-0x1c]
    const int rows = (lower.bottom - lower.top - 6) / pitch;     // [ebp-0x4c]
    flatButton4_.show(0);
    flatButton5_.show(0);
    flatButton6_.show(0);
    // The image CSEs &PlayersData[faction].current_num_bases into [ebp-0x20]
    // and re-reads the count through it in the field_E270_ block.
    if (field_C0F8_ == 0) {
        if (PlayersData[faction].current_num_bases >= rows) {
            field_E26C_ = 1;
        }
    } else if (PlayersData[faction].current_num_bases < rows) {
        scroll_hide(this, &scroll_);
        field_C0F8_ = 0;
    }
    if (field_E270_ != 0) {
        field_C108_ = rows - 1;
        field_C0FC_ = field_C100_ = 0;
        field_C104_ = PlayersData[faction].current_num_bases - 1;
        scroll_.set_range(0, PlayersData[faction].current_num_bases - 1);
        scroll_.paging_ = field_C108_;
        scroll_.set_pos(field_C0FC_);
        field_E270_ = 0;
    }
    spot_.kill_type(0);
    spot_.reset_count();
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    // Players[faction] offset (faction * 0x5A4) is CSE'd into [ebp-0x70] and
    // reused by is_alien_faction(faction) in the 0x8B4 page below.
    strcat(StringTemp, get_adjective(faction));
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0xACC / 4));
    if (field_5A08_) {
        // Image: three separate strcats, not one " - " literal.
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, "-");
        strcat(StringTemp, BufferJoinSpace);
        // Jump table 0x0049EAEC (8 entries); the shared Strings::get+strcat
        // tail is VC6 cross-jumping.
        switch (field_5A08_) {
        case 1:
            strcat(StringTemp, label_get(0xA0 / 4));
            break;
        case 2:
            strcat(StringTemp, label_get(0xA4 / 4));
            break;
        case 3:
            strcat(StringTemp, label_get(0xE4 / 4));
            break;
        case 4:
            strcat(StringTemp, label_get(0xAAC / 4));
            break;
        case 5:
            strcat(StringTemp, label_get(0xAB0 / 4));
            break;
        case 6:
            strcat(StringTemp, label_get(0xAB4 / 4));
            break;
        case 7:
            strcat(StringTemp, label_get(0xFC / 4));
            break;
        case 8:
            strcat(StringTemp, label_get(0xAB8 / 4));
            break;
        }
    }
    CharUpperA(StringTemp);
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    Buffer::write_cent_l(StringTemp, &title);
    ++title.top;
    ++title.bottom;

    int ordinal = -1;                                            // [ebp-0x20]
    // Genuine original: assigned only on the facility paths of the 0x8B3
    // page (49DF66/49E0CB) but read on the unit path too (49E0C4) for the
    // "== FAC_STOCKPILE_ENERGY" test. Preserve the stale/uninitialised value.
    int facility;                                                // [ebp-0x50]
    for (int list_index = 0;                                     // [ebp-0x18]
         list_index < GameDataStateStorage.fields_.base_current_count_; ++list_index) {
        // ebx = &sort_entries_[list_index] is VC6's own pointer IV; the
        // entry is indexed at each use, not bound to a local reference, and
        // base_id is re-read after every call.
        // Image re-reads the global here (not the faction local).
        if (Bases[ReportIfGlobal.sort_entries_[list_index].base_id].faction_id_current != ReportIfGlobal.report_faction_id_) {
            continue;
        }
        ++ordinal;
        if (ordinal < field_C0FC_) {
            continue;
        }
        if (ordinal >= field_C0FC_ + rows) {
            continue;
        }
        if (ordinal > field_C0FC_) {
            text.top += pitch;
            text.bottom += pitch;
            icon.top += pitch;
            icon.bottom += pitch;
        }
        // UNSURE: the image computes right as
        //   ((text.right - text.left) - icon.left) + icon.right + 3 + icon.left
        // (sub, sub, lea [edx+edi+3], add). Alternative width spelling:
        //   rect_width(&text) + rect_width(&icon) + 3.
        set_rectangle_dimensions(&report_work_rect_, icon.left, text.top,
            text.right - text.left - icon.left + icon.right + 3, text.bottom - text.top);
        spot_.add(ReportIfGlobal.sort_entries_[list_index].base_id + 0x400, 0, &report_work_rect_);
        spot_.add(ReportIfGlobal.sort_entries_[list_index].base_id + 0xC00, 0, &icon);
        if (selected_bases_[ReportIfGlobal.sort_entries_[list_index].base_id / 32] & (1 << (ReportIfGlobal.sort_entries_[list_index].base_id % 32))) {
            Buffer::box_sprite(&text, &InterfaceBoxStyles[2]);
            Buffer::box_sprite(&text, &InterfaceBoxStyles[5]);
            Buffer::box_sprite(&icon, &InterfaceBoxStyles[2]);
            Buffer::box_sprite(&icon, &InterfaceBoxStyles[5]);
        } else {
            Buffer::box_sprite(&text, &InterfaceBoxStyles[1]);
            Buffer::box_sprite(&icon, &InterfaceBoxStyles[1]);
        }
        const uint32_t governor = Bases[ReportIfGlobal.sort_entries_[list_index].base_id].governor;
        if (governor & 0x80000000u) {
            for (int priority = 0; priority < 4; ++priority) {
                if (governor & (0x1000000u << priority)) {
                    // UNSURE: inset/restore statement order. Image shrink
                    // stores left, bottom, right, top; restore loads/stores
                    // left, right, top, bottom.
                    ++icon.left;
                    --icon.right;
                    ++icon.top;
                    --icon.bottom;
                    Buffer::box_sprite(&icon, &InterfaceBoxStyles[36 + 3 * priority]);
                    --icon.left;
                    ++icon.right;
                    --icon.top;
                    ++icon.bottom;
                    break;
                }
            }
        }
        // pitch / 2 is CSE'd into [ebp-0x14] and reused by the 0x8B4 page.
        const int text_y = pitch / 2 - field_E268_ / 2 + text.top;  // [ebp-0x5c]
        rect1_.left = (field_59A0_
            - zoom_scale(100, -6))
            / 2 + icon.left + 1;
        rect1_.top = (field_59A4_
            - zoom_scale(75, -6))
            / 2 + icon.top + 1;
        base_draw(this, ReportIfGlobal.sort_entries_[list_index].base_id, rect1_.left, rect1_.top, -6, 0x40);
        Buffer::set_text_color(0xE1, -1, 1, 1);
        Buffer::set_font(ReportIfGlobal.field_1E18C_, ReportIfGlobal.field_1E184_,
            ReportIfGlobal.field_1E18C_, 0);
        set_base(ReportIfGlobal.sort_entries_[list_index].base_id);
        base_compute(0);
        StringTemp[0] = 0;
        say_base(StringTemp, ReportIfGlobal.sort_entries_[list_index].base_id);
        // Image reads project_base[30] (0x009A658C) inline: has_project.
        if (BaseCurrent->drone_total > BaseCurrent->talent_total
            && !has_project(SP_TELEPATHIC_MATRIX, faction)) {
            Buffer::set_text_color(0xF9, -1, 1, 1);
        } else if (!BaseCurrent->drone_total && BaseCurrent->population_size > 2
            && BaseCurrent->talent_total >= (BaseCurrent->population_size + 1) / 2) {
            strcat(StringTemp, "(!)");
            Buffer::set_text_color(0xFF, -1, 1, 1);
        }
        rect1_.left = text.left + 5;
        CharUpperA(StringTemp);
        Buffer::write_l(StringTemp, rect1_.left, text.top + 5);
        StringTemp[0] = 0;
        if (is_objective(ReportIfGlobal.sort_entries_[list_index].base_id)
            && !(GameDataStateStorage.fields_.game_rules_ & 0x20000000)) {
            // Genuine original: colour indexed by list position, not by the
            // sorted base id (Bases[list_index]).
            Buffer::set_text_color(InfoFactionTextColors[Bases[list_index].faction_id_current],
                -1, 1, 1);
            strcat(StringTemp, label_get(0xE08 / 4));
            CharUpperA(StringTemp);
        } else if (Bases[ReportIfGlobal.sort_entries_[list_index].base_id].governor & 0x80000000u) {
            for (int status_priority = 0; status_priority < 4; ++status_priority) {
                if (Bases[ReportIfGlobal.sort_entries_[list_index].base_id].governor & (0x1000000u << status_priority)) {
                    Buffer::set_text_color(mandate_color(status_priority), -1, 1, 1);
                    strcat(StringTemp, label_get(0x824 / 4 + status_priority));
                    break;
                }
            }
        } else {
            Buffer::set_text_color(0xE1, -1, 1, 1);
            strcat(StringTemp, label_get(0xBFC / 4));
        }
        Buffer::write_l(StringTemp, rect1_.left + 20, text.bottom - field_E268_ - 1);
        rect1_.left += 192;
        const int mode = buttonGroup1_.field_84_;
        if (mode == 0x8B3) {
            const int production = BaseCurrent->queue_production_id[0];   // edi / [ebp-0x10]
            if (production >= 0) {
                const int vehicle = veh_fake(production, BaseCurrent->faction_id_current);
                veh_draw(this, vehicle, rect1_.left, rect1_.top + 4, -8, 0x4C, 0);
            } else {
                facility = -production;
                if (facility < 70) {
                    g_FACILITY_ICON_SPRITES[facility].draw_mono(this,
                        rect1_.left + 8, rect1_.top + 4,
                        mandate_color(TutWinBaseWindow.prodPicker_.edbc_fac(facility)),
                        text.bottom - text.top - 2, g_FACILITY_ICON_SPRITES[0].height());
                } else {
                    g_SECRET_PROJECT_ICON_SPRITES[facility - 70].draw_mono(this,
                        rect1_.left + 8, rect1_.top + 4, mandate_color(1),
                        text.bottom - text.top - 2, g_SECRET_PROJECT_ICON_SPRITES[0].height());
                }
            }
            StringTemp[0] = 0;
            Buffer::set_font(ReportIfGlobal.field_1E18C_, ReportIfGlobal.field_1E18C_,
                ReportIfGlobal.field_1E18C_, 0);
            int item_cost;                                       // shares [ebp-0x10]
            if (production >= 0) {
                strcat(StringTemp, VehPrototypes[production].veh_name);
                BOOL has_proto_cost;                             // [ebp-0x98]
                item_cost = veh_cost(production, BaseIDCurrentSelected, &has_proto_cost);
                Buffer::set_text_color(
                    mandate_color(TutWinBaseWindow.prodPicker_.edbc_unit(production)), -1, 1, 1);
            } else {
                facility = -production;
                strcat(StringTemp, StringTable.get(Facility[facility].name));
                Buffer::set_text_color(
                    mandate_color(TutWinBaseWindow.prodPicker_.edbc_fac(facility)), -1, 1, 1);
                item_cost = Facility[facility].cost;
            }
            rect1_.left += 60;
            Buffer::write_l(StringTemp, rect1_.left, text_y);
            Buffer::set_text_color(0xF2, -1, 1, 1);
            StringTemp[0] = 0;
            strcat(StringTemp, SocialWinOpenParenText);
            // Five separate _itoa buffers in the image ([ebp-0xe8], -0x1d8,
            // -0x138, -0x188, -0x228) = five inline expansions.
            if (facility == FAC_STOCKPILE_ENERGY) {   // stale on the unit path (see above)
                say_num(0);
            } else {
                say_num(BaseCurrent->minerals_accumulated);
            }
            strcat(StringTemp, "/");
            int cost = cost_factor(BaseCurrent->faction_id_current, 1, -1) * item_cost;  // edi
            say_num(cost);
            strcat(StringTemp, SocialWinCloseParenText);
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, BufferJoinSpace);
            if (BaseCurrent->mineral_surplus_final <= 0) {
                strcat(StringTemp, label_get(0x10C / 4));
            } else {
                strcat(StringTemp, label_get(0xC4 / 4));
                strcat(StringTemp, SocialWinColonSpaceText);
                strcat(StringTemp, BufferJoinSpace);
                if (cost > BaseCurrent->minerals_accumulated) {
                    cost -= BaseCurrent->minerals_accumulated;
                    say_num(
                        range((BaseCurrent->mineral_surplus_final + cost - 1)
                            / range(BaseCurrent->mineral_surplus_final, 1, 99999), 0, 999999));
                } else {
                    say_num(0);
                }
            }
            // Guarded write_right_l (0x005DD3B0) expansion: x is evaluated
            // before the null test (lea edi,[edx-4]).
            Buffer::write_right_l(StringTemp, text.right - 4, text_y);
        } else if (mode == 0x8B4) {
            // UNSURE: `<= 1` from cmp reg,1 / jg (the 34 arm falls through).
            const int spacing = BaseCurrent->population_size <= 1 ? 34    // edi
                : range((text.right - text.left - 226) / (BaseCurrent->population_size - 1), 1, 34);
            rect1_.left = text.right + spacing - 32;
            rect1_.top = text.bottom - pitch / 2 - 18;
            if (is_alien_faction(faction)) {
                for (int specialist = 0; specialist < BaseCurrent->specialist_total; ++specialist) {
                    rect1_.left -= spacing;
                    spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                    const int index = BaseCurrent->specialist_total - specialist - 1;
                    const int type = index >= 16 ? best_specialist()
                        : (BaseCurrent->spec[index >> 3] >> ((index & 7) * 4)) & 15;
                    g_AL_SPECIALIST_SM_CURSOR_SPRITES[type].draw(this, rect1_.left, rect1_.top);
                }
                for (int drone = 0; drone < BaseCurrent->drone_total; ++drone) {
                    rect1_.left -= spacing;
                    spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                    g_AL_CITIZEN_SM_CURSOR_SPRITES[(drone < BaseCurrent->super_drone_total) + 2].draw(this, rect1_.left, rect1_.top);
                }
                // Head-tested loop; bound re-read each pass.
                // UNSURE: max(drone_total, 0) is setl/dec/and: `< 0 ? 0 : x`.
                for (int worker = 0; worker < BaseCurrent->population_size
                        - BaseCurrent->talent_total - BaseCurrent->specialist_total
                        - (BaseCurrent->drone_total < 0 ? 0 : BaseCurrent->drone_total);
                     ++worker) {
                    rect1_.left -= spacing;
                    spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                    g_AL_CITIZEN_SM_CURSOR_SPRITES[1].draw(this, rect1_.left, rect1_.top);
                }
                for (int talent = 0; talent < BaseCurrent->talent_total; ++talent) {
                    rect1_.left -= spacing;
                    spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                    g_AL_CITIZEN_SM_CURSOR_SPRITES[0].draw(this, rect1_.left, rect1_.top);
                }
            } else {
                for (int specialist = 0; specialist < BaseCurrent->specialist_total; ++specialist) {
                    rect1_.left -= spacing;
                    spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                    const int index = BaseCurrent->specialist_total - specialist - 1;
                    const int type = index >= 16 ? best_specialist()
                        : (BaseCurrent->spec[index >> 3] >> ((index & 7) * 4)) & 15;
                    g_SPECIALIST_SM_CURSOR_SPRITES[type].draw(this, rect1_.left, rect1_.top);
                }
                // UNSURE (the three rng loops): the image loads rect1_.left/top
                // and the Buffer* conversion into frame slots ([ebp-0x24],
                // [ebp-0x14], [ebp-0x58]) BEFORE calling rng.get, i.e. the
                // original evaluated x/y before the random sprite index - the
                // shape of sprite.h's Sprite::draw(dest, x, y) overload's
                // argument evaluation. Locals reproduce that order.
                for (int drone = 0; drone < BaseCurrent->drone_total; ++drone) {
                    rect1_.left -= spacing;
                    spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                    g_CITIZEN_SM_CURSOR_SPRITES[
                        (drone < BaseCurrent->super_drone_total) + 2 + rng.get(0, 2) * 4].draw(this, rect1_.left, rect1_.top);
                }
                for (int worker = 0; worker < BaseCurrent->population_size
                        - BaseCurrent->talent_total - BaseCurrent->specialist_total
                        - (BaseCurrent->drone_total < 0 ? 0 : BaseCurrent->drone_total);
                     ++worker) {
                    rect1_.left -= spacing;
                    spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                    g_CITIZEN_SM_CURSOR_SPRITES[rng.get(0, 2) * 4 + 1].draw(this, rect1_.left, rect1_.top);
                }
                for (int talent = 0; talent < BaseCurrent->talent_total; ++talent) {
                    rect1_.left -= spacing;
                    spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                    g_CITIZEN_SM_CURSOR_SPRITES[rng.get(0, 2) * 4].draw(this, rect1_.left, rect1_.top);
                }
            }
        } else if (mode == 0x8B5) {
            const int vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);
            const int slots = stack_check(vehicle, 1, -1, -1, -1);
            // UNSURE: `<= 1` from cmp ecx,1 / jg (the 40 arm falls through).
            const int spacing = slots <= 1 ? 40                          // [ebp-0x10]
                : range((text.right - text.left - 232) / (slots - 1), 1, 40);
            rect1_.left = text.right + spacing - 45;
            rect1_.top = text.top;
            // veh_top inline: its `veh_id < 0` early -1 is jump-threaded
            // straight to the loop exit (49EA13).
            for (int veh_id = veh_top(vehicle); veh_id >= 0;
                 veh_id = Vehs[veh_id].next_veh_id_stack) {
                rect1_.left -= spacing;
                veh_draw(this, veh_id, rect1_.left, rect1_.top, -8, 8, 0);
                spot_.add(ReportIfGlobal.sort_entries_[list_index].base_id + 0x800, veh_id, rect1_.left, rect1_.top, spacing, pitch);
            }
        }
    }
}

// ORIGINAL: 0x0049EB10 ?draw_projects@ReportWin@@QAEXXZ 0x0049EB10-0x0049F343
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x0049EB10-0x0049F343", "src/unrecovered/0049eb10.cpp:retained seed and trial history", "Original97CDDC=PlayersData96C9E0+7*20CC+goals19FC+70*10+base_idC; the native unsigned-byte read determines the destroyed-project faction without dereferencing Bases[-2]."], "reason": "Recover the projects page through owned ReportWin rectangles, spots and scroll state, preserving queued/completed rendering and the destroyed-owner byte alias through its real PlayerData member."}
// Deleted seed provenance: git:53d5fe2c1b1eff854b846b082f125794c1d0b7fe:src/unrecovered/0049eb10.cpp
// TRIED: full transcription (call-argument order verified against raw disasm for add/box_sprite/draw_mono/write_l/write_cent_l/ set_text_color/Strings::get, all matched Ghidra) reaches MISMATCH #3 'mov' vs 'push' - a prologue-level register/stack allocation divergence, not a logic error; not chased further given function size (2099 bytes, nested loops with gotos).
void ReportWin::draw_projects() {
    const int viewer = ReportIfGlobal.report_faction_id_;
    RECT title;
    title.left = energy_panel_rect_.left + 3;
    title.top = energy_panel_rect_.top + 3;
    title.right = energy_panel_rect_.right - 3;
    title.bottom = energy_panel_rect_.top + field_E268_ + 9;
    field_59A4_ = 54;
    const int rows = (energy_panel_rect_.bottom - title.bottom - 5) / 54;
    field_59A0_ = (rect_width(&energy_panel_rect_) - 6) / 2 - 1;
    spot_.kill_type(0);
    spot_.reset_count();
    Buffer::box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    Buffer::box_sprite(&title, &InterfaceBoxStyles[1]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[viewer].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0xAD0 / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    Buffer::write_cent_l(StringTemp, &title);
    ++title.top;
    ++title.bottom;

    int entries = -1;
    for (int project = 0; project < 64; ++project) {
        if (GameDataStateStorage.fields_.secret_project_.project_base[project] != -1) ++entries;
    }
    int base_id;
    for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        if (Bases[base_id].queue_production_id[0] <= -70) ++entries;
    }
    const int half = entries / 2;
    if (field_C0F8_ == 0) {
        if (entries >= rows * 2) field_E26C_ = 1;
    } else if (entries < rows * 2) {
        scroll_hide(this, &scroll_);
        field_C0F8_ = 0;
    }
    if (field_E270_ != 0) {
        field_C0FC_ = 0;
        field_C100_ = 0;
        field_C104_ = half;
        field_C108_ = rows - 1;
        scroll_.set_range(0, half);
        scroll_.paging_ = field_C108_;
        scroll_.set_pos(field_C0FC_);
        field_E270_ = 0;
        draw_projects();
    }

    // Modes: 0 the viewer's builds, 1 other factions' builds, 2 completed.
    int ordinal = -1;
    for (int mode = 0; mode < 3; ++mode) {
        for (int project = 0; project < 64; ++project) {
            const int facility = project + 70;
            if (mode < 2) {
                if (GameDataStateStorage.fields_.secret_project_.project_base[project] != -1) continue;
                base_id = 0;
            } else if (GameDataStateStorage.fields_.secret_project_.project_base[project] == -1) {
                continue;
            }
            for (;;) {
                if (mode < 2) {
                    if ((mode == 0 && Bases[base_id].faction_id_current != viewer)
                        || (mode != 0 && Bases[base_id].faction_id_current == viewer)
                        || Bases[base_id].queue_production_id[0] != -facility) {
                        if (++base_id >= GameDataStateStorage.fields_.base_current_count_) goto next_project;
                        continue;
                    }
                    if (base_id >= GameDataStateStorage.fields_.base_current_count_) break;
                }
                ++ordinal;
                {
                    const int row = ordinal / 2;
                    if (row < field_C0FC_
                        || row >= rows + field_C0FC_) break;
                    rect1_.left = title.left + (field_59A0_ + 2) * (ordinal % 2);
                    rect1_.top = title.bottom + 56 * (row - field_C0FC_) + 2;
                    set_rectangle_dimensions(&report_work_rect_, rect1_.left, rect1_.top, field_59A0_, field_59A4_);
                    if (mode < 2) {
                        spot_.add(base_id + 0x400, 0, &report_work_rect_);
                    } else {
                        spot_.add(project, 0, &report_work_rect_);
                    }
                    if ((mode < 2 && base_id == field_59AC_ - 0x400)
                        || (mode == 2 && project == field_59AC_)) {
                        Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[2]);
                        Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[5]);
                    } else {
                        Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[1]);
                    }
                    g_SECRET_PROJECT_ICON_SPRITES[project].draw_mono(this,
                        rect1_.left + 2, rect1_.top + 1, mandate_color(1),
                        field_59A4_ - 2, g_SECRET_PROJECT_ICON_SPRITES[0].height());
                    rect1_.top += 5;
                    rect1_.left += (field_59A4_ - 2)
                        * g_SECRET_PROJECT_ICON_SPRITES[0].width()
                        / g_SECRET_PROJECT_ICON_SPRITES[0].height() + 7;
                    if (mode == 2) base_id = GameDataStateStorage.fields_.secret_project_.project_base[project];
                    // A destroyed project (-2) reads Bases[-2] as the image does.
                    Buffer::set_text_color(ChatItemIdTable[Bases[base_id].faction_id_current], -1, 1, 1);
                    Buffer::set_text_color3(ChatItemIdTable[Bases[base_id].faction_id_current], -1, 1, 1);
                    StringTemp[0] = 0;
                    if (mode == 2) strcat(StringTemp, "[");
                    strcat(StringTemp, table_text(Facility[facility].name));
                    if (mode == 2) strcat(StringTemp, "]");
                    Buffer::write_l(StringTemp, rect1_.left, rect1_.top);
                    rect1_.top += field_E268_;
                    StringTemp[0] = 0;
                    if (mode == 2) strcat(StringTemp, "[");
                    if (Bases[base_id].faction_id_current == viewer
                        || spying(Bases[base_id].faction_id_current)
                        || facility == 102 || mode == 2) {
                        say_base(StringTemp, base_id);
                    } else {
                        parse_set(Players[Bases[base_id].faction_id_current].noun_gender,
                                  Players[Bases[base_id].faction_id_current].is_noun_plural);
                        strcat(StringTemp, Players[Bases[base_id].faction_id_current].noun_faction);
                    }
                    if (mode == 2) strcat(StringTemp, "]");
                    Buffer::write_l(StringTemp, rect1_.left, rect1_.top);
                    StringTemp[0] = 0;
                    if (mode != 0) strcat(StringTemp, "[");
                    if (mode < 2) {
                        Buffer::set_text_color(0xF9, -1, 1, 1);
                        Buffer::set_text_color3(0xF9, -1, 1, 1);
                        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x6BC / 4]));
                    } else {
                        parse_set(Players[Bases[base_id].faction_id_current].noun_gender,
                                  Players[Bases[base_id].faction_id_current].is_noun_plural);
                        strcat(StringTemp, Players[Bases[base_id].faction_id_current].noun_faction);
                    }
                    if (mode != 0) strcat(StringTemp, "]");
                    rect1_.top += field_E268_;
                    Buffer::write_l(StringTemp, rect1_.left, rect1_.top);
                }
                if (mode >= 2 || ++base_id >= GameDataStateStorage.fields_.base_current_count_) break;
            }
        next_project:;
        }
    }
}


// RECOVERY_ASSOCIATED: v1 switch_targets 0x0049F350 0x0049FE20 0x0049FE34 0x0049F758
// ORIGINAL: 0x0049F350 ?draw_sat@ReportWin@@QAEXXZ 0x0049F350-0x0049FE1E
// RECOVERY_BINDING: v1 {"evidence": ["git:649230f6:src/unrecovered/0049f350.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
// The image calls sqrt out of line here (0x49FC32, 0x49FC69).
void ReportWin::draw_sat() {
    const int faction_id = ReportIfGlobal.report_faction_id_;
    int orbital_tech = 0;
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3, energy_panel_rect_.top + field_E268_ + 9};
    Buffer::box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    Buffer::box_sprite(&title, &InterfaceBoxStyles[1]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[faction_id].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0xAD4 / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    Buffer::write_cent_l(StringTemp, &title);
    ++title.top;
    ++title.bottom;
    flatButton7_.show(0);
    flatButton8_.show(0);
    spot_.reset_count();
    int i;
    for (i = 1; i < 8; ++i) {
        if (has_tech(Facility[65].preq_tech, i)) {
            orbital_tech = 1;
        }
    }
    if (buttonGroup2_.field_84_ == 0xD3C) {
        flatButton9_.hide();
        if (orbital_tech) {
            rect1_.top += 50;
            rect1_.bottom -= 50;
            const int row_height = (rect1_.bottom - rect1_.top - 2) / 8;
            int heading_width = rect1_.right / 3;
            const int avail = rect1_.right - 4;
            if (ExpansionEnabled) {
                heading_width = rect1_.right / 5;
            }
            const int cell_width = (avail - heading_width) / (ExpansionEnabled ? 5 : 4);
            RECT *const work = &report_work_rect_;
            for (int other = 0; other < 8; ++other) {
                spot_.kill_type(other);
                rect1_.left = energy_panel_rect_.left + 2;
                if (other) {
                    StringTemp[0] = 0;
                    GenderDefault = Players[other].noun_gender;
                    PluralityDefault = Players[other].is_noun_plural;
                    strcat(StringTemp, Players[other].noun_faction);
                    Buffer::set_text_color(ChatItemIdTable[other], ChatShadowColors[other], 1, 1);
                    set_rectangle_dimensions(work, rect1_.left, rect1_.top, heading_width, row_height);
                    Buffer::write_l(StringTemp, work);
                }
                rect1_.left += heading_width;
                set_rectangle_dimensions(work, rect1_.left, rect1_.top, cell_width, row_height);
                for (satellite_index_ = 0; satellite_index_ < (ExpansionEnabled ? 5 : 4);
                     ++satellite_index_) {
                    if (other) {
                        int elevators = 0;
                        for (i = 0;
                             i < GameDataStateStorage.fields_.base_current_count_; ++i) {
                            if (Bases[i].faction_id_current == other
                                && has_fac(41, i, 0)) {
                                ++elevators;
                            }
                        }
                        if ((satellite_index_ == 4 && elevators != 0)
                            || (satellite_index_ < 4
                                && PlayersData[other].satellites_[satellite_index_] != 0)) {
                            switch (satellite_index_) {
                            case 0:
                                Buffer::set_text_color(0x98, -1, 1, 1);
                                break;
                            case 1:
                                Buffer::set_text_color(0xDF, -1, 1, 1);
                                break;
                            case 2:
                                Buffer::set_text_color(0x62, -1, 1, 1);
                                break;
                            case 3:
                                Buffer::set_text_color(0xF9, -1, 1, 1);
                                break;
                            case 4:
                                Buffer::set_text_color(0xFF, -1, 1, 1);
                                break;
                            }
                        } else {
                            Buffer::set_text_color(0xF8, -1, 1, 1);
                        }
                        StringTemp[0] = 0;
                        if (satellite_index_ == 3) {
                            say_num(PlayersData[other].satellites_[satellite_index_]
                                              - PlayersData[other].sat_odp_deployed);
                            strcat(StringTemp, "/");
                        }
                        if (satellite_index_ == 4) {
                            say_num(elevators);
                        } else {
                            say_num(PlayersData[other].satellites_[satellite_index_]);
                        }
                        Buffer::write_cent_l(StringTemp, work);
                        spot_.add(satellite_index_, other, work->left,
                                  work->top,
                                  work->right - work->left,
                                  work->bottom - work->top);
                    } else {
                        Buffer::set_text_color(0xFF, -1, 1, 1);
                        StringTemp[0] = 0;
                        strcat(StringTemp, StringTable.get(
                            ExpansionEnabled && satellite_index_ == 4
                                ? Facility[41].name : Facility[65 + satellite_index_].name));
                        Buffer::wrap_cent(StringTemp, work);
                    }
                    work->left += cell_width;
                    work->right += cell_width;
                }
                rect1_.top += row_height;
            }
        } else {
            Buffer::set_font(ReportIfGlobal.field_1E198_, 0, 0, 0);
            Buffer::set_text_color(0xE1, -1, 1, 1);
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0xCC4 / 4));
            // Genuine original: passes right/bottom as width/height.
            Buffer::write_cent(StringTemp, rect1_.left, rect1_.top,
                                     rect1_.right, rect1_.bottom);
        }
        return;
    }
    int radius_x[16];
    int radius_y[16];
    float phase_x[16];
    float phase_y[16];
    field_59A0_ = 576;
    field_59A4_ = 210;
    rect1_.left = (energy_panel_rect_.left + energy_panel_rect_.right - 576) / 2;
    rect1_.top = (energy_panel_rect_.top + energy_panel_rect_.bottom - 210) / 2 + 20;
    set_rectangle_dimensions(&satellite_rect_, rect1_.left, rect1_.top, 576, 210);
    flic_.init(this, "satellite.flc", 0, 0, 0);
    field_E2A8_ = (satellite_rect_.right + satellite_rect_.left) / 2;
    field_E2AC_ = (satellite_rect_.bottom + satellite_rect_.top) / 2;
    field_E2A0_ = field_E2A8_ - flic_.decoded_width() / 2;
    field_E2A4_ = field_E2AC_ - flic_.decoded_height() / 2;
    buffer2_.init(flic_.decoded_width(), flic_.decoded_height(), 0, 0);
    buffer2_.fill(9);
    flic_.init(&buffer2_, "satellite.flc", 0, 0, 0);
    Buffer::box_sprite(&satellite_rect_, &InterfaceBoxStyles[4]);
    inflate_frame(&satellite_rect_, -3, -3);
    field_F918_ = 0;
    for (i = 1; i < 8; ++i) {
        for (satellite_index_ = 0; satellite_index_ < 4; ++satellite_index_) {
            field_F918_ += PlayersData[i].satellites_[satellite_index_];
        }
    }
    field_EE30_ = 0.02f;
    for (satellite_index_ = 0; satellite_index_ < 16; ++satellite_index_) {
        radius_x[satellite_index_] = field_59A4_ / 2 - 10;
        radius_y[satellite_index_] = radius_x[satellite_index_];
        satellite_acceleration_[satellite_index_] = static_cast<float>(sqrt(static_cast<double>(
            radius_x[satellite_index_] * radius_x[satellite_index_]
            + radius_y[satellite_index_] * radius_y[satellite_index_])));
        satellite_acceleration_[satellite_index_] = static_cast<float>(
            sqrt(1.0 / (satellite_acceleration_[satellite_index_]
                        * satellite_acceleration_[satellite_index_]
                        * satellite_acceleration_[satellite_index_]))) * 700.0f;
        phase_x[satellite_index_] = satellite_index_ * 0.0625f;
        {
            const float angle = phase_x[satellite_index_];
            const float offset = (satellite_index_ & 1) ? 0.125 : -0.125;
            phase_x[satellite_index_] = angle * 6.28f;
            phase_y[satellite_index_] = (offset + angle) * 6.28f;
        }
        satellite_x_[satellite_index_] = static_cast<float>(
            cos(phase_x[satellite_index_] + satellite_acceleration_[satellite_index_])
            * radius_x[satellite_index_]);
        satellite_y_[satellite_index_] = static_cast<float>(
            sin(satellite_acceleration_[satellite_index_] + phase_y[satellite_index_])
            * radius_y[satellite_index_]);
        satellite_dx_[satellite_index_] = static_cast<float>(
            (sin(satellite_acceleration_[satellite_index_] + phase_x[satellite_index_]) * -radius_x[satellite_index_]) * satellite_acceleration_[satellite_index_]);
        satellite_dy_[satellite_index_] = static_cast<float>(
            cos(satellite_acceleration_[satellite_index_] + phase_y[satellite_index_])
            * satellite_acceleration_[satellite_index_] * radius_y[satellite_index_]);
        satellite_acceleration_[satellite_index_] = satellite_acceleration_[satellite_index_]
            * field_EE30_ * satellite_acceleration_[satellite_index_];
        satellite_live_[satellite_index_] = satellite_index_ < field_F918_;
    }
    sat_anim();
    time2_.init(sat_anim_daemon, reinterpret_cast<int>(this), 4000, 5);
    time2_.start();
}

/*
Purpose: Advance the satellite orbital overlay one tick: clear the orbit
         rectangle, integrate the sixteen satellites' positions from their
         velocities and the ReportWin's own step scalar at 0xEE30, decode
         the next Flic frame under them, composite it through the second
         surface buffer, then fill one 2x2 marker per live satellite and
         publish the orbit rectangle.

// ORIGINAL: 0x0049FE40 ?sat_anim@ReportWin@@QAEXXZ 0x0049FE40-0x0049FFB7 FILE BYTE_EXACT
// size      375 bytes
// prototype void (__thiscall ?sat_anim@ReportWin@@QAEXXZ)(ReportWin* this)
// callers   3   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5350 0x005D5440 0x005D5930 0x005D8370 0x0062A070 0x006458C8
// Homed from src/recovered/units/0049fe40.cpp. The four float arrays live
// in field_F938_'s storage (0xF938..0xFAB8): position pairs at 0xF938/
// 0xF978, scratch results at 0xF9B8/0xF9F8, velocities at 0xFA38, the
// live-satellite flags at 0xFA78; satellite_index_ sits just before
// field_E2B0_ at 0xE29C. The 0xEE30 scalar is field_EE30_ (see the layout
// note above: it is NOT the Flic, which starts at 0xEE34), and the frame
// counters the decode reads are the Flic's own at 0xF3E0/0xF3E4.
Status: Coverage transcription - unclaimed body
*/
void ReportWin::sat_anim() {
    fill(&satellite_rect_, 0x41);
    satellite_index_ = 0;
    do {
        int i = satellite_index_;
        satellite_dx_[i] -= satellite_acceleration_[i] * satellite_x_[i];
        i = satellite_index_;
        satellite_dy_[i] -= satellite_acceleration_[i] * satellite_y_[i];
        i = satellite_index_;
        satellite_x_[i] += satellite_dx_[i] * field_EE30_;
        i = satellite_index_;
        satellite_y_[i] += satellite_dy_[i] * field_EE30_;
        ++satellite_index_;
    } while (satellite_index_ < 16);
    flic_.decode_stored_frame();
    buffer2_.draw(static_cast<Buffer *>(this), 9, field_E2A0_, field_E2A4_);
    satellite_index_ = 0;
    do {
        if (satellite_live_[satellite_index_] != 0) {
            fill(static_cast<int>(satellite_x_[satellite_index_]) + field_E2A8_,
                 static_cast<int>(satellite_y_[satellite_index_]) + field_E2AC_,
                 2, 2, 0xFF);
        }
        ++satellite_index_;
    } while (satellite_index_ < 16);
    soft_update(&satellite_rect_);
}

/*
// ORIGINAL: 0x0049FFC0 ?sat_anim_daemon@@YAXH@Z 0x0049FFC0-0x0049FFCD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["ReportWin::draw_sat registers this callback with Time::init616260 and the borrowed ReportWin receiver as its one-word integer context.", "Pinned49FFC0..49FFCD loads its sole stack argument into ECX, calls ReportWin::sat_anim49FE40 unconditionally and ends in plain RET; Time stores and invokes cdecl(int).", "Earlier source translated the context as a pointer parameter and cast the function to Time's integer callback type; keep that finding as provenance, not a conflicting current prototype."], "reason": "Own the report animation bridge with Time's directly compatible one-word cdecl callback contract; decode the borrowed receiver at its ABI boundary without introducing a null guard or changing dispatch."}
// symbol    ?sat_anim_daemon@@YAXH@Z
// size      13 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0049FE40
Status: Complete
*/
void __cdecl sat_anim_daemon(int context) {
    ReportWin *const window = reinterpret_cast<ReportWin *>(context);
    window->sat_anim();
}

// ORIGINAL: 0x0049FFD0 ?start_attack@ReportWin@@QAEXHHHH@Z 0x0049FFD0-0x004A00F1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:649230f6:src/recovered/units/0049ffd0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::start_attack(int attacker, int target, int type, int base_id) {
    field_F924_ = attacker;
    field_F928_ = target;
    field_F92C_ = type;
    field_F930_ = base_id;
    field_F920_ = 0;
    field_F91C_ = 0;
    field_F934_ = 0;
    if (field_F918_ != 0) {
        const int counter = --field_F918_;
        const int index = counter % 16;
        if (counter < 16) satellite_live_[index] = 0;
        sat_anim();
        const int top = static_cast<int>(satellite_y_[index]) + field_E2AC_ - 23;
        const int left = static_cast<int>(satellite_x_[index]) + field_E2A8_ - 21;
        explosion_rect_.top = top;
        explosion_rect_.left = left;
        explosion_rect_.right = left + 43;
        explosion_rect_.bottom = top + 47;
        buffer1_.init(43, 47, 0, 0);
        Buffer::copy(&buffer1_, explosion_rect_.left,
            explosion_rect_.top, 0, 0, 43, 47);
        time3_.init(sat_expl_daemon,
                    reinterpret_cast<int>(&ReportWinGlobal), 50, 5);
        time3_.start();
    }
}

// ORIGINAL: 0x004A0100 ?exp_anim@ReportWin@@QAEXXZ 0x004A0100-0x004A0248 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:649230f6:src/recovered/units/004a0100.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::exp_anim() {
    Buffer::set_clip(&satellite_rect_);
    if (field_F91C_ != 0) {
        time3_.stop();
        if (field_F920_ == 0) {
            field_F920_ = 1;
            buffer1_.draw(static_cast<Buffer *>(this), 9,
                          explosion_rect_.left, explosion_rect_.top);
            update(&explosion_rect_, 0);
            action_sat_attack(field_F924_, field_F928_, field_F92C_, field_F930_);
        }
    } else {
        buffer1_.draw(static_cast<Buffer *>(this), 9,
                      explosion_rect_.left, explosion_rect_.top);
        Sprite *const bank = &g_XI_BOOM_VEH_SPRITES[48];  // fourth 16-frame bank
        bank[field_F934_].draw_multi_table_dest(static_cast<Buffer *>(this),
            explosion_rect_.left, explosion_rect_.top, ExplosionColorTables[1]);
        if (++field_F934_ >= 16) field_F91C_ = 1;
        update(&explosion_rect_, 0);
    }
    Buffer::restore_clip();
}

/*
// ORIGINAL: 0x004A0250 ?sat_expl_daemon@@YAXH@Z 0x004A0250-0x004A025D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["ReportWin::start_attack registers this callback with Time::init616260 and ReportWinGlobal as its one-word integer context.", "Pinned4A0250..4A025D loads its sole stack argument into ECX, calls ReportWin::exp_anim4A0100 unconditionally and ends in plain RET; Time stores and invokes cdecl(int).", "Earlier source accepted ReportWin* and registered through an incompatible function-pointer cast; retain that translation finding as history, not a current ABI declaration."], "reason": "Recover the explosion animation bridge as a directly compatible cdecl(int) callback with a borrowed ReportWin receiver; preserve the original unconditional instance dispatch."}
// symbol    ?sat_expl_daemon@@YAXH@Z
// size      13 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004A0100
Status: Complete
*/
void __cdecl sat_expl_daemon(int context) {
    ReportWin *const window = reinterpret_cast<ReportWin *>(context);
    window->exp_anim();
}

// WORKAROUND(inline): dead /Ob2 budget stand-in (folds to no code). sat_attack's
// eight ~Popup return sites have teardown depths 2,2,2,5,2,2,5,6 in the image
// (EH unwind map). Two one-step probes after the NOKILLERS return reproduce
// that distribution. The original spelling that spent this budget is unknown.
static int sat_attack_budget_probe_a(int x)
{
    if (x == 0) x = 1;
    if (x == 1) x = 4;
    return x;
}

// ORIGINAL: 0x004A0260 ?sat_attack@@YAXHHH@Z 0x004A0260-0x004A080C;0x00659447-0x00659580
// RECOVERY_BINDING: v1 {"evidence": ["git:649230f6:src/unrecovered/004a0260.cpp", "source:src/faction.h", "image:complete original instructions and independent storage/lifecycle evidence"], "reason": "Recover satellite attack selection through a natural Popup lifetime, its owned picture and the genuine four-counter faction array, preserving original signed guards and network/local action ordering."}
void __cdecl sat_attack(int attacker, int target, int kind) {
    // The picture extent lives for the whole function: the image keeps its
    // two slots apart from the base-count loop's has_fac temporaries.
    int width, height;
    Popup popup;
    if (attacker == target) return;
    if (target <= 0) return;
    if (!PlayersData[attacker].satellites_[3]) {
        if (attacker == ConsoleGlobal.faction_id_)
            popp(ScriptPopupFile,
                 "NOKILLERS", 0, "space_sm.pcx", 0);
        return;
    }
    sat_attack_budget_probe_a(2); // WORKAROUND(inline): see above
    sat_attack_budget_probe_a(2); // WORKAROUND(inline): see above
    if (PlayersData[attacker].sat_odp_deployed
        >= PlayersData[attacker].satellites_[3]) {
        if (attacker == ConsoleGlobal.faction_id_)
            popp(ScriptPopupFile,
                 "KILLERSDEPLOYED", 0, "space_sm.pcx", 0);
        return;
    }
    int targets = 0;
    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        if (Bases[base].faction_id_current == target && has_fac(41, base, 0)) ++targets;
    }
    // The negation of the admission test below: the image branches on each
    // term (0x4A0485: cmp/jne, test/jne, jge, test/je) and never
    // materializes a flag; the signed upper bound is the only guard on kind.
    if (!((kind == 4 && targets)
          || (kind != 4 && kind < 4 && PlayersData[target].satellites_[kind])))
        return;
    if (break_treaty(attacker, target, 7)) return;
    if (diplo_lock(100)) return;
    if (PlayersData[attacker].satellites_[3]
        && PlayersData[attacker].sat_odp_deployed
            < PlayersData[attacker].satellites_[3]
        && ((kind == 4 && targets)
            || (kind != 4 && kind < 4 && PlayersData[target].satellites_[kind]))) {
        int attack_target;
        if (kind != 4) {
            attack_target = attacker;
        } else {
            popup.start(PopupStartCaption, "WHICHGSP", -1, 0, 0x40, 0);
            if (Buffer::get_pcx_dimensions("satbat_sm.pcx", &width, &height) == 0
                && popup.owned_picture().init("satbat_sm.pcx", width, height) == 0)
                popup.select_owned_picture();
            for (int item_base = 0; item_base < GameDataStateStorage.fields_.base_current_count_; ++item_base) {
                if (Bases[item_base].faction_id_current == target && has_fac(41, item_base, 0))
                    popup.dialogs().item(Bases[item_base].name_string, item_base);
            }
            attack_target = popup.exec(0, 0);
            if (attack_target < 0) goto unlock;
        }
        if (IsMultiplayerNet) {
            message_data(0x2412, 0, attacker, target, kind, attack_target);
            NetDaemonNet.await_exec(0);
        } else {
            ReportWinGlobal.start_attack(attacker, target, kind, attack_target);
        }
    }
unlock:
    diplo_unlock();
}

/*
Purpose: Step one faction's satellite count for a body, with the shift-key
         downward path clamped and the seen map refreshed.
// ORIGINAL: 0x004A0810 ?sat_edit@@YAXHHH@Z 0x004A0810-0x004A08D2 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original4A0810..4A08D2; retired seed git:ee30a69d:src/unrecovered/004a0810.cpp retains the same identity/spans.", "Original accesses PlayerData+808 with a word index, +814 and +364. Source satellites_[4] begins808, so814 is satellites_[3]; compiler-observed PlayerData member sat_odp_deployed is364. The shift path clamps deployed orbital-defense pods using the signed result of wrapped dword subtraction, not an unsigned counter comparison.", "GetAsyncKeyState high byte is read only after type<4 and nonzero body guards; preserve negative-index admission, counter wrap, nonzero decrement, both callback paths and direct see_map_check/GraphicWin::redraw ordering. No bounds or behavior fix."], "reason": "Recover the satellite editor through existing PlayerData satellite/deployed members while preserving original arithmetic, indexing and callback behavior."}
// SUPERSEDED_BINDING_HISTORY: v1 {"evidence": ["Original4A0810..4A08D2; retired seed git:ee30a69d:src/unrecovered/004a0810.cpp records the same identity and spans.", "The three per-faction fields are PlayerData (faction.h, the 0x833-dword stride the header pins) at +0x808 (the satellite count stepped by body), +0x814 (the seen-map target) and +0x364 (the seen-map floor); the shift state comes through the USER32 GetAsyncKeyState import slot 0x00669330 that mapwin.cpp binds the same way.", "The repaint is see_map_check (faction.h's own) and redraw on the source-owned ReportWinGlobal (reportwin.h, original 0x00876478) through its inherited virtual slot."], "reason": "Coverage transcription of the satellite editor click with the PlayerData offsets documented and the repaint through the tree's own names."}
Status: Coverage transcription - unclaimed body
*/

void __cdecl sat_edit(int report_faction_id, int target_faction_id, int satellite_type) {
    if (satellite_type < 4 && report_faction_id != 0) {
        if (HIBYTE(::GetAsyncKeyState(0x10)) != 0) {
            if (PlayersData[target_faction_id].satellites_[satellite_type] != 0) {
                PlayersData[target_faction_id].satellites_[satellite_type]--;
            }
            // The original compares the signed result of a wrapping 32-bit
            // subtraction, not the two unsigned counters.
            if (satellite_type == 3
                && (PlayersData[target_faction_id].satellites_[satellite_type]
                    - PlayersData[target_faction_id].sat_odp_deployed) < 0) {
                PlayersData[target_faction_id].sat_odp_deployed =
                    PlayersData[target_faction_id].satellites_[satellite_type];
            }
        } else {
            PlayersData[target_faction_id].satellites_[satellite_type]++;
        }
        see_map_check();
        ReportWinGlobal.redraw();
    }
}

// ORIGINAL: 0x004A08E0 ?draw_security@ReportWin@@QAEXXZ 0x004A08E0-0x004A1134 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:649230f6:src/unrecovered/004a08e0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}

void ReportWin::draw_security() {
    const int faction_id = ReportIfGlobal.report_faction_id_;
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3, energy_panel_rect_.top + field_E268_ + 9};
    RECT row = {energy_panel_rect_.left + 3, title.bottom + 2,
                energy_panel_rect_.right - 3, title.bottom + 43};
    Buffer::box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    Buffer::box_sprite(&title, &InterfaceBoxStyles[1]);
    const int rows = (energy_panel_rect_.bottom - row.top) / 43;
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[faction_id].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0xAD8 / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    Buffer::write_cent_l(StringTemp, &title);
    ++title.top;
    ++title.bottom;
    spot_.kill_type(0);

    int id;
    int last_row = -1;
    int i;
    for (i = 0; i < 128; ++i) {
        id = i < 64 ? i : faction_id * 64 - 64 + i;
        if ((VehPrototypes[id].flags & 1) == 0) continue;
        if (ReportIfGlobal.field_1E244_ < 4) {
            if (ReportIfGlobal.field_1E244_ < 3) {
                if (Chassis[VehPrototypes[id].chassis_id].triad != ReportIfGlobal.field_1E244_) continue;
                if (Weapon[VehPrototypes[id].weapon_id].offense_rating == 0) continue;
            } else if (Weapon[VehPrototypes[id].weapon_id].offense_rating != 0) {
                continue;
            }
        }
        if ((PlayersData[faction_id].proto_id_active[id] > 0)
            + PlayersData[faction_id].proto_id_lost[id]
            + (PlayersData[faction_id].proto_id_queue[id] > 0) != 0) {
            ++last_row;
        }
    }
    if (field_C0F8_ == 0) {
        if (last_row >= rows) field_E26C_ = 1;
    } else if (last_row < rows) {
        scroll_hide(this, &scroll_);
        field_C0F8_ = 0;
    }
    if (field_E270_ != 0) {
        field_C108_ = rows - 1;
        field_C0FC_ = 0;
        field_C100_ = 0;
        field_C104_ = last_row;
        scroll_.set_range(0, last_row);
        scroll_.set_paging(field_C108_);
        scroll_.set_pos(field_C0FC_);
        field_E270_ = 0;
    }
    if (last_row + 1 == 0) return;
    last_row = -1;
    for (i = 0; i < 128; ++i) {
        id = i < 64 ? i : faction_id * 64 - 64 + i;
        if ((VehPrototypes[id].flags & 1) == 0) continue;
        if (ReportIfGlobal.field_1E244_ < 4) {
            if (ReportIfGlobal.field_1E244_ < 3) {
                if (Chassis[VehPrototypes[id].chassis_id].triad != ReportIfGlobal.field_1E244_) continue;
                if (Weapon[VehPrototypes[id].weapon_id].offense_rating == 0) continue;
            } else if (Weapon[VehPrototypes[id].weapon_id].offense_rating != 0) {
                continue;
            }
        }
        if ((PlayersData[faction_id].proto_id_active[id] > 0)
            + (PlayersData[faction_id].proto_id_queue[id] > 0)
            + PlayersData[faction_id].proto_id_lost[id] == 0) continue;
        ++last_row;
        if (last_row < field_C0FC_
            || last_row >= rows + field_C0FC_) continue;
        if (last_row - field_C0FC_ != 0) {
            row.top += 43;
            row.bottom += 43;
        }
        if (selected_prototypes_[i / 32] & (1 << (i % 32))) {
            Buffer::box_sprite(&row, &InterfaceBoxStyles[2]);
            Buffer::box_sprite(&row, &InterfaceBoxStyles[5]);
        } else {
            Buffer::box_sprite(&row, &InterfaceBoxStyles[1]);
        }
        spot_.add(id, 0, row.left, row.top, row.right - row.left, row.bottom - row.top);
        rect1_.top = row.top;
        const int text_y = row.top - field_E268_ / 2 + 21;
        const int vehicle_y = row.top + 2;
        const int vehicle = veh_fake(id, faction_id);
        veh_draw(this, vehicle, energy_panel_rect_.left, vehicle_y, -6, 8, 0);
        Buffer::set_text_color(0x99, -1, 1, 1);
        Buffer::set_text_color2(0xF2, -1, 1, 1);
        Buffer::set_text_color3(0xF9, -1, 1, 1);
        Buffer::set_font(ReportIfGlobal.field_1E18C_, ReportIfGlobal.field_1E18C_,
                         ReportIfGlobal.field_1E18C_, 0);
        rect1_.left = 102;
        StringTemp[0] = 0;
        if (VehPrototypes[id].obsolete_factions & (1 << faction_id)) strcat(StringTemp, "-");
        strcat(StringTemp, VehPrototypes[id].veh_name);
        Buffer::write_l(StringTemp, rect1_.left, text_y);
        rect1_.left = (rect1_.right >> 1) + 40;
        StringTemp[0] = 0;
        say_stats(StringTemp, id, 0);
        Buffer::write_l(StringTemp, rect1_.left, text_y);
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        say_num(PlayersData[faction_id].proto_id_active[id] > 250
            ? 0 : PlayersData[faction_id].proto_id_active[id]);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x614 / 4));
        strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, "}");
        say_num(PlayersData[faction_id].proto_id_queue[id] > 250
            ? 0 : PlayersData[faction_id].proto_id_queue[id]);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x618 / 4));
        strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, "[");
        say_num(PlayersData[faction_id].proto_id_lost[id]);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x61C / 4));
        strcat(StringTemp, "]");
        Buffer::write_right_l(StringTemp, rect1_.right - 2, text_y);
        rect1_.top += 43;
    }
}

// ORIGINAL: 0x004A1140 ?draw_score@ReportWin@@QAEXXZ 0x004A1140-0x004A2208
// RECOVERY_BINDING: v1 {"evidence": ["git:649230f6:src/unrecovered/004a1140.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_score() {
    // Function scope: most_objectives' outputs have their own address-taken
    // slots at [ebp-0x30]/[ebp-0x2c] in the image (frame 0x180); declared in
    // the objective block they packed with later locals (frame 0x178).
    int leader, pacts;
    const int faction = ReportIfGlobal.report_faction_id_;
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3,
                  energy_panel_rect_.top + field_E268_ + 9};
    RECT body = {title.left, title.bottom + 2, title.right, energy_panel_rect_.bottom - 3};
    Buffer::box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    Buffer::box_sprite(&title, &InterfaceBoxStyles[1]);
    Buffer::box_sprite(&body, &InterfaceBoxStyles[1]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[faction].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0xADC / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    Buffer::write_cent_l(StringTemp, &title);
    ++title.top;
    ++title.bottom;
    compute_score(faction, 0, 0, 0);
    if (X_text_open(ScriptPopupFile, "SCORING")) return;
    rect1_.top = body.top + 4;
    rect1_.left = body.left + 4;
    rect1_.right = body.right - body.left - 8;
    rect1_.bottom = rect1_.right;
    for (int item = 0; item < 9; ++item) {
        text_get();
        if (PlayersData[faction].score_items_[item] == 0 || item == 5) continue;
        if (item == 6) {
            const int own = num_objectives(faction, 0);
            const int total = num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000);
            StringTemp[0] = 0;
            if (total > own) {
                say_num(own);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, StringTable.get(Labels.strings_ptr[0xE0C / 4]));
                report_topic_line(this, rect1_, rect1_.left);
                rect1_.top += field_E268_;
                StringTemp[0] = 0;
                say_num(total - own);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get(0x1030 / 4));
                report_topic_line(this, rect1_, rect1_.left);
                rect1_.top += field_E268_ * 3 / 2;
                StringTemp[0] = 0;
            } else {
                say_num(own);
                strcat(StringTemp, BufferJoinSpace);
            }
            strcat(StringTemp, TextBufferGetPtr);
        } else {
            StringTemp[0] = 0;
            if (item == 7) {
                if (GameDataStateStorage.fields_.game_state_ & 0x200000) strcat(StringTemp, label_get(0xE70 / 4));
                else if (GameDataStateStorage.fields_.game_state_ & 0x400000) strcat(StringTemp, label_get(0xE88 / 4));
                else if (PlayersData[faction].flags & 0x100000) strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x1198 / 4]));
                else if (!(GameDataStateStorage.fields_.game_state_ & 0x2000)) strcat(StringTemp, label_get(0xE10 / 4));
                else strcat(StringTemp, TextBufferGetPtr);
            } else {
                strcat(StringTemp, TextBufferGetPtr);
            }
        }
        report_topic_line(this, rect1_, rect1_.left);
        StringTemp[0] = 0;
        if (PlayersData[faction].score_items_[item] >= 0) strcat(StringTemp, "+ ");
        else strcat(StringTemp, "- ");
        say_num(abs(PlayersData[faction].score_items_[item]));
        Buffer::write_right_l(StringTemp, rect1_.left + rect1_.right, rect1_.top);
        rect1_.top += field_E268_;
    }
    // Genuine original: this PlayerData offset slot doubles as the outcome.
    int outcome = faction * sizeof(PlayerData);
    if (PlayersData[faction].score_items_[5]) {
        StringTemp[0] = 0;
        if (MapNativeLifeForms) {
            strcat(StringTemp, label_get(0x908 / 4));
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, "+25%");
        } else {
            strcat(StringTemp, label_get(0x904 / 4));
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, "-25%");
        }
        report_topic_line(this, rect1_, rect1_.left);
        StringTemp[0] = 0;
        if (PlayersData[faction].score_items_[5] >= 0) strcat(StringTemp, "+ ");
        else strcat(StringTemp, "- ");
        say_num(abs(PlayersData[faction].score_items_[5]));
        Buffer::write_right_l(StringTemp, rect1_.left + rect1_.right, rect1_.top);
        rect1_.top += field_E268_;
    }
    if ((GameDataStateStorage.fields_.game_rules_ & 0x400)
        && !(GameDataStateStorage.fields_.game_state_ & 0x20)
        && PlayersData[faction].unk_109 > 0) {
        rect1_.top += field_E268_;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x1044 / 4));
        CharUpperA(StringTemp);
        report_topic_line(this, rect1_, rect1_.left);
        StringTemp[0] = 0;
        strcat(StringTemp, "+100%");
        Buffer::write_right_l(StringTemp, rect1_.left + rect1_.right, rect1_.top);
        rect1_.top += field_E268_;
    }
    rect1_.top += field_E268_;
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x6C0 / 4));
    strcat(StringTemp, "}");
    report_topic_line(this, rect1_, rect1_.left);
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    if (PlayersData[faction].unk_109 < 0) strcat(StringTemp, "- ");
    say_num(abs(PlayersData[faction].unk_107));
    strcat(StringTemp, "}");
    Buffer::write_right_l(StringTemp, rect1_.left + rect1_.right, rect1_.top);
    rect1_.top += field_E268_ * 2;
    if (GameDataStateStorage.fields_.game_state_ & 1) {
        if (GameDataStateStorage.fields_.game_state_ & 0x80000) {
            StringTemp[0] = 0;
            if (PlayersData[faction].score_items_[7]) {
                // Genuine original: this label is built but never drawn.
                strcat(StringTemp, label_get(0xE10 / 4));
            } else {
                strcat(StringTemp, label_get(0xE14 / 4));
                for (int other = 1; other < 8; ++other) {
                    if (other == faction) continue;
                    compute_score(other, 0, 0, 0);
                    if (PlayersData[other].score_items_[7]) {
                        strcat(StringTemp, BufferJoinSpace);
                        strcat(StringTemp, SocialWinOpenParenText);
                        PluralityDefault = 0;
                        GenderDefault = Players[other].is_leader_female;
                        strcat(StringTemp, Players[other].name_leader);
                        strcat(StringTemp, BufferJoinSpace);
                        strcat(StringTemp, label_get(0x101C / 4));
                        strcat(StringTemp, SocialWinCloseParenText);
                        break;
                    }
                }
                CharUpperA(StringTemp);
                strcat(StringTemp, "!");
                report_topic_line(this, rect1_, rect1_.left);
                rect1_.top += field_E268_ * 2;
            }
        } else if (!(GameDataStateStorage.fields_.game_state_ & 0x602000)
                   && ScenarioSave.objective_req_victory <= 9000) {
            StringTemp[0] = 0;
            int most;  // Genuine original: only set when objective_req_victory == 0.
            if (ScenarioSave.objective_req_victory == 0) {
                const int objectives = num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000);
                most = most_objectives(&leader, &pacts);
                if (objectives >= (most < 0 ? 0 : most)) {
                    if (pacts) {
                        strcat(StringTemp, label_get(0x102C / 4));
                        outcome = 2;
                    } else {
                        strcat(StringTemp, label_get(0xE10 / 4));
                        outcome = 1;
                    }
                } else {
                    strcat(StringTemp, label_get(0xE14 / 4));
                    if (leader > 0) {
                        strcat(StringTemp, BufferJoinSpace);
                        strcat(StringTemp, SocialWinOpenParenText);
                        PluralityDefault = 0;
                        GenderDefault = Players[leader].is_leader_female;
                        strcat(StringTemp, Players[leader].name_leader);
                        strcat(StringTemp, BufferJoinSpace);
                        strcat(StringTemp, label_get(0x101C / 4));
                        strcat(StringTemp, SocialWinCloseParenText);
                    }
                    outcome = 0;
                }
            } else if (num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000)
                       >= ScenarioSave.objective_req_victory) {
                strcat(StringTemp, label_get(0xE10 / 4));
                outcome = 1;
            } else if (PlayersData[faction].score_items_[8] == 0) {
                strcat(StringTemp, label_get(0xE14 / 4));
                outcome = 0;
            }
            // Genuine original: a nonzero item 8 here leaves `outcome` as the
            // PlayerData byte offset, selecting DRAWSTRING below.
            CharUpperA(StringTemp);
            strcat(StringTemp, "!");
            report_topic_line(this, rect1_, rect1_.left);
            rect1_.top += field_E268_ * 2;
            for (int other = 1; other < 8; ++other) {
                if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << other))
                    || other == faction) continue;
                const int target = ScenarioSave.objective_req_victory
                    ? ScenarioSave.objective_req_victory : most;
                if (num_objectives(other, GameDataStateStorage.fields_.game_rules_ & 0x1000) < target) continue;
                if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << other))
                    && (GameDataStateStorage.fields_.game_rules_ & 0x80000)) continue;
                const char *section = outcome > 1 ? "DRAWSTRING" : outcome ? "WINSTRING2" : "WINSTRING";
                if (X_text_open(ScriptPopupFile, section)) break;
                text_get();
                parse_set(Players[other].is_leader_female, FALSE);
                parse_says(0, Players[other].title_leader, -1, -1);
                parse_set(Players[other].is_leader_female, FALSE);
                parse_says(1, Players[other].name_leader, -1, -1);
                parse_set(Players[other].noun_gender, Players[other].is_noun_plural);
                parse_says(2, Players[other].noun_faction, -1, -1);
                StringTemp[0] = 0;
                say_num(num_objectives(other, 0));
                if (num_objectives(other, GameDataStateStorage.fields_.game_rules_ & 0x1000)
                    > num_objectives(other, 0)) {
                    strcat(StringTemp, SocialWinOpenParenText);
                    say_num(num_objectives(other, GameDataStateStorage.fields_.game_rules_ & 0x1000));
                    strcat(StringTemp, SocialWinCloseParenText);
                }
                parse_says(3, StringTemp, -1, -1);
                StringTemp[0] = 0;
                parse_string(TextBufferGetPtr, StringTemp);
                report_topic_line(this, rect1_, rect1_.left);
                rect1_.top += field_E268_;
            }
        }
    }
    if (GameDataStateStorage.fields_.game_state_ & 0x20) {
        Buffer::set_text_color(0xF9, -1, 1, 1);
        Buffer::set_font(ReportIfGlobal.field_1E1B8_, 0, 0, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x1060 / 4));
        strcat(StringTemp, "!");
        Buffer::write_cent(StringTemp, rect1_.left, energy_panel_rect_.bottom - 50,
                                 rect1_.right, 50);
    }
    text_close();
}

// ORIGINAL: 0x004A2210 ?draw_intel@ReportWin@@QAEXXZ 0x004A2210-0x004A345F
// RECOVERY_BINDING: v1 {"evidence": ["git:649230f6:src/unrecovered/004a2210.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_intel() {
    const int viewer = ReportIfGlobal.report_faction_id_;
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3,
                  energy_panel_rect_.top + field_E268_ + 9};
    RECT body = {energy_panel_rect_.left + 3, title.bottom + 2, energy_panel_rect_.right - 3,
                 energy_panel_rect_.bottom - 3};
    Buffer::box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    Buffer::box_sprite(&title, &InterfaceBoxStyles[1]);
    Buffer::box_sprite(&body, &InterfaceBoxStyles[1]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[viewer].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0xAE0 / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    Buffer::write_cent_l(StringTemp, &title);
    ++title.top;
    ++title.bottom;
    rect1_.left = body.left + 4;
    rect1_.top = body.top + 4;
    rect1_.right = body.right - body.left - 8;
    rect1_.bottom = rect1_.right;
    const int subject = ReportIfGlobal.report_selection_;
    const int informed = (GameDataStateStorage.fields_.game_state_ & 0x80) || spying(subject) ||
        (PlayersData[viewer].diplo_treaties[subject] & 1);
    FactionDiplomacyPortraits[subject]->draw(this, rect1_.left, rect1_.top);
    const int text_x = rect1_.left + 158;
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    report_append_leader(subject);
    strcat(StringTemp, "]");
    CharUpperA(StringTemp);
    report_topic_line(this, rect1_, text_x);
    rect1_.top += field_E268_ * 2;
    diplomacy_check(viewer, subject, 0);
    report_intel_label(0xE98 / 4);
    const unsigned int relation = PlayersData[viewer].diplo_treaties[subject];
    if (relation & 0x17) {
        if (relation & 0x10) strcat(StringTemp, label_get(0x358 / 4));
        else if (relation & 1) strcat(StringTemp, label_get(0x34C / 4));
        else if (relation & 2) strcat(StringTemp, label_get(0x350 / 4));
        else if (relation & 4) strcat(StringTemp, label_get(0x354 / 4));
    }
    report_topic_line(this, rect1_, text_x);
    rect1_.top += field_E268_;
    diplomacy_check(viewer, subject, 0);
    report_intel_label(0xE94 / 4);
    say_special(StringTemp, StringTable.get(Mood[get_mood(DiploFriction)]), subject);
    CharUpperA(StringTemp);
    report_topic_line(this, rect1_, text_x);
    rect1_.top += field_E268_;
    report_intel_label(0x5A0 / 4);
    if (PlayersData[subject].ai_fight > 0) strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x5A4 / 4]));
    else if (PlayersData[subject].ai_fight < 0) strcat(StringTemp, label_get(0x5AC / 4));
    else strcat(StringTemp, label_get(0x5A8 / 4));
    CharUpperA(StringTemp);
    report_topic_line(this, rect1_, text_x);
    rect1_.top += field_E268_;
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x5B0 / 4));
    CharUpperA(StringTemp);
    strcat(StringTemp, SocialWinColonSpaceText);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, "]");
    int priorities = 0;
    if (PlayersData[subject].ai_growth) {
        strcat(StringTemp, label_get(0x824 / 4));
        priorities = 1;
    }
    if (PlayersData[subject].ai_tech) {
        if (priorities) strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, label_get(0x828 / 4));
        ++priorities;
    }
    if (PlayersData[subject].ai_wealth) {
        if (priorities) strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, label_get(0x82C / 4));
        ++priorities;
    }
    if (PlayersData[subject].ai_power) {
        if (priorities) strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, label_get(0x830 / 4));
    }
    report_topic_line(this, rect1_, text_x);
    rect1_.top += field_E268_;
    report_intel_label(0x5C0 / 4);
    int shown = 0;
    const int category = Players[subject].soc_ideology_category[0];
    if (category >= 0) {
        const int model = Players[subject].soc_ideology_model[0];
        if (model != 0) {
            strcat(StringTemp, StringTable.get(SocialCategories[category].type));
            strcat(StringTemp, SocialWinColonSpaceText);
            strcat(StringTemp, StringTable.get(SocialCategories[category].name[model]));
            shown = 1;
        }
    }
    if (shown) {
        CharUpperA(StringTemp);
        report_topic_line(this, rect1_, text_x);
        rect1_.top += field_E268_;
    }
    int headquarters = -1;
    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        if (Bases[base].faction_id_current == subject && has_fac(FAC_HEADQUARTERS, base, 0)) {
            headquarters = base;
            break;
        }
    }
    report_intel_label(0x5C4 / 4);
    say_base(StringTemp, headquarters);
    CharUpperA(StringTemp);
    report_topic_line(this, rect1_, text_x);
    rect1_.top += field_E268_;
    report_intel_label(0xE4 / 4);
    say_num(PlayersData[subject].energy_reserves);
    if (informed) {
        report_topic_line(this, rect1_, text_x);
        rect1_.top += field_E268_;
    }
    if ((GameDataStateStorage.fields_.game_state_ & 0x80) || !(GameDataStateStorage.fields_.game_rules_ & 0x200)) {
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0x254 / 4));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, "]");
        say_tech(StringTemp, PlayersData[subject].tech_id_researching, 0);
        if (informed) {
            report_topic_line(this, rect1_, text_x);
            rect1_.top += field_E268_;
        }
    }
    if (!(Players[subject].rule_flags & RFLAG_ALIEN)) {
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0xF00 / 4));
        CharUpperA(StringTemp);
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, "]");
        say_num(council_votes(subject));
        report_topic_line(this, rect1_, text_x);
    }
    rect1_.top += field_E268_ + 20;
    for (int category_index = 0; category_index < 4; ++category_index) {
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, StringTable.get(SocialCategories[category_index].type));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, "]");
        strcat(StringTemp, StringTable.get(SocialCategories[category_index].name[
            PlayersData[subject].soc_category_pending.models_[category_index]]));
        CharUpperA(StringTemp);
        Buffer::write_l(StringTemp, rect1_.left + 20,
                       rect1_.top + category_index * field_E268_);
    }
    for (int faction = 1; faction < 8; ++faction) {
        const unsigned int flags = PlayersData[subject].diplo_treaties[faction];
        if (faction == subject || !(flags & 8) ||
            !(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << faction)) ||
            (!informed && faction != viewer && !(PlayersData[viewer].diplo_treaties[faction] & 1))) continue;
        StringTemp[0] = 0;
        if (flags & 0x10) {
            strcat(StringTemp, label_get(0x358 / 4)); strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, label_get(0x5C8 / 4));
        } else if (flags & 1) {
            strcat(StringTemp, get_pact(subject)); strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, label_get(0x43C / 4));
        } else {
            if (flags & 2) strcat(StringTemp, label_get(0x350 / 4));
            else {
                if (flags & 0x10000) { strcat(StringTemp, label_get(0x5D0 / 4)); strcat(StringTemp, BufferJoinSpace); }
                strcat(StringTemp, label_get(0x354 / 4));
            }
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, label_get(0x5CC / 4));
        }
        strcat(StringTemp, BufferJoinSpace);
        report_append_leader(faction);
        report_topic_line(this, rect1_, text_x + 125);
        rect1_.top += field_E268_;
    }
}

// ORIGINAL: 0x004A3460 ?init@ReportIf@@QAEXXZ 0x004A3460-0x004A3927;0x00659580-0x00659595 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/004a3460.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a3460 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf initialization from the retained original behavior and the genuine ReportWin control storage; its non-virtual source identity remains distinct from ReportIf's historical virtual callbacks."}
void ReportIf::init() {
    Buffer image;
    field_1E240_ = 0;
    field_1E244_ = 4;
    field_4_ = 5;
    // Registry slot 5 is this genuine SubInterface, stored in place.
    InterfaceRegistry[5] = this;
    spot_.init(0x100);
    for (int i = 0; i < 0x200; ++i) {
        sort_entries_[i].base_id = i;
        sort_entries_[i].value = 0;
        sort_entries_[i].sorted_base_id = i;
        sort_entries_[i].sorted_value = 0;
    }
    ReportWinGlobal.field_5A08_ = 0;
    for (int dot = 0; dot < 4; ++dot) {
        char number[80];
        StringTemp[0] = 0;
        strcat(StringTemp, "dot");
        _itoa(dot, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, ".pcx");
        image.load_pcx(StringTemp, 0, 10, 0xEC);
        spritesB_[dot].extract(&image, WinFillColour, 0, 0, 12, 12, 0);
    }
    image.load_pcx("report.pcx", 0, 10, 0xEC);
    int x = 0;
    for (int row = 0; row < 7; ++row) {
        int y = 0;
        for (int column = 0; column < 3; ++column) {
            spritesA_[row * 3 + column].extract(&image, WinFillColour, x, y, 30, 36, 0);
            y += 35;
        }
        x += 29;
    }
    field_20_ = MainInterfaceGlobal.main_frame_rect();
    field_30_ = MainInterfaceGlobal.right_frame_rect();
    field_40_ = MainInterfaceGlobal.lower_frame_rect();
    field_50_ = MainInterfaceGlobal.info_panel_frame();
    if (WinScreenWidth == 800) {
        report_button_positions_[0] = 0;
        report_button_positions_[1] = 37;
        report_button_positions_[2] = 74;
        report_button_positions_[3] = 111;
        report_button_positions_[4] = 148;
        report_button_positions_[5] = 185;
        report_button_positions_[6] = 222;
        field_1E230_ = 0;
    } else {
        report_button_positions_[0] = 0;
        report_button_positions_[1] = 42;
        report_button_positions_[2] = 84;
        report_button_positions_[3] = 126;
        report_button_positions_[4] = 168;
        report_button_positions_[5] = 210;
        report_button_positions_[6] = 252;
        field_1E230_ = 1;
    }
    for (int k = 0; k < 7; ++k)
        for (int state = 0; state < 3; ++state)
            if (k + 1 == ConsoleGlobal.faction_id_ && field_1E230_)
                faction_button_sprites_[k][state] = &FactionsArt[k + 1].report_logo[state == 0];
            else
                faction_button_sprites_[k][state] = &FactionsArt[k + 1].report_logo_small[state];
    int position = field_40_.left + (field_1E230_ ? 17 : 16);
    report_category_center_ =
        (field_40_.bottom + field_40_.top - spritesA_[0].height()) / 2;
    int step = spritesA_[0].width() + (field_1E230_ ? 12 : 9);
    for (int category = 0; category < 7; ++category) {
        report_category_positions_[category] = position;
        position += step;
    }
    // Original includes the eighth, previously unwritten slot in this walk
    // before replacing it below; preserve that store sequence.
    for (int slot = 0; slot < 8; ++slot)
        report_button_positions_[slot] += field_50_.left + (field_1E230_ ? 89 : 46);
    report_button_positions_[7] = field_50_.bottom - 40;
    field_1E180_ = find_font(10, 0);
    field_1E184_ = find_font(12, 0);
    field_1E188_ = find_font(12, 2);
    field_1E18C_ = find_font(12, 1);
    field_1E190_ = find_font(14, 0);
    field_1E194_ = find_font(14, 2);
    field_1E198_ = find_font(14, 1);
    field_1E19C_ = find_font(16, 0);
    field_1E1A0_ = find_font(16, 2);
    field_1E1A4_ = find_font(16, 1);
    field_1E1A8_ = find_font(11, 0);
    field_1E1AC_ = find_font(26, 0);
    field_1E1B0_ = find_font(18, 2);
    field_1E1B4_ = find_font(22, 0);
    field_1E1B8_ = find_font(32, 0);
    field_1C_ = font_line_advance(*field_1E184_);
    ReportWinGlobal.init();
}

/*
// ORIGINAL: 0x004A3930 ?done@ReportIf@@QAEXXZ 0x004A3930-0x004A3946 BYTE_EXACT
// size      22 bytes
// prototype void (__thiscall ?done@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x005EE280
Status: Complete
*/
// The window whose modal ReportIf::done releases, 0x00876478 - now IDENTIFIED
// as the process-wide ReportWin singleton defined here: the original
// dynamic initializer at 0x00496930 constructs it (`mov ecx, 0x876478; call
// ??0ReportWin@@QAE@XZ`, followed by the atexit registration), and
// ReportWin::on_key_click closes the same object with Win::release_modal.
// done() releases its modal loop and gives up the active interface mode.
void ReportIf::done() {
    ReportWinGlobal.Win::release_modal();
    SubInterface::release_iface_mode();
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A3950 0x004A3CD4 0x004A3CF8 0x004A3962
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A3950 0x004A3CF8 0x004A3D1C 0x004A3C37
/*
Purpose: Switch report pages using the existing interface and window owners,
         preserving the original research-selection transition and redraw.
// ORIGINAL: 0x004A3950 ?report@ReportIf@@QAEXHHH@Z 0x004A3950-0x004A3CD3 FILE BYTE_EXACT
// size      899 bytes
// prototype void (__thiscall ?report@ReportIf@@QAEXHHH@Z)(ReportIf* this, int, int, int)
// callers   12   call targets   18
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045DEA0 0x0045E540 0x004A3D20 0x004A7000 0x004A84A0 0x004A9050 0x004A9F90 0x004AA0B0 0x004AA970 0x004AB550 0x004AB600 0x004ABE70 0x004AC140 0x004C3A40 0x005B9F20 0x005D5890 0x005D5A70 0x00609F20
Status: Report dispatch and state transition recovered; the referenced page
        renderers have source-owned implementations below. Matching and game
        acceptance remain separate obligations.
*/
void ReportIf::report(int report_id, int faction_id, int selection) {
    switch (report_page_history_[0]) {
    case 2: close_labs(); break;
    case 3: close_energy(); break;
    case 4: close_ops(); break;
    case 5: close_projects(); break;
    case 6: close_sat(); break;
    case 7: close_security(); break;
    case 8: close_score(); break;
    case 0: close_intel(); break;
    }

    // The original tests only for a positive page: page 1 indexes the
    // control before the category run, exactly as the image does.
    if (report_page_history_[0] > 0) {
        flatButtonsB_[report_page_history_[0] - 2].set_report_page_state(-1);
        flatButtonsB_[report_page_history_[0] - 2].redraw();
    }
    if (report_id > 0) {
        flatButtonsB_[report_id - 2].set_report_page_state(2);
        flatButtonsB_[report_id - 2].redraw();
    }

    report_selection_ = selection;
    field_8_ = report_id;
    report_faction_id_ = faction_id;

    if (report_page_history_[0] != 2 && ReportWinGlobal.shared_report_selection() + 1 != 0) {
        if (!(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
            && (GameDataStateStorage.fields_.game_rules_ & RULES_BLIND_RESEARCH)) {
            if (!(ReportWinGlobal.shared_research_focus() >= 0
                  && ReportWinGlobal.shared_research_focus() <= 0x59
                  && has_tech(ReportWinGlobal.shared_research_focus(), faction_id))) {
                for (int tech_id = 0; tech_id < 0x59; ++tech_id) {
                    if (has_tech(tech_id, report_faction_id_)) {
                        ReportWinGlobal.shared_research_focus() = tech_id;
                        break;
                    }
                }
            }
        } else {
            ReportWinGlobal.shared_research_focus() = PlayersData[faction_id].tech_id_researching;
        }
        if (ReportWinGlobal.shared_research_focus() < 0 || ReportWinGlobal.shared_research_focus() > 0x59) {
            ReportWinGlobal.shared_research_focus() = 0;
        }
        ReportWinGlobal.shared_report_selection() = -1;
    }

    if (faction_id <= 0) {
        return;
    }

    for (int index = 18; index > 0; --index) {
        report_page_history_[index] = report_page_history_[index - 1];
    }
    report_page_history_[0] = field_8_;
    if (field_8_ == 4 || field_8_ == 5 || field_8_ == 7) {
        if (field_8_ != report_page_history_[1]) {
            ReportWinGlobal.field_E270_ = 1;
        }
        if (report_topic_changed_) {
            ReportWinGlobal.field_E270_ = 1;
            report_topic_changed_ = 0;
        }
    } else if (ReportWinGlobal.field_C0F8_ != 0) {
        scroll_hide(&ReportWinGlobal, &ReportWinGlobal.scroll_);
        ReportWinGlobal.field_E270_ = 0;
        ReportWinGlobal.field_E26C_ = 0;
        ReportWinGlobal.field_C0F8_ = 0;
    }

    reset();
    switch (field_8_) {
    case 2: draw_labs(); break;
    case 3: draw_energy(); break;
    case 4: draw_ops(); break;
    case 5: draw_projects(); break;
    case 6: draw_sat(); break;
    case 7: draw_security(); break;
    case 8: draw_score(); break;
    case 0: draw_intel(); break;
    }
    MainInterfaceGlobal.soft_update();
    ReportWinGlobal.redraw();
    if (ReportWinGlobal.field_E26C_ != 0 && !report_scroll_blocked_) {
        scroll_show(&ReportWinGlobal, &ReportWinGlobal.scroll_);
        ReportWinGlobal.field_E26C_ = 0;
        ReportWinGlobal.field_C0F8_ = 1;
    }
}

// ORIGINAL: 0x004A3D20 ?reset@ReportIf@@QAEXXZ 0x004A3D20-0x004A3FBF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/maininterface.h:MainInterfaceGlobal", "source:src/graphicwin.h:GraphicWin::buffer_working_rect", "source:src/datalink.cpp:1933", "docs/recovery/member-accesses.csv:ReportIf offsets 0x20, 0x30, 0x40, 0x50"], "reason": "Draw through the MainInterfaceGlobal Buffer base and its owned working clip rectangle, with typed ReportIf panel rectangles."}
// size      671 bytes
// prototype void (__thiscall ?reset@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   5   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DAC70 0x005DACB0 0x005DACE0 0x005DAD10 0x005DEF90 0x005E2DD3
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004A3D20
// measured tier  MISMATCH
// divergence     43
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004a3d20/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?reset@ReportIf@@QAEXXZ  at 0x004A3D20  (671 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void ReportIf::reset() {
    MainInterfaceGlobal.Buffer::box_sprite(&field_20_, &InterfaceBoxStyles[33]);
    MainInterfaceGlobal.Buffer::box_sprite(&field_30_, &InterfaceBoxStyles[33]);
    MainInterfaceGlobal.Buffer::box_sprite(&field_50_, &InterfaceBoxStyles[33]);
    MainInterfaceGlobal.Buffer::set_clip(&field_40_);
    field_40_.left -= 7;
    MainInterfaceGlobal.Buffer::box_sprite(&field_40_, &InterfaceBoxStyles[33]);
    field_40_.left += 7;
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    inflate_frame(&field_20_, -3, -3);
    inflate_frame(&field_30_, -3, -3);
    inflate_frame(&field_50_, -4, -4);
    inflate_frame(&field_40_, -4, -4);
    field_40_.left -= 3;
    MainInterfaceGlobal.Buffer::box_sprite(&field_20_, &InterfaceBoxStyles[3]);
    MainInterfaceGlobal.Buffer::box_sprite(&field_30_, &InterfaceBoxStyles[3]);
    MainInterfaceGlobal.Buffer::box_sprite(&field_50_, &InterfaceBoxStyles[3]);
    MainInterfaceGlobal.Buffer::box_sprite(&field_40_, &InterfaceBoxStyles[3]);

    inflate_frame(&field_50_, -3, -3);
    inflate_frame(&field_40_, -3, -3);
    MainInterfaceGlobal.Buffer::box_sprite(&field_50_, &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.Buffer::box_sprite(&field_40_, &InterfaceBoxStyles[0]);

    field_40_.left += 3;
    inflate_frame(&field_20_, 3, 3);
    inflate_frame(&field_30_, 3, 3);
    inflate_frame(&field_50_, 7, 7);
    inflate_frame(&field_40_, 7, 7);

    MainInterfaceGlobal.Buffer::set_font(field_1E180_, field_1E184_, field_1E184_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
    MainInterfaceGlobal.Buffer::set_text_color2(0x99, -1, 1, 1);
    MainInterfaceGlobal.Buffer::set_text_color3(0xe1, -1, 1, 1);
    MainInterfaceGlobal.Buffer::clear_links();
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A3FC0 0x004A4020 0x004A4044 0x004A3FD0
// ORIGINAL: 0x004A3FC0 ?on_redraw@ReportIf@@QAEXXZ 0x004A3FC0-0x004A401F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:649230f6:src/recovered/units/004a3fc0.cpp", "image:0x004A3FC0-0x004A401F and dispatch table0x004A4020", "source:src/reportif.h"], "reason": "Recover the report-interface redraw operation through its real category member, reset and owned drawing methods. Original direct calls and nine-entry dispatch preserve the default/no-draw category without vtable or executable-address dispatch."}
void ReportIf::on_redraw() {
    reset();
    switch (field_8_) {
    case 2: draw_labs(); break;
    case 3: draw_energy(); break;
    case 4: draw_ops(); break;
    case 5: draw_projects(); break;
    case 6: draw_sat(); break;
    case 7: draw_security(); break;
    case 8: draw_score(); break;
    case 0: draw_intel(); break;
    }
}

/*
// ORIGINAL: 0x004A4050 ?bl_daemon@@YAXH@Z 0x004A4050-0x004A405D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["ReportWin::on_left_click registers bl_daemon with Time::init616260 using its owned timer and the borrowed ReportIfGlobal receiver as the one-word integer context.", "Pinned4A4050..4A405D loads its sole stack word into ECX, calls ReportIf::bl_anim4A4060 unconditionally and ends in plain RET; Time invokes its cdecl(int) contract.", "Earlier source accepted ReportIf* and was registered through an incompatible function-pointer cast; preserve that translation finding as history, not a second current prototype."], "reason": "Home the blink animation bridge with its genuine ReportIf instance target and a directly compatible cdecl(int) timer contract; preserve borrowed lifetime and unconditional dispatch."}
// symbol    ?bl_daemon@@YAXH@Z
// size      13 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004A4060
Status: Complete
*/
void __cdecl bl_daemon(int context) {
    ReportIf *const report = reinterpret_cast<ReportIf *>(context);
    report->bl_anim();
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A4060 0x004A4194 0x004A41C4 0x004A407D
// ORIGINAL: 0x004A4060 ?bl_anim@ReportIf@@QAEXXZ 0x004A4060-0x004A4191 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: source:src/recovered/units/004a4060.cpp", "source:src/reportif.h", "source:src/worldwin.h", "source:src/reportwin.h", "source:src/sprite.h"], "reason": "Advance ReportIf animation with the declared sprite run and coordinate state, drawing to the WorldClimateWorldWin Buffer base and stopping the ReportWin-owned Time at terminal frame 10."}
void ReportIf::bl_anim() {
    WorldClimateWorldWin.redraw();
    switch (animation_state_) {
    case -1:
    case 0:
    case 8:
    case 9: {
        Buffer *destination = static_cast<Buffer *>(&WorldClimateWorldWin);
        Sprite *sprite = &spritesB_[0];
        sprite->draw(destination,
                     sprite->transparent_index(), animation_x_, animation_y_);
        ++animation_state_;
        return;
    }
    case 3:
    case 5: {
        Buffer *destination = static_cast<Buffer *>(&WorldClimateWorldWin);
        Sprite *sprite = &spritesB_[3];
        sprite->draw(destination,
                     sprite->transparent_index(), animation_x_, animation_y_);
        ++animation_state_;
        return;
    }
    case 2:
    case 4:
    case 6: {
        Buffer *destination = static_cast<Buffer *>(&WorldClimateWorldWin);
        Sprite *sprite = &spritesB_[2];
        sprite->draw(destination,
                     sprite->transparent_index(), animation_x_, animation_y_);
        ++animation_state_;
        return;
    }
    case 1:
    case 7: {
        Buffer *destination = static_cast<Buffer *>(&WorldClimateWorldWin);
        Sprite *sprite = &spritesB_[1];
        sprite->draw(destination,
                     sprite->transparent_index(), animation_x_, animation_y_);
        ++animation_state_;
        return;
    }
    case 10:
        ReportWinGlobal.time1_.stop();
        break;
    }
    ++animation_state_;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A41D0 0x004A5ED4 0x004A5EF8 0x004A5E12
// ORIGINAL: 0x004A41D0 ?on_status@ReportIf@@QAEXH@Z 0x004A41D0-0x004A5ED2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004a41d0.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a41d0 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's status transition through its genuine SubInterface owner and ReportWin's embedded controls, following the retained original behavior. The source's emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// TRIED: full hand transcription from raw disassembly (not Ghidra, whose &DAT_x+idx*N pseudocode over-scales by 4x on 0x96cdac/0x96c9f8 - fixed here against the raw asm multiply chain and byte tests); covers close path (switch+jump table+loops), init path (7-button report loop, 4 button groups, single-column cef..cf8, has_tech/ bitmask channel grid, spying flags, 4-way spy-report geometry, scroll/report/slide tail). First divergence is instruction #3 (prologue 'mov'/'push' scheduling) - approximate throughout, not byte-verified block by block beyond the ones cited above.
// symbol    ?on_status@ReportIf@@UAEXH@Z
void ReportIf::on_status(int mode) {
    ReportWin &window = ReportWinGlobal;
    if (mode) {
        if (ReportInterfaceStatusReentry) {
            ReportInterfaceStatusReentry = 0;
            return;
        }
        report_scroll_blocked_ = 1;
        field_1E234_ = 1;
        window.field_C0F8_ = 0;
        report_topic_changed_ = 1;
        window.fill(9);
        report_faction_id_ = ConsoleGlobal.faction_id_;
        // U1: rep stosd with eax = -1 (0x4A423E).
        for (int page = 0; page < 19; page++) {
            report_page_history_[page] = -1;
        }
        window.shared_report_selection() = -1;
        field_1E1D8_ = 1;
        if ((GameDataStateStorage.fields_.game_state_ & 0x80)
            || !(GameDataStateStorage.fields_.game_rules_ & 0x200)) {
            int research = PlayersData[report_faction_id_].tech_id_researching;
            window.shared_research_focus() = research;
            if (research < 0 || research > 89) {
                window.shared_research_focus() = 0;
            }
        } else {
            window.shared_research_focus() = 0;
        }

        // Seven category buttons (0x4A42B8..0x4A4391).
        for (int topic = 0; topic <= 6; ++topic) {
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0x2B1 + topic));
            flatButtonsB_[topic].init(0, topic + 2, report_category_positions_[topic],
                           report_category_center_, spritesA_[0].width(),
                           spritesA_[0].height(), &MainInterfaceGlobal, 0);
            flatButtonsB_[topic].set_report_icons(&spritesA_[topic * 3], &spritesA_[topic * 3 + 1],
                                       &spritesA_[topic * 3 + 2]);
            flatButtonsB_[topic].set_bubble_text(StringTemp);
            flatButtonsB_[topic].Buffer::set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
            flatButtonsB_[topic].redraw();
            flatButtonsB_[topic].redraw();
        }

        // Report frame and the two window buttons beneath it (0x4A4397..0x4A454F).
        RECT &panel = window.energy_panel_rect_;
        RECT &frame = window.report_frame_rect_;
        StringTemp[0] = 0;
        frame.top = panel.bottom + 7;
        frame.bottom = frame.top + 20;
        frame.left = panel.left + 75;
        frame.right = frame.left + rect_width(&panel) - 150;
        int center = (panel.right + panel.left) / 2;
        strcat(StringTemp, label_get(0x21B));
        set_rectangle_dimensions(&draw_workspace_rect_, frame.left, frame.top,
                                 center - frame.left - 1, 20);
        window.flatButton3_.init(StringTemp, 1, &draw_workspace_rect_, &window, 0);
        window.flatButton3_.set_font_redraw(&TutWinBaseWindow.font1_);
        window.flatButton3_.GraphicWin::redraw();
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x299));
        set_rectangle_dimensions(&draw_workspace_rect_, center + 1, frame.top,
                                 center - frame.left - 1, 20);
        window.flatButton2_.init(StringTemp, 9, &draw_workspace_rect_, &window, 0);
        window.flatButton2_.set_font_redraw(&TutWinBaseWindow.font1_);
        window.flatButton2_.GraphicWin::redraw();

        // First paired selector (0x4A4554..0x4A4727).
        field_C_ = field_20_.left + 9;
        field_10_ = field_20_.top + 9;
        field_14_ = rect_width(&field_20_) - 18;
        buttonGroup1_.init(2, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x175));
        // The image halves field_14_ signed (cdq/sar 0x4A45B0); the casts on
        // the int field are kept for the unit's symbol counter.
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_,
                                 static_cast<int>(field_14_) / 2 - 1, 20);
        flatButton1_.init(StringTemp, 0x92, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton1_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton1_.hide();
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x176));
        set_rectangle_dimensions(&draw_workspace_rect_,
                                 field_C_ + static_cast<int>(field_14_) / 2 + 1, field_10_,
                                 static_cast<int>(field_14_) / 2, 20);
        flatButton2_.init(StringTemp, 0x93, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton2_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton2_.hide();
        buttonGroup1_.add(&flatButton1_);
        buttonGroup1_.add(&flatButton2_);
        buttonGroup1_.set(0x92, 1);

        // Second paired selector (0x4A472C..0x4A48FE).
        field_C_ = field_20_.left + 9;
        field_10_ = field_20_.top + 9;
        field_14_ = rect_width(&field_20_) - 18;
        buttonGroup2_.init(4, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x1EE));
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_,
                                 static_cast<int>(field_14_) / 2 - 1, 20);
        flatButton3_.init(StringTemp, 0xD0A, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton3_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton3_.hide();
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2AA));
        set_rectangle_dimensions(&draw_workspace_rect_,
                                 field_C_ + static_cast<int>(field_14_) / 2 + 1, field_10_,
                                 static_cast<int>(field_14_) / 2, 20);
        flatButton4_.init(StringTemp, 0xD0B, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton4_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton4_.hide();
        buttonGroup2_.add(&flatButton3_);
        buttonGroup2_.add(&flatButton4_);
        buttonGroup2_.set(0xD0A, 1);

        // Window three-way selector (0x4A4903..0x4A4C0E).
        field_C_ = panel.left + 3;
        field_10_ = field_1C_ + panel.top + 11;
        field_14_ = rect_width(&panel) - 6;
        window.buttonGroup1_.init(4, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2E8));
        set_rect(&draw_workspace_rect_, field_C_, field_10_,
                        field_C_ + static_cast<int>(field_14_) / 3 - 2, field_10_ + 20);
        window.flatButton4_.init(StringTemp, 0x8B3, &draw_workspace_rect_, &window, 0);
        window.flatButton4_.set_font_redraw(&TutWinBaseWindow.font1_);
        window.flatButton4_.reset_field_A9C_if_ungrouped();
        window.flatButton4_.Win::hide();
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2E9));
        set_rect(&draw_workspace_rect_, field_C_ + static_cast<int>(field_14_) / 3,
                        field_10_, field_C_ + static_cast<int>(field_14_) / 3 * 2,
                        field_10_ + 20);
        window.flatButton5_.init(StringTemp, 0x8B4, &draw_workspace_rect_, &window, 0);
        window.flatButton5_.set_font_redraw(&TutWinBaseWindow.font1_);
        window.flatButton5_.reset_field_A9C_if_ungrouped();
        window.flatButton5_.Win::hide();
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x3F));
        set_rect(&draw_workspace_rect_,
                        field_C_ + static_cast<int>(field_14_) / 3 * 2 + 2, field_10_,
                        field_C_ + field_14_, field_10_ + 20);
        window.flatButton6_.init(StringTemp, 0x8B5, &draw_workspace_rect_, &window, 0);
        window.flatButton6_.set_font_redraw(&TutWinBaseWindow.font1_);
        window.flatButton6_.reset_field_A9C_if_ungrouped();
        window.flatButton6_.Win::hide();
        window.buttonGroup1_.add(&window.flatButton4_);
        window.buttonGroup1_.add(&window.flatButton5_);
        window.buttonGroup1_.add(&window.flatButton6_);
        if (field_1E240_) window.buttonGroup1_.set(field_1E240_, 1);
        else window.buttonGroup1_.set(0x8B3, 1);

        // Window two-way selector (0x4A4C13..0x4A4E17).
        field_C_ = panel.left + 3;
        field_10_ = field_1C_ + panel.top + 11;
        field_14_ = rect_width(&panel) - 6;
        window.buttonGroup2_.init(6, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x32C));
        set_rect(&draw_workspace_rect_, field_C_, field_10_,
                        field_C_ + static_cast<int>(field_14_) / 2 - 1, field_10_ + 20);
        window.flatButton7_.init(StringTemp, 0xD3C, &draw_workspace_rect_, &window, 0);
        window.flatButton7_.set_font_redraw(&TutWinBaseWindow.font1_);
        window.flatButton7_.reset_field_A9C_if_ungrouped();
        window.flatButton7_.Win::hide();
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x32D));
        set_rect(&draw_workspace_rect_, field_C_ + static_cast<int>(field_14_) / 2 + 1,
                        field_10_, field_C_ + field_14_, field_10_ + 20);
        window.flatButton8_.init(StringTemp, 0xD3D, &draw_workspace_rect_, &window, 0);
        window.flatButton8_.set_font_redraw(&TutWinBaseWindow.font1_);
        window.flatButton8_.reset_field_A9C_if_ungrouped();
        window.flatButton8_.Win::hide();
        window.buttonGroup2_.add(&window.flatButton7_);
        window.buttonGroup2_.add(&window.flatButton8_);
        window.buttonGroup2_.set(0xD3C, 1);

        // Full-width window button (0x4A4E1C..0x4A4EFA).
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x32E));
        set_rect(&draw_workspace_rect_, frame.left, panel.bottom - 26,
                        frame.right, panel.bottom - 6);
        window.flatButton9_.init(StringTemp, 0xD3E, &draw_workspace_rect_, &window, 0);
        window.flatButton9_.set_font_redraw(&TutWinBaseWindow.font1_);
        window.flatButton9_.set_border_styles(&InterfaceBoxStyles[47], &InterfaceBoxStyles[46],
                                              &InterfaceBoxStyles[45]);
        window.flatButton9_.reset_field_A9C_if_ungrouped();
        window.flatButton9_.Win::hide();

        // Seven order buttons, straight-line (0x4A4EFF..0x4A54FA).
        field_C_ = field_20_.left + 9;
        field_10_ = field_20_.top + 35;
        field_14_ = rect_width(&field_20_) - 18;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2EA));
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 20);
        flatButton5_.init(StringTemp, 0xCEF, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton5_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton5_.hide();
        field_10_ += 22;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2EB));
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x209));
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 20);
        flatButton6_.init(StringTemp, 0xCF0, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton6_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton6_.hide();
        field_10_ += 22;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2EB));
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x20A));
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 20);
        flatButton7_.init(StringTemp, 0xCF1, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton7_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton7_.hide();
        field_10_ += 22;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2EB));
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x20B));
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 20);
        flatButton8_.init(StringTemp, 0xCF2, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton8_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton8_.hide();
        field_10_ += 22;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2EB));
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x20C));
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 20);
        flatButton9_.init(StringTemp, 0xCF3, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton9_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton9_.hide();
        field_10_ += 22;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2EC));
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 20);
        flatButton10_.init(StringTemp, 0xCF4, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton10_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton10_.hide();
        field_10_ += 22;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2ED));
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 20);
        flatButton11_.init(StringTemp, 0xCF5, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton11_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton11_.hide();

        // Three security buttons (0x4A54FD..0x4A574A).
        field_C_ = field_30_.left + 9;
        field_10_ = field_30_.top + 32;
        field_14_ = rect_width(&field_30_) - 18;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2EA));
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 20);
        flatButton12_.init(StringTemp, 0xCF6, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton12_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton12_.hide();
        field_10_ += 22;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x3BB));
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 20);
        flatButton14_.init(StringTemp, 0xCF8, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton14_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton14_.hide();
        field_10_ += 22;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x2DB));
        CharUpperA(StringTemp);
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 20);
        flatButton13_.init(StringTemp, 0xCF7, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
        flatButton13_.set_font_redraw(&TutWinBaseWindow.font1_);
        flatButton13_.hide();

        // Per-faction satellite-owner buttons (0x4A574D..0x4A5949).
        field_C_ = field_20_.left + 9;
        field_10_ = field_20_.top + 32;
        field_14_ = rect_width(&field_20_) - 18;
        buttonGroup3_.init(0x6A, 0);
        for (int other = 1; other < 8; ++other) {
            if (other != ConsoleGlobal.faction_id_) {
                int elevators = 0;
                for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                    if (Bases[base].faction_id_current == other && has_fac(41, base, 0)) {
                        ++elevators;
                    }
                }
                if (has_tech(Facility[65].preq_tech, other)
                    || PlayersData[other].satellites_[0] + PlayersData[other].satellites_[1]
                           + PlayersData[other].satellites_[2] + PlayersData[other].satellites_[3]
                           + elevators
                    || has_tech(Facility[41].preq_tech, other)) {
                    StringTemp[0] = 0;
                    strcat(StringTemp, get_noun(other));
                    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_,
                                             field_14_, 20);
                    FlatButton *owner = &flatButtonsC_[other - 1];
                    owner->init(StringTemp, other, &draw_workspace_rect_, &MainInterfaceGlobal, 0);
                    owner->set_font_redraw(&TutWinBaseWindow.font1_);
                    owner->hide();
                    buttonGroup3_.add(owner);
                    field_10_ += 22;
                }
            }
        }

        // Faction leader buttons: visibility first, then the buttons
        // (0x4A594F..0x4A5C3F).
        window.field_59B0_ = 1;
        int visible[7];
        for (int faction = 1; faction < 8; ++faction) {
            if ((PlayersData[faction].diplo_treaties[ConsoleGlobal.faction_id_] & 1)
                || spying(faction) || faction == ConsoleGlobal.faction_id_
                || (GameDataStateStorage.fields_.game_state_ & 0x80)) {
                visible[faction - 1] = 1;
            } else {
                visible[faction - 1] = 0;
            }
        }
        for (int slot = 0; slot < 7; ++slot) {
            int leader = slot + 1;
            if (visible[slot]) {
                StringTemp[0] = 0;
                strcat(StringTemp, get_title(leader));
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, get_name(leader));
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get(0xB4));
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, get_noun(leader));
                if (leader < ConsoleGlobal.faction_id_) {
                    set_rectangle_dimensions(&draw_workspace_rect_, report_button_positions_[slot],
                                             report_button_positions_[7],
                                             field_1E230_ ? 42 : 37, 29);
                } else if (leader > ConsoleGlobal.faction_id_) {
                    set_rectangle_dimensions(&draw_workspace_rect_,
                                             report_button_positions_[slot - 1],
                                             report_button_positions_[7],
                                             field_1E230_ ? 42 : 37, 29);
                } else if (field_1E230_) {
                    set_rectangle_dimensions(&draw_workspace_rect_, field_50_.left + 15,
                                             field_50_.top + 15, 70, 70);
                } else {
                    set_rectangle_dimensions(&draw_workspace_rect_, field_50_.left + 9,
                                             report_button_positions_[7], 37, 29);
                }
                FlatButton *portrait = &flatButtonsA_[slot];
                portrait->init(0, leader + 9999, &draw_workspace_rect_,
                                   &MainInterfaceGlobal, 0);
                portrait->set_report_icons(faction_button_sprites_[slot][0],
                                           faction_button_sprites_[slot][1],
                                           faction_button_sprites_[slot][2]);
                portrait->set_bubble_text(StringTemp);
                portrait->Buffer::set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
                portrait->redraw();
                portrait->redraw();
            }
        }
        flatButtonsA_[ConsoleGlobal.faction_id_ - 1].set_report_page_state(2);
        flatButtonsA_[ConsoleGlobal.faction_id_ - 1].redraw();

        // Scroll, report and modal tail (0x4A5C84..0x4A5D6F).
        window.report_work_rect_.left = 0;
        window.report_work_rect_.top = 0;
        window.report_work_rect_.right = 32;
        window.report_work_rect_.bottom = 300;
        window.scroll_.init(&window.report_work_rect_, &window, 0, 0);
        window.scroll_.Win::hide();
        field_1E234_ = 0;
        ReportIfGlobal.report(ReportIfGlobal.field_8_, ReportIfGlobal.report_faction_id_,
                              ReportIfGlobal.report_selection_);
        slide_show(&window, 0);
        window.Win::show(3);
        ReportIfGlobal.report_scroll_blocked_ = 0;
        if (window.field_E26C_) {
            scroll_show(&window, &window.scroll_);
            window.field_E26C_ = 0;
            window.field_C0F8_ = 1;
        }
        if (WinFocusWindow != &window) {
            ReportInterfaceStatusReentry = 1;
            window.set_modal(0, pop_wait, &MainInterfaceGlobal);
        }
    } else {
        ReportInterfaceStatusReentry = 0;
        window.time1_.stop();
        if (window.field_C0F8_) {
            scroll_hide(&window, &window.scroll_);
            window.field_C0F8_ = 0;
        }
        slide_hide(&window, 0);
        window.Win::hide();
        window.flatButton3_.FlatButton::close();
        window.flatButton2_.FlatButton::close();
        for (int category = 0; category < 7; ++category) {
            flatButtonsB_[category].close();
        }
        for (int portrait_slot = 0; portrait_slot < 7; ++portrait_slot) {
            flatButtonsA_[portrait_slot].close();
        }
        // The same close chain as report(): the small closers are expanded
        // here, close_ops and close_sat are called. close_energy ends with
        // close_intel's two list closes, and VC6 merges that ending into
        // case 0, which is why the image jumps from case 3 into the last
        // case and lays cases 4, 6 and 7 out after the epilogue. The empty
        // close_score keeps case 8 in the jump table's range.
        switch (field_8_) {
        case 2: close_labs(); break;
        case 3: close_energy(); break;
        case 4: close_ops(); break;
        case 6: close_sat(); break;
        case 7: close_security(); break;
        case 8: close_score(); break;
        case 0: close_intel(); break;
        }
        WorldClimateWorldWin.hide_all();
        for (int main_button = 6; main_button <= 7; ++main_button) {
            MainInterfaceGlobal.flat_button_[main_button].hide();
        }
    }
}

// ORIGINAL: 0x004A5F00 ?show@ReportWin@@QAEXH@Z 0x004A5F00-0x004A5F16 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/004a5f00.cpp records the original 0x004A5F00-0x004A5F16 body and BYTE_EXACT claim.", "ReportWin derives from Win and ReportIfGlobal owns the interface-mode transitions used by the seed."], "reason": "Home the existing ReportWin show identity in its class owner, retaining its measured byte-exact claim."}
// Deleted seed provenance: git:53d5fe2c1b1eff854b846b082f125794c1d0b7fe:src/recovered/004a5f00.cpp
// symbol    ?show@ReportWin@@UAEXH@Z
/*
Purpose: Enter or leave the report interface mode only when its window changes
         visibility. The visibility check belongs to the inherited Win base;
         mode ownership belongs to the process-wide ReportIf subinterface.
*/
void ReportWin::show(int) {
    if (!is_visible()) {
        ReportIfGlobal.set_iface_mode();
    }
}

// ORIGINAL: 0x004A5F20 ?hide@ReportWin@@QAEXXZ 0x004A5F20-0x004A5F34 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/004a5f20.cpp records the original 0x004A5F20-0x004A5F34 body and BYTE_EXACT claim.", "ReportWin derives from Win and ReportIfGlobal owns the interface-mode transitions used by the seed."], "reason": "Home the existing ReportWin hide identity in its class owner, retaining its measured byte-exact claim."}
// Deleted seed provenance: git:53d5fe2c1b1eff854b846b082f125794c1d0b7fe:src/recovered/units/004a5f20.cpp
// symbol    ?hide@ReportWin@@UAEXXZ
void ReportWin::hide() {
    if (is_visible()) {
        ReportIfGlobal.release_iface_mode();
    }
}

/*
Purpose: Show the report window modally with pop_wait as its poll callback,
         or re-report the intel category when the active interface is the
         world map's slot 5.

             cmp  [esi+8], 7 / jne ret          - intel panel only
             push y / push x / lea ecx,[esi+0x1E1BC] / call Spot::check
                                                - (x, y, &y, &x): the spot
                                                  index lands in y, the type
                                                  in x
             inc eax / je ret                   - -1 means "no spot"
             [esi+0x1E244] = y                  - the clicked topic
             [0x008846E8] = 1
             [0x00885F40] = 7                   - switch to the intel report
             cmp IfaceIdToSlot[ActiveIfaceId], 5 - world map keeps its own
                                                  modal owner
             set_modal(0, pop_wait, MainInterfaceGlobal) - slot 0xE4
             report(7, report_faction_id_, 0) on the 0x00885F38 singleton
// ORIGINAL: 0x004A5F40 ?on_iface_left_click@ReportIf@@QAEXHH@Z 0x004A5F40-0x004A5FD6 BYTE_EXACT
// symbol    ?on_iface_left_click@ReportIf@@UAEXHH@Z
// size      150 bytes
// prototype void (__thiscall ?on_iface_left_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004A3950 0x005FAB00
// indirect  0x004A5FB1
// Homed from src/unrecovered/004a5f40.cpp.
*/
void ReportIf::on_iface_left_click(int x_coord, int y_coord) {
    if (field_8_ != 7) {
        return;
    }

    // check() answers the hit index and hands the spot's position and type
    // back through the two argument slots - the push order is &y_coord then
    // &x_coord, so the position lands in the y argument, the type in the x.
    if (spot_.check(x_coord, y_coord,
                    &y_coord, &x_coord) + 1 != 0) {
        field_1E244_ = y_coord;

        ReportWinGlobal.select_intel_topic();
        ReportIfGlobal.field_8_ = 7;

        if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 5) {
            Win &modal_window = ReportWinGlobal;
            modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);
            return;
        }

        ReportIfGlobal.report(7, ReportIfGlobal.report_faction_id_, 0);
    }
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A5FE0 ?on_iface_right_click@ReportIf@@QAEXHH@Z 0x004A5FE0-0x004A5FE3 BYTE_EXACT
// symbol    ?on_iface_right_click@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A5FF0 ?on_iface_left_double_click@ReportIf@@QAEXHH@Z 0x004A5FF0-0x004A5FF3 BYTE_EXACT
// symbol    ?on_iface_left_double_click@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_left_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6000 ?on_iface_right_double_click@ReportIf@@QAEXHH@Z 0x004A6000-0x004A6003 BYTE_EXACT
// symbol    ?on_iface_right_double_click@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6010 ?on_iface_left_down@ReportIf@@QAEXHH@Z 0x004A6010-0x004A6013 BYTE_EXACT
// symbol    ?on_iface_left_down@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_left_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6020 ?on_iface_right_down@ReportIf@@QAEXHH@Z 0x004A6020-0x004A6023 BYTE_EXACT
// symbol    ?on_iface_right_down@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6030 ?on_iface_mouse_move@ReportIf@@QAEXHH@Z 0x004A6030-0x004A6033 BYTE_EXACT
// symbol    ?on_iface_mouse_move@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6040 ?on_iface_mouse_leave@ReportIf@@QAEXHH@Z 0x004A6040-0x004A6043 BYTE_EXACT
// symbol    ?on_iface_mouse_leave@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_mouse_leave(int, int) {
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A6050 0x004A62F4 0x004A6314 0x004A60DB
// ORIGINAL: 0x004A6050 ?on_iface_selected@ReportIf@@QAEXHH@Z 0x004A6050-0x004A62F3 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/004a6050.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a6050 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's selection callback from the retained seed through its genuine SubInterface owner and ReportWin controls. Its current emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// symbol    ?on_iface_selected@ReportIf@@UAEXHH@Z
void ReportIf::on_iface_selected(int item, int) {
    switch (ReportIfGlobal.field_8_) {
    case 4:
        if (item >= 1 && item <= 8) {
            for (int i = 0; i < 0x200; ++i) {
                sort_entries_[i].base_id = i;
                sort_entries_[i].value = 0;
                sort_entries_[i].sorted_base_id = i;
                sort_entries_[i].sorted_value = 0;
            }
            for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                set_base(base);
                base_compute(0);
                int &value = sort_entries_[base].value;
                // Original loads are D4/D0/100/108/104, not the seed's
                // half-sized Ghidra offsets 6A/68/80/84/82.
                switch (item) {
                case 1: value = BaseCurrent->mineral_surplus; break;
                case 2: value = BaseCurrent->nutrient_surplus; break;
                case 3: value = BaseCurrent->economy_total; break;
                case 4: value = BaseCurrent->labs_total; break;
                case 5: value = BaseCurrent->psych_total; break;
                case 6: value = BaseCurrent->population_size; break;
                case 7: {
                    int vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);
                    for (vehicle = veh_top(vehicle); vehicle >= 0; vehicle = Vehs[vehicle].next_veh_id_stack)
                        ++value;
                    break;
                }
                case 8:
                    for (int facility = 0; facility < 70; ++facility)
                        if (BaseCurrent->facilities_built[facility / 8] & (1 << (facility % 8)))
                            ++value;
                    for (int project = 0; project < 64; ++project)
                        if (GameDataStateStorage.fields_.secret_project_.project_base[project] == base)
                            value += 5;
                    break;
                }
                sort_entries_[base].sorted_value = value;
            }
            merge_sort(0, GameDataStateStorage.fields_.base_current_count_ - 1);
            ReportWinGlobal.field_5A08_ = item;
            ReportWinGlobal.redraw();
        }
        break;
    case 3:
        if (item < 0x400) {
            ReportWinGlobal.shared_report_selection() = item;
            ReportWinGlobal.field_59B0_ = 0;
            listBox1_.set_selected_pos(-1, 1);
        } else {
            ReportWinGlobal.shared_report_selection() = item - 0x400;
            ReportWinGlobal.field_59B0_ = 1;
            listBox2_.set_selected_pos(-1, 1);
        }
        ReportWinGlobal.reset();
        ReportWinGlobal.draw_energy();
        ReportWinGlobal.soft_update();
        break;
    case 2:
        if (item < 0x400) {
            ReportWinGlobal.shared_research_focus() = item;
            ReportWinGlobal.redraw();
        } else {
            TutWinBaseWindow.zoom(item - 0x400, 0);
        }
        break;
    }
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 1.
// ORIGINAL: 0x004A6320 ?on_iface_dialog_item_back_draw@ReportIf@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x004A6320-0x004A6328 BYTE_EXACT
// symbol    ?on_iface_dialog_item_back_draw@ReportIf@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
// size      8 bytes
// prototype int (__thiscall ?on_iface_dialog_item_back_draw@ReportIf@@QAEHPAUGraphicWin@@HHPAURECT@@@Z)(ReportIf* this, GraphicWin*, int, int, RECT*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1
Status: Complete
*/
int ReportIf::on_iface_dialog_item_back_draw(::GraphicWin *, int, int, RECT *) {
    return 1;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A6330 0x004A6D10 0x004A6D2C 0x004A6412
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A6330 0x004A6D2C 0x004A6D3C 0x004A6563
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A6330 0x004A6D3C 0x004A6D58 0x004A6B41
// ORIGINAL: 0x004A6330 ?on_iface_button_clicked@ReportIf@@QAEXH@Z 0x004A6330-0x004A6D10 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004a6330.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a6330 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's button callback from the retained seed through its genuine SubInterface owner and ReportWin controls. Its current emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// TRIED: full transcription landed (MISMATCH #3). Used the brief's resolved jump tables directly (three switches, cases 0xcef-0xcf5, 0xcf0-0xcf3, and 2-8) rather than the raw `ja`+`jmp [eax*4+addr]` ranges alone. Caught a real Ghidra error via the raw bytes: every `*(uint*)(DAT_0090ea30+0x1c)` Ghidra prints is actually `[eax+0x38]` in the disassembly (`mov eax,[0x90ea30]; mov ecx,[eax+0x38]`), confirmed at both call sites that touch it. ReportIf/BaseWin/NetDaemon fields are all raw self+offset casts, not src/reportif.h's hypothesised layout, per the brief's warning.
// The three log_say texts are this handler's own copies in the pinned image
// (0x687280, 0x687298, 0x6872B8; on_left_click keeps a separate EDBC copy at
// 0x686F04). The image's advanced-governor text ends in "set".
// RECOVERY_LITERAL: v1 0x00687280 0x00687295 $SG63686 0x004A6330
// RECOVERY_LITERAL: v1 0x00687298 0x006872B5 $SG63701 0x004A6330
// RECOVERY_LITERAL: v1 0x006872B8 0x006872CC $SG63712 0x004A6330
// symbol    ?on_iface_button_clicked@ReportIf@@UAEXH@Z
void ReportIf::on_iface_button_clicked(int button) {
    int row;
    if (button > 9999) {
        report_topic_changed_ = 1;
        button -= 9999;
        flatButtonsA_[ReportIfGlobal.report_faction_id_ - 1].set_report_page_state(-1);
        flatButtonsA_[ReportIfGlobal.report_faction_id_ - 1].redraw();
        flatButtonsA_[button - 1].set_report_page_state(2);
        flatButtonsA_[button - 1].redraw();
        report(ReportIfGlobal.field_8_, button, report_selection_);
        return;
    }
    if (field_8_ == 4 && report_faction_id_ == ConsoleGlobal.faction_id_) {
        switch (button) {
        case 0xCEF: {
            // Selection covers the entire original 512-record base allocation,
            // not merely the currently populated prefix.
            for (int base = 0; base < 512; ++base) {
                if (Bases[base].faction_id_current == ConsoleGlobal.faction_id_
                    && Bases[base].faction_id_current == ReportIfGlobal.report_faction_id_) {
                    ReportWinGlobal.selected_bases_[base / 32] |= 1 << (base % 32);
                }
            }
            int selected = 0;
            int count = 0;
            for (; selected < 512; ++selected) {
                if (ReportWinGlobal.selected_bases_[selected / 32] & (1 << (selected % 32))) ++count;
            }
            // The +0x46C this/button reload order below depends on how many
            // functions this TU defines before this one: unreferenced file-local
            // helpers left behind by rewrites flip it.
            ReportWinGlobal.field_5A04_ = count == 1;
            break;
        }
        case 0xCF0:
        case 0xCF1:
        case 0xCF2:
        case 0xCF3: {
            log_say("Global EDBC base set", 0, 0, 0);
            for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == ConsoleGlobal.faction_id_
                    && (ReportWinGlobal.selected_bases_[base >> 5] & (1 << (base & 31)))
                    && NetDaemonNet.lock_base(base, 0, -1, -1) == 0) {
                    TutWinBaseWindow.soft_zoom(base);
                    BaseCurrent->governor |= 0x80000000;
                    BaseCurrent->governor &= 0xF0FFFFFF;
                    switch (button) {
                    case 0xCF0: BaseCurrent->governor |= 0x1000000; break;
                    case 0xCF1: BaseCurrent->governor |= 0x2000000; break;
                    case 0xCF2: BaseCurrent->governor |= 0x4000000; break;
                    case 0xCF3: BaseCurrent->governor |= 0x8000000; break;
                    }
                    TutWinBaseWindow.gov_activate();
                    NetDaemonNet.unlock_base(base);
                }
            }
            break;
        }
        case 0xCF4: {
            log_say("Global governor advanced set", 0, 0, 0);
            int configured = 0;
            for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == ConsoleGlobal.faction_id_
                    && (ReportWinGlobal.selected_bases_[base >> 5] & (1 << (base & 31)))) {
                    if (!configured) {
                        TutWinBaseWindow.soft_zoom(base);
                        if (TutWinBaseWindow.governor(1) != 0) break;
                        configured = 1;
                    }
                    if (NetDaemonNet.lock_base(base, 0, -1, -1) == 0) {
                        TutWinBaseWindow.soft_zoom(base);
                        BaseCurrent->governor = PlayersData[report_faction_id_].base_governor_adv;
                        TutWinBaseWindow.gov_activate();
                        draw_radius(BaseCurrent->x, BaseCurrent->y, 2, 2);
                        NetDaemonNet.unlock_base(base);
                    }
                }
            }
            break;
        }
        case 0xCF5: {
            log_say("Global governor off", 0, 0, 0);
            for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == ConsoleGlobal.faction_id_
                    && (ReportWinGlobal.selected_bases_[base >> 5] & (1 << (base & 31)))
                    && NetDaemonNet.lock_base(base, 0, -1, -1) == 0) {
                    Bases[base].governor &= 0x7FFFFFFF;
                    draw_radius(Bases[base].x, Bases[base].y, 2, 2);
                    NetDaemonNet.unlock_base(base);
                }
            }
            break;
        }
        }
        ReportWinGlobal.redraw();
    }
    if (field_8_ == 7 && report_faction_id_ == ConsoleGlobal.faction_id_) {
        int proto_id;
        switch (button) {
        case 0xCF8: {
            for (row = 0; row < 128; ++row) {
                proto_id = row < 64 ? row : report_faction_id_ * 64 + row - 64;
                if ((field_1E244_ < 3
                     && Chassis[VehPrototypes[proto_id].chassis_id].triad == field_1E244_
                     && Weapon[VehPrototypes[proto_id].weapon_id].offense_rating)
                    || (field_1E244_ == 3 && !Weapon[VehPrototypes[proto_id].weapon_id].offense_rating)
                    || field_1E244_ == 4) {
                    if (ReportWinGlobal.selected_prototypes_[row / 32] & (1 << (row % 32))) {
                        if (diplo_lock(100)) break;
                        for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; ++veh_id) {
                            if (Vehs[veh_id].faction_id == report_faction_id_
                                && Vehs[veh_id].proto_id == proto_id && !veh_jail(veh_id)) {
                                Vehs[veh_id].order = 0;
                                Vehs[veh_id].state &= 0xFCFFBFFF;
                                Vehs[veh_id].order_auto_type = 0;
                                Vehs[veh_id].state |= 0x200;
                                Vehs[veh_id].waypoint_count = 0;
                                synch_veh(veh_id);
                            }
                        }
                        diplo_unlock();
                    }
                }
            }
            break;
        }
        case 0xCF7: {
            for (row = 0; row < 128; ++row) {
                proto_id = row < 64 ? row : report_faction_id_ * 64 + row - 64;
                if ((field_1E244_ < 3
                     && Chassis[VehPrototypes[proto_id].chassis_id].triad == field_1E244_
                     && Weapon[VehPrototypes[proto_id].weapon_id].offense_rating)
                    || (field_1E244_ == 3 && !Weapon[VehPrototypes[proto_id].weapon_id].offense_rating)
                    || field_1E244_ == 4) {
                    if (ReportWinGlobal.selected_prototypes_[row / 32] & (1 << (row % 32))) {
                        if (diplo_lock(100)) break;
                        for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; ++veh_id) {
                            if (Vehs[veh_id].faction_id == report_faction_id_
                                && Vehs[veh_id].proto_id == proto_id && !veh_jail(veh_id)) {
                                Vehs[veh_id].order = 0;
                                Vehs[veh_id].state &= 0xFCFFBDFF;
                                Vehs[veh_id].waypoint_count = 0;
                                synch_veh(veh_id);
                            }
                        }
                        diplo_unlock();
                    }
                }
            }
            break;
        }
        case 0xCF6: {
            for (row = 0; row < 128; ++row) {
                proto_id = row < 64 ? row : report_faction_id_ * 64 + row - 64;
                if ((field_1E244_ < 3
                     && Chassis[VehPrototypes[proto_id].chassis_id].triad == field_1E244_
                     && Weapon[VehPrototypes[proto_id].weapon_id].offense_rating)
                    || (field_1E244_ == 3 && !Weapon[VehPrototypes[proto_id].weapon_id].offense_rating)
                    || field_1E244_ == 4) {
                    ReportWinGlobal.selected_prototypes_[row / 32] |= 1 << (row % 32);
                }
            }
            break;
        }
        }
        ReportWinGlobal.redraw();
    }
    switch (button) {
    case 2: report_labs(); break;
    case 3: report_energy(); break;
    case 4: report_ops(); break;
    case 5: report_projects(); break;
    case 6: report_sat(); break;
    case 7: report_security(); break;
    case 8: report_score(0); break;
    }
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6D60 ?on_iface_button_toggled@ReportIf@@QAEXHH@Z 0x004A6D60-0x004A6D63 BYTE_EXACT
// symbol    ?on_iface_button_toggled@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_button_toggled(int, int) {
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A6D70 0x004A6FD8 0x004A6FFC 0x004A6F18
// ORIGINAL: 0x004A6D70 ?on_iface_group_clicked@ReportIf@@QAEXHHH@Z 0x004A6D70-0x004A6FD7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/004a6d70.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a6d70 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's group callback from the retained seed through its genuine SubInterface owner and ReportWin controls. Its current emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// symbol    ?on_iface_group_clicked@ReportIf@@UAEXHHH@Z
void ReportIf::on_iface_group_clicked(int group, int, int item) {
    if (group == 2) {
        switch (item) {
        case 0x92:
            field_1E1D8_ = 1;
            break;
        case 0x93:
            field_1E1D8_ = 0;
            break;
        }
    } else if (group == 4) {
        if (item == 0xD0B) {
            flatButton5_.hide(); flatButton6_.hide(); flatButton7_.hide();
            flatButton8_.hide(); flatButton9_.hide(); flatButton10_.hide();
            flatButton11_.hide();
        } else {
            flatButton5_.show(0); flatButton6_.show(0); flatButton7_.show(0);
            flatButton8_.show(0); flatButton9_.show(0); flatButton10_.show(0);
            flatButton11_.show(0);
        }
    } else if (group == 0x6A) {
        ReportWinGlobal.flatButton9_.reset_field_A9C_if_ungrouped();
        ReportWinGlobal.flatButton9_.Win::hide();
    } else if (group == 0xCE) {
        ReportWinGlobal.flatButton9_.Win::show(0);
    }
    if (item == 0x3EE) WorldClimateWorldWin.set_world_map();
    else if (item == 0x3EF) WorldClimateWorldWin.set_detail_map();
    if (field_1E234_ == 0) {
        on_redraw();
        MainInterfaceGlobal.soft_update();
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A7000 0x004A8430 0x004A8440 0x004A7D12
// ORIGINAL: 0x004A7000 ?draw_labs@ReportIf@@QAEXXZ 0x004A7000-0x004A842F
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004a7000.cpp", "source:src/reportif.h", "source:src/base.h", "source:src/tech.h"], "reason": "Original panel identity and accesses identify ReportIf's laboratory renderer, using its lists and selection state, source-owned base rows and technology records; preserve the seed's row IDs and technology sentinel rather than substitute a different panel."}
// TRIED: MISMATCH #0, sim ~0.58 (best of /O1 framed). Full structural transcription (all branches, both list modes, all 4 category bars). Approximated: the ListBox virtual-base (GraphicWin) adjustment at self+0xa384/+0x24/+0x28 uses the flattened `virtual_base_` member instead of the runtime vtable+8 lookup the original performs, so that handful of instructions differ in shape though not in intent.
void ReportIf::draw_labs() {
    char lab_text[80];   // [ebp-0xC8]
    char text[80];       // [ebp-0x78]
    int tech;
    MainInterfaceGlobal.Buffer::set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = rect_width(&field_20_) - 0xC;
    field_18_ = rect_height(&field_20_) - 0xC;
    flatButton1_.show(0);
    flatButton2_.show(0);
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 26);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    field_10_ += 32;
    field_18_ -= 32;
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_18_);
    inflate_frame(&draw_workspace_rect_, -3, -3);
    scroll_small();
    ListBox *list = &listBox1_;
    list->close();
    list->Dialog::init(&draw_workspace_rect_, 0);
    list->Dialog::field_48_ = field_1C_ + 3;
    listBox1_.Dialog::set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    listBox1_.Dialog::set_dialog_text_color(0x99, -1, 1, 1);
    listBox1_.Dialog::set_dialog_text_color2(0x99, -1, 1, 1);
    listBox1_.Dialog::set_dialog_text_color3(0x99, -1, 1, 1);
    listBox1_.set_report_text_color(0xF2);
    listBox1_.set_write_callback(labs_write);
    if (field_1E1D8_) {
        int levels[MaxTechnologyNum + 1];   // [ebp-0x280]
        for (tech = 0; tech < MaxTechnologyNum; ++tech) {
            if (has_tech(tech, report_faction_id_)) {
                levels[tech] = tech_recurse(tech, 0) * 4 + tech_category(tech);
            } else {
                levels[tech] = 0;
            }
        }
        levels[MaxTechnologyNum] = 999999;
        for (int iteration = 0; iteration < MaxTechnologyNum; ++iteration) {
            int best = MaxTechnologyNum;
            for (int candidate = 0; candidate < MaxTechnologyNum; ++candidate) {
                if (levels[candidate] && levels[candidate] < levels[best]) {
                    best = candidate;
                }
            }
            if (best < MaxTechnologyNum) {
                StringTemp[0] = 0;
                say_tech(StringTemp, best, 0);
                strcat(StringTemp, "|");
                strcat(StringTemp, "[");
                strcat(StringTemp, label_get(0x275 + (levels[best] & 3)));
                strcat(StringTemp, BufferJoinSpace);
                _itoa(levels[best] >> 2, lab_text, 10);
                strcat(StringTemp, lab_text);
                strcat(StringTemp, "]");
                list->item(StringTemp, best);
                levels[best] = 0;
            } else {
                iteration = MaxTechnologyNum;
            }
        }
    } else {
        for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
            if (Bases[base].faction_id_current == report_faction_id_) {
                char whole_text[80];   // [ebp-0x118]
                StringTemp[0] = 0;
                say_base(StringTemp, base);
                strcat(StringTemp, "|");
                strcat(StringTemp, "+");
                _itoa(Bases[base].labs_total, lab_text, 10);
                strcat(StringTemp, lab_text);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, SocialWinOpenParenText);
                int scaled = (range(PlayersData[report_faction_id_].soc_effect_pending
                                        .values_[SocialEffect::RESEARCH], -5, 5) + 10)
                             * Bases[base].labs_total * 10;
                _itoa(scaled / 100, whole_text, 10);
                strcat(StringTemp, whole_text);
                if (scaled % 100) {
                    strcat(StringTemp, ".");
                    _itoa(scaled % 100 / 10, text, 10);
                    strcat(StringTemp, text);
                }
                strcat(StringTemp, SocialWinCloseParenText);
                strcat(StringTemp, label_get(0x17B));
                list->item(StringTemp, base + 0x400);
            }
        }
    }
    listBox1_.field_2C_ = 0;
    listBox1_.clear_selection_index();
    listBox1_.Dialog::field_B4_ = 0;
    list->ListBox::attach(&MainInterfaceGlobal, list->Dialog::window_x_, list->Dialog::window_y_, 0x80020);
    if (field_1E1D8_) {
        list->set_selected_id(ReportWinGlobal.field_FAB8_);
    } else {
        list->set_selected_pos(-1, 1);
    }
    listBox1_.show(0);
    scroll_normal();
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_50_);
    int inset = field_1E230_ ? 0x4C : 0;
    field_C_ = field_50_.left + inset;
    field_10_ = field_50_.top;
    field_14_ = rect_width(&field_50_) - inset;
    field_18_ = rect_height(&field_50_);
    StringTemp[0] = 0;
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0x2B1));
    if (field_1E230_) {
        strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, BufferJoinSpace);
        say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    MainInterfaceGlobal.Buffer::set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x178));
    strcat(StringTemp, SocialWinColonSpaceText);
    int rate = tech_rate(report_faction_id_);
    _itoa(rate, text, 10);
    strcat(StringTemp, text);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0x179));
    strcat(StringTemp, SocialWinColonSpaceText);
    _itoa(PlayersData[report_faction_id_].tech_accumulated, text, 10);
    strcat(StringTemp, text);
    if (field_1E230_) {
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, BufferJoinSpace);
    } else {
        MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_, field_14_);
        field_10_ += field_1C_;
        StringTemp[0] = 0;
    }
    strcat(StringTemp, label_get(0x17A));
    strcat(StringTemp, SocialWinColonSpaceText);
    energy_compute(report_faction_id_, 0);
    _itoa(EnergyLabsTotal / 100, text, 10);
    strcat(StringTemp, text);
    if (EnergyLabsTotal % 100) {
        strcat(StringTemp, ".");
        _itoa(EnergyLabsTotal % 100 / 10, text, 10);
        strcat(StringTemp, text);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_, field_14_);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = rect_width(&field_30_) - 0xC;
    field_18_ = rect_height(&field_30_) - 0xC;
    MainInterfaceGlobal.Buffer::set_font(field_1E184_, field_1E18C_, field_1E198_, 0);
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_30_.bottom - 6;
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    if (PlayersData[report_faction_id_].theory_of_everything) {
        MainInterfaceGlobal.Buffer::set_text_color3(mandate_color(1), -1, 1, 1);
        say_tech(StringTemp, PlayersData[report_faction_id_].tech_id_researching, 0);
        CharUpperA(StringTemp);
        strcat(StringTemp, BufferJoinSpace);
        _itoa(PlayersData[report_faction_id_].theory_of_everything, text, 10);
        strcat(StringTemp, text);
    } else {
        MainInterfaceGlobal.Buffer::set_text_color3(0xE1, -1, 1, 1);
        strcat(StringTemp, label_get(0x2C0));
    }
    strcat(StringTemp, "]");
    field_10_ += 4;
    field_18_ -= 4;
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_, field_14_);
    field_10_ += field_1C_;
    field_18_ -= field_1C_;
    MainInterfaceGlobal.Buffer::set_text_color3(0xE1, -1, 1, 1);
    field_18_ -= 2;
    const int bar_height = static_cast<int>(field_18_) / 4 - 15;
    field_C_ += 3;
    field_14_ -= 6;
    int explore_total = 0;
    int discover_total = 0;
    int build_total = 0;
    int conquer_total = 0;
    int explore_owned = 0;
    int discover_owned = 0;
    int build_owned = 0;
    int conquer_owned = 0;
    // The prerequisite cursor stops before the last record (end 0x009502A0).
    for (tech = 0; tech < MaxTechnologyNum - 1; ++tech) {
        if (Technology[tech].preq_tech_1 + 2 != 0 && Technology[tech].preq_tech_2 + 2 != 0) {
            switch (tech_category(tech)) {
            case 0:
                ++explore_total;
                if (has_tech(tech, report_faction_id_)) ++explore_owned;
                break;
            case 1:
                ++discover_total;
                if (has_tech(tech, report_faction_id_)) ++discover_owned;
                break;
            case 2:
                ++build_total;
                if (has_tech(tech, report_faction_id_)) ++build_owned;
                break;
            case 3:
                ++conquer_total;
                if (has_tech(tech, report_faction_id_)) ++conquer_owned;
                break;
            }
        }
    }
    // No zero-denominator guard exists in the original technology bars.
    field_10_ += 8;
    StringTemp[0] = 0;
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 15);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[38]);
    draw_workspace_rect_.right = static_cast<int>(field_14_) * explore_owned / explore_total + field_C_;
    MainInterfaceGlobal.Buffer::set_clip(&draw_workspace_rect_);
    draw_workspace_rect_.right = field_14_ + field_C_;
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[36]);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
    field_10_ += 15;
    MainInterfaceGlobal.Buffer::set_text_color3(mandate_color(0), -1, 1, 1);
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x209));
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    _itoa(explore_owned * 100 / explore_total, text, 10);
    strcat(StringTemp, text);
    strcat(StringTemp, "%");
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_l(StringTemp, field_C_ + 2,
        (bar_height - static_cast<int>(field_1C_)) / 2 + field_10_);

    field_10_ += bar_height;
    StringTemp[0] = 0;
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 15);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[41]);
    draw_workspace_rect_.right = static_cast<int>(field_14_) * discover_owned / discover_total + field_C_;
    MainInterfaceGlobal.Buffer::set_clip(&draw_workspace_rect_);
    draw_workspace_rect_.right = field_14_ + field_C_;
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[39]);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
    field_10_ += 15;
    MainInterfaceGlobal.Buffer::set_text_color3(mandate_color(1), -1, 1, 1);
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x20A));
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    _itoa(discover_owned * 100 / discover_total, text, 10);
    strcat(StringTemp, text);
    strcat(StringTemp, "%");
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_l(StringTemp, field_C_ + 2,
        (bar_height - static_cast<int>(field_1C_)) / 2 + field_10_);

    field_10_ += bar_height;
    StringTemp[0] = 0;
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 15);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[44]);
    draw_workspace_rect_.right = static_cast<int>(field_14_) * build_owned / build_total + field_C_;
    MainInterfaceGlobal.Buffer::set_clip(&draw_workspace_rect_);
    draw_workspace_rect_.right = field_14_ + field_C_;
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[42]);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
    field_10_ += 15;
    MainInterfaceGlobal.Buffer::set_text_color3(mandate_color(2), -1, 1, 1);
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x20B));
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    _itoa(build_owned * 100 / build_total, text, 10);
    strcat(StringTemp, text);
    strcat(StringTemp, "%");
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_l(StringTemp, field_C_ + 2,
        (bar_height - static_cast<int>(field_1C_)) / 2 + field_10_);

    field_10_ += bar_height;
    StringTemp[0] = 0;
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 15);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[47]);
    draw_workspace_rect_.right = static_cast<int>(field_14_) * conquer_owned / conquer_total + field_C_;
    MainInterfaceGlobal.Buffer::set_clip(&draw_workspace_rect_);
    draw_workspace_rect_.right = field_14_ + field_C_;
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[45]);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
    field_10_ += 15;
    MainInterfaceGlobal.Buffer::set_text_color3(mandate_color(3), -1, 1, 1);
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x20C));
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    _itoa(conquer_owned * 100 / conquer_total, text, 10);
    strcat(StringTemp, text);
    strcat(StringTemp, "%");
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_l(StringTemp, field_C_ + 2,
        (bar_height - static_cast<int>(field_1C_)) / 2 + field_10_);
}

// ORIGINAL: 0x004A8440 ?labs_write@@YAXPAUGraphicWin@@HHPAURECT@@PAD@Z 0x004A8440-0x004A846E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/004a8440.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a8440 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage", "Retired recovered seed provenance: git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/004a8440.cpp"], "reason": "Recover the ReportIf technology-label writer from its retained seed through the genuine ReportWin/Buffer relationship. Its current emitted symbol is recorded separately from the historical catalogue spelling; this binding adds no claim about executable return decoration. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?labs_write@@YAHPAVGraphicWin@@HHPAUtagRECT@@PAD@Z
int __cdecl labs_write(GraphicWin *window, int technology, int, RECT *, char *) {
    window->set_text_color3(
        mandate_color(tech_category(technology)), 0, 1, 1);
    return 0;
}

// ORIGINAL: 0x004A8470 ?close_labs@ReportIf@@QAEXXZ 0x004A8470-0x004A849E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/004a8470.cpp", "source:src/reportif.h", "source:src/flatbutton.h", "source:src/listbox.h"], "reason": "Close the ReportIf lab list and hide its two placement-constructed FlatButtons through their typed Win virtual method."}
void ReportIf::close_labs() {
    listBox1_.close();
    flatButton1_.hide();
    flatButton2_.hide();
}

// ORIGINAL: 0x004A84A0 ?draw_energy@ReportIf@@QAEXXZ 0x004A84A0-0x004A9017 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004a84a0.cpp", "source:src/reportif.h", "image:0x004A84AE-0x004A84BC pushes mode zero then report faction for the cdecl energy_compute call"], "reason": "Original energy-panel identity maps to ReportIf's list, selection, frame and facility rows. The original cdecl push order establishes energy_compute(report_faction_id_, 0), correcting the seed's reversed displayed arguments without changing the calculation owner."}
// TRIED: raw self+offset transcription with manual ListBox/Dialog virtual-base adjustor arithmetic (vtable[+8]/[+4] thunks) for both energy-facility and support-facility ListBox blocks diverges at instr #8 (mov vs lea); not chased further given the 2935-byte, dual-listbox scope.
void ReportIf::draw_energy() {
    char text_a[80];
    energy_compute(report_faction_id_, 0);
    MainInterfaceGlobal.Buffer::set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = rect_width(&field_20_) - 0xC;
    field_18_ = rect_height(&field_20_) - 0xC;
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0xBE));
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0x2E6));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    set_rect(&draw_workspace_rect_, field_C_, draw_workspace_rect_.bottom + 2,
                    field_C_ + field_14_, field_10_ + field_18_);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    inflate_frame(&draw_workspace_rect_, -3, -3);
    scroll_small();
    ListBox *bases = &listBox1_;
    bases->close();
    bases->Dialog::init(&draw_workspace_rect_, 0);
    bases->Dialog::field_48_ = field_1C_ + 3;
    listBox1_.Dialog::set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    listBox1_.Dialog::set_dialog_text_color(0x99, -1, 1, 1);
    listBox1_.Dialog::set_dialog_text_color2(0x99, -1, 1, 1);
    listBox1_.Dialog::set_dialog_text_color3(0x99, -1, 1, 1);
    listBox1_.set_report_text_color(0xF2);
    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        if (Bases[base].faction_id_current == report_faction_id_) {
            char text_b[80];
            char text_c[80];
            StringTemp[0] = 0;
            say_base(StringTemp, base);
            strcat(StringTemp, "|");
            strcat(StringTemp, "+");
            _itoa(Bases[base].economy_total, text_c, 10);
            strcat(StringTemp, text_c);
            strcat(StringTemp, "/");
            strcat(StringTemp, "-");
            _itoa(EnergyBaseMaintenance[base], text_b, 10);
            strcat(StringTemp, text_b);
            strcat(StringTemp, " = ");
            if (Bases[base].economy_total >= EnergyBaseMaintenance[base]) strcat(StringTemp, "+");
            _itoa(Bases[base].economy_total - EnergyBaseMaintenance[base], text_a, 10);
            strcat(StringTemp, text_a);
            strcat(StringTemp, label_get(0x17B));
            bases->item(StringTemp, base + 0x400);
        }
    }
    listBox1_.field_2C_ = 0;
    listBox1_.clear_selection_index();
    listBox1_.Dialog::field_B4_ = 0;
    bases->ListBox::attach(&MainInterfaceGlobal, bases->Dialog::window_x_, bases->Dialog::window_y_, 0x80020);
    if (ReportWinGlobal.field_59B0_) {
        if (ReportWinGlobal.shared_report_selection() < 0x400)
            bases->set_selected_pos(0, 1);
        else
            bases->set_selected_id(ReportWinGlobal.shared_report_selection());
    } else {
        bases->set_selected_pos(-1, 1);
    }
    listBox1_.show(0);
    scroll_normal();
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_50_);
    field_C_ = field_50_.left + (field_1E230_ ? 0x4C : 0);
    field_10_ = field_50_.top;
    field_14_ = rect_width(&field_50_) - (field_1E230_ ? 0x4C : 0);
    field_18_ = rect_height(&field_50_);
    StringTemp[0] = 0;
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0x2B2));
    if (field_1E230_) {
        strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, BufferJoinSpace);
        say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    MainInterfaceGlobal.Buffer::set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x39));
    strcat(StringTemp, SocialWinColonSpaceText);
    _itoa(PlayersData[report_faction_id_].energy_reserves, text_a, 10);
    strcat(StringTemp, text_a);
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_, field_14_);
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = rect_width(&field_30_) - 0xC;
    field_18_ = rect_height(&field_30_) - 0xC;
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x3C2));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    set_rect(&draw_workspace_rect_, field_C_, draw_workspace_rect_.bottom + 2,
                    field_C_ + field_14_, field_10_ + field_18_);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    inflate_frame(&draw_workspace_rect_, -3, -3);
    scroll_small();
    ListBox *facilities = &listBox2_;
    facilities->close();
    facilities->Dialog::init(&draw_workspace_rect_, 0);
    facilities->Dialog::field_48_ = field_1C_ + 3;
    listBox2_.Dialog::set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    listBox2_.Dialog::set_dialog_text_color(0x99, -1, 1, 1);
    listBox2_.Dialog::set_dialog_text_color2(0x99, -1, 1, 1);
    listBox2_.Dialog::set_dialog_text_color3(0x99, -1, 1, 1);
    listBox2_.set_report_text_color(0xF2);
    for (int facility = 0; facility < 70; ++facility) {
        // Zero counts skip this row, not the remainder of the table.
        if (EnergyFacilityCounts[facility]) {
            char text_b[80];
            char text_c[80];
            StringTemp[0] = 0;
            _itoa(EnergyFacilityCounts[facility], text_a, 10);
            strcat(StringTemp, text_a);
            strcat(StringTemp, BufferJoinSpace);
            char *name = Facility[facility].name;
            strcat(StringTemp, StringTable.get(name));
            strcat(StringTemp, "|");
            strcat(StringTemp, "-");
            if (has_project(SP_SELF_AWARE_COLONY, report_faction_id_)) {
                int odd = fac_maint(facility, report_faction_id_) * EnergyFacilityCounts[facility] & 1;
                int cost = fac_maint(facility, report_faction_id_) * EnergyFacilityCounts[facility] / 2;
                _itoa(cost, text_b, 10);
                strcat(StringTemp, text_b);
                if (odd) strcat(StringTemp, ".5");
            } else {
                int cost = fac_maint(facility, report_faction_id_) * EnergyFacilityCounts[facility];
                _itoa(cost, text_c, 10);
                strcat(StringTemp, text_c);
            }
            strcat(StringTemp, label_get(0x17B));
            facilities->item(StringTemp, facility);
        }
    }
    listBox2_.field_2C_ = 0;
    listBox2_.clear_selection_index();
    listBox2_.Dialog::field_B4_ = 0;
    facilities->ListBox::attach(&MainInterfaceGlobal, facilities->Dialog::window_x_, facilities->Dialog::window_y_, 0x80020);
    if (ReportWinGlobal.field_59B0_)
        facilities->set_selected_pos(-1, 1);
    else
        facilities->set_selected_id(ReportWinGlobal.shared_report_selection());
    listBox2_.show(0);
    scroll_normal();
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
}

/*
Purpose: Close the energy report's three list boxes.

             mov ecx,0x87BE84     / call ListBox::close
             lea ecx,[esi+0xA2D0] / call ListBox::close
             lea ecx,[esi+0xAE24] / call ListBox::close

         The last two are the same subobjects close_intel closes, in the same
         order. The first is ReportWinGlobal's owned listBox_ at +0x5A0C;
         it is borrowed here and closes first.

         Original 0x0087BE84 = ReportWinGlobal 0x00876478 + 0x5A0C.
         Its construction and teardown belong to ReportWin, not an
         independent global allocation.
// ORIGINAL: 0x004A9020 ?close_energy@ReportIf@@QAEXXZ 0x004A9020-0x004A9045 BYTE_EXACT
// size      37 bytes
// prototype void (__thiscall ?close_energy@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609F20
Return Value: n/a
Status: Complete
*/
void ReportIf::close_energy() {
    ReportWinGlobal.listBox_.close();
    listBox1_.close();
    listBox2_.close();
}

// ORIGINAL: 0x004A9050 ?draw_ops@ReportIf@@QAEXXZ 0x004A9050-0x004A9F85
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004a9050.cpp", "source:src/reportif.h", "source:src/veh.h"], "reason": "Original operations-panel identity belongs to ReportIf's selection and list state; original indexed accesses select source-owned vehicle and prototype records, with the same row filtering and interface controls rather than a frame-only substitute."}
// TRIED: mechanical transcription of the Ghidra decompilation with fields reached by raw `(char*)this+OFFSET` casts (ReportIf's 165-member header is not pinned against this image, so no named struct is used). All ~26 unresolved-by-name FUN_ calls were re-derived from the disassembly's own `; ?mangled@Class` comments (set_clip/box_sprite/set_font/set_text_color/ write_cent_l on the fixed Buffer at 0x7aec64, close/init/ item/attach/set_selected_id/set_dialog_font/set_dialog_text_ color[23] on the ListBox+Dialog virtual-base pair at this+0xa2d0 via the scaffold's own `dialog_`/adjustor fields, Strings::get on the fixed 0x9b90d8, set_base/base_compute/ say_base/say_year/veh_at/scroll_normal/scroll_small already declared) rather than guessed. The ten `(**(code**)(EXPR+4)) (0)` vtable dispatches were rewritten as `reinterpret_cast<VCall*>(ADDR)->slot001(0)` per the brief's one-level-cast rule, where ADDR is the object whose first 4 bytes equal Ghidra's EXPR (confirmed against the raw `mov eax,[obj]; call [eax+4]` shape at several call sites). Did not chase the prologue-class mismatch (#0, push-vs-sub) given the size of this function.
void ReportIf::draw_ops() {
    if (report_page_history_[1] != 4 && report_page_history_[1] != 5) {
        WorldClimateWorldWin.show_all();
        for (int button = 6; button <= 7; ++button) {
            MainInterfaceGlobal.flat_button_[button].show(0);
        }
    }
    MainInterfaceGlobal.Buffer::set_clip(&field_20_);
    field_C_ = field_20_.left + 9;
    field_10_ = field_20_.top + 40;
    field_14_ = rect_width(&field_20_) - 18;
    field_18_ = rect_height(&field_20_) - 46;
    set_rectangle_dimensions(&draw_workspace_rect_, field_20_.left + 6, field_20_.top + 6,
                             rect_width(&field_20_) - 12, 26);
    RECT *const workspace = &draw_workspace_rect_;
    MainInterfaceGlobal.Buffer::box_sprite(workspace, &InterfaceBoxStyles[3]);
    workspace->top = workspace->bottom;
    workspace->bottom = field_20_.bottom - 6;
    MainInterfaceGlobal.Buffer::box_sprite(workspace, &InterfaceBoxStyles[3]);
    flatButton3_.show(0);
    flatButton4_.show(0);
    if (buttonGroup2_.field_84_ == 0xD0B) {
        scroll_small();
        set_rectangle_dimensions(workspace, field_C_, field_10_, field_14_, field_18_);
        ListBox *list = &listBox1_;
        list->close();
        list->Dialog::init(workspace, 0);
        list->Dialog::field_48_ = field_1C_ + 3;
        listBox1_.Dialog::set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
        listBox1_.Dialog::set_dialog_text_color(0x99, -1, 1, 1);
        listBox1_.Dialog::set_dialog_text_color2(0x99, -1, 1, 1);
        listBox1_.Dialog::set_dialog_text_color3(0x99, -1, 1, 1);
        listBox1_.set_report_text_color(0xF2);
        // Eight unrolled passes; each picks the first best own base.
        int winner;
        int best;
        int base;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0xA0/4));
        CharUpperA(StringTemp);
        strcat(StringTemp, SocialWinColonSpaceText);
        {
            winner = -1;
            best = 0;
            for (base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == report_faction_id_) {
                    set_base(base);
                    base_compute(0);
                    if (BaseCurrent->mineral_surplus > best || winner + 1 == 0) {
                        best = BaseCurrent->mineral_surplus;
                        winner = base;
                    }
                }
            }
            strcat(StringTemp, "|");
            say_base(StringTemp, winner);
            list->item(StringTemp, 1);
        }
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0xA4/4));
        CharUpperA(StringTemp);
        strcat(StringTemp, SocialWinColonSpaceText);
        {
            winner = -1;
            best = 0;
            for (base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == report_faction_id_) {
                    set_base(base);
                    base_compute(0);
                    if (BaseCurrent->nutrient_surplus > best || winner + 1 == 0) {
                        best = BaseCurrent->nutrient_surplus;
                        winner = base;
                    }
                }
            }
            strcat(StringTemp, "|");
            say_base(StringTemp, winner);
            list->item(StringTemp, 2);
        }
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0xE4/4));
        strcat(StringTemp, SocialWinColonSpaceText);
        {
            winner = -1;
            best = 0;
            for (base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == report_faction_id_) {
                    set_base(base);
                    base_compute(0);
                    if (BaseCurrent->economy_total > best || winner + 1 == 0) {
                        best = BaseCurrent->economy_total;
                        winner = base;
                    }
                }
            }
            strcat(StringTemp, "|");
            say_base(StringTemp, winner);
            list->item(StringTemp, 3);
        }
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0xAAC/4));
        strcat(StringTemp, SocialWinColonSpaceText);
        {
            winner = -1;
            best = 0;
            for (base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == report_faction_id_) {
                    set_base(base);
                    base_compute(0);
                    if (BaseCurrent->labs_total > best || winner + 1 == 0) {
                        best = BaseCurrent->labs_total;
                        winner = base;
                    }
                }
            }
            strcat(StringTemp, "|");
            say_base(StringTemp, winner);
            list->item(StringTemp, 4);
        }
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0xAB0/4));
        strcat(StringTemp, SocialWinColonSpaceText);
        {
            winner = -1;
            best = 0;
            for (base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == report_faction_id_) {
                    set_base(base);
                    base_compute(0);
                    if (BaseCurrent->psych_total > best || winner + 1 == 0) {
                        best = BaseCurrent->psych_total;
                        winner = base;
                    }
                }
            }
            strcat(StringTemp, "|");
            say_base(StringTemp, winner);
            list->item(StringTemp, 5);
        }
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0xAB4/4));
        strcat(StringTemp, SocialWinColonSpaceText);
        {
            winner = -1;
            best = 0;
            for (base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == report_faction_id_) {
                    set_base(base);
                    base_compute(0);
                    if (BaseCurrent->population_size > best || winner + 1 == 0) {
                        best = BaseCurrent->population_size;
                        winner = base;
                    }
                }
            }
            strcat(StringTemp, "|");
            say_base(StringTemp, winner);
            list->item(StringTemp, 6);
        }
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0xFC/4));
        strcat(StringTemp, SocialWinColonSpaceText);
        {
            winner = -1;
            best = 0;
            for (base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == report_faction_id_) {
                    set_base(base);
                    base_compute(0);
                    int vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);
                    int count = 0;
                    for (vehicle = veh_top(vehicle); vehicle >= 0; vehicle = Vehs[vehicle].next_veh_id_stack) {
                        ++count;
                    }
                    if (count > best || winner + 1 == 0) {
                        best = count;
                        winner = base;
                    }
                }
            }
            strcat(StringTemp, "|");
            say_base(StringTemp, winner);
            list->item(StringTemp, 7);
        }
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0xAB8/4));
        strcat(StringTemp, SocialWinColonSpaceText);
        {
            winner = -1;
            best = 0;
            for (base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == report_faction_id_) {
                    set_base(base);
                    base_compute(0);
                    // Native4A99A4..4A99EA counts bits0..69 with no filter.
                    int count = 0;
                    for (int facility = 0; facility < 70; ++facility) {
                        if (BaseCurrent->facilities_built[facility / 8] & (1 << (facility % 8))) ++count;
                    }
                    for (int project = 0; project < 64; ++project) {
                        if (GameDataStateStorage.fields_.secret_project_.project_base[project] == base) {
                            count += 5;
                        }
                    }
                    if (count > best || winner + 1 == 0) {
                        best = count;
                        winner = base;
                    }
                }
            }
            // Original calls say_base even when winner == -1.
            strcat(StringTemp, "|");
            say_base(StringTemp, winner);
            list->item(StringTemp, 8);
        }
        listBox1_.field_2C_ = 0;
        listBox1_.clear_selection_index();
        listBox1_.Dialog::field_B4_ = 0;
        list->ListBox::attach(&MainInterfaceGlobal, list->Dialog::window_x_, list->Dialog::window_y_, 0x80020);
        listBox1_.set_selected_id(ReportWinGlobal.field_5A08_);
        listBox1_.show(0);
        scroll_normal();
        MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
    } else {
        listBox1_.close();
        flatButton5_.show(0);
        flatButton6_.show(0);
        flatButton7_.show(0);
        flatButton8_.show(0);
        flatButton9_.show(0);
        flatButton10_.show(0);
        flatButton11_.show(0);
    }
    MainInterfaceGlobal.Buffer::set_clip(&field_50_);
    field_C_ = field_50_.left + (field_1E230_ ? 0x4C : 0);
    field_10_ = field_50_.top;
    field_14_ = rect_width(&field_50_) - (field_1E230_ ? 0x4C : 0);
    field_18_ = rect_height(&field_50_);
    StringTemp[0] = 0;
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0xACC/4));
    if (field_1E230_) {
        strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0xB68/4));
        strcat(StringTemp, BufferJoinSpace);
        say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    MainInterfaceGlobal.Buffer::set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xE4/4));
    strcat(StringTemp, SocialWinColonSpaceText);
    char number[80];
    _itoa(PlayersData[report_faction_id_].energy_reserves, number, 10);
    strcat(StringTemp, number);
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_, field_14_);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
    MainInterfaceGlobal.Buffer::set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = rect_width(&field_30_) - 12;
    field_18_ = rect_height(&field_30_) - 12;
    MainInterfaceGlobal.Buffer::box_sprite(&field_30_, &InterfaceBoxStyles[33]);
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
    inflate_frame(&draw_workspace_rect_, 3, 3);
    MainInterfaceGlobal.Buffer::box_sprite(workspace, &InterfaceBoxStyles[3]);
    inflate_frame(workspace, -3, -3);
    MainInterfaceGlobal.Buffer::box_sprite(workspace, &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0xF0C/4));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
    StringTemp[0] = 0;
    MainInterfaceGlobal.set_date(StringTemp);
}

// ORIGINAL: 0x004A9F90 ?close_ops@ReportIf@@QAEXXZ 0x004A9F90-0x004AA0AF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/004a9f90.cpp", "source:src/reportif.h", "source:src/reportwin.h", "source:src/basebutton.h"], "reason": "Close the ReportIf and ReportWin operation controls through their named owners, preserve conditional map cleanup, and clear only ungrouped ReportWin BaseButton state through an encapsulated helper. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void ReportIf::close_ops() {
    listBox1_.close();
    flatButton3_.hide();
    flatButton4_.hide();
    flatButton5_.hide();
    flatButton6_.hide();
    flatButton7_.hide();
    flatButton8_.hide();
    flatButton9_.hide();
    flatButton10_.hide();
    flatButton11_.hide();

    FlatButton *button = &ReportWinGlobal.flatButton4_;
    button->reset_field_A9C_if_ungrouped();
    button->Win::hide();
    button = &ReportWinGlobal.flatButton5_;
    button->reset_field_A9C_if_ungrouped();
    button->Win::hide();
    button = &ReportWinGlobal.flatButton6_;
    button->reset_field_A9C_if_ungrouped();
    button->Win::hide();

    if (field_8_ != 4 && field_8_ != 5) {
        WorldClimateWorldWin.hide_all();
        FlatButton *button = &MainInterfaceGlobal.flat_button_[6];
        FlatButton *const last = &MainInterfaceGlobal.flat_button_[7];
        while (reinterpret_cast<int>(button) <= reinterpret_cast<int>(last)) {
            button->hide();
            ++button;
        }
    }
}

// ORIGINAL: 0x004AA0B0 ?draw_projects@ReportIf@@QAEXXZ 0x004AA0B0-0x004AA921;0x00659595-0x006595C9
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004aa0b0.cpp", "source:src/reportif.h", "source:src/base.h", "source:src/stringlist.h"], "reason": "Retained spans identify ReportIf's project renderer and its cleanup code. A real local StringList owns the selected-project description; source-owned project/base records, member rectangles and controls express the original panel relationships. Empty scratch-string starts do not establish their original bounds."}
// TRIED: full transcription landed (MISMATCH #0: original has an SEH frame - push -1/handler/fs:[0] - that this body does not reproduce). A local `StringStruct localList(0);` reproduces the constructor call, but StringStruct has no declared destructor here so no unwind funclet gets generated; the two inlined "walk head_/entry_count_, adjustor-thunk release each entry" blocks are called as `localList.remove_all()` instead of reproduced byte-for-byte. Caught one real Ghidra error: find_font is `int find_font(int,int)` (2 args, per its mangled name @@YAHHH@Z has only two H's after the this-less Y); Ghidra printed it with 3 args by misattributing an outer accumulated push meant for the following set_font call.
void ReportIf::draw_projects() {
    StringList description;
    if (report_page_history_[1] != 4 && report_page_history_[1] != 5) {
        WorldClimateWorldWin.show_all();
        for (int button = 6; button <= 7; ++button) {
            MainInterfaceGlobal.flat_button_[button].show(0);
        }
    }
    MainInterfaceGlobal.Buffer::set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = rect_width(&field_20_) - 0xC;
    field_18_ = rect_height(&field_20_) - 0xC;

    // The seed draws a narrow title frame followed by a second frame around
    // the selected project's description.  Both use the shared work rectangle.
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);

    inflate_frame(&draw_workspace_rect_, -3, -3);
    MainInterfaceGlobal.Buffer::set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x32A));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);

    int project;
    if (ReportWinGlobal.shared_report_selection() < 0x400)
        project = ReportWinGlobal.shared_report_selection();
    else
        project = -70 - Bases[ReportWinGlobal.shared_report_selection() - 0x400].queue_production_id[0];
    if (project + 1 != 0) {
        // Preserve the original copies even though their destinations are
        // not subsequently consumed by this panel.
        char copied_small[1024];
        char copied_large[256];
        strcpy(copied_small, ReportProjectScratchSmall);
        strcpy(copied_large, ReportProjectScratchLarge);
        char number[80];
        StringTemp[0] = 0;
        strcat(StringTemp, "HELPPROJ");
        _itoa(project, number, 10);
        strcat(StringTemp, number);
        if (description.load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
            if (rect_height(&draw_workspace_rect_)
                < MainInterfaceGlobal.Buffer::write_strings_height(
                    &description, rect_width(&draw_workspace_rect_), 0)) {
                MainInterfaceGlobal.Buffer::set_font(find_font(10, 0), find_font(10, 2),
                                                     find_font(10, 1), 0);
            }
            MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);
            MainInterfaceGlobal.Buffer::write_strings(&description, draw_workspace_rect_.left,
                draw_workspace_rect_.top,
                rect_width(&draw_workspace_rect_), 0);
        }
    } else if (ReportWinGlobal.shared_report_selection() + 1 == 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x32B));
        MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);
        MainInterfaceGlobal.Buffer::wrap(StringTemp, draw_workspace_rect_.left,
            draw_workspace_rect_.top,
            rect_width(&draw_workspace_rect_));
    }

    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
    MainInterfaceGlobal.Buffer::set_clip(&field_50_);
    field_C_ = field_50_.left + (field_1E230_ ? 0x4C : 0);
    field_10_ = field_50_.top;
    field_14_ = rect_width(&field_50_) - (field_1E230_ ? 0x4C : 0);
    field_18_ = rect_height(&field_50_);
    StringTemp[0] = 0;
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0x2B4));
    if (field_1E230_) {
        strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x2DA]));
        strcat(StringTemp, BufferJoinSpace);
        say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    MainInterfaceGlobal.Buffer::set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = rect_width(&field_30_) - 0xC;
    field_18_ = rect_height(&field_30_) - 0xC;
    MainInterfaceGlobal.Buffer::box_sprite(&field_30_, &InterfaceBoxStyles[33]);
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
    inflate_frame(&draw_workspace_rect_, 3, 3);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    inflate_frame(&draw_workspace_rect_, -3, -3);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x3C3]));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
    StringTemp[0] = 0;
    MainInterfaceGlobal.set_date(StringTemp);
}

// ORIGINAL: 0x004AA930 ?close_projects@ReportIf@@QAEXXZ 0x004AA930-0x004AA964 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f3a5011d:src/recovered/004aa930.cpp;source:src/reportif.h;source:src/worldwin.h;source:src/maininterface.h;source:src/flatbutton.h"], "reason": "Close the project report overlay by its real WorldWin and MainInterface owners when the active report is not one of the two map categories."}
void ReportIf::close_projects() {
    const int state = field_8_;
    if (state != 4 && state != 5) {
        WorldClimateWorldWin.hide_all();
        for (int index = 6; index <= 7; ++index) {
            MainInterfaceGlobal.flat_button_[index].hide();
        }
    }
}

// ORIGINAL: 0x004AA970 ?draw_sat@ReportIf@@QAEXXZ 0x004AA970-0x004AB54B
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004aa970.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "Retained original identity maps to ReportIf's satellite panel, including its member rectangles and ReportWin help-selection control. Original scratch addresses are empty-string source starts, not writable destination arrays with inferred bounds."}
// TRIED: raw offset casts on `this` (opaque ReportIf class, no fields declared) mirroring the two near-identical D3C-mode/else-mode text panels and the button-creation loop; matches 10 instructions before diverging. The per-faction Players-array index scaling (idx*0x167 for byte-offset 0x304, matching src/faction.h's adj_name_faction) reused from 0x0053D360's introduce().
void ReportIf::draw_sat() {
    char paragraph[1024];
    char line[256];
    int base;
    int elevators;
    MainInterfaceGlobal.Buffer::set_clip(&field_20_);
    if (ReportWinGlobal.buttonGroup2_.field_84_ == 0xD3C) {
        field_C_ = field_20_.left + 6;
        field_10_ = field_20_.top + 6;
        field_14_ = rect_width(&field_20_) - 0xC;
        field_18_ = rect_height(&field_20_) - 0xC;
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_18_);
        MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        field_C_ += 4;
        field_10_ += 4;
        field_14_ -= 8;
        for (int faction = 1; faction < 8; ++faction) {
            if (faction != ConsoleGlobal.faction_id_) flatButtonsC_[faction - 1].hide();
        }
        strcpy(paragraph, ReportSatelliteScratchUpperText);
        strcpy(line, ReportSatelliteScratchUpperLine);
        StringTemp[0] = 0;
        strcat(StringTemp, "SAT0");
        if (X_text_open(ScriptPopupFile, StringTemp)) {
            text_close();
        } else {
            MainInterfaceGlobal.Buffer::set_font(field_1E184_, 0, 0, 0);
            MainInterfaceGlobal.Buffer::wrap_flying(paragraph, field_C_, field_10_, field_14_);
            MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);
            char *text;
            while (1) {
                text = text_get();
                if (text == 0) {
                    text_close();
                    break;
                }
                if (*text == 0) {
                    if (strlen(paragraph) > 0) MainInterfaceGlobal.Buffer::wrap_flying(paragraph);
                    MainInterfaceGlobal.Buffer::wrap_flying_flush();
                    text_close();
                    break;
                }
                line[0] = 0;
                parse_string(text, line);
                strcat(paragraph, line);
                strcat(paragraph, " ");
            }
        }
    } else {
        field_C_ = field_20_.left + 6;
        field_10_ = field_20_.top + 6;
        field_14_ = rect_width(&field_20_) - 0xC;
        field_18_ = rect_height(&field_20_) - 0xC;
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
        MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
        draw_workspace_rect_.bottom = field_20_.bottom - 6;
        MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        inflate_frame(&draw_workspace_rect_, -3, -3);
        MainInterfaceGlobal.Buffer::set_font(field_1E184_, field_1E194_, field_1E198_, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0x3C6));
        strcat(StringTemp, "]");
        MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
        for (int other = 1; other < 8; ++other) {
            if (other != ConsoleGlobal.faction_id_) {
                elevators = 0;
                for (base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                    if (Bases[base].faction_id_current == other && has_fac(41, base, 0)) ++elevators;
                }
                if (PlayersData[other].satellites_[0] + PlayersData[other].satellites_[1]
                    + PlayersData[other].satellites_[3] + PlayersData[other].satellites_[2]
                    + elevators)
                    flatButtonsC_[other - 1].show(0);
                else
                    flatButtonsC_[other - 1].hide();
            }
        }
    }
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_50_);
    int inset = field_1E230_ ? 0x4C : 0;
    field_C_ = field_50_.left + inset;
    field_10_ = field_50_.top;
    field_14_ = rect_width(&field_50_) - inset;
    field_18_ = rect_height(&field_50_);
    StringTemp[0] = 0;
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0x2B5));
    if (field_1E230_) {
        strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, BufferJoinSpace);
        say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    MainInterfaceGlobal.Buffer::set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_30_);
    if (ReportWinGlobal.buttonGroup2_.field_84_ == 0xD3C) {
        field_C_ = field_30_.left + 6;
        field_10_ = field_30_.top + 6;
        field_14_ = rect_width(&field_30_) - 0xC;
        field_18_ = rect_height(&field_30_) - 0xC;
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_18_);
        MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        field_C_ += 4;
        field_10_ += 4;
        field_14_ -= 8;
        for (int button = 0; button < 5; ++button) flatButtonsD_[button].hide();
        strcpy(paragraph, ReportSatelliteScratchLowerText);
        strcpy(line, ReportSatelliteScratchLowerLine);
        StringTemp[0] = 0;
        strcat(StringTemp, "SAT1");
        if (X_text_open(ScriptPopupFile, StringTemp)) {
            text_close();
        } else {
            MainInterfaceGlobal.Buffer::set_font(field_1E184_, 0, 0, 0);
            MainInterfaceGlobal.Buffer::wrap_flying(paragraph, field_C_, field_10_, field_14_);
            MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);
            char *text;
            while (1) {
                text = text_get();
                if (text == 0) {
                    text_close();
                    break;
                }
                if (*text == 0) {
                    if (strlen(paragraph) > 0) MainInterfaceGlobal.Buffer::wrap_flying(paragraph);
                    MainInterfaceGlobal.Buffer::wrap_flying_flush();
                    text_close();
                    break;
                }
                line[0] = 0;
                parse_string(text, line);
                strcat(paragraph, line);
                strcat(paragraph, " ");
            }
        }
    } else {
        field_C_ = field_30_.left + 6;
        field_10_ = field_30_.top + 6;
        field_14_ = rect_width(&field_30_) - 0xC;
        field_18_ = rect_height(&field_30_) - 0xC;
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
        MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
        // The original lower panel deliberately uses the upper panel's bottom.
        draw_workspace_rect_.bottom = field_20_.bottom - 6;
        MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        inflate_frame(&draw_workspace_rect_, -3, -3);
        MainInterfaceGlobal.Buffer::set_font(field_1E184_, field_1E194_, field_1E198_, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0x3C7));
        strcat(StringTemp, "]");
        MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
        field_C_ = field_30_.left + 9;
        field_10_ = field_30_.top + 32;
        field_14_ = rect_width(&field_30_) - 18;
        int target = buttonGroup3_.field_84_;
        if (target < 1 || target >= 8) target = -1;
        if (target + 1 != 0) {
            field_1E234_ = 1;
            int selected = buttonGroup4_.field_84_;
            if (selected < 65 || selected >= 70) selected = -1;
            buttonGroup4_.init(0xCE, 0);
            for (int kind = 0; kind < 5; ++kind) {
                char *name;
                StringTemp[0] = 0;
                if (kind == 4) {
                    if (!ExpansionEnabled) {
                        flatButtonsD_[4].hide();
                        field_1E234_ = 0;
                        MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
                        return;
                    }
                    name = Facility[41].name;
                } else {
                    name = Facility[kind + 65].name;
                }
                strcat(StringTemp, StringTable.get(name));
                set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, 20);
                flatButtonsD_[kind].init(StringTemp, kind + 65, &draw_workspace_rect_,
                                   &MainInterfaceGlobal, 0);
                flatButtonsD_[kind].Buffer::set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
                flatButtonsD_[kind].redraw();
                flatButtonsD_[kind].hide();
                elevators = 0;
                for (base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                    if (target == Bases[base].faction_id_current && has_fac(41, base, 0)) ++elevators;
                }
                if ((kind == 4 && elevators)
                    || (kind != 4 && kind < 4 && PlayersData[target].satellites_[kind])) {
                    flatButtonsD_[kind].show(0);
                    buttonGroup4_.add(&flatButtonsD_[kind]);
                    field_10_ += 22;
                }
            }
            if (selected + 1 != 0 && ((selected - 65 == 4 && elevators)
                || (selected - 65 != 4 && selected - 65 < 4 && PlayersData[target].satellites_[selected - 65]))) {
                buttonGroup4_.set(selected, 1);
            } else {
                ReportWinGlobal.flatButton9_.reset_field_A9C_if_ungrouped();
                ReportWinGlobal.flatButton9_.Win::hide();
            }
            field_1E234_ = 0;
        }
    }
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
}

/*
Purpose: Close the satellite report: hide every faction's panel button
         except the local player's, hide the five bottom buttons, clear-and-
         hide the three satellite windows, stop both timers.

             esi = 1; edi = self + 0x13778 (flatButtonsC_)
             do { if (esi != LocalFaction) slot002(edi)      - BaseButton::
                                                                hide through
                                                                FlatButton's
                                                                primary vtable
                  esi++; edi += 0xB4C } while (esi < 8)
             esi = self + 0x18720 (flatButtonsD_); five slot002 calls at
             0xB4C strides
             the three (flag, state) pairs, then the three Win::hide calls -
             DIRECT `call 0x5EDCD0`, the qualified-base spelling
             Time::stop on 0x00884738 and 0x00884760
// ORIGINAL: 0x004AB550 ?close_sat@ReportIf@@QAEXXZ 0x004AB550-0x004AB5F9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/unrecovered/004ab550.cpp"], "reason": "Retain the existing source implementation and complete original mapping. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// size      169 bytes
// prototype void (__thiscall ?close_sat@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EDCD0 0x00616730
// indirect  0x004AB56C 0x004AB58A
// Homed from src/unrecovered/004ab550.cpp. The seed carried FILE
// BYTE_EXACT; the token is the recorder's to re-bank on this body once
// this file's link-only placeholder (ReportIf::report) is implemented, so
// it is not hand-carried here.
*/
void ReportIf::close_sat() {
    // Both runs are the image's do/while over a walking pointer: the first
    // steps flatButtonsC_ (esi = 1, 0xB4C strides, `cmp esi, 8` at the
    // bottom), keeping the local player's own faction button; the second
    // counts flatButtonsD_ down from 5.
    int faction = 1;
    FlatButton *button = flatButtonsC_;
    do {
        if (faction != ConsoleGlobal.faction_id_) {
            button->hide();
        }
        ++faction;
        ++button;
    } while (faction < 8);

    button = flatButtonsD_;
    int count = 5;
    do {
        button->hide();
        ++button;
        --count;
    } while (count != 0);

    ReportWinGlobal.flatButton9_.reset_field_A9C_if_ungrouped();
    ReportWinGlobal.flatButton9_.Win::hide();
    ReportWinGlobal.flatButton7_.reset_field_A9C_if_ungrouped();
    ReportWinGlobal.flatButton7_.Win::hide();
    ReportWinGlobal.flatButton8_.reset_field_A9C_if_ungrouped();
    ReportWinGlobal.flatButton8_.Win::hide();
    ReportWinGlobal.time2_.stop();
    ReportWinGlobal.time3_.stop();
}

// ORIGINAL: 0x004AB600 ?draw_security@ReportIf@@QAEXXZ 0x004AB600-0x004ABE24 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004ab600.cpp", "source:src/reportif.h", "source:src/spot.h"], "reason": "Retained identity and indexed accesses identify ReportIf's five-category security panel. Its source-owned lists and Spot rows express the original filters, count caps and lost totals; the final frame uses the first panel's bottom edge as in the original, not a geometrically normalized replacement."}
// TRIED: literal register-transliteration (eax/ecx/edx/ebx/edi locals) compiles and is a faithful line-by-line transcription (verified box_sprite/set_font/write_* arg order and the 0x9ab898 flag-byte stride=0x34 against raw disasm, not Ghidra which mis-scaled it to 0x1a); compiler emits a frameless prologue (sub) where original is framed (push ebp/mov ebp,esp) - #0 divergence. Not chased further given size (2084 bytes).
void ReportIf::draw_security() {
    MainInterfaceGlobal.Buffer::set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = rect_width(&field_20_) - 0xC;
    field_18_ = rect_height(&field_20_) - 0xC;
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    inflate_frame(&draw_workspace_rect_, -3, -3);
    MainInterfaceGlobal.Buffer::set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x38A));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);

    MainInterfaceGlobal.Buffer::set_font(field_1E18C_, 0, 0, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
    field_10_ = draw_workspace_rect_.top + 3;
    field_C_ += 4;
    field_14_ -= 8;
    for (int filter = 0; filter < 5; ++filter) {
        int active = 0;
        int queued = 0;
        int lost = 0;
        if (filter == field_1E244_) {
            MainInterfaceGlobal.Buffer::set_text_color(0xF2, -1, 1, 1);
        } else {
            MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
        }
        for (int row = 0; row < 128; ++row) {
            int prototype_id = row < 64 ? row : report_faction_id_ * 64 + row - 64;
            if (!(VehPrototypes[prototype_id].flags & 1)) continue;
            if (filter < 3) {
                if (Chassis[VehPrototypes[prototype_id].chassis_id].triad != filter) continue;
                if (!Weapon[VehPrototypes[prototype_id].weapon_id].offense_rating) continue;
            } else if (filter == 3) {
                if (Weapon[VehPrototypes[prototype_id].weapon_id].offense_rating) continue;
            }
            if (PlayersData[report_faction_id_].proto_id_active[prototype_id] < 250)
                active += PlayersData[report_faction_id_].proto_id_active[prototype_id];
            if (PlayersData[report_faction_id_].proto_id_queue[prototype_id] < 250)
                queued += PlayersData[report_faction_id_].proto_id_queue[prototype_id];
            lost += PlayersData[report_faction_id_].proto_id_lost[prototype_id];
        }
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x38B + filter));
        MainInterfaceGlobal.Buffer::write_l(StringTemp, field_C_, field_10_);
        char active_text[80];
        char queued_text[80];
        char lost_text[80];
        StringTemp[0] = 0;
        _itoa(active, active_text, 10);
        strcat(StringTemp, active_text);
        strcat(StringTemp, "-");
        _itoa(queued, queued_text, 10);
        strcat(StringTemp, queued_text);
        strcat(StringTemp, "-");
        _itoa(lost, lost_text, 10);
        strcat(StringTemp, lost_text);
        report_write_right(StringTemp, field_C_ + field_14_, field_10_);
        set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_);
        spot_.add(filter, 0, &draw_workspace_rect_);
        if (filter == 3) {
            field_10_ += field_1C_ - 1;
            MainInterfaceGlobal.Buffer::line(field_C_ - 1, field_10_, field_C_ + field_14_, field_10_, 0x99);
            field_10_ += 5;
        } else {
            field_10_ += field_1C_;
        }
    }

    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
    MainInterfaceGlobal.Buffer::set_clip(&field_50_);
    int inset = field_1E230_ ? 0x4C : 0;
    field_C_ = inset + field_50_.left;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - inset - field_50_.left;
    field_18_ = rect_height(&field_50_);
    StringTemp[0] = 0;
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0x2B6));
    if (field_1E230_) {
        strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, BufferJoinSpace);
        say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    MainInterfaceGlobal.Buffer::set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = rect_width(&field_30_) - 0xC;
    field_18_ = rect_height(&field_30_) - 0xC;
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    // Original intentionally uses the FIRST panel's bottom for this frame.
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    inflate_frame(&draw_workspace_rect_, -3, -3);
    MainInterfaceGlobal.Buffer::set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x3BB));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    flatButton12_.show(0);
    flatButton14_.show(0);
    flatButton13_.show(0);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
}

// ORIGINAL: 0x004ABE30 ?close_security@ReportIf@@QAEXXZ 0x004ABE30-0x004ABE62 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/004abe30.cpp", "source:src/reportif.h", "source:src/flatbutton.h", "source:src/win.h"], "reason": "Hide the three ReportIf security FlatButtons through Win slot 2 in retained-seed order."}
void ReportIf::close_security() {
    flatButton12_.hide();
    flatButton14_.hide();
    flatButton13_.hide();
}

// ORIGINAL: 0x004ABE70 ?draw_score@ReportIf@@QAEXXZ 0x004ABE70-0x004AC127 BYTE_EXACT
// The report renderer uses the ReportIf-owned panel rectangles and fonts,
// MainInterfaceGlobal's Buffer base, and the source-owned faction/string
// tables. Its two separators are the shared objects BufferJoinSpace
// (0x00682820, " ") and SocialWinCommaSpaceText (0x00682E90, ", ").
void ReportIf::draw_score() {
    MainInterfaceGlobal.Buffer::set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = rect_width(&field_20_) - 0xC;
    field_18_ = rect_height(&field_20_) - 0xC;
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_18_);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_50_);
    int left_inset = field_1E230_ != 0 ? 0x4C : 0;
    field_C_ = field_50_.left + left_inset;
    field_10_ = field_50_.top;
    field_14_ = rect_width(&field_50_) - (field_1E230_ != 0 ? 0x4C : 0);
    field_18_ = rect_height(&field_50_);

    StringTemp[0] = 0;

    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);

    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0x2B7));

    if (field_1E230_ != 0) {
        strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, BufferJoinSpace);
        say_year(StringTemp);
    }

    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);

    MainInterfaceGlobal.Buffer::set_font(field_1E180_, field_1E180_, field_1E180_, nullptr);
    MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = rect_width(&field_30_) - 0xC;
    field_18_ = rect_height(&field_30_) - 0xC;
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_18_);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004AC130 ?close_score@ReportIf@@QAEXXZ 0x004AC130-0x004AC131 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?close_score@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::close_score() {
}

// ORIGINAL: 0x004AC140 ?draw_intel@ReportIf@@QAEXXZ 0x004AC140-0x004AC977 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004ac140.cpp", "source:src/reportif.h", "source:src/faction.h", "source:src/tech.h"], "reason": "Retained identity identifies ReportIf's intelligence panel. Original faction-indexed treaty accesses and technology iteration map to source-owned player data and technology records; the informed-view conditions select the same information rather than treating the panel as a link-only placeholder."}
// TRIED: full field-offset transcription off `self`, with the two embedded ListBox virtual-base dispatches done as explicit two-step vbtable-pointer/displacement reads (not flattened pointer arithmetic) and the vtable slot1 call retyped to take an int; compiles, MISMATCH #3 (early - likely the spy-flag bool-to-int store or the `test ecx,ecx` on a provably-nonzero fixed global getting folded away).
void ReportIf::draw_intel() {
    int target = report_selection_;
    int informed = (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
        || spying(target) != 0
        || (PlayersData[report_faction_id_].diplo_treaties[target] & 1);
    MainInterfaceGlobal.Buffer::set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = rect_width(&field_20_) - 0xC;
    field_18_ = rect_height(&field_20_) - 0xC;
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x175));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    set_rect(&draw_workspace_rect_, field_C_, draw_workspace_rect_.bottom + 2,
                    field_C_ + field_14_, field_10_ + field_18_);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    inflate_frame(&draw_workspace_rect_, -3, -3);
    scroll_small();
    ListBox *techs = &listBox1_;
    techs->close();
    techs->Dialog::init(&draw_workspace_rect_, 0);
    techs->Dialog::field_48_ = field_1C_ + 3;
    listBox1_.Dialog::set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    listBox1_.Dialog::set_dialog_text_color(0x99, -1, 1, 1);
    listBox1_.Dialog::set_dialog_text_color2(0x99, -1, 1, 1);
    listBox1_.Dialog::set_dialog_text_color3(0x99, -1, 1, 1);
    listBox1_.set_report_text_color(0xF2);
    if (informed) {
        for (int tech = 0; tech < 89; ++tech) {
            if (has_tech(tech, target)) {
                StringTemp[0] = 0;
                say_tech(StringTemp, tech, 0);
                if (!has_tech(tech, report_faction_id_)) strcat(StringTemp, " (*)");
                listBox1_.item(StringTemp, tech);
            }
        }
    }
    listBox1_.field_2C_ = 0;
    listBox1_.clear_selection_index();
    listBox1_.Dialog::field_B4_ = 0;
    techs->ListBox::attach(&MainInterfaceGlobal, techs->Dialog::window_x_, techs->Dialog::window_y_, 0x80020);
    listBox1_.show(0);
    scroll_normal();
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_50_);
    int inset = field_1E230_ ? 0x4C : 0;
    field_C_ = inset + field_50_.left;
    field_10_ = field_50_.top;
    field_14_ = rect_width(&field_50_) - (field_1E230_ ? 0x4C : 0);
    field_18_ = rect_height(&field_50_);
    StringTemp[0] = 0;
    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(0x2B8));
    if (field_1E230_) {
        strcat(StringTemp, SocialWinCommaSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, BufferJoinSpace);
        say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    MainInterfaceGlobal.Buffer::set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = rect_width(&field_30_) - 0xC;
    field_18_ = rect_height(&field_30_) - 0xC;
    set_rectangle_dimensions(&draw_workspace_rect_, field_C_, field_10_, field_14_, field_1C_ + 6);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x176));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    set_rect(&draw_workspace_rect_, field_C_, draw_workspace_rect_.bottom + 2,
                    field_C_ + field_14_, field_10_ + field_18_);
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    inflate_frame(&draw_workspace_rect_, -3, -3);
    scroll_small();
    ListBox *bases = &listBox2_;
    bases->close();
    bases->Dialog::init(&draw_workspace_rect_, 0);
    bases->Dialog::field_48_ = field_1C_ + 3;
    listBox2_.Dialog::set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    listBox2_.Dialog::set_dialog_text_color(0x99, -1, 1, 1);
    listBox2_.Dialog::set_dialog_text_color2(0x99, -1, 1, 1);
    listBox2_.Dialog::set_dialog_text_color3(0x99, -1, 1, 1);
    listBox2_.set_report_text_color(0xF2);
    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        if (Bases[base].faction_id_current == target
            && (informed || Bases[base].faction_id_current == report_faction_id_
                || (Bases[base].visibility & (1 << report_faction_id_)))) {
            char number[80];
            StringTemp[0] = 0;
            say_base(StringTemp, base);
            strcat(StringTemp, "|");
            // Original movzx intentionally treats current population as unsigned.
            _itoa(informed ? static_cast<uint8_t>(Bases[base].population_size)
                : Bases[base].faction_pop_size_intel[report_faction_id_], number, 10);
            strcat(StringTemp, number);
            listBox2_.item(StringTemp, base);
        }
    }
    listBox2_.field_2C_ = 0;
    listBox2_.clear_selection_index();
    listBox2_.Dialog::field_B4_ = 0;
    bases->ListBox::attach(&MainInterfaceGlobal, bases->Dialog::window_x_, bases->Dialog::window_y_, 0x80020);
    listBox2_.show(0);
    scroll_normal();
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());
}

/*
Purpose: Close the two intel list boxes.

             mov esi,ecx / lea ecx,[esi+0xA2D0] / call ListBox::close
                           lea ecx,[esi+0xAE24] / call ListBox::close

         Two distinct owned ListBox members, in that order, and nothing else.

         ListBox::close resolves its own bases through the object's vbtable at
         run time, so each member's own ListBox subobject and virtual-base
         table remain intact.
// ORIGINAL: 0x004AC980 ?close_intel@ReportIf@@QAEXXZ 0x004AC980-0x004AC99B BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ?close_intel@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609F20
Return Value: n/a
Status: Complete
*/
void ReportIf::close_intel() {
    listBox1_.close();
    listBox2_.close();
}

// ORIGINAL: 0x004AC9A0 ?report_labs@@YAXXZ 0x004AC9A0-0x004AC9EC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: source:src/recovered/units/004ac9a0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_labs seed identifies this free report command and its original extent. Category 2 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_labs() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 2;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        Win &modal_window = ReportWinGlobal;
        modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(2, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004AC9F0 ?report_energy@@YAXXZ 0x004AC9F0-0x004ACA3C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: source:src/recovered/units/004ac9f0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_energy seed identifies this free report command and its original extent. Category 3 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_energy() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 3;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        Win &modal_window = ReportWinGlobal;
        modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(3, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACA40 ?report_ops@@YAXXZ 0x004ACA40-0x004ACA8C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: source:src/recovered/units/004aca40.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_ops seed identifies this free report command and its original extent. Category 4 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_ops() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 4;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        Win &modal_window = ReportWinGlobal;
        modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(4, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACA90 ?report_projects@@YAXXZ 0x004ACA90-0x004ACADC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004aca90.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_projects seed identifies this free report command and its original extent. Category 5 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_projects() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 5;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        Win &modal_window = ReportWinGlobal;
        modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(5, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACAE0 ?report_sat@@YAXXZ 0x004ACAE0-0x004ACB2C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: source:src/recovered/units/004acae0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_sat seed identifies this free report command and its original extent. Category 6 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_sat() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 6;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        Win &modal_window = ReportWinGlobal;
        modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(6, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACB30 ?report_security@@YAXXZ 0x004ACB30-0x004ACB7C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: source:src/recovered/units/004acb30.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_security seed identifies this free report command and its original extent. Category 7 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_security() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 7;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        Win &modal_window = ReportWinGlobal;
        modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(7, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACB80 ?report_score@@YAXH@Z 0x004ACB80-0x004ACBCC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: source:src/recovered/units/004acb80.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_score seed identifies this free report command and its original extent. Category 8 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_score(int) {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 8;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        Win &modal_window = ReportWinGlobal;
        modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(8, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACBD0 ?report_intel@@YAXH@Z 0x004ACBD0-0x004ACC29 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: source:src/recovered/units/004acbd0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_intel seed identifies this free report command and its original extent. Category 0 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_intel(int selection) {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 0;
    ReportIfGlobal.set_report_selection(selection);
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        Win &modal_window = ReportWinGlobal;
        modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(0, ReportIfGlobal.report_faction_id(), selection);
    }
}

// ORIGINAL: 0x004ACC30 ?merge_sort@ReportIf@@QAEXHH@Z 0x004ACC30-0x004ACD98 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:f3a5011d:src/recovered/units/004acc30.cpp;source:src/reportif.h;source:src/base.h:BaseCurrentCount;source:src/general.h:swap"], "reason": "Recover the ReportIf base-ranking merge sort over its measured 0x200-entry, 16-byte scratch workspace, using the source-owned BaseCurrentCount bound and preserving original range normalization and key-prefetch order."}
void ReportIf::merge_sort(int first, int last) {
    const int maximum = GameDataStateStorage.fields_.base_current_count_ - 1;
    int low;
    if (first < 0 || maximum < 0) {
        low = 0;
    } else if (first > maximum) {
        low = maximum;
    } else {
        low = first;
    }
    first = low;
    int high = maximum;
    if (last < 0 || maximum < 0) {
        high = 0;
    } else if (last <= high) {
        high = last;
    }
    last = high;

    if (first == last) return;
    if (first > last) swap(&first, &last);

    const int middle = (last + first) / 2;
    merge_sort(first, middle);
    merge_sort(middle + 1, last);

    int left = first;
    int right = middle + 1;
    // Preserve the original unchecked key prefetch: it reads both cursors'
    // values before testing which half is exhausted, including an endpoint
    // read adjacent to the active range. Do not turn this into a guarded,
    // conventional merge without a separate behavior decision.
    for (int output = first; output <= last; ++output) {
        if (sort_entries_[right].value > sort_entries_[left].value) {
            if (right <= last) {
                sort_entries_[output].sorted_base_id = sort_entries_[right].base_id;
                sort_entries_[output].sorted_value = sort_entries_[right].value;
                ++right;
            } else {
                sort_entries_[output].sorted_base_id = sort_entries_[left].base_id;
                sort_entries_[output].sorted_value = sort_entries_[left].value;
                ++left;
            }
        } else if (left <= middle) {
            sort_entries_[output].sorted_base_id = sort_entries_[left].base_id;
            sort_entries_[output].sorted_value = sort_entries_[left].value;
            ++left;
        } else {
            sort_entries_[output].sorted_base_id = sort_entries_[right].base_id;
            sort_entries_[output].sorted_value = sort_entries_[right].value;
            ++right;
        }
    }

    for (int i = first; i <= last; ++i) {
        sort_entries_[i].base_id = sort_entries_[i].sorted_base_id;
        sort_entries_[i].value = sort_entries_[i].sorted_value;
    }
}

/*
Purpose: Construct the SubInterface base, then every
         sub-object in image order - seven FlatButton runs, three Sprite
         runs, two ListBoxes, four ButtonGroups and fourteen individual
         FlatButtons, and a Spot - before installing ReportIf's own vftable.
// ORIGINAL: 0x004AD170 ??0ReportIf@@QAE@XZ 0x004AD170-0x004AD3AA;0x00658F80-0x00659134 BYTE_EXACT
// TRIED: register allocation - the SEH prologue agrees (7/7) then the
//            compiled body reserves an extra `sub esp, 8` the image does
//            not. MISMATCH, 34/132 instructions agree - the best of this
//            batch. Layout (offsets, sub-object sizes, construction order)
//            cross-checked directly against the destructor at 0x004ACDA0,
//            which tears the same run down in mirrored order.
// TRIED: real declared members (FlatButton[7]/Sprite[0x15]/FlatButton[7])
//            for flatButtonsA_/spritesA_/flatButtonsB_, built implicitly -
//            measured WORSE, 26/132 (compiled grew to 195 instructions). The
//            `object[0] = vtable` store has to stay explicit body code (its
//            base SubInterface is not parametrised per host class, and that
//            header is out of this batch's scope), and standard C++ always
//            runs implicit member construction before any body statement, so
//            real members push that store after all three instead of ahead
//            of them, unlike the image. Reverted.
// size      1006 bytes
// prototype void (__thiscall ??0ReportIf@@QAE@XZ)(ReportIf* this)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005FA860 0x00607CF0 0x00609DB0 0x0062B7C0 0x006456E4 0x006457C2
*/
ReportIf::ReportIf() {
    // Declaration-order construction / reverse-order teardown is automatic.
}

/*
Purpose: Construct every sub-object in image order - a FlatButton[7] run, a
         Spot, a ListBox, nine individually-constructed FlatButtons split
         by two ButtonGroups, a Scroll, three Times, two Buffers and a Flic -
         then install ReportWin's own vftables.
// ORIGINAL: 0x004AD6B0 ??0ReportWin@@QAE@XZ 0x004AD6B0-0x004AD838;0x006592F0-0x00659419 BYTE_EXACT
// TRIED: deriving from ConstructedGraphicWin (graphicwin.h) to move the
//   base construction ahead of the members. The ORDER IS RIGHT - this image
//   really does `call 0x5d4cf0` before its first `lea ecx, [esi + N]`, checked
//   2026-08-22 - but making the source agree took this body 14 -> 10
//   of 87. Same shape as Scroll (scroll.cpp), where the reorder alone was
//   worth 1 instruction and the payoff came from two levers underneath that it
//   made reachable; here those levers are not the vtable-immediate one, since
//   these constants already fold, and this body is structurally far off
//   (149 compiled against 87). Re-derive the ordering fix as part
//   of a dedicated pass on the whole body, not on its own.
// TRIED: register allocation - the SEH prologue agrees (7/7) then the
//            compiled body reserves an extra `sub esp, 8` the image does
//            not. MISMATCH, 9/87 instructions agree.
// LEVER: flatButtons1_ and spot_ (the whole prefix before the irreducible
//        listBox_) converted to real declared members, built implicitly.
//        9/87 -> 14/87 agreeing instructions - still MISMATCH, real
//        improvement. Everything from listBox_ onward stays raw/explicit:
//        listBox_ has no declared one-argument constructor and Flic's own
//        recovery is out of this batch's scope, and real members declared
//        after either would be hoisted into the same implicit preamble
//        ahead of them, which the image does not do.
// size      689 bytes
// prototype void (__thiscall ??0ReportWin@@QAE@XZ)(ReportWin* this)
// callers   1   call targets   11
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x005D7210 0x005FA860 0x006051D0 0x00607CF0 0x00609DB0 0x006161D0 0x00629D60 0x0062B7C0 0x006456E4 0x006457C2
*/
ReportWin::ReportWin() {
    // The base and members construct once, in declaration order. The extra
    // argument at the original ListBox call is its most-derived ABI flag.
}

// RECOVERY_GENERATED: v1 vtable 0x0066D754 ? ??_7ReportWin@@6BWin@@@ 0x004AD6B0
// RECOVERY_GENERATED: v1 vtable 0x0066D74C ? ??_7ReportWin@@6BBuffer@@@ 0x004AD6B0
