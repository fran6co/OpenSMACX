#!/usr/bin/env -S uv run python
r"""Work this tree does by hand that the COMPILER should be doing.

Every large gain on 2026-08-22 came from deleting something, not writing
something: a hand-installed vtable, a hand-written adjustor thunk, an explicit
base-destructor call, a `construct()` method standing in for a constructor.
None of them was a near-miss to grind. Each was the tree doing a job VC6 does
better, and doing it in a place VC6 would not have put it.

WHY IT IS A CORRECTNESS PROBLEM AND NOT A STYLE ONE. The compiler emits these
constructs in a fixed ORDER relative to everything else it generates, and that
order is part of the bytes:

  * A class that stores its own vtable dword gets NO compiler vtable, and
    therefore none of the adjustor thunks that live in one. That single fact
    made 46 deleting thunks look blocked.
  * MSVC assigns the vfptr AFTER the member-initialiser list. Stores written in
    the constructor BODY land after it instead of before, and Win::Win() sat at
    99 of 107 until they moved into the list.
  * In a destructor the vptr restores come BEFORE the EH-state store.
    FlatButton's hand-written ones landed between them, and its own lesson
    blamed "VC6's own EH-state scheduling" rather than the hand-writing.
  * An explicit `Base::destroy()` duplicates the base destructor the compiler
    already calls, so the body has one call too many.

A RATCHET PER SHAPE. These cannot all be fixed at once, and a permanently red
gate is one nobody reads. No shape may GROW. When one shrinks the tool says so
and asks for its ceiling to be lowered in the same commit, because a ratchet
with unclaimed slack is where the next regression hides.

    uv run tools/compiler_work.py            # the census, with what to do
    uv run tools/compiler_work.py --check    # exit 1 if any shape grew
"""
import argparse, collections, pathlib, re, sys

REPO = pathlib.Path(__file__).resolve().parent.parent

