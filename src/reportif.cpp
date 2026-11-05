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
// ORIGINAL: 0x00496900 _$E21 0x00496900-0x00496916 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825e8 selects original initializer0x496900..0x496916, registering fini0x496920..0x49692a; complete independent decode establishes exclusive boundaries.", "Source-owned object?ReportIfGlobal@@3VReportIf@@A identifies original storage0x885f38. Startup arguments/calls: call 0x4ad170; push 0x496920. Cleanup receiver/calls: mov ecx, 0x885f38; jmp 0x4acda0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/reportif.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?ReportIfGlobal@@3VReportIf@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E14
// ORIGINAL: 0x00496920 _$E19 0x00496920-0x0049692A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825e8 selects original initializer0x496900..0x496916, registering fini0x496920..0x49692a; complete independent decode establishes exclusive boundaries.", "Source-owned object?ReportIfGlobal@@3VReportIf@@A identifies original storage0x885f38. Startup arguments/calls: call 0x4ad170; push 0x496920. Cleanup receiver/calls: mov ecx, 0x885f38; jmp 0x4acda0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/reportif.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?ReportIfGlobal@@3VReportIf@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E12
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
#include "text.h"

extern char *DatalinkHelpFile;
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
// RECOVERY_OBJECT: v1 0x008A6240 ?ReportInterfaceStatusReentry@@3HA
int ReportInterfaceStatusReentry = 0;

static void report_iface_button(FlatButton &button, RECT &rect, int id,
                                Win *parent, bool hide) {
    button.init(StringTemp, id, rect.left, rect.top,
        rect.right - rect.left, rect.bottom - rect.top, parent, 0);
    static_cast<Buffer *>(&button)->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
    button.redraw();
    if (hide) button.hide();
}



// Superseded seed deleted; seed paths below are historical Git provenance.


// Superseded seed deleted; seed paths below are historical Git provenance.




// Superseded seed deleted; seed paths below are historical Git provenance.








// ListBox's ordinary member teardown includes its complete-object virtual
// bases: GraphicWin at +0x48 and Dialog at +0xA60, matching the original
// adjusted destructor calls (609EC0, 608E10, 5D4DD0).

/*
Purpose: Tear down every sub-object the constructor built, in exactly the
         reverse order.
// ORIGINAL: 0x004ACDA0 ??1ReportIf@@QAE@XZ 0x004ACDA0-0x004AD16C;0x00658D50-0x00658F74 FILE
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





// The seams and definitions for the unrecovered bodies, at the end of the file
// so no recovered body above shifts.
//
// auto_inline(off) is load-bearing, not tidiness, and being defined after the
// caller is NOT enough on its own: VC6 at /Ob2 defers codegen to the end of
// the translation unit and folds a forwarder defined later back into a caller
// defined earlier. Measured - without the pragma cl reports C4711 at
// reportif.cpp(219), which is the release_iface_mode call inside ReportIf::done,
// and done's 22-byte BYTE_EXACT match is a `call rel32` there.



#pragma auto_inline(on)

// Research state87BE24/885F30 is ReportWinGlobal+59AC/+FAB8.
// The energy list87BE84 is ReportWinGlobal+5A0C. No duplicate storage.

// The process-wide ReportIf itself, 0x00885F38 - see reportif.h. The
// image's dynamic initializer at 0x00496900 constructs it through the real
// recovered constructor above, so the storage sits here.
// Original constructor 0x004AD170 initializes this singleton; its two list
// receivers at 0x00890208/0x00890D5C are owned +0xA2D0/+0xAE24 members.
// This start-only binding does not infer a complete original allocation.
// RECOVERY_OBJECT: v1 0x00885F38 ?ReportIfGlobal@@3VReportIf@@A
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
// ORIGINAL: 0x00496930 _$E21 0x00496930-0x00496946 FILE
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
            & (1u << (ConsoleGlobal.faction_id_ & 31))) != 0
        || NetDaemonNet.channels_[1].payload_.flags != 0
        || NetDaemonNet.primary_state_.leading_words_[2] != 0;
}

static bool reportwin_security_filter(int prototype_id, int filter) {
    if (filter >= 4) return true;
    const VehPrototype &prototype = VehPrototypes[prototype_id];
    if (filter < 3) return Chassis[prototype.chassis_id].triad == filter
        && Weapon[prototype.weapon_id].offense_rating != 0;
    return Weapon[prototype.weapon_id].offense_rating == 0;
}

static void reportwin_append_number(char *text, int value) {
    char number[80];
    _itoa(value, number, 10);
    strcat(text, number);
}

// These chart coordinates are stored in unsigned layout words, but the
// original renderer consumes them as signed 32-bit coordinates. Avoid an
// out-of-range unsigned-to-signed conversion while preserving that bit view.
static int reportwin_signed_coordinate(uint32_t bits) {
    if (bits <= 0x7FFFFFFFu) return static_cast<int>(bits);
    return -1 - static_cast<int>(~bits);
}

// The image's loop tests for more than four matches before each slot, but
// stores every match unconditionally. It therefore writes five slots; only
// the first four are consumed after the displayed count is clamped at
// 0x499AE2..0x499AF3.
static int reportwin_collect_lab_children(int technology, int *children) {
    int count = 0;
    // 0x4996CA checks count > 4 before processing a slot, while 0x4996E7
    // stores unconditionally and increments the count afterward.
    for (int slot = 0; slot < MaxTechnologyNum * 2 && count <= 4; ++slot) {
        const int candidate = slot / 2;
        const int prerequisite = (slot & 1)
            ? Technology[candidate].preq_tech_2
            : Technology[candidate].preq_tech_1;
        if (prerequisite == technology) {
            children[count] = candidate;
            ++count;
        }
    }
    return count;
}

// 0x004997A1..7CB, 0x004998D7..9995, 0x004999A0..A9A, and the
// repeated child-button setup 0x00499B80..C0F all use BaseButton::init with
// the same font, earned-tech emphasis and three border-style records.
static void reportwin_init_lab_button(Buffer &canvas, FlatButton &button,
                                     int technology,
                                     int button_id, int x, int y,
                                     int width, int height, Win *parent,
                                     int faction) {
    StringTemp[0] = 0;
    say_tech(StringTemp, technology, 0);
    button.init(StringTemp, button_id, x, y, width, height, parent, 0);
    button.set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
    button.redraw();
    if (has_tech(technology, faction)) {
        button.set_text_color(0xFF, -1, 1, 1);
    }
    button.set_border_styles(&InterfaceBoxStyles[27], &InterfaceBoxStyles[28],
                             &InterfaceBoxStyles[29]);
    button.redraw();
    RECT frame = {x - 3, y - 3, x + width + 3, y + height + 3};
    canvas.box_sprite(&frame, &InterfaceBoxStyles[3]);
}

// 0x499FE3..0x49A389 performs a measure pass, then a centered draw pass, for
// the selected technology's two status lines. Their string pieces are the
// native bracket literals, `say_tech`, the category label at StringTable
// offset 0x209+category, and the prerequisite-depth number.
static void reportwin_draw_lab_status_rows(Buffer &canvas, int technology,
                                           int faction, int x,
                                           int vertical_sum,
                                           uint32_t &frame_width,
                                           int &cursor_y,
                                           int &measured_height,
                                           bool draw_pass,
                                           bool child_panel,
                                           Font *first_row_font,
                                           Font *second_row_font) {
        StringTemp[0] = 0;
        canvas.set_font(first_row_font, first_row_font, first_row_font, 0);
        canvas.set_text_color(has_tech(technology, faction) ? 0xFF : 0xE1,
                              -1, 1, 1);
        strcat(StringTemp, "[");
        say_tech(StringTemp, technology, 0);
        strcat(StringTemp, "]");
        CharUpperA(StringTemp);
        if (draw_pass && child_panel) {
            const int center_y = (vertical_sum - measured_height) / 2;
            cursor_y = center_y;
            const int current_frame_width = static_cast<int>(frame_width);
            RECT outer = {x - 6, center_y - 9, x + current_frame_width,
                          measured_height + center_y + 6};
            canvas.box_sprite(&outer, &InterfaceBoxStyles[3]);
            RECT inner = {outer.left + 3, outer.top + 3,
                          outer.right - 3, outer.bottom - 3};
            canvas.box_sprite(&inner, &InterfaceBoxStyles[0]);
        }
        if (!draw_pass) {
            measured_height += canvas.wrap_height(StringTemp,
                static_cast<int>(frame_width) - 6);
        } else {
            cursor_y = canvas.wrap_cent(StringTemp, x, cursor_y,
                static_cast<int>(frame_width) - 6);
        }

        StringTemp[0] = 0;
        canvas.set_font(second_row_font, second_row_font, second_row_font, 0);
        strcat(StringTemp, "[");
        const int category = tech_category(technology);
        strcat(StringTemp, label_get(0x209 + category));
        strcat(StringTemp, " ");
        reportwin_append_number(StringTemp, tech_recurse(technology, 0));
        strcat(StringTemp, "]");
        canvas.set_text_color(mandate_color(category), -1, 1, 1);
        if (!draw_pass) {
            measured_height += canvas.wrap_height(StringTemp,
                static_cast<int>(frame_width) - 6);
        } else {
            cursor_y = canvas.wrap_cent(StringTemp, x, cursor_y,
                static_cast<int>(frame_width) - 6);
            if (child_panel) {
                Sprite &icon = g_TECH_ICON_SPRITES[technology];
                icon.draw_mono(&canvas, icon.transparent_index(),
                    x + (static_cast<int>(frame_width) - 0x34) / 2,
                    cursor_y + 3, mandate_color(category), 0x32,
                    icon.width());
            }
            cursor_y += 0x3A;
        }
        if (!draw_pass) measured_height += 0x3A;
}

static void reportwin_append_lab_entry(bool datalink, int &entry_count,
                                       const char *section, LPSTR name,
                                       bool name_in_string_table,
                                       int category, int index) {
    if (entry_count == 0) {
        strcat(StringTemp, section);
        strcat(StringTemp, ": ");
    } else {
        strcat(StringTemp, ", ");
    }
    LPSTR resolved_name = name_in_string_table ? StringTable.get(name) : name;
    if (datalink && category != 0) {
        char id[32];
        DatalinkMain.construct_id(id, category, index);
        help_create_link(StringTemp, resolved_name, id);
    } else {
        strcat(StringTemp, resolved_name);
    }
    ++entry_count;
}

static void reportwin_finish_lab_section(Buffer &canvas, bool draw_pass,
                                         int x, int &y, uint32_t *frame_width,
                                         int &measured_height) {
    if (StringTemp[0] == 0) return;
    if (draw_pass) {
        y = canvas.wrap(StringTemp, x, y,
            static_cast<int>(*frame_width) - 6);
    } else {
        measured_height += canvas.wrap_height(StringTemp,
            static_cast<int>(*frame_width) - 6);
    }
    StringTemp[0] = 0;
}

// 0x49A5F2..0x49A654 formats RulesChassis bytes +0x48/+0x4A.
static void reportwin_append_chassis_lab_detail(int speed, int range) {
    strcat(StringTemp, " ");
    strcat(StringTemp, "(");
    if (speed < 0) {
        strcat(StringTemp, label_get(0x310 / 4));
    } else if (range >= 3) {
        strcat(StringTemp, label_get(0x200 / 4));
    } else {
        reportwin_append_number(StringTemp, speed);
        strcat(StringTemp, label_get(0x200 / 4));
    }
    strcat(StringTemp, ")");
}

// 0x49A7C3..0x49A866 formats RulesWeapon signed +8 and mode byte +0xA.
static void reportwin_append_weapon_lab_detail(int offense, int mode) {
    strcat(StringTemp, " ");
    strcat(StringTemp, "(");
    if (offense < 0) {
        strcat(StringTemp, label_get(0x310 / 4));
    } else if (mode >= 3) {
        strcat(StringTemp, label_get(0x200 / 4));
    } else {
        reportwin_append_number(StringTemp, offense);
        strcat(StringTemp, label_get(0x200 / 4));
    }
    strcat(StringTemp, ")");
}

// 0x49A9D7..0x49AA66 formats RulesArmor signed defense byte +8.
static void reportwin_append_armor_lab_detail(int defense) {
    strcat(StringTemp, " ");
    strcat(StringTemp, "(");
    if (defense < 0) {
        strcat(StringTemp, label_get(0x310 / 4));
    } else {
        reportwin_append_number(StringTemp, defense);
    }
    strcat(StringTemp, ")");
}

// Native section scans and their inclusive/exclusive bounds are visible at
// 0x49A389..0x49B162. They are intentionally not replaced by Max* counts:
// several original ranges stop before the end of their source arrays.
static void reportwin_draw_lab_unlock_sections(Buffer &canvas, int technology,
                                               bool datalink, bool draw_pass,
                                               int x, int &y,
                                               uint32_t *frame_width,
                                               int &measured_height) {
    int entries = 0;
    StringTemp[0] = 0;
    for (int i_reactor = 0; i_reactor < 4; ++i_reactor) {
        if (Reactor[i_reactor].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x234 / 4),
                                   Reactor[i_reactor].name, true,
                                   5, i_reactor + 1);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_chassis = 0; i_chassis < 9; ++i_chassis) {
        if (Chassis[i_chassis].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x120 / 4),
                                   Chassis[i_chassis].offsv_1_name, true,
                                   4, i_chassis);
        if (draw_pass) {
            TechnologyHelpEntryKind = 0x62;
            TechnologyHelpEntryName = Chassis[i_chassis].offsv_1_name;
        }
        reportwin_append_chassis_lab_detail(static_cast<signed char>(Chassis[i_chassis].speed),
                                            Chassis[i_chassis].range);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_weapon = 0; i_weapon < 26; ++i_weapon) {
        if (Weapon[i_weapon].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x124 / 4),
                                   Weapon[i_weapon].name, true,
                                   6, i_weapon);
        if (draw_pass) {
            TechnologyHelpEntryKind = 0x63;
            TechnologyHelpEntryName = Weapon[i_weapon].name;
        }
        reportwin_append_weapon_lab_detail(Weapon[i_weapon].offense_rating,
                                           Weapon[i_weapon].mode);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_armor = 0; i_armor < 14; ++i_armor) {
        if (Armor[i_armor].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x128 / 4),
                                   Armor[i_armor].name, true,
                                   7, i_armor);
        if (draw_pass) {
            TechnologyHelpEntryKind = 0x64;
            TechnologyHelpEntryName = Armor[i_armor].name;
        }
        reportwin_append_armor_lab_detail(Armor[i_armor].defense_rating);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_ability = 0; i_ability < 29; ++i_ability) {
        if (Ability[i_ability].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x12C / 4),
                                   Ability[i_ability].name, true,
                                   8, i_ability);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    // 0x49AC42..0x49AD90 scans the 29 built-in vehicle prototypes after
    // abilities. The native loop initializes EBX to zero at 0x49AC42 and
    // increments it unconditionally at 0x49AD2B, so Datalink item IDs use
    // the prototype table index even when earlier records do not match.
    entries = 0;
    for (int i_prototype = 0; i_prototype < 29; ++i_prototype) {
        VehPrototype &prototype = VehPrototypes[i_prototype];
        if ((prototype.flags & 1) == 0
            || prototype.preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries,
                                   label_get(0x318 / 4),
                                   prototype.veh_name, false,
                                   3, i_prototype);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_facility = 1; i_facility < 134; ++i_facility) {
        if (Facility[i_facility].preq_tech != technology
            && Facility[i_facility].free_tech != technology) continue;
        const int category = i_facility >= 70 ? 0xB : 0xA;
        reportwin_append_lab_entry(datalink, entries, label_get(0x110 / 4),
                                   Facility[i_facility].name, true,
                                   category, i_facility);
        if (Facility[i_facility].free_tech == technology) {
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x4DC / 4));
            strcat(StringTemp, ")");
        }
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_citizen = 0; i_citizen < 7; ++i_citizen) {
        if (Citizen[i_citizen].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x4CC / 4),
                                   Citizen[i_citizen].singular_name, true,
                                   0, i_citizen);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);
}

// The three-per-category records are existing InterfaceBoxStyles entries:
// native jump tables at 0x499199 and 0x499224 select 38+3*category and
// 36+3*category respectively.
static BoxSpriteParams *reportwin_lab_category_style(int category,
                                                      int first_style) {
    if (category < 0 || category > 3) return 0;
    return &InterfaceBoxStyles[first_style + category * 3];
}



// The process-wide report window, 0x00876478 - see reportwin.h. The image's
// dynamic initializer at 0x00496930 constructs it through the real recovered
// constructor below. The fixed list/timer receivers used by ReportIf are
// members of this object, not separate global storage. Complete original
// allocation bounds remain unproved by this start-only binding.
// RECOVERY_OBJECT: v1 0x00876478 ?ReportWinGlobal@@3VReportWin@@A
ReportWin ReportWinGlobal;  // 0x00876478

// MainInterface owns the interface-slot array and active interface ID.



// Fixed890208/890D5C receivers are ReportIfGlobal.listBox1_/listBox2_.

// 0x005398E0 ?pop_wait@@YAHXZ - defined in faction.cpp (BYTE_EXACT); no
// header declares it, and the set_modal popups pass it as their wait
// callback (the designwin.cpp spelling).
int __cdecl pop_wait();



// ORIGINAL: 0x004AD3B0 ??1ReportWin@@QAE@XZ 0x004AD3B0-0x004AD6A9;0x00659140-0x006592EE FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:a538e0a3:src/recovered/units/004ad3b0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
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

static void report_topic_number(int n) {
    char number[80];
    _itoa(n, number, 10);
    strcat(StringTemp, number);
}
static void report_topic_line(Buffer &canvas, RECT &cursor, int x) {
    canvas.write_l(StringTemp, x, cursor.top, strlen(StringTemp));
}
static void report_intel_label(int label) {
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(label));
    strcat(StringTemp, ": ");
    strcat(StringTemp, " ");
    strcat(StringTemp, " ");
    strcat(StringTemp, "]");
}
static int report_social_model(const SocialCategory &selection, int category) {
    switch (category) {
    case 0: return selection.models_[SocialCategory::POLITICS];
    case 1: return selection.models_[SocialCategory::ECONOMICS];
    case 2: return selection.models_[SocialCategory::VALUES];
    default: return selection.models_[SocialCategory::FUTURE];
    }
}
static void report_append_leader(const Player &player) {
    GenderDefault = player.is_leader_female;
    PluralityDefault = 0;
    strcat(StringTemp, player.title_leader);
    strcat(StringTemp, " ");
    GenderDefault = player.is_leader_female;
    PluralityDefault = 0;
    strcat(StringTemp, player.name_leader);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2D0 / 4));
    strcat(StringTemp, " ");
    GenderDefault = player.noun_gender;
    PluralityDefault = player.is_noun_plural;
    strcat(StringTemp, player.noun_faction);
}



// Requires coordinator-reviewed PlayerData final-dword migration:
// int score_items_[9] at +0x20A8, replacing unk_110..unk_118 including the
// intervening char[4]. Original4A1318 accesses9signedints;4A16C9 bounds9.

static unsigned int report_sat_count(const PlayerData &data, int kind) {
    switch (kind) {
    case 0: return data.satellites_[0];
    case 1: return data.satellites_[1];
    case 2: return data.satellites_[2];
    default: return data.satellites_[3];
    }
}


static void report_sat_number(int value) {
    char number[80];
    _itoa(value, number, 10);
    strcat(StringTemp, number);
}















































































































































































// ORIGINAL: 0x00496960 ?init@ReportWin@@QAEXXZ 0x00496960-0x00496BA5
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:a538e0a3:src/recovered/units/00496960.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::init() {
    for (int i = 0; i < 16; ++i) selected_bases_[i] = 0;
    for (int j = 0; j < 4; ++j) selected_prototypes_[j] = 0;
    rect1_.right = 650;
    rect1_.bottom = 353;
    spot_.init(256);
    const bool large = ReportIfGlobal.field_1E230_ != 0;
    rect1_.bottom += large ? 200 : 20;
    rect1_.left = ((large ? WinScreenWidth : 800) - rect1_.right) / 2;
    rect1_.top = large ? WinScreenHeight - rect1_.bottom - 212
                      : 388 - rect1_.bottom;
    report_panel_rect_.left = 23;
    report_panel_rect_.top = 0;
    report_panel_rect_.right = rect1_.right + 23;
    report_panel_rect_.bottom = rect1_.bottom;
    energy_panel_rect_.left = 26;
    energy_panel_rect_.top = 3;
    energy_panel_rect_.right = rect1_.right - 6 + 26;
    energy_panel_rect_.bottom = rect1_.bottom - 56 + 3;
    GraphicWin::init(rect1_.left - 23, rect1_.top, rect1_.right + 78,
        rect1_.bottom, large ? ReportWindowCaptionLarge : ReportWindowCaptionSmall,
        0x1000000, static_cast<Win *>(&ConsoleGlobal), 0, 0);
    field_E268_ = font_line_advance(*ReportIfGlobal.field_1E184_);
}

/*
Purpose: Reset the seven report buttons, initialize the report text buffer,
         copy the current report bounds, draw both panel frames, and restore
         the inset frame rectangle after its temporary expansion.
// ORIGINAL: 0x00496BB0 ?reset@ReportWin@@QAEXXZ 0x00496BB0-0x00496CFC
// RECOVERY_BINDING: v1 {"evidence": ["Independent IDA/Ghidra function records agree on the original extent 0x00496BB0-0x00496CFC and classify it as game code.", "Retired recovered seed provenance: git:caf4963a:src/recovered/units/00496bb0.cpp records the button-close loop, Buffer font/color setup, rectangle copies and box_sprite calls in this order.", "docs/recovery/member-accesses.csv records reset's writes through the report geometry and embedded button/panel members; reportwin.h provides the corresponding source-owned member layout."], "reason": "Recover the report reset in its existing ReportWin owner using the declared button array, buffer storage, rectangles and shared interface box styles."}
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

    report_frame_rect_.left -= 3;
    report_frame_rect_.top -= 3;
    report_frame_rect_.right += 3;
    report_frame_rect_.bottom += 3;
    Buffer::box_sprite(&report_frame_rect_, &InterfaceBoxStyles[3]);
    report_frame_rect_.left += 3;
    report_frame_rect_.top += 3;
    report_frame_rect_.right -= 3;
    report_frame_rect_.bottom -= 3;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00496D00 0x00496DE4 0x00496E08 0x00496D35
/*
Purpose: Repaint the report window's selected page inside the topic panel.
// ORIGINAL: 0x00496D00 ?on_redraw@ReportWin@@QAEXXZ 0x00496D00-0x00496DE4 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original496D00..496DE4; retired seed git:117282ee:src/unrecovered/00496d00.cpp records the same identity and spans with its switch fold TRIED note kept below.", "The receiver is reportwin.h's own window: reset() is declared there (0x00496BB0), the +0x444 Buffer (set_clip is buffer.h's own) carries the topic rectangle at +0x5974, and the page selector is 0x00885F40.", "The eight page drawers are the seed's measured call targets 0x00498B30/0x0049BCD0/0x0049D540/0x0049EB10/0x0049F350/0x004A08E0/0x004A1140/0x004A2210, unrecovered members reached at their original addresses until declarations land."], "reason": "Coverage transcription of the report repaint with the page dispatch kept at the measured switch shape."}
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
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/00496e10.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
// symbol    ?on_left_click@ReportWin@@UAEXHH@Z
void ReportWin::on_left_click(int x, int y) {
    if (reportwin_input_blocked()) return;
    int selected, payload;
    const int hit = spot_.check(x, y, &selected, &payload);
    const int page = ReportIfGlobal.field_8_;
    if (hit == -1) {
        if (page != 4) return;
        field_5A08_ = 0;
        for (int i = 0; i < 512; ++i) {
            ReportSortEntry &entry = ReportIfGlobal.sort_entries_[i];
            entry.base_id = entry.sorted_base_id = i;
            entry.value = entry.sorted_value = 0;
        }
        redraw();
        ReportIfGlobal.draw_ops();
        MainInterfaceGlobal.soft_update();
        return;
    }
    int blink_base = 0;
    bool blink = false;
    if (page == 2) {
        if (selected != 0x29A) return;
        const int tech = PlayersData[ReportIfGlobal.report_faction_id_].tech_id_researching;
        if (tech > -1 && ((GameDataStateStorage.fields_.game_state_ & 0x80) != 0 || (GameDataStateStorage.fields_.game_rules_ & 0x200) == 0)) {
            field_FAB8_ = field_59AC_ = tech;
            reset(); draw_labs(); soft_update();
            ReportIfGlobal.reset(); ReportIfGlobal.draw_labs();
            MainInterfaceGlobal.soft_update();
        }
        if ((GameDataStateStorage.fields_.game_rules_ & 0x200) == 0) ConsoleGlobal.set_tech(ConsoleGlobal.faction_id_);
        else ConsoleGlobal.set_ai(ConsoleGlobal.faction_id_);
        reset(); draw_labs(); soft_update();
        ReportIfGlobal.reset(); ReportIfGlobal.draw_labs();
        MainInterfaceGlobal.soft_update();
        return;
    }
    if (page == 4) {
        int last = selected - 1024;
        if (selected < 1024) {
            if ((GetAsyncKeyState(VK_SHIFT) & 0xFF00) != 0) {
                if (selected > -1) {
                    TutWinBaseWindow.soft_zoom(selected);
                    const int specialist = selected < BaseCurrent->specialist_total
                        ? BaseCurrent->specialist_total - selected - 1 : -1;
                    TutWinBaseWindow.pop_click(specialist, 0, 0, 0);
                    base_compute(1);
                }
            } else {
                TutWinBaseWindow.soft_zoom(selected);
                log_say("Base report psych click", 0, 0, 0);
                if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) != 0) return;
                for (int square = 20; square > 0; --square) {
                    const uint32_t bit = 1u << (square & 31);
                    if (BaseCurrent->resource_sqr_radius & bit) {
                        BaseCurrent->resource_sqr_radius &= ~bit;
                        break;
                    }
                }
                ++BaseCurrent->specialist_total;
                base_compute(1);
                NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            }
        } else if (last > 1023) {
            int base_id = selected - 2048;
            if (base_id < 1024 ||
                (base_id = selected - 3072, Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_)) {
                if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_) return;
                log_say("Reports left_click garrison", base_id, payload, 0);
                TutWinBaseWindow.soft_zoom(base_id);
                TutWinBaseWindow.garrison_click(payload, x, y, 0);
                return;
            }
            log_say("Global EDBC base set", 0, 0, 0);
            if (NetDaemonNet.lock_base(base_id, 0, -1, -1) == 0) {
                TutWinBaseWindow.soft_zoom(base_id);
                const uint32_t governor = BaseCurrent->governor;
                if ((governor & 0x80000000u) == 0 || (governor & 0x8000000u) == 0) {
                    unsigned category;
                    if ((governor & 0x1000000u) != 0) category = 0;
                    else if ((governor & 0x2000000u) != 0) category = 1;
                    else category = ((~governor & 0x4000000u) | 0x8000000u) >> 26;
                    BaseCurrent->governor = (governor | 0x80000000u) & 0xF0FFFFFFu;
                    switch (category) {
                        case 0: BaseCurrent->governor |= 0x2000000u; break;
                        case 1: BaseCurrent->governor |= 0x4000000u; break;
                        case 2: BaseCurrent->governor |= 0x8000000u; break;
                        case 3: BaseCurrent->governor |= 0x1000000u; break;
                    }
                    TutWinBaseWindow.gov_activate();
                } else BaseCurrent->governor = governor & 0x7FFFFFFFu;
                draw_radius(Bases[base_id].x, Bases[base_id].y, 2, 2);
                NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            }
        } else {
            if (Bases[last].faction_id_current != ConsoleGlobal.faction_id_) return;
            const bool shift = (GetAsyncKeyState(VK_SHIFT) & 0xFF00) != 0;
            const bool control = (GetAsyncKeyState(VK_CONTROL) & 0xFF00) != 0;
            if (!shift && control) selected_bases_[last / 32] ^= 1u << (last & 31);
            else if (!control) {
                for (int word = 0; word < 16; ++word) selected_bases_[word] = 0;
                if (!shift || field_59AC_ == 0) selected_bases_[last / 32] |= 1u << (last & 31);
                else {
                    int first = static_cast<int>(field_59AC_) - 1024;
                    field_59AC_ = first;
                    if (last < first) { field_59AC_ = last; last = first; first = field_59AC_; }
                    for (int base_id = first; base_id <= last; ++base_id) {
                        if (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_)
                            selected_bases_[base_id / 32] |= 1u << (base_id & 31);
                    }
                }
            }
        }
        int count = 0;
        for (int id = 0; id < 512; ++id) if (selected_bases_[id / 32] & (1u << (id & 31))) ++count;
        field_5A04_ = count == 1;
        field_59AC_ = selected;
        reset(); draw_ops(); soft_update();
        ReportIfGlobal.reset(); ReportIfGlobal.draw_ops();
        MainInterfaceGlobal.soft_update();
        const int base_id = selected % 1024;
        if (field_5A04_ && (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
            || (Bases[base_id].visibility & (1u << (ConsoleGlobal.faction_id_ & 31))) != 0)) { blink_base = base_id; blink = true; }
    } else if (page == 5) {
        field_59AC_ = selected;
        reset(); draw_projects(); soft_update();
        ReportIfGlobal.reset(); ReportIfGlobal.draw_projects();
        MainInterfaceGlobal.soft_update();
        const int base_id = selected < 1024 ? base_project(selected) : selected - 1024;
        if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_
            && (Bases[base_id].visibility & (1u << (ConsoleGlobal.faction_id_ & 31))) == 0) return;
        blink_base = base_id;
        blink = true;
    } else if (page == 7) {
        if (ReportIfGlobal.report_faction_id_ != ConsoleGlobal.faction_id_) return;
        const bool shift = (GetAsyncKeyState(VK_SHIFT) & 0xFF00) != 0;
        const bool control = (GetAsyncKeyState(VK_CONTROL) & 0xFF00) != 0;
        const int filter = ReportIfGlobal.field_1E244_;
        const int clicked_triad = Chassis[VehPrototypes[selected].chassis_id].triad;
        if (!control) {
            for (int row = 0; row < 128; ++row) {
                const int id = row < 64 ? row : ReportIfGlobal.report_faction_id_ * 64 - 64 + row;
                const bool matches = filter < 3
                    ? Chassis[VehPrototypes[id].chassis_id].triad == clicked_triad
                        && Weapon[VehPrototypes[id].weapon_id].offense_rating != 0
                    : filter == 3 ? Weapon[VehPrototypes[id].weapon_id].offense_rating == 0 : filter == 4;
                if (matches) selected_prototypes_[row / 32] &= ~(1u << (row & 31));
            }
        }
        if (!shift) {
            const int row = selected % 64 + (selected > 63 ? 64 : 0);
            if (control) selected_prototypes_[row / 32] ^= 1u << (row & 31);
            else selected_prototypes_[row / 32] |= 1u << (row & 31);
        } else {
            int first_id = field_59AC_, last_id = selected;
            if (last_id < first_id) { field_59AC_ = last_id; last_id = first_id; first_id = field_59AC_; }
            int first = first_id % 64 + (first_id > 63 ? 64 : 0);
            const int last_row = last_id % 64 + (last_id > 63 ? 64 : 0);
            for (; first <= last_row; ++first) {
                const int id = first < 64 ? first : ReportIfGlobal.report_faction_id_ * 64 - 64 + first;
                const bool matches = filter < 3
                    ? Chassis[VehPrototypes[id].chassis_id].triad == Chassis[VehPrototypes[last_id].chassis_id].triad
                        && Weapon[VehPrototypes[id].weapon_id].offense_rating != 0
                    : filter == 3 ? Weapon[VehPrototypes[id].weapon_id].offense_rating == 0 : filter == 4;
                if (matches) selected_prototypes_[first / 32] |= 1u << (first & 31);
            }
        }
        field_59AC_ = selected;
        reset(); draw_security(); soft_update();
        ReportIfGlobal.reset(); ReportIfGlobal.draw_security();
        MainInterfaceGlobal.soft_update();
        return;
    }
    if (!blink) return;
    WorldClimateWorldWin.tile_to_pixel(Bases[blink_base].x, Bases[blink_base].y,
        &ReportIfGlobal.animation_x_, &ReportIfGlobal.animation_y_);
    ReportIfGlobal.animation_x_ -= 7;
    ReportIfGlobal.animation_y_ -= 5;
    ReportIfGlobal.animation_state_ = -1;
    ReportIfGlobal.bl_anim();
    time1_.init(bl_daemon,
        reinterpret_cast<int>(&ReportIfGlobal), 150, 5);
    time1_.start();
}

// ORIGINAL: 0x00497A60 ?on_left_double_click@ReportWin@@QAEXHH@Z 0x00497A60-0x00497AB2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/00497a60.cpp records the original 0x00497A60-0x00497AB2 method and BYTE_EXACT claim.", "src/reportwin.h places spot_ at 0x5994; src/tutwin.h binds TutWinBaseWindow at the seed's 0x006A7628 zoom receiver."], "reason": "Home ReportWin's existing double-click handler against its declared Spot member and the verified base-window object."}
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/units/00497a60.cpp
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
// ORIGINAL: 0x00497AC0 ?right_menu@ReportWin@@QAEXHHH@Z 0x00497AC0-0x00497E69;0x00659420-0x00659435
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:a538e0a3:src/recovered/units/00497ac0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::right_menu(int base_id, int x, int y) {
    PullDown menu;
    if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_
        || reportwin_input_blocked()) return;
    client_to_screen(&x, &y);
    menu.init(0);
    menu.add_item(1, label_get(0x894 / 4));
    menu.add_item(100, label_get(0x910 / 4));
    menu.add_separator(99);
    menu.add_item(2, label_get(0x898 / 4));
    menu.add_item(3, label_get(0x89C / 4));
    menu.add_separator(99);
    menu.add_item(4, label_get(0x8A0 / 4));
    menu.add_item(5, label_get(0x8A4 / 4));
    menu.add_item(6, label_get(0x8A8 / 4));
    menu.add_item(7, label_get(0x8AC / 4));
    menu.add_item(8, label_get(0xBFC / 4));
    menu.add_item(9, label_get(0xA54 / 4));
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
        case 8: BaseCurrent->state &= 0x7FFFFFFF; break;
        case 9: TutWinBaseWindow.governor(0); break;
        case 100:
            MapWinTable[0]->set_center(BaseCurrent->x, BaseCurrent->y, 1);
            ReportWinGlobal.release_modal();
            ReportIfGlobal.release_iface_mode();
            break;
    }
    redraw();
}

// ORIGINAL: 0x00497F00 ?on_right_click@ReportWin@@QAEXHH@Z 0x00497F00-0x004980EF
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:a538e0a3:src/recovered/units/00497f00.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
// symbol    ?on_right_click@ReportWin@@UAEXHH@Z
void ReportWin::on_right_click(int x, int y) {
    if (reportwin_input_blocked()) return;
    int spot_id, payload;
    if (spot_.check(x, y, &spot_id, &payload) < 0) return;
    switch (ReportIfGlobal.field_8_) {
    case 4:
        if (spot_id > 1023) {
            const int first_id = spot_id - 1024;
            if (first_id < 1024) {
                if (Bases[first_id].faction_id_current != ConsoleGlobal.faction_id_) return;
                right_menu(first_id, x, y);
                return;
            }
            const int base_id = spot_id - 2048;
            if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_) return;
            log_say("Reports right_click garrison", payload, base_id, 0);
            TutWinBaseWindow.soft_zoom(base_id);
            TutWinBaseWindow.garrison_click(payload, x, y, 1);
            return;
        }
        log_say("Base report psych right click", 0, 0, 0);
        TutWinBaseWindow.soft_zoom(spot_id);
        if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) != 0) return;
        if (BaseCurrent->specialist_total != 0) --BaseCurrent->specialist_total;
        base_compute(1);
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        break;
    case 6:
        if ((GameDataStateStorage.fields_.game_state_ & 0x40) != 0
            && (GameDataStateStorage.fields_.game_state_ & 0x80) != 0) {
            sat_edit(ReportIfGlobal.report_faction_id_, payload, spot_id);
            redraw();
            return;
        }
        break;
    default:
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
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/units/00498110.cpp
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
// ORIGINAL: 0x00498180 ?on_scrolled@ReportWin@@QAEXHH@Z 0x00498180-0x004981D8
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/00498180.cpp records the original 0x00498180-0x004981D8 handler and page dispatch.", "src/reportwin.h places field_C0FC_ at 0xC0FC; src/graphicwin.h identifies redraw as Win slot 62."], "reason": "Home the ReportWin scrollbar callback using its declared position member and inherited redraw virtual."}
// symbol    ?on_scrolled@ReportWin@@UAEXHH@Z
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/units/00498180.cpp
/*
Purpose: Track the report scrollbar's committed position and repaint the
         project page directly; recognized remaining pages use the window's
         redraw virtual, while unlisted pages do nothing.
*/
void ReportWin::on_scrolled(int, int pos) {
    field_C0FC_ = pos;
    switch (ReportIfGlobal.field_8_) {
    case 5:
        reset();
        draw_projects();
        soft_update();
        return;
    case 3:
    case 6:
    case 8:
        redraw();
        return;
    case 0:
    case 2:
    case 4:
    case 7:
        redraw();
        break;
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00498200 0x00498258 0x0049827C 0x00498219
// ORIGINAL: 0x00498200 ?on_scrolling@ReportWin@@QAEXHH@Z 0x00498200-0x00498258
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/00498200.cpp records the original 0x00498200-0x00498258 handler and page dispatch.", "src/reportwin.h places field_C0FC_ at 0xC0FC; src/graphicwin.h identifies redraw as Win slot 62."], "reason": "Home the ReportWin scroll-in-progress callback using the same source-owned position and redraw behavior."}
// symbol    ?on_scrolling@ReportWin@@UAEXHH@Z
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/units/00498200.cpp
void ReportWin::on_scrolling(int, int pos) {
    field_C0FC_ = pos;
    switch (ReportIfGlobal.field_8_) {
    case 5:
        reset();
        draw_projects();
        soft_update();
        return;
    case 3:
    case 6:
    case 8:
        redraw();
        return;
    case 0:
    case 2:
    case 4:
    case 7:
        redraw();
        break;
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
    if (code == 0x0D) {
        ReportWinGlobal.Win::release_modal();
        ReportIfGlobal.SubInterface::release_iface_mode();
        return 1;
    }
    if (code == 0x70) {
        on_button_clicked(1);
        return 1;
    }
    if (code > 0x70 && code < 0x78) {
        if (ReportIfGlobal.field_8_ == code - 0x6F) {
            ReportWinGlobal.Win::release_modal();
            ReportIfGlobal.SubInterface::release_iface_mode();
            return 1;
        }
        if (code == 0x71) goto select_page_2;
        if (code == 0x72) {
select_page_3:
            ReportIfGlobal.field_8_ = 3;
            if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5) {
                ReportIfGlobal.report(3,
                                      ReportIfGlobal.report_faction_id(), 0);
                return 1;
            }
            goto modal_report;
        }
        if (code == 0x73) {
select_page_4:
            ReportIfGlobal.field_8_ = 4;
            if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5) {
                ReportIfGlobal.report(4,
                                      ReportIfGlobal.report_faction_id(), 0);
                return 1;
            }
            goto modal_report;
        }
        if (code == 0x74) {
select_page_5:
            ReportIfGlobal.field_8_ = 5;
            if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5) {
                ReportIfGlobal.report(5,
                                      ReportIfGlobal.report_faction_id(), 0);
                return 1;
            }
            goto modal_report;
        }
        if (code == 0x75) {
select_page_6:
            ReportIfGlobal.field_8_ = 6;
            if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5) {
                ReportIfGlobal.report(6,
                                      ReportIfGlobal.report_faction_id(), 0);
                return 1;
            }
            goto modal_report;
        }
        if (code == 0x76) {
select_page_7:
            ReportIfGlobal.field_8_ = 7;
            if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5) {
                ReportIfGlobal.report(7,
                                      ReportIfGlobal.report_faction_id(), 0);
                return 1;
            }
            goto modal_report;
        }
        {
select_page_8:
            ReportIfGlobal.field_8_ = 8;
            if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5) {
                ReportIfGlobal.report(8,
                                      ReportIfGlobal.report_faction_id(), 0);
                return 1;
            }
            goto modal_report;
        }
