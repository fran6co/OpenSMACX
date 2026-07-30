#!/usr/bin/env python3
"""Whole-image static recompilation: emit every game function as C++.

The incremental recovery path cannot resolve indirect control flow. 5,064
indirect call sites and 373 indirect jumps live in 1,542 of the 5,673 game
functions (27.2%); `call eax` has no answer while only a fraction of the image
exists in source. Lifting *everything* inverts that: once every original
address has a lifted counterpart, an indirect call is a lookup in a table that
is total by construction. The same move deletes the data problem - map the
image at its canonical base and absolute operands simply work, with no
per-global seam - and reduces the ABI surface from 5,673 hand-derived
prototypes to the 221 imports at the boundary.

This tool runs in two modes.

`--mode skeleton` is a SCALE PROBE, and it is the first thing to run. It emits
the real skeleton - memory model, dispatch table, import shims, one TU shard
per group of functions - but fills each function body with placeholder
statements at a realistic density rather than correct lowerings. The question
it answers is not "is the translation right" but "does an artifact this size
compile and link at all", which is the assumption every later phase rests on
and the cheapest one to falsify. Emitting correct code first and discovering
at 5 million lines that the toolchain will not link it is the expensive
ordering.

`--mode lower` is the real translation and is the default. Every decoded
instruction goes through `x86_lower.lower`; an instruction the lowerer refuses
becomes ONE `opensmacx_trap` naming it, and the rest of the function is lowered
around it. Abandoning a body at its first refusal would be the easier rule and
a much worse one: a function that is 99% translated with a single trap in it
still runs until it reaches that trap, and the trap says what to build next,
while a stub says only that something was missing. The refusals are counted by
mnemonic and that histogram, printed and stored in the manifest, is phase 4's
work queue - so it is a measurement, not a summary, and it comes from calling
the lowerer rather than from any table of what the lowerer is believed to
support.

Nothing here is distributable. Output is mechanically derived from the user's
own executable and lands under build/, which is ignored, exactly as
docs/STATIC_RECOMPILATION.md and docs/LEGACY_ISLANDS.md require.
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
import time
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from generator_support import read_bytes  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from capstone.x86 import X86_OP_IMM  # noqa: E402

import jump_tables  # noqa: E402
import x86_lower  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

# Sections that are not ordinary code and must not be lifted. _SELFMOD is
# executable AND writable and holds two copies of one four-function stub that
# rewrites itself; a static translation of self-modifying code is a
# contradiction, so those addresses are emitted as traps and the decision about
# what replaces them is deferred to the point where the game is booting and can
# say whether it needs them at all.
UNLIFTABLE_SECTIONS = {"_selfmod"}

# The headers a lowered shard needs. They are listed rather than chained
# through a single umbrella include so that each one stays a separate,
# separately testable piece of the runtime: lifted_x86.h is the integer core,
# and the other five are separately verified pieces beside it - the double
# shifts, the multiply/divide register pairs, the fs:-relative thread block,
# the string operations' implicit ESI/EDI/ECX, and the x87 stack. Only the
# double shifts fit the integer core's operand model; the rest are there
# because they do not. Every one is self-contained, so the order here is
# presentation only.
LOWER_HEADERS = ("lifted_x86.h", "lifted_dblshift.h", "lifted_muldiv.h",
                 "lifted_tls.h", "lifted_string.h", "lifted_x87.h")

# Statements emitted per decoded instruction in skeleton mode. A real lowering
# of a mid-range instruction - operand fetch, the operation, flag updates,
# writeback - lands around three, so this is the honest figure rather than a
# flattering one. Raise it to see how much headroom the toolchain has.
STATEMENTS_PER_INSTRUCTION = 3


def section_of(pe: pefile.PE, address: int) -> str:
    base = pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        begin = base + section.VirtualAddress
        end = begin + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if begin <= address < end:
            return section.Name.decode("ascii", "replace").rstrip("\0").lower()
    return ""


def image_span(pe: pefile.PE) -> tuple[int, int]:
    """(base, size) covering every section, so one flat array models memory."""
    base = pe.OPTIONAL_HEADER.ImageBase
    end = base
    for section in pe.sections:
        section_end = (base + section.VirtualAddress
                       + max(section.Misc_VirtualSize, section.SizeOfRawData))
        end = max(end, section_end)
    return base, ((end - base) + 0xFFF) & ~0xFFF


def body_spans(row: dict) -> list[tuple[int, int]]:
    """The half-open [start, end) ranges a function's code actually occupies.

    `size` is the SUM of these spans, not `end_address - address`, and reading
    that many bytes from the entry point is wrong for every function MSVC
    outlined. 402 of the 5,673 game functions (7.1%) carry a second span in the
    0x0065xxxx-0x0066xxxx cold region, and for all 402 a contiguous read runs
    past the first span into the NEXT function - by up to 2,102 bytes. The
    functions it hits hardest are the largest in the game: probe (28,292 B),
    base_production (16,430 B), tech_trade, terraform, upgrade.

    In skeleton mode that error only inflated an instruction count. Under real
    lowering it would translate a neighbour's bytes as part of this body, so
    the spans are authoritative here and `size` is used only to cross-check
    them. The same confusion was fixed once already for adjustor thunks in
    commit 5e4c362.
    """
    spans = []
    for part in (row.get("body_ranges") or "").split(";"):
        if "-" not in part:
            continue
        low, high = part.split("-", 1)
        try:
            spans.append((int(low, 16), int(high, 16)))
        except ValueError:
            continue
    if not spans:
        address = int(row["address"], 16)
        spans = [(address, address + int(row["size"] or 0))]
    spans.sort()
    return spans


def load_external_library(functions_csv: Path) -> list[dict]:
    """The rows load_functions drops: the CRT the lift calls and never lifts.

    They are dropped from the LIFT and they are not dropped from the PROGRAM -
    a direct call to one is a real call that has to go somewhere. Emitting the
    catalogue's address and name for each is what lets tools/lifted_crt.cpp
    supply them, and what lets the dispatcher name `__itoa` rather than an
    address when it cannot.
    """
    with functions_csv.open() as handle:
        rows = list(csv.DictReader(handle))
    externals = []
    for row in rows:
        if row["recovery_state"] != "external_library":
            continue
        if not int(row["size"] or 0):
            continue
        externals.append({"address": int(row["address"], 16),
                          "name": row["name"]})
    externals.sort(key=lambda entry: entry["address"])
    return externals


def load_functions(pe: pefile.PE, functions_csv: Path) -> list[dict]:
    with functions_csv.open() as handle:
        rows = list(csv.DictReader(handle))
    functions = []
    for row in rows:
        # external_library rows are CRT and Windows code that the lift must
        # call, never translate: they are supplied by the host toolchain.
        if row["recovery_state"] == "external_library":
            continue
        size = int(row["size"] or 0)
        if not size:
            continue
        address = int(row["address"], 16)
        spans = body_spans(row)
        functions.append({
            "address": address,
            "size": size,
            "spans": spans,
            "span_bytes": sum(high - low for low, high in spans),
            "name": row["name"],
            "section": section_of(pe, address),
            "state": row["recovery_state"],
        })
    functions.sort(key=lambda item: item["address"])
    return functions


def count_instructions(pe: pefile.PE, functions: list[dict]) -> None:
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    for function in functions:
        total = 0
        covered = 0
        for low, high in function["spans"]:
            instructions = list(md.disasm(read_bytes(pe, low, high - low), low))
            total += len(instructions)
            covered += sum(one.size for one in instructions)
        function["instructions"] = total
        # Capstone stopping early means the range holds data, or an encoding it
        # does not know. Either way the byte count is the honest denominator,
        # so fall back to it rather than silently under-budgeting the shard.
        function["fully_decoded"] = covered >= function["span_bytes"] * 0.95
        if not function["fully_decoded"]:
            function["instructions"] = max(total, function["span_bytes"] // 3)


def symbol_for(function: dict) -> str:
    return f"lifted_{function['address']:08x}"


RUNTIME_HEADER = """\
#ifndef OPENSMACX_LIFTED_RUNTIME_H
#define OPENSMACX_LIFTED_RUNTIME_H

