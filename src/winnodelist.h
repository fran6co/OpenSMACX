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
#pragma once
#include <cstdlib>
#include <stdint.h>

class Win;  // the payload the nodes carry
class Heap; // allocator borrowed by the parent window's node list

// One node shared by dialog-focus rings. The original allocator requests
// exactly 0x1C bytes; only the four fields below are observed. Keep the
// leading and trailing storage opaque and untouched.
struct WinDialogNode {
  uint32_t unknown_00_;
  Win *focus_owner_;
  void *allocation_;
  WinDialogNode *next_;
  WinDialogNode *previous_;
  uint8_t unknown_14_[8];
};

// THE EMBEDDED POLYMORPHIC OBJECT AT Win+0xC8. Its genuine virtual destructor
// gives this member its own vfptr and the one-slot table at 0x0066FF30; the
// Win class has its separate virtual hierarchy at offset zero. It is a
// circular doubly-linked list: `head_` walks typed nodes whose +0x4 is the
// focus owner, +0x8 is an opaque payload allocation, +0xC is next, and +0x10
// is previous, with the links shown by set_parent_dialog and the
// teardown at 0x005F8770 and Win::~Win() (0x005EBC90) performing the identical
// walk INLINE at Win+0xCC.._0xDC rather than calling it. `allocator_` is a
// Heap pointer used for list allocations; nonzero also skips individual frees.
// `allocator_` at +0x14 (Win+0xDC) is a Heap pointer: Win::init at
// 0x005EC493/0x005EC4A7 and set_parent_dialog at 0x005EC9A0 select
// Heap::get for 0x1C-byte nodes and 4-byte payloads when it is nonzero,
// retaining mem_get otherwise. Win::~Win at 0x005EBCD7 skips individual
// frees when it is nonzero. Construction initializes it to zero. No writer
// of a nonzero allocator is identified; its installation remains unresolved.
// The node's leading field at +0 is not established by this evidence.
// `current_` is scratch during that loop, but is read live elsewhere too -
// `Win::is_dialog_focus` (0x005F2CA0) reads it as the top-of-stack node,
// whose own +0x4 holds the `Win *` currently holding focus.
//
// NOT A SECOND BASE: it is a plain member (`Win::list_`), so its vfptr is at
// Win+0xC8 and Win's genuine vfptr at offset zero is unchanged.
class WinNodeList {
 public:
  WinNodeList() : head_(0), current_(0), count_(0), tail_(0), allocator_(0) {}
  // DEFINED IN THE CLASS so VC6 can inline it into `??_GWinNodeList`,
  // which is what the image is: ONE function at 0x005F8770 that restores
  // the vfptr and walks the list. Out-of-line, the deleting destructor
  // tail-CALLS `??1WinNodeList@@UAE@XZ` instead and measures 14
  // instructions against the image's 54.
  virtual ~WinNodeList() {
      // IMAGE ORDER: the head is read before the vfptr-restore that every
      // scalar deleting destructor opens with, matching `Win::~Win()`
      // performing the same walk inline. A nonzero allocator_ skips the
      // individual frees, as it does in the inline Win teardown.
      WinDialogNode *node = head_;
      if (node != 0) {
          if (allocator_ == 0 && count_ > 0) {
              int i = 0;
              do {
                  WinDialogNode *next = node->next_;
                  current_ = next;
                  void *payload = node->allocation_;
                  if (payload != 0) {
                      std::free(payload);
                  }
                  head_->allocation_ = 0;
                  if (head_ != 0) {
                      std::free(head_);
                  }
                  node = current_;
                  head_ = node;
                  ++i;
              } while (i < count_);
          }
          head_ = 0;
          tail_ = 0;
          count_ = 0;
      }
      tail_ = 0;
  }

  WinDialogNode *head_;    // 0x4  (Win+0xCC)
  WinDialogNode *current_; // 0x8  (Win+0xD0)
  int count_;         // 0xC  (Win+0xD4)
  // AN INDEX, not a pointer: the node walk does `++tail_ == count_`,
  // which is a ring position. Same four bytes either way, so the
  // layout is unchanged and the assert still holds.
  int tail_;          // 0x10 (Win+0xD8)
  Heap *allocator_;   // 0x14 (Win+0xDC)
};
