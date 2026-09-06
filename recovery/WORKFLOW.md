# Recovery workflow

This is the active coordinator/worker contract. The old address-only batch,
class-pass variant and overriding agent brief are retired. Compiler experiments
are hints in `COMPILER_EVIDENCE.md`, never workflow policy. The implementation
boundary and unfinished capabilities are described in `README.md` here and
`docs/recovery/FRAMEWORK.md` at the repository root.

## 1. Establish ownership and evidence

Run `uv run recover inventory`, `uv run recover components list`, then
`uv run recover partition`. Inspect a candidate with
`uv run recover partition --group class:InfoWin --details --impact` (substitute
the actual group). These are derived reports, not another source catalogue.

Separate application work from verified third-party, runtime, system and
compiler-generated components. A suspected upstream version is a hint until
source, binary and ABI evidence support it; unknown dependencies stay unknown.
Do not manually decompile a recognized library to increase application coverage.

Seed material may be old recovered/unrecovered bodies, hand analysis, notes,
Kuna, Ghidra or IDA evidence. Preserve provenance and contradictions. No native
Kuna/Ghidra/IDA importer is supplied yet; do not invent commands for them. An
existing exact seed is not automatically the correct source owner or C++ model.

Current source homes, task ownership groups and historical translation units
are different facts. Adjacency alone cannot prove an original TU boundary.
Prefer a class/lifetime/global ownership group and its affected callers to one
task per address. Keep uncertain assignments explicit.

## 2. Authorize a task, then prepare its worktree

Create a source-derived task with `uv run recover tasks create <task> --group
<group> --author <coordinator> --intent <intent> --evidence <evidence>`, or use
`--file` with an explicit task decision. Additional files require `--edit-file`.
Use each command's `--help` for the exact arguments. Checkpoint task/configuration
decisions before running the driver.

Creation uses the target from `recovery/driver.json` (or explicit `--target`) and
the current committed `HEAD` as baseline. It never guesses from a branch name.
To reuse an older checkpoint, pass its full revision with `--base`; it must be
an ancestor with identical configured source and source configuration. Commit
source/configuration changes first. Scope expansion is an explicit authorization,
not a new baseline or permission to discard historical obligations.

The coordinator prepares an isolated worktree with its own build database,
caches and compiler prefix. Workers must not share writable Wine prefixes or
overlapping ownership/header scopes. There is no measured fleet size promised
here and no automatic worktree creation/collection in the driver yet. Select
parallelism from measured compiler capacity; do not restart the old fleet loop.
Update against master only at safe coordinator-owned checkpoints. Never stash
across worktrees, discard another worker's edits, or auto-fast-forward a worker
after its task baseline has been established.

## 3. Read context and model the group

Run `uv run recover tasks context <task>` and `uv run recover tasks obligations
<task>`. Give workers the task identity, authorized files, current context and
outstanding discoveries—not just addresses or a copied global brief.

Recover bases, embedded versus pointed-to objects, construction/destruction,
virtual slot evidence, globals and caller relationships before optimizing body
spellings. Ordinary declarations express the structure. Source annotations hold
original image bindings and evidence that cannot be inferred from those
declarations. Keep names provisional when the evidence is weak.

Do not manually install vtables, copy image bytes, introduce raw original-address
storage, or hide missing members behind pointer arithmetic just to obtain a
match. A constructor's members must not also be placement-constructed in its body.
Real reset methods and real pointer arithmetic are allowed when evidence shows
that behavior; names and a good byte score alone do not establish it.

## 4. Run, discover, and expand

With local worker/reviewer command configurations, use
`uv run recover run <task> --worker <worker-config> --reviewer <reviewer-config>`.
Set `OPENSMACX_IMAGE` and an isolated `VC6_PREFIX` for this project's check.
Provider commands must implement the package's JSON protocol. The Markdown
worker/reviewer prompts are guidance, not installed provider integrations.

The driver rebuilds context, checks, calls the worker and rechecks bounded rounds.
A cross-entity finding pauses the loop for coordinator action; it is not discarded
as out of scope. Inspect `tasks status`, then use `tasks expand` and `tasks resolve`
with their revision-checked decision arguments. Incorporation needs source
evidence; rejection needs an explanation. Checkpoint decisions before resuming.

## 5. Preserve structural progress and restore matching

A justified structural edit may temporarily regress several functions. Do not
automatically revert it or weaken the byte/reference check. Preserve the work and
every historical restoration obligation; expand to the coupled declarations,
callers, globals and generated code needed to close the migration.

Admission requires explicit coordinator permission, independent snapshot review,
and a configured migration check proving its build/structure prerequisites.
`recover run --migration <permission>` is not a bypass: this project's supplied
driver has no migration-check adapter yet, so it cannot currently admit such a
checkpoint. Park the scoped work for review instead of calling it green or
merging it as accepted. The generic driver never automatically reverts it.

After structure is established, compare a small deck of plausible source forms
under the configured compiler flags. The legacy measurement command's `--dir`
can rank candidates without editing product source. Automated task-integrated
candidate decks are not implemented yet. A plateau is a resumable finding, not
an exclusion. Never fabricate code or relax a check to bank a result.

## 6. Verify and hand back

`uv run recover tasks check <task> --image <image> --summary` jointly checks
bounded code/references and declared layout. Unknown bindings, missing original
extents, unsupported CodeView records and incomplete lifetime evidence remain
unknown. A layout observation only describes what this compiler emitted.

During cutover, also run `cmake --build build` and the full unpiped
`uv run tools/osmx.py check`. Exit 3 is unmeasurable, not green; “0 REGRESSED”
alone is not acceptance. The retained `uv run tools/osmx.py record <addresses>`
is the only current claim-stamping entry point and proves legacy masked-byte
agreement, not full structural/reference correctness. Never hand-edit markers.

Full acceptance requires the independent review plus current build, structure,
references, protected obligations and required runtime checks. The OpenSMACX
acceptance adapter is not complete. A scoped pass, legacy claim or approved
design therefore cannot yet produce a fully accepted recovery through this
configuration. Report exact results, coverage gaps, discoveries, changed source
and remaining obligations; do not equate “compiles” with “working game.”

The coordinator alone collects and commits approved work, checking the actual
index/source after application and rerunning checks in the integrated checkout.
Do not pipe mutating commands or the gate. Do not delete a worktree until its
changes have been verified as preserved.

## Where information lives

- Source: original addresses/spans/symbols, structural interpretations with
  evidence, object/component bindings and required recovery obligations.
- Versioned task/history JSON: authorization, external hypotheses, scope changes,
  review and interpretation decisions needed for resumption. Promote accepted
  load-bearing facts into source; JSON must not become another source map.
- Git messages: completed-change rationale. Ignored caches: reproducible reports,
  listings, compiler objects, parsed inventories and run output.

Existing `LEVER`/`TRIED` comments are preserved seed history, not authoritative
current status. Do not add run transcripts, attempt counters or transient status
to source. Preserve non-reproducible external hints and decisions in task history;
recompute everything else unless caching it.
