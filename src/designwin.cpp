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
// ORIGINAL: 0x0043EDB0 ??_GDesignWin@@UAEPAXI@Z 0x0043EDB0-0x0043EDD1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x0043EDB0-0x0043EDD1", "Pinned class constructor ??0DesignWin@@QAE@XZ at 0x00434480 stores original class/view table VA 0x66b148 at 0x0043454C (mov dword ptr [esi], 0x66b148); the current genuine owner vftable ??_7DesignWin@@6BWin@@@ selects generated ??_GDesignWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EDesignWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GDesignWin@@UAEPAXI@Z at 0x0043EDB0 calls the source-mapped ??1DesignWin@@QAE@XZ at 0x0043ECB0, then tests byte ptr [ebp + 8], 1; je 0x43edca bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/designwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original DesignWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GDesignWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x0043EFA0 ??_EDesignWin@@WEEE@AEPAXI@Z 0x0043EFA0-0x0043EFAB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor at 0x00434480 installs Buffer table 0x0066B140 at this+0x444; its first slot points to 0x0043EFA0.", "Pinned entry 0x0043EFA0 has exactly eleven bytes: SUB ECX,0x444 then JMP to already mapped primary deleting destructor 0x0043EDB0, followed by INT3 padding.", "Owner designwin.cpp physical COFF emits eleven-byte COMDAT ??_EDesignWin@@WEEE@AEPAXI@Z; Buffer table ??_7DesignWin@@6BBuffer@@@ selects it, and its REL32 targets weak ??_EDesignWin@@UAEPAXI@Z with scalar ??_GDesignWin@@UAEPAXI@Z fallback.", "The genuine GraphicWin:Win,Buffer hierarchy provides the receiver adjustment. Independent original constructor/table/body and physical COFF review supports this binding; local equality does not prove full linked table selection or runtime acceptance."], "reason": "Bind the original Buffer-view deleting receiver adjustment to the genuine DesignWin compiler-emitted operation. The symbol names this tree emits, not an original debug name; no adapter body is authored."}
// ORIGINAL: 0x00432880 _$E21 0x00432880-0x00432896 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x0068207C selects init 0x00432880, which registers exit callback 0x004328A0.", "Original init span 0x00432880-0x00432896 and exit span 0x004328A0-0x004328AA are independently disassembled; both identify receiver 0x0071F240 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Current designwin.cpp physical COFF selects local STATIC init _$E21 via .CRT$XCU, relocating to g_DESIGNWIN_WAVE, Wave construction, paired local fini _$E19 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_DESIGNWIN_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E26
// ORIGINAL: 0x004328A0 _$E19 0x004328A0-0x004328AA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x0068207C selects init 0x00432880, which registers exit callback 0x004328A0.", "Original init span 0x00432880-0x00432896 and exit span 0x004328A0-0x004328AA are independently disassembled; both identify receiver 0x0071F240 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Current designwin.cpp physical COFF selects local STATIC init _$E21 via .CRT$XCU, relocating to g_DESIGNWIN_WAVE, Wave construction, paired local fini _$E19 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_DESIGNWIN_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E24
// ORIGINAL: 0x00432850 _$E21 0x00432850-0x00432866 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682078 selects initializer0x432850..0x432866, registering exit0x432870..0x43287a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x71f2b0 bound to?TutWinDesWindow@@3VDesignWin@@A.", "Original startup instruction evidence: [[\"0x432850\", \"mov\", \"ecx, 0x71f2b0\"], [\"0x432855\", \"call\", \"0x434480\"], [\"0x43285a\", \"push\", \"0x432870\"], [\"0x43285f\", \"call\", \"0x645398\"], [\"0x432864\", \"pop\", \"ecx\"], [\"0x432865\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x432870\", \"mov\", \"ecx, 0x71f2b0\"], [\"0x432875\", \"jmp\", \"0x43ecb0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/designwin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?TutWinDesWindow@@3VDesignWin@@A\", \"??0DesignWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?TutWinDesWindow@@3VDesignWin@@A\", \"??1DesignWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2dc; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?TutWinDesWindow@@3VDesignWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// ORIGINAL: 0x00432870 _$E19 0x00432870-0x0043287A FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682078 selects initializer0x432850..0x432866, registering exit0x432870..0x43287a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x71f2b0 bound to?TutWinDesWindow@@3VDesignWin@@A.", "Original startup instruction evidence: [[\"0x432850\", \"mov\", \"ecx, 0x71f2b0\"], [\"0x432855\", \"call\", \"0x434480\"], [\"0x43285a\", \"push\", \"0x432870\"], [\"0x43285f\", \"call\", \"0x645398\"], [\"0x432864\", \"pop\", \"ecx\"], [\"0x432865\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x432870\", \"mov\", \"ecx, 0x71f2b0\"], [\"0x432875\", \"jmp\", \"0x43ecb0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/designwin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?TutWinDesWindow@@3VDesignWin@@A\", \"??0DesignWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?TutWinDesWindow@@3VDesignWin@@A\", \"??1DesignWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2dc; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?TutWinDesWindow@@3VDesignWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
#include "stdafx.h"
#include "interface_fonts.h"
#include "map.h"
#include "wave.h"
#include "designwin.h"
#include "win.h"
#include "worldwin.h"
#include "faction.h"
#include "game.h"
#include "interface_palette.h"
#include "mapwin.h"     // the virtual GraphicWin host for the design tutorial
#include "popup.h"
#include "basepop.h"
#include "base.h"       // cost_factor and upgrade_prototype
#include "fx.h"         // workshop selection effect
#include "netmsg.h"     // existing network-message dependency
#include "strings.h"
#include "text.h"       // the design preset labels are read from its script
#include "technology.h"

#include "general.h"
#include "veh.h"
#include "selectpartwin.h"
#include "tutwin.h"     // TutWinGlobal - the tutorial popup every click dismisses
#include "alpha.h"      // Labels - the label-id block on_left_click reads
#include "netdaemon.h"  // synch_proto, message_data, NetDaemonNet
#include "xpops.h"      // x_pop_caption_label
#include "console.h"    // ConsoleGlobal.faction_id_ is the local faction

void __cdecl clear_bunglist(int faction);
int __cdecl diplo_lock(int patience);
void __cdecl diplo_unlock();
extern uint32_t UpgradePromptAnswered; // base.cpp owns original90EA3C


// DISCOVERY 0x00439D30: the retained seed's 0x009AB550 is
// Ability[].preq_tech, 0x009A6490 is GamePreferences, 0x00949780 is
// Rules.tech_two_spec_abilities and 0x00939284 is ConsoleGlobal.faction_id_.
// Its faction-stride scan reaches Players[].faction_bonus_id/val1. The
// original scans 41 dwords from each member start, despite the eight declared
// bonus slots. The source-owned byte view below preserves those reads; its
// windows extend past each Player by 0x18 and 0x38 bytes respectively, and the final faction's
// reads exceed the complete Players[8] allocation. This remains an original
// out-of-bounds/runtime obligation, not made defined by the typed owner.

// ---- shared seams and bindings the recovered bodies reach -----------------
void __cdecl proto_sort_2(int faction);
extern int ProtoSortCount;
extern int ProtoSortIds[128];
void __cdecl name_proto(char *name, int prototype, int faction, int chassis,
                       int weapon, int armor, int abilities, int reactor);
int __cdecl X_text_open(char *src_file_id, const char *section_id);
extern int DesignGridButtonIndex;

// Distinct mutable loader-zero name inputs at 0x00733978 and 0x0073397C.
// Their containing extents and indirect writers remain unresolved.
// RECOVERY_OBJECT: v1 0x00733978 ?DesignButtonName23@@3PADA
char DesignButtonName23[1];
// RECOVERY_OBJECT: v1 0x0073397C ?DesignButtonName24@@3PADA
char DesignButtonName24[1];

// The automatic-design editor exchanges these fields with consider_designs
// (0x00581260). setup_veh writes them, enables the override, then reloads
// the resulting component choices. Their shared consumer remains unfinished.
// RECOVERY_OBJECT: v2 0x0093FC34 0x0093FC38 ?DesignConsiderAbilities@@3IA 0x0043E490
uint32_t DesignConsiderAbilities;
// RECOVERY_OBJECT: v2 0x0093FC38 0x0093FC3C ?DesignConsiderPrototype@@3HA 0x0043E490
int DesignConsiderPrototype;
// RECOVERY_OBJECT: v2 0x0093FC3C 0x0093FC40 ?DesignConsiderWeapon@@3HA 0x0043E490
int DesignConsiderWeapon;
// RECOVERY_OBJECT: v2 0x0093FC40 0x0093FC44 ?DesignConsiderChassis@@3HA 0x0043E490
int DesignConsiderChassis;
// propose_proto exchanges its sixth (reactor) argument with the editor at
// 0x580F07; both original writes use this independent four-byte scalar.
// RECOVERY_OBJECT: v2 0x0093FC44 0x0093FC48 ?DesignConsiderReactor@@3HA 0x00580860
int DesignConsiderReactor;
// Workshop583FA2 publishes its faction here; preview583DB5/583DC1 reloads
// it around veh_fake. This is distinct from the preceding reactor scalar.
// RECOVERY_OBJECT: v2 0x0093FC48 0x0093FC4C ?WorkshopFaction@@3HA 0x00583E30
int WorkshopFaction;
// Preview583CD7 and workshop583F9A read this signed zoom, initially -6.
// Its four-byte initializer is independently backed by the pinned image;
// the preceding FirstProposedPrototype ends at this address, exclusively.
// RECOVERY_OBJECT: v2 0x0068F0F4 0x0068F0F8 ?WorkshopPreviewZoom@@3HA 0x00583CD0
int WorkshopPreviewZoom = -6;
// RECOVERY_OBJECT: v2 0x0093FC50 0x0093FC54 ?DesignConsiderArmor@@3HA 0x0043E490
int DesignConsiderArmor;
// RECOVERY_OBJECT: v2 0x0093FC54 0x0093FC58 ?DesignConsiderOverride@@3HA 0x0043E490
int DesignConsiderOverride;

// Three separate, four-byte zero-initialized string stores are passed to
// strcpy on the original setup paths, at E679, E6A9 and E829 respectively.
// RECOVERY_OBJECT: v2 0x00733980 0x00733984 ?DesignDefaultName@@3PADA 0x0043E490
char DesignDefaultName[4];
// RECOVERY_OBJECT: v2 0x00733984 0x00733988 ?DesignPredefinedName@@3PADA 0x0043E490
char DesignPredefinedName[4];
// RECOVERY_OBJECT: v2 0x00733988 0x0073398C ?DesignAutomaticName@@3PADA 0x0043E490
char DesignAutomaticName[4];

// ORIGINAL: 0x00434480 ??0DesignWin@@QAE@XZ 0x00434480-0x004345B5;0x00653F6F-0x00653FFE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/units/00434480.cpp"], "reason": "Natural bases and embedded members construct once; original initializes editor component state after their construction."}
DesignWin::DesignWin() {
    factionID_ = 0;
    protoID_ = 0;
    protoChassisType_ = 0;
    protoWeaponType_ = 0;
    protoArmorType_ = 0;
    protoAbilityFlags_ = 0;
    protoReactorType_ = 1;
    field_14210_ = 0;
    mineralCostFactor_ = 0;
    prototype_name_[0] = 0;
}

// ORIGINAL: 0x0043C270 ?init@DesignWin@@QAEXXZ 0x0043C270-0x0043D0B7;0x006546C5-0x006546DA FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/0043c270.cpp", "image:0x0043c270; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// TRIED: exact stat-button rect/offset arithmetic for the last four BaseButtons (0xbac/0xbbc/0xb9c region) was approximated with a single receiver address, not derived per-button from the disassembly; the field-init prologue and font/spot/caviar setup above it were transcribed from confirmed call sites.
void DesignWin::init() {
    // The initializer is also the reopen path. Close each prior child/resource
    // before rebuilding it, then keep the original stack Buffer lifetime even
    // though the loaded member buffer supplies the actual copy below.
    Buffer stack_buffer;
    (void)stack_buffer;
    close();
    field_CE4_ = 0;
    field_4_ = 1;
    register_iface_mode(1, static_cast<SubInterface *>(this));
    font1_.init(DefaultFontFace, 10, 0);
    font1_.unk_1_ = 3;
    font2_.init(DefaultFontFace, 12, 1);
    font2_.unk_1_ = 5;
    font3_.init(DefaultFontFace, 16, 1);
    font3_.unk_1_ = 4;
    field_B0C_rect_ = MainInterfaceGlobal.status_hit_rect();
    field_B0C_rect_.left = MainInterfaceGlobal.main_frame_rect().left + 4;
    RECT stat_panel = MainInterfaceGlobal.main_frame_rect();
    preview_panel_rect_.left = stat_panel.left;
    preview_panel_rect_.top = stat_panel.top;
    preview_panel_rect_.right = stat_panel.right;
    preview_panel_rect_.bottom = stat_panel.bottom;
    preview_panel_rect_.bottom = field_B0C_rect_.top;
    preview_panel_rect_.left += 3;
    preview_panel_rect_.top += 3;
    preview_panel_rect_.right -= 3;
    preview_panel_rect_.bottom -= 3;
    info_panel_rect_ = MainInterfaceGlobal.lower_frame_rect();
    info_panel_rect_.left += 4;
    info_panel_rect_.top += 4;
    info_panel_rect_.right -= 4;
    info_panel_rect_.bottom -= 4;
    frame_rect_ = MainInterfaceGlobal.right_frame_rect();
    frame_rect_.left += 4;
    frame_rect_.top += 4;
    frame_rect_.right -= 4;
    frame_rect_.bottom -= 4;
    caption_rect_ = frame_rect_;
    caption_rect_.left += 3;
    caption_rect_.top += 3;
    caption_rect_.right -= 3;
    caption_rect_.bottom = caption_rect_.top + 21;
    field_A7C_ = caption_rect_.left;
    field_A80_ = caption_rect_.top;
    field_A84_ = caption_rect_.right;
    field_A88_ = caption_rect_.bottom;
    field_A80_ = caption_rect_.bottom + 3;
    int i;
    int row;
    int column;

    SetRect(&vehicle_panel_rect_, 0x8C, 9, 0x1C3, 0x140);
    SetRect(&chassis_panel_rect_, 0x1E, 7, 0x81, 0x5C);
    SetRect(&weapon_panel_rect_, 0x1E, 0x65, 0x81, 0xBA);
    SetRect(&armor_panel_rect_, 0x1E, 0xC3, 0x81, 0x118);
    SetRect(&reactor_panel_rect_, 0x1CE, 7, 0x231, 0x5C);
    SetRect(&special_one_panel_rect_, 0x1CE, 0x65, 0x231, 0xBA);
    SetRect(&special_two_panel_rect_, 0x1CE, 0xC3, 0x231, 0x118);

    spot1_.init(0x40);
    spot2_.init(6);
    RECT *component_rects[6] = {
        &chassis_panel_rect_, &weapon_panel_rect_, &armor_panel_rect_,
        &reactor_panel_rect_, &special_one_panel_rect_, &special_two_panel_rect_
    };
    for (i = 0; i < 6; ++i) {
        spot2_.add(i, i, component_rects[i]->left, component_rects[i]->top,
                   component_rects[i]->right - component_rects[i]->left,
                   component_rects[i]->bottom - component_rects[i]->top);
    }

    // The vehicle preview uses the same light direction as the game renderer.
    caviar_.init();
    caviar_.set_field_A4(0);
    const float angles[3] = {0.0f, 0.0f, 0.785f};
    VOX_Matrix rotation;
    caviar_apply_rotation(angles, &rotation);
    VOX_Vect camera;
    for (i = 0; i < 3; ++i) {
        rotation.values[2][i] = -rotation.values[2][i];
        camera.values[i] = rotation.values[2][i] * -10000.0f;
    }
    caviar_.set_camera_direct(&camera, &rotation);
    caviar_.set_scaling(0.18f);
    caviar_.shadow_vector() = VehLightDirMap;
    caviar_.light_vector() = VehLightDirMap;

    const int frame_height = 0x258 - MainInterfaceGlobal.field_D7C_;
    buffer_.load_pcx("deswin.pcx", 0, 0x0A, 0xEC);
    Win *parent = static_cast<Win *>(static_cast<GraphicWin *>(&ConsoleGlobal));
    GraphicWin::init(0, 0, buffer_.pixel_width(), frame_height, 0,
                     0x01000000, parent, 0, 0);

    const int move_x = (WinScreenWidth - pixel_width() + 0x20) / 2;
    const int move_y = -pixel_height() - MainInterfaceGlobal.field_D7C_
        + WinScreenHeight;
    move(move_x, move_y);

    Buffer *const inherited_buffer = static_cast<Buffer *>(
        static_cast<GraphicWin *>(this));
    buffer_.copy(inherited_buffer, 0, 0, inherited_buffer->pixel_width(),
                 inherited_buffer->pixel_height());

    // Lay out the two-row prototype grid. The original counter is shared
    // storage, but its observable sequence is a simple row-major 0..13.
    X_text_open(ScriptPopupFile, "DESIGNAUTO");
    field_A7C_ = caption_rect_.left;
    field_A80_ = caption_rect_.top;
    field_A84_ = caption_rect_.right;
    field_A88_ = caption_rect_.bottom;
    field_A80_ = caption_rect_.bottom + 3;
    const int grid_width = (static_cast<int>(field_A84_) -
                            static_cast<int>(field_A7C_) + 3) / 2 - 3;
    DesignGridButtonIndex = 0;
    int column_x = static_cast<int>(field_A7C_);
    for (row = 0; row < 2; ++row) {
        int y = static_cast<int>(field_A80_);
        for (column = 0; column < 7; ++column) {
            text_get();
            CharUpperA(TextBufferGetPtr);
            const int button = 9 + DesignGridButtonIndex;
            flatButtons_[button].init(TextBufferGetPtr, button, column_x, y,
                grid_width, 0x15, static_cast<Win *>(&MainInterfaceGlobal), 0);
            flatButtons_[button].set_bubble_text(label_get(0xCEC / 4));
            ++DesignGridButtonIndex;
            y += 0x17;
        }
        column_x += 3 + grid_width;
    }
    text_close();

    // These paired-button rectangles are copied after the grid is populated.
    const RECT &stack_rect = MainInterfaceGlobal.stack_button_rect_;
    const int frame_shift = MainInterfaceGlobal.main_frame_rect().left
        - stack_rect.left + 8;
    field_CBC_rect_.left = stack_rect.left + frame_shift;
    field_CBC_rect_.top = stack_rect.top;
    field_CBC_rect_.right = stack_rect.right + frame_shift;
    field_CBC_rect_.bottom = stack_rect.bottom;
    const RECT &adjacent_rect = MainInterfaceGlobal.adjacent_panel_right_rect_;
    field_CCC_ = adjacent_rect.left;
    field_CD0_ = adjacent_rect.top;
    field_CD4_ = adjacent_rect.right;
    field_CD8_ = adjacent_rect.bottom;

    // Five lower-panel rows retain the original unusual coordinate ordering;
    // drawing and button setup consume the exact same four stored integers.
    RECT *const rows[5] = {&lower_panel_rect_, &lower_panel_extra_rect_,
        &field_BAC_rect_, &field_BBC_rect_, &lower_panel_end_rect_};
    int left = buffer_working_rect()->left + 0x1B;
    const int right_limit = buffer_working_rect()->right - 0x3B;
    const int bottom = buffer_working_rect()->bottom - 0x14;
    const int top = bottom - 0x14;
    int edge = (right_limit - left + 3) / 5 - 3 + left;
    for (row = 0; row < 5; ++row) {
        rows[row]->left = left;
        rows[row]->top = top;
        rows[row]->right = edge;
        rows[row]->bottom = bottom;
        const int step = edge - left + 3;
        left += step;
        edge += step;
    }
    const int last_step = edge - left + 3;
    field_B5C_rect_ = lower_panel_rect_;
    const int vertical_delta = top - bottom - 6;
    field_B5C_rect_.top += vertical_delta;
    field_B5C_rect_.bottom += vertical_delta;
    field_B6C_rect_ = field_B5C_rect_;
    field_B6C_rect_.left += last_step;
    field_B6C_rect_.right += last_step;
    field_B4C_rect_ = lower_panel_end_rect_;
    field_B4C_rect_.top += vertical_delta;
    field_B4C_rect_.bottom += vertical_delta;
    field_B7C_rect_ = field_B4C_rect_;
    const int horizontal_delta = left - edge - 3;
    field_B7C_rect_.left += horizontal_delta;
    field_B7C_rect_.right += horizontal_delta;

    // Localized labels are loaded in the original order, and each button keeps
    // its own id, parent, hit rectangle, and bubble label.
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xE58 / 4));
    CharUpperA(StringTemp);
    flatButtons_[1].init(StringTemp, 1, field_B5C_rect_.left,
        field_B5C_rect_.top, field_B5C_rect_.right - field_B5C_rect_.left,
        field_B5C_rect_.bottom - field_B5C_rect_.top, static_cast<Win *>(this), 0);
    flatButtons_[1].set_bubble_text(label_get(0xE7C / 4));

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x1024 / 4));
    CharUpperA(StringTemp);
    flatButtons_[2].init(StringTemp, 2, field_B6C_rect_.left,
        field_B6C_rect_.top, field_B6C_rect_.right - field_B6C_rect_.left,
        field_B6C_rect_.bottom - field_B6C_rect_.top, static_cast<Win *>(this), 0);
    flatButtons_[2].set_bubble_text(label_get(0x1028 / 4));

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xE74 / 4));
    CharUpperA(StringTemp);
    flatButtons_[0].init(StringTemp, 0, field_B4C_rect_.left,
        field_B4C_rect_.top, field_B4C_rect_.right - field_B4C_rect_.left,
        field_B4C_rect_.bottom - field_B4C_rect_.top, static_cast<Win *>(this), 0);
    flatButtons_[0].set_bubble_text(label_get(0xE78 / 4));

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x1034 / 4));
    CharUpperA(StringTemp);
    flatButtons_[3].init(StringTemp, 3, field_B7C_rect_.left,
        field_B7C_rect_.top, field_B7C_rect_.right - field_B7C_rect_.left,
        field_B7C_rect_.bottom - field_B7C_rect_.top, static_cast<Win *>(this), 0);
    flatButtons_[3].set_bubble_text(label_get(0x1038 / 4));

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xBD0 / 4));
    CharUpperA(StringTemp);
    flatButtons_[6].init(StringTemp, 6, field_BAC_rect_.left,
        field_BAC_rect_.top, field_BAC_rect_.right - field_BAC_rect_.left,
        field_BAC_rect_.bottom - field_BAC_rect_.top, static_cast<Win *>(this), 0);
    flatButtons_[6].set_bubble_text(label_get(0xC58 / 4));

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xBD4 / 4));
    CharUpperA(StringTemp);
    flatButtons_[7].init(StringTemp, 7, field_BBC_rect_.left,
        field_BBC_rect_.top, field_BBC_rect_.right - field_BBC_rect_.left,
        field_BBC_rect_.bottom - field_BBC_rect_.top, static_cast<Win *>(this), 0);
    flatButtons_[7].set_bubble_text(label_get(0xC5C / 4));

    const int prototype_id = static_cast<int>(protoID_);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get((VehPrototypes[prototype_id].flags & 0x80)
                                 ? 0x864 / 4 : 0x198 / 4));
    if (Language != 2) CharUpperA(StringTemp);
    flatButtons_[8].init(StringTemp, 8, lower_panel_end_rect_.left,
        lower_panel_end_rect_.top,
        lower_panel_end_rect_.right - lower_panel_end_rect_.left,
        lower_panel_end_rect_.bottom - lower_panel_end_rect_.top,
        static_cast<Win *>(this), 0);
    const int faction_bubble =
        (VehPrototypes[prototype_id].flags & 0x80) != 0
            ? 0xC64 : 0xC60;
    flatButtons_[8].set_bubble_text(label_get(faction_bubble / 4));

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x438 / 4));
    CharUpperA(StringTemp);
    flatButtons_[4].init(StringTemp, 4, lower_panel_rect_.left,
        lower_panel_rect_.top, lower_panel_rect_.right - lower_panel_rect_.left,
        lower_panel_rect_.bottom - lower_panel_rect_.top, static_cast<Win *>(this), 0);
    flatButtons_[4].set_bubble_text(label_get(0xC50 / 4));
    flatButtons_[5].init(StringTemp, 5, lower_panel_extra_rect_.left,
        lower_panel_extra_rect_.top,
        lower_panel_extra_rect_.right - lower_panel_extra_rect_.left,
        lower_panel_extra_rect_.bottom - lower_panel_extra_rect_.top,
        static_cast<Win *>(this), 0);
    flatButtons_[5].set_bubble_text(label_get(0xC54 / 4));

    // Two lower navigation controls use the original distinct data groups and
    // the process-wide MainInterface as their parent.
    flatButtons_[23].init(DesignButtonName23, 0x17, field_CBC_rect_.left,
        field_CBC_rect_.top, field_CBC_rect_.right - field_CBC_rect_.left,
        field_CBC_rect_.bottom - field_CBC_rect_.top,
        static_cast<Win *>(&MainInterfaceGlobal), 0);
    flatButtons_[24].init(DesignButtonName24, 0x18, field_CCC_, field_CD0_,
        field_CD4_ - field_CCC_, field_CD8_ - field_CD0_,
        static_cast<Win *>(&MainInterfaceGlobal), 0);
    for (i = 0; i < 25; ++i) {
        flatButtons_[i].hide();
        flatButtons_[i].set_text_color(0x41, -1, 1, 1);
        flatButtons_[i].set_border_styles(&InterfaceBoxStyles[27],
            &InterfaceBoxStyles[29], &InterfaceBoxStyles[29]);
        static_cast<Buffer *>(static_cast<GraphicWin *>(&flatButtons_[i]))
            ->set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
        flatButtons_[i].redraw();
    }
    for (i = 0; i < 3; ++i)
        flatButtons_[23].field_AD4_[i] = &MainInterfaceGlobal.sprites12_[i];
    flatButtons_[23].redraw();
    flatButtons_[23].set_field_A44(100);
    for (i = 0; i < 3; ++i)
        flatButtons_[24].field_AD4_[i] = &MainInterfaceGlobal.sprites12_[i + 3];
    flatButtons_[24].redraw();
    flatButtons_[24].set_field_A44(100);
}

// ORIGINAL: 0x00439700 ?on_right_click@DesignWin@@QAEXHH@Z 0x00439700-0x00439948
// RECOVERY_BINDING: v1 {"evidence": ["git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/units/00439700.cpp"], "reason": "Spot palette categories select localized help labels; the two ability categories share a label."}
// symbol    ?on_right_click@DesignWin@@UAEXHH@Z
void DesignWin::on_right_click(int x, int y) {
    TutorialWindow.reset();
    int position;
    int category;
    if (spot2_.check(x, y, &position, &category) == -1 ||
        static_cast<unsigned int>(category) > 5) return;
    // Both special-ability spots (4 and 5) use the same help label.
    int label = 0xCD4 / 4 + (category == 5 ? 4 : category);
    char *text = StringTable.get(
        Labels.strings_ptr[label]);
    client_to_screen(&x, &y);
    RECT rect = {x, y, x, y};
    set_bubble_text(text, &rect);
}