select_page_2:
        ReportIfGlobal.field_8_ = 2;
        if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5) {
            ReportIfGlobal.report(2, ReportIfGlobal.report_faction_id(), 0);
            return 1;
        }
modal_report:
        Win &modal_window = ReportWinGlobal;
        modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);
        return 1;
    }

    if (code < 0x21 || code > 0x28) {
        if ((code < 0x30 || code > 0x5A) && code != 0x1B) {
            return 1;
        }
        ReportWinGlobal.Win::release_modal();
        ReportIfGlobal.SubInterface::release_iface_mode();
        return 1;
    }

    if (code == 0x25 || code == 0x27) {
        int moved = (code == 0x25) ? (ReportIfGlobal.field_8_ + 4)
                                   : (ReportIfGlobal.field_8_ + 6);
        int remainder = moved % 7;
        ReportIfGlobal.field_8_ = remainder + 2;

        switch (remainder) {
        case 0: goto select_page_2;
        case 1: goto select_page_3;
        case 2: goto select_page_4;
        case 3: goto select_page_5;
        case 4: goto select_page_6;
        case 5: goto select_page_7;
        case 6: goto select_page_8;
        default: return 1;
        }
    }

    switch (code) {
    case 0x21: {
        if (field_C0F8_ == 0) {
            return 1;
        }
        int range_maximum = static_cast<int>(field_C104_);
        int range_minimum = static_cast<int>(field_C100_);
        int target = static_cast<int>(field_C0FC_) -
                     static_cast<int>(field_C108_);
        if (range_minimum <= target && range_minimum <= range_maximum) {
            range_minimum = target;
            if (range_maximum < target) {
                range_minimum = range_maximum;
            }
        }
        field_C0FC_ = static_cast<uint32_t>(range_minimum);
        break;
    }
    case 0x22: {
        if (field_C0F8_ == 0) {
            return 1;
        }
        int range_maximum = static_cast<int>(field_C104_);
        int range_minimum = static_cast<int>(field_C100_);
        int target = static_cast<int>(field_C108_) +
                     static_cast<int>(field_C0FC_);
        if (range_minimum <= target && range_minimum <= range_maximum) {
            range_minimum = target;
            if (range_maximum < target) {
                range_minimum = range_maximum;
            }
        }
        field_C0FC_ = static_cast<uint32_t>(range_minimum);
        break;
    }
    case 0x23:
        if (field_C0F8_ == 0) {
            return 1;
        }
        field_C0FC_ = field_C104_;
        break;
    case 0x24:
        if (field_C0F8_ == 0) {
            return 1;
        }
        field_C0FC_ = field_C100_;
        break;
    case 0x26: {
        if (field_C0F8_ == 0) {
            return 1;
        }
        int range_maximum = static_cast<int>(field_C104_);
        int range_minimum = static_cast<int>(field_C100_);
        int target = static_cast<int>(field_C0FC_) - 1;
        if (range_minimum <= target && range_minimum <= range_maximum) {
            range_minimum = target;
            if (range_maximum < target) {
                range_minimum = range_maximum;
            }
        }
        field_C0FC_ = static_cast<uint32_t>(range_minimum);
        break;
    }
    case 0x28: {
        if (field_C0F8_ == 0) {
            return 1;
        }
        int range_maximum = static_cast<int>(field_C104_);
        int range_minimum = static_cast<int>(field_C100_);
        int target = static_cast<int>(field_C0FC_) + 1;
        if (range_minimum <= target && range_minimum <= range_maximum) {
            range_minimum = target;
            if (range_maximum < target) {
                range_minimum = range_maximum;
            }
        }
        field_C0FC_ = static_cast<uint32_t>(range_minimum);
        break;
    }
    default:
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
// ORIGINAL: 0x004986D0 ?on_button_clicked@ReportWin@@QAEXH@Z 0x004986D0-0x004989A8
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:a538e0a3:src/recovered/units/004986d0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
// symbol    ?on_button_clicked@ReportWin@@UAEXH@Z
void ReportWin::on_button_clicked(int id) {
    const bool datalink_mode = IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 8;
    const int page = ReportIfGlobal.field_8_;
    if (id < 0x378) {
        if (datalink_mode) return;
        if (id == 9) {
            ReportWinGlobal.release_modal();
            ReportIfGlobal.release_iface_mode();
            return;
        }
        if (id != 1) return;
        switch (page) {
            case 0: DatalinkMain.exec(15, ReportIfGlobal.report_selection_); return;
            case 2: DatalinkMain.exec(14, static_cast<int>(field_FAB8_) == -1 ? 62 : field_FAB8_); return;
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
        }
        return;
    }
    if (page == 2) {
        const int old_tech = field_FAB8_;
        switch (id) {
            case 0x379: field_FAB8_ = Technology[old_tech].preq_tech_1; break;
            case 0x37A: field_FAB8_ = Technology[old_tech].preq_tech_2; break;
            case 0x37B: case 0x37C: case 0x37D: case 0x37E: {
                int count = 0;
                for (int i = 0; i < 178; ++i) {
                    int preq = (i & 1) ? Technology[i >> 1].preq_tech_2
                                       : Technology[i >> 1].preq_tech_1;
                    if (preq == old_tech && ++count == id - 0x37A) {
                        field_FAB8_ = i >> 1;
                    }
                }
                break;
            }
        }
        ReportIfGlobal.listBox1_.set_selected_id(field_FAB8_);
        if (datalink_mode) DatalinkMain.set_index(field_FAB8_, 0);
        else redraw();
        return;
    }
    if (page == 6 && id == 0xD3E) {
        const int target = ReportIfGlobal.buttonGroup3_.field_84_;
        const int type = static_cast<int>(ReportIfGlobal.buttonGroup4_.field_84_) - 65;
        if (target > 0 && target < 8 && type >= 0 && type < 5) {
            sat_attack(ConsoleGlobal.faction_id_, target, type);
            return;
        }
        flatButton9_.hide();
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004989F0 0x00498B08 0x00498B2C 0x00498A3F
// ORIGINAL: 0x004989F0 ?on_group_clicked@ReportWin@@QAEXHHH@Z 0x004989F0-0x00498B06 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:a538e0a3:src/recovered/units/004989f0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
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
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00498B30-0x0049B1DC", "git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00498b30.cpp", "Retired recovered seed provenance: source:src/recovered/units/00430b30.cpp", "pinned:0x0049A5FC stores kind0x62 then0x0049A606 stores the chassis name value after the link text and before chassis detail formatting", "pinned:0x0049A7CD stores kind0x63 then0x0049A7D7 stores the weapon name value after the link text and before weapon detail formatting", "pinned:0x0049A9E1 stores kind0x64 then0x0049A9EB stores the armor name value after the link text and before armor detail formatting", "source:src/datalink.h TechnologyHelpEntryName/Kind are the independently observed 4-byte shared objects; Datalink help and tech_achieved reset use the same pair", "source:src/reportif.cpp reportwin_draw_lab_unlock_sections already traverses these same typed Chassis/Weapon/Armor rows and preserves its original lab detail and callback paths"], "reason": "The ReportWin laboratory page is shared by the ordinary report and Datalink entry renderer. Recover its real Buffer hosts, FlatButton members, technology Sprite array and ReportIf fonts; original chart geometry, status rows and unlock-list scans identify these owners. Retain the historical seed for further matching and runtime verification. Preserve the shared technology-help entry name/kind pair at the original chassis, weapon and armor emission points while keeping ReportWin's real two-pass lab/unlock rendering, existing table bounds, section order and link/detail ordering."}
// TRIED: full pixel-geometry reconstruction of the lab-tier bar chart (labs_line/FUN_0049b450/vtable-slot draw calls with exact coordinate math across up to 15 rows and 4 switch-selected sprite tables) - this pass preserves top-level branch shape and call order, not the per-row layout arithmetic.
void ReportWin::draw_labs() {
    const int faction = ReportIfGlobal.report_faction_id_;
    const bool datalink = IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 8;
    Buffer &canvas = datalink
        ? *static_cast<Buffer *>(&DatalinkMain)
        : *static_cast<Buffer *>(this);
    if (datalink) canvas.set_text_color(0xFE, -1, 1, 1);
    // Native 0x498B90 reads the four Datalink entry-clip edges. The ordinary
    // report instead reads rect1_ as left/top/width/height at 0x498C5A.
    RECT host_area;
    if (datalink) {
        host_area = DatalinkMain.entry_clip_rect();
    } else {
        host_area.left = rect1_.left;
        host_area.top = rect1_.top;
        host_area.right = rect1_.left + rect1_.right;
        host_area.bottom = rect1_.top + rect1_.bottom;
    }
    const int left = host_area.left + 3;
    const int top = host_area.top + 3;
    const int right = host_area.right - 3;
    const int bottom = host_area.bottom - 3;
    const int width = right - left;
    const int height = bottom - top;
    // These are the button-cell dimensions published before chart setup;
    // they differ when the ordinary renderer is embedded by Datalink.
    field_59A0_ = datalink ? 0x96 : 0xBE;
    field_59A4_ = 0x28;
    field_59A8_ = datalink ? 0xAA : 0xBE;
    // Chart anchors are prepared before its controls and connector graph.
    // Native 0x498B90..C55 uses the Datalink entry clip; 0x498C5A..D1A
    // uses ReportIf's established wide-screen state and ReportWin::rect1_.
    const bool wide_layout = !datalink
        && ReportIfGlobal.field_1E230_ != 0;
    if (datalink) {
        field_A2C_ = top + 0x1E;
        field_A28_ = host_area.left + 3
            + ((host_area.right - host_area.left - 0xB0) / 2);
        field_A30_ = field_A28_ + 0xAA;
        field_A34_ = field_A2C_ + 0xF0;
    } else {
        field_A2C_ = host_area.top + 0x63
            + (wide_layout ? 0x62 : 0);
        field_A28_ = host_area.left
            + ((host_area.right - host_area.left - 0xBE) / 2);
        field_A30_ = field_A28_ + 0xBE;
        field_A34_ = field_A2C_ + 0xAE;
    }
    // Native 0x498CC3 snapshots the wide-layout flag and publishes these
    // anchor members before calling energy_compute at 0x498D20. That call is
    // allowed to update global accounting, but cannot retroactively change
    // this frame's already-selected chart layout.
    if (!datalink) {
        energy_compute(faction, 0);
    }
    // Native 0x498D26 samples the current player's research income only after
    // energy_compute has run. Keep this read here: that call updates the live
    // accounting used by the research header below.
    const int current_tech_rate = datalink ? 0 : tech_rate(faction);
    const bool hide_research = !datalink && (GameDataStateStorage.fields_.game_state_ & 0x80) == 0
        && (GameDataStateStorage.fields_.game_rules_ & RULES_BLIND_RESEARCH) != 0;
    int research_header_end_x = host_area.left + 0x49;
    int research = -1;
    if (!datalink) {
        RECT research_header = {rect1_.left, rect1_.top,
            rect1_.left + rect1_.right, rect1_.top + 0x46};
        canvas.set_text_color(0xE1, -1, 1, 1);
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        canvas.box_sprite(&research_header, &InterfaceBoxStyles[3]);

        // Native 0x498DA0..0x498DF7 suppresses current-tech detail only for
        // blind research when the game-state reveal bit is clear.
        if (hide_research) {
            RECT icon_frame = {host_area.left + 3, host_area.top + 3,
                host_area.right - 3, 0x2F};
            canvas.box_sprite(&icon_frame, &InterfaceBoxStyles[1]);
            strcat(StringTemp, label_get(0xAC4 / 4));
        } else {
            research = PlayersData[faction].tech_id_researching;
            if (research >= 0) {
                const int research_color = mandate_color(tech_category(research));
                Sprite &research_icon = g_TECH_ICON_SPRITES[research];
                research_icon.draw_mono(&canvas,
                    research_icon.transparent_index(), host_area.left + 3,
                    host_area.top + 3, research_color, 0x40,
                    research_icon.width());
            }

            // Original 0x498E61 draws style 1 over the inset icon area. Its
            // bottom edge is the literal 0x2f local retained by the image.
            RECT icon_frame = {host_area.left + 3, host_area.top + 3,
                host_area.right - 3, 0x2F};
            canvas.box_sprite(&icon_frame, &InterfaceBoxStyles[1]);

            // 0x498E71..0x498EF1 emits the research label first; write_l's
            // returned pen x is then reused by the adjacent tech name.
            strcat(StringTemp, label_get(0x254 / 4));
            strcat(StringTemp, "  ");
            strcat(StringTemp, "]");
            const int text_x = host_area.left + 0x49;
            const int text_y = host_area.top + 8;
            research_header_end_x = canvas.write_l(StringTemp, text_x,
                text_y, strlen(StringTemp));
            StringTemp[0] = 0;
            strcat(StringTemp, "[");
            if (research >= 0) {
                say_tech(StringTemp, research, 0);
                strcat(StringTemp, "]");
                CharUpperA(StringTemp);
                if (research == MaxTechnologyNum - 1) {
                    StringTemp[strlen(StringTemp) - 1] = 0;
                    strcat(StringTemp, " ");
                    reportwin_append_number(StringTemp,
                        PlayersData[faction].tech_accumulated + 1);
                } else {
                    canvas.write_l(StringTemp, research_header_end_x,
                        text_y, strlen(StringTemp));
                }
            }
        }
    }
    // Native 0x49907C..0x4990AA rebuilds this ordinary-report hit region
    // after the header text/technology icon phase and before chart setup.
    if (!datalink) {
        spot_.kill_type(0);
        spot_.add(0, 0x29A, left, top, width, height);
        if (hide_research) {
            strcat(StringTemp, "DISCOV");
            canvas.write_cent_l(StringTemp, research_header_end_x,
                host_area.top + 8,
                host_area.right - 3 - (host_area.left + 0x46),
                strlen(StringTemp));
        } else {
            strcat(StringTemp, "]");
            canvas.write_l(StringTemp, research_header_end_x,
                host_area.top + 8, strlen(StringTemp));
        }

        const int chart_technology = PlayersData[faction].tech_id_researching;
        if (chart_technology >= 0 && chart_technology < MaxTechnologyNum) {
            RECT category_bar = {left, 0x31, right, host_area.top + 0x43};
            const int original_left = category_bar.left;
            const int original_right = category_bar.right;
            int category = tech_category(
                PlayersData[faction].tech_id_researching);
            BoxSpriteParams *style = reportwin_lab_category_style(category, 38);
            if (style != 0) canvas.box_sprite(&category_bar, style);

            int fill_right = original_left
                + ((original_right - original_left)
                    * PlayersData[faction].tech_accumulated)
                    / current_tech_rate;
            if (fill_right < original_left || original_right < original_left)
                fill_right = original_left;
            else if (fill_right > original_right)
                fill_right = original_right;
            category_bar.right = fill_right;
            canvas.set_clip(&category_bar);
            category = tech_category(PlayersData[faction].tech_id_researching);
            style = reportwin_lab_category_style(category, 36);
            if (style != 0) canvas.box_sprite(&category_bar, style);
            canvas.set_clip(buffer_working_rect());

            const int next_fill_right = fill_right
                + (((original_right - original_left) * EnergyLabsTotal / 100)
                    / current_tech_rate);
            const int bounded_next_fill_right = next_fill_right < fill_right
                || original_right < fill_right ? fill_right
                : next_fill_right > original_right ? original_right
                : next_fill_right;
            RECT next_turn = {fill_right, category_bar.top,
                bounded_next_fill_right, category_bar.bottom};
            canvas.set_clip(&next_turn);

            next_turn.left -= 7;
            next_turn.top -= 7;
            next_turn.right += 7;
            next_turn.bottom += 7;
            category = tech_category(PlayersData[faction].tech_id_researching);
            style = reportwin_lab_category_style(category, 36);
            if (style != 0) canvas.box_sprite(&next_turn, style);

            next_turn.left += 7;
            next_turn.top += 7;
            next_turn.right -= 7;
            next_turn.bottom -= 7;
            canvas.set_clip(buffer_working_rect());
            RECT inner_clip = {next_turn.left + 1, next_turn.top + 1,
                next_turn.right - 1, next_turn.bottom - 1};
            canvas.set_clip(&inner_clip);
            RECT outer_edge = {inner_clip.left - 7, inner_clip.top - 7,
                inner_clip.right + 7, inner_clip.bottom + 7};
            category = tech_category(PlayersData[faction].tech_id_researching);
            style = reportwin_lab_category_style(category, 38);
            if (style != 0) canvas.box_sprite(&outer_edge, style);
            canvas.set_clip(buffer_working_rect());
        }
    }
    // Native 0x499433..0x499521 emits the discovery and estimated-next-
    // discovery captions after the category fill. Its x starts at the
    // header-text column (0x49 from the host left); the y is the same native
    // host-top + field_E268_ + 0x0b cursor established at 0x499172..189.
    if (!datalink) {
        const int progress_x = host_area.left + 0x49;
        const int progress_y = host_area.top + field_E268_ + 0x0B;
        canvas.set_text_color(0xE1, -1, 1, 1);
        parse_num(0, BreakthroughTurnsGlobal);
        parse_it(ScriptPopupFile, "DISCOV");
        const int next_caption_x = canvas.write_l(StringTemp, progress_x,
            progress_y, static_cast<int>(strlen(StringTemp))) + 4;

        int remaining = current_tech_rate
            - PlayersData[faction].tech_accumulated;
        if (remaining < 0) remaining = 0;
        else if (remaining > 0x5F5E0FF) remaining = 0x5F5E0FF;
        int turns_to_discovery = 0x270F;
        if (EnergyLabsTotal != 0) {
            turns_to_discovery = (remaining * 100 + EnergyLabsTotal - 1)
                / EnergyLabsTotal;
        }
        parse_num(0, static_cast<int>(game_year(
            GameDataStateStorage.fields_.turn_current_num_ + turns_to_discovery)));
        parse_it(ScriptPopupFile, "NEXTDISCOV");
        canvas.write_l(StringTemp, next_caption_x, progress_y,
            static_cast<int>(strlen(StringTemp)));
    }
    // Native 0x499524..0x499660 sizes the chart from the report frame and
    // lays out its three horizontal tiers around the selected-tech column.
    const int chart_left = datalink ? left : host_area.left;
    const int chart_width = datalink ? width : rect1_.right;
    const int chart_y = field_E268_
        + (datalink ? host_area.top + 3 + 0x14 : host_area.top + 0x46) + 0x0F;
    const int chart_height = static_cast<int>(energy_panel_rect_.bottom)
        - chart_y - 0x1E;
    int tier_height = chart_height;
    if (!datalink && WinScreenWidth != 0x320) tier_height -= 0xC8;
    rect1_.right = chart_width;
    rect1_.bottom = chart_height;
    lab_prerequisite_x_ = chart_left + 5;
    lab_center_x_ = chart_left + 5
        + (chart_width - static_cast<int>(field_59A0_)) / 2 - 5;
    lab_child_x_ = chart_left + 5 + chart_width
        - static_cast<int>(field_59A0_) - 10;
    lab_center_x_ = field_A28_;
    lab_center_y_ = chart_y
        + (tier_height - static_cast<int>(field_59A4_)) / 2;
    lab_three_child_upper_y_ = lab_center_y_ - static_cast<int>(field_59A4_) - 0x10;
    lab_three_child_lower_y_ = lab_center_y_ + static_cast<int>(field_59A4_) + 0x10;
    const int tier_pitch = static_cast<int>(field_59A4_) + 0x10;
    lab_four_child_top_y_ = lab_center_y_ - tier_pitch - tier_pitch / 2;
    lab_upper_inner_y_ = lab_center_y_ - tier_pitch / 2;
    lab_lower_inner_y_ = lab_center_y_ + tier_pitch - tier_pitch / 2;
    lab_four_child_bottom_y_ = lab_center_y_ + tier_pitch * 2 - tier_pitch / 2;

    // The native two-prerequisite route creates the left/right prerequisite
    // controls (0x499867..0x499A99); the one-prerequisite route creates the
    // center control (0x499722..0x499866). The common one/four-child routes
    // use the four owned FlatButtons at indices 3..6 (0x499AE2..0x499C7E).
    int status_child_count = 0;
    if (field_FAB8_ >= 0 && field_FAB8_ < MaxTechnologyNum) {
        const int prerequisite1 = Technology[field_FAB8_].preq_tech_1;
        const int prerequisite2 = Technology[field_FAB8_].preq_tech_2;
        Win *const parent = datalink ? static_cast<Win *>(&DatalinkMain)
                                     : static_cast<Win *>(this);
        if (prerequisite1 >= 0 && prerequisite2 >= 0) {
            labs_line(reportwin_signed_coordinate(field_A28_),
                      reportwin_signed_coordinate(lab_center_y_),
                      reportwin_signed_coordinate(lab_prerequisite_x_),
                      reportwin_signed_coordinate(lab_upper_inner_y_),
                      field_59A0_, field_59A4_, 0);
            reportwin_init_lab_button(canvas, flatButtons1_[1], prerequisite1,
                0x379, reportwin_signed_coordinate(lab_prerequisite_x_),
                reportwin_signed_coordinate(lab_upper_inner_y_), field_59A0_, field_59A4_,
                parent, faction);
            labs_line(reportwin_signed_coordinate(field_A28_),
                      reportwin_signed_coordinate(lab_center_y_),
                      reportwin_signed_coordinate(lab_prerequisite_x_),
                      reportwin_signed_coordinate(lab_lower_inner_y_),
                      field_59A0_, field_59A4_, 0);
            reportwin_init_lab_button(canvas, flatButtons1_[2], prerequisite2,
                0x37A, reportwin_signed_coordinate(lab_prerequisite_x_),
                reportwin_signed_coordinate(lab_lower_inner_y_), field_59A0_, field_59A4_,
                parent, faction);
        } else if (prerequisite1 >= 0) {
            labs_line(reportwin_signed_coordinate(field_A28_),
                      reportwin_signed_coordinate(lab_center_y_),
                      reportwin_signed_coordinate(lab_prerequisite_x_),
                      reportwin_signed_coordinate(lab_center_y_),
                      field_59A0_, field_59A4_, 0);
            reportwin_init_lab_button(canvas, flatButtons1_[1], prerequisite1,
                0x379, reportwin_signed_coordinate(lab_prerequisite_x_),
                reportwin_signed_coordinate(lab_center_y_), field_59A0_, field_59A4_,
                parent, faction);
        }

        // Native scratch begins at EBP-0x7c and spans five slots to EBP-0x6c.
        // That final slot's prior local use ends at 0x499169; it is not
        // initialized for this scan and is written only on a fifth match.
        int children[5];
        children[0] = children[1] = children[2] = children[3] = -1;
        int child_count = reportwin_collect_lab_children(field_FAB8_, children);
        status_child_count = child_count;
        if (child_count > 4) child_count = 4;
        if (child_count == 1 || child_count == 2
            || child_count == 3 || child_count == 4) {
            int x = reportwin_signed_coordinate(lab_child_x_);
            int y = child_count == 4 ? reportwin_signed_coordinate(lab_four_child_top_y_)
                : child_count == 3 ? reportwin_signed_coordinate(lab_three_child_upper_y_)
                : child_count == 2 ? reportwin_signed_coordinate(lab_upper_inner_y_)
                : reportwin_signed_coordinate(lab_center_y_);
            for (int child = 0; child < child_count; ++child) {
                labs_line(x, y, reportwin_signed_coordinate(lab_center_x_),
                          reportwin_signed_coordinate(lab_center_y_),
                          field_59A0_, field_59A4_, 0);
                reportwin_init_lab_button(canvas, flatButtons1_[child + 3],
                    children[child], 0x37B + child, x, y,
                    field_59A0_, field_59A4_, parent, faction);
                if (child_count == 4) {
                    if (child == 0) y = reportwin_signed_coordinate(lab_upper_inner_y_);
                    else if (child == 1) y = reportwin_signed_coordinate(lab_lower_inner_y_);
                    else if (child == 2) y = reportwin_signed_coordinate(lab_four_child_bottom_y_);
                } else if (child_count == 2) {
                    if (child == 0) y = reportwin_signed_coordinate(lab_lower_inner_y_);
                } else if (child_count == 3) {
                    y = child == 0 ? reportwin_signed_coordinate(lab_center_y_)
                        : child == 1 ? reportwin_signed_coordinate(lab_three_child_lower_y_) : y;
                }
            }
        }
    }

    int measured_height = 0;
    int status_y = 0;
    const int section_x = left + (datalink ? 3 : 0);
    field_A24_ = 0;
    for (int pass = 0; pass < 2; ++pass) {
        const bool draw_pass = pass != 0;
        reportwin_draw_lab_status_rows(canvas, field_FAB8_, faction,
            field_A28_ + 3, field_A34_ + field_A2C_,
            field_59A8_, status_y, measured_height, draw_pass,
            status_child_count == 3,
            ReportIfGlobal.field_1E19C_, ReportIfGlobal.field_1E190_);

        if (field_FAB8_ != -1) {
            // The native host configuration runs in each outer pass. HOST
            // chooses the font and colors; PASS chooses measure versus draw.
            if (datalink) {
                canvas.set_wrap_width(10);
                canvas.set_font(DatalinkMain.entry_text_font(), 0, 0,
                                DatalinkMain.entry_link_font());
                canvas.set_text_color(0x99, -1, 1, 1);
                canvas.set_text_color_hyper(0x96, -1, 1, 1);
            } else {
                canvas.set_text_color(0xE1, -1, 1, 1);
                canvas.set_text_color2(0xE1, -1, 1, 1);
            }
            int section_y = status_y;
            reportwin_draw_lab_unlock_sections(canvas, field_FAB8_, datalink,
                draw_pass, section_x, section_y, &field_59A8_, measured_height);
        }
        if (!draw_pass) field_A24_ = measured_height;
    }

    // The six native list controls have completed their status rows. The
    // host's wrap width is cleared before mouse feedback at 0x49B186.
    canvas.set_wrap_width(0);

    // 0x49B178..0x49B1B5 refreshes mouse feedback on the six technology
    // controls after their geometry has settled. The original uses the
    // inherited Win::get_mouse_pos and virtual on_mouse_move methods
    // (slot 17); these are real FlatButton subobjects, not an address table.
    int mouse_x = 0;
    int mouse_y = 0;
    for (int button = 1; button < 7; ++button) {
        flatButtons1_[button].get_mouse_pos(&mouse_x, &mouse_y);
        flatButtons1_[button].on_mouse_move(mouse_x, mouse_y);
    }

    // The native host clears Buffer::field_10_ at GraphicWin+0x454, refreshes
    // the six buttons above, then restores the host's working clip at
    // 0x49B1BA..1D5. This typed host path supplies that real Buffer subobject.
    canvas.set_clip(datalink ? DatalinkMain.buffer_working_rect()
                            : buffer_working_rect());
    // 0x49B1C3..1CF publishes the final chart cursor in the owned RECT.
    rect1_.left = mouse_x;
    rect1_.top = mouse_y;
    // Native 0x498D25..0x499523 is covered above: the icon uses the sprite's
    // native width as its draw_mono denominator, the final technology formats
    // accumulated research instead of a normal technology name, and the four
    // category-bar boxes retain their native style dispatch and clip sequence.
}

// ORIGINAL: 0x0049B220 ?on_button_passover@ReportWin@@QAEXHH@Z 0x0049B220-0x0049B446
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0049B220-0x0049B446 and 0x0065C0C2-0x0065C20E", "Retired recovered seed provenance: git:a538e0a3:src/recovered/units/0049b220.cpp retains the recovered seed and original claim", "source:src/reportwin.h", "source:src/reportif.h:ReportIfGlobal.field_8_"], "reason": "Recover the category-button passover drawing through the seven owned FlatButtons, shared work RECT, real host window and existing labs_line. The original seed and historical claim remain restoration obligations; current byte correspondence is measured separately."}
// symbol    ?on_button_passover@ReportWin@@UAEXHH@Z
void ReportWin::on_button_passover(int button_id, int state) {
    GraphicWin *host = IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 8
        ? static_cast<GraphicWin *>(&DatalinkMain)
        : static_cast<GraphicWin *>(this);
    if (ReportIfGlobal.field_8_ != 2 || button_id < 0x378) return;

    const int index = button_id - 0x378;
    // Original 0x0049B26C indexes without an upper-bound check. Preserve that
    // path rather than silently adding a behavior-changing safety guard.
    report_work_rect_ = flatButtons1_[index].outer_rect();

    int first_x, first_y, last_x, last_y;
    if (index < 3) {
        first_x = reportwin_signed_coordinate(lab_center_x_);
        first_y = reportwin_signed_coordinate(lab_center_y_);
        last_x = report_work_rect_.left;
        last_y = report_work_rect_.top;
    } else {
        first_x = report_work_rect_.left;
        first_y = report_work_rect_.top;
        last_x = reportwin_signed_coordinate(lab_center_x_);
        last_y = reportwin_signed_coordinate(lab_center_y_);
    }

    report_work_rect_.left -= 3;
    report_work_rect_.top -= 3;
    report_work_rect_.right += 3;
    report_work_rect_.bottom += 3;

    const int center_y = (reportwin_signed_coordinate(field_A34_)
        + reportwin_signed_coordinate(field_A2C_)
        - reportwin_signed_coordinate(field_A24_)) / 2;
    RECT marker = {
        reportwin_signed_coordinate(field_A28_) - 3,
        center_y - 9,
        reportwin_signed_coordinate(field_59A8_ + field_A28_) + 3,
        reportwin_signed_coordinate(field_A24_) + center_y + 6
    };
    if (state != 0) {
        host->Buffer::box_sprite(&marker, &InterfaceBoxStyles[5]);
        marker.left += 3;
        marker.top += 3;
        marker.right -= 3;
        marker.bottom -= 3;
        host->Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[5]);
        labs_line(first_x, first_y, last_x, last_y,
                  static_cast<int>(field_59A0_), static_cast<int>(field_59A4_), 1);
    } else {
        host->Buffer::box_sprite(&marker, &InterfaceBoxStyles[3]);
        marker.left += 3;
        marker.top += 3;
        marker.right -= 3;
        marker.bottom -= 3;
        host->Buffer::box_sprite(&report_work_rect_, &InterfaceBoxStyles[3]);
        labs_line(first_x, first_y, last_x, last_y,
                  static_cast<int>(field_59A0_), static_cast<int>(field_59A4_), 0);
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
*/
void ReportWin::labs_line(int first_id, int second_id, int, int branch_id,
                          int x_offset, int line_height, int style_select) {
    // The research screen draws on the Datalink's own surface; every other
    // interface draws on this window's. Both hosts are GraphicWin-derived,
    // so the Buffer base sits at +0x444 either way.
    Buffer *buffer;
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 8) {
        buffer = static_cast<Buffer *>(this);
    } else {
        buffer = static_cast<Buffer *>(&DatalinkMain);
    }
    // The arrow draw's guarded receiver: the image passes the buffer only
    // after re-testing the host, so the seed kept the null-guard form.
    Buffer *draw_buffer = (buffer != 0) ? buffer : static_cast<Buffer *>(0);

    int style_flag = (style_select != 0) ? 2 : 0;
    Sprite *arrow = &g_IFACE_TECH_TREE_ARROW_SPRITES[style_flag];
    unsigned char sprite_byte = arrow->transparent_index();
    BoxSpriteParams *box_style = (style_select == 0)
                                     ? &InterfaceBoxStyles[3]
                                     : &InterfaceBoxStyles[5];

    RECT *work_rect = &report_work_rect_;

    const int line_id = reportwin_signed_coordinate(lab_center_x_);
    int half_height = line_height / 2;
    int centre_y = reportwin_signed_coordinate(lab_center_y_) + half_height;

    int tail_bottom, tail_right;

    if (first_id == line_id) {
        const int line_left = reportwin_signed_coordinate(lab_prerequisite_x_);
        int centre_x = (line_left + line_id + x_offset) / 2;
        if (branch_id == reportwin_signed_coordinate(lab_upper_inner_y_)
            || branch_id == reportwin_signed_coordinate(lab_lower_inner_y_)) {
            work_rect->left = line_left + 3 + x_offset;
            work_rect->top = reportwin_signed_coordinate(lab_upper_inner_y_) + half_height;
            work_rect->right = centre_x - 2;
            work_rect->bottom = reportwin_signed_coordinate(lab_lower_inner_y_) + half_height;
            if (branch_id == reportwin_signed_coordinate(lab_upper_inner_y_)) {
                work_rect->bottom = centre_y;
            } else {
                work_rect->top = centre_y + 1;
            }
            buffer->set_clip(work_rect);
            work_rect->bottom = reportwin_signed_coordinate(lab_lower_inner_y_) + half_height;
            work_rect->left = reportwin_signed_coordinate(lab_prerequisite_x_) - 4 + x_offset;
            work_rect->top = reportwin_signed_coordinate(lab_upper_inner_y_) + half_height;
            work_rect->right = centre_x - 2;
            box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                            : &InterfaceBoxStyles[5];
            buffer->box_sprite(work_rect, box_style);
        } else {
            int left_edge = line_left - 4 + x_offset;
            work_rect->right = centre_x + 5;
            work_rect->top = centre_y;
            work_rect->left = left_edge;
            work_rect->bottom = centre_y + 0x14;
            work_rect->left = left_edge + 7;
            work_rect->right -= 7;
            work_rect->bottom -= 7;
            buffer->set_clip(work_rect);
            work_rect->left -= 7;
            work_rect->right += 7;
            work_rect->bottom += 7;
            box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                            : &InterfaceBoxStyles[5];
            buffer->box_sprite(work_rect, box_style);
        }
        buffer->restore_clip();
        work_rect->left = centre_x - 9;
        work_rect->top = centre_y;
        work_rect->right = line_id + 4;
        work_rect->bottom = centre_y + 0x14;
        work_rect->left += 7;
        work_rect->right -= 7;
        work_rect->bottom -= 7;
        buffer->set_clip(work_rect);
        work_rect->left -= 7;
        work_rect->right += 7;
        work_rect->bottom += 7;
        box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                        : &InterfaceBoxStyles[5];
        buffer->box_sprite(work_rect, box_style);
        buffer->restore_clip();
        arrow->draw(draw_buffer, sprite_byte, line_id - 10,
                    reportwin_signed_coordinate(lab_center_y_) - 6 + half_height);
        return;
    }

    const int line_right = reportwin_signed_coordinate(lab_child_x_);
    const int mid_anchor = reportwin_signed_coordinate(field_59A8_);
    int mid_x = (mid_anchor + line_right + line_id) / 2;
    int mid_x2 = mid_x - 2;

    if (second_id == reportwin_signed_coordinate(lab_three_child_upper_y_) || second_id == reportwin_signed_coordinate(lab_three_child_lower_y_)) {
        work_rect->left = mid_x2;
        work_rect->top = reportwin_signed_coordinate(lab_three_child_upper_y_) + half_height;
        work_rect->right = line_right - 3;
        work_rect->bottom = reportwin_signed_coordinate(lab_three_child_lower_y_) + half_height;
        int draw_x, draw_y;
        if (second_id == reportwin_signed_coordinate(lab_three_child_upper_y_)) {
            work_rect->bottom = centre_y;
            draw_x = line_right;
            draw_y = reportwin_signed_coordinate(lab_three_child_upper_y_) - 6;
        } else {
            work_rect->top = centre_y + 1;
            draw_x = line_right;
            draw_y = reportwin_signed_coordinate(lab_three_child_lower_y_) - 7;
        }
        arrow->draw(draw_buffer, sprite_byte, draw_x - 10,
                    draw_y + half_height);
        buffer->set_clip(work_rect);
        tail_right = line_right + 4;
        tail_bottom = reportwin_signed_coordinate(lab_three_child_lower_y_) + half_height;
        work_rect->top = reportwin_signed_coordinate(lab_three_child_upper_y_) + half_height;
        work_rect->left = mid_x2;
    } else if (second_id == reportwin_signed_coordinate(lab_four_child_top_y_) || second_id == reportwin_signed_coordinate(lab_four_child_bottom_y_)) {
        work_rect->left = mid_x2;
        work_rect->top = reportwin_signed_coordinate(lab_four_child_top_y_) + half_height;
        work_rect->right = line_right - 3;
        work_rect->bottom = reportwin_signed_coordinate(lab_four_child_bottom_y_) + half_height;
        int draw_x, draw_y;
        if (second_id == reportwin_signed_coordinate(lab_four_child_top_y_)) {
            work_rect->bottom = centre_y;
            draw_x = line_right;
            draw_y = reportwin_signed_coordinate(lab_four_child_top_y_) - 6;
        } else {
            work_rect->top = centre_y + 1;
            draw_x = line_right;
            draw_y = reportwin_signed_coordinate(lab_four_child_bottom_y_) - 7;
        }
        arrow->draw(draw_buffer, sprite_byte, draw_x - 10,
                    draw_y + half_height);
        buffer->set_clip(work_rect);
        tail_right = line_right + 4;
        tail_bottom = reportwin_signed_coordinate(lab_four_child_bottom_y_) + half_height;
        work_rect->top = reportwin_signed_coordinate(lab_four_child_top_y_) + half_height;
        work_rect->left = mid_x2;
    } else if (second_id != reportwin_signed_coordinate(lab_upper_inner_y_) && second_id != reportwin_signed_coordinate(lab_lower_inner_y_)) {
        arrow->draw(draw_buffer, sprite_byte, line_right - 10,
                    centre_y - 6);
        work_rect->right = line_right + 4;
        work_rect->top = centre_y;
        work_rect->left = mid_x - 9;
        work_rect->bottom = centre_y + 0x14;
        work_rect->left += 7;
        work_rect->right -= 7;
        work_rect->bottom -= 7;
        buffer->set_clip(work_rect);
        work_rect->left -= 7;
        work_rect->right += 7;
        work_rect->bottom += 7;
        goto common_tail;
    } else {
        work_rect->left = mid_x2;
        work_rect->top = reportwin_signed_coordinate(lab_upper_inner_y_) + half_height;
        work_rect->right = line_right - 3;
        work_rect->bottom = reportwin_signed_coordinate(lab_lower_inner_y_) + half_height;
        int draw_x, draw_y;
        if (second_id == reportwin_signed_coordinate(lab_upper_inner_y_)) {
            work_rect->bottom = centre_y;
            draw_x = line_right;
            draw_y = reportwin_signed_coordinate(lab_upper_inner_y_) - 6;
        } else {
            work_rect->top = centre_y + 1;
            draw_x = line_right;
            draw_y = reportwin_signed_coordinate(lab_lower_inner_y_) - 7;
        }
        arrow->draw(draw_buffer, sprite_byte, draw_x - 10,
                    draw_y + half_height);
        buffer->set_clip(work_rect);
        tail_right = line_right + 4;
        tail_bottom = reportwin_signed_coordinate(lab_lower_inner_y_) + half_height;
        work_rect->top = reportwin_signed_coordinate(lab_upper_inner_y_) + half_height;
        work_rect->left = mid_x2;
    }
    work_rect->right = tail_right;
    work_rect->bottom = tail_bottom;
common_tail:
    box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                    : &InterfaceBoxStyles[5];
    buffer->box_sprite(work_rect, box_style);
    buffer->restore_clip();
    {
        int left_edge = reportwin_signed_coordinate(field_59A8_) - 4
            + reportwin_signed_coordinate(lab_center_x_);
        work_rect->left = left_edge;
        work_rect->top = centre_y;
        if (style_select == 0) {
            work_rect->right = mid_x + 5;
            work_rect->bottom = centre_y + 0x14;
            work_rect->left += 7;
            work_rect->right -= 7;
            work_rect->bottom -= 7;
            buffer->set_clip(work_rect);
            box_style = &InterfaceBoxStyles[3];
            work_rect->left -= 7;
            work_rect->right += 7;
            work_rect->bottom += 7;
        } else {
            work_rect->right = mid_x + 5;
            work_rect->bottom = centre_y + 0x14;
            work_rect->left = left_edge + 7;
            work_rect->right -= 7;
            work_rect->bottom -= 7;
            buffer->set_clip(work_rect);
            box_style = &InterfaceBoxStyles[5];
            work_rect->left -= 7;
            work_rect->right += 7;
            work_rect->bottom += 7;
        }
    }
    buffer->box_sprite(work_rect, box_style);
    buffer->restore_clip();
}

/*
Purpose: Draw the energy page's five-panel frame and calculate the faction's
         underlying accounts. Text, row breakdowns and selection are pending.
// ORIGINAL: 0x0049BCD0 ?draw_energy@ReportWin@@QAEXXZ 0x0049BCD0-0x0049D481 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0049bcd0.cpp", "src/reportwin.h", "src/socialwin.cpp"], "reason": "Recover the energy-page frame and shared accounting through owned ReportWin geometry and the typed energy computation; preserve the remaining row and selection work explicitly."}
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
    Buffer *canvas = static_cast<Buffer *>(this);
    const int viewer = ReportIfGlobal.report_faction_id();
    const int line_height = static_cast<int>(field_E268_);
    const int middle = (energy_panel_rect_.left + energy_panel_rect_.right) / 2;
    const int first_break = energy_panel_rect_.top + line_height + 9;
    const int second_break = first_break + line_height + 9;

    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3, first_break};
    RECT reserves = {energy_panel_rect_.left + 3, first_break + 2,
                     middle - 1, second_break};
    RECT income = {middle + 2, first_break + 2,
                   energy_panel_rect_.right - 3, second_break};
    RECT expenses = {energy_panel_rect_.left + 3, second_break + 2,
                     middle - 1, energy_panel_rect_.bottom - 3};
    RECT balance = {middle + 2, second_break + 2,
                    energy_panel_rect_.right - 3,
                    energy_panel_rect_.bottom - 3};
    canvas->box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[3]);
    canvas->box_sprite(&title, &InterfaceBoxStyles[1]);
    canvas->box_sprite(&reserves, &InterfaceBoxStyles[1]);
    canvas->box_sprite(&income, &InterfaceBoxStyles[1]);
    canvas->box_sprite(&expenses, &InterfaceBoxStyles[0]);
    canvas->box_sprite(&balance, &InterfaceBoxStyles[0]);
    canvas->set_text_color(0x99, -1, 1, 1);
    canvas->set_text_color2(0xf2, -1, 1, 1);
    canvas->set_text_color3(0xe1, -1, 1, 1);

    energy_compute(viewer, 0);
    rect1_.left = middle + 8;
    rect1_.top = second_break + 8;
    rect1_.right = -15;

    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[viewer].adj_name_faction);
    CharUpperA(StringTemp);
    GenderDefault = Players[viewer].noun_gender;
    PluralityDefault = Players[viewer].is_noun_plural;
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xA98 / 4));
    strcat(StringTemp, "]");
    canvas->write_cent_l(StringTemp, &title, strlen(StringTemp));

    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0xB9C / 4));
    strcat(StringTemp, "]");
    canvas->write_cent_l(StringTemp, &reserves, strlen(StringTemp));
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0xE4 / 4));
    strcat(StringTemp, ": ");
    canvas->write_l(StringTemp, reserves.left + 8, reserves.top + line_height + 5,
                    strlen(StringTemp));
    strcat(StringTemp, "}");
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    reportwin_append_number(StringTemp, PlayersData[viewer].energy_reserves);
    strcat(StringTemp, "}");
    canvas->write_right_l(StringTemp, reserves.left + 8,
                          reserves.top + line_height + 5,
                          reserves.right - reserves.left - 16,
                          strlen(StringTemp));

    const int value_x = rect1_.left;
    // The original passes -15 as the right-text width, not the remaining
    // panel width (0x0049BCD0 stores it in ReportWin+0xA1C).
    const int value_width = static_cast<int>(rect1_.right);
    const int value_pitch = line_height;
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x5F4 / 4));
    strcat(StringTemp, ": ");
    canvas->write_l(StringTemp, value_x, rect1_.top, strlen(StringTemp));
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    reportwin_append_number(StringTemp, EnergyEconomyTotal);
    strcat(StringTemp, "}");
    canvas->write_right_l(StringTemp, value_x, rect1_.top, value_width,
                          strlen(StringTemp));

    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x5F8 / 4));
    strcat(StringTemp, ": ");
    canvas->write_l(StringTemp, value_x, rect1_.top + value_pitch,
                    strlen(StringTemp));

    StringTemp[0] = 0;
    strcat(StringTemp, "{-");
    reportwin_append_number(StringTemp, EnergyMaintenanceTotal);
    strcat(StringTemp, "}");
    canvas->write_right_l(StringTemp, value_x, rect1_.top + value_pitch * 2,
                          value_width, strlen(StringTemp));

    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x5FC / 4));
    strcat(StringTemp, ": ");
    canvas->write_l(StringTemp, value_x, rect1_.top + value_pitch * 3,
                    strlen(StringTemp));
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    if (EnergyCommerceDelta > 0) strcat(StringTemp, "+");
    reportwin_append_number(StringTemp, EnergyCommerceDelta);
    strcat(StringTemp, "}");
    canvas->write_right_l(StringTemp, value_x, rect1_.top + value_pitch * 3,
                          value_width, strlen(StringTemp));

    int relationship_y = rect1_.top + value_pitch * 4;
    int have_relationship_heading = 0;
    if ((Players[viewer].rule_flags & 0x4000) != 0) {
        have_relationship_heading = 1;
        relationship_y += line_height;
        const int interest = Players[viewer].rule_interest;
        parse_num(0, interest);
        char section[32];
        strcpy(section, "FACT");
        if (interest < 0) {
            strcat(section, "PEN");
        } else {
            strcat(section, "BON");
            if (interest == 0) strcat(section, "2");
        }
        parse_it(ScriptPopupFile, section);
        canvas->wrap_flying(StringTemp, rect1_.left, relationship_y,
                            rect1_.right);
        canvas->wrap_flying_flush();
        relationship_y += line_height * 2;
    }

    // The native table iterates the other seven factions, using the live
    // participant mask and PlayerData's typed bilateral treaty/loan arrays.
    // It keeps the original asymmetric balance/payment choices for outgoing
    // versus incoming loans.
    const unsigned char live_factions = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
    for (int other = 1; other < MaxPlayerNum; ++other) {
        if (other == viewer) continue;
        if (((live_factions >> (other & 31)) & 1) == 0
            || ((live_factions >> (viewer & 31)) & 1) == 0) continue;

        const int outgoing_balance = PlayersData[viewer].loan_balance[other];
        const int incoming_balance = PlayersData[other].loan_balance[viewer];
        const int incoming_sanction = PlayersData[other].sanction_turns;
        if (outgoing_balance == 0 && incoming_balance == 0) continue;

        const bool loan_payment =
            (PlayersData[viewer].diplo_treaties[other] & 0x10) != 0;
        if (!have_relationship_heading) {
            have_relationship_heading = 1;
            relationship_y += 10;
        }
        relationship_y += line_height * 2;

        char relationship_prefix[256];
        relationship_prefix[0] = 0;
        if (!loan_payment) {
            if ((outgoing_balance != 0 && incoming_sanction != 0)
                || (incoming_balance != 0
                    && PlayersData[viewer].energy_reserves != 0)) {
                strcat(relationship_prefix, "*");
                strcat(relationship_prefix, label_get(0x708 / 4));
                strcat(relationship_prefix, " ");
            }
        } else {
            strcat(relationship_prefix, "2");
            strcat(relationship_prefix, label_get(0x604 / 4));
            strcat(relationship_prefix, " ");
        }

        parse_says(3, relationship_prefix, -1, -1);
        GenderDefault = Players[other].noun_gender;
        PluralityDefault = 0;
        parse_says(0, Players[other].title_leader, -1, -1);
        GenderDefault = Players[other].noun_gender;
        PluralityDefault = 0;
        parse_says(1, Players[other].name_leader, -1, -1);
        GenderDefault = Players[other].noun_gender;
        PluralityDefault = Players[other].is_noun_plural;
        parse_says(2, Players[other].noun_faction, -1, -1);

        const int principal = outgoing_balance != 0
            ? outgoing_balance : incoming_balance;
        parse_num(1, principal);
        const int payment = outgoing_balance != 0
            ? PlayersData[viewer].loan_payment[other]
            : PlayersData[other].loan_payment[viewer];
        parse_num(0, payment);
        parse_it(ScriptPopupFile, loan_payment ? "LOANPAYMENT" : "LOANINCOME");
        canvas->wrap_flying(StringTemp, rect1_.left, relationship_y,
                            rect1_.right);
        canvas->wrap_flying_flush();
    }

    relationship_y += line_height * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xEFC / 4));
    strcat(StringTemp, ": ");
    canvas->write_l(StringTemp, value_x, relationship_y, strlen(StringTemp));
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    reportwin_append_number(StringTemp, NetIncomeGlobal);
    strcat(StringTemp, "}");
    canvas->write_right_l(StringTemp, value_x, relationship_y, value_width,
                          strlen(StringTemp));

    if (field_59AC_ == -1) {
        field_59B0_ = 0;
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (Bases[base_id].faction_id_current == viewer) {
                field_59B0_ = 1;
                field_59AC_ = base_id;
                break;
            }
        }
    }
    if (field_59AC_ == -1) return;

    rect1_.left = middle + 8;
    rect1_.top = second_break + 8;
    rect1_.right = energy_panel_rect_.right - 3 - (middle + 2) - 12;
    rect1_.bottom = energy_panel_rect_.bottom - 6;
    listBox_.close();
    Dialog &dialog = static_cast<Dialog &>(listBox_);
    RECT list_rect = {middle + 4, second_break + 5,
                      energy_panel_rect_.right - 6,
                      energy_panel_rect_.bottom - 6};
    dialog.init(&list_rect, 0);
    dialog.field_48_ = line_height + 3;
    dialog.set_dialog_font(ReportIfGlobal.field_1E184_,
                           ReportIfGlobal.field_1E184_,
                           ReportIfGlobal.field_1E184_);
    dialog.set_dialog_text_color(0x99, -1, 1, 1);
    dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    listBox_.set_report_text_color(0xF2);

    StringTemp[0] = 0;
    if (field_59B0_ == 0) {
        const int facility = field_59AC_;
        strcat(StringTemp, "[");
        say_facility(StringTemp, facility, 0);
        CharUpperA(StringTemp);
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x160 / 4));
        strcat(StringTemp, "]");
        RECT heading = {middle + 2, second_break + 2,
                        energy_panel_rect_.right - 6, second_break + 2};
        canvas->write_cent_l(StringTemp, &heading, strlen(StringTemp));

        int maintenance_total = 0;
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (facility >= 0x41
                || Bases[base_id].faction_id_current != viewer) continue;
            int byte_offset;
            int bit;
            bitmask(facility, &byte_offset, &bit);
            if ((Bases[base_id].facilities_built[byte_offset] & bit) == 0)
                continue;

            StringTemp[0] = 0;
            say_base(StringTemp, base_id);
            strcat(StringTemp, "|");
            const int maintenance = fac_maint(facility, viewer);
            const int project_base = GameDataStateStorage.fields_.secret_project_.planetary_energy_grid();
            const bool halved = project_base >= 0
                && Bases[project_base].faction_id_current == viewer;
            maintenance_total += maintenance;
            strcat(StringTemp, "{");
            reportwin_append_number(StringTemp,
                halved ? maintenance / 2 : maintenance);
            if (halved && (maintenance & 1) != 0) strcat(StringTemp, ".5");
            strcat(StringTemp, "}");
            listBox_.item(StringTemp, base_id + 0x400);
        }
        listBox_.item(" | ", 0x05F5E0FE);
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        strcat(StringTemp, label_get(0xA9C / 4));
        strcat(StringTemp, ": ");
        strcat(StringTemp, "|");
        const int project_base = GameDataStateStorage.fields_.secret_project_.planetary_energy_grid();
        const bool halved = project_base >= 0
            && Bases[project_base].faction_id_current == viewer;
        if (halved) {
            reportwin_append_number(StringTemp, maintenance_total / 2);
            if ((maintenance_total & 1) != 0) strcat(StringTemp, ".5");
        } else {
            reportwin_append_number(StringTemp, maintenance_total);
        }
        strcat(StringTemp, "}");
        listBox_.item(StringTemp, 99999999);
    } else {
        const int base_id = field_59AC_;
        strcat(StringTemp, "[");
        say_base(StringTemp, base_id);
        CharUpperA(StringTemp);
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0xA98 / 4));
        strcat(StringTemp, "]");
        RECT heading = {middle + 2, second_break + 2,
                        energy_panel_rect_.right - 6, second_break + 2};
        canvas->write_cent_l(StringTemp, &heading, strlen(StringTemp));

        int maintenance_total = 0;
        for (int facility = 0; facility < 0x41; ++facility) {
            if (Bases[base_id].faction_id_current != viewer) continue;
            int byte_offset;
            int bit;
            bitmask(facility, &byte_offset, &bit);
            if ((Bases[base_id].facilities_built[byte_offset] & bit) == 0)
                continue;

            StringTemp[0] = 0;
            say_facility(StringTemp, facility, 0);
            strcat(StringTemp, "|");
            const int maintenance = fac_maint(facility, viewer);
            const int project_base = GameDataStateStorage.fields_.secret_project_.planetary_energy_grid();
            const bool halved = project_base >= 0
                && Bases[project_base].faction_id_current == viewer;
            maintenance_total += maintenance;
            strcat(StringTemp, "{");
            reportwin_append_number(StringTemp,
                halved ? maintenance / 2 : maintenance);
            if (halved && (maintenance & 1) != 0) strcat(StringTemp, ".5");
            strcat(StringTemp, "}");
            listBox_.item(StringTemp, facility);
        }
        listBox_.item(" | ", 0x05F5E0FE);
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        strcat(StringTemp, label_get(0xA9C / 4));
        strcat(StringTemp, ": ");
        strcat(StringTemp, "|");
        const int project_base = GameDataStateStorage.fields_.secret_project_.planetary_energy_grid();
        const bool halved = project_base >= 0
            && Bases[project_base].faction_id_current == viewer;
        if (halved) {
            reportwin_append_number(StringTemp, maintenance_total / 2);
            if ((maintenance_total & 1) != 0) strcat(StringTemp, ".5");
        } else {
            reportwin_append_number(StringTemp, maintenance_total);
        }
        strcat(StringTemp, "}");
        listBox_.item(StringTemp, 99999999);
    }

    listBox_.reset_report_selection();
    dialog.field_B4_ = 0;
    listBox_.attach(this, dialog.window_x_, dialog.window_y_, 0x81020);
    listBox_.set_selected_id(99999999);
    static_cast<GraphicWin &>(listBox_).show(0);
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
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0049d540.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_ops() {
    Buffer &canvas = *static_cast<Buffer *>(this);
    const int faction = ReportIfGlobal.report_faction_id_;
    Random rng;
    rng.reseed(timeGetTime());
    field_59A0_ = field_59A4_ = 40;
    const int split = energy_panel_rect_.top + 34 + field_E268_;
    RECT upper = {energy_panel_rect_.left, energy_panel_rect_.top,
        energy_panel_rect_.right, split};
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
        energy_panel_rect_.right - 3, energy_panel_rect_.top + field_E268_ + 9};
    RECT lower = {energy_panel_rect_.left, split + 2,
        energy_panel_rect_.right, energy_panel_rect_.bottom};
    RECT icon = {energy_panel_rect_.left + 3, split + 5,
        energy_panel_rect_.left + 43, split + 45};
    RECT text = {energy_panel_rect_.left + 45, split + 5,
        energy_panel_rect_.right - 3, split + 45};
    canvas.box_sprite(&upper, &InterfaceBoxStyles[0]);
    canvas.box_sprite(&title, &InterfaceBoxStyles[1]);
    canvas.box_sprite(&lower, &InterfaceBoxStyles[0]);
    const int pitch = field_59A4_ + 2;
    const int rows = (lower.bottom - lower.top - 6) / pitch;
    flatButton4_.show(0); flatButton5_.show(0); flatButton6_.show(0);
    const int count = PlayersData[faction].current_num_bases;
    if (!field_C0F8_) {
        if (rows <= count) field_E26C_ = 1;
    } else if (count < rows) {
        scroll_hide(this, &scroll_);
        field_C0F8_ = 0;
    }
    if (field_E270_) {
        field_C108_ = rows - 1;
        field_C0FC_ = field_C100_ = 0;
        field_C104_ = count - 1;
        scroll_.set_range(0, count - 1);
        scroll_.paging_ = field_C108_;
        scroll_.set_pos(field_C0FC_);
        field_E270_ = 0;
    }
    spot_.kill_type(0);
    spot_.reset_count();
    strcpy(StringTemp, "[");
    strcat(StringTemp, get_adjective(faction));
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xACC / 4));
    if (field_5A08_) {
        strcat(StringTemp, " - ");
        const int labels[8] = {0xA0, 0xA4, 0xE4, 0xAAC, 0xAB0, 0xAB4, 0xFC, 0xAB8};
        if (field_5A08_ >= 1 && field_5A08_ <= 8)
            strcat(StringTemp, label_get(labels[field_5A08_ - 1] / 4));
    }
    CharUpperA(StringTemp);
    strcat(StringTemp, "]");
    --title.top; --title.bottom;
    canvas.write_cent_l(StringTemp, &title, strlen(StringTemp));
    int ordinal = -1;
    // Original 49DF66/49E0CB assign this only for facilities; 49E0C4 reads
    // it on the unit path too. Preserve the stale/uninitialized value.
    int facility_scratch;
    for (int list_index = 0; list_index < GameDataStateStorage.fields_.base_current_count_; ++list_index) {
        const int base_id = ReportIfGlobal.sort_entries_[list_index].base_id;
        Base &base = Bases[base_id];
        if (base.faction_id_current != faction) continue;
        ++ordinal;
        if (ordinal < static_cast<int>(field_C0FC_) || ordinal >= static_cast<int>(field_C0FC_) + rows) continue;
        if (ordinal > static_cast<int>(field_C0FC_)) {
            text.top += pitch; text.bottom += pitch;
            icon.top += pitch; icon.bottom += pitch;
        }
        report_work_rect_.left = icon.left;
        report_work_rect_.top = text.top;
        report_work_rect_.right = text.right + 1;
        report_work_rect_.bottom = text.bottom;
        spot_.add(base_id + 0x400, 0, &report_work_rect_);
        spot_.add(base_id + 0xC00, 0, &icon);
        const bool selected = (selected_bases_[base_id / 32] & (1u << (base_id & 31))) != 0;
        canvas.box_sprite(&text, &InterfaceBoxStyles[selected ? 2 : 1]);
        if (selected) canvas.box_sprite(&text, &InterfaceBoxStyles[5]);
        canvas.box_sprite(&icon, &InterfaceBoxStyles[selected ? 2 : 1]);
        if (selected) canvas.box_sprite(&icon, &InterfaceBoxStyles[5]);
        if (base.governor & 0x80000000u) {
            for (int priority = 0; priority < 4; ++priority) {
                if (base.governor & (0x1000000u << priority)) {
                    RECT inner = {icon.left + 1, icon.top + 1, icon.right - 1, icon.bottom - 1};
                    canvas.box_sprite(&inner, &InterfaceBoxStyles[36 + 3 * priority]);
                    break;
                }
            }
        }
        const int text_y = pitch / 2 - static_cast<int>(field_E268_) / 2 + text.top;
        const int base_width = MapWinCaptionScale < 1 ? 62 : MapWinCaptionScale * 100 / MapWinCaptionDivisor;
        const int base_height = MapWinCaptionScale < 1 ? 46 : MapWinCaptionScale * 75 / MapWinCaptionDivisor;
        rect1_.left = (static_cast<int>(field_59A0_) - base_width) / 2 + 1 + icon.left;
        rect1_.top = (static_cast<int>(field_59A4_) - base_height) / 2 + 1 + icon.top;
        base_draw(&canvas, base_id, rect1_.left, rect1_.top, -6, 0x40);
        canvas.set_text_color(0xE1, -1, 1, 1);
        canvas.set_font(ReportIfGlobal.field_1E18C_, ReportIfGlobal.field_1E184_, ReportIfGlobal.field_1E18C_, 0);
        set_base(base_id);
        base_compute(0);
        StringTemp[0] = 0;
        say_base(StringTemp, base_id);
        const int matrix = base_project(FAC_TELEPATHIC_MATRIX);
        if (BaseCurrent->talent_total < BaseCurrent->drone_total
            && (matrix < 0 || Bases[matrix].faction_id_current != faction)) {
            canvas.set_text_color(0xF9, -1, 1, 1);
        } else if (!BaseCurrent->drone_total && BaseCurrent->population_size > 2
            && (BaseCurrent->population_size + 1) / 2 <= BaseCurrent->talent_total) {
            strcat(StringTemp, "(!)");
            canvas.set_text_color(0xFF, -1, 1, 1);
        }
        rect1_.left = text.left + 5;
        CharUpperA(StringTemp);
        canvas.write_l(StringTemp, rect1_.left, text.top + 5, strlen(StringTemp));
        StringTemp[0] = 0;
        if (is_objective(base_id) && !(GameDataStateStorage.fields_.game_rules_ & 0x20000000)) {
            // The original indexes colour by list position, not sorted base ID.
            canvas.set_text_color(InfoFactionTextColors[Bases[list_index].faction_id_current], -1, 1, 1);
            strcat(StringTemp, label_get(0xE08 / 4));
            CharUpperA(StringTemp);
        } else if (!(base.governor & 0x80000000u)) {
            canvas.set_text_color(0xE1, -1, 1, 1);
            strcat(StringTemp, label_get(0xBFC / 4));
        } else {
            for (int status_priority = 0; status_priority < 4; ++status_priority) {
                if (base.governor & (0x1000000u << status_priority)) {
                    canvas.set_text_color(mandate_color(status_priority), -1, 1, 1);
                    strcat(StringTemp, label_get(0x824 / 4 + status_priority));
                    break;
                }
            }
        }
        canvas.write_l(StringTemp, rect1_.left + 20, text.bottom - field_E268_ - 1, strlen(StringTemp));
        rect1_.left += 192;
        const int mode = buttonGroup1_.field_84_;
        if (mode == 0x8B3) {
            const int production = BaseCurrent->queue_production_id[0];
            if (production < 0) {
                const int facility = -production;
                facility_scratch = facility;
                Sprite &sprite = facility < 70 ? g_FACILITY_ICON_SPRITES[facility]
                    : g_SECRET_PROJECT_ICON_SPRITES[facility - 70];
                const int denominator = facility < 70 ? g_FACILITY_ICON_SPRITES[0].height()
                    : g_SECRET_PROJECT_ICON_SPRITES[0].height();
                const int color = mandate_color(facility < 70 ? TutWinBaseWindow.prodPicker_.edbc_fac(facility) : 1);
                sprite.draw_mono(&canvas, sprite.transparent_index(), rect1_.left + 8,
                    rect1_.top + 4, color, text.bottom - text.top - 2, denominator);
            } else {
                veh_draw(&canvas, veh_fake(production, BaseCurrent->faction_id_current),
                    rect1_.left, rect1_.top + 4, -8, 0x4C, 0);
            }
            StringTemp[0] = 0;
            canvas.set_font(ReportIfGlobal.field_1E18C_, ReportIfGlobal.field_1E18C_, ReportIfGlobal.field_1E18C_, 0);
            int item_cost;
            if (production < 0) {
                const int facility = -production;
                item_cost = Facility[facility].cost;
                facility_scratch = facility;
                strcat(StringTemp, StringTable.get(Facility[facility].name));
                canvas.set_text_color(mandate_color(TutWinBaseWindow.prodPicker_.edbc_fac(facility)), -1, 1, 1);
            } else {
                strcat(StringTemp, VehPrototypes[production].veh_name);
                BOOL prototype_cost;
                item_cost = veh_cost(production, BaseIDCurrentSelected, &prototype_cost);
                canvas.set_text_color(mandate_color(TutWinBaseWindow.prodPicker_.edbc_unit(production)), -1, 1, 1);
            }
            rect1_.left += 60;
            canvas.write_l(StringTemp, rect1_.left, text_y, strlen(StringTemp));
            canvas.set_text_color(0xF2, -1, 1, 1);
            strcpy(StringTemp, "(");
            reportwin_append_number(StringTemp, facility_scratch == 69 ? 0 : BaseCurrent->minerals_accumulated);
            strcat(StringTemp, "/");
            const int cost = cost_factor(BaseCurrent->faction_id_current, 1, -1) * item_cost;
            reportwin_append_number(StringTemp, cost);
            strcat(StringTemp, ")    ");
            const int minerals = BaseCurrent->mineral_surplus_final;
            if (minerals < 1) strcat(StringTemp, label_get(0x10C / 4));
            else {
                strcat(StringTemp, label_get(0xC4 / 4)); strcat(StringTemp, ": "); strcat(StringTemp, " ");
                int turns = 0;
                if (cost > BaseCurrent->minerals_accumulated) {
                    const int divisor = minerals < 1 ? 1 : minerals > 99999 ? 99999 : minerals;
                    turns = (minerals - 1 + cost - BaseCurrent->minerals_accumulated) / divisor;
                    if (turns < 0) turns = 0; else if (turns > 999999) turns = 999999;
                }
                reportwin_append_number(StringTemp, turns);
            }
            canvas.write_right_l(StringTemp, text.right - 4, text_y, strlen(StringTemp));
        } else if (mode == 0x8B4) {
            const int population = BaseCurrent->population_size;
            int spacing = population < 2 ? 34 : (text.right - text.left - 226) / (population - 1);
            if (spacing < 1) spacing = 1; else if (spacing > 34) spacing = 34;
            rect1_.left = spacing - 32 + text.right;
            rect1_.top = text.bottom - pitch / 2 - 18;
            const bool alien = ExpansionEnabled && (Players[faction].rule_flags & RFLAG_ALIEN);
            for (int specialist = 0; specialist < BaseCurrent->specialist_total; ++specialist) {
                rect1_.left -= spacing;
                spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                const int index = BaseCurrent->specialist_total - specialist - 1;
                const int type = index < 16 ? (BaseCurrent->spec[index >> 3] >> ((index & 7) * 4)) & 15 : best_specialist();
                Sprite &sprite = alien ? g_AL_SPECIALIST_SM_CURSOR_SPRITES[type] : g_SPECIALIST_SM_CURSOR_SPRITES[type];
                sprite.draw(&canvas, sprite.transparent_index(), rect1_.left, rect1_.top);
            }
            for (int drone = 0; drone < BaseCurrent->drone_total; ++drone) {
                rect1_.left -= spacing;
                spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                const int type = 2 + (drone < BaseCurrent->super_drone_total) + (alien ? 0 : rng.get(0, 2) * 4);
                Sprite &sprite = alien ? g_AL_CITIZEN_SM_CURSOR_SPRITES[type] : g_CITIZEN_SM_CURSOR_SPRITES[type];
                sprite.draw(&canvas, sprite.transparent_index(), rect1_.left, rect1_.top);
            }
            const int workers = population - BaseCurrent->talent_total - BaseCurrent->specialist_total
                - (BaseCurrent->drone_total > 0 ? BaseCurrent->drone_total : 0);
            for (int worker = 0; worker < workers; ++worker) {
                rect1_.left -= spacing;
                spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                const int type = 1 + (alien ? 0 : rng.get(0, 2) * 4);
                Sprite &sprite = alien ? g_AL_CITIZEN_SM_CURSOR_SPRITES[type] : g_CITIZEN_SM_CURSOR_SPRITES[type];
                sprite.draw(&canvas, sprite.transparent_index(), rect1_.left, rect1_.top);
            }
            for (int talent = 0; talent < BaseCurrent->talent_total; ++talent) {
                rect1_.left -= spacing;
                spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                const int type = alien ? 0 : rng.get(0, 2) * 4;
                Sprite &sprite = alien ? g_AL_CITIZEN_SM_CURSOR_SPRITES[type] : g_CITIZEN_SM_CURSOR_SPRITES[type];
                sprite.draw(&canvas, sprite.transparent_index(), rect1_.left, rect1_.top);
            }
        } else if (mode == 0x8B5) {
            int vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);
            const int slots = stack_check(vehicle, 1, -1, -1, -1);
            int spacing = slots < 2 ? 40 : (text.right - text.left - 232) / (slots - 1);
            if (spacing < 1) spacing = 1; else if (spacing > 40) spacing = 40;
            rect1_.left = spacing - 45 + text.right;
            rect1_.top = text.top;
            if (vehicle >= 0) {
                while (Vehs[vehicle].prev_veh_id_stack >= 0) vehicle = Vehs[vehicle].prev_veh_id_stack;
                for (; vehicle >= 0; vehicle = Vehs[vehicle].next_veh_id_stack) {
                    rect1_.left -= spacing;
                    veh_draw(&canvas, vehicle, rect1_.left, rect1_.top, -8, 8, 0);
                    spot_.add(base_id + 0x800, vehicle, rect1_.left, rect1_.top, spacing, pitch);
                }
            }
        }
    }
}

// ORIGINAL: 0x0049EB10 ?draw_projects@ReportWin@@QAEXXZ 0x0049EB10-0x0049F343
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x0049EB10-0x0049F343", "src/unrecovered/0049eb10.cpp:retained seed and trial history", "Original97CDDC=PlayersData96C9E0+7*20CC+goals19FC+70*10+base_idC; the native unsigned-byte read determines the destroyed-project faction without dereferencing Bases[-2]."], "reason": "Recover the projects page through owned ReportWin rectangles, spots and scroll state, preserving queued/completed rendering and the destroyed-owner byte alias through its real PlayerData member."}
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/unrecovered/0049eb10.cpp
// TRIED: full transcription (call-argument order verified against raw disasm for add/box_sprite/draw_mono/write_l/write_cent_l/ set_text_color/Strings::get, all matched Ghidra) reaches MISMATCH #3 'mov' vs 'push' - a prologue-level register/stack allocation divergence, not a logic error; not chased further given function size (2099 bytes, nested loops with gotos).
void ReportWin::draw_projects() {
    Buffer &canvas = *static_cast<Buffer *>(this);
    const int viewer = ReportIfGlobal.report_faction_id_;
    RECT frame = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3, energy_panel_rect_.bottom - 3};
    const int pitch = static_cast<int>(field_E268_) + 9;
    const int row_pitch = 54;
    const int content_top = energy_panel_rect_.top + pitch;
    const int rows = (energy_panel_rect_.bottom - content_top - 5) / row_pitch;
    const int panel_width = frame.right - frame.left;
    // Native width is (panel.right - panel.left - 6) / 2 - 1. `frame`
    // already has the three-pixel inset on each side, so do not inset again.
    field_59A0_ = panel_width / 2 - 1;
    field_59A4_ = row_pitch;
    spot_.kill_type(0);
    // The original follows kill_type with an inlined add_count_ = 0 store
    // at ReportWin+0x599C before any drawing or scroll recursion.
    spot_.reset_count();
    canvas.box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    canvas.box_sprite(&frame, &InterfaceBoxStyles[1]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[viewer].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    // The original obtains this title through Strings::get using the entry
    // at StringTable + 0xAD0, rather than through a numeric label key.
    strcat(StringTemp, label_get(0xAD0 / 4));
    strcat(StringTemp, "]");
    RECT heading = {frame.left, frame.top - 1, frame.right, content_top - 1};
    canvas.write_cent_l(StringTemp, &heading, strlen(StringTemp));

    // The image lays the entries out in three passes: projects currently in
    // production by the viewed faction, production by other factions, and
    // projects already present in the completed-project owner table.
    int entry_count = -1;
    for (int project = 0; project < MaxSecretProjectNum; ++project) {
        if (base_project(project) != SP_Unbuilt)
            ++entry_count;
    }
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id)
        if (Bases[base_id].queue_production_id[0] <= -FacilitySPStart)
            ++entry_count;
    if (!field_C0F8_) {
        if (entry_count >= rows * 2) field_E26C_ = 1;
    } else if (entry_count < rows * 2) {
        scroll_hide(this, &scroll_);
        field_C0F8_ = 0;
    }
    if (field_E270_) {
        field_C108_ = rows - 1;
        field_C0FC_ = field_C100_ = 0;
        field_C104_ = entry_count / 2;
        scroll_.set_range(0, field_C104_);
        scroll_.paging_ = field_C108_;
        scroll_.set_pos(field_C0FC_);
        field_E270_ = 0;
        draw_projects();
    }
    int logical_row = 0;
    for (int mode = 0; mode < 3; ++mode) {
        for (int project = 0; project < MaxSecretProjectNum; ++project) {
            const int facility = FacilitySPStart + project;
            const int owner_base = base_project(project);
            if (mode < 2 && owner_base != SP_Unbuilt) continue;
            if (mode == 2 && owner_base == SP_Unbuilt) continue;

            int base_id = owner_base;
            int faction = -1;
            if (mode < 2) {
                for (int candidate = 0; candidate < GameDataStateStorage.fields_.base_current_count_; ++candidate) {
                    const Base &base = Bases[candidate];
                    const bool same_faction = base.faction_id_current == viewer;
                    if ((mode == 0) != same_faction) continue;
                    if (base.queue_production_id[0] != -facility) continue;
                    base_id = candidate;
                    faction = base.faction_id_current;

                    const int ordinal = logical_row++;
                    const int visible_row = ordinal / 2
                        - static_cast<int>(field_C0FC_);
                    if (visible_row < 0 || visible_row >= rows) continue;
                    const int column = ordinal & 1;
                    const int card_left = frame.left
                        + column * (static_cast<int>(field_59A0_) + 2);
                    const int card_top = content_top + visible_row * (row_pitch + 2) + 2;
                    RECT item = {card_left, card_top,
                                 card_left + static_cast<int>(field_59A0_),
                                 card_top + row_pitch};
                    rect1_.left = item.left;
                    rect1_.top = item.top;
                    report_work_rect_ = item;
                    const int selection = base_id + 0x400;
                    spot_.add(selection, 0, &report_work_rect_);
                    const bool selected = field_59AC_ == selection;
                    canvas.box_sprite(&report_work_rect_, &InterfaceBoxStyles[selected ? 2 : 1]);
                    if (selected) canvas.box_sprite(&report_work_rect_, &InterfaceBoxStyles[5]);

                    Sprite &project_icon = g_SECRET_PROJECT_ICON_SPRITES[project];
                    project_icon.draw_mono(&canvas, project_icon.transparent_index(),
                        rect1_.left + 2, rect1_.top + 1, mandate_color(1),
                        row_pitch - 2, g_SECRET_PROJECT_ICON_SPRITES[0].height());
                    StringTemp[0] = 0;
                    strcat(StringTemp, StringTable.get(Facility[facility].name));
                    canvas.set_text_color(ChatItemIdTable[faction], -1, 1, 1);
                    canvas.set_text_color3(ChatItemIdTable[faction], -1, 1, 1);
                    rect1_.top = item.top + 5;
                    canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top,
                                   strlen(StringTemp));
                    rect1_.top += field_E268_;
                    StringTemp[0] = 0;
                    if (mode == 1) strcat(StringTemp, "[");
                    if (mode == 0 || spying(faction)) {
                        say_base(StringTemp, base_id);
                    } else {
                        GenderDefault = Players[faction].noun_gender;
                        PluralityDefault = Players[faction].is_noun_plural;
                        strcat(StringTemp, Players[faction].noun_faction);
                    }
                    if (mode == 1) strcat(StringTemp, "]");
                    canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top,
                        strlen(StringTemp));
                    rect1_.top += field_E268_;
                    StringTemp[0] = 0;
                    if (mode == 1) strcat(StringTemp, "[");
                    strcat(StringTemp, label_get(0x6BC / 4));
                    if (mode == 1) strcat(StringTemp, "]");
                    canvas.set_text_color(0xF9, -1, 1, 1);
                    canvas.set_text_color3(0xF9, -1, 1, 1);
                    canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top,
                        strlen(StringTemp));
                }
            } else {
                if (base_id == SP_Destroyed) {
                    // The original byte load at 0x0097CDDC aliases
                    // PlayersData[7].goals[70].base_id. Preserve its low-byte
                    // faction value and the subsequent native table indexing.
                    faction = static_cast<uint8_t>(PlayersData[7].goals[70].base_id);
                } else {
                    faction = Bases[base_id].faction_id_current;
                }
            }
            if (mode < 2) continue;

            const int ordinal = logical_row++;
            const int scroll_row = ordinal / 2
                - static_cast<int>(field_C0FC_);
            if (scroll_row < 0 || scroll_row >= rows) continue;
            const int card_left = frame.left
                + (ordinal & 1) * (static_cast<int>(field_59A0_) + 2);
            const int card_top = content_top + scroll_row * (row_pitch + 2) + 2;
            RECT item = {card_left, card_top,
                         card_left + static_cast<int>(field_59A0_),
                         card_top + row_pitch};
            rect1_.left = item.left;
            rect1_.top = item.top;
            report_work_rect_ = item;
            const int selection = mode == 2 ? project : base_id + 0x400;
            spot_.add(selection, 0, &report_work_rect_);
            const bool selected = field_59AC_ == selection;
            canvas.box_sprite(&report_work_rect_, &InterfaceBoxStyles[selected ? 2 : 1]);
            if (selected) canvas.box_sprite(&report_work_rect_, &InterfaceBoxStyles[5]);

            g_SECRET_PROJECT_ICON_SPRITES[project].draw_mono(
                &canvas, g_SECRET_PROJECT_ICON_SPRITES[project].transparent_index(),
                rect1_.left + 2, rect1_.top + 1, mandate_color(1),
                row_pitch - 2, g_SECRET_PROJECT_ICON_SPRITES[0].height());
            StringTemp[0] = 0;
            strcat(StringTemp, "[");
            strcat(StringTemp, StringTable.get(Facility[facility].name));
            strcat(StringTemp, "]");
            canvas.set_text_color(ChatItemIdTable[faction], -1, 1, 1);
            canvas.set_text_color3(ChatItemIdTable[faction], -1, 1, 1);
            rect1_.top = item.top + 5;
            canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top,
                           strlen(StringTemp));
            rect1_.top += field_E268_;
            StringTemp[0] = 0;
            strcat(StringTemp, "[");
            say_base(StringTemp, base_id);
            strcat(StringTemp, "]");
            canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top, strlen(StringTemp));
            rect1_.top += field_E268_;
            StringTemp[0] = 0;
            strcat(StringTemp, "[");
            GenderDefault = Players[faction].noun_gender;
            PluralityDefault = Players[faction].is_noun_plural;
            strcat(StringTemp, Players[faction].noun_faction);
            strcat(StringTemp, "]");
            canvas.set_text_color(ChatItemIdTable[faction], -1, 1, 1);
            canvas.set_text_color3(ChatItemIdTable[faction], -1, 1, 1);
            canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top, strlen(StringTemp));
        }
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x0049F350 0x0049FE20 0x0049FE34 0x0049F758
// ORIGINAL: 0x0049F350 ?draw_sat@ReportWin@@QAEXXZ 0x0049F350-0x0049FE1E
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0049f350.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_sat() {
    Buffer &canvas = *static_cast<Buffer *>(this);
    const int faction = ReportIfGlobal.report_faction_id();
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
        energy_panel_rect_.right - 3, energy_panel_rect_.top + static_cast<int>(field_E268_) + 9};
    canvas.box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    canvas.box_sprite(&title, &InterfaceBoxStyles[1]);
    strcpy(StringTemp, "[");
    strcat(StringTemp, Players[faction].formal_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xAD4 / 4));
    strcat(StringTemp, "]");
    --title.top; --title.bottom;
    canvas.write_cent_l(StringTemp, &title, strlen(StringTemp));
    flatButton7_.show(0);
    flatButton8_.show(0);
    spot_.reset_count();
    bool orbital_tech = false;
    for (int technology_faction = 1; technology_faction < 8; ++technology_faction)
        if (has_tech(Facility[65].preq_tech, technology_faction)) orbital_tech = true;
    if (buttonGroup2_.field_84_ == 0xD3C) {
        flatButton9_.hide();
        if (!orbital_tech) {
            canvas.set_font(ReportIfGlobal.field_1E1A0_, 0, 0, 0);
            canvas.set_text_color(0xE1, -1, 1, 1);
            strcpy(StringTemp, label_get(0xCC4 / 4));
            canvas.write_cent_l(StringTemp, rect1_.left, rect1_.top,
                rect1_.right, rect1_.bottom, strlen(StringTemp));
            return;
        }
        rect1_.top += 50;
        rect1_.bottom -= 50;
        const int column_width = (rect1_.bottom - rect1_.top - 2) / 8;
        const int heading_width = rect1_.right / (ExpansionEnabled ? 5 : 3);
        const int row_width = (rect1_.right - 4 - heading_width) / (ExpansionEnabled ? 5 : 4);
        for (int other = 0; other < 8; ++other) {
            // Genuine original: clears only type 0 on every column iteration.
            spot_.kill_type(0);
            rect1_.left = energy_panel_rect_.left + 2;
            if (other) {
                StringTemp[0] = 0;
                GenderDefault = Players[other].noun_gender;
                PluralityDefault = Players[other].is_noun_plural;
                strcat(StringTemp, Players[other].noun_faction);
                canvas.set_text_color(ChatItemIdTable[other], ChatShadowColors[other], 1, 1);
                report_work_rect_.left = rect1_.left;
                report_work_rect_.top = rect1_.top;
                report_work_rect_.right = rect1_.left + heading_width;
                report_work_rect_.bottom = rect1_.top + column_width;
                canvas.write_l(StringTemp, &report_work_rect_, strlen(StringTemp));
            }
            rect1_.left += heading_width;
            report_work_rect_.left = rect1_.left;
            report_work_rect_.top = rect1_.top;
            report_work_rect_.right = rect1_.left + row_width;
            report_work_rect_.bottom = rect1_.top + column_width;
            satellite_index_ = 0;
            while (satellite_index_ < (ExpansionEnabled ? 5 : 4)) {
                if (!other) {
                    canvas.set_text_color(0xFF, -1, 1, 1);
                    StringTemp[0] = 0;
                    if (ExpansionEnabled && satellite_index_ == 4) {
                        strcat(StringTemp, StringTable.get(Facility[41].name));
                    } else {
                        const int facility_index = satellite_index_;
                        strcat(StringTemp, StringTable.get(Facility[65 + facility_index].name));
                    }
                    canvas.wrap_cent(StringTemp, &report_work_rect_);
                } else {
                    int elevators = 0;
                    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base)
                        if (Bases[base].faction_id_current == other && has_fac(41, base, 0)) ++elevators;
                    const int color_kind = satellite_index_;
                    bool nonzero = false;
                    if (color_kind < 4) {
                        nonzero = PlayersData[other].satellites_[color_kind] != 0;
                    } else if (color_kind == 4) {
                        nonzero = elevators != 0;
                    }
                    const int colors[5] = {0x98, 0xDF, 0x62, 0xF9, 0xFF};
                    switch (color_kind) {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                            canvas.set_text_color(nonzero ? colors[color_kind] : 0xF8, -1, 1, 1);
                            break;
                        default:
                            // The native switch default bypasses color setting when
                            // the signed less-than-four satellite value is nonzero.
                            if (!nonzero) canvas.set_text_color(0xF8, -1, 1, 1);
                            break;
                    }
                    StringTemp[0] = 0;
                    const int formatted_kind = satellite_index_;
                    if (formatted_kind == 3) {
                        report_sat_number(PlayersData[other].satellites_[3] - PlayersData[other].sat_odp_deployed);
                        strcat(StringTemp, "/");
                    }
                    const int amount_kind = satellite_index_;
                    report_sat_number(amount_kind == 4
                        ? elevators : PlayersData[other].satellites_[amount_kind]);
                    canvas.write_cent_l(StringTemp, &report_work_rect_, strlen(StringTemp));
                    spot_.add(satellite_index_, other, report_work_rect_.left, report_work_rect_.top,
                        report_work_rect_.right - report_work_rect_.left,
                        report_work_rect_.bottom - report_work_rect_.top);
                }
                report_work_rect_.left += row_width;
                report_work_rect_.right += row_width;
                ++satellite_index_;
            }
            rect1_.top += column_width;
        }
        return;
    }
    field_59A0_ = 576;
    field_59A4_ = 210;
    rect1_.left = (energy_panel_rect_.right + energy_panel_rect_.left - 576) / 2;
    rect1_.top = (energy_panel_rect_.bottom + energy_panel_rect_.top - 210) / 2 + 20;
    satellite_rect_.left = rect1_.left;
    satellite_rect_.top = rect1_.top;
    satellite_rect_.right = rect1_.left + 576;
    satellite_rect_.bottom = rect1_.top + 210;
    flic_.init(&canvas, "satellite.flc", 0, 0, 0);
    field_E2A8_ = (satellite_rect_.right + satellite_rect_.left) / 2;
    field_E2AC_ = (satellite_rect_.bottom + satellite_rect_.top) / 2;
    field_E2A0_ = field_E2A8_ - flic_.decoded_width() / 2;
    field_E2A4_ = field_E2AC_ - flic_.decoded_height() / 2;
    buffer2_.init(flic_.decoded_width(), flic_.decoded_height(), 0, 0);
    buffer2_.fill(9);
    flic_.init(&buffer2_, "satellite.flc", 0, 0, 0);
    canvas.box_sprite(&satellite_rect_, &InterfaceBoxStyles[4]);
    satellite_rect_.left += 3; satellite_rect_.top += 3;
    satellite_rect_.right -= 3; satellite_rect_.bottom -= 3;
    field_F918_ = 0;
    // Original includes factions 1..7 (833..4198 dwords, exclusive endpoint).
    for (int other = 1; other < 8; ++other) {
        satellite_index_ = 0;
        while (satellite_index_ < 4) {
            field_F918_ += report_sat_count(PlayersData[other], satellite_index_);
            ++satellite_index_;
        }
    }
    field_EE30_ = 0.02f;
    satellite_index_ = 0;
    while (satellite_index_ < 16) {
        const int i = satellite_index_;
        const int radius_x = static_cast<int>(field_59A4_) / 2 - 10;
        const int radius_y = radius_x;
        satellite_acceleration_[i] = static_cast<float>(sqrt(static_cast<double>(radius_x * radius_x * 2)));
        const float radius = satellite_acceleration_[i];
        // x87 49FC58..64 multiplies THREE radius factors, unlike seed.
        satellite_acceleration_[i] = static_cast<float>(sqrt(1.0 / (static_cast<double>(radius) * radius * radius)) * 700.0f);
        const float angle = static_cast<float>(i) * 0.0625f;
        const float phase_x = angle * 6.28000020980835f;
        const float phase_y = static_cast<float>((((i & 1) ? 0.125 : -0.125) + angle) * 6.28000020980835f);
        const float speed = satellite_acceleration_[i];
        satellite_x_[i] = static_cast<float>(cos(static_cast<double>(speed) + phase_x) * radius_x);
        satellite_y_[i] = static_cast<float>(sin(static_cast<double>(speed) + phase_y) * radius_y);
        satellite_dx_[i] = static_cast<float>(sin(static_cast<double>(speed) + phase_x) * -radius_x * speed);
        satellite_dy_[i] = static_cast<float>(cos(static_cast<double>(speed) + phase_y) * speed * radius_y);
        satellite_acceleration_[i] = static_cast<float>(static_cast<double>(speed) * field_EE30_ * speed);
        satellite_live_[i] = i < static_cast<int>(field_F918_);
        ++satellite_index_;
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
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:a538e0a3:src/recovered/units/0049ffd0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::start_attack(int attacker, int target, int type, int base_id) {
    field_F924_ = attacker;
    field_F928_ = target;
    field_F92C_ = type;
    field_F930_ = base_id;
    field_F920_ = 0;
    field_F91C_ = 0;
    field_F934_ = 0;
    if (field_F918_ != 0) {
        const int counter = static_cast<int>(--field_F918_);
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

// ORIGINAL: 0x004A0100 ?exp_anim@ReportWin@@QAEXXZ 0x004A0100-0x004A0248
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:a538e0a3:src/recovered/units/004a0100.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
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
            Buffer::restore_clip();
            return;
        }
    } else {
        buffer1_.draw(static_cast<Buffer *>(this), 9,
                      explosion_rect_.left, explosion_rect_.top);
        Sprite &frame = g_XI_BOOM_VEH_SPRITES[48 + field_F934_];
        frame.draw_multi_table_dest(static_cast<Buffer *>(this),
            frame.transparent_index(),
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

// ORIGINAL: 0x004A0260 ?sat_attack@@YAXHHH@Z 0x004A0260-0x004A080C;0x00659447-0x00659580
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/004a0260.cpp", "source:src/faction.h", "image:complete original instructions and independent storage/lifecycle evidence"], "reason": "Recover satellite attack selection through a natural Popup lifetime, its owned picture and the genuine four-counter faction array, preserving original signed guards and network/local action ordering."}
void __cdecl sat_attack(int attacker, int target, int kind) {
    Popup popup;
    if (attacker == target || target < 1) return;
    if (!PlayersData[attacker].satellites_[3]) {
        if (attacker == ConsoleGlobal.faction_id_)
            popp(ScriptPopupFile,
                 "NOKILLERS", 0, "space_sm.pcx", 0);
        return;
    }
    if (static_cast<int>(PlayersData[attacker].sat_odp_deployed)
        >= static_cast<int>(PlayersData[attacker].satellites_[3])) {
        if (attacker == ConsoleGlobal.faction_id_)
            popp(ScriptPopupFile,
                 "KILLERSDEPLOYED", 0, "space_sm.pcx", 0);
        return;
    }
    int targets = 0;
    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        if (Bases[base].faction_id_current == target && has_fac_built(41, base)) ++targets;
    }
    if (kind == 4) {
        if (!targets) return;
    } else {
        // Original guards only the signed upper bound; no new lower guard.
        if (kind >= 4 || !PlayersData[target].satellites_[kind]) return;
    }
    if (break_treaty(attacker, target, 7)) return;
    if (diplo_lock(100)) return;
    if (PlayersData[attacker].satellites_[3]
        && static_cast<int>(PlayersData[attacker].sat_odp_deployed)
            < static_cast<int>(PlayersData[attacker].satellites_[3])) {
        int attack_target = attacker;
        bool attack = false;
        if (kind == 4) {
            if (targets) {
                popup.start(PopupStartCaption, "WHICHGSP", -1, 0, 0x40, 0);
                int width, height;
                if (Buffer::get_pcx_dimensions("satbat_sm.pcx", &width, &height) == 0
                    && popup.owned_picture().init("satbat_sm.pcx", width, height) == 0)
                    popup.select_owned_picture();
                for (int item_base = 0; item_base < GameDataStateStorage.fields_.base_current_count_; ++item_base) {
                    if (Bases[item_base].faction_id_current == target && has_fac_built(41, item_base))
                        popup.dialogs().item(Bases[item_base].name_string, item_base);
                }
                attack_target = popup.exec(0, 0);
                attack = attack_target >= 0;
            }
        } else if (kind < 4 && PlayersData[target].satellites_[kind]) {
            attack = true;
        }
        if (attack) {
            if (IsMultiplayerNet) {
                message_data(0x2412, 0, attacker, target, kind, attack_target);
                NetDaemonNet.await_exec(0);
            } else {
                ReportWinGlobal.start_attack(attacker, target, kind, attack_target);
            }
        }
    }
    diplo_unlock();
    // Popup's genuine destructor closes and tears down all member/base state.
}

/*
Purpose: Step one faction's satellite count for a body, with the shift-key
         downward path clamped and the seen map refreshed.
// ORIGINAL: 0x004A0810 ?sat_edit@@YAXHHH@Z 0x004A0810-0x004A08D2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4A0810..4A08D2; retired seed git:117282ee:src/unrecovered/004a0810.cpp retains the same identity/spans.", "Original accesses PlayerData+808 with a word index, +814 and +364. Source satellites_[4] begins808, so814 is satellites_[3]; compiler-observed PlayerData member sat_odp_deployed is364. The shift path clamps deployed orbital-defense pods using the signed result of wrapped dword subtraction, not an unsigned counter comparison.", "GetAsyncKeyState high byte is read only after type<4 and nonzero body guards; preserve negative-index admission, counter wrap, nonzero decrement, both callback paths and direct see_map_check/GraphicWin::redraw ordering. No bounds or behavior fix."], "reason": "Recover the satellite editor through existing PlayerData satellite/deployed members while preserving original arithmetic, indexing and callback behavior."}
// SUPERSEDED_BINDING_HISTORY: v1 {"evidence": ["Original4A0810..4A08D2; retired seed git:117282ee:src/unrecovered/004a0810.cpp records the same identity and spans.", "The three per-faction fields are PlayerData (faction.h, the 0x833-dword stride the header pins) at +0x808 (the satellite count stepped by body), +0x814 (the seen-map target) and +0x364 (the seen-map floor); the shift state comes through the USER32 GetAsyncKeyState import slot 0x00669330 that mapwin.cpp binds the same way.", "The repaint is see_map_check (faction.h's own) and redraw on the source-owned ReportWinGlobal (reportwin.h, original 0x00876478) through its inherited virtual slot."], "reason": "Coverage transcription of the satellite editor click with the PlayerData offsets documented and the repaint through the tree's own names."}
Status: Coverage transcription - unclaimed body
*/

void __cdecl sat_edit(int report_faction_id, int target_faction_id, int satellite_type) {
    if (satellite_type < 4 && report_faction_id != 0) {
        const unsigned char shift_high = static_cast<unsigned char>(
            ::GetAsyncKeyState(0x10) >> 8);
        if (shift_high == 0) {
            ++PlayersData[target_faction_id].satellites_[satellite_type];
        } else {
            uint32_t *const field = &PlayersData[target_faction_id].satellites_[satellite_type];
            if (*field != 0) {
                *field = *field - 1;
            }
            if (satellite_type == 3
                // The original compares the signed result of a wrapping
                // 32-bit subtraction, not the two unsigned counters.
                && ((PlayersData[target_faction_id].satellites_[3]
                    - PlayersData[target_faction_id].sat_odp_deployed)
                    & 0x80000000U) != 0) {
                PlayersData[target_faction_id].sat_odp_deployed =
                    PlayersData[target_faction_id].satellites_[3];
                see_map_check();
                ReportWinGlobal.redraw();
                return;
            }
        }
        see_map_check();
        ReportWinGlobal.redraw();
    }
}

// ORIGINAL: 0x004A08E0 ?draw_security@ReportWin@@QAEXXZ 0x004A08E0-0x004A1134
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/004a08e0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_security() {
    Buffer *canvas = static_cast<Buffer *>(this);
    const int faction_id = ReportIfGlobal.report_faction_id_;
    const PlayerData &faction = PlayersData[faction_id];
    const int filter = ReportIfGlobal.field_1E244_;
    RECT title = { energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                   energy_panel_rect_.right - 3, 0 };
    const int first_y = energy_panel_rect_.top + 9 + field_E268_;
    title.bottom = first_y;
    RECT row = { title.left, first_y + 2, title.right, first_y + 43 };
    canvas->box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    canvas->box_sprite(&title, &InterfaceBoxStyles[1]);
    const int rows = (energy_panel_rect_.bottom - row.top) / 43;
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, get_adjective(faction_id));
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xAD8 / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    canvas->write_cent_l(StringTemp, &title, strlen(StringTemp));
    ++title.top;
    ++title.bottom;
    spot_.kill_type(0);

    int last_row = -1;
    for (int i = 0; i < 128; ++i) {
        const int id = i < 64 ? i : faction_id * 64 - 64 + i;
        if ((VehPrototypes[id].flags & 1) != 0
            && reportwin_security_filter(id, filter)
            && faction.proto_id_lost[id] + (faction.proto_id_queue[id] != 0)
                + (faction.proto_id_active[id] != 0) != 0) ++last_row;
    }
    if (field_C0F8_ == 0) {
        if (rows <= last_row) field_E26C_ = 1;
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
        scroll_.paging_ = field_C108_;
        scroll_.set_pos(field_C0FC_);
        field_E270_ = 0;
    }
    if (last_row == -1) return;
    int ordinal = -1;
    for (int index = 0; index < 128; ++index) {
        const int id = index < 64 ? index : faction_id * 64 - 64 + index;
        const VehPrototype &prototype = VehPrototypes[id];
        if ((prototype.flags & 1) == 0 || !reportwin_security_filter(id, filter)
            || (faction.proto_id_queue[id] != 0) + (faction.proto_id_active[id] != 0)
                + faction.proto_id_lost[id] == 0) continue;
        ++ordinal;
        const int first = field_C0FC_;
        if (ordinal < first || ordinal >= rows + first) continue;
        if (ordinal != first) { row.top += 43; row.bottom += 43; }
        const bool selected = (selected_prototypes_[index >> 5] & (1u << (index & 31))) != 0;
        if (selected) canvas->box_sprite(&row, &InterfaceBoxStyles[2]);
        canvas->box_sprite(&row, &InterfaceBoxStyles[selected ? 5 : 1]);
        spot_.add(id, 0, row.left, row.top, row.right - row.left, row.bottom - row.top);
        rect1_.top = row.top;
        const int text_y = row.top - field_E268_ / 2 + 21;
        const int vehicle = veh_fake(id, faction_id);
        veh_draw(canvas, vehicle, energy_panel_rect_.left, row.top + 2, -6, 8, 0);
        canvas->set_text_color(0x99, -1, 1, 1);
        canvas->set_text_color2(0xF2, -1, 1, 1);
        canvas->set_text_color3(0xF9, -1, 1, 1);
        canvas->set_font(ReportIfGlobal.field_1E18C_, ReportIfGlobal.field_1E18C_,
                         ReportIfGlobal.field_1E18C_, 0);
        rect1_.left = 102;
        StringTemp[0] = 0;
        if (prototype.obsolete_factions & (1u << (faction_id & 31))) strcat(StringTemp, "-");
        strcat(StringTemp, VehPrototypes[id].veh_name);
        canvas->write_l(StringTemp, rect1_.left, text_y, strlen(StringTemp));
        rect1_.left = (rect1_.right >> 1) + 40;
        StringTemp[0] = 0;
        say_stats(StringTemp, id, 0);
        canvas->write_l(StringTemp, rect1_.left, text_y, strlen(StringTemp));
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        reportwin_append_number(StringTemp, faction.proto_id_active[id] > 250 ? 0 : faction.proto_id_active[id]);
        strcat(StringTemp, " "); strcat(StringTemp, label_get(0x614 / 4)); strcat(StringTemp, ", ");
        strcat(StringTemp, "}");
        reportwin_append_number(StringTemp, faction.proto_id_queue[id] > 250 ? 0 : faction.proto_id_queue[id]);
        strcat(StringTemp, " "); strcat(StringTemp, label_get(0x618 / 4)); strcat(StringTemp, ", ");
        strcat(StringTemp, "[");
        reportwin_append_number(StringTemp, faction.proto_id_lost[id]);
        strcat(StringTemp, " "); strcat(StringTemp, label_get(0x61C / 4)); strcat(StringTemp, "]");
        canvas->write_right_l(StringTemp, rect1_.right - 2, text_y, strlen(StringTemp));
        rect1_.top += 43;
    }
}

// ORIGINAL: 0x004A1140 ?draw_score@ReportWin@@QAEXXZ 0x004A1140-0x004A2208
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/004a1140.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_score() {
    Buffer &canvas = *static_cast<Buffer *>(this);
    const int faction = ReportIfGlobal.report_faction_id();
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3,
                  energy_panel_rect_.top + static_cast<int>(field_E268_) + 9};
    RECT body = {title.left, title.bottom + 2, title.right, energy_panel_rect_.bottom - 3};
    canvas.box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    canvas.box_sprite(&title, &InterfaceBoxStyles[1]);
    canvas.box_sprite(&body, &InterfaceBoxStyles[1]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[faction].formal_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xADC / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    canvas.write_cent_l(StringTemp, &title, strlen(StringTemp));
    compute_score(faction, 0, 0, 0);
    if (X_text_open(ScriptPopupFile, "SCORING")) return;
    rect1_.left = body.left + 4;
    rect1_.top = body.top + 4;
    rect1_.right = body.right - body.left - 8;
    rect1_.bottom = rect1_.right;
    for (int item = 0; item < 9; ++item) {
        text_get();
        if (PlayersData[faction].score_items_[item] == 0 || item == 5) continue;
        StringTemp[0] = 0;
        if (item == 6) {
            const int own = num_objectives(faction, 0);
            const int total = num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000);
            report_topic_number(own);
            strcat(StringTemp, " ");
            if (total > own) {
                strcat(StringTemp, label_get(0xE0C / 4));
                report_topic_line(canvas, rect1_, rect1_.left);
                rect1_.top += field_E268_;
                StringTemp[0] = 0;
                report_topic_number(total - own);
                strcat(StringTemp, " ");
                strcat(StringTemp, label_get(0x1030 / 4));
                report_topic_line(canvas, rect1_, rect1_.left);
                rect1_.top += static_cast<int>(field_E268_ * 3) / 2;
                StringTemp[0] = 0;
            }
            strcat(StringTemp, TextBufferGetPtr);
        } else if (item == 7) {
            if (GameDataStateStorage.fields_.game_state_ & 0x200000) strcat(StringTemp, label_get(0xE70 / 4));
            else if (GameDataStateStorage.fields_.game_state_ & 0x400000) strcat(StringTemp, label_get(0xE88 / 4));
            else if (PlayersData[faction].flags & 0x100000) strcat(StringTemp, label_get(0x1198 / 4));
            else if (!(GameDataStateStorage.fields_.game_state_ & 0x2000)) strcat(StringTemp, label_get(0xE10 / 4));
            else strcat(StringTemp, TextBufferGetPtr);
        } else strcat(StringTemp, TextBufferGetPtr);
        report_topic_line(canvas, rect1_, rect1_.left);
        StringTemp[0] = 0;
        const int value = PlayersData[faction].score_items_[item];
        strcat(StringTemp, value < 0 ? "- " : "+ ");
        report_topic_number(abs(value));
        canvas.write_right_l(StringTemp, rect1_.left + rect1_.right,
                             rect1_.top, strlen(StringTemp));
        rect1_.top += field_E268_;
    }
    if (PlayersData[faction].score_items_[5]) {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(MapNativeLifeForms ? 0x908 / 4 : 0x904 / 4));
        strcat(StringTemp, " ");
        strcat(StringTemp, MapNativeLifeForms ? "+25%" : "-25%");
        report_topic_line(canvas, rect1_, rect1_.left);
        StringTemp[0] = 0;
        const int value = PlayersData[faction].score_items_[5];
        strcat(StringTemp, value < 0 ? "- " : "+ ");
        report_topic_number(abs(value));
        canvas.write_right_l(StringTemp, rect1_.left + rect1_.right,
                             rect1_.top, strlen(StringTemp));
        rect1_.top += field_E268_;
    }
    if ((GameDataStateStorage.fields_.game_rules_ & 0x400) && !(GameDataStateStorage.fields_.game_state_ & 0x20) && PlayersData[faction].unk_109 > 0) {
        rect1_.top += field_E268_;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x1044 / 4));
        CharUpperA(StringTemp);
        report_topic_line(canvas, rect1_, rect1_.left);
        strcpy(StringTemp, "+100%");
        canvas.write_right_l(StringTemp, rect1_.left + rect1_.right,
                             rect1_.top, strlen(StringTemp));
        rect1_.top += field_E268_;
    }
    rect1_.top += field_E268_;
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x6C0 / 4));
    strcat(StringTemp, "}");
    report_topic_line(canvas, rect1_, rect1_.left);
    strcpy(StringTemp, "{");
    if (PlayersData[faction].unk_109 < 0) strcat(StringTemp, "- ");
    report_topic_number(abs(PlayersData[faction].unk_107));
    strcat(StringTemp, "}");
    canvas.write_right_l(StringTemp, rect1_.left + rect1_.right,
                         rect1_.top, strlen(StringTemp));
    rect1_.top += field_E268_ * 2;
    if (GameDataStateStorage.fields_.game_state_ & 1) {
        if (GameDataStateStorage.fields_.game_state_ & 0x80000) {
            StringTemp[0] = 0;
            if (PlayersData[faction].score_items_[7]) strcat(StringTemp, label_get(0xE10 / 4));
            else {
                strcat(StringTemp, label_get(0xE14 / 4));
                for (int other = 1; other < 8; ++other) {
                    if (other == faction) continue;
                    compute_score(other, 0, 0, 0);
                    if (PlayersData[other].score_items_[7]) {
                        strcat(StringTemp, " "); strcat(StringTemp, "(");
                        PluralityDefault = 0;
                        GenderDefault = Players[other].is_leader_female;
                        strcat(StringTemp, Players[other].name_leader);
                        strcat(StringTemp, " "); strcat(StringTemp, label_get(0x101C / 4));
                        strcat(StringTemp, ")");
                        break;
                    }
                }
                CharUpperA(StringTemp);
                strcat(StringTemp, "!");
                report_topic_line(canvas, rect1_, rect1_.left);
                rect1_.top += field_E268_ * 2;
            }
        } else if (!(GameDataStateStorage.fields_.game_state_ & 0x602000) && ScenarioSave.objective_req_victory <= 9000) {
            StringTemp[0] = 0;
            // Original reuses the faction byte-index local as the outcome.
            // The unmet nonzero objective target with item 8 nonzero leaves it
            // intact; preserve that malformed DRAWSTRING selection path.
            int outcome = faction * sizeof(PlayerData);
            int most = 0;
            if (ScenarioSave.objective_req_victory == 0) {
                const int objectives = num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000);
                int leader, pacts;
                most = most_objectives(&leader, &pacts);
                if (objectives >= (most < 0 ? 0 : most)) {
                    if (pacts) { strcat(StringTemp, label_get(0x102C / 4)); outcome = 2; }
                    else { strcat(StringTemp, label_get(0xE10 / 4)); outcome = 1; }
                } else {
                    outcome = 0;
                    strcat(StringTemp, label_get(0xE14 / 4));
                    if (leader > 0) {
                        strcat(StringTemp, " "); strcat(StringTemp, "(");
                        PluralityDefault = 0;
                        GenderDefault = Players[leader].is_leader_female;
                        strcat(StringTemp, Players[leader].name_leader);
                        strcat(StringTemp, " "); strcat(StringTemp, label_get(0x101C / 4));
                        strcat(StringTemp, ")");
                    }
                }
            } else if (num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000) >= ScenarioSave.objective_req_victory) {
                strcat(StringTemp, label_get(0xE10 / 4)); outcome = 1;
            } else if (PlayersData[faction].score_items_[8] == 0) {
                outcome = 0;
                strcat(StringTemp, label_get(0xE14 / 4));
            }
            CharUpperA(StringTemp);
            strcat(StringTemp, "!");
            report_topic_line(canvas, rect1_, rect1_.left);
            rect1_.top += field_E268_ * 2;
            for (int other = 1; other < 8; ++other) {
                if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << other)) || other == faction) continue;
                const int target = ScenarioSave.objective_req_victory ? ScenarioSave.objective_req_victory : most;
                if (num_objectives(other, GameDataStateStorage.fields_.game_rules_ & 0x1000) < target) continue;
                if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << other)) && (GameDataStateStorage.fields_.game_rules_ & 0x80000)) continue;
                const char *section = outcome > 1 ? "DRAWSTRING" : outcome ? "WINSTRING2" : "WINSTRING";
                if (X_text_open(ScriptPopupFile, section)) break;
                text_get();
                PluralityDefault = 0; GenderDefault = Players[other].is_leader_female;
                parse_says(0, Players[other].title_leader, -1, -1);
                PluralityDefault = 0; GenderDefault = Players[other].is_leader_female;
                parse_says(1, Players[other].name_leader, -1, -1);
                GenderDefault = Players[other].noun_gender;
                PluralityDefault = Players[other].is_noun_plural;
                parse_says(2, Players[other].noun_faction, -1, -1);
                StringTemp[0] = 0;
                report_topic_number(num_objectives(other, 0));
                const int all = num_objectives(other, GameDataStateStorage.fields_.game_rules_ & 0x1000);
                if (all > num_objectives(other, 0)) {
                    strcat(StringTemp, "(");
                    report_topic_number(num_objectives(other, GameDataStateStorage.fields_.game_rules_ & 0x1000));
                    strcat(StringTemp, ")");
                }
                parse_says(3, StringTemp, -1, -1);
                StringTemp[0] = 0;
                parse_string(TextBufferGetPtr, StringTemp);
                report_topic_line(canvas, rect1_, rect1_.left);
                rect1_.top += field_E268_;
            }
        }
    }
    if (GameDataStateStorage.fields_.game_state_ & 0x20) {
        canvas.set_text_color(0xF9, -1, 1, 1);
        canvas.set_font(ReportIfGlobal.field_1E1B8_, 0, 0, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x1060 / 4));
        strcat(StringTemp, "!");
        canvas.write_cent_l(StringTemp, rect1_.left, energy_panel_rect_.bottom - 50,
                            rect1_.right, 50, strlen(StringTemp));
    }
    text_close();
}

