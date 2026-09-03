#!/usr/bin/env -S uv run python
r"""Semantic debt: what a body can carry while measuring BYTE_EXACT.

The byte ratchet proves the COMPILED BYTES reproduce the image. It is
structurally blind to everything that lowers identically: a method named
UNK2, an `operator=` spelled `__as` returning `int`, a dead `*_redirect`
wrapper nothing calls, a function address smuggled through a `void *` data
global. All of it measured exact in palette.cpp on 2026-08-24, because the
bytes WERE exact - the lies live in the source model, not the object file.

Five shapes, counted like compiler_work.py: line-wise over the product tree
(src/*.cpp + *.h; the artifact archives are excluded - they die by homing,
not by cleanup), comment lines skipped, a ceiling per shape that only falls.
`--check` is wired into the gate; the class pass (recover-class.md pass 6)
is what lowers the numbers, and a class is DONE only at debt 0.

    uv run tools/class_debt.py            # the census, with what to do
    uv run tools/class_debt.py --check    # exit 1 if any shape grew
    uv run tools/class_debt.py --json     # per-file counts, for --by-class
"""
from __future__ import annotations

import collections
import json
import re
import sys
from bisect import bisect_right
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))

from decomp import read  # noqa: E402

# Measured 2026-08-24 on a clean tree at 09c18640. Lower a ceiling when its
# count falls - the gate fails on slack, same as compiler_work. The zero is
# not decorative: 284 trivial BYTE_EXACT claims were scanned and every one
# carries Purpose/Verification prose (thunk families document per-file);
# the ceiling holds that at zero. All 66 function-address bindings were
# eyeballed before pinning - vector-dtor iterators, atexit callbacks, and
# `g_00406850`-style disguises, every one a real function reference.
CEILINGS = {
    # 170 -> 162, same edit as the evidence-free reclassification above:
    # BaseWin's four UNK methods move shapes, they do not disappear.
    # 162 -> 170 (2026-09-02, DipEdit UNK1-4 batch): the four checkbox
    # handlers are catalogued ?UNK1@DipEdit@@QAEXH@Z .. ?UNK4@DipEdit@@QAEXH@Z,
    # and that catalogue name is what our own object has to emit for the claim
    # to match - AGENTS.md's mangled-name rule, not a naming failure. The
    # image offers nothing better: UNK1 has no symbol, no E8 caller, no vtable
    # slot and no data word anywhere in the shipped binary (every section
    # scanned for 0x004DA760), and UNK2-4 are reached only by read_check. Not
    # EVIDENCE-FREE, so the escape hatch does not apply: UNK1 has a
    # member-accesses.csv entry and all four have measured behaviour, which is
    # written down in the markers in src/dipedit.cpp. Renaming them to clear
    # this count would trade a true symbol for a false name.
    # 170 -> 176 -> 162 (2026-09-02, DipEdit batch then rename): the read_check
    # homing added the six call sites that drive the four catalogue-named
    # helpers; the rename then took all fourteen code-line mentions out -
    # UNK1..UNK4 are now sync_treaty_bit/sync_agenda_bit/set_treaty_bit/
    # set_agenda_bit, named from the measured 2x2 (which word, mirrored or
    # single-row writer), with `// symbol` facts carrying the emitted names
    # under the catalogue's markers. The image names nothing (every PE
    # section scanned), so the new names are ours, evidenced by behaviour.
    # 162 -> 167 (2026-09-03, menu batch): Menu::UNK5 and Menu::UNK11 homed
    # out of src/unrecovered/005fb240.cpp and src/recovered/units/005fb8e0.cpp
    # into src/menu.cpp. UNKn is the image's own catalogue name here - the
    # bodies are search-and-forward to a PullDown member, so nothing in them
    # says what they are FOR. The homing is coverage; naming them is the
    # class pass's job.
    # 167 -> 169 (2026-09-03, the PullDown-declaration batch): Menu::UNK12
    # homed the same way, out of src/recovered/units/005fb930.cpp into
    # src/menu.cpp BYTE_EXACT - its menu.h declaration and its definition are
    # the two mentions. Same shape as UNK5/UNK11: search-and-forward, the
    # catalogue name is all the image gives.
    # 169 -> 171 (2026-09-03, P1b ambience/sounddevice batch): Midi::UNK
    # and TexHeap-family UNKn homed with their claims - the catalogue name
    # is the match (the DipEdit UNK1-4 precedent).
    # 171 -> 173 (2026-09-03, P1d batch): two more UNKn catalogue names
    # homed with their claims (DipEdit UNK precedent).
    "unk-method": 173,
    # THE ESCAPE HATCH, RATCHETED. An UNKn whose comment block says
    # EVIDENCE-FREE and names it has been investigated and found unnameable -
    # no caller, no dispatch, no CSV entry. That is a result, not unfinished
    # work, and counting it as debt pressures someone into inventing a name.
    # It is ratcheted anyway: widening this requires writing the
    # justification, and the number cannot drift up unnoticed.
    # 18 -> 20 IS A RECLASSIFICATION, NOT A WIDENING: unk-method fell by
    # exactly two in the same edit, when font.h's note was made to NAME the
    # UNK1 it had already investigated and refused to name. The total is
    # unchanged. Raising this ceiling for any other reason means someone is
    # excusing work rather than recording a result - the two counts must
    # move against each other, or it is not a reclassification.
    # 22 -> 31 IS A RECLASSIFICATION, NOT A WIDENING (2026-08-27
    # BASE-CLASS pass): BaseWin's four UNK methods (UNK4, UNK5, UNK6, UNK7)
    # are now documented EVIDENCE-FREE - catalogue callers 0 / call targets
    # 0, no slot in any of BaseWin's three vtables, no CSV entry, no UNK
    # string in the shipped binary - and unk-method fell by exactly the same
    # eight mentions in the same edit (170 -> 162), plus one scaffold
    # parameter the excuse now covers. The totals moved against each other.
    # 31 -> 32 (2026-09-03, P1b ambience/sounddevice batch): one more
    # investigated-and-unnameable homed body, same escape-hatch terms.
    "evidence-free unk": 32,
    # 55 -> 54 (2026-08-29, init_thunks batch): destroy_vehdraw_caviar and
    # siblings reached their globals through raw address casts; the real
    # objects need no cast.
    # 54 -> 38 (2026-08-29, lifecycle-retirement batch): the construct and
    # atexit teardown thunks reached their globals through raw address casts;
    # the CRT-owned lifecycle needs none of them.
    # 38 -> 3 (2026-08-29, fx batch): the vector element callbacks stopped
    # being address constants into the original - each is a real __fastcall
    # function calling the recovered lifecycle body on the element.
    # 3 -> 2 (2026-08-30, boot-path batch): amovie_project's homing retired
    # the last MCI address constant.
    "function-address binding": 2,
    # 208 -> 207 (sound batch): retiring field_accessor_0062d390_redirect for
    # the real `sub_62d390` in general.cpp leaves one fewer redirect nothing
    # calls.
    # 207 -> 206 (batch 4, stringstruct): the orphan string_struct_close_redirect
    # deleted - zero callers; close() performs its own 0x1C, as the image does.
    # 206 -> 203 (TU homing): the dead injected-DLL redirect wrappers around
    # BasePop's two font setters and mandate_color left with the homing.
    # 203 -> 202 (dipedit): the dip_edit_do_check_redirect wrapper removed -
    # zero callers; do_check is called directly.
    # 202 -> 201 (2026-09-03, boot-callee batch): leaf_00455e50_redirect took
    # its image name, load_deswin_sprites, when load_fixed_sprites needed to
    # call it - the seam name was the only thing orphan about it, and the body
    # stays BYTE_EXACT under the rename.
    "orphan redirect": 201,
    "pointer-as-int": 0,
    "undocumented trivial body": 0,
    # 239 IS BELOW WHAT THE TREE CAN REACH TODAY, and this is the only
    # ceiling knowingly left above its count. Homing moved Win's bodies out
    # of the uncounted archives, and 26 of win.cpp's 28 remaining sites are
    # `reinterpret_cast<int>(this)` compared against `int *const` bindings
    # that MEASUREMENT says must keep that type: retyping the three window
    # bindings to `Win **const` cost `show_maximize` and `maximize` their
    # claims (2026-08-25, recorded in win_slots.h). The pun is load-bearing.
    # The rest of the excess is SpriteBox, CheckBox and Dialogs walking
    # their own objects by offset, which needs each of those classes'
    # layout model - their passes, not Win's.
    # CORRECTED with the same fix - and this shape was hit hardest,
    # because `*reinterpret_cast<T *>(self + 0xNN)` IS a line starting
    # with `*`. Eleven of them were invisible to the census that
    # exists to count them.
    # 241 as of the 2026-08-27 WIN pass: Win::Win's member-form rewrite and
    # the palette_seed_cache_/key_hook_ retypes removed two more of these;
    # the pun-per-marker floors they used to sit on are gone.
    # 239 as of the 2026-08-27 WIN pass, second sweep: on_activate's
    # IDirectDrawSurface slots go by name and resize_event's callback/slot
    # dispatch reads its own members; two more offset walks retired.
    # 230 as of the 2026-08-27 BASE-CLASS pass: Win's and Buffer's field
    # blocks went protected (the AutoSound precedent) and GraphicWin's five
    # bodies stopped punning `this` to reach them - the ctor/close
    # field_134_/field_138_ pun, fill's nine memcpy walks, redraw's
    # field_1A0_/field_A10_/rect2_, update's rect2_ pair, init's poOwner_
    # store and iFlags_ read. Every touched claim re-measured BYTE_EXACT,
    # and fill(int) went 1/80 -> 11/80 on the honest form.
    # 230 -> 229, still the 2026-08-27 BASE-CLASS pass: Sound::ramp names
    # device_ instead of walking this+0x3C (claim held 15/15) and
    # BaseWin::show calls its two bases by unqualified name - the
    # static_cast spelling was measured and REFUTED (VC6 inserts a null
    # guard around the upcast that the image has no room for), the
    # unqualified call compiles exactly.
    # 229 -> 228: show's rewrite cleared two census lines, not one - the
    # census counts the pun line AND the SubInterface pointer arithmetic
    # hanging off it, which died together.
    # 228 -> 227, still the base-class pass: Buffer::setup_buff_sprite reads
    # the DIB dimensions and the lock count through its own members instead
    # of a char* pun (unclaimed working copy; score unchanged at 14/64).
    # 224 -> 223 (2026-08-28, scroll batch): compute_thumb_rect's member
    # rewrite reads the thumb RECT through flat_button_right_'s own members
    # instead of a this+0xA4C walk.
    # 223 -> 222 (2026-08-28, buffer batch): get_pixel's char* walk is gone
    # - and the body went 1/107 -> BYTE_EXACT 107/107 on members plus one
    # chained bounds guard sharing a single epilogue.
    # 222 -> 221 (2026-08-29, stringbox batch): StringBox::add walked the
    # StringList through a char* off &field_2B70_; the member is a real
    # StringList now, and add's staging is unchanged on the list pointer.
    # 221 -> 222 (2026-08-30, boot-path batch): +1 from the control_game /
    # caviar homings, proven byte-identical to HEAD by the dispatching
    # agent's clean-base measurement; the class pass claims it.
    # 222 -> 230 (2026-08-29, promotion batch agent-prom2): eight window-event
    # handlers homed from proved artifacts (RadioButton/CheckBox/SpriteBox
    # on_left_double_click + on_left_down, EditGroup on_dialog_focus, Dialogs
    # on_left_click + on_scroll_delete). Each carries exactly ONE
    # reinterpret_cast<char*>(this): the image enters every one of them on a
    # base-subobject-adjusted pointer (the vbtable sits at this-0x18/-0x1C/
    # -0x88, the GraphicWin subobject itself), so the class member model
    # cannot name what `this` points at and the pun IS the layout finding -
    # the family pass that converts these entries to real adjustor thunks
    # claims it back.
    # 227 -> 225 (2026-08-30, Console/MapWin destructor pass): the
    # TextureStore/Buffer/Font/ImageButton carve in mapwin.h and the
    # GraphicWin/Buffer/Time/Menu/Sprite carve in console.h replaced whole
    # blocks of reinterpret_cast member access with named members, and the
    # hand-spelled ~MapWin/~Console bodies came out of the .cpp files with
    # them. ??1MapWin went BYTE_EXACT 73/73 on the declared model.
    # 225 -> 224 (2026-09-01, console real-constructor round): Console(int)
    # dropped its `char *const self = reinterpret_cast<char *>(this)` anchor
    # and the placement news went with it - the compiler builds the declared
    # members.
    # 224 -> 222 (batch 2, leaf families): stripping the Scroll/PushButton/
    # FlatButton/FileWin re-stage stores removed their reinterpret_cast
    # object anchors.
    # 218 -> 217 (batch 4, dialog family): Dialog::destroy's primary overlay
    # store stripped with its constant.
    # 217 -> 216 (batch 4, pulldown): the stripped stores' reinterpret_cast
    # object anchor left with them.
    # 216 -> 214 (batch 5, mapwin attempt #2): MapWin's and PlanWin's
    # construct(int) bodies' raw self casts left with the real constructors.
    # 214 -> 211 (batch 6, dialog/prodpicker): the item-list and the three
    # ProdPicker blocks are real members - the raw self casts of the old
    # item()/a7-block spellings left with them.
    # 211 -> 209 (batch 6, stringstruct dtor chain): close()'s vbtable-walk
    # stores and the redirect's adjusted-receiver recovery left with the hand
    # close chain.
    # 209 -> 205 (batch 7, virtual-base four): CheckBox/RadioButton/EditGroup/
    # SpriteBox ctor+dtor staging blocks stripped - their vbtable reads and
    # raw self casts went with the derived-stage stores.
    # 205 -> 204 (batch 8, listbox): the redirect's walked-back receiver
    # recovery simplified with its re-stores.
    # 204 -> 200 (idiomatic-dispatch direction, 2026-09-01): the walks' raw
    # self casts left with the qualified base calls that replaced them.
    # 200 -> 196 (idiomatic-access direction): the state-word accessors'
    # raw self casts left with the plain member writes through the vbase.
    # 196 -> 180 (entered-receiver conversions): the four on_mouse_leave/
    # on_dialog_focus bodies' nested cast chains left with the plain
    # field_F0_ = -1 + vslot_62() member forms.
    # 180 -> 170 (batch 10 sweep): the thirteen VCall conversions' raw self
    # casts left with the named virtual calls that replaced them.
    # 170 -> 167 (fx element identification): the element calls' last raw
    # self arithmetic left with the Wave-typed member dispatches.
    # 167 -> 166 (dipedit check_box_ member): the embedded CheckBox is a
    # real typed member now - do_check's reads go through it, not raw casts.
    # 166 -> 171 (2026-09-01, dipedit read_check batch): CheckBox::get_state_id
    # promoted out of src/unrecovered/0060ec10.cpp beside set_state_id, in the
    # SAME hand vbtable spelling that body carries - the five new sites are its
    # second half. The idiomatic member spelling was measured and diverges at
    # instruction 0 (VC6 grows the EBP frame the image does not have), so the
    # debt is the one set_state_id's marker already owns: a class-model pass
    # takes both accessors together or neither.
    # 171 -> 172 (2026-09-03, boot-callee batch): MultiDebug::init homed
    # BYTE_EXACT into multidebug.cpp, and its once-a-second tick is
    # Time::start's callback context - `start(fn, int param, ...)`, so the
    # receiver crosses as `reinterpret_cast<int>(this)`. Same site as the one
    # Win::sub_63c340's marker already owns: the pointer is crossing an
    # int-typed API, the member spelling cannot apply, and retyping that
    # context is the Time work the plan calls A3.
    # 172 -> 168 (2026-09-03, menu batch): Menu's four flag operations
    # (UNK6-9) dropped their `void **const vtable =
    # *reinterpret_cast<void ***>(this)` anchor plus the ORIGINAL() shim it
    # fed - the dispatch is Win's own slot 62, which win.h declares virtual,
    # so the plain call `vslot_62()` compiles the same two instructions.
    # Measured 33/35 before and after on all four.
    # 168 -> 165 (2026-09-03, sound batch): ~Sound's `Sound volatile *const
    # self` view and the two `reinterpret_cast<Sound volatile *>(prev/next)`
    # receiver puns it fed are gone with it - the plain member spelling
    # measures the same 0/62 (the body is held at zero by the image's /GX
    # unwind frame and family-root vtable publish, neither of which a
    # base-less Sound can produce), so the pun bought nothing.
    # 165 -> 164 (2026-09-03, net_class batch): Net::close's
    # `char *const self = reinterpret_cast<char *>(this)` view is gone - the
    # eleven offsets it reached are named members now (poll_timeout_E0_,
    # field_760_/764_, poll_time_DC_, field_6DC_, field_1C_, self_20_,
    # buffer_24_, thread_id_4_, lock_2C_), which is what the two defects the
    # marker records were hiding behind.
    # 164 -> 171 (2026-09-03, P1b ambience batch): seven claimed bodies
    # homed VERBATIM with their pointer forms load-bearing - de-punning
    # GAmbience::general was measured and REGRESSED it to MISMATCH, then
    # reverted. The de-pun pass runs with measure in the loop, not as a sweep.
    # 171 -> 181 (2026-09-03, P1d batch): ten more load-bearing pointer
    # forms homed verbatim - the measured-and-regressed rule from the P1b
    # raise applies; the de-pun pass runs with measure in the loop.
    "raw self-access": 181,
    "pointer-parameter as int": 4,
    # CORRECTED 2026-08-26, not raised to absorb a regression: this
    # census skipped every code line starting with `*`, so two of these
    # were never counted. The tree did not change; the ruler did.
    # 1427 -> 1426: UNK5's `int a1` is covered by the EVIDENCE-FREE block
    # that excuses the method (same commit as the reclassification).
    # 1426 -> 1383 (2026-08-28, graphicwin/basewin/basebutton batch): 43
    # parameters renamed from evidence - the annotation `prototype` lines
    # (on_iface_left_click's xCoord/yCoord) and call sites (click's
    # right/is_double). Two renames REVERTED after the evidence check:
    # UNK2's four ints stay scaffold because garrison_click is sub_40B140
    # to IDA and FUN_0040b140 to Ghidra - placeholders one remove is still
    # invention. All ten renamed bodies re-measured BYTE_EXACT.
    # 1379 -> 1380 (2026-08-30, boot-path batch): +1 net from the
    # control_game / caviar homings, proven byte-identical to HEAD by the
    # dispatching agent's clean-base measurement; the class pass names it.
    # 1325 -> 1313 (batch 9, family ctor conversion): the four families'
    # construct()-era scaffold parameters (the a1 flag spellings) left with
    # the bare real constructors.
    # 1313 -> 1334 (2026-09-01, NOT this work): the five seams of the
    # concurrent DipEdit::on_button_clicked homing sharing this worktree
    # (Buffer::box_sprite, ListBox::set_selected_id, CheckBox::item,
    # EditGroup::edit, X_pop_ask_number) name their parameters a1../a3..
    # pending each body's own evidence. That batch's number to bring down.
    # 1334 -> 1322 (2026-09-02, NOT this work either): the on_button_clicked
    # batch named twelve of them (button_id, and the EditGroup/X_pop_ask_number
    # params). DipEdit::init's three new seams (Buffer::box_sprite,
    # ListBox::set_selected_id, CheckBox::item) land named, adding none.
    # 1322 -> 1328 (2026-09-03, menu batch): Menu::UNK5's three forwarded ints
    # come home still scaffold. There is nothing to name them from - the body's
    # only use of a2/a3/a4 is the argument list of PullDown::UNK3, which is
    # itself a stub that reads none of its three, so a name here would be
    # invented, not inferred.
    # 1328 -> 1341 (2026-09-03, P1 calibration batches): thirteen more names
    # homed still-scaffold with their bodies (ambience's MAmbience/UAmbience
    # drafts, VehLink's pad_/field_2x_). Homing-first is the campaign's own
    # rule - renaming is the class pass's semantic half, done against a
    # MEASURED body (the home_bodies reconcile comment) - so these ride the
    # ratchet until their class passes name them from evidence.
    # 1341 -> 1506 (2026-09-03, P1d designwin/diplowin/socialwin/reportif
    # batch): 165 more names homed still-scaffold with 18 bodies - the
    # vehdraw stat blocks, the social tables, the DesignWin drafts. Homing-
    # first stands: renaming is the class pass's semantic half against a
    # measured body.
    "scaffold name": 1506,
}

