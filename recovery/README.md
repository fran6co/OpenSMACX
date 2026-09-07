# OpenSMACX recovery integration

Project-owned configuration and compiler interaction for the independently
installed `match-recovery` engine. Source, task, dependency and scoped application
verification run in the library; the former application operation bridge is
removed. Local compiler scripts remain in `environment/`. See
[the portable package](../packages/match-recovery/README.md) for starting an unrelated project.

The active agent/coordinator contract is [WORKFLOW.md](WORKFLOW.md). Historical
compiler observations are in [COMPILER_EVIDENCE.md](COMPILER_EVIDENCE.md); they do
not override task scope, structural evidence or acceptance policy.

Source records, strict CodeView parsing (`class_layout`) and emitted-table checks
(`check_generated`) now live in the installed library. These are format/ABI
capabilities, not project/compiler-environment adapters. `environment/` contains
only the local compiler interaction. The old production command/gate and other
remaining tools still need absorption or retirement before `tools/` can disappear.

The native `recover tasks variants` command now replaces serial edit-and-restore
spelling experiments. It reads the source map once and compiles atomic header/body
overlays across task TUs. Unchanged-replica controls per TU and compiled damage
witnesses per edited file guard the experiment. It reports
strict reference-aware results without banking a winner. See the package README
for the deck schema. This project's compiler script supports v4 multi-file overrides
by copying configured source/build-input roots into verified-ignored
`build/recovery/variants/` and remapping explicit include
and forced-include paths. PCH, response files, hidden CL options and custom include
environment overrides are refused. Copies preserve quote/angle include ordering
and physical header identity; no absolute `#line` rewrite is injected because
VC6 can use it to search the original directories and break `#pragma once`.
Path/time-sensitive differences remain unsupported when the replica control
cannot agree. Reports include mapped outside-task neighbors in compiled units,
but do not prove preservation of all other code. The native `tasks check-preservation`
command now compares baseline/current compiled residuals separately (see below).
Historical header/seed file sets use v5 complete-tree materialization. Full
preservation, historical product-TU recipe mapping and prefix pooling remain unfinished.
Default concurrency is 1.

## Available now

From the OpenSMACX workspace:

```sh
uv run recover --help
uv run recover check-build --summary
uv run recover inventory
uv run recover partition
uv run recover partition --group class:InfoWin --details --impact
uv run recover inventory --image /path/to/the/pinned/original.exe
uv run recover components list
uv run recover components check-source zlib --archive /path/to/zlib-1.0.2.tar.gz
uv run recover components check-binary zlib --image /path/to/the/pinned/original.exe --summary
uv run recover audits check compiler-work --summary
uv run recover audits check class-debt --summary
uv run pytest packages/match-recovery/tests recovery/tests
uv build --package match-recovery
```

Reports are JSON on stdout. Errors go to stderr and return exit 3. Inventory never
changes source or decisions. Inventory and partition are native package commands
when `recovery/project.json` supplies `source` configuration; they do not launch
a project adapter. Task and component commands likewise use the package directly.

The compiler-work audit is also native. Its reusable pattern rules live in the
package; `audits.json` holds this project's explicit ceilings and scaffold counter
scopes. These numbers are policy limits, not cached measurements or another
function ledger. All 24 ceilings and per-file pattern counts were preserved on
cutover. The former compiler-work tool is removed; its change explanations remain
in Git. The legacy full gate invokes the installed audit command directly.
Both growth and slack return failure (exit 1); missing/malformed inputs return 3.
The report's `lowerings` gives only downward edits to apply to `audits.json` in the
same commit as the improvement. There is no self-rewriting Python ceiling table.
Neither this heuristic audit nor a zero count proves correct hierarchy or lifetime.

The class-debt audit now uses the package's `cpp-model-v1` ruleset. The former
project tool is removed, with all nine ceilings and per-file counts preserved.
Its 3,747-count cutover baseline is historical, not a current status ledger.
`audits.json` opts this audit into task context; filtered source hints locate raw
self-access, pointer-typed parameters, disguised function references and naming
questions. Counts are recomputed from source, including seed mappings as address
hints. Seed/vendor bodies are not product cleanup targets. Evidence-free names
remain explicitly classified, not silently excused or automatically renamed.
Orphan totals count unique textual names while per-file findings count mentions;
neither establishes linker reachability. The native audit also corrects interval
overlap lookup and prevents a naming refusal leaking to another directory or a
different token such as UNK10 beside UNK1. Those fixes change no current counts.
The unused global naming queue and text-count orphan-deletion helper are retired
as well. Their observations are now scoped source hints. A body's availability is
not naming evidence, and a low textual reference count plus a successful link
does not establish that deleting a definition preserves behavior. Use scoped,
reviewed source edits after establishing evidence; historical implementations and
compiler lessons remain recoverable from Git.

