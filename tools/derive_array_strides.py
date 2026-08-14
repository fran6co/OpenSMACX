#!/usr/bin/env python3
"""What stride does the image index each global array by?

`static_assert(sizeof(X) == N)` is how `src/` states that a layout was checked
against the image, and `class_layouts.pinned_layouts()` will not consider a
class without one. Writing that assertion by hand means hand-tracing an index
computation per class, which is why 206 NO_COMPILE rows were still blocked on
`Map`, `Player`, `RulesBasic` and the rest of the game-data structs on
2026-08-14 - not because the sizes are hard, but because each one costs a
disassembly session.

The size is not hidden. An array of structs is indexed by its own size, and
cl 12.00.8168 spells that in exactly two ways:

    lea  edx, [eax + eax*4]          ; a chain of lea, for a size that
    lea  eax, [eax + edx*2]          ; factors into 2/3/5/9 and a scale
    mov  edx, dword ptr [0x94a30c]
    lea  eax, [edx + eax*4]          ; stride = 11 * 4 = 44

    imul eax, eax, 0x2c              ; the general case
    add  eax, 0x75b710

So the stride can be READ, per global, from the image - and the answer is a
measurement rather than a restatement of the header, which is the whole point
of the assertion.

DECODED ONLY INSIDE CATALOGUED SPANS. A linear sweep of `.text` starts at
whatever byte it is handed and decodes jump tables, padding and the tails of
instructions as if they were code: the first version of this scan reported
`add byte ptr [ebp - 0x1e2e76f4], cl` in the middle of a perfectly ordinary
function and counted the immediates it invented. `src/` knows where every
function begins.

    tools/derive_array_strides.py                 # every global, ranked
    tools/derive_array_strides.py --global 0x94a30c
    tools/derive_array_strides.py --check         # the committed asserts agree
"""
from __future__ import annotations

import argparse
import bisect
import collections
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile                                                 # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs              # noqa: E402

import byte_match                                             # noqa: E402
import emit_translation_unit as emit                          # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"

SPAN = re.compile(r"0x([0-9A-Fa-f]{8})-0x([0-9A-Fa-f]{8})")
# `Map **MapTiles = (Map **)0x0094A30C;` - src/ binding a name to an address.
GLOBAL_BINDING = re.compile(
    r"^\s*(?P<type>[A-Za-z_]\w*)\s*\**\s*(?P<name>\w+)\s*=\s*"
    r"\((?P=type)\s*\**\)\s*0x(?P<address>[0-9A-Fa-f]{8})\s*;", re.M)
SIZE_ASSERT = re.compile(
    r"static_assert\s*\(\s*sizeof\s*\(\s*(\w+)\s*\)\s*==\s*"
    r"(0[xX][0-9A-Fa-f]+|\d+)")

# `lea r, [b + b*k]` multiplies by k+1 when both registers are the same one.
LEA_SELF = re.compile(r"^(?P<dst>\w+), \[(?P<b>\w+) \+ (?P<i>\w+)\*(?P<k>\d)\]$")
LEA_BASE = re.compile(r"^(?P<dst>\w+), \[(?P<b>\w+) \+ (?P<i>\w+)\*(?P<k>\d)\]$")
MEM_INDEX = re.compile(r"\[(?P<b>\w+) \+ (?P<i>\w+)\*(?P<k>\d)"
                       r"(?: \+ (?P<disp>0x[0-9a-f]+|\d+))?\]")
GLOBAL_LOAD = re.compile(r"^(?P<dst>\w+), dword ptr \[0x(?P<address>[0-9a-f]+)\]$")
# `mov ecx, dword ptr [eax*4 + 0x96c9e0]` - a DIRECT array, whose base is an
# immediate rather than a pointer read from memory. `Player *Players =
# (Player *)0x00946A50;` is the shape: most of the game-data tables are
# declared this way, so a scan that only followed `mov reg, [global]` saw none
# of them.
MEM_IMMEDIATE = re.compile(
    r"\[(?P<i>[a-z]\w+)\*(?P<k>\d)(?: \+ 0x(?P<address>[0-9a-f]{6,8}))\]")
