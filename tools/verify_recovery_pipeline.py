#!/usr/bin/env python3
"""Pin what the analysis pipeline says about a fixed slate of functions.

WHAT THIS IS FOR. Recovering a function means asking the tools a fixed sequence
of questions - how big is the body, where are its spans, what does it call, are
any of those calls invisible to the callgraph, how many bytes does it purge on
return, what does its mangled name imply. The ANSWERS are deterministic
functions of the pinned executable and the committed catalogue. The authoring
step in the middle is not automatable, but everything around it is, and until
now nothing checked that a change to the tools left those answers alone.

So this is the regression test for the recovery process itself: change a tool,
run this, and see whether the pipeline still says the same things about
functions whose answers are already known to be right. A diff here is either a
bug you just introduced or an improvement you should be able to explain.

WHY A GOLDEN FILE AND NOT ASSERTIONS. The facts are too numerous to hand-write
and too specific to guess, and a hand-written expectation drifts into agreeing
with whatever the code currently does. A committed golden makes every change
visible as a diff in review, which is the property that matters.

WHY THE SLATE IS WHAT IT IS. Each entry was chosen for a shape that has broken
something here before, so a regression in any of them is a regression that has
already cost time once:

  * a split body, where end_address stops short of the real function;
  * a body with indirect call sites, which read as a perfect leaf;
  * a thiscall method, where the purge excludes `this`;
  * a free function whose mangled name pins its arity exactly;
  * a recovered function and an unrecovered one, because the seam
    classification differs and both directions must stay stable.

THE GOLDEN IS NOT SELF-CERTIFYING, and this is the honest limit. `--update`
rewrites it from whatever the code now does; running it after breaking
something records the breakage as the new truth. It is a change DETECTOR, not a
correctness oracle. What keeps it useful is that updating it requires an
explicit flag and shows up as a reviewable diff.
"""
import argparse
import csv
import json
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "tools"))

from generator_support import parse_body_ranges, read_bytes  # noqa: E402

DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_CALLGRAPH = REPO_ROOT / "docs" / "recovery" / "callgraph.json"
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
DEFAULT_GOLDEN = REPO_ROOT / "docs" / "recovery" / "pipeline-golden.json"

RESOLVED = {"source_complete", "external_library", "thunk"}

# Addresses, and the shape each one is here to defend. Keep the comment: it is
# the only record of why a given function is on the slate.
SLATE = (
    ("0x004F07E0", "split body - 2,102 B of it live in an outlined cold span"),
    ("0x005E10C0", "dispatches through a vtable slot; reads as a perfect leaf"),
    ("0x00526030", "recently recovered, 8 callees, no indirect sites"),
    ("0x0054ACC0", "free function, mangled name pins two int arguments"),
    ("0x005665D0", "recovered, single caller, zero seams"),
    ("0x00565320", "the largest body recovered; 267 mutants"),
    ("0x0059EEE0", "recovered and observed by nothing - see RECOVERY_ECONOMICS"),
    ("0x005B4790", "3,714 B, the largest unobserved recovery"),
)


def load_rows(path=None):
    """The catalogued rows, from `src/` unless a path is given.

    This gate pins that the pipeline answers a fixed slate the same way twice
    running, so it has to read whatever the pipeline reads - which is `src/`
    now, not the deleted export.
    """
    if path is not None and path.is_file():
        with path.open(newline="", encoding="utf-8-sig") as handle:
            return {row["address"].lower(): row for row in csv.DictReader(handle)}
    import sys as _sys
    _sys.path.insert(0, str(Path(__file__).resolve().parent))
    import emit_translation_unit as _emit
    return {f"0x{a:08x}": r for a, r in _emit.load_functions().items()}


def load_callees(path=None):
    """Call edges, from `src/`'s `calls` lines unless a graph file is given."""
    if path is None or not path.is_file():
        import sys as _sys
        _sys.path.insert(0, str(Path(__file__).resolve().parent))
        import project_catalogue as _pc
        return {f"0x{a:08x}": sorted(f"0x{t2:08x}" for t2 in (r.get("_calls") or ()))
                for a, r in _pc.from_source().items()}
    graph = json.loads(path.read_text(encoding="utf-8"))
    if "edges" not in graph:
        raise SystemExit(f"{path} has no 'edges' key; the callgraph shape "
                         f"changed and every callee list would read as empty")
    callees = defaultdict(set)
    for edge in graph["edges"]:
        callees[int(edge["source"], 16)].add(int(edge["target"], 16))
    return callees


