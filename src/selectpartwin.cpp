#include "stdafx.h"
#include "interface_fonts.h"
#include "selectpartwin.h"
#include "technology.h"
#include "general.h"
#include "console.h"  // ConsoleGlobal.faction_id_ owns old 0x00939284
#include "veh.h"
#include "alpha.h"
#include "faction.h"  // pop_wait is the typed modal pump at 0x005398E0
#include "game.h"
#include "strings.h"
#include "win.h"
#include <cstring>

// The picker background tile and its colour remain image-owned aliases.
static Sprite *const SelectPartTile = (Sprite *)0x006B33F8;
static const unsigned char *const SelectPartTileColour =
    (const unsigned char *)0x006B3400;
// Renderer origin is the existing veh.cpp object VehLightDirMap.

void __cdecl scroll_show(GraphicWin *window, Scroll *scroll);
void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll);

// ORIGINAL: 0x00433F70 ?exec@SelectPartWin@@QAEHHHHHHHHHHHPAE@Z 0x00433F70-0x0043446F;0x00653F5A-0x00653F6F FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a:src/unrecovered/00433f70.cpp", "SelectPartWin member layout in src/selectpartwin.h", "Caviar renderer API in src/caviar.h", "RulesChassis/RulesWeapon/RulesArmor/RulesReactor in src/veh.h"], "reason": "Implement the shared part-picker modal with typed window, buffer, renderer, scroll and rules-table members while retaining the unresolved image-owned renderer/modal seams."}
int SelectPartWin::exec(int x, int y, int value0, int kind, int selected,
                        int chassis, int weapon, int armor, int abilities,
                        int reactor, unsigned char *palette) {
    (void)x;
    (void)y;
    (void)value0;
    const int saved_selection = selected;
    field_2B68_ = value0;
    field_2B88_ = chassis;
    field_2B8C_ = weapon;
    field_2B90_ = armor;
    field_2B84_ = kind;
    field_2B94_ = abilities;
    field_2B98_ = reactor;
    field_4644_ = reinterpret_cast<uint32_t>(palette);
    if (kind == 3) {
        --selected;
    }

    caviar_.init();
    caviar_.set_field_A4(0);
    font1_.init(DefaultFontFace, 10, 0);
    font2_.init(DefaultFontFace, 12, 1);
    static_cast<Buffer *>(this)->set_font(&font1_, 0, 0, 0);

    union { uint32_t bits; float value; } view_angle;
    view_angle.bits = 0x3f48f5c3;
    float angles[3] = {0.0f, 0.0f, view_angle.value};
    VOX_Matrix rotation;
    caviar_apply_rotation(angles, &rotation);
    VOX_Vect camera;
    for (int axis = 0; axis < 3; ++axis) {
        camera.values[axis] = -rotation.values[2][axis] *
                              CaviarCameraDistance;
    }
    caviar_.set_camera_direct(&camera, &rotation);
    union { uint32_t bits; float value; } scene_angle;
    scene_angle.bits = 0x3f05f92d;
    caviar_.set_scene_rotation(scene_angle.value, 0.0f, 0.0f);
    caviar_.light_vector() = VehLightDirMap;
    caviar_.shadow_vector() = VehLightDirMap;

    field_2B74_ = 0;
    switch (kind) {
        case 0: {
            for (int i = 0; i <= CHSI_MISSILE; ++i) {
                if (Chassis[i].preq_tech >= -1) ++field_2B74_;
            }
            break;
        }
        case 1: {
            for (int i = 0; i < MaxWeaponNum; ++i) {
                if (Weapon[i].preq_tech >= -1) ++field_2B74_;
            }
            break;
        }
        case 2: {
            if (Chassis[chassis].missile == 0) {
                for (int i = 0; i < MaxArmorNum; ++i) {
                    if (Armor[i].preq_tech >= -1) ++field_2B74_;
                }
            } else {
                field_2B74_ = 1;
            }
            break;
        }
        case 3: {
            for (int i = 0; i < MaxReactorNum; ++i) {
                if (Reactor[i].preq_tech >= -1) ++field_2B74_;
            }
            break;
        }
        default:
            return saved_selection;
    }

    field_2B6C_ = 5;
    field_2B70_ = 4;
    field_2B64_ = 0x14;

    Buffer backdrop;
    backdrop.init(10, 10, 0, 0);
    SelectPartTile->draw(&backdrop, *SelectPartTileColour & 0xff,
                         0, 0, 1, 1);

    const int width = WinScreenWidth;
    const int height = WinScreenHeight;
    GraphicWin::init(0, 0, width, height, 0, 0x1000020,
                     static_cast<Win *>(this), 0, 0);
    GraphicWin::fill(9);
    RECT frame = {0x17, 0, width - 0x37, height};
    static_cast<Buffer *>(this)->box_sprite(&frame, &InterfaceBoxStyles[0]);

    if (field_2B74_ > 0x12) {
        scroll_.init(width - 0x2b, 0x15, 0x18, 0xf6,
                     static_cast<Win *>(this), 0, 1);
        scroll_.hide();
        field_2B60_ = 0;
        scroll_.set_range(0, field_2B74_ / field_2B6C_ - field_2B70_ + 2);
        scroll_.paging_ = field_2B70_ - 1;
        scroll_.field_A30_ = 0;
        scroll_.redraw();
    }

    field_2B60_ = 0;
    if (selected >= field_2B64_) {
        int first_visible = 0;
        do {
            first_visible += field_2B6C_;
            field_2B60_ = first_visible;
            scroll_.set_pos(first_visible / field_2B6C_);
        } while (selected >= first_visible + field_2B64_);
    }
    for (int index = field_2B60_;
         index < field_2B60_ + field_2B64_; ++index) {
        draw_part(index, selected == index ? 1 : 0);
    }

    field_2B80_ = selected;
    if (field_2B64_ < field_2B74_) {
        scroll_show(this, &scroll_);
    }
    set_modal(1, pop_wait, 0);
    if (field_2B64_ < field_2B74_) {
        scroll_hide(this, &scroll_);
    }
    scroll_.show(0);
    GraphicWin::hide();
    do_all_draws();

    if (field_2B80_ >= 0 && field_4648_ != 0) {
        return field_2B80_ + (kind == 3 ? 1 : 0);
    }
    return saved_selection;
}

