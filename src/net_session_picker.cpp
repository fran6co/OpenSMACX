#include "stdafx.h"
#include "net_session_picker.h"

#include <cstdlib>
#include <cstring>

#include "buffer.h"
#include "basepop.h"
#include "dialog.h"
#include "dialogs.h"
#include "general.h"
#include "main.h"
#include "strings.h"
#include "temp.h"
#include "win.h"
#include "xpops.h"

// The pinned 0x63C510 parse_says input is the zero-initialized byte at
// 0x009BEB18. It has no other immediate-address reference in the image and
// its only observed use is as an empty-string input. Extent is one accessed
// byte only; the start binding does not assert a larger allocation.
// RECOVERY_OBJECT: v1 0x009BEB18 ?SessionPickerPasswordPrompt@@3DA
char SessionPickerPasswordPrompt;

// ORIGINAL: 0x0063BB20 sub_63bb20 0x0063BB20-0x0063BD06;0x00663940-0x00663998 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ??0NetSessionPickerWin@@QAE@XZ
// TRIED: __cdecl no-arg contract - disasm has ecx=this (mov esi,ecx), no stack args, plain ret; used __fastcall(void*) instead (the free-thiscall-avoids-C4234 pattern from 005cb050.cpp). Placement-new construction of the 6 sub-objects (GraphicWin, Time, Heap, ListBox(1), PushButton x3) at their disasm offsets DOES reproduce the SEH frame's first two pushes (push -1; push funclet) - divergence starts at #2, likely an extra EBP-frame instruction from added local temporaries.
NetSessionPickerWin::NetSessionPickerWin() {
  // C++ constructs the GraphicWin and every real member above this body.
  // AlphaNetLedger owns its own list/vbase stages; do not reproduce the seed's
  // manual table stores or placement construction here.
  net_ = 0;
  tail_.field_3710_ = 2;
  tail_.row_count_ = 2;
  tail_.action_result_ = 0;
  tail_.field_3708_ = 0;
  tail_.field_370c_ = 0;
  tail_.field_3714_ = 0;
  tail_.field_3718_ = 7;
  tail_.frame_color_ = 0xff;
  tail_.box_color_a_ = 0;
  tail_.box_color_b_ = 0;
  tail_.field_373c_ = 0;
  tail_.field_3740_ = -1;
  tail_.field_3744_ = 1;
  tail_.field_3748_ = 1;
  tail_.selected_row_color_ = 7;
  tail_.row_box_color_a_ = 0xf7;
  tail_.row_box_color_b_ = 0;
  tail_.boxes_per_row_ = 3;
  tail_.field_3780_ = 0;
  tail_.selected_text_color_a_ = 0xf8;
  tail_.selected_text_color_b_ = 0;
  tail_.selected_text_color_c_ = -1;
  tail_.selected_text_color_d_ = 1;
  tail_.field_3794_ = 1;
}
// ORIGINAL: 0x00633160 sub_633160 0x00633160-0x0063317E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ??_GNetSessionPickerWin@@UAEPAXI@Z

// ORIGINAL: 0x00633740 sub_633740 0x00633740-0x0063374B FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ??_ENetSessionPickerWin@@WEEE@AEPAXI@Z


// ORIGINAL: 0x00633010 sub_633010 0x00633010-0x00633152;0x00663730-0x006637AF FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ??1NetSessionPickerWin@@UAE@XZ
// TRIED: the tail at 0x00663730-0x006637AF is a chain of EH unwind funclets protecting the partial destruction of several embedded members if an earlier member's destructor throws. Reproducing that needs this class's exact multiple/virtual-inheritance layout modelled with real inheritance so the compiler regenerates the funclets itself; the flat, unprotected call sequence is landed instead. The string-list member at +0x36a8 is StringStruct::close_with_tables inlined at the call site with two different literal table pairs, remove_all left as a real call.
// TRIED: the teardown the 0x633160 deleting thunk calls - see the note in `deleting_thunks.h`.
NetSessionPickerWin::~NetSessionPickerWin() {
  // Historical 0x633010 evidence includes the separate 0x663730-0x6637AF
  // EH-unwind tail. The old flat scaffold manually staged vtables and
  // called Heap::shutdown; those are not product operations. Exact
  // compiler-owned partial-unwind behavior and Heap teardown spelling
  // remain fidelity obligations for this real member-lifetime owner.
  // The original dtor calls the same reusable close path; member/base
  // destruction after this body is compiler-owned.
  close();
}