def purge_bytes(decoder, body, spans):
    """Bytes the function pops on return - the only ground truth for arity.

    `ret imm16` states it outright. A body that only tail-jumps has no `ret` of
    its own, and that is reported as unknown rather than as zero, because the
    two mean very different things and conflating them has produced a wrong
    prototype here before.
    """
    seen = set()
    for start, end in spans:
        code = body(start, end)
        if not code:
            continue
        for one in decoder.disasm(code, start):
            if one.mnemonic == "ret":
                seen.add(int(one.op_str, 0) if one.op_str.strip() else 0)
    if not seen:
        return None
    return sorted(seen)


def describe(address, rows, callees, body, decoder, indirect_sites):
    row = rows.get(address.lower())
    if row is None:
        return {"error": "not in the catalogue"}
    start = int(row["address"], 16)
    spans = parse_body_ranges(row.get("body_ranges", "")) or [
        (start, int(row["end_address"], 16))]
    targets = sorted(callees.get(start, ()))
    unresolved = [f"0x{one:08X}" for one in targets
                  if rows.get(f"0x{one:08x}", {}).get("recovery_state")
                  not in RESOLVED]
    sites = []
    for span_start, span_end in spans:
        sites.extend(indirect_sites(body, decoder, span_start, span_end))
    return {
        "name": row["name"],
        "size": int(row["size"]),
        "recovery_state": row["recovery_state"],
        "binary_kind": row["binary_kind"],
        # The catalogued size and the span total disagree exactly when the body
        # is split, which is the whole reason 416 functions need this.
        "span_count": len(spans),
        "span_bytes": sum(end - begin for begin, end in spans),
        "contiguous_bytes": int(row["end_address"], 16) - start,
        "callgraph_callees": len(targets),
        "unresolved_callees": unresolved,
        "indirect_call_sites": len(sites),
        "return_purges": purge_bytes(decoder, body, spans),
        "caller_count": int(row.get("caller_count") or 0),
    }


def collect(functions, callgraph, exe):
    import capstone
    import pefile

    rows = load_rows(functions)
    callees = load_callees(callgraph)
    image = pefile.PE(str(exe))
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

    def body(start, end):
        return read_bytes(image, start, end - start)

    from indirect_call_sites import indirect_sites

    facts = {}
    for address, why in SLATE:
        facts[address] = {"why_on_the_slate": why,
                          **describe(address, rows, callees, body, decoder,
                                     indirect_sites)}
    return facts


def differences(golden, current):
    out = []
    for address in sorted(set(golden) | set(current)):
        was, now = golden.get(address), current.get(address)
        if was is None:
            out.append(f"{address}: new on the slate")
            continue
        if now is None:
            out.append(f"{address}: no longer on the slate")
            continue
        for key in sorted(set(was) | set(now)):
            if was.get(key) != now.get(key):
                out.append(f"{address} {was.get('name', '?')}: {key} "
                           f"{was.get(key)!r} -> {now.get(key)!r}")
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--callgraph", type=Path, default=DEFAULT_CALLGRAPH)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--golden", type=Path, default=DEFAULT_GOLDEN)
    parser.add_argument("--update", action="store_true",
                        help="rewrite the golden from what the code now says")
    arguments = parser.parse_args()

    if not arguments.exe.is_file():
        print(f"recovery-pipeline: the pinned executable is absent at "
              f"{arguments.exe}, so no fact could be derived and this check "
              f"verified NOTHING", file=sys.stderr)
        return 2

    current = collect(arguments.functions, arguments.callgraph, arguments.exe)
    empty = [address for address, facts in current.items() if "error" in facts]
    if empty:
        print(f"recovery-pipeline: {len(empty)} slate entries produced no "
              f"facts: {', '.join(empty)}. A slate that cannot be resolved "
              f"pins nothing.", file=sys.stderr)
        return 1

    rendered = json.dumps(current, indent=2, sort_keys=True) + "\n"
    if arguments.update:
        arguments.golden.write_text(rendered, encoding="utf-8")
        print(f"recovery-pipeline: rewrote {arguments.golden} over "
              f"{len(current)} functions. This records what the code says "
              f"NOW - review the diff, it is not evidence of correctness.")
        return 0

    if not arguments.golden.is_file():
        print(f"recovery-pipeline: {arguments.golden} is absent. Run with "
              f"--update to create it, then review the diff.", file=sys.stderr)
        return 1

    golden = json.loads(arguments.golden.read_text(encoding="utf-8"))
    changes = differences(golden, current)
    if changes:
        print(f"recovery-pipeline: the pipeline's answers changed for "
              f"{len(changes)} fact(s):", file=sys.stderr)
        for line in changes:
            print(f"    {line}", file=sys.stderr)
        print("  If this is intended, re-run with --update and commit the "
              "diff alongside the change that caused it.", file=sys.stderr)
        return 1

    print(f"recovery-pipeline: {len(current)} functions, every derived fact "
          f"unchanged")
    return 0


if __name__ == "__main__":
    sys.exit(main())