# Measured 2026-08-22 on a clean tree. Lower a ceiling when its count falls.
SHAPES = [
    # CORRECTED with the same fix: 5 of these sat on `*`-leading lines.
    # 24 -> 29 (2026-08-30, boot-path batch): amovie_project's faithful
    # transcription hand-installs the two vtables the IMAGE itself installs -
    # a stack GraphicWin dressed as an AlphaMovie (0x669458/0x669450); the
    # STRUCTURE finding is banked under the marker.
    # 28 -> 12 (batch 2, leaf families): the Scroll/PushButton/FlatButton/
    # WorldWin re-stage stores stripped with their constants.
    # 12 -> 3 (batch 3, alphamovie): amovie_project's five install/re-install
    # pairs stripped with the two AlphaMovie constants.
    # 3 -> 2 (batch 4, dialog family): Dialog::destroy's primary overlay store
    # stripped with its constant.
    # 2 -> 0 (batch 4, pulldown): PullDown::destroy's two dead stores stripped
    # with PullDownPrimary/BufferVtable - the last vtable-shaped stores in the
    # tree. The body demoted (1 of 36): the stores anchored this in ebx.
    ("vtable", 0,
     re.compile(r"""(?x)
        (?: \w+ \s* \[ \s* 0x[0-9A-Fa-f]+ \s* / \s* 4 \s* \]
          | \* \s* reinterpret_cast \s* < [^>]*? \* \s* > \s* \([^)]*\)
          | \w+ \s* \[ \s* \d+ \s* \] )
        \s* = \s* [A-Za-z_]\w*Vtable\w* \s* ;"""),
     "a class that stores its own vtable dword gets no compiler vtable, and no "
     "adjustor thunks in it. Give it a real constructor and delete the store."),

    # 11 -> 10 (batch 2): WorldWin::construct became the real WorldWin()
    # constructor (the image's ??0WorldWin has zero callers, so the conversion
    # cannot break a call site).
    # 10 -> 9 (EditBox ctor): EditBox::construct forwarder retired - the
    # real EditBox() ctor carries the construction the forwarder delegated
    # to the image's 0x00614E50.
    # 9 -> 8 (Dialog ctor): 0x00608C10 is homed and BYTE_EXACT in dialog.cpp,
    # so the Dialog::construct seam and its pending forwarder are gone - the
    # real constructor IS the body the seam used to delegate to.
    # 8 -> 7 (2026-09-03, mapwin batch): TextureStore::construct retired for a
    # real out-of-line TextureStore() constructor - independent of the Dialog
    # retirement above, which landed on the other side of this merge.
    ("construct", 7,
     re.compile(r"^\w[\w :*&]*\b\w+::construct\(\)\s*\{", re.M),
     "a `construct()` method standing in for a constructor. Convert it; put "
     "anything the image writes BEFORE the vfptr store in the member-"
     "initialiser list, and drop the trailing `return this;`."),

    ("free-function destructor", 5,
     re.compile(r"^\w[\w :*&]*\b\w*destructor_redirect\w*\s*\(", re.M),
     "a destructor modelled as a free function. Make it `X::~X()`. A free "
     "function is only needed where C++ cannot take a destructor's address."),

    # 12 -> 8 (batch 9, family ctor conversion): CheckBox/RadioButton/
    # EditGroup/SpriteBox ctors became real bare constructors - their
    # Dialog::construct()/GraphicWin placement seams left with the compiler's
    # own virtual-base machinery (??0CheckBox BYTE_EXACT 53/53,
    # ??0RadioButton BYTE_EXACT, ??0EditGroup SHAPE_EXACT).
    ("explicit base construct/destroy", 8,
     re.compile(r"\b\w+::(?:destroy|construct)\(\)\s*;"),
     "duplicates the base constructor or destructor the compiler already "
     "calls, so the body carries one call too many."),

    # 21 -> 19 (console batch): Console's real constructor lets the compiler
    # build graphic_win_ and the virtual GraphicWin base from the declared
    # members - the two GraphicWin placements in the old construct() body
    # left with it.
    # 19 -> 16 (batch 2, leaf families): the WorldWin ctor placements joined
    # the compiler-owned set with the real WorldWin(int) constructor.
    # 16 -> 14 (batch 5, mapwin attempt #2): MapWin's and PlanWin's GraphicWin
    # placements left with their real constructors - ??0MapWin BYTE_EXACT
    # 92/92, ??0Console BYTE_EXACT 106/106 on the bare spellings.
    # 14 -> 10 (batch 9, family ctor conversion): CheckBox's GraphicWin
    # placement and SpriteBox's GraphicWin/Dialog placements left with the
    # compiler's virtual-base machinery on the bare ctor spellings.
    ("placement new on a subobject", 10,
     re.compile(r"new\s*\(\s*(?:&|static_cast|reinterpret_cast)"),
     "costs a null guard (`test ecx, ecx; je`) the image does not have. A real "
     "base or member is constructed unconditionally."),

    # 17 -> 12 (batch 8, dialogs/listbox staging strips): the staging reads
    # left with the derived-stage stores they fed. The remaining 12 are the
    # image's own dispatch (close/handler receivers it walks itself).
    # 12 -> 8 (idiomatic-dispatch direction, 2026-09-01): MapWin close/
    # on_sys_close, CheckBox/RadioButton/ListBox closes and SpriteBox's
    # buffer-default calls are qualified base calls now - EVERY one of them
    # measures BYTE_EXACT through the compiler's own dispatch, including the
    # three that held claims. The hand walk was masking convergence.
    # 8 -> 6 (idiomatic-access direction): CheckBox's four state-word
    # accessors and ListBox's field_B4_ re-arm are plain member writes
    # through the virtual Dialog base (selected_position_/field_B4_ made
    # public). set_state_flag holds BYTE_EXACT through it.
    # 6 -> 2 (entered-receiver conversions): the four on_mouse_leave/
    # on_dialog_focus bodies were field_F0_ = -1 + vslot_62() all along -
    # every one measures BYTE_EXACT as plain member forms. The remaining
    # sites are the Q-spelled entered-receiver handlers (ListBox's three,
    # demoted with the finding) and mapwin's helper.
    ("hand-walked vbtable", 2,
     re.compile(r"\*reinterpret_cast<(?:const )?int32_t \*(?:const )?\*>\([^)]*\)\)\[[12]\]"),
     "reads a virtual-base displacement out of the vbtable by hand. That is "
     "what `public virtual` makes the compiler do."),

    # 21 -> 15 (idiomatic-dispatch direction, 2026-09-01): CheckBox's
    # set_state_flag repaint and the three family on_mouse_leave bodies are
    # plain `vslot_62()` calls - every one BYTE_EXACT - and the ListBox
    # click-handler shim is gone with its three vslot_20/28/49 conversions.
    # 15 -> 5 (batch 10 sweep): stringbox/multidebug vslot_62, diplowin/
    # pulldown release_modal, worldwin vslot_12 x3, checkbox/radiobutton/
    # spritebox on_dialog_focus vslot_62, and DipEdit's do_check
    # (cb->vslot_62() with the re-derived receiver) - all BYTE_EXACT.
    # Remaining: fx.cpp's three array-element dispatches (element type
    # unidentified) and win.cpp's seven shim classes.
    # 5 -> 2 (fx.cpp element identification): the 0x6C/97-element array is
    # Wave objects - FX::stop/release/fade are the elements' own stop()/
    # release()/fade() virtuals (the FX methods were NAMED after them), all
    # three BYTE_EXACT. Fixing fade forced the Sound-table discovery: VC6
    # assigns same-name overloads to vtable slots in REVERSE declaration
    # order - int fade() landed at slot 9 where the image has fade(unsigned
    # long); declaring fade() first aligns the 9/10 pair with the image.
    # 2 -> 0 (batch 10 complete): tutwin's on_move is
    # field_53D4_->vslot_63() with the member typed Win* (pointer = pointer,
    # no layout change), and Dialogs::on_scroll_delete's scroll is a typed
    # Scroll calling vslot_09 - the declared default whose RETURN-THE-
    # ARGUMENT is exactly the body's documented pass-back. ZERO sites.
    # 0 -> 29 (2026-09-03, P1a maininterface batch): the on_iface_* family
    # homed verbatim with their BYTE_EXACT shims intact. The real remedy -
    # declaring the dispatch virtual - belongs to the SubInterface class
    # pass (the 12-slot registry at 0x007AE7F0 whose dynamic types are not
    # modelled; subinterface.h already records the reading, and P1d's
    # +0xA14 finding is the same interface). Homing-first; the class pass
    # retires these.
    # 29 -> 32 (2026-09-03, P2 console batch): three more homed-verbatim
    # shims; the biggest receiver is the MainInterface object at 0x007B0CB8
    # (scenario.cpp binds it) - the MainInterface virtual declarations are
    # that class pass's, not console's. SubInterface pass retires the family.
    ("VCall shim", 32,
     re.compile(r"\bVCall\b"),
     "dispatches through a fake class because the real function is not "
     "declared virtual. Declare it."),

    # ONE SHAPE UNTIL 2026-08-23, and one shape was the mistake: the remedy
    # decides the split, not the syntax. Both spell `ORIGINAL(`, but a
    # vtable-slot dispatch is cured by DECLARING THE METHOD VIRTUAL (it is the
    # VCall shim's defect wearing different clothes), while a named-pointer
    # seam is cured by declaring the method and forwarding it from
    # pending_bodies.cpp. A single ratchet let one grow while the other
    # shrank, invisibly. The two regexes classify by what follows on the
    # LINE - the census is line-wise, so a lookahead cannot leak across
    # statements.
    # 34 -> 30 (2026-09-03, menu batch): Menu's four flag operations
    # (UNK6-9, 0x005FB2A0/0x005FB360/0x005FB420/0x005FB4E0) call Win slot 62 by
    # name - `vslot_62()`, which win.h already declares virtual - and their
    # ORIGINAL()/vtable[] shims with their `reinterpret_cast` of `this` are
    # gone. Both measured 33/35 before and after.
    ("ORIGINAL() vtable-slot dispatch", 30,
     re.compile(r"\bORIGINAL\((?=.*vtable\[)"),
     "reads the target out of a vtable slot by hand because the method is not "
     "declared virtual. Declare it - the compiler writes this dispatch."),

    # 87 -> 91 (2026-09-04, P2 veh batch): four more, documented at the
    # seams in veh.cpp - the check_base/check_loc/base_compute cascade is
    # reached through original_method the way mapwin.cpp reaches
    # MapWin::draw_radius; the class pass that homes the base.cpp cascade
    # retires them.
    # 91 -> 94 (2026-09-04, P2 net batch): three dispatches through the
    # NetWin singleton at 0x0080A6F8 (setup_faction_data, is_visible,
    # pick_pass) - netwin.h's owner clears them.
    # 94 -> 99 (2026-09-04, P3 veh action batch): five more, each a
    # genuinely-unrecovered callee reached by a homed body (BaseWin::
    # check_veh 0x41A8B0, TutWin::tut_map 0x4BE650, NetDaemon::order_veh
    # 0x531780 2485B archived, atrocity 0x55EEE0 1391B); GraphicWin::redraw
    # came back for free - a plain non-virtual member. Homing each callee
    # retires its seam.
    ("ORIGINAL() named-pointer seam", 99,
     re.compile(r"\bORIGINAL\((?!.*vtable\[)"),
     "reaches a member through a function-pointer slot, which compiles "
     "`FF 15` where the image has `E8`. Call it by name once its body lands."),
]