WHY = {
    "evidence-free unk":
        "an UNKn whose comment block says EVIDENCE-FREE and NAMES it: "
        "investigated and found unnameable - no caller, no dispatch, no CSV "
        "entry. Do not invent a name to clear this; the refusal is the "
        "finding. Only the names inside the block are excused.",
    "unk-method":
        "a method named UNKn is a body whose behaviour nobody wrote down. "
        "Name it from evidence: callers, the image's data, "
        "docs/recovery/behaviour-member-names.csv.",
    "function-address binding":
        "a cast of an address that lands INSIDE a catalogued function - a "
        "function reference wearing a data global's clothes. Declare the "
        "callee (forward it in pending_bodies.cpp if unpromoted) and delete "
        "the global.",
    "orphan redirect":
        "a *_redirect wrapper referenced only by its own declaration and "
        "definition. Delete it; the gate's link step is the proof nothing "
        "non-textual needed it.",
    "pointer-as-int":
        "a pointer travelling as int - `return reinterpret_cast<int>(this)`, "
        "`return (int)new`, or an invented operator name (__as is operator=). "
        "Same bytes, wrong model; fix the type and re-measure.",
    "undocumented trivial body":
        "a BYTE_EXACT claim on a <=4-byte body with no Purpose:/Verification "
        "note prose. The claim is honest - the image really is that trivial - "
        "but nothing SAYS so, and it reads as a stub.",
    "pointer-parameter as int":
        "the body casts a scaffold parameter (aN) to a pointer - the body "
        "PROVING the parameter's type; the census's H was a guess. Retype "
        "from use (set_rgbquad's RGBQUAD dual went 6->11 agreeing once "
        "typed), update the `// symbol` fact to what the build emits, and "
        "re-measure. A null-check alone is a hint, not proof - only a cast "
        "counts here.",
    "scaffold name":
        "an aN identifier surviving in code is a parameter (or local) nobody "
        "has examined: the name records that no one yet asked what it IS. "
        "Name it from inferred use - the body's own reads and writes say. "
        "Renames of value parameters change no mangled name and no bytes; "
        "palette.cpp went to zero in one pass.",
    "raw self-access":
        "a body walking its own object through reinterpret_cast<char*>(this) "
        "and raw offsets when the class declares the members. Palette's "
        "copy_from proved the member form byte-identical (19/19) - and its "
        "do-while-plus-dead-store was a while loop in disguise. Use the "
        "members; a divergence is a LAYOUT finding, not a style problem. "
        "Do not trade a this-pun for a MEMBER-pun either: a dword pun of an "
        "entry feeding mask/shift extraction is the entry's channels read as "
        "members, an int copy of an entry is a struct assignment - and "
        "Palette's honest forms measured CLOSER (12->15, 6->10 agreeing).",
}