#include <cstdint>

#include "static_recompile_runtime.h"

// The whole original image lives in one flat array based at its canonical
// address, so an absolute operand needs no relocation, no per-global binding
// and no class layout: it is an index. This is the single largest
// simplification the whole-image approach buys over per-function recovery,
// where every global costs a hand-declared seam.
extern unsigned char opensmacx_image[];

constexpr uint32_t OpensmacxImageBase = %(base)#010xU;
constexpr uint32_t OpensmacxImageSize = %(size)#010xU;

inline unsigned char *opensmacx_at(uint32_t address) {
    return opensmacx_image + (address - OpensmacxImageBase);
}

inline uint32_t opensmacx_mem32(uint32_t address) {
    uint32_t value;
    __builtin_memcpy(&value, opensmacx_at(address), sizeof(value));
    return value;
}

inline void opensmacx_store32(uint32_t address, uint32_t value) {
    __builtin_memcpy(opensmacx_at(address), &value, sizeof(value));
}

typedef void (*OpensmacxLiftedFunction)(OpensmacxStaticRecompileState &);

// Total by construction: every catalogued game address has an entry, which is
// what makes an indirect call expressible at all.
OpensmacxLiftedFunction opensmacx_dispatch(uint32_t address);

// Reached when a lifted body meets something not yet translated. In skeleton
// mode that is every body; once lowering starts, each trap is a stack trace
// naming exactly one address, which is the whole debugging loop for phase 5.
[[noreturn]] void opensmacx_trap(uint32_t address, const char *reason);

// Called FIRST by opensmacx_trap, before the message and the abort, so that a
// harness can take the trap over and unwind instead of dying. Null by default,
// which is exactly the old behaviour.
//
// THIS EXISTS BECAUSE OVERRIDING THE SYMBOL DOES NOT WORK, and the way it
// failed is worth writing down. tools/lifted_oracle_build.sh used to run
// `objcopy --weaken-symbol` on lifted_dispatch.cpp.o so that the oracle's own
// strong opensmacx_trap would win. It won for the SHARDS, whose calls carry a
// relocation - and it could not win for opensmacx_dispatch, because the trap
// is defined in the same translation unit and -O2 INLINES it: the emitted
// dispatcher contains the fprintf and the abort with no relocation and no
// symbol left to redirect. So every lifted body that reached a callee with no
// lifted body killed the whole harness instead of being recorded, which is
// precisely the case a runtime "did this path actually execute" test needs.
// Measured before the fix: `--run-anyway` over the extcall cohort printed
// `opensmacx: trap at 0x00644f3a: indirect call to an address with no
// function` and wrote zero report rows.
//
// A hook is immune to inlining because it is a load and an indirect call that
// survive into the inlined copy. It also replaces the objcopy step rather than
// joining it: one mechanism, and it is visible in the source.
extern void (*opensmacx_trap_hook)(uint32_t address, const char *reason);