# SCANNED OVER .cpp AND .h TOGETHER, which is why they are a second list: the
# members live in HEADERS, and turning the whole census loose on headers
# inflates three existing shapes with DECLARATIONS (free-function destructor
# 10 -> 20, VCall 21 -> 23, named-pointer seam 121 -> 123 - measured
# 2026-08-23). Same ratchet, narrower lens.
HEADER_SHAPES = [
    # 4 -> 3 (batch 6, dialog family): DialogEntry's vtable/secondary_vtable
    # pair left with the struct - the embedded list is a real StringList
    # member (item_list_) and the node type is StringStructEntry.
    ("vtable-as-member", 3,
     re.compile(r"\b[A-Za-z_]\w*\s+vtable\w*\s*;"),
     "a vtable pointer spelled as a data member is a base class that has "
     "not been declared. Declare the base whose vfptr lives at that offset "
     "- or the class's own virtuals - and let the compiler own slot 0."),
    ("vtable-initialiser store", 0,
     re.compile(r"\w+\s*\(\s*[A-Za-z_]\w*Vtable\w*\s*\)"),
     "a base installed by an initialiser-list store is a base that has not "
     "been declared (Win's second base at 0xC8). Declare it."),
    # 75 -> 73 (2026-08-29, stringbox batch): StringBoxPrimaryVtable and
    # StringBoxBufferVtable are gone - the class is real, and the compiler
    # installs ??_7StringBox itself.
    # 73 -> 71 (checkbutton batch): CheckButtonPrimaryVtable and
    # CheckButtonBufferVtable were declared but never referenced - the class
    # is real (: public GraphicWin, virtual destructor), and the compiler
    # installs its vptrs in the ctor itself.
    # 71 -> 42 (batch 1, dead-constant sweep): the 29 zero-use vtable-address
    # constants died - Menu/MultiDebug/NetMsg/TutWin/PickWin/ReportWin/
    # ProdPicker pairs, BaseWin's four, SocialWin's four, Gamma's pair,
    # GraphicWin's pair, BufferVtable, the ImageButton pair - plus net_class's
    # six and alphanet's five unused construction-table arrays.
    # 24 -> 22 (batch 3, alphamovie): AlphaMoviePrimary/BufferVtable stripped
    # with amovie_project's five install/re-install pairs.
    # 21 -> 19 (batch 4, pulldown + dead externs): PullDownPrimary/BufferVtable
    # deleted with their dead stores; the eleven orphan externs (DialogPrimary,
    # the four Ambience, the six Dialogs stage tables) left with zero users.
    # 19 -> 18 (batch 6, dialog family): DialogVirtualBaseFinalVtable left -
    # the final-table install is ~StringAllocationBase's own body now, run as
    # item_list_'s explicit destructor in Dialog::destroy.
    # 18 -> 14 (batch 6, stringstruct dtor chain): the four list-stage
    # constants left - destroy()'s hand close_with_tables calls folded into
    # the one compiler-owned ~StringList chain, which stages the same tables
    # (derived pair, base pair) from the real declarations.
    # 14 -> 9 (batch 6, chain deletion): StringStruct's four table constants
    # left with close_with_tables and the orphaned derived-close redirect -
    # the image's 0x004066C0 was the compiler-generated teardown the real
    # ~StringStruct now emits, and its only in-tree caller was the demoted
    # StringList::destroy.
    # 9 -> 0 (batch 8, dialogs/listbox staging strips): the Dialogs six
    # function-local stage tables and ListBox's three teardown re-stage
    # constants left with their stores. ZERO - the named vtable-address
    # constant is extinct in src/.
    ("vtable address constant", 0,
     re.compile(r"\b[A-Za-z_]\w*Vtable\w*\s*=\s*\(?\s*0x"),
     "the raw material every hand-installed vtable is built from. When the "
     "classes are real, these constants have nothing left to point at."),
    # EVERY MENTION COUNTS, not just the declaration, because the name is
    # the defect: `g_0069710c` tells the reader nothing at every one of its
    # use sites. Renaming the declaration clears them all at once, which is
    # what lets this ratchet fall in class-pass-sized steps. Homing imports
    # bindings from the artifacts (6ee2b94a) - this ceiling is what forces
    # it to import them NAMED.
    # A ONE- OR TWO-LETTER BINDING IS ANONYMOUS TOO, and worse: the shape
    # below only counts `g_<address>` names, so `static int *const g =
    # (int *)0x00669310` slipped past it entirely. win.cpp reached through
    # that single `g` for EIGHT different Win32 imports - GetCursorPos,
    # BeginPaint, EndPaint, GetWindowLongA, ShowWindow, SetRect, ReleaseDC -
    # while the address is SetCursorPos, and every one of those bodies was
    # BYTE_EXACT throughout, because a call target is a relocation and
    # asm.py masks those. The name is what hid it: too short to inform a
    # reader, too unlike `g_00669310` to be counted. Ceiling 0; there is no
    # legitimate one-letter binding to a fixed address.
    ("short-named fixed-address binding", 0,
     re.compile(r"\bconst\s+[a-z][a-z0-9]?\s*=\s*\(\s*[\w:\s\*]+\)\s*0x00"),
     "a fixed-address binding whose name is one or two letters. Name it "
     "from the import table (tools/iat_names.py) or from its use; a short "
     "name is invisible to the anonymous-global census AND to the reader."),
    # CORRECTED, not raised: the comment walk skipped every code line
    # starting with `*`, hiding 69 of these. The tree did not change;
    # the ruler did. See _code_lines.
    # 155 -> 146 (2026-08-30, boot-path batch): the BasePop ctor's defaults
    # became real named globals and the windtor/amovie homings retired more
    # raw bindings.
    # 63 -> 57 (2026-08-30, Console/MapWin destructor pass): the member
    # carves replaced g_-named raw bindings with named class members, and
    # the vtable-restore puns in the hand-spelled destructors left with the
    # bodies.
    # 57 -> 51 (sound batch): load_sound_dll's homing named its bindings from
    # the image - SoundDllModule, the three kernel32 slots, the version proc
    # and the four complaint strings - and init_sound's shared bindings took
    # the same names.
    # 51 -> 49 (sound batch): the export table recovered as sound.h's
    # SoundDllProcs(), and init_sound's raw table binding left with it.
    # 49 -> 45 (sound batch): init_sound went BYTE_EXACT through the named
    # instances - MidiDeviceGlobal in sounddevice.h, SoundDllInitSlot's table
    # view - and its last two raw bindings left with them.
    # 45 -> 0 (2026-08-31, anonymous-globals sweep): every remaining
    # g_<address> binding renamed from evidence - the vftable installs by the
    # deleting-destructor slot each table carries (ConsoleVftable,
    # MapWinVftable, AlphaMenuVftable, PlanWinVftable, MonuWinVftable,
    # PrefWinVftable and their virtual-base pairs), the blit-state family by
    # its neighbours (BlitSourceField70/74/78 beside BlitSourceField4A8),
    # CheckButton's two ctor defaults, and FlatButtonDefaultsTable. The two
    # prose mentions of retired bindings now carry plain hex.
    ("anonymous fixed-address global", 0,
     re.compile(r"\bg_00[0-9a-f]{4,6}\b"),
     "a global named by its address instead of its meaning. Name it from "
     "evidence - the image's .data value, the arithmetic identity, the "
     "behaviour CSVs - in the class pass that touches it."),
]

# THE SCAFFOLD RATCHET. Twelve files exist because a body had no real home or
# the tree did by hand what a faithful class makes the compiler do; the
# recovery includes their retirement, so their sizes are ceilings like any
# shape's. A scaffold at zero is deleted in its own commit. Measured
# 2026-08-23 on a clean tree at 2bbecdbd.
SCAFFOLD_MARKERS = ("adjustor_thunks.cpp", "deleting_thunks.cpp",
                    "delegation_thunks.cpp", "field_accessors.cpp",
                    "leaf_recoveries.cpp", "nullsub_thunks.cpp")