// ORIGINAL: 0x004379A0 ?on_redraw@DesignWin@@QAEXXZ 0x004379A0-0x00437B95
// RECOVERY_BINDING: v1 {"evidence": ["git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/units/004379a0.cpp;InterfaceBoxStyles source ownership in src/game.cpp;MainInterface primary_font and working rectangle"], "reason": "Restore the design frame and caption through real screen and embedded Buffer objects, redraw all editor panels, and frame the union of the lower panels."}
// symbol    ?on_redraw@DesignWin@@UAEXXZ
void DesignWin::on_redraw() {
    Buffer *screen = static_cast<Buffer *>(&MainInterfaceGlobal);
    screen->set_clip(MainInterfaceGlobal.buffer_working_rect());
    Buffer::set_clip(buffer_working_rect());
    MainInterfaceGlobal.restore_back(1);
    screen->box_sprite(&frame_rect_, &InterfaceBoxStyles[3]);
    RECT caption = caption_rect_;
    screen->box_sprite(&caption, &InterfaceBoxStyles[6]);
    caption.left += 3;
    caption.top += 3;
    caption.right -= 3;
    caption.bottom -= 3;
    StringTemp[0] = 0;
    screen->set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
    strcat(StringTemp, StringTable.get(
        Labels.strings_ptr[0xA70 / 4]));
    screen->set_text_color(0xF0, -1, 1, 1);
    CharUpperA(StringTemp);
    screen->write_cent_l(StringTemp, &caption, strlen(StringTemp));
    check_abil();
    draw_chassis(0);
    draw_weapon(0);
    draw_armor(0);
    draw_reactor(0);
    draw_special_1(0);
    draw_special_2(0);
    draw_vehicle(0);
    draw_info();
    draw_stack();
    draw_flash();
    Buffer::set_clip(buffer_working_rect());
    UnionRect(&caption, &lower_panel_end_rect_, &lower_panel_rect_);
    UnionRect(&caption, &caption, &lower_panel_extra_rect_);
    caption.left -= 3;
    caption.top -= 3;
    caption.right += 3;
    caption.bottom += 3;
    Buffer::box_sprite(&caption, &InterfaceBoxStyles[3]);
}

// ORIGINAL: 0x0043E490 ?setup_veh@DesignWin@@QAEXHH@Z 0x0043E490-0x0043E952
// RECOVERY_BINDING: v1 {"evidence": ["image:0x43E496,0x43E6D1;git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/units/0043e490.cpp"], "reason": "Original first argument is the prototype and second enables automatic component selection; real prototype records, component tables and shared design state preserve each branch."}
void DesignWin::setup_veh(int proto_id, int automatic) {
    // Original E496 reads the FIRST argument as the prototype index; the
    // retained seed reversed the formals and also used byte-array flags
    // with half the real VehPrototype stride. The image uses 0x34 throughout.
    if (proto_id < 0 || !(VehPrototypes[proto_id].flags & 1) ||
        (proto_id < 64 &&
         (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating < 0 ||
          proto_id == 15))) {
        proto_sort_2(factionID_);
        int row = 0;
        while (row < ProtoSortCount &&
               (VehPrototypes[ProtoSortIds[row]].flags & 1)) ++row;
        protoID_ = row == ProtoSortCount ? factionID_ << 6 : ProtoSortIds[row];
    } else {
        protoID_ = proto_id;
    }
    protoReactorType_ = best_reactor(factionID_);
    VehPrototype &prototype = VehPrototypes[protoID_];
    if (prototype.flags & 1) {
        prototype.flags &= ~0x80;
        protoChassisType_ = prototype.chassis_id;
        protoWeaponType_ = prototype.weapon_id;
        protoArmorType_ = prototype.armor_id;
        protoAbilityFlags_ = prototype.ability_flags;
        protoReactorType_ = prototype.reactor_id;
        protoAbilityID1_ = protoAbilityID2_ = static_cast<uint32_t>(-1);
        for (int ability = 0; ability < 29; ++ability) {
            if (protoAbilityFlags_ & (1u << ability)) {
                if (static_cast<int>(protoAbilityID1_) < 0) protoAbilityID1_ = ability;
                else if (static_cast<int>(protoAbilityID2_) < 0) protoAbilityID2_ = ability;
            }
        }
        if ((prototype.flags & 2) && static_cast<int>(protoID_) >= 64) {
            strcpy(prototype_name_, prototype.veh_name);
            char generated_name[80];
            name_proto(generated_name, protoID_, factionID_, protoChassisType_,
                       protoWeaponType_, protoArmorType_, protoAbilityFlags_,
                       protoReactorType_);
            if (strcmp(prototype_name_, generated_name) == 0) {
                strcpy(prototype_name_, DesignDefaultName);
                prototype.flags &= ~2;
            }
        } else {
            prototype.flags &= ~2;
            strcpy(prototype_name_, DesignPredefinedName);
        }
        field_14210_ = 0;
        return;
    }

    protoArmorType_ = protoWeaponType_ = 0;
    if (automatic == 0) {
        protoChassisType_ = protoAbilityFlags_ = 0;
        protoReactorType_ = 1;
        protoAbilityID1_ = protoAbilityID2_ = static_cast<uint32_t>(-1);
        field_14210_ = 0;
        return;
    }
    protoAbilityFlags_ = 0;
    int best = 0;
    int rating = 0;
    for (protoChassisType_ = 0; protoChassisType_ < 9; ++protoChassisType_) {
        const RulesChassis &chassis = Chassis[protoChassisType_];
        if (has_tech(chassis.preq_tech, factionID_) && chassis.triad == 0 &&
            chassis.speed >= rating) {
            best = protoChassisType_;
            rating = chassis.speed;
        }
    }
    protoChassisType_ = best;
    best = rating = 0;
    for (protoWeaponType_ = 0; protoWeaponType_ < 26; ++protoWeaponType_) {
        const RulesWeapon &weapon = Weapon[protoWeaponType_];
        if (has_tech(weapon.preq_tech, factionID_) && weapon.offense_rating < 99 &&
            weapon.offense_rating >= rating) {
            best = protoWeaponType_;
            rating = weapon.offense_rating;
        }
    }
    protoWeaponType_ = best;
    best = rating = 0;
    for (protoArmorType_ = 0; protoArmorType_ < 14; ++protoArmorType_) {
        const RulesArmor &armor = Armor[protoArmorType_];
        if (has_tech(armor.preq_tech, factionID_) && armor.defense_rating >= rating) {
            best = protoArmorType_;
            rating = armor.defense_rating;
        }
    }
    protoArmorType_ = best;
    strcpy(prototype_name_, DesignAutomaticName);
    DesignConsiderWeapon = protoWeaponType_;
    DesignConsiderArmor = protoArmorType_;
    DesignConsiderChassis = protoChassisType_;
    DesignConsiderAbilities = protoAbilityFlags_;
    DesignConsiderOverride = 1;
    DesignConsiderPrototype = -1;
    consider_designs(factionID_);
    protoWeaponType_ = DesignConsiderWeapon;
    protoArmorType_ = DesignConsiderArmor;
    protoChassisType_ = DesignConsiderChassis;
    protoAbilityFlags_ = DesignConsiderAbilities;
    protoAbilityID1_ = protoAbilityID2_ = static_cast<uint32_t>(-1);
    for (int bit = 0; bit < 29; ++bit) {
        if (protoAbilityFlags_ & (1u << bit)) {
            if (static_cast<int>(protoAbilityID1_) < 0) protoAbilityID1_ = bit;
            else if (static_cast<int>(protoAbilityID2_) < 0) protoAbilityID2_ = bit;
        }
    }
    DesignConsiderOverride = 0;
    prototype.flags &= ~0x80;
    field_14210_ = 0;
}

// ORIGINAL: 0x0043BE30 ?select_name@DesignWin@@QAEXXZ 0x0043BE30-0x0043BF4C
// RECOVERY_BINDING: v1 {"evidence": ["git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/units/0043be30.cpp;ParseStrBuffer in src/general.h;image:0x6838F4 DESIGNNAME;typed32-byte prototype name"], "reason": "Prompt for the prototype name through the shared popup helper, preserve cancellation and unchanged-name paths, update its custom-name flag and redraw through the genuine virtual update slot."}
void DesignWin::select_name() {
    char name[80];
    strcpy(name, prototype_name_);
    if (strlen(name) == 0) {
        name_proto(name, protoID_, factionID_, protoChassisType_, protoWeaponType_,
                   protoArmorType_, protoAbilityFlags_, protoReactorType_);
        VehPrototypes[protoID_].flags &= ~2;
    }
    name[31] = 0;
    // The original prompt token at 0x006838F4 is DESIGNNAME.
    if (X_pop_ask("DESIGNNAME", 31, name, 0, 0) == 0) {
        if (strlen(ParseStrBuffer[0].str) == 0) {
            prototype_name_[0] = 0;
            VehPrototypes[protoID_].flags &= ~2;
        } else if (strcmp(name, ParseStrBuffer[0].str) != 0) {
            strncpy(prototype_name_, ParseStrBuffer[0].str, sizeof(prototype_name_));
            prototype_name_[31] = 0;
            VehPrototypes[protoID_].flags |= 2;
        }
        draw_vehicle(0);
        on_update();
    }
}

// The slide helpers are still original bodies (0x0045D440 / 0x0045D970);
// councwin.cpp declares and forwards them, and the definitions are
// external-linkage so this TU links against those (same shape as
// battlewin.cpp).
void __cdecl slide_show(GraphicWin *window, int mode);   // 0x0045D440
void __cdecl slide_hide(GraphicWin *window, int mode);   // 0x0045D970

// map.cpp owns the prototype ranking shared with this window's stack list.

// 0x00749C88 - menu.cpp's g_MENU_UP_WAVE; on_status plays it when the window
// opens. Defined (and owned) there; no header declares it.
extern Wave g_MENU_UP_WAVE;

// ---- editor-panel bindings (the per-file convention basewin.cpp and
// socialwin.cpp use; each folds to a bare immediate) ------------------------

// The shared canvas is MainInterfaceGlobal's GraphicWin Buffer subobject;
// restore_clip() uses that Buffer's own rect2_, not MainInterface's panel rect2_.
// draw_flash picks its small font size off WinScreenWidth (win.h, 0x320 on
// the shipped UI).
// diplo_lock (0x00539820) and diplo_unlock (0x005398C0) are unhomed;
// design_it reaches them through their original addresses like the other
// pending seams.

// 0x005398E0 ?pop_wait@@YAHXZ - defined in faction.cpp (BYTE_EXACT); no
// header declares it, and the editor popups pass it as their wait callback.
int __cdecl pop_wait();
int __cdecl tut_check2(int flag);
/*
 * Pinned-image provenance for the retired immutable string aliases above:
 * original popup text/path references included name keys 0x683604/0x6835E8/
 * 0x68367C/0x683660 and sprites 0x6835F8/0x6835D8/0x683670/0x683650;
 * type keys 0x683640/0x683624 and sprites 0x683634/0x683614; overwrite keys
 * 0x6836B8/0x68369C and sprites 0x6836AC/0x68368C; special keys
 * 0x68385C/0x683894/0x683878/0x6838AC/0x6838BC and sprite references
 * 0x683888/0x68384C/0x6838A0/0x683868. These point to immutable initialized
 * spellings (including `unit_sm.pcx` and `alunit_sm.pcx`), not source-owned
 * storage. Punctuation references included 0x682E9C/0x682E98/0x683920/
 * 0x683918 and 0x6838CC. The bytes at 0x682E90 and 0x682E94 are respectively
 * comma-space and colon-space. Distinct original copies of `-1` were referenced
 * at 0x6838D0/0x6838D4/0x6838D8, `+` at 0x6838DC/0x6838E4/0x6838EC, and `-2`
 * at 0x6838E0/0x6838E8/0x6838F0. These sites now use ordinary same-content
 * literals. Literal pooling can merge emitted storage; original per-occurrence
 * storage/reference correspondence remains a restoration obligation. No
 * native literal symbol or extent binding is asserted here.
 */
/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004345C0 ?UNK1@DesignWin@@QAEXXZ 0x004345C0-0x004345C1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK1@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::UNK1() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D330 ?on_iface_left_down@DesignWin@@QAEXHH@Z 0x0043D330-0x0043D333 BYTE_EXACT
// symbol    ?on_iface_left_down@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D340 ?on_iface_right_down@DesignWin@@QAEXHH@Z 0x0043D340-0x0043D343 BYTE_EXACT
// symbol    ?on_iface_right_down@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D400 ?on_iface_selected@DesignWin@@QAEXHH@Z 0x0043D400-0x0043D403 BYTE_EXACT
// symbol    ?on_iface_selected@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D410 ?on_iface_left_double_click@DesignWin@@QAEXHH@Z 0x0043D410-0x0043D413 BYTE_EXACT
// symbol    ?on_iface_left_double_click@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D420 ?on_iface_right_double_click@DesignWin@@QAEXHH@Z 0x0043D420-0x0043D423 BYTE_EXACT
// symbol    ?on_iface_right_double_click@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D470 ?on_iface_button_toggled@DesignWin@@QAEXHH@Z 0x0043D470-0x0043D473 BYTE_EXACT
// symbol    ?on_iface_button_toggled@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D480 ?on_iface_group_clicked@DesignWin@@QAEXHHH@Z 0x0043D480-0x0043D483 BYTE_EXACT
// symbol    ?on_iface_group_clicked@DesignWin@@UAEXHHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_group_clicked@DesignWin@@QAEXHHH@Z)(DesignWin* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_group_clicked(int, int, int) {
}

// ---- the interface table's own slots -------------------------------------

// Real slot44 workshop handler; compiler owns the local Popup lifetime.
// ORIGINAL: 0x00438460 ?on_button_clicked@DesignWin@@QAEXH@Z 0x00438460-0x004395E1;0x00654010-0x006541FC
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/unrecovered/00438460.cpp", "task:designwin-button-owner", "Front virtual slot44 dispatches here; real Players/PlayersData, VehPrototypes, Weapon/Armor, design-selection globals and NetDaemon own referenced state. Retire, upgrade and component branches retain native order; helper design selection remains unfinished."], "reason": "Recover the complete DesignWin button operation using actual C++ owners and automatic Popup lifetime. Original main/cold spans and seed lessons remain restoration evidence, not game acceptance."}
// symbol    ?on_button_clicked@DesignWin@@UAEXH@Z
// TRIED: fresh-temporary destructor sequencing for the Popup/Scroll/ BasePop/Dialogs teardown tail (same technique as Console::editor_diff, 0x004DD6F0) instead of hand-replaying the SEH funclet's exact interleave - compiles, MISMATCH #10 push/mov, sim ~0.59 on the framed row.
void DesignWin::on_button_clicked(int button_id) {
    Popup popup;
    TutWinGlobal->reset();

    if (button_id == -2 || button_id == 8) {
        field_CDC_ = 0;
        release_modal();
        g_FX.fade(0x18);
        return;
    }

    if (button_id == -1 || button_id == 4) {
        g_FX.fade(0x18);
        field_CDC_ = design_it();
        if (field_CDC_ != 0) {
            release_modal();
        }
        return;
    }

    if (button_id == 0x17 || button_id == 0x18) {
        if (ProtoSortCount > 0) {
            if (button_id == 0x17) {
                --field_CE4_;
                if (field_CE4_ < 0) {
                    field_CE4_ = ProtoSortCount;
                }
            } else {
                ++field_CE4_;
                if (field_CE4_ == ProtoSortCount) {
                    field_CE4_ = 0;
                }
            }
        }
        redraw();
        return;
    }

    if (button_id == 1) {
        field_CDC_ = design_it();
        setup_veh(static_cast<int>(protoID_), 0);
        proto_sort_2(static_cast<int>(factionID_));
        const int visible_count = ProtoSortCount;
        if (visible_count > 0) {
            for (int walk = 0; walk < visible_count; ++walk) {
                const int slot = (static_cast<int>(field_CE4_) + walk)
                    % visible_count;
                if (ProtoSortIds[slot] == static_cast<int>(protoID_)) {
                    field_CE4_ = (slot + visible_count - 5) % visible_count;
                    break;
                }
            }
        }
        redraw();
        return;
    }

    if (button_id == 0) {
        const int faction = static_cast<int>(factionID_);
        const int prototype_id = static_cast<int>(protoID_);
        const VehPrototype &prototype = VehPrototypes[prototype_id];

        parse_says(0, prototype.veh_name, -1, -1);
        const int active_count =
            PlayersData[faction].proto_id_active[prototype_id];
        const bool alien_rules = (Players[faction].rule_flags & 0x80) != 0;
        if (active_count != 0) {
            parse_num(0, active_count);
        }

        const int answer = alien_rules
            ? (active_count != 0
                ? popp(ScriptPopupFile, "RETIRETYPE", 0,
                       "alunit_sm.pcx", pop_wait)
                : popp(ScriptPopupFile, "RETIRETYPE2", 0,
                       "alunit_sm.pcx", pop_wait))
            : (active_count != 0
                ? popp(ScriptPopupFile, "RETIRETYPE", 0,
                       "unit_sm.pcx", pop_wait)
                : popp(ScriptPopupFile, "RETIRETYPE2", 0,
                       "unit_sm.pcx", pop_wait));
        if (answer == 0) {
            return;
        }

        if (!IsMultiplayerNet) {
            retire_proto(prototype_id, faction);
        } else {
            if (diplo_lock(0x19) != 0) {
                return;
            }
            message_data(0x2415, 0, prototype_id, faction, 0, 0);
            NetDaemonNet.await_exec(0);
            diplo_unlock();
        }

        setup_veh(prototype_id, 0);
        proto_sort_2(faction);
        const int count = ProtoSortCount;
        if (count > 0) {
            for (int walk = 0; walk < count; ++walk) {
                const int slot = (static_cast<int>(field_CE4_) + walk) % count;
                if (ProtoSortIds[slot] == prototype_id) {
                    field_CE4_ = (slot + count - 5) % count;
                    break;
                }
            }
        }
        redraw();
        return;
    }

    if (button_id == 2) {
        if ((GameDataStateStorage.fields_.game_state_ & 0x40) == 0) {
            return;
        }
        field_CDC_ = design_it();
        setup_veh(static_cast<int>(protoID_), 0);
        proto_sort_2(static_cast<int>(factionID_));
        if (ProtoSortCount > 0) {
            for (int walk = 0; walk < ProtoSortCount; ++walk) {
                const int slot = (static_cast<int>(field_CE4_) + walk)
                    % ProtoSortCount;
                if (ProtoSortIds[slot] == static_cast<int>(protoID_)) {
                    field_CE4_ = (slot + ProtoSortCount - 5) % ProtoSortCount;
                    break;
                }
            }
        }
        VehPrototypes[protoID_].flags ^= 4;
        redraw();
        return;
    }

    if (button_id == 3) {
        if ((GameDataStateStorage.fields_.game_state_ & 0x40) == 0) {
            return;
        }
        const bool alien_rules =
            (Players[static_cast<int>(factionID_)].rule_flags & 0x80) != 0;
        const int answer = alien_rules
            ? popp(ScriptPopupFile, "RETIREALL", 0,
                   "alunit_sm.pcx", 0)
            : popp(ScriptPopupFile, "RETIREALL", 0,
                   "unit_sm.pcx", 0);
        if (answer == 0) {
            return;
        }
        for (int faction = 1; faction < MaxPlayerNum; ++faction) {
            if (answer < 2 && faction != static_cast<int>(factionID_)) {
                continue;
            }
            // Native4386EF shifts by the current loop faction, not the editor.
            const uint8_t faction_mask = static_cast<uint8_t>(1u << faction);
            for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
                VehPrototypes[slot].obsolete_factions &=
                    static_cast<uint8_t>(~faction_mask);
                VehPrototypes[faction * MaxVehProtoFactionNum + slot].flags &=
                    static_cast<uint16_t>(~1u);
            }
            clear_bunglist(faction);
        }

        setup_veh(0, 0);
        proto_sort_2(static_cast<int>(factionID_));
        if (ProtoSortCount > 0) {
            for (int walk = 0; walk < ProtoSortCount; ++walk) {
                const int slot = (static_cast<int>(field_CE4_) + walk)
                    % ProtoSortCount;
                if (ProtoSortIds[slot] != 0) {
                    field_CE4_ = (slot + ProtoSortCount - 5) % ProtoSortCount;
                    break;
                }
            }
        }
        redraw();
        return;
    }

    if (button_id == 5) {
        select_name();
        return;
    }

    if (button_id == 6) {
        const int faction = static_cast<int>(factionID_);
        const int prototype_id = static_cast<int>(protoID_);
        const uint8_t faction_mask = static_cast<uint8_t>(1u << faction);
        VehPrototype &prototype = VehPrototypes[prototype_id];
        if ((prototype.obsolete_factions & faction_mask) != 0) {
            if ((prototype.flags & 8) != 0) {
                prototype.flags |= 0x200;
            }
            prototype.flags |= 8;
        } else {
            prototype.flags &= static_cast<uint16_t>(0xFDF7);
        }
        prototype.obsolete_factions ^= faction_mask;
        synch_obs(prototype_id);

        proto_sort_2(faction);
        const int count = ProtoSortCount;
        if (count > 0) {
            for (int walk = 0; walk < count; ++walk) {
                const int slot = (static_cast<int>(field_CE4_) + walk) % count;
                if (ProtoSortIds[slot] == prototype_id) {
                    const int next = (slot + 1) % count;
                    protoID_ = static_cast<uint32_t>(ProtoSortIds[next]);
                    break;
                }
            }
        }
        setup_veh(static_cast<int>(protoID_), 0);
        proto_sort_2(faction);
        const int sorted_count = ProtoSortCount;
        if (sorted_count > 0) {
            for (int walk = 0; walk < sorted_count; ++walk) {
                const int slot = (static_cast<int>(field_CE4_) + walk)
                    % sorted_count;
                if (ProtoSortIds[slot] == static_cast<int>(protoID_)) {
                    const int new_top = (slot + sorted_count - 5) % sorted_count;
                    if (new_top != static_cast<int>(field_CE4_)) {
                        field_CE4_ = new_top;
                        draw_stack();
                    }
                    break;
                }
            }
        }
        redraw();
        return;
    }

    if (button_id == 7) {
        const int faction = static_cast<int>(factionID_);
        const int old_prototype_id = static_cast<int>(protoID_);
        if (NetDaemonNet.lock(2, -1, -1, 0, -1, -1) != 0) {
            return;
        }

        UpgradePromptAnswered = 0;
        StringTemp[0] = '\0';
        ::strcat(StringTemp, VehPrototypes[old_prototype_id].veh_name);
        parse_says(0, StringTemp, -1, -1);
        popup.start(PopupStartCaption, "WHICHUPGRADE2", -1, nullptr,
                    0x40, nullptr);

        int match_count = 0;
        int selected_prototype = -1;
        const int faction_base = faction * MaxVehProtoFactionNum;
        const VehPrototype &old_prototype = VehPrototypes[old_prototype_id];
        for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
            const int candidate_id = faction_base + slot;
            const VehPrototype &candidate = VehPrototypes[candidate_id];
            if (candidate_id == old_prototype_id) {
                continue;
            }
            if ((candidate.flags & 1) == 0) {
                continue;
            }
            if ((candidate.obsolete_factions & (1u << faction)) != 0) {
                continue;
            }
            if (candidate.chassis_id != old_prototype.chassis_id) {
                continue;
            }
            if (((candidate.ability_flags ^ old_prototype.ability_flags)
                 & 0x8000) != 0) {
                continue;
            }

            if (candidate.plan != old_prototype.plan) {
                if (candidate.plan > 3 || old_prototype.plan > 3) {
                    continue;
                }
            }

            const int old_weapon_rating =
                Weapon[old_prototype.weapon_id].offense_rating;
            const int candidate_weapon_rating =
                Weapon[candidate.weapon_id].offense_rating;
            if (old_weapon_rating != 0) {
                if (old_weapon_rating > candidate_weapon_rating) {
                    continue;
                }
            } else if (candidate.weapon_id != 0) {
                continue;
            }

            const int old_armor_rating =
                Armor[old_prototype.armor_id].defense_rating;
            const int candidate_armor_rating =
                Armor[candidate.armor_id].defense_rating;
            // Native438C70 skips rejection when old defense <= candidate.
            if (old_armor_rating > candidate_armor_rating) {
                if (candidate_armor_rating == 1) {
                    continue;
                }
                if (old_weapon_rating != 0 &&
                    candidate_weapon_rating <= old_weapon_rating) {
                    continue;
                }
            }

            ++match_count;
            StringTemp[0] = '\0';
            ::strcat(StringTemp, candidate.veh_name);
            ::strcat(StringTemp, " |");
            say_stats_3(StringTemp, candidate_id);
            if ((candidate.flags & 4) == 0 && (old_prototype.flags & 4) != 0) {
                ::strcat(StringTemp, " *");
            }
            popup.item(StringTemp, candidate_id);
            selected_prototype = candidate_id;
        }
        NetDaemonNet.unlock();

        if (match_count == 0) {
            if ((Players[faction].rule_flags & 0x80) != 0) {
                popp(ScriptPopupFile, "NOUPGRADE", 0, "alunit_sm.pcx",
                     pop_wait);
            } else {
                popp(ScriptPopupFile, "NOUPGRADE", 0, "unit_sm.pcx",
                     pop_wait);
            }
            return;
        }
        if (match_count > 1) {
            selected_prototype = popup.exec(0, pop_wait);
        }
        if (selected_prototype < 0) {
            return;
        }

        const bool has_local_units =
            PlayersData[faction].proto_id_active[selected_prototype] != 0;
        const int upgraded = upgrade_prototype(
            faction, selected_prototype, old_prototype_id, 0);
        if (upgraded != 0) {
            protoID_ = static_cast<uint32_t>(selected_prototype);
            setup_veh(selected_prototype, 0);
            proto_sort_2(faction);
            const int sorted_count = ProtoSortCount;
            if (sorted_count > 0) {
                for (int walk = 0; walk < sorted_count; ++walk) {
                    const int slot = (static_cast<int>(field_CE4_) + walk)
                        % sorted_count;
                    if (ProtoSortIds[slot] == selected_prototype) {
                        const int new_top = (slot + sorted_count - 5)
                            % sorted_count;
                        if (new_top != static_cast<int>(field_CE4_)) {
                            field_CE4_ = new_top;
                            draw_stack();
                        }
                        break;
                    }
                }
            }
            redraw();
        }

        if (has_local_units) {
            if ((Players[faction].rule_flags & 0x80) != 0) {
                popp(ScriptPopupFile, "YESUPGRADE", 0, "alunit_sm.pcx",
                     pop_wait);
            } else {
                popp(ScriptPopupFile, "YESUPGRADE", 0, "unit_sm.pcx",
                     pop_wait);
            }
        }
        return;
    }

    int selected_component = -1;
    switch (button_id) {
        case 9: selected_component = 2; break;
        case 10: selected_component = 3; break;
        case 11: selected_component = 1; break;
        case 12: selected_component = 4; break;
        case 13: selected_component = 0x0A; break;
        case 14: selected_component = 6; break;
        case 15: selected_component = 7; break;
        case 16: selected_component = 8; break;
        case 17: selected_component = 9; break;
        case 18: selected_component = 0x11; break;
        case 19: selected_component = 0x0D; break;
        case 20: selected_component = 0x0E; break;
        case 21: selected_component = 0x0B; break;
        case 22: selected_component = 0x0C; break;
        default: break;
    }

    DesignConsiderOverride = 1;
    DesignConsiderPrototype = selected_component;
    DesignConsiderWeapon = static_cast<int>(protoWeaponType_);
    DesignConsiderArmor = static_cast<int>(protoArmorType_);
    DesignConsiderChassis = static_cast<int>(protoChassisType_);
    DesignConsiderAbilities = protoAbilityFlags_;
    DesignConsiderReactor = static_cast<int>(protoReactorType_);
    consider_designs(factionID_);

    protoWeaponType_ = static_cast<uint32_t>(DesignConsiderWeapon);
    protoArmorType_ = static_cast<uint32_t>(DesignConsiderArmor);
    protoAbilityFlags_ = DesignConsiderAbilities;
    protoChassisType_ = static_cast<uint32_t>(DesignConsiderChassis);
    protoReactorType_ = static_cast<uint32_t>(DesignConsiderReactor);
    protoAbilityID1_ = static_cast<uint32_t>(-1);
    protoAbilityID2_ = static_cast<uint32_t>(-1);
    for (int ability = 0; ability < MaxAbilityNum; ++ability) {
        if ((protoAbilityFlags_ & (1u << ability)) == 0) {
            continue;
        }
        if (static_cast<int>(protoAbilityID1_) < 0) {
            protoAbilityID1_ = static_cast<uint32_t>(ability);
        } else if (static_cast<int>(protoAbilityID2_) < 0) {
            protoAbilityID2_ = static_cast<uint32_t>(ability);
        }
    }

    if (DesignConsiderOverride <= 1) {
        if ((Players[static_cast<int>(factionID_)].rule_flags & 0x80) != 0) {
            popp(ScriptPopupFile, "NOPRESET", 0, "alunit_sm.pcx", pop_wait);
        } else {
            popp(ScriptPopupFile, "NOPRESET", 0, "unit_sm.pcx", pop_wait);
        }
        g_FX.play(0x27);
    } else {
        g_FX.play(0x2B);
    }
    DesignConsiderOverride = 0;
    on_redraw();
    on_update();
}

