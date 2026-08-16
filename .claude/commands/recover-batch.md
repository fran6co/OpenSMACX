---
description: Select a batch of unrecovered functions and fan out byte-match-recovery subagents over them
---

Run one byte-match recovery batch. `$ARGUMENTS` may name a size cap (default
`128`) and a batch size (default `12`).

You are the **coordinator**. You alone touch the shared ledger and the map;
subagents only score candidates. Follow these six steps in order.

## 1. Select

```sh
uv run tools/recovery_frontier.py --max-size <cap> --queue <n>
```

This is already joined against `annotation_scan`, so it will not hand out work
`src/` has proved, nor anything measured `SHARED_TAIL` or `REFUSED`. Take the
top **n** by callers — a zero-caller leaf unblocks nothing and does not count
as progress.

Do not select by address order. It correlates with nothing.

**When the leaf queue is empty, the leaf strategy is finished, not the work.**
As of 2026-08-14 there are no fresh leaves left under 512 bytes at all. What
remains is two populations, and they need different handouts:

- ~350 addresses with **no body at all**, every one 500 B or more and most of
  them virtual handlers reached only through a vtable, so they have zero
  direct callers and will never appear in a leaf queue. These are the
  coverage gap. Rank them by size ascending and hand them out directly.
- ~3,700 that have a body which does not match. This is IMPROVE work, and the
  brief must carry the incumbent body.

Ranking by callers is right while leaves exist and is meaningless afterwards;
say which population a batch came from when you report it.

## 2. Prepare

For each address:

```sh
uv run tools/decomp_status.py --work <addr>
```

This materialises the scaffold over the placeholder, with `mizuchi_declfix`
applied — so the unit an agent iterates on is the unit that will bank the
result. One address is one file, so no two agents can collide.

## 3. Brief — the AGENT generates it, not you

```sh
uv run tools/agent_brief.py <addr> [--tier T --note "..."]
```

The output **is** the prompt for that one function. **Tell each agent to run
this itself, per address, as its first step.** Do not generate the briefs and
paste them into the handout.

A brief is a SNAPSHOT. Pasting it freezes the tool as it stood when the batch
was selected, and a batch runs for a long time: batch 11 taught 13 agents the
`// LEVER:` convention backwards and all 13 landings had to be demoted, batch
12 handed two agents mutually exclusive instructions, and a mid-batch fix to
`verify_recovered_function.py` never reached the agents already holding a
brief — two of them worked around the stale refusal by hand. An agent that
generates its own brief has the current one by construction, and the failure
mode disappears instead of being remembered.

It also keeps the coordinator's context free, which is what lets a batch be
48 addresses instead of 12.

## 4. Fan out

Launch `byte-match-recovery` subagents, **at most 6 concurrently**, 6–8
addresses each. Give each agent its addresses, the command above, and the
standing rules: land the body in the ONE file the brief names (a body left in
`/tmp` banks nothing), report the verdict line verbatim, no `__asm`, no `git`,
and never edit the `// name` / `// size` / `// spans` / `// calls` fact lines —
those are the catalogue it is being measured against.

Six is a measured ceiling, not a guess: every VC6 compile runs against the one
prefix at `~/opt/vc6/.wineprefix`, `byte_match.compile_batches` already runs up
to 8 concurrent `wine CL` inside a single call, and an agent scoring ≤120
candidates forms one chunk and runs serially. Eight agents is the ceiling.

## 5. Collect — and do not trust the reports

```sh
uv run tools/decomp_status.py --addresses <all of them> --record-matches
```

**Re-measure everything.** An agent's report is a claim about a run you did not
observe, produced by the process that had an incentive to stop. `--record-matches`
stamps only what *it* measured. This is the one serial step: it rewrites files
across `src/` and must not run beside anything.

## 6. Gate

```sh
uv run tools/decomp_status.py --check
```

The claim count is the floor and there is no constant to bump. Then report:
how many addresses were attempted, how many reached `BYTE_EXACT` **by your
measurement**, and the new claim total.

If an agent's report and your measurement disagree, say so — a divergence
between claim and measurement is a finding about the harness, not a rounding
error.

## 7. Between batches

The gate being green is not the end of a batch. Three of these have paid for
themselves more than once, and none of them costs agent time.

```sh
# Landed units freeze their scaffolding, so an emitter fix reaches only the
# units written after it. A re-scaffold pass banked 20 BYTE_EXACT with no
# agent time at all; it keeps bodies and ratchets, and reverts regressions.
uv run tools/refresh_file_units.py --apply

# A field name that lies about its offset is read straight into a body. Every
# brief for a class with observed accesses tells the agent that the field at
# 0x838 is spelled `field_838_`; this is what holds that true, against the
# compiler rather than against a header comment.
uv run tools/verify_member_offsets.py --check-names

# What a class's own code proves about where its fields are, for any class an
# agent reported as unverified. This is already in every brief - run it
# directly when triaging a DEFERRED note that blames a layout.
uv run tools/member_map.py <Class>

# Names that say void over bodies that return a status. A review aid, not a
# gate: it reads constants only, and the caller-side half is still unbuilt.
uv run tools/verify_void_returns.py
```

**Act on the agents' STRUCTURE rows, and check the ones that blame a header.**
Two functions were deferred in one batch against headers that named every
offset they needed — one agent read "sizeof is not pinned" in `infowin.h` as
"these offsets are guesses", another read the unspelled inheritance edge in
`dialogs.h` as "a `Dialogs` cannot be declared". Both headers were right and
both agents were wrong, which is only discoverable by reading the header
rather than the report.