ADD_IMMEDIATE = re.compile(r"^(?P<dst>\w+), 0x(?P<address>[0-9a-f]{6,8})$")
# `mov edx, dword ptr [eax + 0x946a50]` - the index has already been scaled
# into the register by a `shl`, so the array base is the whole displacement
# and there is no `*k` left to read.
MEM_BASE_IMMEDIATE = re.compile(
    r"\[(?P<b>[a-z]\w+) \+ 0x(?P<address>[0-9a-f]{6,8})\]")
SHIFT = re.compile(r"^(?P<dst>\w+), (?P<bits>\d+)$")
REG_REG = re.compile(r"^(?P<dst>[a-z]\w+), (?P<src>[a-z]\w+)$")


def image():
    pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    section = [s for s in pe.sections if s.Name.startswith(b".text")][0]
    return (section.get_data(),
            pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress)


def spans():
    """(low, high) for every catalogued body range, so decoding starts on an
    instruction boundary."""
    for address, row in sorted(emit.load_functions().items()):
        for low, high in SPAN.findall(row.get("body_ranges") or ""):
            yield address, int(low, 16), int(high, 16)


def bindings() -> dict:
    """{address: [name, ...]} - the globals `src/` has already named."""
    found = collections.defaultdict(list)
    for source in sorted(SRC.rglob("*.cpp")):
        for hit in GLOBAL_BINDING.finditer(source.read_text(errors="ignore")):
            found[int(hit.group("address"), 16)].append(hit.group("name"))
    return found


def _multiplier(track: dict, register: str):
    """How many times the ORIGINAL index `register` currently holds, or None.

    A `lea` chain is only a stride when every step stays on the same value:
    `lea edi, [eax + eax*4]` is `eax*5` and `lea eax, [eax + edi*2]` is
    `eax*11` ONLY because edi is that same eax times five. Tracking the factor
    per register is what separates the two from an unrelated address sum.
    """
    return track.get(register)


