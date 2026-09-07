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
#include "original_seam.h"
#include "datalink.h"
// Datalink::show and Datalink::hide reach the SubInterface subobject the
// original places at +0xA14 by casting a hand-computed address, so the type
// must be complete.
#include "subinterface.h"
#include "worldwin.h"
#include "menu.h"
#include "maininterface.h"
#include "win.h"
#include "spritebox.h"
#include "vtable_shim.h"
#include "reportwin.h"
#include "strings.h"
#include "alpha.h"
#include "veh.h"
#include "base.h"
#include "text.h"
#include "faction.h"
#include "general.h"

// The Datalink singleton, 0x00703EA0 in the shipped image - the storage the
// `Datalink *const DatalinkMain` binding used to name (see datalink.h). Plain
// data, not a call target; the Datalink constructor that builds it runs from
// the game's class bring-up, as in the image.
Datalink DatalinkMain;  // 0x00703EA0

/*
 * Fixed-slot bindings the homed category fillers arrived carrying, named from
 * evidence. Each folds to the bare immediate, so a rename moves no byte.
 *
 * CategoryIdSeparator  0x006832CC - image .data reads "|" (0x7C, NUL). It is
 *                      the first of seven one-character separator strings
 *                      packed four bytes apart (0x6832CC..0x6832E4), one per
 *                      category filler, and each filler appends it before the
 *                      id suffix that `GameState & 0x1000` asks for.
 * BoxSpriteStyle6      0x0078D618 - buffer.h documents this block as
 *                      ReplayWin's entry in the BoxSpriteParams pool, and
 *                      socialwin.cpp already carries the same binding under
 *                      this name; the pool index is the name.
 * CategoryEntryRect    the category_rect_ member, not a global - listed here
 *                      only to say the fillers share it.
 */
static const char *const CategoryIdSeparator = (const char *)0x006832CC;
static const char *const TerraformSecondSeparator = (const char *)0x0068333C;
static const char *const ConceptArticle = (const char *)0x006832E8;
static const char *const ConceptFile = (const char *)0x006832F8;
static const char *const ConceptSection = (const char *)0x00683300;
static const char *const AdvConceptFile = (const char *)0x00683310;
static int *const BoxSpriteStyle6 = (int *)0x0078D618;

/*
 * The rules tables the remaining fillers walk. Each is named from what its
 * own loop does - the id range it files and the stride it steps - and from
 * the tree's own neighbours where those pin it:
 *
 *   SecretProjectTable   0x009A5888, 64 records of 0x30, ids 68534..68597,
 *                        preq at +0x14. base.h's SecretProject (0x009A6514)
 *                        sits just past its end, which is what makes this a
 *                        measurement and not a guess.
 *   TechRowBase          0x0094F380 + 9 dwords, so the walk's `rec[-9]` name
 *                        read lands on the technology record's own head.
 *   SocialCategoryRow    0x0094B014, SocialCategories (faction.h,
 *                        0x0094B000) + 0x14 - the first category's first
 *                        choice, whose header id the walk reads at -5.
 *   SocialEffectRowBase  0x00946598, SocialEffects (faction.h, 0x00946580)
 *                        + 0x18, rows 0x68 apart, end 0x00946A10; the Null
 *                        row the filler skips is 0x009466D0.
 *   FactionRowBase       0x009472EC, records 0x167 bytes, ids 150001..150008.
 *   TerraformNameBase    0x0096C8AC, three dwords per entry.
 *   TerraformFlagBase    0x00691884, eight dwords per entry - the preq words
 *                        the walk reads at -1 and 0.
 *   ProjectPrefixHidden  0x009BC054, clear while the "The " article is to be
 *                        stripped.
 *   the faction filler publishes each record's head dwords to general.h's
 *   GenderDefault / PluralityDefault pair before filing its name, and reads
 *   alpha.h's Language to decide whether the "The " article stays.
 */
static RulesFacility *const SecretProjectRules = (RulesFacility *)0x009A5888;
static const int *const TechRowBase = (const int *)0x0094F3A4;
static const int *const SocialCategoryRow = (const int *)0x0094B014;
static const char *const SocialEffectRowBase = (const char *)0x00946598;
static const char *const SocialEffectNullRow = (const char *)0x009466D0;
static const char *const SocialEffectRowEnd = (const char *)0x00946A10;
static const char *const FactionRowBase = (const char *)0x009472EC;
static const int *const TerraformNameBase = (const int *)0x0096C8AC;
static const int *const TerraformFlagBase = (const int *)0x00691884;

// The label each category's filler draws centred in the entry rectangle, read
// out of the label blob the way dipedit.cpp reads it:
// `mov eax, ds:0x9b90f8; mov ecx, [eax + <slot>]` - slot / 4 is the index.
#define CATEGORY_LABEL(slot) \
    StringTable.get((int) * ((LPSTR *)Labels.strings_ptr + (slot)))

