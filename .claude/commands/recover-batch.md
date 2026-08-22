---
description: Select a batch of unrecovered functions and fan out byte-match-recovery subagents over them
---

Run one byte-match recovery batch. `$ARGUMENTS` may name a batch size
(default `12`).

You are the **coordinator**. You alone touch the map and run `git`; subagents
only edit bodies in their own worktrees. Follow these steps in order.

Everything here names a tool that exists. `uv run tools/stale_references.py`
is wired into the gate and will fail if that stops being true — the previous
version of this file named **seven** deleted tools and `/recover-batch` failed
at step 1 for as long as that went unnoticed.

## 1. Select

```sh
uv run tools/frontier.py --untouched   # neither a TRIED nor a LEVER
uv run tools/frontier.py --fresh       # no TRIED (may carry a LEVER)
uv run tools/frontier.py               # everything the build can compile
uv run tools/frontier.py --all         # ...plus what it cannot
```

Depth-first from `_WinMain@16`, nearest the root first. **Take from the top.**
Do not select by address order — it correlates with nothing.

Measured 2026-08-23, and the shape of the work is the reason the flags matter:

| queue | size | what it is |
|---|---|---|
| `--untouched` | **2** | effectively exhausted |
| `--fresh` | **54** | no prior `TRIED:`; the real handout queue |
| default | **364** | + bodies a previous pass already worked (IMPROVE) |
| `--all` | **1,594** | + **1,230 in files the build does not compile** |

Three consequences:

- **`--untouched` is dry.** Do not report an empty queue as "no work left".
- **A batch off `--fresh` is 54 addresses deep and then that is dry too.**
  After it, batches are IMPROVE work — the body exists and does not match — and
  the agent needs to know that, because starting over on a transcription that
  is already correct is how a pass produces nothing.
- **The 1,230 are not agent work.** A body in a file the build does not compile
  cannot be *measured*, so no agent can finish it. It first needs a home in a
  translation unit. Say which population a batch came from when you report it.

## 2. Fan out

Launch `byte-match-recovery` subagents with **`isolation: "worktree"`**, at most
**6 concurrently**, 6–8 addresses each.

Six is measured, not guessed: every VC6 compile runs against the one wine
prefix at `~/opt/vc6/.wineprefix`, and `osmx check` alone runs up to
`WINE_CEILING` concurrent compiles inside a single call.

Give each agent **its addresses and nothing else**. The agent's own system
prompt already carries the loop, the rules, and the report format, and it is
told to read `docs/recovery/AGENT_BRIEF.md` itself as its first act.

**Do not paste a brief into the handout.** A brief is a snapshot, a batch runs
for a long time, and the tree records what that costs: one batch taught 13
agents the `// LEVER:` convention backwards and all 13 landings had to be
demoted; another handed two agents mutually exclusive instructions. An agent
that reads the current brief has the current one by construction. It also keeps
your context free, which is what lets a batch be 48 addresses instead of 12.

## 3. Collect

```sh
uv run tools/collect_agent.py <agent-id-or-worktree-path>
```

One agent at a time. It never pipes `git apply`, never trusts
`--check --3way`, and reads the tree afterwards to find out what actually
happened — all three of those were real defects that reported success and wrote
nothing. Conflicts are reported, not resolved; `--resolve-ours` prints every
discarded line and you are expected to read them.

## 4. Re-measure — and do not trust the reports

```sh
uv run tools/osmx.py record <all of them>
```

**Re-measure everything.** An agent's report is a claim about a run you did not
observe, produced by a process with an incentive to stop. `record` measures
first and writes only what it measured, so it cannot bank something false.

## 5. Gate

```sh
uv run tools/osmx.py check
```

Compiles every claim, **links the tree**, and runs the `compiler_work.py`
census. Exit `1` = a claim REGRESSED, `3` = something could not be measured
(**not** a green gate), `0` = clean. The claim count is the floor and there is
no constant to bump.

**Never pipe it.** `cmd | tail` reports tail's exit status, and that has called
a red gate green.

## 6. Reap

```sh
uv run tools/reap_worktrees.py          # report
uv run tools/reap_worktrees.py --reap   # remove
```

Agents leave their worktrees dirty by design and nothing else removes them;
twelve accumulated before this existed, each a full checkout. Its four refusals
are what make it safe to run.

## 7. Report

How many addresses were attempted, how many reached `BYTE_EXACT` **by your
measurement**, which population the batch came from, and the new claim total.

If an agent's report and your measurement disagree, **say so** — a divergence
between claim and measurement is a finding about the harness, not a rounding
error.

## 8. Between batches

None of these costs agent time, and each has paid for itself more than once.

```sh
# Byte-exact bodies sitting in an artifact the build never compiles, and
# unclaimed bodies already in the tree. Free claims; re-run after every batch.
uv run tools/promotable.py
uv run tools/promotable.py --unmarked

# One address claimed by BOTH product source and a leftover artifact. The
# gate's duplicate check does not look across directories.
uv run tools/orphan_artifacts.py

# Work the tree is doing that the compiler should do - hand-installed vtables,
# `construct()` stand-ins, placement new on a subobject. Ratcheted; every big
# win in this project came from DELETING one of these.
uv run tools/compiler_work.py

# What the build ACTUALLY emits for a marker's mangled name. Every catalogued
# name is a reconstruction: `QAE` hides `UAE`, `??_G` hides `??_E`, `QAA`
# invents a receiver. Do not diagnose these by eye.
uv run tools/marker_symbols.py
uv run tools/compiler_thunks.py

# Prose that refuses to do something for a reason that was true once.
uv run tools/prose_refusals.py
```

**Act on the agents' STRUCTURE rows, and check the ones that blame a header.**
Two functions were deferred in one batch against headers that named every
offset they needed — one agent read "sizeof is not pinned" in `infowin.h` as
"these offsets are guesses", another read the unspelled inheritance edge in
`dialogs.h` as "a `Dialogs` cannot be declared". Both headers were right and
both agents were wrong, which is only discoverable by reading the header rather
than the report.
