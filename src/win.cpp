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
#include "win.h"

// Application-region copies of JACKAL.LIB's Win. The image places these two
// no-op virtuals in the game object around Popup (0x00406A80, 0x00406B30),
// not in Win's library member (src/jackal/win.cpp, 0x005EB350..). The
// originals were header-inline COMDATs; they stay out of line in this
// application unit until the header form is restored.

// Typed owners for folded original no-op virtual targets.  The pinned
// Win table binds slot 25 to 0x00406A80 and slots 6, 8 and 61 to
// 0x00406B30. Other class views of those addresses remain
// shared-owner or unresolved evidence, not one synthetic ABI.
// ORIGINAL: 0x00406A80 sub_406a80 0x00406A80-0x00406A85 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; Win primary table 0x0066FDD0 slot 25 at 0x0066FE34 relocates to 0x00406A80", "src/win.h declares the existing virtual as int on_key_click(int, int); its source body returns zero, and derived real overrides retain their own implementations", "historical catalog facts retained: sub_406a80 was a five-byte zero-return leaf with callee cleanup of eight stack bytes; the retired fastcall redirect was only an opaque annotation carrier", "retired source symbol retained as historical evidence: ?nullsub_00406a80_redirect@@YIHPAX0HH@Z", "twenty pinned base-relocation sites reference 0x00406A80; a base slot is evidence for this member, not proof that every descendant view has the same owner"], "reason": "Move the pinned two-argument zero-return virtual identity from the opaque redirect to Win::on_key_click. This is one genuine typed Win view; other table views that fold to the same bytes remain separate source-owner evidence."}
// symbol    ?on_key_click@Win@@UAEHHH@Z
// kind      game
int Win::on_key_click(int, int) { return 0; }

// ORIGINAL: 0x00406B30 sub_406b30 0x00406B30-0x00406B33 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; Win primary table 0x0066FDD0 slot 61 at 0x0066FEC4 relocates to 0x00406B30", "Win slots 6 and 8 at 0x0066FDE8 and 0x0066FDF0 independently target the same identity; the historical source contract was int() in each declaration, while current Scroll* spelling is inferred from Win::init storing slot 6 EAX at +0x440/+0x43C and later treating it as Scroll*; the stripped executable does not prove the original declared C++ return type", "historical class-table finding retained: 129 virtual slots across 70 classes reach this shared code address; recorded slots have different arities, so no one synthetic prototype represents them", "historical Buffer view remains a research lead, not a new owner mapping: the pinned secondary table selects adjustor 0x00404420 to GraphicWin::get_graphic_win, so no independent Buffer-to-406B30 slot is asserted", "retired source symbol retained as historical evidence: ?nullsub_00406b30_redirect@@YIHPAX0@Z", "existing source-shared bindings for BasePop, VideoSoundState, VoiceRx and VoiceTx remain unchanged as separate typed owners; Buffer's unproved view is not added", "the old opaque fastcall redirect was scaffolding, not ABI evidence; unknown virtual views and incomplete table extents remain restoration/ownership obligations"], "reason": "Place the original identity on one genuine Win method only as a navigation anchor for the folded shared body; preserve the independently typed Win, VideoSoundState, Voice, and BasePop source views with shared mappings. This does not claim that Win::get_graphic_win was the unique historical source owner."}
// symbol    ?get_graphic_win@Win@@UAEPAVGraphicWin@@XZ
// kind      game
GraphicWin *Win::get_graphic_win() { return nullptr; }

