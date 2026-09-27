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
#include "stdafx.h"
#include "reportwin.h"
#include "reportif.h"  // ReportIfGlobal - the singleton on_key_click re-reports
#include "menu.h"
#include "win.h"
#include "sprite.h"  // g_IFACE_TECH_TREE_ARROW_SPRITES - labs_line's arrow
#include "datalink.h"  // DatalinkMain - the research screen's host window
#include "maininterface.h"  // MainInterfaceGlobal - the set_modal owner
#include "vector_teardown.h"

const void *const ReportWinFlatButtonCtor = (const void *)0x00607CF0;
const void *const ReportWinFlatButtonDtor = (const void *)0x00406880;

// The process-wide report window, 0x00876478 - see reportwin.h. The image's
// dynamic initializer at 0x00496930 constructs it through the real recovered
// constructor below, so the storage sits here, ahead of the two list boxes.
ReportWin ReportWinGlobal;  // 0x00876478

// The topic panel's two process-wide selection list boxes - see reportwin.h.
// Nothing in mapped code constructs them (the image leaves them zero until
// ReportWin's own init fills the panel), and ListBox's constructor here is
// the empty inline the tree models, so the objects are that zero storage.
ListBox ReportWinListBox1;  // 0x00890208
ListBox ReportWinListBox2;  // 0x00890D5C

// maininterface.cpp's interface-id bindings, restated per file (the
// designwin.cpp convention): IfaceIdToSlot maps the active interface id to
// its sub-interface slot, ActiveIfaceId is the id.
static int *const IfaceIdToSlot = (int *)0x007AE778;
static int *const ActiveIfaceId = (int *)0x007D392C;

// 0x008A4160 - the faction the report is displayed for; ReportIf::report's
// retained unit names its second argument factionID, and on_key_click passes
// this dword there. No catalogue name.
static int *const ReportFactionId = (int *)0x008A4160;

// 0x005398E0 ?pop_wait@@YAHXZ - defined in faction.cpp (BYTE_EXACT); no
// header declares it, and the set_modal popups pass it as their wait
// callback (the designwin.cpp spelling).
int __cdecl pop_wait();

