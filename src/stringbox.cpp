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
// WORKAROUND(decl): same declaration context as src/jackal/stringbox.cpp.
#define STRINGSTRUCT_NEXT_ENTRY_OUT_OF_LINE
#include "stdafx.h"
#include "graphicwin.h"
#include "scroll.h"
#include "stringstruct.h"
#pragma optimize("y", off)
#include "stringbox.h"
#pragma optimize("", on)

// Application-region copy of JACKAL.LIB's StringBox::on_link_clicked. The
// image places it at 0x00432720 inside the game's Datalink object, beside
// ~StringBox (0x004325C0) and ??_GStringBox, not in StringBox's library
// member (src/jackal/stringbox.cpp, 0x00629110..). The original was a
// header-inline COMDAT; it stays out of line in this application unit until
// the header form is restored.

// The getter is only an admission predicate. Reload the actual parent after
// its callback; the returned GraphicWin pointer is not the dispatch receiver.
// ORIGINAL: 0x00432720 ?on_link_clicked@StringBox@@QAEXHPAD@Z 0x00432720-0x00432761 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Full original432720..432761 returns1 on parent/getter refusal and forwards parent slot88 result otherwise; table66ADC8 selects the same override. The seed void+assembly return is an incorrect signature.", "Reload win_parent_ after virtual get_graphic_win. Its returned GraphicWin pointer is predicate-only; current parent is the callback receiver."], "reason": "Recover original GraphicWin link-hook overrides through the genuine slot88 hierarchy, preserving seed identity and historical obligations."}
// symbol    ?on_link_clicked@StringBox@@UAEHHPAD@Z
#pragma optimize("y", off)
int StringBox::on_link_clicked(int id, char *link) {
    Win *parent = win_parent_;
    if (parent == 0 || parent->get_graphic_win() == 0) {
        return 1;
    }
    return static_cast<GraphicWin *>(win_parent_)->on_link_clicked(id, link);
}
#pragma optimize("", on)

