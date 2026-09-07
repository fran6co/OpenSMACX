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
Component commands use source configuration and the local compiler protocol;
there is no dependency-specific project dispatcher. A matching source archive
alone does not establish ABI or binary compatibility. Preserve unresolved
references and unsupported object cases as remaining verification work.

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
Run `uv run recover prepare` before standalone task measurements/recording and
again after changing build inputs. This publishes fresh configured recipes while
retaining their generated include context in ignored storage. It does not clear
the existing build tree. Never run preparation during a measurement epoch.

## 3. Read context and model the group

Run `uv run recover tasks context <task>` and `uv run recover tasks obligations
<task>`. Give workers the task identity, authorized files, current context and
outstanding discoveries—not just addresses or a copied global brief.

Class groups and layout observations follow current source-mapped member symbols
when available, including deleting destructors and fixed/vtordisp adjustors. A
generated entry belongs with its owning class, not a catalogue scaffold class.
Layout reports retain catalogue ownership hints alongside the current owner;
neither interpretation adds or removes original-layout requirements. Review
each entity's ownership basis and alternative hints against the actual declarations
and callers; a catalogue disagreement is not erased by a new grouping. Seed
scaffold class names do not establish a current source owner. These derived hints
help select work; they do not grant a scope expansion or prove a structural model.

Both commands are native source-configuration operations. Historical source
roots, pin locations and seed classifications come from the commits being read;
old claims do not vanish when a directory moves or an attempt becomes a seed.
For pre-framework task history, document the old source interpretation in
`source_history` configuration with evidence. Do not reset a task baseline or
substitute an empty obligations report to bypass missing history inputs.

Context also includes source-model hints from the configured audits, with original
source line locations and counts for authorized files. Structural questions come
before naming volume in the bounded preview; omitted findings remain available
through `uv run recover audits check class-debt --in src/infowin.cpp` (use the
actual file). Filtering details never narrows reference analysis or the gate.
The configured storage audit comes first and connects potentially conflicting
global names to their other source locations. Inspect all those owners with
`uv run recover audits check storage --in src/infowin.cpp` (substitute the file).
Related locations outside the task are evidence for a discovery/scope request,
not permission to edit them. A suspicious pointer sum needs byte-versus-element
analysis; a shared address may denote a containing object and its first member.
These are recomputed clues, not decisions or fresh structural proofs. A textual
orphan needs reachability evidence before removal; a documented naming refusal
must not turn into an invented name just to make a count fall.

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

Bind a real global beside its source definition. Native binary checks and decks
follow external references to declared object owners and report additional
`object_dependencies`. This read-only compilation does not expand worker editing
scope. An unresolved owner needs evidence or a scope request, not a copied original
operand; successful compilation alone does not prove the object's lifetime.
Where original accesses and data evidence establish an object's extent, use a
bounded `RECOVERY_OBJECT: v2` interpretation beside its definition and inspect
`uv run recover check-data --image <image> --symbol <emitted-name>`. Scope the
evidence anchor to the function/family requiring that storage. Task checks then
compile the declared owner even when today's incorrect body has lost its reference.
Read definition, extent, content and relocation results separately. A pointer's
numeric bytes are not its target identity; initial image storage is not CRT or
post-construction state. Do not derive original bounds from candidate debug types.
When replacing an address alias, distinguish the global's storage from its loaded
pointer value. A relocation to the right symbol alone does not establish whether
the code assigns that global or dereferences it to overwrite another object.

For a recovered pointed-to class member, preserve its original evidence with a
`RECOVERY_MEMBER: v3` expectation beside the declaration. Read the layout check's
separate offset/type results: an integer, reference or wrong class pointer can
occupy the same bytes. A passing unqualified pointer-target check is not ownership
or lifetime proof; those still require the coupled constructor/destructor evidence.

For an SDK import, bind the actual emitted pointer symbol to its DLL and export
with `RECOVERY_IMPORT` in source. The native checker derives the slot address from
the pinned PE import directory; do not copy an IAT address into C++ or infer a DLL
from a familiar function name. Read `import_bindings` in checks/decks. A verified
slot identity still leaves candidate link selection and dependency ABI unproved.

Compiler-generated functions need source-owned mappings, not handwritten bodies.
For a secondary-base adjustor, express the real inheritance and virtual override,
then bind its emitted symbol with a `FILE` annotation in the owning TU. Preserve
the independent original span and distinguish the catalogued seed name from the
actual emitted symbol. Check both the adjustor and its target, then the affected
table/callers; a matching thunk alone does not establish the entire hierarchy.
Restore mappings lost during earlier homing from their source history before
trying to fix an unresolved reference by changing otherwise-correct C++.