#endif
"""


def write_runtime(out: Path, base: int, size: int) -> None:
    (out / "lifted_runtime.h").write_text(
        RUNTIME_HEADER % {"base": base, "size": size}, encoding="utf-8")


def write_image(out: Path, pe: pefile.PE, base: int, size: int) -> None:
    """Emit the image as initialized data.

    Committing the bytes of the original executable into a C array is exactly
    the derived-code problem the project's policy is about, so the array is
    zero-filled here and populated at startup from the user's own file. The
    generated tree therefore carries the image's SHAPE, never its contents.
    """
    source = out / "lifted_image.cpp"
    source.write_text(
        "#include \"lifted_loader.h\"\n\n"
        f"// {size} bytes of image spanning {base:#010x}..{base + size:#010x},\n"
        "// with the guest stack and its guard band in the same allocation\n"
        "// directly above, so opensmacx_at() stays one subtraction.\n"
        "// Zero-filled on purpose: the bytes are loaded at startup from the\n"
        "// user's own executable so no original content enters this tree.\n"
        "unsigned char opensmacx_image[OpensmacxImageSize + OpensmacxStackSpanSize];\n"
        "\n"
        "// The stack geometry in lifted_loader.h is derived from\n"
        "// OpensmacxImageSize, not from this array, so the two could drift\n"
        "// apart silently: lifted_runtime.h declares opensmacx_image as an\n"
        "// incomplete type, which means a translation unit that only sees the\n"
        "// declaration can address the stack without any diagnostic. This is\n"
        "// the one place that sees the definition, so it is the only place\n"
        "// that can tie them together.\n"
        "static_assert(sizeof(opensmacx_image) == OpensmacxSpanSize,\n"
        "              \"opensmacx_image must cover the image AND the guest \"\n"
        "              \"stack span the loader hands out\");\n",
        encoding="utf-8")


def body_statements(function: dict) -> list[str]:
    """Placeholder statements at the density a real lowering would produce."""
    lines = []
    address = function["address"]
    total = function["instructions"] * STATEMENTS_PER_INSTRUCTION
    for index in range(total):
        slot = index % 6
        if slot == 0:
            lines.append(f"    state.eax = opensmacx_mem32(state.esp + {index % 64});")
        elif slot == 1:
            lines.append(f"    state.ecx = state.eax + {index % 251}U;")
        elif slot == 2:
            lines.append("    opensmacx_cmp32(state, state.eax, state.ecx);")
        elif slot == 3:
            lines.append(f"    opensmacx_store32(state.esp + {index % 32}, state.ecx);")
        elif slot == 4:
            lines.append("    state.edx ^= state.ecx;")
        else:
            lines.append(f"    state.esi = opensmacx_mem32({address:#010x}U);")
    return lines


def decode_body(pe: pefile.PE, function: dict, md: Cs) -> list[tuple]:
    """The function's body as ('code', instruction) and ('data', span) items.

    Capstone stopping short of the end of a span is not an error to swallow:
    the remaining bytes are either data the catalogue included in the body or
    an encoding capstone does not know, and both need to be visible. They are
    carried through as a 'data' item so the emitter can trap on them in
    position rather than dropping them and producing a body that silently
    skips a stretch of the original.
    """
    items: list[tuple] = []
    for low, high in function["spans"]:
        data = read_bytes(pe, low, high - low)
        consumed = 0
        for one in md.disasm(data, low):
            items.append(("code", one))
            consumed += one.size
        if consumed < len(data):
            items.append(("data", (low + consumed, len(data) - consumed)))
    return items


def branch_targets(items: list[tuple]) -> set[int]:
    """Addresses this function branches to from inside itself.

    Only these get labels. A branch to anything else leaves the function and
    has to become a dispatch, which is why the set is computed from the
    function's own instruction starts rather than from every branch target:
    `goto` cannot cross a function boundary, and a tail call must not become
    one. CALL is excluded even when it targets this same function - a
    self-recursive call is still a call, and lowering it as a jump would skip
    the pushed return address.
    """
    starts = {item[1].address for item in items if item[0] == "code"}
    targets = set()
    for kind, payload in items:
        if kind != "code" or not payload.mnemonic.startswith("j"):
            continue
        operands = payload.operands
        if (len(operands) == 1 and operands[0].type == X86_OP_IMM
                and operands[0].imm in starts):
            targets.add(operands[0].imm)
    return targets


def internal_call_targets(items: list[tuple], entries: set) -> list[int]:
    """Addresses this function CALLS inside its own body.

    `branch_targets` deliberately excludes CALL - a call is a call even when it
    lands in the same function, and lowering it as a `goto` would skip the
    pushed return address - so these lower to `opensmacx_dispatch(target)`. The
    dispatch table holds catalogued function ENTRIES only, so a target that is
    merely an address inside a body has no entry and traps at run time.

    MSVC does this for the EH vector iterators, which CALL an unwind funclet
    embedded in their own body. Measured over the whole image: exactly TWO such
    sites, in `??_L` (0x006457c2) and `??_M` (0x006456e4). Both are on the boot
    path, because the C++ dynamic initialisers construct arrays.

    Returning them lets write_shards emit each as an additional entry point, so
    the dispatch is total for these as well.
    """
    starts = {item[1].address for item in items if item[0] == "code"}
    found = set()
    for kind, payload in items:
        if kind != "code" or payload.mnemonic != "call":
            continue
        operands = payload.operands
        if (len(operands) == 1 and operands[0].type == X86_OP_IMM
                and operands[0].imm in starts
                and operands[0].imm not in entries):
            found.add(operands[0].imm)
    return sorted(found)


def lower_body(items: list[tuple], tables: dict | None = None
               ) -> tuple[list[str], Counter]:
    """(emitted lines, refusals by mnemonic) for one function.

    One refusal is one trap and the surrounding instructions are still
    lowered. The trap is reachable code, so the function runs correctly right
    up to the missing instruction and then names it - which is both a more
    useful artifact than a whole-body stub and the only way the histogram
    below reflects what is actually missing rather than what is missing in the
    first function that happens to contain it.
    """
    targets = branch_targets(items)
    # Switch arms need labels too, and `branch_targets` cannot see them: it
    # reads immediate operands, and a switch's destinations live in a data
    # table. Intersecting with the decoded instruction starts is the same
    # guard `branch_targets` applies - a target that is not one gets no label,
    # so the lowerer refuses the jump and it becomes a trap rather than a
    # `goto` into the middle of an instruction.
    starts = {item[1].address for item in items if item[0] == "code"}
    for case_targets in (tables or {}).values():
        targets |= {target for target in case_targets if target in starts}

    def label_for(address: int):
        return f"L_{address:08x}" if address in targets else None

    lines: list[str] = []
    refused: Counter = Counter()
    for kind, payload in items:
        if kind == "data":
            address, length = payload
            refused["(undecodable)"] += 1
            lines.append(f"    opensmacx_trap({address:#010x}U,"
                         f" \"{length} undecodable bytes\");")
            continue
        instruction = payload
        if instruction.address in targets:
            # The trailing `;` is not cosmetic: in C++17 a label must be
            # followed by a statement, and the next lowering is legally empty
            # (a NOP emits nothing) or absent (the label is the last thing in
            # the body).
            lines.append(f"L_{instruction.address:08x}: ;")
        try:
            statements = x86_lower.lower(
                instruction, label_for, (tables or {}).get(instruction.address))
        except x86_lower.Unsupported:
            refused[instruction.mnemonic] += 1
            lines.append(f"    opensmacx_trap({instruction.address:#010x}U,"
                         f" \"{instruction.mnemonic}\");")
            continue
        lines.extend("    " + statement for statement in statements)
    return lines, refused


def write_shards(out: Path, functions: list[dict], shards: int,
                 mode: str = "skeleton", pe: pefile.PE | None = None
                 ) -> list[Path]:
    buckets: list[list[dict]] = [[] for _ in range(shards)]
    for index, function in enumerate(functions):
        buckets[index % shards].append(function)
    entry_addresses = {function["address"] for function in functions}

    # The lowerer needs operand detail, which the counting pass deliberately
    # does without: detail costs roughly double, and only this mode reads it.
    decoder = None
    if mode == "lower":
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
    # `s` is what the lowerer's emitted expressions name; skeleton bodies were
    # written against `state` and are left alone.
    parameter = "s" if mode == "lower" else "state"
    headers = LOWER_HEADERS if mode == "lower" else ("lifted_runtime.h",)

    paths = []
    for number, bucket in enumerate(buckets):
        path = out / f"lifted_{number:03d}.cpp"
        parts = ["".join(f"#include \"{name}\"\n" for name in headers)
                 + "\n"]
        for function in bucket:
            symbol = symbol_for(function)
            ranges = ";".join(f"{low:#010x}-{high:#010x}"
                              for low, high in function["spans"])
            parts.append(
                f"// {function['name'][:70]}\n"
                f"// {ranges}  {function['span_bytes']} bytes, "
                f"{function['instructions']} instructions\n"
                f"void {symbol}(OpensmacxStaticRecompileState &{parameter}) {{\n")
            if function["section"] in UNLIFTABLE_SECTIONS:
                function["refused"] = Counter()
                function["lowered"] = False
                parts.append(
                    f"    (void){parameter};\n"
                    f"    opensmacx_trap({function['address']:#010x}U,"
                    " \"self-modifying section\");\n}\n\n")
                continue
            if mode == "lower":
                items = decode_body(pe, function, decoder)
                tables = jump_tables.resolve_decoded(
                    pe, function["spans"],
                    [item[1] for item in items if item[0] == "code"])
                lines, refused = lower_body(items, tables)
                function["refused"] = refused
                function["lowered"] = True
                parts.append("\n".join(lines))
                # Falling off the end of a body means the catalogued extent
                # ended before the control flow did. Unreachable after a RET,
                # so the compiler drops it in the ordinary case and keeps it
                # exactly where it is a real answer.
                parts.append(
                    f"\n    opensmacx_trap({function['address']:#010x}U,"
                    " \"fell off the end of the body\");\n}\n\n")
                # An embedded funclet this body CALLS. Emitted as its own
                # entry point by lowering the suffix of the same items, which
                # duplicates those instructions in two translation units'
                # worth of code and is the honest trade: the alternative is a
                # goto into the middle of another C++ function, which does not
                # exist. The funclet ends in RET, so the suffix terminates.
                for extra in internal_call_targets(items, entry_addresses):
                    suffix = [item for item in items
                              if (item[1].address if item[0] == "code"
                                  else item[1][0]) >= extra]
                    extra_lines, _ = lower_body(suffix, tables)
                    function.setdefault("extra_entries", []).append(extra)
                    parts.append(
                        f"// embedded funclet of {function['name'][:50]}, "
                        f"called from inside it\n"
                        f"void lifted_{extra:08x}"
                        f"(OpensmacxStaticRecompileState &{parameter}) {{\n")
                    parts.append("\n".join(extra_lines))
                    parts.append(
                        f"\n    opensmacx_trap({extra:#010x}U,"
                        " \"fell off the end of the funclet\");\n}\n\n")
                continue
            parts.append("\n".join(body_statements(function)))
            parts.append(
                f"\n    opensmacx_trap({function['address']:#010x}U,"
                " \"not lowered\");\n}\n\n")
        path.write_text("".join(parts), encoding="utf-8")
        paths.append(path)
    return paths


# __cinit states the bounds of both initialiser arrays as push immediates, so
# they can be read rather than guessed:
#
#   push 0x6826e4 / push 0x6826d0 / call __initterm    ; .CRT$XI, FIRST
#   push 0x6826cc / push 0x682000 / call __initterm    ; .CRT$XC, SECOND
#
# Order matters and it is not the order a name-based guess would pick. XI holds
# the C initialisers - ___onexitinit, ___initstdio, ___initmbctable - and XC the
# C++ ones. Nothing in XC can safely run before ___onexitinit has built the
# atexit table those constructors register with, which is exactly why a boot
# driven by the `??__E` name filter stops in __onexit.
CINIT_ADDRESS = 0x00644DD2
INITTERM_ADDRESS = 0x00644ED8
# __cinit is external_library, so load_functions deliberately drops it and it
# cannot be looked up in the lift's own function list. Its catalogued size is 45
# bytes; 64 covers it and the decode stops at the second __initterm regardless.
CINIT_EXTENT = 64


def crt_init_bounds(pe) -> list[tuple[str, int, int]]:
    """[(section, begin, end)] for the two arrays, decoded from `__cinit`.

    Read from the image, not hardcoded and not inferred from names. A name
    filter cannot do this job and the measurement says how badly: of the 434
    live .CRT$XC entries, 42 are not named `??__E` - `sub_440f30`, a run of
    twenty-eight `sub_48d5xx` - and NONE of the four .CRT$XI entries are.
    """
    code = read_bytes(pe, CINIT_ADDRESS, CINIT_EXTENT)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    pushed: list[int] = []
    bounds: list[tuple[str, int, int]] = []
    for instruction in md.disasm(code, CINIT_ADDRESS):
        if instruction.mnemonic == "push":
            try:
                pushed.append(int(instruction.op_str, 0))
            except ValueError:
                pushed.clear()
        elif instruction.mnemonic == "call":
            try:
                target = int(instruction.op_str, 0)
            except ValueError:
                target = None
            if target == INITTERM_ADDRESS and len(pushed) >= 2:
                # cdecl pushes right to left, so the LAST push is the FIRST
                # argument: __initterm(begin, end). Reading it the other way
                # round yields a negative span, so the ordering is asserted
                # rather than trusted - I got it backwards once.
                begin, end = pushed[-1], pushed[-2]
                if begin >= end:
                    raise SystemExit(
                        f"__initterm range {begin:#010x}..{end:#010x} is not "
                        f"ascending; the push order was misread")
                bounds.append(("XI" if not bounds else "XC", begin, end))
            pushed.clear()
    if len(bounds) != 2:
        raise SystemExit(
            f"__cinit at {CINIT_ADDRESS:#010x} did not yield two __initterm "
            f"ranges; got {bounds}")
    return bounds


def write_crt_init(out: Path, functions: list[dict], pe=None) -> None:
    """The `??__E` C++ dynamic initialisers, so a boot can run them.

    `start` is external_library and cannot be lifted, so nothing runs the CRT
    startup - and CRT startup is what walks the .CRT$XC pointer array calling
    these. Skipping them leaves every global they construct as the zeroed bytes
    the image was loaded with, which is precisely why the ORIGINAL side of the
    oracle faults on 68.11% of image bytes and why 391 of them fault inside one
    address. This table is the input to fixing that.

    Addresses only. The ORDER is not taken from here - address order is not
    initialisation order - it is recovered at run time from the image's own
    pointer array, for which this set is the membership test.
    """
    bounds = crt_init_bounds(pe) if pe is not None else []
    initialisers = sorted(function["address"] for function in functions
                          if function["name"].startswith("??__E"))
    entries = "\n".join(f"    {address:#010x}U," for address in initialisers)
    ranges = "\n".join(
        f'    {{"{name}", {begin:#010x}U, {end:#010x}U}},'
        for name, begin, end in bounds)
    (out / "lifted_crt_init.h").write_text(
        f"""#ifndef OPENSMACX_LIFTED_CRT_INIT_H