// Constructor 0x4328B0 installs these two compiler-owned tables. Their
// extents remain unproved; native table diagnostics are positional evidence.
// RECOVERY_GENERATED: v1 vtable 0x0066B2B8 ? ??_7SelectPartWin@@6BWin@@@ 0x004328B0
// RECOVERY_GENERATED: v1 vtable 0x0066B2B0 ? ??_7SelectPartWin@@6BBuffer@@@ 0x004328B0

/*
Purpose: Reset six fields, then close the window and its Buffer.

             xor eax,eax / mov [esi+0x2b60],eax / mov [esi+0x2b6c],eax
             mov [esi+0x2b70],eax / mov [esi+0x2b80],eax
             mov [esi+0x2b68],0xffffffff / mov [esi+0x4648],eax
             call GraphicWin::close / lea ecx,[esi+0x406c]
             call Buffer::close / ret

         Five of the six are cleared and ONE - 0x2b68 - is set to -1. It is
         written FIFTH, between 0x2b80 and 0x4648, so the write order is not
         ascending and the odd value is not at the end either.

         GraphicWin::close is called on `this` itself; Buffer::close on the
         subobject at 0x406c.

// ORIGINAL: 0x00432970 ?close@SelectPartWin@@QAEXXZ 0x00432970-0x004329AF BYTE_EXACT
// symbol    ?close@SelectPartWin@@QAEXXZ
// size      63 bytes
// prototype void (__thiscall ?close@SelectPartWin@@QAEXXZ)(SelectPartWin* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x005D7470
Return Value: n/a
Status: Complete
*/
void SelectPartWin::close() {
    field_2B60_ = 0;
    field_2B6C_ = 0;
    field_2B70_ = 0;
    field_2B80_ = 0;
    field_2B68_ = -1;
    field_4648_ = 0;
    GraphicWin::close();
    buffer_.close();
}

/*
Purpose: Construct the SelectPartWin frame: the GraphicWin base, the Scroll
         (0xa14), Caviar (0x2b9c), Buffer (0x406c) and two Font members, the
         five cleared state fields, and the two vtable stores the window and
         its Buffer subobject carry.

// ORIGINAL: 0x004328B0 ??0SelectPartWin@@QAE@XZ 0x004328B0-0x00432961;0x00653F10-0x00653F5A FILE BYTE_EXACT
// TRIED: same shape as 0x00484AC0 (PickTech's ctor) - placement-new for the 6 subobjects (GraphicWin base, Scroll/Caviar/Buffer/Font/Font members) plus the 5 zeroed fields and 2 vtable stores, in disassembly order, reproduce the call sequence but not the opaque scaffold's missing SEH-unwind prologue (see 0x00484AC0's note for why).
// symbol   ??0SelectPartWin@@QAE@XZ
// size      251 bytes
// prototype void (__thiscall ??0SelectPartWin@@QAE@XZ)(SelectPartWin* this)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// notes     Seed provenance (TRIED): the placement-new sequence, the five
//           zeroed fields and the two vtable stores reproduce the call
//           order but not the SEH-unwind prologue the image opens with -
//           the same wall as the Credits constructor (placement-new
//           registers no unwinding; real members would).
Return Value: n/a
Status: Coverage transcription - unclaimed body, seed
        git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004328b0.cpp retained.
*/
SelectPartWin::SelectPartWin() {
    field_2B6C_ = 0;
    field_2B70_ = 0;
    field_2B80_ = 0;
    field_4648_ = 0;
    field_2B60_ = 0;
}

