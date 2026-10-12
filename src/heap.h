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

 /*
  * Heap class: Handles managing or allocating memory.
  */
class Heap {
  friend struct TexHeap;
  // TexHeap635050 constructs descriptors over borrowed arena regions.
  // Unlike init(), this never allocates/reallocates the region; bit 0 makes
  // get() return null instead of growing it. TexHeap releases descriptor
  // storage without invoking the owning destructor, which would free base_.
  Heap(void *region, size_t size) {
    err_flags_ = 1;
    base_ = region;
    current_ = region;
    base_size_ = size;
    free_size_ = size;
  }
 public:
  // OUT OF LINE, and that is what `Strings` needs: its own constructor
  // opens `push esi; mov esi, ecx; call ?0Heap`, a real call to the base.
  // Defined in-class the compiler inlines it and the caller never matches.
  Heap();   // 005D4560
  // Distinct original operation; invocation/lifetime role unknown.
  void Heap2();
  // OUT OF LINE, like the constructor above - and for the same reason plus
  // one: an in-class destructor is EXPANDED at every implicit base/member
  // teardown site, so `~Strings` and `~TextIndex` open-code nineteen
  // instructions (and, in TextIndex's case, an SEH unwind frame) where the
  // image's own are a five-byte `jmp` and a four-instruction tail. Defined
  // in heap.cpp each site emits the one call the image makes.
  ~Heap();  // 005D4580

  // Dialog::close calls the cleanup body at 005D4580 while retaining its
  // embedded Heap for later initialization. A reset does not end its lifetime.
  void reset();
  void shutdown();
  void squeeze(int UNUSED(toggle) toggle);
    BOOL init(int req_size);
  LPVOID get(int req_size);
  // additional functions to assist with encapsulation
  size_t get_base_size() { return base_size_; } // Get base total memory size
  LPVOID get_base() { return base_; }   // Get base memory address pointer

 private:
  int8_t err_flags_; // (+0) -> error toggle
  LPVOID base_;      // (+4) -> pointer to base memory address
  // PUBLIC, not private: TexHeap::get_mem walks the block table and reads
  // each block's free tail straight off the object - the `+0x10` compare in
  // the image - and TexHeap::get_tex_mem reads `current_` the same way for
  // its low-half alignment test. Same offsets; only the access run changes.
  // DECLARATION ORDER IS UNCHANGED - moving a member between access sections
  // moves its offset, and only the section headers move here.
 public:
  LPVOID current_;   // (+8) -> current memory address position
 private:
  size_t base_size_; // (+12) -> size of total memory
 public:
  size_t free_size_; // (+16) -> size of free available memory
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(Heap) == 0x14, "Heap layout must match the legacy ABI");
#endif