The source reader preserves all notes, including unparsed
or non-UTF-8 text, without declaring current matches or choosing between
conflicting candidates. The old bulk legacy-import command is removed: source
snapshots are already recomputable and the duplicate snapshot store had no
workflow consumer. The package memoizes individual files under ignored
`build/recovery/parse_cache/`, only when Git confirms that directory is ignored
and contains no tracked files. Otherwise parsing remains uncached.
Keys include source bytes, relative/resolved path
shape, image pin, source configuration and parser implementation digests; timestamps are not
authority. Entries have integrity checks and atomic publication. Missing, damaged
or unwritable caches fall back to parsing. Set `RECOVERY_PARSE_CACHE=off` to bypass
this memo, or delete its directory. Source inventory and content race checks still
run on every snapshot; no compilation or verification verdict is cached here.

The snapshot is a disposable parse result, not durable history. These operations
do not write versioned inventories or measurement reports. No database is required.

The text journal uses atomic, revision-checked appends. Concurrent stale writers
are refused instead of overwriting discoveries, and retrying the same request
does not duplicate it. Payload corruption and missing event positions are errors.
Input fingerprints use file contents and explicit configuration, never timestamps
alone; equivalent inputs in another worktree produce the same fingerprint.
Cross-cutting hypotheses and migration decisions may use tracked text records;
`recover history --stream NAME` reads a decision stream without creating files.
Load-bearing image mappings, provenance, structural choices and obligations belong
in source. Anything derivable from those inputs is recomputed or cached, never a
second authority. Raw compiler output and binary derivatives stay ignored.

Vendor mappings live in `*.recovery.h` companion source annotations, leaving the
upstream files themselves unmodified. Zlib's original spans, symbols and units
were imported from retired wrappers' explicit historical metadata, not from the
candidate's length. That one-time importer is now retired; all 39 bindings and
their Git provenance remain in source and were checked against those wrappers
before removal. The importer remains recoverable from Git if new migration
evidence calls for it. These bindings are dependency checks,
not application recoveries. The source-integrity command derives file hashes from
the pinned upstream archive; without that archive it returns unknown (exit 3).
An altered source or wrong archive returns failure (exit 1). Even a passing source
check does not establish binary matching or ABI correctness.

All component commands now run in the package. Source configuration selects the
manifest roots and seed exclusions. The `compiler` configuration supplies the
command protocol implemented locally by `environment/adapter.py`; no project
component dispatcher remains. It sends source/arguments/flags and receives
transient object bytes. Declared compiler scripts, configuration and relevant
environment variables are rechecked around compilation. Manifests, vendor source
and original/archive inputs are rechecked around verification. No object response
or process exit alone constitutes a matching claim.

Application checks additionally use `measurement` configuration: backend, flag
trials, standalone layout flags, TU suffixes, and build inputs. The `application`
compiler profile resolves this repository's compile database locally; dependency
compilation uses the default profile. The library does not select VC6 flags,
guess a build entry, or import an application adapter. Compiler protocol v2 carries
the selected profile explicitly. Scoped joint checks retain one-invocation object
sharing, and recheck source/build/compiler/environment/library inputs plus the
task and original image. This is not a persistent measurement cache or full gate.

The binary checks currently use the OpenSMACX VC6 launcher. Set `VC6_PREFIX` to
an isolated prefix before running it alongside other agents. It compiles once per
selected translation unit and resolves references only from explicit source
bindings, including STATIC functions and labels within a mapped COMDAT. Missing
targets remain unknown; an address is never inferred from the operand being
verified. Exact COFF symbol selection is required, with no spelling fallback.

