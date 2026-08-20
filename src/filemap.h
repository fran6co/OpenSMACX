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

 /*
  * FileMap class: Handles file IO operations.
  */
class Filemap {
 public:
  Filemap();  // 00628380
  ~Filemap(); // 006283E0

  Filemap *init(LPCSTR file_name, BOOL is_sequential);
  Filemap *init(LPCSTR file_name);
  LPVOID open_read(LPCSTR file_name, BOOL is_sequential);
  LPVOID open(LPCSTR file_name, BOOL is_sequential);
  LPVOID create(LPCSTR file_name, uint32_t size, BOOL is_sequential);
  // IN-CLASS, because `open_read` and `open` INLINE it: the image opens both
  // with the UnmapViewOfFile/CloseHandle chain written out, not a call. VC6
  // inlines only what it can see here, and the standalone COMDAT it still
  // emits keeps the claim beside its marker in filemap.cpp.
  MEASURED void close() {   // 006287C0
      if (map_view_addr_) {
          UnmapViewOfFile(map_view_addr_);
          map_view_addr_ = 0;
      }
      if (file_map_) {
          CloseHandle(file_map_);
          file_map_ = 0;
      }
      if (file_ != INVALID_HANDLE_VALUE) {
          CloseHandle(file_);
          file_ = 0;
      }
      // NO `file_size_ = 0` HERE. The image's body runs from 0x006287C0 to
      // 0x00628803 and stores to +0x0, +0x8 and +0x4 only; a closed Filemap
      // keeps the size of whatever it last mapped. Clearing it is one store
      // more than the shipped code has, and it showed up inlined in
      // `open_read` where the image has nothing.
  }
  void close(LPVOID new_addr);
  // additional functions to assist with encapsulation
  uint32_t get_size() { return file_size_; }
  // `Buffer::load_pcx` calls `GetFileSize(file_, NULL)` itself rather than
  // reading `file_size_`, even though `open_read` has just set the two to
  // the same value - so the handle has to be reachable to reproduce the
  // call the image makes.
  HANDLE get_handle() { return file_; }
  // Likewise the mapped view: `open_read` returns it, but the image reads
  // the MEMBER back afterwards (`mov eax, [esp+0x18]`) rather than keeping
  // the returned register, so reproducing that needs the member.
  LPVOID get_view() { return map_view_addr_; }

 private:
  LPVOID map_view_addr_; // (+0)
  HANDLE file_;          // (+4)
  HANDLE file_map_;      // (+8)
  uint32_t file_size_;   // (+12)
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(Filemap) == 0x10, "Filemap layout must match the legacy ABI");
#endif
