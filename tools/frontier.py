#!/usr/bin/env -S uv run python
"""The reachable call graph, in depth-first order, with verdicts.

WHY. The standing goal is to walk depth first from WinMain until everything
reachable is byte exact or semantically equivalent, and nothing in the tree
answers "what is reachable and not yet exact, nearest the root first". `check`
scores every body in the build's own files - claims and candidates alike -
but as a flat list over the whole tree, so it cannot say which of them is ON
THE PATH.

The edges come from the shipped bytes, not from any annotation's `calls` fact,
which is a sorted set produced by a pass that no longer runs.

WHAT "REACHABLE" MEANS HERE. Direct and tail calls only. An indirect call
through a vtable or a bound slot is a runtime fact this cannot resolve, so a
node reached ONLY that way does not appear - the frontier is a lower bound on
the work, never an upper one, and the count is printed so that is visible.

THE SECOND ROOT. WinMain is not the only entry into this program: the CRT's
dynamic initializers run FIRST, constructing every global object. Measured
2026-08-23, the 768 init/atexit thunk bodies make 101 distinct direct calls,
and 62 of their targets are annotated but unclaimed - invisible to any
WinMain-only walk. `--roots crtinit` seeds the walk with those targets too;
WinMain's order still comes out on top, the pre-WinMain construction graph
after it. The global objects are where those lives are lived.

WORKED BODIES ARE MARKED, because handing out eight already-exhausted addresses
costs a whole agent. A body carrying `TRIED:` notes has had someone attack
it and write down what failed; a batch of those comes back "no change, already
plateaued" eight times over, which is exactly what happened to a faction.cpp
batch picked off the raw depth order. `--fresh` drops them.
"""

from __future__ import annotations

import collections
import hashlib
import json
import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import build_inputs
from decomp.asm import original_asm, shared_spans, span_refusal
from decomp.calls import call_sites
from decomp.model import State


REPO_ROOT = Path(__file__).resolve().parent.parent
# HONOURS `OPENSMACX_IMAGE`, because `.opensmacx/` is gitignored and a
# worktree does not have it - an agent working in one must be able to
# point every tool back at the root copy. osmx.py has always taken it;
# these did not, and an agent had to symlink the image to work around it.
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"))
ROOT = 0x0045F950  # _WinMain@16


def _blanked(text: str) -> str:
    """Comments replaced by spaces, so brace matching stays honest.

    ORDER MATTERS. Stripping comments AFTER finding the opening brace finds a
    brace inside a comment instead of the function's - a first draft of this
    reported 486 of 487 frontier bodies as stubs, including `read_rules`, which
    is 883 image instructions.
    """
    text = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group(0)),
                  text, flags=re.S)
    return re.sub(r"//[^\n]*", lambda m: " " * len(m.group(0)), text)


_BLANK_CACHE: dict = {}