// ORIGINAL: 0x0063BD10 sub_63bd10 0x0063BD10-0x0063BEAD FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?close@NetSessionPickerWin@@QAEXXZ
// TRIED: MISMATCH #0 push/sub - unknown containing class, receiver expressed as an opaque thiscall shim (no field names); stack frame shape differs from the original's spill choices.
void NetSessionPickerWin::close() {
  refresh_timer_.close();
  sessions_.remove_all();
  sessions_.current_position_ = 0;
  sessions_view_.close();
  create_button_.close();
  action_button_.close();
  cancel_button_.close();
  GraphicWin::close();

  tail_.field_3718_ = 7;
  tail_.selected_row_color_ = 7;
  net_ = 0;
  tail_.action_result_ = 0;
  tail_.client_width_ = 0;
  tail_.client_height_ = 0;
  tail_.field_3708_ = 0;
  tail_.field_370c_ = 0;
  tail_.field_3710_ = 2;
  tail_.field_3714_ = 0;
  tail_.frame_color_ = 0xff;
  tail_.box_color_a_ = 0;
  tail_.box_color_b_ = 0;
  tail_.row_count_ = 2;
  tail_.field_373c_ = 0;
  tail_.field_3740_ = -1;
  tail_.field_3744_ = 1;
  tail_.field_3748_ = 1;
  tail_.row_box_color_a_ = 0;
  tail_.row_box_color_b_ = 0;
  tail_.boxes_per_row_ = 2;
  tail_.field_3780_ = 0;
  tail_.selected_text_color_a_ = 0xf8;
  tail_.selected_text_color_b_ = 0;
  tail_.selected_text_color_c_ = -1;
  tail_.selected_text_color_d_ = 1;
  tail_.field_3794_ = 1;
}

// ORIGINAL: 0x0063CD50 sub_63cd50 0x0063CD50-0x0063CDBB FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?draw_frame@NetSessionPickerWin@@QAEXXZ
void NetSessionPickerWin::draw_frame() {
  RECT *rect = &tail_.frame_rect_;
  GraphicWin::fill(rect, tail_.frame_color_);
  int count = tail_.row_count_;
  if (count > 0) {
    int i = 0;
    do {
      Buffer::box(rect, tail_.box_color_a_, tail_.box_color_b_);
      ++rect->left;
      --rect->right;
      ++rect->top;
      --rect->bottom;
      count = tail_.row_count_;
      ++i;
    } while (i < count);
  }
}

// ORIGINAL: 0x0063C9D0 sub_63c9d0 0x0063C9D0-0x0063CA98 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?vslot_14@NetSessionPickerWin@@UAEXXZ
// TRIED: signature __cdecl(void) -> __thiscall member (ecx=this throughout, plain `ret`, class-member wrapper like other anonymous subs). sub_63caa0 called via the scaffold's existing nullary __cdecl decl though the original passes ecx=this too - can't redeclare it (C2733), so that one call is known to diverge. First divergence #0 push/mov, 210 vs 200 B.
void NetSessionPickerWin::vslot_14() {
  // These bytes are the pinned UI literals at 0x698C88 and 0x698C9C. Keep
  // their exact display spelling; no external data address is used.
  static char available_sessions[] = "Available Sessions";
  static char current_players[] = "Current Players";

  Buffer::write_l(available_sessions, &tail_.first_label_rect_,
                  static_cast<unsigned int>(std::strlen(available_sessions)));
  Buffer::write_l(current_players, &tail_.second_label_rect_,
                  static_cast<unsigned int>(std::strlen(current_players)));
  draw_selected_rows();
  draw_frame();
}