#define OPENSMACX_LIFTED_CRT_INIT_H

// Generated by tools/lift_whole_image.py - edit that, not this.

#include <cstdint>

// The two arrays __cinit walks, in the order it walks them. XI first: it holds
// ___onexitinit, which builds the atexit table every C++ initialiser in XC then
// registers with, so running XC first stops in __onexit reading a table that
// does not exist yet.
struct OpensmacxCrtInitRange {{
    const char *name;
    uint32_t begin;
    uint32_t end;
}};

inline const OpensmacxCrtInitRange OpensmacxCrtInitRanges[] = {{
{ranges}
}};

inline constexpr unsigned OpensmacxCrtInitRangeCount =
    sizeof(OpensmacxCrtInitRanges) / sizeof(OpensmacxCrtInitRanges[0]);

// Cross-check only: the catalogued `??__E` addresses. NOT the membership test
// for what to run - 42 live XC entries are not named `??__E` and none of the
// four XI entries are - so a walk driven by this set runs the wrong subset.
inline const uint32_t OpensmacxCrtInit[] = {{
{entries}
}};

inline constexpr unsigned OpensmacxCrtInitCount =
    sizeof(OpensmacxCrtInit) / sizeof(OpensmacxCrtInit[0]);

#endif  // OPENSMACX_LIFTED_CRT_INIT_H
""", encoding="utf-8")


def write_crt_table(out: Path, externals: list[dict]) -> None:
    """address -> catalogued name, for every external_library row."""
    entries = "\n".join(
        f'    {{{entry["address"]:#010x}U, "{entry["name"]}"}},'
        for entry in externals)
    (out / "lifted_crt_table.h").write_text(
        f"""#ifndef OPENSMACX_LIFTED_CRT_TABLE_H