// ORIGINAL: 0x004A2210 ?draw_intel@ReportWin@@QAEXXZ 0x004A2210-0x004A345F
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/004a2210.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_intel() {
    Buffer &canvas = *static_cast<Buffer *>(this);
    const int viewer = ReportIfGlobal.report_faction_id();
    const int subject = ReportIfGlobal.report_selection_;
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3,
                  energy_panel_rect_.top + static_cast<int>(field_E268_) + 9};
    RECT body = {title.left, title.bottom + 2, title.right,
                 energy_panel_rect_.bottom - 3};
    canvas.box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    canvas.box_sprite(&title, &InterfaceBoxStyles[1]);
    canvas.box_sprite(&body, &InterfaceBoxStyles[1]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[viewer].formal_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xAE0 / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    canvas.write_cent_l(StringTemp, &title, strlen(StringTemp));
    rect1_.left = body.left + 4;
    rect1_.top = body.top + 4;
    rect1_.right = body.right - body.left - 8;
    rect1_.bottom = rect1_.right;
    const bool informed = (GameDataStateStorage.fields_.game_state_ & 0x80) || spying(subject) ||
        (PlayersData[viewer].diplo_treaties[subject] & 1);
    Sprite &portrait = FactionsArt[subject].council_big;
    portrait.draw(&canvas, portrait.transparent_index(), rect1_.left, rect1_.top);
    const int text_x = rect1_.left + 158;
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    report_append_leader(Players[subject]);
    strcat(StringTemp, "]");
    CharUpperA(StringTemp);
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_ * 2;
    diplomacy_check(viewer, subject, 0);
    report_intel_label(0xE98 / 4);
    const unsigned int relation = PlayersData[viewer].diplo_treaties[subject];
    if (relation & 0x10) strcat(StringTemp, label_get(0x358 / 4));
    else if (relation & 1) strcat(StringTemp, label_get(0x34C / 4));
    else if (relation & 2) strcat(StringTemp, label_get(0x350 / 4));
    else if (relation & 4) strcat(StringTemp, label_get(0x354 / 4));
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_;
    diplomacy_check(viewer, subject, 0);
    report_intel_label(0xE94 / 4);
    say_special(StringTemp, StringTable.get(Mood[get_mood(DiploFriction)]), subject);
    CharUpperA(StringTemp);
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_;
    report_intel_label(0x5A0 / 4);
    const int fight = PlayersData[subject].ai_fight;
    strcat(StringTemp, label_get(fight < 0 ? 0x5AC / 4 : fight == 0 ? 0x5A8 / 4 : 0x5A4 / 4));
    CharUpperA(StringTemp);
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_;
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x5B0 / 4));
    CharUpperA(StringTemp);
    strcat(StringTemp, ":   ]");
    int priorities = 0;
    const PlayerData &data = PlayersData[subject];
    const int interests[4] = {data.ai_growth, data.ai_tech, data.ai_wealth, data.ai_power};
    for (int i = 0; i < 4; ++i) {
        if (interests[i]) {
            if (priorities) strcat(StringTemp, ", ");
            strcat(StringTemp, label_get(0x824 / 4 + i));
            ++priorities;
        }
    }
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_;
    report_intel_label(0x5C0 / 4);
    const int category = Players[subject].soc_ideology_category;
    const int model = Players[subject].soc_ideology_model;
    if (category >= 0 && model != 0) {
        strcat(StringTemp, StringTable.get(SocialCategories[category].type));
        strcat(StringTemp, ": ");
        strcat(StringTemp, StringTable.get(SocialCategories[category].name[model]));
        CharUpperA(StringTemp);
        report_topic_line(canvas, rect1_, text_x);
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
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_;
    report_intel_label(0xE4 / 4);
    report_topic_number(PlayersData[subject].energy_reserves);
    if (informed) {
        report_topic_line(canvas, rect1_, text_x);
        rect1_.top += field_E268_;
    }
    if ((GameDataStateStorage.fields_.game_state_ & 0x80) || !(GameDataStateStorage.fields_.game_rules_ & 0x200)) {
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0x254 / 4));
        strcat(StringTemp, ": ]");
        say_tech(StringTemp, PlayersData[subject].tech_id_researching, 0);
        if (informed) {
            report_topic_line(canvas, rect1_, text_x);
            rect1_.top += field_E268_;
        }
    }
    if (!(Players[subject].rule_flags & RFLAG_ALIEN)) {
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0xF00 / 4));
        CharUpperA(StringTemp);
        strcat(StringTemp, ":   ]");
        report_topic_number(council_votes(subject));
        report_topic_line(canvas, rect1_, text_x);
    }
    rect1_.top += field_E268_ + 20;
    for (int category_index = 0; category_index < 4; ++category_index) {
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, StringTable.get(SocialCategories[category_index].type));
        strcat(StringTemp, ":   ]");
        strcat(StringTemp, StringTable.get(SocialCategories[category_index].name[
            report_social_model(PlayersData[subject].soc_category_pending, category_index)]));
        CharUpperA(StringTemp);
        canvas.write_l(StringTemp, rect1_.left + 20,
                       rect1_.top + category_index * field_E268_, strlen(StringTemp));
    }
    for (int faction = 1; faction < 8; ++faction) {
        const unsigned int flags = PlayersData[subject].diplo_treaties[faction];
        if (faction == subject || !(flags & 8) ||
            !(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << faction)) ||
            (!informed && faction != viewer && !(PlayersData[viewer].diplo_treaties[faction] & 1))) continue;
        StringTemp[0] = 0;
        if (flags & 0x10) {
            strcat(StringTemp, label_get(0x358 / 4)); strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x5C8 / 4));
        } else if (flags & 1) {
            strcat(StringTemp, get_pact(subject)); strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x43C / 4));
        } else {
            if (flags & 2) strcat(StringTemp, label_get(0x350 / 4));
            else {
                if (flags & 0x10000) { strcat(StringTemp, label_get(0x5D0 / 4)); strcat(StringTemp, " "); }
                strcat(StringTemp, label_get(0x354 / 4));
            }
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x5CC / 4));
        }
        strcat(StringTemp, " ");
        report_append_leader(Players[faction]);
        report_topic_line(canvas, rect1_, text_x + 125);
        rect1_.top += field_E268_;
    }
}

