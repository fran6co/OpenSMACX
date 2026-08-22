# OpenSMACX agent handoff

OpenSMACX recovers Sid Meier's Alpha Centauri (SMACX) as C++ by MATCH
DECOMPILATION. Every mapped function carries an `ORIGINAL: 0x...` annotation
in `src/` naming the piece, its image spans, and its measured facts. A body is
done when VC6 (`cl 12.00.8168`) lowers it to the exact shipped bytes;
`BYTE_EXACT` on an annotation is a ratchet claim the gate re-proves on every
run. There is no second ledger - `src/` is the record.

## Read next

- `docs/DECOMP_MAP.md` - the annotation grammar, LEVER/TRIED rules, what state means
- `.claude/commands/recover-batch.md` - the batch loop a coordinator runs
- `.claude/agents/byte-match-recovery.md` - the worker loop, also every dispatched subagent's system prompt
- `docs/recovery/AGENT_BRIEF.md` - measured levers; read before attacking a body
- `docs/EXCLUSIONS.md` - declared out of scope, with populations
- `docs/RETIRED_ROUTES.md` - retired routes and why; do not rebuild one without new evidence

## The command line

Everything runs through `uv run`; never a bare `python3`.

    uv run tools/osmx.py status            # map census; report, always exit 0
    uv run tools/osmx.py show <addr|name> [--in src/text.cpp|unrecovered]
    uv run tools/osmx.py calls <addr>      # call sites in image order
    uv run tools/osmx.py measure <addr | --body F | --dir D>   # exit 0 only BYTE_EXACT
    uv run tools/osmx.py record <addrs...> # measure AND stamp annotations
    uv run tools/osmx.py semantic <addr> [--withdraw]          # equivalence claim
    uv run tools/osmx.py configure         # regenerate the build database
    uv run tools/osmx.py check             # THE GATE

The seam/global instruments:

    uv run tools/bound_callees.py   # function-typed pointer bindings that cost call sites their E8
    uv run tools/fold_globals.py    # object-pointer bindings that cost data references their immediate
    uv run tools/jackal_seam.py     # how far the engine layer is from separable translation units

## The gate

`osmx check` compiles every BYTE_EXACT claim against the pinned image, links
the tree, and runs the `compiler_work.py` census of hand-done compiler work.
Exit 0 clean; 1 = a claim REGRESSED; 3 = something could not be measured,
which is NOT green. NEVER PIPE IT: `cmd | tail` reports tail's exit status,
and that has called a red gate green.

## Working rules

- One commit per step or batch; the tree is the log.
- Only `osmx record` banks BYTE_EXACT, and it stamps only what it measured.
  Never hand-type the token; never edit an `ORIGINAL:` marker line by hand.
- Parameters are `int`, never `uint32_t`: MSVC decorates them `H` versus `I`,
  and the wrong spelling stops the symbol matching.
- No `__asm`, no `_emit`, no copied machine code. A body that does not match
  is unfinished work, never excluded for difficulty.
- State is measured, not claimed: `TRIED:` notes belong under markers of
  bodies that do not yet match, `LEVER:` notes on bodies that do. Grammar in
  `docs/DECOMP_MAP.md`.
- Subagents work in their own worktrees (`tools/agent_setup.sh`); the
  coordinator alone runs git, collection (`tools/collect_agent.py`), and
  reaping (`tools/reap_worktrees.py`).
- Proprietary bytes and derived artifacts stay in ignored `.opensmacx/` and
  `build/`. Never commit objects, listings, or fixtures derived from the
  original executable.
- Measure with existing tools before writing a new one.
- Docs name only paths that exist: `tools/stale_references.py`, wired into
  the gate, fails any live instruction naming a missing file.
