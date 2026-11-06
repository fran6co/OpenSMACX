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
/*
Purpose: Step the receiver back to the subobject ??_GMainInterface@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0045F1F0 ??_GMainInterface@@WEEE@AEPAXI@Z 0x0045F1F0-0x0045F1FB BYTE_EXACT
// symbol    ??_EMainInterface@@WEEE@AEPAXI@Z
// CORRECTED from ??3MainInterface@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0045F1C0` into
//   ??_GMainInterface@@UAEPAXI@Z, which executes `ret 4`; no stack
//   access and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// ORIGINAL: 0x0045F1C0 ??_GMainInterface@@UAEPAXI@Z 0x0045F1C0-0x0045F1E1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x0045F1C0-0x0045F1E1", "Pinned class constructor ??0MainInterface@@QAE@XZ at 0x0045EF10 stores original class/view table VA 0x66c598 at 0x0045F196 (mov dword ptr [esi], 0x66c598); the current genuine owner vftable ??_7MainInterface@@6BWin@@@ selects generated ??_GMainInterface@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EMainInterface@@UAEPAXI@Z (mode 2).", "Pinned ??_GMainInterface@@UAEPAXI@Z at 0x0045F1C0 calls the source-mapped ??1MainInterface@@QAE@XZ at 0x0045EB80, then tests byte ptr [ebp + 8], 1; je 0x45f1da bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/maininterface.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original MainInterface scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GMainInterface@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "interface_fonts.h"
#include "interface_palette.h"
#include "maininterface.h"
#include "messagewin.h"
#include "worldwin.h"
#include "graphicwin.h"
#include "filewin.h"
// Owner operations use the shared year, base, vehicle and faction interfaces.
#include "game.h"
#include "base.h"
#include "map.h"  // WorldClimateSkipTerrainClear
#include "statuswin.h"
#include "infowin.h"
#include "console.h"   // ConsoleStatusWin, 0x008C5568
#include "alpha.h"     // label_get, the Labels.strings_ptr walk
#include "veh.h"
#include "faction.h"
#include "multiwin.h"
#include "reportif.h"
#include "mapwin.h"
#include "terraforming.h"
#include "netdaemon.h"
#include "wave.h"
#include "strings.h"

// ORIGINAL: 0x00459560 _$E21 0x00459560-0x00459576 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682534 selects initializer0x459560..0x459576 and registers exit0x459580..0x45958a; complete independent decode establishes both ends.", "Original operations identify storage0x7ae820, source object?MainInterfaceGlobal@@3VMainInterface@@A. MainInterface uses constructor45EF10/destructor45EB80; FarmMap uses virtual-base constructor4626E0 with hidden most-derived argument1 and exit receiver7F5ACC calling own-part420F90 then virtual GraphicWin5D4DD0.", "Historical pre-transport physical mapwin.cpp COFF selected the MainInterfaceGlobal initializer/finalizer and established its prior compiler-generated callbacks. After this move the emitting source is src/maininterface.cpp; native COFF qualification and the MainInterface-before-FarmMap CRT order require a fresh coordinator measurement.", "Current compiler-local symbols interpret present emission, not original debug names or historical TU identity. The bounded two-object ordering does not establish complete CRT order, strict linked preservation, runtime or game acceptance.", "Source-owned MainInterfaceGlobal and its dedicated Wave share src/maininterface.cpp in pinned local startup order. Fresh physical linked evidence qualifies only their order before FarmMapWindow, not complete CRT/runtime or historical TU proof."], "reason": "Keep the pinned MainInterfaceGlobal initializer and exit mapping with the genuine MainInterface source-owned object and compiler-generated lifetime. Preserve its ordering ahead of FarmMapWindow; do not author callbacks or infer a historical filename."}
// symbol    _$E17
// ORIGINAL: 0x00459580 _$E19 0x00459580-0x0045958A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682534 selects initializer0x459560..0x459576 and registers exit0x459580..0x45958a; complete independent decode establishes both ends.", "Original operations identify storage0x7ae820, source object?MainInterfaceGlobal@@3VMainInterface@@A. MainInterface uses constructor45EF10/destructor45EB80; FarmMap uses virtual-base constructor4626E0 with hidden most-derived argument1 and exit receiver7F5ACC calling own-part420F90 then virtual GraphicWin5D4DD0.", "Fresh physical mapwin.cpp COFF selects _$E21 via .CRT$XCU and registers _$E19, both naming the genuine global. The corrected declarations and actual linked table initialize MainInterface before FarmMap, matching original slots682534/682540; C++ owns reverse destruction and virtual-base lifetime.", "Current compiler-local symbols interpret present emission, not original debug names or historical TU identity. The bounded two-object ordering does not establish complete CRT order, strict linked preservation, runtime or game acceptance.", "Source-owned MainInterfaceGlobal and its dedicated Wave share src/maininterface.cpp in pinned local startup order. Fresh physical linked evidence qualifies only their order before FarmMapWindow, not complete CRT/runtime or historical TU proof."], "reason": "Bind original exit to genuine source-owned ?MainInterfaceGlobal@@3VMainInterface@@A compiler-generated lifetime after correcting declaration order; no hand-authored callback or explicit virtual-base flag."}
// symbol    _$E15

// The hit-test rectangle at 0x007AF50C is this object's +0xCEC member;
// MapWin::main_caption also selects its complete receiver at 0x007AE820.
// RECOVERY_OBJECT: v1 0x007AE820 ?MainInterfaceGlobal@@3VMainInterface@@A
// The former independent Console gate identity is this embedded word:
// 0x007AE820 + 0x1EAC8 = 0x007CD2E8. MainInterface::init clears it at
// 0x004596CD and Console's Enter/Space paths read it at 0x00517A0B/0x00517A93.
// Keep its historical unbounded identity without asserting a new extent.
// RECOVERY_OBJECT: v3 0x007CD2E8 ?ConsoleModalState7CD2E8@@3HA member ?MainInterfaceGlobal@@3VMainInterface@@A field_1EAC8_
MainInterface MainInterfaceGlobal;  // 0x007AE820

// ORIGINAL: 0x00459590 _$E46 0x00459590-0x004595A6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682538 selects init 0x00459590, which registers exit callback 0x004595B0.", "Original init span 0x00459590-0x004595A6 and exit span 0x004595B0-0x004595BA are independently disassembled; both identify receiver 0x007D38B8 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E46 via .CRT$XCU, relocating to g_MAININTERFACE_WAVE, Wave construction, paired local fini _$E44 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "Current source correspondence moves the real g_MAININTERFACE_WAVE object and these compiler-owned lifetime identities to src/maininterface.cpp; the earlier wave.cpp COFF observation remains historical and must be freshly requalified after transport.", "Source-owned MainInterfaceGlobal and its dedicated Wave share src/maininterface.cpp in pinned local startup order. Fresh physical linked evidence qualifies only their order before FarmMapWindow, not complete CRT/runtime or historical TU proof."], "reason": "Keep the genuine MainInterface music Wave object and its compiler-generated initialization/cleanup identities with the MainInterface configuration and animation source correspondence; preserve its pinned CRT slot without manual callbacks."}
// symbol    _$E22
// ORIGINAL: 0x004595B0 _$E44 0x004595B0-0x004595BA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682538 selects init 0x00459590, which registers exit callback 0x004595B0.", "Original init span 0x00459590-0x004595A6 and exit span 0x004595B0-0x004595BA are independently disassembled; both identify receiver 0x007D38B8 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E46 via .CRT$XCU, relocating to g_MAININTERFACE_WAVE, Wave construction, paired local fini _$E44 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "Source-owned MainInterfaceGlobal and its dedicated Wave share src/maininterface.cpp in pinned local startup order. Fresh physical linked evidence qualifies only their order before FarmMapWindow, not complete CRT/runtime or historical TU proof."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_MAININTERFACE_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E20

// These start-only identities are witnessed by each pinned init and its
// registered exit callback loading the same receiver for Wave construction
// and destruction. They do not establish full bounds or startup ordering.
// RECOVERY_OBJECT: v1 0x007D38B8 ?g_MAININTERFACE_WAVE@@3VWave@@A
Wave g_MAININTERFACE_WAVE;  // 0x007D38B8

// Scroll animation state: distinct original DWORD starts, not interface/Wave members.
// Allocation extents beyond the observed DWORD accesses remain unverified.
// RECOVERY_OBJECT: v1 0x007D38B0 ?PopupScrollHorizontalStep@@3HA
int PopupScrollHorizontalStep;
// RECOVERY_OBJECT: v1 0x007D3924 ?PopupScrollVerticalFrameCount@@3HA
int PopupScrollVerticalFrameCount;
extern int WindowAnimationFrameDelay;

#include <mmsystem.h>
#include "win.h"
#include "autosound.h"

// Original dword writes/reads in45D170,45C3A0,45CA40 respectively.
// Independently read initial bytes:7D3934/7D3938/7D3944 all zero;
// 68526C is initialized to one. Names retain bounded observed roles.
// RECOVERY_OBJECT: v2 0x007D3934 0x007D3938 ?MainInterfaceStatusRedrawFlag@@3HA 0x0045D170
int MainInterfaceStatusRedrawFlag;
// RECOVERY_OBJECT: v2 0x007D3938 0x007D393C ?MainInterfaceRedrawCount@@3HA 0x0045C3A0
int MainInterfaceRedrawCount;
// RECOVERY_OBJECT: v2 0x007D3944 0x007D3948 ?MainInterfaceTimersStopped@@3HA 0x0045CA40
int MainInterfaceTimersStopped;
// RECOVERY_OBJECT: v2 0x0068526C 0x00685270 ?MainInterfaceEndTurnAnimation@@3HA 0x0045C3A0
int MainInterfaceEndTurnAnimation = 1;
// Original4596C8..6D5 clears twelve pointer slots; its first registration
// is the actual secondary base at this+A14. Mixed derived interfaces are
// registered as SubInterface, not falsely presented as IfaceRegistryEntry.
// RECOVERY_OBJECT: v2 0x007AE7F0 0x007AE820 ?InterfaceRegistry@@3PAPAVSubInterface@@A 0x00459680
// BattleWin::init publishes its interface in slot 10, not independent storage.
// Retain the historical receiver-slot identity without claiming a new extent.
// RECOVERY_OBJECT: v3 0x007AE818 ?BattleWinInitReceiver@@3PAVBattleWin@@A member ?InterfaceRegistry@@3PAPAVSubInterface@@A [10]
SubInterface *InterfaceRegistry[12];
void register_iface_mode(int slot, SubInterface *interface_entry) {
    InterfaceRegistry[slot] = interface_entry;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x0045B550 0x0045BBF8 0x0045BC24 0x0045B720
// RECOVERY_ASSOCIATED: v1 switch_selector 0x0045B550 0x0045BC24 0x0045BC42 0x0045B71A
// ORIGINAL: 0x0045B550 ?draw_edit_pane@MainInterface@@QAEXXZ 0x0045B550-0x0045BBF7 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/maininterface.h", "git:b4601390:recovery/tasks/maininterface-owner-coverage.json", "git:f7bbce35:src/unrecovered/0045b550.cpp"], "reason": "Original owner calls, secondary interface receiver and ordinary embedded-member lifecycle identify this MainInterface operation; compatible historical seed spans are retained, without a matching claim."}
// Historical seed interpretation below mistook retained font arguments
// for extra find_font arguments; original45B5B7..45B5E5 retains all three.
// TRIED: the 30-way switch is transcribed from the actual jump table (0x0045BBF8) and case table (0x0045BC24) read out of the image with pefile, not hand-decoded, so the VALUES it produces should be right even though the emitted `cmp`/`jmp` shape won't match a real jump table; find_font's three call sites each push one more dword than the declared 2-int signature consumes (`add esp,8` after three pushes) - the extra value is dropped rather than guessed at.
void MainInterface::draw_edit_pane() {
    if (WinScreenWidth < 1024 || MainInterfaceGlobal.button_group_[0].field_84_ != 1002) return;
    Buffer &canvas = MainInterfaceGlobal;
    RECT rect = editor_text_rect_;
    canvas.set_text_color(TextColourTable[22], TextColourTable[23], 1, 1);
    Font *style_one = find_font(10, 1);
    Font *style_two = find_font(10, 2);
    Font *normal = find_font(10, 0);
    canvas.set_font(normal, style_two, style_one, 0);
    canvas.box_sprite(&rect, &InterfaceBoxStyles[0]);
    const int x = rect.left + 4;
    int y = rect.top + 4;
    char line[300], number[80];
    line[0] = 0;
    strcat(line, label_get(0x6A8 / 4));
    strcat(line, ": "); strcat(line, "{");
    strcat(line, MapFilePath); strcat(line, "}");
    canvas.write_l(line, x, y, strlen(line));
    y += canvas.text_line_height();
    line[0] = 0;
    strcat(line, label_get(0x6A4 / 4)); strcat(line, ": ");
    const char *tool;
    switch (MapEditorPaintId) {
    case 0: tool = label_get(0xEBC / 4); break;
    case 1: tool = label_get(0xEC0 / 4); break;
    case 2: tool = label_get(0xEC4 / 4); break;
    case 3: tool = label_get(0xEC8 / 4); break;
    case 4: tool = label_get(0xECC / 4); break;
    case 5: tool = label_get(0xED0 / 4); break;
    case 26: tool = StringTable.get(Natural[2].name_short); break;
    case 27: tool = StringTable.get(Natural[6].name_short); break;
    case 28: tool = StringTable.get(Natural[3].name_short); break;
    case 29: tool = StringTable.get(Natural[10].name_short); break;
    default: tool = StringTable.get(Terraforming[MapEditorPaintId - 6].name); break;
    }
    strcat(line, tool);
    canvas.write_l(line, x, y, strlen(line));
    y += canvas.text_line_height();
    line[0] = 0;
    strcat(line, label_get(0x684 / 4)); strcat(line, ": ");
    if (MapEditorBrushId == 1) strcat(line, label_get(0x688 / 4));
    else {
        _itoa(MapEditorBrushId == 0 ? 0 : MapEditorBrushId - 1, number, 10);
        strcat(line, number);
    }
    canvas.write_l(line, x, y, strlen(line));
    y += canvas.text_line_height();
    line[0] = 0;
    strcat(line, label_get(0xD7C / 4)); strcat(line, ": ");
    _itoa(MapLongitudeBounds / 2, number, 10); strcat(line, number);
    strcat(line, "x");
    _itoa(MapLatitudeBounds, number, 10); strcat(line, number);
    strcat(line, " ");
    strcat(line, label_get((MapIsFlat & 1 ? 0xD80 : 0xD84) / 4));
    canvas.write_l(line, x, y, strlen(line));
    y += canvas.text_line_height();
    line[0] = 0;
    strcat(line, label_get(0x84 / 4)); strcat(line, ": "); strcat(line, "{");
    // Original45BA7E loads the cloud setting, not the planet-size setting;
    // its indexed label offset is a low-dword addition, without a bounds check.
    uint32_t cloud_label_bits = 0x50u / 4u + static_cast<uint32_t>(MapCloudCover);
    int cloud_label;
    memcpy(&cloud_label, &cloud_label_bits, sizeof(cloud_label));
    strcat(line, label_get(cloud_label)); strcat(line, "}");
    canvas.write_l(line, x, y, strlen(line));
    y += canvas.text_line_height();
    // The original deliberately appends to the preceding cloud-setting line.
    strcat(line, "{");
    int label;
    if (GetKeyState(VK_SCROLL) & 1) label = 0xF50;
    else label = ConsoleGlobal.edit_lock() ? 0x1020 : 0xF54;
    strcat(line, label_get(label / 4)); strcat(line, "}");
    canvas.write_l(line, x, y, strlen(line));
    canvas.text_line_height();
    soft_update(&editor_text_rect_);
}

// ORIGINAL: 0x0045C3A0 ?redraw_complete@MainInterface@@QAEXXZ 0x0045C3A0-0x0045C5AF FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/maininterface.h", "git:b4601390:recovery/tasks/maininterface-owner-coverage.json", "git:f7bbce35:src/recovered/units/0045c3a0.cpp"], "reason": "Original owner calls, secondary interface receiver and ordinary embedded-member lifecycle identify this MainInterface operation; compatible historical seed spans are retained, without a matching claim."}
void MainInterface::redraw_complete() {
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 0
        && MainInterfaceGlobal.field_4_ != 10) return;
    char text[256];
    RECT button_rect;
    MainInterfaceGlobal.set_text_color(0x65, -1, 1, 1);
    text[0] = 0;
    strcat(text, StringTable.get(Labels.strings_ptr[0x68 / 4]));
    CharUpperA(text);
    spot_.get_rect(5, 0, &button_rect);
    MainInterfaceGlobal.set_font(&font1_, 0, 0, 0);
    if ((GameDataStateStorage.fields_.game_state_ & 2) && (!IsMultiplayerNet
        || !(GameDataStateStorage.fields_.net_turn_flags_ & 0x10) || GameDataStateStorage.fields_.net_turn_faction_ == ConsoleGlobal.faction_id_)
        && (ConsoleGlobal.field_23BE4_ || (IsMultiplayerNet
            && NetDaemonNet.primary_state_.trailing_words_[6] != 0xFF
            && NetDaemonNet.primary_state_.trailing_words_[6] != 0))) {
        flic2_.decode_stored_frame();
        MainInterfaceEndTurnAnimation = 1;
    } else {
        flic2_.rewind();
        flic2_.decode_stored_frame();
        MainInterfaceEndTurnAnimation = 0;
    }
    if (IsMultiplayerNet && (GameDataStateStorage.fields_.game_state_ & 2)
        && !(GameDataStateStorage.fields_.net_turn_flags_ & 0x10)
        && NetDaemonNet.primary_state_.trailing_words_[6] != 0xFF
        && (NetDaemonNet.primary_state_.trailing_words_[6] & (1u << ConsoleGlobal.faction_id_))) {
        text[0] = 0;
        strcat(text, StringTable.get(Labels.strings_ptr[0x3FC / 4]));
    } else {
        text[0] = 0;
        strcat(text, StringTable.get(Labels.strings_ptr[0x68 / 4]));
    }
    ++MainInterfaceRedrawCount;
    CharUpperA(text);
    MainInterfaceGlobal.set_text_color(0x65, -1, 1, 1);
    // Same null test of the buffer address before strlen as set_date.
    if (text) MainInterfaceGlobal.write_cent_l(text, &button_rect, strlen(text));
    MainInterfaceGlobal.soft_update(&button_rect);
}

// ORIGINAL: 0x0045EF10 ??0MainInterface@@QAE@XZ 0x0045EF10-0x0045F1BE;0x00655530-0x00655723 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/maininterface.h", "git:b4601390:recovery/tasks/maininterface-owner-coverage.json", "git:f7bbce35:src/recovered/units/0045ef10.cpp"], "reason": "Original owner calls, secondary interface receiver and ordinary embedded-member lifecycle identify this MainInterface operation; compatible historical seed spans are retained, without a matching claim."}
MainInterface::MainInterface() {
    // Genuine embedded members and both bases construct in declaration order.
}

void __cdecl main_interface_timer_callback(int, void *);

// ORIGINAL: 0x004595C0 ?bung_momma@@YAXXZ 0x004595C0-0x0045967F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b84036a86445148d25d4e216ac24c131e108bea8:src/unrecovered/004595c0.cpp", "Pinned4595C0..45967F font7CFEDC is MainInterface7AE820+216BC, its third Font; native title bounds and shared StringBox identify actual members.", "recovery/tasks/startup-desktop-owner-oct5.json", "recovery/tests/test_startup_actual_source.py", "source:src/maininterface.cpp: the sole startup helper initializes MainInterfaceGlobal.tertiary_font(), shared_string_box() and title_bounds() through the actual MainInterface/Console bases."], "reason": "Keep the shared MainInterface message-box setup beside its real MainInterface owner and owned resources; preserve the free __cdecl API and startup call. This current source correspondence does not prove a historical filename."}
// kind      game
void __cdecl bung_momma() {
    const int screen_width = WinScreenWidth;
    const int box_width = screen_width - 0x140;
    Font *const font = MainInterfaceGlobal.tertiary_font();
    const int font_mode = font->unk_1_;
    const int rows = font_mode >= 0
        ? font_mode + font->pixel_height()
        : font->line_height();
    const int box_height = rows * 4;
    const int box_x = (screen_width - box_width) / 2;
    const int box_y = MainInterfaceGlobal.title_bounds().bottom - box_height;

    StringBox &box = MainInterfaceGlobal.shared_string_box();
    Win *const parent = static_cast<Win *>(&ConsoleGlobal);
    box.init(box_x, box_y, box_width, box_height, parent, 0, 0x1000022);
    box.set_draw_mode(1);
    box.set_visible_rows(4);

    box.set_font(font, 0, 0, 0);
    box.set_text_color(0x5F, -1, 1, 1);
    box.clear();
    static_cast<Win &>(box).Win::show(0);
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00459680 0x0045B500 0x0045B52C 0x0045B1DC
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00459680 0x0045B52C 0x0045B545 0x0045B1D6
// ORIGINAL: 0x00459680 ?init@MainInterface@@QAEHH@Z 0x00459680-0x0045B500;0x00655730-0x00655745 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/maininterface.h", "git:b4601390:recovery/tasks/maininterface-owner-coverage.json", "git:f7bbce35:src/unrecovered/00459680.cpp"], "reason": "Original owner calls, secondary interface receiver and ordinary embedded-member lifecycle identify this MainInterface operation; compatible historical seed spans are retained, without a matching claim."}
int MainInterface::init(int) {
    Buffer assets;
    const int screen_height_on_entry = WinScreenHeight;
    const int screen_width_on_entry = WinScreenWidth;
    field_1EAC8_ = 0;
    memset(InterfaceRegistry, 0, sizeof(InterfaceRegistry));
    memset(IfaceIdToSlotStorage, 0, sizeof(IfaceIdToSlotStorage));
    field_4_ = 0;
    register_iface_mode(0, this);
    alert_ = -1;
    field_216E8_ = iface_redraw_pending_ = field_216E4_ = 0;
    spot_.init(1);
    field_D7C_ = 213;
    SetRect(&rect4_, 4, 56, 245, 252);
    if (WinScreenWidth == 800) {
        SetRect(&right_frame_rect_, 555, 57, 796, 253);
        SetRect(&info_panel_frame_, 261, 66, 540, 172);
        SetRect(&lower_frame_rect_, 244, 188, 544, 252);
        SetRect(&window_bounds_, 0, 343, 800, 600);
        SetRect(&menu_button_rect_, 10, 20, 116, 34);
        SetRect(&multiplayer_button_rect_, 684, 22, 790, 36);
        SetRect(&menu_animation_bounds_, 116, 0, 247, 51);
        SetRect(&multiplayer_animation_bounds_, 516, 17, 682, 65);
        SetRect(&title_bounds_, 253, 0, 586, 43);
        if (assets.load_pcx("console_A.pcx", 0, 10, 236)) return 1;
    } else {
        SetRect(&right_frame_rect_, 779, 56, 1020, 252);
        SetRect(&info_panel_frame_, 262, 66, 763, 172);
        SetRect(&lower_frame_rect_, 244, 188, 768, 252);
        const int x = (screen_width_on_entry - 1024) / 2;
        SetRect(&window_bounds_, x, screen_height_on_entry - 257,
                x + 1024, screen_height_on_entry);
        SetRect(&menu_button_rect_, 11, 20, 117, 34);
        SetRect(&multiplayer_button_rect_, 908, 22, 1014, 36);
        SetRect(&menu_animation_bounds_, 118, 0, 282, 60);
        SetRect(&multiplayer_animation_bounds_, 742, 10, 906, 74);
        SetRect(&title_bounds_, 282, 0, 787, 42);
    }
    rect1_ = info_panel_frame_;
    rect1_.bottom += 62;
    lower_animation_bounds_ = lower_frame_rect_;
    lower_animation_bounds_.top += 62;
    const int height = window_bounds_.bottom - window_bounds_.top;
    SetRect(&bottom_bounds_, 244, height - 96, 779, height);
    SetRect(&scroll_up_rect_, info_panel_frame_.right - 29, info_panel_frame_.top + 9,
            info_panel_frame_.right - 9, info_panel_frame_.top + 29);
    SetRect(&scroll_down_rect_, info_panel_frame_.right - 29, info_panel_frame_.bottom - 29,
            info_panel_frame_.right - 9, info_panel_frame_.bottom - 9);
    SetRect(&editor_tab_rect_, scroll_up_rect_.left, scroll_up_rect_.bottom + 5,
            scroll_up_rect_.right, scroll_down_rect_.top - 5);
    SetRect(&message_bounds_, info_panel_frame_.left + 7, info_panel_frame_.top + 7,
            scroll_down_rect_.left - 10, info_panel_frame_.bottom - 7);
    status_left_rect_ = rect4_;
    status_left_rect_.right = (rect4_.right - rect4_.left) * 7 / 12;
    InflateRect(&status_left_rect_, -4, -4);
    status_hit_rect_ = lower_frame_rect_;
    InflateRect(&status_hit_rect_, -4, -4);
    status_hit_rect_.left = status_left_rect_.right + 3;
    status_right_rect_ = rect4_;
    status_right_rect_.left = status_left_rect_.right + 4;
    status_right_rect_.top += 4; status_right_rect_.right -= 4;
    status_right_rect_.bottom = status_hit_rect_.top - 3;
    SetRect(&end_turn_rect_, right_frame_rect_.left + 4, right_frame_rect_.top + 5,
            right_frame_rect_.left + 237, right_frame_rect_.top + 28);
    SetRect(&tutorial_bounds_, right_frame_rect_.left + 1, end_turn_rect_.bottom + 4,
            right_frame_rect_.right - 1, 0);
    tutorial_bounds_.bottom = tutorial_bounds_.top +
        (tutorial_bounds_.right - tutorial_bounds_.left) / 2;
    SetRect(&rect3_, right_frame_rect_.right - 60, right_frame_rect_.bottom - 38,
            right_frame_rect_.right - 6, right_frame_rect_.bottom - 6);
    rect2_ = rect3_;
    OffsetRect(&rect2_, rect3_.left - rect3_.right - 2, 0);
    SetRect(&date_rect_, right_frame_rect_.left + 6, right_frame_rect_.bottom - 38,
            rect2_.left - 3, right_frame_rect_.bottom - 6);
    font1_.init(DefaultFontFace, 14, 1);
    font2_.init(DefaultFontFace, 12, 0);
    font3_.init(DefaultFontFace, 12, 1);
    if (assets.load_pcx(WinScreenWidth == 800 ? "console_A.pcx" : "console2_A.pcx",
                        0, 10, 236)) return 1;
    spot_.add(5, 0, &end_turn_rect_);
    GraphicWin::init(window_bounds_.left, window_bounds_.top,
                    window_bounds_.right - window_bounds_.left, height,
                    0, 0x1400000, 0, 0, 0);
    Buffer &canvas = *static_cast<Buffer *>(this);
    assets.copy(&canvas, 0, 0, assets.pixel_width(), assets.pixel_height(),
                0, 0, canvas.pixel_width(), canvas.pixel_height());
    if (WinScreenWidth == 800) {
        sprites13_[0].init("iface_up_A.pcx", 0, 0);
        sprites13_[1].init("iface_down_A.pcx", 0, 0);
    } else {
        sprites13_[0].init("iface_up2_A.pcx", 0, 0);
        sprites13_[1].init("iface_down2_A.pcx", 0, 0);
    }
    sprites13_[0].iWidth_ = sprites13_[1].iWidth_ = 535;
    sprites13_[0].iHeight_ = sprites13_[1].iHeight_ = 96;
    flic2_.init(&canvas, "turn.flc", end_turn_rect_.left, end_turn_rect_.top, 1);
    assets.load_pcx(WinScreenWidth == 800 ? "console_x_A.pcx" : "console_x2_A.pcx",
                    0, 10, 236);
    int i, frame;
    for (i = 0; i < 10; ++i)
        sprites6_[i].extract(&assets, WinFillColour, 1 + (i / 5) * 191,
                            1 + (i % 5) * 27, 190, 26, 0);
    if (WinScreenWidth == 800) {
        sprites4_[0].extract(&assets, WinFillColour, 194, 136, 131, 51, 0);
        sprites4_[1].extract(&assets, WinFillColour, 194, 188, 166, 48, 0);
        sprites3_[0].extract(&assets, WinFillColour, 194, 237, 131, 51, 0);
        sprites3_[1].extract(&assets, WinFillColour, 194, 289, 166, 48, 0);
        for (i = 0; i < 5; ++i)
            sprites5_[2*i].extract(&assets, WinFillColour, 384, 1 + i * 52,
                                  131, 51, 0);
        for (i = 0; i < 5; ++i)
            sprites5_[2*i+1].extract(&assets, WinFillColour, 516,
                                    1 + i * 49, 166, 48, 0);
    } else {
        sprites4_[0].extract(&assets, WinFillColour, 194, 136, 164, 60, 0);
        sprites4_[1].extract(&assets, WinFillColour, 194, 197, 164, 64, 0);
        sprites3_[0].extract(&assets, WinFillColour, 194, 262, 164, 60, 0);
        sprites3_[1].extract(&assets, WinFillColour, 194, 323, 164, 64, 0);
        for (i = 0; i < 5; ++i)
            sprites5_[2*i].extract(&assets, WinFillColour, 384, 1 + i * 61,
                                  164, 60, 0);
        for (i = 0; i < 5; ++i)
            sprites5_[2*i+1].extract(&assets, WinFillColour, 549,
                                    1 + i * 65, 164, 64, 0);
    }
    for (i = 0; i < 3; ++i)
        sprites8_[i].extract(&assets, WinFillColour, 1, 217 + i*15, 106, 14, 0);
    const int columns[4] = {683, 699, 708, 717};
    const int widths[4] = {15, 8, 8, 15};
    const int column_offset = WinScreenWidth == 800 ? 0 : 30;
    for (i = 0; i < 4; ++i)
        sprites2_[i].extract(&assets, WinFillColour, columns[i] + column_offset,
                            1, widths[i], 529, 0);
    const int rows[4] = {279, 306, 341, 367};
    const int heights[4] = {26, 34, 25, 34};
    for (i = 0; i < 4; ++i)
        sprites1_[i].extract(&assets, WinFillColour, 1, rows[i], 32, heights[i], 0);
    assets.load_pcx("interface.pcx", 0, 10, 236);
    for (i = 0; i < 6; ++i)
        sprites7_[i].extract(&assets, WinFillColour, 532 + (i/3)*55,
                            143 + (i%3)*33, 54, 32, 0);
    sprite11_.extract(&assets, WinFillColour, 212, 1, 9, 147, 0);
    for (i = 0; i < 6; ++i)
        sprites12_[i].extract(&assets, WinFillColour, 397 + (i%3)*13,
                             153 + (i/3)*43, 12, 42, 0);
    for (i = 0; i < 25; ++i) {
        for (frame = 0; frame < 3; ++frame)
            sprites9_[3*i+frame].extract(&assets, WinFillColour,
                350 + (i%8)*44, 245 + (i/8)*57 + frame*19, 43, 18, 0);
    }
    for (i = 0; i < 18; ++i)
        sprites10_[i].extract(&assets, WinFillColour, 465 + (i/3)*25,
                             438 + (i%3)*25, 24, 24, 0);
    assets.close();
    int top = status_hit_rect_.top +
        (status_hit_rect_.bottom - status_hit_rect_.top - sprites12_[0].height()) / 2;
    SetRect(&stack_button_rect_, status_hit_rect_.left + 4, top,
            status_hit_rect_.left + 4 + sprites12_[0].width(), top + sprites12_[0].height());
    SetRect(&adjacent_panel_left_rect_, lower_frame_rect_.left, top,
            lower_frame_rect_.left + sprites12_[0].width(), top + sprites12_[0].height());
    adjacent_panel_right_rect_ = adjacent_panel_left_rect_;
    OffsetRect(&adjacent_panel_right_rect_, status_hit_rect_.right - sprites12_[0].width() - 4
               - adjacent_panel_right_rect_.left, 0);
    canvas.set_font(&font1_, 0, 0, 0);
    canvas.set_text_color(64, -1, 1, 1);
    for (i = 0; i < 4; ++i) button_group_[i].init(i, i >= 2);
    RECT *rects[11] = {&scroll_up_rect_, &scroll_down_rect_, &editor_tab_rect_,
        &menu_button_rect_, &multiplayer_button_rect_, 0, &rect2_, &rect3_,
        &stack_button_rect_, &adjacent_panel_left_rect_, &adjacent_panel_right_rect_};
    // Buttons are initialized in the original order: down, up, editor, menu,
    // multiplayer, the two map selectors and the three stack controls.
    const int order[10] = {1, 0, 2, 3, 4, 6, 7, 8, 9, 10};
    for (int step = 0; step < 10; ++step) {
        i = order[step];
        RECT &r = *rects[i];
        char *caption = 0;
        if (i < 3) {
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(i == 0 ? 0xAC0/4 : i == 1 ? 0xABC/4 : 0xDBC/4));
            StringTemp[1] = 0;
            caption = StringTemp;
        } else if (i == 3 || i == 4) caption = label_get(i == 3 ? 0x9C0/4 : 0x83C/4);
        flat_button_[i].init(caption, 1000+i, r.left, r.top,
                            r.right-r.left, r.bottom-r.top, this, 0);
        if (i < 3) {
            flat_button_[i].set_bubble_text(label_get(i == 0 ? 0xC44/4 : i == 1 ? 0xC48/4 : 0xDC0/4));
            if (i == 2) {
                flat_button_[i].set_border_styles(&InterfaceBoxStyles[27], &InterfaceBoxStyles[28], &InterfaceBoxStyles[29]);
                flat_button_[i].hide();
            }
            if (i == 2) {
                for (int j = 0; j < 3; ++j) {
                    button_group_[0].add(&flat_button_[j]);
                    static_cast<Buffer *>(&flat_button_[j])->set_font(&font1_, 0, 0, 0);
                    flat_button_[j].redraw(); flat_button_[j].redraw();
                }
                button_group_[0].set(1, 1);
            }
        } else if (i == 3 || i == 4) {
            flat_button_[i].field_AD4_[0] = &sprites8_[0];
            flat_button_[i].field_AD4_[1] = i == 3 ? &sprites8_[1] : 0;
            flat_button_[i].field_AD4_[2] = &sprites8_[2];
            static_cast<Buffer *>(&flat_button_[i])->set_font(&font1_, 0, 0, 0);
            flat_button_[i].redraw();
            flat_button_[i].set_text_color(64, -1, 1, 1);
            flat_button_[i].field_ABC_[0] = 101;
            flat_button_[i].field_ABC_[1] = flat_button_[i].field_ABC_[2] = 65;
            flat_button_[i].redraw();
            flat_button_[i].set_bubble_text(label_get(i == 3 ? 0xC3C/4 : 0xC40/4));
        } else if (i == 6 || i == 7) {
            flat_button_[i].set_bubble_text(label_get(i == 6 ? 0x4C8/4 : 0xF20/4));
            for (frame = 0; frame < 3; ++frame)
                flat_button_[i].field_AD4_[frame] = &sprites7_[3*(i-6)+frame];
            if (i == 7) {
                for (int j = 6; j < 8; ++j) {
                    button_group_[1].add(&flat_button_[j]); flat_button_[j].redraw();
                }
                button_group_[1].set(1006, 1);
            }
        } else {
            flat_button_[i].hide();
            int sprite = i == 10 ? 3 : 0;
            for (frame = 0; frame < 3; ++frame)
                flat_button_[i].field_AD4_[frame] = &sprites12_[sprite+frame];
        }
    }
    int left = info_panel_frame_.left + 7;
    top = info_panel_frame_.top + 7;
    top += (info_panel_frame_.bottom - 7 - 5*sprites9_[0].height() - top - 4) / 2;
    for (i = 0; i < 25; ++i) {
        SetRect(&editor_button_rects_[i], left + (i/5)*(sprites9_[0].width()+1),
            top + (i%5)*(sprites9_[0].height()+1),
            left + (i/5)*(sprites9_[0].width()+1) + sprites9_[0].width(),
            top + (i%5)*(sprites9_[0].height()+1) + sprites9_[0].height());
    }
    left = editor_button_rects_[24].right + 9;
    top = info_panel_frame_.top + 7;
    top += (info_panel_frame_.bottom - 7 - 3*sprites10_[0].height() - 2 - top) / 2;
    for (i = 0; i < 6; ++i) {
        SetRect(&brush_button_rects_[i], left + (i/3)*(sprites10_[0].width()+1),
            top + (i%3)*(sprites10_[0].height()+1),
            left + (i/3)*(sprites10_[0].width()+1) + sprites10_[0].width(),
            top + (i%3)*(sprites10_[0].height()+1) + sprites10_[0].height());
    }
    editor_panel_rect_.left = editor_panel_rect_.right = 0;
    for (i = 0; i < 25; ++i) {
        RECT &r = editor_button_rects_[i];
        UnionRect(&editor_panel_rect_, &editor_panel_rect_, &r);
        FlatButton &b = flat_button_[i+11];
        b.init(0, 1011+i, r.left, r.top, r.right-r.left, r.bottom-r.top, this, 0);
        b.hide();
        for (frame = 0; frame < 3; ++frame) b.field_AD4_[frame] = &sprites9_[3*i+frame];
        int tool = i - 6;
        if (i >= 6 && i <= 20) {
            for (int t = 0; t < 20; ++t) if (Terraforming[t].bit == 0 && tool >= t) ++tool;
            b.set_bubble_text(StringTable.get(Terraforming[tool].name));
        } else {
            StringTemp[0] = 0;
            if (i < 6) strcat(StringTemp, label_get(0xEBC/4 + i));
            else if (i == 21) strcat(StringTemp, StringTable.get(Natural[2].name_short));
            else if (i == 22) strcat(StringTemp, StringTable.get(Natural[6].name_short));
            else if (i == 23) strcat(StringTemp, StringTable.get(Natural[3].name_short));
            else if (i == 24) strcat(StringTemp, StringTable.get(Natural[10].name_short));
            b.set_bubble_text(StringTemp);
        }
        button_group_[2].add(&b);
    }
    InflateRect(&editor_panel_rect_, 3, 3);
    button_group_[2].set(1011, 1);
    editor_tools_rect_.left = editor_tools_rect_.right = 0;
    if (WinScreenWidth != 800) {
        for (i = 0; i < 6; ++i) {
            RECT &r = brush_button_rects_[i];
            UnionRect(&editor_tools_rect_, &editor_tools_rect_, &r);
            FlatButton &b = flat_button_[36+i];
            b.init(0, 1036+i, r.left, r.top, r.right-r.left, r.bottom-r.top, this, 0);
            b.hide();
            for (frame = 0; frame < 3; ++frame) b.field_AD4_[frame] = &sprites10_[3*i+frame];
            b.set_bubble_text(label_get(0xF64/4+i)); button_group_[3].add(&b);
        }
        InflateRect(&editor_tools_rect_, 3, 3); button_group_[3].set(1036, 1);
    }
    editor_text_rect_ = editor_tools_rect_;
    editor_text_rect_.left = editor_tools_rect_.right + 3;
    editor_text_rect_.right = scroll_down_rect_.left - 3;
    time1_.init(::main_interface_timer_callback, 0, this, 4000, 5);
    time2_.init(::main_interface_timer_callback, 1, this, 100, 5);
    g_MAININTERFACE_WAVE.init("fx\\main menu.wav", 4);
    g_MAININTERFACE_WAVE.set_volume(35);
    return 0;
}

// ORIGINAL: 0x0045EB80 ??1MainInterface@@QAE@XZ 0x0045EB80-0x0045EF0A;0x006552C0-0x0065552F FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/maininterface.h", "git:b4601390:recovery/tasks/maininterface-owner-coverage.json", "git:f7bbce35:src/unrecovered/0045eb80.cpp"], "reason": "Original owner calls, secondary interface receiver and ordinary embedded-member lifecycle identify this MainInterface operation; compatible historical seed spans are retained, without a matching claim."}
// symbol    ??1MainInterface@@UAE@XZ
// TRIED: plain reinterpret_cast pointer-offset calls into close()/dtors in disasm order (same shape as 0x00408170's AlphaSave dtor); compiles and matches through instruction #17, then diverges because the real destructor's ~35-state SEH unwind table (one `mov byte ptr [ebp-4], N` per sub-object cleanup, with matching funclets) only emerges from genuine implicit member/base destruction, not explicit calls in a hand-written body.
MainInterface::~MainInterface() {
    // Reverse member/base order supplies ordinary and exceptional teardown.
}

// ORIGINAL: 0x0045BC50 ?on_redraw@MainInterface@@QAEXXZ 0x0045BC50-0x0045BE7D FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/maininterface.h", "git:b4601390:recovery/tasks/maininterface-owner-coverage.json", "git:f7bbce35:src/recovered/units/0045bc50.cpp"], "reason": "Original owner calls, secondary interface receiver and ordinary embedded-member lifecycle identify this MainInterface operation; compatible historical seed spans are retained, without a matching claim."}
// symbol    ?on_redraw@MainInterface@@UAEXXZ
void MainInterface::on_redraw() {
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 0) return;
    restore_back(1);
    RECT rect = info_panel_frame_;
    switch (button_group_[0].field_84_ - 1000) {
    case 0: {
        rect.left += 4; rect.right -= 4;
        rect.top += 4; rect.bottom -= 4;
        box_sprite(&rect, &InterfaceBoxStyles[3]);
        ConsoleInfoWin.timer_.stop();
        InterfaceMessageWin.listBox_.show(0);
        for (int button = 11; button < 42; ++button) flat_button_[button].hide();
        break;
    }
    case 1: {
        rect.left += 4; rect.right -= 4;
        rect.top += 4; rect.bottom -= 4;
        box_sprite(&rect, &InterfaceBoxStyles[3]);
        box_sprite(&ConsoleInfoWin.pane_rects_[0], &InterfaceBoxStyles[18]);
        box_sprite(&ConsoleInfoWin.pane_rects_[1], &InterfaceBoxStyles[18]);
        ConsoleInfoWin.reset();
        InterfaceMessageWin.listBox_.hide();
        ConsoleInfoWin.timer_.start();
        for (int button = 11; button < 42; ++button) flat_button_[button].hide();
        break;
    }
    case 2: {
        box_sprite(&editor_panel_rect_, &InterfaceBoxStyles[3]);
        if (WinScreenWidth != 800)
            box_sprite(&editor_tools_rect_, &InterfaceBoxStyles[3]);
        InterfaceMessageWin.listBox_.hide();
        ConsoleInfoWin.timer_.stop();
        for (int button = 11; button < 42; ++button) {
            if (WinScreenWidth == 800 && button == 36) break;
            flat_button_[button].show(0);
        }
        draw_edit_pane();
        break;
    }
    }
    ConsoleGlobal.update_data(1);
    redraw_complete();
}

// ORIGINAL: 0x0045CA40 ?on_status@MainInterface@@QAEXH@Z 0x0045CA40-0x0045CC78 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/maininterface.h", "git:b4601390:recovery/tasks/maininterface-owner-coverage.json", "git:f7bbce35:src/recovered/units/0045ca40.cpp"], "reason": "Original owner calls, secondary interface receiver and ordinary embedded-member lifecycle identify this MainInterface operation; compatible historical seed spans are retained, without a matching claim."}
// symbol    ?on_status@MainInterface@@UAEXH@Z
void MainInterface::on_status(int active) {
    if (!active) {
        ConsoleInfoWin.timer_.stop();
        const int mode = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        if (mode != 10 && mode != 3 && mode != 2) {
            stop_timers();
            MainInterfaceTimersStopped = 1;
        }
        ConsoleGlobal.alpha_menu_.hide();
        MultiWindow.hide();
        InterfaceMessageWin.listBox_.hide();
        for (int button = 0; button < 42; ++button) flat_button_[button].hide();
        if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 10) string_box_.hide();
        const int destination = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        if (destination != 10 && !(destination == 5
            && (ReportIfGlobal.field_8_ == 4 || ReportIfGlobal.field_8_ == 5))) {
            WorldClimateWorldWin.hide_all();
            return;
        }
    } else {
        flic2_.rewind();
        for (int button = 0; button < 42; ++button) {
            if (button == 2 && !((GameDataStateStorage.fields_.game_state_ & 0x80)
                    && (GameDataStateStorage.fields_.game_state_ & 0x40)))
                continue;
            if (button >= 8 && button <= 10) {
                flat_button_[button].hide();
                continue;
            }
            if (button >= 11 && button < 42 && button_group_[0].field_84_ != 1002) {
                flat_button_[button].hide();
                continue;
            }
            flat_button_[button].show(0);
        }
        on_redraw();
        ConsoleStatusWin.on_redraw();
        WorldClimateWorldWin.show_all();
        if (MainInterfaceGlobal.button_group_[0].field_84_ == 1000)
            InterfaceMessageWin.listBox_.show(0);
        button_group_[1].set(1006, 1);
        string_box_.show(0);
        ConsoleInfoWin.timer_.start();
        if (MainInterfaceTimersStopped) start_timers();
        MainInterfaceTimersStopped = 0;
        MainInterfaceGlobal.soft_update();
    }
}

// The unsigned button-1003 guard admits eight entries; the indexed jump at
// 0x0045D184 selects the DWORD targets following the independent function body.
// Each target is an instruction start within that body. The intervening
// three-byte LEA preserves its register and flags; it is not extra body code.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0045D170 0x0045D274 0x0045D294 0x0045D184
// ORIGINAL: 0x0045D170 ?on_iface_button_clicked@MainInterface@@QAEXH@Z 0x0045D170-0x0045D271 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/maininterface.h", "git:b4601390:recovery/tasks/maininterface-owner-coverage.json", "git:f7bbce35:src/recovered/units/0045d170.cpp"], "reason": "Original owner calls, secondary interface receiver and ordinary embedded-member lifecycle identify this MainInterface operation; compatible historical seed spans are retained, without a matching claim."}
// symbol    ?on_iface_button_clicked@MainInterface@@UAEXH@Z
void MainInterface::on_iface_button_clicked(int button) {
    switch (button) {
    case 1008:
    case 1009: {
        int vehicle = veh_at(ConsoleStatusWin.iTileX_, ConsoleStatusWin.iTileY_);
        if (vehicle < 0) return;
        veh_demote(vehicle);
        break;
    }
    case 1010: {
        int vehicle = veh_at(ConsoleStatusWin.iTileX_, ConsoleStatusWin.iTileY_);
        if (vehicle < 0) return;
        while (Vehs[vehicle].next_veh_id_stack >= 0)
            vehicle = Vehs[vehicle].next_veh_id_stack;
        veh_promote(vehicle);
        break;
    }
    case 1003:
        if (ConsoleGlobal.alpha_menu_.is_visible()) {
            ConsoleGlobal.alpha_menu_.hide_menus();
            ConsoleGlobal.alpha_menu_.hide();
        } else ConsoleGlobal.alpha_menu_.show();
        return;
    case 1004:
        if (MultiWindow.panel_open()) MultiWindow.hide();
        else MultiWindow.show();
        return;
    default: return;
    }
    MainInterfaceStatusRedrawFlag = 1;
    ConsoleStatusWin.on_redraw();
    MainInterfaceStatusRedrawFlag = 0;
}

// ORIGINAL: 0x0045BE80 ?set_date@MainInterface@@QAEXPAD@Z 0x0045BE80-0x0045C0AF FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/maininterface.h", "git:b4601390:recovery/tasks/maininterface-owner-coverage.json", "git:f7bbce35:src/recovered/units/0045be80.cpp"], "reason": "Original owner calls, secondary interface receiver and ordinary embedded-member lifecycle identify this MainInterface operation; compatible historical seed spans are retained, without a matching claim."}
void MainInterface::set_date(char *) {
    const int mode = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
    if (mode != 0 && mode != 10
        && !(mode == 5 && (ReportIfGlobal.field_8_ == 4 || ReportIfGlobal.field_8_ == 5))) return;
    set_font(&font2_, 0, 0, 0);
    set_text_color(MessageWinTextColor, MessageWinTextBackground, 1, 1);
    RECT rect;
    UnionRect(&rect, &date_rect_, &rect3_);
    rect.left -= 3; rect.right += 3;
    rect.top -= 3; rect.bottom += 3;
    MainInterfaceGlobal.box_sprite(&rect, &InterfaceBoxStyles[21]);
    RECT outline = rect;
    MainInterfaceGlobal.box_sprite(&date_rect_, &InterfaceBoxStyles[18]);
    rect = date_rect_;
    rect.left += 3; rect.top += 3;
    rect.right -= 3; rect.bottom -= 3;
    const int x = rect.left;
    int y = rect.top;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0));
    strcat(StringTemp, " ");
    say_year(StringTemp);
    // The image keeps a null test of the StringTemp address before strlen.
    if (StringTemp) write_l(StringTemp, x, y, strlen(StringTemp));
    y += text_line_height();
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x90 / 4));
    strcat(StringTemp, ": ");
    char number[80];
    _itoa(static_cast<int>(static_cast<uint32_t>(PlayersData[ConsoleGlobal.faction_id_].energy_reserves)
          - PlayersData[ConsoleGlobal.faction_id_].hurry_cost_total), number, 10);
    strcat(StringTemp, number);
    if (StringTemp) write_l(StringTemp, x, y, strlen(StringTemp));
    soft_update(&outline);
}

 /*
 Purpose: Set the flat button's bubble text.
 Original Offset: n/a
 Return Value: Zero on success, non-zero on error
 Status: Complete
 */