SCAFFOLD_CEILINGS = {
    "adjustor_thunks.cpp markers": 116,
    "deleting_thunks.cpp markers": 73,
    "delegation_thunks.cpp markers": 26,
    # 42 -> 41 (sound batch): field_accessor_0062d390_redirect retired for the
    # real `sub_62d390` in general.cpp.
    "field_accessors.cpp markers": 41,
    "leaf_recoveries.cpp markers": 52,
    "nullsub_thunks.cpp markers": 56,
    # 234 -> 231 (2026-08-30, caviar-voxel batch): the three seams Caviar's
    # homing had declared for sub_639390 / sub_63ad60 / sub_63f9b0 retired -
    # all three bodies now live at the foot of src/caviar.cpp.
    # 209 -> 214 (2026-09-02, DipEdit UNK1-4 batch): the frontier grows by the
    # five callees those four bodies need and nothing in the build defines -
    # CheckBox::get_state_id (0x0060EC10) and treaty_on/treaty_off/agenda_on/
    # agenda_off (0x0055B870/0x0055B760/0x0055BA80/0x0055B820).
        # 209 -> 214 (2026-09-01, dipedit read_check batch): read_check's own
    # forwarder went with the BYTE_EXACT body in src/dipedit.cpp, and the body
    # needed six seams in its place - DipEdit::apply_treaty_check /
    # apply_treaty_check_alt / apply_agenda_check (the three per-flag helpers
    # it drives, homed separately), CheckBox::get_state_id (set_state_id's
    # read-side twin) and the free treaty_on/treaty_off pair. None of those six
    # had a homed caller before.
    # 214 -> 212 (same batch): two of those six did not stay seams.
    # get_state_id and treaty_off PROMOTED out of src/unrecovered/ into
    # checkbox.cpp and faction.cpp, their archive units deleted with them.
    # 212 -> 217 (same batch, NOT this work): the five seams of the
    # concurrent DipEdit::on_button_clicked homing sharing this worktree -
    # Buffer::box_sprite, ListBox::set_selected_id, CheckBox::item,
    # EditGroup::edit, X_pop_ask_number. That batch's own number to bring
    # down when its bodies land.
    # 217 -> 220 (this merge, the DipEdit batch collected): master's UNK
    # batch brought its five callees onto the branch's frontier; the branch's
    # own promotions (CheckBox::get_state_id, treaty_off, X_pop_ask_number)
    # retired three forwarders from the union, and its homings added the
    # apply_treaty_check/apply_treaty_check_alt/apply_agenda_check trio plus
    # Buffer::box_sprite, ListBox::set_selected_id, CheckBox::item and
    # EditGroup::edit; the merged tree's own census then retired the duplicated
    # treaty_on forwarder and the three apply_* seams whose addresses the UNK
    # batch had already homed as claimed UNK2-4 bodies.
    # 216 -> 217 (2026-09-02, Buffer::box_sprite batch): box_sprite's own
    # forwarder died with the homed body (-1) and the two callees it reaches
    # came onto the frontier as forwarders (+2) - Buffer::tile_mask
    # (0x005DA6C0) and the Sprite overload of Buffer::tile (0x005DA9F0),
    # neither previously declared anywhere in src/.
    # 216 -> 225 (2026-09-02, boot-sequence batch): system_init and game_init
    # homed into src/game.cpp. game_init's own forwarder went with the body,
    # and the boot sequence brought eleven callees onto the frontier that the
    # build defines nothing for - load_sprites, load_fixed_sprites,
    # textcolor_init, config_popups, bung_momma, and the six member `init`s
    # (MainInterface, BattleWin, Console, MultiDebug, MessageWin, Datalink).
    # Six of the eleven are member functions whose declarations belong in
    # maininterface.h / battlewin.h / console.h / multidebug.h / messagewin.h /
    # datalink.h, which that batch does not own, so game.h spells them as
    # __fastcall receiver-first free functions. Each is one line to convert
    # when its own header lands.
    # 225 -> 226 (this merge): the boot batch's 11 callees and the box_sprite
    # batch's two land in one tree; the union census above is the count.
    # 216 -> 217 (2026-09-03, Net::check_for_lobby homed into net_class.cpp):
    # the check_for_lobby forwarder retired, but the body's first callee
    # (Net::init, 0x0062D8E0) and its last (Net::join_session, 0x0062E540) are
    # both unrecovered, so homing the one body put TWO new forwarders on the
    # frontier - a net +1. Both were already archived (0062d8e0.cpp,
    # 0062e540.cpp) with no compiled caller; naming them is what lets the homed
    # body emit the image's own `E8`.
    # (this merge): the union census is the count.
    # 227 -> 230 (2026-09-03, boot-callee batch): three of the boot batch's
    # callees homed - load_fixed_sprites (0x00455E80) into game.cpp,
    # textcolor_init (0x004BA220) into palette.cpp and MultiDebug::init
    # (0x005C9860) into multidebug.cpp - so their forwarders died with the
    # bodies (-3, and the game.h `multidebug_init` free-function shape with
    # them). But load_fixed_sprites' seven callees were declared nowhere in the
    # tree, so naming them for the body put forwarders on the frontier for six
    # of them (load_palette, load_misc, load_interface, load_voxels,
    # colortables_init, load_cycling) - the seventh, load_deswin_sprites, was
    # already compiled and claimed in leaf_recoveries.cpp under the seam name
    # leaf_00455e50_redirect, so it took its own image name instead of a
    # forwarder that would have landed on it as a LANDMINE. Net +3.
    # 230 -> 227 (2026-09-03, the archive-only-forwarder batch): the treaty and
    # agenda writers plus Net::init homed - treaty_on (0x0055B870, MISMATCH
    # 94/152), agenda_on (0x0055BA80, BYTE_EXACT 56/56), agenda_off
    # (0x0055B820, BYTE_EXACT 30/30) into src/faction.cpp and Net::init
    # (0x0062D8E0, BYTE_EXACT 87/87) into src/net_class.cpp - so their four
    # forwarders died with the bodies. Net::init hands one back:
    # ?ambience@@YAHH@Z (0x00446B40) is the event-sound player treaty_on asks
    # for on a vendetta, and nothing in the build defined it. Net -3.
    # 227 -> 229 (2026-09-03, the PullDown-declaration batch): the four menu
    # bodies homed into src/menu.cpp (all four BYTE_EXACT) needed three
    # PullDown members pulldown.h had never declared, so declaring them put
    # the two big ones on the frontier - add_item (0x005F89D0, 527 bytes) and
    # rename_item (0x005F8E50, 494 bytes), both unrecovered, both already
    # archived with drafted bodies. Their sibling add_separator
    # (0x005F8BE0, 161 bytes) was small enough to home straight into
    # src/pulldown.cpp BYTE_EXACT, so it cost no forwarder. Net +2.
    # 227 -> 224 (2026-09-03, the Buffer tile batch): the three tile
    # forwarders died with the bodies homed into src/buffer.cpp - tile_mask
    # (0x005DA6C0), tile/Buffer (0x005DA860) and tile/Sprite (0x005DA9F0).
    # tile_mask's per-tile blit copy_mask (0x005E2B00) was homed too, from
    # 005e2b00.cpp, and never needed a forwarder: both of its own callees,
    # get_data(2) and free_data, were already homed and BYTE_EXACT.
    # 224 -> 222 (2026-09-03, net_class batch): process_voice and
    # check_polling homed into src/net_class.cpp (BYTE_EXACT and MISMATCH
    # 11/135); Net::sub_62f390 and Net::drop_player, the two callees the two
    # bodies needed, took in-file reinterpret_cast forwarders beside their
    # callers instead of PENDING_BODY ones.
    # 222 -> 221 (2026-09-03, the BasePop destructor): ??1BasePop@@QAE@XZ
    # left pending_bodies.cpp for an in-class definition in basepop.h, whose
    # out-of-line COMDAT basepop.cpp emits and claims against the image.
    # 221 -> 221 (2026-09-03, the dialog batch): Dialog::construct's
    # forwarder died with 0x00608C10 homed and BYTE_EXACT in dialog.cpp - it
    # had no caller left, and a live one would have been a second definition
    # of the same construction.
    # 221 -> 220 (2026-09-03, merge of agent/dialog): the BasePop and dialog
    # forwarder deaths were independent -1s off the same 222 base, one on
    # each side; the merged pending_bodies.cpp carries both.
    # 220 -> 226 (2026-09-03, P1c sprite/pulldown/console batch): six added,
    # all named by the agent. Three are LOAD-BEARING now - Sprite::draw_dest
    # ->0x005E5833, draw_dest_unk2->0x005E81BC, draw_multi_table_dest->
    # 0x005E8FF5, the five-argument workers the homed wrappers call by name
    # so the E8 survives. Three are frontier-ahead-of-declaration, slack
    # until the blocked callees' owners declare them: stack_kill->0x005B9510,
    # veh_ready->0x005C1B70, world_rocky->0x005C32A0.
    # UNCHANGED AT 226 (2026-09-03, stack_kill landmine fix): stack_kill
    # homed to veh.cpp BYTE_EXACT (it was archive-CLAIMED while a forwarder
    # named it - the address-index landmine) and its forwarder died; its
    # callee veh_kill->0x005C08C0 is FILE-only (unclaimed), so the forwarder
    # that took the slot is landmine-free. Net zero.
    # 226 -> 230 (2026-09-03, P1d designwin batch): the four
    # ?vehdraw_construct_*@@YAH... forwarders, named callees of homed
    # DesignWin bodies - both forwarder and prototype move to a vehdraw TU
    # when one is split out.
    # 229 -> 241 (2026-09-03, P2 console batch): twelve forwarders, every
    # target an UNCLAIMED archive body (landmine-free, verified): veh_ready,
    # draw_cursor, save_game, base_kill, end_of_game, load_game, load_map,
    # set_dirty, size_of_planet, supply_options, time_controls_dialog,
    # veh_pick, world_build minus the two already forwardered. diplo_unlock
    # was added and REMOVED - archive-claimed, the landmine rule caught it.
    # 241 -> 253 (2026-09-03, P2 datalink/faction batch): twelve more, all
    # FILE-only callees of homed bodies (landmine-free, agent-verified) -
    # the diplomacy/net family's check_spock, do_withdrawal, pact partners.
    # Union-collected beside console's fourteen (both blocks appended at EOF).
    # 252 -> 254 (2026-09-04, caviar P2 wave 2): two documented stand-ins for
    # callees a homed body reaches but whose own bodies are still archived -
    # vox_load_record (0x006381F0) for CaviarData::init and Caviar::render
    # (0x00617230) for Caviar::UNK2, which homed BYTE_EXACT around them.
    # 250 -> 251 (2026-09-04, P2 veh batch, net +1 after the union): three
    # added (battle_kill, battle_fight's 7-arg overload, DeletionList::
    # add_deletion - all FILE-tier, landmine-free) against three retired in
    # the merge (veh_kill/veh_ready into veh.cpp, world_rocky into map.cpp)
    # and one stand-in that pre-dates this collection.
    # 251 -> 256 (2026-09-04, P2 net batch): five FILE-only stand-ins,
    # landmine-verified (action_airdrop, order_veh, NetFifo::sub_634200,
    # NetFifo::sub_6340c0, AlphaNet::setup/Net::send/convert family minus
    # those retired beside homed claims).
    # 256 -> 257 (2026-09-04, console homing): MapWin::draw_cursor
    # (0x0046AD60) declared so Console::move_cursor/set_cursor stop calling
    # the free ::draw_cursor the image does not call at those sites.
    # 257 -> 263 (2026-09-04, console homing, +6): console.cpp's handoff block
    # has declared tut_check/tut_check2/world_fungus since an earlier pass with
    # NO definition anywhere - a landmine that only needed a caller. Six
    # forwarders arrive with the first bodies that call them: those three plus
    # MainInterface::draw_edit_pane (0x0045B550), TutWin::tut_map (0x004BE650)
    # and Console::set_view (0x00511480, goes when its claimed body homes).
    # 263 -> 265 (2026-09-04, console claimed wave): BaseWin::zoom
    # (0x0041A920) and MapWin::set_center (0x0046B1F0) declared so
    # Console::base_me and Console::center could home with their claims.
    # 265 -> 262 (2026-09-04, console claimed wave): tut_check (0x004BA5F0),
    # tut_check2 (0x004BA650) and Console::set_view (0x00511480) homed with
    # their claims, so the three forwarders that had been the only compiled
    # definition of each went with them.
    # (union-collected: the terrain batch's MapWin::clear retirement and the
    # veh action stand-ins fold into this trail; live census sets the number)
    "PENDING_BODY forwarders": 259,
    # 1326 -> 1325 (2026-08-30, caviar-voxel batch): recovered/00639390.cpp
    # homed BYTE_EXACT into src/caviar.cpp.
    # 1325 -> 1294 (2026-08-29, caviar-voxel batch): the six vx_* IO shims
    # homed into src/caviar.cpp as callable Caviar statics (three BYTE_EXACT:
    # vx_read/vx_write/vx_seek); their four archive claims died with the
    # files and three live claims were re-banked by osmx record.
    # 1294 -> 1293 (batch 9, Dialog ctor): 00608c10.cpp homed - the Dialog
    # constructor recovered as a real out-of-line Dialog() in dialog.cpp
    # (MNEMONIC_ONLY 99/103, the residual is one load/store scheduling nit).
    # 1293 -> 1292 (batch 9, Dialogs ctor): 00612830.cpp homed - the Dialogs
    # real bare constructor landed (MISMATCH 68/201, the span covers the ctor
    # plus the jump-dispatch function our binary lays out elsewhere).
    # 1292 is the count after 00612830.cpp homed; 004ba830.cpp had no archive
    # (the body was already in win.cpp, only MOVED to tutwin.cpp this pass).
    # 1291 -> 1290 (2026-09-01, dipedit read_check batch): units/004da990.cpp
    # deleted with the body it drafted, homed BYTE_EXACT in src/dipedit.cpp.
    # 1290 -> 1289 (2026-09-02, DipEdit::init batch): units/0055b760.cpp and
    # 0060ec10.cpp deleted with the bodies they drafted - treaty_on and
    # CheckBox::get_state_id, both homed as pending forwarders / src bodies.
    # 1289 -> 1288 (2026-09-03, editor_diplomacy): units/004db870.cpp deleted
    # with the body it drafted - Console::editor_diplomacy homed in
    # src/console.cpp (MISMATCH 58/157; the residual is the two inline
    # teardown expansions the image pays for and this tree's out-of-line
    # ~Popup/~DipEdit definitions cannot spell).
    # 1289 -> 1287 (2026-09-03, boot-callee batch): 00455e80.cpp homed BYTE_EXACT
    # into src/game.cpp and units/004ba220.cpp homed BYTE_EXACT into
    # src/palette.cpp, both files gone with the bodies.
    # 1286 -> 1283 (2026-09-03, the archive-only-forwarder batch): the treaty
    # and agenda writers' archives gone with their bodies - recovered/
    # 0055b820.cpp (agenda_off BYTE_EXACT), units/0055b820.cpp (the same body's
    # preserved-unit twin) and units/0055b870.cpp (treaty_on, whose
    # preservation was the only reason it was on the index at all - it held no
    # claim, and its body is homed MISMATCH 94/152 into src/faction.cpp).
    # 1287 -> 1285 (2026-09-03, menu batch): units/005fb8e0.cpp deleted with the
    # body it drafted - Menu::UNK11 homed in src/menu.cpp (MISMATCH 30/32, the
    # residual is the shared dead `-1` check the seven-clone family records).
    # units/005fb930.cpp STAYED: Menu::UNK12 reads PullDown::items_, which
    # pulldown.h declared private, and that pass did not own the header.
    # 1285 -> 1282 (2026-09-03, the PullDown-declaration batch): the blocker
    # is gone - pulldown.h declares the three missing members and grants Menu
    # friendship over items_ - so units/005fb160.cpp (Menu::add_separator
    # BYTE_EXACT), units/005fb930.cpp (Menu::UNK12 BYTE_EXACT) and
    # units/005f8be0.cpp (PullDown::add_separator, homed into pulldown.cpp
    # BYTE_EXACT 52/52) are deleted with the bodies they drafted.
    # 1279 -> 1278 (2026-09-03, wave batch): units/00445bb0.cpp deleted with
    # the body it drafted - Effect::init homed into src/wave.cpp
    # (MISMATCH 42/49; the residual is the register-swap plateau the marker
    # records).
    # 1278 -> 1277 (2026-09-03, net_class batch): units/006320e0.cpp deleted
    # with the body it drafted - check_polling homed into src/net_class.cpp
    # (MISMATCH 11/135, the register-assignment plateau the marker records).
    # 1278 -> 1277 (2026-09-03, the BasePop destructor): units/004064d0.cpp
    # deleted with the verbatim body it preserved - the real chain is the
    # compiler's, from basepop.h's member list, and basepop.cpp claims it.
    # 1133 -> 1129 (2026-09-04, caviar P2 wave 2): 00638430.cpp,
    # 006385d0.cpp, 00638620.cpp, 00638650.cpp, 006386d0.cpp homed into
    # src/caviar.cpp with their g_ bindings renamed from the image.
    # 1129 -> 1124 (2026-09-04, same wave): 00617670.cpp, 006386b0.cpp,
    # 00618520.cpp, 00618560.cpp, 00618430.cpp homed into src/caviar.cpp.
    # 1124 -> 1123 (same wave, set_buffer): 006183d0.cpp homed into
    # src/caviar.cpp.
    # 1063 -> 1059 (2026-09-04, P2/P3 wave 3): four homed into src/game.cpp
    # (say_special's markup literals named out of the image, the rest against
    # Players/StringTable/Labels/TextBufferGetPtr) and the terrain batch's
    # mapwin family; then 1059 -> 1058 by the veh action batch.
    # 1063 -> 1062 (2026-09-04, console P2/P3 wave 3): 004e14a0.cpp homed
    # into src/console.cpp BYTE_EXACT once world_rocky was declared in
    # map.h and the ROCKOSITY label bound at 0x00689174.
    # 1062 -> 1061 (2026-09-04, same wave, prev_cursor): 00510a60.cpp homed
    # with its claim - 63/63 on the first measurement, on the declared member
    # form and AlphaMenu::check.
    # 1061 -> 1060 (2026-09-04, same wave, base_names): 00515600.cpp homed
    # BYTE_EXACT 18/18 once the two preference dwords were named
    # GameMorePreferences and AlphaIniPrefs.more_preferences.
    # 1060 -> 1059 (2026-09-04, same wave, toggle_flatten): 00515640.cpp homed
    # BYTE_EXACT 20/20.
    # 1059 -> 1058 (2026-09-04, same wave, toggle_grid): 005156a0.cpp homed
    # BYTE_EXACT 20/20.
    # 1058 -> 1057 (2026-09-04, same wave, toggle_base_grid): 005156f0.cpp
    # homed BYTE_EXACT 20/20.
    # 1057 -> 1056 (2026-09-04, same wave, center): 00515400.cpp homed
    # BYTE_EXACT 9/9 once MapWin::set_center was declared.
    # 1056 -> 1055 (2026-09-04, same wave, base_me): 005169b0.cpp homed
    # BYTE_EXACT 16/16 once BaseWin::zoom was declared.
    # 1055 -> 1054 (2026-09-04, same wave, survey): 00515450.cpp homed
    # BYTE_EXACT 47/47 - the overlay-latch state machine on the declared
    # members, no field carve needed. base_me (005169b0.cpp, BYTE_EXACT 16/16,
    # unblocked by the BaseWin::zoom declaration) was homed in the same wave
    # and is counted in the 1055 above, not here.
    # 1055 -> 1054 (2026-09-04, same wave, editor_clear_terrain): 004e1380.cpp
    # homed BYTE_EXACT 30/30. Both this and survey above are whole-tile walks
    # over MapTiles, and both wanted the stride advance in the for-increment
    # clause rather than at the end of the body.
    "artifact files (recovered/)": 1045,
    # 1683 -> 1681 (2026-08-30, caviar-voxel batch): 0063ad60.cpp and
    # 0063f9b0.cpp transcribed into src/caviar.cpp (one BYTE_EXACT, one
    # honest MISMATCH).
    # 1677 -> 1675 (sound batch): load_sound_dll homed into
    # src/sound_loader.cpp and its message helper with it; 004c5e50.cpp and
    # 004c9080.cpp deleted.
    # 1674 -> 1672 (2026-09-01, dipedit read_check batch): 0060ec10.cpp
    # (CheckBox::get_state_id) and 0055b760.cpp (treaty_off) promoted into
    # checkbox.cpp and faction.cpp - the two archive-only forwarders the
    # address index had begun to count.
    # 1672 -> 1670 (same batch, the concurrent on_button_clicked homing's
    # two): 004d9de0.cpp and 004db080.cpp, whose bodies are homed and claimed
    # in dipedit.cpp and whose only remaining effect was an address-index
    # DUPLICATE apiece.
    # 1675 -> 1670 (2026-09-02, DipEdit UNK1-4 batch): 004da760/004da7e0/
    # 004da870/004da900.cpp homed BYTE_EXACT into src/dipedit.cpp.
    # 1666 -> 1665 (2026-09-02, Buffer::box_sprite batch): 005e2dd3.cpp homed
    # into src/buffer.cpp; the three tile()/tile_mask() transcriptions it
    # calls (005da6c0/005da860/005da9f0) are still artifacts, so they stay.
    # 1670 -> 1664 (same day, boot-sequence batch): 0058f040.cpp and
    # 0058f2f0.cpp homed into src/game.cpp - system_init MISMATCH 32/135 and
    # game_init MISMATCH 74/82, both compiling, both with their scaffold
    # archive units deleted.
    # 1664 -> 1663 (this merge): box_sprite's archive deletion joins the boot
    # batch's two; the union census above is the count.
    # 1666 -> 1665 (2026-09-03, Net::check_for_lobby): 0062d9e0.cpp deleted
    # with the body it drafted, homed into src/net_class.cpp as the DirectPlay
    # lobby probe - honest MISMATCH 20/432, the residual documented on the
    # marker (VC6 merges the three bare returns; the DP struct store order is
    # scheduler output, not source order).
    # (this merge): the union census is the count.
    # 1662 -> 1661 (2026-09-03, boot-callee batch): 005c9860.cpp deleted with
    # the MultiDebug::init body it drafted - homed BYTE_EXACT into
    # src/multidebug.cpp as a real member, its GraphicWinInitShim gone with it.
    # 1661 -> 1659 (2026-09-03, the archive-only-forwarder batch): 0055ba80.cpp
    # (agenda_on BYTE_EXACT 56/56) and 0062d8e0.cpp (Net::init BYTE_EXACT
    # 87/87) homed into src/faction.cpp and src/net_class.cpp, both files gone
    # with the bodies they drafted.
    # 1661 -> 1660 (2026-09-03, menu batch): 005fb240.cpp deleted with the body
    # it drafted - Menu::UNK5 homed in src/menu.cpp (MISMATCH 30/32, same
    # residual). 005fb100/005fb160/005fb700 STAYED: they call
    # PullDown::add_item / add_separator / rename_item, which pulldown.h did
    # not declare, and that pass did not own the header.
    # 1653 -> 1649 (2026-09-03, the Buffer tile batch): 005da6c0.cpp,
    # 005da860.cpp, 005da9f0.cpp and 005e2b00.cpp deleted with the
    # tile()/tile_mask()/copy_mask() bodies homed into src/buffer.cpp (all
    # four MISMATCH, now compiling rather than NO_COMPILE - the frame and
    # register-save-set residual is documented on the markers).
    # 1660 -> 1656 (2026-09-03, the PullDown-declaration batch): the header
    # declares the three members now, so 005fb100.cpp (Menu::add_menu_item)
    # and 005fb700.cpp (Menu::rename_menu_item) are deleted with their
    # BYTE_EXACT bodies in src/menu.cpp; 005fb160.cpp and 005fb930.cpp sat in
    # recovered/units/ and are counted on that ceiling. 005f89d0.cpp
    # (PullDown::add_item) and 005f8e50.cpp (rename_item) STAY unrecovered -
    # 527 and 494 bytes, declared and standing as pending_bodies forwarders
    # for the two homed callers that reach them.
    # 1649 -> 1648 (2026-09-03, net_class batch): 00631a60.cpp deleted with
    # process_voice homed into src/net_class.cpp BYTE_EXACT 78/78 - the
    # unsigned level local (the image's `jae`/`jbe` against a getter the
    # catalogue spells `H`) is the lever the marker records.
    # 1648 -> 1647 (2026-09-03, gamma/imagebutton batch): 0062bf20.cpp
    # promoted - PushButton::PushButton() homed into src/gamma.cpp BYTE_EXACT
    # 44/44 once BaseButton's constructor went out of line, which is also
    # what took ??0ImageButton and ??0Gamma byte-exact.
    # 1251 -> 1247 (2026-09-04, caviar P2 wave 2): 00638430.cpp,
    # 00638720.cpp, 00638cc0.cpp, 00638de0.cpp homed into src/caviar.cpp as
    # the CVR loader's teardown and chunk readers.
    # 1247 -> 1245 (2026-09-04, same wave): 00616bd0.cpp homed into
    # src/caviar.cpp; 0063b072.cpp homed into src/caviar.cpp.
    # 1245 -> 1244 (same wave, set_shadow_vector): 006181d0.cpp homed into
    # src/caviar.cpp, SHAPE_EXACT 40/44 on a TU-local pragma function(cos).
    # 1189 -> 1181 (2026-09-04, P2/P3 wave 3): four homed into src/game.cpp
    # by the general/game batch, three into src/mapwin.cpp by the terrain
    # batch (00462870/00470550/00470790), the rest by the veh action batch.
    "unrecovered files": 1177,
    # 1189 -> 1188 (2026-09-04, console homing): 005148b0.cpp homed into
    # src/console.cpp as Console::move_cursor, its four fixed addresses all
    # already named in the tree.
    # 1188 -> 1184 (2026-09-04, same batch): 005147e0.cpp (set_cursor),
    # 00515530.cpp (survey2, BYTE_EXACT), 004e13e0.cpp (editor_fungus) and
    # 004e1f70.cpp (editor_lock2) homed into src/console.cpp.
    # 1184 -> 1183 (2026-09-04, console claimed wave): 00515450.cpp homed into
    # src/console.cpp with its claim.
    # 1183 -> 1180 (2026-09-04, console claimed wave): 004ba5f0.cpp and
    # 004ba650.cpp homed into src/tutwin.cpp (the tutorial gates, with their
    # two shown-bit words named), 00511480.cpp into src/console.cpp.
    "unrecovered files": 1168,
    # 2717 -> 2748 (2026-09-04, P2 net batch): NetFifo promoted with
    # close/add/get/get, NetFifoNode, head_/tail_/count_ - the deliberate
    # raise this ceiling exists for (class modelling grows it; the class
    # pass retiring the file deletes it).
    "hypothesis_layouts.h lines": 2748,
}