/*
// ORIGINAL: 0x0043D430 ?on_iface_button_clicked@DesignWin@@QAEXH@Z 0x0043D430-0x0043D44D BYTE_EXACT
// symbol    ?on_iface_button_clicked@DesignWin@@UAEXH@Z
// size      29 bytes
// prototype void (__thiscall ?on_iface_button_clicked@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0043D443
Return Value: n/a
Status: Complete
*/
// LEVER: the receiver is the ENTRY (the image's `add ecx,-0xa14` is VC6's own
//   adjustment to the front) and the dispatch is the front's slot 44, so the
//   whole body is one unqualified virtual call - `on_button_clicked(button_id)` is the
//   image's `call dword ptr [eax + 0xb0]`.
void DesignWin::on_iface_button_clicked(int button_id) {
    on_button_clicked(button_id);
}

/*
// ORIGINAL: 0x0043D450 ?on_iface_button_pressed@DesignWin@@QAEXH@Z 0x0043D450-0x0043D46F BYTE_EXACT
// symbol    ?on_iface_button_pressed@DesignWin@@UAEHH@Z
// size      31 bytes
// prototype int (__thiscall ?on_iface_button_pressed@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0043D463
Return Value: 0
Status: Complete
*/
// LEVER: the same slot-44 dispatch as vslot_13 with a constant 0 returned
//   after it - the interface wants an answer, the design window forwards the
//   click to the front and reports "not pressed".
int DesignWin::on_iface_button_pressed(int button_id) {
    on_button_clicked(button_id);
    return 0;
}

// ---- slots still carrying an original body ------------------------------

// The interface slot bodies run with the second-base receiver in the image
// (the slot holds the raw body address, so ECX is DesignWin+0xA14 - the
// seed for this body proves it: `self - 0xa14` is handed to the Win base,
// and flatButtons_ appears at ECX+0x2D8). The tree spells the body as a
// plain override and the compiled object shows NO adjustor thunks: the body
// compiles straight to the original's receiver convention, byte for byte.
// ORIGINAL: 0x0043D120 ?on_status@DesignWin@@QAEXH@Z 0x0043D120-0x0043D215 FILE BYTE_EXACT
// symbol    ?on_status@DesignWin@@UAEXH@Z
// size      245 bytes
// prototype void (__thiscall ?on_status@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D440 0x0045D970 0x004C6920 0x005D56B0 0x005ED9D0 0x005EDCD0 0x005FCBB0
// Homed from src/unrecovered/0043d120.cpp: open the design interface - paint
// the front's contents, show the 25 palette buttons (and the two toggles when
// interface bit 0x40 allows), update the main view, play the menu-up sound,
// slide and show the window - or close it all back down.
void DesignWin::on_status(int mode) {
    if (mode != 0) {
        static_cast<GraphicWin *>(this)->on_redraw();
        FlatButton *button = flatButtons_;
        int count = 0x19;
        do {
            button->show(4);
            ++button;
            --count;
        } while (count != 0);
        if ((GameDataStateStorage.fields_.game_state_ & 0x40) != 0) {
            flatButtons_[2].show(0);
            flatButtons_[3].show(0);
        } else {
            flatButtons_[2].hide();
            flatButtons_[3].hide();
        }
        MainInterfaceGlobal.update(0);
        do_all_draws();
        g_MENU_UP_WAVE.play();
        slide_show(this, 0);
        this->Win::show(3);
    } else {
        FlatButton *button = flatButtons_;
        int count = 0x19;
        do {
            button->hide();
            ++button;
            --count;
        } while (count != 0);
        slide_hide(this, 0);
        this->Win::hide();
        do_all_draws();
    }
}

// PENDING 0x0043D220 ?on_iface_left_click@DesignWin@@QAEXHH@Z 0x0043D220-0x0043D281
// symbol    ?on_iface_left_click@DesignWin@@UAEXHH@Z
// size      97 bytes
// prototype void (__thiscall ?on_iface_left_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FAB00 0x005169F0
// ORIGINAL: 0x0043D220 ?on_iface_left_click@DesignWin@@QAEXHH@Z 0x0043D220-0x0043D281 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x43D243,0x43D260,0x43D274;git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/units/0043d220.cpp"], "reason": "Interface secondary-base callback selects a stack prototype and invokes setup plus virtual redraw through the genuine DesignWin inheritance."}
// symbol    ?on_iface_left_click@DesignWin@@UAEXHH@Z
void DesignWin::on_iface_left_click(int a1, int a2) {
    int position;
    int type;
    TutorialWindow.reset();
    if (spot1_.check(a1, a2, &position, &type) != -1 && position >= 0) {
        protoID_ = position;
        setup_veh(position, 0);
        redraw();
    }
}

// ORIGINAL: 0x0043D290 ?on_iface_right_click@DesignWin@@QAEXHH@Z 0x0043D290-0x0043D327
// RECOVERY_BINDING: v1 {"evidence": ["src/designwin.cpp:DesignWin::on_iface_right_click", "src/designwin.h:spot1_", "src/alpha.h:Labels.strings_ptr"], "reason": "Resolve the prototype-stack hover target and show its localized label at the target's screen-space anchor."}
// symbol    ?on_iface_right_click@DesignWin@@UAEXHH@Z
// size      151 bytes
// prototype void (__thiscall ?on_iface_right_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDDE0 0x005ED240 0x005F83D0 0x005FAB00 0x006169A0
// Homed from src/unrecovered/0043d290.cpp: a right-click on the prototype
// stack (spot1_) raises the tutorial-style bubble naming the prototype under
// the pointer - the label pointer comes out of the game block at
// Labels.strings_ptr+0xCE8, and the bubble anchor is that map square pushed
// through the main interface's client_to_screen.
void DesignWin::on_iface_right_click(int xCoord, int yCoord) {
    TutWinGlobal->reset();
    int position;
    int type;
    if (spot1_.check(xCoord, yCoord, &position, &type) != -1 && position >= 0) {
        // The image swaps its two argument slots so the client_to_screen call
        // receives (&xCoord, &yCoord); the anchor rect is that screen square.
        MainInterfaceGlobal.client_to_screen(&xCoord, &yCoord);
        // The label pointer sits at byte offset 0xCE8 of the game block - slot
        // 0x33A of the LPSTR array, the same walk datalink.cpp's
        // CATEGORY_LABEL makes over Labels.strings_ptr.
        LPSTR label = Labels.strings_ptr[0x33A];
        RECT rect;
        rect.left = xCoord;
        rect.top = yCoord;
        rect.right = xCoord;
        rect.bottom = yCoord;
        // MainInterface declares its own (button_id, text) set_bubble_text,
        // which would hide the Win member the image calls directly.
        MainInterfaceGlobal.Win::set_bubble_text(StringTable.get(label),
                                                 &rect);
    }
}

// ORIGINAL: 0x00437BA0 ?on_mouse_move@DesignWin@@QAEXHH@Z 0x00437BA0-0x00437BD5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00437ba0.cpp", "DesignWin spot2_ member and Win::clear_bubble_text in src/"], "reason": "Home the direct pointer hover handler through the real DesignWin spot member and Win interface."}
// symbol    ?on_mouse_move@DesignWin@@UAEXHH@Z
// size      53 bytes
// prototype void (__thiscall ?on_mouse_move@DesignWin@@QAEXHH@Z)(DesignWin* this, int xCoord, int yCoord)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005F8500 0x005FAB00
// Homed from src/recovered/units/00437ba0.cpp.
void DesignWin::on_mouse_move(int xCoord, int yCoord) {
    Win::clear_bubble_text();
    Win::clear_bubble_text();

    int position;
    int type;
    spot2_.check(xCoord, yCoord, &position, &type);
}

// ORIGINAL: 0x0043D350 ?on_iface_mouse_move@DesignWin@@QAEXHH@Z 0x0043D350-0x0043D3B9 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0043d350.cpp", "DesignWin spot1_, field_CE0_ and draw_flash in src/", "Existing secondary-interface override symbol mapping"], "reason": "Replace the original-address forwarder with a typed interface hover handler while retaining the historical exact seed for restoration."}
// symbol    ?on_iface_mouse_move@DesignWin@@UAEXHH@Z
// size      105 bytes
// prototype void (__thiscall ?on_iface_mouse_move@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// Homed from src/recovered/units/0043d350.cpp.
void DesignWin::on_iface_mouse_move(int a1, int a2) {
    Win::clear_bubble_text();
    Win::clear_bubble_text();

    int position;
    int type;
    int result = spot1_.check(a1, a2, &position, &type);
    int selected_spot;
    if (result != -1) {
        selected_spot = position;
        if (field_CE0_ == selected_spot) {
            return;
        }
    } else {
        selected_spot = 0x3F;
        if (field_CE0_ == selected_spot) {
            return;
        }
    }
    field_CE0_ = selected_spot;
    draw_flash();
}

// ORIGINAL: 0x0043D3C0 ?on_iface_mouse_leave@DesignWin@@QAEXHH@Z 0x0043D3C0-0x0043D3F1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0043d3c0.cpp", "DesignWin field_CE0_, draw_flash and Win::clear_bubble_text in src/", "Existing secondary-interface override symbol mapping"], "reason": "Replace the original-address forwarder with a typed interface leave handler while retaining the historical exact seed for restoration."}
// symbol    ?on_iface_mouse_leave@DesignWin@@UAEXHH@Z
// size      49 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// Homed from src/recovered/units/0043d3c0.cpp.
void DesignWin::on_iface_mouse_leave(int a1, int a2) {
    Win::clear_bubble_text();
    Win::clear_bubble_text();
    if (field_CE0_ != 0x3F) {
        field_CE0_ = 0x3F;
        draw_flash();
    }
}


/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
// ORIGINAL: 0x00437BE0 ?on_mouse_leave@DesignWin@@QAEXHH@Z 0x00437BE0-0x00437BED BYTE_EXACT
// symbol    ?on_mouse_leave@DesignWin@@UAEXHH@Z
// size      13 bytes
// prototype void (__thiscall ?on_mouse_leave@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void DesignWin::on_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}


/*
// ORIGINAL: 0x0043BE10 ?select_special_1@DesignWin@@QAEXXZ 0x0043BE10-0x0043BE18 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?select_special_1@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0043AC70
Status: Complete
*/
void DesignWin::select_special_1() {
    select_special(0);
}

/*
// ORIGINAL: 0x0043BE20 ?select_special_2@DesignWin@@QAEXXZ 0x0043BE20-0x0043BE28 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?select_special_2@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0043AC70
Status: Complete
*/
void DesignWin::select_special_2() {
    select_special(1);
}

// The seam and definition for the unrecovered select_special, at the end of
// the file so no recovered body above shifts.
//
// auto_inline(off) is load-bearing, not tidiness, and being defined after the
// callers is NOT enough on its own: VC6 at /Ob2 defers codegen to the end of
// the translation unit and folds a forwarder defined later back into a caller
// defined earlier. Measured - without the pragma cl reports C4711 at
// designwin.cpp(153) and (161), the two select_special_N thunks, and each is
// an 8-byte BYTE_EXACT `call rel32` today.

/*
Purpose: Close the design window: dismiss all 25 palette buttons, release the
         spot list, stop the clock, release the buffer, reset the prototype
         selection, and close the three fonts.
// ORIGINAL: 0x0043C1A0 ?close@DesignWin@@QAEXXZ 0x0043C1A0-0x0043C261
// size      193 bytes
// prototype void (__thiscall ?close@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   6   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7470 0x005FA830 0x00616780 0x00619230
// indirect  0x0043C1B4
// MEASURED 2026-08-29: /O2 best at 12/42. Image instructions 0..11 agree -
//   the prologue and the whole 25-button loop, the slot-0x168 dispatch
//   arriving as the image's own single `call dword ptr [eax + 0x168]` out of
//   vtable_slot once /Ob2 inlines it (at /Ob0 the template function itself
//   is CALLED - `push 0x168 / push edi / call`, which is why /Ob0's
//   higher-similarity 6/42 is the worse listing; do not chase it).
//   Everything from image instruction 12 on diverges for the catalogued
//   MEASURED-inline ceiling: spot2_.shutdown(), time_.close() and the three
//   font_.close() calls are in-class definitions in spot.h / time.h /
//   font.h (in-class so ~Spot / ~Time / ~Font inline them), and VC6 inlines
//   all five HERE where the image makes five plain calls (0x005FA830,
//   0x00616780, 0x00619230 x3) - 141 instructions against the image's 42.
//   Out-of-line bodies would regress those destructor claims, and VC6
//   honours no per-site noinline.
Return Value: n/a
Status: Complete
*/
void DesignWin::close() {
    // A do/while counting DOWN from 25 over a walking pointer - the image
    // keeps `mov ebx, 0x19` and steps `add edi, 0xb4c` (sizeof FlatButton).
    FlatButton *button = flatButtons_;
    int count = 0x19;
    do {
        button->close();
        ++button;
        --count;
    } while (count != 0);

    spot2_.shutdown();
    // IMAGE ORDER: scene_rotation_ and field_CE8_ are written between the
    // clock receiver's computation and the call itself.
    scene_rotation_ = 0.0f;
    field_CE8_ = 0;
    time_.close();
    buffer_.close();
    factionID_ = 0;
    protoID_ = 0;
    protoChassisType_ = 0;
    protoWeaponType_ = 0;
    protoArmorType_ = 0;
    protoAbilityFlags_ = 0;
    protoReactorType_ = 1;
    field_14210_ = 0;
    mineralCostFactor_ = 0;
    field_CE0_ = static_cast<uint32_t>(-1);
    prototype_name_[0] = 0;
    font1_.close();
    font2_.close();
    font3_.close();
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
// The process-wide design window, 0x0071F2B0 in the image - REAL STORAGE,
// not the `Win *const TutWinDesWindow` binding that named terranx.exe data
// (see tutwin.h). The image's dynamic initializer at 0x00432850 constructs
// it at its fixed address before WinMain; DesignWin's constructor here is
// the empty inline the tree models, so this object is zero storage the
// game's own init populates, exactly as the image leaves it.
// The vehicle chassis animation queries this same window's Win base at
// 0x004C2BE9; the original receiver is the complete object at 0x0071F2B0.
// RECOVERY_OBJECT: v1 0x0071F2B0 ?TutWinDesWindow@@3VDesignWin@@A
DesignWin TutWinDesWindow;  // 0x0071F2B0
// Original CRT slots 0x00682078/0x0068207C construct DesignWin before Wave;
// their registered destructors run in reverse order.
// Pinned init 0x00432880 and registered fini 0x004328A0 load this same
// Wave receiver; start identity only, not complete allocation bounds.
// RECOVERY_OBJECT: v1 0x0071F240 ?g_DESIGNWIN_WAVE@@3VWave@@A
Wave g_DESIGNWIN_WAVE;  // 0x0071F240
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

// ORIGINAL: 0x00439670 ?bubble_me_elmo2@DesignWin@@QAEXPADHH@Z 0x00439670-0x004396B0
// RECOVERY_BINDING: v1 {"evidence": ["src/designwin.cpp:DesignWin::bubble_me_elmo2", "Win::client_to_screen and Win::set_bubble_text"], "reason": "Convert the pointer anchor from client to screen coordinates and raise the named bubble through the real Win base interface."}
// size      64 bytes
// prototype void (__thiscall ?bubble_me_elmo2@DesignWin@@QAEXPADHH@Z)(DesignWin* this, LPCSTR, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240 0x005F83D0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00439670
// measured tier  MISMATCH
// divergence     12
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00439670/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?bubble_me_elmo2@DesignWin@@QAEXPADHH@Z  at 0x00439670  (64 bytes)
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
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DesignWin::bubble_me_elmo2(char *a1, int a2, int a3) {
    client_to_screen(&a2, &a3);
    RECT anchor;
    anchor.left = a2;
    anchor.top = a3;
    anchor.right = a2;
    anchor.bottom = a3;
    set_bubble_text(a1, &anchor);
}

// ORIGINAL: 0x00439EB0 ?delete_abil@DesignWin@@QAEXH@Z 0x00439EB0-0x00439F18
// RECOVERY_BINDING: v1 {"evidence": ["src/designwin.h:protoAbilityFlags_ at 0x14208", "src/designwin.h:protoAbilityID1_ and protoAbilityID2_ at 0x14218/0x1421C"], "reason": "Remove one selected ability flag and rebuild the two displayed ability identifiers from the remaining flags."}
// size      104 bytes
// prototype void (__thiscall ?delete_abil@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00439EB0
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00439eb0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?delete_abil@DesignWin@@QAEXH@Z  at 0x00439EB0  (104 bytes)
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
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DesignWin::delete_abil(int a1) {
    if ((static_cast<uint32_t>(a1) & protoAbilityFlags_) != 0) {
        // Keep the UI slots in named object state; the old seed's
        // register-allocation observation does not apply to this body.
        protoAbilityFlags_ &= ~static_cast<uint32_t>(a1);
        protoAbilityID1_ = static_cast<uint32_t>(-1);
        protoAbilityID2_ = static_cast<uint32_t>(-1);
        int i = 0;
        do {
            if ((protoAbilityFlags_ & (1u << i)) != 0) {
                if (protoAbilityID1_ == static_cast<uint32_t>(-1)) {
                    protoAbilityID1_ = static_cast<uint32_t>(i);
                } else if (protoAbilityID2_ == static_cast<uint32_t>(-1)) {
                    protoAbilityID2_ = static_cast<uint32_t>(i);
                }
            }
            i++;
        } while (i < 0x1d);
    }
}

// ORIGINAL: 0x0043D0C0 ?show@DesignWin@@QAEXH@Z 0x0043D0C0-0x0043D0E8 BYTE_EXACT
// symbol    ?show@DesignWin@@UAEXH@Z
// LEVER: homed from 0043d0c0.cpp; needed the `symbol` fact (the tree emits
// UAEXH - it overrides Win::show's virtual - where the image has QAEXH) before
// measure could find it, then held BYTE_EXACT on the idiomatic base calls
// `is_visible()` / `set_iface_mode()` where the artifact had two
// reinterpret_casts of `this`.
// size      40 bytes
// prototype void (__thiscall ?show@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00446B40 0x0045D310 0x005F7E90
// 0x0043D0C0  ?show@DesignWin@@QAEXH@Z  ->  ?show@DesignWin@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::show(int a1) {
    if (!is_visible()) {
        ambience(0xD3);
        set_iface_mode();
    }
}

// ORIGINAL: 0x0043D0F0 ?hide@DesignWin@@QAEXXZ 0x0043D0F0-0x0043D116 BYTE_EXACT
// size      38 bytes
// prototype void (__thiscall ?hide@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00446B40 0x0045D380 0x005F7E90
// symbol    ?hide@DesignWin@@UAEXXZ
// LEVER: homed from 0043d0f0.cpp; same `symbol` fact as show - the tree emits
// the virtual UAEXX form that overrides Win::hide. Idiomatic base calls held
// BYTE_EXACT on the first measure.
// 0x0043D0F0  ?hide@DesignWin@@QAEXXZ  ->  ?hide@DesignWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::hide() {
    if (is_visible()) {
        release_iface_mode();
        ambience(0xd4);
    }
}

// ORIGINAL: 0x0043C070 ?construct_chassis@DesignWin@@QAEXXZ 0x0043C070-0x0043C09E BYTE_EXACT
// size      46 bytes
// prototype void (__thiscall ?construct_chassis@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C2470
// 0x0043C070  ?construct_chassis@DesignWin@@QAEXXZ  ->  ?construct_chassis@DesignWin@@QAEXXZ
// LEVER: homed from 0043c070.cpp. The named fields (factionID_ 0x141F4,
// protoChassisType_ 0x141FC, protoWeaponType_ 0x14200, protoReactorType_
// 0x1420C, caviar_ 0x1279C) replaced the artifact's five
// reinterpret_casts of `this` and held BYTE_EXACT first measure; the callee
// needed a pending_bodies forwarder plus the prototype now at the end of
// designwin.h.
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::construct_chassis() {
    vehdraw_construct_chassis(factionID_, protoChassisType_, protoWeaponType_, 0,
                              protoReactorType_, &caviar_);
}

// The image's shared design-window auto-build latch at 0x00733974. The
// construct_vehicle seed brackets its automatic weapon construction with
// writes of 1 and 0; construct_weapon and draw_weapon read the same slot.
// Keep it as source-owned mutable state rather than an alias into the image.
// RECOVERY_OBJECT: v1 0x00733974 ?DesignAutoBuildActive@@3HA
int DesignAutoBuildActive = 0;

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0043BF50 ?construct_vehicle@DesignWin@@QAEXXZ 0x0043BF50-0x0043C062 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0043bf50.cpp; src/designwin.h:DesignWin prototype and Caviar members; src/veh.h:RulesArmor and vehdraw_construct_*"], "reason": "Rebuild all component preview objects from the selected DesignWin state, using the typed armor table and existing vehicle-render helpers."}
void DesignWin::construct_vehicle() {
    caviar_.reset_objects();

    const int saved_chassis = static_cast<int>(protoChassisType_);
    if (protoWeaponType_ == 0x15 && saved_chassis == 1
        && Armor[protoArmorType_].defense_rating <= 1) {
        protoChassisType_ = 0;
    }
    if (protoID_ == 0x14) {
        protoChassisType_ = 0;
    }

    vehdraw_construct_chassis(static_cast<int>(factionID_),
                              static_cast<int>(protoChassisType_),
                              static_cast<int>(protoWeaponType_),
                              static_cast<int>(protoArmorType_),
                              static_cast<int>(protoReactorType_), &caviar_);

    DesignAutoBuildActive = 1;
    vehdraw_construct_weapon(static_cast<int>(factionID_),
                             static_cast<int>(protoChassisType_),
                             static_cast<int>(protoWeaponType_), &caviar_);
    DesignAutoBuildActive = 0;

    vehdraw_construct_armor(ConsoleGlobal.faction_id_,
                            static_cast<int>(protoChassisType_),
                            static_cast<int>(protoWeaponType_),
                            static_cast<int>(protoArmorType_), 0, &caviar_);
    vehdraw_construct_reactor(ConsoleGlobal.faction_id_,
                              static_cast<int>(protoChassisType_),
                              static_cast<int>(protoWeaponType_),
                              static_cast<int>(protoReactorType_),
                              static_cast<int>(protoAbilityFlags_), &caviar_);
    vehdraw_construct_abil(protoAbilityFlags_, &caviar_,
                           static_cast<int>(factionID_));

    protoChassisType_ = static_cast<uint32_t>(saved_chassis);
}

// ORIGINAL: 0x0043C0A0 ?construct_weapon@DesignWin@@QAEXXZ 0x0043C0A0-0x0043C0EF FILE BYTE_EXACT
// size      79 bytes
// prototype void (__thiscall ?construct_weapon@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C2CF0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043C0A0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043c0a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?construct_weapon@DesignWin@@QAEXXZ  at 0x0043C0A0  (79 bytes)
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
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DesignWin::construct_weapon() {
    int weapon_id = protoWeaponType_;
    if (weapon_id == 0x15 && DesignAutoBuildActive == 0) {
        vehdraw_construct_weapon(factionID_, -1, 0x15, &caviar_);
        return;
    }
    vehdraw_construct_weapon(factionID_, protoChassisType_, weapon_id, &caviar_);
}

// Fixed-slot bindings carried from 0043c0f0.cpp

// ORIGINAL: 0x0043C0F0 ?construct_armor@DesignWin@@QAEXH@Z 0x0043C0F0-0x0043C126 FILE BYTE_EXACT
// size      54 bytes
// prototype void (__thiscall ?construct_armor@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C3090
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043C0F0
// measured tier  NO_COMPILE
// refusal        u0043c0f0.cpp(44) : error C2061: syntax error : identifier 'Caviar' u0043c0f0.cpp(63) : error C2061: syntax error : identifier 'Caviar'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043c0f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?construct_armor@DesignWin@@QAEXH@Z  at 0x0043C0F0  (54 bytes)
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
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?construct_armor@DesignWin@@QAEXH@Z  at 0x0043C0F0  (54 bytes)
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