/*
Purpose: Answer the design-parts list's keyboard navigation: the arrow keys
         move the highlight one cell within the current row and scroll the
         list when the selection leaves the visible page, the page keys move
         the first-shown row by one page step, end snaps to the last entry
         and home clears both anchor slots, escape drops the selection, and
         Enter releases the modal window. Whenever the anchor
         moved the window's slot-62 redraw fires; otherwise a moved highlight redraws
         only the old and new cells through draw_part and parks its result
         in the window's +0x4648 slot. Every path reaching the common tail,
         including unhandled keys, invokes slot 63; boundary-key early
         returns do not.

// ORIGINAL: 0x00433A10 ?on_key_click@SelectPartWin@@QAEHHH@Z 0x00433A10-0x00433CD0 FILE
// RECOVERY_BINDING: v1 {"evidence": ["The original SelectPartWin vtable at 0x0066B2B8 selects 0x00433A10 at inherited slot 25.", "The pinned key table dispatches Enter, Escape, navigation keys and their numpad equivalents; original seed at git:08336bcc:src/unrecovered/00433a10.cpp."], "reason": "The real SelectPartWin override in slot 25 owns keyboard navigation of its Scroll and selection members."}
// symbol   ?on_key_click@SelectPartWin@@UAEHHH@Z
// size      704 bytes
// prototype int (__thiscall ?on_key_click@SelectPartWin@@QAEHHH@Z)(SelectPartWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00433D60 0x00605D20
Return Value: 1 for every key, handled or not
Status: Coverage transcription - unclaimed body; historical seed at
        git:08336bcc:src/unrecovered/00433a10.cpp. The switch below is spelled
        over `key - 13` because that is the subtraction the image folds into
        its two-level jump-table index; the numpad keys share their arrow
        blocks exactly as the image's table cells do.
*/
int SelectPartWin::on_key_click(int, int key) {
    const int first_cached = field_2B60_;
    const int highlight = field_2B80_;

    switch (key - 13) {
        case 0:  // 13, Enter: release modal
            release_modal();
            break;
        case 14:  // 27, escape: drop the selection, then release modal
            field_2B80_ = -1;
            release_modal();
            break;
        case 20:  // 33, page up
            if (field_2B74_ <= field_2B64_) return 1;
            scroll_.set_pos(scroll_.position_ - scroll_.paging_);
            field_2B60_ = field_2B6C_ * scroll_.position_;
            break;
        case 21:  // 34, page down
            if (field_2B74_ <= field_2B64_) return 1;
            scroll_.set_pos(scroll_.paging_ + scroll_.position_);
            field_2B60_ = field_2B6C_ * scroll_.position_;
            break;
        case 22: {  // 35, end: highlight the last entry, re-anchor its row
            const int count = field_2B74_;
            field_2B80_ = count - 1;
            if (field_2B64_ < count) {
                field_2B60_ =
                    field_2B6C_ *
                        ((count - 1) /
                             field_2B6C_ +
                         1) -
                    field_2B64_;
            }
            break;
        }
        case 23:  // 36, home: clear both anchor slots
            field_2B60_ = 0;
            field_2B80_ = 0;
            break;
        case 24:   // 37, left
        case 87: { // 100, numpad 4
            const int per_row =
                field_2B6C_;
            const int row_start = highlight / per_row * per_row;
            int limit = row_start + per_row - 1;
            if (limit >
                field_2B74_ - 1) {
                limit = field_2B74_ - 1;
            }
            const int target = highlight - 1;
            field_2B80_ = target;
            if (target < row_start || limit < row_start) {
                field_2B80_ = row_start;
            } else if (target > limit) {
                field_2B80_ = limit;
            }
            break;
        }
        case 26:   // 39, right
        case 89: { // 102, numpad 6
            const int per_row =
                field_2B6C_;
            const int row_start = highlight / per_row * per_row;
            int limit = row_start + per_row - 1;
            if (limit >
                field_2B74_ - 1) {
                limit = field_2B74_ - 1;
            }
            const int target = highlight + 1;
            field_2B80_ = target;
            if (target < row_start || limit < row_start) {
                field_2B80_ = row_start;
            } else if (target > limit) {
                field_2B80_ = limit;
            }
            break;
        }
        case 25:   // 38, up
        case 91: { // 104, numpad 8
            const int per_row =
                field_2B6C_;
            if (highlight < per_row) return 1;
            do {
                field_2B80_ -= per_row;
            } while (field_2B80_ >= first_cached + field_2B64_);
            if (field_2B80_ < first_cached) {
                field_2B60_ -= per_row;
                scroll_.set_pos(scroll_.position_ - 1);
            }
            break;
        }
        case 27:   // 40, down
        case 85: { // 98, numpad 2
            const int per_row =
                field_2B6C_;
            const int count = field_2B74_;
            if (highlight > count - per_row - 1) {
                if (count <= field_2B64_) return 1;
                scroll_.set_pos(scroll_.position_ + 1);
                field_2B60_ = per_row * scroll_.position_;
            } else {
                int moved = field_2B80_;
                do {
                    moved += per_row;
                } while (moved < first_cached);
                field_2B80_ = moved;
                if (moved >= first_cached + field_2B64_) {
                    field_2B60_ = first_cached + per_row;
                    scroll_.set_pos(scroll_.position_ + 1);
                }
            }
            break;
        }
        default:
            break;
    }

    if (field_2B60_ != first_cached) {
        redraw();
    } else if (field_2B80_ != highlight) {
        draw_part(highlight, 0);
        field_4648_ = draw_part(field_2B80_, 1);
    }
    on_update();
    return 1;
}

/*
Purpose: ?on_scrolling@SelectPartWin@@QAEXHH@Z - dispatch to slot 0xc0 of this object's OWN
         vtable, forwarding 2 argument(s).
// ORIGINAL: 0x00433F50 ?on_scrolling@SelectPartWin@@QAEXHH@Z 0x00433F50-0x00433F67 BYTE_EXACT
// symbol    ?on_scrolling@SelectPartWin@@UAEXHH@Z
// size      23 bytes
// prototype void (__thiscall ?on_scrolling@SelectPartWin@@QAEXHH@Z)(SelectPartWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00433F5D
Return Value: n/a
Status: Complete
*/
void SelectPartWin::on_scrolling(int a1, int a2) {
    on_scrolled(a1, a2);
}

// 0x0043EE10 destructor seed: reset and close the enclosing GraphicWin and
// owned Buffer, then let C++ destroy Font, Buffer, Caviar, Scroll and the base
// in reverse construction order. The compiler owns both vptrs and EH unwind.
// Source seed: git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/units/0043ee10.cpp (measured MISMATCH).
// ORIGINAL: 0x0043EE10 ??1SelectPartWin@@UAE@XZ 0x0043EE10-0x0043EF6A;0x00654720-0x006547AC
// RECOVERY_BINDING: v1 {"evidence": ["Original seed git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/units/0043ee10.cpp; its original spans and historical obligations are retained.", "SelectPartWin constructor 0x004328B0 establishes GraphicWin, Scroll+0xA14, Caviar+0x2B9C, Buffer+0x406C and Font members+0x45F4/+0x461C.", "The historical QAE destructor spelling is superseded by UAE from the actual GraphicWin-derived virtual destructor; automatic reverse member/base teardown remains owned by SelectPartWin.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/units/0043ee10.cpp", "Historical seed spelling ??1SelectPartWin@@QAE@XZ; current source mapping ??1SelectPartWin@@UAE@XZ; emitted symbol ??1SelectPartWin@@UAE@XZ."], "reason": "The original complete destructor resets/closes the GraphicWin and embedded Buffer before reverse member destruction. The real SelectPartWin destructor owns that lifecycle. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
SelectPartWin::~SelectPartWin() {
    close();
}

// 0x004339C0 on_redraw: redraw only the visible cells, selecting the current
// highlight. Historical seed: git:08336bcc:src/recovered/units/004339c0.cpp.
// ORIGINAL: 0x004339C0 ?on_redraw@SelectPartWin@@UAEXXZ 0x004339C0-0x00433A02 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original seed at git:08336bcc:src/recovered/units/004339c0.cpp; its original spans and historical obligations are retained.", "SelectPartWin constructor 0x004328B0 establishes GraphicWin, Scroll+0xA14, Caviar+0x2B9C, Buffer+0x406C and Font members+0x45F4/+0x461C."], "reason": "The SelectPartWin primary vtable at 0x0066B2B8 selects this original body in slot 12; the real GraphicWin-derived override retains its provisional inherited name."}
void SelectPartWin::on_redraw() {
    for (int i = field_2B60_; i < field_2B60_ + field_2B64_; ++i) {
        draw_part(i, field_2B80_ == i);
    }
}