/*
Purpose: Construct every sub-object in image order - a FlatButton[7] run, a
         Spot, a ListBox(1), nine individually-constructed FlatButtons split
         by two ButtonGroups, a Scroll, three Times, two Buffers and a Flic -
         then install ReportWin's own vftables.
// ORIGINAL: 0x004AD6B0 ??0ReportWin@@QAE@XZ 0x004AD6B0-0x004AD838;0x006592F0-0x00659419
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
// ListBox has no one-argument constructor declared and Flic's own recovery
// (0x00629D60) is out of this batch's scope, so both are called directly at
// their image address, the same treatment pending_bodies.cpp gives every
// not-yet-promoted callee.
ReportWin::ReportWin() {
    new (static_cast<GraphicWin *>(this)) GraphicWin();

    // flatButtons1_ and spot_ are real members now (see reportwin.h) and
    // build implicitly, in declaration order, before this body runs.

    typedef void(__fastcall *pending_listbox_ctor)(void *, void *, int);
    reinterpret_cast<pending_listbox_ctor>(0x00609DB0)(listBox_, nullptr, 1);

    new (flatButton2_) FlatButton();
    new (flatButton3_) FlatButton();
    new (buttonGroup1_) ButtonGroup();
    new (flatButton4_) FlatButton();
    new (flatButton5_) FlatButton();
    new (flatButton6_) FlatButton();
    new (buttonGroup2_) ButtonGroup();
    new (flatButton7_) FlatButton();
    new (flatButton8_) FlatButton();
    new (flatButton9_) FlatButton();
    new (scroll_) Scroll();
    new (time1_) Time();
    new (time2_) Time();
    new (time3_) Time();
    new (buffer1_) Buffer();
    new (buffer2_) Buffer();

    typedef void(__fastcall *pending_flic_ctor)(void *, void *);
    reinterpret_cast<pending_flic_ctor>(0x00629D60)(flic_, nullptr);

}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004980F0 ?on_mouse_move@ReportWin@@QAEXHH@Z 0x004980F0-0x004980F3 BYTE_EXACT
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

/*
Purpose: Unimplemented dependency - the retained draw_energy unit is not
         recovered yet; this body only keeps on_selected linking until its
         own body lands.
// ORIGINAL: 0x0049BCD0 ?draw_energy@ReportWin@@QAEXXZ 0x0049BCD0-0x0049D481 FILE
// size      6065 bytes
// prototype void (__thiscall ?draw_energy@ReportWin@@QAEXXZ)(ReportWin* this)
// callers   3   call targets   30
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00445130 0x0048C060 0x004E3A00 0x004F6510 0x0050BA00 0x00590910 0x005DACB0 0x005DACE0 0x005DAD10 0x005DCEA0 0x005DD130 0x005DD300 0x005DDF60 0x005DE1D0 0x005E2DD3 0x00609470 0x00609C60 0x00609C90 0x00609CC0 0x00609CF0 0x00609F20 0x00609FA0 0x0060A670 0x0060C920 0x006169A0 0x00625E30 0x00625EC0 0x006453E0 0x00645470 0x0064FC88
Status: Link-only placeholder from src/unrecovered/0049bcd0.cpp; original behavior is not implemented.
*/
void ReportWin::draw_energy() {
    // BODY GOES HERE.
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
// ORIGINAL: 0x0049D490 ?on_selected@ReportWin@@QAEXHH@Z 0x0049D490-0x0049D535 FILE
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
        ReportWinListBox1.set_selected_pos(-1, 1);
        ReportWinListBox2.set_selected_id(selection_id);
    } else {
        if (selection_id > 0x5F5E0FD) {
            reinterpret_cast<ListBox *>(listBox_)->set_selected_id(99999999);
            return;
        }
        field_59AC_ = selection_id - 0x400;
        field_59B0_ = 1;
        ReportWinListBox1.set_selected_id(selection_id);
        ReportWinListBox2.set_selected_pos(-1, 1);
    }
    reset();
    draw_energy();
    soft_update();
}

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
// ORIGINAL: 0x00498280 ?on_key_click@ReportWin@@QAEHHH@Z 0x00498280-0x00498691 FILE
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
        int report_code;
        if (code == 0x71) {
            ReportIfGlobal.field_8_ = 2;
            report_code = 2;
        } else if (code == 0x72) {
            ReportIfGlobal.field_8_ = 3;
            report_code = 3;
        } else if (code == 0x73) {
            ReportIfGlobal.field_8_ = 4;
            report_code = 4;
        } else if (code == 0x74) {
            ReportIfGlobal.field_8_ = 5;
            report_code = 5;
        } else if (code == 0x75) {
            ReportIfGlobal.field_8_ = 6;
            report_code = 6;
        } else if (code == 0x76) {
            ReportIfGlobal.field_8_ = 7;
            report_code = 7;
        } else {
            ReportIfGlobal.field_8_ = 8;
            report_code = 8;
        }

        if (IfaceIdToSlot[*ActiveIfaceId] == 5) {
            ReportIfGlobal.report(report_code, *ReportFactionId, 0);
            return 1;
        }
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
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

        int report_code;
        switch (remainder) {
        case 0:
            ReportIfGlobal.field_8_ = 2;
            report_code = 2;
            break;
        case 1:
            ReportIfGlobal.field_8_ = 3;
            report_code = 3;
            break;
        case 2:
            ReportIfGlobal.field_8_ = 4;
            report_code = 4;
            break;
        case 3:
            ReportIfGlobal.field_8_ = 5;
            report_code = 5;
            break;
        case 4:
            ReportIfGlobal.field_8_ = 6;
            report_code = 6;
            break;
        case 5:
            ReportIfGlobal.field_8_ = 7;
            report_code = 7;
            break;
        default:
            ReportIfGlobal.field_8_ = 8;
            report_code = 8;
            break;
        }

        if (IfaceIdToSlot[*ActiveIfaceId] == 5) {
            ReportIfGlobal.report(report_code, *ReportFactionId, 0);
            return 1;
        }
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
        return 1;
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
        reinterpret_cast<Scroll *>(scroll_)->set_pos(field_C0FC_);
        redraw();
    }
    return 1;
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

