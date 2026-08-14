#!/usr/bin/env python3
"""Publish the prototype that each MSVC-mangled name ALREADY STATES.

1,605 catalogue rows carry a mangled name and no prototype. A mangled name is
not a hint: MSVC writes the calling convention, the return type and the full
argument type list into it mechanically, and the linker relies on that being
exact. Those 1,605 prototypes are therefore already in this repository, written
in a different alphabet, and nothing was reading them.

WHY A COMMITTED CSV. Deriving this needs `undname` (tools/requirements.txt) to
turn the mangling into types, and the gate below needs the hash-pinned
executable. `--check`, `load()` and the test suite must work on a machine that
has none of the three, so the answer is derived locally and committed, exactly
like every other catalogue under docs/recovery.

WHAT THIS IS NOT. It is not a decompilation and it is not a claim about
parameter MEANING. A mangled name says `int`; it never says `factionID`. It
says `void *`; it never says `HWND`. IDA's recorded prototypes carry both of
those and this generator cannot and does not reproduce them - it refuses to
overwrite a row that already has a prototype, and emits only for rows that have
none.

THE BODY WINS, AND NOW IT IS ASKED. The two controls below read the NAME and
IDA's RECORDING. Neither is the machine code, and for nine published rows the
machine code said something else: the `ret imm` those bodies execute
contradicted the callee purge their own derived prototype implied. Every
candidate is now checked against the bytes before it is published, and a
contradicted row is REFUSED under its own reason rather than emitted. Eight of
the original nine were one mistake made eight times and are now correct.

AND WHERE THE BODY IS A TAIL-CALL THUNK, THE HOP IS TAKEN. A body that ends in
a direct `jmp` executes no `ret` of its own, and an earlier version of this gate
called that "no evidence". For 345 of the 1,601 candidates it was instead
evidence one instruction away: resolve the jump ONE hop and read the target's
`ret imm`. Measured 2026-07-31 over the 1,601 candidates:

    purge-VERIFIED, from the row's own body                 1,253
    purge-VERIFIED-VIA-TAIL-JUMP, from one hop                277
    purge-UNKNOWN, no single REACHABLE `ret imm` either way     23
    refused, contradicted by the row's own body                 1
    refused, contradicted through a tail jump                  47
                                                            -----
                                                            1,601

The 47 are the finding and they are one shape - every `??3<Class>@@SAXPAXI@Z`
row in the image, 47 of 47, published as purging 0 while the body they jump to
executes `ret 4`. The 277 are the control that makes the hop a method rather
than a way of manufacturing refusals. See "The machine-code refusal gate" below
for both, for the stack-neutrality condition that keeps 16 more rows honestly
unknown, and for why the committed docs/recovery/callee-purge.csv exists.

THE CONTROL, replayed on every run before anything is written. 3,213 rows carry
BOTH a mangled name and an IDA-recorded prototype. Deriving from the name alone
and comparing against the recording, measured on 2026-07-31:

    calling convention          3213 / 3213   100.00%
    argument stack layout       3213 / 3213   100.00%
    return type, normalised     3197 / 3213    99.50%
    receiver present or not     3204 / 3213    99.72%
    argument types, normalised  3163 / 3213    98.44%

The two figures the ABI rests on - the convention and the stack layout - agree
on every one of the 3,213 rows. Nothing was tuned to get there: the type
normaliser was written from the recorded column before the control was run.

All five floors are pinned in AGREEMENT_FLOOR and the generator REFUSES TO WRITE
if any of them regresses, the same way derive_class_size_bounds.py refuses when
a bound falls below a pinned size. A drop means the premise - that the name and
the recording encode the same function - has broken somewhere, and a wrong
prototype is worse than a missing one because every consumer treats the column
as certain.

The residual 75 disagreements are named, not rounded away; run --disagreements
to print every one. They are IDA disagreeing with the linker, and in most of
them IDA is carrying information the mangling never had rather than
contradicting it:

  * receiver, 9 rows. Every one is a `QAA`/`QAG` member whose recorded
    prototype DROPS the receiver a C++ instance method must have
    (?OnNCHitTest@Win, ?on_sys_key@Win, ?veh_draw@TutWin ...).
  * return type, 16 rows. Eight methods whose mangling encodes `X` (void) but
    which IDA typed `int`, and eight constructors IDA typed as returning the
    class pointer (??0FactionArt, ??0Log, ??0Text ...).
  * argument types, 50 rows. Mostly IDA's APPLIED types where the mangling has
    only the underlying scalar - `DatalinkID` for `I`, `SortType` for `H`,
    `social_category*` for `int*`, `#120*` for `TexHeap*`. Four rows are real
    contradictions: ?load_daemon, ?setup_player, ?UNK8@Palette and
    ?TimerProc@Time, where IDA typed a parameter as a pointer that the mangling
    spells as an integer.

THE WEAK SOURCE IS A SEPARATE FILE. 1,133 rows have no mangled name at all.
For 910 of them IDA offers an argument COUNT, which agrees with the mangled
population 94.1% of the time - good enough to publish as a hypothesis, not as a
prototype. Those go to docs/recovery/prototype-hypotheses.csv, carry no
prototype string, and deliberately carry NO calling convention: IDA's stated
convention agrees only 30.3% of the time and is not worth writing down. The two
files never merge, and `load()` reads only the certain one.

WHAT THE CONTROLS STILL CANNOT SEE, named because a control that is silent
about its blind spots is read as covering them. The agreement control's
population is the 3,213 rows that carry a recording, and the emitted population
is by construction disjoint from it: 788 emitted rows are `??__E`/`??__F`
dynamic initialisers (396 and 392), a shape the control population does not
contain at all. Not one of the 1,553 emitted rows carries a two-slot argument,
so `stack_slots` is all-ones throughout and the "stack layout 100%" axis is
measuring argument POSITIONING and not argument WIDTH. The purge gate is the one
axis that reaches the emitted rows directly, and it now reaches 1,530 of them -
1,253 through the body and 277 through one hop.

AND THE EVIDENCE MUST BE REACHABLE. A linear sweep cannot tell a live `ret` from
a dead one, which let two rows claim VERIFIED on a number no path executes:
`?_JumpToContinuation` ends at `jmp eax` and its epilogue is unreachable, and
`?terminate`'s only `ret` sits in an EH funclet. Both numbers were right, which
is precisely why the label was wrong - "verified" asserted an observation the
processor never makes. A recursive walk from the entry now filters the evidence,
and because an indirect jump contributes no edge the result is a LOWER bound: it
can withhold the word, never award it wrongly. The same filter EARNED a claim
too - `?__ArrayUnwind` read as ambiguous on two distinct `ret`s until the dead
one in its unwind funclet was discarded, leaving a single reachable `ret 0x10`.

The `static` kind used to be the sharpest of these blind spots: 47 emitted rows
against 0 in the control, an entire kind nothing had ever checked. The hop
closed it by refusing all 47 - which is what a blind spot looks like when
something finally looks at it. There are now 0 `static`-kind rows in the
published catalogue and the kind needs no separate caveat.

Addresses, names, type spellings and one integer per function - the callee purge
- reach the output. The gate reads the pinned executable; no byte of it, no
disassembly text and no instruction count is written to any file.
"""

from __future__ import annotations

import argparse
import csv
import io
import re
import sys
from pathlib import Path
from typing import NamedTuple

sys.path.insert(0, str(Path(__file__).resolve().parent))

import recover_conventions as conventions  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
# No FUNCTIONS_CSV here on purpose. This module reads the catalogue through
# `load_rows()` -> emit_translation_unit.load_functions() -> src/ annotations;
# docs/recovery/functions.csv is deleted. The constant outlived the file and was
# dead for every caller except a test, which happily opened the missing path and
# errored - so keeping a name for a store this module no longer uses is an
# invitation to read it again.
PROTOTYPES_CSV = REPO_ROOT / "docs" / "recovery" / "derived-prototypes.csv"
HYPOTHESES_CSV = REPO_ROOT / "docs" / "recovery" / "prototype-hypotheses.csv"
IDA_CSV = REPO_ROOT / ".opensmacx" / "game" / "ida94-functions.csv"

FIELDS = ("address", "name", "prototype", "convention", "stack_slots",
          "purge")
HYPOTHESIS_FIELDS = ("address", "name", "argument_count", "evidence")