// 0x00433820: the catalogue/seed calls this on_mouse_move, while inherited
// Win slot 17 is still provisionally named on_mouse_move. The image's
// SelectPartWin vtable selects this body at slot 17. Grid cells are 108x94
// with a 4px inset.
// Historical seed: git:08336bcc:src/recovered/units/00433820.cpp (measured MISMATCH).
// ORIGINAL: 0x00433820 ?on_mouse_feedback@SelectPartWin@@UAEXHH@Z 0x00433820-0x00433934
// RECOVERY_BINDING: v1 {"evidence": ["Original seed at git:08336bcc:src/recovered/units/00433820.cpp; its original spans and historical obligations are retained.", "SelectPartWin constructor 0x004328B0 establishes GraphicWin, Scroll+0xA14, Caviar+0x2B9C, Buffer+0x406C and Font members+0x45F4/+0x461C."], "reason": "The SelectPartWin primary vtable at 0x0066B2B8 selects this original body in slot 17; the real GraphicWin-derived override retains its provisional inherited name."}
void SelectPartWin::on_mouse_move(int x, int y) {
    int old = field_2B80_;
    if (x >= 4 && y >= 4 &&
        x < field_2B6C_ * 108 + 4 && y < field_2B70_ * 94 + 4) {
        int next = (y - 4) / 94 * field_2B6C_ + (x - 4) / 108 + field_2B60_;
        if (next == old) {
            return;
        }
        if (next < field_2B74_) {
            field_4648_ = draw_part(next, 1);
            if (old >= 0) {
                draw_part(old, 0);
            }
            field_2B80_ = next;
            on_update();
            return;
        }
    }
    if (old >= 0) {
        draw_part(old, 0);
    }
    field_4648_ = 0;
    field_2B80_ = -1;
}

// 0x00433940 on_left_up: release the pointer capture and modal state after
// forwarding the point through the real inherited feedback virtual.
// Historical seed: git:08336bcc:src/unrecovered/00433940.cpp.
// ORIGINAL: 0x00433940 ?vslot_21@SelectPartWin@@UAEXHH@Z 0x00433940-0x004339BF
// RECOVERY_BINDING: v1 {"evidence": ["Original seed at git:08336bcc:src/unrecovered/00433940.cpp; its original spans and historical obligations are retained.", "SelectPartWin constructor 0x004328B0 establishes GraphicWin, Scroll+0xA14, Caviar+0x2B9C, Buffer+0x406C and Font members+0x45F4/+0x461C."], "reason": "The SelectPartWin primary vtable at 0x0066B2B8 selects this original body in slot 21; the real GraphicWin-derived override retains its provisional inherited name."}
void SelectPartWin::on_left_up(int x, int y) {
    on_mouse_move(x, y);
    if (field_4648_ == 0) {
        field_2B80_ = -1;
        if (WinPointerOwner3 == this) {
            WinPointerOwner3 = 0;
            on_lose_mouse_capture();
        }
    } else if (WinPointerOwner3 == this) {
        WinPointerOwner3 = 0;
        on_lose_mouse_capture();
    }
    if (WinPointerOwner4 == this) {
        WinPointerOwner4 = 0;
    }
    release_modal();
}

// 0x00433F30 on_scrolled: the Scroll reports its row in the second argument.
// Historical seed: git:08336bcc:src/recovered/00433f30.cpp (image-only exact).
// ORIGINAL: 0x00433F30 ?vslot_48@SelectPartWin@@UAEXHH@Z 0x00433F30-0x00433F4F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original seed at git:08336bcc:src/recovered/00433f30.cpp; its original spans and historical obligations are retained.", "SelectPartWin constructor 0x004328B0 establishes GraphicWin, Scroll+0xA14, Caviar+0x2B9C, Buffer+0x406C and Font members+0x45F4/+0x461C."], "reason": "The SelectPartWin primary vtable at 0x0066B2B8 selects this original body in slot 48; the real GraphicWin-derived override retains its provisional inherited name."}
// symbol    ?on_scrolled@SelectPartWin@@UAEXHH@Z
void SelectPartWin::on_scrolled(int, int row) {
    field_2B60_ = field_2B6C_ * row;
    redraw();
}

// The picker consumes the existing vehicle rules owners. Independent UI
// label-color slots still require storage ownership evidence.

// Existing RulesChassis[] owns the picker records: preq_tech is at +0x8E,
// speed at +0x48, triad at +0x49, and the ordinary picker range is nine
// records (MaxChassisNum). The independent label-color slots remain raw.
static int *const PartLabelColorA1 = (int *)0x008C6DD4;
static int *const PartLabelColorA2 = (int *)0x008C6DD8;
static int *const PartLabelColorB1 = (int *)0x008C6DDC;
static int *const PartLabelColorB2 = (int *)0x008C6DE0;