def scaffold_census():
    """The scaffold sizes, counted live against the ceilings above."""
    counts = {}
    for name in SCAFFOLD_MARKERS:
        text = (REPO / "src" / name).read_text(errors="replace")
        counts[f"{name} markers"] = len(re.findall(r"ORIGINAL: 0x", text))
    pending = (REPO / "src" / "pending_bodies.cpp").read_text(errors="replace")
    counts["PENDING_BODY forwarders"] = len(
        re.findall(r"PENDING_BODY\(", pending))
    counts["artifact files (recovered/)"] = sum(
        1 for _ in (REPO / "src" / "recovered").rglob("*.cpp"))
    counts["unrecovered files"] = sum(
        1 for _ in (REPO / "src" / "unrecovered").glob("*.cpp"))
    counts["hypothesis_layouts.h lines"] = sum(
        1 for _ in (REPO / "src" / "hypothesis_layouts.h").open())
    return counts


def census(root):
    r"""Counted LINE BY LINE, and comment lines are skipped.

    Both halves matter. Prose describing a shape is not an instance of it - the
    explanations in this file would otherwise count themselves. And joining the
    file into one string lets `\s*` inside a pattern span newlines, which
    silently found five matches that do not exist: 66 against the 61 the
    line-wise count agrees on.
    """
    counts = collections.Counter()
    files = collections.defaultdict(set)
    for name_of_file, per_shape in census_by_file(root).items():
        for name, n in per_shape.items():
            counts[name] += n
            files[name].add(name_of_file)
    return counts, files