// ORIGINAL: 0x0063CAA0 sub_63caa0 0x0063CAA0-0x0063CD4E FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?draw_selected_rows@NetSessionPickerWin@@QAEXXZ
// TRIED: `this` is GraphicWin* (esi passed straight to ?fill@GraphicWin@@ with no offset, and buffer_ member at self+0x444 matches every Buffer:: call site's `lea ecx,[esi+0x444]`), reading a RECT from self+0x374c and driving a nested box()-drawing loop, then (if Dialog::get_selected_id() is nonzero) reinterpreting that int return as a pointer into an unrelated list-cursor structure (+0x20/+0x34/+0x38/+0x3c/+0x40) to walk entries and write_l() their text - transcribed as raw offset arithmetic since that structure does not match the catalogued Dialog field layout at those offsets. Compiles and matches the first 6 instructions; diverges at #6 (original hoists `ebx = esi+0x374c` once and reuses it for all three RECT reads for the whole function, rebuilt recomputes the offset at each use) - register allocation, not source form.
void NetSessionPickerWin::draw_selected_rows() {
  RECT rect = tail_.rows_rect_;
  GraphicWin::fill(&rect, tail_.selected_row_color_);
  rect.bottom = tail_.row_height_ + rect.top;

  int row = 0;
  if (0 < tail_.rows_visible_) {
    do {
      int column = 0;
      if (0 < tail_.boxes_per_row_) {
        do {
          Buffer::box(&rect, tail_.row_box_color_a_, tail_.row_box_color_b_);
          ++rect.left;
          --rect.right;
          ++rect.top;
          --rect.bottom;
          ++column;
        } while (column < tail_.boxes_per_row_);
      }
      const int boxes = tail_.boxes_per_row_;
      rect.left -= boxes;
      rect.right += boxes;
      rect.top = rect.top - boxes + tail_.row_height_;
      rect.bottom = rect.bottom + boxes + tail_.row_height_;
      ++row;
    } while (row < tail_.rows_visible_);
  }

  Dialog &dialog = static_cast<Dialog &>(sessions_view_);
  const int selected_id = dialog.get_selected_id();
  if (selected_id == 0) return;

  // ListBox::item stores the given ID in Dialog::item_list_; the refresh
  // operation passes the exact NetAllocationNode* payload as that ID.
  NetAllocationNode *selected = reinterpret_cast<NetAllocationNode *>(
      static_cast<uintptr_t>(static_cast<unsigned int>(selected_id)));
  NetUnk72C &list = selected->entries_;

  rect.left = tail_.rows_rect_.left;
  rect.right = tail_.rows_rect_.right;
  rect.bottom = tail_.rows_rect_.bottom;
  rect.top = tail_.rows_rect_.top +
      static_cast<int>(selected->max_players_) * tail_.row_height_;
  Buffer::change_color(&rect, tail_.selected_row_color_,
                       tail_.selected_text_color_a_);

  const int inset = tail_.boxes_per_row_;
  rect.left = tail_.rows_rect_.left + inset;
  rect.right = tail_.second_label_rect_.right - inset;
  rect.top = tail_.rows_rect_.top + inset;
  rect.bottom = tail_.row_height_ + tail_.rows_rect_.top - inset;

  if (-2 < list.entry_count_ - 1) {
    list.current_ = list.head_;
    int steps = abs(-1);
    if (steps <= list.entry_count_) {
      steps = abs(-1);
      if (0 < steps) {
        do {
          --steps;
          list.current_ = list.current_->previous;
        } while (steps != 0);
      }
      list.current_position_ = list.entry_count_ - 1;
    }
  }

  Buffer::set_text_color(tail_.selected_text_color_b_,
                         tail_.selected_text_color_c_,
                         tail_.selected_text_color_d_, tail_.field_3794_);

  int i = 0;
  if (0 < list.entry_count_) {
    do {
      if (list.head_ != 0) {
        const int next_position = list.current_position_ + 1;
        list.current_ = list.current_->next;
        list.current_position_ = next_position;
        if (next_position == list.entry_count_) list.current_position_ = 0;
      }
      if (list.current_ != 0) {
        NetTextPayload *payload = list.head_ == 0 ? 0 : list.current_->payload;
        char *text = payload->text;
        if (text != 0) {
          Buffer::write_l(text, &rect,
                          static_cast<unsigned int>(std::strlen(text)));
        }
      }
      rect.top += tail_.row_height_;
      rect.bottom += tail_.row_height_;
      ++i;
    } while (i < list.entry_count_);
  }
}