#define OPENSMACX_LIFTED_CRT_TABLE_H

// Generated by tools/lift_whole_image.py - edit that, not this.
//
// The external_library rows of the catalogue: CRT and Windows library code the
// lift calls and never translates. Names come from the catalogue rather than
// from a hand-written list, so a shim cannot be bound to an address the
// catalogue does not agree about.

#include <cstdint>

struct OpensmacxCrtEntry {{
    uint32_t address;
    const char *name;
}};

inline const OpensmacxCrtEntry OpensmacxCrtTable[] = {{
{entries}
}};

inline constexpr unsigned OpensmacxCrtTableSize =
    sizeof(OpensmacxCrtTable) / sizeof(OpensmacxCrtTable[0]);

#endif  // OPENSMACX_LIFTED_CRT_TABLE_H
""", encoding="utf-8")


def write_dispatch(out: Path, functions: list[dict]) -> None:
    # Catalogued entries, plus the embedded funclets write_shards emitted as
    # extra entry points. Sorted together, because the table is binary-searched.
    rows = [(function["address"], symbol_for(function))
            for function in functions]
    for function in functions:
        for extra in function.get("extra_entries", ()):
            rows.append((extra, f"lifted_{extra:08x}"))
    rows.sort()
    declarations = "\n".join(
        f"void {symbol}(OpensmacxStaticRecompileState &);"
        for _, symbol in rows)
    entries = "\n".join(f"    {{{address:#010x}U, &{symbol}}},"
                         for address, symbol in rows)
    (out / "lifted_dispatch.cpp").write_text(
        f"""#include "lifted_runtime.h"