// ORIGINAL: 0x004A3460 ?init@ReportIf@@QAEXXZ 0x004A3460-0x004A3927;0x00659580-0x00659595 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/004a3460.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a3460 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf initialization from the retained original behavior and the genuine ReportWin control storage; its non-virtual source identity remains distinct from ReportIf's historical virtual callbacks."}
void ReportIf::init() {
    Buffer image;
    field_1E240_ = 0;
    field_1E244_ = 4;
    field_4_ = 5;
    // Registry slot5 is this genuine SubInterface, not another object.
    register_iface_mode(5, this);
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
    for (int row = 0; row < 7; ++row)
        for (int column = 0; column < 3; ++column)
            spritesA_[row * 3 + column].extract(&image, WinFillColour,
                row * 29, column * 35, 30, 36, 0);
    field_20_ = MainInterfaceGlobal.main_frame_rect();
    field_30_ = MainInterfaceGlobal.right_frame_rect();
    field_40_ = MainInterfaceGlobal.lower_frame_rect();
    field_50_ = MainInterfaceGlobal.info_panel_frame();
    field_1E230_ = WinScreenWidth != 800;
    const int step = field_1E230_ ? 42 : 37;
    for (int slot_index = 0; slot_index < 7; ++slot_index)
        report_button_positions_[slot_index] = slot_index * step;
    for (int faction = 1; faction <= 7; ++faction)
        for (int state = 0; state < 3; ++state)
            faction_button_sprites_[faction - 1][state] =
                faction == ConsoleGlobal.faction_id_ && field_1E230_
                    ? &FactionsArt[faction].report_logo[state == 0]
                    : &FactionsArt[faction].report_logo_small[state];
    int position = field_40_.left + 16 + (field_1E230_ != 0);
    report_category_center_ =
        (field_40_.bottom + field_40_.top - spritesA_[0].height()) / 2;
    for (int category = 0; category < 7; ++category) {
        report_category_positions_[category] = position;
        position += spritesA_[0].width() + 9 + (field_1E230_ ? 3 : 0);
    }
    // Original includes the eighth, previously unwritten slot in this walk
    // before replacing it below; preserve that store sequence.
    for (int slot = 0; slot < 8; ++slot)
        report_button_positions_[slot] += field_50_.left + 46 + (field_1E230_ ? 43 : 0);
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
    field_1C_ = field_1E184_->line_advance();
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
// ORIGINAL: 0x004A3950 ?report@ReportIf@@QAEXHHH@Z 0x004A3950-0x004A3CD3 FILE
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
    const int previous_page = report_page_history_[0];
    switch (previous_page) {
    case 2:
        listBox1_.close();
        flatButton1_.hide();
        flatButton2_.hide();
        break;
    case 3:
        ReportWinGlobal.listBox_.close();
        // Fall through: energy and intel share both embedded lists.
    case 0:
        listBox1_.close();
        listBox2_.close();
        break;
    case 4:
        close_ops();
        break;
    case 5:
        if (field_8_ != 4 && field_8_ != 5) {
            WorldClimateWorldWin.hide_all();
            MainInterfaceGlobal.flat_button_[6].hide();
            MainInterfaceGlobal.flat_button_[7].hide();
        }
        break;
    case 6:
        close_sat();
        break;
    case 7:
        flatButton12_.hide();
        flatButton14_.hide();
        flatButton13_.hide();
        break;
    }

    // The original category run is 2..8; category 1 has no report page or
    // typed control in the class layout. Zero denotes the intel list.
    if (previous_page >= 2 && previous_page <= 8) {
        FlatButton &old_button = flatButtonsB_[previous_page - 2];
        old_button.set_report_page_state(-1);
        old_button.redraw();
    }
    if (report_id >= 2 && report_id <= 8) {
        FlatButton &new_button = flatButtonsB_[report_id - 2];
        new_button.set_report_page_state(2);
        new_button.redraw();
    }

    report_selection_ = selection;
    field_8_ = report_id;
    report_faction_id_ = faction_id;

    if (previous_page != 2 && ReportWinGlobal.shared_report_selection() != -1) {
        int next_focus;
        if (!(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
            && (GameDataStateStorage.fields_.game_rules_ & RULES_BLIND_RESEARCH)) {
            if (ReportWinGlobal.shared_research_focus() >= 0 && ReportWinGlobal.shared_research_focus() <= 0x59
                && has_tech(ReportWinGlobal.shared_research_focus(), faction_id)) {
                next_focus = ReportWinGlobal.shared_research_focus();
            } else {
                next_focus = ReportWinGlobal.shared_research_focus();
                for (int tech_id = 0; tech_id < 0x59; ++tech_id) {
                    if (has_tech(tech_id, faction_id)) {
                        next_focus = tech_id;
                        break;
                    }
                }
            }
        } else {
            next_focus = PlayersData[faction_id].tech_id_researching;
        }
        ReportWinGlobal.shared_research_focus() = next_focus;
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
    const int current_page = report_page_history_[0];
    if (current_page == 4 || current_page == 5 || current_page == 7) {
        if (current_page != report_page_history_[1] || report_topic_changed_) {
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
    case 0: draw_intel(); break;
    case 2: draw_labs(); break;
    case 3: draw_energy(); break;
    case 4: draw_ops(); break;
    case 5: draw_projects(); break;
    case 6: draw_sat(); break;
    case 7: draw_security(); break;
    case 8: draw_score(); break;
    }
    MainInterfaceGlobal.soft_update();
    ReportWinGlobal.redraw();
    if (ReportWinGlobal.field_E26C_ != 0 && !report_scroll_blocked_) {
        scroll_show(&ReportWinGlobal, &ReportWinGlobal.scroll_);
        ReportWinGlobal.field_E26C_ = 0;
        ReportWinGlobal.field_C0F8_ = 1;
    }
}

// ORIGINAL: 0x004A3D20 ?reset@ReportIf@@QAEXXZ 0x004A3D20-0x004A3FBF
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

    RECT *r1 = &field_20_;
    MainInterfaceGlobal.Buffer::box_sprite(r1, &InterfaceBoxStyles[33]);
    RECT *r2 = &field_30_;
    MainInterfaceGlobal.Buffer::box_sprite(r2, &InterfaceBoxStyles[33]);
    RECT *r3 = &field_50_;
    MainInterfaceGlobal.Buffer::box_sprite(r3, &InterfaceBoxStyles[33]);
    RECT *r4 = &field_40_;
    MainInterfaceGlobal.Buffer::set_clip(r4);
    r4->left -= 7;
    MainInterfaceGlobal.Buffer::box_sprite(r4, &InterfaceBoxStyles[33]);
    r4->left += 7;
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    r1->left += 3;
    r1->right -= 3;
    r1->top += 3;
    r1->bottom -= 3;
    r2->left += 3;
    r2->top += 3;
    r2->right -= 3;
    r2->bottom -= 3;
    r3->top += 4;
    r3->left += 4;
    r3->right -= 4;
    r3->bottom -= 4;
    r4->left += 4;
    r4->top += 4;
    r4->right -= 4;
    r4->bottom -= 4;
    r4->left -= 3;

    MainInterfaceGlobal.Buffer::box_sprite(r1, &InterfaceBoxStyles[3]);
    MainInterfaceGlobal.Buffer::box_sprite(r2, &InterfaceBoxStyles[3]);
    MainInterfaceGlobal.Buffer::box_sprite(r3, &InterfaceBoxStyles[3]);
    MainInterfaceGlobal.Buffer::box_sprite(r4, &InterfaceBoxStyles[3]);

    r3->left += 3;
    r3->right -= 3;
    r3->top += 3;
    r3->bottom -= 3;
    r4->left += 3;
    r4->right -= 3;
    r4->top += 3;
    r4->bottom -= 3;

    MainInterfaceGlobal.Buffer::box_sprite(r3, &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.Buffer::box_sprite(r4, &InterfaceBoxStyles[0]);

    r4->left += 3;
    r1->left -= 3;
    r1->right += 3;
    r1->top -= 3;
    r1->bottom += 3;
    r2->left -= 3;
    r2->right += 3;
    r2->top -= 3;
    r2->bottom += 3;
    r3->left -= 7;
    r3->right += 7;
    r3->top -= 7;
    r3->bottom += 7;
    r4->left -= 7;
    r4->right += 7;
    r4->top -= 7;
    r4->bottom += 7;

    MainInterfaceGlobal.Buffer::set_font(field_1E180_, field_1E184_, field_1E184_, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
    MainInterfaceGlobal.Buffer::set_text_color2(0x99, -1, 1, 1);
    MainInterfaceGlobal.Buffer::set_text_color3(0xe1, -1, 1, 1);
    MainInterfaceGlobal.Buffer::clear_links();
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A3FC0 0x004A4020 0x004A4044 0x004A3FD0
// ORIGINAL: 0x004A3FC0 ?on_redraw@ReportIf@@QAEXXZ 0x004A3FC0-0x004A401F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:a538e0a3:src/recovered/units/004a3fc0.cpp", "image:0x004A3FC0-0x004A401F and dispatch table0x004A4020", "source:src/reportif.h"], "reason": "Recover the report-interface redraw operation through its real category member, reset and owned drawing methods. Original direct calls and nine-entry dispatch preserve the default/no-draw category without vtable or executable-address dispatch."}
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
// ORIGINAL: 0x004A4060 ?bl_anim@ReportIf@@QAEXXZ 0x004A4060-0x004A4191
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: source:src/recovered/units/004a4060.cpp", "source:src/reportif.h", "source:src/worldwin.h", "source:src/reportwin.h", "source:src/sprite.h"], "reason": "Advance ReportIf animation with the declared sprite run and coordinate state, drawing to the WorldClimateWorldWin Buffer base and stopping the ReportWin-owned Time at terminal frame 10."}
void ReportIf::bl_anim() {
    WorldClimateWorldWin.redraw();
    Buffer *const destination = static_cast<Buffer *>(&WorldClimateWorldWin);
    switch (animation_state_) {
    case -1:
    case 0:
    case 8:
    case 9:
        spritesB_[0].draw(destination, spritesB_[0].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 1:
    case 7:
        spritesB_[1].draw(destination, spritesB_[1].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 2:
    case 4:
    case 6:
        spritesB_[2].draw(destination, spritesB_[2].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 3:
    case 5:
        spritesB_[3].draw(destination, spritesB_[3].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 10:
        ReportWinGlobal.time1_.stop();
        break;
    }
    ++animation_state_;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A41D0 0x004A5ED4 0x004A5EF8 0x004A5E12
// ORIGINAL: 0x004A41D0 ?on_status@ReportIf@@QAEXH@Z 0x004A41D0-0x004A5ED2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004a41d0.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a41d0 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's status transition through its genuine SubInterface owner and ReportWin's embedded controls, following the retained original behavior. The source's emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// TRIED: full hand transcription from raw disassembly (not Ghidra, whose &DAT_x+idx*N pseudocode over-scales by 4x on 0x96cdac/0x96c9f8 - fixed here against the raw asm multiply chain and byte tests); covers close path (switch+jump table+loops), init path (7-button report loop, 4 button groups, single-column cef..cf8, has_tech/ bitmask channel grid, spying flags, 4-way spy-report geometry, scroll/report/slide tail). First divergence is instruction #3 (prologue 'mov'/'push' scheduling) - approximate throughout, not byte-verified block by block beyond the ones cited above.
// symbol    ?on_status@ReportIf@@UAEXH@Z
void ReportIf::on_status(int mode) {
    ReportWin &window = ReportWinGlobal;
    if (!mode) {
        ReportInterfaceStatusReentry = 0;
        window.time1_.stop();
        if (window.field_C0F8_) {
            scroll_hide(&window, &window.scroll_); window.field_C0F8_ = 0;
        }
        slide_hide(&window, 0); window.hide();
        window.flatButton3_.close(); window.flatButton2_.close();
        for (int category = 0; category < 7; ++category) flatButtonsB_[category].close();
        for (int faction = 0; faction < 7; ++faction) flatButtonsA_[faction].close();
        switch (field_8_) {
        case 3: window.listBox_.close(); // Original falls through to both local lists.
        case 0: listBox1_.close(); listBox2_.close(); break;
        case 2: listBox1_.close(); flatButton1_.hide(); flatButton2_.hide(); break;
        case 4: close_ops(); break;
        case 6: close_sat(); break;
        case 7: flatButton12_.hide(); flatButton14_.hide(); flatButton13_.hide(); break;
        }
        WorldClimateWorldWin.hide_all();
        MainInterfaceGlobal.flat_button_[6].hide(); MainInterfaceGlobal.flat_button_[7].hide();
        return;
    }
    if (ReportInterfaceStatusReentry) { ReportInterfaceStatusReentry = 0; return; }
    report_scroll_blocked_ = 1; field_1E234_ = 1;
    window.field_C0F8_ = 0; report_topic_changed_ = 1;
    window.fill(9); report_faction_id_ = ConsoleGlobal.faction_id_;
    for (int history = 0; history < 19; ++history) report_page_history_[history] = -1;
    window.shared_report_selection() = -1; field_1E1D8_ = 1;
    if ((GameDataStateStorage.fields_.game_state_ & 0x80) || !(GameDataStateStorage.fields_.game_rules_ & 0x200)) {
        int research = PlayersData[report_faction_id_].tech_id_researching;
        window.shared_research_focus() = research;
        if (research < 0 || research > 89)
            window.shared_research_focus() = 0;
    } else {
        window.shared_research_focus() = 0;
    }
    for (int topic = 0; topic < 7; ++topic) {
        StringTemp[0] = 0; strcat(StringTemp, label_get(0x2B1 + topic));
        FlatButton &button = flatButtonsB_[topic];
        button.init(0, topic + 2, report_category_positions_[topic], report_category_center_,
            spritesA_[0].width(), spritesA_[0].height(), &MainInterfaceGlobal, 0);
        button.set_report_icons(&spritesA_[topic*3], &spritesA_[topic*3+1], &spritesA_[topic*3+2]);
        button.set_bubble_text(StringTemp);
        static_cast<Buffer *>(&button)->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
        button.redraw(); button.redraw();
    }
    RECT &frame = window.report_frame_rect_;
    RECT &panel = window.energy_panel_rect_;
    frame.top = panel.bottom + 7; frame.bottom = panel.bottom + 27;
    frame.left = panel.left + 75; frame.right = panel.right - panel.left - 150 + frame.left;
    const int center = (panel.right + panel.left) / 2;
    StringTemp[0] = 0; strcat(StringTemp, label_get(0x21B));
    draw_workspace_rect_.left = frame.left; draw_workspace_rect_.top = frame.top;
    draw_workspace_rect_.right = center - 1; draw_workspace_rect_.bottom = frame.top + 20;
    report_iface_button(window.flatButton3_, draw_workspace_rect_, 1, &window, false);
    window.flatButton3_.redraw();
    StringTemp[0] = 0; strcat(StringTemp, label_get(0x299));
    draw_workspace_rect_.left = center + 1;
    draw_workspace_rect_.right = center + 1 + center - frame.left - 1;
    report_iface_button(window.flatButton2_, draw_workspace_rect_, 9, &window, false);
    window.flatButton2_.redraw();
    // The two paired mode selectors share precisely the same panel geometry.
    ButtonGroup *pairs[2] = {&buttonGroup1_, &buttonGroup2_};
    FlatButton *pair_buttons[2][2] = {{&flatButton1_, &flatButton2_}, {&flatButton3_, &flatButton4_}};
    const int pair_labels[2][2] = {{0x175,0x176},{0x1EE,0x2AA}};
    const int pair_ids[2][2] = {{0x92,0x93},{0xD0A,0xD0B}};
    for (int pair = 0; pair < 2; ++pair) {
        field_C_ = field_20_.left + 9; field_10_ = field_20_.top + 9;
        field_14_ = field_20_.right - field_20_.left - 18;
        pairs[pair]->init(pair == 0 ? 2 : 4, 0);
        const int half = static_cast<int>(field_14_) / 2;
        for (int side = 0; side < 2; ++side) {
            StringTemp[0] = 0; strcat(StringTemp, label_get(pair_labels[pair][side]));
            draw_workspace_rect_.left = field_C_ + (side ? half + 1 : 0);
            draw_workspace_rect_.top = field_10_;
            draw_workspace_rect_.right = side ? draw_workspace_rect_.left + half : field_C_ + half - 1;
            draw_workspace_rect_.bottom = field_10_ + 20;
            report_iface_button(*pair_buttons[pair][side], draw_workspace_rect_, pair_ids[pair][side], &MainInterfaceGlobal, true);
            pairs[pair]->add(pair_buttons[pair][side]);
        }
        pairs[pair]->set(pair_ids[pair][0], 1);
    }
    field_C_ = panel.left + 3; field_10_ = field_1C_ + panel.top + 11;
    field_14_ = panel.right - panel.left - 6;
    window.buttonGroup1_.init(4, 0);
    FlatButton *thirds[3] = {&window.flatButton4_, &window.flatButton5_, &window.flatButton6_};
    const int third_labels[3] = {0x32C,0x2E9,0x3F};
    const int third = static_cast<int>(field_14_) / 3;
    for (int part = 0; part < 3; ++part) {
        StringTemp[0] = 0; strcat(StringTemp, label_get(third_labels[part]));
        draw_workspace_rect_.left = field_C_ + (part == 2 ? third*2+2 : third*part);
        draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + (part == 0 ? third-2 : part == 1 ? third*2 : field_14_);
        draw_workspace_rect_.bottom = field_10_ + 20;
        report_iface_button(*thirds[part], draw_workspace_rect_, 0x8B3+part, &window, false);
        thirds[part]->reset_field_A9C_if_ungrouped(); thirds[part]->Win::hide();
        window.buttonGroup1_.add(thirds[part]);
    }
    window.buttonGroup1_.set(field_1E240_ ? field_1E240_ : 0x8B3, 1);
    field_C_ = panel.left + 3; field_10_ = field_1C_ + panel.top + 11;
    field_14_ = panel.right - panel.left - 6;
    window.buttonGroup2_.init(6, 0);
    FlatButton *halves[2] = {&window.flatButton7_, &window.flatButton8_};
    for (int side = 0; side < 2; ++side) {
        StringTemp[0] = 0; strcat(StringTemp, label_get(0x32C+side));
        const int half = static_cast<int>(field_14_) / 2;
        draw_workspace_rect_.left = field_C_ + (side ? half+1 : 0);
        draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + (side ? field_14_ : half-1);
        draw_workspace_rect_.bottom = field_10_ + 20;
        report_iface_button(*halves[side], draw_workspace_rect_, 0xD3C+side, &window, false);
        halves[side]->reset_field_A9C_if_ungrouped(); halves[side]->Win::hide();
        window.buttonGroup2_.add(halves[side]);
    }
    window.buttonGroup2_.set(0xD3C, 1);
    StringTemp[0] = 0; strcat(StringTemp, label_get(0x32E));
    draw_workspace_rect_.left = frame.left; draw_workspace_rect_.right = frame.right;
    draw_workspace_rect_.top = panel.bottom - 26; draw_workspace_rect_.bottom = panel.bottom - 6;
    report_iface_button(window.flatButton9_, draw_workspace_rect_, 0xD3E, &window, false);
    window.flatButton9_.set_border_styles(&InterfaceBoxStyles[47], &InterfaceBoxStyles[46], &InterfaceBoxStyles[45]);
    window.flatButton9_.reset_field_A9C_if_ungrouped(); window.flatButton9_.Win::hide();
    field_C_ = field_20_.left + 9; field_10_ = field_20_.top + 35;
    field_14_ = field_20_.right - field_20_.left - 18;
    FlatButton *orders[7] = {&flatButton5_, &flatButton6_, &flatButton7_, &flatButton8_, &flatButton9_, &flatButton10_, &flatButton11_};
    const int order_labels[7] = {0x2EA,0x2EB,0x2EB,0x2EB,0x2EB,0x2EC,0x2ED};
    for (int order = 0; order < 7; ++order) {
        StringTemp[0] = 0; strcat(StringTemp, label_get(order_labels[order]));
        if (order >= 1 && order <= 4) { strcat(StringTemp, " "); strcat(StringTemp, label_get(0x209+order-1)); }
        draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_; draw_workspace_rect_.bottom = field_10_ + 20;
        report_iface_button(*orders[order], draw_workspace_rect_, 0xCEF+order, &MainInterfaceGlobal, true);
        field_10_ += 22;
    }
    field_C_ = field_30_.left + 9; field_10_ = field_30_.top + 32;
    field_14_ = field_30_.right - field_30_.left - 18;
    FlatButton *security[3] = {&flatButton12_, &flatButton14_, &flatButton13_};
    const int security_ids[3] = {0xCF6,0xCF8,0xCF7};
    for (int action = 0; action < 3; ++action) {
        StringTemp[0] = 0; strcat(StringTemp, label_get(action ? 0x2DB : 0x3BB));
        if (action == 2) CharUpperA(StringTemp);
        draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_; draw_workspace_rect_.bottom = field_10_ + 20;
        report_iface_button(*security[action], draw_workspace_rect_, security_ids[action], &MainInterfaceGlobal, true);
        if (action < 2) field_10_ += 22;
    }
    field_C_ = field_20_.left + 9; field_10_ = field_20_.top + 32;
    field_14_ = field_20_.right - field_20_.left - 18;
    buttonGroup3_.init(0x6A, 0);
    for (int other = 1; other < 8; ++other) {
        if (other == ConsoleGlobal.faction_id_) continue;
        int elevators = 0;
        for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base)
            if (Bases[base].faction_id_current == other && has_fac(41, base, 0)) ++elevators;
        const PlayerData &data = PlayersData[other];
        if (!(has_tech(Facility[65].preq_tech, other)
            || data.satellites_[0]+data.satellites_[1]+data.satellites_[2]+data.satellites_[3]+elevators
            || has_tech(Facility[41].preq_tech, other))) continue;
        StringTemp[0] = 0;
        GenderDefault = Players[other].noun_gender;
        PluralityDefault = Players[other].is_noun_plural;
        strcat(StringTemp, Players[other].noun_faction);
        draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_; draw_workspace_rect_.bottom = field_10_ + 20;
        report_iface_button(flatButtonsC_[other-1], draw_workspace_rect_, other, &MainInterfaceGlobal, true);
        buttonGroup3_.add(&flatButtonsC_[other-1]); field_10_ += 22;
    }
    window.field_59B0_ = 1;
    for (int faction = 1; faction < 8; ++faction) {
        const int current = ConsoleGlobal.faction_id_;
        if (!(PlayersData[faction].diplo_treaties[current] & 1) && !spying(faction)
            && faction != current && !(GameDataStateStorage.fields_.game_state_ & 0x80)) continue;
        StringTemp[0] = 0; strcat(StringTemp, get_title(faction)); strcat(StringTemp, " ");
        strcat(StringTemp, get_name(faction)); strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0xB4)); strcat(StringTemp, " ");
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        strcat(StringTemp, Players[faction].noun_faction);
        int left, top, width, height;
        if (faction != current) {
            left = report_button_positions_[faction < current ? faction-1 : faction-2];
            top = report_button_positions_[7]; width = 37 + (field_1E230_ ? 5 : 0); height = 29;
        } else if (field_1E230_) {
            left = field_50_.left + 15; top = field_50_.top + 15; width = height = 70;
        } else {
            left = field_50_.left + 9; top = report_button_positions_[7]; width = 37; height = 29;
        }
        draw_workspace_rect_.left = left; draw_workspace_rect_.top = top;
        draw_workspace_rect_.right = left + width; draw_workspace_rect_.bottom = top + height;
        FlatButton &button = flatButtonsA_[faction-1];
        button.init(0, faction+9999, left, top, width, height, &MainInterfaceGlobal, 0);
        button.set_report_icons(faction_button_sprites_[faction-1][0], faction_button_sprites_[faction-1][1], faction_button_sprites_[faction-1][2]);
        button.set_bubble_text(StringTemp);
        static_cast<Buffer *>(&button)->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
        button.redraw(); button.redraw();
    }
    flatButtonsA_[ConsoleGlobal.faction_id_-1].set_report_page_state(2);
    flatButtonsA_[ConsoleGlobal.faction_id_-1].redraw();
    window.report_work_rect_.left = window.report_work_rect_.top = 0;
    window.report_work_rect_.right = 32; window.report_work_rect_.bottom = 300;
    window.scroll_.init(&window.report_work_rect_, &window, 0, 0); window.scroll_.hide();
    field_1E234_ = 0;
    ReportIfGlobal.report(ReportIfGlobal.field_8_, ReportIfGlobal.report_faction_id_, ReportIfGlobal.report_selection_);
    slide_show(&window, 0); window.show(3); report_scroll_blocked_ = 0;
    if (window.field_E26C_) {
        scroll_show(&window, &window.scroll_); window.field_E26C_ = 0; window.field_C0F8_ = 1;
    }
    if (WinModalWindow != static_cast<Win *>(&window)) {
        ReportInterfaceStatusReentry = 1; window.set_modal(0, pop_wait, &MainInterfaceGlobal);
    }
}

// ORIGINAL: 0x004A5F00 ?show@ReportWin@@QAEXH@Z 0x004A5F00-0x004A5F16 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/004a5f00.cpp records the original 0x004A5F00-0x004A5F16 body and BYTE_EXACT claim.", "ReportWin derives from Win and ReportIfGlobal owns the interface-mode transitions used by the seed."], "reason": "Home the existing ReportWin show identity in its class owner, retaining its measured byte-exact claim."}
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/004a5f00.cpp
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
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/units/004a5f20.cpp
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
// ORIGINAL: 0x004A6050 ?on_iface_selected@ReportIf@@QAEXHH@Z 0x004A6050-0x004A62F3 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/004a6050.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a6050 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's selection callback from the retained seed through its genuine SubInterface owner and ReportWin controls. Its current emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// symbol    ?on_iface_selected@ReportIf@@UAEXHH@Z
void ReportIf::on_iface_selected(int item, int) {
    if (ReportIfGlobal.field_8_ == 2) {
        if (item < 0x400) {
            ReportWinGlobal.shared_research_focus() = item;
            ReportWinGlobal.redraw();
        } else {
            TutWinBaseWindow.zoom(item - 0x400, 0);
        }
    } else if (ReportIfGlobal.field_8_ == 3) {
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
    } else if (ReportIfGlobal.field_8_ == 4 && item >= 1 && item <= 8) {
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
                if (vehicle >= 0) {
                    while (Vehs[vehicle].prev_veh_id_stack >= 0)
                        vehicle = Vehs[vehicle].prev_veh_id_stack;
                    for (; vehicle >= 0; vehicle = Vehs[vehicle].next_veh_id_stack)
                        ++value;
                }
                break;
            }
            case 8:
                for (int facility = 0; facility < 70; ++facility)
                    if (BaseCurrent->facilities_built[facility / 8] & (1 << (facility % 8)))
                        ++value;
                for (int project = 0; project < 64; ++project)
                    if (base_project(project) == base) value += 5;
                break;
            }
            sort_entries_[base].sorted_value = value;
        }
        merge_sort(0, GameDataStateStorage.fields_.base_current_count_ - 1);
        ReportWinGlobal.field_5A08_ = item;
        ReportWinGlobal.redraw();
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
// ORIGINAL: 0x004A6330 ?on_iface_button_clicked@ReportIf@@QAEXH@Z 0x004A6330-0x004A6D10 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004a6330.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a6330 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's button callback from the retained seed through its genuine SubInterface owner and ReportWin controls. Its current emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// TRIED: full transcription landed (MISMATCH #3). Used the brief's resolved jump tables directly (three switches, cases 0xcef-0xcf5, 0xcf0-0xcf3, and 2-8) rather than the raw `ja`+`jmp [eax*4+addr]` ranges alone. Caught a real Ghidra error via the raw bytes: every `*(uint*)(DAT_0090ea30+0x1c)` Ghidra prints is actually `[eax+0x38]` in the disassembly (`mov eax,[0x90ea30]; mov ecx,[eax+0x38]`), confirmed at both call sites that touch it. ReportIf/BaseWin/NetDaemon fields are all raw self+offset casts, not src/reportif.h's hypothesised layout, per the brief's warning.
// symbol    ?on_iface_button_clicked@ReportIf@@UAEXH@Z
void ReportIf::on_iface_button_clicked(int button) {
    int faction = ConsoleGlobal.faction_id_;
    if (button > 9999) {
        report_topic_changed_ = 1;
        flatButtonsA_[ReportIfGlobal.report_faction_id_ - 1].set_report_page_state(-1);
        flatButtonsA_[ReportIfGlobal.report_faction_id_ - 1].redraw();
        flatButtonsA_[button - 10000].set_report_page_state(2);
        flatButtonsA_[button - 10000].redraw();
        report(ReportIfGlobal.field_8_, button - 9999, report_selection_);
        return;
    }
    if (field_8_ == 4 && report_faction_id_ == faction) {
        switch (button) {
        case 0xCEF: {
            // Selection covers the entire original 512-record base allocation,
            // not merely the currently populated prefix.
            for (int base = 0; base < 512; ++base)
                if (Bases[base].faction_id_current == faction
                    && Bases[base].faction_id_current == ReportIfGlobal.report_faction_id_)
                    ReportWinGlobal.selected_bases_[base >> 5] |= 1u << (base & 31);
            int count = 0;
            for (int selected = 0; selected < 512; ++selected)
                if (ReportWinGlobal.selected_bases_[selected >> 5] & (1u << (selected & 31))) ++count;
            ReportWinGlobal.field_5A04_ = count == 1;
            break;
        }
        case 0xCF0: case 0xCF1: case 0xCF2: case 0xCF3: {
            log_say("Global EDBC base set", 0, 0, 0);
            for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == faction
                    && (ReportWinGlobal.selected_bases_[base >> 5] & (1u << (base & 31)))
                    && NetDaemonNet.lock_base(base, 0, -1, -1) == 0) {
                    TutWinBaseWindow.soft_zoom(base);
                    BaseCurrent->governor |= 0x80000000;
                    BaseCurrent->governor &= 0xF0FFFFFF;
                    BaseCurrent->governor |= 0x01000000u << (button - 0xCF0);
                    TutWinBaseWindow.gov_activate();
                    NetDaemonNet.unlock_base(base);
                }
            }
            break;
        }
        case 0xCF4: {
            log_say("Global governor advanced", 0, 0, 0);
            bool configured = false;
            for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current != faction
                    || !(ReportWinGlobal.selected_bases_[base >> 5] & (1u << (base & 31)))) continue;
                if (!configured) {
                    TutWinBaseWindow.soft_zoom(base);
                    if (TutWinBaseWindow.governor(1) != 0) break;
                    configured = true;
                }
                if (NetDaemonNet.lock_base(base, 0, -1, -1) == 0) {
                    TutWinBaseWindow.soft_zoom(base);
                    BaseCurrent->governor = PlayersData[report_faction_id_].base_governor_adv;
                    TutWinBaseWindow.gov_activate();
                    draw_radius(BaseCurrent->x, BaseCurrent->y, 2, 2);
                    NetDaemonNet.unlock_base(base);
                }
            }
            break;
        }
        case 0xCF5: {
            log_say("Global governor off", 0, 0, 0);
            for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == faction
                    && (ReportWinGlobal.selected_bases_[base >> 5] & (1u << (base & 31)))
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
        faction = ConsoleGlobal.faction_id_;
    }
    if (field_8_ == 7 && report_faction_id_ == faction) {
        if (button == 0xCF6 || button == 0xCF7 || button == 0xCF8) {
            for (int row = 0; row < 128; ++row) {
                const int prototype_id = row < 64 ? row : report_faction_id_ * 64 - 64 + row;
                const VehPrototype &prototype = VehPrototypes[prototype_id];
                const int offense = Weapon[prototype.weapon_id].offense_rating;
                const int filter = field_1E244_;
                if (!(filter == 4 || (filter == 3 && offense == 0)
                    || (filter < 3 && Chassis[prototype.chassis_id].triad == filter && offense != 0))) continue;
                if (button == 0xCF6) {
                    ReportWinGlobal.selected_prototypes_[row >> 5] |= 1u << (row & 31);
                    continue;
                }
                if (!(ReportWinGlobal.selected_prototypes_[row >> 5] & (1u << (row & 31)))) continue;
                if (diplo_lock(100) != 0) break;
                for (int vehicle = 0; vehicle < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle) {
                    Veh &record = Vehs[vehicle];
                    const bool selected = button == 0xCF7
                        ? static_cast<uint8_t>(record.unk_1) == report_faction_id_
                            && record.proto_id == prototype_id
                        : record.faction_id == report_faction_id_
                            && record.proto_id == prototype_id;
                    if (!selected || veh_jail(vehicle) != 0) continue;
                    if (button == 0xCF7) {
                        record.order = 0;
                        record.state &= 0xFCFFBDFF;
                        record.waypoint_count = 0;
                    } else {
                        record.order_auto_type = 0;
                        record.state &= 0xFCFFBFFF;
                        record.order = 0;
                        record.state |= 0x200;
                        record.waypoint_count = 0;
                    }
                    synch_veh(vehicle);
                }
                diplo_unlock();
            }
        }
        ReportWinGlobal.redraw();
    }
    if (button >= 2 && button <= 8) {
        ReportIfGlobal.field_8_ = button;
        if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5)
            ReportIfGlobal.report(button, ReportIfGlobal.report_faction_id_, 0);
        else {
            Win &modal_window = ReportWinGlobal;
            modal_window.set_modal(0, pop_wait, &MainInterfaceGlobal);
        }
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
// ORIGINAL: 0x004A6D70 ?on_iface_group_clicked@ReportIf@@QAEXHHH@Z 0x004A6D70-0x004A6FD7 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: src/recovered/units/004a6d70.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a6d70 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's group callback from the retained seed through its genuine SubInterface owner and ReportWin controls. Its current emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// symbol    ?on_iface_group_clicked@ReportIf@@UAEXHHH@Z
void ReportIf::on_iface_group_clicked(int group, int, int item) {
    if (group == 2) {
        switch (item) {
        case 0x92:
            field_1E1D8_ = 1;
            goto redraw_tail;
        case 0x93:
            field_1E1D8_ = 0;
            goto redraw_tail;
        default:
            goto map_mode;
        }
    }
    if (group == 4) {
        if (item == 0xD0B) {
            flatButton5_.hide(); flatButton6_.hide(); flatButton7_.hide();
            flatButton8_.hide(); flatButton9_.hide(); flatButton10_.hide();
            flatButton11_.hide();
            goto redraw_tail;
        }
        flatButton5_.show(0); flatButton6_.show(0); flatButton7_.show(0);
        flatButton8_.show(0); flatButton9_.show(0); flatButton10_.show(0);
        flatButton11_.show(0);
    } else if (group == 0x6A) {
        ReportWinGlobal.flatButton9_.reset_field_A9C_if_ungrouped();
        ReportWinGlobal.flatButton9_.Win::hide();
    } else if (group == 0xCE) {
        ReportWinGlobal.flatButton9_.Win::show(0);
    }
map_mode:
    if (item == 0x3EE) WorldClimateWorldWin.set_world_map();
    else if (item == 0x3EF) WorldClimateWorldWin.set_detail_map();
redraw_tail:
    if (field_1E234_ == 0) {
        on_redraw();
        MainInterfaceGlobal.soft_update();
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004A7000 0x004A8430 0x004A8440 0x004A7D12
// ORIGINAL: 0x004A7000 ?draw_labs@ReportIf@@QAEXXZ 0x004A7000-0x004A842F
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004a7000.cpp", "source:src/reportif.h", "source:src/base.h", "source:src/tech.h"], "reason": "Original panel identity and accesses identify ReportIf's laboratory renderer, using its lists and selection state, source-owned base rows and technology records; preserve the seed's row IDs and technology sentinel rather than substitute a different panel."}
// TRIED: MISMATCH #0, sim ~0.58 (best of /O1 framed). Full structural transcription (all branches, both list modes, all 4 category bars). Approximated: the ListBox virtual-base (GraphicWin) adjustment at self+0xa384/+0x24/+0x28 uses the flattened `virtual_base_` member instead of the runtime vtable+8 lookup the original performs, so that handful of instructions differ in shape though not in intent.
void ReportIf::draw_labs() {
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    flatButton1_.show(0);
    flatButton2_.show(0);

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + 26;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);

    field_10_ += 32;
    field_18_ -= 32;
    draw_workspace_rect_.left = field_C_ + 3;
    draw_workspace_rect_.top = field_10_ + 3;
    draw_workspace_rect_.right = field_C_ + field_14_ - 3;
    draw_workspace_rect_.bottom = field_10_ + field_18_ - 3;
    scroll_small();
    // The complete-object ListBox owns its compiler-adjusted virtual bases.
    ListBox &list = listBox1_;
    list.close();
    Dialog &dialog = static_cast<Dialog &>(list);
    dialog.init(&draw_workspace_rect_, 0);
    dialog.field_48_ = field_1C_ + 3;
    dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    dialog.set_dialog_text_color(0x99, -1, 1, 1);
    dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    list.set_report_text_color(0xF2);
    list.set_write_callback(labs_write);
    char number[80];
    if (field_1E1D8_ == 0) {
        for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
            if (Bases[base].faction_id_current != report_faction_id_) continue;
            StringTemp[0] = 0;
            say_base(StringTemp, base);
            strcat(StringTemp, "|"); strcat(StringTemp, "+");
            const int labs = Bases[base].labs_total;
            _itoa(labs, number, 10); strcat(StringTemp, number);
            strcat(StringTemp, " "); strcat(StringTemp, "(");
            int research = PlayersData[report_faction_id_].soc_effect_pending.values_[SocialEffect::RESEARCH];
            if (research < -5) research = -5;
            else if (research > 5) research = 5;
            const int scaled = (research + 10) * labs * 10;
            _itoa(scaled / 100, number, 10); strcat(StringTemp, number);
            if (scaled % 100 != 0) {
                strcat(StringTemp, ".");
                _itoa((scaled % 100) / 10, number, 10); strcat(StringTemp, number);
            }
            strcat(StringTemp, ")"); strcat(StringTemp, label_get(0x17B));
            list.item(StringTemp, base + 0x400);
        }
    } else {
        int levels[MaxTechnologyNum + 1];
        for (int tech = 0; tech < MaxTechnologyNum; ++tech)
            levels[tech] = has_tech(tech, report_faction_id_)
                ? tech_category(tech) + tech_recurse(tech, 0) * 4 : 0;
        levels[MaxTechnologyNum] = 999999;
        for (int iteration = 0; iteration < MaxTechnologyNum; ++iteration) {
            int best = MaxTechnologyNum;
            for (int candidate = 0; candidate < MaxTechnologyNum; ++candidate)
                if (levels[candidate] != 0 && levels[candidate] < levels[best]) best = candidate;
            if (best < MaxTechnologyNum) {
                StringTemp[0] = 0;
                say_tech(StringTemp, best, 0);
                strcat(StringTemp, "|"); strcat(StringTemp, "[");
                strcat(StringTemp, label_get(0x275 + (levels[best] & 3)));
                strcat(StringTemp, " ");
                _itoa(levels[best] >> 2, number, 10); strcat(StringTemp, number);
                strcat(StringTemp, "]");
                list.item(StringTemp, best);
                levels[best] = 0;
            }
        }
    }
    list.reset_report_selection();
    dialog.field_B4_ = 0;
    list.attach(&MainInterfaceGlobal, dialog.window_x_, dialog.window_y_, 0x80020);
    if (field_1E1D8_ == 0) list.set_selected_pos(-1, 1);
    else list.set_selected_id(ReportWinGlobal.shared_research_focus());
    static_cast<GraphicWin &>(list).show(0);
    scroll_normal();
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    const int inset = field_1E230_ ? 76 : 0;
    field_C_ = field_50_.left + inset;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction); CharUpperA(StringTemp);
    strcat(StringTemp, " "); strcat(StringTemp, label_get(0x2B1));
    if (field_1E230_) {
        strcat(StringTemp, ", "); strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA)); strcat(StringTemp, " "); say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    buffer->set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x178)); strcat(StringTemp, ": ");
    _itoa(tech_rate(report_faction_id_), number, 10); strcat(StringTemp, number);
    strcat(StringTemp, "    "); strcat(StringTemp, label_get(0x179)); strcat(StringTemp, ": ");
    _itoa(PlayersData[report_faction_id_].tech_accumulated, number, 10); strcat(StringTemp, number);
    if (field_1E230_ == 0) {
        MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_, field_14_);
        field_10_ += field_1C_;
        StringTemp[0] = 0;
    } else strcat(StringTemp, "    ");
    strcat(StringTemp, label_get(0x17A)); strcat(StringTemp, ": ");
    energy_compute(report_faction_id_, 0);
    _itoa(EnergyLabsTotal / 100, number, 10); strcat(StringTemp, number);
    if (EnergyLabsTotal % 100 != 0) {
        strcat(StringTemp, ".");
        _itoa((EnergyLabsTotal % 100) / 10, number, 10); strcat(StringTemp, number);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_, field_14_);
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    buffer->set_font(field_1E184_, field_1E18C_, field_1E198_, 0);
    draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_30_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0; strcat(StringTemp, "[");
    const int theory = PlayersData[report_faction_id_].theory_of_everything;
    if (theory == 0) {
        buffer->set_text_color3(0xE1, -1, 1, 1);
        strcat(StringTemp, label_get(0x2C0));
    } else {
        buffer->set_text_color3(mandate_color(1), -1, 1, 1);
        say_tech(StringTemp, PlayersData[report_faction_id_].tech_id_researching, 0);
        CharUpperA(StringTemp); strcat(StringTemp, " ");
        _itoa(theory, number, 10); strcat(StringTemp, number);
    }
    strcat(StringTemp, "]");
    field_18_ -= 4; field_10_ += 4;
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_, field_14_);
    field_18_ -= field_1C_; field_10_ += field_1C_;
    buffer->set_text_color3(0xE1, -1, 1, 1);
    field_18_ -= 2; field_C_ += 3;
    const int bar_height = static_cast<int>(field_18_) / 4 - 15;
    field_14_ -= 6;
    int total[4] = {0, 0, 0, 0};
    int owned[4] = {0, 0, 0, 0};
    // The original pre-requisite cursor stops before Transcendent Thought.
    for (int technology = 0; technology < MaxTechnologyNum - 1; ++technology) {
        if (Technology[technology].preq_tech_1 == -2 || Technology[technology].preq_tech_2 == -2) continue;
        const int category = tech_category(technology);
        if (category >= 0 && category <= 3) {
            ++total[category];
            if (has_tech(technology, report_faction_id_)) ++owned[category];
        }
    }
    field_10_ += 8;
    for (int category = 0; category < 4; ++category) {
        StringTemp[0] = 0;
        draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_;
        draw_workspace_rect_.bottom = field_10_ + 15;
        buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[38 + category * 3]);
        // No zero-denominator guard exists in the original technology bars.
        draw_workspace_rect_.right = field_C_ + static_cast<int>(field_14_) * owned[category] / total[category];
        buffer->set_clip(&draw_workspace_rect_);
        draw_workspace_rect_.right = field_C_ + field_14_;
        buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[36 + category * 3]);
        buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
        field_10_ += 15;
        buffer->set_text_color3(mandate_color(category), -1, 1, 1);
        strcat(StringTemp, "["); strcat(StringTemp, label_get(0x209 + category)); strcat(StringTemp, "   ");
        _itoa(owned[category] * 100 / total[category], number, 10); strcat(StringTemp, number);
        strcat(StringTemp, "%"); strcat(StringTemp, "]");
        buffer->write_l(StringTemp, field_C_ + 2,
            static_cast<int>(field_10_) + (bar_height - static_cast<int>(field_1C_)) / 2, strlen(StringTemp));
        if (category != 3) field_10_ += bar_height;
    }
}