int MainInterface::set_bubble_text(uint32_t button_id, LPCSTR text) {
    return flat_button_[button_id].set_bubble_text(text);
}

/*
Purpose: Clear the interface message; the legacy implementation is intentionally empty.
// ORIGINAL: 0x0045C170 ?clear_message@MainInterface@@QAEXXZ 0x0045C170-0x0045C171 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?clear_message@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void MainInterface::clear_message() {
}

/*
Purpose: Update the desktop; the legacy implementation is intentionally empty.
// ORIGINAL: 0x0058EE50 ?desktop_update@@YAXXZ 0x0058EE50-0x0058EE51 BYTE_EXACT
// size      1 bytes
// prototype 
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void __cdecl desktop_update() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0045C5B0 ?UNK2@MainInterface@@QAEXXZ 0x0045C5B0-0x0045C5B1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK2@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0045EB60 ?UNK3@MainInterface@@QAEXXZ 0x0045EB60-0x0045EB61 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK3@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK3() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0045EB70 ?UNK4@MainInterface@@QAEXXZ 0x0045EB70-0x0045EB71 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK4@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK4() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045C280 ?UNK1@MainInterface@@QAEXH@Z 0x0045C280-0x0045C283 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK1@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK1(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CC80 ?on_iface_scrolled@MainInterface@@QAEXHH@Z 0x0045CC80-0x0045CC83 BYTE_EXACT
// symbol    ?on_iface_scrolled@MainInterface@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_scrolled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_scrolled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CC90 ?on_iface_scrolling@MainInterface@@QAEXHH@Z 0x0045CC90-0x0045CC93 BYTE_EXACT
// symbol    ?on_iface_scrolling@MainInterface@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_scrolling@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_scrolling(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CCA0 ?on_iface_button_toggled@MainInterface@@QAEXHH@Z 0x0045CCA0-0x0045CCA3 BYTE_EXACT
// symbol    ?on_iface_button_toggled@MainInterface@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CDB0 ?on_iface_right_double_click@MainInterface@@QAEXHH@Z 0x0045CDB0-0x0045CDB3 BYTE_EXACT
// symbol    ?on_iface_right_double_click@MainInterface@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x0045D160 ?on_iface_button_pressed@MainInterface@@QAEHH@Z 0x0045D160-0x0045D165 BYTE_EXACT
// symbol    ?on_iface_button_pressed@MainInterface@@UAEHH@Z
// size      5 bytes
// prototype int (__thiscall ?on_iface_button_pressed@MainInterface@@QAEHH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int MainInterface::on_iface_button_pressed(int) {
    return 0;
}

// ORIGINAL: 0x0045CD80 ?on_iface_left_double_click@MainInterface@@QAEXHH@Z 0x0045CD80-0x0045CDB0 BYTE_EXACT
// symbol    ?on_iface_left_double_click@MainInterface@@UAEXHH@Z
// Original boundary evidence: the return at 0x0045CDAD ends at 0x0045CDB0;
// the separate right-double-click entry at 0x0045CDB0 has its own return.
// The former exclusive end 0x0045CDB1 cut into that next instruction.
// LEVER: 2026-09-05 homed from 0045cd80.cpp, the second of the wave-4
//   handoffs: it needs StatusWin::on_left_double_click (0x004B8990) as a real
//   member, which statuswin.cpp now owns. `mov ecx, 0x8c5568` falls out of
//   naming ConsoleStatusWin (console.h, real storage in statuswin.cpp) and
//   `push 0x7af50c` out of StatusWinRedrawRect1 (statuswin.h).
// size      48 bytes
// prototype void (__thiscall ?on_iface_left_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B8990 0x005FA7E0
void MainInterface::on_iface_left_double_click(int x, int y) {
    if (in_box(x, y, &MainInterfaceGlobal.status_hit_rect())) {
        ConsoleStatusWin.on_left_double_click(x, y);
    }
}

// THE RECEIVER MAP, RESOLVED. Every body below this line until the next
// non-iface method takes the IfaceRegistryEntry subobject as `this`, not the
// window front - so the members it names compile base-relative:
//
//     spot_                  ->  [esi + 0x24620]   (front + 0x25034)
//     iface_redraw_pending_  ->  [esi + 0x20CD8]   (front + 0x216EC)
//     redraw_complete()      ->  lea ecx,[esi - 0xA14] then call
//
// That is not a spelling this tree chose. VC6 12.00.8168 compiles an override
// of a SECONDARY base's virtual with the base subobject as `this`, puts the
// override's raw address in the secondary vftable, and emits no adjustor
// thunk - probed against a GraphicWin-sized first base under /O2 /Gy /GR- /GX
// on 2026-09-05. The image is the same shape: the handlers read the entry-
// relative offsets above, the table at 0x66C544 holds the raw bodies, and
// .text has no `sub ecx,0xA14` thunk anywhere. See maininterface.h.

// The second cursor hit-test and hovered ids belong to ConsoleStatusWin:
// its +0x15E0 Spot and +0x15C4/+0x15C8 ids are original 0x008C6B48 and
// 0x008C6B2C/0x008C6B30. Vehicle removal compacts the same vehicle member.


// ORIGINAL: 0x0045D110 ?on_iface_mouse_leave@MainInterface@@QAEXHH@Z 0x0045D110-0x0045D13F BYTE_EXACT
// symbol    ?on_iface_mouse_leave@MainInterface@@UAEXHH@Z
// LEVER: 2026-09-05 the receiver map. This was the body that named the wall:
//   as a MainInterface method with field_A14_/field_A18_ standing in for the
//   SubInterface subobject there was no spelling for `cmp [esi+0x20cd8],1` at
//   all - a downcast folded the displacement to 0x216EC and cost the match.
//   Declaring IfaceRegistryEntry as MainInterface's second base is what makes
//   it fall out: `this` is the entry, iface_redraw_pending_ is entry-relative,
//   and the unqualified redraw_complete() call is where VC6's own -0xA14 goes.
//   Neither argument is read. ConsoleStatusWin.reset_active is the real
//   0x004B6530, homed in statuswin.cpp.
// size      47 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045C3A0 0x004B6530
void MainInterface::on_iface_mouse_leave(int, int) {
    ConsoleStatusWin.reset_active();
    if (iface_redraw_pending_ == 1) {
        iface_redraw_pending_ = 0;
        redraw_complete();
    }
}

// The original secondary receiver reaches spot_ at entry+24620; the
// declared secondary base supplies the complete-object adjustment.
// ORIGINAL: 0x0045CF40 ?on_iface_left_down@MainInterface@@QAEXHH@Z 0x0045CF40-0x0045CF75 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/maininterface.h", "git:f7bbce35:src/recovered/units/0045cf40.cpp"], "reason": "Original secondary interface receiver, spot hit test and mapped owner callbacks identify this MainInterface operation; compatible historical seed and its obligations remain retained."}
// symbol    ?on_iface_left_down@MainInterface@@UAEXHH@Z
void MainInterface::on_iface_left_down(int a1, int a2) {
    if (spot_.check(a1, a2, &a2, 0) != -1 && a2 == 5)
        ConsoleGlobal.end_my_turn();
}

// ORIGINAL: 0x0045CDC0 ?on_iface_left_click@MainInterface@@QAEXHH@Z 0x0045CDC0-0x0045CE72 BYTE_EXACT
// symbol    ?on_iface_left_click@MainInterface@@UAEXHH@Z
// The first three hit regions belong to MainInterfaceGlobal; the last
// is InfoWin's union rectangle, established by its init at 0x004592A0.
// The original interface-receiver +0x1DEE8 is complete-object +0x1E8FC:
// button_group_[0] at +0x1E878, with its still-unnamed word at +0x84.
// size      178 bytes
// prototype void (__thiscall ?on_iface_left_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00458930 0x004B8E40 0x005FA7E0
void MainInterface::on_iface_left_click(int x, int y) {
    if (in_box(x, y, &MainInterfaceGlobal.status_left_rect())) {
        ConsoleStatusWin.on_left_click(x, y);
        return;
    }
    if (in_box(x, y, &MainInterfaceGlobal.status_hit_rect())) {
        ConsoleStatusWin.on_left_click(x, y);
        return;
    }
    if (in_box(x, y, &MainInterfaceGlobal.status_right_rect())) {
        ConsoleStatusWin.on_left_click(x, y);
        return;
    }
    if (in_box(x, y, &ConsoleInfoWin.bounds()) &&
        button_group_[0].field_84_ == 0x3E9) {
        ConsoleInfoWin.on_left_click(x, y);
    }
}

// ORIGINAL: 0x0045CE80 ?on_iface_right_click@MainInterface@@QAEXHH@Z 0x0045CE80-0x0045CF38 BYTE_EXACT
// symbol    ?on_iface_right_click@MainInterface@@UAEXHH@Z
// Each hit returns independently. The middle StatusWin region selects 1;
// its two neighbors select 0. InfoWin owns the final hit rectangle.
// The ret 8 at 0x0045CF35 ends at 0x0045CF38, not the old pending extent.
// size      184 bytes
// prototype void (__thiscall ?on_iface_right_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00458D40 0x004B9010 0x005FA7E0
void MainInterface::on_iface_right_click(int x, int y) {
    if (in_box(x, y, &MainInterfaceGlobal.status_left_rect())) {
        ConsoleStatusWin.on_right_click(x, y, 0);
        return;
    }
    if (in_box(x, y, &MainInterfaceGlobal.status_hit_rect())) {
        ConsoleStatusWin.on_right_click(x, y, 1);
        return;
    }
    if (in_box(x, y, &MainInterfaceGlobal.status_right_rect())) {
        ConsoleStatusWin.on_right_click(x, y, 0);
        return;
    }
    if (in_box(x, y, &ConsoleInfoWin.bounds()) &&
        button_group_[0].field_84_ == 0x3E9) {
        ConsoleInfoWin.on_right_click(x, y);
    }
}

// Group0 dispatches the primary window's virtual on_redraw slot12.
// Its secondary receiver is adjusted by the declared C++ base, not offsets.
// ORIGINAL: 0x0045CCB0 ?on_iface_group_clicked@MainInterface@@QAEXHHH@Z 0x0045CCB0-0x0045CD75 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/maininterface.h", "git:f7bbce35:src/unrecovered/0045ccb0.cpp"], "reason": "Original secondary interface receiver, spot hit test and mapped owner callbacks identify this MainInterface operation; compatible historical seed and its obligations remain retained."}
// symbol    ?on_iface_group_clicked@MainInterface@@UAEXHHH@Z
void MainInterface::on_iface_group_clicked(int group, int, int selected) {
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 0 || selected < 0) return;
    selected -= 1000;
    if (group == 0) { on_redraw(); return; }
    if (group == 2) {
        int tool;
        if (selected >= 17) {
            tool = selected - 17;
            for (int index = 0; index < MaxTerrainNum; ++index)
                if (!Terraforming[index].bit && tool >= index) ++tool;
            tool += 6;
        } else tool = selected - 11;
        ConsoleGlobal.editor_paints(tool);
        return;
    }
    if (group == 3) {
        MapEditorBrushId = selected - 36;
        MainInterfaceGlobal.draw_edit_pane();
        return;
    }
    if (selected == 6) WorldClimateWorldWin.set_world_map();
    else WorldClimateWorldWin.set_detail_map();
}

// ORIGINAL: 0x0045CF80 ?on_iface_right_down@MainInterface@@QAEXHH@Z 0x0045CF80-0x0045CFFD BYTE_EXACT
// SPAN CORRECTED from the catalogue's 0x0045D0F5 (size 0x175): that span runs
// over on_iface_mouse_move's own record at 0x0045D000 - two catalogue entries
// cannot overlap, and the image's right_down body ends at 0x0045CFFD with the
// rest alignment padding into the next function. The old span could never
// measure BYTE_EXACT because its tail compared against mouse_move's bytes.
// symbol    ?on_iface_right_down@MainInterface@@UAEXHH@Z
// LEVER: 2026-09-05 the receiver map. spot_ is entry-relative; the bubble text
//   goes through the FRONT, which is what `Win *front = this` is for - inside
//   an override of a secondary base's virtual the conversion emits the
//   compiler's own -0xA14. Labels [+0xC4C] is the string id, StringTable the
//   table it indexes.
// size      373 bytes
// prototype void (__thiscall ?on_iface_right_down@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FAB00 0x00617940 0x00617C70
void MainInterface::on_iface_right_down(int a1, int a2) {
    // BUG IN THE ORIGINAL: spotType is handed to Spot::check UNINITIALIZED -
    // the image's `lea eax,[ebp-4]` takes its address before any store, and
    // only the callee's write makes the `cmp dword ptr [ebp-4],5` below mean
    // anything. Spelling the `= 0` a natural reading wants adds a store the
    // image does not have, so it is left exactly as shipped.
    int spotType;
    int result = spot_.check(a1, a2, &spotType, 0);
    if (result != -1 && spotType == 5) {
        Win::client_to_screen(&a1, &a2);
        RECT rect;
        rect.left = a1;
        rect.top = a2;
        rect.right = a1;
        rect.bottom = a2;
        Win::set_bubble_text(StringTable.get(Labels.strings_ptr[0x313]), &rect);
    }
}

// ORIGINAL: 0x0045D000 ?on_iface_mouse_move@MainInterface@@QAEXHH@Z 0x0045D000-0x0045D10F BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Use the existing StatusWin hover state and embedded hit-test Spot, not executable-address aliases or separate selection storage.","evidence":["ConsoleStatusWin starts8C5568; its existing signed fields+15C4/+15C8 are8C6B2C/30, and Spot+15E0 is8C6B48.","Pinned4BA1F7 constructs Spot at StatusWin this+15E0;45D062 hit-tests8C6B48 and45D07B..45D0C7 updates the hover IDs then invokes draw_active on8C5568.","Pinned veh_kill5C0A58..5C0A6C compacts the same8C6B2C vehicle field; adjacent8C6B34 remains independently unresolved."]}
// symbol    ?on_iface_mouse_move@MainInterface@@UAEXHH@Z
// LEVER: 2026-09-05 the receiver map. Both spot_ and iface_redraw_pending_ are
//   entry-relative and the two redraw_complete calls are the compiler's own
//   -0xA14. The status bar's two ids are the 0x008C6B2C/0x008C6B30 pair, and
//   the second Spot is at 0x008C6B48.
// size      271 bytes
// prototype void (__thiscall ?on_iface_mouse_move@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FAB00 0x004B4CF0 0x0045C3A0 0x004B6530
void MainInterface::on_iface_mouse_move(int a1, int a2) {
    // VC6 packs the two Spot outputs into the dead parameter slots while the
    // coordinates stay in registers for the second hit-test.
    int type;
    int id;
    if (spot_.check(a1, a2, &type, &id) != -1 && type == 5) {
        if (iface_redraw_pending_ != 0) {
            return;
        }
        iface_redraw_pending_ = 1;
        redraw_complete();
        return;
    }

    int check2 = ConsoleStatusWin.spot_.check(a1, a2, &type, &id);
    if (check2 >= 0 && id == 0xb) {
        if (type != ConsoleStatusWin.hover_base_id_) {
            ConsoleStatusWin.hover_base_id_ = type;
            ConsoleStatusWin.hover_vehicle_id_ = -1;
            ConsoleStatusWin.draw_active(-1, type, 0);
        }
    } else if (check2 >= 0 && id == 0) {
        if (type != ConsoleStatusWin.hover_vehicle_id_) {
            ConsoleStatusWin.hover_base_id_ = -1;
            ConsoleStatusWin.hover_vehicle_id_ = type;
            ConsoleStatusWin.draw_active(type, -1, 0);
        }
    } else {
        ConsoleStatusWin.reset_active();
    }

    if (iface_redraw_pending_ == 1) {
        iface_redraw_pending_ = 0;
        redraw_complete();
    }
}

// Eight faction foreground colors, independently paired with eight shadows.
// load_faction_art4537D9/4537E8 writes the two banks and load_vehicles
// 45443C/454446 publishes entry0. The following 68FA70 string is NOT an
// array tail. Original chat caller537AD5 passes the packet faction index;
// malformed packet indices remain unvalidated, not a larger storage owner.
// Other original-only color users and load_faction_art still need recovery.
// RECOVERY_OBJECT: v2 0x0068FA30 0x0068FA50 ?ChatItemIdTable@@3PAHA 0x00454270
int ChatItemIdTable[8] = {
    249, 111, 252, 255, 251, 65, 102, 187,
};
// RECOVERY_OBJECT: v2 0x0068FA50 0x0068FA70 ?ChatShadowColors@@3PAHA 0x00454270
int ChatShadowColors[8] = {
    134, 177, 172, 55, 146, 51, 145, 169,
};

// ORIGINAL: 0x0045C0F0 ?set_message@MainInterface@@QAEXPADHHHH@Z 0x0045C0F0-0x0045C163;0x00655745-0x00655757 BYTE_EXACT
// LEVER: 2026-09-05 homed from 0045c0f0.cpp, exactly as that record's own
//   LEVER line says: the LOCAL `Font font;` is what earns the SEH frame
//   (`push -1 / push 0x65574d / mov fs:[0]`) - a non-trivially-destructible
//   automatic under /GX - and direct calls to string_box_.add and the real
//   InterfaceMessageWin reproduce the rest. Nothing names the three ints.
//   `ret 0x14` pops this plus four arguments, so the signature is four ints
//   after the text, exactly as the mangled name spells - none of them read.
// size      114 bytes
// prototype void (__thiscall ?set_message@MainInterface@@QAEXPADHHHH@Z)(MainInterface* this, int8*, int, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00471530 0x00618EA0 0x00618EE0 0x00629710
void MainInterface::set_message(char *text, int, int, int, int) {
    Font font;
    string_box_.add(text, 0x1388, 2);
    InterfaceMessageWin.add_message(text, 0);
}

// ORIGINAL: 0x0045C0B0 ?chat@MainInterface@@QAEXPADH@Z 0x0045C0B0-0x0045C0E6 BYTE_EXACT
// LEVER: 2026-09-05 homed from 0045c0b0.cpp. No local Font here and so no SEH
//   frame - the body is one straight line. `mov ecx, 0x7f67f8` is the real
//   InterfaceMessageWin, and `[eax*4 + 0x68fa30]` is ChatItemIdTable indexed
//   by the channel, which VC6 folds into the SIB addressing on its own.
// size      53 bytes
// prototype void (__thiscall ?chat@MainInterface@@QAEXPADH@Z)(MainInterface* this, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00471530 0x00629710
void MainInterface::chat(char *text, int channel) {
    string_box_.add(text, 0x2710, 2);
    InterfaceMessageWin.add_message(text, ChatItemIdTable[channel]);
}

// RECOVERY_OBJECT: v2 0x007D3930 0x007D3934 ?MainInterfaceTileFlag@@3HA 0x0045C290
int MainInterfaceTileFlag;

// The four frame strips share one inset tile helper; the lower strip
// widens one pixel back to the left.
static inline void restore_back_strip(RECT *rect, int widen) {
    RECT r = *rect;
    ++r.left;
    --r.right;
    ++r.top;
    --r.bottom;
    r.left -= widen;
    MainInterfaceGlobal.Buffer::tile(InterfaceBoxStyles[9].mask_, r.left, r.top, r.left, r.top,
                                     r.right - r.left, r.bottom - r.top);
}

// ORIGINAL: 0x0045C290 ?restore_back@MainInterface@@QAEXH@Z 0x0045C290-0x0045C393 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The receiver owns the six frame RECTs; drawing uses the separate process-global MainInterfaceGlobal Buffer, not the receiver's Buffer.","evidence":["pinned:0x0045C290 reads receiver+D80/D90/DA0/DB0/DC0/DD0","src/maininterface.h:rect4_,right_frame_rect_,info_panel_frame_,lower_frame_rect_,rect1_,lower_animation_bounds_","src/maininterface.h:MainInterfaceGlobal;src/buffer.h:restore_clip uses rect2_; global+444 is Buffer and global+474 is rect2_"]}
// size      259 bytes
// prototype void (__thiscall ?restore_back@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   12   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DA860

// Kept out of line: the image calls it from on_redraw.
#pragma auto_inline(off)
void MainInterface::restore_back(int a1) {
    if (a1 != 0) {
        MainInterfaceGlobal.Buffer::restore_clip();
    }
    restore_back_strip(&rect4_, 0);
    restore_back_strip(MainInterfaceTileFlag != 0 ? &lower_animation_bounds_
                                                  : &lower_frame_rect_, 1);
    restore_back_strip(MainInterfaceTileFlag != 0 ? &rect1_ : &info_panel_frame_, 0);
    restore_back_strip(&right_frame_rect_, 0);
}
#pragma auto_inline(on)


/*
// ORIGINAL: 0x0045C180 ?timer_callback_daemon@MainInterface@@QAEXH@Z 0x0045C180-0x0045C1F4 FILE BYTE_EXACT
// TRIED: the scaffold bound the IAT slot as `static int *const g_00669328`
// and dispatched through its own `UnionRectProc` typedef; the slot is
// USER32!UnionRect (tools/iat_names.py), and windows.h already declares it, so
// TRIED: 2026-09-03 homing pass. The body is faithful - alert_/time1_/time2_
//        by name, the switch and the UnionRect call all land - and it still
//        measures MISMATCH 6/42, because the image CALLS Time::stop
//        (0x00616730) here while time.h keeps `stop` in-class MEASURED so
//        that `close` and `~Time` inline it. VC6 has no __declspec(noinline),
//        so the one source cannot serve both call sites: this tree expands
//        the whole teardown (the `not edi / and edi, 1` flush test) in both
//        switch arms, 80 instructions against the image's 42. Same ceiling
//        time.h already documents for port_to_port / has_tech /
//        do_all_non_input. Fixing it means splitting `stop` into an in-class
//        inline for `close`/`~Time` and an out-of-line definition for the
//        daemon callers - time.h, not this TU.
// TRIED: the scaffold bound the IAT slot as `static int *const g_00669328`
// `call dword ptr [__imp_UnionRect]`. The offsets the scaffold cast through
// are the declared members alert_ / time1_ / time2_
// (tools/header_offsets.py: 0x216f0 / 0x25040 / 0x25068), so the body reads
// them by name now.
*/
void MainInterface::timer_callback_daemon(int a1) {
    RECT local;
    switch (a1) {
    case 0:
        time1_.stop();
        break;
    case 1:
        if (alert_ == -1) {
            time2_.stop();
            return;
        }
        alert_ += 1;
        UnionRect(&local,
                  &menu_animation_bounds_, &multiplayer_animation_bounds_);
        GraphicWin::soft_update(&local);
        break;
    }
}