// ORIGINAL: 0x0063C7C0 sub_63c7c0 0x0063C7C0-0x0063C9B6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?refresh_player_list@NetSessionPickerWin@@QAEXXZ
// TRIED: `__thiscall` function-pointer casts for the dynamically adjusted vtable dispatches (C4234, VC6 refuses them) - routed through a second VCall-style shim (VCallSlot0) instead, one per distinct slot/arity pair, matching the brief's guidance. Did not chase the remaining register-allocation divergence (first mismatch at instruction #2) further given the size and structural complexity of this function (three raw vtable adjustor dispatches plus ListBox::attach/item/update_changes).
void NetSessionPickerWin::refresh_player_list() {
  // The list implementation owns the observed hook/delete/cursor-reset
  // sequence; it preserves the original callback and allocator protocol.
  sessions_.remove_all();
  sessions_.current_position_ = 0;

  // Direct pinned call 0x62E420: ECX is net_, arguments are this exact ledger
  // and a null password. A nonzero result returns before the UI lists change.
  if (net_->enumerate_sessions(&sessions_, 0) != 0) return;

  Dialog &dialog = static_cast<Dialog &>(sessions_view_);
  dialog.item_list_.remove_all();
  dialog.item_list_.current_position_ = 0;
  // The duplicate native call and the following allocator clear are retained;
  // they are not collapsed into a single normalized reset.
  dialog.item_list_.remove_all();
  dialog.item_list_.current_position_ = 0;
  dialog.item_list_.allocator_ = 0;

  static char no_sessions[] = "NO SESSIONS (Translate this)";
  if (sessions_.entry_count_ == 0) {
    sessions_view_.item(no_sessions, 0);
  } else {
    if (sessions_.entry_count_ - 1 >= -1) {
      sessions_.current_ = sessions_.head_;
      int steps = abs(-1);
      if (steps <= sessions_.entry_count_) {
        steps = abs(-1);
        if (steps > 0) {
          do {
            --steps;
            sessions_.current_ = sessions_.current_->previous_;
          } while (steps != 0);
        }
        sessions_.current_position_ = sessions_.entry_count_ - 1;
      }
    }

    int index = 0;
    if (sessions_.entry_count_ > 0) {
      do {
        AlphaNetLedgerEntry *head = sessions_.head_;
        int item_id = 0;
        if (head != 0) item_id = static_cast<int>(reinterpret_cast<uintptr_t>(
            sessions_.current_->payload_));

        if (head != 0) {
          sessions_.current_ = sessions_.current_->previous_;
          const int position = sessions_.current_position_ - 1;
          sessions_.current_position_ = position;
          if (position == -1) {
            sessions_.current_position_ = sessions_.entry_count_ - 1;
          }
        }

        AlphaNetLedgerEntry *current = sessions_.current_;
        char *text = 0;
        if (current != 0) {
          // The original's null-head/inconsistent-current path reads through
          // an invalid payload expression; under the actual circular-list
          // invariant this arm is unreachable. Do not fabricate a fallback
          // label or repair the list state here.
          NetAllocationNode *payload = head == 0 ? 0 : current->payload_;
          text = payload->session_name_;
        }
        sessions_view_.item(text, item_id);
        ++index;
      } while (index < sessions_.entry_count_);
    }
  }

  int result;
  if (HandleMain != 0) {
    result = sessions_view_.update_changes();
  } else {
    // This is the Dialog virtual base entry in the embedded ListBox. The
    // source cast performs the real virtual-base adjustment; no hand offset
    // or shim object stands in for it.
    result = dialog.attach(this, static_cast<int>(dialog.field_24_),
                           static_cast<int>(dialog.field_28_), 0x80000);
  }
  tail_.action_result_ = result;
  if (result != 0) return;
  show(0);
}

// ORIGINAL: 0x0063C340 sub_63c340 0x0063C340-0x0063C38F FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?start_refresh_timer@NetSessionPickerWin@@QAEHXZ
int NetSessionPickerWin::start_refresh_timer() {
  // Native 0x63C340 passes this owner through Time's existing integer
  // callback-context ABI; the old Win attribution read beyond sizeof(Win).
  if (refresh_timer_.start(refresh_timer_callback,
                           static_cast<int>(reinterpret_cast<uintptr_t>(this)),
                           1000, 5) != 0) {
    return 0;
  }
  refresh_player_list();
  set_modal(0, 0, 0);
  hide();
  return net_->field_760_;
}

// ORIGINAL: 0x0063C4E0 sub_63c4e0 0x0063C4E0-0x0063C4F6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?refresh_timer_callback@NetSessionPickerWin@@SAXH@Z
void __cdecl NetSessionPickerWin::refresh_timer_callback(int context) {
  // Keep the image's int payload ABI and the nonzero/focus guard order.
  if (context == 0) return;
  NetSessionPickerWin *owner = reinterpret_cast<NetSessionPickerWin *>(
      static_cast<uintptr_t>(static_cast<unsigned int>(context)));
  if (WinFocusWindow == static_cast<Win *>(owner)) owner->refresh_player_list();
}