// ORIGINAL: 0x004A8440 ?labs_write@@YAXPAUGraphicWin@@HHPAURECT@@PAD@Z 0x004A8440-0x004A846E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004a8440.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a8440 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage", "Retired recovered seed provenance: git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004a8440.cpp"], "reason": "Recover the ReportIf technology-label writer from its retained seed through the genuine ReportWin/Buffer relationship. Its current emitted symbol is recorded separately from the historical catalogue spelling; this binding adds no claim about executable return decoration. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
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

// ORIGINAL: 0x004A84A0 ?draw_energy@ReportIf@@QAEXXZ 0x004A84A0-0x004A9017
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004a84a0.cpp", "source:src/reportif.h", "image:0x004A84AE-0x004A84BC pushes mode zero then report faction for the cdecl energy_compute call"], "reason": "Original energy-panel identity maps to ReportIf's list, selection, frame and facility rows. The original cdecl push order establishes energy_compute(report_faction_id_, 0), correcting the seed's reversed displayed arguments without changing the calculation owner."}
// TRIED: raw self+offset transcription with manual ListBox/Dialog virtual-base adjustor arithmetic (vtable[+8]/[+4] thunks) for both energy-facility and support-facility ListBox blocks diverges at instr #8 (mov vs lea); not chased further given the 2935-byte, dual-listbox scope.
void ReportIf::draw_energy() {
    energy_compute(report_faction_id_, 0);
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);

    StringTemp[0] = 0;
    strcat(StringTemp, "["); strcat(StringTemp, label_get(0xBE));
    strcat(StringTemp, " "); strcat(StringTemp, label_get(0x2E6)); strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.right = field_14_ + field_C_;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.bottom -= 3;
    scroll_small();
    ListBox &list = listBox1_;
    list.close();
    Dialog &dialog = static_cast<Dialog &>(list);
    dialog.init(&draw_workspace_rect_, 0);
    dialog.field_48_ = field_1C_ + 3;
    dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    dialog.set_dialog_text_color(0x99, -1, 1, 1);
    dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    list.set_report_text_color(0xF2);
    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        if (Bases[base].faction_id_current != report_faction_id_) continue;
        char number[80];
        const int income = Bases[base].economy_total;
        const int maintenance = EnergyBaseMaintenance[base];
        StringTemp[0] = 0;
        say_base(StringTemp, base);
        strcat(StringTemp, "|"); strcat(StringTemp, "+");
        _itoa(income, number, 10); strcat(StringTemp, number);
        strcat(StringTemp, "/"); strcat(StringTemp, "-");
        _itoa(maintenance, number, 10); strcat(StringTemp, number);
        strcat(StringTemp, " = ");
        if (income >= maintenance) strcat(StringTemp, "+");
        _itoa(income - maintenance, number, 10); strcat(StringTemp, number);
        strcat(StringTemp, label_get(0x17B));
        list.item(StringTemp, base + 0x400);
    }
    list.reset_report_selection();
    dialog.field_B4_ = 0;
    list.attach(&MainInterfaceGlobal, dialog.window_x_, dialog.window_y_, 0x80020);
    if (ReportWinGlobal.field_59B0_) {
        if (ReportWinGlobal.shared_report_selection() >= 0x400)
            list.set_selected_id(ReportWinGlobal.shared_report_selection());
        else list.set_selected_pos(0, 1);
    } else list.set_selected_pos(-1, 1);
    static_cast<GraphicWin &>(list).show(0);
    scroll_normal();
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    const int inset = field_1E230_ ? 76 : 0;
    field_C_ = field_50_.left + inset;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction); CharUpperA(StringTemp);
    strcat(StringTemp, " "); strcat(StringTemp, label_get(0x2B2));
    if (field_1E230_) {
        strcat(StringTemp, ", "); strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA)); strcat(StringTemp, " "); say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    buffer->set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x39)); strcat(StringTemp, ": ");
    char number[80];
    _itoa(PlayersData[report_faction_id_].energy_reserves, number, 10); strcat(StringTemp, number);
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_, field_14_);
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0;
    strcat(StringTemp, "["); strcat(StringTemp, label_get(0x3C2)); strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3; draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.top += 3; draw_workspace_rect_.bottom -= 3;
    scroll_small();
    listBox2_.close();
    Dialog &fac_dialog = static_cast<Dialog &>(listBox2_);
    fac_dialog.init(&draw_workspace_rect_, 0);
    fac_dialog.field_48_ = field_1C_ + 3;
    fac_dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    fac_dialog.set_dialog_text_color(0x99, -1, 1, 1);
    fac_dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    fac_dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    listBox2_.set_report_text_color(0xF2);
    for (int facility = 0; facility < 70; ++facility) {
        const int count = EnergyFacilityCounts[facility];
        // Zero counts skip this row, not the remainder of the table.
        if (count == 0) continue;
        StringTemp[0] = 0;
        _itoa(count, number, 10); strcat(StringTemp, number); strcat(StringTemp, " ");
        strcat(StringTemp, StringTable.get(Facility[facility].name));
        strcat(StringTemp, "|"); strcat(StringTemp, "-");
        const int project_base = GameDataStateStorage.fields_.secret_project_.planetary_energy_grid();
        if (project_base >= 0 && Bases[project_base].faction_id_current == report_faction_id_) {
            const int half = fac_maint(facility, report_faction_id_) * count;
            const int cost = fac_maint(facility, report_faction_id_) * count;
            _itoa(cost / 2, number, 10); strcat(StringTemp, number);
            if (half & 1) strcat(StringTemp, ".5");
        } else {
            _itoa(fac_maint(facility, report_faction_id_) * count, number, 10);
            strcat(StringTemp, number);
        }
        strcat(StringTemp, label_get(0x17B));
        listBox2_.item(StringTemp, facility);
    }
    listBox2_.reset_report_selection();
    fac_dialog.field_B4_ = 0;
    listBox2_.attach(&MainInterfaceGlobal, fac_dialog.window_x_, fac_dialog.window_y_, 0x80020);
    if (ReportWinGlobal.field_59B0_) listBox2_.set_selected_pos(-1, 1);
    else listBox2_.set_selected_id(ReportWinGlobal.shared_report_selection());
    static_cast<GraphicWin &>(listBox2_).show(0);
    scroll_normal();
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
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
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004a9050.cpp", "source:src/reportif.h", "source:src/veh.h"], "reason": "Original operations-panel identity belongs to ReportIf's selection and list state; original indexed accesses select source-owned vehicle and prototype records, with the same row filtering and interface controls rather than a frame-only substitute."}
// TRIED: mechanical transcription of the Ghidra decompilation with fields reached by raw `(char*)this+OFFSET` casts (ReportIf's 165-member header is not pinned against this image, so no named struct is used). All ~26 unresolved-by-name FUN_ calls were re-derived from the disassembly's own `; ?mangled@Class` comments (set_clip/box_sprite/set_font/set_text_color/ write_cent_l on the fixed Buffer at 0x7aec64, close/init/ item/attach/set_selected_id/set_dialog_font/set_dialog_text_ color[23] on the ListBox+Dialog virtual-base pair at this+0xa2d0 via the scaffold's own `dialog_`/adjustor fields, Strings::get on the fixed 0x9b90d8, set_base/base_compute/ say_base/say_year/veh_at/scroll_normal/scroll_small already declared) rather than guessed. The ten `(**(code**)(EXPR+4)) (0)` vtable dispatches were rewritten as `reinterpret_cast<VCall*>(ADDR)->slot001(0)` per the brief's one-level-cast rule, where ADDR is the object whose first 4 bytes equal Ghidra's EXPR (confirmed against the raw `mov eax,[obj]; call [eax+4]` shape at several call sites). Did not chase the prologue-class mismatch (#0, push-vs-sub) given the size of this function.
void ReportIf::draw_ops() {
    Buffer &canvas = *static_cast<Buffer *>(&MainInterfaceGlobal);
    if (report_page_history_[1] != 4 && report_page_history_[1] != 5) {
        WorldClimateWorldWin.show_all();
        MainInterfaceGlobal.flat_button_[6].show(0);
        MainInterfaceGlobal.flat_button_[7].show(0);
    }
    canvas.set_clip(&field_20_);
    field_C_ = field_20_.left + 9;
    field_10_ = field_20_.top + 40;
    field_14_ = field_20_.right - field_20_.left - 18;
    field_18_ = field_20_.bottom - field_20_.top - 46;
    draw_workspace_rect_.left = field_20_.left + 6;
    draw_workspace_rect_.top = field_20_.top + 6;
    draw_workspace_rect_.right = field_20_.right - 6;
    draw_workspace_rect_.bottom = field_20_.top + 32;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    flatButton3_.show(0);
    flatButton4_.show(0);
    if (buttonGroup2_.field_84_ == 0xD0B) {
        scroll_small();
        draw_workspace_rect_.left = field_C_;
        draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_;
        draw_workspace_rect_.bottom = field_10_ + field_18_;
        listBox1_.close();
        Dialog &dialog = static_cast<Dialog &>(listBox1_);
        dialog.init(&draw_workspace_rect_, 0);
        dialog.field_48_ = field_1C_ + 3;
        dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
        dialog.set_dialog_text_color(0x99, -1, 1, 1);
        dialog.set_dialog_text_color2(0x99, -1, 1, 1);
        dialog.set_dialog_text_color3(0x99, -1, 1, 1);
        listBox1_.set_report_text_color(0xF2);
        const int labels[8] = {0xA0/4, 0xA4/4, 0xE4/4, 0xAAC/4,
                               0xAB0/4, 0xAB4/4, 0xFC/4, 0xAB8/4};
        // Preserve eight distinct base-computation passes and first-base
        // tie selection. Negative values still select the first own base.
        for (int row = 0; row < 8; ++row) {
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(labels[row]));
            if (row < 2) CharUpperA(StringTemp);
            strcat(StringTemp, ": ");
            int winner = -1;
            int best = 0;
            for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (static_cast<int>(Bases[base].faction_id_current) != report_faction_id_) continue;
                set_base(base);
                base_compute(0);
                int value;
                switch (row) {
                case 0: value = BaseCurrent->mineral_surplus; break;
                case 1: value = BaseCurrent->nutrient_surplus; break;
                case 2: value = BaseCurrent->economy_total; break;
                case 3: value = BaseCurrent->labs_total; break;
                case 4: value = BaseCurrent->psych_total; break;
                case 5: value = BaseCurrent->population_size; break;
                case 6: {
                    int vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);
                    value = 0;
                    if (vehicle >= 0) {
                        while (Vehs[vehicle].prev_veh_id_stack >= 0)
                            vehicle = Vehs[vehicle].prev_veh_id_stack;
                        while (vehicle >= 0) {
                            ++value;
                            vehicle = Vehs[vehicle].next_veh_id_stack;
                        }
                    }
                    break;
                }
                default: {
                    value = 0;
                    // Native4A99A4..4A99EA explicitly counts bits0..69;
                    // there is no facility-prerequisite or reputation filter.
                    for (int facility = 0; facility < 70; ++facility)
                        if (BaseCurrent->facilities_built[facility / 8] & (1 << (facility % 8))) ++value;
                    for (int project = 0; project < 64; ++project)
                        if (base_project(project) == base) value += 5;
                    break;
                }
                }
                if (value > best || winner == -1) {
                    winner = base;
                    best = value;
                }
            }
            strcat(StringTemp, "|");
            // Original calls say_base even when winner==-1. Do not silently
            // remove or guard its original no-base/error path here.
            say_base(StringTemp, winner);
            listBox1_.item(StringTemp, row + 1);
        }
        listBox1_.reset_report_selection();
        dialog.field_B4_ = 0;
        listBox1_.attach(&MainInterfaceGlobal, dialog.window_x_, dialog.window_y_, 0x80020);
        listBox1_.set_selected_id(ReportWinGlobal.shared_report_selection());
        static_cast<GraphicWin &>(listBox1_).show(0);
        scroll_normal();
        canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
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
    canvas.set_clip(&field_50_);
    field_10_ = field_50_.top;
    const int inset = field_1E230_ != 0 ? 76 : 0;
    field_C_ = field_50_.left + inset;
    field_14_ = field_50_.right - inset - field_50_.left;
    field_18_ = field_50_.bottom - field_50_.top;
    StringTemp[0] = 0;
    canvas.set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    canvas.set_text_color(0xE1, -1, 1, 1);
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xACC/4));
    if (field_1E230_) {
        strcat(StringTemp, ", ");
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0xB68/4));
        strcat(StringTemp, " ");
        say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    canvas.set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    canvas.set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xE4/4));
    strcat(StringTemp, ": ");
    char number[80];
    _itoa(PlayersData[report_faction_id_].energy_reserves, number, 10);
    strcat(StringTemp, number);
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_, field_14_);
    canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
    canvas.set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    canvas.box_sprite(&field_30_, &InterfaceBoxStyles[33]);
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    InflateRect(&draw_workspace_rect_, 3, 3);
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    InflateRect(&draw_workspace_rect_, -3, -3);
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    canvas.set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0xF0C/4));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
    StringTemp[0] = 0;
    MainInterfaceGlobal.set_date(StringTemp);
}