// Fixed-slot bindings carried from 0045c5c0.cpp. IfaceRegistry is typed as
// the registry entry the receiver map in maininterface.h names, so the
// dispatch is a call by name instead of a shim.
// The original id-to-slot run ends where the separate registry begins.
// Bind its start; the exact historical allocation remains to be checked.
// RECOVERY_OBJECT: v1 0x007AE778 ?IfaceIdToSlotStorage@@3PAHA
int IfaceIdToSlotStorage[30];
// RECOVERY_OBJECT: v1 0x007D392C ?ActiveIfaceIdStorage@@3HA
int ActiveIfaceIdStorage;

bool __cdecl active_interface_slot_occupied() {
    return IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 0;
}

// The MessageWin the interface events are delivered to is now the real global
// InterfaceMessageWin (messagewin.h / messagewin.cpp) - game.cpp's system_init
// builds the same object through InterfaceMessageWin.init(), and the image's
// handlers reach it by its address, `mov ecx, 0x7f67f8` before the call.

// ORIGINAL: 0x0045D140 ?on_iface_selected@MainInterface@@QAEXHH@Z 0x0045D140-0x0045D159 BYTE_EXACT
// symbol    ?on_iface_selected@MainInterface@@UAEXHH@Z
// LEVER: 2026-09-05 homed from 0045d140.cpp once InterfaceMessageWin became a
//   real global (messagewin.h / messagewin.cpp). The body is one line - the
//   handoff to the desktop's MessageWin - and the receiver never touches its
//   own SubInterface subobject, so it is byte-exact homed on the registry
//   entry exactly as it was as a MainInterface method. `mov ecx, 0x7f67f8`
//   falls out of naming the global: a const pointer binding costs the call
//   its `E8` (the brief's first lever) and a real object restores it. The
//   `symbol` fact is the mangled name this tree emits - slot 10 of the
//   interface table maininterface.h declares, hence `U` (virtual) on
//   IfaceRegistryEntry rather than `Q` on MainInterface - and the `fact`
//   value is one token by contract, so the explanation lives here.
// size      25 bytes
// prototype void (__thiscall ?on_iface_selected@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004718C0
void MainInterface::on_iface_selected(int code, int mode) {
    InterfaceMessageWin.on_selected(code, mode);
}


