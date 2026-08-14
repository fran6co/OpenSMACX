#!/usr/bin/env python3
"""Recompile every already-recovered body with VC6 and record what it scores.

This costs ZERO agent time. 2,518 bodies are already committed under `src/`
with a catalogued `source_locations`, so the whole corpus can be put through the
oracle for the price of a batched compile - measured at 2 ms per file through a
response file, against 44 ms per isolated `wine CL` invocation.

WHAT IT IS FOR, in order of how much it is worth:

1. **The size-stratified match rate.** The four known matches are 17, 30, 39 and
   54 bytes. The remaining population has a median of 209 B and a mean of 709 B,
   and there is NO evidence of any kind about a 200-byte body. Failure
   probability is not linear in size: an eighty-instruction body needs every
   instruction to be the form VC6 would have chosen, and both known non-matches
   were caused by source FORM rather than semantics - which is exactly what
   scales badly. This is the cheapest way to learn that, and it runs before
   agent time is spent rather than after.

2. **It tests the emitter.** The census builds its units with the same
   `emit_translation_unit` scaffolding the fan-out will use, so the NO_COMPILE
   histogram is the emitter's defect list rather than a mystery.

3. **It finds real bugs.** `?bitmask@@YAXHPAHPAH@Z` divided signed in the
   original and was recovered as `uint32_t` - a difference invisible to any
   oracle over non-negative inputs. Byte matching found it on first contact.

WHAT IT IS NOT. These bodies were written to BEHAVE, not to MATCH, and the
authoring style `AGENTS.md` mandates is byte-visible: `volatile` field access
costs two instructions, a rebindable seam turns `call rel32` into
`call dword ptr [X]`, and a global reached through `extern int *g` costs a
`mov`. So a low match rate here is a statement about the SOURCES, not about the
compiler, and must be reported as one.

Two populations are therefore split in the output and must never be totalled
together: 1,183 of the 2,518 live in GENERATED files - `init_thunks.cpp`,
`atexit_thunks.cpp`, `adjustor_thunks.cpp` and the rest - and are uniformly in
seam-pointer style. Mixing them with hand-written bodies produces a headline
number that means nothing.

Output holds addresses, counts, tier names and mnemonic NAMES. No opcode byte
reaches it, so it is committable under the same rule the rest of
`docs/recovery/` follows.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import csv
import functools
import collections
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402

import byte_match  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import declfix  # noqa: E402
import src_declarations  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
# THE LEDGER IS A CACHE, NOT A RECORD, and it lives under the ignored
# `.opensmacx/` for that reason. `tools/decomp_status.py` recomputes every row
# from `src/` and the compiler in about two minutes, and the one thing that
# must SURVIVE a delete - "this body was proved byte-exact" - is a `BYTE_EXACT`
# claim on the annotation itself. Committing 1.5 MB of derived verdicts made
# the tree carry a second answer to a question `src/` already answers, and
# they drifted: one row claimed BYTE_EXACT for a body reset to
# `// BODY GOES HERE.`.
LEDGER = REPO_ROOT / ".opensmacx" / "byte-match.csv"

# Bodies that cannot go through a VC6 compile and should be refused by NAME
# rather than debugged one at a time.
REFUSE_SUBSTRINGS = (
    ("__asm", "inline assembly: VC6 and GCC spell it differently"),
)

# `std::` USED TO BE REFUSED HERE, on the ground that "VC6's headers differ
# enough to be noise". Half of that is true - VC6 predates the C++98 rule that
# `<cstring>` puts `memcpy` in namespace `std`, so `std::memcpy` really does
# not compile against its headers - and the other half stopped being true
# without anyone noticing: `src/vc6_compat.h` has carried the shim that fixes
# it for the whole product build for a long time. The census was refusing 43
# bodies that the DLL compiles every day.
#
# So the shim is READ OUT OF THAT HEADER rather than copied. A second copy
# would drift, and this file has no business having its own opinion about
# which CRT names the tree uses.
VC6_COMPAT = REPO_ROOT / "src" / "vc6_compat.h"
STD_BLOCK = re.compile(r"^namespace std \{\nusing ::.*?^\}", re.M | re.S)


@functools.lru_cache(maxsize=2)
def std_shim(needs_string: bool = False) -> str:
    """`src/vc6_compat.h`'s `namespace std` using-declarations, verbatim.

    Returns "" when the header cannot be read or no longer holds the block, in
    which case a `std::` body simply fails to compile - the state it was in
    before, reported as a verdict instead of as a refusal.

    `<string>` IS CONDITIONAL, and it has to be. The C headers declare
    functions and nothing else, but VC6's `<string>` instantiates
    `std::ctype<unsigned short>` and emits `?id@?$ctype@G@std@@$E` as a second
    external symbol - and `byte_match.object_code` refuses a unit holding two,
    because a helper defined beside the subject would be inlined into it. So
    the header that costs a symbol is included only for a body that names
    something in it.
    """
    try:
        text = VC6_COMPAT.read_text(errors="ignore")
    except OSError:
        return ""
    found = STD_BLOCK.search(text)
    if not found:
        return ""
    return ("#include <math.h>\n#include <stdarg.h>\n#include <stdio.h>\n"
            "#include <stdlib.h>\n#include <string.h>\n"
            + ("#include <string>\n" if needs_string else "")
            + found.group(0) + "\n")


# Names that only `<string>` declares. Anything else `std::` reaches in this
# tree is a C function the using-declarations above republish.
STRING_HEADER_NAMES = ("std::string", "std::wstring", "std::to_string",
                       "std::basic_string", "std::stringstream",
                       "std::ostringstream", "std::istringstream")

GENERATED_FILES = (
    "init_thunks.cpp", "atexit_thunks.cpp", "adjustor_thunks.cpp",
    "deleting_thunks.cpp", "delegation_thunks.cpp", "nullsub_thunks.cpp",
    "field_accessors.cpp", "global_arith.cpp", "generated_signature_oracle.cpp",
)

FIELDS = ("address", "name", "source_location", "generated", "size",
          "span_classes", "tier", "original_bytes", "rebuilt_bytes",
          "original_mnemonics", "rebuilt_mnemonics", "first_divergence",
          "note", "refusal_reason")


def body_span(source: str):
    """(path, lines, start, end) for a catalogued `src/foo.cpp:123` location.

    `end` is INCLUSIVE and indexes the closing brace's line; both indices are
    0-based into `lines`. Split out of `extract_body` so a writer can replace
    exactly the span this reader compiles - `tools/writeback.py` puts a
    byte-exact Mizuchi match back into `src/`, and it must not carry a second
    opinion about where a body ends.

    Measured over all 2,518 rows carrying a source location: every one extracts,
    every one ends in `}`, none has a brace inside a string or char literal, and
    none has more than one column-zero closing brace. The brace counter is more
    robust than it looks because this tree's style happens to satisfy its
    assumptions - which is a fact about the tree, not about the parser, so the
    invariants are asserted per row rather than trusted.
    """
    path_part, _, line_part = source.rpartition(":")
    path = REPO_ROOT / path_part
    lines = path.read_text().splitlines()
    start = int(line_part) - 1
    # ONE READER. This counted braces itself, line by line and blind to
    # comments and string literals, and stopped at the FIRST return to depth
    # zero - so a helper class or an edited VCall shim ahead of the definition
    # truncated the body, and a `{` inside a RULED-OUT note did the same.
    # `test_annotation_scan.ExtractorEquivalence` pins these two together
    # precisely so the copy cannot drift, and it is the test that caught this
    # change being made in one place only.
    import annotation_scan
    end = annotation_scan.region_end(lines, start)
    if end is None:
        raise ValueError(f"{source}: no closing brace within the file")
    return path, lines, start, end


def extract_body(source: str) -> str:
    """The committed recovery verbatim, from its catalogued line to its `}`.

    `functions.csv` points at a line inside the doc comment that precedes the
    definition, so the extract usually opens mid-comment; it is re-opened with
    `/*` rather than trimmed, because trimming would make this tool decide
    where a body starts.
    """
    _, lines, start, end = body_span(source)
    text = "\n".join(lines[start:end + 1]) + "\n"
    if "*/" in text.split("{", 1)[0]:
        text = "/*\n" + text
    return text


def census_rows(functions: dict) -> list:
    out = []
    for address, row in sorted(functions.items()):
        if row.get("recovery_state") != "source_complete":
            continue
        location = (row.get("source_locations") or "").split(";")[0].strip()
        out.append((address, row, location))
    return out


# The vocabulary `src/` uses to call into the original image, mirrored because
# the scaffolding unit cannot include the tree's headers - it is built from
# opaque shells and fixed-address globals precisely so it does not depend on
# them (see src/recovered/README.md).
#
# WITHOUT THIS, 792 OF 2,184 NO_COMPILE ROWS ARE ONE MISSING DECLARATION. 590
# bodies use `ORIGINAL(...)`, the pointer-to-member seam that replaced the
# `__thiscall` typedefs VC6 refuses, and 202 more use the EH vector iterators.
# Every one of them failed with C2065 and, until the census started recording
# what CL said, failed indistinguishably from everything else.
#
# Mirrors src/original_seam.h and src/vector_teardown.h. Kept minimal and
# emitted only for bodies that reference it, so a unit that compiled before
# still compiles to the same bytes.
SEAM_PREAMBLE = """
class __single_inheritance OriginalObject;
template <class Method>
Method original_method(unsigned long address) {
  union { unsigned long address; Method method; } cast;
  cast.address = address;
  return cast.method;
}
#define ORIGINAL(pointer) (reinterpret_cast<OriginalObject *>(pointer))
typedef void (OriginalObject::*func_thiscall_teardown)();
typedef void(__stdcall func_vector_dtor_iterator)(
    void *array, unsigned int element_size, int count,
    func_thiscall_teardown teardown);