// ORIGINAL: 0x004A9F90 ?close_ops@ReportIf@@QAEXXZ 0x004A9F90-0x004AA0AF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004a9f90.cpp", "source:src/reportif.h", "source:src/reportwin.h", "source:src/basebutton.h"], "reason": "Close the ReportIf and ReportWin operation controls through their named owners, preserve conditional map cleanup, and clear only ungrouped ReportWin BaseButton state through an encapsulated helper. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
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
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004aa0b0.cpp", "source:src/reportif.h", "source:src/base.h", "source:src/stringlist.h"], "reason": "Retained spans identify ReportIf's project renderer and its cleanup code. A real local StringList owns the selected-project description; source-owned project/base records, member rectangles and controls express the original panel relationships. Empty scratch-string starts do not establish their original bounds."}
// TRIED: full transcription landed (MISMATCH #0: original has an SEH frame - push -1/handler/fs:[0] - that this body does not reproduce). A local `StringStruct localList(0);` reproduces the constructor call, but StringStruct has no declared destructor here so no unwind funclet gets generated; the two inlined "walk head_/entry_count_, adjustor-thunk release each entry" blocks are called as `localList.remove_all()` instead of reproduced byte-for-byte. Caught one real Ghidra error: find_font is `int find_font(int,int)` (2 args, per its mangled name @@YAHHH@Z has only two H's after the this-less Y); Ghidra printed it with 3 args by misattributing an outer accumulated push meant for the following set_font call.
void ReportIf::draw_projects() {
    StringList description;
    if (report_page_history_[1] != 4 && report_page_history_[1] != 5) {
        WorldClimateWorldWin.show_all();
        MainInterfaceGlobal.flat_button_[6].show(0);
        MainInterfaceGlobal.flat_button_[7].show(0);
    }
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;

    // The seed draws a narrow title frame followed by a second frame around
    // the selected project's description.  Both use the shared work rectangle.
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);

    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.bottom -= 3;
    buffer->set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x32A));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, draw_workspace_rect_.left, field_10_ + 4,
                         field_14_);

    int project = ReportWinGlobal.shared_report_selection();
    if (project >= 0x400)
        project = -70 - Bases[project - 0x400].queue_production_id[0];
    if (project == -1) {
        if (ReportWinGlobal.shared_report_selection() == -1) {
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0x32B));
            buffer->set_text_color(0xE1, -1, 1, 1);
            buffer->wrap(StringTemp, draw_workspace_rect_.left,
                draw_workspace_rect_.top,
                draw_workspace_rect_.right - draw_workspace_rect_.left);
        }
    } else {
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
            const int width = draw_workspace_rect_.right - draw_workspace_rect_.left;
            if (draw_workspace_rect_.bottom - draw_workspace_rect_.top
                < buffer->write_strings_height(&description, width, 0)) {
                Font *italic = find_font(10, 1);
                Font *bold = find_font(10, 2);
                Font *normal = find_font(10, 0);
                buffer->set_font(normal, bold, italic, 0);
            }
            buffer->set_text_color(0xE1, -1, 1, 1);
            buffer->write_strings(&description, draw_workspace_rect_.left,
                draw_workspace_rect_.top, width, 0);
        }
    }

    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    field_10_ = field_50_.top;
    const int inset = field_1E230_ ? 0x4C : 0;
    field_C_ = field_50_.left + inset;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    buffer->set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2B4));
    if (field_1E230_) {
        strcat(StringTemp, ", ");
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, " ");
        say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    buffer->set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());

    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    buffer->box_sprite(&field_30_, &InterfaceBoxStyles[33]);
    draw_workspace_rect_.left = field_C_ - 3;
    draw_workspace_rect_.top = field_10_ - 3;
    draw_workspace_rect_.right = field_C_ + field_14_ + 3;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 9;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.bottom -= 3;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x3C3));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    StringTemp[0] = 0;
    MainInterfaceGlobal.set_date(StringTemp);
}