When restoring an override, recheck its callers as well as its table entry.
An originally direct base call needs explicit qualification; an originally virtual
call must keep dispatching through the object. Do not infer the call form from a
shared method name, or merge distinct operations just to reuse that name.

Read generated-table `diagnostic.entries` in the native binary check before
writing a table probe. Positional target differences can expose missing overrides;
reverse source-symbol correlations can expose stale wrapper bindings. Weak-external
fallbacks need link-selection evidence, not a renamed annotation. If the original
extent is unknown, every displayed word remains a positional observation; it may
belong to a neighboring object. Establish bounds and source bindings independently,
and preserve cross-class findings as discoveries rather than declaring the table
verified from its observed prefix.

## 4. Run, discover, and expand

With local worker/reviewer command configurations, use
`uv run recover run <task> --worker <worker-config> --reviewer <reviewer-config>`.
Set `OPENSMACX_IMAGE` and an isolated `VC6_PREFIX` for this project's check.
The configured `tasks check-restoration` is a native library command. It combines
scoped diagnostics with fresh measurements of the historical obligations; its
`candidate` field contains the joint binary/layout report. `measurement` configuration
selects flag trials and the local application compiler profile; no project
operation dispatcher is needed.
The supplied worker/reviewer configurations now call the package-owned `agent`
bridge. `recovery/agents.json` supplies prompt-driven Codex commands: separate
ephemeral invocations, workspace-write worker and read-only reviewer. Existing
Markdown role instructions are inputs to that bridge, not executable adapters.
Configure provider authentication/model policy and verify worktree/compiler
isolation before launching. The installed CLI flags and synthetic subprocess
handshake have been checked; a live model recovery pilot is not yet verified.

The driver rebuilds context, checks, calls the worker and rechecks bounded rounds.
A native `review_structure` next action hands passing scoped evidence to the
independent reviewer when only historical interpretation review or broader
structural obligations remain unknown;
the configured build must pass too. This avoids spending more spelling rounds
on a proof/review gap. Reviewer corrections return to the worker, while approval
alone leaves an unverified checkpoint, not an admitted migration or accepted task.
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
and a configured migration check proving its build/structure/reference prerequisites.
`recover run --migration <permission>` uses the package-owned native verifier
configured in this project's driver. It checks source/mapping/claim integrity,
audits, a fresh build, bounded declarations, references and outside-scope
compiled preservation under unchanged build and compiler policy. A permitted
measured byte regression yields a checkpoint (exit 2), never collection readiness.
Missing measurements, failed prerequisites and unsupported preservation cases
still refuse admission: park the scoped work for review instead of calling it
green or merging it as accepted. The driver never automatically reverts it.
Source annotations establish `declared_structure` requirements, not complete
hierarchy/lifetime claims. Independent review must explicitly dispose of historical
interpretations; it cannot waive a missing or failing current declaration witness.

Before migration review, inspect `uv run recover tasks check-preservation <task>
--summary`. This native command compares fresh current and Git-baseline compiles,
allowing only source-mapped task function COMDATs and their compiler-owned children
to vary. Outside code/references, independent data/linker metadata and outside
source bindings remain protected. It compares unfinished outside code to the task
baseline, not to the original executable. Existing restoration requirements are
still checked separately and cannot be weakened by a preservation result.

To investigate a particular TU first, add `--unit src/infowin.cpp --diagnostic`
(substitute its actual source path). This avoids the full per-file damage sweep,
while still checking the current replica and compiling the historical tree.
It always exits 3/unverified. Read `compiled_comparison`, changed symbol/reference
details and source-binding hints to identify affected owners before proposing
scope changes. A source-name correlation is not ownership proof; do not add
compiler-local labels to application recovery merely to clear a diagnostic.
Neither a diagnostic match nor an unrelated upstream rename permits moving the
task baseline or waiving outside changes. Full migration review remains separate.

The check uses current compiler policy for both versions, not historical objects.
Historical header/seed additions and deletions use complete private source trees,
not patches against today's file set. Changed product-TU file sets still need
explicit recipe/coverage mapping. Changed source interpretation, unobservable
substitution controls and compile failures remain gaps. `--unit`
can narrow diagnostic work but keeps the overall result unknown when units are
omitted. Do not move the task baseline forward to hide a gap or outside change.
This is compiled preservation evidence, not full-link, runtime or lifecycle proof;
it supplies one prerequisite to the native migration verifier, not admission by
itself. Run it at coordinator checkpoints,
not as an extra full sweep for every spelling candidate.