extern func_vector_dtor_iterator *VectorDtorIterator;
typedef void(__stdcall func_vector_ctor_iterator)(
    void *array, unsigned int element_size, int count,
    func_thiscall_teardown ctor, func_thiscall_teardown dtor);
extern func_vector_ctor_iterator *VectorCtorIterator;
"""

SEAM_TRIGGERS = ("ORIGINAL(", "original_method", "VectorDtorIterator",
                 "VectorCtorIterator", "OriginalObject")

# Spellings `src/` uses that cl 12.00.8168 does not have without a header.
# `nullptr` is C++11 and `vc6_compat.h` defines it away for the real build; the
# scaffolding unit includes no headers at all, so it needs the same courtesy.
#
# Emitted PER SYMBOL and only when the body names it. A blanket preamble would
# risk `C2371: redefinition; different basic types` against any scaffolding
# that already declares one of these, which would break units that compile
# today - and this file's whole job is to measure, so it must not change what
# it is measuring.
COMPAT_DECLARATIONS = {
    "nullptr": "#ifndef nullptr\n#define nullptr 0\n#endif",
    "LPSTR": "typedef char *LPSTR;",
    "LPCSTR": "typedef const char *LPCSTR;",
    "BOOL": "typedef int BOOL;",
}


def compat_preamble(body: str, scaffolding: str, source_path=None) -> str:
    """Declarations the body needs that neither VC6 nor the scaffolding has.

    Two sources, in this order. The hand-written table above covers spellings
    that exist in no header of this repository - `nullptr`, the Win32 typedefs
    - and stays small for that reason. Everything else is DERIVED from `src/`
    itself by `src_declarations`, because a body that calls its own
    translation unit's helper is not a recovery defect and must not be scored
    as one; see that module's docstring for the 638-piece measurement that
    made it necessary.

    A declaration is added only for a name the scaffolding does not already
    carry, so a unit that compiles today gains nothing, keeps its text byte
    for byte, and keeps its cached verdict.
    """
    wanted = []
    for name, declaration in COMPAT_DECLARATIONS.items():
        if re.search(rf"\b{re.escape(name)}\b", body) and name not in scaffolding:
            wanted.append(declaration)
    # THE CRT TABLE IS DERIVED FROM, NOT COPIED. `declfix.CRT_SIGNATURES` is
    # already this tree's record of what a CRT routine's arity is, and the
    # emitter reaches it for a CATALOGUED CALLEE. A body that calls `memcpy`
    # where the original INLINED it has no such callee, so nothing declared it
    # and the unit failed `C2065` - 11 bodies on `memcpy`, 8 on `_stricmp`,
    # and a long tail. Restating them here would be a second hand-maintained
    # list of the same facts, which is this repository's highest-yield defect
    # shape; reading the one table means today's additions to it reach both
    # consumers at once.
    #
    # DECLARED, NOT INCLUDED. Pulling in the real `<string.h>` would also arm
    # the intrinsics, and VC6 lowers an intrinsic `memcpy` of a constant size
    # to `rep movsd` - which is exactly the divergence an agent had to cast a
    # function pointer to avoid on 0x00422F20.
    for key, signature in declfix.CRT_SIGNATURES.items():
        # BOTH SPELLINGS. The table is keyed by the DECORATED name because
        # that is what the image's import carries, and a body may call either
        # - `_stricmp` decorated, `memcpy` not. `\b` finds no boundary inside
        # `_stricmp`, so testing only the stripped spelling missed every
        # underscored call, which is 8 bodies on `_stricmp` alone.
        for spelled in (key, key.lstrip("_")):
            if not re.search(rf"(?<![\w])({re.escape(spelled)})\s*\(", body):
                continue
            if re.search(rf"(?<![\w]){re.escape(spelled)}\b", scaffolding):
                break
            head, _, tail = signature.partition("(")
            wanted.append(
                f'extern "C" {head.rsplit(key, 1)[0]}{spelled}({tail};')
            break
    fixed = ("\n".join(wanted) + "\n") if wanted else ""
    derived = src_declarations.for_body(body, scaffolding + fixed, source_path)
    return fixed + derived


@functools.lru_cache(maxsize=1)
def file_mode_units() -> dict:
    """{address: repo-relative path} for recoveries whose FILE is the unit.

    A `FILE` marker means the whole file is the translation unit - it carries
    its own typedefs, globals, callee declarations and classes, and `region`
    is the file. There is no body to extract and nothing to scaffold around
    it, which is exactly what this census used to try: it read the file from
    line 1, got a text that begins with the annotation comment and does not
    end in a brace, and REFUSED it.

    That is not a missed row, it is a FALSE DEMOTION written into a shared
    ledger. Measured on 2026-08-14: one run turned 606 rows that other tools
    had scored BYTE_EXACT into REFUSED, 1,727 in total, and any histogram
    taken afterwards describes a population with the tree's best recoveries
    cut out of it. `verify_recovered_function` had already learned this for
    its own path; the census had not.
    """
    import annotation_scan
    return {a.address: a.path for a in annotation_scan.scan_tree()
            if a.mode == annotation_scan.MODE_FILE}


def build_unit(address, row, location, functions, derived, callees, pe):
    """(unit text, refusal reason)."""
    if not location:
        return None, "no source_locations; not censusable"
    verbatim = file_mode_units().get(address)
    if verbatim is not None:
        text = (REPO_ROOT / verbatim).read_text(errors="ignore")
        for needle, why in REFUSE_SUBSTRINGS:
            if needle in text:
                return None, why
        return text, ""
    try:
        body = extract_body(location)
    except (ValueError, OSError, FileNotFoundError) as error:
        return None, f"extract failed: {error}"
    if not body.rstrip().endswith("}"):
        return None, "extract does not end in a closing brace"
    for needle, why in REFUSE_SUBSTRINGS:
        if needle in body:
            return None, why
    try:
        scaffolding = emit.emit(address, functions, derived, callees, pe,
                                scaffolding_only=True)
    except emit.Unsettled as error:
        return None, f"no scaffolding: {error}"
    seam = SEAM_PREAMBLE if any(t in body for t in SEAM_TRIGGERS) else ""
    # The shim goes FIRST: it includes real CRT headers, and the scaffolding
    # below typedefs names those headers also declare.
    shim = std_shim(any(n in body for n in STRING_HEADER_NAMES)) \
        if "std::" in body else ""
    # The SEAM is part of what the unit already declares, so it is handed to
    # the preamble too - otherwise `OriginalObject` and the two iterator seams
    # would be redeclared and the unit would fail C2011 where it compiles now.
    compat = compat_preamble(body, scaffolding + seam,
                             location.split(":")[0] if location else None)
    return shim + scaffolding + seam + compat + "\n" + body, ""


# Per-process state for the unit-building pool. Each worker loads the
# catalogue once and keeps it; the alternative is pickling 6,000 rows and a
# parsed PE across the wire for every one of 4,790 tasks.
_WORKER = {}


def _worker_init() -> None:
    _WORKER["functions"] = emit.load_functions()
    _WORKER["derived"] = emit.load_derived()
    _WORKER["callees"] = emit.load_callees()
    _WORKER["pe"] = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)


def _worker_build(task):
    address, row, location = task
    return address, build_unit(address, row, location, _WORKER["functions"],
                               _WORKER["derived"], _WORKER["callees"],
                               _WORKER["pe"])


def build_units(subjects: list, jobs: int, functions, derived, callees, pe):
    """{address: (text, refusal)} for every subject, across `jobs` processes.

    MEASURED 2026-08-14: building the 4,790 units took 665 s of straight-line
    Python - two thirds of the census wall clock, and none of it the compiler
    the tool exists to run. `emit.emit` re-derives a scaffold per address and
    `src_declarations.for_body` re-walks the headers per body; both are pure
    functions of files that do not change during a run, so the work divides
    with no shared state at all.

    A worker pays about ten seconds to load the catalogue, so the pool is only
    worth building when there is real work for it - one process is faster for a
    `--limit 20` spot check than sixteen that each read the catalogue first.
    """
    if jobs <= 1 or len(subjects) < 64:
        return {address: build_unit(address, row, location, functions,
                                    derived, callees, pe)
                for address, row, location in subjects}
    out = {}
    with concurrent.futures.ProcessPoolExecutor(
            max_workers=jobs, initializer=_worker_init) as pool:
        for address, result in pool.map(_worker_build, subjects,
                                        chunksize=16):
            out[address] = result
    return out


def run(limit: int, jobs: int, verbose: bool) -> int:
    functions = emit.load_functions()
    derived = emit.load_derived()
    callees = emit.load_callees()
    pe_fast = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    pe = pefile.PE(str(byte_match.DEFAULT_EXE))
    shared = byte_match.shared_span_index(byte_match.load_rows())

    subjects = census_rows(functions)
    if limit:
        subjects = subjects[:limit]
    print(f"census over {len(subjects)} source_complete functions")

    units, results = {}, {}
    built = build_units(subjects, jobs, functions, derived, callees, pe_fast)
    for address, row, location in subjects:
        stem = f"c{address:08x}"
        text, refusal = built[address]
        # `source_locations` is "src/foo.cpp:123", so the line number has to
        # come off before matching a filename. Leaving it on made every row
        # report as hand-written, which would have merged the 1,183 generated
        # seam-style bodies into the headline the docstring says must never
        # total them together.
        source_file = location.rpartition(":")[0] or location
        generated = any(source_file.endswith(name) for name in GENERATED_FILES)
        base = {"address": f"0x{address:08X}", "name": row.get("name", ""),
                "source_location": location, "generated": int(generated),
                "size": row.get("size", ""), "tier": "", "note": "",
                "refusal_reason": refusal, "span_classes": "",
                "original_bytes": "", "rebuilt_bytes": "",
                "original_mnemonics": "", "rebuilt_mnemonics": "",
                "first_divergence": ""}
        if refusal:
            base["tier"] = "REFUSED"
        else:
            units[stem] = text
        results[stem] = base

    # Body layouts depend only on the original image, not on the compile
    # flags, so compute them once; the flag loop below only re-compiles.
    layouts = {}
    pending = set()
    for stem in units:
        address = int(stem[1:], 16)
        entry = results[stem]
        layout = byte_match.classify_body(pe, functions[address], shared)
        entry["span_classes"] = (
            f"primary={len(layout.primary)};eh={len(layout.eh)};"
            f"shared={len(layout.shared)};selfmod={len(layout.selfmod)}")
        if layout.refusal:
            entry.update(tier="REFUSED", refusal_reason=layout.refusal)
        elif layout.shared:
            entry.update(tier="SHARED_TAIL", refusal_reason="COMDAT-folded span")
        else:
            layouts[stem] = layout
            pending.add(stem)

    # The flag set is NOT one-size-fits-all - that is the whole lesson in
    # byte_match.FLAG_SETS, whose own docstring records that a fixed `/Oy-`
    # is right for only ~half the image and guarantees an instruction-#0
    # divergence for the rest. So try every flag set, keep the BEST verdict
    # per unit (byte_match._better ranks tier, then closeness), and stop
    # re-compiling a unit once it is BYTE_EXACT.
    best = {}
    for flags in byte_match.FLAG_SETS:
        if not pending:
            break
        stems = sorted(pending)
        chunks = [{s: units[s] for s in stems[start:start + jobs]}
                  for start in range(0, len(stems), jobs)]
        # Concurrent, one temporary directory per batch. The flag SETS stay
        # sequential on purpose - each pass re-reads `pending`, which the
        # previous pass shrinks, and running them together would recompile
        # units that are already BYTE_EXACT.
        for objects, diagnostics in byte_match.compile_batches(chunks, flags):
            for stem, data in objects.items():
                if data is None:
                    # WHAT CL SAID, not merely that it said something. "CL
                    # emitted no object" was recorded for all 1,988
                    # NO_COMPILE rows, and it is true of every compile
                    # failure there has ever been, so it separates nothing.
                    candidate = {
                        "tier": "NO_COMPILE",
                        "refusal_reason": diagnostics.get(
                            stem, "CL emitted no object")}
                else:
                    low, high = layouts[stem].primary[0]
                    try:
                        original = byte_match.original_span_bytes(pe, low, high)
                        mask = byte_match.original_relocation_mask(pe, low, high)
                        rebuilt, rebuilt_mask = byte_match.object_code(data)
                    except ValueError as error:
                        candidate = {"tier": "NO_COMPILE",
                                     "refusal_reason": str(error)}
                    else:
                        candidate = byte_match.compare(original, mask, low,
                                                       rebuilt, rebuilt_mask)
                if stem not in best or byte_match._better(candidate, best[stem]):
                    best[stem] = candidate
                if best[stem]["tier"] == "BYTE_EXACT":
                    pending.discard(stem)
        print(f"  after {flags!r}: {len(pending)} unit(s) short of BYTE_EXACT",
              flush=True)

    for stem, verdict in best.items():
        results[stem].update(
            tier=verdict["tier"], note=verdict.get("note", ""),
            original_bytes=verdict.get("original_bytes", ""),
            rebuilt_bytes=verdict.get("rebuilt_bytes", ""),
            original_mnemonics=verdict.get("original_mnemonics", ""),
            rebuilt_mnemonics=verdict.get("rebuilt_mnemonics", ""),
            first_divergence=verdict.get("first_divergence", ""),
            # `refusal_reason` was built above and then not written. Every
            # column this touches is listed here by hand, so a verdict field
            # missing from the list is dropped silently - which is what
            # happened, for every NO_COMPILE row this census has ever
            # produced.
            refusal_reason=verdict.get("refusal_reason", ""))

    # MERGE, never overwrite. The fan-out writes unrecovered rows into this
    # same ledger; a census re-run that truncated the file would erase every
    # banked match without failing anything.
    existing = {}
    if LEDGER.is_file():
        with LEDGER.open() as handle:
            existing = {row["address"]: row for row in csv.DictReader(handle)}
    for entry in results.values():
        existing[entry["address"]] = entry
    LEDGER.parent.mkdir(parents=True, exist_ok=True)
    with LEDGER.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=FIELDS)
        writer.writeheader()
        for key in sorted(existing):
            writer.writerow({field: existing[key].get(field, "")
                             for field in FIELDS})
    print(f"wrote {LEDGER} ({len(existing)} rows, "
          f"{len(results)} touched by this run)")
    report(list(results.values()))
    return 0


def report(rows: list) -> None:
    def bucket(size):
        size = int(size or 0)
        for limit, label in ((64, "  <=64 B"), (256, "  <=256 B"),
                             (1024, "  <=1 kB"), (4096, "  <=4 kB")):
            if size <= limit:
                return label
        return "  > 4 kB"

    print("\nverdict by tier")
    tiers = collections.Counter(r["tier"] for r in rows)
    for tier, count in tiers.most_common():
        print(f"  {tier:14s} {count:5d}")

    matched = {"BYTE_EXACT"}
    for label, wanted in (("hand-written", 0), ("generated", 1)):
        subset = [r for r in rows if r["generated"] == wanted
                  or r["generated"] == str(wanted)]
        if not subset:
            continue
        hit = sum(1 for r in subset if r["tier"] in matched)
        print(f"\n{label}: {hit}/{len(subset)} at BYTE_EXACT")
        by_size = collections.defaultdict(lambda: [0, 0])
        for entry in subset:
            slot = by_size[bucket(entry["size"])]
            slot[1] += 1
            if entry["tier"] in matched:
                slot[0] += 1
        for key in ("  <=64 B", "  <=256 B", "  <=1 kB", "  <=4 kB", "  > 4 kB"):
            if key in by_size:
                hit, total = by_size[key]
                print(f"  {key:10s} {hit:4d}/{total:4d}"
                      f"  {100 * hit // max(total, 1):3d}%")

    reasons = collections.Counter(
        r["refusal_reason"][:60] for r in rows
        if r["tier"] in ("NO_COMPILE", "REFUSED") and r["refusal_reason"])
    if reasons:
        print("\nwhy rows did not reach a verdict (the emitter's to-do list)")
        for reason, count in reasons.most_common(10):
            print(f"  x{count:4d}  {reason}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--limit", type=int, default=0)
    parser.add_argument("--jobs", type=int, default=200,
                        help="units per CL response file; 200 measured best")
    parser.add_argument("--verbose", action="store_true")
    arguments = parser.parse_args()

    reason = byte_match.available()
    if reason:
        print(f"SKIP: {reason}. This needs Visual C++ 6.0 (12.00.8168) under "
              f"Wine and the pinned executable.")
        return 0
    return run(arguments.limit, arguments.jobs, arguments.verbose)


if __name__ == "__main__":
    sys.exit(main())