// ORIGINAL: 0x0045C5C0 ?on_scrolling@MainInterface@@QAEXHH@Z 0x0045C5C0-0x0045C5F4 FILE BYTE_EXACT
// symbol    ?on_scrolling@MainInterface@@UAEXHH@Z
// LEVER: 2026-09-04 every dispatch in this file went from a VCall shim to a
//   named virtual - IfaceRegistryEntry (maininterface.h) declares the
//   nineteen slots the concrete interface table at 0x66C544 installs, so
//   `target->on_iface_left_click(a1, a2)` is the image's
//   `mov edx,[ecx] / call dword ptr [edx+0xNN]` with no shim. All sixteen
//   bodies that dispatch re-measured BYTE_EXACT through the change. Binding
//   IfaceRegistry as `IfaceRegistryEntry *const *const` instead of
//   `int *const` costs the match: VC6 then materialises the table pointer
//   with its own `mov reg,[imm]` instead of folding the index into the
//   address, so the binding stays `int *const` and the cast happens at the
//   use.
// size      52 bytes
// prototype void (__thiscall ?on_scrolling@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C5ED
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C5C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c5c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_scrolling@MainInterface@@QAEXHH@Z  at 0x0045C5C0  (52 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_scrolling(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        SubInterface *target = InterfaceRegistry[subindex];
        if (target != 0) {
            target->on_iface_scrolling(a1, a2);
        }
    }
}