# The measured control, pinned. Each entry is (agreeing, population) as taken on
# 2026-07-31; the run refuses to write if the live rate falls below the ratio.
# Not a target to be edged upward silently - moving one of these down is an
# admission that a derived prototype disagrees with a recorded one, and the row
# belongs in --disagreements before the floor moves.
#
# MOVED 2026-08-14, 3213 -> 3215, and the two rows are named because that is
# what this comment demands. `0x0060E4D0 ?init_class@RadioButton@@QAAHXZ` and
# `0x0060FC60 ?init_class@CheckBox@@QAAHXZ` had an EMPTY `// prototype` line
# until agents recovering them filled it in from the mangled name. They joined
# the control and both disagree with IDA on the receiver, which drops the
# ratio without any row changing its answer.
#
# Both are `QAA` - public, non-static, `__cdecl` - and every one of the eleven
# receiver disagreements is that family or its `QAG` sibling. The name says a
# member with a receiver passed on the stack; IDA reads it as a free function.
# The mangling is the linker's own record, so the name is right and the
# disagreement is IDA's, which is why this is a floor and not a defect: it
# counts how far the two sources are apart, and the distance did not grow.
# `return type` rose by one on the same two rows.
#
# `return type` RATCHETED 3198 -> 3203 on 2026-08-14. Five catalogued names
# spell `void` for bodies that end `neg eax; sbb eax, eax; and al, 0xfe; add
# eax, 2` and the corrections in `catalogue_corrections` follow the bytes -
# `?init@GraphicWin@@...` and four `Time` methods. Left at 3198 this floor
# would accept losing all five again in silence, which is the whole reason
# these are ratchets and not targets.
AGREEMENT_FLOOR = {
    "convention": (3215, 3215),
    "receiver": (3204, 3215),
    "stack layout": (3215, 3215),
    "argument types": (3163, 3215),
    "return type": (3203, 3215),
}

# The control's own size, pinned separately, because every entry above is a
# RATIO and a ratio is blind to its denominator. If rows stop reaching the
# control - a parse regression, a column rename, a filter that quietly excludes
# a shape - all five rates can stay green on a shrinking population. Measured
# 2026-07-31: a 10% row loss holds every rate and moves this from 3213 to 2892.
CONTROL_POPULATION_FLOOR = 3215

# Likewise the published catalogue. A generator that emits fewer rows for a
# reason nobody chose is a silent loss of coverage, and the purge gate's own
# refusals are counted and named, so a legitimate drop is always explainable.
#
# RATCHETED 1553 -> 1600 on 2026-08-13, when `catalogue_corrections` renamed 47
# rows catalogued `??3<Class>@@SAXPAXI@Z` to the this-adjusting thunk spellings
# their bodies carry. They were the whole of the purge gate's hop refusals;
# under the corrected names all 47 derive and publish. Left at 1553 the gate
# would have accepted losing every one of them again in silence.
#
# 1600 -> 1598 on 2026-08-14, and this is a GRADUATION rather than a loss. A
# row is a candidate here only while the catalogue has no prototype for it;
# `0x0060E4D0 ?init_class@RadioButton@@QAAHXZ` and `0x0060FC60
# ?init_class@CheckBox@@QAAHXZ` had an empty `// prototype` line until agents
# recovering them filled it in, so they stopped needing a derived one. The
# coverage did not drop - it moved to a better source, which is the direction
# this whole generator exists to make unnecessary.
PUBLISHED_ROWS_FLOOR = 1598

# One copy, in recover_conventions, because two copies of this list drifted.
CONVENTION_TOKEN = conventions.CONVENTION_TOKEN

# ---------------------------------------------------------------------------
# IDA's type vocabulary
# ---------------------------------------------------------------------------
#
# IDA does not spell C++ types the way a demangler does, and the catalogue's
# recorded prototypes are IDA's spelling verbatim (tools/export_recovery_
# inventory.py:311 is `idc.GetType(start)`). Emitting a different spelling for
# the same type would make the derived rows parse differently from the recorded
# ones in tools/generate_signature_oracles.py, so the demangled type is
# rewritten into IDA's alphabet here. Every entry below was READ OFF the 3,213
# rows that have both a name and a recording - none of it is invented.

SCALAR_SPELLING = {
    "char": "int8",
    "signed char": "int8",
    "unsigned char": "unsigned int8",
    "short": "int16",
    "unsigned short": "unsigned int16",
    "long": "int",
    "unsigned long": "unsigned int",
    "__int64": "__int64",
    "unsigned __int64": "unsigned __int64",
}

# Widths in 4-byte stack slots. Everything not listed is one slot; a pointer is
# always one slot on this 32-bit image.
WIDE_SCALAR = re.compile(r"\b(double|__int64|long long)\b")


def ida_type(text: str) -> str:
    """A demangled MSVC type, respelled the way IDA writes it."""
    text = text.strip()
    if "(" in text:
        return _ida_function_pointer(text)
    text = re.sub(r"\b(struct|class|union|enum)\s+", "", text)
    text = re.sub(r"\b(const|volatile)\b", " ", text)
    # A reference occupies one pointer-sized slot and IDA writes it as one.
    text = text.replace("&", "*")
    text = re.sub(r"\s+", " ", text).strip()
    stars = text.count("*")
    # Strip EVERY trailing star, not just the last run: `struct Buffer * *`
    # leaves a star behind under `\s*\*+\s*$` and renders as `Buffer ***`.
    base = re.sub(r"[\s*]+$", "", text).strip()
    base = SCALAR_SPELLING.get(base, base)
    return base + "*" * stars


def _ida_function_pointer(text: str) -> str:
    """`int (__cdecl*)(void)` the demangler's way -> `int (__cdecl *)()` IDA's.

    Measured on 18 rows carrying `P6AHXZ` and 8 carrying an argument list; the
    two differences are the space before the `*` and the empty parameter list
    where the demangler writes `void`.
    """
    text = re.sub(r"\s+", " ", text.strip())
    text = re.sub(r"(__\w+)\s*\*", r"\1 *", text)
    inner = text.rfind("(")
    if inner >= 0 and text.endswith(")"):
        args = text[inner + 1:-1].strip()
        parts = [] if args in ("", "void") else [
            ida_type(part) for part in conventions.split_args(args)]
        text = text[:inner] + "(" + ", ".join(parts) + ")"
    return text


def stack_slots(types: list[str]) -> list[int]:
    """4-byte slots each declared argument occupies, in order."""
    return [2 if ("*" not in one and WIDE_SCALAR.search(one)) else 1
            for one in types]


# ---------------------------------------------------------------------------
# Reading a demangled signature
# ---------------------------------------------------------------------------

class Derived:
    """What a mangled name says, rendered the way the catalogue writes it."""

    __slots__ = ("address", "name", "convention", "kind", "return_type",
                 "argument_types", "receiver", "purge")

    def __init__(self, address, name, convention, kind, return_type,
                 argument_types, receiver):
        self.address = address
        self.name = name
        self.convention = convention
        self.kind = kind
        self.return_type = return_type
        self.argument_types = argument_types
        self.receiver = receiver
        # Filled in by the machine-code gate. UNKNOWN until something has
        # actually read the bytes, so a row nobody checked never reads as one
        # that was checked and passed.
        self.purge = UNKNOWN

    @property
    def takes_this(self) -> bool:
        return self.receiver is not None

    @property
    def prototype(self) -> str:
        """`<ret> (<conv> <MANGLED>)(<params>)`, byte-for-byte the catalogue's
        shape. The mangled name goes in raw; `this` is explicit and first;
        there is no `(void)` form, because the recorded column has none."""
        params = list(self.argument_types)
        if self.receiver is not None:
            params.insert(0, f"{self.receiver}* this")
        return (f"{self.return_type} ({self.convention} {self.name})"
                f"({', '.join(params)})")

    @property
    def slots(self) -> str:
        return "".join(str(width) for width in stack_slots(self.argument_types))


def qualified_name(demangled: str) -> str:
    """`StringStruct::close` out of a demangled signature, or ''."""
    found = _top_level_convention(demangled)
    if found is None:
        return ""
    start = demangled.find("(", found.end())
    if start < 0:
        return ""
    return demangled[found.end():start].strip()


def _top_level_convention(demangled: str):
    for found in CONVENTION_TOKEN.finditer(demangled):
        head = demangled[:found.start()]
        if head.count("(") == head.count(")"):
            return found
    return None


def parse_ranges(value: str):
    """The body's spans, which is NOT `end_address - address`.

    416 catalogued functions are split, and 402 carry a second cold span, so a
    contiguous read from the entry runs past the body and into whatever follows
    it - up to 2,102 bytes of another function in the worst measured case. The
    same arithmetic, taken from the LAST span instead of the first, cost 241
    BYTE_EXACT claims once.

    Moved here from classify_recovered_shapes.py on 2026-08-13 when that tool
    was retired: it was the only thing left in it that anything still used.
    """
    out = []
    for item in (value or "").split(";"):
        if not item:
            continue
        start, end = item.split("-", 1)
        out.append((int(start, 0), int(end, 0)))
    return sorted(out)