#include "lifted_imports.h"
#include "lifted_crt.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

{declarations}

namespace {{

struct Entry {{
    uint32_t address;
    OpensmacxLiftedFunction function;
}};

// Sorted by address at generation time, so the lookup is a binary search over
// a read-only table rather than a hash built at startup.
const Entry Table[] = {{
{entries}
}};

}}  // namespace

OpensmacxLiftedFunction opensmacx_dispatch(uint32_t address) {{
    // The PE imports live in a reserved synthetic range far above the image,
    // so one compare separates them from every lifted address and the shim is
    // found by indexing rather than searching. The range is a fixed size, not
    // derived from the import count, so this test never changes when the
    // import table does.
    if (opensmacx_is_import(address)) {{
        return opensmacx_import_dispatch(address);
    }}
    const Entry *const end = Table + (sizeof(Table) / sizeof(Table[0]));
    const Entry *const found = std::lower_bound(
        Table, end, address,
        [](const Entry &entry, uint32_t key) {{ return entry.address < key; }});
    if (found == end || found->address != address) {{
        // Two different origins land here and the reason names both, because a
        // consumer has to be able to tell this apart from an instruction the
        // lowerer refused. A DIRECT call or jmp to an address that is not a
        // catalogued entry - a CRT routine, an import thunk, the middle of a
        // neighbour - lowers to a dispatch on a constant and arrives here just
        // as an indirect call through a computed value does. Either way the
        // lift has no body to run, and saying so is the point.
        // Before giving up: the address may be one of the external_library
        // rows the lift drops by policy - the CRT. Those are real calls that
        // have to go somewhere, and tools/lifted_crt.cpp supplies the ones
        // written so far.
        if (OpensmacxLiftedFunction shim = opensmacx_crt_dispatch(address)) {{
            return shim;
        }}
        if (const char *name = opensmacx_crt_name(address)) {{
            // Named, so the work item is "__itoa" rather than an address.
            opensmacx_trap(address, name);
        }}
        opensmacx_trap(address, "call to an address with no lifted body");
    }}
    return found->function;
}}