void DesignWin::construct_armor(int a1) {
    vehdraw_construct_armor(ConsoleGlobal.faction_id_, protoChassisType_, protoWeaponType_,
                            protoArmorType_, a1, &caviar_);
}

// ORIGINAL: 0x0043C130 ?construct_reactor@DesignWin@@QAEXXZ 0x0043C130-0x0043C163 BYTE_EXACT
// size      51 bytes
// prototype void (__thiscall ?construct_reactor@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C3800
// 0x0043C130  ?construct_reactor@DesignWin@@QAEXXZ  ->  ?construct_reactor@DesignWin@@QAEXXZ
// LEVER: homed from 0043c130.cpp. Same named-field lever as construct_armor.
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::construct_reactor() {
    vehdraw_construct_reactor(ConsoleGlobal.faction_id_, protoChassisType_, protoWeaponType_,
                              protoReactorType_, protoAbilityFlags_, &caviar_);
}

// The six renderer scratch writes at 0x009AC554..0x009AC562 are the
// fields of VehPrototypes[0x3f]: VehPrototypes begins at 0x009AB868 with a
// measured 0x34 stride, so row 63 begins at 0x009AC534. Keep the signed
// icon_offset and uint32 ability_flags types owned by VehPrototype itself.

// ORIGINAL: 0x00434650 ?UNK3@DesignWin@@QAEXXZ 0x00434650-0x00434691 FILE BYTE_EXACT
// size      65 bytes
// prototype void (__thiscall ?UNK3@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// LEVER: homed from units/00434650.cpp. The vehdraw stat block's six raw
// bindings renamed (VehDrawChassis/Weapon/Armor/Reactor/AbilityFlags/
// ComponentTail) and the named DesignWin fields put in their place; the
// byte-width of each target is carried by the binding's own type, so no
// reinterpret_cast of `this` survives.
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00434650
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00434650/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK3@DesignWin@@QAEXXZ  at 0x00434650  (65 bytes)
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
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DesignWin::UNK3() {
    VehPrototypes[0x3f].chassis_id = static_cast<uint8_t>(protoChassisType_);
    VehPrototypes[0x3f].weapon_id = static_cast<uint8_t>(protoWeaponType_);
    VehPrototypes[0x3f].armor_id = static_cast<uint8_t>(protoArmorType_);
    VehPrototypes[0x3f].reactor_id = static_cast<uint8_t>(protoReactorType_);
    VehPrototypes[0x3f].ability_flags = static_cast<uint32_t>(protoAbilityFlags_);
    VehPrototypes[0x3f].icon_offset = static_cast<int8_t>(0xFF);
}

// Original exec and shutdown share the writable tutor-state dword at
// 0x0073396C, immediately before the separate DesignWindowSelf pointer.
// The loader initializes this storage to zero. Its broader semantic role
// and containing allocation are not established; do not infer a bounded
// original object or a DesignWin member from the candidate's sizeof.
// RECOVERY_OBJECT: v1 0x0073396C ?DesignTutorState@@3HA
int DesignTutorState;
// The initializer's 14-entry grid uses process-wide scratch storage.
// RECOVERY_OBJECT: v1 0x009B7B10 ?DesignGridButtonIndex@@3HA
int DesignGridButtonIndex;
// exec stores its receiver here before starting time_; shutdown clears it.
// RECOVERY_OBJECT: v2 0x00733970 0x00733974 ?DesignWindowSelf@@3PAVDesignWin@@A 0x0043C170
DesignWin *DesignWindowSelf;

// ORIGINAL: 0x0043E9A0 ?exec@DesignWin@@QAEHHH@Z 0x0043E9A0-0x0043ECAD;0x00654709-0x0065471E FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/0043e9a0.cpp", "image:0x0043e9a0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// TRIED: a plain `Buffer buf;` local (RAII ctor/dtor, no explicit __try) reproduces the whole SEH prologue/epilogue (fs:[0] chain, the 0x654709/0x654714 unwind funclet) for free - matched through instruction #9 without writing any of that by hand.
// TRIED: MISMATCH #9 'push' vs 'mov', not chased further (802B, one of the largest bodies in this batch): vtable slot001/slot057 retyped in this file to (int) / (int,int,int) from the call sites' push counts.
int DesignWin::exec(int faction_id, int prototype_id) {
    Buffer execution_buffer;
    (void)execution_buffer;

    if (DesignTutorState != 2) init();

    const int interactive_run = prototype_id == -2;
    if (prototype_id < 0) {
        const int faction_prototype_base = faction_id * MaxVehProtoFactionNum;
        int fallback_prototype = faction_prototype_base;
        int most_recent_obsolete = prototype_id;
        int candidate = faction_prototype_base;
        int scanned = 0;
        int result = candidate;
        while (scanned < MaxVehProtoFactionNum) {
            const VehPrototype &record = VehPrototypes[candidate];
            result = candidate;
            if ((record.flags & PROTO_ACTIVE) == 0) break;
            const unsigned int obsolete = static_cast<unsigned char>(
                record.obsolete_factions);
            if ((obsolete & (1U << (faction_id & 0x1f))) != 0) {
                most_recent_obsolete = candidate;
            }
            ++scanned;
            ++candidate;
            result = most_recent_obsolete;
        }
        prototype_id = result < 0 ? fallback_prototype : result;
    }

    int normalized_faction = faction_id;
    if (normalized_faction < 0) normalized_faction = -normalized_faction;
    protoID_ = static_cast<uint32_t>(prototype_id);
    factionID_ = static_cast<uint32_t>(normalized_faction);
    enemy_capabilities(normalized_faction);
    mineralCostFactor_ = static_cast<uint32_t>(
        cost_factor(static_cast<int>(factionID_), 1, -1));
    protoAbilityID1_ = static_cast<uint32_t>(-1);
    protoAbilityID2_ = static_cast<uint32_t>(-1);
    setup_veh(prototype_id, 1);
    proto_sort_2(static_cast<int>(factionID_));

    const int sort_count = ProtoSortCount;
    int sorted_index = static_cast<int>(field_CE4_);
    const int sort_limit = sorted_index + sort_count;
    if (sorted_index < sort_limit) {
        do {
            const int relative_index = sorted_index % sort_count;
            if (prototype_id == ProtoSortIds[relative_index]) {
                field_CE4_ = static_cast<uint32_t>(
                    (relative_index + sort_count - 5) % sort_count);
                break;
            }
            ++sorted_index;
        } while (sorted_index < sort_limit);
    }

    field_14210_ = 0;
    if (!has_tech(Rules.tech_two_spec_abilities,
                  static_cast<int>(factionID_))) {
        int selected_abilities = 0;
        if (static_cast<int32_t>(protoAbilityID1_) >= 0) {
            ++selected_abilities;
        }
        if (static_cast<int32_t>(protoAbilityID2_) >= 0 &&
            ++selected_abilities > 1) {
            protoAbilityID2_ = static_cast<uint32_t>(-1);
        }
    }

    X_text_open(ScriptPopupFile, "DESIGNLABELS");
    for (int row = 0; row < 16; ++row) {
        text_get();
        preset_names_[row][0] = 0;
        strcat(preset_names_[row], TextBufferGetPtr);
    }
    text_close();

    if (DesignTutorState == 2) {
        redraw();
    } else {
        show(0);
        time_.start(DesignWin::timer_function, 0, 0x28, 5);
    }
    field_CDC_ = 0;
    DesignWindowSelf = this;

    if (DesignTutorState == 3) {
        DesignTutorState = 0;
    } else {
        if (tut_check2(0x2000) != 0) {
            TutorialWindow.set_tour_stage(7);
            int x;
            int y;
            TutorialWindow.des_rect(&vehicle_panel_rect_, &x, &y);
            MapWin *const map = MapWinTable[0];
            GraphicWin *const owner = map != 0
                ? static_cast<GraphicWin *>(map) : 0;
            TutorialWindow.tut_win(owner, "DESWIN", x, y, 0, 1, -1, -1);
        }
        if (!interactive_run) {
            set_modal(0, pop_wait, &MainInterfaceGlobal);
            if (DesignTutorState == 0) {
                hide();
                do_all_draws();
                DesignWindowSelf = 0;
                time_.stop();
                time_.release_modal();
                DesignTutorState = 0;
            } else {
                DesignTutorState = 2;
            }
        }
    }

    return static_cast<int>(field_CDC_);
}


// ORIGINAL: 0x0043C170 ?timer_function@DesignWin@@QAAXH@Z 0x0043C170-0x0043C19B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/0043c170.cpp;Time cdecl callback interface"], "reason": "Receiver-free timer callback reloads the shared active design owner between preview drawing and rectangle update, then draws the tutorial arrow."}
// symbol    ?timer_function@DesignWin@@SAXH@Z
void __cdecl DesignWin::timer_function(int) {
    DesignWin *window = DesignWindowSelf;
    window->draw_vehicle(0);
    // The callback reloads the shared owner after draw_vehicle returns.
    window = DesignWindowSelf;
    window->update(&window->vehicle_panel_rect_, 0);
    TutorialWindow.draw_arrow();
}

// ORIGINAL: 0x0043E960 ?shut_that_badboy_down@DesignWin@@QAEXXZ 0x0043E960-0x0043E99D FILE
// size      61 bytes
// prototype void (__thiscall ?shut_that_badboy_down@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FCBB0 0x00616730 0x00616870
// indirect  0x0043E96F
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043E960
// measured tier  MISMATCH
// divergence     8
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043e960/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?shut_that_badboy_down@DesignWin@@QAEXXZ  at 0x0043E960  (61 bytes)
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
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DesignWin::shut_that_badboy_down() {
    if (DesignTutorState != 1) {
        // Constructor 0x0043454C installs front table 0x0066B148;
        // its slot 2 is DesignWin::hide (0x0043D0F0). Preserve virtual
        // dispatch, including a further-derived override, on the real owner.
        hide();
        do_all_draws();
        Time *modal = &time_;
        DesignWindowSelf = 0;
        modal->stop();
        modal->release_modal();
    }
    DesignTutorState = 0;
}

// MISMATCH, ruled out (mnemonic_similarity 0.875, 65 vs 61 bytes):
// divergence #8, original 'lea' vs rebuilt 'add'+'mov', plus one inserted
// 'mov' before the second call. The original computes `this + 0x12774` ONCE
// into ecx (`lea ecx,[esi+0x12774]`) and reuses that same ecx, UNRELOADED,
// as the receiver for BOTH `modal->stop()` and `modal->release_modal()` -
// two back-to-back thiscall calls with a global store in between that does
// not touch ecx. Every VC6 phrasing tried here (named pointer local, named
// reference local, and the un-named expression repeated verbatim at both
// call sites, which CSEs to the same code) makes /O2 instead promote the
// address into esi (destructively overwriting `this`, which is dead by
// this point) and reload ecx with a 2-byte `mov ecx,esi` before EACH call -
// the textbook-safe thing to do, since a compiler that only sees `Time`
// as an opaque extern declaration cannot prove `Time::stop()` (66 bytes,
// calls timeKillEvent/KillTimer/flush_timer - genuinely not a leaf) leaves
// ecx holding the caller's value on return. src/time.h's
// `release_modal` declaration (00616870) is what this tree calls; the unit that
// carried that body was folded away (commit 619631a0), and it showed
// `Time::release_modal()` never touches `this` (`TimeModal =
// nullptr;`), so the no-reload choice the original compiler made is only
// explainable by whole-translation-unit visibility into that callee's body
// - something this scaffold's opaque-class model structurally cannot give
// it. Ruled out rather than re-derive: this is a same-shape sibling of the
// documented INSTRUCTION SCHEDULING wall, but for cross-call register
// liveness instead of instruction order.


// ==== coverage batch: 004345d0 / 00439960 / 00439a20 / 0043d490 ============

// ORIGINAL: 0x004345D0 ?UNK2@DesignWin@@QAEHH@Z 0x004345D0-0x00434649 FILE
// size      121 bytes
// prototype int (__thiscall ?UNK2@DesignWin@@QAEHH@Z)(DesignWin* this, int protoID)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// Homed from src/unrecovered/004345d0.cpp: the editor's "is this the same
// design" test - VehPrototypes[proto_id] against the component set the window
// currently holds. The seed's opaque `g_design_table` at 0x9AB888 is
// VehPrototypes shifted past veh_name: 0x9AB888 is [0].ability_flags and
// 0x9AB88C..[.F] the chassis/weapon/armor/reactor bytes.
int DesignWin::UNK2(int proto_id) {
    const VehPrototype &proto = VehPrototypes[proto_id];
    if (protoChassisType_ != proto.chassis_id) {
        return 0;
    }
    if (protoWeaponType_ != proto.weapon_id) {
        return 0;
    }
    if (protoArmorType_ != proto.armor_id) {
        return 0;
    }
    if (protoReactorType_ != proto.reactor_id) {
        return 0;
    }
    return protoAbilityFlags_ == proto.ability_flags;
}

// ORIGINAL: 0x00439960 ?on_left_click@DesignWin@@QAEXHH@Z 0x00439960-0x004399FA FILE
// symbol    ?on_left_click@DesignWin@@UAEXHH@Z
// size      154 bytes
// prototype void (__thiscall ?on_left_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int xCoord, int yCoord)
// callers   0   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00439F20 0x0043A300 0x0043A760 0x0043AA00 0x0043AC70 0x004BDDE0 0x005FAB00
// Homed from src/unrecovered/00439960.cpp: a click on the palette spots
// (spot2_) picks the component its type names; types 4 and 5 are the two
// special-ability slots.
void DesignWin::on_left_click(int xCoord, int yCoord) {
    TutWinGlobal->reset();
    int position;
    int type;
    if (spot2_.check(xCoord, yCoord, &position, &type) != -1) {
        switch (type) {
        case 0:
            select_chassis();
            return;
        case 1:
            select_weapon();
            return;
        case 2:
            select_armor();
            return;
        case 3:
            select_reactor();
            return;
        case 4:
            select_special(0);
            return;
        case 5:
            select_special(1);
            return;
        }
    }
}

// ORIGINAL: 0x00439A20 ?on_key_click@DesignWin@@QAEHHH@Z 0x00439A20-0x00439B3B FILE
// symbol    ?on_key_click@DesignWin@@UAEHHH@Z
// size      283 bytes
// prototype int (__thiscall ?on_key_click@DesignWin@@QAEHHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0043D510 0x004BDDE0
// indirect  0x00439A5D 0x00439AE1 0x00439AEF 0x00439B15 0x00439B2B
// Homed from src/unrecovered/00439a20.cpp: the second argument scrolls the
// prototype stack (page up/down/home/end by nines) or forwards to the front
// vtable's slot 44 (on_button_clicked) with 4/8/0x17/0x18; the first
// argument forwards 6 or 7 the same way. Always answers 1.
int DesignWin::on_key_click(int a1, int a2) {
    TutWinGlobal->reset();
    switch (a2) {
    case 0x0D:
        on_button_clicked(4);
        break;
    case 0x1B:
        on_button_clicked(8);
        break;
    case 0x21:
    case 0x69: {
        int moved = (field_CE4_ + ProtoSortCount - 9) % ProtoSortCount;
        field_CE4_ = (moved < 0) ? 0 : moved;
        draw_stack();
        break;
    }
    case 0x22:
    case 0x63: {
        int moved = (field_CE4_ + ProtoSortCount + 9) % ProtoSortCount;
        field_CE4_ = (moved < 0) ? 0 : moved;
        draw_stack();
        break;
    }
    case 0x23:
    case 0x61: {
        int bottom = ProtoSortCount - 10;
        field_CE4_ = (bottom < 0) ? 0 : bottom;
        draw_stack();
        break;
    }
    case 0x24:
    case 0x67:
        field_CE4_ = 0;
        draw_stack();
        break;
    case 0x25:
    case 0x64:
        on_button_clicked(0x17);
        break;
    case 0x27:
    case 0x66:
        on_button_clicked(0x18);
        break;
    }
    switch (a1) {
    case 0x4D:
    case 0x4F:
    case 0x6D:
    case 0x6F:
        on_button_clicked(6);
        break;
    case 0x55:
    case 0x75:
        on_button_clicked(7);
        break;
    }
    return 1;
}

// ORIGINAL: 0x0043D490 ?center_stack@DesignWin@@QAEXHH@Z 0x0043D490-0x0043D50B FILE
// size      123 bytes
// prototype void (__thiscall ?center_stack@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0043D510 0x005A63D0
// Homed from src/unrecovered/0043d490.cpp: re-rank the faction's prototypes,
// find proto_id in the ranked list and move the stack window's anchor
// (field_CE4_) so that entry sits five rows from the top.
void DesignWin::center_stack(int proto_id, int redraw) {
    // Re-rank through the source-owned map helper and its shared sort state.
    proto_sort_2(factionID_);

    int count = ProtoSortCount;
    int start = field_CE4_;
    bool found = false;
    int rem = 0;
    if (start < start + count) {
        int scan = start;
        do {
            rem = scan % count;
            if (proto_id == ProtoSortIds[rem]) {
                found = true;
                break;
            }
            ++scan;
        } while (scan < field_CE4_ + count);
    }
    if (found) {
        int old = field_CE4_;
        int centred = (rem + count - 5) % count;
        field_CE4_ = centred;
        if (redraw != 0 && centred != old) {
            draw_stack();
        }
    }
}

// ---- component picker -----------------------------------------------------
template <typename Record>
static int picker_row_before(int selected, const Record *records, int count) {
    int row = 0;
    for (int index = 0; index < count; ++index) {
        if (records[index].preq_tech >= -1) {
            if (index == selected) return row;
            ++row;
        }
    }
    return row;
}

template <typename Record>
static int picker_record_at(int row, const Record *records, int count) {
    for (int index = 0; index < count; ++index) {
        if (records[index].preq_tech >= -1) {
            if (row == 0) return index;
            --row;
        }
    }
    return count;
}

static const int DesignSelectableChassisCount = CHSI_MISSILE + 1;

static void redraw_design_picker_changes(DesignWin *window) {
    window->check_abil();
    window->draw_chassis(0);
    window->draw_armor(0);
    window->draw_reactor(0);
    window->draw_weapon(0);
    window->draw_vehicle(0);
    window->draw_special_1(0);
    window->draw_special_2(0);
    window->draw_flash();
    window->draw_info();
    window->on_update();
}