Original extents remain authoritative. The comparator permits only NOP/INT3
alignment fill after a recorded extent, according to the object's alignment; it
does not expand an original span to include extra code or a candidate jump table.
It checks both absent and additional absolute relocations, and applies DIR32/REL32
addends before comparing bytes. Output is a fresh report, not a stored claim.
Neither this check nor source integrity proves C++ ownership, ABI, or runtime
correctness. One explicitly associated code companion can be checked against one
independently recorded tail span. Multiple companions are not paired by ordering,
label spelling or byte similarity. Every span and its relocations must agree;
unresolved exception metadata/runtime bindings remain unknown. An unresolved
DIR32 target is not assumed to be an image address requiring a base relocation:
its binding must establish whether it is an image address or linker constant.
Missing data bindings and more complex section layouts still require work; the
production gate has not yet switched to this verifier.

The policy API distinguishes byte/instruction-shape, reference, structure and
runtime obligations. `check_obligations` returns 0 only if all required current
measurements pass. Missing/stale measurements return 3; unexpected failures
return 1; explicitly admitted matching regressions return 2, never green.
An independent review of the exact migration snapshot and passing build and
structural checks are prerequisites for admission. Closing requires restoration
of every protected obligation.

## Ownership-first tasks

`partition` derives candidate class families, existing source homes and declared
dependencies from source. Seeds contribute evidence without replacing product
definitions. Conflicting names/extents/homes remain explicit, and address
adjacency never proves a translation-unit boundary. These reports are not stored.
Current mapped member symbols drive class grouping; old catalogue/seed owners
remain visible as alternative hints, not automatic overrides of recovered source.
This includes the shared GraphicWin query and its compiler-generated adjustor.
The grouping is not structural proof and does not change historical obligations.
`--impact` adds a conservative include-based recheck scope, not a completeness
claim about object ownership or dynamic calls. Unresolved includes widen the
scope; this view is not yet used to narrow the production check cache.

Source-derived task creation and expansion run in the package, without the local
adapter. Creation takes the target from `recovery/driver.json` or `--target` and
defaults to committed `HEAD`. An explicit full `--base` revision must be an
ancestor with equivalent configured source/configuration. Dirty, ignored or
Git-hidden source bytes cannot silently enter a committed baseline. No branch
name is special. Discovery and resolution commands need only the task journal,
not a source adapter. Context and historical-restoration collection now run in
the library too. History follows each commit's configured roots, suffixes, seed
classification and image pin, including merged side histories. Rehoming or
reclassifying source does not erase prior matching or structural obligations.
The explicit `source_history` configuration describes the source layout before
this repository had project configuration, preserving the existing InfoWin
baseline. A new project configured from its task baseline needs no such fallback.
There is no remaining project-specific obligation collector.

## Configured recovery driver

`recover run` executes a bounded context → check → worker → recheck loop. The
engine imports no OpenSMACX, VC6, executable-format or agent-provider code.
Project configuration selects JSON command adapters; `recovery/driver.json`
wires OpenSMACX's existing context and strict task check into that engine.

```sh
uv run recover run infowin-ownership --config recovery/driver.json \
  --worker recovery/worker.json --reviewer recovery/reviewer.json --iterations 3
```

Set `OPENSMACX_IMAGE` and an isolated `VC6_PREFIX` for the supplied project
configuration. `worker.json` and `reviewer.json` supply distinct attribution
labels and invoke the package-owned `recover agent` bridge. `agents.json` contains
the actual provider argument arrays and role instruction paths, with no local
Python adapter. The worker bridge receives one JSON
object on stdin containing the task, authorized files, fresh context and check
report. It edits source in the provided worktree and returns only
`{"discoveries":[]}`, or discoveries containing `hypothesis`, `affects` (original
entity ids) and `evidence`. Worker output never establishes a passing verdict.