/*
Purpose: Draw one entry of the design parts list: translate the entry index
         into the grid cell inside the scroll's viewport, clip there, draw
         the panel box in the requested style half, then hand the inset cell
         to whichever of the four part drawers the window's mode field
         selects; out-of-range entries tile the background sprite instead.

// ORIGINAL: 0x00433D60 ?draw_part@SelectPartWin@@QAEHHH@Z 0x00433D60-0x00433F1D FILE
// symbol   ?draw_part@SelectPartWin@@QAEHHH@Z
// size      445 bytes
// prototype int (__thiscall ?draw_part@SelectPartWin@@QAEHHH@Z)(SelectPartWin* this, int, int)
// callers   4   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004329B0 0x00432CC0 0x00433170 0x00433520 0x005D8000 0x005DA9F0 0x005E2DD3
// notes     The four dispatch targets are same-class members hypothesis_
//           layouts.h does not declare; they land beside this body as free
//           functions and the switch calls them by name.
Return Value: 1 the cell was drawn or tiled, 0 the index was past the table
Status: Coverage transcription - unclaimed body; historical seed at
        git:08336bcc:src/unrecovered/00433d60.cpp.
*/
int SelectPartWin::draw_part(int a1, int a2) {
    const int base = field_2B60_;

    if (a1 >= base && a1 < base + field_2B64_) {
        const int rel = a1 - base;
        const int per_row = field_2B6C_;
        const int col = rel / per_row;
        const int row = rel % per_row;

        // The seed's own axis naming: the row-scaled pair lands in the
        // rect's left/right slots and the col-scaled pair in top/bottom.
        const int x0 = row * 0x6C + 0x1B;
        const int x1 = row * 0x6C + 0x84;
        const int y0 = col * 0x5E + 4;
        const int y1 = col * 0x5E + 0x5F;

        if (a1 < field_2B74_) {
            RECT rect;
            rect.left = x0;
            rect.top = y0;
            rect.right = x1;
            rect.bottom = y1;

            Buffer *const buf = static_cast<Buffer *>(this);
            buf->set_clip(&rect);

            buf->box_sprite(
                &rect, &InterfaceBoxStyles[2 * (a2 != 0 ? 1 : 0)]);

            rect.left += 3;
            rect.top += 3;
            rect.right -= 3;
            rect.bottom -= 3;

            int result;
            switch (field_2B84_) {
                case 0:
                    result = draw_chassis(a1, &rect,
                                                        a2);
                    break;
                case 1:
                    result = draw_weapon(a1, &rect,
                                                       a2);
                    break;
                case 2:
                    result = draw_armor(a1, &rect,
                                                      a2);
                    break;
                case 3:
                    result = draw_reactor(a1, &rect,
                                                        a2);
                    break;
                default:
                    buf->restore_clip();
                    return reinterpret_cast<int>(buf);
            }
            buf->restore_clip();
            return result;
        }

        static_cast<Buffer *>(this)
            ->tile(reinterpret_cast<Sprite *>(0x006B33F8), x0, y0, x0,
                   y0, 0x69, 0x5B);
    }
    return 1;
}

/*
Purpose: Draw one chassis entry's label and render: find the a1'th enabled
         record of the chassis table, reject it when the window's faction
         lacks the record's tech, build the chassis into the window's Caviar
         renderer at the cell centre (greyed to the missing-tech colour when
         incomplete), then write the record's upper-cased name centred above
         the cell and its count/order byte values in the two label corners.

// ORIGINAL: 0x004329B0 ?draw_chassis@SelectPartWin@@QAEHHPAURECT@@H@Z 0x004329B0-0x00432CB1 FILE
// symbol   ?draw_chassis@SelectPartWin@@QAEHHPAUtagRECT@@H@Z
// size      769 bytes
// prototype int (__thiscall ?draw_chassis@SelectPartWin@@QAEHHPAURECT@@H@Z)(SelectPartWin* this, int, int, RECT*, int)
// callers   1   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// notes     Seed provenance (TRIED): MISMATCH #6 push/mov - direct
//           self+offset pointer arithmetic gave the allocator no reason to
//           keep `this` pinned in edi across the body the way the image
//           does. Not measured exact.
Return Value: 1 the entry was drawn, 0 the index ran past the table
Status: Coverage transcription - unclaimed body; historical seed at
        git:08336bcc:src/unrecovered/004329b0.cpp.
*/
int SelectPartWin::draw_chassis(int a1, RECT *a2, int a3) {

    // Walk the nine ordinary chassis records whose signed prerequisite
    // is eligible (>= -1) until the a1'th entry.
    const RulesChassis *entry = Chassis;
    int idx = 0;
    int slot = 0;
    int has_all_tech_flag = 0;

    for (;;) {
        if (entry->preq_tech >= -1) {
            if (idx == a1) {
                const int tech_id = entry->preq_tech;
                if (has_tech(tech_id,
                             field_2B68_) == 0) {
                    has_all_tech_flag = 1;
                }
                break;
            }
            idx++;
        }
        ++entry;
        slot++;
        if (!(entry < Chassis + MaxChassisNum)) {
            break;
        }
    }

    if (slot == 9) {
        return 0;
    }

    caviar_.reset_objects();
    vehdraw_construct_chassis(field_2B68_, slot,
                              0, 0, field_2B98_,
                              &caviar_);
    caviar_.set_scaling_bits(0x3D75C28F);

    Buffer *const buf = static_cast<Buffer *>(this);
    const int x_avg = (a2->right + a2->left) / 2;
    const int y_avg = (a2->bottom + a2->top) / 2;

    if (has_all_tech_flag != 0) {
        caviar_.render_mono(buf, x_avg, y_avg, 0xB9, 6);
        return has_all_tech_flag == 0;
    }

    caviar_.render(buf, x_avg, y_avg, 6);

    buf->set_text_color(*PartLabelColorA1, *PartLabelColorA2, 1, 1);

    Font *const font1 = &font1_;
    Font *const font2 = &font2_;
    RECT temp;

    temp.left = a2->left;
    temp.right = a2->right;
    temp.top = a2->top;
    temp.bottom = a2->top + font1->line_advance();

    StringTemp[0] = 0;
    strcat(StringTemp,
           StringTable.get(*reinterpret_cast<const int *>(
               &Chassis[slot].offsv_1_name)));
    CharUpperA(StringTemp);
    buf->wrap_cent(font1, StringTemp, &temp);

    buf->set_text_color(*PartLabelColorB1, *PartLabelColorB2, 1, 1);

    temp.left = a2->left;
    temp.right = a2->right;
    temp.bottom = a2->bottom;
    temp.top = a2->bottom - font1->line_advance();

    StringTemp[0] = 0;
    strcat(StringTemp,
           StringTable.get(*reinterpret_cast<const int *>(
               &Triad[Chassis[slot].triad])));

    buf->set_font(font1, 0, 0, 0);
    buf->write_l(StringTemp, &temp,
                 static_cast<int>(strlen(StringTemp)));

    char itoa_buf[80];
    _itoa(Chassis[slot].speed, itoa_buf, 10);
    StringTemp[0] = 0;
    strcat(StringTemp, itoa_buf);

    buf->set_font(font2, 0, 0, 0);
    buf->write_right_l(StringTemp, &temp,
                       static_cast<int>(strlen(StringTemp)));

    return has_all_tech_flag == 0;
}