UNK = re.compile(r"\bUNK\d+\b")
ADDRESS_CAST = re.compile(
    r"(?:\(\s*[A-Za-z_][\w :]*?\*\s*\)|reinterpret_cast<[^>]*\*\s*>\s*\()"
    r"\s*0x(00[0-9A-Fa-f]{6})\b")
REDIRECT = re.compile(r"\b\w+_redirect\b")
POINTER_AS_INT = re.compile(
    r"return\s+reinterpret_cast<\s*int\s*>\s*\(\s*this\s*\)"
    r"|return\s*\(\s*int\s*\)\s*new\b"
    r"|\b__(?:as|ct|dt)\b")
DOCUMENTED = re.compile(r"Purpose:|Verification note", re.I)
# ANY reinterpret_cast of `this`, ANY target - the user's rule, stated as a
# ban: "all reinterpret_cast involving this should be banned". A cast to
# char*/int* hides members behind offsets; a cast to another CLASS hides an
# inheritance edge behind a pun. Either way the class model is being
# bypassed, and the remedy is to say what the class IS. C-style puns of
# `this` count the same.
SCAFFOLD_NAME = re.compile(r"\ba\d+\b")
PARAM_CAST = re.compile(
    r"(?:reinterpret_cast<[^>]*\*[^>]*>|\(\s*[A-Za-z_][\w :]*\*+\s*\))"
    r"\s*\(?\s*a\d+\b")