def strides(data, base, only=None):
    """{global address: Counter of stride} read off the image.

    A stride is counted when a scaled memory reference is reached through a
    register whose factor is known and whose base register was loaded from a
    global - `base + i*factor*scale`.
    """
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True   # `operands` is empty without it
    found = collections.defaultdict(collections.Counter)
    sites = collections.defaultdict(list)
    for _, low, high in spans():
        if not base <= low < base + len(data):
            continue
        window = data[low - base:min(high, base + len(data)) - base]
        # factor[r]: r holds (one index) * factor, KNOWN. origin[r]: r was
        # loaded from that global. An absent factor means unknown, and it is
        # never defaulted to 1 - doing that reported stride 4 at 1,364 sites
        # for MapTiles, which is the scale of the final `[base + i*4]` with
        # the `*11` in front of it thrown away.
        factor, origin = {}, {}
        for instruction in engine.disasm(window, low):
            operands = instruction.op_str
            mnemonic = instruction.mnemonic

            if mnemonic == "mov":
                hit = GLOBAL_LOAD.match(operands)
                if hit:
                    origin[hit.group("dst")] = int(hit.group("address"), 16)
                    factor.pop(hit.group("dst"), None)
                    continue

            if mnemonic == "lea":
                hit = LEA_SELF.match(operands)
                if hit:
                    dst, b, i = hit.group("dst"), hit.group("b"), hit.group("i")
                    k = int(hit.group("k"))
                    if b in origin and i in factor:
                        # `lea eax, [edx + eax*4]` - the array access itself.
                        _record(found, sites, origin[b], factor[i] * k,
                                instruction.address, only)
                        origin.pop(dst, None)
                        factor.pop(dst, None)
                        continue
                    if b == i:
                        # `lea edi, [eax + eax*4]` both establishes eax as an
                        # index and gives edi five of it. Seeding the base is
                        # what lets the NEXT lea in the chain resolve.
                        factor.setdefault(b, 1)
                        factor[dst] = factor[b] * (k + 1)
                    elif b in factor and i in factor:
                        factor[dst] = factor[b] + factor[i] * k
                    else:
                        factor.pop(dst, None)
                    origin.pop(dst, None)
                    continue

            # `shl eax, 3` / `sub eax, edi` - multiply-by-constant spelled
            # without an imul. `sizeof(Player)` is 359*4 and reaches the
            # index as i*5, *9, <<3, -i: no single instruction in that chain
            # names 1436, and skipping any one of them loses the array.
            if mnemonic in ("shl", "sal"):
                hit = SHIFT.match(operands)
                if hit and hit.group("dst") in factor:
                    factor[hit.group("dst")] <<= int(hit.group("bits"))
                    origin.pop(hit.group("dst"), None)
                    continue
            if mnemonic in ("add", "sub"):
                hit = REG_REG.match(operands)
                if hit and hit.group("dst") in factor \
                        and hit.group("src") in factor:
                    sign = 1 if mnemonic == "add" else -1
                    factor[hit.group("dst")] += sign * factor[hit.group("src")]
                    origin.pop(hit.group("dst"), None)
                    continue

            if mnemonic == "imul" and len(instruction.operands) == 3:
                dst = operands.split(",")[0].strip()
                factor[dst] = instruction.operands[2].imm
                origin.pop(dst, None)
                continue

            hit = MEM_INDEX.search(operands)
            if hit and hit.group("b") in origin and hit.group("i") in factor:
                _record(found, sites, origin[hit.group("b")],
                        factor[hit.group("i")] * int(hit.group("k")),
                        instruction.address, only)

            hit = MEM_IMMEDIATE.search(operands)
            if hit and hit.group("i") in factor:
                _record(found, sites, int(hit.group("address"), 16),
                        factor[hit.group("i")] * int(hit.group("k")),
                        instruction.address, only)

            hit = MEM_BASE_IMMEDIATE.search(operands)
            if hit and hit.group("b") in factor:
                _record(found, sites, int(hit.group("address"), 16),
                        factor[hit.group("b")], instruction.address, only)

            if mnemonic == "add":
                hit = ADD_IMMEDIATE.match(operands)
                if hit and hit.group("dst") in factor:
                    # `imul eax, eax, 0x2c` / `add eax, 0x75b710` - the same
                    # array access spelled without a memory operand.
                    _record(found, sites, int(hit.group("address"), 16),
                            factor[hit.group("dst")],
                            instruction.address, only)

            # A register this does not model must not keep a stale factor:
            # that is how a real chain gets attributed to an index overwritten
            # two instructions ago.
            if mnemonic in ("mov", "xor", "add", "sub", "and", "or", "shl",
                            "sar", "shr", "pop", "movzx", "movsx", "not",
                            "neg", "inc", "dec"):
                dst = operands.split(",")[0].strip()
                if dst.isalpha():
                    factor.pop(dst, None)
                    if mnemonic != "mov":
                        origin.pop(dst, None)
            elif mnemonic == "call":
                factor.clear()
                origin.clear()
    return found, sites


def _record(found, sites, address, stride, where, only):
    if stride < 2 or stride > 0x10000:
        return
    if only is not None and address != only:
        return
    found[address][stride] += 1
    if len(sites[(address, stride)]) < 5:
        sites[(address, stride)].append(where)


def by_owner(found: dict, named: dict = None) -> dict:
    """{global address: Counter of stride}, folded onto the array each
    reference belongs to.

    A REFERENCE RARELY NAMES THE BASE. `lea esi, [eax*4 + 0x946a54]` reaches
    field +4 of `Players` at 0x946A50, so keying on the raw immediate scatters
    one array across as many keys as it has touched fields - `Chassis` had 5
    hits on its base address and 497 across the struct. Each immediate is
    folded onto the nearest global `src/` has named at or below it, and only
    when it lands inside the FIRST element; past that it belongs to whatever
    global comes next, not to this one.
    """
    if named is None:
        named = bindings()
    keys = sorted(named)
    out = collections.defaultdict(collections.Counter)
    for address, counts in found.items():
        index = bisect.bisect_right(keys, address) - 1
        if index < 0:
            continue
        owner = keys[index]
        for stride, hits in counts.items():
            if address - owner < stride:
                out[owner][stride] += hits
    return out