def _code_lines(path):
    """Non-comment lines, and that includes `/* */` INTERIORS. This walk
    skipped only `//`-prefixed lines, so a Purpose block explaining why a
    homed body's `g_00612bf8` binding was dropped COUNTED that mention as a
    live anonymous global. class_debt.py already had this fix; the two
    censuses disagreed about what a comment is."""
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
        yield line.split("//", 1)[0]


def _selftest_code_lines() -> str:
    """`_code_lines` must keep a `*`-leading STATEMENT and drop real prose.

    A CHECK, NOT A COMMENT, because this exact walk was wrong for a long
    time in two censuses at once and nothing noticed: `*reinterpret_cast<T *>
    (self + 0xNN) = v;` starts with `*`, was classified as block-comment
    continuation, and vanished from the very census that exists to count it.
    Returns a complaint, or "" when the walk behaves.
    """
    import tempfile, os
    sample = (
        "/* a block\n"
        " * continuation must be dropped\n"
        " */\n"
        "// a line comment must be dropped\n"
        "*reinterpret_cast<int *>(self + 0x10) = g_00dead01;\n"
        "    *g_00dead02 = 0;\n"
    )
    fd, path = tempfile.mkstemp(suffix=".cpp")
    try:
        os.write(fd, sample.encode()); os.close(fd)
        kept = [ln.strip() for ln in _code_lines(pathlib.Path(path)) if ln.strip()]
    finally:
        os.unlink(path)
    if len(kept) != 2 or not all(k.startswith("*") for k in kept):
        return (f"_code_lines is misclassifying code as comment: kept {kept!r}, "
                f"expected the two `*`-leading statements and nothing else")
    return ""


