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

#include "original_seam.h"
#include "graphicwin.h"
#include "scroll.h"
#include "stringstruct.h"

 /*
  * StringBox class
  */
// GraphicWin DIRECTLY, not through ConstructedGraphicWin: that shim is an
// empty pass-through now that GraphicWin::GraphicWin() is a real constructor,
// but its inlined implicit constructor stores the SHIM's own two vftables
// between the GraphicWin base call and the member construction - a store pair
// the image does not have (its StringBox derives from GraphicWin directly).
// The base's real constructor runs implicitly either way.
class StringBox : public GraphicWin {
 public:
  // 0x00629490, a pending_bodies forwarder.
  void add_fixup();

 public:
  // (code, pos), the Win32 WM_VSCROLL shape the image mirrors. Evidence
  // is in the bodies: BaseWin::on_scrolled does `switch (a1)` and then
  // stores a2 in field_40B10_; StringBox::on_scrolled stores a2 in
  // top_line_; BaseWin::on_iface_scrolled tests `a1 != 2` and copies a2.
  // The first is switched on, the second is kept - a code and a position.
  void on_scrolled(int code, int pos);
  // 0x00629110. A REAL constructor: the image constructs its GraphicWin
  // base and its Scroll member, and carries the unwind frame that implies.
  StringBox();
  // 0x004325C0 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~StringBox();
  void add(char *text, int index, int flag);

 private:
  // Draw-mode flag: 0 and on_redraw paints the whole list with one bulk
  // Buffer::write_strings; set, it walks the visible lines individually
  // (wrap_height / wrap_cent / wrap), honouring each entry's alignment bit.
  uint32_t per_line_draw_;  // 0x0a14
  // The visible-row budget. -1 from the constructor means "do not trim";
  // add_fixup deletes oldest entries while the list measures taller than
  // this many text lines.
  uint32_t visible_rows_;
  // The first visible line, kept in step with the scrollbar: on_scrolled
  // stores the WM_VSCROLL position here, add_fixup copies the clamped
  // position back from the Scroll after re-fitting, and on_redraw starts
  // drawing from this entry.
  uint32_t top_line_;
  Scroll scroll_;
  // An optional ALTERNATE StringList. add_fixup and on_redraw both read it
  // first and fall back to the embedded list when it is null, so measuring,
  // trimming and drawing all target the override while one is installed.
  uint32_t list_override_;  // 0x2b6c
  // 0x2B70. The StringList, constructed IMPLICITLY - the image inlines its
  // constructor here, which is why the image's vftable stores for StringBox
  // itself land after this block.
  StringList stringList_;
};

static_assert(sizeof(StringBox) == 0x2BA0,
              "StringBox layout must match the original executable");