def derive_one(address: str, name: str, demangled: str) -> Derived | None:
    """The prototype a mangled name states, or None when it states none.

    None is the honest answer for a data decoration: a vftable and an RTTI
    record are not functions and have no argument list. `split_infix` refuses
    those on the slot check rather than by prefix, and refuses a template name
    outright because it cannot locate the infix in one.

    A `??__E` / `??__F` DYNAMIC INITIALISER IS NOT REFUSED, and an earlier
    version of this docstring said it was. 396 `??__E` and 392 `??__F` rows are
    emitted - 788 rows, 49% of the published catalogue and its single largest
    category. That is the right output: a dynamic initialiser really is a
    `void __cdecl f(void)`, the linker calls it as one, and `undname` prints it
    as one. What refuses them is `recover_conventions.recover`, for its own
    reason (they are seam-forwarding bodies, so they say nothing about a
    convention worth seeding) - not this generator, and not `split_infix`.
    """
    infix = conventions.split_infix(name)
    if infix is None:
        return None
    kind_char, convention_char = infix
    kind = conventions.KIND[kind_char][1]
    convention = conventions.CONV[convention_char]

    found = _top_level_convention(demangled)
    if found is None:
        return None
    argument_text = conventions.demangled_args(demangled)
    if argument_text is None:
        return None
    arguments = conventions.split_args(argument_text)
    if arguments == ["void"]:
        arguments = []
    if any(one == "..." for one in arguments):
        # varargs: the caller decides the layout, so there is no argument list
        # to state. Refused rather than truncated.
        return None
    argument_types = [ida_type(one) for one in arguments]

    head = demangled[:found.start()].strip()
    # `[thunk]:` is a marker, not a return type. The demangler prefixes an
    # adjustor or vtordisp thunk's signature with it, ahead of the access
    # keyword - `[thunk]:public: virtual void * __thiscall Foo::...` - and the
    # anchored strips below then match nothing, so the whole marker-plus-access
    # run would be published as the return type. Measured on
    # `??_GAlphaMovie@@WEEE@AEPAXI@Z`: without this the row reads
    # `[thunk]:public: virtual void*` instead of `void*`.
    head = re.sub(r"^\[thunk\]\s*:\s*", "", head)
    head = re.sub(r"^(public|private|protected)\s*:\s*", "", head)
    # `\b\s*`, not `\s+`. A virtual DESTRUCTOR's head is exactly
    # `public: virtual` with nothing after it, so a pattern that demands
    # trailing whitespace strips nothing and `??1Foo@@UAE@XZ` publishes the
    # literal word `virtual` as its return type.
    head = re.sub(r"^(virtual|static)\b\s*", "", head).strip()
    # A constructor and a destructor have no return type in the mangling; the
    # recorded column writes `void` for 220 of the 228 it holds.
    return_type = ida_type(head) if head else "void"

    receiver = None
    if kind in conventions.THIS_KINDS:
        qualified = qualified_name(demangled)
        scope = qualified.rsplit("::", 1)[0] if "::" in qualified else ""
        if not scope:
            return None
        receiver = scope
    return Derived(address, name, convention, kind, return_type,
                   argument_types, receiver)


# ---------------------------------------------------------------------------
# The machine-code refusal gate
# ---------------------------------------------------------------------------
#
# Everything above this line reads the NAME. The agreement control below reads
# IDA's RECORDING. Neither ever asks the third and only authoritative source -
# the bytes - and tools/disasm.py states the repository's doctrine outright:
# the callee-pop byte count is "the only ground truth for arity".
#
# It matters, because the name and the bytes do disagree. Measured over the
# 1,601 candidates on 2026-07-31, nine published rows were contradicted by the
# `ret imm` their own body executes. Eight were the same mistake made eight
# times (see IMPLIED PURGE below); the ninth,
# ?draw_upper_threshold_trans@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@H@Z
# at 0x0061A760, declares six arguments (24 bytes) and its epilogue executes
# `ret 0x10`. The name's argument list is simply wrong there, so the row is
# refused rather than published.
#
# IMPLIED PURGE. MSVC 32-bit callee-pop, and the one rule that is not obvious:
#
#     __cdecl      0                       the CALLER pops
#     __thiscall   4 * explicit arguments  `this` rides in ECX
#     __stdcall    4 * explicit arguments  `this` ALSO rides in ECX
#
# The receiver is never charged to the stack, on either convention. Charging it
# on __stdcall is what produced all eight of the repeated failures: every `QAG`
# row - public, instance, __stdcall - over-declared by exactly four bytes.
# `?get_hdc@Win@@QAGPAUHDC__@@XZ` was said to purge 4 and executes a plain
# `ret`; `?window_proc@Win@@QAGJPAXIIJ@Z` was said to purge 20 and executes
# `ret 0x10`. All eight QAG bodies agree with the ECX rule and none agrees with
# the stack rule, which is 8/8 in one direction and 0/8 in the other.
# tools/recover_conventions.py reached the same conclusion from a wider
# population and by two further routes - call-site push counts, and this
# repository's own headers declaring these methods `static`.
#
# __pascal and __fastcall are not modelled and yield no implied purge: no
# candidate carries either, and inventing a rule for a population of zero would
# be a rule nothing could check.
#
# THE EVIDENCE IS SOMETIMES ONE INSTRUCTION AWAY. An earlier version of this
# gate swept the body linearly, and where the sweep met no `ret` it filed the
# row `unknown` and said the body "reaches no ret at all". That was false for
# almost all of them. Measured on 2026-07-31, all 345 such bodies END IN A
# DIRECT JMP: they are tail-call thunks, and their `ret` is in the target.
#
# So the gate now takes ONE HOP. Where a body's terminating instruction is a
# direct `jmp <immediate>`, the target is resolved once and ITS single `ret imm`
# becomes the thunk's evidence. Four conditions, each of which is a refusal
# rather than a guess:
#
#   * the jump must be DIRECT - an immediate target. A register or memory jump
#     names no target to resolve, and a conditional branch is not a terminator.
#   * the linear sweep must reach the last byte of the last span. A sweep that
#     desynchronised on embedded data can land on a `jmp` that is not an
#     instruction at all.
#   * the thunk must be STACK-NEUTRAL before the jump - see below.
#   * the target must be a catalogued function with exactly ONE `ret imm`. A
#     target that itself ends in a jump is a CHAIN; this gate does not take a
#     second hop and files the chain `unknown`. 5 rows are chains.
#
# WHY THE TARGET'S PURGE IS THE THUNK'S, AND WHERE THAT REASONING FAILS. The
# target's `ret imm` pops `imm` bytes measured from the stack pointer AS IT
# STANDS AT THE JUMP. The thunk's effective callee purge is therefore the
# target's purge only if the thunk left the stack pointer exactly where it found
# it. That is an assumption about the bytes, so it is CHECKED against them: a
# thunk is refused unless every instruction before the terminating jump leaves
# ESP alone - no push, no pop, no call, no enter/leave, and no operand naming
# ESP. It is not a formality. 16 of the 345 fail it (11 call out, 4 push, 1
# addresses ESP directly), and for those 16 the target's purge is NOT the
# thunk's, so they stay `unknown`. Taking the hop unconditionally would have
# published 16 numbers nothing had verified.
#
# WHAT THE HOP FOUND, and this is the whole reason it is worth taking. Of the
# 324 rows that pass all four conditions, 277 AGREE with the purge their
# published prototype implies and 47 CONTRADICT it. The 277 are the control: if
# one-hop resolution were a biased method it would manufacture disagreements,
# and instead it confirms 85% of the rows it reaches. The 47 are the finding,
# and they are one shape - every `??3<Class>@@SAXPAXI@Z` row in the catalogue,
# 47 of 47. Each is 10 or 11 bytes of `sub ecx, <imm> ; jmp <??_G...>`, each
# target executes `ret 4`, and each was published as a `__cdecl` taking two
# stack arguments and purging 0. A caller compiled against that pushes 8 and
# gets 4 popped. The `??3` (`operator delete`) naming is IDA's: an operator
# delete does not adjust ECX, and `sub ecx, <imm> ; jmp` is the MSVC ADJUSTOR
# THUNK shape - these are adjusted entries to the class's own scalar deleting
# destructor. All 47 are refused.
#
# THE SOURCE OF EACH NUMBER IS RECORDED. docs/recovery/callee-purge.csv carries
# a `source` column - `body`, `tail-jump` or `none` - and the published `purge`
# column distinguishes `verified` from `verified-via-tail-jump`. A number that
# took an indirection must never read as a direct observation of the body.
#
# WHAT "UNKNOWN" MEANS, AND WHY IT IS NOT "CHECKED". After the hop, 22 of the
# 1,601 bodies still expose no single `ret imm`: 16 tail jumps whose thunk is
# not stack-neutral, 5 chains, and one row,
# ?__ArrayUnwind@@YGXPAXIHP6EX0@Z@Z, that executes both `ret` and `ret 0x10`.
# That is an absence of evidence, not a contradiction, so those rows are
# published - and carry `unknown` in the `purge` column, and are counted
# separately in every total this prints. A reader must never be able to mistake
# one for a row the bytes confirmed.
#
# WHY A SECOND COMMITTED FILE. The executable is hash-pinned and uncommitted,
# so this gate cannot run on a machine without it - and `--check` runs in the
# test suite on exactly such machines. Same arrangement as
# derive_class_size_bounds.py and docs/recovery/recovered-shapes.csv: read the
# bytes locally, publish the ANSWER as addresses and integers, and let every
# later run gate itself on the committed answer. When the executable IS present
# the committed answer is re-derived and compared, so it cannot go stale
# unnoticed. Only integers reach the file; no byte, disassembly text or
# instruction count does.