// NOT __attribute__((weak)), and no longer overridden by symbol games either.
// On this PE target a weak DEFINITION with no strong alternate is simply
// dropped, and the whole image then fails to link with "undefined reference to
// opensmacx_trap" from every division site. The oracle used to weaken this
// symbol with objcopy on its own copy of the object; that worked for the
// shards and silently did NOT work for opensmacx_dispatch above, whose call is
// inlined at -O2 into a copy with no symbol left to redirect. The hook is the
// replacement, and it is checked before anything is printed so a harness can
// unwind out of a trap that would otherwise abort the process.
void (*opensmacx_trap_hook)(uint32_t address, const char *reason) = nullptr;

void opensmacx_trap(uint32_t address, const char *reason) {{
    if (opensmacx_trap_hook) opensmacx_trap_hook(address, reason);
    std::fprintf(stderr, "opensmacx: trap at %#010x: %s\\n",
                 static_cast<unsigned>(address), reason);
    std::abort();
}}
""", encoding="utf-8")


BUILD_SCRIPT = """\
#!/bin/sh
# Build the lifted image. Generated by tools/lift_whole_image.py - edit that,
# not this.
#
# -static is LOAD-BEARING and its absence fails in a way that looks unrelated
# to it: the image dies at load with status c0000135 before main runs, naming
# no symbol. Two separate dependencies pull it in - the 64-bit divide in
# lifted_muldiv.h reaches __divdi3/__udivdi3/__moddi3 in libgcc, and the
# thread_local TIB in lifted_tls.h reaches libwinpthread-1.dll.
# -static-libgcc alone covers only the first and still dies on the second.
set -e
# $CXX, then whatever is on PATH, then the Homebrew location as a last resort.
# That ordering matters and the old one - Homebrew path as the DEFAULT - is why
# this needed an explicit $CXX on every non-macOS host: an absolute path that
# does not exist is not a fallback, it is a failure with a confusing message.
CXX="${CXX:-$(command -v i686-w64-mingw32-g++ \\
    || echo /opt/homebrew/bin/i686-w64-mingw32-g++)}"