// ORIGINAL: 0x004AA930 ?close_projects@ReportIf@@QAEXXZ 0x004AA930-0x004AA964 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:4097a7c0:src/recovered/004aa930.cpp;source:src/reportif.h;source:src/worldwin.h;source:src/maininterface.h;source:src/flatbutton.h"], "reason": "Close the project report overlay by its real WorldWin and MainInterface owners when the active report is not one of the two map categories."}
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
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004aa970.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "Retained original identity maps to ReportIf's satellite panel, including its member rectangles and ReportWin help-selection control. Original scratch addresses are empty-string source starts, not writable destination arrays with inferred bounds."}
// TRIED: raw offset casts on `this` (opaque ReportIf class, no fields declared) mirroring the two near-identical D3C-mode/else-mode text panels and the button-creation loop; matches 10 instructions before diverging. The per-faction Players-array index scaling (idx*0x167 for byte-offset 0x304, matching src/faction.h's adj_name_faction) reused from 0x0053D360's introduce().
void ReportIf::draw_sat() {
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    const bool help = ReportWinGlobal.buttonGroup2_.field_84_ == 0xD3C;
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + (help ? field_18_ : field_1C_ + 6);
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    if (help) {
        field_C_ += 4; field_10_ += 4; field_14_ -= 8;
        for (int faction = 1; faction < 8; ++faction)
            if (faction != ConsoleGlobal.faction_id_) flatButtonsC_[faction-1].hide();
        char paragraph[1024], line[256];
        strcpy(paragraph, ReportSatelliteScratchUpperText);
        strcpy(line, ReportSatelliteScratchUpperLine);
        StringTemp[0] = 0; strcat(StringTemp, "SAT0");
        if (X_text_open(ScriptPopupFile, StringTemp)) {
            text_close();
        } else {
            buffer->set_font(field_1E184_, 0, 0, 0);
            buffer->wrap_flying(paragraph, field_C_, field_10_, field_14_);
            buffer->set_text_color(0xE1, -1, 1, 1);
            for (;;) {
                char *text = text_get();
                if (!text) { text_close(); break; }
                if (!*text) {
                    if (strlen(paragraph)) buffer->wrap_flying(paragraph);
                    buffer->wrap_flying_flush(); text_close(); break;
                }
                line[0] = 0; parse_string(text, line);
                strcat(paragraph, line); strcat(paragraph, " ");
            }
        }
    } else {
        draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
        draw_workspace_rect_.bottom = field_20_.bottom - 6;
        buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        const int panel_bottom = field_20_.bottom - 6;
        const int previous_bottom = draw_workspace_rect_.bottom;
        draw_workspace_rect_.bottom = panel_bottom;
        draw_workspace_rect_.right = previous_bottom + 2;
        buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        draw_workspace_rect_.left += 3; draw_workspace_rect_.top += 3;
        draw_workspace_rect_.right -= 3; draw_workspace_rect_.bottom -= 3;
        buffer->set_font(field_1E184_, field_1E194_, field_1E198_, 0);
        StringTemp[0] = 0; strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0x3C6)); strcat(StringTemp, "]");
        MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
        for (int other = 1; other < 8; ++other) {
            if (other == ConsoleGlobal.faction_id_) continue;
            int elevators = 0;
            for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base)
                if (Bases[base].faction_id_current == other && has_fac(41, base, 0)) ++elevators;
            const PlayerData &data = PlayersData[other];
            if (data.satellites_[0] + data.satellites_[1] + data.satellites_[2]
                + data.satellites_[3] + elevators)
                flatButtonsC_[other-1].show(0);
            else flatButtonsC_[other-1].hide();
        }
    }
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    const int indent = field_1E230_ ? 76 : 0;
    field_C_ = field_50_.left + indent;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - field_50_.left - indent;
    field_18_ = field_50_.bottom - field_50_.top;
    StringTemp[0] = 0;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp); strcat(StringTemp, " "); strcat(StringTemp, label_get(0x2B5));
    if (field_1E230_) {
        strcat(StringTemp, ", "); strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA)); strcat(StringTemp, " "); say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    buffer->set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6; field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + (help ? field_18_ : field_1C_ + 6);
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    if (help) {
        field_C_ += 4; field_10_ += 4; field_14_ -= 8;
        for (int button = 0; button < 5; ++button) flatButtonsD_[button].hide();
        char paragraph[1024], line[256];
        strcpy(paragraph, ReportSatelliteScratchLowerText);
        strcpy(line, ReportSatelliteScratchLowerLine);
        StringTemp[0] = 0; strcat(StringTemp, "SAT1");
        if (X_text_open(ScriptPopupFile, StringTemp)) {
            text_close();
        } else {
            buffer->set_font(field_1E184_, 0, 0, 0);
            buffer->wrap_flying(paragraph, field_C_, field_10_, field_14_);
            buffer->set_text_color(0xE1, -1, 1, 1);
            for (;;) {
                char *text = text_get();
                if (!text) { text_close(); break; }
                if (!*text) {
                    if (strlen(paragraph)) buffer->wrap_flying(paragraph);
                    buffer->wrap_flying_flush(); text_close(); break;
                }
                line[0] = 0; parse_string(text, line);
                strcat(paragraph, line); strcat(paragraph, " ");
            }
        }
    } else {
        draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
        // The original lower panel deliberately uses the upper panel's bottom.
        draw_workspace_rect_.bottom = field_20_.bottom - 6;
        buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        draw_workspace_rect_.right = draw_workspace_rect_.bottom + 2;
        draw_workspace_rect_.bottom = field_20_.bottom - 6;
        buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        draw_workspace_rect_.left += 3; draw_workspace_rect_.top += 3;
        draw_workspace_rect_.right -= 3; draw_workspace_rect_.bottom -= 3;
        buffer->set_font(field_1E184_, field_1E194_, field_1E198_, 0);
        StringTemp[0] = 0; strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0x3C7)); strcat(StringTemp, "]");
        MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
        field_C_ = field_30_.left + 9; field_10_ = field_30_.top + 32;
        field_14_ = field_30_.right - field_30_.left - 18;
        int target = buttonGroup3_.field_84_;
        if (target < 1 || target >= 8) target = -1;
        if (target != -1) {
            field_1E234_ = 1;
            int selected = buttonGroup4_.field_84_;
            if (selected < 65 || selected >= 70) selected = -1;
            buttonGroup4_.init(0xCE, 0);
            int elevators = 0;
            for (int kind = 0; kind < 5; ++kind) {
                if (kind == 4 && !ExpansionEnabled) { flatButtonsD_[4].hide(); break; }
                StringTemp[0] = 0;
                strcat(StringTemp, StringTable.get(Facility[kind == 4 ? 41 : 65+kind].name));
                draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
                draw_workspace_rect_.right = field_C_ + field_14_;
                draw_workspace_rect_.bottom = field_10_ + 20;
                FlatButton &button = flatButtonsD_[kind];
                button.init(StringTemp, kind + 65, draw_workspace_rect_.left,
                    draw_workspace_rect_.top, field_14_, 20, &MainInterfaceGlobal, 0);
                static_cast<Buffer *>(&button)->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
                button.redraw(); button.hide();
                elevators = 0;
                for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base)
                    if (Bases[base].faction_id_current == target && has_fac(41, base, 0)) ++elevators;
                if (kind == 4 ? elevators != 0 : PlayersData[target].satellites_[kind] != 0) {
                    button.show(0); buttonGroup4_.add(&button); field_10_ += 22;
                }
            }
            if (!ExpansionEnabled) {
                // Original skips selection restoration entirely in this mode.
            } else if (selected != -1 && (selected - 65 == 4 ? elevators != 0
                : selected - 65 < 4 && PlayersData[target].satellites_[selected - 65] != 0)) {
                buttonGroup4_.set(selected, 1);
            } else {
                ReportWinGlobal.flatButton9_.reset_field_A9C_if_ungrouped();
                ReportWinGlobal.flatButton9_.Win::hide();
            }
            field_1E234_ = 0;
        }
    }
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
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
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/004ab550.cpp"], "reason": "Retain the existing source implementation and complete original mapping. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
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