// The reactor walk runs on veh.h's Reactor[] itself: the enable test reads
// Reactor[i].preq_tech, and the tech-id and name reads sit one record back
// (the image bases them at 0x009527F4/0x009527EC, Reactor-4/-12).

/*
Purpose: Draw one reactor entry of the design parts list: find the a1'th
         enabled reactor record, reject on the faction's missing tech, build
         the reactor into the window's Caviar renderer at the cell centre
         (greyed when the tech is missing), then write the record's name
         centred above the cell, the fixed "Reactors" label below-left, and
         the reactor's own number below-right.

// ORIGINAL: 0x00433520 ?draw_reactor@SelectPartWin@@QAEHHPAURECT@@H@Z 0x00433520-0x00433811 FILE
// symbol   ?draw_reactor@SelectPartWin@@QAEHHPAUtagRECT@@H@Z
// size      753 bytes
// prototype int (__thiscall ?draw_reactor@SelectPartWin@@QAEHHPAURECT@@H@Z)(SelectPartWin* this, int, int, RECT*, int)
// callers   1   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// notes     Seed provenance (TRIED): instruction-for-instruction on the
//           table scan, has_tech dispatch and every call target/argument
//           order; the prologue register assignment differs (the image
//           keeps this in ebx, the slot counter in esi, the tech flag in
//           edi). Not measured exact.
Return Value: 1 the entry was drawn, 0 the index ran past the table
Status: Coverage transcription - unclaimed body; historical seed at
        git:08336bcc:src/unrecovered/00433520.cpp.
*/
int SelectPartWin::draw_reactor(int a1, RECT *a2, int a3) {

    int slot = 1;
    int valid_count = 0;
    int has_tech_failed = 0;
    int i;
    for (i = 0; i < 4; ++i) {
        if (Reactor[i].preq_tech >= -1) {
            if (valid_count == a1) {
                const short tech_id = Reactor[slot - 1].preq_tech;
                if (!has_tech(tech_id,
                              field_2B68_)) {
                    has_tech_failed = 1;
                }
                break;
            }
            ++valid_count;
        }
        ++slot;
    }

    if (slot == 5) {
        return 0;
    }

    caviar_.reset_objects();
    vehdraw_construct_reactor(ConsoleGlobal.faction_id_,
                              field_2B88_,
                              field_2B8C_, slot,
                              field_2B94_,
                              &caviar_);
    caviar_.set_scaling(0.2f);

    Buffer *const buf = static_cast<Buffer *>(this);
    const int cy = (a2->bottom + a2->top) / 2;
    const int cx = (a2->right + a2->left) / 2;

    if (has_tech_failed != 0) {
        caviar_.render_mono(buf, cx, cy, 0xB9, 6);
        return has_tech_failed == 0;
    }

    caviar_.render(buf, cx, cy, 6);

    buf->set_text_color(*PartLabelColorA1, *PartLabelColorA2, 1, 1);

    Font *const font1 = &font1_;
    RECT rect1;
    rect1.left = a2->left;
    rect1.top = a2->top;
    rect1.right = a2->right;
    rect1.bottom = a2->top + font1->line_advance();
    buf->set_font(font1, 0, 0, 0);

    StringTemp[0] = 0;
    strcat(StringTemp,
           StringTable.get(*reinterpret_cast<const int *>(
               &Reactor[slot - 1].name)));
    CharUpperA(StringTemp);
    buf->wrap_cent(StringTemp, &rect1);

    buf->set_text_color(*PartLabelColorB1, *PartLabelColorB2, 1, 1);

    RECT rect2;
    rect2.left = a2->left;
    rect2.right = a2->right;
    rect2.bottom = a2->bottom;
    rect2.top = rect2.bottom - font1->line_advance();

    StringTemp[0] = 0;
    strcat(StringTemp, StringTable.get(*reinterpret_cast<const int *>(
                           Labels.strings_ptr + 0x1E4)));
    buf->write_l(StringTemp, &rect2,
                 static_cast<int>(strlen(StringTemp)));

    buf->set_font(&font2_, 0, 0, 0);

    char itoa_buf[80];
    StringTemp[0] = 0;
    _itoa(slot, itoa_buf, 10);
    strcat(StringTemp, itoa_buf);
    buf->write_right_l(StringTemp, &rect2,
                       static_cast<int>(strlen(StringTemp)));

    return has_tech_failed == 0;
}

// Armor availability is RulesChassis::missile. The armor prerequisite walk
// uses the existing 14-record Armor[] owner and its preq_tech member.

