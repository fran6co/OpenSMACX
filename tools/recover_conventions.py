#!/usr/bin/env python3
"""Recover each catalogued function's CALLING CONVENTION from its name, and
check that answer against the machine code before anything is allowed to use it.

WHY THIS EXISTS, AND WHAT IT TURNED OUT NOT TO BE
-------------------------------------------------
The catalogue holds 4,821 MSVC-mangled names. A mangled name encodes the return
type, the argument list AND the calling convention, so it is the only place in
this repository where the ABI of a function is written down mechanically rather
than guessed. Nothing was reading it.

The suspicion that prompted this tool was that the lifter and the differential
oracle assume "member function implies `this` in ECX", and that the ~139
non-static members whose names say `__cdecl` or `__stdcall` are therefore
mis-seeded. That suspicion is FALSE, and it is worth writing down why, because
the measurement is cheap to repeat and the conclusion is not obvious:

  * `tools/lift_whole_image.py` and `tools/x86_lower.py` contain no convention
    logic at all - grep them for `thiscall`, `stdcall`, `cdecl` and `fastcall`
    and every one comes back empty. The lift is explicit-CPU-state: it moves
    the machine's registers and memory, and a convention is a statement about
    which of those a caller may rely on, which the lift never needs.

  * the oracle's seeding is `make_profile(address, case_index)`. It never reads
    a name, a prototype or a convention, and both sides of the differential are
    handed the IDENTICAL seeded state. A wrong convention therefore cannot
    produce a wrong verdict; it can only waste cases.

So this tool does NOT correct a wrong assumption. What it does is supply
information the oracle never had, and the measured gap it closes is a different
one, described under "the pointer-argument gap" below.

WHAT THE NAME SAYS
------------------
After the `@@` that closes the qualified name, MSVC writes:

    free / namespace-scope   [Y|Z] [conv] ...
    static member            [C|D|K|L|S|T] [conv] ...
    instance / virtual       [A|B|I|J|Q|R|E|F|M|N|U|V] [cv] [conv] ...

The cv slot on an instance member is the qualifier on `this`, and reading it as
the convention is the classic mis-parse: it turns every `QAE` (public instance
__thiscall) into `A` (__cdecl) and reports that 2,905 of 4,026 functions have
the wrong convention. This module parses the slot correctly and, in
`--cross-check`, verifies the answer against `undname` on every row rather than
trusting either alone.

WHAT THE BODY SAYS, AND WHY IT WINS
-----------------------------------
MSVC 32-bit callee-pop:

    __cdecl     ret 0             (caller pops; always a bare `ret`)
    __stdcall   ret 4*(args + this-if-any)
    __thiscall  ret 4*args        (`this` rides in ECX and is never popped)

`ret N` alone cannot separate `__thiscall` with no arguments from `__cdecl` with
no arguments - both emit a bare `ret`. The second signal is whether ECX is READ
before it is WRITTEN on the entry path, which is what taking `this` in ECX looks
like. Neither signal is sufficient alone and this module reports a convention
only when they agree, or flags the row when they do not.

Where the name and the body disagree, the BODY wins, and the disagreement is
itself the finding: it means the catalogue's name is attached to the wrong
address, or its declared argument list is wrong.

THE POINTER-ARGUMENT GAP
------------------------
Measured over the 1,041 of the 1,465 PASS rows in `build/oracle/report.tsv`
whose catalogue name yields an argument list at all (the other 424 are `??__E`
initialisers and rows with no function infix). "Declares a pointer" means
`stack_slots_for("none")` - the DECLARED arguments, with no `this` slot in
front, because a `this` that travels in ECX is already guaranteed and counting
it here would move 31 rows into the wrong column and flatten the gap:

    PASS, name declares a pointer argument   n=143  mean compared= 7.74
                                                    median 7   25.9% have <=2
    PASS, name declares no pointer argument  n=898  mean compared=12.71
                                                    median 16   7.7% have <=2

An earlier version of this block printed n=139/n=902 and means of 7.76/12.68.
Those digits do not reproduce from these inputs under any definition of
"declares a pointer" that was tried, and the 139 in them is a different set
from the 139 above, which is the likeliest way the two got crossed. The 1.6x
gap and the ~3.4x thin-evidence rate are what the paragraph below rests on, and
those hold.

The oracle GUARANTEES a usable object pointer in ECX - all four legacy cases
set it and the drawn cases set it three times in four - so a `__thiscall`
function gets a valid `this` in about thirteen of sixteen cases. Nothing
equivalent is guaranteed for a pointer that arrives on the STACK: those words
are drawn against `argument_pointer_weight`, itself drawn uniformly per case, so
a function taking `(RECT *, int *)` may not get a usable pointer in any case at
all. That asymmetry, not the convention, is what makes evidence thin.

The `--seed-table` output exists to close it: for each address it emits the mask
of argument slots the NAME says are pointers, so the oracle can guarantee those
slots the same way it already guarantees ECX. Slot 0 is the first stack dword,
which is `this` when `this` travels on the stack.

Usage:
    recover_conventions.py --report            # what the names say, with checks
    recover_conventions.py --cross-check       # infix parse vs undname, per row
    recover_conventions.py --disagreements     # name vs body, body wins
    recover_conventions.py --seed-table OUT    # address -> pointer-slot mask
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from collections import Counter
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

# ---------------------------------------------------------------------------
# The mangling infix
# ---------------------------------------------------------------------------

# access/kind char -> (access, kind). `kind` is what decides whether there is an
# implicit `this` at all, which is the only part the seeding cares about.
KIND: dict[str, tuple[str, str]] = {}
for _pair, _access in ((("A", "B"), "private"), (("I", "J"), "protected"),
                       (("Q", "R"), "public")):
    for _c in _pair:
        KIND[_c] = (_access, "instance")
for _pair, _access in ((("C", "D"), "private"), (("K", "L"), "protected"),
                       (("S", "T"), "public")):
    for _c in _pair:
        KIND[_c] = (_access, "static")
for _pair, _access in ((("E", "F"), "private"), (("M", "N"), "protected"),
                       (("U", "V"), "public")):
    for _c in _pair:
        KIND[_c] = (_access, "virtual")
for _pair, _access in ((("G", "H"), "private"), (("O", "P"), "protected"),
                       (("W", "X"), "public")):
    for _c in _pair:
        KIND[_c] = (_access, "thunk")
KIND["Y"] = ("none", "free")
KIND["Z"] = ("none", "free")

CONV = {"A": "__cdecl", "B": "__cdecl", "C": "__pascal", "D": "__pascal",
        "E": "__thiscall", "F": "__thiscall", "G": "__stdcall",
        "H": "__stdcall", "I": "__fastcall", "J": "__fastcall"}

# The cv-qualifier slot that only an instance/virtual/thunk member carries.
CV_SLOT = set("ABCD")

# Kinds that take an implicit `this`.
THIS_KINDS = {"instance", "virtual", "thunk"}


def split_infix(name: str) -> tuple[str, str] | None:
    """(kind_char, convention_char) for a mangled function name, else None.

    Returns None for anything that is not a function: data decorations,
    vftables, RTTI records and the `??__E` dynamic initialisers all fail the
    slot check rather than being special-cased by prefix.
    """
    if not name.startswith("?"):
        return None
    marker = name.find("@@")
    if marker < 0:
        return None
    rest = name[marker + 2:]
    if len(rest) < 2:
        return None
    kind_char = rest[0]
    if kind_char not in KIND:
        return None
    _access, kind = KIND[kind_char]
    if kind in THIS_KINDS:
        # [kind][cv][conv]: the cv slot qualifies `this`.
        if rest[1] not in CV_SLOT or len(rest) < 3:
            return None
        conv_char = rest[2]
    else:
        # free and static: no `this`, so no cv slot.
        conv_char = rest[1]
    if conv_char not in CONV:
        return None
    return kind_char, conv_char


# ---------------------------------------------------------------------------
# The declared argument list
# ---------------------------------------------------------------------------

def demangled_args(demangled: str) -> str | None:
    """The text between the argument parentheses of a demangled signature.

    Scans with a depth counter from the FIRST `(` so that a function-pointer
    parameter - `void (__thiscall *)(void *)` - does not end the list early.
    """
    start = demangled.find("(")
    if start < 0:
        return None
    depth = 0
    for index in range(start, len(demangled)):
        if demangled[index] == "(":
            depth += 1
        elif demangled[index] == ")":
            depth -= 1
            if depth == 0:
                return demangled[start + 1:index]
    return None


def split_args(text: str) -> list[str]:
    """Split an argument list on commas that are not nested."""
    out: list[str] = []
    depth = 0
    current = ""
    for char in text:
        if char in "(<[":
            depth += 1
        elif char in ")>]":
            depth -= 1
        if char == "," and depth == 0:
            out.append(current.strip())
            current = ""
        else:
            current += char
    if current.strip():
        out.append(current.strip())
    return out


SCALAR8 = re.compile(r"\b(double|__int64|long long)\b")
SCALAR4 = re.compile(r"\b(char|short|int|long|float|bool|unsigned|signed|"
                     r"void|wchar_t|__int32|__int16|__int8|enum)\b")


def arg_slots(args: list[str]) -> list[bool] | None:
    """One entry per stack dword: True when that dword is a pointer.

    None when the list cannot be laid out - varargs (the caller decides), or a
    class passed by value (width unknown without the class layout). Refusing is
    the point: a guessed width shifts every later slot.
    """
    if len(args) == 1 and args[0] == "void":
        return []
    slots: list[bool] = []
    for arg in args:
        arg = arg.strip()
        if arg == "..." or not arg:
            return None
        if "*" in arg or "&" in arg:
            slots.append(True)
        elif SCALAR8.search(arg):
            slots.extend((False, False))
        elif SCALAR4.search(arg):
            slots.append(False)
        else:
            return None
    return slots


# ---------------------------------------------------------------------------
# The recovered signature
# ---------------------------------------------------------------------------

class Signature:
    """What the NAME says about how a function is called."""

    def __init__(self, address: str, name: str, demangled: str,
                 kind: str, convention: str, slots: list[bool] | None):
        self.address = address
        self.name = name
        self.demangled = demangled
        self.kind = kind
        self.convention = convention
        self.slots = slots

    @property
    def takes_this(self) -> bool:
        return self.kind in THIS_KINDS

    @property
    def this_location(self) -> str:
        """Where the NAME says `this` arrives: 'ecx', 'stack', or 'none'.

        This is C++ semantics applied to the access char, and in this image the
        access char is not trustworthy on its own - see `resolve_this_location`,
        which consults the machine code and wins.
        """
        if not self.takes_this:
            return "none"
        return "ecx" if self.convention == "__thiscall" else "stack"

    def stack_slots_for(self, location: str) -> list[bool] | None:
        """Pointer-ness of each stack dword under a given `this` location.

        This is the layout the oracle seeds: slot 0 is the dword at [ESP+4] on
        entry. A `this` that travels on the stack occupies slot 0 and is a
        pointer; a `this` in ECX does not appear here at all.
        """
        if self.slots is None or location == "unresolved":
            return None
        if location == "stack":
            return [True] + self.slots
        return list(self.slots)

    def pointer_mask_for(self, location: str) -> int:
        """Bit i set when stack slot i is a declared pointer."""
        slots = self.stack_slots_for(location)
        if slots is None:
            return 0
        mask = 0
        for index, is_pointer in enumerate(slots):
            if is_pointer:
                mask |= 1 << index
        return mask

    @property
    def stack_slots(self) -> list[bool] | None:
        """The layout the NAME implies. `stack_slots_for` is the one to use
        once the body has been consulted."""
        return self.stack_slots_for(self.this_location)

    @property
    def pointer_mask(self) -> int:
        return self.pointer_mask_for(self.this_location)

    @property
    def callee_pop_bytes(self) -> int | None:
        """Bytes the callee pops on return, or None when undecidable."""
        if self.slots is None:
            return None
        if self.convention == "__cdecl":
            return 0
        if self.convention == "__thiscall":
            return 4 * len(self.slots)
        if self.convention == "__stdcall":
            slots = self.stack_slots
            return 4 * len(slots) if slots is not None else None
        return None


def load_rows(functions_csv: Path = FUNCTIONS_CSV) -> list[dict[str, str]]:
    with functions_csv.open() as handle:
        return list(csv.DictReader(handle))


def recover(rows, demangle) -> tuple[list[Signature], Counter]:
    """Signatures for every row that carries a real function name."""
    out: list[Signature] = []
    skipped: Counter = Counter()
    for row in rows:
        name = row["name"]
        infix = split_infix(name)
        if infix is None:
            skipped["no function infix"] += 1
            continue
        if name.startswith("??__E") or name.startswith("??__F"):
            skipped["dynamic initialiser"] += 1
            continue
        demangled = demangle(name) or ""
        argtext = demangled_args(demangled)
        slots = None if argtext is None else arg_slots(split_args(argtext))
        kind_char, conv_char = infix
        out.append(Signature(row["address"], name, demangled,
                             KIND[kind_char][1], CONV[conv_char], slots))
    return out, skipped


# ---------------------------------------------------------------------------
# What the BODY says
# ---------------------------------------------------------------------------

class BodyEvidence:
    def __init__(self, ret_imms: list[int], ecx_read_first: bool,
                 instructions: int):
        self.ret_imms = ret_imms
        self.ecx_read_first = ecx_read_first
        self.instructions = instructions

    @property
    def callee_pop_bytes(self) -> int | None:
        """The single callee-pop value, or None when the body cannot say."""
        if len(self.ret_imms) != 1:
            return None
        return self.ret_imms[0]


def body_conventions(evidence: BodyEvidence) -> set[str]:
    """Every convention consistent with this body.

    A bare `ret` admits `__cdecl`, a zero-argument `__stdcall` and a
    zero-argument `__thiscall`; the ECX signal is what narrows it.
    """
    pop = evidence.callee_pop_bytes
    if pop is None:
        return set()
    if pop == 0:
        allowed = {"__cdecl", "__stdcall"}
        if evidence.ecx_read_first:
            allowed.add("__thiscall")
        return allowed
    allowed = {"__stdcall"}
    if evidence.ecx_read_first:
        allowed.add("__thiscall")
    else:
        # A callee-pop body that never reads ECX is still a legal __thiscall -
        # a member that ignores `this` - so this is not an exclusion, only a
        # weaker preference recorded by `agreement` below.
        allowed.add("__thiscall")
    return allowed


def resolve_this_location(signature: Signature,
                          evidence: BodyEvidence | None) -> str:
    """Where `this` REALLY arrives, body first. 'ecx', 'stack', 'none' or
    'unresolved'.

    THE MEASUREMENT THIS ENCODES. 139 catalogue names are `QAA` or `QAG` -
    public, INSTANCE, and not `__thiscall` - which by C++ semantics means `this`
    is pushed as a hidden first stack argument. Three independent checks say it
    is not there:

      * callee-pop. THIRTEEN of the 14 `QAG` (__stdcall) bodies pop exactly
        4*args, four bytes SHORT of the 4*(args+1) a pushed `this` would
        require. `?window_proc@Win@@QAGJPAXIIJ@Z` declares four arguments plus
        `this` and pops 16, not 20 - it is a WndProc, and a WndProc is static.
        The fourteenth, `?adjust_menus@Win@@QAGHPAX@Z` at 0x005F0540, declares
        one argument and pops 8, which IS the pushed-`this` shape - so this
        check is not unanimous and the code below refuses that row rather than
        calling it a member. Its body reads [ESP+4] and hands it to an import
        as a window handle: slot 0 is the first real argument and the name
        under-declares the arity, which this catalogue does seven other times.
      * call sites. `--call-sites` scans .text for `call rel32` landing on any
        of the 139: 71 of them are reached that way, over 390 sites, and 63
        have at least one site where the pushes immediately before the call can
        be counted. 287 sites push exactly the declared argument count and
        ZERO sites anywhere push declared+1. `?flip@Win@@QAAXPAURECT@@@Z`
        declares one argument and has 40 call sites, not the three an earlier
        hand-count of this claim reported; that count came from an ad-hoc
        artifact, which is why the scan now lives in this file.
      * the repository's own headers already declare them static -
        `static HDC Win::get_hdc()` in src/win.h,
        `static void BasePop::set_def_string_color(int,int,int,int)` in
        src/basepop.h, `static int __cdecl Time::init_class()` in src/time.h.

    So the access char in these names is simply wrong - `Q` (public instance)
    written where the original had `S` (public static) - and there is no `this`
    to place anywhere. Trusting the access char would put a phantom pointer in
    stack slot 0 and shift every real argument up by one.

    A `__stdcall` body settles itself, because the two layouts pop different
    byte counts. A `__cdecl` body cannot: the caller pops either way. That one
    returns 'unresolved' rather than a guess, and the seed table omits it.
    """
    if not signature.takes_this:
        return "none"
    if signature.convention == "__thiscall":
        return "ecx"
    if signature.slots is None:
        return "unresolved"
    if signature.convention == "__stdcall":
        if evidence is None:
            return "unresolved"
        observed = evidence.callee_pop_bytes
        if observed is None:
            return "unresolved"
        if observed == 4 * len(signature.slots):
            return "none"
        # Popping one word MORE than the declared arguments looks like a pushed
        # `this` and is not evidence of one, because it is equally explained by
        # a static function with one argument the name failed to declare - and
        # this catalogue has seven proven cases of exactly that error
        # (`?on_redraw@Win@@QAEHXZ` declares none and pops 8).
        #
        # The one row that reached here, `?adjust_menus@Win@@QAGHPAX@Z`, is the
        # second reading: it declares one argument, pops 8, and its body reads
        # [ESP+4] and hands it straight to an import as a window handle. A
        # `this` would have been dereferenced as an object. So the rule refuses.
        return "unresolved"
    # __cdecl: the callee pops nothing under either layout, so the body is
    # silent and only a caller could say.
    return "unresolved"


def agreement(signature: Signature, evidence: BodyEvidence) -> str:
    """'agree', 'unknown', or a reason the body contradicts the name."""
    pop_observed = evidence.callee_pop_bytes
    if pop_observed is None:
        return "unknown"
    pop_declared = signature.callee_pop_bytes
    if pop_declared is None:
        return "unknown"
    if pop_declared != pop_observed:
        return (f"callee-pop {pop_observed} but the name declares "
                f"{pop_declared}")
    if signature.this_location == "ecx" and pop_observed == 0 \
            and not evidence.ecx_read_first and evidence.instructions > 1:
        return "named __thiscall but never reads ECX and pops nothing"
    return "agree"


def read_bodies(signatures, exe: Path, sizes: dict[str, int]):
    """BodyEvidence per address, using capstone over the pinned executable."""
    import pefile
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from capstone import x86 as cs_x86

    pe = pefile.PE(str(exe), fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    ecx_sub = {cs_x86.X86_REG_ECX, cs_x86.X86_REG_CX, cs_x86.X86_REG_CL,
               cs_x86.X86_REG_CH}

    out: dict[str, BodyEvidence] = {}
    for signature in signatures:
        size = sizes.get(signature.address.lower(), 0)
        if not size:
            continue
        try:
            data = pe.get_data(int(signature.address, 16) - base, size)
        except Exception:                                    # noqa: BLE001
            continue
        instructions = list(md.disasm(data, int(signature.address, 16)))
        if not instructions:
            continue
        imms = sorted({(i.operands[0].imm if i.operands else 0)
                       for i in instructions if i.mnemonic in ("ret", "retf")})
        ecx_first = False
        for one in instructions:
            read, written = one.regs_access()
            if any(r in ecx_sub for r in read):
                ecx_first = True
                break
            if any(w in ecx_sub for w in written):
                break
            if one.group(cs_x86.X86_GRP_JUMP) or one.group(cs_x86.X86_GRP_CALL):
                break
        out[signature.address] = BodyEvidence(imms, ecx_first,
                                              len(instructions))
    return out


def count_pushes_before(instructions, index: int) -> int:
    """The unbroken run of `push` immediately before instructions[index].

    Deliberately crude, and deliberately not extended: an argument set up with
    `mov [esp+8], eax` leaves a run of zero, and that reads as INCONCLUSIVE
    rather than as "pushed nothing". A cleverer reader would have to model the
    frame, and a wrong model here would manufacture agreement.
    """
    run = 0
    index -= 1
    while index >= 0 and instructions[index].mnemonic == "push":
        run += 1
        index -= 1
    return run


def call_sites(signatures, exe: Path, rows) -> dict:
    """Direct `call rel32` sites landing on `signatures`, and what they push.

    Two independent passes over the SAME executable, because the interesting
    number here was got badly wrong once by a hand-count that no tool could
    reproduce:

      * a raw byte scan of .text for E8, which needs no disassembler to be
        right and so cannot be fooled by a body boundary this catalogue has
        misplaced;
      * capstone over the catalogued bodies, which is what can attribute a site
        to a caller and count the pushes in front of it.

    The two site totals are returned separately and are expected to differ by a
    little: a byte sequence inside data or inside a body the catalogue does not
    hold decodes as E8 in one pass and not the other.
    """
    import pefile
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs

    pe = pefile.PE(str(exe), fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    section = next(s for s in pe.sections
                   if s.Name.rstrip(b"\0") == b".text")
    start = base + section.VirtualAddress
    blob = section.get_data()
    stop = start + len(blob)

    wanted = {int(s.address, 16): s for s in signatures}
    raw: dict[int, list[int]] = {}
    for offset in range(len(blob) - 5):
        if blob[offset] != 0xE8:
            continue
        site = start + offset
        target = site + 5 + int.from_bytes(blob[offset + 1:offset + 5],
                                           "little", signed=True)
        if target in wanted:
            raw.setdefault(target, []).append(site)

    md = Cs(CS_ARCH_X86, CS_MODE_32)
    bodies = {}
    for row in rows:
        address = int(row["address"], 16)
        size = int(row["size"] or 0)
        if size > 0 and start <= address and address + size <= stop:
            bodies[address] = blob[address - start:address - start + size]

    decoded: dict[int, list[tuple[int, int]]] = {}
    for caller, body in bodies.items():
        for one in md.disasm(body, caller):
            if one.mnemonic != "call" or not one.bytes or one.bytes[0] != 0xE8:
                continue
            try:
                target = int(one.op_str, 16)
            except ValueError:
                continue
            if target in wanted:
                decoded.setdefault(target, []).append((caller, one.address))

    summary = {"raw_targets": len(raw),
               "raw_sites": sum(len(v) for v in raw.values()),
               "decoded_targets": len(decoded),
               "decoded_sites": sum(len(v) for v in decoded.values()),
               "pushes_declared": 0, "pushes_declared_plus_one": 0,
               "inconclusive": 0, "conclusive_targets": 0,
               "sites_per_target": {f"0x{t:08X}": len(v)
                                    for t, v in raw.items()}}
    for target, sites in decoded.items():
        declared = len(wanted[target].stack_slots_for("none") or [])
        settled = False
        for caller, site in sites:
            seq = list(md.disasm(bodies[caller], caller))
            index = next((k for k, one in enumerate(seq)
                          if one.address == site), None)
            if index is None:
                summary["inconclusive"] += 1
                continue
            run = count_pushes_before(seq, index)
            if run == declared:
                summary["pushes_declared"] += 1
                settled = True
            elif run == declared + 1:
                summary["pushes_declared_plus_one"] += 1
                settled = True
            else:
                summary["inconclusive"] += 1
        summary["conclusive_targets"] += settled
    return summary


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--functions", type=Path, default=FUNCTIONS_CSV)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--report", action="store_true")
    parser.add_argument("--cross-check", action="store_true")
    parser.add_argument("--disagreements", action="store_true")
    parser.add_argument("--call-sites", action="store_true",
                        help="scan .text for direct calls to the 139 and count "
                             "what each site pushes")
    parser.add_argument("--seed-table", type=Path)
    args = parser.parse_args(argv)

    import undname
    rows = load_rows(args.functions)
    signatures, skipped = recover(rows, undname.undname)
    sizes = {r["address"].lower(): int(r["size"]) for r in rows}
    total = sum(sizes.get(s.address.lower(), 0) for s in signatures)

    if args.report or not any((args.cross_check, args.disagreements,
                               args.call_sites, args.seed_table)):
        print(f"catalogue rows            {len(rows)}")
        for key, value in skipped.most_common():
            print(f"  skipped {key:34s} {value:5d}")
        print(f"real named functions      {len(signatures)}  "
              f"bytes {total}")
        table = Counter((s.kind, s.convention) for s in signatures)
        print("\nkind x convention:")
        for key in sorted(table):
            size = sum(sizes.get(s.address.lower(), 0) for s in signatures
                       if (s.kind, s.convention) == key)
            print(f"  {key[0]:9s} {key[1]:11s} {table[key]:5d}  {size:8d} B")
        odd = [s for s in signatures
               if s.takes_this and s.convention != "__thiscall"]
        print(f"\nimplicit `this` NOT in ECX: {len(odd)}  bytes "
              f"{sum(sizes.get(s.address.lower(), 0) for s in odd)}")
        for conv, count in Counter(s.convention for s in odd).most_common():
            size = sum(sizes.get(s.address.lower(), 0) for s in odd
                       if s.convention == conv)
            print(f"  {conv:11s} {count:5d}  {size:8d} B")

    if args.cross_check:
        bad = 0
        for signature in signatures:
            printed = next((c for c in ("__thiscall", "__cdecl", "__stdcall",
                                        "__fastcall", "__pascal")
                            if c in signature.demangled.split("(")[0]), None)
            if printed and printed != signature.convention:
                bad += 1
                print(f"  {signature.address} infix={signature.convention} "
                      f"undname={printed}  {signature.demangled}")
        print(f"cross-check: {bad} disagreement(s) between the infix parse "
              f"and undname over {len(signatures)} rows")

    if args.disagreements:
        evidence = read_bodies(signatures, args.exe, sizes)
        tally: Counter = Counter()
        for signature in signatures:
            found = evidence.get(signature.address)
            if found is None:
                tally["no body"] += 1
                continue
            verdict = agreement(signature, found)
            tally[verdict if verdict in ("agree", "unknown") else "DISAGREE"] += 1
            if verdict not in ("agree", "unknown"):
                print(f"  {signature.address} {signature.convention:10s} "
                      f"{verdict}  {signature.name[:64]}")
        print()
        for key, value in tally.most_common():
            print(f"  {key:12s} {value}")

    if args.call_sites:
        odd = [s for s in signatures
               if s.takes_this and s.convention != "__thiscall"]
        found = call_sites(odd, args.exe, rows)
        print(f"the 139 (public instance, not __thiscall): {len(odd)}")
        print(f"  reached by a direct call rel32   "
              f"{found['raw_targets']} targets, {found['raw_sites']} sites "
              f"(raw E8 scan of .text)")
        print(f"  attributed to a catalogued body  "
              f"{found['decoded_targets']} targets, {found['decoded_sites']} "
              f"sites (capstone)")
        print(f"  targets with a countable site    "
              f"{found['conclusive_targets']}")
        print(f"  sites pushing the declared count {found['pushes_declared']}")
        print(f"  sites pushing declared + 1       "
              f"{found['pushes_declared_plus_one']}   <- a pushed `this` "
              f"would live here")
        print(f"  sites the push run cannot settle {found['inconclusive']}")
        busiest = sorted(found["sites_per_target"].items(),
                         key=lambda kv: -kv[1])[:5]
        for address, count in busiest:
            name = next(s.name for s in odd
                        if s.address.lower() == address.lower())
            print(f"    {address} {count:4d} sites  {name[:56]}")

    if args.seed_table:
        # The body is consulted before anything is written: a row whose `this`
        # location cannot be resolved is OMITTED rather than guessed, because a
        # phantom slot 0 shifts every real argument up by one and would seed
        # the wrong words.
        evidence = read_bodies(signatures, args.exe, sizes)
        lines = ["# address\tpointer-slot-mask\tthis\tconvention   "
                 "(generated by recover_conventions.py)"]
        omitted: Counter = Counter()
        for signature in sorted(signatures, key=lambda s: int(s.address, 16)):
            location = resolve_this_location(
                signature, evidence.get(signature.address))
            slots = signature.stack_slots_for(location)
            if slots is None:
                omitted["unresolved `this` or unknown argument widths"] += 1
                continue
            lines.append(f"{int(signature.address, 16):#010x}\t"
                         f"{signature.pointer_mask_for(location):#x}\t"
                         f"{location}\t{signature.convention}")
        args.seed_table.write_text("\n".join(lines) + "\n")
        print(f"wrote {args.seed_table} ({len(lines) - 1} rows)")
        for key, value in omitted.most_common():
            print(f"  omitted {key}: {value}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