JOBS="${JOBS:-8}"
cd "$(dirname "$0")"
rm -f ./*.o lifted_probe.exe
ls lifted_*.cpp | xargs -P "$JOBS" -I{} \\
    "$CXX" -std=c++17 -O2 -c -I. -I%(tools)s {} -o {}.o
# -lgdi32 and -lwinmm are both required by the import shims and neither is in
# the default specs; without them the link fails naming _imp__GdiFlush@0 and
# _imp__timeGetTime@0 respectively. user32, advapi32 and kernel32 arrive
# through the default specs.
"$CXX" -std=c++17 -O2 -static -o lifted_probe.exe ./*.o -lgdi32 -lwinmm
echo "built lifted_probe.exe"
"""


def write_loader(out: Path, tools: Path) -> None:
    """Copy the image loader in so build.sh's lifted_*.cpp glob compiles it.

    Only the translation unit has to live in the build tree; lifted_loader.h
    stays in tools/ and is reached through build.sh's -I, exactly like
    lifted_x86.h and the other semantics headers.
    """
    (out / "lifted_loader.cpp").write_text(
        (tools / "lifted_loader.cpp").read_text(encoding="utf-8"),
        encoding="utf-8")


def write_crt_source(out: Path, tools: Path) -> None:
    """Copy the CRT shims in, for the same reason the loader is copied.

    build.sh globs lifted_*.cpp in the build tree, and lifted_dispatch.cpp now
    calls opensmacx_crt_dispatch, so the definition has to be inside that glob
    or every link fails undefined. The header stays in tools/ and is reached
    through -I, exactly like lifted_loader.h.
    """
    (out / "lifted_crt.cpp").write_text(
        (tools / "lifted_crt.cpp").read_text(encoding="utf-8"),
        encoding="utf-8")


def write_build_script(out: Path, tools: Path) -> None:
    path = out / "build.sh"
    path.write_text(BUILD_SCRIPT % {"tools": tools}, encoding="utf-8")
    path.chmod(0o755)


def write_main(out: Path, functions: list[dict]) -> None:
    """A driver whose only job is to keep the linker from discarding the lift."""
    probes = "\n".join(
        f"    resolved += (opensmacx_dispatch({function['address']:#010x}U) != nullptr);"
        for function in functions[:64])
    (out / "lifted_main.cpp").write_text(
        f"""#include "lifted_runtime.h"
#include "lifted_loader.h"
#include "lifted_imports.h"

#include <cstdio>

int main() {{
    // Order is load-bearing. The loader writes the file's own IAT contents
    // over the import slots, so binding must come after it; and every IAT slot
    // holds zero until binding runs, so the first call through an import would
    // dispatch to address 0 if any lifted code ran before it.
    opensmacx_load_image_or_die();
    opensmacx_bind_imports();

    OpensmacxStaticRecompileState state{{}};
    opensmacx_init_stack(state);

    int resolved = 0;
{probes}
    std::printf("resolved %d of 64 probes across %u bytes of image\\n",
                resolved, static_cast<unsigned>(OpensmacxImageSize));
    std::printf("image sha256 %s, entry %#010x, esp %#010x, guard %s\\n",
                opensmacx_image_sha256(),
                static_cast<unsigned>(opensmacx_entry_point()),
                static_cast<unsigned>(state.esp),
                opensmacx_stack_guard_intact() ? "intact" : "BREACHED");
    return (resolved == 64 && opensmacx_stack_guard_intact()) ? 0 : 1;
}}
""", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--functions", type=Path, default=FUNCTIONS_CSV)
    parser.add_argument("--out", type=Path,
                        default=REPO_ROOT / "build" / "lifted")
    parser.add_argument("--mode", choices=("skeleton", "lower"),
                        default="lower")
    parser.add_argument("--shards", type=int, default=64,
                        help="translation units, so the build parallelises")
    args = parser.parse_args()

    started = time.time()
    pe = pefile.PE(str(args.exe), fast_load=True)
    base, size = image_span(pe)
    functions = load_functions(pe, args.functions)
    count_instructions(pe, functions)

    args.out.mkdir(parents=True, exist_ok=True)
    write_runtime(args.out, base, size)
    write_image(args.out, pe, base, size)
    write_crt_table(args.out, load_external_library(args.functions))
    write_crt_init(args.out, functions, pe)
    write_main(args.out, functions)
    write_loader(args.out, Path(__file__).resolve().parent)
    write_crt_source(args.out, Path(__file__).resolve().parent)
    write_build_script(args.out, Path(__file__).resolve().parent)
    # BEFORE write_dispatch, and that order is load-bearing: lowering is what
    # discovers the embedded funclets that need their own entry points, and the
    # dispatch table has to contain them.
    shards = write_shards(args.out, functions, args.shards, args.mode, pe)
    write_dispatch(args.out, functions)

    lines = sum(
        len(path.read_text(encoding="utf-8").splitlines()) for path in shards)
    instructions = sum(function["instructions"] for function in functions)
    undecoded = sum(
        1 for function in functions if not function["fully_decoded"])
    unliftable = sum(1 for function in functions
                     if function["section"] in UNLIFTABLE_SECTIONS)
    outlined = sum(1 for function in functions if len(function["spans"]) > 1)

    # Coverage is MEASURED, by counting what the lowerer refused, not inferred
    # from a table of mnemonics it is believed to handle: `mov` is handled and
    # `mov eax, fs:[0]` is not, and only calling the lowerer knows the
    # difference. The histogram is ordered by frequency because that ordering
    # is the phase-4 work queue.
    refusals: Counter = Counter()
    fully = partial = 0
    for function in functions:
        counted = function.get("refused")
        if counted is None or not function.get("lowered"):
            continue
        refusals += counted
        if counted:
            partial += 1
        else:
            fully += 1
    traps = sum(refusals.values())
    histogram = dict(sorted(refusals.items(),
                            key=lambda item: (-item[1], item[0])))

    manifest = {
        "mode": args.mode,
        "image_base": f"{base:#010x}",
        "image_size": size,
        "functions": len(functions),
        "outlined_functions": outlined,
        "code_bytes": sum(function["span_bytes"] for function in functions),
        "instructions": instructions,
        # A budget in skeleton mode and a measurement in lower mode: the
        # skeleton picks the density, the lowering has one.
        "statements_per_instruction": (
            STATEMENTS_PER_INSTRUCTION if args.mode == "skeleton"
            else round(lines / instructions, 2) if instructions else 0),
        "generated_lines": lines,
        "shards": len(shards),
        "not_fully_decoded": undecoded,
        "unliftable_section_functions": unliftable,
        "fully_lowered_functions": fully,
        "partially_lowered_functions": partial,
        "traps": traps,
        "unsupported_mnemonics": histogram,
        "seconds": round(time.time() - started, 1),
    }
    (args.out / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    print(f"functions            {len(functions)}")
    print(f"instructions         {instructions}")
    print(f"generated lines      {lines} across {len(shards)} shards")
    print(f"image span           {base:#010x} + {size} bytes")
    print(f"not fully decoded    {undecoded}")
    print(f"unliftable section   {unliftable}")
    if args.mode == "lower":
        lowerable = fully + partial
        share = 100.0 * fully / lowerable if lowerable else 0.0
        print(f"fully lowered        {fully} of {lowerable} ({share:.1f}%)")
        print(f"partially lowered    {partial}")
        print(f"traps                {traps}")
        if histogram:
            print("\nunsupported mnemonics (the phase 4 queue)")
            for mnemonic, count in list(histogram.items())[:25]:
                print(f"  {count:>7}  {mnemonic}")
            if len(histogram) > 25:
                print(f"  ... {len(histogram) - 25} more, all in manifest.json")
    print(f"generation seconds   {manifest['seconds']}")
    print(f"\nwrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