// ORIGINAL: 0x004AB600 ?draw_security@ReportIf@@QAEXXZ 0x004AB600-0x004ABE24
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004ab600.cpp", "source:src/reportif.h", "source:src/spot.h"], "reason": "Retained identity and indexed accesses identify ReportIf's five-category security panel. Its source-owned lists and Spot rows express the original filters, count caps and lost totals; the final frame uses the first panel's bottom edge as in the original, not a geometrically normalized replacement."}
// TRIED: literal register-transliteration (eax/ecx/edx/ebx/edi locals) compiles and is a faithful line-by-line transcription (verified box_sprite/set_font/write_* arg order and the 0x9ab898 flag-byte stride=0x34 against raw disasm, not Ghidra which mis-scaled it to 0x1a); compiler emits a frameless prologue (sub) where original is framed (push ebp/mov ebp,esp) - #0 divergence. Not chased further given size (2084 bytes).
void ReportIf::draw_security() {
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.bottom -= 3;
    buffer->set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x38A));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);

    buffer->set_font(field_1E18C_, 0, 0, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ = draw_workspace_rect_.top + 3;
    field_C_ += 4;
    field_14_ -= 8;
    for (int filter = 0; filter < 5; ++filter) {
        int active = 0, queued = 0, lost = 0;
        buffer->set_text_color(filter == field_1E244_ ? 0xF2 : 0x99, -1, 1, 1);
        for (int row = 0; row < 128; ++row) {
            const int prototype_id = row < 64 ? row : report_faction_id_ * 64 - 64 + row;
            const VehPrototype &prototype = VehPrototypes[prototype_id];
            if (!(prototype.flags & 1)) continue;
            const int offense = Weapon[prototype.weapon_id].offense_rating;
            if (!(filter == 4 || (filter == 3 && offense == 0)
                || (filter < 3 && Chassis[prototype.chassis_id].triad == filter && offense != 0)))
                continue;
            const PlayerData &faction = PlayersData[report_faction_id_];
            if (faction.proto_id_active[prototype_id] < 250)
                active += faction.proto_id_active[prototype_id];
            if (faction.proto_id_queue[prototype_id] < 250)
                queued += faction.proto_id_queue[prototype_id];
            lost += faction.proto_id_lost[prototype_id];
        }
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x38B + filter));
        buffer->write_l(StringTemp, field_C_, field_10_, strlen(StringTemp));
        char number[80];
        StringTemp[0] = 0;
        _itoa(active, number, 10); strcat(StringTemp, number); strcat(StringTemp, "-");
        _itoa(queued, number, 10); strcat(StringTemp, number); strcat(StringTemp, "-");
        _itoa(lost, number, 10); strcat(StringTemp, number);
        buffer->write_right_l(StringTemp, field_C_ + field_14_, field_10_, strlen(StringTemp));
        draw_workspace_rect_.left = field_C_;
        draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_;
        draw_workspace_rect_.bottom = field_10_ + field_1C_;
        spot_.add(filter, 0, draw_workspace_rect_.left, draw_workspace_rect_.top,
            draw_workspace_rect_.right - draw_workspace_rect_.left,
            draw_workspace_rect_.bottom - draw_workspace_rect_.top);
        if (filter == 3) {
            field_10_ += field_1C_ - 1;
            buffer->line(field_C_ - 1, field_10_, field_C_ + field_14_, field_10_, 0x99);
            field_10_ += 5;
        } else field_10_ += field_1C_;
    }

    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    const int inset = field_1E230_ ? 0x4C : 0;
    field_C_ = field_50_.left + inset;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    buffer->set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2B6));
    if (field_1E230_) {
        strcat(StringTemp, ", ");
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, " ");
        say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    buffer->set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    // Original intentionally uses the FIRST panel's bottom for this frame.
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3; draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.top += 3; draw_workspace_rect_.bottom -= 3;
    buffer->set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x3BB));
    strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    flatButton12_.show(0);
    flatButton14_.show(0);
    flatButton13_.show(0);
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
}

// ORIGINAL: 0x004ABE30 ?close_security@ReportIf@@QAEXXZ 0x004ABE30-0x004ABE62 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/004abe30.cpp", "source:src/reportif.h", "source:src/flatbutton.h", "source:src/win.h"], "reason": "Hide the three ReportIf security FlatButtons through Win slot 2 in retained-seed order."}
void ReportIf::close_security() {
    flatButton12_.hide();
    flatButton14_.hide();
    flatButton13_.hide();
}

// ORIGINAL: 0x004ABE70 ?draw_score@ReportIf@@QAEXXZ 0x004ABE70-0x004AC127
// The report renderer uses the ReportIf-owned panel rectangles and fonts,
// MainInterfaceGlobal's Buffer base, and the source-owned faction/string
// tables. The original's two static separators are represented by ordinary
// literals; their bytes at 0x00682820 and 0x00682E90 are respectively " "
// and ", ".
void ReportIf::draw_score() {
    RECT *first_panel = &field_20_;

    MainInterfaceGlobal.Buffer::set_clip(first_panel);
    field_C_ = first_panel->left + 6;
    field_10_ = first_panel->top + 6;
    field_14_ = first_panel->right - first_panel->left - 0xC;
    field_18_ = first_panel->bottom - first_panel->top - 0xC;

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    MainInterfaceGlobal.Buffer::box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    RECT *score_panel = &field_50_;
    MainInterfaceGlobal.Buffer::set_clip(score_panel);
    field_10_ = score_panel->top;
    const int left_inset = field_1E230_ != 0 ? 0x4C : 0;
    field_C_ = score_panel->left + left_inset;
    field_14_ = score_panel->right - left_inset - score_panel->left;
    field_18_ = score_panel->bottom - score_panel->top;

    StringTemp[0] = 0;

    MainInterfaceGlobal.Buffer::set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    MainInterfaceGlobal.Buffer::set_text_color(0xE1, -1, 1, 1);

    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2B7));

    if (field_1E230_ != 0) {
        strcat(StringTemp, ", ");
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, " ");
        say_year(StringTemp);
    }

    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);

    MainInterfaceGlobal.Buffer::set_font(field_1E180_, field_1E180_, field_1E180_, nullptr);
    MainInterfaceGlobal.Buffer::set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    MainInterfaceGlobal.Buffer::set_clip(MainInterfaceGlobal.buffer_working_rect());

    RECT *last_panel = &field_30_;
    MainInterfaceGlobal.Buffer::set_clip(last_panel);
    field_C_ = last_panel->left + 6;
    field_10_ = last_panel->top + 6;
    field_14_ = last_panel->right - last_panel->left - 0xC;
    field_18_ = last_panel->bottom - last_panel->top - 0xC;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
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

// ORIGINAL: 0x004AC140 ?draw_intel@ReportIf@@QAEXXZ 0x004AC140-0x004AC977
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004ac140.cpp", "source:src/reportif.h", "source:src/faction.h", "source:src/tech.h"], "reason": "Retained identity identifies ReportIf's intelligence panel. Original faction-indexed treaty accesses and technology iteration map to source-owned player data and technology records; the informed-view conditions select the same information rather than treating the panel as a link-only placeholder."}
// TRIED: full field-offset transcription off `self`, with the two embedded ListBox virtual-base dispatches done as explicit two-step vbtable-pointer/displacement reads (not flattened pointer arithmetic) and the vtable slot1 call retyped to take an int; compiles, MISMATCH #3 (early - likely the spy-flag bool-to-int store or the `test ecx,ecx` on a provably-nonzero fixed global getting folded away).
void ReportIf::draw_intel() {
    Buffer &canvas = static_cast<Buffer &>(MainInterfaceGlobal);
    const int target = report_selection_;
    const bool informed = (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
        || spying(target) != 0
        || (PlayersData[report_faction_id_].diplo_treaties[target] & 1);
    canvas.set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    canvas.set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0;
    strcat(StringTemp, "["); strcat(StringTemp, label_get(0x175)); strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3; draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.top += 3; draw_workspace_rect_.bottom -= 3;
    scroll_small();
    listBox1_.close();
    Dialog &tech_dialog = static_cast<Dialog &>(listBox1_);
    tech_dialog.init(&draw_workspace_rect_, 0);
    tech_dialog.field_48_ = field_1C_ + 3;
    tech_dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    tech_dialog.set_dialog_text_color(0x99, -1, 1, 1);
    tech_dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    tech_dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    listBox1_.set_report_text_color(0xF2);
    if (informed) {
        for (int tech = 0; tech < 89; ++tech) {
            if (has_tech(tech, target)) {
                // The clear precedes say_tech at4AC3E1, despite seed ordering.
                StringTemp[0] = 0;
                say_tech(StringTemp, tech, 0);
                if (!has_tech(tech, report_faction_id_)) strcat(StringTemp, " (*)");
                listBox1_.item(StringTemp, tech);
            }
        }
    }
    listBox1_.reset_report_selection();
    tech_dialog.field_B4_ = 0;
    listBox1_.attach(&MainInterfaceGlobal, tech_dialog.window_x_, tech_dialog.window_y_, 0x80020);
    static_cast<GraphicWin &>(listBox1_).show(0);
    scroll_normal();
    canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
    canvas.set_clip(&field_50_);
    const int inset = field_1E230_ ? 76 : 0;
    field_C_ = field_50_.left + inset;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    canvas.set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    canvas.set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " "); strcat(StringTemp, label_get(0x2B8));
    if (field_1E230_) {
        strcat(StringTemp, ", "); strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA)); strcat(StringTemp, " ");
        say_year(StringTemp);
    }
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 10, field_14_);
    canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
    canvas.set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0;
    strcat(StringTemp, "["); strcat(StringTemp, label_get(0x176)); strcat(StringTemp, "]");
    MainInterfaceGlobal.Buffer::write_cent_text(StringTemp, field_C_, field_10_ + 4, field_14_);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3; draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.top += 3; draw_workspace_rect_.bottom -= 3;
    scroll_small();
    listBox2_.close();
    Dialog &base_dialog = static_cast<Dialog &>(listBox2_);
    base_dialog.init(&draw_workspace_rect_, 0);
    base_dialog.field_48_ = field_1C_ + 3;
    base_dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    base_dialog.set_dialog_text_color(0x99, -1, 1, 1);
    base_dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    base_dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    listBox2_.set_report_text_color(0xF2);
    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        const Base &record = Bases[base];
        if (record.faction_id_current == target && (informed
            || record.faction_id_current == report_faction_id_
            || (record.visibility & (1 << (report_faction_id_ & 31))))) {
            char number[96];
            StringTemp[0] = 0;
            say_base(StringTemp, base);
            strcat(StringTemp, "|");
            // Original movzx intentionally treats current population as unsigned.
            _itoa(informed ? static_cast<unsigned char>(record.population_size)
                : record.faction_pop_size_intel[report_faction_id_], number, 10);
            strcat(StringTemp, number);
            listBox2_.item(StringTemp, base);
        }
    }
    listBox2_.reset_report_selection();
    base_dialog.field_B4_ = 0;
    listBox2_.attach(&MainInterfaceGlobal, base_dialog.window_x_, base_dialog.window_y_, 0x80020);
    static_cast<GraphicWin &>(listBox2_).show(0);
    scroll_normal();
    canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
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

// ORIGINAL: 0x004ACC30 ?merge_sort@ReportIf@@QAEXHH@Z 0x004ACC30-0x004ACD98
// RECOVERY_BINDING: v1 {"evidence": ["Retired recovered seed provenance: git:4097a7c0:src/recovered/units/004acc30.cpp;source:src/reportif.h;source:src/base.h:BaseCurrentCount;source:src/general.h:swap"], "reason": "Recover the ReportIf base-ranking merge sort over its measured 0x200-entry, 16-byte scratch workspace, using the source-owned BaseCurrentCount bound and preserving original range normalization and key-prefetch order."}
void ReportIf::merge_sort(int first, int last) {
    const int maximum = GameDataStateStorage.fields_.base_current_count_ - 1;
    if (first < 0 || maximum < 0) {
        first = 0;
    } else if (first > maximum) {
        first = maximum;
    }

    int high = maximum;
    if (last < 0 || maximum < 0) {
        high = 0;
    } else if (last <= high) {
        high = last;
    }

    if (first == high) return;

    int end = high;
    if (high < first) swap(&first, &end);

    const int middle = (end + first) / 2;
    merge_sort(first, middle);
    int right = middle + 1;
    merge_sort(right, end);

    int left = first;
    int output = first;
    // Preserve the original unchecked key prefetch: it reads both cursors'
    // values before testing which half is exhausted, including an endpoint
    // read adjacent to the active range. Do not turn this into a guarded,
    // conventional merge without a separate behavior decision.
    while (output <= end) {
        int selected;
        if (sort_entries_[left].value < sort_entries_[right].value) {
            if (right <= end) {
                selected = right++;
            } else {
                selected = left++;
            }
        } else if (left <= middle) {
            selected = left++;
        } else {
            selected = right++;
        }
        sort_entries_[output].sorted_base_id = sort_entries_[selected].base_id;
        sort_entries_[output].sorted_value = sort_entries_[selected].value;
        ++output;
    }

    for (int i = first; i <= end; ++i) {
        sort_entries_[i].base_id = sort_entries_[i].sorted_base_id;
        sort_entries_[i].value = sort_entries_[i].sorted_value;
    }
}

/*
Purpose: Construct the SubInterface base, then every
         sub-object in image order - seven FlatButton runs, three Sprite
         runs, two ListBoxes, four ButtonGroups and fourteen individual
         FlatButtons, and a Spot - before installing ReportIf's own vftable.
// ORIGINAL: 0x004AD170 ??0ReportIf@@QAE@XZ 0x004AD170-0x004AD3AA;0x00658F80-0x00659134
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