// ORIGINAL: 0x0045C640 ?on_left_double_click@MainInterface@@QAEXHH@Z 0x0045C640-0x0045C674 FILE BYTE_EXACT
// symbol    ?on_left_double_click@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_left_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C66D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C640
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c640/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_double_click@MainInterface@@QAEXHH@Z  at 0x0045C640  (52 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_left_double_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        SubInterface *target = InterfaceRegistry[subindex];
        if (target != 0) {
            target->on_iface_left_double_click(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C680 ?on_right_double_click@MainInterface@@QAEXHH@Z 0x0045C680-0x0045C6B4 FILE BYTE_EXACT
// symbol    ?on_right_double_click@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_right_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C6AD
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C680
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c680/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_double_click@MainInterface@@QAEXHH@Z  at 0x0045C680  (52 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_right_double_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        SubInterface *target = InterfaceRegistry[interface_index];
        if (target != 0) {
            target->on_iface_right_double_click(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C6C0 ?on_left_up@MainInterface@@QAEXHH@Z 0x0045C6C0-0x0045C6F4 FILE BYTE_EXACT
// symbol    ?on_left_up@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_left_up@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C6ED
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C6C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c6c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_up@MainInterface@@QAEXHH@Z  at 0x0045C6C0  (52 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_left_up(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int group = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        SubInterface *target = InterfaceRegistry[group];
        if (target) {
            target->on_iface_left_up(a1, a2);
        }
    }
}


// ORIGINAL: 0x0045C700 ?on_left_click@MainInterface@@QAEXHH@Z 0x0045C700-0x0045C734 FILE BYTE_EXACT
// symbol    ?on_left_click@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_left_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C72D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C700
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c700/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_click@MainInterface@@QAEXHH@Z  at 0x0045C700  (52 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_left_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        SubInterface *target = InterfaceRegistry[subindex];
        if (target != 0) {
            target->on_iface_left_click(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C740 ?on_right_click@MainInterface@@QAEXHH@Z 0x0045C740-0x0045C774 FILE BYTE_EXACT
// symbol    ?on_right_click@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_right_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C76D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C740
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c740/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_click@MainInterface@@QAEXHH@Z  at 0x0045C740  (52 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_right_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        SubInterface *target = InterfaceRegistry[interface_index];
        if (target != 0) {
            target->on_iface_right_click(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C780 ?on_left_down@MainInterface@@QAEXHH@Z 0x0045C780-0x0045C7B4 FILE BYTE_EXACT
// symbol    ?on_left_down@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_left_down@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C7AD
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C780
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c780/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_down@MainInterface@@QAEXHH@Z  at 0x0045C780  (52 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_left_down(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int group = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        SubInterface *target = InterfaceRegistry[group];
        if (target) {
            target->on_iface_left_down(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C7C0 ?on_right_down@MainInterface@@QAEXHH@Z 0x0045C7C0-0x0045C7F4 FILE BYTE_EXACT
// symbol    ?on_right_down@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_right_down@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C7ED
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C7C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c7c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_down@MainInterface@@QAEXHH@Z  at 0x0045C7C0  (52 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

void MainInterface::on_right_down(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        SubInterface *target = InterfaceRegistry[subindex];
        if (target != 0) {
            target->on_iface_right_click(a1, a2);
        }
    }
}

// Fixed-slot bindings carried from 0045c830.cpp


// ORIGINAL: 0x0045C830 ?on_button_pressed@MainInterface@@QAEHH@Z 0x0045C830-0x0045C866 FILE BYTE_EXACT
// symbol    ?on_button_pressed@MainInterface@@UAEHH@Z
// size      54 bytes
// prototype int (__thiscall ?on_button_pressed@MainInterface@@QAEHH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C859
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C830
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c830/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_button_pressed@MainInterface@@QAEHH@Z  at 0x0045C830  (54 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


int MainInterface::on_button_pressed(int a1) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index1 = ActiveIfaceIdStorage;
        int index2 = IfaceIdToSlotStorage[index1];
        SubInterface *obj = InterfaceRegistry[index2];
        if (obj != 0) {
            return obj->on_iface_button_pressed(a1);
        }
    }
    return 0;
}

// ORIGINAL: 0x0045C870 ?on_button_toggled@MainInterface@@QAEXHH@Z 0x0045C870-0x0045C8A4 FILE BYTE_EXACT
// symbol    ?on_button_toggled@MainInterface@@UAEXHH@Z
//           Q->U 2026-09-06, Win's front slots 44/45 taking the image's
//           own names (on_button_clicked / on_button_toggled): the body is
//           now an override of the window's slot, so VC6 mangles it virtual
//           and enters it on the front table, while the image spells the
//           same body Q and its own table holds the raw address.
// size      52 bytes
// prototype void (__thiscall ?on_button_toggled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C89D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C870
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c870/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_button_toggled@MainInterface@@QAEXHH@Z  at 0x0045C870  (52 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_button_toggled(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        SubInterface *target = InterfaceRegistry[interface_index];
        if (target != 0) {
            target->on_iface_button_toggled(a1, a2);
        }
    }
}


// ORIGINAL: 0x0045C8B0 ?on_dialog_back_draw@MainInterface@@QAEHPAUGraphicWin@@@Z 0x0045C8B0-0x0045C8E6 FILE BYTE_EXACT
// symbol    ?on_dialog_back_draw@MainInterface@@UAEHPAVGraphicWin@@@Z
// size      54 bytes
// prototype int (__thiscall ?on_dialog_back_draw@MainInterface@@QAEHPAUGraphicWin@@@Z)(MainInterface* this, GraphicWin*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C8D9
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C8B0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c8b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_dialog_back_draw@MainInterface@@QAEHPAUGraphicWin@@@Z  at 0x0045C8B0  (54 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


int MainInterface::on_dialog_back_draw(GraphicWin* a1) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index2 = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        SubInterface *obj = InterfaceRegistry[index2];
        if (obj != 0) {
            return obj->on_iface_dialog_back_draw(a1);
        }
    }
    return 0;
}

// ORIGINAL: 0x0045C8F0 ?on_dialog_item_back_draw@MainInterface@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x0045C8F0-0x0045C932 FILE BYTE_EXACT
// symbol    ?on_dialog_item_back_draw@MainInterface@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
// size      66 bytes
// prototype int (__thiscall ?on_dialog_item_back_draw@MainInterface@@QAEHPAUGraphicWin@@HHPAURECT@@@Z)(MainInterface* this, GraphicWin*, int, int, RECT*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C925
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C8F0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c8f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_dialog_item_back_draw@MainInterface@@QAEHPAUGraphicWin@@HHPAURECT@@@Z  at 0x0045C8F0  (66 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


int MainInterface::on_dialog_item_back_draw(GraphicWin* a1, int a2, int a3, RECT* a4) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int idx1 = ActiveIfaceIdStorage;
        int idx2 = IfaceIdToSlotStorage[idx1];
        SubInterface *obj = InterfaceRegistry[idx2];
        if (obj != 0) {
            return obj->on_iface_dialog_item_back_draw(a1, a2, a3, a4);
        }
    }
    return 0;
}


// ORIGINAL: 0x0045C980 ?on_mouse_move@MainInterface@@QAEXHH@Z 0x0045C980-0x0045C9B4 BYTE_EXACT
// symbol    ?on_mouse_move@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_mouse_move@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C9AD
// 0x0045C980  ?on_mouse_move@MainInterface@@QAEXHH@Z  ->  ?on_mouse_move@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MainInterface::on_mouse_move(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int mid = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        SubInterface *obj = InterfaceRegistry[mid];
        if (obj != 0) {
            obj->on_iface_mouse_move(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C940 ?on_group_clicked@MainInterface@@QAEXHHH@Z 0x0045C940-0x0045C978 FILE BYTE_EXACT
// symbol    ?on_group_clicked@MainInterface@@UAEXHHH@Z
// size      56 bytes
// prototype void (__thiscall ?on_group_clicked@MainInterface@@QAEXHHH@Z)(MainInterface* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C971
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C940
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c940/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_group_clicked@MainInterface@@QAEXHHH@Z  at 0x0045C940  (56 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_group_clicked(int a1, int a2, int a3) {
    if (WorldClimateSkipTerrainClear != 1u) {
        SubInterface *target = InterfaceRegistry[IfaceIdToSlotStorage[ActiveIfaceIdStorage]];
        if (target != 0) {
            target->on_iface_group_clicked(a1, a2, a3);
        }
    }
}


// ORIGINAL: 0x0045C9C0 ?on_mouse_leave@MainInterface@@QAEXHH@Z 0x0045C9C0-0x0045C9F4 BYTE_EXACT
// symbol    ?on_mouse_leave@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_mouse_leave@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C9ED
// 0x0045C9C0  ?on_mouse_leave@MainInterface@@QAEXHH@Z  ->  ?on_mouse_leave@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MainInterface::on_mouse_leave(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        SubInterface *target = InterfaceRegistry[subindex];
        if (target != 0) {
            target->on_iface_mouse_leave(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045CA00 ?on_selected@MainInterface@@QAEXHH@Z 0x0045CA00-0x0045CA34 FILE BYTE_EXACT
// symbol    ?on_selected@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_selected@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045CA2D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045CA00
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045ca00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_selected@MainInterface@@QAEXHH@Z  at 0x0045CA00  (52 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_selected(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        SubInterface *target = InterfaceRegistry[interface_index];
        if (target != 0) {
            target->on_iface_selected(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045D2A0 ?set_alert@MainInterface@@QAEXH@Z 0x0045D2A0-0x0045D2D1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Alert state and timer are owned by this MainInterface instance, not fixed image-address views.","evidence":["pinned:0x0045D2A0 writes receiver+216F0 before Time::start on receiver+25068","src/maininterface.h:alert_,time2_;src/maininterface.cpp:timer_callback_daemon"]}
// size      49 bytes
// prototype void (__thiscall ?set_alert@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00616650


void MainInterface::set_alert(int a1) {
    if (a1 != 0) {
        alert_ = 0;
        time2_.start();
        return;
    }
    alert_ = -1;
}

// Fixed-slot bindings carried from 0045c600.cpp

// ORIGINAL: 0x0045C600 ?on_scrolled@MainInterface@@QAEXHH@Z 0x0045C600-0x0045C634 FILE BYTE_EXACT
// symbol    ?on_scrolled@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_scrolled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C62D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C600
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c600/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_scrolled@MainInterface@@QAEXHH@Z  at 0x0045C600  (52 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_scrolled(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int group = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        SubInterface *target = InterfaceRegistry[group];
        if (target) {
            target->on_iface_scrolled(a1, a2);
        }
    }
}


/*
// ORIGINAL: 0x0045C200 ?timer_callback_daemon@MainInterface@@QAAXHH@Z 0x0045C200-0x0045C273 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: 0x45C200 reads callback parameters from [ebp+8]/[ebp+0xC], branches on mode 0/1, and accesses context+0x216F0,+0x25040,+0x25068,+0xDE0,+0xDF0", "src/maininterface.h declares MainInterface at the exact receiver layout, with alert_ at +0x216F0, time1_ at +0x25040, time2_ at +0x25068, and the corresponding RECT members at +0xDE0/+0xDF0", "MainInterface::init registers the free callback with borrowed this context for both embedded Time instances; mode 0/1 selects time1_/time2_", "neighboring mapped MainInterface::timer_callback_daemon(int) and set_alert(int) access the same genuine members; the callback remains a free two-argument __cdecl entry"], "reason": "Keep the exact free callback entry, express the borrowed context as the existing MainInterface owner, and use that owner's fields. The friend declaration grants only this callback access; it does not turn it into a member or alter registration/lifetime. The stripped catalogue spelling is not proof of the intended source type."}
// symbol    ?main_interface_timer_callback@@YAXHPAX@Z
// TRIED: both stack parameters are read directly ([ebp+8], [ebp+0xc]) with no
//        `this` anywhere, so the scaffold's MainInterface member cannot be
//        defined out-of-line - that adds an implicit `this` ahead of them and
//        shifts both to +0xc/+0x10. It is a free function here.
// TRIED: the same `Time::stop` ceiling as 0x0045C180. The image CALLS
//        0x00616730; time.h keeps `stop` in-class MEASURED so `close`/`~Time`
//        inline it, and VC6 has no noinline. Demoted rather than mismatched
//        forever; split `stop` in time.h and both bodies re-earn.
*/
void __cdecl main_interface_timer_callback(int a1, void *context) {
    if (context == nullptr) return;

    MainInterface *self = static_cast<MainInterface *>(context);
    switch (a1) {
    case 0:
        self->time1_.stop();
        break;
    case 1: {
        if (self->alert_ == static_cast<uint32_t>(-1)) {
            self->time2_.stop();
            return;
        }
        self->alert_ += 1;

        RECT rc;
        UnionRect(&rc, &self->menu_animation_bounds_,
                  &self->multiplayer_animation_bounds_);
        self->soft_update(&rc);
        break;
    }
    }
}

/*
Purpose: Hand the clicked button id to whichever interface mode is active.
// ORIGINAL: 0x0045C800 ?on_button_clicked@MainInterface@@QAEXH@Z 0x0045C800-0x0045C830 BYTE_EXACT
// size      48 bytes
// prototype void (__thiscall ?on_button_clicked@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C829
// LEVER: homed 2026-09-04 by the receiver map. The receiver is never touched,
//   so the body is the two guards and the dispatch - and the dispatch is now
//   a call by name: IfaceRegistryEntry declares the nineteen slots the
//   concrete table at 0x66C544 installs, slot 13 (offset 0x34) being
//   on_iface_button_clicked, so `target->on_iface_button_clicked(a1)` is the
//   image's `mov edx,[ecx] / call dword ptr [edx+0x34]` and the VCall shim
//   that used to stand in for it is gone. receiver stays unspelled because
//   the registry entries are SubInterface subobjects (front+0xA14), not
//   windows, and this method's own `this` never reaches the body.
// symbol    ?on_button_clicked@MainInterface@@UAEXH@Z
//           was `QAEXH@Z - unchanged; this one is a MainInterface member and
//           stays one`, until 2026-09-06 when Win's front slots 44/45 took
//           the image's own names: this body now overrides Win's slot 44, so
//           VC6 mangles it virtual and enters it on the front table
//           (0x66C648 holds it) while the image still spells the body Q.
Return Value: n/a
Status: homed from src/recovered/0045c800.cpp on this pass.
*/
void MainInterface::on_button_clicked(int a1) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        SubInterface *target = InterfaceRegistry[subindex];
        if (target != 0) {
            target->on_iface_button_clicked(a1);
        }
    }
}

// Original interface table 0x0066C544 slot 6 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_left_up@MainInterface@@UAEXHH@Z 0x0045CA40
void MainInterface::on_iface_left_up(int, int) {}

// Original interface table 0x0066C544 slot 17 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_dialog_back_draw@MainInterface@@UAEHPAVGraphicWin@@@Z 0x0045CA40
int MainInterface::on_iface_dialog_back_draw(GraphicWin *) {
    return 0;
}

// Original interface table 0x0066C544 slot 18 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404240 0x00404245 ?on_iface_dialog_item_back_draw@MainInterface@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z 0x0045CA40
int MainInterface::on_iface_dialog_item_back_draw(GraphicWin *, int, int,
                                                   RECT *) {
    return 0;
}

// ORIGINAL: 0x004B1B70 ?bubble_me_elmo2@SocialWin@@QAGXPADHH@Z 0x004B1B70-0x004B1BB2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-interface-callback-owner", "Pinned4B1B70..4B1BB2 is a three-argument stdcall callback using explicit MainInterfaceGlobal7AE820 and its primary Win base. Historical SocialWin label remains canonical identity but does not imply a receiver. Converts coordinates before passing a point RECT to Win5F83D0.", "historical seed:src/recovered/004b1b70.cpp"], "reason": "Recover typed UI helper in its genuine source owner, retaining original identity, spans and historical obligations; remove superseded fixed-address seed."}
// symbol    ?main_interface_bubble_callback@@YGXPADHH@Z
void __stdcall main_interface_bubble_callback(char *text, int x, int y) {
    MainInterfaceGlobal.client_to_screen(&x, &y);
    RECT rectangle;
    rectangle.left = x;
    rectangle.top = y;
    rectangle.right = x;
    rectangle.bottom = y;
    MainInterfaceGlobal.Win::set_bubble_text(text, &rectangle);
}

// ORIGINAL: 0x004396B0 ?bubble_me_elmo2@DesignWin@@QAGXPADHH@Z 0x004396B0-0x004396F2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/004396b0.cpp", "task:owned-helper-coverage-batch", "Pinned4396B0 ignores incomingECX, consumes text/x/y from stack and returns ret0C; MainInterfaceGlobal7AE820 is the actual receiver. Historical DesignWin label retained, no caller/owning table claim. Distinct original entry and body from existing4B1B70, never a shared-span alias."], "reason": "Recover a complete native helper through genuine source owners and SDK calls, preserving original identity, extent and historical obligations. Source home is not proof of original TU or complete-game acceptance."}
// symbol    ?main_interface_bubble_callback_4396b0@@YGXPADHH@Z
void __stdcall main_interface_bubble_callback_4396b0(char *text, int x, int y) {
    MainInterfaceGlobal.client_to_screen(&x, &y);
    RECT point;
    point.left = x;
    point.top = y;
    point.right = x;
    point.bottom = y;
    MainInterfaceGlobal.Win::set_bubble_text(text, &point);
}

/*
// ORIGINAL: 0x004325B0 ??1ButtonGroup@@QAE@XZ 0x004325B0-0x004325B5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original4325B0..4325B5 jumps to ButtonGroup::close62B7F0.", "MainInterface constructor45EF10 constructs four embedded ButtonGroup objects at offset1E878, stride94, with original destructor callback4325B0.", "PrefWin finalizer48D540 loads its ButtonGroup object8577F0 and jumps directly to close62B7F0; an out-of-line-only destructor adds an incorrect intermediate target.", "Ordinary inline destruction preserves cleanup while the real MainInterface member array naturally requires the standalone callback. No dummy use, export, virtualness or compiler-policy change."], "reason": "ButtonGroup owns the inline destructor body. This current translation unit emits its standalone array-lifetime entry; original translation-unit provenance remains unknown."}
// body src/buttongroup.h
// size      5 bytes
// prototype void (__thiscall ??1ButtonGroup@@QAE@XZ)(ButtonGroup* this)
// callers   0   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)
*/
// RECOVERY_GENERATED: v1 vtable 0x0066C598 ? ??_7MainInterface@@6BWin@@@ 0x0045EF10

// These SubInterface mode-stack methods use the shared registry and mode
// storage declared with MainInterface. ReportIf::done calls the shared
// declaration, preserving its out-of-line release boundary.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0045D2E0 ?delete_iface_mode@SubInterface@@QAEXXZ 0x0045D2E0-0x0045D303 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/0045d2e0.cpp", "image:terranx_original.exe:0x0045D2E0", "source:src/maininterface.h:InterfaceRegistry and mode-stack storage; source:src/multiwin.h:owned Sprite offsets; native callback-sensitive mode reload and downward repeated removal", "recovery/tasks/interface-contract-report-closure.json; owner expansion interface-registry-multiwin-small-20261001"], "reason": "Recover the original in its genuine C++ owner using source-owned interface registry and mode stack, real virtual callbacks, or owned MultiWin sprite geometry. Preserve historical seed obligations; this binding grants no current byte or runtime claim."}
void SubInterface::delete_iface_mode() {
    for (int index = 0; index < 12; ++index) {
        if (InterfaceRegistry[index] == this) {
            InterfaceRegistry[index] = 0;
            return;
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0045D310 ?set_iface_mode@SubInterface@@QAEXXZ 0x0045D310-0x0045D37D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0045d310.cpp", "image:terranx_original.exe:0x0045D310", "source:src/maininterface.h:InterfaceRegistry and mode-stack storage; source:src/multiwin.h:owned Sprite offsets; native callback-sensitive mode reload and downward repeated removal", "recovery/tasks/interface-contract-report-closure.json; owner expansion interface-registry-multiwin-small-20261001"], "reason": "Recover the original in its genuine C++ owner using source-owned interface registry and mode stack, real virtual callbacks, or owned MultiWin sprite geometry. Preserve historical seed obligations; this binding grants no current byte or runtime claim."}
void SubInterface::set_iface_mode() {
    int mode = static_cast<int>(field_4_);
    const int current = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
    if (current == mode) {
        return;
    }
    if (mode == 10 && current != 0) {
        return;
    }
    // The image repeats the mode/current comparison here.
    if (static_cast<int>(field_4_) != current) {
        SubInterface *previous = InterfaceRegistry[current];
        ++ActiveIfaceIdStorage;
        IfaceIdToSlotStorage[ActiveIfaceIdStorage] = static_cast<int>(field_4_);
        if (previous != 0) {
            previous->on_status(0);
        }
    }
    MainInterfaceGlobal.set_clip(MainInterfaceGlobal.buffer_working_rect());
    // Both callbacks may change the receiver's mode or registry entry.
    mode = static_cast<int>(field_4_);
    SubInterface *next = InterfaceRegistry[mode];
    if (next != 0) {
        next->on_status(1);
    }
}

// ORIGINAL: 0x0045D380 ?release_iface_mode@SubInterface@@QAEXXZ 0x0045D380-0x0045D43F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0045d380.cpp", "image:terranx_original.exe:0x0045D380", "source:src/maininterface.h:InterfaceRegistry and mode-stack storage; source:src/multiwin.h:owned Sprite offsets; native callback-sensitive mode reload and downward repeated removal", "recovery/tasks/interface-contract-report-closure.json; owner expansion interface-registry-multiwin-small-20261001"], "reason": "Recover the original in its genuine C++ owner using source-owned interface registry and mode stack, real virtual callbacks, or owned MultiWin sprite geometry. Preserve historical seed obligations; this binding grants no current byte or runtime claim."}
// TRIED: byte-exact - a stack-search-and-shift over a linear "iface mode" stack (0x7AE778) plus a parallel object table (0x7AE7F0); the found-and-shifted case does not break, it keeps scanning downward exactly like the disassembly, so no attempt used `break`. 59% mnemonic similarity; landed for the control-flow shape (nested for-loops, no early exit) rather than chase the remaining scheduling/frame differences.
void SubInterface::release_iface_mode() {
    int depth = ActiveIfaceIdStorage;
    if (depth != 0) {
        if (IfaceIdToSlotStorage[depth] != static_cast<int>(field_4_)) {
            // Remove every buried occurrence, scanning downward after each
            // shift. The original does not stop after the first match.
            for (int index = depth - 1; index >= 1; --index) {
                if (IfaceIdToSlotStorage[index] == static_cast<int>(field_4_)) {
                    for (int move = index; move < depth; ++move) {
                        IfaceIdToSlotStorage[move] = IfaceIdToSlotStorage[move + 1];
                    }
                    --depth;
                    ActiveIfaceIdStorage = depth;
                }
            }
        } else {
            --depth;
            ActiveIfaceIdStorage = depth;
            const int mode = static_cast<int>(field_4_);
            const int next_mode = IfaceIdToSlotStorage[depth];
            if (mode != next_mode) {
                SubInterface *previous = InterfaceRegistry[mode];
                if (previous != 0) {
                    previous->on_status(0);
                }
            }
            MainInterfaceGlobal.set_clip(MainInterfaceGlobal.buffer_working_rect());
            SubInterface *next = InterfaceRegistry[next_mode];
            if (next != 0) {
                next->on_status(1);
            }
        }
    }
}

// ORIGINAL: 0x0045D440 ?slide_show@@YAXPAUGraphicWin@@H@Z 0x0045D440-0x0045D966;0x00655757-0x00655777
// RECOVERY_BINDING: v1 {"evidence": ["git:3e11f655:src/recovered/units/0045d440.cpp complete original identity/spans and historical discoveries", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x45d440 full native stack/register/call/lifetime audit.", "source:src/maininterface.h existing Sprite arrays; src/buffer.h source-receiver copy and signed DIB height; src/console.h real virtual GraphicWin base, no raw-offset alias.", "source:src/maininterface.cpp: the helper uses MainInterfaceGlobal-owned slide/scroll sprites, interface geometry and shared transition state; its Wave configuration belongs to MainInterface::init.", "pinned:Pinned cold unwind regions remain consecutive with MainInterface dtor/ctor/init/set_message: 0x6552C0-0x65552F, 0x655530-0x655723, 0x655730-0x655745, 0x655745-0x655757, followed by these four helper spans 0x655757-0x655777, 0x655777-0x6557A2, 0x6557A2-0x6557CD, and 0x6557CD-0x6557F8; no other mapped span intersects this detached range."], "reason": "Keep this free ?slide_show UI transition helper with the MainInterface resource and lifetime provider while preserving its existing __cdecl signature and external window callers. This is an inferred source correspondence, not proof of an original filename or historical TU boundary."}
// symbol    ?slide_show@@YAXPAVGraphicWin@@H@Z
void __cdecl slide_show(GraphicWin *window, int mode) {
    Buffer frame_buffer;
    Buffer background;

    if (window != 0 && window->Win::is_visible()) {
        return;
    }

    if (window != 0) {
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SOUND_EFFECTS) != 0) {
            g_MAININTERFACE_WAVE.play();
        }

        const int frame_edge = WinScreenWidth == 800 ? 0x5E : 0xEE;
        const int frame_divisor = IsMultiplayerNet != 0 ? 3 : 5;
        RECT area = window->Buffer::clip_rect();
        window->client_to_screen(&area);
        background.init(MainInterfaceGlobal.Buffer::pixel_width(),
                        MainInterfaceGlobal.Buffer::pixel_height(), 0, 0);
        background.fill(WinFillColour);
        MainInterfaceGlobal.update_window_to_buffer(&background);

        frame_buffer.resize(window->Buffer::pixel_width(),
                            window->Buffer::pixel_height());
        frame_buffer.fill(WinFillColour);
        window->update_window_to_buffer(&frame_buffer);

        int frame_count = (area.bottom - area.top) / frame_divisor;
        if (area.top < area.bottom - frame_count * frame_divisor) {
            ++frame_count;
        }

        Win::screen_buffer_.set_clip(&area);
        RECT animation_strip = { area.left, 0, area.right, 0x39 };
        window->client_to_screen(&animation_strip);

        Sprite &left_outer = MainInterfaceGlobal.slide_frame_sprite(0);
        Sprite &left_inner = MainInterfaceGlobal.slide_frame_sprite(1);
        Sprite &right_inner = MainInterfaceGlobal.slide_frame_sprite(2);
        Sprite &right_outer = MainInterfaceGlobal.slide_frame_sprite(3);
        const int left_x = area.left;
        const int left_inner_x = left_x + left_outer.width();
        const int right_inner_x = area.left + area.right
            - right_inner.width() - right_outer.width() - 0x20;
        const int right_x = area.left + area.right
            - right_outer.width() - 0x20;

        int animation_y = area.bottom - frame_count;
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SLIDING_WINDOWS) != 0
            && IsMultiplayerNet == 0) {
            WinFlipClipped = 1;
            for (int frame = 0; frame < frame_divisor; ++frame) {
                const DWORD started = timeGetTime();
                background.draw(&Win::screen_buffer_, 9, area.left, animation_y, 1, 1);

                int left_y = animation_y;
                if (left_y <= frame_edge) left_y = frame_edge;
                left_outer.draw(&Win::screen_buffer_, left_outer.transparent_index(),
                                left_x, left_y, 1, 1);
                left_inner.draw(&Win::screen_buffer_, left_inner.transparent_index(),
                                left_inner_x, animation_y, 1, 1);
                right_inner.draw(&Win::screen_buffer_, right_inner.transparent_index(),
                                 right_inner_x, animation_y, 1, 1);
                int right_y = animation_y;
                if (right_y <= frame_edge) right_y = frame_edge;
                right_outer.draw(&Win::screen_buffer_, right_outer.transparent_index(),
                                 right_x, right_y, 1, 1);

                Win::screen_buffer_.set_clip(&animation_strip);
                background.draw(&Win::screen_buffer_, 9,
                    MainInterfaceGlobal.outer_rect().left,
                    MainInterfaceGlobal.outer_rect().top, 1, 1);
                Win::flip(&area);
                animation_y -= frame_count;
                if (animation_y <= area.top) animation_y = area.top;
                Win::screen_buffer_.set_clip(&area);
                do_sound();
                while (static_cast<DWORD>(timeGetTime() - started)
                       < static_cast<DWORD>(WindowAnimationFrameDelay)) {
                    do_sound();
                }
            }
            Win::screen_buffer_.restore_clip();
            WinFlipClipped = 0;
        }

        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SOUND_EFFECTS) != 0) {
            g_MAININTERFACE_WAVE.fade(0x32);
        }

        RECT window_clip = window->Buffer::clip_rect();
        window->Buffer::set_clip(&window_clip);
        int left_y = animation_y;
        if (left_y <= frame_edge) left_y = frame_edge;
        left_outer.draw(static_cast<Buffer *>(window), left_outer.transparent_index(),
                        0, left_y - area.top, 1, 1);
        left_inner.draw(static_cast<Buffer *>(window), left_inner.transparent_index(),
                        left_inner_x - area.left, 0, 1, 1);
        right_inner.draw(static_cast<Buffer *>(window), right_inner.transparent_index(),
                         right_inner_x - area.left, 0, 1, 1);
        int right_y = animation_y;
        if (right_y <= frame_edge) right_y = frame_edge;
        right_outer.draw(static_cast<Buffer *>(window), right_outer.transparent_index(),
                         right_x - area.left, right_y - area.top, 1, 1);
    }

    if ((mode & 2) != 0) {
        MainInterfaceGlobal.interface_down_sprite().draw(
            static_cast<Buffer *>(&MainInterfaceGlobal),
            MainInterfaceGlobal.interface_down_sprite().transparent_index(),
            MainInterfaceGlobal.bottom_bounds().left,
            MainInterfaceGlobal.bottom_bounds().top, 1, 1);
        MainInterfaceTileFlag = 1;
        RECT interface_clip = MainInterfaceGlobal.main_layout_rect();
        MainInterfaceGlobal.Buffer::set_clip(&interface_clip);
        MainInterfaceGlobal.restore_back(0);
        RECT main_buffer_clip = MainInterfaceGlobal.Buffer::clip_rect();
        MainInterfaceGlobal.Buffer::set_clip(&main_buffer_clip);
        MainInterfaceGlobal.soft_update();
    }
    WinFlipClipped = 0;
}
// ORIGINAL: 0x0045D970 ?slide_hide@@YAXPAUGraphicWin@@H@Z 0x0045D970-0x0045DEA0;0x00655777-0x006557A2
// RECOVERY_BINDING: v1 {"evidence": ["git:14a0bcaf:src/recovered/units/0045d970.cpp original hot/cold spans and retained seed findings.", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x45d970 full native register, rectangle mutation, receiver, call and lifetime audit.", "source:src/console.h virtual GraphicWin base; src/maininterface.h existing Sprite arrays and interface-up member; src/buffer.h source-receiver copy, full bounds and DIB geometry.", "source:src/maininterface.cpp: the helper uses MainInterfaceGlobal-owned slide/scroll sprites, interface geometry and shared transition state; its Wave configuration belongs to MainInterface::init.", "pinned:Pinned cold unwind regions remain consecutive with MainInterface dtor/ctor/init/set_message: 0x6552C0-0x65552F, 0x655530-0x655723, 0x655730-0x655745, 0x655745-0x655757, followed by these four helper spans 0x655757-0x655777, 0x655777-0x6557A2, 0x6557A2-0x6557CD, and 0x6557CD-0x6557F8; no other mapped span intersects this detached range."], "reason": "Keep this free ?slide_hide UI transition helper with the MainInterface resource and lifetime provider while preserving its existing __cdecl signature and external window callers. This is an inferred source correspondence, not proof of an original filename or historical TU boundary."}
// symbol    ?slide_hide@@YAXPAVGraphicWin@@H@Z
void __cdecl slide_hide(GraphicWin *window, int mode) {
    Buffer window_frame;          // native F498, constructed first
    Buffer interface_background;  // native FA20, constructed second
    Buffer screen_restore;        // native EF10, constructed third

    if (window != 0 && window->Win::is_visible() != 0
        && (GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SLIDING_WINDOWS) != 0
        && IsMultiplayerNet == 0) {
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SOUND_EFFECTS) != 0)
            g_MAININTERFACE_WAVE.play();

        const int frame_edge = WinScreenWidth == 0x320 ? 0x5e : 0xee;
        const int frame_divisor = (IsMultiplayerNet != 0
                                   || MapMouseFeedbackBusy != 0
                                   || ConsoleControlTurnActive != 0) ? 3 : 5;
        RECT area = window->Buffer::clip_rect();
        window->client_to_screen(&area);

        interface_background.init(MainInterfaceGlobal.Buffer::pixel_width(),
                                  MainInterfaceGlobal.Buffer::pixel_height(), 0, 0);
        interface_background.fill(WinFillColour);
        MainInterfaceGlobal.update_window_to_buffer(&interface_background);

        // Native initializes the third local before initializing/filling the first.
        // The declarations above still preserve compiler cleanup order.
        screen_restore.init(window->Buffer::pixel_width(),
                            window->Buffer::pixel_height(), 0, 0);

        window_frame.init(window->Buffer::pixel_width(),
                          window->Buffer::pixel_height(), 0, 0);
        window_frame.fill(WinFillColour);

        // Native 0x45DB05/1B clears the two client-side bands before capturing
        // the target frame at 0x45DB3A.
        window->fill(0, 0, 0x17, window->Buffer::pixel_height(), 9);
        window->fill(window->Buffer::pixel_width() - 0x17, 0, 0x17,
                     window->Buffer::pixel_height(), 9);
        window->update_window_to_buffer(&window_frame);
        window->update_back_to_window(&screen_restore);

        RECT animation_strip = { area.left, 0, area.right, 0x39 };
        MainInterfaceGlobal.client_to_screen(&animation_strip);
        const int frame_count = (area.bottom - area.top) / frame_divisor;

        Sprite &left_outer = MainInterfaceGlobal.slide_frame_sprite(0);
        Sprite &left_inner = MainInterfaceGlobal.slide_frame_sprite(1);
        Sprite &right_inner = MainInterfaceGlobal.slide_frame_sprite(2);
        Sprite &right_outer = MainInterfaceGlobal.slide_frame_sprite(3);
        const int left_x = area.left;
        const int left_inner_x = left_x + left_outer.width();
        const int right_inner_x = area.left + window_frame.pixel_width()
            - right_outer.width()
            - right_inner.width() - 0x20;
        const int right_outer_x = area.left + window_frame.pixel_width()
            - right_outer.width() - 0x20;
        int frame_y = area.top + frame_count;

        Win::screen_buffer_.set_clip(&area);
        WinFlipClipped = 1;
        // Native stores frame_divisor into its reused [ebp+8] scratch slot at
        // 0x45DA52; incoming mode at [ebp+0xC] is only tested for bit 2.
        for (int frame = 0; frame < frame_divisor; ++frame) {
            const DWORD started = timeGetTime();
            screen_restore.copy(&Win::screen_buffer_, 0, 0, area.left, area.top,
                                window->Buffer::pixel_width(),
                                window->Buffer::pixel_height());
            window_frame.draw(&Win::screen_buffer_, 9, area.left, frame_y, 1, 1);

            int left_y = frame_y;
            if (left_y <= frame_edge) left_y = frame_edge;
            left_outer.draw(&Win::screen_buffer_, left_outer.transparent_index(),
                            left_x, left_y, 1, 1);
            left_inner.draw(&Win::screen_buffer_, left_inner.transparent_index(),
                            left_inner_x, frame_y, 1, 1);
            right_inner.draw(&Win::screen_buffer_, right_inner.transparent_index(),
                             right_inner_x, frame_y, 1, 1);
            int right_y = frame_y;
            if (right_y <= frame_edge) right_y = frame_edge;
            right_outer.draw(&Win::screen_buffer_, right_outer.transparent_index(),
                             right_outer_x, right_y, 1, 1);

            Win::screen_buffer_.set_clip(&animation_strip);
            interface_background.draw(&Win::screen_buffer_, 9,
                MainInterfaceGlobal.outer_rect().left,
                MainInterfaceGlobal.outer_rect().top, 1, 1);
            Win::flip(&area);
            frame_y += frame_count;
            Win::screen_buffer_.set_clip(&area);
            do {
                do_sound();
                int delay = WindowAnimationFrameDelay;
                if (MapMouseFeedbackBusy != 0 || ConsoleControlTurnActive != 0)
                    delay = delay / 2;
                if (static_cast<DWORD>(timeGetTime() - started)
                    >= static_cast<DWORD>(delay))
                    break;
            } while (true);
        }
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SOUND_EFFECTS) != 0)
            g_MAININTERFACE_WAVE.fade(0x32);
        Win::screen_buffer_.restore_clip();
    }

    if ((mode & 2) != 0) {
        MainInterfaceGlobal.interface_up_sprite().draw(
            static_cast<Buffer *>(&MainInterfaceGlobal),
            MainInterfaceGlobal.interface_up_sprite().transparent_index(),
            MainInterfaceGlobal.bottom_bounds().left,
            MainInterfaceGlobal.bottom_bounds().top, 1, 1);
        MainInterfaceTileFlag = 0;
        RECT interface_clip = MainInterfaceGlobal.info_frame_rect();
        interface_clip.bottom += 0x64;
        RECT original_interface_clip = MainInterfaceGlobal.Buffer::clip_rect();
        MainInterfaceGlobal.Buffer::set_clip(&interface_clip);
        MainInterfaceGlobal.restore_back(0);
        MainInterfaceGlobal.Buffer::set_clip(&original_interface_clip);
        MainInterfaceGlobal.soft_update();
    }
    WinFlipClipped = 0;
}
// ORIGINAL: 0x0045DEA0 ?scroll_show@@YAXPAUGraphicWin@@PAUScroll@@@Z 0x0045DEA0-0x0045E535;0x006557A2-0x006557CD
// RECOVERY_BINDING: v1 {"evidence": ["git:3e11f655:src/unrecovered/0045dea0.cpp complete original identity/spans and historical discoveries", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x45dea0 full native stack/register/call/lifetime audit.", "source:src/maininterface.h existing Sprite arrays; src/buffer.h source-receiver copy and signed DIB height; src/console.h real virtual GraphicWin base, no raw-offset alias.", "source:src/maininterface.cpp: the helper uses MainInterfaceGlobal-owned slide/scroll sprites, interface geometry and shared transition state; its Wave configuration belongs to MainInterface::init.", "pinned:Pinned cold unwind regions remain consecutive with MainInterface dtor/ctor/init/set_message: 0x6552C0-0x65552F, 0x655530-0x655723, 0x655730-0x655745, 0x655745-0x655757, followed by these four helper spans 0x655757-0x655777, 0x655777-0x6557A2, 0x6557A2-0x6557CD, and 0x6557CD-0x6557F8; no other mapped span intersects this detached range."], "reason": "Keep this free ?scroll_show UI transition helper with the MainInterface resource and lifetime provider while preserving its existing __cdecl signature and external window callers. This is an inferred source correspondence, not proof of an original filename or historical TU boundary."}
// symbol    ?scroll_show@@YAXPAVGraphicWin@@PAVScroll@@@Z
// TRIED: full transcription of scroll_show (twin of the landed scroll_hide at 0x0045E540), using automatic Buffer ctor/dtor for the 3 local RAII objects (relying on the compiler's own SEH frame for unwind, not manual tracking), plain-register-name locals for the straight-line arithmetic, and ORIGINAL()/original_slot() for the two indirect vtable calls (slots 1 and 3). Compiles and matches through instr #9 (push vs lea) - not chased further, large SEH-heavy function, landed for coverage.
// The retained TRIED note describes the superseded initial transcription,
// not this owner body with real members, virtual calls and automatic lifetime.
void __cdecl scroll_show(GraphicWin *window, Scroll *scroll) {
    Buffer scroll_frame;
    Buffer interface_background;
    Buffer screen_save;

    if (scroll->Win::is_visible()) {
        return;
    }

    const int frame_edge = WinScreenWidth == 800 ? 0x5E : 0xEE;
    const int vertical_step = IsMultiplayerNet != 0 ? 3 : 6;

    RECT area = window->Buffer::clip_rect();
    window->client_to_screen(&area);
    const RECT original_area = area;

    interface_background.init(MainInterfaceGlobal.Buffer::pixel_width(),
                              MainInterfaceGlobal.Buffer::pixel_height(), 0, 0);
    interface_background.fill(WinFillColour);
    MainInterfaceGlobal.update_window_to_buffer(&interface_background);

    Sprite &right_edge = MainInterfaceGlobal.slide_frame_sprite(3);
    const int right_edge_x = area.right - right_edge.width() - 0x20;
    RECT vertical_strip = { right_edge_x, 0, area.right, 0x39 };
    MainInterfaceGlobal.client_to_screen(&vertical_strip);

    const int area_width = area.right - area.left;
    const int area_height = area.bottom - area.top;
    screen_save.init(area_width, area_height, 0, 0);
    RECT screen_save_area = area;
    ConsoleGlobal.screen_to_client(&screen_save_area);
    // `Buffer::copy` reads from its receiver and writes to its first argument.
    // Native 0x45E071..0x45E0A0 uses ConsoleGlobal's GraphicWin Buffer as
    // source and local F49C as destination.
    static_cast<Buffer *>(static_cast<GraphicWin *>(&ConsoleGlobal))->copy(
        &screen_save, screen_save_area.left, screen_save_area.top,
        0, 0, area_width, area_height);

    int edge_y = frame_edge;
    if (edge_y <= area.top) edge_y = area.top;
    int first_y = area.top + 0xD;
    if (first_y >= edge_y) first_y = edge_y;
    PopupScrollVerticalFrameCount = (edge_y - first_y) / vertical_step;
    // The native stores first_y over the transformed area.top at 0x45E0C1.
    area.top = first_y;
    if (PopupScrollVerticalFrameCount > 1) {
        Win::screen_buffer_.set_clip(&area);
        edge_y -= PopupScrollVerticalFrameCount;
        if ((GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AV_SLIDING_SCROLLBARS) != 0
            && IsMultiplayerNet == 0) {
            WinFlipClipped = 1;
            for (int frame = vertical_step; frame > 0; --frame) {
                const DWORD started = timeGetTime();
                int sprite_y = edge_y;
                if (sprite_y <= first_y) sprite_y = first_y;
                right_edge.draw(&Win::screen_buffer_, right_edge.transparent_index(),
                                right_edge_x, sprite_y, 1, 1);
                Win::screen_buffer_.set_clip(&vertical_strip);
                interface_background.draw(&Win::screen_buffer_, 9,
                    MainInterfaceGlobal.outer_rect().left,
                    MainInterfaceGlobal.outer_rect().top, 1, 1);
                Win::flip(&area);
                edge_y -= PopupScrollVerticalFrameCount;
                Win::screen_buffer_.set_clip(&area);
                do_sound();
                while (static_cast<DWORD>(timeGetTime() - started)
                       < static_cast<DWORD>(WindowAnimationFrameDelay) / 2) {
                    do_sound();
                }
            }
            Win::screen_buffer_.restore_clip();
            WinFlipClipped = 0;
        }
        RECT window_clip = window->Buffer::clip_rect();
        window->Buffer::set_clip(&window_clip);
    }

    // The captured image keeps its original height. The panel is created
    // after area.top changes, so its negative DIB height is different.
    const int frame_height = area.bottom - area.top;
    scroll_frame.init(area_width, frame_height, 0, 0);
    scroll_frame.fill(9);
    Sprite &upper_left = MainInterfaceGlobal.scroll_frame_sprite(0);
    Sprite &top_edge = MainInterfaceGlobal.scroll_frame_sprite(1);
    Sprite &lower_left = MainInterfaceGlobal.scroll_frame_sprite(2);
    Sprite &side_edge = MainInterfaceGlobal.scroll_frame_sprite(3);

    const int panel_bottom = upper_left.height()
        - ((-frame_height + lower_left.height() + 0x2C + upper_left.height())
           / top_edge.height()) * top_edge.height()
        + lower_left.height();
    scroll_frame.tile(&top_edge, 0, 0, 0, 3, top_edge.width(), panel_bottom);
    scroll_frame.tile(&side_edge, 0, panel_bottom + 3, 0, panel_bottom,
                      side_edge.width(), frame_height - panel_bottom);
    upper_left.draw(&scroll_frame, upper_left.transparent_index(), 0, 3, 1, 1);
    lower_left.draw(&scroll_frame, lower_left.transparent_index(), 0,
                    panel_bottom - lower_left.height() + 3, 1, 1);
    right_edge.draw(&scroll_frame, right_edge.transparent_index(),
                    area_width - right_edge.width(), 0, 1, 1);

    Win::screen_buffer_.set_clip(&area);
    const int horizontal_frame_count = IsMultiplayerNet != 0 ? 4 : 8;
    PopupScrollHorizontalStep = IsMultiplayerNet != 0 ? 8 : 4;
    int frame_x = area.left + PopupScrollHorizontalStep - 0x20;
    if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SLIDING_WINDOWS) != 0
        && IsMultiplayerNet == 0) {
        WinFlipClipped = 1;
        for (int frame = horizontal_frame_count; frame > 0; --frame) {
            const DWORD started = timeGetTime();
            // Restore the saved screen image at its original screen position.
            screen_save.copy(&Win::screen_buffer_, 0, 0, area.left, area.top,
                             area_width, area_height);
            scroll_frame.draw(&Win::screen_buffer_, 9, frame_x, area.top, 1, 1);
            Win::screen_buffer_.set_clip(&vertical_strip);
            interface_background.draw(&Win::screen_buffer_, 9,
                MainInterfaceGlobal.outer_rect().left,
                MainInterfaceGlobal.outer_rect().top, 1, 1);
            Win::flip(&area);
            frame_x += PopupScrollHorizontalStep;
            Win::screen_buffer_.set_clip(&area);
            do_sound();
            while (static_cast<DWORD>(timeGetTime() - started)
                   < static_cast<DWORD>(WindowAnimationFrameDelay) / 2) {
                do_sound();
            }
        }
        Win::screen_buffer_.restore_clip();
        WinFlipClipped = 0;
    }

    const int destination_y = area.top - original_area.top;
    const int destination_x = window->Buffer::pixel_width()
        - scroll_frame.pixel_width();
    scroll_frame.draw(static_cast<Buffer *>(window), 9,
                      destination_x, destination_y, 1, 1);

    scroll->resize(0x18, panel_bottom - 0xA, 0);
    scroll->move(window->Buffer::pixel_width() - 0x2B, 0x15);
    scroll->show(0);
}
// ORIGINAL: 0x0045E540 ?scroll_hide@@YAXPAUGraphicWin@@PAUScroll@@@Z 0x0045E540-0x0045EB5A;0x006557CD-0x006557F8
// RECOVERY_BINDING: v1 {"evidence": ["git:14a0bcaf:src/unrecovered/0045e540.cpp original hot/cold spans and retained seed findings.", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x45e540 full native register, rectangle mutation, receiver, call and lifetime audit.", "source:src/console.h virtual GraphicWin base; src/maininterface.h existing Sprite arrays and interface-up member; src/buffer.h source-receiver copy, full bounds and DIB geometry.", "source:src/maininterface.cpp: the helper uses MainInterfaceGlobal-owned slide/scroll sprites, interface geometry and shared transition state; its Wave configuration belongs to MainInterface::init.", "pinned:Pinned cold unwind regions remain consecutive with MainInterface dtor/ctor/init/set_message: 0x6552C0-0x65552F, 0x655530-0x655723, 0x655730-0x655745, 0x655745-0x655757, followed by these four helper spans 0x655757-0x655777, 0x655777-0x6557A2, 0x6557A2-0x6557CD, and 0x6557CD-0x6557F8; no other mapped span intersects this detached range."], "reason": "Keep this free ?scroll_hide UI transition helper with the MainInterface resource and lifetime provider while preserving its existing __cdecl signature and external window callers. This is an inferred source correspondence, not proof of an original filename or historical TU boundary."}
// symbol    ?scroll_hide@@YAXPAVGraphicWin@@PAVScroll@@@Z
// TRIED: Ghidra's decompilation was unusable here (stack-frame analysis fell apart on the SEH-heavy huge frame); rebuilt from raw disassembly only. Three real `Buffer` locals (RAII) matching construction/destruction order, register-variable/goto transcription for the two scroll-animation throttle loops (timeGetTime via IAT-slot fn ptr + do_sound), the fixed global screen Buffer at 0x9B7490, and a pointer-to-member vtable-slot call (slot 2) on `a2` at the very end. Compiles and matches through instruction #35 (diverges #36, mov vs push) - the deepest match of this batch's giant functions. The two `0x9156b0`/`0x915af4`-relative receiver computations (`*(int*)(*(int*)g_009156b0+4) + <base>`) are transcribed literally; their semantic meaning (which sub-object they reach) was not resolved.
void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll) {
    // The seed's 0x9156B0/0x915AF4 vbtable arithmetic reaches Console's real
    // GraphicWin virtual base and its Buffer base. Buffer::copy names its
    // source receiver; the final hide is the inherited virtual operation.
    Buffer window_frame;          // native F490, constructed first
    Buffer interface_background;  // native EF08, constructed second
    Buffer screen_restore;        // native FA18, constructed third

    if (scroll->Win::is_visible() == 0)
        return;

    const int frame_edge = WinScreenWidth == 0x320 ? 0x5e : 0xee;
    RECT area = window->Buffer::clip_rect();
    window->client_to_screen(&area);
    const int screen_left = area.left;
    const int screen_top = area.top;
    const int area_height = area.bottom - area.top;

    interface_background.init(MainInterfaceGlobal.Buffer::pixel_width(),
                              MainInterfaceGlobal.Buffer::pixel_height(), 0, 0);
    interface_background.fill(WinFillColour);
    MainInterfaceGlobal.update_window_to_buffer(&interface_background);

    Sprite &right_edge = MainInterfaceGlobal.slide_frame_sprite(3);
    const int right_edge_x = area.right - right_edge.width() - 0x20;
    area.left = right_edge_x;
    const int area_width = area.right - area.left;

    RECT vertical_strip = { right_edge_x, 0, area.right, 0x39 };
    MainInterfaceGlobal.client_to_screen(&vertical_strip);

    screen_restore.init(area_width, area_height, 0, 0);
    RECT console_area = area;
    ConsoleGlobal.screen_to_client(&console_area);
    static_cast<Buffer *>(static_cast<GraphicWin *>(&ConsoleGlobal))->copy(
        &screen_restore, console_area.left, console_area.top, 0, 0,
        area_width, area_height);

    window_frame.init(area_width, area_height, 0, 0);
    static_cast<Buffer *>(window)->copy(
        &window_frame, area.left - screen_left, area.top - screen_top,
        0, 0, area_width, area_height);

    const int horizontal_step = IsMultiplayerNet != 0 ? 8 : 4;
    PopupScrollHorizontalStep = horizontal_step;
    int frame_x = area.left - horizontal_step;
    const int horizontal_frames = IsMultiplayerNet != 0 ? 4 : 8;
    Win::screen_buffer_.set_clip(&area);
    if ((GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AV_SLIDING_SCROLLBARS) != 0
        && IsMultiplayerNet == 0) {
        WinFlipClipped = 1;
        for (int frame = 0; frame < horizontal_frames; ++frame) {
            const DWORD started = timeGetTime();
            screen_restore.copy(&Win::screen_buffer_, 0, 0, area.left, screen_top,
                                area_width, area_height);
            window_frame.draw(&Win::screen_buffer_, 9, frame_x, screen_top, 1, 1);
            Win::screen_buffer_.set_clip(&vertical_strip);
            interface_background.draw(&Win::screen_buffer_, 9,
                MainInterfaceGlobal.outer_rect().left,
                MainInterfaceGlobal.outer_rect().top, 1, 1);
            Win::flip(&area);
            frame_x -= PopupScrollHorizontalStep;
            Win::screen_buffer_.set_clip(&area);
            do_sound();
            while (static_cast<DWORD>(timeGetTime() - started)
                   < static_cast<DWORD>(WindowAnimationFrameDelay) / 2)
                do_sound();
        }
        Win::screen_buffer_.restore_clip();
        WinFlipClipped = 0;
        frame_x += PopupScrollHorizontalStep;
    } else {
        frame_x += horizontal_step;
    }

    const int vertical_frame_step = IsMultiplayerNet != 0 ? 3 : 6;
    const int vertical_iterations = vertical_frame_step + 1;
    int closing_y = screen_top + PopupScrollVerticalFrameCount;
    const bool enter_vertical_close = PopupScrollVerticalFrameCount > 1;
    if (enter_vertical_close) {
        Win::screen_buffer_.set_clip(&area);
        int edge_y = frame_edge - 0xD;
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SLIDING_WINDOWS) != 0
            && IsMultiplayerNet == 0) {
            WinFlipClipped = 1;
            for (int frame = 0; frame < vertical_iterations; ++frame) {
                const DWORD started = timeGetTime();
                screen_restore.copy(&Win::screen_buffer_, 0, 0, area.left, screen_top,
                                    area_width, area_height);
                int draw_y = closing_y;
                if (draw_y >= edge_y) draw_y = edge_y;
                window_frame.draw(&Win::screen_buffer_, 9, frame_x, draw_y, 1, 1);
                Win::screen_buffer_.set_clip(&vertical_strip);
                interface_background.draw(&Win::screen_buffer_, 9,
                    MainInterfaceGlobal.outer_rect().left,
                    MainInterfaceGlobal.outer_rect().top, 1, 1);
                Win::flip(&area);
                closing_y += PopupScrollVerticalFrameCount;
                Win::screen_buffer_.set_clip(&area);
                do_sound();
                while (static_cast<DWORD>(timeGetTime() - started)
                       < static_cast<DWORD>(WindowAnimationFrameDelay) / 2)
                    do_sound();
            }
            Win::screen_buffer_.restore_clip();
            WinFlipClipped = 0;
        }
    }

    RECT original_buffer_clip = window->Buffer::clip_rect();
    if (enter_vertical_close)
        window->Buffer::set_clip(&original_buffer_clip);

    RECT local_area = area;
    local_area.left -= screen_left;
    local_area.right -= screen_left;
    local_area.top -= screen_top;
    local_area.bottom -= screen_top;
    window->Buffer::set_clip(&local_area);
    window->fill(9);
    int final_y_screen = closing_y;
    if (final_y_screen < local_area.top) final_y_screen = local_area.top;
    const int final_y = final_y_screen - screen_top;
    window_frame.draw(static_cast<Buffer *>(window), 9,
                      frame_x - screen_left, final_y, 1, 1);
    window->Buffer::set_clip(&original_buffer_clip);
    scroll->hide();
}