/*
Purpose: Draw one armor entry of the design parts list: missile chassis select
         armor zero; otherwise find the a1'th enabled armor record. Build the
         armor into the window's Caviar renderer (the fifth slot through the
         multi-table destination renderer, the rest plainly, greyed when the
         faction lacks the tech), then write the record's upper-cased name
         centred above the cell and the label/level pair below it.

// ORIGINAL: 0x00433170 ?draw_armor@SelectPartWin@@QAEHHPAURECT@@H@Z 0x00433170-0x0043351C FILE
// symbol   ?draw_armor@SelectPartWin@@QAEHHPAUtagRECT@@H@Z
// size      940 bytes
// prototype int (__thiscall ?draw_armor@SelectPartWin@@QAEHHPAURECT@@H@Z)(SelectPartWin* this, int, int, RECT*, int)
// callers   1   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// notes     Seed provenance (TRIED): the goto-literal transliteration
//           reaches MISMATCH #5 (prologue push order) - a register
//           allocation difference, not chased further. Not measured exact.
Return Value: 1 the entry was drawn, 0 the index ran past the table
Status: Coverage transcription - unclaimed body; historical seed at
        git:08336bcc:src/unrecovered/00433170.cpp.
*/
int SelectPartWin::draw_armor(int a1, RECT *a2, int a3) {
    Buffer *const buf444 = static_cast<Buffer *>(this);
    Caviar *const cav2b9c = &caviar_;

    int slot;
    int noTechFlag = 0;
    RECT rectScratch;

    const int chassis = field_2B88_;
    const unsigned char flagByte = Chassis[chassis].missile;

    if (flagByte != 0) {
        slot = 0;
    } else {
        const int target = a1;
        int idx = 0;
        int activeCount = 0;
        const RulesArmor *entry = Armor;
        bool matched = false;
        for (;;) {
            if (entry->preq_tech > -2) {
                if (activeCount == target) {
                    slot = idx;
                    const short techShort = entry->preq_tech;
                    if (has_tech(techShort,
                                 field_2B68_) ==
                        0) {
                        noTechFlag = 1;
                    }
                    matched = true;
                    break;
                }
                activeCount++;
            }
            ++entry;
            idx++;
            if (entry >= Armor + MaxArmorNum) {
                break;
            }
        }
        if (!matched) {
            slot = idx;
        }
    }

    if (slot == 0xE) {
        return 0;
    }

    if (slot != 0) {
        const int v2b8c = field_2B8C_;
        const int v2b88 = field_2B88_;
        caviar_.reset_objects();
        vehdraw_construct_armor(ConsoleGlobal.faction_id_, v2b88, v2b8c, slot, 1,
                                cav2b9c);
        cav2b9c->set_scaling(0.1f);
    }

    if (noTechFlag != 0) {
        if (slot != 0) {
            const int midY = (a2->bottom + a2->top) / 2;
            const int midX = (a2->right + a2->left) / 2;
            cav2b9c->render_mono(buf444, midX, midY, 0xB9, 6);
            return noTechFlag == 0;
        }
    } else if (slot != 0) {
        const int midY = (a2->bottom + a2->top) / 2;
        const int midX = (a2->right + a2->left) / 2;
        if (slot == 5) {
            cav2b9c->render_multi_table_dest(buf444, midX, midY, 0x18,
                reinterpret_cast<unsigned char *>(0x6EEF7C), 6);
        } else {
            cav2b9c->render(buf444, midX, midY, 6);
        }
    }

    buf444->set_text_color(*PartLabelColorA1, *PartLabelColorA2, 1, 1);
    {
        Font *const font45f4 = &font1_;
        rectScratch.left = a2->left;
        rectScratch.top = a2->top;
        rectScratch.right = a2->right;
        rectScratch.bottom = a2->bottom;
        rectScratch.bottom = rectScratch.top + font45f4->line_advance();
        buf444->set_font(0, 0, 0, font45f4);
    }
    {
        const int slot16 = slot << 4;
        StringTemp[0] = 0;
        strcat(StringTemp,
               StringTable.get(
                   *reinterpret_cast<const int *>(&Armor[slot].name)));
        CharUpperA(StringTemp);
        buf444->wrap_cent(StringTemp, &rectScratch);
        buf444->set_text_color(*PartLabelColorB1, *PartLabelColorB2, 1, 1);

        Font *const font45f4b = &font1_;
        rectScratch.left = a2->left;
        rectScratch.top = a2->top;
        rectScratch.right = a2->right;
        rectScratch.bottom = a2->bottom;
        rectScratch.top =
            a2->bottom - font45f4b->line_advance();

        const unsigned char flag2 = Armor[slot].mode;
        if (flag2 < 3) {
            strcat(StringTemp,
                   StringTable.get(*reinterpret_cast<const int *>(
                       Labels.strings_ptr + 0x1F8)));
        }
        buf444->write_l(StringTemp, &rectScratch,
                        static_cast<int>(strlen(StringTemp)));

        const signed char armLevel = Armor[slot].defense_rating;
        StringTemp[0] = 0;
        if (armLevel < 0) {
            strcat(StringTemp,
                   StringTable.get(*reinterpret_cast<const int *>(
                       Labels.strings_ptr + 0x310)));
        } else {
            char itoaBuf[32];
            _itoa(armLevel, itoaBuf, 10);
            strcat(StringTemp, itoaBuf);
        }

        buf444->set_font(0, 0, 0,
                         &font2_);
        buf444->write_right_l(StringTemp, &rectScratch,
                              static_cast<int>(strlen(StringTemp)));
    }

    return noTechFlag == 0;
}

// SelectPartWin::draw_weapon's own slot: the selected weapon's sprite index
// the mode-0xB rows read (0x009AB9CE, signed char). Nothing else names it.
static signed char *const WeaponSpriteIndexSlot = (signed char *)0x009AB9CE;