Use `uv run recover tasks check-restoration <task> --image <image> --summary`
to identify the protected requirements still needing work. This recomputes
measurements, not just the inventory. Missing product bindings and unresolved
references remain unknown; compiler trials cannot lend each other partial passes.
Its `structural_requirements` joins fresh member/base/table measurements to exact
source interpretation identities. Inspect `counts` and `next_action`; omit
`--summary` for per-TU/trial evidence. No current pass substitutes for a historical
interpretation requiring review, and no passing TU hides a missing witness.
A measured current counterexample marks the structural obligation failed without
reverting source; determine whether the declaration or its interpretation needs
correction. Passing declared requirements still cannot discharge full hierarchy
or lifetime, and non-exact instruction-shape proof remains unsupported.

The joint task/restoration check automatically attempts controlled same-TU probes
when a declared class is absent from debug information. For a layout-only
investigation, use `uv run recover tasks check-layout <task> --witness-missing`, or
`uv run recover check-layout --unit src/listbox.cpp --witness-missing` to investigate
one product TU (substitute the actual path). These private compiler overlays add
no product declarations or includes. Inspect `witnessed` coverage and its replica/
changed-body controls separately from ordinary product-object observations.
Do not add includes merely to make a witness pass or use a successful probe as
lifetime/preservation proof. For missing definitions, separate counterfactual
controls may establish `no_complete_definition`: two distinct diagnostic types
must compile and emit their exact sentinel layouts at the requested namespace
scope. Those private types are never recovery facts. A compile error alone cannot
establish absence, and failed controls remain unknown. Every declared requirement
still needs a real layout witness in every flag set; absence everywhere cannot pass.
Probes add compiles only for missing declared classes. The joint check reports
`compilation.overlay_attempts`; restoration trials preserve `layout_basis` for
review. No product-object cache entry can satisfy an overlay control, and probe
objects are never used to score product functions. Restoration marks controlled
absence as `coverage_only`, not an observed member offset. It closes only bounded
declaration coverage, not ABI independence or the safety of raw/untyped callers.
Incomplete coverage still requires recovery; automatic probing does not authorize
new includes, scope changes or structural acceptance.

After structure is established, compare a small deck of plausible source forms
with `uv run recover tasks variants <task> --image <image> --deck <ignored-json>`.
The package README documents multi-file exact-text anchors, source digests and
original entities. Header and dependent-body edits can be one atomic candidate;
all mapped task TUs are compiled with that same overlay. Score 3–5 plausible candidates together
before editing product source.
Use ordered anchor/replacement arrays for separated declaration, initialization,
teardown and caller changes in one file; do not copy a whole TU just to span them.
Every compiled TU needs an unchanged-replica control,
and every edited file needs its own deliberately changed compiled witness. These controls
must pass; coarse tier equality is not a valid control. Default `--jobs 1` is
deliberate until isolated compiler capacity is measured. Review all dimensions
and the cross-TU neighbor rows, including outside-task neighbors where available,
apply the chosen change deliberately, then rerun the task checks. Unmeasured
callers remain outside this experiment's coverage; it does not replace full
structural, preservation or review checks.
Function `// symbol` proposals for task subjects are resolved separately for
each candidate, including their call/reference correlations. Inspect
`binding_changes`; an object still emitting only the old name cannot prove the
proposed binding. Original identities, spans, historical claims and other
recovery interpretations stay fixed during a deck. Change those only at a
reviewed checkpoint, never as a way to improve a candidate score.
A plateau is a resumable finding, not an exclusion. Never fabricate code or
relax a check to bank a result.

## 6. Verify and hand back

`uv run recover tasks check <task> --image <image> --summary` jointly checks
bounded code/references and declared layout. Unknown bindings, missing original
extents, unsupported CodeView records and incomplete lifetime evidence remain
unknown. A layout observation only describes what this compiler emitted.

The driver now also runs `uv run recover check-build` each round. Read its
`check.build_check` report alongside the scoped `candidate` diagnostics. It builds
a fresh Release tree with the configured isolated compiler; existing incremental
objects cannot satisfy it. Passing build output is not structural or runtime proof.
For a standalone build summary, use `uv run recover check-build --summary`.

Pass related addresses together to `uv run recover record <addresses> --image
<image>`: its
measurement engine groups them by host and compiles once per flag trial, while
each function retains its own outcome. This does not authorize new scope or
turn a byte result into structural acceptance.