SELF_CAST = re.compile(
    r"reinterpret_cast<[^>]*>\s*\(\s*this\s*\)"
    # `(?!\s*(?:->|\.))` - A MEMBER CAST IS NOT A SELF-PUN. `->` binds
    # tighter than a C-style cast, so `(Buffer *)this->field_FC_` casts the
    # MEMBER; the cast never touches `this` at all. Without the guard the
    # census counted six of those in win.cpp alone and called them
    # raw-self-access, which is a defect the shape cannot be cleared of
    # because there is nothing wrong with the code.
    r"|\(\s*[A-Za-z_][\w :]*\*+\s*\)\s*this\b(?!\s*(?:->|\.))"
    # memcpy(dst, this, n) needs NO cast - void* swallows `this` silently,
    # which is how `memcpy(selfCopy, this, 0x400)` outlived the cast ban.
    # The member being copied has a name and a sizeof; use them.
    r"|\bmem(?:cpy|set|move)\s*\([^;)]*\bthis\b")


# AN INVESTIGATED REFUSAL IS NOT THE SAME AS UNFINISHED WORK, and a census
# that cannot tell them apart pushes people to invent a name to clear it -
# which is a false statement that outlives whoever wrote it. font.h says so
# in as many words about 0x00618F30: no caller, `QAE` so no dispatch names
# it either, and a body that is `mov eax,1; ret 0x10` with four arguments
# discarded. Nothing can say what it was called.
#
# So a comment block containing EVIDENCE-FREE excuses the UNKn identifiers
# NAMED INSIDE THAT BLOCK, and only those - a note cannot excuse a file it
# happens to sit in. The excused ones move to their own ratcheted shape
# rather than vanishing, so the escape hatch cannot widen quietly. A
# header's refusal also covers its own .cpp, because the declaration and
# the definition are the same method.
EVIDENCE_FREE = re.compile(r"EVIDENCE-FREE")