PURGE_CSV = REPO_ROOT / "docs" / "recovery" / "callee-purge.csv"
PURGE_FIELDS = ("address", "purge", "source", "evidence")

VERIFIED = "verified"
VERIFIED_VIA_TAIL_JUMP = "verified-via-tail-jump"
UNKNOWN = "unknown"
CONTRADICTED = "contradicted"

# Where a purge came from. Carried beside the number everywhere it travels, so
# a hopped verdict cannot be read as a direct observation of the body.
SOURCE_BODY = "body"
SOURCE_TAIL_JUMP = "tail-jump"
SOURCE_NONE = "none"

# The refusal reasons the gate files a contradicted row under. Their own
# reasons, never folded into "not a function" - and the two are kept apart
# because they are backed by different amounts of indirection.
PURGE_REFUSAL = "contradicted by the callee purge"
PURGE_REFUSAL_HOP = "contradicted by the callee purge of its tail-jump target"

# Of the rows whose purge came from a hop, the share that AGREED. This is what
# makes one-hop resolution a method rather than a way of manufacturing
# refusals: if it collapses, the hop is resolving the wrong thing and the run
# refuses to write rather than refusing hundreds of rows on bad evidence.
#
# RATCHETED (277, 324) -> (324, 324) on 2026-08-13. The 47 rows that made up
# the entire shortfall were not the hop misfiring: every one was a
# `??3<Class>@@SAXPAXI@Z` name IDA had wrong, and the hop was RIGHT to
# contradict them. With `catalogue_corrections` supplying the thunk spellings
# the bodies actually carry, the hop now confirms every row it reaches, so the
# honest floor is total agreement - any future contradiction here is a name to
# investigate, not a tolerance to spend.
HOP_AGREEMENT_FLOOR = (324, 324)


class Purge(NamedTuple):
    """One function's observed callee purge, and where the number came from."""

    observed: int | None
    source: str
    evidence: str


NO_PURGE = Purge(None, SOURCE_NONE, "")


def implied_purge(derived: Derived) -> int | None:
    """Bytes the derived prototype says the callee pops, or None if it cannot.

    The receiver is deliberately NOT charged a stack slot on any convention -
    see the block above; that is the measured behaviour of all eight `QAG`
    bodies in this image and of every `QAE` one.
    """
    declared = 4 * sum(stack_slots(derived.argument_types))
    if derived.convention == "__cdecl":
        return 0
    if derived.convention in ("__thiscall", "__stdcall"):
        return declared
    return None


def purge_verdict(derived: Derived, purge: Purge) -> str:
    """VERIFIED, VERIFIED_VIA_TAIL_JUMP, UNKNOWN or CONTRADICTED for one row.

    The source travels WITH the number rather than beside it, because the one
    mistake this must not make is letting a purge that took an indirection read
    as a purge somebody read straight off the body.
    """
    implied = implied_purge(derived)
    if purge.observed is None or implied is None:
        return UNKNOWN
    if purge.observed != implied:
        return CONTRADICTED
    return (VERIFIED_VIA_TAIL_JUMP if purge.source == SOURCE_TAIL_JUMP
            else VERIFIED)


# ---------------------------------------------------------------------------
# Reading one body, and the one hop
# ---------------------------------------------------------------------------
#
# These four take a decoded instruction sequence and nothing else - no PE, no
# capstone, no executable - so the hop's rules are testable on the machines
# where the rest of this file is testable. Anything with `.mnemonic`, `.op_str`,
# `.address` and `.size` will do.

# Everything that moves ESP without naming it. `call` is here because a callee
# that purges moves the stack pointer on the caller's behalf, and `ret` because
# a body that returns is not a thunk in the first place.
STACK_MNEMONIC = frozenset((
    "push", "pusha", "pushal", "pushad", "pushf", "pushfd",
    "pop", "popa", "popal", "popad", "popf", "popfd",
    "call", "lcall", "enter", "leave", "ret", "retf", "iret", "iretd",
    "int", "int1", "int3", "into", "syscall", "sysenter"))

ESP_OPERAND = re.compile(r"\besp\b")

# A DIRECT jump: one immediate operand. `jmp eax`, `jmp dword ptr [0x...]` and
# every conditional branch fail this deliberately - there is nothing to resolve.
DIRECT_JUMP = re.compile(r"^0x[0-9a-f]+$")


def ret_immediates(instructions) -> set:
    """Every distinct byte count the sequence's `ret`s pop."""
    return {int(one.op_str, 16) if one.op_str else 0
            for one in instructions
            if one.mnemonic in ("ret", "retf")}


def reachable_addresses(instructions, entry: int) -> set:
    """Instruction addresses reachable from `entry` by nameable control flow.

    A LINEAR SWEEP CANNOT TELL A LIVE `ret` FROM A DEAD ONE, and twice now this
    gate has been caught calling something evidence that the processor never
    executes. `?_JumpToContinuation` ends at `jmp eax`; the `pop/pop/pop/leave/
    ret 8` after it is an epilogue no path reaches. `?terminate`'s only `ret`
    lives in an EH funclet with no edge from the entry. Both numbers happen to
    be right, but "verified" claimed a direct observation, and that is the same
    overstatement as filing a contradiction under "unknown".

    Conditional branches contribute both edges; an unconditional `jmp` breaks
    the fallthrough. An INDIRECT jump contributes no edge at all - past
    `jmp eax` this cannot say where control went. That makes the result a
    LOWER BOUND on reachability, which is the safe direction: it can only
    withhold the word "verified", never award it wrongly.
    """
    by_address = {one.address: one for one in instructions}
    reachable: set = set()
    pending = [entry]
    while pending:
        at = pending.pop()
        while at in by_address and at not in reachable:
            reachable.add(at)
            one = by_address[at]
            if one.mnemonic in ("ret", "retf"):
                break
            if one.mnemonic == "jmp":
                if DIRECT_JUMP.match(one.op_str):
                    pending.append(int(one.op_str, 16))
                break                       # no fallthrough, resolved or not
            if one.mnemonic.startswith("j") and DIRECT_JUMP.match(one.op_str):
                pending.append(int(one.op_str, 16))   # conditional: both edges
            at = one.address + one.size
    return reachable


def stack_shift_reason(instructions) -> str | None:
    """Why this sequence may have moved ESP, or None if it provably did not.

    A tail jump's target pops from the stack pointer AS IT STANDS AT THE JUMP,
    so the target's purge is the thunk's only where this returns None.
    """
    for one in instructions:
        if one.mnemonic in STACK_MNEMONIC:
            if one.mnemonic in ("call", "lcall"):
                return ("it calls out first, and a callee that purges moves "
                        "the stack pointer")
            return "it moves the stack pointer first"
        if ESP_OPERAND.search(one.op_str):
            return "it names the stack pointer first"
    return None


def tail_jump_target(instructions, end: int) -> int | None:
    """The address a body tail-jumps to, or None if it does not tail-jump.

    `end` is the last byte of the last span, and the sweep must reach it
    exactly. A sweep that desynchronised on embedded data can decode a `jmp`
    out of the middle of some other instruction, and that jump names an
    address that was never a branch target.
    """
    if not instructions:
        return None
    last = instructions[-1]
    if last.address + last.size != end:
        return None
    if last.mnemonic != "jmp" or not DIRECT_JUMP.match(last.op_str):
        return None
    return int(last.op_str, 16)


