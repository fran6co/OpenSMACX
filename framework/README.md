# Match recovery

An independently installable Python package for structure-led matching
decompilation. The core uses only the Python standard library. OpenSMACX's
one-time legacy importer additionally uses the old `decomp` package supplied by
this workspace; it is not a dependency of the portable core.

## Available now

From the OpenSMACX workspace:

```sh
uv run recover --help
uv run recover inventory
uv run recover partition
uv run recover partition --group class:InfoWin --details --impact
uv run recover import legacy --image /path/to/the/pinned/original.exe
uv run recover components list
uv run recover components check-source zlib --archive /path/to/zlib-1.0.2.tar.gz
uv run recover components check-binary zlib --image /path/to/the/pinned/original.exe --summary
uv run pytest framework/tests
uv build --package match-recovery
```

Reports are JSON on stdout. Errors go to stderr and return exit 3. Inventory never
changes source or decisions. Import stores a content-addressed snapshot under the selected root's
ignored `.opensmacx/recovery/objects/` directory and prints its id. It does not
rewrite source, declare current matches, or choose between conflicting source
candidates. Raw files preserve all notes, including unparsed or non-UTF-8 text.
The legacy adapter also memoizes individual files under ignored
`build/recovery/parse_cache/`, only when Git confirms that directory is ignored
and contains no tracked files. Otherwise parsing remains uncached.
Keys include source bytes, relative/resolved path
shape, image pin and parser/adapter implementation digests; timestamps are not
authority. Entries have integrity checks and atomic publication. Missing, damaged
or unwritable caches fall back to parsing. Set `RECOVERY_PARSE_CACHE=off` to bypass
this memo, or delete its directory. Source inventory and content race checks still
run on every snapshot; no compilation or verification verdict is cached here.

The snapshot is a disposable parse cache, not durable history. Imports do not
write versioned inventories or measurement reports. No database is required.

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
upstream files themselves unmodified. The zlib importer recovers original spans,
symbols and compilation units from the retired wrappers' explicit historical
metadata, not from the candidate's length. Its 39 bindings are dependency checks,
not application recoveries. The source-integrity command derives file hashes from
the pinned upstream archive; without that archive it returns unknown (exit 3).
An altered source or wrong archive returns failure (exit 1). Even a passing source
check does not establish binary matching or ABI correctness.

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
correctness. Missing data bindings and multi-span section placement still require
implementation; the production gate has not yet switched to this verifier.

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
`--impact` adds a conservative include-based recheck scope, not a completeness
claim about object ownership or dynamic calls. Unresolved includes widen the
scope; this view is not yet used to narrow the production check cache.

## Configured recovery driver

`recover run` executes a bounded context → check → worker → recheck loop. The
engine imports no OpenSMACX, VC6, executable-format or agent-provider code.
Project configuration selects JSON command adapters; `recovery/driver.json`
wires OpenSMACX's existing context and strict task check into that engine.

```sh
uv run recover run infowin-ownership --config recovery/driver.json \
  --worker /path/to/worker.json --iterations 3
```

Set `OPENSMACX_IMAGE` and an isolated `VC6_PREFIX` for the supplied project
configuration. A trusted worker configuration supplies an attribution label and
an adapter command, for example `{"author":"worker-name","command":{"argv":
["/path/to/agent-adapter"],"timeout":900}}`. This is a protocol contract, not a
bundled agent-provider implementation. The worker adapter receives one JSON
object on stdin containing the task, authorized files, fresh context and check
report. It edits source in the provided worktree and returns only
`{"discoveries":[]}`, or discoveries containing `hypothesis`, `affects` (original
entity ids) and `evidence`. Worker output never establishes a passing verdict.

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
The configured check is now `tasks check <task> --image <image>`, which runs both
binary/reference and declared-layout checks. Workers receive both reports under
`checks.binary` and `checks.layout`, including coverage gaps, even when the other
dimension fails. A passing body cannot suppress a layout failure or unknown.
Both reports must share unchanged source, task decisions, build inputs and image;
a change between stages refuses the combined result. Unknown evidence keeps the
combined result unknown. Even two passing scoped checks do not authorize collection.
The individual `check-binary` and `check-layout` commands remain useful diagnostics;
the joint command's `--summary` retains both verdicts and layout coverage.
The joint check measures both dimensions from fresh standalone `/Z7` objects.
Its binary trial records include that debug flag explicitly; the individual
`check-binary` command keeps its original non-debug configurations. Identical
source/command/flag requests reuse object bytes only within that invocation, with
all project inputs checked before reuse. No objects or verdicts survive into the
next check. The top-level `compilation` reports actual compiler attempts and reuse;
the subreports count requested trials. Failed compilations are never reused.
An alternate-target test runs real subprocess worker/check adapters without
importing the OpenSMACX adapter.

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
`snapshot`, `changed` (original entity ids), `build`, `structure`, `outside_scope`,
`measurements`, and `current`. Build/structure/outside-scope outcomes must be `pass`.
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

Direct-member expectations are load-bearing source comments beside declarations:

```cpp
// RECOVERY_MEMBER: v1 InfoWin redraw_phase_ 0x90 0x004E09E0
int redraw_phase_;
```

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
No debugger-style partial field lists or type-index voting supplies gate evidence.
InfoWin's `redraw_phase_` expectation passes in both Console and InfoWin units.
This checks declared offsets, **not** completeness of the declarations, original
evidence interpretation, inheritance, member types, full object size or lifetime.
The result therefore leaves full structural verification unknown. Layout
interpretation history now survives in the source-derived restoration inventory;
explicit reviewed supersession is supported, but the full structural admission
verifier remains unfinished.

The package is not yet the production gate. The durable task runner, target
verification, complete dependency checking, source annotation migration and performance
infrastructure are tracked in `docs/recovery/FRAMEWORK.md` at the repository
root. No compatibility wrappers will be retained at cutover. Removed tools'
useful verification responsibilities must be replaced, not silently dropped.