// check_abil uses the class's named selection state and the project Rules
// object rather than reproducing the seed's byte-offset view.
// ORIGINAL: 0x00439C00 ?check_abil@DesignWin@@QAEXXZ 0x00439C00-0x00439D24
// RECOVERY_BINDING: v1 {"evidence": ["DesignWin selection members in src/designwin.h", "abil_cond in src/veh.cpp and bit_count in src/general.cpp", "Rules.tech_two_spec_abilities in src/alpha.h"], "reason": "Validate each selected ability against the current chassis/weapon/armor and maintain the two displayed ability slots."}
void DesignWin::check_abil() {
    if (Chassis[protoChassisType_].missile != 0) {
        protoAbilityFlags_ = 0;
        protoAbilityID1_ = static_cast<uint32_t>(-1);
        protoAbilityID2_ = static_cast<uint32_t>(-1);
        return;
    }

    for (int ability_id = 0; ability_id < 0x1D; ++ability_id) {
        const uint32_t bit = 1u << ability_id;
        if ((protoAbilityFlags_ & bit) == 0) {
            continue;
        }

        if ((bit == 0x80 && (protoAbilityFlags_ & 0x40) != 0)
            || !abil_cond(ability_id, static_cast<int>(protoChassisType_),
                          static_cast<int>(protoWeaponType_),
                          static_cast<int>(protoArmorType_))
            || (bit_count(static_cast<int>(protoAbilityFlags_)) > 1
                && (bit_count(static_cast<int>(protoAbilityFlags_)) != 2
                    || !has_tech(Rules.tech_two_spec_abilities,
                                 static_cast<int>(factionID_))))) {
            protoAbilityFlags_ &= ~bit;
            protoAbilityID1_ = static_cast<uint32_t>(-1);
            protoAbilityID2_ = static_cast<uint32_t>(-1);
            for (int candidate = 0; candidate < 0x1D; ++candidate) {
                if ((protoAbilityFlags_ & (1u << candidate)) == 0) {
                    continue;
                }
                if (protoAbilityID1_ == static_cast<uint32_t>(-1)) {
                    protoAbilityID1_ = static_cast<uint32_t>(candidate);
                } else if (protoAbilityID2_ == static_cast<uint32_t>(-1)) {
                    protoAbilityID2_ = static_cast<uint32_t>(candidate);
                }
            }
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00439D30 ?suggest_abil@DesignWin@@QAEXH@Z 0x00439D30-0x00439EA3 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00439d30.cpp", "src/veh.h:RulesAbility and abil_index", "src/faction.h:Player faction bonus members", "src/game.h:GamePreferences", "src/alpha.h:Rules.tech_two_spec_abilities", "src/console.h:ConsoleGlobal.faction_id_"], "reason": "Suggest a valid researched ability through the real design selection and faction bonus records, retaining the original seed's anomalous bonus scan as a restoration obligation."}
void DesignWin::suggest_abil(int ability) {
    check_abil();
    if ((GameDataStateStorage.fields_.game_preferences_ & 0x4000) == 0) return;

    const int ability_id = abil_index(ability);
    const int faction = static_cast<int>(factionID_);
    if (!has_tech(Ability[ability_id].preq_tech, faction)
        || !abil_cond(ability_id, static_cast<int>(protoChassisType_),
                      static_cast<int>(protoWeaponType_),
                      static_cast<int>(protoArmorType_))
        || bit_count(static_cast<int>(protoAbilityFlags_)) > 1
        || (protoAbilityFlags_ != 0
            && !has_tech(Rules.tech_two_spec_abilities, faction))) {
        return;
    }

    const Player &local = Players[ConsoleGlobal.faction_id_];
    for (int i = 0; i < local.faction_bonus_count && i < 8; ++i) {
        if (local.faction_bonus_id[i] == 15
            && local.faction_bonus_val1[i] == ability_id) {
            return;
        }
    }

    protoAbilityFlags_ |= static_cast<uint32_t>(ability);
    protoAbilityID1_ = static_cast<uint32_t>(-1);
    protoAbilityID2_ = static_cast<uint32_t>(-1);
    for (int id = 0; id < 0x1D; ++id) {
        if ((protoAbilityFlags_ & (1u << id)) == 0) continue;
        if (protoAbilityID1_ == static_cast<uint32_t>(-1)) {
            protoAbilityID1_ = static_cast<uint32_t>(id);
        } else if (protoAbilityID2_ == static_cast<uint32_t>(-1)) {
            protoAbilityID2_ = static_cast<uint32_t>(id);
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00434BC0 ?draw_chassis@DesignWin@@QAEXH@Z 0x00434BC0-0x00434F36
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:434BC0..434F36", "historical seed git:20b88b523:src/recovered/units/00434bc0.cpp preserves the full renderer, palette sequence, imported uppercase call, title/name wrap branch, unchecked triad lookup and exact 0x90 chassis stride", "src/designwin.h:chassis_panel_rect_, component fields, font1_/font2_, caviar_; src/veh.h:RulesChassis/Chassis/Triad; src/interface_palette.cpp:TextColourTable; src/strings.h:StringTemp/StringTable"], "reason": "Restore the complete chassis editor panel through the real DesignWin, Caviar, RulesChassis, string and palette owners. Preserve native callback order, line geometry, width-dependent wrapping and unchecked localized triad indexing; retain the original identity and span without a new byte claim."}
static int designwin_panel_signed(uint32_t bits);
static int designwin_panel_add(int left, int right);
static int designwin_panel_subtract(int left, int right);
static int designwin_panel_font_height(int extra, int line_height, int height);
static void designwin_panel_append_label(char *output, int label_id);
extern int TextColourTable[195];

void DesignWin::draw_chassis(int redraw) {
    (void)redraw;
    Buffer *const canvas = static_cast<Buffer *>(this);
    RECT rect = chassis_panel_rect_;
    canvas->set_clip(&rect);
    buffer_.copy(canvas, &rect);

    RECT rect2;
    rect2.left = rect.left;
    rect2.right = rect.right;

    const int title_height = designwin_panel_font_height(
        font2_.unk_1_, font2_.line_height(), font2_.pixel_height());
    rect.top = designwin_panel_add(rect.top, title_height);
    // Native initializes only left/right/top here; rect2.bottom remains the
    // original stack value passed through the Buffer call.
    rect2.top = designwin_panel_subtract(rect.top, title_height);

    StringTemp[0] = '\0';
    designwin_panel_append_label(StringTemp, 0x140);
    CharUpperA(StringTemp);
    canvas->set_text_color(TextColourTable[156], TextColourTable[157], 1, 1);
    canvas->write_cent_l(&font2_, StringTemp, &rect2,
                         static_cast<int>(strlen(StringTemp)));

    caviar_.reset_objects();
    const int chassis = static_cast<int>(protoChassisType_);
    const int model_bottom = designwin_panel_subtract(
        rect.bottom,
        designwin_panel_add(
            designwin_panel_signed(static_cast<uint32_t>(font1_.pixel_height()) * 2u),
            5));
    rect.bottom = model_bottom;
    vehdraw_construct_chassis(static_cast<int>(factionID_), chassis,
                              static_cast<int>(protoWeaponType_), 0,
                              static_cast<int>(protoReactorType_), &caviar_);
    caviar_.set_scaling(0.05f);
    caviar_.set_scene_rotation(0.5233333706855774f, 0.0f, 0.0f);
    caviar_.render(canvas,
        designwin_panel_add(rect.right, rect.left) / 2,
        designwin_panel_add(rect.bottom, rect.top) / 2, 2);

    canvas->set_font(&font1_, 0, 0, 0);
    rect2.top = rect.bottom;
    rect.bottom = designwin_panel_add(rect.bottom, font1_.pixel_height());
    canvas->set_text_color(TextColourTable[158], TextColourTable[159], 1, 1);

    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(Chassis[chassis].offsv_1_name));
    CharUpperA(StringTemp);

    const int available_width = designwin_panel_subtract(rect.right, rect.left);
    const int required_width = font1_.width(StringTemp);
    if (available_width < required_width) {
        const int line_height = designwin_panel_font_height(
            font2_.unk_1_, font2_.line_height(), font2_.pixel_height());
        rect.bottom = designwin_panel_subtract(rect.bottom, line_height);
        rect2.top = designwin_panel_subtract(rect2.top, line_height);
        canvas->wrap_cent(StringTemp, &rect);
        rect2.top = designwin_panel_add(rect2.top, line_height);
        rect.bottom = designwin_panel_add(rect.bottom, line_height);
    } else {
        canvas->write_cent_l(StringTemp, &rect,
                             static_cast<int>(strlen(StringTemp)));
    }

    const int detail_advance = designwin_panel_add(font1_.pixel_height(), 3);
    rect2.top = designwin_panel_add(rect2.top, detail_advance);
    rect.bottom = designwin_panel_add(rect.bottom, detail_advance);
    canvas->set_text_color(TextColourTable[160], TextColourTable[161], 1, 1);

    StringTemp[0] = '\0';
    const unsigned char triad = Chassis[chassis].triad;
    strcat(StringTemp, StringTable.get(Triad[triad]));
    strcat(StringTemp, BufferJoinSpace);
    char speed[80];
    _itoa(static_cast<int>(Chassis[chassis].speed), speed, 10);
    strcat(StringTemp, speed);
    canvas->write_cent_l(StringTemp, &rect,
                         static_cast<int>(strlen(StringTemp)));
}

// Palette storage is owned by palette.cpp; VW00 is owned by caviar.cpp.
extern int TextColourTable[195];

// Preserve the original 32-bit coordinate arithmetic without signed overflow.
static int designwin_panel_signed(uint32_t bits) {
    return bits <= 0x7FFFFFFFu ? static_cast<int>(bits)
        : -1 - static_cast<int>(~bits);
}

static int designwin_panel_add(int left, int right) {
    return designwin_panel_signed(static_cast<uint32_t>(left) +
                                  static_cast<uint32_t>(right));
}

static int designwin_panel_subtract(int left, int right) {
    return designwin_panel_signed(static_cast<uint32_t>(left) -
                                  static_cast<uint32_t>(right));
}

static int designwin_panel_font_height(int extra, int line_height, int height) {
    return extra < 0 ? line_height : designwin_panel_add(height, extra);
}

static void designwin_panel_append_number(char *output, int value) {
    char digits[32];
    _itoa(value, digits, 10);
    strcat(output, digits);
}

static void designwin_panel_append_label(char *output, int label_id) {
    char *text = label_get(label_id / 4);
    strcat(output, text);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00434F40 ?draw_weapon@DesignWin@@QAEXH@Z 0x00434F40-0x004353FF
// RECOVERY_BINDING: v1 {"evidence": ["git:09f23e99:src/designwin.cpp preserves the original identity and historical seed provenance", "Pinned434F40..4353FF reads real DesignWin panel/font/selection members, VW00 at7871C0 and TextColourTable swatches156..161", "src/caviar.cpp:g_VW00_CAVIARDATA and src/interface_palette.cpp:textcolor_init own model and palette storage"], "reason": "Render the weapon heading, model, localized name and offense/cost detail through genuine source-owned objects; preserve live callback reads and original 32-bit coordinate arithmetic."}
void DesignWin::draw_weapon(int redraw) {
    (void)redraw;
    Buffer *const canvas = static_cast<Buffer *>(this);
    RECT area = weapon_panel_rect_;
    canvas->set_clip(&area);
    buffer_.copy(canvas, &area);

    const int title_height = designwin_panel_font_height(
        font2_.unk_1_, font2_.line_height(), font2_.pixel_height());
    RECT title = area;
    title.bottom = designwin_panel_add(title.top, title_height);
    StringTemp[0] = '\0';
    designwin_panel_append_label(StringTemp,
        Weapon[protoWeaponType_].offense_rating == 0 ? 0x200 : 0x204);
    CharUpperA(StringTemp);
    canvas->set_text_color(TextColourTable[156], TextColourTable[157], 1, 1);
    canvas->write_cent_l(&font2_, StringTemp, &title,
                         static_cast<int>(strlen(StringTemp)));

    const int name_top = designwin_panel_subtract(
        designwin_panel_subtract(area.bottom,
            designwin_panel_add(font1_.pixel_height(), font1_.pixel_height())), 5);
    caviar_.reset_objects();
    const int weapon_id = static_cast<int>(protoWeaponType_);
    if (weapon_id == 0) {
        caviar_.add_object(&g_VW00_CAVIARDATA, 0);
        caviar_.set_scaling(0.15f);
    } else {
        const int chassis_id = weapon_id == 0x15 &&
                               DesignAutoBuildActive == 0
            ? -1 : static_cast<int>(protoChassisType_);
        vehdraw_construct_weapon(static_cast<int>(factionID_), chassis_id,
                                 weapon_id,
                                 &caviar_);
        const int mode = Weapon[protoWeaponType_].mode;
        caviar_.set_scaling(mode < 3 ? 0.15f
            : (mode == 9 ? 0.06f : 0.08f));
    }
    const int center_x = designwin_panel_add(weapon_panel_rect_.left, weapon_panel_rect_.right) / 2;
    const int center_y = designwin_panel_add(weapon_panel_rect_.top, weapon_panel_rect_.bottom) / 2;
    caviar_.set_scene_rotation(0.5233333706855774f, 0.0f, 0.0f);
    caviar_.render(canvas, center_x, center_y, 6);

    canvas->set_font(&font1_, 0, 0, 0);
    RECT name_line = area;
    name_line.top = name_top;
    name_line.bottom = designwin_panel_add(name_line.top, font1_.pixel_height());
    canvas->set_text_color(TextColourTable[158], TextColourTable[159], 1, 1);
    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(Weapon[protoWeaponType_].name));
    CharUpperA(StringTemp);
    if (font1_.width(StringTemp) > designwin_panel_subtract(area.right, area.left)) {
        const int wrap_height = designwin_panel_font_height(
            font1_.unk_1_, font1_.line_height(), font1_.pixel_height());
        name_line.top = designwin_panel_subtract(name_line.top, wrap_height);
        name_line.bottom = designwin_panel_subtract(name_line.bottom, wrap_height);
        canvas->wrap_cent(StringTemp, &name_line);
        const int restored_height = designwin_panel_font_height(
            font1_.unk_1_, font1_.line_height(), font1_.pixel_height());
        name_line.top = designwin_panel_add(name_line.top, restored_height);
        name_line.bottom = designwin_panel_add(name_line.bottom, restored_height);
    } else {
        canvas->write_cent_l(StringTemp, &name_line,
                             static_cast<int>(strlen(StringTemp)));
    }

    RECT detail = name_line;
    const int advance = designwin_panel_add(font1_.pixel_height(), 3);
    detail.top = designwin_panel_add(detail.top, advance);
    detail.bottom = designwin_panel_add(detail.bottom, advance);
    canvas->set_text_color(TextColourTable[160], TextColourTable[161], 1, 1);
    StringTemp[0] = '\0';
    if (Weapon[protoWeaponType_].mode < 3) {
        designwin_panel_append_label(StringTemp, 0x204);
        strcat(StringTemp, " ");
        const int offense = Weapon[protoWeaponType_].offense_rating;
        if (offense < 0)
            designwin_panel_append_label(StringTemp, 0x310);
        else if (offense < 0x63)
            designwin_panel_append_number(StringTemp, offense);
        else
            strcat(StringTemp, "*");
    } else {
        designwin_panel_append_label(StringTemp, 0x200);
        strcat(StringTemp, " ");
        designwin_panel_append_number(StringTemp, Weapon[protoWeaponType_].cost);
    }
    canvas->write_cent_l(StringTemp, &detail,
                         static_cast<int>(strlen(StringTemp)));
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00435400 ?draw_armor@DesignWin@@QAEXH@Z 0x00435400-0x00435800
// RECOVERY_BINDING: v1 {"evidence": ["git:fd72c001:src/designwin.cpp preserves original identity and historical seed provenance", "Pinned435400..435800 reads live armor selection, signed defense at rule+8 and unsigned mode at+9", "src/caviar.h:Caviar::render_multi_table_dest and src/game.h:ArmorTransparentColorTables"], "reason": "Render the armor heading, transparent or ordinary preview, localized name and conditional defense detail through genuine source objects; preserve callback reads and native text geometry."}
void DesignWin::draw_armor(int redraw) {
    (void)redraw;
    Buffer *const canvas = static_cast<Buffer *>(this);
    RECT area = armor_panel_rect_;
    canvas->set_clip(&area);
    buffer_.copy(canvas, &area);

    RECT title = area;
    title.bottom = designwin_panel_add(title.top, designwin_panel_font_height(
        font2_.unk_1_, font2_.line_height(), font2_.pixel_height()));
    area.top = title.bottom;
    StringTemp[0] = '\0';
    designwin_panel_append_label(StringTemp, 0x1F8);
    CharUpperA(StringTemp);
    canvas->set_text_color(TextColourTable[156], TextColourTable[157], 1, 1);
    canvas->write_cent_l(&font2_, StringTemp, &title,
                         static_cast<int>(strlen(StringTemp)));

    area.bottom = designwin_panel_subtract(designwin_panel_subtract(
        area.bottom, designwin_panel_add(font1_.pixel_height(), font1_.pixel_height())), 5);
    if (protoArmorType_ != 0) {
        caviar_.reset_objects();
        vehdraw_construct_armor(ConsoleGlobal.faction_id_,
            static_cast<int>(protoChassisType_),
            static_cast<int>(protoWeaponType_),
            static_cast<int>(protoArmorType_), 1, &caviar_);
        caviar_.set_scaling(0.07f);
        caviar_.set_scene_rotation(0.5233333706855774f, 0.0f, 0.0f);
        const int center_x = designwin_panel_add(area.left, area.right) / 2;
        const int center_y = designwin_panel_add(area.top, area.bottom) / 2;
        if (protoArmorType_ == 5) {
            caviar_.render_multi_table_dest(canvas, center_x, center_y, 24,
                reinterpret_cast<unsigned char *>(&ArmorTransparentColorTables), 6);
        } else {
            caviar_.render(canvas, center_x, center_y, 6);
        }
    }

    canvas->set_font(&font1_, 0, 0, 0);
    area.top = area.bottom;
    area.bottom = designwin_panel_add(area.top, font1_.pixel_height());
    canvas->set_text_color(TextColourTable[158], TextColourTable[159], 1, 1);
    StringTemp[0] = '\0';
    if (protoArmorType_ == 0)
        designwin_panel_append_label(StringTemp, 0x64);
    else
        strcat(StringTemp, StringTable.get(Armor[protoArmorType_].name));
    CharUpperA(StringTemp);
    if (font1_.width(StringTemp) > designwin_panel_subtract(area.right, area.left)) {
        const int advance = designwin_panel_font_height(
        font1_.unk_1_, font1_.line_height(), font1_.pixel_height());
        area.top = designwin_panel_subtract(area.top, advance);
        area.bottom = designwin_panel_subtract(area.bottom, advance);
        canvas->wrap_cent(StringTemp, &area);
        const int restored_advance = designwin_panel_font_height(
        font1_.unk_1_, font1_.line_height(), font1_.pixel_height());
        area.top = designwin_panel_add(area.top, restored_advance);
        area.bottom = designwin_panel_add(area.bottom, restored_advance);
    } else {
        canvas->write_cent_l(StringTemp, &area,
                             static_cast<int>(strlen(StringTemp)));
    }

    const int advance = designwin_panel_add(font1_.pixel_height(), 3);
    area.top = designwin_panel_add(area.top, advance);
    area.bottom = designwin_panel_add(area.bottom, advance);
    canvas->set_text_color(TextColourTable[160], TextColourTable[161], 1, 1);
    if (Armor[protoArmorType_].mode < 3) {
        StringTemp[0] = '\0';
        designwin_panel_append_label(StringTemp, 0xA6C);
        strcat(StringTemp, " ");
        const int defense = Armor[protoArmorType_].defense_rating;
        if (defense < 0)
            designwin_panel_append_label(StringTemp, 0x310);
        else
            designwin_panel_append_number(StringTemp, defense);
        canvas->write_cent_l(StringTemp, &area,
                             static_cast<int>(strlen(StringTemp)));
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00435800 ?draw_reactor@DesignWin@@QAEXH@Z 0x00435800-0x00435B29
// RECOVERY_BINDING: v1 {"evidence": ["git:fd72c001:src/designwin.cpp preserves original identity and historical seed provenance", "Pinned435800..435B29 uses1-based reactor selection, ability bit8000 and wrap_cent return as numeric-row top", "src/alpha.cpp reactor loader and src/veh.h:RulesReactor establish interned name pointers and12-byte records"], "reason": "Render the reactor heading, ability-prefixed description, model and live numeric selection through genuine source objects; retain native font reads and wrap-return geometry."}
void DesignWin::draw_reactor(int redraw) {
    (void)redraw;
    Buffer *const canvas = static_cast<Buffer *>(this);
    RECT area = reactor_panel_rect_;
    canvas->set_clip(&area);
    buffer_.copy(canvas, &area);

    RECT title = area;
    title.bottom = designwin_panel_add(title.top, designwin_panel_font_height(
        font2_.unk_1_, font2_.line_height(), font2_.pixel_height()));
    area.top = title.bottom;
    StringTemp[0] = '\0';
    designwin_panel_append_label(StringTemp, 0x13C);
    CharUpperA(StringTemp);
    canvas->set_text_color(TextColourTable[156], TextColourTable[157], 1, 1);
    canvas->write_cent_l(&font2_, StringTemp, &title,
                         static_cast<int>(strlen(StringTemp)));

    canvas->set_font(&font1_, 0, 0, 0);
    StringTemp[0] = '\0';
    if ((protoAbilityFlags_ & 0x8000u) != 0) {
        designwin_panel_append_label(StringTemp, 0x584);
        strcat(StringTemp, " ");
    }
    strcat(StringTemp, StringTable.get(Reactor[protoReactorType_ - 1].name));
    CharUpperA(StringTemp);
    const int description_height = canvas->wrap_height(
        StringTemp, designwin_panel_subtract(area.right, area.left));
    area.bottom = designwin_panel_subtract(designwin_panel_subtract(
        designwin_panel_subtract(area.bottom, 5), description_height), font1_.pixel_height());

    caviar_.reset_objects();
    vehdraw_construct_reactor(ConsoleGlobal.faction_id_,
        static_cast<int>(protoChassisType_),
        static_cast<int>(protoWeaponType_),
        static_cast<int>(protoReactorType_), protoAbilityFlags_, &caviar_);
    caviar_.set_scaling(0.2f);
    caviar_.set_scene_rotation(0.5233333706855774f, 0.0f, 0.0f);
    caviar_.render(canvas, designwin_panel_add(area.left, area.right) / 2,
                   designwin_panel_add(area.top, area.bottom) / 2, 6);

    area.top = area.bottom;
    area.bottom = designwin_panel_add(area.top, font1_.pixel_height());
    canvas->set_text_color(TextColourTable[158], TextColourTable[159], 1, 1);
    area.top = canvas->wrap_cent(StringTemp, &area);
    area.bottom = designwin_panel_add(area.top, font1_.pixel_height());
    canvas->set_text_color(TextColourTable[160], TextColourTable[161], 1, 1);
    StringTemp[0] = '\0';
    designwin_panel_append_label(StringTemp, 0x1E4);
    strcat(StringTemp, " ");
    designwin_panel_append_number(StringTemp, static_cast<int>(protoReactorType_));
    canvas->write_cent_l(StringTemp, &area,
                         static_cast<int>(strlen(StringTemp)));
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00435B30 ?draw_special_1@DesignWin@@QAEXH@Z 0x00435B30-0x00435DD1 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00435b30.cpp", "RulesAbility and StringTable in src/veh.h and src/strings.h", "DesignWin::protoAbilityID1_ and panel/font members in src/designwin.h"], "reason": "Draw the first selected special ability through the typed ability record and the existing DesignWin panel, font and selection state."}
void DesignWin::draw_special_1(int redraw) {
    (void)redraw;
    Buffer *canvas = static_cast<Buffer *>(this);
    RECT area = special_one_panel_rect_;
    canvas->set_clip(&area);
    buffer_.copy(canvas, &area);

    const int title_height = font2_.unk_1_ < 0
        ? font2_.line_height() : font2_.pixel_height() + font2_.unk_1_;
    RECT title = area;
    title.bottom = title.top + title_height;
    canvas->set_text_color(0xE0, -1, 1, 1);
    char *caption = label_get(0x228 / 4);
    if (caption != 0) {
        canvas->write_cent_l(&font2_, caption, &title,
                             static_cast<int>(strlen(caption)));
    }

    RECT text = area;
    text.left += 5;
    text.right -= 5;
    text.top += title_height + 2;
    const int ability = static_cast<int>(protoAbilityID1_);
    if (ability < 0 || ability >= MaxAbilityNum) {
        char *empty = label_get(0x64 / 4);
        if (empty != 0) {
            canvas->write_cent_l(&font1_, empty, &text,
                                 static_cast<int>(strlen(empty)));
        }
        return;
    }

    const RulesAbility &record = Ability[ability];
    char *name = StringTable.get(record.name);
    char *description = StringTable.get(record.description);
    if (name != 0) {
        canvas->set_text_color(0xE0, -1, 1, 1);
        text.top = canvas->wrap_cent(&font2_, name, &text);
    }
    if (description != 0) {
        canvas->set_text_color(0xE0, -1, 1, 1);
        canvas->wrap_cent(&font1_, description, &text);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00435DE0 ?draw_special_2@DesignWin@@QAEXH@Z 0x00435DE0-0x00436081 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00435de0.cpp", "RulesAbility and StringTable in src/veh.h and src/strings.h", "DesignWin::protoAbilityID2_ and panel/font members in src/designwin.h"], "reason": "Draw the second selected special ability through the typed ability record and the existing DesignWin panel, font and selection state."}
void DesignWin::draw_special_2(int redraw) {
    (void)redraw;
    Buffer *canvas = static_cast<Buffer *>(this);
    RECT area = special_two_panel_rect_;
    canvas->set_clip(&area);
    buffer_.copy(canvas, &area);

    const int title_height = font2_.unk_1_ < 0
        ? font2_.line_height() : font2_.pixel_height() + font2_.unk_1_;
    RECT title = area;
    title.bottom = title.top + title_height;
    canvas->set_text_color(0xE0, -1, 1, 1);
    char *caption = label_get(0x228 / 4);
    if (caption != 0) {
        canvas->write_cent_l(&font2_, caption, &title,
                             static_cast<int>(strlen(caption)));
    }

    RECT text = area;
    text.left += 5;
    text.right -= 5;
    text.top += title_height + 2;
    const int ability = static_cast<int>(protoAbilityID2_);
    if (ability < 0 || ability >= MaxAbilityNum) {
        char *empty = label_get(0x64 / 4);
        if (empty != 0) {
            canvas->write_cent_l(&font1_, empty, &text,
                                 static_cast<int>(strlen(empty)));
        }
        return;
    }

    const RulesAbility &record = Ability[ability];
    char *name = StringTable.get(record.name);
    char *description = StringTable.get(record.description);
    if (name != 0) {
        canvas->set_text_color(0xE0, -1, 1, 1);
        text.top = canvas->wrap_cent(&font2_, name, &text);
    }
    if (description != 0) {
        canvas->set_text_color(0xE0, -1, 1, 1);
        canvas->wrap_cent(&font1_, description, &text);
    }
}

// ORIGINAL: 0x00436090 ?draw_vehicle@DesignWin@@QAEXH@Z 0x00436090-0x00437999 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/veh.h:VehPrototype is size 0x34; row 63 begins at 0x009AC534 and fields at 0x20,0x24-0x27,0x2E own the observed stores", "source:src/designwin.cpp:UNK3, draw_vehicle and draw_flash share VehPrototypes[0x3f] preview storage", "source:src/designwin.h:draw_special_2(int), draw_special_1(int), draw_vehicle(int), draw_flash(), draw_info() are declared DesignWin methods", "source:src/veh.cpp:abil_cond(int,int,int,int) is the existing __cdecl definition at 0x00584B50", "historical seed (Git tree 01a84efe; retired from the live source tree): src/unrecovered/00436090.cpp; preserve its original draw_vehicle identity, spans and TRIED lesson"], "reason": "Publish the editor preview scratch through the real 0x34-stride VehPrototype row 63, and close its repaint/ability-validation calls through the declared DesignWin methods and existing cdecl abil_cond owner."}
// TRIED: full transcription (mnemonic_similarity 0.576, 930/1780 original mnemonics in common); two Font structs are embedded back to back at +0x14568 (detail line font) and +0x14590 (name font), each with the compiler inlining `unk1<0 ? line_height_ : height_+unk1` at every call site rather than through a getter - kept as a helper here. First divergence is in the opening RECT-copy prologue (four field reads compiled as one push/lea shuffle in the original vs plain assignment here).
void DesignWin::draw_vehicle(int redraw) {
    (void)redraw;

    Buffer *screen = static_cast<Buffer *>(this);
    Buffer *backing = &buffer_;
    Font *cur_font = &font3_;
    const int prototype_id = static_cast<int>(protoID_);
    const int faction_id = static_cast<int>(factionID_);
    const int chassis_id = static_cast<int>(protoChassisType_);
    const int weapon_id = static_cast<int>(protoWeaponType_);
    const int armor_id = static_cast<int>(protoArmorType_);
    const uint32_t abilities = protoAbilityFlags_;
    const int reactor_id = static_cast<int>(protoReactorType_);

    RECT src = vehicle_panel_rect_;
    screen->set_clip(&src);
    backing->copy(this != 0 ? screen : 0, &src);

    const int chassis_slot = prototype_id;
    field_CE8_ += 1;
    const VehPrototype &selected = VehPrototypes[chassis_slot];
    const bool matches_current = !((selected.icon_offset < 0)
        || chassis_id != selected.chassis_id
        || weapon_id != selected.weapon_id
        || armor_id != selected.armor_id
        || reactor_id != selected.reactor_id
        || abilities != selected.ability_flags);
    const bool needs_construct = !matches_current
        && ((Weapon[weapon_id].icon < 0) || chassis_id != 0);

    int cx = 0;
    int cy = 0;
    if (needs_construct) {
        caviar_.set_scene_rotation(scene_rotation_, 0.0f, 0.0f);
        scene_rotation_ += 0.05f;
        construct_vehicle();
        caviar_.set_scaling(0.3921568691730499267578125f);
        cx = (src.right + src.left) / 2;
        cy = (src.bottom + src.top) / 2;
        caviar_.render(screen, cx, cy, 0x17);
        if (armor_id == 5) {
            caviar_.reset_objects();
            vehdraw_construct_armor(ConsoleGlobal.faction_id_, chassis_id,
                                    weapon_id, armor_id, 1, &caviar_);
            caviar_.render_multi_table_dest(
                screen, cx, cy, 0x18,
                reinterpret_cast<unsigned char *>(&ArmorTransparentColorTables),
                0x22);
        } else {
            caviar_.clear_buffers(0);
        }
    } else {
        VehPrototypes[0x3f].chassis_id = static_cast<uint8_t>(chassis_id);
        VehPrototypes[0x3f].weapon_id = static_cast<uint8_t>(weapon_id);
        VehPrototypes[0x3f].armor_id = static_cast<uint8_t>(armor_id);
        VehPrototypes[0x3f].reactor_id = static_cast<uint8_t>(reactor_id);
        VehPrototypes[0x3f].ability_flags = static_cast<uint32_t>(abilities);
        VehPrototypes[0x3f].icon_offset = selected.icon_offset;
        const int lock = veh_fake(0x3f, faction_id);
        const int pct_w = MapWinCaptionScale < 1 ? 0xbb
            : (MapWinCaptionScale * 100) / MapWinCaptionDivisor;
        const int pct_h = MapWinCaptionScale < 1 ? 0x8c
            : (MapWinCaptionScale * 0x4b) / MapWinCaptionDivisor;
        veh_draw(screen, lock,
                 src.left + ((src.right - pct_w) - src.left) / 2,
                 src.top + ((src.bottom - pct_h) - src.top) / 2,
                 0xe, 0x44, 0);
    }

    screen->set_font(cur_font, 0, 0, 0);
    screen->set_text_color(TextColourTable[140], TextColourTable[141], 1, 1);

    RECT dest;
    int adv = cur_font->unk_1_ < 0
        ? cur_font->line_height() : cur_font->pixel_height() + cur_font->unk_1_;
    dest.top = src.top + adv;
    dest.bottom = src.top + adv + adv * 2;
    dest.left = src.left + 0x2d;
    dest.right = src.right - 0x2d;

    StringTemp[0] = 0;
    if (chassis_slot < 0x40
        && chassis_id == VehPrototypes[chassis_slot].chassis_id
        && weapon_id == VehPrototypes[chassis_slot].weapon_id
        && armor_id == VehPrototypes[chassis_slot].armor_id
        && reactor_id == VehPrototypes[chassis_slot].reactor_id
        && abilities == VehPrototypes[chassis_slot].ability_flags) {
        strcat(StringTemp, VehPrototypes[chassis_slot].veh_name);
    } else {
        const unsigned int name_len = static_cast<unsigned int>(strlen(prototype_name_));
        if (name_len == 0 || (VehPrototypes[prototype_id].flags & 2) == 0) {
            char name_buf[256];
            name_proto(name_buf, prototype_id, faction_id, chassis_id, weapon_id,
                       armor_id, static_cast<int>(abilities), reactor_id);
            strcat(StringTemp, name_buf);
            VehPrototypes[prototype_id].flags &= static_cast<uint16_t>(~2u);
        } else {
            strcat(StringTemp, prototype_name_);
        }
    }
    CharUpperA(StringTemp);
    dest.top = screen->wrap_cent(StringTemp, &dest);

    VehPrototypes[0x3f].chassis_id = static_cast<uint8_t>(chassis_id);
    VehPrototypes[0x3f].weapon_id = static_cast<uint8_t>(weapon_id);
    VehPrototypes[0x3f].armor_id = static_cast<uint8_t>(armor_id);
    VehPrototypes[0x3f].reactor_id = static_cast<uint8_t>(reactor_id);
    VehPrototypes[0x3f].ability_flags = static_cast<uint32_t>(abilities);
    VehPrototypes[0x3f].icon_offset = static_cast<int8_t>(0xff);
    dest.right = src.right;
    dest.left = src.left;
    adv = cur_font->unk_1_ < 0
        ? cur_font->line_height() : cur_font->pixel_height() + cur_font->unk_1_;
    dest.bottom = (src.bottom - adv) + adv * -3;
    dest.top = src.bottom + adv * -3;

    StringTemp[0] = 0;
    say_stats_3(StringTemp, 0x3f);
    char ability_names[256];
    ability_names[0] = 0;
    for (int bit = 0; bit < MaxAbilityNum; ++bit) {
        if (abilities & (1 << (bit & 0x1f))) {
            char *name = StringTable.get(Ability[bit].name);
            if (strlen(name) != 0) {
                if (strlen(ability_names) != 0) strcat(ability_names, ", ");
                strcat(ability_names, StringTable.get(Ability[bit].name));
            }
        }
    }
    strcat(StringTemp, " ");
    strcat(StringTemp, ability_names);
    screen->write_cent_l(StringTemp, &dest, static_cast<int>(strlen(StringTemp)));
    field_B3C_ = dest.left;
    field_B40_ = dest.top;
    field_B44_ = dest.right;
    field_B48_ = dest.bottom;

    if (Weapon[weapon_id].mode == 7) {
        adv = ((cur_font->unk_1_ < 0 ? cur_font->line_height()
                : cur_font->pixel_height() + cur_font->unk_1_) * 3) / 2;
        dest.bottom += adv;
        dest.top += adv;
        screen->set_font(&font2_, 0, 0, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x360 / 4));
        strcat(StringTemp, ": ");
        char numbuf[80];
        _itoa(transport_val(chassis_id, abilities, reactor_id), numbuf, 10);
        strcat(StringTemp, numbuf);
        screen->write_cent_l(StringTemp, &dest, static_cast<int>(strlen(StringTemp)));
    }
    if (Weapon[weapon_id].offense_rating > 1
        && Chassis[chassis_id].triad == 0
        && Chassis[chassis_id].speed == 1
        && Rules.combat_pct_infantry_vs_base != 0
        && (abilities & 0x8000) == 0) {
        adv = ((cur_font->unk_1_ < 0 ? cur_font->line_height()
                : cur_font->pixel_height() + cur_font->unk_1_) * 3) / 2;
        dest.top += adv;
        dest.bottom += adv;
        screen->set_font(&font2_, 0, 0, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, "+");
        char numbuf[80];
        _itoa(Rules.combat_pct_infantry_vs_base, numbuf, 10);
        strcat(StringTemp, numbuf);
        strcat(StringTemp, "% ");
        strcat(StringTemp, label_get(0x88c / 4));
        screen->write_cent_l(StringTemp, &dest, static_cast<int>(strlen(StringTemp)));
    }
    if (Weapon[weapon_id].offense_rating > 0
        && Chassis[chassis_id].triad == 0
        && Chassis[chassis_id].speed > 1
        && Rules.combat_pct_mobile_open_ground != 0
        && (abilities & 0x8000) != 0) {
        adv = ((cur_font->unk_1_ < 0 ? cur_font->line_height()
                : cur_font->pixel_height() + cur_font->unk_1_) * 3) / 2;
        dest.bottom += adv;
        dest.top += adv;
        screen->set_font(&font2_, 0, 0, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, "+");
        char numbuf[80];
        _itoa(Rules.combat_pct_mobile_open_ground, numbuf, 10);
        strcat(StringTemp, numbuf);
        strcat(StringTemp, "% ");
        strcat(StringTemp, label_get(0x98c / 4));
        screen->write_cent_l(StringTemp, &dest, static_cast<int>(strlen(StringTemp)));
    }

    screen->set_text_color2(TextColourTable[142], TextColourTable[143], 1, 1);
    screen->set_text_color(TextColourTable[144], TextColourTable[145], 1, 1);
    screen->set_font(&font2_, 0, 0, 0);
    dest.left = src.left;
    dest.right = src.right;
    dest.top = src.top;
    adv = cur_font->unk_1_ < 0
        ? cur_font->line_height() : cur_font->pixel_height() + cur_font->unk_1_;
    dest.bottom = src.top + adv;

    const int proto_price = static_cast<int>(proto_cost(
        chassis_id, weapon_id, armor_id, static_cast<int>(abilities), reactor_id));
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x160 / 4));
    strcat(StringTemp, ": ");
    strcat(StringTemp, " ");
    strcat(StringTemp, "}");
    int total_cost;
    char numbuf80[80];
    if (proto_price < reactor_id) {
        strcat(StringTemp, "(");
        strcat(StringTemp, label_get(0x1e8 / 4));
        strcat(StringTemp, ": ");
        total_cost = reactor_id * static_cast<int>(mineralCostFactor_);
        _itoa(total_cost, numbuf80, 10);
        strcat(StringTemp, numbuf80);
        strcat(StringTemp, ")");
    } else {
        total_cost = static_cast<int>(mineralCostFactor_) * proto_price;
        _itoa(total_cost, numbuf80, 10);
        strcat(StringTemp, numbuf80);
    }
    if (Chassis[chassis_id].speed > 1
        && Chassis[chassis_id].triad == 0
        && Armor[armor_id].defense_rating > 1
        && Weapon[weapon_id].cost > 1) {
        strcat(StringTemp, "!");
        screen->set_text_color(TextColourTable[46], TextColourTable[47], 1, 1);
        screen->set_text_color2(TextColourTable[46], TextColourTable[47], 1, 1);
    }
    CharUpperA(StringTemp);
    screen->write_cent_l(StringTemp, &dest, static_cast<int>(strlen(StringTemp)));
    field_B2C_rect_ = dest;

    bool custom = (0x3f < prototype_id);
    if (custom) {
        bool armor_ok = false;
        bool reactor_ok = false;
        bool weapon_ok = false;
        int j = 0;
        while (j < 0x80) {
            int jj = j & 0x8000003f;
            if (jj < 0) jj = (jj - 1 | 0xffffffc0) + 1;
            if (jj != j) jj += faction_id * 0x40;
            if (VehPrototypes[jj].flags & 1) {
                bool ok = false;
                if (jj < 0x40) {
                    ok = has_tech(VehPrototypes[jj].preq_tech, faction_id) != 0;
                } else if (VehPrototypes[jj].flags & 4) {
                    ok = true;
                }
                if (ok) {
                    const int j_armor = VehPrototypes[jj].weapon_id;
                    if (weapon_id == j_armor) armor_ok = true;
                    const int j_reactor = VehPrototypes[jj].armor_id;
                    if (armor_id == j_reactor) reactor_ok = true;
                    if (chassis_id == VehPrototypes[jj].chassis_id) weapon_ok = true;
                    if (Weapon[weapon_id].offense_rating > 0
                        && Weapon[weapon_id].offense_rating
                            <= Weapon[j_armor].offense_rating) {
                        armor_ok = true;
                    }
                    if (Armor[armor_id].defense_rating > 0
                        && Armor[armor_id].defense_rating
                            <= Armor[j_reactor].defense_rating) {
                        reactor_ok = true;
                    }
                    const int wj = VehPrototypes[jj].chassis_id;
                    if (Chassis[chassis_id].triad == Chassis[wj].triad
                        && Chassis[chassis_id].speed <= Chassis[wj].speed) {
                        weapon_ok = true;
                    }
                }
            }
            ++j;
        }
        if (armor_ok && reactor_ok && weapon_ok) custom = false;
    }

    const bool already_designed = !((
        (VehPrototypes[chassis_slot].obsolete_factions
         & (1 << (faction_id & 0x1f))) == 0)
        || chassis_id != VehPrototypes[chassis_slot].chassis_id
        || weapon_id != VehPrototypes[chassis_slot].weapon_id
        || armor_id != VehPrototypes[chassis_slot].armor_id
        || reactor_id != VehPrototypes[chassis_slot].reactor_id
        || abilities != VehPrototypes[chassis_slot].ability_flags);

    bool skip_to_name = false;
    if (!already_designed) {
        screen->set_text_color(TextColourTable[148], TextColourTable[149], 1, 1);
        screen->set_text_color2(TextColourTable[146], TextColourTable[147], 1, 1);
        unsigned int slen;
        if (!custom) {
            if (prototype_id < 0x40) {
                skip_to_name = true;
            } else {
                StringTemp[0] = 0;
                strcat(StringTemp, label_get(0x3a4 / 4));
                CharUpperA(StringTemp);
                slen = static_cast<unsigned int>(strlen(StringTemp));
                screen->write_cent_l(&font2_, StringTemp, &dest,
                                     static_cast<int>(slen));
            }
        } else {
            const int base_cost = static_cast<int>(proto_cost(
                chassis_id, weapon_id, armor_id, 0, reactor_id));
            const int factor = prototype_factor(prototype_id);
            total_cost = total_cost + ((base_cost * factor + 0x32) / 100)
                * static_cast<int>(mineralCostFactor_);
            StringTemp[0] = 0;
            strcat(StringTemp, "{");
            strcat(StringTemp, label_get(0x2fc / 4));
            strcat(StringTemp, ": ");
            strcat(StringTemp, " ");
            strcat(StringTemp, "}");
            char numbuf3[80];
            _itoa(total_cost, numbuf3, 10);
            strcat(StringTemp, numbuf3);
            if (Chassis[chassis_id].speed > 1
                && Armor[armor_id].defense_rating > 1
                && Weapon[weapon_id].cost > 1) {
                strcat(StringTemp, "!");
                screen->set_text_color(TextColourTable[46], TextColourTable[47], 1, 1);
                screen->set_text_color2(TextColourTable[46], TextColourTable[47], 1, 1);
            }
            CharUpperA(StringTemp);
            slen = static_cast<unsigned int>(strlen(StringTemp));
            screen->write_cent_l(&font2_, StringTemp, &dest,
                                 static_cast<int>(slen));
        }
    } else {
        screen->set_text_color(TextColourTable[46], TextColourTable[47], 1, 1);
        screen->set_text_color2(TextColourTable[46], TextColourTable[47], 1, 1);
        if ((static_cast<int>(field_CE8_) / 0xc) % 3 != 0) {
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0xe40 / 4));
            CharUpperA(StringTemp);
            const unsigned int slen2 = static_cast<unsigned int>(strlen(StringTemp));
            screen->write_cent_l(&font2_, StringTemp, &dest,
                                 static_cast<int>(slen2));
        }
    }
    if (!skip_to_name) {
        adv = font2_.unk_1_ < 0 ? font2_.line_height()
            : font2_.pixel_height() + font2_.unk_1_;
        src.top += adv;
        src.bottom += adv;
    }

    screen->set_text_color2(0xde, -1, 1, 1);
    screen->set_text_color(TextColourTable[150], TextColourTable[151], 1, 1);
    const int y = src.left + 0x14;
    adv = font2_.unk_1_ < 0 ? font2_.line_height()
        : font2_.pixel_height() + font2_.unk_1_;
    char *veh_name = preset_names_[6];
    int cap_x = (vehicle_panel_rect_.bottom + vehicle_panel_rect_.top) / 2
        + adv * -4;
    CharUpperA(veh_name);
    if (veh_name != 0) {
        const unsigned int slen = static_cast<unsigned int>(strlen(veh_name));
        screen->write_l(&font2_, veh_name, y, cap_x, static_cast<int>(slen));
    }
    adv = font2_.unk_1_ < 0 ? font2_.line_height()
        : font2_.pixel_height() + font2_.unk_1_;
    cap_x += adv;

    screen->set_text_color2(TextColourTable[152], TextColourTable[153], 1, 1);
    screen->set_text_color(TextColourTable[154], TextColourTable[155], 1, 1);
    StringTemp[0] = 0;
    if (Weapon[weapon_id].mode < 3) {
        strcat(StringTemp, "*");
        strcat(StringTemp, preset_names_[8]);
        strcat(StringTemp, "{");
        strcat(StringTemp, " ");
        const signed char techlevel = Weapon[weapon_id].offense_rating;
        char numbuf4[80];
        if (techlevel < 0) {
            strcat(StringTemp, label_get(0x310 / 4));
        } else if (techlevel < 0x63) {
            _itoa(static_cast<int>(techlevel), numbuf4, 10);
            strcat(StringTemp, numbuf4);
        } else {
            strcat(StringTemp, "}");
        }
    } else {
        strcat(StringTemp, "{");
        strcat(StringTemp, preset_names_[14]);
        strcat(StringTemp, "}");
        strcat(StringTemp, " ");
        char numbuf5[80];
        _itoa(static_cast<int>(Weapon[weapon_id].cost), numbuf5, 10);
        strcat(StringTemp, numbuf5);
    }
    unsigned int slen = static_cast<unsigned int>(strlen(StringTemp));
    screen->write_l(StringTemp, y, cap_x, static_cast<int>(slen));
        adv = font2_.unk_1_ < 0 ? font2_.line_height()
            : font2_.pixel_height() + font2_.unk_1_;
    cap_x += adv;

    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, preset_names_[9]);
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    if (Armor[armor_id].mode < 3) {
        if (Armor[armor_id].defense_rating < 0) {
            strcat(StringTemp, label_get(0x310 / 4));
        } else {
            char numbuf6[80];
            _itoa(static_cast<int>(Armor[armor_id].defense_rating), numbuf6, 10);
            strcat(StringTemp, numbuf6);
        }
        slen = static_cast<unsigned int>(strlen(StringTemp));
        screen->write_l(StringTemp, y, cap_x, static_cast<int>(slen));
    }
        adv = font2_.unk_1_ < 0 ? font2_.line_height()
            : font2_.pixel_height() + font2_.unk_1_;
    cap_x += adv;

    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, preset_names_[10]);
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    strcat(StringTemp, StringTable.get(Triad[Chassis[chassis_id].triad]));
    StringTemp[4] = 0;
    strcat(StringTemp, " ");
    char numbuf7[80];
    _itoa(speed_proto(0x3f) / static_cast<int>(Rules.move_rate_roads), numbuf7, 10);
    strcat(StringTemp, numbuf7);
    slen = static_cast<unsigned int>(strlen(StringTemp));
    screen->write_l(StringTemp, y, cap_x, static_cast<int>(slen));
        adv = font2_.unk_1_ < 0 ? font2_.line_height()
            : font2_.pixel_height() + font2_.unk_1_;
    cap_x += adv;

    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, preset_names_[11]);
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    char numbuf8[80];
    _itoa(reactor_id, numbuf8, 10);
    strcat(StringTemp, numbuf8);
    slen = static_cast<unsigned int>(strlen(StringTemp));
    screen->write_l(StringTemp, y, cap_x, static_cast<int>(slen));
    adv = font2_.unk_1_ < 0 ? font2_.line_height()
        : font2_.pixel_height() + font2_.unk_1_;
    cap_x += adv;

    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, preset_names_[12]);
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    if (protoAbilityID1_ < 0) {
        strcat(StringTemp, "-");
    } else {
        ability_names[0] = 0;
        strcat(ability_names, StringTable.get(Ability[protoAbilityID1_].name));
        strcat(StringTemp, ability_names);
        StringTemp[6] = 0;
        char *sp2 = strchr(ability_names, 0x20);
        if (sp2 != 0 && sp2[1] != 0) {
            strcat(StringTemp, sp2 + 1);
            StringTemp[7] = 0;
        }
    }
    slen = static_cast<unsigned int>(strlen(StringTemp));
    screen->write_l(StringTemp, y, cap_x, static_cast<int>(slen));
    adv = font2_.unk_1_ < 0 ? font2_.line_height()
        : font2_.pixel_height() + font2_.unk_1_;

    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, preset_names_[13]);
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    if (protoAbilityID2_ < 0) {
        strcat(StringTemp, "-");
    } else {
        ability_names[0] = 0;
        strcat(ability_names, StringTable.get(Ability[protoAbilityID2_].name));
        strcat(StringTemp, ability_names);
        StringTemp[6] = 0;
        char *sp3 = strchr(ability_names, 0x20);
        if (sp3 != 0 && sp3[1] != 0) {
            strcat(StringTemp, sp3 + 1);
            StringTemp[7] = 0;
        }
    }
    slen = static_cast<unsigned int>(strlen(StringTemp));
    screen->write_l(StringTemp, y, cap_x + adv, static_cast<int>(slen));
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004346A0 ?draw_info@DesignWin@@QAEXXZ 0x004346A0-0x00434BB4;0x00653FFE-0x00654010
// RECOVERY_BINDING: v1 {"evidence": ["task:design-info-owner", "pinned:4346A0..434BB4 primary body and653FFE..654010 Font cleanup/EH span; real stack Font owns normal and exceptional cleanup.", "pinned:MainInterfaceGlobal Buffer receiver7AEC64; actual inheritance selects +444; InterfaceBoxStyles[3]/[0] at78D5A0/78D528.", "pinned:replacement loop348C1..34A2C retains later-wins selected-ID stores and current-category rereads; typed PlayerData active/queue arrays.", "pinned:fallback literals682820/682E9C/68354C/682E98 and USER32 CharUpperA; historical seed git:bb602081^:src/recovered/units/004346a0.cpp."], "reason": "Recover the information-panel renderer and selection scan through genuine DesignWin members, MainInterface Buffer inheritance, typed prototype/faction arrays and automatic Font lifetime; preserve original primary and compiler cleanup spans."}
void DesignWin::draw_info() {
    Font font;
    Buffer &canvas = static_cast<Buffer &>(MainInterfaceGlobal);

    RECT source = info_panel_rect_;
    RECT panel = source;
    canvas.set_clip(&panel);
    font.init(DefaultFontFace, 0x12, 1);

    canvas.box_sprite(&panel, &InterfaceBoxStyles[3]);
    panel.left += 3;
    panel.top += 3;
    panel.right -= 3;
    panel.bottom -= 3;
    canvas.box_sprite(&panel, &InterfaceBoxStyles[0]);
    panel.left += 3;
    panel.top += 3;
    panel.right -= 3;
    panel.bottom -= 3;

    canvas.set_text_color(0xE0, -1, 1, 1);
    canvas.set_font(&font, 0, 0, 0);

    RECT text = panel;
    text.bottom = text.top + font.line_advance();

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xE4C / 4));
    const int length = static_cast<int>(strlen(StringTemp));
    canvas.write_cent_l(StringTemp, &text, length);

    StringTemp[0] = 0;
    const int faction = static_cast<int>(factionID_);
    const int selected = protoID_ <= 0x7FFFFFFFu
        ? static_cast<int>(protoID_)
        : -1 - static_cast<int>(~protoID_);
    const uint32_t faction_bit = 1u << (faction & 0x1F);
    int selected_label_offset = 0xE54 / 4;
    bool use_fallback = false;

    // Native uses a signed <64 gate, so a negative selection also reaches
    // this lookup. Its preceding storage has no proven source object model;
    // preserve the branch, not a silent clamp, and leave that invalid-input
    // behavior unverified rather than claiming the lookup is source-safe.
    if (selected < 0x40) {
        const VehPrototype &selected_prototype = VehPrototypes[selected];
        const bool selected_matches =
            protoChassisType_ == selected_prototype.chassis_id
            && protoWeaponType_ == selected_prototype.weapon_id
            && protoArmorType_ == selected_prototype.armor_id
            && protoReactorType_ == selected_prototype.reactor_id
            && protoAbilityFlags_ == selected_prototype.ability_flags;
        selected_label_offset = selected_matches
            ? 0xE44 / 4 : 0xE54 / 4;
    } else {
        const VehPrototype &selected_prototype = VehPrototypes[selected];
        if ((selected_prototype.flags & PROTO_ACTIVE) == 0) {
            use_fallback = true;
        } else if ((selected_prototype.obsolete_factions & faction_bit) != 0) {
            use_fallback = true;
        } else {
            const bool selected_matches =
                protoChassisType_ == selected_prototype.chassis_id
                && protoWeaponType_ == selected_prototype.weapon_id
                && protoArmorType_ == selected_prototype.armor_id
                && protoReactorType_ == selected_prototype.reactor_id
                && protoAbilityFlags_ == selected_prototype.ability_flags;
            if (selected_matches) {
                selected_label_offset = 0xE50 / 4;
            } else {
                bool found_replacement = false;
                bool inactive_slot_redirect = false;
                const int first = faction * 0x40;
                for (int index = 0; index < 0x40; ++index) {
                    const int candidate_id = first + index;
                    const VehPrototype &candidate = VehPrototypes[candidate_id];
                    if ((candidate.flags & PROTO_ACTIVE) == 0) {
                        protoID_ = static_cast<uint32_t>(candidate_id);
                        inactive_slot_redirect = true;
                        break;
                    }
                    if ((candidate.obsolete_factions & faction_bit) == 0)
                        continue;

                    const int current_selected = protoID_ <= 0x7FFFFFFFu
                        ? static_cast<int>(protoID_)
                        : -1 - static_cast<int>(~protoID_);
                    const int current_category =
                        VehPrototypes[current_selected].unk_1;
                    uint8_t active_count =
                        PlayersData[faction].proto_id_active[candidate_id];
                    if (candidate.unk_1 != current_category && active_count != 0)
                        continue;

                    active_count =
                        PlayersData[faction].proto_id_active[candidate_id];
                    if (active_count != 0) continue;
                    const uint8_t queued_count =
                        PlayersData[faction].proto_id_queue[candidate_id];
                    if (queued_count == active_count) {
                        // The original continues scanning after this store; a
                        // later eligible zero/zero slot may overwrite protoID_.
                        protoID_ = static_cast<uint32_t>(candidate_id);
                        found_replacement = true;
                        continue;
                    }
                    if (active_count != 0 || found_replacement) continue;
                    if (candidate.unk_1 != current_category) continue;
                    protoID_ = static_cast<uint32_t>(candidate_id);
                    found_replacement = true;
                }
                if (found_replacement || inactive_slot_redirect)
                    selected_label_offset = 0xE54 / 4;
                else
                    selected_label_offset = 0xC38 / 4;
            }
        }
    }

    if (use_fallback) {
        strcat(StringTemp, label_get(0xC34 / 4));
        strcat(StringTemp, " ");
        strcat(StringTemp, "(");
        strcat(StringTemp, label_get(0xE48 / 4));
        strcat(StringTemp, " #");

        uint32_t masked_id = protoID_ & 0x8000003Fu;
        if (masked_id & 0x80000000u)
            masked_id = ((masked_id - 1u) | 0xFFFFFFC0u) + 1u;
        const int display_id = masked_id <= 0x7FFFFFFFu
            ? static_cast<int>(masked_id)
            : -1 - static_cast<int>(~masked_id);
        char number[80];
        _itoa(display_id + 1, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, ")");
        CharUpperA(StringTemp);
    } else {
        strcat(StringTemp, label_get(selected_label_offset));
    }

    const int wrapped_height =
        canvas.wrap_height(StringTemp, text.right - text.left);
    const int vertical_shift = (panel.bottom - panel.top) - wrapped_height;
    text.top += vertical_shift;
    text.bottom += vertical_shift;
    canvas.wrap_cent(StringTemp, &text);

    MainInterfaceGlobal.soft_update(&source);
    canvas.set_clip(const_cast<RECT *>(&canvas.current_clip()));
}