// ORIGINAL: 0x0063C390 sub_63c390 0x0063C390-0x0063C39E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?refresh_if_focused@NetSessionPickerWin@@QAEXXZ
void NetSessionPickerWin::refresh_if_focused() {
  if (WinFocusWindow == static_cast<Win *>(this)) refresh_player_list();
}

// ORIGINAL: 0x0063BF10 sub_63bf10 0x0063BF10-0x0063C33F FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?initialize@NetSessionPickerWin@@QAEHPAVNet@@PADPAVFont@@HHH@Z
// TRIED: not __stdcall(int,int,int) as the brief's fallback signature guessed - it's __thiscall(int,char*,Font*,int,int,int) returning int, 6 stack args popped by `ret 0x18`, cross-checked against Ghidra's param numbering (param_1 is `this`). Dialog init/ set_dialog_* calls do NOT address self+0xa50 directly: the embedded Dialog's real base is `self+0xa50+*(*(self+0xa50)+8)`, an owner/index indirection re-derived from the raw disasm since Ghidra's flattened `*(*piVar1+8)+OFFSET+param_1` addressing looked at first like a plain field write. slot062 (0xf8/4) called nullary, no retype needed. sim 0.85 at plain /O2; first divergence #53 is a register-vs-recompute choice in the width/height layout arithmetic, not chased further.
int NetSessionPickerWin::initialize(Net *network, char *caption,
                                    Font *font, int width, int height,
                                    int rows) {
  if (rows == 0) return 3;
  if (network == 0 || font == 0) return 0x10;
  if (!font->is_initialized()) return 7;

  tail_.rows_visible_ = rows;
  net_ = network;
  tail_.client_width_ = width;
  tail_.client_height_ = height;

  const int margin_x = (width * 10) / 600;
  const int margin_y = (height * 10) / 440;
  int result = GraphicWin::init(
      0, 0, width, height, caption, 0x800, BasePopDefaultField322C, 0, 0);
  if (result != 0) return result;

  center();
  Buffer::set_font(font, 0, 0, 0);

  const int stored_width = tail_.client_width_;
  int left = (stored_width * 200) / 600 + stored_width;
  const int scaled_line_height = font->pixel_height() * 7;
  int top = ((scaled_line_height + ((scaled_line_height >> 31) & 3)) >> 2)
      + tail_.client_height_;
  const int inset_width = stored_width - (stored_width * 10) / 600 - left;
  int right = stored_width + inset_width;
  const int vertical_step = (tail_.client_height_ - top - margin_y) * 3;
  top += vertical_step;
  int bottom = vertical_step + tail_.client_height_;
  int button_width = (left + inset_width) - right;
  int button_height = top - bottom;

  result = action_button_.init(BasePopDefaultOkText, -1, right, bottom,
                               button_width, button_height, this, 0);
  if (result != 0) return result;
  action_button_.set_font(font, font, font, font);
  action_button_.redraw();

  top += margin_y;
  bottom += top;
  right += top;
  button_height = bottom - right;

  result = create_button_.init(const_cast<char *>("Create Session (Translate)"),
                               0, right, button_width, bottom, button_height,
                               this, 0);
  if (result != 0) return result;
  create_button_.set_font(font, font, font, font);
  create_button_.redraw();

  bottom += top;
  button_width += top;
  const int third_button_width = button_width - bottom;
  result = cancel_button_.init(BasePopDefaultCancelText, -2, bottom,
                               third_button_width, right, button_height,
                               this, 0);
  if (result != 0) return result;
  cancel_button_.set_font(font, font, font, font);
  cancel_button_.redraw();

  tail_.first_label_rect_.left = margin_x;
  tail_.first_label_rect_.top = margin_y;
  tail_.first_label_rect_.right = margin_x * -2 - cancel_button_.pixel_width()
      + width;
  tail_.first_label_rect_.bottom = margin_y + cancel_button_.pixel_height();

  tail_.second_label_rect_.right = width - margin_x;
  const int second_left = tail_.first_label_rect_.right + margin_x;
  tail_.second_label_rect_.left = second_left;
  const int second_top = tail_.first_label_rect_.bottom;
  tail_.rows_rect_.left = second_left;
  tail_.rows_rect_.right = width - margin_x;
  tail_.rows_rect_.top = second_top;
  tail_.second_label_rect_.top = margin_y;
  tail_.second_label_rect_.bottom = second_top;
  tail_.panel_rect_.left = margin_x;
  tail_.panel_rect_.top = second_top;
  tail_.panel_rect_.right = tail_.first_label_rect_.right;
  tail_.panel_rect_.bottom = height - margin_y;

  const int row_region_height = action_button_.client_rect().right - margin_y * 2;
  tail_.rows_rect_.bottom = row_region_height;
  tail_.row_height_ = (row_region_height - tail_.boxes_per_row_ * 2 - second_top)
      / rows;
  tail_.rows_rect_.bottom = tail_.row_height_ * rows + second_top;

  sessions_view_.close();
  Dialog &dialog = static_cast<Dialog &>(sessions_view_);
  result = dialog.init(&tail_.panel_rect_, 0);
  if (result != 0) return result;

  dialog.field_AC_ = 0;
  dialog.field_B0_ = 0;
  dialog.field_B4_ = 2;
  dialog.field_5C_ = 2;
  dialog.set_dialog_text_color(tail_.field_3714_, -1, 1, 1);
  dialog.set_dialog_text_color2(tail_.field_3718_, -1, 1, 1);
  dialog.set_dialog_font(font, 0, 0);
  return 0;
}