def _skipped(path) -> bool:
    """Directories no census counts: the artifact archives (they die by
    homing, not by cleanup) and the vendored zlib (not our code to fix)."""
    return bool({"recovered", "unrecovered", "vendor"} & set(path.parts))


def census_by_file(root):
    """The same census, keyed per file: {filename: Counter{shape: n}}.

    ONE LOOP, TWO CONSUMERS. `frontier.py --by-class` needs per-file counts
    to join them onto the class that owns the file, and it used to carry its
    own copy of this walk. The copy drifted: it iterated `SHAPES` alone and
    globbed `*.cpp` alone, so the DONE bar could not see 320 vtable-shape
    sites (34 of them in win.cpp) and tagged classes finished that were not.
    A second list someone has to keep in step IS the defect; there is one
    walk now, and `census` is derived from it.
    """
    per_file = collections.defaultdict(collections.Counter)
    for path in sorted(root.rglob("*.cpp")):
        if _skipped(path):
            continue
        for line in _code_lines(path):
            # SHAPES are .cpp-only BY MEASUREMENT: loosing them on headers
            # counts DECLARATIONS as sites (free-function destructor alone
            # went 10 -> 20). HEADER_SHAPES were measured over BOTH.
            for name, _ceiling, rx, _why in SHAPES + HEADER_SHAPES:
                n = len(rx.findall(line))
                if n:
                    per_file[path.name][name] += n
    # rglob, MATCHING THE .cpp WALK ABOVE. This was `glob` while the .cpp
    # side was `rglob`, so the function silently counted no headers at all
    # when handed anything but `src` itself - a caller passing the repo root
    # got 1044 where the gate got 943 and neither number named the other.
    # Both walks are recursive now and both refuse the same directories.
    for path in sorted(root.rglob("*.h")):
        if _skipped(path):
            continue
        for line in _code_lines(path):
            for name, _ceiling, rx, _why in HEADER_SHAPES:
                n = len(rx.findall(line))
                if n:
                    per_file[path.name][name] += n
    return per_file


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--lower-ceilings", action="store_true",
                    help="rewrite ceilings DOWN to the measured census; "
                         "never up - raising one stays a deliberate edit")
    ap.add_argument("--root", type=pathlib.Path, default=REPO / "src",
                    help="scan elsewhere; used to positive-control --check")
    args = ap.parse_args()
    complaint = _selftest_code_lines()
    if complaint:
        print(f"COMPILER WORK CENSUS IS BROKEN: {complaint}")
        return 1

    counts, files = census(args.root)

    if args.lower_ceilings:
        # Same workflow defect class_debt had: a pass that empties four
        # shapes reports one drop per run and stays red until each ceiling
        # matches. Lowering can only tighten a ratchet, so it is mechanical;
        # RAISING one is still a deliberate edit and this refuses to do it.
        source = pathlib.Path(__file__).read_text()
        changed = []
        for name, ceiling, _rx, _why in SHAPES + HEADER_SHAPES:
            n = counts[name]
            if n >= ceiling:
                continue
            pat = re.compile(r'(\(\s*"' + re.escape(name) + r'",\s*)' + str(ceiling) + r'\b')
            source, hits = pat.subn(r"\g<1>" + str(n), source)
            if hits:
                changed.append((name, ceiling, n))
        for name, ceiling in list(SCAFFOLD_CEILINGS.items()):
            n = counts.get(name)
            if n is None or n >= ceiling:
                continue
            pat = re.compile(r'(^\s*"' + re.escape(name) + r'":\s*)' + str(ceiling) + r',', re.M)
            source, hits = pat.subn(r"\g<1>" + str(n) + ",", source)
            if hits:
                changed.append((name, ceiling, n))
        if changed:
            pathlib.Path(__file__).write_text(source)
            for name, was, now in changed:
                print(f"  lowered {name}: {was} -> {now}")
        print(f"{len(changed)} ceiling(s) lowered to the measured census")
        return 0

    grew, shrank = [], []
    for name, ceiling, _rx, why in SHAPES + HEADER_SHAPES:
        n = counts[name]
        if n > ceiling:
            grew.append((name, n, ceiling))
        elif n < ceiling:
            shrank.append((name, n, ceiling))
        if not args.check:
            flag = "GREW" if n > ceiling else "down" if n < ceiling else "    "
            print(f"  {flag} {n:4d}/{ceiling:<4d} {name}")
            print(f"           {why}")
            print(f"           {len(files[name])} file(s)")

    scaffold_grew, scaffold_shrank = [], []
    scaffolds = scaffold_census()
    for name, ceiling in SCAFFOLD_CEILINGS.items():
        n = scaffolds.get(name, 0)
        if n > ceiling:
            scaffold_grew.append((name, n, ceiling))
        elif n < ceiling:
            scaffold_shrank.append((name, n, ceiling))
        if not args.check:
            flag = ("GREW" if n > ceiling
                    else "down" if n < ceiling else "    ")
            print(f"  {flag} {n:4d}/{ceiling:<4d} {name}  [scaffold]")
            print(f"           retired by homing and the class passes; at zero"
                  f" it is deleted in its own commit")

    total = sum(counts.values())
    if grew or scaffold_grew:
        for name, n, ceiling in grew + scaffold_grew:
            print(f"COMPILER WORK GREW: {name} is {n}, above its ceiling of {ceiling}")
        return 1
    # SLACK IS A REGRESSION TOO, not advice. A ceiling above its count is
    # re-fillable space: the next regression hides inside it exactly as it
    # would above a stale total, which is how one unit of drift went
    # unnoticed for the whole day a ratchet existed (finding 4, 2026-08-23:
    # artifact files sat at 1395 under a 1396 ceiling from birth). A count
    # below its ceiling means an improvement landed WITHOUT claiming its
    # ratchet - so every improvement commit must set the ceiling from its
    # own measured number, and anything else fails here.
    if shrank or scaffold_shrank:
        for name, n, ceiling in shrank + scaffold_shrank:
            kind = "scaffold" if name in SCAFFOLD_CEILINGS else "compiler work"
            print(f"{kind} SLACK: {name} is {n}, below its ceiling of "
                  f"{ceiling} - set the ceiling to {n} in this same commit")
        return 1
    if not args.check:
        print(f"\n{total} site(s) doing the compiler's work, across "
              f"{len(SHAPES) + len(HEADER_SHAPES)} shapes; "
              f"{len(SCAFFOLD_CEILINGS)} scaffold ceilings watched")
    else:
        print(f"compiler work: {total} site(s), every ceiling exact")
    return 0


if __name__ == "__main__":
    sys.exit(main())