def asserted() -> dict:
    """{class: size} for every `static_assert(sizeof(X) == N)` in src/."""
    out = {}
    for header in sorted(SRC.glob("*.h")):
        for name, size in SIZE_ASSERT.findall(
                header.read_text(errors="ignore")):
            out[name] = int(size, 0)
    return out


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--global", dest="only",
                        help="one global address, e.g. 0x94a30c")
    parser.add_argument("--check", action="store_true",
                        help="report asserts the image seems to disagree with "
                             "(a REVIEW AID - see the note in main)")
    parser.add_argument("--min-sites", type=int, default=2,
                        help="ignore a stride seen fewer times (default 2)")
    arguments = parser.parse_args()

    data, base = image()
    only = int(arguments.only, 16) if arguments.only else None
    found, sites = strides(data, base, only)
    named = bindings()
    owned = by_owner(found, named)

    if arguments.check:
        # NOT A GATE, AND DELIBERATELY NOT REGISTERED AS ONE. The rule -
        # "the image never indexes this global by the size src/ asserts" -
        # has a false positive this scan cannot remove: a body stepping
        # through an ARRAY MEMBER INSIDE the struct indexes the same base
        # register by that member's size. `BaseCurrent` steps 0xC through a
        # `Base` of 0x134 for exactly that reason, and `g_FX` steps 0x6C
        # through 0x28F0.
        #
        # Two earlier shapes were removed because they were not
        # disagreements at all: a MULTIPLE of the asserted size is a 2-D
        # array (`Sprite [N][4]` steps 0xB0 through 0x2C, `Font [N][4]`
        # steps 0xA0 through 0x28), and a DIVISOR is the member case above.
        # Filtering those took the first run from 5 findings to 2, and both
        # survivors are still false. Publishing this as a gate would mean
        # publishing a red check nobody can make green.
        #
        # It is kept because it points a human at the handful of globals
        # worth reading, which is what it is good at.
        sizes, bad = asserted(), []
        for address, counts in owned.items():
            for name in named.get(address, ()):
                declared = _declared_type(name)
                if declared not in sizes:
                    continue
                best = [s for s, c in counts.items()
                        if c >= arguments.min_sites]
                # A MULTIPLE IS NOT A CONTRADICTION. `Sprite [N][4]` is
                # indexed by the ROW, so the image steps 0xB0 through a
                # struct of 0x2C - four of them - and `Font [N][4]` steps
                # 0xA0 through 0x28. Both were reported as the tree
                # disagreeing with the binary on the first run; neither is.
                # A divisor is not one either: a body indexing a member
                # array inside the struct steps by that member's size.
                size = sizes[declared]
                if best and not any(s % size == 0 or size % s == 0
                                    for s in best):
                    bad.append((name, declared, size, sorted(best)))
        for name, klass, says, seen in bad:
            print(f"{name}: src/ asserts sizeof({klass}) == 0x{says:X}, the "
                  f"image indexes it by {[hex(s) for s in seen]}",
                  file=sys.stderr)
        print(f"{len(owned)} indexed global(s), {len(bad)} worth a look "
              f"(each may be an array member inside the struct)")
        return 0

    ranked = sorted(owned.items(),
                    key=lambda kv: -sum(kv[1].values()))
    for address, counts in ranked[:40]:
        best = [(s, c) for s, c in counts.most_common(3)
                if c >= arguments.min_sites]
        if not best:
            continue
        names = ", ".join(named.get(address, ())) or "(unnamed)"
        print(f"0x{address:08X}  {names}")
        for stride, count in best:
            where = " ".join(f"0x{a:08X}"
                             for a in sites[(address, stride)][:3])
            print(f"    stride 0x{stride:<5X} {stride:>6}  x{count:<4} {where}")
    return 0


def _declared_type(name: str) -> str:
    """The type `src/` binds to that global name, e.g. MapTiles -> Map."""
    for source in sorted(SRC.rglob("*.cpp")):
        for hit in GLOBAL_BINDING.finditer(source.read_text(errors="ignore")):
            if hit.group("name") == name:
                return hit.group("type")
    return ""


if __name__ == "__main__":
    raise SystemExit(main())