// ORIGINAL: 0x0063C510 sub_63c510 0x0063C510-0x0063C6CA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?connect_session@NetSessionPickerWin@@QAEHXZ
// TRIED: __thiscall helper class (NetConnectHelper) with pop()/parse_says() overload resolution by arg count and a 1-arg VCall::slot000; diverges at #0, frame setup differs
int NetSessionPickerWin::connect_session() {
  parse_says(0, "My Session", -1, -1);
  parse_says(1, "My Name", -1, -1);
  parse_says(2, &SessionPickerPasswordPrompt, -1, -1);

  int status = x_pop_caption_value_flags("jackal", "NETCONNECT_CREATE",
                                         0x3c, 0x44, 0);
  for (;;) {
    if (status == -1) return 1;
    if (ParseStrBuffer[0].str[0] == 0) {
      x_pop_caption_label("jackal", "NETCONNECT_NEED_SESSION", 0);
    } else if (ParseStrBuffer[1].str[0] == 0) {
      x_pop_caption_label("jackal", "NETCONNECT_NEED_PLAYER", 0);
    } else {
      if (ParseStrBuffer[2].str[0] != 0) {
        x_pop_caption_label("jackal", "NETCONNECT_PASSWORD_SET", 0);
      }

      BasePop *popup = PopupAllocHook();
      if (popup == 0) return 4;
      popup->start(const_cast<char *>("jackal"), "NETCONNECT_MAX_PLAYERS",
                   -1, 0, 0x50, 0);

      int faction_count = tail_.rows_visible_;
      char letter = '2';
      for (int i = 2; i < faction_count + 1; ++i) {
        char label[2];
        label[0] = letter;
        label[1] = 0;
        popup->dialogs().item(label, i);
        ++letter;
        faction_count = tail_.rows_visible_;
      }

      const int selected_players = popup->exec(0, 0);
      delete popup;
      if (selected_players != -1) {
        NetAllocationNode *session = net_->create_session(
            ParseStrBuffer[0].str, selected_players, ParseStrBuffer[2].str);
        if (session != 0) {
          const int joined = net_->join_session(
              session, ParseStrBuffer[1].str, 0);
          return joined == 0;
        }
        return 1;
      }
    }
    status = x_pop_caption_value_flags("jackal", "NETCONNECT_CREATE",
                                       0x3c, 0x44, 0);
  }
}

