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
  * DeletionList class
  *
  * An array of fixed-size records. clear(index) resets one record - a byte at
  * its start and a word eight in - at stride 0x3C (index * 15 dwords). The
  * record count is not established, so the entry is reached by computed offset
  * rather than a modelled array, and nothing pins this sizeof.
  */
class DLLEXPORT DeletionList {
 public:
  DeletionList() { ; }
  ~DeletionList() { ; }
  void clear(int index);

  // Storage the image proves is here: its own methods reach 0x5A4.
  // Extent only - nothing here is named, and this class carries no size assertion.
  uint8_t field_0_[0x5A4];
};

void __fastcall deletion_list_clear_redirect(DeletionList *self, void *, int index);