// ORIGINAL: 0x00439F20 ?select_chassis@DesignWin@@QAEXXZ 0x00439F20-0x0043A300;0x006541FC-0x00654281
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a:src/recovered/units/00439f20.cpp", "RulesChassis in src/veh.h", "DesignWin component members in src/designwin.h", "SelectPartWin::exec in src/selectpartwin.cpp"], "reason": "Open the chassis picker, translate its eligible-row result into the chassis table, preserve chassis-dependent weapon and ability constraints, and redraw the design preview."}
void DesignWin::select_chassis() {
    SelectPartWin picker;
    const int row = picker_row_before(protoChassisType_, Chassis,
                                      DesignSelectableChassisCount);
    const int selected_row = picker.exec(
        outer_rect_.left, outer_rect_.top, factionID_, 0, row,
        protoChassisType_, protoWeaponType_, protoArmorType_,
        protoAbilityFlags_, protoReactorType_, SpotlightColorTable);
    const int previous_kind = Chassis[protoChassisType_].triad;
    const int next_chassis = picker_record_at(selected_row, Chassis,
                                              DesignSelectableChassisCount);
    protoChassisType_ = next_chassis;

    if (Chassis[next_chassis].missile == 0) {
        protoArmorType_ = 0;
        const int weapon_kind = static_cast<unsigned char>(
            Weapon[protoWeaponType_].offense_rating);
        if (weapon_kind >= 2) protoWeaponType_ = 0;
        if (protoWeaponType_ != 0x10 && protoWeaponType_ != 0x18 &&
            protoWeaponType_ != 0x19) {
            protoWeaponType_ = 0x17;
        }
    } else {
        const int weapon_kind = static_cast<unsigned char>(
            Weapon[protoWeaponType_].offense_rating);
        if (weapon_kind >= 0x63 || protoWeaponType_ == 0x17 ||
            protoWeaponType_ == 0x18 || protoWeaponType_ == 0x19) {
            protoWeaponType_ = 0;
        }
    }

    const int next_kind = Chassis[next_chassis].triad;
    if (next_kind != previous_kind) {
        if (next_kind != 0) {
            suggest_abil(2);
            if (next_kind == 2) {
                if (Weapon[protoWeaponType_].mode == 0x0b) {
                    protoWeaponType_ = 0;
                }
                if (Weapon[protoWeaponType_].mode == 9 &&
                    Chassis[next_chassis].range != 0) {
                    protoWeaponType_ = 0;
                }
            }
        } else if ((protoAbilityFlags_ & 2) != 0) {
            protoAbilityFlags_ &= ~2U;
            protoAbilityID1_ = -1;
            protoAbilityID2_ = -1;
            for (int bit = 0; bit < 0x1d; ++bit) {
                if ((protoAbilityFlags_ & (1U << bit)) == 0) continue;
                if (static_cast<int32_t>(protoAbilityID1_) < 0) {
                    protoAbilityID1_ = bit;
                } else if (static_cast<int32_t>(protoAbilityID2_) < 0) {
                    protoAbilityID2_ = bit;
                }
            }
        }
    }
    redraw_design_picker_changes(this);
}

// ORIGINAL: 0x0043A300 ?select_weapon@DesignWin@@QAEXXZ 0x0043A300-0x0043A754;0x00654281-0x00654306
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a:src/recovered/units/0043a300.cpp", "RulesWeapon/RulesArmor in src/veh.h", "DesignWin component members in src/designwin.h", "SelectPartWin::exec in src/selectpartwin.cpp"], "reason": "Open the weapon picker, apply the selected record's chassis restrictions, update implicit abilities, and redraw the design preview."}
void DesignWin::select_weapon() {
    SelectPartWin picker;
    const int row = picker_row_before(protoWeaponType_, Weapon, MaxWeaponNum);
    const int selected_row = picker.exec(
        outer_rect_.left, outer_rect_.top, factionID_, 1, row,
        protoChassisType_, protoWeaponType_, protoArmorType_,
        protoAbilityFlags_, protoReactorType_, SpotlightColorTable);
    const int previous_weapon = protoWeaponType_;
    const int selected_weapon = picker_record_at(selected_row, Weapon,
                                                 MaxWeaponNum);
    protoWeaponType_ = selected_weapon;
    const int chassis_kind = Chassis[protoChassisType_].triad;

    if (chassis_kind == 2) {
        if (Weapon[selected_weapon].mode == 0x0b) {
            protoWeaponType_ = previous_weapon;
        }
        if (Weapon[protoWeaponType_].mode == 9 &&
            Chassis[protoChassisType_].range != 0) {
            protoWeaponType_ = previous_weapon;
        }
        const int compatibility = Chassis[protoChassisType_].missile;
        if (compatibility != 0 && protoWeaponType_ != 0x10 &&
            protoWeaponType_ != 0x18 && protoWeaponType_ != 0x19) {
            protoWeaponType_ = 0x17;
        } else if (compatibility == 0 &&
                   (protoWeaponType_ == 0x10 || protoWeaponType_ == 0x17 ||
                    protoWeaponType_ == 0x18)) {
            protoWeaponType_ = previous_weapon;
        }
    } else if (selected_weapon == 0x10 || protoWeaponType_ == 0x17 ||
               protoWeaponType_ == 0x18) {
        protoWeaponType_ = previous_weapon;
    }
    if (protoWeaponType_ == 0x19) protoWeaponType_ = previous_weapon;

    if (static_cast<unsigned char>(
            Weapon[protoWeaponType_].offense_rating) > 'b') {
        protoAbilityID1_ = -1;
        protoAbilityID2_ = -1;
        protoAbilityFlags_ = 0;
    }
    if (protoAbilityFlags_ == 0 && protoWeaponType_ != previous_weapon) {
        if (Weapon[protoWeaponType_].mode == 7) suggest_abil(0x80000);
        if (Weapon[protoWeaponType_].mode == 9) {
            suggest_abil(1);
            suggest_abil(0x2000);
        }
        if (Weapon[protoWeaponType_].offense_rating >=
            Armor[protoArmorType_].defense_rating * 2) {
            suggest_abil(0x20000);
        }
    }
    redraw_design_picker_changes(this);
}

