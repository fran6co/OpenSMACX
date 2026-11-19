// The original TU calls StringStruct::remove_all out of line (0x0063C7C0).
#define STRINGSTRUCT_REMOVE_ALL_OUT_OF_LINE
#include "stdafx.h"
#include "net_session_picker.h"
#include "alphanet_inline.h"

#include <cstdlib>
#include <cstring>

#include "buffer.h"
#include "basepop.h"
#include "dialog.h"
#include "dialogs.h"
#include "general.h"
#include "main.h"
#include "popup.h"
#include "strings.h"

#include "win.h"
#include "xpops.h"

// The pinned 0x63C510 parse_says input is the zero-initialized byte at
// 0x009BEB18. It has no other immediate-address reference in the image and
// its only observed use is as an empty-string input. Extent is one accessed
// byte only; the start binding does not assert a larger allocation.
// RECOVERY_OBJECT: v1 0x009BEB18 ?SessionPickerPasswordPrompt@@3DA
char SessionPickerPasswordPrompt;

// ORIGINAL: 0x0063BB20 sub_63bb20 0x0063BB20-0x0063BD06;0x00663940-0x00663998 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ??0NetSessionPickerWin@@QAE@XZ
// TRIED: __cdecl no-arg contract - disasm has ecx=this (mov esi,ecx), no stack args, plain ret; used __fastcall(void*) instead (the free-thiscall-avoids-C4234 pattern from 005cb050.cpp). Placement-new construction of the 6 sub-objects (GraphicWin, Time, Heap, ListBox(1), PushButton x3) at their disasm offsets DOES reproduce the SEH frame's first two pushes (push -1; push funclet) - divergence starts at #2, likely an extra EBP-frame instruction from added local temporaries.
// ORIGINAL: 0x0063C500 nullsub_175 0x0063C500-0x0063C503 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor 0x63BB20 stores addresspoint 0x670DDC at [this] at 0x63BC48; its slot 54 entry at 0x670EB4 targets 0x63C500.", "The pinned 0x63C500 body is a bare ret 4 with no reads or writes. The typed void NetSessionPickerWin::on_selected(int) override preserves that no-op and stack argument contract without a manual thunk. The prior vslot_54 name was transitional; the one-argument selection interface is shared with Win, ListBox and the other constructor-selected derived handlers.", "Historical seed catalog described nullsub_175 as a do-nothing leaf returning with stack pop 4; it recorded kind game, size 3, flags hidden;sp_ready;purged_ok, zero callers/call targets, no calls, return n/a, and prior adapter symbol ?nullsub_0063c500_redirect@@YIXPAX0H@Z. The original identity, full span, and byte claim remain on this source-owned method. That adapter symbol is provenance, not a claim about the new emitted member symbol."], "reason": "Typed no-op selection override for the constructor-installed NetSessionPickerWin table entry; original mapping and match claim are retained. Its current emitted symbol names the real virtual member rather than the historical slot adapter."}
// symbol    ?on_selected@NetSessionPickerWin@@UAEXH@Z
void NetSessionPickerWin::on_selected(int) {
}

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
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ??_GNetSessionPickerWin@@UAEPAXI@Z

// ORIGINAL: 0x00633740 sub_633740 0x00633740-0x0063374B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ??_ENetSessionPickerWin@@WEEE@AEPAXI@Z


// ORIGINAL: 0x00633010 sub_633010 0x00633010-0x00633152;0x00663730-0x006637AF FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ??1NetSessionPickerWin@@UAE@XZ
// body src/jackal/net_session_picker.h
// TRIED: the tail at 0x00663730-0x006637AF is a chain of EH unwind funclets protecting the partial destruction of several embedded members if an earlier member's destructor throws. Reproducing that needs this class's exact multiple/virtual-inheritance layout modelled with real inheritance so the compiler regenerates the funclets itself; the flat, unprotected call sequence is landed instead. The string-list member at +0x36a8 is StringStruct::close_with_tables inlined at the call site with two different literal table pairs, remove_all left as a real call.
// Historical reference: the retired `deleting_thunks.h` carried the
// original teardown finding; its formal TRIED record remains unchanged.
// TRIED: the teardown the 0x633160 deleting thunk calls - see the note in `deleting_thunks.h`.