// See datalink.h. Defined out of line only so VCall's declaration, which
// basewin.cpp also carries locally, is not dragged into every TU that includes
// this header; /Ob2 re-inlines the body at each of the five callers below.
void Datalink::repaint_slot() {
    listBox_.redraw();
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0042BEA0 ?UNK6@Datalink@@QAEXXZ 0x0042BEA0-0x0042BEA1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK6@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0042BF00 ?UNK8@Datalink@@QAEXXZ 0x0042BF00-0x0042BF01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK8@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::UNK8() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00431DB0 ?UNK9@Datalink@@QAEXXZ 0x00431DB0-0x00431DB1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK9@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::UNK9() {
}




/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00431EB0 ?on_left_click@Datalink@@QAEXHH@Z 0x00431EB0-0x00431EB3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_left_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00431FB0 ?on_group_clicked@Datalink@@QAEXHH@Z 0x00431FB0-0x00431FB3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_group_clicked@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_group_clicked(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432000 ?on_mouse_move@Datalink@@QAEXHH@Z 0x00432000-0x00432003 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432160 ?on_iface_left_click@Datalink@@QAEXHH@Z 0x00432160-0x00432163 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432170 ?on_iface_right_click@Datalink@@QAEXHH@Z 0x00432170-0x00432173 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432180 ?on_iface_left_down@Datalink@@QAEXHH@Z 0x00432180-0x00432183 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432190 ?on_iface_right_down@Datalink@@QAEXHH@Z 0x00432190-0x00432193 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321A0 ?on_iface_mouse_move@Datalink@@QAEXHH@Z 0x004321A0-0x004321A3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321B0 ?on_iface_mouse_leave@Datalink@@QAEXHH@Z 0x004321B0-0x004321B3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_mouse_leave(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321C0 ?on_iface_selected@Datalink@@QAEXHH@Z 0x004321C0-0x004321C3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321D0 ?on_iface_left_double_click@Datalink@@QAEXHH@Z 0x004321D0-0x004321D3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321E0 ?on_iface_right_double_click@Datalink@@QAEXHH@Z 0x004321E0-0x004321E3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321F0 ?on_iface_button_clicked@Datalink@@QAEXH@Z 0x004321F0-0x004321F3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_clicked@Datalink@@QAEXH@Z)(Datalink* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_button_clicked(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432200 ?on_iface_button_toggled@Datalink@@QAEXHH@Z 0x00432200-0x00432203 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_button_toggled(int, int) {
}


/*
Purpose: Close the datalink display, clearing the three values it caches.
// ORIGINAL: 0x0042BD20 ?close@Datalink@@QAEXXZ 0x0042BD20-0x0042BD35 BYTE_EXACT
// size      21 bytes
// prototype void (__thiscall ?close@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::close() {
    history_[0] = 0;
    field_2A34_ = 0;
    field_2A38_ = 0;
}


/*
Purpose: Show the technology help topic for the given id.
// ORIGINAL: 0x0044C880 ?help_tech@@YAXH@Z 0x0044C880-0x0044C895 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_tech(int id) {
    DatalinkMain.exec(0xE, id);
}

/*
Purpose: Show the weapon help topic for the given id.
// ORIGINAL: 0x0044C910 ?help_weapon@@YAXH@Z 0x0044C910-0x0044C925 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the armor help topic for the given id.
// ORIGINAL: 0x0044C940 ?help_armor@@YAXH@Z 0x0044C940-0x0044C955 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the chassis help topic for the given id.
// ORIGINAL: 0x0044C980 ?help_chassis@@YAXH@Z 0x0044C980-0x0044C995 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the facility help topic for the given id.
// ORIGINAL: 0x0044C9B0 ?help_facility@@YAXH@Z 0x0044C9B0-0x0044C9C5 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the secret project/ability help topic for the given id.
// ORIGINAL: 0x0044CA10 ?help_abil@@YAXH@Z 0x0044CA10-0x0044CA25 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the social engineering help topic for the given id.
// ORIGINAL: 0x0044CA40 ?help_social@@YAXH@Z 0x0044CA40-0x0044CA55 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the faction help topic for the given id.
// ORIGINAL: 0x0044CA70 ?help_faction@@YAXH@Z 0x0044CA70-0x0044CA85 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the vehicle help topic for the given id.
// ORIGINAL: 0x0044CAD0 ?help_veh@@YAXH@Z 0x0044CAD0-0x0044CAE5 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show an arbitrary help topic/index pair, for callers that already
         hold the topic id rather than going through one of the typed
         help_* wrappers above.
// ORIGINAL: 0x0044CB60 ?help_topic@@YAXIH@Z 0x0044CB60-0x0044CB77 BYTE_EXACT
// size      23 bytes
// prototype void (__cdecl ?help_topic@@YAXIH@Z)(DatalinkID id, int entryID)
// callers   9   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Combine two values as `a1 * 10000 + a2`.

             lea eax,[eax+eax*4]  x4   -> a1 * 625
             shl eax,4                 -> a1 * 10000
             add eax,ecx

         The four `lea` steps multiply by five each time and the shift by
         sixteen, which is 625 * 16 = 10000. Written as the constant, because
         the shift-and-add chain is the compiler's encoding of it and not a
         behaviour of its own - it wraps identically at 32 bits either way.

         Touches no field; both operands are arguments.
// ORIGINAL: 0x0042A020 ?UNK1@Datalink@@QAEHHH@Z 0x0042A020-0x0042A03E BYTE_EXACT
// size      30 bytes
// prototype int (__thiscall ?UNK1@Datalink@@QAEHHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: a1 * 10000 + a2, wrapping at 32 bits
Status: Complete
*/
int Datalink::UNK1(int a1, int a2) {
    return static_cast<int32_t>(static_cast<uint32_t>(a1) * 10000U
                                + static_cast<uint32_t>(a2));
}


/*
Purpose: Split a combined id into its two halves - `id / 10000` into the first
         output and `id % 10000` into the second.

             mov eax,0x68DB8BAD / imul ecx / sar edx,0xC
             mov eax,edx / shr eax,0x1F / add edx,eax   ; id / 10000
             mov [eax],edx
             lea .. x4 / shl edx,4 / sub ecx,edx        ; id - quotient*10000
             mov [eax],ecx

         The magic multiply is a signed divide by 10000, checked against C++
         truncating division over 200,000 random dividends and the boundaries,
         and the remainder is formed as `id - quotient * 10000` rather than by
         a second divide - which is what C++ `%` means for truncating division,
         verified the same way.

         This is the exact INVERSE of UNK1 above, which builds `a1 * 10000 + a2`
         from a shift-and-add chain. The two were decoded independently and
         agree on the constant.

         Neither output is null-checked, because the original checks neither.
// ORIGINAL: 0x0042A040 ?parse_id@Datalink@@QAEXHPAUDatalinkID@@PAH@Z 0x0042A040-0x0042A076 BYTE_EXACT
// size      54 bytes
// prototype int (__thiscall ?parse_id@Datalink@@QAEXHPAUDatalinkID@@PAH@Z)(Datalink* this, int, DatalinkID*, int*)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::parse_id(int id, DatalinkID *out_id, int *out_remainder) {
    const int32_t quotient = id / 10000;
    // Only the first four bytes of the DatalinkID are written; the type stays
    // incomplete, so the store is expressed against its address.
    *reinterpret_cast<int32_t *>(out_id) = quotient;
    *out_remainder = static_cast<int32_t>(
        static_cast<uint32_t>(id) - static_cast<uint32_t>(quotient) * 10000U);
}


/*
// ORIGINAL: 0x00432040 ?on_selected@Datalink@@QAEXH@Z 0x00432040-0x00432052 BYTE_EXACT
// size      18 bytes
// prototype void (__thiscall ?on_selected@Datalink@@QAEXH@Z)(Datalink* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0042BF10
Status: Complete
*/
void Datalink::on_selected(int index) {
    int v = field_2A38_;
    if (v == 0) {
        draw_entry();
    }
}

/*
// ORIGINAL: 0x0042BD40 ?show@Datalink@@QAEXH@Z 0x0042BD40-0x0042BD5B BYTE_EXACT
// symbol    ?show@Datalink@@UAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?show@Datalink@@QAEXH@Z)(Datalink* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void Datalink::show(int visible) {
    if (!reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<SubInterface *>(reinterpret_cast<char *>(this) + 0xa14)->set_iface_mode();
    }
}

/*
// ORIGINAL: 0x0042BD60 ?hide@Datalink@@QAEXXZ 0x0042BD60-0x0042BD79 BYTE_EXACT
// symbol    ?hide@Datalink@@UAEXXZ
// size      25 bytes
// prototype void (__thiscall ?hide@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x005F7E90
Status: Complete
*/
void Datalink::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<SubInterface *>(reinterpret_cast<char *>(this) + 0xa14)->release_iface_mode();
    }
}



/*
Purpose: Push a topic onto the back-history ring UNK5 pops, unless the panel
         is already showing it or the ring is switched off.

         The 10000-folded topic id is compared against the ring slot AFTER
         the last one pushed - `0x29e0 + count*4` - and stored one slot past
         that, `0x29e4 + count*4`. Slot 0 of the ring is the facility id, so
         the first store lands at ring[1] and never at ring[0]: that is the
         original's own arithmetic, not a slipped base.
// ORIGINAL: 0x0042BD80 ?UNK4@Datalink@@QAEXHH@Z 0x0042BD80-0x0042BDFA FILE
// MISMATCH 19/39, first divergence at 6. TRIED: the ring declared as `facilityID_` plus a tail array (two bases where the image's compare and store read `0x29e0+count*4` and `0x29e4+count*4` off one), and the eviction loop as `int *slot = &history_[1]` (which matches).
//            The image's catalogue name is ours to spell from behaviour:
//            the catalogue calls this ?UNK4@Datalink@@QAEXHH@Z.
// symbol    ?push_history@Datalink@@QAEXHH@Z
// size      122 bytes
// prototype void (__thiscall ?UNK4@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: a single named `int *ring` base for the 20-entry buffer (the compiler keeps it live across the whole body and schedules the two prologue pushes differently); recomputing `self+0x29e0` and `self+0x29e4` separately (matching the two distinct bases the disasm itself uses) got the divergence from #4 to #29, where `--*(int*)(...); count = *(int*)(...);` CSEs into a register (`mov`) instead of the original's separate `dec [mem]` + reload.
Return Value: n/a
Status: Complete
*/
void Datalink::push_history(int category, int index) {
    if (field_2A38_ != 0) {
        return;
    }

    const int topic = category * 10000 + index;
    int count = field_2A34_;
    if (count != 0 && history_[count] == topic) {
        return;
    }

    int *slot = &history_[1];
    if (count == 0x14) {
        for (int i = 0; i < 0x13; ++i) {
            slot[i] = slot[i + 1];
        }
        --field_2A34_;
        count = field_2A34_;
    }

    slot[count] = topic;
    field_2A34_ = count + 1;
}

/*
Purpose: Pop the back-history ring and re-show the topic it held.

         Reads `0x29e0 + --count*4`, the slot UNK4 last wrote, splits it back
         into its category and facility halves the way parse_id does, then
         walks the chain set_cat / set_selected_id / slot 62 / draw_entry that
         UNK3 runs in the forward direction.
// ORIGINAL: 0x0042BE00 ?UNK5@Datalink@@QAEXXZ 0x0042BE00-0x0042BE9D FILE
// MISMATCH 8/49, first divergence at 4 - the receiver-spill order again: the image takes `this` into esi after saving ebx/edi, ours before. TRIED: `history_[--field_2A34_]` for the pop, `topic - category*10000` for the remainder instead of `%`, and a hoisted `&listBox_` base.
//            The image's catalogue name is ours to spell from behaviour:
//            the catalogue calls this ?UNK5@Datalink@@QAEXXZ.
// symbol    ?pop_history@Datalink@@QAEXXZ
// size      157 bytes
// prototype void (__thiscall ?UNK5@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0042A2C0 0x0042BF10 0x00609FA0
// indirect  0x0042BE8C
Return Value: n/a
Status: Complete
*/
void Datalink::pop_history() {
    if (field_2A34_ <= 1) {
        return;
    }

    const int topic = history_[--field_2A34_];
    const int category = topic / 10000;
    const int facility = topic - category * 10000;

    set_cat(static_cast<unsigned int>(category), 0);
    history_[0] = facility;
    listBox_.set_selected_id(field_29DC_ * 10000 + facility);
    repaint_slot();
    draw_entry();
}

/*
Purpose: Show one topic outright - the forward direction of UNK5.

         Writes the facility id into ring slot 0, so the topic UNK4 pushes
         next lands after it, then runs the same chain set_cat /
         set_selected_id / slot 62 that every other entry point here runs.
// ORIGINAL: 0x0042A260 ?UNK3@Datalink@@QAEXIH@Z 0x0042A260-0x0042A2BF FILE
// TRIED: inlining repaint_slot in the header - basewin.cpp carries its own local `class VCall`, which then collides with vtable_shim.h's, so the definition lives in this TU and /Ob2 re-inlines it. MISMATCH 6/31 - the image is 31 instructions where ours is 25: its `push edi` sits before the `push 0` argument of set_cat, ours after, so the argument is evaluated before the callee-saved saves here and after them there.
//            The image's catalogue name is ours to spell from behaviour:
//            the catalogue calls this ?UNK3@Datalink@@QAEXIH@Z.
// symbol    ?show_topic@Datalink@@QAEXIH@Z
// size      95 bytes
// prototype void (__thiscall ?UNK3@Datalink@@QAEXIH@Z)(Datalink* this, DatalinkID, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0042A2C0 0x00609FA0
// indirect  0x0042A2B3
Return Value: n/a
Status: Complete
*/
void Datalink::show_topic(unsigned int category, int index) {
    set_cat(category, 0);
    history_[0] = index;
    listBox_.set_selected_id(field_29DC_ * 10000 + index);
    repaint_slot();
}

/*
Purpose: Follow a hyperlink in the topic text: "category*10000+facility".

         Facilities above 1000 are not real ones - they are the ids the
         prototype list invented - so they are ignored. The return is 0
         whatever happened; the image returns 0 on the ignored path and
         falls off the end (also 0) after the chain.
// ORIGINAL: 0x00431DC0 ?on_link_clicked@Datalink@@QAEHHPAD@Z 0x00431DC0-0x00431E64 FILE
// MISMATCH 24/58, first divergence at 24 - the image reloads the facility slot after the set_cat call where ours folds it into the store. TRIED: the guard as `facility > 1000` in both polarities, and `atoi` against the CRT's own binding.
// size      161 bytes
// prototype int (__thiscall ?on_link_clicked@Datalink@@QAEHHPAD@Z)(Datalink* this, int, char *)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0042A2C0 0x00609FA0
// indirect  0x00431E5B
Return Value: always 0
Status: Complete
*/
int Datalink::on_link_clicked(int, char *link) {
    const int topic = atoi(link);
    const int category = topic / 10000;
    const int facility = topic - category * 10000;

    if (facility > 1000) {
        return 0;
    }

    set_cat(static_cast<unsigned int>(category), 0);
    history_[0] = facility;
    listBox_.set_selected_id(field_29DC_ * 10000 + facility);
    repaint_slot();
    return 0;
}

/*
Purpose: Rebuild the topic list for one category and redraw it.

         The early-out asks three questions in the image's order: was this a
         plain re-select (index == 0), is that category already the one on
         show, and does the entry list still hold anything. Only when all
         three hold does it do nothing.

         The 0xE test and the unconditional second reset bracket the category
         store - the report window is told twice when the category was 0xE,
         once before and once after.

         Category 0xC skips the sort that every other category gets, and so
         falls straight into the update. The image's `cmp ebx, 0xc / je` after
         the jump table is that same skip reached from the switch's default
         arm, which no category above 0xF can satisfy.
// ORIGINAL: 0x0042A2C0 ?set_cat@Datalink@@QAEXIH@Z 0x0042A2C0-0x0042A4AC FILE
// MISMATCH 34/152, first divergence at 3. TRIED: transcribing StringStruct::remove_all's walk inline the way the image does - it is a real body here (stringstruct.cpp), so a call is what ours emits, 139 instructions against the image's 152.
// size      492 bytes
// prototype void (__thiscall ?set_cat@Datalink@@QAEXIH@Z)(Datalink* this, DatalinkID, int)
// callers   6   call targets   20
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0042A4F0 0x0042A630 0x0042A790 0x0042A900 0x0042AA60 0x0042ABC0 0x0042AD20 0x0042AE80 0x0042AFE0 0x0042B190 0x0042B300 0x0042B490 0x0042B630 0x0042B7E0 0x0042B940 0x0042BAB0 0x00496BB0 0x006087A0 0x0060C9B0 0x0062B870
// indirect  0x0042A34D 0x0042A35F 0x0042A37F
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat(unsigned int category, int index) {
    if (index == 0 && field_29DC_ == category
        && listBox_.item_list_.entry_count_ != 0) {
        return;
    }

    // The topic panel's report window - the fixed 0x00876478 object
    // reportif.cpp names ReportIfModalWindow.
    if (field_29DC_ == 0xE) {
        reinterpret_cast<ReportWin *>(0x876478)->reset();
    }

    field_29DC_ = category;
    reinterpret_cast<ReportWin *>(0x876478)->reset();

    listBox_.item_list_.remove_all();
    listBox_.selected_position_ = 0;
    listBox_.field_18_ = 0;
    listBox_.field_20_ = 0;

    buttonGroup_.set(static_cast<int>(category), 0);

    switch (category) {
    case 2:  set_cat_all();         break;
    case 3:  set_cat_unit();        break;
    case 4:  set_cat_chassis();     break;
    case 5:  set_cat_reactor();     break;
    case 6:  set_cat_weapon();      break;
    case 7:  set_cat_defense();     break;
    case 8:  set_cat_ability();     break;
    case 9:  set_cat_terraform();   break;
    case 10: set_cat_facility();    break;
    case 11: set_cat_project();     break;
    case 12: set_cat_social();      break;
    case 13: set_cat_effect();      break;
    case 14: set_cat_tech();        break;
    case 15: set_cat_faction();     break;
    case 0:  set_cat_concept();     break;
    case 1:  set_cat_adv_concept(); break;
    }

    if (category != 0xC) {
        listBox_.item_list_.sort(0);
    }

    field_2A38_ = 1;
    listBox_.update_changes();
    field_2A38_ = 0;
}

// ---- the fourteen category fillers ----------------------------------------
//
// Every one walks its rules table, appends the matching names to StringTemp
// (with a "|" and the entry's own id when `GameState & 0x1000` says the ids
// should be visible), hands each line to the entry list, then frames the
// shared category rectangle, centres the category's label in it and repaints.
// The eight lines of the tail are the same eight lines in all of them, which
// is why they read as a family rather than as one function.

/*
Purpose: Refresh all fifteen categories at once - the "ALL" entry of the
         category strip.

         The image's call order is not sorted: terraform comes seventh and
         facility ninth, and that order is what this body keeps.
// ORIGINAL: 0x0042A4F0 ?set_cat_all@Datalink@@QAEXXZ 0x0042A4F0-0x0042A621 FILE BYTE_EXACT
// size      305 bytes
// prototype void (__thiscall ?set_cat_all@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   15
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0042A630 0x0042A790 0x0042A900 0x0042AA60 0x0042ABC0 0x0042AD20 0x0042BAB0 0x0042B300 0x0042AE80 0x0042AFE0 0x0042B630 0x0042B7E0 0x0042B190 0x0042B940
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_all() {
    set_cat_unit();
    set_cat_chassis();
    set_cat_reactor();
    set_cat_weapon();
    set_cat_defense();
    set_cat_ability();
    set_cat_terraform();
    set_cat_concept();
    set_cat_facility();
    set_cat_project();
    set_cat_social();
    set_cat_effect();
    set_cat_tech();
    set_cat_faction();

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(714);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the built unit designs, ids 30000 upward.

         Prototype 4 is skipped outright - it is the placeholder slot - and
         the walk stops after a hundred prototypes regardless of how many the
         rules file defines.
// ORIGINAL: 0x0042A630 ?set_cat_unit@Datalink@@QAEXXZ 0x0042A630-0x0042A78E FILE BYTE_EXACT
// size      350 bytes
// prototype void (__thiscall ?set_cat_unit@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_unit() {
    for (int index = 0; index < 100; ++index) {
        VehPrototype &proto = VehPrototypes[index];
        if ((proto.flags & 1) != 0 && index != 4) {
            StringTemp[0] = '\0';
            strcat(StringTemp, proto.veh_name);
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 30000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 30000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(715);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the chassis, ids 40000 upward.

         The preq test reads the LAST word of the 0x90 record, which is where
         veh.h puts preq_tech.
// ORIGINAL: 0x0042A790 ?set_cat_chassis@Datalink@@QAEXXZ 0x0042A790-0x0042A8FA FILE BYTE_EXACT
// size      356 bytes
// prototype void (__thiscall ?set_cat_chassis@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_chassis() {
    for (int index = 0; index < MaxChassisNum; ++index) {
        if (Chassis[index].preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)Chassis[index].offsv_1_name));
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 40000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 40000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(716);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the reactors, ids 50001 upward - the table starts at id 1, not
         0.

         The walk runs five records against a four-entry array: the fifth is
         the zero record past the end, whose name id 0 makes it an empty line.
         That is the original's own bound, reproduced here on purpose.
// ORIGINAL: 0x0042A900 ?set_cat_reactor@Datalink@@QAEXXZ 0x0042A900-0x0042AA5A FILE BYTE_EXACT
// size      346 bytes
// prototype void (__thiscall ?set_cat_reactor@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_reactor() {
    int id = 50001;
    const RulesReactor *record = Reactor;
    for (; id - 50000 < 5; ++record, ++id) {
        if (record->preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)record->name));
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(id, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, id);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(717);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the weapons, ids 60000 upward.
// ORIGINAL: 0x0042AA60 ?set_cat_weapon@Datalink@@QAEXXZ 0x0042AA60-0x0042ABBC FILE BYTE_EXACT
// size      346 bytes
// prototype void (__thiscall ?set_cat_weapon@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_weapon() {
    for (int index = 0; index < MaxWeaponNum; ++index) {
        if (Weapon[index].preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)Weapon[index].name));
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 60000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 60000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(718);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the armour, ids 70000 upward.
// ORIGINAL: 0x0042ABC0 ?set_cat_defense@Datalink@@QAEXXZ 0x0042ABC0-0x0042AD1C FILE BYTE_EXACT
// size      346 bytes
// prototype void (__thiscall ?set_cat_defense@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_defense() {
    for (int index = 0; index < MaxArmorNum; ++index) {
        if (Armor[index].preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)Armor[index].name));
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 70000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 70000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(719);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the abilities, ids 80000 upward.
// ORIGINAL: 0x0042AD20 ?set_cat_ability@Datalink@@QAEXXZ 0x0042AD20-0x0042AE7C FILE BYTE_EXACT
// size      346 bytes
// prototype void (__thiscall ?set_cat_ability@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_ability() {
    for (int index = 0; index < MaxAbilityNum; ++index) {
        if (Ability[index].preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)Ability[index].name));
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 80000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 80000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(720);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}


/*
Purpose: List the facilities, ids 100001 upward - the table's slot 0 is the
         unused one, so the walk starts one record in.

         The preq word is at +0x14, the fifth dword, which is where base.h
         puts preq_tech.
// ORIGINAL: 0x0042AE80 ?set_cat_facility@Datalink@@QAEXXZ 0x0042AE80-0x0042AFDC FILE BYTE_EXACT
// size      345 bytes
// prototype void (__thiscall ?set_cat_facility@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_facility() {
    int id = 100001;
    const RulesFacility *record = Facility + 1;
    for (; id - 100000 < 0x46; ++record, ++id) {
        if (record->preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)record->name));
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(id, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, id);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(722);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the secret projects, ids 68534 upward.

         When `ProjectPrefixHidden` is clear the "The " prefix the rules file
         carries is stripped off the front of the name first, so the list
         reads "Ascension" rather than "The Ascension".
// ORIGINAL: 0x0042AFE0 ?set_cat_project@Datalink@@QAEXXZ 0x0042AFE0-0x0042B18E FILE
// MISMATCH 10/133 against our 113, first divergence at 2 - the image keeps a pointer to the current record; ours indexes SecretProjectRules every iteration. TRIED: the record hoisted out of the loop as `const RulesFacility *`.
// size      426 bytes
// prototype void (__thiscall ?set_cat_project@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_project() {
    for (int index = 0; index < 0x40; ++index) {
        if (SecretProjectRules[index].preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp,
                   StringTable.get((int)SecretProjectRules[index].name));
            if (Language == 0 && strncmp(StringTemp, ConceptArticle, 4) == 0) {
                char full[256];
                strcpy(full, StringTemp);
                strcpy(StringTemp, full + 4);
            }
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 68534, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 68534);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(723);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the technologies, ids 140000 upward.

         The technology record the tree materialises is eleven dwords wide and
         the walk reads its two preq words at -1 and 0 from the pointer it
         holds, which is why both are tested before the name at -9 is fetched.
// ORIGINAL: 0x0042B190 ?set_cat_tech@Datalink@@QAEXXZ 0x0042B190-0x0042B2FA FILE
// MISMATCH 31/116, first divergence at 6. LEVER: `char digits[0x54]` not [80] - the image's frame is 0x64 and the 80-byte buffer gives 0x60. TRIED: the record walk as `record[-1] >= -1 && record[0] >= -1` against a hoisted `TechRowBase`, which is the image's own shape (base in esi, stepped 11 dwords); the residual divergence is the image taking `this` into edi and ours into ebx.
// size      355 bytes
// prototype void (__thiscall ?set_cat_tech@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_tech() {
    const int *record = TechRowBase;
    for (int index = 0; index < 0x59; ++index) {
        if (record[-1] >= -1 && record[0] >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(record[-9]));
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[0x54];
                _itoa(index + 140000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 140000);
        }
        record += 11;
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(726);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the concept encyclopedia entries straight out of CONCEPTS.TXT.

         The walk is the text reader's own: get a line, file it, get the next,
         until the reader hands back an empty one. ids run 0 upward, which is
         why this is the one category whose ids are not five-digit.
// ORIGINAL: 0x0042B300 ?set_cat_concept@Datalink@@QAEXXZ 0x0042B300-0x0042B48C FILE
// MISMATCH 36/125, first divergence at 3 - register pressure, not frame size: the frame matches at 0x60 but the image saves only edi where ours saves ebx/esi/edi, because the reader loop keeps the line pointer live across the text_get call here.
// size      391 bytes
// prototype void (__thiscall ?set_cat_concept@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330 0x005FD570 0x005FD820 0x005FD910
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_concept() {
    if (text_open(ConceptSection, ConceptFile) == 0) {
        text_get();
        for (int index = 0; TextBufferGetPtr != 0
                            && strlen(TextBufferGetPtr) != 0;
             ++index) {
            StringTemp[0] = '\0';
            strcat(StringTemp, TextBufferGetPtr);
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index);
            text_get();
        }
        text_close();
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(712);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the advanced concepts, the second CONCEPTS.TXT section.

         Same reader walk as set_cat_concept, with the ids starting at 10000
         so the two lists cannot collide.
// ORIGINAL: 0x0042B490 ?set_cat_adv_concept@Datalink@@QAEXXZ 0x0042B490-0x0042B62A FILE
// MISMATCH 32/127, first divergence at 10. TRIED: the reader walk spelled exactly as set_cat_concept's, with the early return as the image's inverted `!= 0`.
// size      410 bytes
// prototype void (__thiscall ?set_cat_adv_concept@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330 0x005FD570 0x005FD820 0x005FD910
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_adv_concept() {
    if (text_open(ConceptSection, AdvConceptFile) != 0) {
        return;
    }

    text_get();
    for (int index = 0;
         TextBufferGetPtr != 0 && strlen(TextBufferGetPtr) != 0; ++index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, TextBufferGetPtr);
        if ((GameState & 0x1000) != 0) {
            strcat(StringTemp, CategoryIdSeparator);
            char digits[80];
            _itoa(index + 10000, digits, 10);
            strcat(StringTemp, digits);
        }
        listBox_.item(StringTemp, index + 10000);
        text_get();
    }
    text_close();

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(713);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the social engineering categories and, under each, the four
         choices it offers in that column.

         The header line carries the category's own id - 120000 upward - and
         the four lines under it carry the choice ids. Rows are 0xD4 apart and
         the four choices are the first four dwords of the row.
// ORIGINAL: 0x0042B630 ?set_cat_social@Datalink@@QAEXXZ 0x0042B630-0x0042B7DC FILE
// MISMATCH 130/137, first divergence at 12 - the loop tail reloads `category` before `row` here and the image the other way round. TRIED: the increment reordered `row += 53, category += 5` to swap them - 123/137 against 130/137, so the source order is already the image's.
// size      421 bytes
// prototype void (__thiscall ?set_cat_social@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_social() {
    const int *row = SocialCategoryRow;
    for (int category = 120001; category < 120021; category += 5, row += 53) {
        listBox_.item(StringTable.get(row[-5]), category - 1);

        for (int choice = 0; choice < 4; ++choice) {
            int id = category + choice;
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(row[choice]));
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(id, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, id);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(724);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the social effects, ids 130000 upward.

         Row 3 - the NullEffect slot - is skipped outright, which is why the
         list is one shorter than the table.
// ORIGINAL: 0x0042B7E0 ?set_cat_effect@Datalink@@QAEXXZ 0x0042B7E0-0x0042B938 FILE
// MISMATCH 21/110, first divergence at 6 - register allocation around the loop counter. TRIED: the walk as `row < SocialEffectRowEnd` with the Null row excluded by address, and the name read as `row` itself (the effects table holds its LPSTRs inline).
// size      340 bytes
// prototype void (__thiscall ?set_cat_effect@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_effect() {
    const char *row = SocialEffectRowBase;
    for (int index = 0; row < SocialEffectRowEnd; row += 0x68, ++index) {
        if (row != SocialEffectNullRow) {
            StringTemp[0] = '\0';
            strcat(StringTemp, row);
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 130000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 130000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(725);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the factions, ids 150000 upward.

         Before each name is filed the two dwords at the head of the record
         are published to the faction-id pair general.h owns, so the list
         builder knows which faction the line it is adding belongs to.
// ORIGINAL: 0x0042B940 ?set_cat_faction@Datalink@@QAEXXZ 0x0042B940-0x0042BACC FILE
// MISMATCH 16/135 against our 111 - the image's frame is 24 bytes larger, so its digits buffer is not the 80 ours is. TRIED: `record += 0x167` on a `const char *` with the head dwords published through general.h's GenderDefault / PluralityDefault, and the name read at +0x84.
// size      355 bytes
// prototype void (__thiscall ?set_cat_faction@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_faction() {
    const char *record = FactionRowBase;
    for (int index = 0; index < 8; record += 0x167, ++index) {
        GenderDefault = reinterpret_cast<const int *>(record)[-1];
        PluralityDefault = reinterpret_cast<const int *>(record)[0];
        StringTemp[0] = '\0';
        strcat(StringTemp, record + 0x84);
        if ((GameState & 0x1000) != 0) {
            strcat(StringTemp, CategoryIdSeparator);
            char digits[80];
            _itoa(index + 150001, digits, 10);
            strcat(StringTemp, digits);
        }
        listBox_.item(StringTemp, index + 150001);
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(727);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}


/*
Purpose: List the terraforming options, ids 90000 upward.

         Two tables drive this one - the name ids from one, the preq/flag
         words from another - and every pass handles TWO entries, the second
         carrying its own preq test and its own id-suffix separator. The two
         ids 0x15FB4 and 0x15FB5 (raise/lower terrain) never reach the list:
         the second half of their pass is skipped, which is the image's own
         pair of compares and not a slipped loop bound.

         The goto shape is the image's: the suffix append and the `item` call
         share one exit, so the "no ids" case still files the name.
// ORIGINAL: 0x0042BAB0 ?set_cat_terraform@Datalink@@QAEXXZ 0x0042BAB0-0x0042BCA4 FILE
// MISMATCH 14/150 against our 159, first divergence at 8. TRIED: the second entry's skip spelled as the image's own pair of compares on ids 90036/90037 and as a `continue` - the two produce the same 159.
// size      513 bytes
// prototype void (__thiscall ?set_cat_terraform@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_terraform() {
    const int *names = TerraformNameBase;
    const int *flags = TerraformFlagBase;
    for (int category = 90020; category - 90020 < 0x14;
         flags += 8, names += 3, ++category) {
        if (flags[-1] >= -1 && names[-1] != 0) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(names[-1]));
            if ((GameState & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(category - 20, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, category - 20);
        }

        if (category == 90036 || category == 90037) {
            continue;
        }
        if (flags[0] < -1) {
            continue;
        }

        StringTemp[0] = '\0';
        strcat(StringTemp, StringTable.get(names[0]));
        if ((GameState & 0x1000) != 0) {
            strcat(StringTemp, TerraformSecondSeparator);
            char digits[80];
            _itoa(category, digits, 10);
            strcat(StringTemp, digits);
        }
        listBox_.item(StringTemp, category);
    }

    RECT rect = category_rect_;
    box_sprite(&rect, (BoxSpriteParams *)BoxSpriteStyle6);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(721);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

// ---- seams the category fillers call out to -------------------------------
//
// Three bodies set_cat reaches are real image functions nobody has recovered
// yet, and their declarations above need definitions or the link breaks. These
// are the pending-body forwarders they need, parked HERE rather than in
// pending_bodies.cpp so the datalink batch stays one file's work; they retire
// the day the real bodies land in listbox.cpp, stringstruct.cpp and
// reportwin.cpp, and until then each one jumps into the image's own address,
// which is the loud failure the pending-body convention asks for.

int ListBox::update_changes() {  // 0x0060C9B0
    typedef int(__fastcall *pending)(ListBox *, void *);
    return reinterpret_cast<pending>(static_cast<unsigned long>(0x0060C9B0))(
        this, nullptr);
}

void StringList::sort(int key) {  // 0x006087A0
    typedef void(__fastcall *pending)(StringList *, void *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x006087A0))(this,
                                                                     nullptr,
                                                                     key);
}

void ReportWin::reset() {  // 0x00496BB0
    typedef void(__fastcall *pending)(ReportWin *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00496BB0))(this,
                                                                     nullptr);
}

// 0x00431EC0 ?on_button_clicked@Datalink@@QAEXH@Z - still an original body
// (src/unrecovered/00431ec0.cpp, claimed). The void spelling makes it a real
// override of Win's slot-44 on_button_clicked, so the link needs a definition
// this soon; the forwarder retires when the body homes.
void Datalink::on_button_clicked(int button_id) {
    typedef void(__fastcall *pending)(Datalink *, void *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00431EC0))(
        this, nullptr, button_id);
}