def is_thunk_name(name: str) -> str:
    """The kind character when `name` is a thunk spelling, else ""."""
    infix = conventions.split_infix(name)
    if infix is None or conventions.KIND[infix[0]][1] != "thunk":
        return ""
    return infix[0]


def thunk_adjustment_disagreements(derived, rows, exe: Path) -> list:
    """Rows whose THUNK NAME states a displacement the body does not apply.

    The one claim in a thunk spelling that no other check in this tree can
    falsify. `??_GAlphaMovie@@WEEE@AEPAXI@Z` asserts two independent things: a
    signature - `void *(unsigned int)` on a `__thiscall` - and an ADJUSTMENT,
    `adjustor{1092}`. The callee-purge gate settles the signature, because the
    implied purge of 4 either matches the `ret 4` or it does not. Nothing
    settled the adjustment: change `WEEE@` to `WEEF@` and the purge still
    agrees, the prototype is still derived, every pinned floor still holds, and
    the catalogue now carries a name for a thunk that adjusts by 1093.

    That mattered the moment `catalogue_corrections` supplied 47 of these
    spellings by reading the constant out of the body. The evidence was real,
    but it lived in a comment, and a comment is not a check - so this reads it
    back out of the image and holds the name to it.

    MSVC emits exactly two shapes, and both are decoded rather than pattern
    matched on a mnemonic string:

        adjustor{N}         sub ecx, N
        vtordisp{V, N}      sub ecx, [ecx + V]   then `sub ecx, N` when N != 0

    A row whose body does not open that way is reported, never excused: this
    returns disagreements, and a thunk-named function that adjusts nothing is
    the most interesting thing it could find.
    """
    import pefile
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs

    from generator_support import read_bytes

    by_address = {row.get("address", ""): row for row in rows}
    pe = None
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    out = []
    for one in derived:
        kind_char = is_thunk_name(one.name)
        if not kind_char:
            continue
        # The numbers the NAME states, read with the same decoder the infix
        # scan uses so the two cannot disagree about where they end.
        rest = one.name[one.name.find("@@") + 2:]
        index, stated = len(kind_char), []
        for _ in range(conventions.thunk_adjustments(kind_char)):
            value, index = conventions.mangled_number(rest, index)
            stated.append(value)
        if any(value is None for value in stated):
            out.append((one.address, one.name, "the name's adjustment does "
                        "not decode, so nothing can be held to it"))
            continue
        spans = parse_ranges(by_address.get(one.address, {}).get(
            "body_ranges", ""))
        if not spans:
            continue                    # no body to read; not a disagreement
        if pe is None:
            pe = pefile.PE(str(exe), fast_load=True)
        start, end = spans[0]
        head = list(engine.disasm(read_bytes(pe, start, end - start), start))[:2]
        applied = _applied_adjustment(head)
        if _as_displacements(applied) != _as_displacements(stated):
            out.append((one.address, one.name,
                        f"the name states {_as_displacements(stated)} and the "
                        f"body applies {_as_displacements(applied)}"))
    return out


def _as_displacements(values) -> list:
    """Both sides as SIGNED 32-bit displacements, which is what they are.

    Not a loosening - this is the one place the two spellings of the same
    number have to be reconciled, and getting it wrong in either direction is
    worse than the check not existing. A mangled number carries no sign unless
    it is written with a `?` prefix, so `PPPPPPPM@` in
    `??_GPlanWin@@$4PPPPPPPM@A@AEPAXI@Z` decodes to 0xFFFFFFFC, while capstone
    reads the same four bytes off `sub ecx, [ecx - 4]` as -4. `llvm-undname`
    agrees with capstone and prints `vtordisp{-4, 0}`, because a vtordisp
    offset IS a signed displacement. Comparing the raw values reported that
    row as a disagreement when the two sides said the identical thing.

    Nothing else collapses: 1092 and 1093 are still different numbers, and a
    body that adjusts by 0x444 still contradicts a name that says 0x448.
    """
    return [value - (1 << 32) if value >= (1 << 31) else value
            for value in values]


def _applied_adjustment(head) -> list:
    """What the first instructions actually subtract from ECX, name-shaped.

    `[N]` for an adjustor and `[V, N]` for a vtordisp, so it compares directly
    against the numbers decoded out of the name. `[]` means the body does not
    open by adjusting ECX at all, which no thunk spelling can be right about.
    """
    # Local, like every other capstone use here: this module has to import on
    # a machine with no capstone, which is where `--check` and the tests run.
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

    if not head:
        return []
    first = head[0]
    if first.mnemonic != "sub" or not first.operands:
        return []
    destination, source = first.operands
    if destination.type != X86_OP_REG or first.reg_name(destination.reg) != "ecx":
        return []
    if source.type == X86_OP_IMM:                     # adjustor: sub ecx, N
        return [source.imm]
    if source.type != X86_OP_MEM:
        return []
    # vtordisp: sub ecx, [ecx + V], optionally followed by `sub ecx, N`.
    memory = source.mem
    if first.reg_name(memory.base) != "ecx" or memory.index != 0:
        return []
    second = 0
    if len(head) > 1 and head[1].mnemonic == "sub" and head[1].operands:
        where, what = head[1].operands
        if (where.type == X86_OP_REG and head[1].reg_name(where.reg) == "ecx"
                and what.type == X86_OP_IMM):
            second = what.imm
    return [memory.disp, second]


def read_purges(addresses, rows, exe: Path) -> dict:
    """address -> Purge, read from the executable.

    Imports are local, the way `recover_conventions.read_bodies` does it, so
    that this module keeps importing on a machine with no capstone and no PE
    reader - which is where `--check` and the test suite run.
    """
    import pefile
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs

    from generator_support import read_bytes

    pe = pefile.PE(str(exe), fast_load=True)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    by_address = {row.get("address", ""): row for row in rows}
    # The hop resolves an integer address, so the catalogue is indexed by one
    # too. Every catalogued function is a candidate TARGET, not just the 1,601
    # candidate rows - a thunk may jump anywhere.
    by_number = {}
    for row in rows:
        try:
            by_number[int(row.get("address", ""), 16)] = row
        except ValueError:
            continue

    def spans_of(row):
        # `body_ranges`, not end_address - address. 402 catalogued functions
        # carry a second cold span, and a contiguous read from the entry runs
        # past the body into whatever follows it. `size` is the SUM of the
        # spans; no arithmetic on the endpoints replaces reading them.
        return parse_ranges(row.get("body_ranges", ""))

    def decode(spans):
        out = []
        for start, end in spans:
            out.extend(engine.disasm(read_bytes(pe, start, end - start), start))
        return out

    out = {}
    for address in addresses:
        spans = spans_of(by_address.get(address, {}))
        if not spans:
            out[address] = Purge(None, SOURCE_NONE,
                                 "no body_ranges in the catalogue")
            continue
        instructions = decode(spans)
        # THE EVIDENCE MUST BE REACHABLE. A `ret` the entry cannot get to is
        # not an observation of anything; see `reachable_addresses`.
        reachable = reachable_addresses(instructions, spans[0][0])
        live = [one for one in instructions if one.address in reachable]
        seen = ret_immediates(live)
        stranded = ret_immediates(instructions) - seen
        count = f"{len(spans)} body span(s)"
        if len(seen) == 1:
            out[address] = Purge(next(iter(seen)), SOURCE_BODY,
                                 f"one reachable ret imm over {count}")
        elif seen:
            out[address] = Purge(None, SOURCE_NONE,
                                 f"{len(seen)} distinct reachable ret imm "
                                 f"over {count}")
        elif stranded:
            # There IS a ret, and no path from the entry reaches it. Publishing
            # its number as `verified` is the overstatement this branch exists
            # to refuse - the row keeps its prototype and loses the claim.
            out[address] = Purge(None, SOURCE_NONE,
                                 f"the only ret imm over {count} is not "
                                 f"reachable from the entry")
        else:
            out[address] = _one_hop(address, instructions, spans, count,
                                    by_number, spans_of, decode)
    return out