// ORIGINAL: 0x0063BD10 sub_63bd10 0x0063BD10-0x0063BEAD FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?close@NetSessionPickerWin@@QAEXXZ
// TRIED: MISMATCH #0 push/sub - unknown containing class, receiver expressed as an opaque thiscall shim (no field names); stack frame shape differs from the original's spill choices.
void NetSessionPickerWin::close() {
  refresh_timer_.close();
  sessions_.close();
  sessions_view_.close();
  create_button_.close();
  action_button_.close();
  cancel_button_.close();
  GraphicWin::close();

  net_ = 0;
  tail_.action_result_ = 0;
  tail_.client_width_ = 0;
  tail_.client_height_ = 0;
  tail_.field_3708_ = 0;
  tail_.field_370c_ = 0;
  tail_.field_3710_ = 2;
  tail_.field_3714_ = 0;
  tail_.field_3718_ = 7;
  tail_.frame_color_ = 0xff;
  tail_.box_color_a_ = 0;
  tail_.box_color_b_ = 0;
  tail_.row_count_ = 2;
  tail_.field_373c_ = 0;
  tail_.field_3740_ = -1;
  tail_.field_3744_ = 1;
  tail_.field_3748_ = 1;
  tail_.selected_row_color_ = 7;
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

// ORIGINAL: 0x0063CD50 sub_63cd50 0x0063CD50-0x0063CDBB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?draw_frame@NetSessionPickerWin@@QAEXXZ
void NetSessionPickerWin::draw_frame() {
  RECT *rect = &tail_.frame_rect_;
  GraphicWin::fill(rect, tail_.frame_color_);
  int count = tail_.row_count_;
  int i = 0;
  if (count > 0) {
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

// ORIGINAL: 0x0063C9D0 sub_63c9d0 0x0063C9D0-0x0063CA98 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?on_redraw@NetSessionPickerWin@@UAEXXZ
// TRIED: signature __cdecl(void) -> __thiscall member (ecx=this throughout, plain `ret`, class-member wrapper like other anonymous subs). sub_63caa0 called via the scaffold's existing nullary __cdecl decl though the original passes ecx=this too - can't redeclare it (C2733), so that one call is known to diverge. First divergence #0 push/mov, 210 vs 200 B.
// Guarded label write, inlined at each call: the image keeps its null test.
static inline void write_label(NetSessionPickerWin *win, char *text,
                               RECT *rect) {
  if (text != 0) {
    win->Buffer::write_l(text, rect, static_cast<int>(strlen(text)));
  }
}

void NetSessionPickerWin::on_redraw() {
  write_label(this, "Available Sessions", &tail_.first_label_rect_);
  write_label(this, "Current Players", &tail_.second_label_rect_);
  draw_selected_rows();
  draw_frame();
}

// ORIGINAL: 0x0063CAA0 sub_63caa0 0x0063CAA0-0x0063CD4E FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?draw_selected_rows@NetSessionPickerWin@@QAEXXZ
// TRIED: `this` is GraphicWin* (esi passed straight to ?fill@GraphicWin@@ with no offset, and buffer_ member at self+0x444 matches every Buffer:: call site's `lea ecx,[esi+0x444]`), reading a RECT from self+0x374c and driving a nested box()-drawing loop, then (if Dialog::get_selected_id() is nonzero) reinterpreting that int return as a pointer into an unrelated list-cursor structure (+0x20/+0x34/+0x38/+0x3c/+0x40) to walk entries and write_l() their text - transcribed as raw offset arithmetic since that structure does not match the catalogued Dialog field layout at those offsets. Compiles and matches the first 6 instructions; diverges at #6 (original hoists `ebx = esi+0x374c` once and reuses it for all three RECT reads for the whole function, rebuilt recomputes the offset at each use) - register allocation, not source form.

// WORKAROUND(inline): NetTextListBase::seek_pos (net_class.cpp) is out of
// line in this tree; draw_selected_rows expands it inline (0x0063CC5B..
// 0x0063CC99, both abs(-1) calls kept as real calls).
static inline int picker_seek_pos(NetTextListBase &list, int position) {
  typedef int (__cdecl *AbsFunction)(int);
  AbsFunction abs_function = &abs;
  if (position > list.entry_count_ - 1) return 0;
  list.current_ = list.head_;
  if (position < 0) {
    if (abs_function(position) > list.entry_count_) return 0;
    int steps = abs_function(position);
    if (steps > 0) {
      do {
        --steps;
        list.current_ = list.current_->previous;
      } while (steps != 0);
    }
    list.current_position_ = position + list.entry_count_;
  } else {
    int steps = position;
    if (steps > 0) {
      do {
        --steps;
        list.current_ = list.current_->next;
      } while (steps != 0);
    }
    list.current_position_ = position;
  }
  return 1;
}

// WORKAROUND(inline): NetTextListBase::next_entry (net_class.cpp) is out of
// line in this tree; draw_selected_rows expands it inline (0x0063CCD0:
// the position copy compared against a fresh entry_count_ load).
static inline NetTextPayload *picker_next_entry(NetTextListBase &list) {
  if (!list.head_) return 0;
  list.current_ = list.current_->next;
  if (++list.current_position_ == list.entry_count_) list.current_position_ = 0;
  return list.current_->payload;
}

void NetSessionPickerWin::draw_selected_rows() {
  RECT *rows = &tail_.rows_rect_;
  RECT rect = *rows;
  GraphicWin::fill(&rect, tail_.selected_row_color_);
  rect.bottom = tail_.row_height_ + rect.top;

  for (int row = 0; row < tail_.rows_visible_; row++) {
    for (int column = 0; column < tail_.boxes_per_row_; column++) {
      Buffer::box(&rect, tail_.row_box_color_a_, tail_.row_box_color_b_);
      ++rect.left;
      --rect.right;
      ++rect.top;
      --rect.bottom;
    }
    expand_rect(&rect, tail_.boxes_per_row_, tail_.boxes_per_row_);
    rect.top += tail_.row_height_;
    rect.bottom += tail_.row_height_;
  }

  const int selected_id = sessions_view_.Dialog::get_selected_id();
  if (selected_id == 0) return;

  // ListBox::item stores the given ID in Dialog::item_list_; the refresh
  // operation passes the exact NetAllocationNode* payload as that ID.
  NetAllocationNode *selected = reinterpret_cast<NetAllocationNode *>(
      static_cast<uintptr_t>(static_cast<unsigned int>(selected_id)));
  NetUnk72C &list = selected->entries_;

  rect = *rows;
  rect.top += static_cast<int>(selected->max_players_) * tail_.row_height_;
  Buffer::change_color(&rect, tail_.selected_row_color_,
                       tail_.selected_text_color_a_);

  rect = *rows;
  rect.bottom = tail_.row_height_ + rect.top;
  expand_rect(&rect, -tail_.boxes_per_row_, -tail_.boxes_per_row_);

  picker_seek_pos(list, -1);
  Buffer::set_text_color(tail_.selected_text_color_b_,
                         tail_.selected_text_color_c_,
                         tail_.selected_text_color_d_, tail_.field_3794_);

  for (int i = 0; i < list.entry_count_; i++) {
    picker_next_entry(list);
    if (list.current_ != 0) {
      NetTextPayload *payload = list.head_ ? list.current_->payload : 0;
      char *text = payload->text;
      if (text != 0) {
        Buffer::write_l(text, &rect,
                        static_cast<unsigned int>(strlen(text)));
      }
    }
    rect.top += tail_.row_height_;
    rect.bottom += tail_.row_height_;
  }
}

// ORIGINAL: 0x0063C7C0 sub_63c7c0 0x0063C7C0-0x0063C9B6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?refresh_player_list@NetSessionPickerWin@@QAEXXZ
// TRIED: `__thiscall` function-pointer casts for the dynamically adjusted vtable dispatches (C4234, VC6 refuses them) - routed through a second VCall-style shim (VCallSlot0) instead, one per distinct slot/arity pair, matching the brief's guidance. Did not chase the remaining register-allocation divergence (first mismatch at instruction #2) further given the size and structural complexity of this function (three raw vtable adjustor dispatches plus ListBox::attach/item/update_changes).
// Inline cursor read: the image reloads head_ after it for the step back.
static inline NetAllocationNode *ledger_current_id(AlphaNetLedger *list) {
  return list->head_ != 0 ? list->current_->payload_ : 0;
}

void NetSessionPickerWin::refresh_player_list() {
  // The list implementation owns the observed hook/delete/cursor-reset
  // sequence; it preserves the original callback and allocator protocol.
  sessions_.remove_all();

  // Direct pinned call 0x62E420: ECX is net_, arguments are this exact ledger
  // and a null password. A nonzero result returns before the UI lists change.
  if (net_->enumerate_sessions(&sessions_, 0) != 0) return;
  ListBox *view = &sessions_view_;

  {
    StringList &items = sessions_view_.item_list_;
    items.remove_all();
    items.current_position_ = 0;
  }
  // Native63C874 reloads the virtual Dialog view between the two clears.
  // Each call's following stores use that call's captured list receiver.
  StringList &items = sessions_view_.item_list_;
  items.reset();

  if (sessions_.entry_count_ == 0) {
    view->item("NO SESSIONS (Translate this)", 0);
  } else {
    sessions_.seek_pos(-1);

    int index = 0;
    if (sessions_.entry_count_ > 0) {
      do {
        AlphaNetLedger *list = &sessions_;
        NetAllocationNode *item_id = ledger_current_id(&sessions_);
        if (list->head_ != 0) {
          list->current_ = list->current_->previous_;
          if (--list->current_position_ == -1) {
            list->current_position_ = list->entry_count_ - 1;
          }
        }
        // The original's null-head/inconsistent-current path reads through
        // an invalid payload expression; under the actual circular-list
        // invariant this arm is unreachable. Do not repair it here.
        char *text = list->current_ != 0
            ? (list->head_ != 0 ? list->current_->payload_ : 0)->session_name_
            : 0;
        view->item(text, static_cast<int>(reinterpret_cast<uintptr_t>(item_id)));
        ++index;
      } while (index < sessions_.entry_count_);
    }
  }

  int result;
  if (HandleMain == 0) {
    // Native63C963 reloads the Dialog coordinates at this late phase;
    // Native63C981 directly calls the real ListBox attachment method.
    result = view->ListBox::attach(
        this, view->window_x_, view->window_y_, 0x80000);
  } else {
    result = view->update_changes();
  }
  tail_.action_result_ = result;
  if (result != 0) return;
  sessions_view_.show(0);
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0063C9C0 nullsub_266 0x0063C9C0-0x0063C9C1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0063C9C0-0x0063C9C1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0063C7C0 sub_63c7c0 (src/net_session_picker.cpp) and 0x0063C9D0 sub_63c9d0 (src/net_session_picker.cpp)"]}
// symbol    ?nullsub_266@@YAXXZ
// kind      game
void __cdecl nullsub_266() {
}

// ORIGINAL: 0x0063C340 sub_63c340 0x0063C340-0x0063C38F FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
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
  return net_->local_player_id();
}

// ORIGINAL: 0x0063C4E0 sub_63c4e0 0x0063C4E0-0x0063C4F6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?refresh_timer_callback@NetSessionPickerWin@@SAXH@Z
void __cdecl NetSessionPickerWin::refresh_timer_callback(int context) {
  // Keep the image's int payload ABI and the nonzero/focus guard order.
  if (context == 0) return;
  NetSessionPickerWin *owner = reinterpret_cast<NetSessionPickerWin *>(
      static_cast<uintptr_t>(static_cast<unsigned int>(context)));
  if (WinFocusWindow == static_cast<Win *>(owner)) owner->refresh_player_list();
}

// ORIGINAL: 0x0063C390 sub_63c390 0x0063C390-0x0063C39E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?refresh_if_focused@NetSessionPickerWin@@QAEXXZ
void NetSessionPickerWin::refresh_if_focused() {
  if (WinFocusWindow == static_cast<Win *>(this)) refresh_player_list();
}

// ORIGINAL: 0x0063BF10 sub_63bf10 0x0063BF10-0x0063C33F FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
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
  const int margin_x = width * 10 / 600;
  const int margin_y = height * 10 / 440;
  tail_.client_width_ = width;
  tail_.client_height_ = height;
  int result = GraphicWin::init(
      0, 0, width, height, caption, 0x800, BasePopDefaultField322C, 0, 0);
  if (result != 0) return result;

  center();
  Buffer::set_font(font, 0, 0, 0);

  // The button column: a third of the client width wide and 7/4 line
  // heights tall, then shifted to the right margin and down.
  RECT rect;
  rect.left = tail_.client_width_;
  rect.top = tail_.client_height_;
  rect.right = tail_.client_width_ * 200 / 600 + tail_.client_width_;
  rect.bottom = font->pixel_height() * 7 / 4 + tail_.client_height_;
  const int dx = tail_.client_width_ - tail_.client_width_ * 10 / 600 - rect.right;
  rect.left += dx;
  rect.right += dx;
  const int dy = (tail_.client_height_ - rect.bottom - margin_y) * 3;
  rect.top += dy;
  rect.bottom += dy;
  int button_width = rect.right - rect.left;
  int button_height = rect.bottom - rect.top;
  result = action_button_.init(BasePopDefaultOkText, -1, rect.left, rect.top,
                               button_width, button_height, this, 0);
  if (result != 0) return result;
  action_button_.set_font(font, 0, 0, 0);
  action_button_.redraw();

  rect.top += button_height + margin_y;
  rect.bottom += button_height + margin_y;
  int create_height = rect.bottom - rect.top;
  result = create_button_.init("Create Session (Translate)",
                               0, rect.left, rect.top, button_width,
                               create_height, this, 0);
  if (result != 0) return result;
  create_button_.set_font(font, 0, 0, 0);
  create_button_.redraw();

  rect.top += button_height + margin_y;
  rect.bottom += button_height + margin_y;
  // The image passes the create button's height as the cancel width.
  result = cancel_button_.init(BasePopDefaultCancelText, -2, rect.left,
                               rect.top, create_height, rect.bottom - rect.top,
                               this, 0);
  if (result != 0) return result;
  cancel_button_.set_font(font, 0, 0, 0);
  cancel_button_.redraw();

  tail_.first_label_rect_.top = margin_y;
  tail_.first_label_rect_.left = margin_x;
  tail_.first_label_rect_.right = margin_x * -2 - cancel_button_.pixel_width()
      + width;
  tail_.first_label_rect_.bottom = margin_y + cancel_button_.pixel_height();
  tail_.second_label_rect_.right = width - margin_x;
  tail_.panel_rect_.left = margin_x;
  const int second_left = tail_.first_label_rect_.right + margin_x;
  tail_.second_label_rect_.left = second_left;
  tail_.panel_rect_.bottom = height - margin_y;
  tail_.second_label_rect_.top = margin_y;
  tail_.second_label_rect_.bottom = tail_.first_label_rect_.bottom;
  tail_.panel_rect_.top = tail_.first_label_rect_.bottom;
  tail_.panel_rect_.right = tail_.first_label_rect_.right;
  tail_.rows_rect_.left = second_left;
  tail_.rows_rect_.right = width - margin_x;
  tail_.rows_rect_.top = tail_.first_label_rect_.bottom;

  tail_.rows_rect_.bottom = action_button_.client_rect().top - margin_y * 2;
  tail_.row_height_ = (tail_.rows_rect_.bottom - tail_.boxes_per_row_ * 2
                       - tail_.rows_rect_.top) / rows;
  tail_.rows_rect_.bottom = tail_.row_height_ * rows + tail_.rows_rect_.top;

  sessions_view_.close();
  result = sessions_view_.Dialog::init(&tail_.panel_rect_, 0);
  if (result != 0) return result;

  sessions_view_.Dialog::field_AC_ = 0;
  sessions_view_.Dialog::field_B0_ = 0;
  sessions_view_.Dialog::field_B4_ = 2;
  sessions_view_.Dialog::field_5C_ = 2;
  sessions_view_.Dialog::set_dialog_text_color(tail_.field_3714_, -1, 1, 1);
  sessions_view_.Dialog::set_dialog_text_color2(tail_.field_3718_, -1, 1, 1);
  sessions_view_.Dialog::set_dialog_font(fonts_[0], 0, 0);
  sessions_view_.Dialog::field_34_ = blank_sprite().width();
  return 0;
}

// The image pushes one "jackal"/"NETCONNECT_CREATE" pair (0x698BC8/0x698BB4)
// at both of connect_session's create prompts while every other literal in
// the unit is a separate object: both prompts expand this one inline body.
static inline int ask_create_session() {
  return pop_caption_value_title("jackal", "NETCONNECT_CREATE", 0x3c, 0x44, 0);
}

// ORIGINAL: 0x0063C510 sub_63c510 0x0063C510-0x0063C6CA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?connect_session@NetSessionPickerWin@@QAEHXZ
// TRIED: __thiscall helper class (NetConnectHelper) with pop()/parse_says() overload resolution by arg count and a 1-arg VCall::slot000; diverges at #0, frame setup differs
int NetSessionPickerWin::connect_session() {
  parse_says(0, "My Session", -1, -1);
  parse_says(1, "My Name", -1, -1);
  parse_says(2, &SessionPickerPasswordPrompt, -1, -1);

  int status = ask_create_session();
  for (;;) {
    if (status == -1) return 1;
    if (ParseStrBuffer[0].str[0] == 0) {
      pop_caption("jackal", "NETCONNECT_NEED_SESSION", 0);
    } else if (ParseStrBuffer[1].str[0] == 0) {
      pop_caption("jackal", "NETCONNECT_NEED_PLAYER", 0);
    } else {
      if (ParseStrBuffer[2].str[0] != 0) {
        pop_caption("jackal", "NETCONNECT_PASSWORD_SET", 0);
      }

      BasePop *popup = PopupAllocHook();
      if (popup == 0) return 4;
      popup->start("jackal", "NETCONNECT_MAX_PLAYERS",
                   -1, 0, 0x50, 0);

      int faction_count = tail_.rows_visible_;
      char letter = '2';
      for (int i = 2; i < faction_count + 1; ++i) {
        char label[12];  // 12-byte frame slot in the image
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
        if (session == 0) {
          return 1;
        }
        return net_->join_session(session, ParseStrBuffer[1].str, 0) == 0;
      }
    }
    status = ask_create_session();
  }
}

// ORIGINAL: 0x0063C6D0 sub_63c6d0 0x0063C6D0-0x0063C7B5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/unrecovered/0063c6d0.cpp; full original marker, spans, byte claim and lessons transported by match_recovery.source.writer", "historical scaffold symbol hint (not an emitted-symbol fact):    ?connect@T63C6D0@@QAEHXZ", "owner proposal: src/net_session_picker.cpp; original identity 0x0063C6D0; symbol observation intentionally pending physical compilation"], "reason": "Recover the selected-session operation as a real NetSessionPickerWin member over the existing ListBox, AlphaNetLedger, Net and popup owners; emitted symbol awaits physical observation."}
// symbol    ?join_selected_session@NetSessionPickerWin@@QAEHXZ
// LEVER: ecx/cdecl the given `extern "C" int __cdecl sub_63c6d0()` head was wrong - every field access reads `[ecx+N]`/`[esi+N]` with `esi=ecx` and no stack argument, so the receiver is `this`. Changing it to a `__thiscall` member of a local helper class is part of what made this match, so it is a lever and not a tried spelling.
// LEVER: reversing a `>=` comparison's operand order (`iVar2 <= *w` instead of `*w >= iVar2`) flipped `jl` to the original's `jg`; and giving the "abs bound exceeded" early-exit its own flag (skipping the trailing `field36bc = iVar1` store) instead of relying on it falling out of a plain if/else, matched the original NOT sharing that store between the negative- and positive-`iVar1` branches - it stores separately in each.
// working copy - scaffold materialised by --work
// size      229 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00627200 0x0062E540 0x00644F3A
int NetSessionPickerWin::join_selected_session() {
    if (pop_caption_value_title("jackal", "NETCONNECT_JOIN", 0x3c, 0x44,
                                0) == -1) {
        return 1;
    }

    sessions_.seek_pos(sessions_view_.selected_position_);
    NetAllocationNode *session;
    if (sessions_.head_ != 0) {
        session = sessions_.current_->payload_;
    } else {
        session = 0;
    }

    const int result = net_->join_session(
        session, ParseStrBuffer[0].str, 0);
    tail_.action_result_ = result;
    if (result != 0) {
        return 0;
    }
    return 1;
}

// ORIGINAL: 0x0063C3A0 sub_63c3a0 0x0063C3A0-0x0063C4D6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?on_button_clicked@NetSessionPickerWin@@UAEXH@Z
// TRIED: IDA guessed 1 stack arg; ecx=esi with no matching stack read confirms a __thiscall receiver, its own minimal class (no catalogued name). The vtable-slot-58 call needs its own dummy-padded virtual class (same reason as 0x5ED880), so this stays FILE mode. `if (a2==-2){A}else if(a2!=-1){...}` compiles the -2 case inline (jne to skip past it); the original places that handler in a separate far block and jumps to it directly (je) - a block-layout difference this source form did not reproduce. Landed at MISMATCH #4.
void NetSessionPickerWin::on_button_clicked(int action) {
  switch (action) {
  case 0:
    if (connect_session() != 0) return;
    release_modal();
    return;
  case -1:
    // join_selected_session is expanded here: the image pushes its
    // "jackal"/"NETCONNECT_JOIN" literals (0x698C60/0x698C50).
    if (!join_selected_session()) release_modal();
    return;
  case -2:
    tail_.action_result_ = 0;
    release_modal();
    return;
  }
}

/*
Purpose: Scale a field by an argument and divide by 600.

             mov ecx,[ecx+0x3798] / mov eax,0x1b4e81b5 / imul ecx,[esp+4]
             imul ecx / mov eax,edx / sar eax,6 / mov ecx,eax / shr ecx,0x1f
             add eax,ecx / ret 4

         MSVC's magic-number division: multiply by 0x1b4e81b5, keep the high
         dword, shift right 6, then add the sign bit so it truncates toward
         zero instead of toward negative infinity. 0x1b4e81b5 is
         ceil(2^38 / 600), and the divisor was confirmed by running the
         original's exact sequence against truncating division over 206,006
         inputs - the whole of -3000..3000, 200,000 random int32s, and both
         extremes - with no disagreement.

         The multiply is an ordinary 32-bit one and WRAPS. Spelling it in
         unsigned and casting back keeps that defined, which signed overflow
         in C++ would not be.

// ORIGINAL: 0x0063BEB0 sub_63beb0 0x0063BEB0-0x0063BED1 BYTE_EXACT
// symbol    ?scale_client_width@NetSessionPickerWin@@QAEHH@Z
// size      33 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: (field 0x3798 * argument) / 600
Status: Complete
*/

int NetSessionPickerWin::scale_client_width(int factor) {
    const uint32_t product = static_cast<uint32_t>(tail_.client_width_) *
                             static_cast<uint32_t>(factor);
    return static_cast<int32_t>(product) / 600;
}


/*
Purpose: Scale a field by an argument and divide by 440.

         The same shape as 0063BEB0 with a different field, magic and shift:
         0x94f2095 is ceil(2^36 / 440). Confirmed the same way, over the same
         206,006 inputs, with no disagreement.

// ORIGINAL: 0x0063BEE0 sub_63bee0 0x0063BEE0-0x0063BF01 BYTE_EXACT
// symbol    ?scale_client_height@NetSessionPickerWin@@QAEHH@Z
// size      33 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: (field 0x379c * argument) / 440
Status: Complete
*/

int NetSessionPickerWin::scale_client_height(int factor) {
    const uint32_t product = static_cast<uint32_t>(tail_.client_height_) *
                             static_cast<uint32_t>(factor);
    return static_cast<int32_t>(product) / 440;
}
// RECOVERY_GENERATED: v1 vbtable 0x0066EAF0 ? ??_8AlphaNetLedger@@7B@ 0x0063BB20
// RECOVERY_GENERATED: v1 vtable 0x0066EAE4 ? ??_7AlphaNetLedger@@6BStringAllocationBase@@@ 0x0063BB20
// RECOVERY_GENERATED: v1 vtable 0x00670DDC ? ??_7NetSessionPickerWin@@6BWin@@@ 0x0063BB20
// RECOVERY_GENERATED: v1 vtable 0x00670DD4 ? ??_7NetSessionPickerWin@@6BBuffer@@@ 0x0063BB20

// The blank-sprite class statics belong to this unit: sub_63cdc0,
// Sprite::init_class and Sprite::close_class (0x0063CDC0..0x0063CEFA) follow
// the picker's functions, and init_class's unwind data (0x00663998) follows
// the picker's (0x00663940). init_class calls Sprite::create_blank
// (0x005EAF3F, the sprite unit) out of line: this unit sees only its
// declaration.

/*
// symbol    ?init_class@Sprite@@SAHXZ
// LEVER 2026-09-03: two things, and the first one is worth stating because
//   this body carried a TRIED note for two passes claiming otherwise. The /GX
//   SEH frame over the local Buffer IS reproduced by the plain /O2 /Ob0 /GX
//   build - `sub esp, 0x588`, the fs:[0] swap and the `mov dword ptr
//   [esp+0x594], -1` state stores are all there - so the frame was never the
//   wall. What was missing was the return shape: two SEPARATE epilogues,
//   `if (result != 0) return result;` then `return 0;`, which is the
//   split-epilogue lever. 34/53 -> 53/53 BYTE_EXACT. The SEH span at 0x663998
//   stays cold code the image lays elsewhere.
// size      220 bytes
// prototype
// callers   1   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7DE0 0x005E39A0 0x005EAF3F
//
// NAMED Sprite::init_class 2026-08-30: jackal_init_real runs it in the
// X::init_class() chain between FileWin::init_class and the font bring-up,
// and it seeds the class-shared sprite (g_BLANK_SPRITE, 0x009BEAE8) from
// jackal.pcx - or blank when the PCX is missing. The catalogue carries no
// name for it, so the address keeps the marker line. The teardown twin,
// 0x0063CEF0 below, is close_class by the same reading.
//
// Promoted 2026-08-15 from src/unrecovered/0063ce20.cpp to retire its
// pending_bodies forwarder. Loads the 16x16 sprite at 0x9BEAE8 from a PCX, or
// creates it blank if the PCX is missing.
Status: Complete
*/
// Historical note: the earlier recovery draft described 0x009BEAE8 using
// guarded_teardowns.h's TeardownObject009BEAE8 scaffold name. The current
// source owner is BlankSpriteStorageOwner and its blank_sprite() accessor;
// retain that older interpretation here as provenance, not as a current owner.
// The pinned short-string read at 0x00698CAC independently establishes this
// filename's spelling and terminator, not its original allocation extent.
// RECOVERY_OBJECT: v1 0x00698CAC _JackalPcxPath
static const char JackalPcxPath[] = "jackal.pcx";

// ORIGINAL: 0x0063CDC0 sub_63cdc0 0x0063CDC0-0x0063CE17 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image span 0x0063CDC0-0x0063CE17: early argument-2/null and dword-at-+8 checks; argument-1 selects null or +0x444 Buffer; final call targets Sprite::draw at 0x005E4B9A.", "Current GraphicWin multiply-inherits Buffer after Win; sizeof(Win)==0x444. Typed GraphicWin-to-Buffer conversion replaces the observed +0x444 destination adjustment.", "The absolute reads 0x009BEAF0 and 0x009BEB04 are the blank Sprite's cTransparentIndex_ (+8) and iHeight_ (+0x1c) in source-owned storage at 0x009BEAE8; access them through blank_sprite().", "The retired seed recorded the same identity/span and historical BYTE_EXACT claim; the claim remains an obligation, not fresh proof.", "Argument 2 remains opaque: only the original dword-at-+8 nonzero test is recovered; no caller edge or enclosing type is established in the current mapped-source closure."], "reason": "Draw the source-owned shared blank Sprite through blank_sprite() and a typed GraphicWin/Buffer destination while preserving the unknown gate context's original dword test, centering and ignored draw result."}
// symbol    ?sub_63cdc0@@YAXPAVGraphicWin@@PAXHHHH@Z
// Purpose: conditionally draw the shared blank sprite centered in a window buffer.
// No caller is inferred: mapped-span inspection reports call target 0x005E4B9A,
// and no caller edge for this entry in the current mapped closure.
// Preserved seed facts: the measured span was 87 bytes; the source prototype
// was unknown; catalog metadata reported zero callers, one call target,
// game code, flags sp_ready/purged_ok, and direct call target 0x005E4B9A.
void __cdecl sub_63cdc0(GraphicWin *window, void *gate_context, int unused,
                       int x, int top, int region_height) {
  if (gate_context != 0 &&
      *reinterpret_cast<const uint32_t *>(
          static_cast<const unsigned char *>(gate_context) + 8) != 0) {
    Buffer *destination;
    if (window != 0) {
      destination = static_cast<Buffer *>(window);
    } else {
      destination = 0;
    }
    const int y = top + (region_height - blank_sprite().height()) / 2;
    blank_sprite().draw(destination, blank_sprite().transparent_index(), x, y);
  }
}

// ORIGINAL: 0x0063CE20 sub_63ce20 0x0063CE20-0x0063CEE7;0x00663998-0x006639AD BYTE_EXACT
// kind      game
// symbol    ?init_class@Sprite@@SAHXZ
int Sprite::init_class() {
    Buffer buf;
    int result;
    if (buf.load_pcx(JackalPcxPath, 0, 10, 0xec) != 0) {
        result = blank_sprite().create_blank(0x10, 0x10, 9);
    } else {
        result = blank_sprite().extract(&buf, 9, 1, 0x12, 0x10, 0x10, 0);
    }
    // TWO RETURNS, not one: the image does not share the Buffer destructor
    // between the two exits, so a single `return result` costs nine
    // instructions (34/53) - a guard clause for the failure value puts the
    // `mov eax,esi` epilogue first and the `xor eax,eax` one second, exactly
    // the image's order, and the state store lands between `mov esi,eax` and
    // the destructor receiver's `lea` where the image has it.
    if (result != 0) {
        return result;
    }
    return 0;
}

/*
Purpose: close_class - close the blank sprite (0x009BEAE8 in the image, the
         real object g_BLANK_SPRITE here), unguarded; the caller
         (jackal_close) tail-calls it as the sprite's teardown stage. Named
         Sprite::close_class 2026-08-30 as the mirror of Sprite::init_class
         (0x0063CE20) - the catalogue carries no name for either.
// ORIGINAL: 0x0063CEF0 sub_63cef0 0x0063CEF0-0x0063CEFA BYTE_EXACT
// symbol    ?close_class@Sprite@@SAXXZ
// size      10 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Sprite::close_class() {
    blank_sprite().close();
}