// ORIGINAL: 0x0063C3A0 sub_63c3a0 0x0063C3A0-0x0063C4D6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?on_button_clicked@NetSessionPickerWin@@UAEXH@Z
// TRIED: IDA guessed 1 stack arg; ecx=esi with no matching stack read confirms a __thiscall receiver, its own minimal class (no catalogued name). The vtable-slot-58 call needs its own dummy-padded virtual class (same reason as 0x5ED880), so this stays FILE mode. `if (a2==-2){A}else if(a2!=-1){...}` compiles the -2 case inline (jne to skip past it); the original places that handler in a separate far block and jumps to it directly (je) - a block-layout difference this source form did not reproduce. Landed at MISMATCH #4.
void NetSessionPickerWin::on_button_clicked(int action) {
  if (action == -2) {
    tail_.action_result_ = 0;
    release_modal();
    return;
  }
  if (action != -1) {
    if (action != 0) return;
    if (connect_session() != 0) return;
    release_modal();
    return;
  }

  const int selection = x_pop_caption_value_flags(
      "jackal", "NETCONNECT_JOIN", 0x3c, 0x44, 0);
  if (selection == -1) return;

  Dialog &dialog = static_cast<Dialog &>(sessions_view_);
  sessions_.seek_pos(dialog.selected_position_);
  NetAllocationNode *session = sessions_.head_ == 0
      ? 0 : sessions_.current_->payload_;
  tail_.action_result_ = net_->join_session(
      session, ParseStrBuffer[0].str, 0);
  if (tail_.action_result_ != 0) release_modal();
}

namespace {

// 0x631050 brackets its stack-owned session structures with the shared
// StringAllocationHeap handoff. Declare this before those objects so it
// restores the incoming value after their natural destruction.
struct SessionPickerAllocationHandoff {
  Heap *saved_;
  SessionPickerAllocationHandoff() : saved_(StringAllocationHeap) {
    StringAllocationHeap = 0;
  }
  ~SessionPickerAllocationHandoff() { StringAllocationHeap = saved_; }
};

}  // namespace