def body_text(record) -> str | None:
    """The braces-matched source body following a record's marker."""
    where = str(record.location)
    path, _, line = where.rpartition(":")
    path = Path(path)
    if path not in _BLANK_CACHE:
        try:
            _BLANK_CACHE[path] = _blanked(
                path.read_text(errors="replace")).splitlines()
        except OSError:
            _BLANK_CACHE[path] = []
    lines = _BLANK_CACHE[path]
    text = "\n".join(lines[int(line) - 1: int(line) + 600])
    start = text.find("{")
    if start < 0:
        return None
    depth = 0
    for i in range(start, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[start + 1:i]
    return None


STUB = re.compile(r"(return\s*(-?\d+|nullptr|false|true)?\s*;)?")


def is_stub(record) -> bool:
    """Empty, or a single bare `return`: a placeholder, not a transcription."""
    inner = body_text(record)
    if inner is None:
        return False
    statements = [s.strip() for s in inner.split("\n") if s.strip()]
    return len(statements) <= 1 and bool(STUB.fullmatch(" ".join(statements)))


def walk(records: list, exe: Path, root: int,
         seen: set | None = None, order: list | None = None) -> tuple[list, int, int]:
    """Depth-first from `root`; returns (order, edges seen, unnamed edges).

    `seen` and `order` let several roots share one walk - `--roots crtinit`
    seeds the pre-WinMain construction graph into the SAME traversal, so a
    body reachable from both appears once, where WinMain put it.
    """
    if seen is None:
        seen = set()
    if order is None:
        order = []
    by_address = {}
    for record in records:
        by_address.setdefault(record.address, record)
    shared = shared_spans(records)

    unnamed, edges = 0, 0
    stack = [root]
    while stack:
        address = stack.pop()
        if address in seen:
            continue
        seen.add(address)
        record = by_address.get(address)
        if record is None:
            unnamed += 1
            continue
        order.append(record)
        if span_refusal(record, exe, shared) is not None:
            continue
        try:
            listing = original_asm(record, exe)
        except (ValueError, KeyError):
            continue
        targets = []
        for site in call_sites(listing):
            edges += 1
            if site.form in ("direct", "tail") and site.target is not None:
                targets.append(site.target)
        # Reversed, so `pop` takes the FIRST call the image makes: the order
        # of the walk is the order of the code.
        stack.extend(reversed(targets))
    return order, edges, unnamed


def crt_roots(records: list, exe: Path) -> list:
    """The direct callees of every init/atexit thunk - the second root set.

    These are the constructors and destructors of the global objects: the
    code the CRT runs BEFORE WinMain, which no WinMain-rooted walk can ever
    reach except by coincidence.
    """
    targets = []
    for record in records:
        if record.path.name not in ("init_thunks.cpp", "atexit_thunks.cpp"):
            continue
        try:
            listing = original_asm(record, exe)
        except (ValueError, KeyError):
            continue
        for site in call_sites(listing):
            if site.form in ("direct", "tail") and site.target is not None:
                targets.append(site.target)
    return targets


# The work queue shows the GAME's code. Library code is never this project's
# work, and an excluded import thunk is a linker artefact (`jmp [__imp_X]`),
# so neither is offered to a batch. Excluded GAME records stay VISIBLE with
# their citation: an exclusion is a decision that can be re-litigated, and a
# queue that hides one hides the argument too. Filter by kind and state
# together - NEVER by exclusion alone.
def never_work(record) -> str | None:
    if record.kind == "library":
        return "library"
    if record.kind == "thunk" and record.state is State.EXCLUDED:
        return "excluded import stub"
    return None


SCAFFOLD_FILES = ("init_thunks.cpp", "atexit_thunks.cpp",
                  "adjustor_thunks.cpp", "deleting_thunks.cpp",
                  "delegation_thunks.cpp", "field_accessors.cpp",
                  "leaf_recoveries.cpp")

BASE_DECL = re.compile(r"\b(?:class|struct)\s+(\w+)\s*(?::\s*([^{;]+))?\{")


def declared_bases() -> dict:
    """Class -> declared bases, read off every header's own text.

    Comments are blanked first, or prose naming a class inside a base list
    invents an edge. Best-effort by design: a class whose bases are not yet
    DECLARED contributes no edges, which is exactly why it sits deep in the
    queue until its pass declares them.
    """
    edges: dict = {}
    for header in sorted((REPO_ROOT / "src").glob("*.h")):
        text = _blanked(header.read_text(errors="replace"))
        for match in BASE_DECL.finditer(text):
            bases = match.group(2)
            if not bases:
                continue
            names = []
            for spec in bases.split(","):
                words = spec.split()
                if words:
                    names.append(words[-1])
            if names:
                edges.setdefault(match.group(1), []).extend(names)
    return edges


def owner_of(record) -> str:
    """The mangled name's class, or the TU for free functions."""
    parts = (record.name or "").split("@")
    if len(parts) > 1 and parts[0].startswith("?") and parts[1]:
        return parts[1]
    return f"free:{record.path.name}"


def file_compiler_work_sites() -> dict:
    """Hand-done compiler-work sites per PRODUCT file, from compiler_work's
    own patterns - the same census the gate runs, keyed per file so
    --by-class can join it by the files a class lives in."""
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import compiler_work
    sites: dict = {}
    for path in sorted((REPO_ROOT / "src").glob("*.cpp")):
        if "recovered" in path.parts or "unrecovered" in path.parts:
            continue
        count = 0
        for line in path.read_text(errors="replace").splitlines():
            if line.lstrip().startswith("//"):
                continue
            for _name, _ceiling, rx, _why in compiler_work.SHAPES:
                count += len(rx.findall(line))
        if count:
            sites[path.name] = count
    return sites


def by_class(records, order, built, visible) -> None:
    """The standing class view: who owns the frontier's work, how far each
    class is from complete, and what still holds it off the ratchet.

    TWO columns define done: exact/total complete, compiler-work sites at 0
    faithful - and a class can be the first without the second, which is the
    state where hand-written thunks hold a family off the ratchet. Rows sort
    BASES BEFORE DERIVED (best-effort topological over DECLARED bases,
    first-appearance as tiebreak); the one-directional invariant is asserted:
    no row may appear before a class it declares as a base. Everything else
    is heuristic and is not checked as if it were law.
    """
    edges = declared_bases()
    sites = file_compiler_work_sites()
    header_texts = {h.name: _blanked(h.read_text(errors="replace"))
                    for h in sorted((REPO_ROOT / "src").glob("*.h"))}

    by_owner = collections.defaultdict(list)
    for record in records:
        by_owner[owner_of(record)].append(record)

    queue: dict = {}
    first_seen: dict = {}
    position = 0
    thunk_seen: set = set()
    for record in order:
        if record.byte_exact or not visible(record):
            continue
        owner = owner_of(record)
        if re.fullmatch(r"thunk\d+_\w+", owner):
            thunk_seen.add(owner)
            continue
        first_seen.setdefault(owner, position)
        position += 1
        slot = queue.setdefault(owner, {"reach": 0, "built": 0, "artifact": 0})
        slot["reach"] += 1
        if record.path in built:
            slot["built"] += 1
        else:
            slot["artifact"] += 1

    depth: dict = {}

    def base_depth(owner: str, trail: tuple = ()) -> int:
        if owner in depth:
            return depth[owner]
        if owner in trail:
            return 0
        bases = [b for b in edges.get(owner, []) if b in queue]
        depth[owner] = max((base_depth(b, trail + (owner,)) + 1
                            for b in bases), default=0)
        return depth[owner]

    for owner in queue:
        base_depth(owner)

    rows = sorted(queue, key=lambda o: (depth[o], first_seen[o]))
    index = {owner: i for i, owner in enumerate(rows)}
    for owner, i in index.items():
        for base in edges.get(owner, []):
            if base in index and index[base] > i:
                sys.exit(f"--by-class invariant violated: {owner} "
                         f"appears before its declared base {base}")

    owner_files: dict = collections.defaultdict(set)
    for record in records:
        path = str(record.path)
        if "recovered" in path or "unrecovered" in path:
            continue
        owner_files[owner_of(record)].add(record.path.name)

    print(f"{'first':>5}  {'owner':22s} {'reach':>5} {'built':>5} "
          f"{'artif':>5} {'exact':>9} {'cw':>4} {'scaf':>4}  declaration")
    for owner in rows:
        slot = queue[owner]
        every = by_owner[owner]
        exact = sum(1 for r in every if r.byte_exact or r.semantic)
        cw = sum(sites.get(name, 0) for name in owner_files[owner])
        scaff = sum(1 for r in every if r.path.name in SCAFFOLD_FILES)
        decl = sorted({name for name, text in header_texts.items()
                       if re.search(rf"\b(?:class|struct)\s+"
                                    rf"{re.escape(owner)}\b", text)})
        where = ",".join(decl[:2]) + ("…" if len(decl) > 2 else "")
        print(f"{first_seen[owner]:>5}  {owner:22.22s} {slot['reach']:>5} "
              f"{slot['built']:>5} {slot['artifact']:>5} "
              f"{exact:>4}/{len(every):<4} {cw:>4} {scaff:>4}  {where}")

    hyp_only = [owner for owner in rows
                if "hypothesis_layouts.h" in
                {name for name, text in header_texts.items()
                 if re.search(rf"\b(?:class|struct)\s+"
                              f"{re.escape(owner)}\b", text)}
                and len({name for name, text in header_texts.items()
                         if re.search(rf"\b(?:class|struct)\s+"
                                      f"{re.escape(owner)}\b", text)}) == 1]
    print(f"\n{len(rows)} classes on the frontier; done = exact/total complete"
          f" AND cw 0 AND scaf 0.")
    # A ZERO HERE IS TWO ANSWERS, so it carries its denominator. `thunkN_`
    # owners are the fake classes the adjustor shims dispatch through, and
    # they dissolve when their real bases go virtual - but the number of them
    # EXCLUDED FROM THIS VIEW is 0 and structurally always will be. An
    # adjustor thunk is reached only through a vtable, and this walk follows
    # direct and tail calls only, so the family was never ON the frontier to
    # be filtered off it. Printing the exclusion alone made a structural
    # invisibility read as "checked, and there were none".
    thunk_all = sorted(o for o in by_owner if re.fullmatch(r"thunk\d+_\w+", o))
    thunk_bodies = sum(len(by_owner[o]) for o in thunk_all)
    print(f"  thunkN_ scaffold owners: {len(thunk_seen)} excluded from this "
          f"view, of {len(thunk_all)} in the catalogue ({thunk_bodies:,} "
          f"bodies).")
    if thunk_all and not thunk_seen:
        print(f"    None are reachable: an adjustor thunk is entered through "
              f"a vtable, and this walk follows direct and tail calls only. "
              f"They dissolve when their real bases go virtual - track them "
              f"by the adjustor_thunks ceiling, not here.")
    if hyp_only:
        print(f"  hypothesis-only declarations ({len(hyp_only)}), homed by the"
              f" hypothesis_layouts.h split before their dependents run:")
        for owner in hyp_only:
            print(f"    {owner}")
    print(f"  cw = compiler-work sites in the owner's product TUs (census"
          f" joined per file; TUs shared between classes count for each).")


def _walk_cache_key(records: list, mode: str) -> str:
    """Everything the walk's answer depends on, hashed.

    The image bytes and every record's (address, spans) pair are the walk's
    INPUTS; the three source files are its SEMANTICS - a fix to call-site
    decoding must invalidate the cache by construction, not by someone
    remembering a version constant. Verdict state (BYTE_EXACT and friends) is
    deliberately NOT in the key: the cache stores only addresses, and the
    records they resolve to are re-read fresh every run.
    """
    digest = hashlib.sha1()
    digest.update(IMAGE.read_bytes())
    for own in ("frontier.py",):
        digest.update((Path(__file__).parent / own).read_bytes())
    for own in ("calls.py", "asm.py"):
        digest.update((REPO_ROOT / "decomp" / own).read_bytes())
    # `image_spans`, and NO getattr DEFAULT - this file's own walk-marking
    # lesson, relearned live: the first spelling here was `r.spans`, which
    # does not exist, and the raise is what caught it. (It also caught the
    # verifier: three "identical" diffs upstream were comparing the empty
    # stdout of three CRASHED runs. Diff outputs only after proving them
    # non-empty.)
    for line in sorted(f"{r.address}:{r.image_spans}" for r in records
                       if r.address):
        digest.update(line.encode())
    digest.update(mode.encode())
    return digest.hexdigest()


WALK_CACHE = REPO_ROOT / "build" / "frontier_walk.json"


if __name__ == "__main__":
    records = read(REPO_ROOT / "src")
    # `--roots <name>` extends the traversal. Only `crtinit` exists; an
    # unknown name is refused rather than silently ignored, because a flag
    # that does nothing reads exactly like one that worked.
    roots_mode = ""
    if "--roots" in sys.argv:
        index = sys.argv.index("--roots")
        roots_mode = sys.argv[index + 1] if index + 1 < len(sys.argv) else ""
        if roots_mode != "crtinit":
            sys.exit(f"frontier.py: unknown --roots mode {roots_mode!r} "
                     f"(only 'crtinit' is defined)")

    # THE WALK IS A PURE FUNCTION OF ITS KEY, so it is cached: ~30 s of
    # re-disassembling a pinned image on every selection run, down to ~1 s.
    # A cache hit stores ADDRESSES ONLY - the records they resolve to, and
    # therefore every verdict printed, are re-read from the tree above on
    # every run. Anything wrong with the cache file falls through to a full
    # walk; `--no-walk-cache` forces one.
    key = _walk_cache_key(records, roots_mode)
    cached = None
    if "--no-walk-cache" not in sys.argv:
        try:
            candidate = json.loads(WALK_CACHE.read_text())
            if candidate.get("key") == key:
                cached = candidate
        except (OSError, ValueError):
            cached = None

    if cached is not None:
        by_address = {}
        for record in records:
            by_address.setdefault(record.address, record)
        try:
            order = [by_address[a] for a in cached["order"]]
            edges, unnamed = cached["edges"], cached["unnamed"]
            seed_count = cached["seeds"]
        except KeyError:
            cached = None
    if cached is None:
        order, edges, unnamed = walk(records, IMAGE, ROOT)
        seed_count = 0
        if roots_mode == "crtinit":
            seeds = sorted(set(crt_roots(records, IMAGE)))
            seed_count = len(seeds)
            seen = {record.address for record in order}
            for seed in seeds:
                if seed in seen:
                    continue
                order, e2, u2 = walk(records, IMAGE, seed, seen=seen,
                                     order=order)
                edges += e2
                unnamed += u2
        try:
            WALK_CACHE.parent.mkdir(exist_ok=True)
            WALK_CACHE.write_text(json.dumps(
                {"key": key, "order": [r.address for r in order],
                 "edges": edges, "unnamed": unnamed, "seeds": seed_count}))
        except OSError:
            pass
    if roots_mode == "crtinit":
        print(f"--roots crtinit: {seed_count:,} pre-WinMain construction "
              f"roots seeded from the init/atexit thunks", file=sys.stderr)
    built = build_inputs(REPO_ROOT / "build" / "compile_commands.json")

    skip_counts: dict = collections.Counter()

    def visible(record) -> bool:
        why = never_work(record)
        if why is None:
            return True
        skip_counts[why] += 1
        return False

    if "--by-class" in sys.argv:
        by_class(records, order, built, visible)
        if skip_counts:
            split = ", ".join(f"{n} {kind}"
                              for kind, n in sorted(skip_counts.items()))
            print(f"  filtered from every view above, never work: {split}")
        sys.exit(0)

    pending = [record for record in order if not record.byte_exact]
    # KIND BEFORE BUILD: under --all the queue's head was Microsoft's libc -
    # 168 library bodies a batch selector would otherwise hand out first.
    pending = [record for record in pending if visible(record)]
    if "--all" not in sys.argv:
        # A record whose file the build does not compile cannot be MEASURED,
        # so it is not work anyone can finish today - it is work that first
        # needs a home in a translation unit.
        pending = [record for record in pending if record.path in built]
    if "--json" in sys.argv:
        print(json.dumps([{"address": r.address_hex, "name": r.name,
                           "file": str(r.path)} for r in pending], indent=2))
        sys.exit(0)
    # How many `TRIED:` notes sit under each marker. The reader already
    # parses lesson lines, so this is the record's own evidence of prior work
    # rather than anything maintained by hand.
    # NO `getattr` DEFAULT. The first draft guessed the field name, and a
    # default of `()` turned the miss into "nothing is worked" - every row
    # unmarked, which reads exactly like a correct answer. Name the field and
    # let a wrong one raise.
    def ruled_out(record) -> int:
        return len(record.ruled_out)

    def levers(record) -> int:
        return len(record.levers)

    if "--fresh" in sys.argv:
        pending = [r for r in pending if not ruled_out(r)]

    # `--untouched` IS THE ONE THAT TARGETS YIELD. `--fresh` only drops bodies
    # carrying a TRIED, and a body can have been worked hard and left a
    # `// LEVER:` line instead - Scroll's three `set_bevel_*` siblings each
    # carry one, and every one reads as "fresh". Picking a batch that way hands
    # an agent work a previous pass already did. Untouched means NEITHER.
    if "--untouched" in sys.argv:
        pending = [r for r in pending if not ruled_out(r) and not levers(r)]

    for depth, record in enumerate(pending):
        worked, moved = ruled_out(record), levers(record)
        mark = ""
        if worked or moved:
            parts = ([f"{worked} tried"] if worked else []) + \
                    ([f"{moved} lever"] if moved else [])
            mark = "  [" + ", ".join(parts) + "]"
        print(f"{depth:4d}  {record.address_hex}  {record.path.name:24s} "
              f"{record.name}{mark}")
    if "--semantic" in sys.argv:
        stubs = [r for r in pending if is_stub(r)]
        granted = [r for r in pending if r.semantic]
        print(f"\n  {len(pending):,} reachable and not byte exact")
        print(f"    {len(pending) - len(stubs):,} carry a real transcription")
        print(f"    {len(stubs):,} are STUBS - empty, or a bare return")
        for record in stubs:
            print(f"      {record.address_hex}  {record.size or 0:,} image "
                  f"bytes   {record.name}")
        print(f"    {len(granted):,} carry a PROVED semantic claim "
              f"(same instructions, different registers)")

        # WHAT IS NOT REACHABLE AT ALL, so the goal's completion criterion can
        # be read honestly. A body whose image bytes are hand-written assembly
        # can never be byte-exact from C++, and `osmx semantic` cannot certify
        # it either - that tier requires the SAME instruction sequence, and a
        # `rep stosd` block is not the sequence any C++ compiles to. Those
        # bodies can only ever be semantically equivalent in the ordinary
        # sense, argued from the source rather than proved by the tool.
        try:
            from subprocess import run
            done = run(["uv", "run", "tools/handwritten_asm.py"],
                       cwd=str(REPO_ROOT), capture_output=True, text=True,
                       timeout=900)
            hand = {int(line.split()[0], 16)
                    for line in done.stdout.splitlines()
                    if line.startswith("  0x")}
        except Exception:                                # noqa: BLE001
            hand = set()
        blocked = [r for r in pending if r.address in hand]
        if blocked:
            print(f"    {len(blocked):,} are HAND-WRITTEN ASSEMBLY in the "
                  f"image - neither tier is reachable:")
            for record in blocked:
                print(f"      {record.address_hex}  {record.name}")
        print("\n  A transcription is EVIDENCE of equivalence, not proof. The"
              "\n  proof this tree can give is `osmx semantic`, and the"
              "\n  strongest cheap check is `call_diff`: a body calling"
              "\n  something the image does not is not equivalent, whatever"
              "\n  its similarity score says.")
        sys.exit(0)

    print(f"{len(order):,} reachable and catalogued, {len(pending):,} not yet "
          f"byte exact; {unnamed:,} direct edges name nothing in the catalogue "
          f"({edges:,} edges walked, indirect ones not followed)")
    # AND THE NUMBER THE GOAL ACTUALLY ASKS FOR. "Byte exact OR semantically
    # the same" is the bar, and a body carrying a PROVED semantic claim has
    # met it - `osmx semantic` grants only where the instruction sequence is
    # identical and the registers differ. Counting those as outstanding
    # overstates the work remaining, so both numbers are printed rather than
    # one being quietly substituted for the other.
    proved = sum(1 for record in pending if record.semantic)
    if proved:
        print(f"  {proved:,} of those carry a PROVED semantic claim, so "
              f"{len(pending) - proved:,} remain against "
              f"\"byte exact or semantically the same\"")
    print("  --all also lists reachable bodies the build does not compile")
    split = ", ".join(f"{n} {kind}" for kind, n in sorted(skip_counts.items()))
    print(f"  filtered from the queue as never-work: {split or 'nothing'}"
          f" (game-side exclusions stay listed, with their citation)")