def excused_unks(root: Path) -> dict[str, set[str]]:
    """stem -> the UNKn names a documented refusal covers."""
    out: dict[str, set[str]] = {}
    for path in sorted(root.glob("*.h")) + sorted(root.glob("*.cpp")):
        text = path.read_text(errors="replace")
        lines = text.split("\n")
        named: set[str] = set()
        for i, line in enumerate(lines):
            if not EVIDENCE_FREE.search(line):
                continue
            # the block is this line and the comment lines that follow it
            j = i
            while j < len(lines) and lines[j].lstrip().startswith("//"):
                named.update(UNK.findall(lines[j]))
                j += 1
        if named:
            out.setdefault(path.stem, set()).update(named)
    return out


def product_files() -> list[Path]:
    return sorted(REPO.glob("src/*.cpp")) + sorted(REPO.glob("src/*.h"))


def code_lines(path: Path):
    """Non-comment lines - and that includes /* */ INTERIORS. The first
    version skipped only `//`-prefixed lines, so prose inside a Purpose
    block ("the catalogue's UNK3 stays on the marker") counted as an
    unnamed method. History must not be renamed to appease a census; the
    census must know history when it sees it."""
    in_block = False
    for line in path.read_text(errors="replace").splitlines():
        s = line.lstrip()
        if in_block:
            if "*/" in s:
                in_block = False
            continue
        # A LINE STARTING WITH `*` IS CODE UNLESS WE ARE INSIDE A BLOCK
        # COMMENT - and that case is already handled above, by `in_block`.
        # Treating a leading `*` as prose out here hid every
        # `*reinterpret_cast<T *>(...) = ...;` statement from this census:
        # 971 code lines across 64 files, which is what every ratcheted
        # ceiling below has been measured against. Found because rewriting
        # three such lines in net_class.cpp to use their members made them
        # VISIBLE and the count "grew" by three without the file gaining a
        # single new site.
        if s.startswith("//"):
            continue
        if s.startswith("/*"):
            if "*/" not in s:
                in_block = True
            continue
        # TRAILING comments are prose too: `uint8_t field_B; // not UNK3`
        # counted its comment as an unnamed method. Naive split - a `//`
        # inside a string literal would be cut, but this tree's code lines
        # do not carry URLs, and losing one would UNDERCOUNT, which the
        # only-falls ceiling turns into a visible "lower it" rather than a
        # silent lie.
        yield line.split("//", 1)[0]