// Descriptive method name only. The pinned 0x631050 entry proves a Net
// receiver, but not its original source identifier. The original local window
// is constructed/closed here; the separate modal BasePop is obtained from
// PopupAllocHook and owns the menus and EditGroup fields below.
// ORIGINAL: 0x00631050 sub_631050 0x00631050-0x00631A06;0x00663528-0x0066372B FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Original table entries establish slots 14 and 44. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?session_picker_dialog@Net@@QAEHXZ
// TRIED: no ESP-relative frame (raw buffer + manual calls, not a real frame pointer omission); the entry receiver (`mov edi,ecx`) and Ghidra's `param_2` are not recoverable under the mandated 0-arg `__cdecl` signature, so both are read from placeholder storage instead of a real caller-supplied value; sub_63bb20/ sub_63bd10/sub_4e34b0/sub_4e3350/sub_4080b0/fn_00401c80/ fn_00401ce0/fn_00402970 called with no receiver/args since their declared prototypes are 0-arg extern "C" and changing them would break linkage to the real symbol.
int Net::session_picker_dialog() {
  SessionPickerAllocationHandoff allocation_handoff;
  AlphaNetLedger available_sessions;
  NetSessionPickerWin picker;
  int result = 0;
  NetAllocationNode *selected_session = 0;

  // Native checks Net+0x6E0 before allocating the modal popup. The one
  // 0x63BD10 call in each teardown is the local picker's destructor body,
  // which the compiler expands before natural member/base destruction; do
  // not add a second explicit close here.
  if (buffer_6E0_ == 0) {
    return 0;
  }

  BasePop *popup = PopupAllocHook();
  if (popup == 0) {
    return 4;
  }

  static char choose_title[] = "Would you like to...?";
  static char create_or_join[] = "Start a new session or...";
  static char find_session[] = "Search for an existing one?";
  static char no_sessions[] =
      "No sessions are currently running.  Would you like to try again?";
  static char yes_text[] = "Yes";
  static char no_text[] = "No";
  static char create_text[] = "Create Session";
  static char session_name_text[] = "Session Name:";
  static char player_limit_text[] = "Maximum Players:";
  static char session_password_text[] = "Session Password (Optional):";
  static char player_name_text[] = "Your Name:";
  static char password_required_text[] =
      "Password required for this session:";

  if (popup->init(0x10, 0) != 0) goto session_picker_cleanup;
  popup->string(choose_title);
  popup->dialogs().Dialogs::item(create_or_join, 0);
  popup->dialogs().Dialogs::item(find_session, 1);

  if (popup->exec(0, 0) == 0) goto session_picker_create;

  for (;;) {
    if (enumerate_sessions(&available_sessions, 0) != 0) {
      result = 0;
      goto session_picker_cleanup;
    }

    if (available_sessions.entry_count_ == 0) {
      if (popup->init(0x10, 0) != 0) goto session_picker_cleanup;
      popup->string(no_sessions);
      popup->dialogs().Dialogs::item(yes_text, 0);
      popup->dialogs().Dialogs::item(no_text, 1);
      popup->button(create_text);
      const int retry_choice = popup->exec(0, 0);
      if (popup->input_poll_state() == 1) goto session_picker_create;
      if (retry_choice != 0) {
        // The original calls exit(2) on this affirmative-looking branch.
        exit(2);
      }
      continue;
    }

    if (popup->init(0x42, 0) != 0) goto session_picker_cleanup;
    available_sessions.seek_pos(-1);
    int row = 0;
    while (row < available_sessions.entry_count_) {
      NetAllocationNode *session = available_sessions.next_entry();
      char *name = session == 0 ? 0 : session->session_name_;
      popup->dialogs().Dialogs::item(name, row);
      ++row;
    }
    popup->button(create_text);

    const int selected_row = popup->exec(0, 0);
    if (selected_row == -1) {
      result = 0;
      goto session_picker_cleanup;
    }
    if (popup->input_poll_state() == 1) goto session_picker_create;

    available_sessions.seek_pos(selected_row);
    selected_session = available_sessions.head_ == 0
        ? 0 : available_sessions.current_->payload_;
    goto session_picker_credentials;
  }

session_picker_create:
  for (;;) {
    if (popup->init(0x44, 0) != 0) {
      result = 0;
      goto session_picker_cleanup;
    }
    popup->set_width(0x190);
    char default_players[16];
    _itoa(0x10, default_players, 10);
    popup->dialogs().EditGroup::edit(session_name_text, 0, 0x32);
    popup->dialogs().EditGroup::edit(player_limit_text, default_players, 2);
    popup->dialogs().EditGroup::edit(session_password_text, 0, 0x32);
    // Native 0x63156D resolves the Dialog virtual-base displacement from
    // Dialogs' vbtable, then writes 1 to Dialog::field_4C_. Qualification is
    // required so C++ applies that genuine shared-virtual-base adjustment.
    popup->dialogs().Dialog::field_4C_ = 1;
    ParseStrBuffer[0].str[0] = 0;
    ParseStrBuffer[1].str[0] = 0;
    ParseStrBuffer[2].str[0] = 0;

    if (popup->exec(0, 0) == -1) {
      result = 0;
      goto session_picker_cleanup;
    }
    if (ParseStrBuffer[1].str[0] != 0 &&
        atoi(ParseStrBuffer[1].str) > 0x10) {
      continue;
    }

    // The native local begins at frame+0x58 and the following integer-to-text
    // buffer begins at frame+0x70, leaving 0x18 bytes for this unchecked
    // strcat destination. Preserve that small local rather than widening it.
    char session_name[0x18];
    session_name[0] = 0;
    strcat(session_name, ParseStrBuffer[0].str);
    char *password = ParseStrBuffer[2].str[0] == 0
        ? 0 : ParseStrBuffer[2].str;
    NetAllocationNode *created = create_session(
        session_name, atoi(ParseStrBuffer[1].str), password);
    if (created == 0) goto session_picker_cleanup;
    selected_session = created;
    goto session_picker_credentials;
  }

session_picker_credentials:
  if (popup->init(4, 0) != 0) goto session_picker_cleanup;
  popup->dialogs().EditGroup::edit(player_name_text, 0, 0x32);
  if (selected_session->password_ != 0 &&
      selected_session->password_[0] != 0) {
    popup->dialogs().EditGroup::edit(password_required_text, 0, 0x32);
  }
  if (popup->exec(0, 0) == -1) goto session_picker_cleanup;
  // The pinned code reloads the node's password pointer after the modal
  // dialog. If a password is now present it uses both parse buffers; if not,
  // it snapshots the player name through the native unchecked strcat local
  // and passes a null password (not the empty second parse buffer).
  if (selected_session->password_ != 0 &&
      selected_session->password_[0] != 0) {
    result = join_session(selected_session, ParseStrBuffer[0].str,
                           ParseStrBuffer[1].str);
  } else {
    char player_name[0x18];
    player_name[0] = 0;
    strcat(player_name, ParseStrBuffer[0].str);
    result = join_session(selected_session, player_name, 0);
  }

session_picker_cleanup:
  delete popup;
  return result;
}
