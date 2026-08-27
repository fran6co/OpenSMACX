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

class Win;  // the payload the nodes carry

// THE EMBEDDED OBJECT AT Win+0xC8. Only `??0Win@@QAE@XZ`, `??1Win@@QAE@XZ`
// and this class's own out-of-line scalar deleting destructor (0x005F8770)
// reference its vtable at 0x0066FF30 anywhere in .text, and that table is
// exactly ONE slot - the destructor's own address - so it belongs to Win
// alone. It is a small singly-linked list: `head_` walks nodes whose own
// +0x8 is a payload `free()`s and +0xC is the next pointer, proven by
// 0x005F8770 and by `Win::~Win()` (0x005EBC90) performing the identical
// walk INLINE at Win+0xCC.._0xDC rather than calling it. `external_` is a
// "do not own this list" guard: nonzero skips the free loop entirely.
// `current_` is scratch during that loop, but is read live elsewhere too -
// `Win::is_dialog_focus` (0x005F2CA0) reads it as the top-of-stack node,
// whose own +0x4 holds the `Win *` currently holding focus.
//
// NOT A SECOND BASE: it is a plain member (`Win::list_`), so Win's own
// vtable at offset 0 stays exactly as it is - hand-installed, not
// compiler-generated.
class WinNodeList {
 public:
  WinNodeList() : head_(0), current_(0), count_(0), tail_(0), external_(0) {}
  // DEFINED IN THE CLASS so VC6 can inline it into `??_GWinNodeList`,
  // which is what the image is: ONE function at 0x005F8770 that restores
  // the vfptr and walks the list. Out-of-line, the deleting destructor
  // tail-CALLS `??1WinNodeList@@UAE@XZ` instead and measures 14
  // instructions against the image's 54.
  virtual ~WinNodeList() {
      // IMAGE ORDER: the head is read before the vfptr-restore that every
      // scalar deleting destructor opens with, matching `Win::~Win()`
      // performing the same walk inline. `external_` guards the whole loop -
      // nonzero means this list does not own its nodes.
      void *node = head_;
      if (node != 0) {
          if (external_ == 0 && count_ > 0) {
              int i = 0;
              do {
                  Win *next = *reinterpret_cast<Win **>(
                      reinterpret_cast<char *>(node) + 0xC);
                  current_ = next;
                  void *payload = *reinterpret_cast<void **>(
                      reinterpret_cast<char *>(node) + 8);
                  if (payload != 0) {
                      std::free(payload);
                  }
                  *reinterpret_cast<void **>(
                      reinterpret_cast<char *>(head_) + 8) = 0;
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

  void *head_;      // 0x4  (Win+0xCC)
  void *current_;   // 0x8  (Win+0xD0)
  int count_;         // 0xC  (Win+0xD4)
  // AN INDEX, not a pointer: the node walk does `++tail_ == count_`,
  // which is a ring position. Same four bytes either way, so the
  // layout is unchanged and the assert still holds.
  int tail_;          // 0x10 (Win+0xD8)
  int external_;      // 0x14 (Win+0xDC)
};
