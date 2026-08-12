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
`src/` has proved. Take the top **n** by callers — a zero-caller leaf unblocks
nothing and does not count as progress.

Do not select by address order. It correlates with nothing.

## 2. Prepare

For each address:

```sh
.opensmacx/venv/bin/python tools/decomp_status.py --work <addr>
```

This materialises the scaffold over the placeholder, with `mizuchi_declfix`
applied — so the unit an agent iterates on is the unit that will bank the
result. One address is one file, so no two agents can collide.

## 3. Brief

```sh
.opensmacx/venv/bin/python tools/agent_brief.py <addr> [--tier T --note "..."]
```

The output **is** the prompt. Pass the tier and note if you already measured
them in a previous collect; otherwise the brief measures that one function.

## 4. Fan out

Launch `byte-match-recovery` subagents, **at most 6 concurrently**, 6–8
addresses each. Give each agent its addresses and the full brief for each.

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