Recording now stages only source updates through the package's guarded
publisher. Run `uv run recover check-claims --summary` to verify historical
original-identity coverage; no count floor remains. Do not move the configured
history baseline or remove unrelated obligations to get past a refusal. Recording keeps
historical notes unchanged and no longer grants weaker claims, withdraws them,
or offers automatic demotion. Existing instruction-shape obligations still need
verification. After interruption, inspect the Git diff and retained backups under
ignored `build/recovery/publications/`; a multi-file batch is not globally atomic.
Recover source at a coordinator checkpoint, then recheck identities and remeasure.

During cutover, also run `cmake --build build` and the full unpiped
`uv run tools/osmx.py check`. Exit 3 is unmeasurable, not green; “0 REGRESSED”
alone is not acceptance. The installed `recover record` command
is the only current claim-stamping entry point and proves historical masked-byte
agreement, not full structural/reference correctness. Never hand-edit markers.
Use `--in` with an exact project-relative source path when seed alternatives make
selection ambiguous. The configured compiler supplies TU recipes; missing ones
remain unmeasurable. Explicit compiler argv may be supplied as a project-relative
JSON array with `--arguments`. This project's compiler now declares a seed-only
donor in `recovery/environment/recipes.json`; missing product recipes cannot
borrow it. The package infers no donor or build reconfiguration.
Default `--jobs 1` is deliberate; concurrent TUs need isolated compiler capacity.

The package-owned `uv run recover check --image <image> --summary` now runs the
whole configured project gate. Configured preparation runs before the measurement
snapshot; failure stops the gate without measuring against an old database.
Its source, mapping/body-link, history and audit
checks precede a fresh build; match, symbol, initial-data and layout sweeps follow
serially. Additional checks are explicit bounded commands in `recovery/gate.json`.
Layout trials report their TU, index/total and elapsed time on stderr; final
verification remains in the JSON report. A completed trial is not a passing claim.
Use both gate routes for cutover comparison until real runs and damage controls
establish equivalent coverage. `recover check-matches` exposes the native claim
sweep without banking its discoveries. Neither command is the unfinished full
acceptance/migration verifier or evidence that the game works.

The real-project damage controls are now opt-in pytest cases in
`recovery/tests/test_gate_controls.py`, replacing the old main-tree mutation
harness. At a clean coordinator checkpoint, with an explicitly isolated compiler
prefix, run:

```sh
OPENSMACX_LIVE_GATE_CONTROLS=1 uv run pytest recovery/tests/test_gate_controls.py -vv -s
```

Set `OPENSMACX_IMAGE` if the pinned original is elsewhere, and
`OPENSMACX_GATE_JOBS` only to measured isolated capacity (default 1). `-k` can
select a case. Each selected case invokes the full native gate in a temporary
detached checkout with its own package installation; fresh initial/final baseline
gates still run. A case must observe its named stage and, for ambiguous missing
symbol evidence, its damaged subject. An unrelated failure or old unknown result
cannot satisfy it. No model worker is launched. The checkouts are removed without
force; unexpected edits are preserved for inspection. Reports remain disposable
under ignored `build/recovery/gate-controls/`. These can be long compiler runs;
ordinary tests check anchors/restoration safety but skip real gates unless enabled.
Passing those fast tests is not evidence that the live damage matrix passed.

The gate uses the native `uv run recover check-source --jobs 8 --summary` for
annotation integrity. Its keyword/demotion/deletion experiments run only in
memory. Standalone `ORIGINAL:` comment heads are reserved for mappings, not run
notes or ordinary prose. A passing text check is not a current byte claim.

Run `uv run recover check-mappings --summary` to check original identities against
intended build hosts. Inspect the full report's `seed_alternatives` before deleting
duplicate-looking artifacts: equal basenames can hide complementary full-unit and
extracted-body evidence. Multiple files containing seed hints do not establish
production ownership. A product mapping plus another mapped candidate, or two
markers for the same entity in one file, needs reconciliation. The command also
rejects claims shadowed by configured forwarder macros; nonliteral addresses stay
unknown. Actual emitted ownership and included-source coverage remain separate.

The gate's compiler-work ratchet is the native `uv run recover audits check
compiler-work` command. Its report identifies exact, grown and slack ceilings.
Apply justified downward `lowerings` to `recovery/audits.json` in the same commit;
do not alter detector rules or exclusions to hide unfinished work. These are
heuristic policy checks, separate from the original-layout evidence and review.

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