The supplied provider commands use Codex's schema-constrained final-message file,
fresh ephemeral sessions, a workspace-write worker and a read-only reviewer.
Only the worker is granted the explicit `VC6_PREFIX` directory in addition to its
worktree; never point that variable at a shared prefix or a broad directory.
Model selection and authentication use the user's CLI configuration; no model or
credential is embedded here. This configuration follows the
[official non-interactive documentation](https://learn.chatgpt.com/docs/non-interactive-mode)
and was checked against installed `codex exec --help`. No live model task has yet
validated recovery quality or this machine's nested-provider sandbox behavior.
Prepare/authorize an isolated run before using these commands. The generic bridge
also supports a single final JSON object on stdout for other provider commands.

The bridge creates request/schema/response files only under ignored
`build/recovery/agents/`, validates the response and removes those temporary files.
It rejects stale reviews, malformed output and source/Git scope violations, while
preserving edits on interruption. Provider errors reach the driver as `agent_error`;
there is no automatic quota retry or provider-specific reset-time parser yet.

Each command has an argv array and explicit timeout; no shell interpolation is
used. Supported placeholders are `{task}`, `{root}` and `{env:VARIABLE}`.
Context commands return a JSON object with exit 0. Check commands return a JSON
object with `result` equal to `pass`, `fail` or `unknown`, and respectively exit
0, 1 or 3. The complete checker response is supplied to the worker on each round.

The driver stops on a discovery needing evaluation/scope expansion, no source
change, budget exhaustion, unavailable context, interruption, or a passing check
needing independent review. Scoped edits remain in place on every stop; nothing
is automatically reverted, committed or collected. Discoveries enter versioned
task history; reproducible context/check results do not. Checkpoint new decision
records before resuming. Existing edits confined to the approved files can be
resumed; unrelated dirty work is refused.

The driver serializes execution in one worktree and detects tracked/nonignored
file, index and HEAD changes outside allowed operations, including on command
failure. **This is detection, not a security sandbox.** Use trusted adapters and
externally sandbox workers; ignored files and writes outside this worktree are
not protected by these checks. Driver commands and VC6 compilations share a bounded
runner. POSIX timeout cleanup requests termination, then kills the command process
group; a main-thread Python checker using the runner can unwind on termination
and stop its separately grouped compiler before the driver's forced cleanup.
Existing custom signal handlers are not replaced. Cleanup waits are bounded even
if a detached child keeps an output pipe open. Detached services, custom signal
handlers and Windows descendant isolation require external job/sandbox containment;
this runner never shuts down a shared Wine server. Compiler timeout/nonzero exit
never yields a usable object or a successful measurement.

Without independent review and a configured full acceptance gate, driver stops
return CLI exit 3 and `ready_for_collection:false`. The supplied OpenSMACX
configuration deliberately has no acceptance command yet: its scoped checker
cannot stand in for complete reference, structure, obligation and runtime checks.
The configured check is now `tasks check-restoration <task> --image <image>`,
which remeasures historical obligations as well as running both binary/reference
and declared-layout checks. Workers receive the joint report under `candidate`,
with `candidate.checks.binary` and `candidate.checks.layout`, including coverage gaps, even when the other
dimension fails. A passing body cannot suppress a layout failure or unknown.
Both reports must share unchanged source, task decisions, build inputs and image;
a change between stages refuses the combined result. Unknown evidence keeps the
combined result unknown. Even two passing scoped checks do not authorize collection.
The `check`, `check-binary` and `check-layout` commands remain useful diagnostics;
the joint command's `--summary` retains both verdicts and layout coverage.
The joint check measures both dimensions from fresh standalone `/Z7` objects.
Its binary trial records include that debug flag explicitly; the individual
`check-binary` command keeps its original non-debug configurations. Identical
source/command/flag requests reuse object bytes only within that invocation, with
all project inputs checked before reuse. No objects or verdicts survive into the
next check. The joint report's `compilation` reports actual compiler attempts and reuse;
the subreports count requested trials. Failed compilations are never reused.
An alternate-target test runs real subprocess worker/check adapters without
importing the OpenSMACX adapter.

The driver also runs the package-owned `check-build` command each round, using
`build.json` to configure a fresh Release CMake/Ninja tree under ignored
`build/recovery/builds/`. Four compiler jobs use the coordinator's isolated Wine
prefix; this is not a fleet/prefix-pool implementation. Build commands and expected
`OpenSMACX.exe` format are project data, not Python adapters. The check does not
reuse `build/`'s incremental objects and cleans its temporary output. Workers get
`check.build_check`, including compiler/linker diagnostics, and the original scoped
result in `check.recovery_result`. Full build/structure/outside-scope migration
admission remains unfinished even though fresh build measurement is now available.

### Independent review and acceptance

`--reviewer /path/to/reviewer.json` supplies a separate read-only command adapter
using the same `author`/`command` configuration shape as the worker. Its author
must differ from the worker's. This is trusted coordinator attribution, not
authentication; independent provider/session isolation belongs to the adapter.
The reviewer receives the task, context, check and content snapshot, and returns:

```json
{"snapshot":"<received SHA-256>","decision":"approve","evidence":["source:src/example.cpp"],"feedback":"Explain the structural judgment here."}
```

An approved reviewer response may also contain `supersessions`, an array of
`{"previous":"<interpretation digest>","replacement":"<interpretation digest>",
"evidence":["source:..."],"reason":"Why the old interpretation was wrong."}`.
The trusted inventory supplies content-identified interpretations, exposed to the
worker/reviewer as `context.review_interpretations`. Replacement requires a known
historical interpretation and a current one at the same original evidence anchor;
unknown references, self-replacement, duplicate dispositions and still-current
old interpretations are refused. Inventory changes are part of review freshness.
The decision remains snapshot-bound review history, never a measurement. Migration
and acceptance adapters receive it with the complete protected obligations and
interpretation inventory; supersession does not remove a proof obligation or waive
fresh structural/build/reference/runtime checks. Target-specific verification of
the review's reasoning and full structural admission remain unfinished.
If an interpretation was wrong and has no replacement, an approved response may
instead supply `retractions` entries with `interpretation` (its digest), `evidence`
and `reason`. Retraction is allowed only for a historical interpretation; it does
not delete history or its original-address structural obligation. Every historical
interpretation must have one explicit supersession or retraction before migration
or acceptance is invoked. A generic approval leaves the driver stopped at
`interpretation_review_incomplete`, listing the pending IDs. After checkpointing
that review, a later run asks the reviewer again rather than endlessly reusing
the incomplete approval. These dispositions remain judgments, not measured proof.

`request_changes` feeds the review back to another worker round within the same
budget. Source changes require a new review. Evidence-backed decisions are kept
in the `review:<task>` JSON history stream, not as measurements or source comments.
The snapshot covers tracked/nonignored source, task decisions and configuration;
only its own review stream is omitted to avoid a circular fingerprint. Committing
a review alone does not invalidate it. Review history is workflow metadata, not
an allowed build input. Compiler/image freshness remains the target gate's job.

On a passing scoped check and approved review, the optional project `acceptance`
command runs afresh. It must implement the actual full-project checks—not echo
worker claims—and return exactly `scope: "full_project"`, an overall `result`,
and `build`, `structure`, `references`, `obligations`, `runtime` outcomes. Outcomes
are `pass`, `fail` or `unknown`; an unknown dimension yields overall unknown/exit
3, otherwise any failure yields fail/exit 1. Only all passing dimensions with
exit 0 can produce `ready_for_collection:true` and driver exit 0. Missing fields,
scoped reports and contradictory exits are refused. The target adapter must
verify pinned inputs, original boundaries and every preserved obligation; these
are contracts for trusted verifier code, not proofs obtained by adding JSON keys.

Acceptance is never cached by this driver, even when a source-identical review is
reused. Readiness is a current output, not a durable status or permission to collect
a later changed tree. Collection execution and its final revalidation remain to
be implemented.

At budget exhaustion an optional reviewer can also assess a still-mismatching
structural checkpoint. Approval preserves its interpretation and source, but
returns `structural_checkpoint_reviewed`, never collection readiness. Connecting
this review to migration admission requires the explicit configuration below;
structural approval alone does not admit or demote a byte obligation.

### Protected migration execution

`uv run recover tasks check-preservation <task> --summary` now supplies one
native prerequisite: fresh baseline/current COFF comparisons outside source-owned
task functions, with per-TU replica and per-changed-file compiler damage controls.
Independent data, linker metadata, relocation targets and outside source
annotations are preserved too. It covers configured non-seed TUs by default;
`--unit` is diagnostic filtering, never a narrower acceptance claim. No local
Python preservation adapter or saved measurement ledger is required.

`--unit src/infowin.cpp --diagnostic --summary` provides a cheaper investigation
path: fresh direct/replica/historical compiles, no per-file damage probes, and
always an unverified exit 3. Full output identifies changed code/data dimensions,
symbol-table records and exact relocation targets; the summary retains source
navigation hints and unbound linker symbols. No report field is scope permission
or proof that an annotation identifies the emitted definition.

It does not yet supply the full `outside_scope` admission result. The current
compiler policy is used for both source versions; full link/runtime/lifetime and
historical build reconstruction are separate work. Compiler v5 now materializes
added/deleted/renamed headers and seeds without leaving newer headers visible in
the old tree. Changed product-TU file sets still require explicit recipe/coverage
mapping. Unobserved seed inputs remain unknown, not silently excluded. The existing
InfoWin task has historical seed and annotation changes; materialization alone
does not discharge its preservation or restoration requirements. Use the report to
identify coverage and collateral changes; do not configure it as a complete
migration checker. Package documentation describes the exact comparison boundary.

The optional project `obligations` command returns a source-derived `obligations`
array (entity id, proof dimension, baseline and validated obligation id; source
evidence may accompany each row). The driver reads it before worker execution
and unions subsequent inventories into the protected set. A later smaller
inventory cannot discard a requirement. Workers receive this set in context;
full acceptance receives it as `protected`. OpenSMACX configures this reader.

To admit a temporary matching regression, supply `run --migration permission.json`
and configure a trusted project `migration_check` command. The permission is a
coordinator decision, suitable for tracked JSON, with exactly these fields:

```json
{"migration_id":"member-layout","author":"task-coordinator","evidence":["source:layout evidence"],"allowed":[{"entity_id":"entity-<SHA-256>","dimension":"bytes"}]}
```

Only `bytes` and `instruction_shape` regressions within task scope may be allowed.
The permission is part of the independent review's snapshot policy. It does not
copy measurements or grant worker authority to change scope. Actor labels remain
trusted coordinator attribution, not authentication.

The migration verifier receives the task, reviewed snapshot, protected requirements
and permission. It returns exit 0 only for a completed report, containing exactly
`snapshot`, `changed` (original entity ids), `build`, `structure`, `references`,
`outside_scope`, `measurements`, and `current`. Build/structure/references/outside-scope
outcomes must be `pass`, even when historical source protected only byte claims.
Each measurement has `obligation_id`, input `fingerprint`, `result`, and `evidence`;
`current` maps obligation ids to fresh verifier-computed input fingerprints.
These are trusted verifier outputs, not worker self-assessments. Missing, stale
or unknown measurements cannot be admitted as matching regressions.

The driver applies the shared migration policy after independent approval.
Allowed fresh failures produce `migration_admitted` and CLI exit **2**, preserving
the source but never collection readiness. A resumed run re-derives obligations
from source history and rechecks them; only complete restoration plus the full
acceptance gate can yield readiness/exit 0. Admission itself is a disposable gate
result, not a stored verdict. Source and review/permission decisions must be
checkpointed for resumption. OpenSMACX does **not** yet configure a complete
migration verifier: its scoped binary checker is insufficient to prove layout,
build and outside-scope preservation. Real-command synthetic tests exercise this
protocol; they are not evidence of a completed game recovery pilot.

## Task commands and source bindings

```sh
uv run recover tasks create example-layout --group class:InfoWin --author coordinator \
  --intent "Recover InfoWin storage ownership" --evidence "source:src/infowin.h"
uv run recover tasks status example-layout
uv run recover tasks context example-layout
uv run recover tasks obligations example-layout
uv run recover tasks check-layout example-layout
uv run recover tasks check-binary example-layout --image /path/to/the/pinned/original.exe --summary
uv run recover tasks discover --help
uv run recover tasks expand --help
uv run recover tasks resolve --help
```

`tasks check-binary` now executes fresh bounded byte/reference checks, sharing
the dependency verifier rather than introducing a second matching algorithm.
It compiles each selected product TU once under each of the two established
OpenSMACX flag sets, then measures all selected functions from those objects.
Exact source-owned function symbols can resolve cross-TU calls. Ambiguous names,
missing build entries, unsupported extents and seed-only subjects remain unknown;
no compilation command is silently borrowed and no old body is promoted.
Explicit object-start bindings are read from source; missing data/import bindings
remain unknown.

The command returns 0 only if every selected subject has a complete passing
trial, 1 if there are failures and no unknown subjects, and 3 if any subject is
unmeasurable. Failure in one flag set does not erase an unmeasurable alternative.
It rejects source/build-input or task-decision changes during the run and writes
neither source claims nor measurement history. Its source fingerprint is a
diagnostic identifier, not a toolchain-complete reusable cache key.

This command is **not an admission gate**: it does not yet prove dependent-caller
coverage, layouts, ownership, lifetimes, full linking, runtime behavior or
independent review. A passing scoped binary result never makes a task ready for
collection. The existing production gate remains required during cutover.

An original global object's identity belongs beside its definition:

```cpp
// RECOVERY_OBJECT: v1 0x007AD2A0 ?ConsoleInfoWin@@3VInfoWin@@A
InfoWin ConsoleInfoWin;
```

The shared `src/IMAGE_PIN` supplies the original image identity. The annotation
names the object's start, not a pointer containing an old executable address.
The target adapter requires that exact data symbol to be defined in the owner's
compiled TU before using it to resolve references. Member accesses then use the
compiler's relocation addend; they cannot obtain an offset from the original
operand to make a check pass. Duplicate object symbols/start addresses and
function/object symbol conflicts are refused. Seeds do not replace live owners.

This compact mapping asserts no object extent, initialization, lifetime or class
layout. Those need independent source evidence and checks. The InfoWin pilot
replaces the independent redraw global with a real member at +0x90, retaining
the offset evidence beside that member. The remaining raw subobjects and unknown
object extent are still unfinished structural recovery, not excused by this fix.

Task files fix the coordinator's chosen subjects, edit permissions, intent and
committed source baseline. They do not duplicate function mappings or current
match results. Discoveries are hypotheses with affected original identities and
evidence. An out-of-scope discovery remains pending and requests scope review;
expanding editing permission does not resolve it or authorize a matching
regression. Resolution records incorporation into source or an evidence-backed
rejection. Fresh build/structure/reference/matching checks and independent review
are still required; these commands cannot declare a task verified or collect it.

Decisions use revision-checked, idempotent JSON events. A fresh checkout can resume
them; deleting private caches loses no scope decision or pending discovery. Actor
names are attribution within the trusted coordinator API, not authentication for
untrusted worker processes. Agent isolation and execution are not implemented yet.

`tasks context` rebuilds the brief from current source and decisions, including
seed locations, source homes, pending discoveries and conservative recheck paths.
It also flags lifetime annotations bound to ordinary wrappers when a real
constructor/destructor definition exists in the same source file. These are
measurement candidates, not automatic symbol corrections: BaseButton's obsolete
wrapper bindings had hidden already-matching real lifetime functions. Missing or
ambiguous definitions yield no hint, and the measured symbol remains unchanged
until an explicit source correction.
It also includes the restoration inventory exposed by `tasks obligations`.
That inventory is recomputed from source at the task's committed baseline,
every subsequent reachable source revision (including merged side histories),
and the working tree. Removing a claim or moving its body cannot erase its
restoration requirement. Duplicate source/seed claims are one requirement per
original entity and proof dimension; their source locations remain navigable
evidence. Image-pin changes or an unavailable/nonancestor baseline are refused.
No obligation report or measurement is saved. This preserves claims from the
task baseline onward, not arbitrary earlier history. An uncommitted claim that
is introduced and removed between runs cannot be reconstructed; banking new
claims must include a source checkpoint. The driver freezes these requirements
across worker execution when its `obligations` command is configured.
Product-source `RECOVERY_MEMBER` interpretations also create a structural review
requirement anchored to their original evidence address, when that address is in
task scope. Their current and historical class/member names, offsets and source
locations travel as evidence; rehoming, renaming or deleting the annotation does
not erase the requirement. These are **not historical layout PASS claims**: a
later interpretation may correctly supersede an earlier one. A future structural
admission verifier must reconcile that history through evidence and independent
review, not demand all historical spellings/offsets simultaneously or equate a
current offset check with a reviewed disposition. Seed annotations remain hints,
not product structural obligations. No extra ledger is written.
It writes no context file. A coordinator can use the output directly or cache it
in ignored storage; neither a stale brief nor a resolved hypothesis is a verdict.

## Still under construction

Compiler-generated vtables can carry source-owned original identities without
defining any tables or writing vptrs in C++:

```cpp
// RECOVERY_GENERATED: v1 vtable 0x00670C60 ? ??_7PushButton@@6BWin@@@ 0x0062C010
```

The fields are original start, exclusive end (`?` when unestablished), exact
emitted symbol, and original evidence address, qualified by the source image pin.
The application verifier requires a compiled, isolated read-only data COMDAT
before using a binding to resolve table references. It separately reports table
contents: unknown extents stay unknown; declared extents compare all bytes and
slot relocations against independently mapped targets. A candidate's size cannot
invent the original boundary, and one passing emitted copy cannot hide another
copy's disagreement. PushButton's two table starts now resolve while their
contents remain unverified. Full class/hierarchy proof is still separate.
Table identities, symbols and independently declared extents also participate in
source-history review, anchored by their original evidence address. Rehoming does
not change an interpretation's identity; changing or removing a mapping preserves
the old interpretation for explicit independent supersession or retraction.
Neither that review nor an unknown extent establishes table-content correctness.

Direct-member expectations are load-bearing source comments beside declarations:

```cpp
// RECOVERY_MEMBER: v1 InfoWin redraw_phase_ 0x90 0x004E09E0
int redraw_phase_;
```

For an embedded class object, `RECOVERY_MEMBER: v2` adds its expected class-value
type before the offset: `// RECOVERY_MEMBER: v2 InfoWin timer_ Time 0x30 0x00459500`.
Both offset and class-value type must agree. A pointer, scalar, array or different
class cannot substitute just because it occupies the same offset; unsupported
type records remain unknown. This is not full qualifier/array/lifetime analysis.
Validated `const`/`volatile` type wrappers preserve class-value identity; the
checker never follows a pointer through to its pointee to claim an embedded
object. Cyclic, truncated or unsupported modifier records remain unknown. These
wrappers do not add a claim about the original object's qualifiers.
The InfoWin pilot now declares `Time`, `Font` and `PushButton` expectations using
its original constructor calls as evidence. Strengthening or changing a type
preserves the previous interpretation for independent review; no byte claim is
banked by these annotations.

Nonvirtual base interpretations use `// RECOVERY_BASE: v1 Derived Base 0x0 0x00401000`
beside the class declaration: derived class, base class, original subobject offset,
and original evidence address. This does not infer inheritance from an offset alone.
The checker requires the compiler to emit the named nonvirtual base at that offset;
a flattened field or virtual base cannot substitute for it. Missing class debug
information remains unknown, not a pass. Base interpretations participate in the
same source-history preservation and independent supersession/retraction review
as member interpretations. The report's `bases` section contains their trials;
summary mode retains combined verdict counts and coverage. No original base
interpretation is automatically created from the candidate compiler's layout.

The fields name the owning class, member, original offset, and original evidence
address (qualified by the source image pin). Surrounding prose explains the
interpretation; no current compiler result is stamped here. `tasks check-layout`
compiles scoped product TUs with both VC6 flag sets and standalone `/Z7`, then
compares strict CodeView direct-member offsets with the scoped source expectations.
Every requested TU/configuration must agree. PCH use, ambiguous class definitions,
unsupported/truncated field lists and missing observations are unmeasurable.
`tasks check-layout <task> --summary` omits repeated per-member details but keeps
offset verdict counts and per-class/TU/configuration coverage. `not_emitted` means
the standalone type stream has no complete definition of that class;
`unmeasurable` means compilation or strict parsing failed. Neither waives that
TU's required observation. Coverage reports suggest whether the next step is a
layout witness/relevance investigation or a compiler/parser repair; they are
fresh diagnostics, not source claims or stored task state.
Observed definitions also expose the compiler's class size, direct base names
and nonvirtual base offsets, plus directly class-typed value members. Pointer
and array members are not mislabeled as embedded objects; this list is not a
complete type/ownership analysis. Virtual bases retain their debug metadata but
have no asserted static base offset. These are observations of the reconstructed
C++, not independently recovered original hierarchy facts, and do not upgrade
the direct-offset check to full structural verification. Malformed base references
remain unmeasurable rather than yielding a partially accepted hierarchy.
Scoped product method mappings also supply class-name search hints, even before
any member-offset expectations exist. Their current compiler layouts appear with
`basis: mapped_method_hint` and the originating entity IDs. This exposes candidate
bases and embedded members to the recovery worker without inventing original
layout claims. Missing/incorrect hints stay diagnostic; they cannot weaken declared
expectations or create a vacuous structural pass. Seeds and unrelated task entities
do not trigger these product compilations. All classes in one TU/flag set share
the existing object, including the joint check's in-memory compilation reuse.
No debugger-style partial field lists or type-index voting supplies gate evidence.
InfoWin's `redraw_phase_` expectation passes in both Console and InfoWin units.
This checks declared member offsets and nonvirtual base relationships, **not**
completeness of the declarations, original evidence interpretation, full hierarchy,
member types, full object size or lifetime.
The result therefore leaves full structural verification unknown. Layout
interpretation history now survives in the source-derived restoration inventory;
explicit reviewed supersession is supported, but the full structural admission
verifier remains unfinished.

The package is not yet the production gate. The durable task runner, target
verification, complete dependency checking, source annotation migration and performance
infrastructure are tracked in `docs/recovery/FRAMEWORK.md` at the repository
root. No compatibility wrappers will be retained at cutover. Removed tools'
useful verification responsibilities must be replaced, not silently dropped.
