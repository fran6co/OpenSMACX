#!/usr/bin/env python3
"""Generate hybrid-runtime oracles for functions whose signature is derivable.

`unproven_recovered` says 96.9% of recovered bytes have never been executed
against the original - 2,498 functions, 190,008 B. Every proof that exists was
hand-written. This turns the cheap cases into a build step.

WHY THIS NEEDS NO SYMBOL LOOKUP, which is the part that makes it possible.

The recovered source is compiled by MinGW with Itanium mangling; the names in
the inventory are MSVC names read out of the original binary. There is no link
between them, so a generated oracle cannot call the recovered function by name.
It does not have to. In the hybrid the recovered body is PATCHED OVER the
original's address, so one call site serves both: with the redirect suspended
the address runs the original, with it active the same address runs the
recovery. `suspend_redirect_at` / `resume_redirect_at` in dllmain.cpp are the
switch, and src/sprite_oracle.cpp uses them by hand already.

WHAT IS COMPARED, and why a return value alone would be worthless. Many
candidates return void and do their work in .data - `base_minerals` computes
into global arrays and returns nothing. So the comparison snapshots .data/.bss
AND the staged object, runs one side, captures the result plus everything it
wrote, restores BOTH, runs the other side, and compares. A recovery that
returns the right value and writes the wrong globals fails here.

MEMBER FUNCTIONS AND ARGUMENTS, and the two ways they lie.

Members with arguments are admitted, which they were not before, because the
shape is readable off the inventory's `prototype` column rather than guessed
from the mangled name: `int (__thiscall ?pos_to_id@Dialog@@QAEHH@Z)(Dialog*
this, int)` states the class, the convention and every argument type. But two
failure modes make an agreeing member oracle worth nothing unless they are
handled, and both are handled here:

  1. GUARD AGREEMENT. A staged object no field of which means anything makes the
     body bail at its first guard on BOTH sides, and the comparison agrees
     having executed four bytes of a hundred. `Scroll::set_pos` opens
     `if (!parent) return 0;`, and the hand-written suite escapes that only
     because a human wrote 0x45454545 into offset 0xC4 - an offset no signature
     states. So every case records whether the original produced ANY observable
     effect, and a function where no seed did is reported
     `INCONCLUSIVE-no-effect` and REFUSED A MARKER. It is not a pass; it is a
     request for a hand-written field seed.

  2. THE OBJECT NOT RESTORED BETWEEN THE TWO CALLS. The globals snapshot spans
     0x00682000..0x009C21F8 and a staged object lives outside it, so restoring
     only globals compares A(s0) against R(A(s0)) instead of R(s0). Every
     idempotent setter then agrees for free, and 325 candidates return void -
     exactly that shape. The fixture is snapshotted and restored alongside the
     globals.

THE OBJECT IS SEEDED WITH ZEROS, deliberately, and that costs reach on purpose.
This runs inside the real game with no isolation. A patterned object makes every
pointer field a wild address, and the first body that dereferences one takes the
game down - which is what `?help_tech@@YAXH@Z` did on the third function of
thirty-six. Zeros make guards bail SAFELY. The consequence is that many members
will report `INCONCLUSIVE-no-effect`, and that is the honest outcome: the
generator says so rather than publishing agreement as proof.

WHAT IS DELIBERATELY NOT GENERATED:

  * `sub_*` functions, whose arity is unknown; `ret N` is the only statement of
    it and reading that is a separate job;
  * members of a class whose `sizeof` is not pinned by a static_assert in
    src/*.h - without the real size the fixture is the wrong length and the
    comparison reads past the object;
  * any argument that is not an integer-like scalar. A pointer argument would
    need a second staged object, and a wrong one proves nothing while looking
    like a proof;
  * bodies below MinimumBodyBytes, which cannot disagree in an interesting way;
  * members that are not __thiscall (QAA/__cdecl, QAG/__stdcall): the prototype
    column is empty for those, so there is nothing to read.

MARKERS ARE EARNED AT RUNTIME, NOT AT GENERATION TIME. This is the correction
that matters most. An earlier revision published a `PROVEN-AGAINST-ORIGINAL:`
marker for every function it emitted; 37 of them had never been run, and
`docs/recovery/{proven,summary}.json` were promoted to match. A marker is now
emitted only for an address that is either already recorded in proven.csv
against this file - those were earned by three runs of three - or listed in the
`--verdicts` file produced by an actual suite run. Generating cannot mint a
proof.
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_PROVEN = REPO_ROOT / "docs" / "recovery" / "proven.csv"
DEFAULT_OUTPUT = REPO_ROOT / "src" / "generated_signature_oracle.cpp"
DEFAULT_DLLMAIN = REPO_ROOT / "src" / "dllmain.cpp"
DEFAULT_HEADERS = REPO_ROOT / "src"

# MSVC mangling: ?name@@<scope><conv><return><args>Z. `Y` is a free function;
# the character after it is the calling convention, then the return type, then
# the parameter list. `X` alone as the parameter list means (void).
# A NON-EMPTY parameter list is terminated by "@Z"; "(void)" is the bare "XZ"
# form. Missing that cost a measurement: `?energy_limit@@YAHH@Z` matched
# nothing, so a whole class of one-argument functions read as absent.
FREE_WITH_ARGS = re.compile(r"^\?([A-Za-z_][\w]*)@@Y([A-Z])([A-Z])([A-Z]+)@Z$")
FREE_NO_ARG = re.compile(r"^\?([A-Za-z_][\w]*)@@Y([A-Z])([A-Z])XZ$")

# `?method@Class@@Q<cv><conv>...` - a public member. `AE` is non-const
# __thiscall, which is the only member shape this generates.
MEMBER_NAME = re.compile(r"^\?([A-Za-z_][\w]*)@([A-Za-z_][\w]*)@@QAE")

# The inventory's own prototype column, which states what the mangled name only
# encodes: `int (__thiscall ?pos_to_id@Dialog@@QAEHH@Z)(Dialog* this, int)`.
PROTOTYPE = re.compile(
    r"^(?P<ret>[A-Za-z_][\w :*]*?)\s*\(\s*(?P<conv>__\w+)\s+\?[^)]*\)"
    r"\s*\((?P<params>.*)\)\s*$")

# A static_assert in a header is the only statement of a class's real size that
# has been checked against the original. Deriving it from the C++ type would be
# circular: the recovered layout is what is under test.
PINNED_SIZE = re.compile(
    r"static_assert\(\s*sizeof\(\s*([A-Za-z_][\w]*)\s*\)\s*==\s*"
    r"(0[xX][0-9A-Fa-f]+|\d+)")

# Argument and return types that can be driven as a plain integer. A pointer or
# a struct would need a staged object of its own.
SCALAR_PARAMS = {
    "int": "int",
    "unsigned int": "unsigned int",
    "unsigned": "unsigned int",
    "long": "long",
    "unsigned long": "unsigned long",
    "short": "short",
    "unsigned short": "unsigned short",
    "char": "char",
    "unsigned char": "unsigned char",
    "signed char": "signed char",
    "bool": "bool",
    "uint32_t": "uint32_t",
    "int32_t": "int32_t",
    "uint16_t": "uint16_t",
    "int16_t": "int16_t",
    "uint8_t": "uint8_t",
    "int8_t": "int8_t",
}

# A body this small cannot disagree in a way worth publishing: `xor eax,eax /
# ret` agrees with any transcription of itself. The existing route already
# claims a 1 B and a 3 B function, which is a wart, not a precedent to extend.
MinimumBodyBytes = 16

# Seeds every argument is driven with. Small, boundary and negative values,
# which is where a transcription slip shows up; the same reasoning as the
# lifted oracle's case list.
ARGUMENT_SEEDS = (0, 1, -1, 2, 7, 0x7FFFFFFF, -0x80000000, 0x55555555)

# Classes whose methods must not be driven twice, because the thing they touch
# is not inside the snapshot and cannot be rolled back by restoring it.
#
#   * the audio and MIDI families own OS device handles. Calling
#     `?close@AutoSound@@QAEXXZ` and then restoring .data does not un-release a
#     wave device; the second call operates on a handle the first one freed.
#     `run_runtime_oracles` has already recorded three runs ending in an
#     unhandled division by zero at 0x004991DD inside original code reading
#     state a .data restore had made inconsistent.
#   * `Time` is written by ANOTHER THREAD. Time::start arms timeSetEvent with a
#     LPTIMECALLBACK, and MultimediaProc writes Time objects that sit inside the
#     0x00682000..0x009C21F8 window. Snapshot and restore are not atomic against
#     it, so a callback landing between two snapshots reads as "globals differ"
#     - a FALSE FAIL - and a restore that reverts the flag loses a WM_USER+1 the
#     game has already posted.
UNSAFE_CLASSES = {
    "AutoSound", "Midi", "Sound", "Wave", "WaveControlGroup", "Wave_Device",
    "Time",
}

# LIFECYCLE METHODS, refused for the same reason as the classes above: their
# effect escapes the snapshot, and a synthesized receiver is the wrong input by
# construction.
#
# MEASURED, not anticipated. The first run of the 122-oracle suite died on its
# FIRST function: `?close@StringStruct@@QAEXXZ` (0x00401060) took an unhandled
# page fault on read access to 0x00000004 at 0x00401074, and the game never
# reached the deferred phase. A zero-filled receiver is safe only for a body that
# GUARDS on its pointer fields; close() walks a chain the constructor guarantees
# non-null, so [this+X] read 0 and [0+4] faulted. Only one "running" line was
# printed, which is what identified it in one step.
#
# Teardown assumes constructed state and frees what it finds; construction
# allocates, and calling it twice leaks a block that restoring .data cannot free.
# `close` alone is the largest single name class in the candidate set - 12 of
# them.
LIFECYCLE_METHODS = {
    "close", "close2", "close3", "destroy", "free", "clear", "purge",
    "shutdown", "flush", "init", "init2", "reinit", "open", "release",
}

# Functions that cannot be driven when the deferred oracle phase runs, each
# with the run that proved it. `?help_tech@@YAXH@Z` reaches
# ?draw_labs@ReportWin@@QAEXXZ, which divides by zero because the report window
# state it reads does not exist yet. This is not an out-of-domain argument: the
# function fails on 0 and on 1 as well.
UNSAFE_AT_ORACLE_TIME = {
    0x0044C880,  # ?help_tech@@YAXH@Z - divide by zero via draw_labs, args 0 and 1
    # ?UNK1@PlanWin@@QAEXXZ - run 2 of the suite: "Unhandled page fault on read
    # access to 00000004 at address 0048B3C2", two bytes into the body, after 9
    # verdicts had already printed. It dereferences state that does not exist
    # when deferred oracles run; a zero-filled receiver cannot supply it.
    0x0048B3C0,
}


def redirected_addresses(dllmain_path: Path) -> set[int]:
    """Addresses the hybrid actually patches, read from the table itself.

    NOT the inventory's `redirect_exports` column, which does not agree with it:
    selecting on that column produced 39 oracles that every one failed with
    "cannot suspend redirect". The proof calls one address twice - redirect
    suspended for the original, restored for the recovery - so an address
    absent from this table has no second body to reach.
    """
    text = dllmain_path.read_text()
    start = text.index("const RedirectSpec specs[] = {")
    end = text.index("\n    };", start)
    return {int(value, 16) for value in
            re.findall(r"^\s*(0x[0-9A-Fa-f]{8}),\s*$", text[start:end], re.M)}


def pinned_class_sizes(header_dir: Path) -> dict:
    """Class sizes asserted against the original, read from src/*.h.

    Only these classes can be staged: a fixture of the wrong length makes the
    comparison read past the object, which is a false FAIL at best.
    """
    sizes = {}
    for header in sorted(header_dir.glob("*.h")):
        for name, value in PINNED_SIZE.findall(header.read_text()):
            sizes[name] = int(value, 0)
    return sizes


def earned_markers(proven_path: Path, output_path: Path,
                   verdict_path: Path | None) -> set[int]:
    """Addresses permitted to carry a PROVEN-AGAINST-ORIGINAL marker.

    Two sources, both of them records of something that actually ran:
    proven.csv rows whose evidence names this generated file, and a verdict file
    written by a suite run. Generation alone never mints a marker - that is what
    put 37 unearned markers in the tree once.
    """
    earned = set()
    if proven_path.exists():
        target = output_path.name
        with proven_path.open(newline="") as handle:
            for row in csv.DictReader(handle):
                if target in (row.get("evidence") or ""):
                    earned.add(int(row["address"], 16))
    if verdict_path is not None:
        # A RUN OVERRIDES THE RECORD, IN BOTH DIRECTIONS, and the demotion half
        # is not hypothetical. The first suite run to reach a verdict reported
        # INCONCLUSIVE-no-effect for 0x004456A0 ?passover_callback@@YAXXZ and
        # 0x00455E50 ?load_deswin_sprites@@YAXXZ - both of which were ALREADY in
        # proven.csv. They agreed because neither side did anything observable,
        # which is the flattering PASS this route exists to prevent, sitting in
        # the published count. Treating proven.csv as the last word would have
        # preserved exactly the two markers the new evidence refutes.
        for line in verdict_path.read_text().splitlines():
            match = re.match(
                r"GENERATED-ORACLE-VERDICT:\s*(0x[0-9A-Fa-f]+)\s+(\S+)",
                line.strip())
            if not match:
                continue
            address, state = int(match.group(1), 16), match.group(2)
            if state == "PASS":
                earned.add(address)
            else:
                # INCONCLUSIVE-no-effect, FAIL, FAIL-no-redirect: all of them
                # are evidence AGAINST a marker, so the address loses one.
                earned.discard(address)
    return earned


def scalar_parameters(params: str) -> list | None:
    """Argument types after `this`, or None if any is not an integer scalar."""
    text = params.strip()
    if not text or text == "void":
        return None
    parts = [part.strip() for part in text.split(",")]
    if "*" not in parts[0]:
        return None  # the first parameter must be the receiver
    if len(parts) == 1:
        # Receiver only. This is the SAFEST shape the generator emits: a staged
        # `this` and no argument domain to get wrong, which is where 588 of the
        # unproven members sit. Refusing it would have thrown away the one
        # cohort with no domain risk at all.
        return []
    types = []
    for part in parts[1:]:
        base = part.split()
        # Drop a parameter name if the column carries one.
        candidate = part if len(base) == 1 else " ".join(base[:-1]) \
            if base[-1].isidentifier() and " ".join(base[:-1]) in SCALAR_PARAMS \
            else part
        candidate = candidate.strip()
        if candidate not in SCALAR_PARAMS:
            return None
        types.append(SCALAR_PARAMS[candidate])
    return types


def member_candidate(row: dict, sizes: dict, bounds: dict | None = None) -> dict | None:
    """A __thiscall member with integer arguments and a stageable class size."""
    bounds = bounds or {}
    name = row["name"]
    named = MEMBER_NAME.match(name)
    if not named:
        return None
    method, class_name = named.groups()
    if method in LIFECYCLE_METHODS:
        return None  # allocates or releases; the effect escapes the snapshot
    if class_name not in sizes:
        return None  # neither pinned nor bounded; nothing to stage into
    if class_name in UNSAFE_CLASSES:
        return None  # owns an OS handle, or another thread writes it
    prototype = (row.get("prototype") or "").strip()
    if not prototype:
        return None  # empty for QAA/__cdecl and QAG/__stdcall members
    shape = PROTOTYPE.match(prototype)
    if not shape:
        return None
    if shape.group("conv") != "__thiscall":
        return None
    ret = shape.group("ret").strip()
    if ret != "void" and ret not in SCALAR_PARAMS:
        return None
    arg_types = scalar_parameters(shape.group("params"))
    if arg_types is None:
        return None
    return {
        "symbol": f"{class_name}_{method}",
        "convention": "__thiscall",
        "return": "void" if ret == "void" else SCALAR_PARAMS[ret],
        "args": arg_types,
        "class": class_name,
        "object_size": sizes[class_name],
        # PINNED means a static_assert checked the recovered class against the
        # original. BOUNDED means only that the object cannot be larger than
        # this, because the next constructed global sits there. The two are
        # never merged: a bounded oracle stages a receiver that is big enough
        # and is NOT evidence about the class's layout, and a reader who
        # cannot tell them apart will eventually quote one as the other.
        "size_source": bounds.get(class_name, "pinned"),
    }


def free_candidate(row: dict) -> dict | None:
    """A free function taking (void). Arguments are refused by DOMAIN.

    `?help_tech@@YAXH@Z` takes a tech id; seeded with -1 and 0x7FFFFFFF it
    walked into ?draw_labs@ReportWin@@QAEXXZ, which divided by zero and took the
    game down. The lifted oracle drives arbitrary integers safely because it
    runs against an isolated memory image where a wild value can only fault the
    harness. This runs inside the real game: a value outside a function's domain
    is a crash, not a result, and nothing in a mangled name states the domain.
    A free function's arguments are unguarded by any receiver, so they stay
    refused; a member's are at least bounded by the object's own fields.
    """
    name = row["name"]
    if FREE_WITH_ARGS.match(name):
        return None
    match = FREE_NO_ARG.match(name)
    if not match:
        return None
    symbol, convention, ret = match.groups()
    conventions = {"A": "__cdecl", "G": "__stdcall", "I": "__fastcall"}
    returns = {
        "X": "void", "H": "int", "I": "unsigned int", "J": "long",
        "K": "unsigned long", "E": "unsigned char", "D": "char",
        "F": "short", "G": "unsigned short", "M": "float", "N": "double",
    }
    if convention not in conventions or ret not in returns:
        return None
    return {
        "symbol": symbol,
        "convention": conventions[convention],
        "return": returns[ret],
        "args": [],
        "class": None,
        "object_size": 0,
    }


def candidates(functions_path: Path, proven_path: Path,
               redirected: set, sizes: dict | None = None,
               bounds: dict | None = None) -> list:
    # proven_path is accepted and deliberately unread for SELECTION - see the
    # note below. It is read separately, by earned_markers(), to decide which
    # rows may carry a marker.
    del proven_path
    sizes = {} if sizes is None else sizes
    bounds = {} if bounds is None else bounds
    found = []
    with functions_path.open(newline="") as handle:
        for row in csv.DictReader(handle):
            address = int(row["address"], 16)
            if row["recovery_state"] != "source_complete":
                continue
            if row["binary_kind"] != "game":
                continue
            # NOT filtered on `proven`, and that is a correction. Excluding
            # already-proven functions is circular here: publishing this file's
            # own markers makes its five functions proven, the next run then
            # selects none, and the committed file reads stale forever. A
            # function proven twice is not double-counted either -
            # export_proven_functions.py unions islands and markers by address
            # and records both mechanisms on one row.
            if address not in redirected:
                continue
            if address in UNSAFE_AT_ORACLE_TIME:
                continue
            shape = free_candidate(row) or member_candidate(row, sizes, bounds)
            if shape is None:
                continue
            size = int(row["size"])
            # A member's whole point is that it does something to its object, so
            # hold it to the size floor. The zero-argument free functions
            # already in the tree predate the floor and keep their place.
            if shape["class"] is not None and size < MinimumBodyBytes:
                continue
            shape.update({
                "address": address,
                "name": row["name"],
                "size": size,
                "source": row["source_locations"],
            })
            found.append(shape)
    return sorted(found, key=lambda item: item["address"])


def emit(rows: list, earned: set | None = None) -> str:
    earned = set() if earned is None else earned
    out: list = []
    w = out.append
    w("// GENERATED by tools/generate_signature_oracles.py - do not edit.")
    w("//")
    w("// Each function below is executed in its ORIGINAL form, at its canonical")
    w("// address inside the hybrid process, and compared against the recovered")
    w("// implementation reached through the same address with the redirect")
    w("// restored. Return value, the globals each side produces AND the staged")
    w("// object each side wrote are compared.")
    w("//")
    w("// A marker below means the function has RUN and agreed - either in the")
    w("// three-of-three runs recorded in docs/recovery/proven.csv or in a suite")
    w("// run whose verdicts were fed back with --verdicts. Generating this file")
    w("// cannot mint one. An earlier revision minted 37 that had never run.")
    w("//")
    for row in rows:
        if row["address"] in earned:
            w(f"// PROVEN-AGAINST-ORIGINAL: 0x{row['address']:08X}  {row['name']}")
    unearned = [row for row in rows if row["address"] not in earned]
    if unearned:
        w("//")
        w(f"// {len(unearned)} function(s) below carry NO marker: they have not run")
        w("// yet, or they ran and reported INCONCLUSIVE-no-effect, which means")
        w("// every seed bailed on a guard and the agreement proves nothing.")
    w("")
    w('#include "stdafx.h"')
    w('#include "generated_signature_oracle.h"')
    w("")
    w('#include "runtime_oracle.h"')
    w("")
    w('#include "globals_diff.h"')
    w('#include "oracle_fault_guard.h"')
    w("")
    w("#include <setjmp.h>")
    w("")
    w("#include <cstdio>")
    w("#include <cstdint>")
    w("#include <cstring>")
    w("#include <vector>")
    w("")
    w("namespace {")
    w("")
    w("// .data and .bss of the original image. The recovered bodies write here")
    w("// and so do the originals; comparing it is what makes a void-returning")
    w("// function testable at all.")
    w("constexpr uintptr_t GlobalsBegin = 0x00682000U;")
    w("constexpr uintptr_t GlobalsEnd   = 0x009C21F8U;")
    w("constexpr size_t GlobalsSize = GlobalsEnd - GlobalsBegin;")
    w("")
    # Namespace scope, not per function. As locals these were three
    # std::vector<uint8_t> per verify_ body, so the suite first-touched
    # 108 x 3 x 3,408,376 = 1.05 GiB of fresh pages - about 275,000 page faults
    # through Wine - to hold three buffers it uses one at a time.
    w("std::vector<uint8_t> GlobalsBefore;")
    w("std::vector<uint8_t> GlobalsAfterOriginal;")
    w("std::vector<uint8_t> GlobalsAfterRecovered;")
    w("")
    w("void snapshot(std::vector<uint8_t> &into) {")
    w("    into.resize(GlobalsSize);")
    w("    std::memcpy(into.data(), reinterpret_cast<const void *>(GlobalsBegin),")
    w("                GlobalsSize);")
    w("}")
    w("")
    w("void restore(const std::vector<uint8_t> &from) {")
    w("    std::memcpy(reinterpret_cast<void *>(GlobalsBegin), from.data(),")
    w("                GlobalsSize);")
    w("}")
    w("")
    w("// Where the two images first disagree, for a message that names an address")
    w("// rather than saying only that something differed. The comparison itself")
    w("// lives in src/globals_diff.h: it was a scalar loop over 3,408,376 bytes")
    w("// run twice per case, and the bisect that keeps the address is the only")
    w("// subtle part, which a hand-written header lets a C++ test check directly.")
    w("bool same_globals(const std::vector<uint8_t> &a,")
    w("                  const std::vector<uint8_t> &b, uintptr_t *where) {")
    w("    size_t first = 0;")
    w("    if (globals_diff::equal(a.data(), b.data(), GlobalsSize, &first)) {")
    w("        return true;")
    w("    }")
    w("    *where = GlobalsBegin + first;")
    w("    return false;")
    w("}")
    w("")
    w("// One verdict line per function, in a form tools/ can read back. Only an")
    w("// unqualified PASS earns a marker on the next regeneration; the whole")
    w("// point of INCONCLUSIVE is that it must NOT.")
    w("void verdict(uintptr_t address, const char *state, const char *name) {")
    w('    std::printf("GENERATED-ORACLE-VERDICT: 0x%08lX %s  %s\\n",')
    w("                (unsigned long)address, state, name);")
    w("    std::fflush(stdout);")
    w("}")
    w("")
    # The suite's cost per function, because it was never measured. The figure
    # this file's notes carried - "a 180 s run" - was prose; nothing timed it,
    # and the arithmetic argues the byte scan alone cannot account for it. Emit
    # the number so a speedup can be attributed instead of assumed.
    w("void timing(uintptr_t address, DWORD elapsed_ms, const char *name) {")
    w('    std::printf("GENERATED-ORACLE-TIMING: 0x%08lX %lu ms  %s\\n",')
    w("                (unsigned long)address, (unsigned long)elapsed_ms, name);")
    w("    std::fflush(stdout);")
    w("}")
    w("")
    w("}  // namespace")
    w("")
    # A __thiscall function POINTER is not a class method, so GCC warns on every
    # one of them. The hand-written suites silence it the same way; the
    # attribute is exactly what makes the receiver arrive in ECX.
    w('#include "original_seam.h"')
    w("")
    w('#pragma GCC diagnostic push')
    w('#pragma GCC diagnostic ignored "-Wattributes"')
    w('#pragma GCC diagnostic ignored "-Wuseless-cast"')
    w("")

    for row in rows:
        ret = row["return"]
        conv = row["convention"]
        addr = f"0x{row['address']:08X}"
        fn = f"verify_{row['symbol']}_{row['address']:08x}"
        args = row["args"]
        staged = row["class"] is not None
        params = ["void *"] if staged else []
        params += args
        w(f"// {row['name']}  ({row['size']} B)")
        w(f"// recovered in {row['source']}")
        if staged:
            w(f"// staged receiver: {row['class']}, "
              f"0x{row['object_size']:X} B, zero-filled, "
              f"size {row.get('size_source', 'pinned')}")
        w(f"static bool {fn}() {{")
        # A pointer-to-member IS thiscall, in every compiler, without naming
        # the convention - which matters because cl 12.00.8168 reserves the
        # `__thiscall` keyword and refuses it. See src/original_seam.h.
        if conv == "__thiscall" and staged:
            w(f"    typedef {ret} (OriginalObject::*Callable)"
              f"({', '.join(params[1:])});")
        else:
            w(f"    typedef {ret} ({conv} *Callable)({', '.join(params)});")
        if conv == "__thiscall" and staged:
            w(f"    Callable target = original_method<Callable>({addr}U);")
        else:
            w(f"    Callable target = reinterpret_cast<Callable>({addr}U);")
        # ANNOUNCE BEFORE CALLING, flushed. If a body takes the game down there
        # is no verdict line for it, and this is the only record of which one it
        # was; commit 7a2c554 credits exactly this with turning "the game
        # crashes" into "crashes on help_tech".
        w(f'    std::printf("  running {row["name"]}\\n");')
        w("    std::fflush(stdout);")
        # Aliases onto the namespace-scope buffers, so the body below reads the
        # same as it did when these were locals while the pages are allocated
        # once for the whole suite rather than three times per function.
        w("    std::vector<uint8_t> &before = GlobalsBefore;")
        w("    std::vector<uint8_t> &after_original = GlobalsAfterOriginal;")
        w("    std::vector<uint8_t> &after_recovered = GlobalsAfterRecovered;")
        w("    const DWORD started_at = GetTickCount();")
        w("    bool passed = true;")
        w("    bool observed_effect = false;")
        if staged:
            size = row["object_size"]
            # static, not stack: Console is 0x247A8 and MapWin 0x22480, and the
            # deferred phase runs on the game's own thread. A sibling harness
            # already took a STATUS_STACK_OVERFLOW from a stack-reserve change.
            w(f"    constexpr size_t ObjectSize = 0x{size:X}U;")
            w("    alignas(16) static uint8_t staged[ObjectSize];")
            w("    alignas(16) static uint8_t staged_seed[ObjectSize];")
            w("    alignas(16) static uint8_t staged_original[ObjectSize];")
        if args:
            # One row per case, so a failure names the arguments that caused it.
            # A 64-bit type, not the argument type: a seed of -1 or INT_MIN is
            # a narrowing conversion into an unsigned parameter and the build
            # refuses it. The call site casts each value back to its real type.
            #
            # `int64_t`, not `long long`. cl 12.00.8168 - the compiler this
            # tree is matched against, and now the only one that builds it -
            # predates `long long` and rejects it outright with
            # `error C2632: 'long' followed by 'long' is illegal`. <cstdint>
            # is already included above, and `int64_t` is the portable
            # spelling, so this reads identically on every other compiler.
            w(f"    static const int64_t cases[][{len(args)}] = {{")
            for index in range(len(ARGUMENT_SEEDS)):
                values = ", ".join(
                    str(ARGUMENT_SEEDS[(index + position) % len(ARGUMENT_SEEDS)])
                    for position in range(len(args)))
                w(f"        {{{values}}},")
            w("    };")
            # An index loop, not `for (const auto &argv : cases)`. VC6 has
            # neither the range-based `for` nor `auto` as a type - it still
            # reads `auto` as the C storage class - so the range form was 91
            # syntax errors in this file alone. `cases` is a plain C array
            # whose extent is known here, so the index form needs no library
            # support and means exactly the same thing everywhere.
            w("    for (size_t case_index = 0;")
            w("         case_index < sizeof(cases) / sizeof(cases[0]);")
            w("         ++case_index) {")
            w("        const int64_t *argv = cases[case_index];")
        else:
            w("    {")
        indent = "        "
        actuals = []
        if staged:
            actuals.append("staged")
        actuals += [f"({args[i]})argv[{i}]" for i in range(len(args))]
        if conv == "__thiscall" and staged:
            call = ("(ORIGINAL(" + actuals[0] + ")->*target)("
                    + ", ".join(actuals[1:]) + ")")
        else:
            call = "target(" + ", ".join(actuals) + ")"
        if staged:
            # ZEROS, not a pattern: a patterned object makes every pointer field
            # a wild address and the first dereference kills the game.
            w(f"{indent}std::memset(staged_seed, 0, ObjectSize);")
            w(f"{indent}std::memcpy(staged, staged_seed, ObjectSize);")
        w(f"{indent}snapshot(before);")
        w(f"{indent}if (!suspend_redirect_at({addr}U)) {{")
        w(f'{indent}    std::printf("  {row["name"]}: cannot suspend redirect\\n");')
        w(f'{indent}    verdict({addr}U, "FAIL-no-redirect", "{row["name"]}");')
        w(f"{indent}    return false;")
        w(f"{indent}}}")
        # GUARDED. A zero-filled receiver makes some bodies walk a chain their
        # constructor guarantees and read [0+4]; unguarded, the first such body
        # takes the process down and forfeits every remaining verdict. Measured:
        # the suite reached function 18 of 108.
        if ret != "void":
            w(f"{indent}{ret} original_result = ({ret})0;")
        w(f'{indent}oracle_fault_guard::begin({addr}U, "original");')
        w(f"{indent}if (setjmp(*oracle_fault_guard::buffer()) == 0) {{")
        if ret == "void":
            w(f"{indent}    {call};")
        else:
            w(f"{indent}    original_result = {call};")
        w(f"{indent}    snapshot(after_original);")
        if staged:
            w(f"{indent}    std::memcpy(staged_original, staged, ObjectSize);")
        w(f"{indent}    oracle_fault_guard::end();")
        w(f"{indent}}} else {{")
        w(f"{indent}    oracle_fault_guard::end();")
        # The faulting body may have written globals before it died, so put them
        # back before anything else runs.
        w(f"{indent}    restore(before);")
        # THE DANGEROUS ONE. The redirect is SUSPENDED at fault time. Escaping
        # without resuming leaves the recovered body uninstalled for the rest of
        # the process, so every later function's "recovered" call runs the
        # original and PASSES trivially - a flattering pass for the whole
        # remainder of the suite.
        w(f"{indent}    if (!resume_redirect_at({addr}U)) {{")
        w(f'{indent}        verdict({addr}U, "FAIL-no-redirect", "{row["name"]}");')
        w(f"{indent}        return false;")
        w(f"{indent}    }}")
        # The seed is outside this body's domain. That does not indict the
        # recovery, so it is INCONCLUSIVE rather than FAIL - and terminal for
        # this function, because the fixture and globals are in a restored but
        # unverified state and continuing would test something the verdict does
        # not name. Never sets observed_effect.
        w(f'{indent}    timing({addr}U, GetTickCount() - started_at, "{row["name"]}");')
        w(f'{indent}    verdict({addr}U, "INCONCLUSIVE-original-faulted", '
          f'"{row["name"]}");')
        w(f"{indent}    return true;")
        w(f"{indent}}}")
        w(f"{indent}restore(before);")
        if staged:
            # THE FIXTURE RESTORE. Without it the recovered call starts from
            # whatever the original left, so the comparison is A(s0) against
            # R(A(s0)) and every idempotent setter agrees for free.
            w(f"{indent}std::memcpy(staged, staged_seed, ObjectSize);")
        # THE PLACEBO. Run the ORIGINAL a second time from the same seed, with
        # the redirect still suspended, and require it to reproduce itself.
        #
        # Without this a function whose effect includes a freshly allocated
        # pointer FAILS forever and reads as a recovery defect.
        # ?clear_links@Buffer@@QAEXXZ was exactly that: it calls Spot::init on
        # the Spot at +0x4B0, which frees and re-allocates, so the compared
        # byte is the low byte of a malloc result - 0x10 on one run and 0xF0 on
        # the next. Nothing about the recovered body was wrong, and the harness
        # had no way to say so.
        #
        # This is the exact control for the recovered call that follows: same
        # seed, same restored globals, same suspended state. If the ORIGINAL
        # cannot agree with ITSELF under those conditions, the comparison has no
        # power and the honest verdict is that it was not measured - never PASS,
        # and never FAIL either.
        w(f'{indent}oracle_fault_guard::begin({addr}U, "original-again");')
        w(f"{indent}bool stable = true;")
        w(f"{indent}if (setjmp(*oracle_fault_guard::buffer()) == 0) {{")
        w(f"{indent}    {call};")
        w(f"{indent}    snapshot(after_recovered);")
        w(f"{indent}    oracle_fault_guard::end();")
        w(f"{indent}    uintptr_t drift = 0;")
        w(f"{indent}    if (!same_globals(after_original, after_recovered, &drift))")
        w(f"{indent}        stable = false;")
        if staged:
            w(f"{indent}    size_t drift_at = 0;")
            w(f"{indent}    if (!globals_diff::equal(staged_original, staged,")
            w(f"{indent}                             ObjectSize, &drift_at))")
            w(f"{indent}        stable = false;")
        w(f"{indent}}} else {{")
        # Faulting only on the second run is itself instability, and it is
        # reported as such rather than blamed on the recovered body.
        w(f"{indent}    oracle_fault_guard::end();")
        w(f"{indent}    stable = false;")
        w(f"{indent}}}")
        w(f"{indent}restore(before);")
        if staged:
            w(f"{indent}std::memcpy(staged, staged_seed, ObjectSize);")
        w(f"{indent}if (!stable) {{")
        w(f'{indent}    std::printf("  {row["name"]}: the ORIGINAL does not '
          f'reproduce itself; nothing here can be judged\\n");')
        w(f'{indent}    timing({addr}U, GetTickCount() - started_at, "{row["name"]}");')
        w(f'{indent}    verdict({addr}U, "INCONCLUSIVE-original-unstable", '
          f'"{row["name"]}");')
        w(f"{indent}    return true;")
        w(f"{indent}}}")
        w(f"{indent}if (!resume_redirect_at({addr}U)) {{")
        w(f'{indent}    std::printf("  {row["name"]}: cannot resume redirect\\n");')
        w(f'{indent}    verdict({addr}U, "FAIL-no-redirect", "{row["name"]}");')
        w(f"{indent}    return false;")
        w(f"{indent}}}")
        # The recovered side is guarded too, and the asymmetry is deliberate: the
        # ORIGINAL faulting says the seed is out of domain, while the RECOVERED
        # body faulting where the original completed is a divergence, and the
        # strongest kind there is. "Both faulted" cannot arise - the original-side
        # escape returns above before this call is reached.
        if ret != "void":
            w(f"{indent}{ret} recovered_result = ({ret})0;")
        w(f'{indent}oracle_fault_guard::begin({addr}U, "recovered");')
        w(f"{indent}if (setjmp(*oracle_fault_guard::buffer()) == 0) {{")
        if ret == "void":
            w(f"{indent}    {call};")
        else:
            w(f"{indent}    recovered_result = {call};")
        w(f"{indent}    snapshot(after_recovered);")
        w(f"{indent}    oracle_fault_guard::end();")
        w(f"{indent}}} else {{")
        w(f"{indent}    oracle_fault_guard::end();")
        w(f"{indent}    restore(before);")
        w(f'{indent}    std::printf("  {row["name"]}: the RECOVERED body faulted '
          f'where the original did not\\n");')
        w(f'{indent}    timing({addr}U, GetTickCount() - started_at, "{row["name"]}");')
        w(f'{indent}    verdict({addr}U, "FAIL-faulted", "{row["name"]}");')
        w(f"{indent}    return false;")
        w(f"{indent}}}")
        w(f"{indent}restore(before);   // leave the process as it was found")
        if ret != "void":
            w(f"{indent}if (original_result != recovered_result) {{")
            w(f'{indent}    std::printf("  {row["name"]}: return value differs\\n");')
            w(f"{indent}    passed = false;")
            w(f"{indent}}}")
        w(f"{indent}uintptr_t where = 0;")
        w(f"{indent}if (!same_globals(after_original, after_recovered, &where)) {{")
        w(f'{indent}    std::printf("  {row["name"]}: globals differ, first at %p\\n",')
        w(f"{indent}                reinterpret_cast<void *>(where));")
        w(f"{indent}    passed = false;")
        w(f"{indent}}}")
        if staged:
            # Report WHICH field, for the same reason the globals comparison
            # does: "staged object differs" names a function, and the offset
            # names the field, which is what turns a FAIL into a work item.
            w(f"{indent}size_t staged_at = 0;")
            w(f"{indent}if (!globals_diff::equal(staged_original, staged,")
            w(f"{indent}                         ObjectSize, &staged_at)) {{")
            w(f'{indent}    std::printf("  {row["name"]}: staged object differs '
              f'at +0x%X (original 0x%02X, recovered 0x%02X)\\n",')
            w(f"{indent}                (unsigned)staged_at,")
            w(f"{indent}                staged_original[staged_at], staged[staged_at]);")
            w(f"{indent}    passed = false;")
            w(f"{indent}}}")
        # EFFECT DETECTION. Did the ORIGINAL actually do anything observable
        # from this seed? If no seed ever did, the two bodies agreed without
        # either of them running, and that is not evidence.
        w(f"{indent}uintptr_t moved = 0;")
        w(f"{indent}if (!same_globals(before, after_original, &moved)) {{")
        w(f"{indent}    observed_effect = true;")
        w(f"{indent}}}")
        if staged:
            w(f"{indent}if (std::memcmp(staged_seed, staged_original, ObjectSize) != 0) {{")
            w(f"{indent}    observed_effect = true;")
            w(f"{indent}}}")
        if ret != "void":
            w(f"{indent}if (original_result != ({ret})0) {{")
            w(f"{indent}    observed_effect = true;")
            w(f"{indent}}}")
        w("    }")
        # Timed on every exit, including the ones that return early: a function
        # that is slow because it bailed at a guard and a function that is slow
        # because it ran are different problems, and only the per-function number
        # tells them apart.
        w(f'    timing({addr}U, GetTickCount() - started_at, "{row["name"]}");')
        w("    if (!passed) {")
        w(f'        verdict({addr}U, "FAIL", "{row["name"]}");')
        w("        return false;")
        w("    }")
        w("    if (!observed_effect) {")
        # Not a failure of the suite - the bodies DID agree - but not a proof
        # either, and the verdict line is what stops it becoming one.
        w(f'        std::printf("  {row["name"]}: no seed produced an observable '
          f'effect\\n");')
        w(f'        verdict({addr}U, "INCONCLUSIVE-no-effect", "{row["name"]}");')
        w("        return true;")
        w("    }")
        w(f'    verdict({addr}U, "PASS", "{row["name"]}");')
        w("    return true;")
        w("}")
        w("")

    w("bool run_generated_signature_oracles() {")
    # Armed here, not in DllMain: this suite is reached only through
    # run_deferred_oracles(), from the scenario_opening_movie call site, on the
    # game's own thread and after the executable's CRT is up - not under the
    # loader lock. The guard gates on the thread that armed it.
    w("    oracle_fault_guard::arm();")
    w("    bool passed = true;")
    for row in rows:
        fn = f"verify_{row['symbol']}_{row['address']:08x}"
        w(f"    passed &= {fn}();")
    w(f"    std::printf(\"generated signature oracles: %d function(s)\\n\", {len(rows)});")
    w("    return passed;")
    w("}")
    return "\n".join(out) + "\n"


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--proven", type=Path, default=DEFAULT_PROVEN)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--dllmain", type=Path, default=DEFAULT_DLLMAIN)
    parser.add_argument("--headers", type=Path, default=DEFAULT_HEADERS,
                        help="directory of src/*.h to read pinned class sizes from")
    parser.add_argument("--verdicts", type=Path,
                        help="a suite run's output; PASS lines in it earn markers")
    parser.add_argument("--check", action="store_true",
                        help="fail if the output would differ; write nothing")
    parser.add_argument("--list", action="store_true",
                        help="print the candidates and stop")
    options = parser.parse_args(argv)

    sizes = pinned_class_sizes(options.headers)
    # Classes with no pinned sizeof but a known UPPER bound. Staging only needs
    # a buffer the method cannot run off the end of, and the receiver is a
    # fresh static array rather than the real global, so over-allocating is
    # harmless while under-allocating corrupts. Read from a committed
    # catalogue because deriving it needs the uncommitted executable and
    # --check runs where there is none.
    import derive_class_size_bounds
    bounds = derive_class_size_bounds.load()
    bounded = {name: size for name, size in bounds.items() if name not in sizes}
    sizes = dict(sizes)
    sizes.update(bounded)
    rows = candidates(options.functions, options.proven,
                      redirected_addresses(options.dllmain), sizes,
                      {name: "bounded" for name in bounded})
    earned = earned_markers(options.proven, options.output, options.verdicts)
    if options.list:
        for row in rows:
            kind = row["class"] or "free"
            mark = "proven" if row["address"] in earned else "-"
            print(f"0x{row['address']:08X}  {row['size']:5d} B  "
                  f"{len(row['args'])} arg  {kind:14s} {mark:7s} {row['name']}")
        members = [row for row in rows if row["class"] is not None]
        print(f"{len(rows)} candidate(s), "
              f"{sum(row['size'] for row in rows)} bytes; "
              f"{len(members)} staged member(s), "
              f"{len(rows) - len(members)} free; "
              f"{len(earned & {row['address'] for row in rows})} carry a marker")
        return 0

    text = emit(rows, earned)
    if options.check:
        current = options.output.read_text() if options.output.exists() else ""
        if current != text:
            print(f"error: {options.output} is stale; regenerate it",
                  file=sys.stderr)
            return 1
        print(f"generated signature oracles: {len(rows)} (up to date)")
        return 0
    options.output.write_text(text)
    print(f"generated {len(rows)} oracle(s) -> {options.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