def _one_hop(address, instructions, spans, count, by_number, spans_of, decode):
    """The evidence a body with no `ret` of its own tail-jumps to. ONE hop.

    Every path out of here that is not a single unambiguous `ret imm` in a
    single catalogued target is UNKNOWN, and says which condition it failed.
    """
    target = tail_jump_target(instructions, spans[-1][1])
    if target is None:
        return Purge(None, SOURCE_NONE,
                     f"no ret and no direct tail jump over {count}")
    lead = f"a direct tail jump over {count} to 0x{target:08X}"

    reason = stack_shift_reason(instructions[:-1])
    if reason is not None:
        return Purge(None, SOURCE_NONE,
                     f"{lead}, but {reason}, so the target's purge is not "
                     f"this body's")
    if target == int(address, 16):
        return Purge(None, SOURCE_NONE, f"{lead}, which is itself")
    row = by_number.get(target)
    if row is None:
        return Purge(None, SOURCE_NONE,
                     f"{lead}, which is not a catalogued function")
    target_spans = spans_of(row)
    if not target_spans:
        return Purge(None, SOURCE_NONE,
                     f"{lead}, which has no body_ranges in the catalogue")

    # The target's `ret` must be reachable from the TARGET's entry, for the same
    # reason the body's must be reachable from its own: a dead epilogue states a
    # number the processor never executes, and hopping to one would launder the
    # overstatement through an extra instruction rather than remove it.
    target_code = decode(target_spans)
    target_live = reachable_addresses(target_code, target_spans[0][0])
    seen = ret_immediates([one for one in target_code
                           if one.address in target_live])
    reached = f"{len(target_spans)} body span(s)"
    if len(seen) == 1:
        return Purge(next(iter(seen)), SOURCE_TAIL_JUMP,
                     f"one reachable ret imm in the tail-jump target "
                     f"0x{target:08X} over {reached}")
    if not seen:
        return Purge(None, SOURCE_NONE,
                     f"{lead}, which reaches no ret of its own; that is a "
                     f"chain, and this gate does not take a second hop")
    return Purge(None, SOURCE_NONE,
                 f"{lead}, which has {len(seen)} distinct ret imm over "
                 f"{reached}")


def render_purges(purges: dict) -> str:
    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(PURGE_FIELDS)
    for address in sorted(purges, key=lambda item: int(item, 16)):
        one = purges[address]
        writer.writerow([address,
                         UNKNOWN if one.observed is None
                         else f"0x{one.observed:X}",
                         one.source, one.evidence])
    return buffer.getvalue()


def load_purges(path: Path = PURGE_CSV) -> dict:
    """The committed machine-code answer, for a machine with no executable.

    A row that carries a NUMBER must carry the source that number came from.
    Defaulting a missing source to `body` is the exact rounding-up this gate
    exists to prevent, so it raises instead - a pre-hop catalogue is stale, not
    a catalogue of direct observations.
    """
    if not path.is_file():
        return {}
    out = {}
    with path.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            value = (row.get("purge") or "").strip()
            source = (row.get("source") or "").strip()
            observed = None if value == UNKNOWN else int(value, 16)
            if observed is not None and source not in (SOURCE_BODY,
                                                       SOURCE_TAIL_JUMP):
                raise ValueError(
                    f"{path}: {row['address']} publishes a callee purge of "
                    f"{value} with no source column saying where it came "
                    f"from ({source!r}). Regenerate it with the executable "
                    f"present; a number whose provenance is unrecorded must "
                    f"not be read as a direct observation of the body.")
            out[row["address"]] = Purge(observed, source or SOURCE_NONE,
                                        row.get("evidence", ""))
    return out


def hop_agreement(candidates, purges: dict) -> tuple[int, int, list]:
    """(agreeing, unscorable, contradictions) over the rows a hop reached.

    The built-in sanity check on one-hop resolution. A method that resolved the
    wrong target would disagree with the published prototype almost everywhere;
    this one agrees with most of what it touches, and the rows it does not
    agree with are the finding.
    """
    agreeing = 0
    unscorable = 0
    contradictions = []
    for one in candidates:
        purge = purges.get(one.address, NO_PURGE)
        if purge.source != SOURCE_TAIL_JUMP or purge.observed is None:
            continue
        verdict = purge_verdict(one, purge)
        if verdict == VERIFIED_VIA_TAIL_JUMP:
            agreeing += 1
        elif verdict == CONTRADICTED:
            contradictions.append((one, purge))
        else:
            unscorable += 1
    return agreeing, unscorable, contradictions


# ---------------------------------------------------------------------------
# The control
# ---------------------------------------------------------------------------
#
# The comparison has to read IDA's recorded prototype, and IDA writes Windows
# typedefs where the mangling writes the underlying type. Each entry here is an
# identity from the Windows SDK, not a knob: collapsing them is what lets the
# control ask "is this the same type" instead of "is this the same word". They
# are used ONLY by the control. Nothing here is ever emitted.
IDA_TYPEDEF = {
    "HWND": "void*", "HDC": "void*", "HINSTANCE": "void*", "HMODULE": "void*",
    "HANDLE": "void*", "HPALETTE": "void*", "HBITMAP": "void*",
    "LPVOID": "void*", "PVOID": "void*",
    "LPSTR": "int8*", "LPCSTR": "int8*", "LPBYTE": "unsigned int8*",
    "LPARAM": "int", "LRESULT": "int", "LONG": "int", "BOOL": "int",
    "WPARAM": "unsigned int", "UINT": "unsigned int", "DWORD": "unsigned int",
    "UINT_PTR": "unsigned int", "uint32_t": "unsigned int",
    "unsigned int32": "unsigned int", "int32_t": "int", "_DWORD": "unsigned int",
    "WORD": "unsigned int16", "CHAR": "int8", "_BYTE": "unsigned int8",
    # IDA's rendering of a `double` it declined to name; the one instance is
    # ?adjust_pal@Gamma@@QAEXN@Z, whose mangling says N (double).
    "unknown float": "double",
}

RECORDED = re.compile(r"^(?P<ret>.+?)\s*\(\s*(?P<conv>__\w+)\s+"
                      r"(?P<name>[^)]*)\)\s*\((?P<params>.*)\)$")


def collapse_typedef(text: str) -> str:
    base = text.rstrip("*")
    stars = len(text) - len(base)
    expanded = IDA_TYPEDEF.get(base.strip())
    if expanded is None:
        return text
    return expanded + "*" * stars if stars else expanded


def strip_parameter_name(text: str) -> str:
    """`int8* sectionID` -> `int8*`. A function-pointer parameter is left whole.

    1,551 of the recorded parameters carry a name and 6,345 do not; the name is
    information the mangling never had, so it plays no part in the comparison.

    A function-pointer parameter is recursed into, because IDA names the inner
    parameters too - it writes `void (__cdecl *)(int this)` where the mangling
    can only say `void (__cdecl *)(int)`. That literal `this` is IDA's naming of
    an inner argument, not a receiver, and 9 rows carry it.
    """
    text = text.strip()
    if "(" in text:
        inner = text.rfind("(")
        if inner >= 0 and text.endswith(")"):
            args = text[inner + 1:-1].strip()
            parts = [] if args in ("", "void") else [
                strip_parameter_name(part)
                for part in conventions.split_args(args)]
            return text[:inner] + "(" + ", ".join(parts) + ")"
        return text
    found = re.match(r"^(?P<type>.*?[\w*])\s+(?P<name>[A-Za-z_]\w*)$", text)
    if not found:
        return text
    kept = found.group("type").strip()
    tail = kept.rsplit(" ", 1)[-1] if " " in kept else kept
    if tail in ("unsigned", "signed", "const", "struct", "class", "enum",
                "long", "short", "unknown"):
        return text
    return kept


def comparable(text: str) -> str:
    return collapse_typedef(strip_parameter_name(text))


class Control:
    """Agreement between what a name states and what IDA recorded."""

    def __init__(self, population_floor: int = CONTROL_POPULATION_FLOOR):
        self.agree = {key: 0 for key in AGREEMENT_FLOOR}
        self.population = 0
        self.disagreements = {key: [] for key in AGREEMENT_FLOOR}
        # Injectable so a unit test can exercise the RATIO logic on a synthetic
        # population without tripping the catalogue-sized pin. The real run
        # always takes the default; a test that wants the pin sets it.
        self.population_floor = population_floor

    def rate(self, key: str) -> float:
        if not self.population:
            return 0.0
        return self.agree[key] / self.population

    def failures(self) -> list[str]:
        out = []
        # THE POPULATION IS A FLOOR TOO, and leaving it out was a real hole:
        # every check below is a RATIO, so rows silently vanishing from the
        # control keeps all five green while the evidence behind them shrinks.
        # Measured: dropping 10% of the control leaves 3213 -> 2892 with every
        # rate unchanged and nothing said. A ratio cannot notice its own
        # denominator falling.
        if self.population < self.population_floor:
            out.append(f"control population {self.population}, below the pinned "
                       f"{self.population_floor}: rows stopped reaching the "
                       f"control, and every rate below is measured on less "
                       f"evidence than when it was pinned")
        for key, (agreeing, sample) in AGREEMENT_FLOOR.items():
            floor = agreeing / sample
            if self.rate(key) < floor - 1e-12:
                out.append(f"{key}: {self.agree[key]}/{self.population} "
                           f"= {100 * self.rate(key):.2f}%, below the pinned "
                           f"{100 * floor:.2f}%")
        return out