/*
Purpose: Draw one weapon entry of the design parts list: find the a1'th
         weapon whose preq_tech is enabled, build it into the window's
         Caviar renderer - the five special-cased prototypes through their
         CaviarData definitions, the rest through vehdraw_construct_weapon
         with the class-dependent scale - overlay the class icon sprite,
         grey the render when the tech is missing, and write the weapon's
         upper-cased name, its attack/defense label and its count.

// ORIGINAL: 0x00432CC0 ?draw_weapon@SelectPartWin@@QAEHHPAURECT@@H@Z 0x00432CC0-0x00433167 FILE
// symbol   ?draw_weapon@SelectPartWin@@QAEHHPAUtagRECT@@H@Z
// size      1191 bytes
// prototype int (__thiscall ?draw_weapon@SelectPartWin@@QAEHHPAURECT@@H@Z)(SelectPartWin* this, int, int, RECT*, int)
// callers   1   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// notes     Seed provenance (TRIED): direct transcription of the Ghidra
//           decompile; diverges at instruction #0 on the frame-pointer vs
//           ESP-frame prologue shape, not control flow. Not measured exact.
Return Value: 1 the entry was drawn, 0 the index ran past the table
Status: Coverage transcription - unclaimed body; historical seed at
        git:08336bcc:src/unrecovered/00432cc0.cpp.
*/
int SelectPartWin::draw_weapon(int a1, RECT *a2, int a3) {
    int visibleCount = 0;
    int weaponMissing = 0;
    int idx = 0;

    do {
        if (Weapon[idx].preq_tech > -2) {
            if (visibleCount == a1) {
                visibleCount =
                    has_tech(Weapon[idx].preq_tech,
                             field_2B68_)
                        ? 1
                        : 0;
                if (visibleCount == 0) {
                    weaponMissing = 1;
                }
                break;
            }
            visibleCount = visibleCount + 1;
        }
        idx = idx + 1;
    } while (idx < 0x1A);
    if (idx == 0x1A) {
        return 0;
    }

    caviar_.reset_objects();
    Caviar *const caviar = &caviar_;
    float scale_imm;
    if (idx == 0) {
        caviar->add_object(&g_VW00_CAVIARDATA, 0);
        scale_imm = 0.2f;
    } else if (idx == 0x10) {
        caviar->add_object(&g_VM13_CAVIARDATA, 0);
        scale_imm = 0.06f;
    } else if (idx == 0x17) {
        caviar->add_object(&g_VM_CAVIARDATA, 0);
        scale_imm = 0.06f;
    } else if (idx == 0x18) {
        caviar->add_object(&g_VM_CAVIARDATA, 0);
        caviar->add_object(&g_TECLOAD_CAVIARDATA, 0);
        scale_imm = 0.06f;
    } else if (idx == 0x19) {
        caviar->add_object(&g_VM_CAVIARDATA, 0);
        caviar->add_object(&g_FUNLOAD_CAVIARDATA, 0);
        scale_imm = 0.06f;
    } else {
        vehdraw_construct_weapon(field_2B68_, -1,
                                 idx, caviar);
        const unsigned char weapon_class = Weapon[idx].mode;
        if (weapon_class < 3) {
            scale_imm = 0.2f;
        } else if (weapon_class == 9) {
            scale_imm = 0.08f;
        } else if (weapon_class == 0xB) {
            scale_imm = 0.2f;
        } else {
            scale_imm = 0.1f;
        }
    }
    caviar->set_scaling(scale_imm);

    const unsigned char cat_byte = Weapon[idx].mode;
    int sprite_idx = -1;
    bool have_sprite_idx = false;
    if (cat_byte == 8 || cat_byte == 9) {
        sprite_idx = Weapon[idx].icon;
        have_sprite_idx = true;
    } else if (cat_byte == 0xB) {
        sprite_idx = *WeaponSpriteIndexSlot;
        have_sprite_idx = true;
    }
    if (have_sprite_idx && sprite_idx >= 0) {
        Buffer *const buf_target = static_cast<Buffer *>(this);
        Sprite *const spr =
            reinterpret_cast<Sprite *>(0x778400 + sprite_idx * 0x160);
        const int x_coord = ((a2->right - a2->left) - 100) / 2 + a2->left;
        const int y_coord = ((a2->bottom - a2->top) - 0x4B) / 2 + a2->top;
        spr->draw(buf_target, spr->transparent_index(), x_coord, y_coord);
    }

    if (weaponMissing != 0) {
        Buffer *const buf_target = static_cast<Buffer *>(this);
        caviar->render_mono(buf_target, (a2->right + a2->left) / 2,
                            (a2->bottom + a2->top) / 2, 0xB9, 6);
        return weaponMissing == 0;
    }

    caviar->render(static_cast<Buffer *>(this),
                   (a2->right + a2->left) / 2, (a2->bottom + a2->top) / 2, 6);

    Buffer *const panel_buf = static_cast<Buffer *>(this);
    panel_buf->set_text_color(*PartLabelColorA1, *PartLabelColorA2, 1, 1);

    Font *const font45f4 = &font1_;
    RECT tmp_rect;
    tmp_rect.left = a2->left;
    tmp_rect.top = a2->top;
    tmp_rect.right = a2->right;
    tmp_rect.bottom = a2->top + font45f4->line_advance();
    StringTemp[0] = 0;
    strcat(StringTemp,
           StringTable.get(*reinterpret_cast<const int *>(&Weapon[idx].name)));
    panel_buf->set_font(font45f4, 0, 0, 0);
    CharUpperA(StringTemp);
    panel_buf->wrap_cent(StringTemp, &tmp_rect);
    panel_buf->set_text_color(*PartLabelColorB1, *PartLabelColorB2, 1, 1);

    tmp_rect.left = a2->left;
    tmp_rect.right = a2->right;
    tmp_rect.bottom = a2->bottom;
    tmp_rect.top = tmp_rect.bottom - font45f4->line_advance();
    StringTemp[0] = 0;
    strcat(StringTemp,
           StringTable.get(*reinterpret_cast<const int *>(
               Labels.strings_ptr + (cat_byte < 3 ? 0x204 : 0x200))));
    panel_buf->write_l(StringTemp, &tmp_rect,
                       static_cast<int>(strlen(StringTemp)));

    panel_buf->set_font(&font2_, 0, 0, 0);
    signed char count_char = Weapon[idx].offense_rating;
    StringTemp[0] = 0;
    if (count_char == -1) {
        strcat(StringTemp,
               StringTable.get(*reinterpret_cast<const int *>(
                   Labels.strings_ptr + 0x310)));
    } else {
        char num_buf[80];
        if (cat_byte > 2) {
            count_char = Weapon[idx].cost;
        }
        _itoa(count_char, num_buf, 10);
        strcat(StringTemp, num_buf);
    }
    panel_buf->write_right_l(StringTemp, &tmp_rect,
                             static_cast<int>(strlen(StringTemp)));

    return weaponMissing == 0;
}

// These mappings describe the compiler-generated deletion and secondary-base
// adjustment for the real class above; no handwritten thunk is installed.
/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GSelectPartWin@@UAEPAXI@Z: run the complete destructor, then
         release the storage through the executable's operator delete only when
         bit 0 of the flags asks.
// ORIGINAL: 0x0043EDE0 ??_GSelectPartWin@@UAEPAXI@Z 0x0043EDE0-0x0043EE01 FILE BYTE_EXACT
// symbol    ??_GSelectPartWin@@UAEPAXI@Z
// size      33 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0043EE10 0x0064557F
Return Value: the object pointer
Status: Complete
*/

/*
Purpose: Step the receiver back to the subobject ??_GSelectPartWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0043EFB0 ??_GSelectPartWin@@WEEE@AEPAXI@Z 0x0043EFB0-0x0043EFBB FILE BYTE_EXACT
// symbol    ??_ESelectPartWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3SelectPartWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0043EDE0` into
//   ??_GSelectPartWin@@UAEPAXI@Z, which executes `ret 4`; no stack
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
