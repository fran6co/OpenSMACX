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
import csv
import collections
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402

import byte_match  # noqa: E402
import emit_translation_unit as emit  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
LEDGER = REPO_ROOT / "docs" / "recovery" / "byte-match.csv"

# Bodies that cannot go through a VC6 compile and should be refused by NAME
# rather than debugged one at a time.
REFUSE_SUBSTRINGS = (
    ("__asm", "inline assembly: VC6 and GCC spell it differently"),
    ("std::", "C++ standard library: VC6's headers differ enough to be noise"),
)

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
    exactly the span this reader compiles - `tools/mizuchi_writeback.py` puts a
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
    depth, opened = 0, False
    for offset, line in enumerate(lines[start:]):
        depth += line.count("{") - line.count("}")
        if "{" in line:
            opened = True
        if opened and depth <= 0:
            return path, lines, start, start + offset
    raise ValueError(f"{source}: no closing brace within the file")


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


def compat_preamble(body: str, scaffolding: str) -> str:
    """Declarations the body needs that neither VC6 nor the scaffolding has."""
    wanted = []
    for name, declaration in COMPAT_DECLARATIONS.items():
        if re.search(rf"\b{re.escape(name)}\b", body) and name not in scaffolding:
            wanted.append(declaration)
    return ("\n".join(wanted) + "\n") if wanted else ""


def build_unit(address, row, location, functions, derived, callees, pe):
    """(unit text, refusal reason)."""
    if not location:
        return None, "no source_locations; not censusable"
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
    compat = compat_preamble(body, scaffolding)
    return scaffolding + seam + compat + "\n" + body, ""


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
    for address, row, location in subjects:
        stem = f"c{address:08x}"
        text, refusal = build_unit(address, row, location, functions, derived,
                                   callees, pe_fast)
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
        with tempfile.TemporaryDirectory() as tmp:
            work = Path(tmp)
            stems = sorted(pending)
            for start in range(0, len(stems), jobs):
                chunk = {s: units[s] for s in stems[start:start + jobs]}
                objects = byte_match.compile_batch(chunk, work, flags)
                diagnostics = getattr(byte_match.compile_batch,
                                      "diagnostics", {})
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