def control(rows, demangle) -> Control:
    """Derive from the name alone on the rows that ALSO carry a recording."""
    result = Control()
    for row in rows:
        recorded = row.get("prototype") or ""
        name = row.get("name") or ""
        if not recorded or not name.startswith("?"):
            continue
        shape = RECORDED.match(recorded)
        if shape is None:
            continue
        derived = derive_one(row.get("address", ""), name, demangle(name) or "")
        if derived is None:
            continue
        result.population += 1

        _score(result, derived, shape, row)
    return result


def _score(result: Control, derived: Derived, shape, row) -> None:
    def note(key, ok, detail):
        if ok:
            result.agree[key] += 1
        else:
            result.disagreements[key].append(
                (row.get("address", ""), derived.name, detail))

    note("convention", derived.convention == shape.group("conv"),
         f"name says {derived.convention}, IDA says {shape.group('conv')}")

    recorded_params = conventions.split_args(shape.group("params"))
    has_receiver = bool(recorded_params) and recorded_params[0].endswith(" this")
    note("receiver", has_receiver == derived.takes_this,
         f"name says receiver={derived.takes_this}, IDA says {has_receiver}")

    declared = recorded_params[1:] if has_receiver else list(recorded_params)
    mine = [comparable(one) for one in derived.argument_types]
    theirs = [comparable(one) for one in declared]
    note("stack layout",
         stack_slots(mine) == stack_slots(theirs),
         f"name says {stack_slots(mine)}, IDA says {stack_slots(theirs)}")
    note("argument types", mine == theirs,
         f"name says {mine}, IDA says {theirs}")
    note("return type",
         comparable(derived.return_type) == comparable(shape.group("ret")),
         f"name says {derived.return_type}, IDA says {shape.group('ret')}")


# ---------------------------------------------------------------------------
# Deriving, and the weak second source
# ---------------------------------------------------------------------------

def derive(rows, demangle, purges=None) -> tuple[list[Derived], dict]:
    """Prototypes for the rows that have a mangled name and NO recording.

    `purges` is address -> `Purge`, from `read_purges` or `load_purges`. A row
    the bytes CONTRADICT is dropped and counted under its own reason - and the
    two contradiction reasons are kept apart, because a refusal backed by a hop
    rests on more indirection than one backed by the row's own body. A row the
    bytes cannot speak to is published carrying `unknown`. Passing no table at
    all is the honest no-evidence case and marks every row `unknown` - `main`
    refuses to publish in that state.
    """
    found = []
    refused = {"not a function": 0, "already recorded": 0, "unmangled": 0,
               PURGE_REFUSAL: 0, PURGE_REFUSAL_HOP: 0}
    for row in rows:
        name = row.get("name") or ""
        if not name.startswith("?"):
            refused["unmangled"] += 1
            continue
        if row.get("prototype"):
            # Never overwrite a recording: IDA's carries parameter names and
            # typedefs the mangling does not have.
            refused["already recorded"] += 1
            continue
        derived = derive_one(row.get("address", ""), name, demangle(name) or "")
        if derived is None:
            refused["not a function"] += 1
            continue
        purge = (purges or {}).get(derived.address, NO_PURGE)
        verdict = purge_verdict(derived, purge)
        if verdict == CONTRADICTED:
            refused[PURGE_REFUSAL_HOP if purge.source == SOURCE_TAIL_JUMP
                    else PURGE_REFUSAL] += 1
            continue
        derived.purge = verdict
        found.append(derived)
    return found, refused


def emitted_control(found: list[Derived], demangle) -> list[str]:
    """Re-check every row that is about to be PUBLISHED, and name any failure.

    The agreement control above measures the rows that already have a recorded
    prototype - which is, by construction, not the population being emitted.
    This one covers the emitted rows themselves by re-reading each derived
    prototype from a second direction:

      * the convention and the argument count must match what the demangler
        prints for the same name, independently of `split_infix`;
      * the rendered prototype must parse under the shape every consumer uses.

    Zero tolerance, because these rows are the output. Measured 2026-07-31:
    0 failures over the 1,553 rows being published.
    """
    problems = []
    for one in found:
        demangled = demangle(one.name) or ""
        found_conv = _top_level_convention(demangled)
        if found_conv is None or found_conv.group(1) != one.convention:
            problems.append(
                f"{one.address} {one.name}: convention {one.convention} is not "
                f"what the demangler prints ({demangled})")
            continue
        argument_text = conventions.demangled_args(demangled)
        arguments = conventions.split_args(argument_text or "")
        if arguments == ["void"]:
            arguments = []
        if len(arguments) != len(one.argument_types):
            problems.append(
                f"{one.address} {one.name}: {len(one.argument_types)} argument"
                f"(s) rendered, demangler prints {len(arguments)}")
        if not RECORDED.match(one.prototype):
            problems.append(
                f"{one.address} {one.name}: rendered prototype does not parse "
                f"under the shape the catalogue uses: {one.prototype}")
    return problems


IDA_ARGUMENTS = re.compile(r"\((?P<params>.*)\)\s*$")


def hypotheses(rows, ida_rows) -> list[tuple[str, str, int, str]]:
    """Argument COUNTS for the rows with no mangled name. NOT prototypes.

    IDA's argument count agrees with the mangled population 94.1% of the time,
    which is worth publishing; its calling convention agrees 30.3% of the time,
    which is not, so no convention is carried here and no prototype string is
    built. A consumer that wants a prototype must not read this file.
    """
    by_address = {}
    for row in ida_rows:
        try:
            by_address[int(row["address"], 16)] = row
        except (KeyError, ValueError):
            continue
    out = []
    for row in rows:
        name = row.get("name") or ""
        if name.startswith("?") or row.get("prototype"):
            continue
        try:
            address = int(row.get("address", ""), 16)
        except ValueError:
            continue
        guess = by_address.get(address)
        if guess is None or not guess.get("type"):
            continue
        shape = IDA_ARGUMENTS.search(guess["type"])
        if shape is None:
            continue
        text = shape.group("params").strip()
        parts = [] if text in ("", "void") else conventions.split_args(text)
        if any(one.strip() == "..." for one in parts):
            continue
        out.append((row["address"], name, len(parts),
                    f"IDA 9.4 guess_type {guess['type']}"))
    return sorted(out, key=lambda item: int(item[0], 16))


# ---------------------------------------------------------------------------
# Files
# ---------------------------------------------------------------------------

def render(found: list[Derived]) -> str:
    buffer = io.StringIO()
    # LF, because every committed catalogue under docs/recovery is LF and csv
    # defaults to CRLF.
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(FIELDS)
    for one in sorted(found, key=lambda item: int(item.address, 16)):
        writer.writerow([one.address, one.name, one.prototype,
                         one.convention, one.slots, one.purge])
    return buffer.getvalue()


def render_hypotheses(guesses) -> str:
    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(HYPOTHESIS_FIELDS)
    for address, name, count, evidence in guesses:
        writer.writerow([address, name, count, evidence])
    return buffer.getvalue()


def write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as handle:
        handle.write(text)


def load(path: Path = PROTOTYPES_CSV) -> dict:
    """address -> prototype, for a consumer. Absent file is not an error."""
    if not path.is_file():
        return {}
    with path.open(newline="", encoding="utf-8-sig") as handle:
        return {row["address"]: row["prototype"]
                for row in csv.DictReader(handle)
                if row.get("address") and row.get("prototype")}


def load_rows(path: Path = None) -> list[dict]:
    """Every catalogued row, from `src/`. See measure_exclusions.load_rows."""
    if path is not None:
        with path.open(newline="", encoding="utf-8-sig") as handle:
            return list(csv.DictReader(handle))
    import sys as _sys
    _sys.path.insert(0, str(Path(__file__).resolve().parent))
    import emit_translation_unit as _emit
    return list(_emit.load_functions().values())


def load_ida(path: Path = IDA_CSV) -> list[dict]:
    if not path.is_file():
        return []
    with path.open(newline="", encoding="utf-8-sig") as handle:
        return list(csv.DictReader(handle))