def _selftest_code_lines() -> str:
    """`code_lines` must keep a `*`-leading STATEMENT and drop real prose.

    A CHECK, NOT A COMMENT, because this exact walk was wrong for a long
    time in two censuses at once and nothing noticed: `*reinterpret_cast<T *>
    (self + 0xNN) = v;` starts with `*`, was classified as block-comment
    continuation, and vanished from the very census that exists to count it -
    `raw self-access` was blind to its own canonical spelling.
    Returns a complaint, or "" when the walk behaves.
    """
    import tempfile, os
    sample = ("/* a block\n"
              " * continuation must be dropped\n"
              " */\n"
              "// a line comment must be dropped\n"
              "*reinterpret_cast<int *>(self + 0x10) = 1;\n"
              "    *self_ptr = 0;\n")
    fd, path = tempfile.mkstemp(suffix=".cpp")
    try:
        os.write(fd, sample.encode()); os.close(fd)
        kept = [ln.strip() for ln in code_lines(Path(path)) if ln.strip()]
    finally:
        os.unlink(path)
    if len(kept) != 2 or not all(k.startswith("*") for k in kept):
        return (f"code_lines is misclassifying code as comment: kept {kept!r}, "
                f"expected the two `*`-leading statements and nothing else")
    return ""


def census():
    """counts per shape, files per shape, per-file detail for --json."""
    records = read(REPO / "src")

    # The catalogued function spans, as a bisectable interval list. An
    # address cast that lands inside one is a FUNCTION reference, whatever
    # the cast says.
    spans = sorted((start, end) for r in records if r.address
                   for start, end in (r.image_spans or ()))
    starts = [s for s, _ in spans]

    def in_function(address: int) -> bool:
        i = bisect_right(starts, address) - 1
        return i >= 0 and spans[i][0] <= address < spans[i][1]

    counts = collections.Counter()
    files = collections.defaultdict(collections.Counter)
    EXCUSED = excused_unks(REPO / "src") if (REPO / "src").is_dir() else {}
    redirect_refs = collections.Counter()

    for path in product_files():
        for line in code_lines(path):
            for name in UNK.findall(line):
                shape = ("evidence-free unk"
                         if name in EXCUSED.get(path.stem, ()) else "unk-method")
                counts[shape] += 1
                files[shape][path.name] += 1
            for hexpart in ADDRESS_CAST.findall(line):
                if in_function(int("0x" + hexpart, 16)):
                    counts["function-address binding"] += 1
                    files["function-address binding"][path.name] += 1
            for name in REDIRECT.findall(line):
                redirect_refs[name] += 1
            if POINTER_AS_INT.search(line):
                counts["pointer-as-int"] += 1
                files["pointer-as-int"][path.name] += 1
            for _ in SELF_CAST.findall(line):
                counts["raw self-access"] += 1
                files["raw self-access"][path.name] += 1
            for _ in PARAM_CAST.findall(line):
                counts["pointer-parameter as int"] += 1
                files["pointer-parameter as int"][path.name] += 1
            for _ in SCAFFOLD_NAME.findall(line):
                # A PARAMETER OF AN EVIDENCE-FREE METHOD IS EVIDENCE-FREE TOO.
                # You cannot name the arguments of a method whose purpose is
                # provably unknowable - Sprite::UNK4(int a1, int a2) has no
                # caller, no dispatch and no CSV entry, so `a1` and `a2` can
                # only be invented. The excuse reaches exactly as far as the
                # declaration line of an UNK the block already names, and no
                # further: a scaffold name anywhere else still counts.
                if any(u in line for u in EXCUSED.get(path.stem, ())):
                    counts["evidence-free unk"] += 1
                    files["evidence-free unk"][path.name] += 1
                    continue
                counts["scaffold name"] += 1
                files["scaffold name"][path.name] += 1

    # Orphans need the whole tree's reference counts first; attribute each to
    # the file that DEFINES it (the one place `name(` appears at line start
    # is unknowable line-wise, so attribute to every file that mentions it -
    # the per-file numbers guide the class pass, the total is the ratchet).
    orphans = {name for name, n in redirect_refs.items() if n <= 2}
    if orphans:
        for path in product_files():
            for line in code_lines(path):
                for name in REDIRECT.findall(line):
                    if name in orphans:
                        counts["orphan redirect"] += 1
                        files["orphan redirect"][path.name] += 1
    # Each orphan was counted once per mention (<=2); normalise to one per
    # redirect so the ceiling is "orphan redirects", not "mentions".
    counts["orphan redirect"] = len(orphans)

    # Trivial bodies come from the records, not from lines.
    for r in records:
        if not r.byte_exact or (r.size or 99) > 4:
            continue
        rel = str(r.path)
        if "/recovered/" in rel or "/unrecovered/" in rel:
            continue
        text = r.path.read_text(errors="replace").splitlines()
        window = "\n".join(text[max(0, r.line - 30):r.line + 3])
        preamble = "\n".join(text[:40])
        if not DOCUMENTED.search(window) and not DOCUMENTED.search(preamble):
            counts["undocumented trivial body"] += 1
            files["undocumented trivial body"][r.path.name] += 1

    return counts, files


