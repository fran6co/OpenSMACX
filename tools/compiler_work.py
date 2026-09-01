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
    ("construct", 10,
     re.compile(r"^\w[\w :*&]*\b\w+::construct\(\)\s*\{", re.M),
     "a `construct()` method standing in for a constructor. Convert it; put "
     "anything the image writes BEFORE the vfptr store in the member-"
     "initialiser list, and drop the trailing `return this;`."),

    ("free-function destructor", 5,
     re.compile(r"^\w[\w :*&]*\b\w*destructor_redirect\w*\s*\(", re.M),
     "a destructor modelled as a free function. Make it `X::~X()`. A free "
     "function is only needed where C++ cannot take a destructor's address."),

    ("explicit base construct/destroy", 12,
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
    ("placement new on a subobject", 14,
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
    ("VCall shim", 15,
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
    ("ORIGINAL() vtable-slot dispatch", 34,
     re.compile(r"\bORIGINAL\((?=.*vtable\[)"),
     "reads the target out of a vtable slot by hand because the method is not "
     "declared virtual. Declare it - the compiler writes this dispatch."),

    ("ORIGINAL() named-pointer seam", 87,
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
    "deleting_thunks.cpp markers": 74,
    "delegation_thunks.cpp markers": 26,
    # 42 -> 41 (sound batch): field_accessor_0062d390_redirect retired for the
    # real `sub_62d390` in general.cpp.
    "field_accessors.cpp markers": 41,
    "leaf_recoveries.cpp markers": 52,
    "nullsub_thunks.cpp markers": 56,
    # 234 -> 231 (2026-08-30, caviar-voxel batch): the three seams Caviar's
    # homing had declared for sub_639390 / sub_63ad60 / sub_63f9b0 retired -
    # all three bodies now live at the foot of src/caviar.cpp.
    "PENDING_BODY forwarders": 211,
    # 1326 -> 1325 (2026-08-30, caviar-voxel batch): recovered/00639390.cpp
    # homed BYTE_EXACT into src/caviar.cpp.
    # 1325 -> 1294 (2026-08-29, caviar-voxel batch): the six vx_* IO shims
    # homed into src/caviar.cpp as callable Caviar statics (three BYTE_EXACT:
    # vx_read/vx_write/vx_seek); their four archive claims died with the
    # files and three live claims were re-banked by osmx record.
    "artifact files (recovered/)": 1294,
    # 1683 -> 1681 (2026-08-30, caviar-voxel batch): 0063ad60.cpp and
    # 0063f9b0.cpp transcribed into src/caviar.cpp (one BYTE_EXACT, one
    # honest MISMATCH).
    # 1677 -> 1675 (sound batch): load_sound_dll homed into
    # src/sound_loader.cpp and its message helper with it; 004c5e50.cpp and
    # 004c9080.cpp deleted.
    "unrecovered files": 1675,
    "hypothesis_layouts.h lines": 2723,
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