def _current(path: Path) -> str:
    # newline="" so a CRLF file reads as CRLF and --check can SEE the drift.
    # Reading with read_text() would translate it away, which has already let a
    # CRLF catalogue pass --check elsewhere in this tree.
    if not path.is_file():
        return ""
    with path.open(newline="", encoding="utf-8") as handle:
        return handle.read()


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--functions", type=Path, default=None)
    parser.add_argument("--ida", type=Path, default=IDA_CSV)
    parser.add_argument("--out", type=Path, default=PROTOTYPES_CSV)
    parser.add_argument("--hypotheses", type=Path, default=HYPOTHESES_CSV)
    parser.add_argument("--purges", type=Path, default=PURGE_CSV)
    parser.add_argument("--exe", type=Path, default=conventions.DEFAULT_EXE,
                        help="the hash-pinned executable. Absent, the gate "
                             "runs from the committed callee-purge catalogue.")
    parser.add_argument("--check", action="store_true",
                        help="fail if any committed file would change")
    parser.add_argument("--disagreements", action="store_true",
                        help="print every row where the name and IDA differ")
    args = parser.parse_args(argv)

    try:
        import undname
    except ImportError:
        print("REFUSED: undname is not installed (tools/requirements.txt). "
              "The derivation cannot run here; the committed CSV is the "
              "answer for consumers.", file=sys.stderr)
        return 2

    rows = load_rows(args.functions)
    measured = control(rows, undname.undname)
    print(f"control on the {measured.population} rows that carry BOTH a "
          f"mangled name and an IDA prototype:")
    for key in AGREEMENT_FLOOR:
        agreeing, sample = AGREEMENT_FLOOR[key]
        print(f"  {key:<18} {measured.agree[key]:5d} / {measured.population}"
              f"  = {100 * measured.rate(key):6.2f}%   "
              f"(pinned floor {100 * agreeing / sample:.2f}%)")

    if args.disagreements:
        for key in AGREEMENT_FLOOR:
            rows_out = measured.disagreements[key]
            print(f"\n{key}: {len(rows_out)} disagreement(s)")
            for address, name, detail in rows_out:
                print(f"  {address} {name}\n      {detail}")

    failures = measured.failures()
    if failures:
        for line in failures:
            print(f"  {line}", file=sys.stderr)
        print("REFUSED: a derived prototype now disagrees with a recorded one "
              "more often than when this was measured. A wrong prototype is "
              "worse than a missing one, because every consumer reads the "
              "column as certain.", file=sys.stderr)
        return 1

    # The machine-code gate. `derive` with no purge table names the candidate
    # set; the bytes then decide which of those may be published.
    candidates, _ = derive(rows, undname.undname)
    addresses = [one.address for one in candidates]
    if args.exe.is_file():
        purges = read_purges(addresses, rows, args.exe)
        print(f"callee purge read from {args.exe.name} for "
              f"{len(purges)} candidate(s)")
    else:
        purges = load_purges(args.purges)
        print(f"no executable at {args.exe}; callee purge read from the "
              f"committed {args.purges.name} ({len(purges)} row(s))")

    unevidenced = [one for one in addresses if one not in purges]
    if unevidenced:
        print(f"REFUSED: {len(unevidenced)} candidate(s) have no machine-code "
              f"evidence at all, starting {unevidenced[:3]}. The gate would "
              f"pass them through unchecked, which is exactly the state this "
              f"gate exists to end. Regenerate {args.purges} with the "
              f"executable present.", file=sys.stderr)
        return 2

    # The built-in sanity check on the hop, printed BEFORE anything is written
    # and gating the write. One-hop resolution is only a method if it mostly
    # CONFIRMS; a run that refused hundreds of rows would be resolving the
    # wrong target, and refusing to ship is the right answer to that.
    agreeing, unscorable, contradictions = hop_agreement(candidates, purges)
    hopped = agreeing + unscorable + len(contradictions)
    floor_agreeing, floor_sample = HOP_AGREEMENT_FLOOR
    floor = floor_agreeing / floor_sample
    rate = agreeing / hopped if hopped else 0.0
    print(f"one-hop tail-jump resolution reached {hopped} row(s) that a linear "
          f"sweep of the body could not:")
    print(f"  {agreeing} AGREE with the published prototype's implied purge, "
          f"{len(contradictions)} CONTRADICT it, {unscorable} unscorable"
          f"   ({100 * rate:.2f}%, pinned floor {100 * floor:.2f}%)")
    if rate < floor - 1e-12:
        print("REFUSED: agreement across the hopped rows has collapsed. "
              "One-hop resolution confirmed the published prototype on most "
              "of the rows it reached when this was measured; a run that "
              "instead contradicts them is resolving the wrong thing, and "
              "the right answer is to not ship the hop - NOT to refuse "
              "hundreds of rows on it.", file=sys.stderr)
        return 1

    found, refused = derive(rows, undname.undname, purges)
    if len(found) < PUBLISHED_ROWS_FLOOR:
        print(f"REFUSED: {len(found)} row(s) derived, below the pinned "
              f"{PUBLISHED_ROWS_FLOOR}. Every refusal this generator makes is "
              f"counted and named, so a legitimate drop can always be explained "
              f"- an unexplained one is coverage lost without a decision.",
              file=sys.stderr)
        return 1
    # The thunk spellings assert a displacement as well as a signature, and
    # only the signature has been checked up to here.
    thunks = sum(1 for one in found if is_thunk_name(one.name))
    # Only the image can answer this one. With no executable the run already
    # falls back to the committed purge catalogue, and that file records a
    # purge, not a displacement - so this says it is UNCHECKED rather than
    # printing a clean count nobody measured.
    off = []
    if args.exe.is_file():
        off = thunk_adjustment_disagreements(found, rows, args.exe)
        print(f"{thunks} thunk name(s) held to the displacement their body "
              f"applies: {len(off)} disagreement(s)")
    else:
        print(f"{thunks} thunk name(s) NOT checked against their displacement: "
              f"that needs the executable, and this run had none")
    if off:
        for address, name, why in off:
            print(f"  ADJUSTMENT {address} {name[:56]}\n          {why}",
                  file=sys.stderr)
        print("REFUSED: a thunk name states an adjustment its body does not "
              "apply. The signature half of these names is settled by the "
              "callee purge; this is the half nothing else can falsify, so a "
              "disagreement here is a wrong name, not a tolerance.",
              file=sys.stderr)
        return 1
    print(f"{len(found)} prototype(s) derived from the name alone")
    named = ", ".join(f"{count} {reason}"
                      for reason, count in sorted(refused.items()) if count)
    print(f"  refused: {named}")
    split = {}
    for one in found:
        split[one.purge] = split.get(one.purge, 0) + 1
    for word in (VERIFIED, VERIFIED_VIA_TAIL_JUMP, UNKNOWN):
        print(f"  purge {word:<24} {split.get(word, 0):5d}")
    print(f"  ({VERIFIED} is a `ret imm` in the row's OWN body; "
          f"{VERIFIED_VIA_TAIL_JUMP} is one in the body it tail-jumps to, one "
          f"hop away; {UNKNOWN} is absence of evidence, never agreement)")
    for one in candidates:
        purge = purges.get(one.address, NO_PURGE)
        if purge_verdict(one, purge) == CONTRADICTED:
            print(f"  REFUSED {one.address} {one.name[:62]}\n"
                  f"          prototype implies a callee purge of "
                  f"{implied_purge(one)}, the {purge.source} executes "
                  f"{purge.observed}")

    problems = emitted_control(found, undname.undname)
    print(f"second control, on the {len(found)} rows being published: "
          f"{len(problems)} failure(s)")
    if problems:
        for line in problems[:20]:
            print(f"  {line}", file=sys.stderr)
        print("REFUSED: a row that would be published does not survive being "
              "re-read from the demangler.", file=sys.stderr)
        return 1

    guesses = hypotheses(rows, load_ida(args.ida))
    print(f"{len(guesses)} argument-count hypothesis(es) from IDA, "
          f"published separately and WITHOUT a convention")

    certain = render(found)
    weak = render_hypotheses(guesses)
    # The purge catalogue is only re-rendered where the bytes were actually
    # read. On a machine with no executable it is the INPUT, and comparing it
    # against itself would be the vacuous check this repository has published
    # before.
    products = [(args.out, certain), (args.hypotheses, weak)]
    if args.exe.is_file():
        products.append((args.purges, render_purges(purges)))

    if args.check:
        stale = [path for path, text in products if _current(path) != text]
        if stale:
            for path in stale:
                print(f"error: {path} is stale; regenerate it", file=sys.stderr)
            return 1
        for path, _text in products:
            print(f"{path} is current")
        return 0

    for path, text in products:
        write(path, text)
        print(f"wrote {path}")
    print(f"{len(found)} prototype row(s), {len(guesses)} hypothesis row(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
