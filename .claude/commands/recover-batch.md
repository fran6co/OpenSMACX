---
description: Select a batch of unrecovered functions and fan out byte-match-recovery subagents over them
---

Run one byte-match recovery batch. `$ARGUMENTS` may name a size cap (default
`128`) and a batch size (default `12`).

You are the **coordinator**. You alone touch the shared ledger and the map;
subagents only score candidates. Follow these six steps in order.

## 1. Select

```sh
.opensmacx/venv/bin/python tools/recovery_frontier.py --max-size <cap> --queue <n>
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
.opensmacx/venv/bin/python tools/decomp_status.py --work <addr>
```

This materialises the scaffold over the placeholder, with `mizuchi_declfix`
applied — so the unit an agent iterates on is the unit that will bank the
result. One address is one file, so no two agents can collide.

## 3. Brief — the AGENT generates it, not you

```sh
.opensmacx/venv/bin/python tools/agent_brief.py <addr> [--tier T --note "..."]
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
.opensmacx/venv/bin/python tools/decomp_status.py --addresses <all of them> --record-matches
```

**Re-measure everything.** An agent's report is a claim about a run you did not
observe, produced by the process that had an incentive to stop. `--record-matches`
stamps only what *it* measured. This is the one serial step: it rewrites files
across `src/` and must not run beside anything.

## 6. Gate

```sh
.opensmacx/venv/bin/python tools/decomp_status.py --check
```

The claim count is the floor and there is no constant to bump. Then report:
how many addresses were attempted, how many reached `BYTE_EXACT` **by your
measurement**, and the new claim total.

If an agent's report and your measurement disagree, say so — a divergence
between claim and measurement is a finding about the harness, not a rounding
error.