// ORIGINAL: 0x0043A760 ?select_armor@DesignWin@@QAEXXZ 0x0043A760-0x0043A9F8;0x00654306-0x0065438B
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a:src/recovered/units/0043a760.cpp", "RulesArmor/RulesWeapon in src/veh.h", "DesignWin component members in src/designwin.h", "SelectPartWin::exec in src/selectpartwin.cpp"], "reason": "Open the armor picker, preserve the armor/weapon compatibility suggestion, and redraw the design preview."}
void DesignWin::select_armor() {
    SelectPartWin picker;
    const int row = picker_row_before(protoArmorType_, Armor, MaxArmorNum);
    const int selected_row = picker.exec(
        outer_rect_.left, outer_rect_.top, factionID_, 2, row,
        protoChassisType_, protoWeaponType_, protoArmorType_,
        protoAbilityFlags_, protoReactorType_, SpotlightColorTable);
    protoArmorType_ = picker_record_at(selected_row, Armor, MaxArmorNum);
    if (Armor[protoArmorType_].defense_rating > 1 &&
        Weapon[protoWeaponType_].offense_rating <= 1) {
        suggest_abil(0x200);
    }
    redraw_design_picker_changes(this);
}

// ORIGINAL: 0x0043AA00 ?select_reactor@DesignWin@@QAEXXZ 0x0043AA00-0x0043AC69;0x0065438B-0x00654410
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a:src/unrecovered/0043aa00.cpp", "RulesReactor in src/veh.h", "DesignWin component members in src/designwin.h", "SelectPartWin::exec in src/selectpartwin.cpp"], "reason": "Open the reactor picker, translate its enabled-row result, update the reactor selection, and redraw the design preview."}
void DesignWin::select_reactor() {
    SelectPartWin picker;
    const int row = picker_row_before(protoReactorType_, Reactor,
                                      MaxReactorNum);
    const int selected_row = picker.exec(
        outer_rect_.left, outer_rect_.top, factionID_, 3, row,
        protoChassisType_, protoWeaponType_, protoArmorType_,
        protoAbilityFlags_, protoReactorType_, SpotlightColorTable);
    protoReactorType_ = picker_record_at(selected_row, Reactor,
                                         MaxReactorNum);
    redraw_design_picker_changes(this);
}

// ORIGINAL: 0x0043D510 ?draw_stack@DesignWin@@QAEXXZ 0x0043D510-0x0043DD43;0x006546DA-0x006546EC FILE
// size      2117 bytes
// prototype void (__thiscall ?draw_stack@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   5   call targets   25
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005594F0 0x0057D7D0 0x005A63D0 0x005B9F20 0x005C1D50 0x005D5930 0x005D8000 0x005DA860 0x005DAC70 0x005DACB0 0x005DCF40 0x005DD130 0x005DDAB0 0x005E2DD3 0x005FA960 0x005FAA90 0x006169A0 0x00618EA0 0x00618EE0 0x00618F40 0x00619280 0x006453E0 0x00645470 0x00645DD0 0x0064FC88
// Homed from src/unrecovered/0043d510.cpp: the prototype stack panel - tile
// the background, frame it, then walk the ranked prototype list drawing up to
// ten entries (box style 2 = the edited prototype, 1 = the highlighted one),
// registering each spot for on_left_click.
// The original word at 0x00952832 is Vehs[0].proto_id: Vehs starts at
// 0x00952828, proto_id is the signed short at +0x0A, and Veh has a 0x34 stride.
// Keep the original index at each consumer: unit_id for the stack row, shown
// for each of the two preview reads.
void DesignWin::draw_stack() {
    Font font;
    RECT area = field_B0C_rect_;
    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    canvas->restore_clip();
    canvas->tile(InterfaceBoxStyles[9].mask_, area.left, area.top,
                 area.left, area.top,
                 area.right - area.left, area.bottom - area.top);
    canvas->box_sprite(&area, &InterfaceBoxStyles[3]);

    RECT rc = area;
    rc.bottom -= 3;
    rc.right = static_cast<int>(field_CCC_) - 3;
    int span = rc.right - (static_cast<int>(field_CBC_rect_.right) + 3) + 2;
    int spacer = span / 10 - 2;
    int x_base = static_cast<int>(field_CBC_rect_.right) + 3;
    rc.left = x_base;
    int y_base = rc.top + 3;
    rc.top = y_base;

    font.init(DefaultFontFace, 10, 0);
    canvas->set_text_color(0x9F, -1, 1, 1);
    canvas->set_font(&font, 0, 0, 0);
    spot1_.kill_type(0);

    rc.bottom = rc.bottom - rc.top + y_base;
    rc.left = x_base;
    rc.top = y_base;
    rc.right = x_base + spacer;

    // Re-rank through the source-owned map helper and its shared sort state.
    proto_sort_2(factionID_);

    int divisor = ProtoSortCount;
    int entry = static_cast<int>(field_CE4_);
    int loop_end = entry + divisor;
    int drawn = 0;
    while (drawn < 10 && entry < loop_end) {
        int proto_id = ProtoSortIds[entry % divisor];
        ++entry;
        // Validity: own faction prototypes need their tech and an offerable
        // weapon class (and never slot 0xF); borrowed ones only their faction.
        if (proto_id / 64 == 0) {
            if (!has_tech(VehPrototypes[proto_id].preq_tech, factionID_)) {
                continue;
            }
            if (proto_id < 0x40) {
                if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating < 0) {
                    continue;
                }
                if (proto_id == 0xF) {
                    continue;
                }
            }
        } else if (proto_id / 64 != factionID_) {
            continue;
        }

        int selector = 0;
        if (proto_id == static_cast<int>(protoID_)) {
            selector = 2;
        } else if (proto_id == static_cast<int>(field_CE0_)) {
            selector = 1;
        }
        ++drawn;
        canvas->box_sprite(&rc, &InterfaceBoxStyles[selector]);
        spot1_.add(proto_id, 0, rc.left, rc.top, rc.right - rc.left,
                   rc.bottom - rc.top);
        rc.left += 2;
        rc.top += 2;
        rc.right -= 2;
        rc.bottom -= 2;
        canvas->set_clip(&rc);

        if (VehPrototypes[proto_id].flags & PROTO_ACTIVE) {
            int unit_id = veh_fake(proto_id, factionID_);
            StringTemp[0] = '\0';
            veh_draw(canvas, unit_id, x_base + 0xF, y_base + 0xC, -10, 0x1048, 0);

            RECT label_rect = rc;
            canvas->set_text_color(0x9F, -1, 1, 1);
            StringTemp[0] = '\0';
            if (proto_id < 0x40) {
                if (VehPrototypes[proto_id].obsolete_factions
                    & (1 << (factionID_ & 0x1F))) {
                    canvas->set_text_color(0xF8, -1, 1, 1);
                    // The label id the interface keeps in the game block at
                    // Labels.strings_ptr+0xE40 (slot 0x390) - same walk as
                    // on_iface_right_click's 0xCE8.
                    strcat(StringTemp, StringTable.get(
                        Labels.strings_ptr[0x390]));
                } else {
                    strcat(StringTemp, VehPrototypes[proto_id].veh_name);
                }
            } else if (VehPrototypes[proto_id].obsolete_factions
                & (1 << (factionID_ & 0x1F))) {
                unsigned char seen =
                    PlayersData[factionID_].proto_id_active[proto_id];
                canvas->set_text_color(0xF8 + ((-(seen != 0)) & 7), -1, 1, 1);
                strcat(StringTemp, StringTable.get(
                    Labels.strings_ptr[0x390]));
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, "(");
                char digits[0x20];
                _itoa(proto_id % 64 + 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, ")");
            } else {
                char digits[0x20];
                _itoa(proto_id % 64 + 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, ". ");
                strcat(StringTemp, VehPrototypes[proto_id].veh_name);
            }

            label_rect.left += 3;
            label_rect.right -= 3;
            if (StringTemp[0] != '\0') {
                while (font.width(StringTemp)
                       > label_rect.right - label_rect.left) {
                    StringTemp[strlen(StringTemp) - 1] = '\0';
                    if (StringTemp[0] == '\0') {
                        break;
                    }
                }
            }
            char *first_space = strchr(StringTemp, ' ');
            if (first_space != 0) {
                char *second_space = strchr(first_space + 1, ' ');
                if (second_space != 0) {
                    *second_space = '\0';
                }
            }
            label_rect.bottom = label_rect.top + font.line_advance();
            canvas->set_clip(&label_rect);
            canvas->write_l(StringTemp, &label_rect, strlen(StringTemp));
            canvas->restore_clip();

            RECT stats_rect = rc;
            stats_rect.top = rc.bottom - font.line_advance();
            StringTemp[0] = '\0';
            say_stats_3(StringTemp, Vehs[unit_id].proto_id);
            const uint32_t game_state =
                GameDataStateStorage.fields_.game_state_;
            if ((game_state & 0x40) != 0 && (game_state & 0x1000) != 0) {
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, "(");
                char digits[0x20];
                _itoa(proto_id, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, ")");
            }
            canvas->write_cent_l(StringTemp, &stats_rect, strlen(StringTemp));
            spot1_.add(proto_id, 0, rc.left, rc.top, rc.right - rc.left,
                       rc.bottom - rc.top);

            x_base += spacer + 2;
            rc.top -= 2;
            rc.bottom += 2;
            rc.left += spacer;
            rc.right += spacer + 4;
        } else {
            StringTemp[0] = '\0';
            // Slot 0x6E of the same label block: the "undesigned" caption.
            strcat(StringTemp, StringTable.get(
                    Labels.strings_ptr[0x6E]));
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, "#");
            char digits[0x20];
            _itoa(proto_id % 64 + 1, digits, 10);
            strcat(StringTemp, digits);
            canvas->set_text_color(0x9F, -1, 1, 1);
            canvas->set_font(&font, 0, 0, 0);
            canvas->wrap_cent(StringTemp, &rc);

            x_base += spacer + 2;
            rc.top -= 2;
            rc.bottom += 2;
            rc.left += spacer;
            rc.right += spacer + 4;
            canvas->restore_clip();
        }
    }

    MainInterfaceGlobal.soft_update(&area);
    canvas->restore_clip();
}


// ORIGINAL: 0x00437BF0 ?design_it@DesignWin@@QAEHXZ 0x00437BF0-0x00438455 FILE
// size      2149 bytes
// prototype int (__thiscall ?design_it@DesignWin@@QAEHXZ)(DesignWin* this)
// callers   1   call targets   17
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0048C0A0 0x00531300 0x00539820 0x005398C0 0x0057E040 0x0057F1D0 0x0057F4B0 0x00592EE0 0x005933F0 0x005A5D40 0x005B9F20 0x005BF330 0x00625E30 0x00625EC0 0x006453E0 0x00645460 0x00645660
// Homed from src/unrecovered/00437bf0.cpp: commit the editor selection as a
// prototype. The scan covers the faction's own 64 slots plus the 64 borrowed
// ones; a slot that already holds exactly this design is re-selected (and
// un-obsoleted), otherwise the current slot is rebuilt, a free one is taken,
// or an occupied one is overwritten behind its confirmation popup.
int DesignWin::design_it() {
    // The vehicle owner supplies these cdecl APIs for naming, retirement,
    // and pruning; calls below use their source-owned declarations.
    char *name = prototype_name_;

    // ---- scan all 128 slots (own 64 + up to 64 borrowed) ------------------
    int scan = 0;
    do {
        int slot = scan;
        if (slot > 0x3F) {
            slot = factionID_ * 0x40 + slot % 0x40;
        }
        VehPrototype &proto = VehPrototypes[slot];
        bool fields_match = (proto.flags & PROTO_ACTIVE) != 0
            && protoChassisType_ == proto.chassis_id
            && protoWeaponType_ == proto.weapon_id
            && protoArmorType_ == proto.armor_id
            && protoReactorType_ == proto.reactor_id
            && protoAbilityFlags_ == proto.ability_flags
            && (slot > 0x3F || has_tech(proto.preq_tech, factionID_));

        if (fields_match) {
            if (slot != static_cast<int>(protoID_)) {
                if (proto.obsolete_factions & (1 << (factionID_ & 0x1F))) {
                    // Flag bit 0x8 has no name in veh.h's enum yet: it marks
                    // a prototype the faction re-designed over.
                    proto.flags |= 8;
                }
                proto.obsolete_factions &= ~(1 << (factionID_ & 0x1F));
            }
            if (slot < 0x40) {
                if (strlen(name) != 0
                    && strcmp(name, proto.veh_name) != 0) {
                    parse_says(0, proto.veh_name, -1, -1);
                    if ((Players[factionID_].rule_flags & RFLAG_ALIEN) == 0) {
                        popp(ScriptTxtID, "INTRINSICNAME", 0,
                             "unit_sm.pcx", pop_wait);
                    } else {
                        popp(ScriptTxtID, "INTRINSICNAME", 0,
                             "alunit_sm.pcx", pop_wait);
                    }
                }
            } else if (strlen(name) != 0
                && (VehPrototypes[protoID_].flags & PROTO_CUSTOM_NAME_SET)) {
                proto.flags |= PROTO_CUSTOM_NAME_SET;
                strcpy(proto.veh_name, name);
            }
            synch_proto(slot);
            protoID_ = slot;
            return 1;
        }
        ++scan;
    } while (scan < 0x80);

    // ---- no exact match: fall back to the currently-selected slot ---------
    if (protoID_ < 0x40) {
        const VehPrototype &current = VehPrototypes[protoID_];
        bool current_match = protoChassisType_ == current.chassis_id
            && protoWeaponType_ == current.weapon_id
            && protoArmorType_ == current.armor_id
            && protoAbilityFlags_ == current.ability_flags
            && protoReactorType_ == current.reactor_id;
        if (current_match) {
            if (strlen(name) == 0) {
                return 0;
            }
            if (strcmp(name, current.veh_name) == 0) {
                return 0;
            }
            parse_says(0, current.veh_name, -1, -1);
            if ((Players[factionID_].rule_flags & RFLAG_ALIEN) == 0) {
                X_pop("INTRINSICNAME", "unit_sm.pcx", pop_wait);
            } else {
                X_pop("INTRINSICNAME", "alunit_sm.pcx",
                                    pop_wait);
            }
            return 0;
        }

        // The components changed under this slot: take the first own slot
        // this faction marked obsolete that carries nothing live, else the
        // last inactive one, and ask before its type is overwritten.
        int own = factionID_ * 0x40;
        const uint8_t mask = static_cast<uint8_t>(1u << (factionID_ & 0x1F));
        int checked = 0;
        do {
            const VehPrototype &candidate = VehPrototypes[own];
            if ((candidate.obsolete_factions & mask) != 0
                && PlayersData[factionID_].proto_id_active[own] == 0
                && PlayersData[factionID_].proto_id_queue[own] == 0) {
                protoID_ = own;
                break;
            }
            if ((candidate.flags & PROTO_ACTIVE) == 0) {
                protoID_ = own;
            }
            ++checked;
            ++own;
        } while (checked < 0x40);

        if (protoID_ < 0x40) {
            parse_says(0, VehPrototypes[protoID_].veh_name, -1, -1);
            if ((Players[factionID_].rule_flags & RFLAG_ALIEN) == 0) {
                popp(ScriptTxtID, "INTRINSICTYPE", 0,
                     "unit_sm.pcx", pop_wait);
            } else {
                popp(ScriptTxtID, "INTRINSICTYPE", 0,
                     "alunit_sm.pcx", pop_wait);
            }
            return 0;
        }
    }

    // ---- the settled slot holds a DIFFERENT design: search for the best
    //      replacement and, if needed, confirm overwriting it --------------
    if ((VehPrototypes[protoID_].flags & PROTO_ACTIVE) != 0
        && (protoChassisType_ != VehPrototypes[protoID_].chassis_id
            || protoWeaponType_ != VehPrototypes[protoID_].weapon_id
            || protoArmorType_ != VehPrototypes[protoID_].armor_id
            || protoAbilityFlags_ != VehPrototypes[protoID_].ability_flags
            || protoReactorType_ != VehPrototypes[protoID_].reactor_id)) {
        int search = factionID_ * 0x40;
        const int8_t current_category =
            VehPrototypes[protoID_].unk_1;
        const uint8_t mask = static_cast<uint8_t>(1u << (factionID_ & 0x1F));
        int replaced = 0;
        int looked = 0;
        do {
            const VehPrototype &candidate = VehPrototypes[search];
            if ((candidate.flags & PROTO_ACTIVE) == 0) {
                protoID_ = search;
                goto settled;
            }
            if ((candidate.obsolete_factions & mask) != 0
                && (candidate.unk_1 == current_category
                    || PlayersData[factionID_].proto_id_active[search] == 0)
                && PlayersData[factionID_].proto_id_active[search] == 0) {
                if (PlayersData[factionID_].proto_id_queue[search] == 0) {
                    protoID_ = search;
                    replaced = 1;
                } else if (replaced == 0 && candidate.unk_1 == current_category) {
                    protoID_ = search;
                }
            }
            ++looked;
            ++search;
        } while (looked < 0x40);

        if (replaced == 0) {
            if (PlayersData[factionID_].proto_id_active[protoID_] != 0
                && (VehPrototypes[protoID_].flags & PROTO_ACTIVE) != 0) {
                parse_says(0, VehPrototypes[protoID_].veh_name, -1, -1);
                parse_num(0, PlayersData[factionID_].proto_id_active[protoID_]);
                char *label;
                char *sprite;
                if ((Players[factionID_].rule_flags & RFLAG_ALIEN) == 0) {
                    label = "OVERWRITEPROTO";
                    sprite = "unit_sm.pcx";
                } else {
                    label = "OVERWRITEPROTO";
                    sprite = "alunit_sm.pcx";
                }
                if (popp(ScriptTxtID, label, 0, sprite, pop_wait) == 0) {
                    return 0;
                }
                if (!IsMultiplayerNet) {
                    retire_proto(static_cast<int>(protoID_), static_cast<int>(factionID_));
                } else {
                    if (diplo_lock(0x32) != 0) {
                        return 0;
                    }
                    message_data(0x2415, 0, protoID_, factionID_, 0, 0);
                    NetDaemonNet.await_exec(0);
                    diplo_unlock();
                }
            }
        }
    }

settled:
    make_proto(protoID_, protoChassisType_, protoWeaponType_, protoArmorType_,
               protoAbilityFlags_, protoReactorType_);
    VehPrototypes[protoID_].flags &= 0x3F;
    if (strlen(name) == 0) {
        // No name typed: generate one straight into the name buffer and trim
        // it to the field's 31 characters.
        name_proto(name, static_cast<int>(protoID_), static_cast<int>(factionID_),
                   static_cast<int>(protoChassisType_),
                   static_cast<int>(protoWeaponType_),
                   static_cast<int>(protoArmorType_),
                   static_cast<int>(protoAbilityFlags_),
                   static_cast<int>(protoReactorType_));
        name[0x1F] = '\0';
        VehPrototypes[protoID_].flags &= 0xFD;
    } else {
        char generated[256];
        name_proto(generated, static_cast<int>(protoID_),
                   static_cast<int>(factionID_),
                   static_cast<int>(protoChassisType_),
                   static_cast<int>(protoWeaponType_),
                   static_cast<int>(protoArmorType_),
                   static_cast<int>(protoAbilityFlags_),
                   static_cast<int>(protoReactorType_));
        if (strcmp(generated, name) != 0) {
            VehPrototypes[protoID_].flags |= PROTO_CUSTOM_NAME_SET;
        }
    }
    strcpy(VehPrototypes[protoID_].veh_name, name);
    synch_proto(protoID_);
    prune_protos(static_cast<int>(factionID_), static_cast<int>(protoID_), 1);
    return 1;
}



// ORIGINAL: 0x0043DD50 ?draw_flash@DesignWin@@QAEXXZ 0x0043DD50-0x0043E486;0x006546EC-0x00654709 FILE
// size      1875 bytes
// prototype void (__thiscall ?draw_flash@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   19
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005A63D0 0x005B9F20 0x005D8000 0x005E2DD3 0x005E3820 0x005DAC70 0x005DACB0 0x005DD130 0x005DDAB0 0x00618EA0 0x00618EE0 0x00618F40 0x00619280 0x006453E0 0x00645470 0x00645DD0 0x0064FC88
// Homed from src/unrecovered/0043dd50.cpp: publish the editor selection into
// the vehdraw stat block, then draw the preview panel - the framed box, the
// spinning unit, its generated or typed name, and its stat line. Ends by
// driving draw_stack, whose panel sits under this one.
void DesignWin::draw_flash() {
    char *name = prototype_name_;

    VehPrototypes[0x3f].chassis_id = static_cast<uint8_t>(protoChassisType_);
    VehPrototypes[0x3f].weapon_id = static_cast<uint8_t>(protoWeaponType_);
    VehPrototypes[0x3f].armor_id = static_cast<uint8_t>(protoArmorType_);
    VehPrototypes[0x3f].reactor_id = static_cast<uint8_t>(protoReactorType_);
    VehPrototypes[0x3f].ability_flags = static_cast<uint32_t>(protoAbilityFlags_);
    VehPrototypes[0x3f].icon_offset = static_cast<int8_t>(0xFF);

    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    int shown = static_cast<int>(field_CE0_);
    if (shown == 0x3F
        && protoChassisType_ == VehPrototypes[protoID_].chassis_id
        && protoWeaponType_ == VehPrototypes[protoID_].weapon_id
        && protoArmorType_ == VehPrototypes[protoID_].armor_id
        && protoReactorType_ == VehPrototypes[protoID_].reactor_id
        && protoAbilityFlags_ == VehPrototypes[protoID_].ability_flags) {
        shown = static_cast<int>(protoID_);
    }

    canvas->set_clip(&preview_panel_rect_);
    RECT rc = preview_panel_rect_;
    canvas->box_sprite(&rc, &InterfaceBoxStyles[3]);

    rc.left += 3;
    rc.right -= 3;
    rc.top += 3;
    rc.bottom -= 3;
    canvas->box_sprite(&rc, &InterfaceBoxStyles[0]);

    rc.left += 3;
    rc.right -= 3;
    rc.top += 3;
    rc.bottom -= 3;

    // The entry under the cursor resolves to a drawable unit only while its
    // prototype is active, with its tech owned for own-faction entries.
    bool drawable = true;
    if (shown != 0x3F) {
        if ((VehPrototypes[shown].flags & PROTO_ACTIVE) == 0) {
            drawable = false;
        } else if (shown >> 6 == 0
            && !has_tech(VehPrototypes[shown].preq_tech, factionID_)) {
            drawable = false;
        }
    }
    if (shown < 0) {
        drawable = false;
    }

    if (drawable) {
        Font title_font;
        Font stat_font;
        int face_size = (WinScreenWidth == 0x320) ? 10 : 12;
        stat_font.init(DefaultFontFace, face_size, 0);
        title_font.init(DefaultFontFace, 12, 1);

        int top = rc.top;
        int left = rc.left;
        int right = (rc.right - left + 3) / 2 + left - 3;
        int centre_x = (right + left) / 2;
        int bottom = rc.bottom;
        int centre_y = (top + bottom) / 2;

        int unit_id = veh_fake(shown, factionID_);
        veh_draw(canvas, unit_id, centre_x - 0x32, centre_y - 0x25, 0, 0x48, 0);

        rc.right = right + (right - left + 3);
        rc.top = top;
        rc.left = left + (right - left + 3);
        rc.bottom = bottom;
        rc.top = top + 3;

        StringTemp[0] = '\0';
        if (shown == 0x3F) {
            if (strlen(name) == 0) {
                name_proto(StringTemp, 0x3F, static_cast<int>(factionID_),
                           static_cast<int>(protoChassisType_),
                           static_cast<int>(protoWeaponType_),
                           static_cast<int>(protoArmorType_),
                           static_cast<int>(protoAbilityFlags_),
                           static_cast<int>(protoReactorType_));
            } else {
                strcat(StringTemp, name);
            }
        } else {
            int prototype_id = Vehs[shown].proto_id;
            strcat(StringTemp, VehPrototypes[prototype_id].veh_name);
        }
        CharUpperA(StringTemp);

        canvas->set_text_color(TextColourTable[82], TextColourTable[83],
                               1, 1);
        canvas->set_font(&title_font, 0, 0, 0);

        int wrap_height = canvas->wrap_cent(StringTemp, &rc);
        rc.right -= 2;
        rc.top = wrap_height + 2;
        rc.left += 2;
        rc.bottom -= 2;
        rc.bottom -= stat_font.line_advance();
        canvas->set_font(&stat_font, 0, 0, 0);

        canvas->set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);

        int line_top = rc.top;
        int line_left = rc.left;

        StringTemp[0] = '\0';
        strcat(StringTemp, "(");
        int prototype_id = Vehs[shown].proto_id;
        say_stats_3(StringTemp, prototype_id);
        int ability_bit = 0;
        for (; ability_bit < MaxAbilityNum; ++ability_bit) {
            if (VehPrototypes[prototype_id].ability_flags & (1 << ability_bit)) {
                if (strlen(StringTable.get(Ability[ability_bit].abbreviation)) != 0) {
                    strcat(StringTemp, ", ");
                    strcat(StringTemp,
                           StringTable.get(Ability[ability_bit].abbreviation));
                }
            }
        }
        strcat(StringTemp, ")");
        line_top = canvas->wrap_cent(StringTemp, line_left, line_top,
                                     rc.right - rc.left);

        if (shown != 0x3F) {
            char digits[0x20];
            StringTemp[0] = '\0';
            _itoa(PlayersData[factionID_].proto_id_active[shown], digits, 10);
            strcat(StringTemp, digits);
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp,
                   StringTable.get(
                       Labels.strings_ptr[0x3A0]));
            line_top = canvas->wrap_cent(StringTemp, line_left, line_top,
                                         rc.right - rc.left);

            StringTemp[0] = '\0';
            _itoa(PlayersData[factionID_].proto_id_queue[shown], digits, 10);
            strcat(StringTemp, digits);
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp,
                   StringTable.get(
                       Labels.strings_ptr[0x3A1]));
            line_top = canvas->wrap_cent(StringTemp, line_left, line_top,
                                         rc.right - rc.left);

            if (VehPrototypes[shown].obsolete_factions
                & (1 << (factionID_ & 0x1F))) {
                StringTemp[0] = '\0';
                strcat(StringTemp, StringTable.get(
                    Labels.strings_ptr[0x390]));
                CharUpperA(StringTemp);
                canvas->wrap_cent(StringTemp, line_left, line_top,
                                  rc.right - rc.left);
            }
        }
    }

    MainInterfaceGlobal.soft_update(&preview_panel_rect_);
    canvas->restore_clip();
    draw_stack();
}