def _lower_ceilings(counts) -> int:
    """Rewrite CEILINGS down to what the census measures. LOWERS ONLY.

    A pass that clears four shapes had to run --check four times, because it
    reports one drop per run and the ratchet fails until the ceiling matches.
    That is a workflow defect, not a safety property: lowering a ceiling can
    never weaken the ratchet, only tighten it. Raising one still has to be a
    deliberate edit, and this refuses to do it.
    """
    source = Path(__file__).read_text()
    changed = []
    for shape, ceiling in CEILINGS.items():
        n = counts[shape]
        if n >= ceiling:
            continue
        pattern = re.compile(r'(^    "' + re.escape(shape) + r'": )' + str(ceiling) + r',',
                             re.M)
        source, hits = pattern.subn(r"\g<1>" + str(n) + ",", source)
        if hits:
            changed.append((shape, ceiling, n))
    if changed:
        Path(__file__).write_text(source)
        for shape, was, now in changed:
            print(f"  lowered {shape}: {was} -> {now}")
    print(f"{len(changed)} ceiling(s) lowered to the measured census")
    return 0


def main() -> int:
    check = "--check" in sys.argv
    # THE WALK CHECKS ITSELF BEFORE IT COUNTS ANYTHING. A census whose
    # comment walk is wrong reports a confident number that is simply
    # not the tree.
    complaint = _selftest_code_lines()
    if complaint:
        print(f"SEMANTIC DEBT CENSUS IS BROKEN: {complaint}")
        return 1
    as_json = "--json" in sys.argv
    counts, files = census()

    if "--lower-ceilings" in sys.argv:
        return _lower_ceilings(counts)

    if as_json:
        print(json.dumps({shape: dict(files[shape]) for shape in CEILINGS},
                         indent=2))
        return 0

    grew, shrank = [], []
    for shape, ceiling in CEILINGS.items():
        n = counts[shape]
        if n > ceiling:
            grew.append((shape, n, ceiling))
        elif n < ceiling:
            shrank.append((shape, n, ceiling))
        if not check:
            flag = "GREW" if n > ceiling else "down" if n < ceiling else "    "
            print(f"  {flag} {n:4d}/{ceiling:<4d} {shape}")
            print(f"           {WHY[shape]}")
            print(f"           {len(files[shape])} file(s)")

    total = sum(counts[s] for s in CEILINGS)
    scanned = len(product_files())
    if grew:
        for shape, n, ceiling in grew:
            print(f"SEMANTIC DEBT GREW: {shape} is {n}, above its ceiling "
                  f"of {ceiling}")
        return 1
    if shrank:
        for shape, n, ceiling in shrank:
            print(f"semantic debt down: {shape} is {n}, below its ceiling of "
                  f"{ceiling} - lower it in this same commit")
        return 1 if check else 0
    print(f"{'semantic debt: ' if check else ''}{total} site(s) across "
          f"{len(CEILINGS)} shapes, {scanned} product file(s) scanned"
          f"{', every ceiling exact' if check else ''}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