The first branch (first_id == fields[0x1654]) draws the tag-to-line
connector and returns; the three middle branches draw the line's right
half for each matched lab pair member; the common tail draws the line box
and the arrow head.
// ORIGINAL: 0x0049B450 ?labs_line@ReportWin@@QAEXHHHHHHH@Z 0x0049B450-0x0049BCC7 FILE
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
    // The declared member block as dwords: fields[0x1653..0x165C] are
    // field_594C_..field_5970_, fields[0x166A] is field_59A8_, and
    // fields[0x3896..0x3899] is the work RECT at field_E258_.
    int *fields = reinterpret_cast<int *>(this);

    // The research screen draws on the Datalink's own surface; every other
    // interface draws on this window's. Both hosts are GraphicWin-derived,
    // so the Buffer base sits at +0x444 either way.
    Buffer *buffer;
    if (IfaceIdToSlot[*ActiveIfaceId] != 8) {
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

    RECT *work_rect = reinterpret_cast<RECT *>(&field_E258_);

    int line_id = fields[0x1654];
    int half_height = line_height / 2;
    int centre_y = fields[0x1656] + half_height;

    int tail_bottom, tail_right;

    if (first_id == line_id) {
        int line_left = fields[0x1653];
        int centre_x = (line_left + line_id + x_offset) / 2;
        if (branch_id == fields[0x165A] || branch_id == fields[0x165B]) {
            fields[0x3896] = line_left + 3 + x_offset;
            fields[0x3897] = fields[0x165A] + half_height;
            fields[0x3898] = centre_x - 2;
            fields[0x3899] = fields[0x165B] + half_height;
            if (branch_id == fields[0x165A]) {
                fields[0x3899] = centre_y;
            } else {
                fields[0x3897] = centre_y + 1;
            }
            buffer->set_clip(work_rect);
            fields[0x3899] = fields[0x165B] + half_height;
            fields[0x3896] = fields[0x1653] - 4 + x_offset;
            fields[0x3897] = fields[0x165A] + half_height;
            fields[0x3898] = centre_x - 2;
            box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                            : &InterfaceBoxStyles[5];
            buffer->box_sprite(work_rect, box_style);
        } else {
            int left_edge = line_left - 4 + x_offset;
            fields[0x3898] = centre_x + 5;
            fields[0x3897] = centre_y;
            fields[0x3896] = left_edge;
            fields[0x3899] = centre_y + 0x14;
            fields[0x3896] = left_edge + 7;
            fields[0x3898] -= 7;
            fields[0x3899] -= 7;
            buffer->set_clip(work_rect);
            fields[0x3896] -= 7;
            fields[0x3898] += 7;
            fields[0x3899] += 7;
            box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                            : &InterfaceBoxStyles[5];
            buffer->box_sprite(work_rect, box_style);
        }
        buffer->restore_clip();
        fields[0x3896] = centre_x - 9;
        fields[0x3897] = centre_y;
        fields[0x3898] = line_id + 4;
        fields[0x3899] = centre_y + 0x14;
        fields[0x3896] += 7;
        fields[0x3898] -= 7;
        fields[0x3899] -= 7;
        buffer->set_clip(work_rect);
        fields[0x3896] -= 7;
        fields[0x3898] += 7;
        fields[0x3899] += 7;
        box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                        : &InterfaceBoxStyles[5];
        buffer->box_sprite(work_rect, box_style);
        buffer->restore_clip();
        arrow->draw(draw_buffer, sprite_byte, line_id - 10,
                    fields[0x1656] - 6 + half_height);
        return;
    }

    int line_right = fields[0x1655];
    int mid_x = (fields[0x166A] + line_right + line_id) / 2;
    int mid_x2 = mid_x - 2;

    if (second_id == fields[0x1657] || second_id == fields[0x1658]) {
        fields[0x3896] = mid_x2;
        fields[0x3897] = fields[0x1657] + half_height;
        fields[0x3898] = line_right - 3;
        fields[0x3899] = fields[0x1658] + half_height;
        int draw_x, draw_y;
        if (second_id == fields[0x1657]) {
            fields[0x3899] = centre_y;
            draw_x = line_right;
            draw_y = fields[0x1657] - 6;
        } else {
            fields[0x3897] = centre_y + 1;
            draw_x = line_right;
            draw_y = fields[0x1658] - 7;
        }
        arrow->draw(draw_buffer, sprite_byte, draw_x - 10,
                    draw_y + half_height);
        buffer->set_clip(work_rect);
        tail_right = line_right + 4;
        tail_bottom = fields[0x1658] + half_height;
        fields[0x3897] = fields[0x1657] + half_height;
        fields[0x3896] = mid_x2;
    } else if (second_id == fields[0x1659] || second_id == fields[0x165C]) {
        fields[0x3896] = mid_x2;
        fields[0x3897] = fields[0x1659] + half_height;
        fields[0x3898] = line_right - 3;
        fields[0x3899] = fields[0x165C] + half_height;
        int draw_x, draw_y;
        if (second_id == fields[0x1659]) {
            fields[0x3899] = centre_y;
            draw_x = line_right;
            draw_y = fields[0x1659] - 6;
        } else {
            fields[0x3897] = centre_y + 1;
            draw_x = line_right;
            draw_y = fields[0x165C] - 7;
        }
        arrow->draw(draw_buffer, sprite_byte, draw_x - 10,
                    draw_y + half_height);
        buffer->set_clip(work_rect);
        tail_right = line_right + 4;
        tail_bottom = fields[0x165C] + half_height;
        fields[0x3897] = fields[0x1659] + half_height;
        fields[0x3896] = mid_x2;
    } else if (second_id != fields[0x165A] && second_id != fields[0x165B]) {
        arrow->draw(draw_buffer, sprite_byte, line_right - 10,
                    centre_y - 6);
        fields[0x3898] = line_right + 4;
        fields[0x3897] = centre_y;
        fields[0x3896] = mid_x - 9;
        fields[0x3899] = centre_y + 0x14;
        fields[0x3896] += 7;
        fields[0x3898] -= 7;
        fields[0x3899] -= 7;
        buffer->set_clip(work_rect);
        fields[0x3896] -= 7;
        fields[0x3898] += 7;
        fields[0x3899] += 7;
        goto common_tail;
    } else {
        fields[0x3896] = mid_x2;
        fields[0x3897] = fields[0x165A] + half_height;
        fields[0x3898] = line_right - 3;
        fields[0x3899] = fields[0x165B] + half_height;
        int draw_x, draw_y;
        if (second_id == fields[0x165A]) {
            fields[0x3899] = centre_y;
            draw_x = line_right;
            draw_y = fields[0x165A] - 6;
        } else {
            fields[0x3897] = centre_y + 1;
            draw_x = line_right;
            draw_y = fields[0x165B] - 7;
        }
        arrow->draw(draw_buffer, sprite_byte, draw_x - 10,
                    draw_y + half_height);
        buffer->set_clip(work_rect);
        tail_right = line_right + 4;
        tail_bottom = fields[0x165B] + half_height;
        fields[0x3897] = fields[0x165A] + half_height;
        fields[0x3896] = mid_x2;
    }
    fields[0x3898] = tail_right;
    fields[0x3899] = tail_bottom;
common_tail:
    box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                    : &InterfaceBoxStyles[5];
    buffer->box_sprite(work_rect, box_style);
    buffer->restore_clip();
    {
        int left_edge = fields[0x166A] - 4 + fields[0x1654];
        fields[0x3896] = left_edge;
        fields[0x3897] = centre_y;
        if (style_select == 0) {
            fields[0x3898] = mid_x + 5;
            fields[0x3899] = centre_y + 0x14;
            fields[0x3896] += 7;
            fields[0x3898] -= 7;
            fields[0x3899] -= 7;
            buffer->set_clip(work_rect);
            box_style = &InterfaceBoxStyles[3];
            fields[0x3896] -= 7;
            fields[0x3898] += 7;
            fields[0x3899] += 7;
        } else {
            fields[0x3898] = mid_x + 5;
            fields[0x3899] = centre_y + 0x14;
            fields[0x3896] = left_edge + 7;
            fields[0x3898] -= 7;
            fields[0x3899] -= 7;
            buffer->set_clip(work_rect);
            box_style = &InterfaceBoxStyles[5];
            fields[0x3896] -= 7;
            fields[0x3898] += 7;
            fields[0x3899] += 7;
        }
    }
    buffer->box_sprite(work_rect, box_style);
    buffer->restore_clip();
}