/*
Purpose: The unit-special ability picker both select_special_N thunks tail
         into: gate the popup on the faction's tech state and chassis kind,
         list every special ability the prototype may take (tech-gated,
         condition-gated, not already built, not already picked), exec it,
         store the pick into the prototype's two ability slots, rebuild the
         ability flag word, and repaint the panels.

// ORIGINAL: 0x0043AC70 ?select_special@DesignWin@@QAEXH@Z 0x0043AC70-0x0043BDE9;0x00654410-0x006546C5
// RECOVERY_BINDING: v1 {"evidence": ["pinned:prolog 0x43AC7B registers handler 0x6546BB; handler selects FuncInfo 0x674980 and unwind map 0x6749A0 with 64 action rows.", "pinned:HIGHLOW-backed action pointers and complete action/handler decoding establish the cold region 0x654410..0x6546C5; its former six-byte prefix omitted the first tail jump and later cleanup paths."], "reason": "The original method owns its complete borrowed-frame exception-cleanup region; these actions are generated lifetime code, not independent callable C++ functions. Exact generated correspondence remains an obligation."}
// size      5166 bytes
// prototype void (__thiscall ?select_special@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   3   call targets   34
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401060 0x00402970 0x004048A0 0x00404900 0x004064D0 0x004066C0 0x00406880 0x00406910 0x00406E60 0x004346A0 0x00435B30 0x00435DE0 0x00436090 0x0043DD50 0x0043EF70 0x0048C0A0 0x00584B50 0x005B9F20 0x005D45B0 0x005D4DD0 0x005E3820 0x005FA870 0x00600F00 0x00602600 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00625E50 0x00645470 0x00645550 0x0064FC88
// TRIED (seed): transcribed the full control flow with the Popup/BasePop/
//         Dialogs layouts; roughly 85-90% of the call sequence by count. The
//         VC6 /GX SEH frame (fs:[0], the unwind-state byte, the funclet table
//         at 0x654410+) is compiler-synthesized plumbing outside the
//         function's own logic; the Dialogs item()-vs-dtor 0x188 adjustment
//         is not modelled; the two 0x40-byte helper objects were raw buffers.
//         The real locals below give the compiler the whole cleanup chain:
//         the two helpers ARE StringLists (0x004066C0 is ??1StringList, see
//         stringstruct.cpp), and the Dialogs/Spot/StringStruct destructors
//         the seed open-coded are the compiler's at every exit.
// TRIED (forwarder): the shared body both byte-exact select_special_N thunks
//         tail into - the definition lived in pending_bodies.cpp until this
//         transcription landed it at the end of this file so no recovered
//         body above shifts.
// HISTORICAL NOTES (before v4 homing): the four panel repaints after a successful pick (0x00435DE0
//         draw_special_2, 0x00435B30 draw_special_1, 0x00436090
//         draw_vehicle, 0x004346A0 draw_info) are unhomed, so they go
//         through the original's addresses; abil_cond (0x00584B50) is
//         unhomed the same way. The trailing slot-63 virtual is Win's
//         on_update.
// v4 resolves those historical seams through existing typed methods and
// the existing __cdecl abil_cond owner while preserving call order.
Status: Coverage transcription - unclaimed body
*/
void DesignWin::select_special(int which) {
    Popup popup;

    const int faction = static_cast<int>(factionID_);
    // Player::rule_flags is at 0x508, the original 0x946F58 byte.
    if ((Players[faction].rule_flags & RFLAG_ALIEN) != 0) {
        if (which != 0) {
            const int tech_id = static_cast<int>(Rules.tech_two_spec_abilities);
            if (!has_tech(tech_id, faction)) {
                parse_say(0, Technology[tech_id].name, -1, -1);
                popp(ScriptTxtID, "TWOSPECIAL", 0,
                     "alunit_sm.pcx", pop_wait);
                return;
            }
        }
        if (Chassis[protoChassisType_].missile != 0) {
            popp(ScriptTxtID, "MISSILESPECIAL", 0,
                 "alunit_sm.pcx", pop_wait);
            return;
        }
    } else {
        if (which != 0) {
            const int tech_id = static_cast<int>(Rules.tech_two_spec_abilities);
            if (!has_tech(tech_id, faction)) {
                parse_say(0, Technology[tech_id].name, -1, -1);
                popp(ScriptTxtID, "TWOSPECIAL", 0,
                     "unit_sm.pcx", pop_wait);
                return;
            }
        }
        if (Chassis[protoChassisType_].missile != 0) {
            popp(ScriptTxtID, "MISSILESPECIAL", 0,
                 "unit_sm.pcx", pop_wait);
            return;
        }
    }

    popup.start(PopupStartCaption, "UNITSPECIAL", -1, nullptr,
                0x200042);

    // Original item receivers are Popup+0x21D0. The apparent Scroll,
    // Spot and list cleanup locals in the transcription are this Popup's
    // members; its destructor owns their original reverse teardown.
    Dialogs &dialogs = popup.dialogs();

    StringTemp[0] = 0;
    strcat(StringTemp,
           StringTable.get(Labels.strings_ptr[0x88]));
    dialogs.item(StringTemp, 0x1D);

    int items_added = 1;
    // The original walks 29 Ability records at their 0x1C stride.
    for (int ability = 0; ability < MaxAbilityNum; ++ability) {
        const int16_t preq_tech = Ability[ability].preq_tech;
        if (!has_tech(preq_tech, faction)) {
            continue;
        }
        if (abil_cond(ability, static_cast<int>(protoChassisType_),
                      static_cast<int>(protoWeaponType_),
                      static_cast<int>(protoArmorType_)) == 0) {
            continue;
        }

        // The historical paired scans start at faction_bonus_id/val1 and
        // inspect 0x29 dwords, beyond the declared eight-entry arrays.
        int already_built = 0;
        {
            const uint8_t *const player_bytes =
                reinterpret_cast<const uint8_t *>(&Players);
            const int *const built_kind = reinterpret_cast<const int *>(
                player_bytes + sizeof(Player) * faction
                    + offsetof(Player, faction_bonus_id));
            const int *const built_id = reinterpret_cast<const int *>(
                player_bytes + sizeof(Player) * faction
                    + offsetof(Player, faction_bonus_val1));
            for (int slot = 0; slot < 0x29; ++slot) {
                if (built_kind[slot] == 0xF && built_id[slot] == ability) {
                    already_built = 1;
                    // Pinned 0x0043AF9E exits this scan on its first match.
                    break;
                }
            }
        }
        if (already_built != 0) {
            continue;
        }
        int already_selected = 0;
        for (int slot = 0; slot < 2; ++slot) {
            const uint32_t selected = slot == 0 ? protoAbilityID1_
                                                 : protoAbilityID2_;
            if (slot != which && selected == static_cast<uint32_t>(ability)) {
                already_selected = 1;
            }
        }
        if (already_selected != 0) {
            continue;
        }

        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(Ability[ability].name));
        strcat(StringTemp, ": ");
        strcat(StringTemp, StringTable.get(Ability[ability].description));
        strcat(StringTemp, "|");
        // The image reads Labels.strings_ptr + 0x160 bytes: dword entry 0x58.
        strcat(StringTemp, StringTable.get(
            Labels.strings_ptr[0x58]));
        strcat(StringTemp, ": ");

        const int kind = static_cast<int>(Ability[ability].cost_factor);
        if (kind >= 0) {
            char digits[0x20];
            int cost = kind;
            const bool special_cost =
                (Ability[ability].flags & 0x400) != 0;
            if (special_cost && Chassis[protoChassisType_].triad == 0) {
                ++cost;
            }
            _itoa(cost, digits, 10);
            strcat(StringTemp, digits);
            if (special_cost) {
                const unsigned char triad =
                    Chassis[protoChassisType_].triad;
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, "(");
                strcat(StringTemp, StringTable.get(Triad[triad]));
                strcat(StringTemp, ")");
            }
        } else if (kind >= -7) {
            const unsigned char weapon_cost =
                Weapon[protoWeaponType_].cost;
            const unsigned char armor_cost = Armor[protoArmorType_].cost;
            const unsigned char chassis_speed =
                Chassis[protoChassisType_].speed;
            char digits[0x20];
            // The following image displacements are byte offsets into the
            // Labels pointer table; these are the corresponding dword slots.
            LPSTR *const labels = Labels.strings_ptr;
            switch (kind + 7) {
            case 6: {
                int ratio = static_cast<int>(weapon_cost) /
                            static_cast<int>(armor_cost);
                if (ratio < 0) {
                    ratio = 0;
                } else if (ratio > 2) {
                    ratio = 2;
                }
                _itoa(ratio, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, "(");
                strcat(StringTemp, StringTable.get(labels[0x7C]));
                strcat(StringTemp, "/");
                strcat(StringTemp, StringTable.get(labels[0x55]));
                strcat(StringTemp, ")");
                break;
            }
            case 5:
                _itoa(static_cast<int>(weapon_cost) - 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, "(");
                strcat(StringTemp, StringTable.get(labels[0x7C]));
                strcat(StringTemp, "-1");
                strcat(StringTemp, ")");
                break;
            case 4:
                _itoa(static_cast<int>(armor_cost) - 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, "(");
                strcat(StringTemp, StringTable.get(labels[0x55]));
                strcat(StringTemp, "-1");
                strcat(StringTemp, ")");
                break;
            case 3:
                _itoa(static_cast<int>(chassis_speed) - 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, "(");
                strcat(StringTemp, StringTable.get(labels[0x2C5]));
                strcat(StringTemp, "-1");
                strcat(StringTemp, ")");
                break;
            case 2:
                _itoa(static_cast<int>(armor_cost + weapon_cost) - 2, digits,
                      10);
                strcat(StringTemp, digits);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, "(");
                strcat(StringTemp, StringTable.get(labels[0x7C]));
                strcat(StringTemp, "+");
                strcat(StringTemp, StringTable.get(labels[0x55]));
                strcat(StringTemp, "-2");
                strcat(StringTemp, ")");
                break;
            case 1:
                _itoa(static_cast<int>(chassis_speed + weapon_cost) - 2,
                      digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, "(");
                strcat(StringTemp, StringTable.get(labels[0x7C]));
                strcat(StringTemp, "+");
                strcat(StringTemp, StringTable.get(labels[0x2C5]));
                strcat(StringTemp, "-2");
                strcat(StringTemp, ")");
                break;
            case 0:
                _itoa(static_cast<int>(chassis_speed + armor_cost) - 2, digits,
                      10);
                strcat(StringTemp, digits);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, "(");
                strcat(StringTemp, StringTable.get(labels[0x55]));
                strcat(StringTemp, "+");
                strcat(StringTemp, StringTable.get(labels[0x2C5]));
                strcat(StringTemp, "-2");
                strcat(StringTemp, ")");
                break;
            default:
                break;
            }
        }

        dialogs.item(StringTemp, ability);
        ++items_added;
    }

    if (items_added == 0) {
        return;
    }

    const int exec_result = popup.exec(0, pop_wait);
    if (exec_result < 0) {
        return;
    }

    int picked = exec_result;
    if (picked >= 0x1D) {
        picked = -1;
    }
    if (which == 0) {
        protoAbilityID1_ = static_cast<uint32_t>(picked);
    } else {
        protoAbilityID2_ = static_cast<uint32_t>(picked);
    }
    protoAbilityFlags_ = 0;
    {
        uint32_t bits = 0;
        for (int slot = 0; slot < 2; ++slot) {
            const int value = static_cast<int>(slot == 0 ? protoAbilityID1_
                                                         : protoAbilityID2_);
            if (value >= 0) {
                bits |= 1u << static_cast<unsigned>(value);
            }
        }
        protoAbilityFlags_ = bits;
    }

    // Refresh the original panels in their original call order.
    draw_special_2(0);
    draw_special_1(0);
    draw_vehicle(0);
    draw_flash();
    draw_info();
    on_update();

}

// Original interface table 0x0066B0F4 slot 1 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolled@DesignWin@@UAEXHH@Z 0x0043D120
void DesignWin::on_iface_scrolled(int, int) {}
// Original interface table 0x0066B0F4 slot 2 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolling@DesignWin@@UAEXHH@Z 0x0043D120
void DesignWin::on_iface_scrolling(int, int) {}
// Original interface table 0x0066B0F4 slot 6 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_left_up@DesignWin@@UAEXHH@Z 0x0043D120
void DesignWin::on_iface_left_up(int, int) {}

// Original interface table 0x0066B0F4 slot 17 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_dialog_back_draw@DesignWin@@UAEHPAVGraphicWin@@@Z 0x0043D120
int DesignWin::on_iface_dialog_back_draw(GraphicWin *) {
    return 0;
}

// Original interface table 0x0066B0F4 slot 18 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404240 0x00404245 ?on_iface_dialog_item_back_draw@DesignWin@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z 0x0043D120
int DesignWin::on_iface_dialog_item_back_draw(GraphicWin *, int, int, RECT *) {
    return 0;
}

// ORIGINAL: 0x00583CB0 ?design_new_veh@@YAXHH@Z 0x00583CB0-0x00583CC7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired exact seed git:5da70027:src/recovered/00583cb0.cpp preserves the23byte original wrapper and direct43E9A0 exec call; designwin.cpp owns the actual71F2B0 singleton and corresponding exec identity."], "reason": "Dispatch the two original design arguments through the genuine source-owned TutWinDesWindow singleton and its DesignWin::exec method, not an original-address receiver."}
void __cdecl design_new_veh(int faction_id, int prototype_id) {
    TutWinDesWindow.exec(faction_id, prototype_id);
}

static int workshop_signed_bits(unsigned int bits) {
    return bits <= 0x7fffffffu ? static_cast<int>(bits)
        : -1 - static_cast<int>(0xffffffffu - bits);
}

static int workshop_scaled_dimension(int dimension, int numerator,
                                     int denominator) {
    const int product = workshop_signed_bits(
        static_cast<unsigned int>(dimension) *
        static_cast<unsigned int>(numerator));
    return product / denominator;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00583CD0 ?hq_list_draw@@YAXPAUGraphicWin@@HHHHH@Z 0x00583CD0-0x00583E2A
// RECOVERY_BINDING: v1 {"evidence": ["Pinned native583CD0..583E2A: signed scaling and centering, active bit0, shared zoom68F0F4 initial-6 and mutable faction93FC48, fake vehicle5C1D50, obsolete order byte11, Buffer secondary base444, veh_draw5594F0 and unity scale epilogue. Retained src/recovered/units/00583cd0.cpp; recovery/tasks/designwin-owner-coverage.json coupled callback authorization; no original-address aliases."], "reason": "Recover the workshop prototype preview through actual GraphicWin Buffer inheritance, VehPrototype and Veh owners, source-owned zoom/faction storage and sprite scaling. Preserve the seed identity and original span; catalogue struct spelling differs from the actual GraphicWin class symbol. No matching claim is granted."}
// symbol    ?hq_list_draw@@YAXPAVGraphicWin@@HHHHH@Z
void __cdecl hq_list_draw(GraphicWin *window, int prototype, int flags,
                          int x, int y, int height) {
    const int zoom = WorkshopPreviewZoom;
    const int override_scale = MapWinCaptionScale;
    if ((flags & 1) != 0) {
        const int width = override_scale > 0
            ? workshop_scaled_dimension(132, override_scale, MapWinCaptionDivisor)
            : workshop_scaled_dimension(132,
                workshop_signed_bits(static_cast<unsigned int>(zoom) + 16u), 16);
        x = workshop_signed_bits(static_cast<unsigned int>(x) +
                                  static_cast<unsigned int>(width));
    }
    if ((VehPrototypes[prototype].flags & 1) == 0) return;

    const int draw_height = override_scale > 0
        ? workshop_scaled_dimension(75, override_scale, MapWinCaptionDivisor)
        : workshop_scaled_dimension(75,
            workshop_signed_bits(static_cast<unsigned int>(zoom) + 16u), 16);
    const int height_difference = workshop_signed_bits(
        static_cast<unsigned int>(height) - static_cast<unsigned int>(draw_height));
    y = workshop_signed_bits(static_cast<unsigned int>(y) +
                              static_cast<unsigned int>(height_difference / 2));
    if (override_scale > 0) {
        SpriteScaleNumerator = override_scale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    } else {
        SpriteScaleDenominator = 16;
        SpriteScaleNumerator = workshop_signed_bits(
            static_cast<unsigned int>(WorkshopPreviewZoom) + 16u);
    }
    const int vehicle = veh_fake(prototype, WorkshopFaction);
    // Reload the faction after veh_fake, matching the native callback.
    const unsigned int faction_bit = 1u <<
        (static_cast<unsigned int>(WorkshopFaction) & 31u);
    if ((VehPrototypes[prototype].obsolete_factions & faction_bit) != 0)
        Vehs[vehicle].order = 1;
    veh_draw(window ? static_cast<Buffer *>(window) : 0,
             vehicle, x, y, zoom, 0x2044, 0);
    // The original publishes unity, rather than restoring the previous scale.
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
}

extern uint32_t UpgradePromptAnswered;

// ORIGINAL: 0x00583E30 ?design_workshop@@YAHHH@Z 0x00583E30-0x00584B4B;0x0065FFD0-0x00660239
// RECOVERY_BINDING: v1 {"evidence": ["Pinned583E30..584B4B and65FFD0..660239; preserved git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00583e30.cpp; mode-zero list bypass, nonzero64-prototype scan,128 sorted list rows, ordered0/2/3/4/default action chain and native Popup cleanup funclets.", "Native5842DB tests priorflags8 before setting200; upgrade scan skips selected/inactive rows, passes faction/replacement/selected/zero, reports only scan exhaustion and rebuilds the entire list. Rename5845AE copies shared prompt result into the prototype name. Callback60BCDF return is discarded; real hq_list_draw owns preview behavior.", "Existing DesignWin/Popup/Dialog owners, WorkshopFaction/PreviewZoom and source-owned prototype/player storage; recovery/tasks/designwin-owner-coverage.json. Existing NetMsgPrimary fixed-address facade and unfinished callee behavior remain separate dependencies; no byte, runtime or game acceptance claim."], "reason": "Recover the complete production workshop dependency through real lexical Popup lifetime, typed dialog and prototype ownership, native list/action flow and source-owned preview callback. Preserve its original identity, seed and historical obligations instead of a link-only placeholder."}
// TRIED: the two "layout" division computations (0x583F94-0x584054) simplified the signed-rounding cdq/and/sar idiom to a plain shift for the likely-positive case; the upgrade-candidate scan (0x5843D5-0x5843EB) approximated the per-slot style comparison rather than tracing every register precisely.
int __cdecl design_workshop(int faction_id, int mode) {
    Popup popup;
    char number[16];

    // Native mode zero skips the automatic-design search and enters the list.
    if (mode != 0) {
        int best_score = 0;
        int best_prototype = -1;
        const int faction_first = faction_id * MaxVehProtoFactionNum;
        for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
            const int prototype_id = faction_first + slot;
            const VehPrototype &prototype = VehPrototypes[prototype_id];
            int score;
            if ((prototype.flags & PROTO_ACTIVE) != 0) {
                if ((prototype.obsolete_factions & (1 << faction_id)) == 0)
                    continue;
                if (PlayersData[faction_id].proto_id_active[prototype_id] != 0)
                    continue;
                score = 0x384 - PlayersData[faction_id].proto_id_queue[prototype_id];
            } else {
                score = 0x3e7;
            }
            if (score > best_score) {
                best_score = score;
                best_prototype = prototype_id;
            }
        }
        if (best_prototype >= 0)
            return TutWinDesWindow.exec(faction_id, best_prototype);
    }

    for (;;) {
        proto_sort(faction_id);
        popups_medium();
        popup.start(PopupStartCaption, "DESIGNSHOP", -1, 0, 2, 0);
        WorkshopFaction = faction_id;
        PopupModalState = 5;
        popup.dialogs().set_callbacks(&hq_list_draw, 0, 0);

        const int caption_scale = MapWinCaptionScale;
        const int caption_divisor = MapWinCaptionDivisor;
        const int metric = WorkshopPreviewZoom;
        int width = caption_scale > 0
            ? workshop_scaled_dimension(132, caption_scale, caption_divisor)
            : workshop_scaled_dimension(132, workshop_signed_bits(static_cast<unsigned int>(metric) + 16u), 16);
        int height = caption_scale > 0
            ? workshop_scaled_dimension(75, caption_scale, caption_divisor)
            : workshop_scaled_dimension(75, workshop_signed_bits(static_cast<unsigned int>(metric) + 16u), 16);
        Dialog &dialog = static_cast<Dialog &>(popup.dialogs());
        dialog.field_34_ = workshop_signed_bits(static_cast<unsigned int>(width) * 2u + 3u);
        dialog.field_48_ = height;
        dialog.field_50_ = 8;

        for (int row = 0; row < 128; ++row) {
            const int prototype_id = ProtoSortIds[row];
            const int prototype_faction = prototype_id / MaxVehProtoFactionNum;
            if (prototype_faction != 0 && prototype_faction != faction_id)
                continue;
            const VehPrototype &prototype = VehPrototypes[prototype_id];
            const bool active = (prototype.flags & PROTO_ACTIVE) != 0;
            if (active && prototype_faction != 0
                && !has_tech(prototype.preq_tech, faction_id))
                continue;

            StringTemp[0] = 0;
            if (active) {
                const bool obsolete =
                    (prototype.obsolete_factions & (1 << faction_id)) != 0;
                if (obsolete) strcat(StringTemp, "-");
                strcat(StringTemp, prototype.veh_name);
                strcat(StringTemp, "|");
                if (obsolete) {
                    strcat(StringTemp, label_get(0x368 / 4));
                    strcat(StringTemp, ", ");
                }
                if ((prototype.flags & 4) == 0 && prototype_id >= 64) {
                    strcat(StringTemp, label_get(0x2e4 / 4));
                    strcat(StringTemp, ", ");
                }
                strcat(StringTemp, label_get(0x160 / 4));
                strcat(StringTemp, ": ");
                _itoa(workshop_signed_bits(static_cast<unsigned int>(cost_factor(faction_id, 1, -1)) * prototype.cost),
                      number, 10);
                strcat(StringTemp, number);
                strcat(StringTemp, ", ");
                say_stats(StringTemp, prototype_id, 0);
            } else if (prototype_faction == 0) {
                strcat(StringTemp, "- ");
                strcat(StringTemp, label_get(0x1b8 / 4));
                strcat(StringTemp, " -");
            } else {
                continue;
            }
            popup.dialogs().item(StringTemp, prototype_id);
        }

        const int selected = popup.BasePop::exec(0, 0);
        popups_normal();
        g_FX.play(0xf);
        const uint32_t action = popup.input_poll_state();
        if (action == 0)
            return -1;
        if (action == 2) {
            char *prototype_name = VehPrototypes[selected].veh_name;
            if (X_pop_ask("DESIGNNAME", 31, prototype_name, 0, 0) != 0)
                return -1;
            strcpy(prototype_name, ParseStrBuffer[0].str);
            synch_proto(selected);
            return -1;
        }
        if (action == 3) {
            VehPrototype &prototype = VehPrototypes[selected];
            if ((prototype.obsolete_factions & (1 << faction_id)) != 0) {
                if ((prototype.flags & 0x8) != 0)
                    prototype.flags |= 0x200;
                prototype.flags |= 8;
            } else {
                prototype.flags &= static_cast<uint16_t>(~(0x200 | 8));
            }
            prototype.obsolete_factions ^= static_cast<uint8_t>(1 << faction_id);
            synch_obs(selected);
            continue;
        }
        if (action == 4) {
            if (NetDaemonNet.lock(2, -1, -1, 0, -1, -1) != 0)
                continue;
            UpgradePromptAnswered = 0;
            int replacement = -1;
            const VehPrototype &old = VehPrototypes[selected];
            int scanned = 0;
            for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
                scanned = slot;
                const int candidate_id = faction_id * MaxVehProtoFactionNum + slot;
                if (candidate_id == selected) continue;
                const VehPrototype &candidate = VehPrototypes[candidate_id];
                if ((candidate.flags & PROTO_ACTIVE) == 0) continue;
                if ((candidate.obsolete_factions & (1 << faction_id)) != 0)
                    continue;
                if ((candidate.flags & 4) == 0) continue;
                if (candidate.plan != old.plan || candidate.unk_1 != old.unk_1)
                    continue;
                if (Chassis[candidate.chassis_id].triad
                    == Chassis[old.chassis_id].triad) {
                    replacement = candidate_id;
                    break;
                }
            }
            if (replacement < 0) scanned = MaxVehProtoFactionNum;
            if (replacement >= 0)
                upgrade_prototype(faction_id, replacement, selected, 0);
            NetDaemonNet.unlock();
            if (scanned >= MaxVehProtoFactionNum) {
                const char *icon = (Players[faction_id].rule_flags & 0x80)
                    ? "alunit_sm.pcx" : "unit_sm.pcx";
                popp(ScriptPopupFile, "NOUPGRADE", 0, icon, 0);
            }
            continue;
        }
        if (selected < MaxVehProtoFactionNum) {
            NetMsgPrimary.pop("DESIGNNO", 0x1388, 0, 0);
            return -1;
        }
        VehPrototypes[selected].flags &= static_cast<uint16_t>(~0x80);
        return TutWinDesWindow.exec(faction_id, selected);
    }
}

// ORIGINAL: 0x0043ECB0 ??1DesignWin@@QAE@XZ 0x0043ECB0-0x0043EDA6;0x00653E70-0x00653F0D
// RECOVERY_BINDING: v1 {"evidence": ["task:designwin-lifetime-owner", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:43ECB0..43EDA6;653E70..653F0D", "actual source:designwin.h member/base declarations; reverse fonts3/2/1,Buffer,Caviar,Time,Spots2/1,FlatButtons25,GraphicWin;SubInterface destructor trivial.", "historical seed:src/recovered/units/0043ecb0.cpp; complete identity/spans/status and useful findings retained; retirement requires native verification."], "reason": "Recover normal and exception-unwind destruction through genuine DesignWin members and bases. Body invokes close(); the compiler owns reverse subobject destruction, EH cleanup and vptr transitions. Original unclaimed seed measured mismatch at instruction10; no matching claim is granted."}
// symbol    ??1DesignWin@@UAE@XZ
DesignWin::~DesignWin() {
    close();
}
