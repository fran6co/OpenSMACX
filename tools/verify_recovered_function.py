#!/usr/bin/env python3
"""Score ONE already-recovered function, without touching the tree.

`byte_match_census.py` scores all 2,518 committed bodies and takes minutes;
`byte_match_fanout.py --prepare` only ever queues functions that are NOT
recovered yet. Between them there was no way to ask "is this one body byte
exact, and if not where does it diverge" - which is the loop anyone improving
an existing recovery needs, and the loop an agent proposing a replacement needs
most of all.

The machinery already existed. `writeback.verify(address, body)`
rebuilds the unit and re-measures it; it simply had no command line. This is
that command line, plus the one thing it was missing: the ability to score a
CANDIDATE body instead of the committed one, so a proposed fix can be checked
before anything is written.

    tools/verify_recovered_function.py 0x006077F0
    tools/verify_recovered_function.py 0x006077F0 --body candidate.cpp
    cat candidate.cpp | tools/verify_recovered_function.py 0x006077F0 --body -

Exit code is 0 only for BYTE_EXACT, so it works directly as a loop condition -
the same contract `byte_match.py` uses, and the reason agent reports are never
taken as evidence: the claim and the measurement are different things, and this
is the measurement.

IT USES THE WRITEBACK RECIPE, NOT THE CENSUS RECIPE. The census omits
`declfix`, which respells callee declarations so VC6 re-mangles them to
the catalogued names; without it a body calling a CRT function reads as
NO_COMPILE. So a function can score NO_COMPILE in the census and BYTE_EXACT
here, and here is the one that matches what the integrator gates on.

THAT DIFFERENCE IS LOAD-BEARING, NOT AN ACCIDENT - measured, because it looks
exactly like one. Two recipes for one question is the shape this tree keeps
finding defects in, so declfix was added to the census recipe and the whole map
re-measured against a saved baseline. It is a REGRESSION: 64 bodies moved and
none improved - 61 MISMATCH -> NO_COMPILE and **3 BYTE_EXACT -> NO_COMPILE**.

The reason is the two body STYLES, not the two paths. A census body lives in
`src/*.cpp` and was written against the real declarations in `src/*.h`, so
respelling its callees from the catalogue's mangled names contradicts the header
it actually calls. A writeback body is emitter-style and was written against the
generated scaffolding, where the catalogue mangling IS the declaration. Same
step, opposite effect.

So the recipes stay two, and the honest fix is at the other end: whatever an
agent iterates on must be the style it will be scored in.
`decomp_status.work_address` now applies declfix when it materialises a scaffold,
which makes the `--work` file and this tool agree by construction.

`--dir` SCORES MANY SPELLINGS AT ONCE, and that is the mode to reach for.
Matching decompilation is a search over SOURCE FORM: the semantics get decided
early, and the rest of the work is finding which of several equivalent spellings
VC6 lowers the way the original was lowered - ternary against `if`, condition
polarity, a temp that changes an operand's addressing mode, a loop counting the
other way. There is no reasoning all the way to the answer. The compiler has to
be asked, and the useful question is "which of these nine" rather than "is this
one it".

    tools/verify_recovered_function.py 0x006281B0 --dir /tmp/variants

Every `*.cpp` in the directory is one candidate body, scored against the same
loaded image and catalogue and ranked best-first. Measured on four candidates:
14.5 s that way against 20.9 s one at a time, so about a third off - the saving
is the image load, not the compiles, and it barely grows with more candidates.
Speed is not the point though. The point is that "which of these nine" is one
question with one answer, and an agent that can ask it stops guessing.

This mode exists BECAUSE AN AGENT WROTE IT FOR ITSELF: recovering 0x006281B0
took nine structural variants to find that binding a component to a local flips
VC6's fmul scheduling, and the agent built its own batch harness mid-task to get
there - spending budget on tooling instead of on the problem. It arrived as a
separate `try_variants.py`, which was a second command asking the same question
through the same `writeback.build_unit` and `byte_match.match_function`, with no
test and no CMake registration. Folding it in leaves one scoring command and
puts it under the tests the other mode already had.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match  # noqa: E402
import byte_match_census as census  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import writeback as writeback  # noqa: E402

MATCHED = "BYTE_EXACT"

# BYTE EXACTNESS IS NOT THE ONLY THING THAT MATTERS. A body can reproduce the
# instruction stream and still be the wrong thing to have in the tree, and the
# compiler cannot tell the difference - so the form is checked separately.
#
# The first entry is a REFUSAL, not a warning: `__asm` pastes the original's
# instructions instead of deriving them, so it proves nothing about the source,
# and `AGENTS.md:5` bars copied machine code from distributable builds.
# `writeback.py` deliberately carries no content policy, so without a
# check here an `__asm` body that happens to be byte-exact lands in `src/`
# unchallenged. That has happened once already.
FORBIDDEN = (
    (r"\b__asm\b|\b_emit\b",
     "inline assembly: pastes the original's instructions instead of deriving "
     "them (AGENTS.md:5)"),
)
QUESTIONABLE = (
    (r"\(\s*(?:[^)]*,)?\s*uint32_t\s+\w+\s*[,)]",
     "uint32_t parameter: AGENTS.md:87 says parameters are int, and the "
     "signedness is visible in the branch the compiler picks"),
    (r"0x00[45678][0-9A-Fa-f]{5}\b",
     "a fixed image address written into the body: the tree reaches those "
     "through a rebindable seam"),
    (r"\bgoto\b", "goto"),
)


def code_only(body: str) -> str:
    """`body` with comments and string literals blanked out.

    THE RULE IS ABOUT WHAT COMPILES, so it has to be read where the compiler
    reads. Scanning the raw text refuses a candidate for the word `__asm`
    appearing in a `// RULED-OUT:` note that EXPLAINS WHY INLINE ASM IS NOT AN
    OPTION - the annotation grammar asks for exactly that prose, and an agent
    hit it on 0x0063E860 and got out by rewording the comment, which is the
    tool teaching a body to lie about itself.

    Blanked rather than deleted so nothing shifts: every offset in the text
    stays where it was, which keeps any position a caller reports honest.
    """
    import re

    def blank(match):
        return re.sub(r"\S", " ", match.group(0))

    # Block comments, line comments, then string and character literals, in
    # that order: a `//` inside a string is not a comment, and a quote inside
    # a comment does not open one, so the first pass has to remove comments
    # before quotes are looked at.
    text = re.sub(r"/\*.*?\*/", blank, body, flags=re.S)
    text = re.sub(r"//[^\n]*", blank, text)
    return re.sub(r'"(?:\\.|[^"\\])*"' r"|'(?:\\.|[^'\\])*'", blank, text)


def form_report(body: str) -> tuple:
    """(hard refusals, things worth a second look)."""
    import re
    code = code_only(body)
    refusals = [why for pattern, why in FORBIDDEN if re.search(pattern, code)]
    notes = [why for pattern, why in QUESTIONABLE if re.search(pattern, code)]
    return refusals, notes


def _operand_similarity(original: bytes, base: int, rebuilt: bytes) -> float:
    """How close the two instruction streams are WITH operands, addresses aside.

    `mnemonic_similarity` is blind to operands, so nine candidates that all emit
    the same opcodes in the same order tie at 1.000 and the ranking falls back to
    filename. Comparing `mnemonic op_str` with long hex normalised to ADDR -
    relocated operands are masked in the verdict anyway - is what separates a
    spelling that got the addressing mode right from one that did not.
    """
    import difflib
    import re

    def rendered(code, at=0):
        out = []
        for instruction in byte_match.decode(code, at):
            operands = re.sub(r"0x[0-9a-f]{5,}", "ADDR", instruction.op_str)
            out.append(f"{instruction.mnemonic} {operands}")
        return out

    try:
        return difflib.SequenceMatcher(a=rendered(original, base),
                                       b=rendered(rebuilt)).ratio()
    except Exception:
        return 0.0


def score_all(address: int, bodies: dict) -> list:
    """[(name, verdict)] for every candidate, best first.

    Each verdict comes from `byte_match.match_function` on a unit assembled the
    way `writeback.verify` assembles it, so a tier here means exactly
    what the integrator's gate means. The only thing shared across candidates is
    the loaded image and catalogue, which is the expensive part.
    """
    import pefile
    import tempfile

    functions = emit.load_functions()
    callees = emit.load_callees()
    derived = emit.load_derived()
    scaffolding_pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)

    pe = pefile.PE(str(byte_match.DEFAULT_EXE))
    catalogue = byte_match.load_rows()
    shared = byte_match.shared_span_index(catalogue)

    # ONE `cl` PER FLAG SET, NOT ONE PER CANDIDATE. `match_function` sweeps four
    # flag sets for a single unit, so scoring N spellings costs 4N compiles and a
    # 24-variant sweep ran past two minutes - which is exactly when an agent
    # stops enumerating and starts guessing, the behaviour this mode exists to
    # prevent. `compile_batch` puts the whole directory through one response
    # file: measured by the agent that hit the wall, 5,760 candidates in about
    # three minutes and 32 in two seconds.
    #
    # The RANKING is done here; the VERDICT is not. The winner goes back through
    # `match_function`, which is the only thing that applies the SHARED_TAIL and
    # self-modifying refusals and the best-of rule across all four flag sets. A
    # fast ranking that quietly disagreed with the authoritative scorer would be
    # worse than a slow one.
    prepared, refused = {}, {}
    for name, body in sorted(bodies.items()):
        blocked, _ = form_report(body)
        if blocked:
            refused[name] = {"tier": "REFUSED", "refusal_reason": blocked[0]}
            continue
        try:
            prepared[name] = writeback.build_unit(address, body, functions,
                                                  callees, derived, scaffolding_pe)
        except emit.Unsettled as error:
            refused[name] = {"tier": "REFUSED",
                             "refusal_reason": f"no scaffolding: {error}"}

    row = catalogue.get(address)
    results = dict(refused)
    if prepared and row is not None:
        low, high = int(row["address"], 16), int(row["end_address"], 16)
        original = byte_match.original_span_bytes(pe, low, high)
        original_mask = byte_match.original_relocation_mask(pe, low, high)
        for flags in byte_match.FLAG_SETS:
            with tempfile.TemporaryDirectory() as directory:
                objects, diagnostics = byte_match.compile_batch(
                    prepared, Path(directory), flags)
            for name, data in objects.items():
                if data is None:
                    verdict = {"tier": "NO_COMPILE",
                               "refusal_reason": (diagnostics.get(name) or "")[:120]}
                else:
                    # A UNIT THAT COMPILES CAN STILL BE UNREADABLE, and
                    # `object_code` says so by raising: two external `.text`
                    # symbols, a body CL folded, an object with no code at all.
                    # `byte_match_census` has caught this since it was written;
                    # this path did not, so the exception left the process and
                    # took the agent's whole run with it. Reported on
                    # 2026-08-14 against `NewTechWin::NewTechWin`, where a
                    # class with an array member makes MSVC synthesise its own
                    # `??_H` array-construction COMDAT - a second symbol the
                    # agent did not write and could not see. It is a verdict,
                    # not a crash: the body did not produce comparable code.
                    try:
                        rebuilt, rebuilt_mask = byte_match.object_code(data)
                    except ValueError as error:
                        verdict = {"tier": "NO_COMPILE",
                                   "refusal_reason": str(error)[:200]}
                    else:
                        verdict = byte_match.compare(original, original_mask,
                                                     low, rebuilt,
                                                     rebuilt_mask)
                        verdict["operand_similarity"] = _operand_similarity(
                            original, low, rebuilt)
                if name not in results or byte_match._better(verdict, results[name]):
                    results[name] = verdict

    order = {tier: index for index, tier in enumerate(byte_match.TIER_ORDER)}
    ranked = sorted(results.items(),
                    key=lambda item: (order.get(item[1].get("tier"), 99),
                                      -(item[1].get("operand_similarity") or 0),
                                      -(item[1].get("mnemonic_similarity") or 0),
                                      item[0]))
    # The winner's verdict is re-taken authoritatively. Ranking above skips the
    # refusals `match_function` applies per address, which are the same for every
    # candidate but must still be applied to the answer.
    if ranked and ranked[0][1].get("tier") in ("BYTE_EXACT", "SHAPE_EXACT"):
        name = ranked[0][0]
        with tempfile.TemporaryDirectory() as directory:
            authoritative = byte_match.match_function(
                pe, catalogue, shared, address, prepared[name],
                Path(directory), "", f"v_{name}")
        results[name] = authoritative
        ranked = sorted(results.items(),
                        key=lambda item: (order.get(item[1].get("tier"), 99),
                                          -(item[1].get("operand_similarity") or 0),
                                          -(item[1].get("mnemonic_similarity") or 0),
                                          item[0]))
    return ranked


def file_mode_annotation(address: int):
    """The `FILE`-mode annotation for `address`, or None.

    A `FILE` marker means THE WHOLE FILE IS THE TRANSLATION UNIT: it carries
    its own typedefs, fixed-address globals, callee declarations and classes.
    `decomp_status` honours that with `recipe == "verbatim"` and compiles the
    text as it stands.

    This tool did not. Every path here funnels into `writeback.build_unit`,
    which ALWAYS regenerates scaffolding and appends the text it is given - so
    pointing it at a FILE-mode unit double-declares that unit's own
    boilerplate and reports `NO_COMPILE` (C2011/C2370) on a file that compiles
    perfectly well.

    MEASURED across batches 3-5: FIVE agents hit this independently, and one
    reproduced it against a control - bare `verify_recovered_function.py
    0x00404B00` reported NO_COMPILE on a COMMITTED, BYTE_EXACT file it had
    never touched. Agents worked around it by scoring a minimal candidate and
    landing the full file, which works and means the thing they score is not
    the thing they land: the same split that produced the FILE-marker bug and
    the `_catalog_facts` drift.
    """
    import annotation_scan
    for annotation in annotation_scan.scan_tree():
        if annotation.address == address:
            if annotation.mode == annotation_scan.MODE_FILE:
                return annotation
            return None
    return None


def verify_verbatim(annotation) -> dict:
    """Score a FILE-mode unit as it stands, the way the ratchet does.

    Deliberately the same call `writeback.verify` ends with, minus the
    `build_unit` step - because the unit already exists. Same comparator, same
    catalogue, same shared-span index, so a verdict here and a verdict from
    `decomp_status` are the same measurement rather than two that agree.
    """
    import tempfile
    import pefile
    pe = pefile.PE(str(byte_match.DEFAULT_EXE))
    catalogue = byte_match.load_rows()
    shared = byte_match.shared_span_index(catalogue)
    with tempfile.TemporaryDirectory() as tmp:
        return byte_match.match_function(
            pe, catalogue, shared, annotation.address, annotation.region,
            Path(tmp), "", f"v{annotation.address:08x}")


def committed_body(address: int) -> tuple:
    """(body text, source location) for the recovery on disk, or (None, why)."""
    functions = emit.load_functions()
    row = functions.get(address)
    if row is None:
        return None, f"0x{address:08X} is not in the catalogue"
    location = (row.get("source_locations") or "").split(";")[0].strip()
    if not location:
        return None, (f"0x{address:08X} has no source_locations, so nothing is "
                      f"recovered for it yet - use byte_match_fanout.py "
                      f"--prepare instead")
    try:
        return census.extract_body(location), location
    except (ValueError, OSError) as error:
        return None, f"{location}: {error}"


def rank_directory(address: int, directory: Path, as_json: bool) -> int:
    """Score every `*.cpp` in `directory` and print them best-first.

    Exit 0 only when the WINNER is BYTE_EXACT, which keeps the same contract
    the single-body mode has: the command is usable as a loop condition and a
    ranking that got closer without arriving is still a failure.
    """
    bodies = {path.stem: path.read_text()
              for path in sorted(directory.glob("*.cpp"))}
    if not bodies:
        print(f"error: no *.cpp found in {directory}", file=sys.stderr)
        return 2

    results = score_all(address, bodies)
    if as_json:
        import json
        print(json.dumps(
            {"address": f"0x{address:08X}",
             "candidates": [{"name": name, **verdict} for name, verdict in results]},
            indent=2, default=str))
    else:
        print(f"{len(bodies)} candidate(s) for 0x{address:08X}, best first:\n")
        for name, verdict in results:
            detail = verdict.get("note") or verdict.get("refusal_reason") or ""
            print(f"   {verdict.get('tier', '?'):14} {name:26} {detail[:54]}")
    if results and results[0][1].get("tier") == MATCHED:
        if not as_json:
            print(f"\n{results[0][0]} is BYTE_EXACT.")
        return 0
    if not as_json:
        print("\nnone reached BYTE_EXACT; the ranking shows which spelling is "
              "closest.")
    return 1


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("address", help="hex address, e.g. 0x006077F0")
    parser.add_argument("--body", type=str,
                        help="score this candidate instead of what is "
                             "committed; '-' reads stdin")
    parser.add_argument("--dir", type=Path,
                        help="score every *.cpp in this directory as a "
                             "candidate body and rank them best-first")
    parser.add_argument("--against-committed", action="store_true",
                        help="with --body: also score what is committed and "
                             "refuse a candidate that is WORSE")
    parser.add_argument("--json", action="store_true")
    arguments = parser.parse_args(argv)

    try:
        address = int(arguments.address, 16)
    except ValueError:
        print(f"error: {arguments.address} is not a hex address",
              file=sys.stderr)
        return 2

    if arguments.dir:
        if arguments.body:
            print("error: --dir and --body are alternatives", file=sys.stderr)
            return 2
        return rank_directory(address, arguments.dir, arguments.json)

    if arguments.body:
        body = (sys.stdin.read() if arguments.body == "-"
                else Path(arguments.body).read_text())
        source = "candidate"
    else:
        # A FILE-mode unit is scored AS IT STANDS. Rebuilding scaffolding
        # around a file that already carries its own is how this path came to
        # report NO_COMPILE on committed, byte-exact work.
        annotation = file_mode_annotation(address)
        if annotation is not None:
            verdict = verify_verbatim(annotation)
            tier = verdict.get("tier", "?")
            note = verdict.get("note") or verdict.get("refusal_reason") or ""
            print(f"    {annotation.path} (FILE mode, compiled verbatim)")
            print(f"0x{address:08X}  {tier}"
                  + (f"   {note}" if note else ""))
            return 0 if tier == "BYTE_EXACT" else 1
        body, source = committed_body(address)
        if body is None:
            print(f"SKIP: {source}")
            return 0

    refusals, notes = form_report(body)
    for why in refusals:
        print(f"    REFUSED on form: {why}", file=sys.stderr)
    for why in notes:
        print(f"    form: {why}")
    if refusals:
        # Refused before compiling. A byte-exact answer would only make this
        # harder to argue with.
        return 1

    verdict = writeback.verify(address, body)
    tier = verdict.get("tier", "?")

    # A CANDIDATE THAT IS WORSE IS NOT AN IMPROVEMENT, and "byte exact or not"
    # cannot see the difference between a candidate that stayed MNEMONIC_ONLY
    # and one that fell to MISMATCH. Both are non-zero exits. This is the
    # per-function form of the ratchet `byte_match_fanout.py --check` applies
    # to the whole ledger: a tier may rise and may not fall.
    if arguments.against_committed and arguments.body:
        incumbent_body, incumbent_source = committed_body(address)
        if incumbent_body is not None:
            incumbent = writeback.verify(address, incumbent_body)
            print(f"    committed ({incumbent_source}): "
                  f"{incumbent.get('tier', '?')}")
            if byte_match._better(incumbent, verdict):
                print(f"    REGRESSION: the candidate is worse than what is "
                      f"already committed; refusing it", file=sys.stderr)
                return 1

    if arguments.json:
        import json
        print(json.dumps({**verdict, "address": f"0x{address:08X}",
                          "source": source}, indent=2, default=str))
    else:
        print(f"0x{address:08X}  {tier}   ({source})")
        for key, label in (("note", "divergence"),
                           ("refusal_reason", "reason"),
                           ("first_divergence", "first differing mnemonic"),
                           ("original_bytes", "original bytes"),
                           ("rebuilt_bytes", "rebuilt bytes")):
            value = verdict.get(key)
            if value not in (None, ""):
                print(f"    {label}: {value}")
    return 0 if tier == MATCHED else 1


if __name__ == "__main__":
    raise SystemExit(main())
