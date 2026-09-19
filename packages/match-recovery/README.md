# Match recovery

An independently installable Python package with a standard-library-only core for
structure-led matching decompilation. It does not register projects, import a
project's Python code or bundle a game's adapters. Project integration is local
configuration plus commands exchanging JSON. Those commands can use any language.

## Explicit installed framework runtime

`recover runtime inspect --python /absolute/venv/bin/python` describes an existing
noneditable installation without installing or updating anything. Its JSON pins
the interpreter, package, full site-packages contents and virtual-environment
configuration. Use an isolated venv without system site packages or bytecode
caches. Execute its interpreter with `-I -B`; source-tree/editable launchers cannot
activate a binding to a different installation, even with identical package code.

A coordinator-owned binding file has this shape (insert the complete returned
descriptor as `runtime`):

```json
{
  "schema_version": 1,
  "runtime": {},
  "launchers": [["uv", "run", "recover"]]
}
```

Invoke the selected interpreter as `-I -B -m match_recovery --runtime
/absolute/binding.json <subcommand>`. The global option precedes the subcommand.
Only exact declared launcher prefixes are redirected; enumerate other project
spellings explicitly. Compiler/provider commands otherwise remain unchanged.
Project Python protocol clients that import the framework can explicitly declare
`["{framework_python}", "-I", "-B", "recovery/compiler.py"]` as their command
argv. Only the complete executable token is expanded: it selects the bound
installation's interpreter, or the current interpreter without a binding. Flags
and remaining arguments are preserved; `-I -B` are explicit project choices.
This avoids a checkout-local `uv run python` loading an older package than its
caller. It does not copy adapter code, change ordinary Python/compiler/provider
commands, or propagate native CLI authority into arbitrary script descendants.
The script must arrange any project-local imports explicitly under `-I`.
Changing a frozen campaign's command configuration still needs separate authority;
runtime reauthorization alone does not grant that change.
Native child drivers carry the binding explicitly, and measurement executors
propagate it to their own workers. Configured command boundaries and verification
fingerprints recheck its identity; runtime changes refuse continued use rather
than replaying earlier evidence. Interrupted commands retain cleanup/quarantine
behavior. Bound command children also receive `PYTHONDONTWRITEBYTECODE=1` so
build tools invoking Python through a shebang cannot write caches into the pinned
installation. This does not change compiler arguments or the parent's environment.
Runtime validation reenumerates installation membership and checks aliases,
nonregular inputs and bytecode on every observation. On coherent POSIX
filesystems, a bounded process-local cache reuses file SHA-256 values only while
device/inode, mode/link count, size, mtime and ctime remain unchanged; forked
children start empty. `MATCH_RECOVERY_RUNTIME_CACHE=off` forces fresh byte reads
and should be used on filesystems without coherent metadata. Other platforms
default to fresh reads. Package and environment digests share one installation
observation. This caches neither binding authority nor verification results.
Installation is not a hermetic OS/Python proof: content-pinned regular
files may be hardlinked by the package installer, but symlinks within the installed
package/environment and aliased configuration paths are refused.

This mechanism does not authorize changing frozen campaign inputs. To resume a
preserved campaign with this installed runtime, add `campaign --resume <run>
--runtime-authorization <decision.json>`. The global runtime option remains
required. The separate coordinator decision has these exact fields:

```json
{
  "schema_version": 1,
  "author": "the-campaign-coordinator",
  "run_sha256": "canonical-digest-of-original-run.json",
  "batch_sha256": "canonical-digest-of-latest-batch-workers.json",
  "profiles_sha256": "canonical-digest-of-current-effective-profiles",
  "destination_revision": "full-current-destination-commit",
  "runtime_binding": "/absolute/binding.json",
  "runtime_sha256": "canonical-digest-of-complete-binding-document",
  "evidence": ["Why this explicit installed framework transition is authorized"]
}
```

Digests use the package's canonical JSON encoding and SHA-256, not formatted JSON
file bytes. The destination must be current and retain assignment ancestry.
The package freezes preserved workers and existing integration candidates before
publishing an immutable run-local `runtime-authorization.json`. Busy checkouts,
changed source/profiles or stale decisions refuse. No project profiles, compiler
hooks, task baselines or original campaign manifests are rewritten. Subsequent
resumes require the same active installation/binding and reread the decision;
exact permission retries are idempotent. Normal authorized destination/batch
progress does not require editing that historical decision. A second runtime
selection requires a new decision with `schema_version: 2` and the additional
`previous_authorization_sha256` field naming the canonical digest of the current
effective grant. The original grant remains unchanged; successors are published
as `runtime-authorization-2.json`, `runtime-authorization-3.json`, and so on.
The chain must be contiguous, with each successor bound to its immediate
predecessor and fresh destination/batch/profile authority. Only the effective
tail's installation must be present; retired runtimes are not probed or executed.
Replaying an earlier permission cannot roll back that authority, and each
transition requires the same stopped-worker/candidate freeze. New campaigns with an active
binding currently refuse; this transition is deliberately for preserved runs.
Runtime selection grants no source scope, claim banking, review or acceptance;
all native verification remains fresh under the selected implementation.

## Direct recovery diagnostics

### Compiler-file observations

`match_recovery.compile_file_evidence` reads version-1 compiler-boundary observations
with `parse_compile_files(bytes)` and freshly checks them with
`verify_compile_files(bytes, object_path=...)`. The optional object path binds the
observation to that exact file, not a same-named substitute. The schema contains
`source`, `tool`, effective `cwd` and ordered `arguments`, an `inputs` array of
`{path, before, after}`, `output: {path, existed_before, after}`, and `exit_code`.
Paths are absolute host paths; content observations are `{sha256, size}` or null.
The source and tool must be among the unique inputs. A pass requires exit zero,
unchanged freshly read declared inputs and a new, nonempty, unchanged output.
Missing observations and preexisting outputs cannot pass. Keep observations in
ignored build storage and read them during the enclosing fresh build epoch.

This is a compiler-neutral file-evidence boundary, not an execution attestation,
complete include/source inventory, interpretation of opaque compiler arguments,
linker-selection proof or acceptance. It does not yet admit changed build graphs
or relax outside-scope preservation. Those require independently bound application
build commands, source/object/link correspondence and remaining linked effects.

`match_recovery.compile_link_evidence.verify_compile_link_inputs(compilations,
link_data, log_data, sources=...)` connects those observations to the exact
explicit linker-input files. Supply compile observation bytes, link-file and
verbose-log bytes, and an explicit expected set of absolute source paths from
the current build context. It freshly verifies all observations, requires unique
source/output identities and one explicit link occurrence per compiled output,
and compares compilation-output and pre/post-link content stamps. Missing sources
stay unknown; extra sources, duplicate identities and substituted objects cannot
pass. Remaining explicit link inputs are reported separately, not claimed as
application-source coverage. The caller must hold the enclosing build context
unchanged. This verifies file correspondence only: it neither proves that the
expected source set is correct nor compares compiler arguments, build graphs or
retained linker sections.

To require this evidence during every native fresh build, add an optional
`compile_link` object to the build configuration, for example:

```json
{
  "database": "compile_commands.json",
  "source_extensions": [".cpp"],
  "directory": "objects/application",
  "suffix": ".compile.json",
  "link_files": "application.link.inputs.json",
  "link_log": "application.link.log",
  "command_stages": {"compile": "compiler-effective", "link": "link-wrapper"}
}
```

Paths are relative to the new build directory. The database, link-file observations
and log must also be declared outputs with formats `compilation-database`,
`link-input-files` and `msvc-link-log`. The configured compiler writes observation
files; the library does not infer compiler flags, filenames or source languages.
The literal extensions select expected recipes; duplicates refuse rather than
collapsing into a set. The collector requires canonical source/cwd paths inside
the project and observed outputs inside this build, verifies artifact hashes from
the current build, and rechecks declared file bytes and observation membership.
Missing or failed correspondence prevents a passing build result. Unconfigured
builds retain artifact-only verification. This extension uses the current MSVC
link-observation backend; it is not a universal linker parser or build-graph
acceptance. Sources/libraries outside this declared evidence scope remain visible
limitations, not implicit exclusions from recovery.

Optional `command_stages` additionally requires the configured compiler JSON client
to describe the freshly captured command batch. Stage names are project-defined;
they explicitly distinguish effective compiler arguments from wrapper arguments.
The request is `{schema_version: 1, operation: "command_paths", profile: <profile>,
commands: [{arguments: [...], cwd: <absolute host path>, stage: <stage>}, ...]}`.
The response is exactly `{schema_version: 1, descriptions: [...]}`, with one
description per command in request order using the schema below. The operation
describes syntax only, without compiling or providing a comparison verdict.
Unsupported syntax, missing descriptions, changed compiler inputs or failed client
execution refuse verification; there is no fallback to artifact-only acceptance.

The native collector binds every described compilation source and object output
to its verified file observations, requires described input files to be observed,
and checks the exact ordered explicit linker-input sequence. Directory roles must
name canonical physical directories, and output paths must stay in the fresh build.
All artifact/input hashes and observation membership are rechecked after the batch.
The returned `command_bindings` is fresh evidence, not a persisted source ledger,
complete include-search closure or permission to integrate a changed build graph.

For comparisons that need live build files, use the context manager
`match_recovery.application_context.application_build(root, roots, files,
versioned_files=...)`. `files` supplies a complete byte map beneath explicit
source roots; `versioned_files` must contain exactly the application's declared
`Build.versioned_inputs`. The caller selects those versions from authorized
history: this helper does not select a baseline or infer authority from a branch.
It creates a private project with current compiler/build policy, copies declared
ignored inputs, and runs the configured application build—not preparation.
Unrelated ignored caches are not copied. Compiler/toolchain policy files cannot
be changed by source/graph substitution.

The yielded value exposes `root`, `directory`, a defensive-copy `report`, and
`check_current()`. Both original and private source/policy/Git inputs stay guarded;
declared artifacts and configured compiler/link evidence are rechecked against
the original build epoch, including receipt bytes and membership. A self-consistent
new receipt cannot refresh that epoch. Use the evidence inside the context; private
files are removed on exit by default. Explicit `retain=True` preserves the complete
ignored replica for artifact inspection, but later freshness checks still refuse.
Unconfigured evidence
remains unverified. This API provides fresh builds, not baseline/candidate graph
equivalence, linked-effect preservation, review or permission to publish a change.

Inside each live application context, the collector verifies compiler/link
semantics and command descriptions once, then retains that immutable validation.
Each freshness check still hashes every unique observed file, rescans receipt
membership, and checks physical paths, recipe/command directory roles and pre/post
file identities. Original/private source, policy, Git, compiler environment and
toolchain guards remain active. This is invocation-local reuse, not a disk or
mtime-only cache; detached reports cannot supply or refresh the validation.
Every new context verifies independently, and all capabilities expire on exit.

`match_recovery.command_paths` supplies lossless command syntax comparison for
build-graph verification. `validate_command_paths(arguments, cwd, description,
stage=...)` checks the project environment's explicit path slices; it does not
parse compiler options. A version-1 description contains `stage`, `codecs`,
`operands`, and an `unsupported` list. Each codec has an absolute host `anchor`,
literal `prefix` and `separator`; each operand names `argument`, `start`, `end`,
absolute host `path`, `kind` and codec index. The encoded path must reproduce its
exact argument slice. Kinds distinguish source, input file/directory and output
file/directory. Unsupported grammar and overlapping slices refuse.

`compare_commands(before, after, path_pairs=...)` revalidates these values and
compares every token and literal character outside explicitly paired paths.
Pairs come from the enclosing verifier, not the compiler parser; descendants
retain their full relative names and nested pairs use the most specific anchor.
Roles, codec syntax, cwd, stage, duplicate arguments and ordering remain visible.
There is no token-deletion or flag-normalization interface. This is pure syntax
evidence: callers still need fresh file/command capture, authorized source/output
bindings, compiler/SDK policy and linked-effect verification. It does not make a
changed build graph acceptable by itself.

Native `tasks check-migration` now consumes application graph evidence when all
non-source policy differences are content changes to explicitly declared application
`versioned_inputs`. Changing compiler/toolchain helpers, preparation policy, graph
declarations or graph file kind/mode cannot use this route. The graph files and
every changed source file must also be coordinator-authorized editable paths.
The verifier reads the original task/integration preservation baseline, holds both
application contexts live, checks actual source/object membership and compares common
compiler identities and commands. Only whole linker arguments naming independently
bound added/removed application objects may be projected out; each is reported.
Remaining objects, libraries, options and ordering must compare exactly through the
derived root/build-directory pairs. No provider-supplied correspondence is accepted.
The candidate build is reused rather than run a third time.

The live graph session now spans restoration and preservation, not just the build
comparison. Preservation keeps each actual compiler trial's immutable object bytes
in memory with its source version, original baseline, task decisions and flags.
Native compile/link membership binds that trial to the corresponding application
object; a different trial cannot borrow a favorable match. A guarded batch indexes
membership once per build, checks each object against its fresh receipt, and refuses
if either build or the original authorization changes before the session closes.
Whole-object replica agreement is reported per unit/version/flag set. It establishes
correspondence only: even matching objects cannot waive an outside-scope residual.
Differing application and measurement output remains unknown for this binding.
These disposable diagnostics neither change task baselines nor grant linked-effect
coverage, and they cannot be supplied back as proof.

While both application builds remain live, native graph verification also compares
the MSVC `.CRT$XCU` initializer contribution when its linked artifacts are available.
The strict map reader retains owner-qualified static functions and bounded section
contributions; absent tables remain absent evidence. The verifier binds actual
pointer slots and independently bounded initializer code to physical selected COFF
inputs, map placements and PE loader relocations. Every explicit possible contributor
needs final-pass representation. Ambiguous archive candidates establish absence only
when all supported records lack the named section. Unsupported long section names
remain unknown. Registration order and multiplicity are preserved; no compiler-local
name or shared guard authorizes deduplication. Exact ordered code agreement still
leaves callback/data contents, initialization effects and residual ownership unproved.
This bounded table comparison is consumed by migration diagnostics, not a new CLI
or a reusable preservation waiver. It does not cover other startup mechanisms.

Initializer comparison also follows code-valued COFF relocations to independently
selected helper and callback definitions, including tail jumps and address operands.
Physical final-pass identities and archive occurrences select the objects; map
names or equal callback addresses do not prove their contents. The rooted code
graph retains ordered references, shared definitions and linked-address aliases.
Its node numbering does not depend on generated local function spellings. Changed
selected helper code is therefore a counterexample even when the initializer's
own bytes still agree. Missing, ambiguous or unsupported target definitions remain
unknown. `ordered_code_correspondence` reports the original table/body dimension;
`code_dependencies` reports this additional prerequisite. Neither proves data
contents, indirect targets, guard behavior, registration effects or runtime safety.

Ordinary undefined pointer references can terminate at independently parsed PE
loader slots. Their map placement, absolute relocation, zero addend and actual
PE import identity must agree; misleading COFF function type hints do not turn an
IAT slot into a function. The graph preserves module/export-or-ordinal identity
and slot sharing. This does not establish import-library intent, ABI or the loaded
DLL implementation. `input_consistency` distinguishes selected-input/actual-PE
mismatches from differences between two internally consistent builds. The former
fail the graph prerequisite; the latter still require scope/effect classification.

Command agreement remains an **UNKNOWN** migration prerequisite until remaining
linked effects are proved. Initializer ordering, common allocation, COMDAT selection,
independent data and unowned code/references cannot disappear with an authorized TU
addition. Existing restoration/preservation checks still run; the graph route neither
waives their findings nor resets a baseline. Reports and temporary builds are
disposable evidence, not acceptance records or a new command-line workflow.

Original-entity scope and permission for byte regressions alone do not authorize
a new startup registration. The explicit `generated_changes` decision described
below now supplies independently reviewed intent, tied to a source owner and an
exact versioned occurrence. Native startup coverage binds its bounded compiled
footprint before projection. An intentionally authorized registration need not be
proved free of allocation effects, just as an owned function may intentionally
change its calls. That authority does not extend to other callback/helper bodies,
storage definitions or linker-selected implementations. Complete outside
dependency/residual composition is still required before this route can admit a
change; it is not full-game runtime equivalence.

`recover inventory` reports source and seed populations, body-presence states,
historical obligations and mapping conflicts. Body states derive from source
text; an implemented or historically matching body is not completed recovery.
Build membership and fresh compilation belong to the mapping/build checks.

`recover inspect <address-or-symbol> [--in <source-file>]` shows the source
mapping, extracted region and historical hints without a compiler. Add
`--image <original>` for pinned, bounded disassembly of every declared span and
ordered call sites. `--callers` additionally searches mapped spans for incoming
direct calls; it is not whole-binary coverage or dynamic dispatch resolution.
Duplicate source/seed mappings require an exact host and remain visible as
alternatives. Imports and unknown targets are not hidden by neighboring labels.
Incomplete decoding and exhausted `--max-bytes` budgets remain explicit.

Inspection and selected-body measurement reuse the existing content-keyed source
inventory for selection and global mapping relationships, then parse full source
records only in selected hosts. All mappings still participate in ambiguity,
shared-span and caller checks; selected identities must agree with their captured
source. Normal live-input checks remain mandatory. No measurement result is
cached or accepted from the inventory.

`recover measure <addresses-or-symbols> --image <original> [--in <source-file>]`
measures current spellings without publication. Subjects sharing a TU reuse the
same configured compiler trials. `--jobs` controls TU concurrency, bounded by the
project's provisioned compiler capacity. Exit 0 means all selected bodies matched
the historical masked-byte comparer, 1 a measured miss, 3 unavailable evidence.
No result establishes structural or runtime acceptance. Existing claims survive
regressions; unfinished placeholders cannot pass. No historical-claim gate must
already be satisfied just to investigate a broken intermediate structure.

Use `recover tasks variants` for candidate decks in private overlays, and only
`recover record` to bank newly measured claims. Diagnostics do not configure a
build or guess donor recipes; run configured preparation at a coordinator
checkpoint first. All reports are recomputed or disposable caches, not records.

Additional methods can share an already mapped original function without
duplicating its canonical annotation. Put `// RECOVERY_SHARED: v1 <start> <end>
<emitted-symbol> <evidence-address>` beside each extra definition, with ordinary
comments explaining the original call/table evidence. Bounds must agree with the
existing complete contiguous original span. The evidence address anchors the
interpretation to a recovery task; it is not a measurement or an inferred class.
Task checks, private decks and bounded-data checks compile referenced shared
owners and prove each emitted method separately before resolving its targets.
Task history retains changed/deleted interpretations for review. Missing bodies,
explicit placeholders and unsupported reference cycles remain unknown. Original
claim identities stay unique; shared-body results are not new banked claims or
proof of names, prototypes, complete hierarchy, or final linker folding.

## Instruction references

`recover check-references --summary` checks literal source-file references and
instructions naming Python/shell scripts under configured roots. It requires no
compiler or original image. Source roots/extensions and seed exclusions come
from the normal source policy; optional project policy supplies the rest:

```json
"references": {
  "documentation_roots": ["docs", "recovery"],
  "command_roots": ["scripts", "recovery"],
  "exclude_roots": []
}
```

Root Markdown files are scanned too. The native gate always runs this check when
the policy is present, even without an entry in its additional-command list.
Missing inputs and aliases are refused; changed scan membership, text, policy or
reference-target existence invalidate the result and whole-gate fingerprint.
Historical mentions stay separate in the report. Historical classification uses
bounded English prose heuristics, not semantic understanding; this does not check
arbitrary links, prove command execution, or verify the truth of recovery notes.

## Debugging a linked candidate

`recover debug-sidecar` creates an ELF32/DWARF symbol file from explicit i386 PE,
MSVC link-map and COFF object inputs; it needs no recovery project configuration.
Supply `--exe`, `--map`, `--out <artifact.sym>`, `--build-root` and `--comp-dir`.
The map should be generated by the same link using `/MAP /MAPINFO:LINES`.
Source path translation is explicit (`--path-map OLD=NEW`); no host drive mapping
is inferred. `--summary` bounds diagnostic output.

Names, source lines and supported CV5 types are debugger hints, not original-image
bindings, verified hierarchy or recovery acceptance. Missing objects, unsupported
types, virtual-base locations and other gaps are reported. Locals are omitted
unless `--frame-base ebp` explicitly guarantees EBP-relative frames for that build.
Public extents are bounded by the next distinct public and image section, not
proven function boundaries. The caller supplies corresponding artifacts; this
does not authenticate the map against the final link.

PCH imports use recorded signatures/counts rather than index guesses. Ambiguous
object basenames, invalid primary inputs, output aliases and concurrent input
changes refuse publication. Output is atomically replaced only after checks;
exit 0 means a debugger artifact was generated, never complete debug coverage or
recovery acceptance. Keep these generated artifacts ignored.

## Start a project

Install a built wheel into your environment, then work in your own Git repository:

```sh
python -m pip install /path/to/match_recovery-0.1.0-py3-none-any.whl
recover init --target my-application --image /path/to/original.exe
```

This creates local, editable configuration and an original-image identity in source:

```text
src/IMAGE_PIN       SHA-256 of the explicitly supplied original; no image bytes
recovery/
  driver.json       native context, obligations, restoration, build and migration commands
  project.json      source selection, compiler interaction and measurement configuration
  build.json        fresh build command and expected output artifact
  gate.json         additional checks; native checks always participate
  audits.json       project-owned source policy, initially empty
  worker.json       your worker command and attribution
  reviewer.json     your independent reviewer command and attribution
  agents.json       prompt-driven provider commands used by the native bridge
```

Initialization refuses to overwrite destination files and preserves existing C++.
Use `--source-root code` for another source directory. Relative `--image` paths
resolve under `--root`; image bytes are read only to establish identity and are
never copied. Without `--image`, supply the configured `IMAGE_PIN` yourself;
missing identity is not silently invented. No Git commits or ignore-rule changes
are made. Publication is per-file, not a globally atomic multi-file transaction;
after an I/O interruption inspect the created paths before retrying.

The generated driver invokes installed package commands, not a project dispatcher.
The interpreter running `recover` is used by default; `--python` must identify
another interpreter with this package installed. Configure these local boundaries:

- `RECOVERY_COMPILER`: executable implementing the compiler JSON protocol below,
  not a raw compiler or shell string. Alternatively replace its `argv` array.
- `RECOVERY_BUILD`: executable taking the project root and a fresh build directory;
  alternatively configure your existing generator/build command arrays. Replace
  the starter `application.exe` output with the actual output path/format.
- `RECOVERY_IMAGE`: original image path for the driver's checks, checked against
  the source pin. Inventory and standalone checks also accept `--image` directly.

Set actual flag trials, debug-type flags, compiler/build input files and environment
dependencies, compile-database location, and source audit policy. The empty starter
flags do not identify the original compiler; an empty audit policy is not structural
proof. No compiler, TU boundaries, history baseline, dependency ABI, or model was
inferred. Native measurements currently support PE/COFF i386; hashing another file
does not add another backend. Source inventory/identity pinning do not validate its
executable format. The generated driver configures native acceptance orchestration;
full structural/runtime acceptance still needs configured proof capabilities.
`inventory` can report zero annotations in a new project; that is an empty census,
not accepted coverage or permission to create an empty recovery task.

No Python inheritance, plugin registration, package fork or library modification
is required for a supported target. Compiler interaction scripts can remain local,
or existing tools can implement that boundary. The package does not supply an agent
service, compiler installation or project verification policy.

## Prepare a worker worktree

`recover worktree prepare <task> --root <coordinator-checkout> --authorization
<permission-file> --worktree <new-path> --branch <new-branch> --resources <pool>`
creates an isolated worker from a committed, source-authorized task. Permission:

```json
{
  "author": "coordinator",
  "revision": "<full-current-coordinator-commit>",
  "evidence": ["Delegate this committed ownership task to an isolated worker"],
  "prepare": true
}
```

The task must already be committed. The author must own it, its original must
agree with the source pin, and its baseline must remain an ancestor of the
explicit revision. The coordinator must be clean and at that revision. Worktree
creation preserves the task baseline, original obligations, scope and discoveries;
it does not select a new baseline or create an acceptance record.

Both the path and branch must be new. Nested paths must be ignored, outside
source roots and Git metadata. Native worker and integration preparation share
the same raw-checkout implementation: no checkout hooks, active filters, external
merge drivers, submodules, escaping symlinks or borrowed caches. Committed internal
file symlinks and executable permissions are preserved. Source files are written
independently, never hardlinked to the coordinator.

With `prepare: true`, an explicit resource pool is required. The coordinator
leases a slot before creating the worktree and runs the existing configured native
preparation there, producing fresh local recipes and generated context. Compiler
installation, executable/test assets and provider credentials remain explicitly
provisioned project inputs; no ignored directory is copied from the coordinator.
Use `prepare: false` and omit `--resources` for checkout-only setup. Neither mode
claims build success or recovery acceptance. Output has `ready_for_collection:
false`; `compiler_prepared` means recipe preparation passed, not that the
application was verified.

Failures preserve any created worktree/branch for inspection. A failed generator
does not publish recipes or roll back source damage. Interruptions quarantine the
compiler slot. After resolving the cause, `recover prepare --root <worker>
--resources <pool>` can regenerate recipes in the existing checkout without
recreating it or resetting its task. This standalone command holds the worker's
driver lease and refuses concurrent recovery. An active or quarantined slot
still requires the normal resource handling; no automatic marker deletion occurs.

Exit 0 means checkout creation and any requested preparation completed; other
outcomes return 3. This is a native setup step for an agent coordinator, not a
persistent campaign, automatic integration or permission to reap other worktrees.

## Dispatch a parallel recovery batch

`recover dispatch --root <coordinator-checkout> --plan <batch-plan>`
supervises native `recover run` children in **already prepared** worktrees. All
CLI definitions and supervision live in the installed package; project files
select compiler resources and worker/reviewer commands (including model choices).
The current supervisor requires POSIX process groups.

The plan is an explicit scheduling authorization, not stored recovery status:

```json
{
  "schema_version": 1,
  "author": "coordinator",
  "evidence": ["Delegate these disjoint ownership tasks for one bounded batch"],
  "max_parallel": 2,
  "resources": "recovery/resources.json",
  "timeout": 3600,
  "jobs": [
    {
      "task": "widget-owner",
      "root": "build/recovery/worktrees/widget-owner",
      "config": "recovery/driver.json",
      "worker": "recovery/worker.json",
      "reviewer": "recovery/reviewer.json",
      "iterations": 3
    }
  ]
}
```

Add independent jobs to the array; `max_parallel` cannot exceed the provisioned
slot count. It does not raise each compiler invocation's internal parallelism.
`timeout` bounds each entire child run, including review/checkpoint work, in
seconds (1–86400); iterations are 1–100. Plans contain 1–256 jobs and parallelism
is bounded to 1–32. Busy/quarantined slots refuse through the normal resource
allocator; no PID-based salvage, retries or resource provisioning is implicit.

Each job needs a distinct task and local branch/worktree in the same repository.
Nested worker worktrees must be ignored and outside source roots. Task identity
must agree with the coordinator checkout, and the plan author must own the tasks.
Effective scope must also agree unless an explicit retained-scope admission is
supplied as described below. Entity identities and editable paths cannot overlap between
jobs. This batch route permits existing application source only, not seed/vendor
work or changes to recovery/build policy. Workers and reviewers have independent
identities. Optional per-job `migration` and `checkpoint` paths retain the native
driver's explicit authorization requirements. All job configuration paths are
relative to that worker; resource configuration is relative to the coordinator.

An optional inline `investigation` object permits one resumed local attempt;
that job must set `iterations: 1`. The coordinator obtains it through
`match_recovery.investigation_round.authorize(root, task, coordinator_policy)`;
`campaign --resume` performs this planning automatically. This API calls the
read-only coordinator for every pending finding; it never resolves or expands one.
Only a complete `status: authorized` result supplies an `authorization` object.
Deferral, insufficient decision budget or unavailable coordination supplies no grant.
Proposed scope/binding/resolution changes refuse this phase and require fresh
serial coordination. Grants bind the task, author, canonical checkout, source,
Git state and task history. They retain the delegation digest and each decision's
evidence. These are trusted coordinator permissions, not signed certificates or
verifier reports. Do not manufacture them from worker receipts.

Dispatch validates a grant before starting children. The child revalidates and
consumes it under its driver lease before any checks or providers; reuse refuses
even after a no-op or interrupted attempt. Consumption markers are operational
state under ignored `build/recovery/investigation-uses/`, not an acceptance ledger.
Obtain new coordination instead of clearing markers or replaying old grants.
Standalone `run --investigation <permission.json> --iterations 1` uses the same
boundary; it cannot be combined with a coordinator, integration or verification-only
mode. Every finding stays pending and blocks review/acceptance. No second worker
round is implicitly authorized after edits.

The coordinator checkout stays leased and unchanged throughout the batch. Child
drivers own their own worktree and compiler leases. There is deliberately no
per-worker `coordinator` option: an out-of-scope discovery is retained by the native
driver and returned for central replanning, not independently expanded into a
sibling's scope. Disjoint edits can still interact after integration; this is why
fresh serial destination verification remains mandatory.

An optional inline job `retained_scope` admission permits an already-expanded
retained task to run without copying its journal or scope into the destination.
`campaign --resume` derives these admissions from its original branch/revision
assignments under coordinator, worker, peer and external-reservation leases.
The library API is `match_recovery.retained_scope.authorize(root, child, task,
revision=..., branch=..., peers=..., reservations=...)`; its trusted coordinator
caller must supply all active/inactive managed peer assignments and external
reservations, not just the jobs chosen for dispatch. This API grants no expansion.

Admission requires unchanged task identity/baseline, original assignment ancestry,
an exact event-and-byte prefix of destination task history, and scoped raw committed
and uncommitted work (including ignored configured source). It checks conflicts
against every destination task, effective peer scope and external reservation.
The permission pins its own source/Git/task/runtime epoch, assigned branch and
destination state. Peer task identities/scopes/branches are pinned, but their
source edits and same-scope discovery/review appends may progress concurrently.
Neither a peer's matching score nor its process report supplies admission.

Dispatch validates before launch; `run --retained-scope <permission.json>` consumes
the permission under its own driver lease before any hooks and rechecks ownership
around hooks, worker attempts and checkpoint publication. It cannot delegate scope,
integrate, or run verification-only collection. Before the first worker attempt,
the admitted state is rechecked, including ignored configured source/history. Only
an exactly accounted native review append can advance this internal check (for
example, a reviewer requesting repair); the original permission remains unchanged.
Subsequent scoped edits remain subject
to ordinary source, preservation, review and acceptance checks. One-use receipts
stay in ignored `build/recovery/retained-scope-uses/`; never clear or replay them.
Pending findings still need independent investigation permissions. Scope changes
remain serial and every returned scope is checked again after the parallel batch.

Binary stdout/stderr logs and the frozen resource declaration stay in unique,
ignored `build/recovery/dispatch/` directories. The returned report lists process
outcomes and scope integrity; it never consumes child stdout as acceptance proof
and always has `ready_for_collection: false`. Exit 0 means every child returned
0 or 2 with intact scopes, no timeout and unchanged coordinator inputs; migration
admission is not restoration or integration acceptance. Other outcomes return 3.
Inspect the actual worktrees and use fresh native integration checks, not this
report, before collection. Worker source and discoveries are never reverted.

Timeout/interruption requests cooperative descendant cleanup and preserves logs;
interrupted compiler slots remain quarantined. Commands are trusted, not sandboxed:
detached descendants need external containment and inspection before resources
can be reused. Supervisor-death resumption, task/worktree creation, automatic
integration and a persistent whole-project campaign loop are not supplied by
`dispatch`. Its synthetic process controls are not an autonomous game-recovery
demonstration.

## Coordinate a bounded recovery campaign

`recover campaign --root <destination> --config recovery/campaign.json` connects
native planning, task-only authorization commits, isolated worker preparation,
parallel dispatch, optional central discovery handling, and fresh serial integration.
It repeats planning against the updated source until its batch budget is exhausted,
the planner defers, or preserved work needs intervention. All public commands and
coordination live in the installed package; providers and compiler interaction
remain project-local commands.

The standing authorization has this exact shape (the two optional profile paths
are `coordinator` and `migration`):

```json
{
  "schema_version": 1,
  "author": "coordinator",
  "destination_branch": "refs/heads/master",
  "planner": "recovery/planner.json",
  "driver": "recovery/driver.json",
  "worker": "recovery/worker.json",
  "reviewer": "recovery/reviewer.json",
  "coordinator": "recovery/coordinator.json",
  "checkpoint": "recovery/checkpoint.json",
  "resources": "recovery/resources.json",
  "max_batches": 10,
  "max_parallel": 2,
  "iterations": 3,
  "timeout": 3600,
  "publication": {
    "enabled": true,
    "allow_migration": false,
    "message": "Integrate freshly verified recovery",
    "cleanup": true
  },
  "evidence": ["Authorize bounded application recovery and verified local integration"]
}
```

Referenced profiles must be committed canonical project-relative files. They use
the existing planner, driver, role, checkpoint, resource and optional discovery/
migration protocols. The driver must explicitly use `acceptance_scope: "change"`.
Coordinator, worker and reviewer identities must differ. Publication is separately
authorized from worker checkpointing; temporary regressions need both the existing
migration policy and the explicit publication permission. No standing game campaign
is enabled by installing the package or by `init`.

`max_batches` and `iterations` are integers in 1..100, `max_parallel` in 1..32 and
no greater than provisioned slots, and `timeout` in 1..86400 seconds per parallel
driver job. Planning and optional serial coordination have their own configured
command/iteration budgets; `timeout` is not a whole-campaign deadline. The destination
must be clean, on the authorized branch, without a staged index or unfinished Git
operation. The campaign retains its coordinator lease across nested native operations.

Each batch selects nonoverlapping existing application source/entity scopes within
the planner's limits. All new task decisions are committed before workers branch
from that batch's coordinator revision. Seeds, dependency source and verifier policy
are not mechanical parallel-worker editing targets. Workers cannot expand scope
during parallel dispatch. Afterwards the optional central coordinator can handle
discoveries within its separately supplied delegation, serially; it cannot invent
unmapped identities or grant itself application-wide editing permission.

Neither a process exit nor a saved report authorizes integration. The campaign
inspects actual worker source/Git state, prepares a merge candidate against the
current destination, then invokes native fresh candidate and destination verification.
Only verified publication permits cleanup of its own clean, integrated worktrees
and merged branches. Cleanup uses ordinary Git removal/deletion, never force/reset.
Timeouts, scope changes, dirty workers, conflicts and failed destination checks
preserve source and branches. A failure after the destination moves stops immediately
for fresh verification; there is no rollback or acceptance-receipt replay.

A clean worker with no checkpoint is deferred for that source snapshot, without
closing its task. Its unchanged branch may be removed under `cleanup`. In-memory
attempt exclusions expire when source changes, allowing earlier work to be revisited
after a later structural change. Task reservations and original baselines remain.
JSON progress goes to stderr; the final report goes to stdout. Exit 0 means a clean
bounded scheduling stop, **not game acceptance**. Unresolved work or an unavailable
planner returns 3. Reports always say `full_project_accepted: false`; they are not
verification inputs. Operational choices and derived logs stay under ignored
`build/recovery/campaigns/` and `build/recovery/dispatch/`, not a completion database.

Resume a preserved campaign with the same configuration and
`recover campaign --config recovery/campaign.json --resume <run-directory>`.
Starting another campaign refuses preserved worker/candidate work and points to
resumption rather than duplicating it. Assignments and pre-transfer integration
contexts are recorded **before** their actions in the ignored run directory.
These are coordination choices, not a source catalogue or acceptance ledger.
The current branch, initial ancestry, frozen profiles and canonical paths must
agree. Resumption uses actual source, task history, branches and leases; old
stdout, PID files and acceptance reports cannot advance it.

The last scheduled batch reuses parallel dispatch for eligible disjoint workers.
Pending findings receive fresh central one-use local-investigation permissions
before dispatch; no scope-changing coordinators run inside the parallel batch.
Retained expanded scopes can receive the fresh admission described above;
proposed serial decisions and pending integration stay on the serial
coordination/publication path. Deferred investigations remain preserved and are
not retried in that phase.
Original assignments and budgets are not rewritten; retired workers are not restarted.
Nonparticipating peers remain leased and checked while the selected workers run.
Existing workers retain their initial task baseline and scoped committed/uncommitted
source, with fresh compiler recipes. New path/branch creation occurs only for an
assignment that has not been materialized, and still needs the authorized starting
revision at the coordinator. Valid append-only task/review decisions survive;
source/policy/index edits outside worker authority refuse before compiler execution.
Live worker leases and interrupted compiler quarantine still block reuse. The
framework does not infer stopped descendants from an old PID or clear quarantine.

An existing untransferred candidate is freshly verified through native publication.
After a transfer, native publication resumption rechecks the destination against
the **original pre-transfer preservation base**, including a fresh independent
collection-aware review. Valid uncommitted review decisions are allowed; arbitrary
source, policy and staged-index edits are not. No rollback or saved verdict is used.
Retired attempts are only routing choices: their revisions must remain in actual
destination ancestry, and any remaining worker/candidate is checked for additional
work even when cleanup is disabled. Clean merged branch-only leftovers can be
removed after interrupted cleanup. This grants no new acceptance from a retirement
record; `retained_in_history` explicitly reports `verification: "not_replayed"`.
Ignored source or task/history files are recovery work, not disposable caches:
they prevent no-op retirement and cleanup, and are rechecked on resumption.

Each explicit invocation/resumption has the configured batch/command budgets.
Except for the explicit coordinator transition below, changed profiles refuse.
Moved pending integration targets, partial Git/index publication,
older-batch additional work and unknown/aliased checkouts require inspection and
reauthorization through native commands; no force update, automatic conflict repair
or adoption of unrelated workers is supplied. Unmapped original discovery and
complete game structure/runtime correctness remain unfinished.
The real VC6 control exercises both existing-task
and new-task selection, structural recovery across a byte regression, central caller
discovery, fresh integration/destination acceptance, cleanup and another planning
round, plus interruption before setup, during structural WIP, at candidate publication,
after transfer and after destination refusal, and during cleanup. Reasoning in that
control is deterministic, not a live-model or game pilot. A separate process control
kills the actual supervisor and proves that resumption refuses its still-live worker.

### Reauthorize a preserved campaign's coordinator

When new findings need a larger existing-source scope or decision budget, commit
the revised coordinator profile, then supply an explicit decision JSON with
`campaign --config ... --resume RUN --coordinator-authorization DECISION.json`.
This authorizes the remainder of the **original bounded campaign**, not another
campaign, acceptance, or an increase in its batch/worker/iteration budgets.
Only coordinator `subjects`, `editable_files`, `max_decisions`, `evidence`, and
the optional `route_discoveries: true` permission may change. Routing permits
source-journal requests to existing managed owners, not reservation activation,
scope transfer or acceptance. Its command, author and seed-binding permission, all other profiles, and
the campaign configuration must remain unchanged. This is not a compiler, provider,
worker-code or proof-policy upgrade path.

The decision is an exact object with these fields:

| Field | Binding |
| --- | --- |
| `schema_version` | Integer `1` |
| `author` | Existing campaign coordinator |
| `run_sha256` | `match_recovery.model.digest` of the original parsed `run.json` |
| `batch_sha256` | Same digest of the latest parsed `batch-N-workers.json` |
| `previous_profiles_sha256` | Previously authorized profiles digest, initially from `run.json` |
| `previous_coordinator` | Complete previously authorized coordinator profile |
| `coordinator` | Complete new profile, identical to the currently configured file |
| `destination_revision` | Current full destination Git commit |
| `evidence` | Nonempty unique strings explaining this authorization |

Keep this invocation-specific JSON under ignored `build/recovery/`: committing a
file that names the current destination would itself move that destination. The
scope and rationale belong in the versioned coordinator profile; the package
retains this run-bound permission alongside its immutable scheduling choices.
These are canonical JSON object digests, not hashes of formatted file bytes.
The native API reconstructs the prior profiles digest using the supplied old
coordinator, preventing unrelated profile changes from borrowing this permission.
Every unretired assigned worker must already exist, retain its original task/baseline and
contain only authorized source/history changes. The coordinator locks them all
while verifying ownership and publishing the permission. New shared expansion
grants cannot overlap an existing task or parked-worktree reservation. Before each
serial coordination round, grants are narrowed against current peer ownership;
one worker acquiring scope does not authorize the next to edit it too.
Peer leases and fresh scope snapshots remain held throughout serial coordination.
Validated retired/collected assignments need not be recreated; their retained
ancestry and any remaining checkout are still checked before reauthorization.

The package appends `coordinator-authorization-N.json` in the ignored run directory;
it never rewrites the original manifest, assignments, task baselines, integration
contexts or worker files. Permission publication can survive a later preparation
or execution failure; it is not evidence that a worker ran or was accepted.
Subsequent ordinary resume validates the complete chain. An exact retry of its last
decision is idempotent; stale predecessors, changed assignments and unexplained
profile changes refuse. All source changes still require fresh checks and review;
changed coordinator delegation is included in the review identity. Existing
integration targets are never retargeted by this decision.

## Lease project-provisioned compiler resources

`run`, `start`, `prepare`, `worktree prepare` and `integrate publish` accept `--resources <pool.json>` (relative
to `--root`) and optional `--resource-timeout <seconds>`. Default wait is zero;
the maximum is 3600 seconds. Other commands inherit an existing allocation or use
their manually configured environment. The package does not create compiler
installations, clone resources, infer their capacity or increase `--jobs`.

Example coordinator-owned configuration, with no project Python adapter:

```json
{
  "schema_version": 1,
  "slots": [
    {
      "name": "compiler-0",
      "directory": "{env:COMPILER_POOL}/compiler-0",
      "environment": {"COMPILER_INSTANCE": "{resource}"}
    },
    {
      "name": "compiler-1",
      "directory": "{env:COMPILER_POOL}/compiler-1",
      "environment": {"COMPILER_INSTANCE": "{resource}"}
    }
  ]
}
```

Directories must already exist and be canonical, absolute and disjoint. All
worktrees using the same actual compiler resource must name the same directory.
The coordinator must ensure different slots really select isolated compiler
resources: the package cannot infer the meaning of an environment variable.
All slots set the same variable names, declared in `compiler.environment` in
each checkout's `recovery/project.json` (`PATH` is implicitly measured). For
integration this includes candidate and destination. Selected values are inherited
by preparation, agents, nested checks and builds, and fingerprinted by the native
compiler interface. The slot remains leased through review and publication, even
while no compiler is running. Children must not request another allocation.

Kernel locks serialize cooperating local runs. Busy slots are skipped; all-busy
or quarantined pools refuse with exit 3 after the bounded wait, without dispatching
agents. Invalid or missing resources refuse before dispatch. Locks do not protect
against manual commands bypassing allocation, edits to leased resources, or
programs deliberately detaching unfinished compiler work. Network-filesystem lock
behavior is not validated. Leases do not isolate source/build directories.

The slot contains `.match-recovery-resource.lock` and, while owned,
`.match-recovery-resource-active.json`. These are ignored operational state,
never recovery evidence. Normal completion removes the active marker and restores
the parent environment; abrupt termination leaves the slot quarantined even after
the kernel unlocks it. A dead PID does not prove descendants have stopped. Before
removing an interrupted active marker, the coordinator must stop/verify all work
using that resource and ensure no allocator is running. Never delete or replace
the lock file to unlock a resource. Provision these directories outside versioned
source or beneath an ignored directory. No cleanup of installations is implicit.

## Survey an executable before source recovery

With the `x86` extra installed, a new project can inspect the pinned original even
before it has function annotations:

```sh
recover survey --image /path/to/original.exe --max-instructions 50000 --summary
recover survey --image /path/to/original.exe --start 0x401000
```

The native PE32/i386 survey follows bounded static control flow from the PE entry
point and optional starting hints supplied from manual analysis or other tools.
It reports executable sections, ordinary imports, instruction/edge observations,
entry/direct-call candidates, source/seed correlations, declared dependency
bindings, unresolved paths and pending addresses when its budget is exhausted.
Removing `--summary` retains individual instructions and edges. Neither mode
writes source, task decisions or a results ledger; redirected proprietary-derived
reports belong in ignored storage.

These candidates are starting hints, not established functions: call fallthrough
is explored without proving that a callee returns, and data can decode as code.
Indirect jumps stop traversal; ordinary IAT operands can identify dependency slots
but are not followed as local functions. Overlapping instruction interpretations,
unbacked targets and undecodable instructions remain diagnostics. Additional export,
TLS, exception and delay-import paths remain explicitly incomplete. A static
reachability budget is not executable coverage, even when its queue empties.

Jumps do not establish tail calls, class ownership or historical TU boundaries.
Original extents remain unknown; existing ownership names remain hints. Source-owned
upstream component declarations route candidates to `verify_dependency`, without
proving the dependency's version/ABI. Unmapped candidates still need independent
boundary/ownership evidence and source mappings before recovery tasks can use them.
Exit 0 means the survey ran, not that any recovery or executable verification passed.

### Establish a source binding, then authorize recovery

Once analysis establishes a name/extent interpretation, the coordinator can bind
an existing C++ candidate through the package writer:

```sh
recover bind 0x401000 --image /path/to/original.exe --in src/owner.cpp --line 12 \
  --source-sha256 <current-file-sha256> --name '?method@Owner@@QAEHXZ' \
  --span 0x401000-0x401020 --reason 'Explain the original entry and extent evidence' \
  --evidence 'source:src/owner.cpp' --evidence 'manual-analysis:original-function-boundary'
```

Replace these illustrative values with independent evidence; survey reachability
or the candidate's compiled length does not establish an original extent. Multiple
`--span` arguments describe separately established pieces, with the entry span first.
The command checks the image pin and executable file backing, not correctness of
the proposed name, function boundaries, definition ownership or implementation.

`bind` adds an unclaimed `ORIGINAL` marker before the selected existing code line
and an immediately adjacent `RECOVERY_BINDING: v1` JSON comment containing only
`reason` and `evidence`. This is non-derivable binding rationale, not a run log,
saved score, file-hash ledger or second source catalogue. Native source inventory
and task context expose that evidence, and source integrity refuses malformed or
orphaned binding comments. Existing C++ bytes and neighboring annotations remain.
The supplied source digest is a publication guard and is not saved in source.

The default `--mode body` binds the following body and continues to refuse directive
anchors. Opt in to `--mode file` for a TU-level function binding, including a
compiler-generated deleting destructor emitted from a header declaration. FILE
uses the existing whole-TU source grammar: supply the symbol with `--name` and
anchor it at an existing code line or `#include` directive in the non-seed source
TU, not in the header. Other directives, comments and blank anchors are refused.
No wrapper body or C++ code is generated. The binding does not prove that the TU
emits the symbol, owns it uniquely, or passes any measurement or acceptance check;
those remain separate fresh verification obligations.

Existing product/component identities, conflicting seed spans, overlapping
identities and linked/stale destinations are refused. Compatible seed attempts
remain as evidence; they are not erased or demoted. Rehoming, shared tails and
additional header/body ownership interpretations require their separate reviewed
workflows, not this new-identity binding operation. Publication uses the existing ignored backup/staging
mechanism and does not commit or authorize an expanded task. Use a coordinator-owned
worktree; concurrent uncooperative edits can still race filesystem replacement.

After reviewing the binding, checkpoint source and use `tasks create --group ...`
or the explicit scope-expansion workflow. Recheck the affected TU. Only `record`
can bank freshly measured historical byte claims; successful binding is not a
measurement or acceptance result.

## Whole-project gate

`recover check --root <project> --image <original> --jobs 1 --summary` runs the
package-owned project gate. Native source integrity, build mappings/body links,
historical claim identities, every configured source audit, a fresh configured
build, current historical matches, emitted symbols, initial data and declared
layouts all participate. There is no project Python dispatcher or switch for
omitting a native check. Compiled stages run serially after a successful fresh
build; only the match sweep uses the explicitly requested compiler concurrency.

Set `gate` in `recovery/project.json` to a local configuration path. A project
with no additional command checks can use:

```json
{
  "schema_version": 1,
  "checks": {},
  "inputs": [],
  "input_roots": [],
  "environment": []
}
```

Additional project tests are named entries in `checks`, each containing `argv`
(an argument array) and `timeout` (1–3600 seconds). Commands run from the project
root and support `{root}`, `{image}` and `{env:NAME}`. Their exits mean 0 pass,
1 failure, 3 unknown; interruption, timeout and other exits remain unknown.
Commands are trusted project tools, not a sandbox or inferred structural proof.
Declare additional inputs, input roots and environment dependencies. The gate
also checks configured build/measurement inputs and Git-visible working-tree
contents before and after each stage. Concurrent edits stop verification and
are preserved. JSON goes to stdout, stage progress to stderr; no verdict ledger
is written. The summary retains all stage results and failure identities.

`recover check-matches --image <original> --jobs 1 --summary` exposes the native
whole-source matching stage separately. It remeasures every current claim,
including seed attempts and unsupported hosts, and scores unclaimed product
candidates in the same TU compilation. It never banks a discovery. Different
attempts at one address remain separate measurements; compiler objects are reused
only within that invocation's TU/flag trials. Recipes are supplied by the
configured compiler, not inferred by the package. A build-mapped claimed subject
that stops compiling or being emitted fails; unresolved seed evidence remains
unknown. Header claims are visible but require unsupported host coverage. An
empty claim set does not pass. Historical identity coverage is a separate stage.

Gate exit 0 means its configured checks passed, 1 means a known failure, and 3
means incomplete evidence. A known failure takes precedence over unknowns.
This is not yet a full acceptance/migration verifier: historical matching still
uses relocation masking, and these checks do not establish complete hierarchy,
object lifetime, final-link reference selection, dependency ABI or runtime
behavior. `ready_for_collection` therefore remains false, even on exit 0.

## Configure recovery agents

Configure these commands before starting real recovery. The generated worker and
reviewer configs call the package-owned `agent` bridge. Set the prompt-driven
commands in `agents.json`; its starter commands use `RECOVERY_WORKER` and
`RECOVERY_REVIEWER` executable paths (not shell strings), consume a prompt on stdin
and return final response JSON on stdout. You can instead keep a direct JSON
protocol adapter in `worker.json`/`reviewer.json`. The interpreter selected by
`--python` must have this package installed to run the bridge. Add
`build/` to your project's ignore rules for disposable caches/locks. Original
proprietary images and derived artifacts also need project-owned ignore rules.
The initializer does not alter your existing ignore rules or run Git mutations.

Source code remains authoritative for original addresses, symbols, ownership and
structural interpretations. A task JSON is an authorization decision, not a second
source map or a measurement ledger. Create it from your source-owned identities:

```json
{
  "schema_version": 1,
  "task_id": "window-ownership",
  "target": "my-application",
  "author": "coordinator",
  "intent": "Recover the window ownership group",
  "base_revision": "<full committed Git revision>",
  "subjects": [{"image_sha256": "<original image SHA-256>", "address_space": "pe:va", "address": 4198400}],
  "editable_files": ["code/window.cpp", "code/window.h"],
  "evidence": ["source-owned constructor and member evidence"]
}
```

Replace the placeholders with actual evidence. Address spaces and target names
are project chosen; `pe:va`, file offsets, other formats and other source homes do
not require different driver code.

```sh
recover tasks create --file /path/to/task-decision.json
recover tasks status window-ownership
# Checkpoint the local configuration and task decision in Git first.
recover run window-ownership --reviewer recovery/reviewer.json
```

`--root` selects a project from anywhere. Relative run configuration paths resolve
under that root. `run` defaults to `recovery/driver.json` and `recovery/worker.json`;
`--config` and `--worker` override them. The driver does not commit, collect or
revert source changes. An independent review and a configured full acceptance gate
are required before it reports ready for collection. Merely installing the package
or generating the starter files cannot establish a recovery result.

## Small integration contract

### Historical obligation identities

`recover check-claims [--root <project>] [--summary]` derives byte and
instruction-shape requirements from source history and checks that their original
identities remain represented in current source. Configure a one-time history
scope decision, with evidence, in `recovery/project.json`:

```json
"source_history": {
  "baseline_revision": "<full committed ancestor revision>",
  "evidence": ["Source checkpoint where protected recovery began"]
}
```

The baseline needs source configuration and an image pin; pre-framework history
also needs explicit `before_configuration` source inputs as described below.
Once committed, the baseline cannot be advanced or reset: the checker inspects
reachable configuration history, including merged branches, before selecting its
source-history interval. The established pre-configuration interpretation cannot
be changed either. Shallow clones must fetch complete history before this check
can establish coverage. Preserve source bindings and obligations, not counts.
Renaming or rehoming a candidate leaves its original identity intact. Dropping a
byte requirement, replacing it with a weaker shape requirement, deleting the last
marker, or swapping claims while keeping the count unchanged fails. Deleting a
redundant seed copy does not require retaining an artificial count.

Exit 0 establishes source coverage only; 1 reports lost obligations; unavailable
history/configuration returns 3. Current and newly declared claims still require
fresh binary verification. No ledger, current verdict or source mutation is
written. This does not authenticate which editor added a token or prove current
matching, original boundaries, ownership or structure.

### Source annotation integrity

`recover check-source --root <project> [--jobs N] [--summary]` checks all files
selected by `source` configuration, including headers and seeds. It needs no
compiler, image, Git database or optional dependency. All rewrite experiments
are in memory: a no-op must preserve exact text; a keyword flip may touch only
its own marker; hypothetical demotion must preserve line count and lesson
meaning; clearing lessons may delete only their attributed lines. A separate
standalone-comment witness catches marker heads the parser silently misses.
Reserve standalone `ORIGINAL:` heads for mappings, not ordinary prose.

The command emits disposable JSON: exit 0 for intact annotations, 1 for an
observed violation, 3 for unavailable evidence. Empty coverage, invalid encoding,
unsupported source symlinks and changing inputs cannot become success. `--jobs`
only parallelizes read-only text checks, not compiler processes. This is not a
C++ lexer, ownership/duplicate-binding check, current match or acceptance gate.
No result or new claim is written into source.

### Source identities and build hosts

`recover check-initializers --image <original> [--root <project>] [--summary]`
preserves conventional trailing-address initializer probes during source migration.
It uses the pinned PE reader and needs no compiler or project Python adapter.
Configure the source dialect and probe policy in `project.json`, for example:

```json
"initializers": {
  "ruleset": "cpp-trailing-address-v1",
  "probe_bytes": 4,
  "scalar_types": ["int", "unsigned int", "int32_t", "uint32_t"],
  "unit_extensions": [".cpp"],
  "exclude_roots": ["src/vendor"]
}
```

This deliberately limited lens recognizes column-zero declarations such as
`int count; // 0x00402000`. It checks configured scalar types, pointers and simple
arrays, excluding seeds, configured dependency roots and commented-out text.
Missing/explicit-zero initialization against nonzero original storage fails;
arbitrary nonzero C++ expressions are not evaluated. The prefix size is probe
policy, not an original extent or ABI inference. Unsupported types are counted;
no supported probes, unbacked/code addresses or missing/wrongly pinned input
remain unknown. Virtual zero fill is distinct from file-backed storage and does
not establish CRT/runtime state. Reports retain failures/errors even in summary
mode. Nothing is persisted or claimed. Use `RECOVERY_OBJECT` and `check-data`
for compiled object widths, exact initialization and relocation-target evidence.

`recover check-mappings [--root <project>] [--summary]` uses the existing source
configuration and two local inputs in `project.json`:

```json
"mappings": {
  "compile_commands": "build/compile_commands.json",
  "forwarder_macros": ["ORIGINAL_CALL"]
}
```

The macro name is illustrative; supply your own literal-address forwarding
macros, or an empty list when none are used. The compilation database supplies
intended source membership; no compiler or project Python module is imported.
Relative file names resolve against each entry's directory, with relative
directories rooted at the project, not the caller's working directory.

The native checker reports full-path candidate identities, unbuilt mapping hosts,
direct seed build inputs and historical claims shadowed by forwarding macros.
Alternative seed files remain visible hints, not additional production owners.
Repeated same-file mappings and multiple product owners remain ambiguous.
A single product owner with an explicit source binding rationale can retain
seed hints with identical original spans when the product carries all of their
historical claim obligations (byte exactness also covers instruction shape).
The report identifies that `product_owner`; conflicting spans, multiple product
owners and claims carried only by a seed still require reconciliation.
Both product and retained seed claims remain subject to fresh matching checks;
this read-only ownership check neither copies nor banks a claim.
This applies to unfinished placeholders as well as implemented candidates and
does not establish behavior or matching.
No claim is erased and no seed is promoted or deleted. The command masks ordinary
comments and literals after line splicing; unresolved address expressions stay
unknown. This lexical check does not expand macros or resolve conditional branches.
Build membership is not compiled/link ownership, included-source coverage or
dependency verification. Exit codes are 0/1/3 for pass/fail/unknown; reports are
disposable. Missing databases, empty coverage and changing inputs never pass.

### Fresh function-symbol coverage

`recover check-symbols [--root <project>] [--in code/owner.cc] [--summary]`
checks every nonexcluded product function mapping through the configured compiler
protocol and measurement flag trials. It also uses `mappings.compile_commands`
to identify intended build hosts. No game adapter, `nm` invocation or preexisting
object is used. Compiler/format failures, absent/undefined/weak symbols and unbuilt
hosts stay unknown; non-function storage or an invalid code start fails. Symbols
must be defined in their mapped TU, not borrowed from a neighbor. One passing
trial establishes presence only; it does not establish exclusive COMDAT ownership
or final linker selection. No ordinal/name alias is silently selected.

The source reader's emitted-symbol fact wins over the catalogue name. Without one,
the catalogue spelling is tried literally; source names are not automatically
mangled or renamed. Definitions are checked together per TU, stopping trials once
every selected symbol in that TU has been observed. `--in` uses exact source paths
and restricts the diagnostic, never whole-project acceptance. Exit 0 means selected
emission coverage; 1 a counterexample; 3 incomplete evidence. There is no allowance
for a population of missing names, stored verdict or source mutation. This does
not prove original bytes, references, class structure or runtime behavior.

### Project-wide declared layout coverage

`recover check-layout [--root <project>] [--unit code/owner.cc] [--summary]`
checks explicit source-owned member and nonvirtual-base expectations using fresh
standalone compiler debug information. Existing source, measurement and compilation
database configuration supplies the inputs; no task or project checker is needed.
It compiles every configured product TU for every flag set once, sharing each
object across expectations. All observed definitions must agree; a later passing
configuration cannot erase a counterexample. Task and project layout checks also
index each fresh object's standalone type stream once, without sharing objects
or verdicts across compilations. Unsupported class field lists remain separate
lookup failures; a malformed type stream never supplies a partial index. A known
wrong offset remains a failure even if its member type is unmeasurable.
The shared type reader rejects PCH/type-server dependencies for both class and
data observations, even when a requested class also has a local definition.

Each expectation needs a complete compiler witness in every configured flag set.
Unsupported/malformed debug information, failed compiles and missing witnesses
remain unknown. Non-emission from a valid type stream is visible in coverage,
not proof the TU has no ABI dependency. `--unit` narrows investigation but omitted
product TUs keep the overall result unknown. Unsupported/external build inputs
likewise cannot silently disappear. Exit codes are 0/1/3 for pass/fail/unknown.
Summary output retains counterexamples; all reports are disposable.

`--conventional-offsets` additionally reads trailing `// 0xNN` / `// (+N)`
comments and `field_<hex>_` names on direct member declarations in configured
interface files. It compares them with the same compiler objects, without a type
size table, offset walk, new annotations or project adapter. These are conventional
source hints, not evidence-backed original-layout interpretations. Conflicting
names and comments remain separate checks. Seeds and source-declared upstream
component roots are excluded (and component exclusions are reported); that does
not establish dependency compatibility. Unsupported hinted declarations/lexing
remain diagnostics and keep coverage unknown. The lens is not a complete C++
parser or conditional-preprocessing analysis. Passing conventional trials are
aggregated per hint; every nonpassing observation retains its TU and flags.
This diagnostic is not a whole-project gate: investigate its coverage and compiler
counterexamples alongside the other configured checks.

`--witness-unobserved` adds a bounded second stage after the ordinary product
trials. For each class/flag set without any complete definition, it selects one
existing TU using the shortest literal-include path to the source declaration,
then smaller TU text and pathname as tie-breakers. The normal baseline object is
retained only within this measurement invocation. Its unchanged-replica and
changed-body overlay controls must pass before the class supplies a witness.
Co-located classes share the probe and changed-body compilation, with independent
symbol-change and layout-agreement checks for every class. Failed batches are
split into freshly compiled groups; partial successes never escape a failed batch.
Positive batches need three overlay compilations including the unchanged replica;
unsupported or absent declarations can require extra split/absence controls.
Includes select a candidate, not proof of visibility; absent declarations, failed
controls, macro includes without a literal host and unsupported types remain
unknown. Alternative candidate hosts are reported, not exhaustively compiled.
This never replaces an already observed definition or hides an ordinary TU's
counterexample. It is distinct from `--witness-missing`, which probes missing
definitions in every selected TU. Neither proves ABI independence or lifetime.

A project can enable these source dialect/coverage options for both the installed
gate and standalone checks without adding a checker script:

```json
"layout": {"conventional_offsets": true, "witness_unobserved": true}
```

Missing options default to false. Explicit `--witness-missing` uses per-TU
witnessing instead of the configured unobserved-only mode. Both modes cannot be
explicitly requested together. Reports retain host selection and every unresolved
supplement; no coverage decision or compiler result is written into source.

The standalone command and whole-project gate emit layout trial start/finish
progress to stderr, naming the TU, trial index/total and elapsed time. The Python
`check_layout` API accepts an optional `progress` callback with those events and
the trial's flags. Progress is transient observation, not a passing layout result;
it does not change the final JSON, coverage or input-freshness checks.

For a missing definition, `recover check-layout --unit code/owner.cc
--witness-missing` or `recover tasks check-layout <task> --witness-missing`
can request controlled same-TU witnesses. This is opt-in because it adds compiles:
one unchanged overlay per affected TU/trial, then a pointer-parameter probe and
a changed-body control per missing class. The library uses the configured
compiler's existing overlay interface; no project-specific checker is required.
Pointer probes append a uniquely named C++ function in a private copy, without
includes, class definitions, instantiation or product-source edits. A visible declaration
can then appear in standalone debug information even when ordinary compilation
omits it. `sizeof` alone is not a reliable emission trigger.

Coverage labels these definitions `witnessed`; member/base rows retain
`layout_basis: same_tu_probe`, distinct from `product_object`. Every definition
still has to satisfy the original source expectation, and observed counterexamples
or malformed definitions are never replaced by probes. An unchanged replica must
agree with product compilation; each probe's deliberately different body must be
observed without changing its class layout. Failed controls, macro-substituted
names and unsupported overlays remain unknown. This
observes the declaration visible at the end of that TU, not product-code
preservation, ABI independence, lifetime or full structure. It cannot waive
omitted TUs or import a layout from a different compilation.

When a pointer probe cannot obtain a definition, independent counterfactual
controls can establish `no_complete_definition` coverage. They append two distinct
diagnostic class definitions at the requested namespace scope in separate private
copies. Both exact sentinel layouts and differing `sizeof` probe bodies must be
observed. An existing complete definition prevents redefinition; this is tested
against real compiler controls, including aliases, using directives and anonymous
namespaces. Nested-class paths that cannot be expressed as namespace scopes remain
unknown. Macro guards and the unchanged-replica control still apply. Neither a
compile error nor missing debug information is evidence of absence by itself.
An absence-eligible failed pointer group tries both positive definition controls
before splitting into individual probes. Only a complete passing group supplies
absence coverage; mixed, unsupported or failed-control groups split and receive
fresh independent checks. Observed layouts and failed pointer-body controls never
become absence proofs. No prior compilation result or partial group proof is reused.
See Microsoft's [type-redefinition diagnostic](https://learn.microsoft.com/en-us/cpp/error-messages/compiler-errors-1/compiler-error-c2011)
for the language restriction used by these counterfactual controls.

Diagnostic class sizes/members are never supplied as recovered layout facts or
scored against the original executable. Absence means no complete declaration at
that scope in this TU; it does **not** prove ABI independence or rule out raw/untyped
consumers. Every requirement must still have at least one actual layout witness
in every configured flag set. An absent-everywhere class stays unknown.
Member/base reports omit nonexistent measurements and retain absence in coverage;
restoration trials label it `coverage_only` with `layout_basis:
same_tu_definition_absence`. Such a trial closes bounded declaration coverage,
not a member offset or whole-class proof. Contradictory, incomplete, duplicate or
wrong-TU/trial absence records are refused. Historical interpretations still need
their independent review dispositions.

Absence controls are batched within one TU/flag trial: two compiles can serve
several missing classes, while every class still needs its own exact sentinel
layouts and changed probe body. Each compiled type stream is indexed once for
the group. If a group fails compilation, parsing or any class's controls, it is
split into smaller independently compiled groups. No partial batch result or
one class's evidence can satisfy another class. A difficult/unsupported sibling
stays unknown without blocking valid independent witnesses. Batching introduces
neither persistent results nor additional project configuration.

The joint task check requests these witnesses automatically for missing declared
requirements, so restoration, migration checking and the recovery driver consume
the evidence in the same fresh invocation. Already emitted classes incur no probe
work. The shared compiler reuses ordinary product objects between binary and
layout checks, but never uses that cache to satisfy a private-replica control or
puts probe output into the product-object cache. `compilation.overlay_attempts`
reports the additional compiler work; no verdicts or probe results persist.
Structural-requirement trials retain `layout_basis` so a reviewer can distinguish
product observations from controlled same-TU declaration witnesses. Missing
overlay support remains unknown without discarding independent binary evidence.
The standalone layout commands remain opt-in for targeted diagnostics; a whole
product layout census does not automatically probe every class in every TU.

Input validation still re-enumerates configured roots, resolves every input path
and hashes all file contents, including checker code. Path handling avoids
repeated ancestor-object construction, and streaming hashes share one bounded
scratch buffer per snapshot. Every file gets a new hash and only bytes actually
read enter it; short reads are not end-of-file and read errors cannot yield a
successful partial digest. This does not cache timestamps, resolved aliases,
file inventories or verdicts. Paths are resolved again even within one snapshot;
same-size edits with restored timestamps and newly introduced headers remain
observable.

This checks explicit `RECOVERY_MEMBER`/`RECOVERY_BASE` interpretations, not legacy
offset prose, self-naming fields, full types, virtual-base placement, hierarchy,
lifetime or linker selection. Original expectations remain source evidence, not
freshly re-derived binary evidence. It is not acceptance or migration admission.

### Prompt-driven agent commands

`recover agent worker` and `recover agent reviewer` implement the driver protocol
on stdin/stdout. They translate the request into a prompt, invoke an ordinary
configured command, and validate its final response. No provider SDK or local
adapter code is needed. `--root` and `--config` select the project and local
configuration (default `recovery/agents.json`). A minimal provider configuration:

```json
{
  "schema_version": 1,
  "artifacts": "build/recovery/agents",
  "worker": {
    "command": {"argv": ["my-worker-command", "--schema", "{schema}"], "timeout": 3300},
    "instructions": [],
    "response": "stdout"
  },
  "reviewer": {
    "command": {"argv": ["my-readonly-reviewer", "--schema", "{schema}"], "timeout": 3300},
    "instructions": [],
    "response": "stdout"
  }
}
```

These executable names stand for your chosen commands, not bundled programs.
`instructions` contains optional project-relative UTF-8 instruction files. The
bridge passes the prompt on stdin; substitutions include `{root}`, `{task}`,
`{role}`, `{request}`, `{schema}`, `{response}` and `{env:NAME}`. Request/schema
files are fresh disposable JSON inputs, not another task ledger. `response: file`
requires a command using `{response}` to write the final JSON there; stdout is
then ignored, so progress events cannot be mistaken for a verdict. `stdout`
requires exactly one JSON object, not fenced Markdown, JSONL or a vendor envelope.
Both modes use the same authoritative response validators as the driver, even
when a provider claims to enforce the supplied JSON Schema.

A filesystem-capable worker, reviewer or coordinator may opt into `"evidence_delivery": "on_demand"`
beside its `command`, `instructions` and `response` settings. The default,
`"inline"`, embeds the full request and requires no request-file reading tools.
Planner requests remain complete inline inputs. Do not enable on-demand delivery for a provider that
cannot read local files through its agent tools.

For on-demand worker/reviewer requests larger than 128 KiB, the bridge routes diagnostic
lists/strings larger than 8 KiB under `context.impact`, `context.restoration`
and `check` to explicit JSON Pointer references in the original request file.
Coordinators use the corresponding observation roots:
`diagnostics.context.report.impact`, `diagnostics.context.report.restoration`
and `diagnostics.check.report`. Exit codes, diagnostic freshness pins and
non-acceptance labels stay visible. The prompt identifies the full request file
and its SHA-256. Task scope, instructions,
pending discoveries, investigation permissions and review/acceptance feedback
remain inline, as do coordinator delegation, action facts and routing choices.
This is not a total prompt-size cap: authorization is never
discarded to meet a budget. Routed evidence is neither empty nor passing; agents
must read relevant complete evidence before editing or deciding. The original request,
native checks, review and acceptance contracts are unchanged. These temporary
files are evidence transport, not persisted results or reusable verdicts.

The bridge verifies the task and editing scope before launch, rejects reviewer
writes, worker scope violations, Git mutations and changed policy, and cleans its
temporary directory on success or failure. It never reverts source edits. Missing,
oversized, symlinked, malformed or stale responses, nonzero provider exits and
timeouts exit 3 with structured error details. The driver reports `agent_error`
and stops; it does not retry quota failures or invent a reset time. Failed commands
retain available stdout/stderr tails (at most 64 KiB each) in private ignored
`failure-*` artifact directories, referenced by `error.diagnostics`. Those raw,
untrusted logs may contain source or sensitive provider output: inspect locally,
do not commit or treat them as evidence for acceptance. Successful command output
and temporary request/schema/response files remain disposable. Review schemas
require `supersessions` and `retractions` arrays; return empty arrays when unused.

Provider sandboxing, authentication, model choice and independent sessions remain
explicit command configuration. Scope checks are detection, not containment.
Prepare isolated worktrees/compiler resources before running a real worker.
The bridge does not authenticate reviewer independence or grant permission for
nested agents. Completed non-derivable decisions use existing task/review history;
prompts, response files and run output do not become a versioned ledger.

### Direct JSON command adapters

Each configured command is `{"argv": ["executable", "arg"], "timeout": 300}`.
Commands run with the project root as their working directory. Arguments support
`{root}`, `{task}` and `{env:VARIABLE}` substitutions, not shell evaluation.
One JSON object arrives on stdin; one JSON object must be returned on stdout.
Use stderr for diagnostics. Exit codes are 0 = pass/success, 1 = measured failure,
3 = unavailable/unknown. Timeouts and malformed responses never become success.

| Role | Input | Required response |
| --- | --- | --- |
| `context` | Task decision | Project-defined source/evidence context object |
| `check` | Task and current context | `result`: `pass`, `fail` or `unknown`; additional diagnostic fields allowed |
| Worker | Task, context, check, authorized files, review and acceptance feedback | `discoveries` array; may edit only authorized files |
| Reviewer | Task, context, check and source `snapshot` | Same `snapshot`, `decision`, nonempty `evidence`, `feedback`; read-only |
| `obligations` | Task | Source-derived `obligations` and optional `interpretations`; read-only |
| `acceptance` | Task, reviewed snapshot and `review_policy`, protected obligations, review, interpretations and fresh `verification_id` | Request-bound full-project gate report and protected measurements; read-only |

Only `context` and `check` are required in the driver configuration. Add source
`obligations` to preserve restoration requirements across rewrites. Without
`acceptance`, even a passing scoped check and approved review stop short of
collection. Review approval is evidence-backed judgment, not a verifier result.

A trusted check may return `next_action: "review_structure"` with an unknown
result to request independent review before another worker round. The native
restoration checker does so only when scoped checks, references and current
declared requirements pass, and the remaining unknown obligations are structural.
Any configured build must also pass before this early handoff. Reviewer corrections
feed the worker; approval hands this review-only unknown result to a configured
full acceptance verifier, which must independently resolve every outstanding
obligation. Without that verifier the loop reports `full_acceptance_gate_required`.
This hint grants no scope expansion and cannot turn unknown evidence into a
passing check. Arbitrary unknown results and measured scoped failures cannot
bypass recovery through this handoff.

Worker discoveries have `hypothesis`, `affects` (original entity identities), and
`evidence`; discoveries affecting other entities pause for a coordinator's scope
decision rather than disappearing at a function boundary. An empty response is
`{"discoveries": []}`. Reviewer decisions are `approve` or `request_changes`.
Source-derived historical interpretations require explicit reviewed supersession
or retraction; generic approval cannot silently erase them.

Run the same task again to resume scoped source edits and its valid append-only
discovery/review decisions without an intermediate Git commit. The driver checks
both complete decision streams and their committed prefixes: rewritten/deleted
committed events, malformed chains, symlinks and unrelated dirty files are refused.
Only the selected task's native decision files receive this allowance; it never
grants workers permission to edit history. Task/configuration and migration-policy
changes still need a coordinator checkpoint. Decision attribution retains the
trusted-coordinator API boundary; a hash chain does not authenticate its author.

A resumed invocation re-derives context/obligations and remeasures source. An
unchanged snapshot may reuse an independent review, never an acceptance receipt;
full acceptance needs a fresh invocation and verified artifacts. Each explicit
`run` supplies a new bounded invocation budget without changing the task baseline.
This is not automatic retry scheduling, recovery of malformed partial publications,
or collection/integration. No current measurement state is stored for resumption.

Acceptance must report `scope: "full_project"` and `result`, `build`,
`structure`, `references`, `obligations`, `runtime`, each one `pass`, `fail` or
`unknown`. All dimensions must pass for acceptance. Source and decision freshness
are rechecked around commands; unrelated writes and Git mutations are refused.

The report also requires `request_fingerprint`, `measurements` and `current`.
`request_fingerprint` is `match_recovery.model.digest(request)` over the complete
stdin request, including its fresh `verification_id`. For non-Python verifiers,
this is SHA-256 of UTF-8 JSON with sorted keys, unescaped Unicode, compact `,`/`:`
separators, no NaN/Infinity and one trailing newline. A receipt cannot be reused
even when the source and independent review are unchanged. The nonce belongs to
the invocation, not versioned source or a new measurement ledger.

The request also supplies `review_policy`, the exact policy used by
`ReviewStore.snapshot` for that independent review. Native acceptance recomputes
the reviewed source snapshot before running the gate, including on standalone
CLI entry: replaying an old approval after an edit is refused even if the new
source would pass every measurement. Only that task's own review journal is
excluded; publishing/committing the review alone does not invalidate it. Source,
other decisions and policy remain bound. Requests without this policy are refused,
not upgraded using today's configuration. Attribution still trusts the coordinator;
this is stale-review protection, not reviewer authentication.

Each measurement is `{obligation_id, fingerprint, result, evidence}`, with a
SHA-256 input fingerprint and nonempty evidence text. `current` maps obligation
ids to the verifier's independently computed current input fingerprints. Every
requested protected obligation needs one fresh passing measurement before the
`obligations` dimension can pass. Duplicate or unrequested identities are refused;
equal counts cannot hide replacement identities. Missing, stale or unknown
measurements prevent a passing obligations summary. Known failures take priority
over unknown evidence in the aggregate, so actionable defects still reach the
worker. The verifier can leave both collections empty when it cannot measure,
but cannot report passing obligations. Passing the requested
subset alone does not discharge other full-project requirements.

Boolean-only legacy reports are no longer accepted. These checks bind a trusted
verifier's observations; echoing the request fingerprint is not itself proof of
compiler/image freshness, structural correctness or runtime behavior. Those
checks still belong to the verifier. `acceptance_result(..., request=request)`
validates admission evidence; `acceptance_outcome` only validates report syntax
for historical diagnostics and must not be used to grant admission.

An optional `findings` array contains `{dimension, message, evidence}` objects:
the named dimension must not pass, and explanation/evidence must be nonempty.
A failed full gate feeds its `{snapshot, report}` to the worker as
`acceptance_feedback`, within the existing round budget and editing scope.
Changed source requires fresh checks, review and acceptance; discoveries still
pause for authorization. Unknown acceptance with diagnostic findings also reaches
the worker within the same budget, so missing evidence can be investigated or
reported as a discovery. It does not authorize speculative edits or changes to
proof policy. Unknown acceptance without findings stops; no source progress also
stops unless the worker reports a discovery. Feedback is a disposable diagnostic for its
recorded snapshot, not a cached verdict or proof about later source.
The native agent-command bridge validates failure and unknown reports using
their respective verdicts before provider dispatch. Unknown feedback requires
diagnostic findings; passing, contradictory or malformed feedback is refused.

### Native acceptance verifier

Recovery changes and completed applications have different acceptance boundaries.
New projects default to `acceptance_scope: "change"` in `recovery/driver.json`.
`recover run` and `recover start` also accept `--acceptance-scope change|full_project`;
the choice is included in the independently reviewed policy snapshot.

Change mode uses the native checkpoint verifier below: fresh build/link, source
integrity, mappings/history, audit ratchets, bounded declared structure, references,
protected obligations and baseline/current outside-function preservation. Newly
recovered bodies must also pass the current scoped binary check. An independent
review remains mandatory. Missing evidence is not an accepted change. No regression
permission is inferred: the default permission has an empty allowed set. An explicit
coordinator `--migration` decision can admit only the named measured matching failures.

- `change_accepted` / exit 0: the reviewed change passes this bounded verification.
- `migration_admitted` / exit 2: a reviewed structural checkpoint retains named
  matching restoration obligations; it is not a completed recovery.
- Both expose `ready_for_checkpoint: true`, retain `ready_for_collection: false`,
  and do not invoke whole-application acceptance. Publication is opt-in; collection
  into another checkout is not implemented.

For a local verified source commit, pass `run --checkpoint AUTHORIZATION.json`
(also supported by `start`) in change mode. The explicit coordinator authorization
has exactly these fields:

```json
{"author":"task-coordinator","message":"Recover the real owner hierarchy","allow_migration":false}
```

The author must be the task coordinator, independent of worker and reviewer.
This policy participates in the reviewed snapshot; it is not a saved acceptance
receipt. The driver retains its lease from verification through publication.
Only authorized source and validated append-only task/review decisions are committed.
The task baseline and historical obligations are not reset. A migration requires
both the separate matching-regression permission and `allow_migration: true` to
publish; otherwise its source remains uncommitted. Reports include `checkpoint`
with `published`, and the new `commit`, `parent` and `files` when publication succeeds.
No authorization means no source commit. No-change publication creates no empty commit.

Use `run TASK --verify-only --reviewer ...` for final verification without repair
dispatch. It runs the ordinary fresh checks and asks the independent reviewer
again, even when an identical attributed decision already exists. Identical
decisions remain idempotent; they are not duplicated just to log a run. A failed
check, requested change or pending discovery stops with zero worker rounds.
Automatic `--coordinator` delegation is refused in this mode. Neither task scope
nor C++ source is edited by the driver. Review decisions may still be recorded,
and explicit `--checkpoint` authorization can publish a freshly verified local
commit; verification-only does not itself authorize publication or collection.
It also supports the existing `--integration` destination context. This is the
source-read-only verification step used by the separate `integrate publish`
workflow below, not itself a destination update or saved-verdict fast-forward.

Publication refuses staged changes, skip-worktree/assume-unchanged index entries,
Git operations in progress, unrelated edits, Git path overrides and detached HEAD.
It checks raw source bytes/modes/symlinks. Unchanged outside files with declared
CRLF checkout spelling of their committed LF bytes keep both their existing Git
blobs and physical checkout bytes; they are not included in the commit. This
exception requires unchanged text attributes, identical file modes and exact
CRLF-to-LF byte equivalence, and refuses active filter/encoding/ident transforms.
Authorized files always retain the exact raw verified bytes, including CRLF.
The publisher
leases the index and compare-and-swaps the branch. It uses Git plumbing without
clean filters or hooks: required checks must be in the recovery verification policy,
not assumed to run from commit hooks. Git identity must be configured. No push,
merge, rebase, claim stamping, branch deletion or cross-worktree collection occurs.
An interruption between branch update and index replacement can leave the old
index staged against the new commit. Preserve and inspect it; do not reset or
replay a receipt. Noncooperating filesystem writers can still race local locks;
detected publication-time changes require inspection and fresh verification.
Integration into another checkout needs a separate target-context verification
boundary, not reuse of this local acceptance or replacement of the task baseline.

For an **already merged candidate checkout**, `recover run TASK --integration
CONTEXT.json --acceptance-scope change --reviewer ...` now supplies that verification
context. This command does not perform the merge or move the destination branch.
The coordinator decision has exactly these fields:

```json
{"author":"task-coordinator","source_revision":"<full incoming commit id>","target_revision":"<full destination commit id before integration>","evidence":["Why these revisions are authorized for integration"]}
```

Both revisions must retain the original task baseline and already be ancestors of
the candidate HEAD. The target must precede collection of the incoming source.
The source must contain the task authorization. Exact committed task/review history
from both revisions must survive; conflicting streams require explicit resolution
outside this interface, not discarded decisions. Differences from the target may
touch only task-authorized source and valid task/review decisions. The task's
original `base_revision` is unchanged: restoration still traverses its complete
reachable source history, including the incoming branch. No claim is demoted and
no matching regression is implicitly allowed.

Only the outside-scope comparison and unchanged-build-policy comparison use the
authorized target revision. Native compilation, scoped original matching, historical
restoration, references, declared structure, build/link and independent review run
fresh in the candidate context. The context is part of the reviewed policy; a
source-branch approval or saved acceptance cannot be replayed on the merged source.
Workers/reviewers receive it as `context.integration_context`. The read-only
`tasks check-preservation TASK --integration CONTEXT.json` supports the same
comparison for investigation; it is not admission. Reports distinguish the
comparison `base_revision` from `task_base_revision` when this context is present.

Combining `run --integration` with `--checkpoint` can publish a verified **local**
checkpoint on the candidate branch, not fast-forward the destination. Resolving
conflicts, revalidating destination freshness and publishing the destination ref
remain unfinished coordinator responsibilities.
Do not merge unverified candidates onto the production branch to use this command.

The package can now prepare the isolated candidate itself:

```sh
recover integrate prepare TASK --root DESTINATION --context CONTEXT.json \
  --worktree NEW_PATH --branch NEW_BRANCH
```

This explicitly authorizes creating a new local branch/worktree, not collection.
The destination must be clean at the context's target commit, honoring Git's
built-in text normalization without running external filters. Its actual checkout
bytes are preserved; the candidate uses exact stored commit bytes. Existing
paths/branches, staged work and ongoing Git operations are refused.
Nested candidate paths must be ignored and outside Git metadata. Keep context JSON
outside the destination checkout or in ignored coordinator storage. The decision
is retained in the unverified candidate's Git message; its two parents also pin
the destination and incoming source. No measurement ledger or new task baseline
is created.

Git constructs the merge tree without checking it out over either input. Conflicts
return `merge_conflicts`, affected paths and exit 3; no candidate branch/worktree is
created for text conflicts. Successful raw materialization preserves executable
bits and internal file symlinks, then validates scope and both sides' task history.
Exit 0 / `candidate_prepared` means **unverified preparation only**, with
`verification: not_measured` and `ready_for_collection: false`. No original image,
ignored build cache, Wine prefix or credentials are copied. Allocate isolated
compiler resources and run the project's native `recover prepare` before driving
verification in the candidate. Neither the destination branch nor its files move.

Preparation disables Git hooks/filters and refuses configured external merge
drivers or active checkout-filter attributes. Unused global filter configuration
does not block ordinary source projects. Directory/external/dangling symlinks and
submodules are unsupported. Git must support `merge-tree --write-tree` and
`check-attr --source`. A failure after candidate creation retains that path/branch
for inspection; it never resets or reaps possible work. Local leases and hashes
are coordination/integrity checks, not a sandbox against arbitrary filesystem writers.

When preparation is configured, preservation regenerates recipes and generated
headers independently for baseline/current source and each damage control. Private
projects own their Git HEAD/index and read historical objects through an alternate;
their current policy and HEAD stay fixed while their source file sets vary. No
original source, index, task baseline or generated cache is edited. These are
disposable ignored contexts, not branches, worktrees requiring collection, or
historical build-policy recreation. Current compiler output must agree with a fresh
current-context control; stale caches and path-sensitive output cannot pass.
Preparation policy and its non-source inputs must match the authorized baseline.

The isolated-worktree VC6 control now reaches per-change acceptance and authorized
local checkpoint publication with the destination untouched. Separate real-compiler
controls detect source-dependent generated-header collateral changes and tampered
current caches. Generated metadata is not a historical C++ substitution, but genuine
generated compiler inputs are independently rebuilt, not copied into both versions.
Unobserved source controls, failed generators, aliased/external recipe identities,
overlapping source/artifact roots and unsupported historical TU coverage remain
unverified. Configured generators/compilers are trusted, not sandboxed; external
toolchain and Python environment availability remain project responsibilities.
This is a test-owned application with deterministic subprocess agents, not proof of
a live-provider Alpha Centauri recovery.

To collect a candidate into its destination, use the separate explicit command:

```sh
recover integrate publish TASK --root DESTINATION --candidate CANDIDATE \
  --authorization AUTHORIZATION.json --reviewer recovery/reviewer.json
```

Authorization has exactly these fields (commit placeholders must be replaced by
full immutable commit ids):

```json
{
  "author": "task-coordinator",
  "candidate_revision": "<candidate HEAD>",
  "destination_branch": "refs/heads/master",
  "integration": {
    "author": "task-coordinator",
    "source_revision": "<original incoming commit>",
    "target_revision": "<destination before integration>",
    "evidence": ["Coordinator rationale for this collection"]
  },
  "message": "Collect the recovered owner hierarchy",
  "allow_migration": false
}
```

Both checkouts must belong to the same local repository and start clean, on the
authorized destination branch and exact candidate revision. Incoming task/history
decisions and original baselines survive. Worker/reviewer/config paths are relative
to the candidate; the authorization path is relative to the destination. Both
driver leases stay held. Configured preparation runs separately in each checkout.
Fresh verification-only admission and an authorized local checkpoint precede the
fast-forward; fresh destination-side verification and local decision publication
follow it. The reviewer sees the collection authorization in context. No worker or
scope delegate is launched and no saved acceptance report is consumed.

`destination_accepted` exits 0 only after fresh destination acceptance.
`destination_migration_admitted` exits 2, retaining matching obligations; it needs
both `allow_migration: true` and separate `--migration` permission. Missing/failing
evidence exits 3. Read `destination_updated` separately from `destination_verified`;
successful Git transfer alone is not acceptance or a working game. `ready_for_collection`
stays false: this command has already attempted collection, not full-project proof.
The package disables hooks, checkout filters, autostash and ignored-file overwrite.
Dirty/staged work, stale revisions/branches, sparse checkouts, ignored path collisions
and unsupported source aliases refuse publication. No force update, push, automatic
conflict resolution, worktree deletion, claim demotion, or reset participates.

The fast-forward and final checks are not a filesystem transaction. On interruption
or a destination check failure, preserve both checkouts; the destination may already
have moved (`destination_updated: null` means the interrupted update is uncertain).
The previous target remains in Git history, and authorization is carried in the
local checkpoint message. Inspect HEAD/index/source before continuing. Fix the
reported prerequisite and run fresh `run TASK --verify-only --integration ...
--reviewer ...`, with explicit `--checkpoint` if publishing the new decision.
Never replay a report, blindly retry the old fast-forward authorization, or reset
away recovery work. Local leases coordinate cooperating agents, not arbitrary Git
or filesystem writers. Compiler/provider availability remains project configuration.
Real-compiler controls cover successful collection, destination-only build failure,
fresh retry, and preservation of unchanged CRLF checkout files.

Unrelated unfinished functions need not match the original to accept a change;
their preservation must still be measured. Global source/mapping/audit defects can
still prevent checkpoint admission. This boundary does not prove whole-linked-image
equivalence, complete lifetime correctness, dependency ABI or runtime behavior.
Missing bounded structural witnesses and unsupported preservation coverage still
refuse admission. Use `--acceptance-scope full_project` for the stronger application
boundary below; configurations without the scope field retain that boundary.

Unresolved discoveries recorded on the task also refuse native checkpoint
verification, including direct `tasks check-migration` calls. A fresh generic
approval or matching-regression permission does not resolve them. Coordinator
disposition remains in the task journal and requires a fresh review afterward;
out-of-scope findings cannot be incorporated without scope authorization. Separate
tasks' unfinished work is not automatically a dependency of this change, but any
dependency discovered by this task must remain recorded here until addressed.
Task decisions must participate in the source review snapshot: ignored task or
journal records refuse verification; unignore them and obtain a fresh review.

In change mode, a valid measured refusal produces disposable
`checkpoint_feedback: {snapshot, result, reason, report}`. The native verifier
includes bounded `diagnostics` for failed/incomplete build, preservation, restoration,
policy and other prerequisite checks. A failure, or an unknown with diagnostic or
non-passing obligation evidence, reaches the worker within the same scope and round
budget. Missing capabilities or outside-scope effects require discoveries, not
speculative edits or changes to proof policy. No-progress and budget stops remain;
source changes require fresh checks and review. The native bridge validates the
feedback envelope before dispatch. Malformed receipts, stale review snapshots,
unauthorized changes and invalid permissions remain protocol errors, not repair
feedback. Final refusal reports use `change_fail` or `change_unknown` and cannot
become checkpoint-ready. No admission is inferred from diagnostics alone.

`recover tasks check-acceptance TASK --image IMAGE` reads the complete driver
request from stdin and returns the request-bound receipt above. `init` configures
this installed command; no project acceptance dispatcher is required. It runs
the whole native/configured gate, retains one fresh build, then verifies strict
task matching, declared structure, historical obligations, outside-scope compiled
and linked preservation and unchanged baseline compiler/build policy. Failed native checks
cannot be waived by review or external proofs. A failed/unknown whole gate stops
before proof commands run. Compiler/image/source/Git/task changes invalidate the
epoch; the verifier never stamps annotations or changes the task baseline.
Full-project acceptance additionally refuses unresolved discoveries in any of the
project's versioned task journals, before running the gate or proof commands.
Per-change approval does not clear that recovery backlog or establish game
acceptance. New or changed task decisions during verification invalidate its epoch.

The final application boundary also runs `source_form.check_source_form`: explicit
`asm`/`__asm`/`_emit` spellings fail even when byte comparisons and proof commands
pass. Comments and literals are ignored; line splices and raw strings are handled.
Unsupported lexing is unknown. All configured application branches are scanned;
seed roots and the exact files in source-owned upstream component declarations
are reported separately, not treated as application recoveries. This is a bounded
lexical check without macro expansion, not proof of genuine hierarchy or upstream
source identity. It does not impose unrelated assembly cleanup on every checkpoint.

Complete structure and runtime are distinct from declared member offsets and
successful linking. Configure trusted proof commands in `recovery/project.json`:

```json
"acceptance": {
  "checks": {
    "structure": {"argv": ["local-structure-check", "{root}"], "timeout": 120},
    "runtime": {"argv": ["local-runtime-check", "{root}"], "timeout": 120}
  },
  "inputs": ["recovery/proof-policy.json"],
  "environment": ["TEST_ASSETS"]
}
```

Commands use the usual JSON stdin/stdout boundary. Declare all additional proof
inputs (including scripts and assets) and environment dependencies; missing inputs
refuse verification. Each command receives the original request plus `dimension`,
`image: {path, sha256}`, and `build: {directory, outputs}` identifying the same
retained build for both proofs. Paths are relative to the project root, except
the original image path. Output rows contain their relative artifact path, format,
SHA-256 and size. The command must test those exact artifacts; the verifier
rechecks their content and rejects symlink substitution after each stage.
The final native receipt exposes `artifacts`, mapping normalized project-relative
paths to SHA-256 content hashes. This identifies the exact retained products, not
the next build or a deployable versioned artifact. Consumers must recheck those
hashes before using them; the manifest is disposable evidence, not a cached pass.
Other trusted verifiers may include the same optional field. It is invalid to
report artifact identities with a non-passing build, and a manifest cannot turn
failed/unknown acceptance into success.

The command's `protected` array contains only that dimension's obligations. Reply
with exactly `request_fingerprint`, `result`, `evidence` (nonempty string array),
`measurements`, and `current`; the fingerprint covers the complete dimension
payload. Exit codes are 0/pass, 1/fail, 3/unknown. Measurement syntax and freshness
rules are the same as the final receipt. A hook cannot report measurements for
native bytes, references or declared structure, or replace a native failed result.
Missing hooks, incomplete measurements and unsupported proof coverage remain
unknown, never a vacuous pass. Proof code is trusted verifier code, not a worker
self-assessment or a script that merely echoes a digest. The package supplies no
universal runtime/semantic oracle. Retained products and receipts are disposable
verification artifacts, not a second source ledger or permission to collect a
later build without rechecking.

Optional `migration_check` and coordinator `--migration` permission allow reviewed
temporary matching regressions while structural work proceeds. They require
source obligations, fresh build/structure checks and independent review. Unknown
proof is not permission to regress. A migration checkpoint exits 2, never green;
closing it requires restoring all protected obligations. The reusable policy APIs
live in `migration.py`, `migration_protocol.py`, `reviews.py` and `tasks.py`.

Migration permission also supports experimental, bounded generated-initializer
intent through an optional nonempty `generated_changes` array:

```json
{
  "entity_id": "entity-<original identity SHA-256>",
  "unit": "code/lifecycle.cc",
  "version": "current",
  "symbol": "_$E21",
  "offset": 0,
  "evidence": ["This exact registration implements the scoped lifecycle change"]
}
```

Each entry names one exact TU-local initializer-table slot: `baseline` selects a
removed occurrence and `current` an added occurrence. Offsets are four-byte
aligned and below 16 MiB; at most 256 entries are allowed, with no repeated slot
or root in one version/TU. Source `ORIGINAL` or `RECOVERY_OBJECT` comments remain
the load-bearing original identities; JSON records the coordinator's intent,
not a measured ledger. The selected entity must have one original source anchor
in that exact source version and editable product TU; header/shared/seed anchors
alone are unsupported. Neither symbol prefixes nor editable-TU scope grant
ownership of other generated code.

The complete permission must match the independently reviewed policy, and the
coordinator must be separate from worker and reviewer. Bounded slot projection
still requires fresh native table/root evidence and matching live application
objects for the actual preservation trial. A permission alone subtracts nothing:
shared code/storage and all uncovered residuals remain protected. This route does
not establish complete linked-graph or runtime acceptance, reset a task baseline,
or waive historical original-byte obligations.

Native root/reachable-code observations can bind an external absolute COFF
constant through the complete candidate
inventory and its unique selected physical definition. Its value and symbol
remain in cross-build fingerprints; input provenance remains in diagnostics.
These DIR32 operands compare the constant plus source addend and require no
loader HIGHLOW. Image-address bindings, including numeric zero, retain their
HIGHLOW obligation. Unknown/duplicate/unselected definitions, conflicting map
placements and unsupported linker policy refuse; relative constant uses remain
unknown. Constant correspondence is not runtime storage or behavior proof.
Names are resolved in a bounded batch per code region, sharing a complete-input
before/after freshness check rather than repeating it for every operand.

Missing private-data placements can additionally use COFF debug symbols embedded
in the **same freshly linked PE**. The complete physical input population must
contain exactly one occurrence of the name, in a plain static data definition
from the exact selected owner. Duplicate names (including unselected objects),
conflicting map entries, malformed metadata, unsupported linker policy and stale
inputs refuse. Debug symbols supply RVAs, not inferred object-section offsets;
the ordinary operand and HIGHLOW checks still apply. No neighboring symbol,
pointer value or other build supplies an address. The compiler environment may
request embedded metadata; the library never injects linker options. Debug
options can alter the linked layout even with explicit optimization flags, so
diagnostic-build addresses must not be transplanted. Target contents, associated
private code and runtime behavior remain unproved. A uniquely identified label
in associative data can bind without placing or proving its code parent. This adds no PDB requirement,
new source ledger, CLI or acceptance exemption.

The `projected_order` dimension qualifies initializer order by the
actual compiler source unit (or exact physical dependency identity), not just
body hashes or object basenames. It removes only
the explicitly selected linked occurrences and compares every remaining root,
reference fingerprint and registration occurrence in order. A selected root
with ambiguous repeated placement refuses. Swapping identical code from different
TUs stays visible. Compiler-local root renaming is supported only at the same
remaining source-qualified slot after native checks: one static root and table
occurrence, no exposed/weak aliases, self-address or other runtime incoming
references, no source symbol binding in the root/table/metadata (original,
object, shared, generated or import annotations, including headers and seeds),
and no public/other-owner linked aliases or exports into the root. The native
PE export inventory includes ordinal-only entries, aliases, holes and forwarders;
unrelated valid exports do not prevent comparison, while incomplete tables refuse.
Exported initializer table/boundary addresses and public aliases into its cells
also refuse, since those expose root addresses indirectly. This
does not pair arbitrary equal-byte functions or use naming prefixes. Observed
spellings remain in the report. A proven unowned-order failure refuses the
generated migration; passing this dimension still cannot waive callback, storage,
COMDAT or other linked-effect obligations.
Failed order reports include the total positional mismatch count and the first
16 baseline/current logical rows, alongside the complete sequences. These are
diagnostics, not an inference that changed root spellings are equivalent.
The narrow recognized associated FPO frame-record form may accompany a renamed
root only when its complete root-relative structure agrees in both versions.
It is retained in the comparison fingerprint and `root_metadata_comparisons`,
not silently ignored as debug data. Final linked treatment and address-identity
effects remain unknown; absent PE debug directories are not discard proof.
Unsupported metadata or failed locality keeps exact names and reports the refusal.
Order comparison alone removes no startup code or metadata and grants no residual coverage.

For explicitly authorized generated changes, the live application-object producer
can cover an exclusive root's exact associated FPO child. The complete trial object
must reproduce the selected application object, and both frame records must match
one unique image-bound PDB record at that root's independently observed address.
Only that child and its root can leave the strict object residual together.
Renumbered trial sections are bound through the original root and association;
shared roots, aliases, protected source bindings and unsupported metadata refuse.
Without native coverage the frame continues to protect its root. This does not
establish debug-data discard, runtime effects or checkpoint acceptance.

The same native preservation path can now explain a redundant **common request
declaration** introduced or removed with a covered generated root. It derives the
exact occurrence from that root's actual removed references; an unreferenced new
declaration or a selected-but-retained root supplies no eligibility. The original
startup capability must belong to the same live producer and exact compiler trial
(including source version, TU, flags, authorization and input fingerprint).
Unchanged declarations keep the ordinary comparison and need no projection.

Both fresh linked applications must independently prove the same named uniform
common extent and zero initialization, without conflicting named definitions or
weak aliases.
A separate, exactly selected compiled TU must retain the same request in both
builds, outside **all** TUs in the complete generated-change permission. Requests
cannot cite each other and then both disappear. Ordinary undefined references do
not supply witnesses; basenames, equal hashes and unselected archive candidates
cannot replace physical input/receipt correspondence. The first supported witness
domain is compiled TUs, not inferred dependency-member substitutions.

Only a live, nonserializable `DuplicateCommonRequestCoverage` can remove the one
exact common symbol row, after scoped/startup projection. Remaining code/data
relocations or weak fallbacks to that name refuse; their identities, addends and
request-size differences are never normalized away. Other data, imports,
directives, helper code, initializer writes and ordering remain protected. Both
linked epochs and task authorization remain checked through consumption and
final exit. This is declaration redundancy, **not** allocation isolation,
alignment, alias correctness, complete linked effects or autonomous acceptance.
No source mapping, historical claim or task baseline changes as a result.

The native dependency-code graph also verifies whole ordinary COFF code sections,
including multiple external entries and nonzero entry offsets. Every external
entry must independently identify the same selected physical section placement;
leading bytes, assembler labels, jump vectors, padding, relocations and loader
fixups remain protected. Entry identities and offsets remain distinct. An inert
end label is metadata only when no relocation or weak fallback addresses it.
These are bounded sections, not guessed function extents, and do not relax the
single-function COMDAT measurement API.

Identical-code folding requires complete final-pass linker groups bound to fresh
physical members and one independent map placement. Every original member's code
and dependencies are verified, including private code attributed to another
owner in the map. The exact multiset of source frame records must match the
image-bound PDB; choosing one matching duplicate or deduplicating metadata is not
proof. Shared code extents are admitted only within that verified group.

The physical application section domain also recognizes independently proven
COMDAT `ANY` duplicates. Inside one live native input epoch, it inventories every
same-key definition (including archive occurrences), validates the original COFF
key and selection policy, and verifies the retained winner's code, references,
MAP placement and image-bound PDB module/contribution. Aliases, conflicting
declarations or unsupported selection policy refuse qualification.
For keys without a positive folding group, an absent fold-log entry is
insufficient: every other possible physical code section needs an immutable byte
mismatch outside relocation fields and possible trailing padding, a positive native
REF certificate for that exact physical root/family, or a separately verified unique
NODUP root at a disjoint final placement. The latter requires its
complete unique-definition inventory, exact original selected input, own-owner
MAP and image-bound PDB module/contribution, and full code/reference/companion
verification. Contradictory REF/ICF evidence, aliases, mixed/ambiguous declarations
and overlapping root intervals refuse. There is no recursive ANY winner guess.
Each placement is shared only within the guarded native epoch; disjointness is
rechecked for each prospective winner. The derived `comdat_unique_placements`
inventory retains complete proofs referenced by no-fold certificates. It rules
out only the exact physical root as a storage competitor: that root remains in
the application graph, and its children and all other candidates still need
their own checks. Same-key root exclusions never hide associated code children.
The observed private/static NODUP function form has a separate strict recognizer;
the global external-key API is unchanged. Its independent placement requires an
exact original object/section/symbol identity and own-owner static MAP entry,
plus the same complete code, frame, module and contribution checks. Private names
may repeat in different objects: a matching spelling never transfers another
owner's placement or event. Same-object ambiguity, aliases, malformed function
auxiliaries and own/ownerless REF/ICF observations refuse. Foreign REF events
need independently authenticated private owners. ICF reconciliation inspects
complete original event families, including unprinted local aliases: possible
rewriting of a private name refuses even when code and PDB ranges agree.
`root_identity_kind` distinguishes private from external entries
in `comdat_unique_placements`; neither kind removes that root or its descendants
from the physical/code obligations. No private REF-discard capability is implied.
The physical section domain composes those placements into retained selection:
when a COMDAT section has neither a removal certificate nor a refused removal
query, `ComdatSelection.placed` returns the root's verified placement and the
section is annotated `retention: pass` with `disposition:
independently_placed_nodup_root`, its `selection_placement` id and its
`root_identity_kind`; the certified ANY winner earns
`disposition: selected_any_winner` referencing its `selection_certificate`
id. Placed sections fall through to ordinary code-root
seeding; annotation never excludes them or their descendants. The placement
proof refuses genuine REF/ICF contradictions itself, so an unsupported removal
form (a private root) does not block its own placement annotation. Sections
with no qualifying placement stay unknown and keep their gap; the report-level
`retained_section_selection` stays unknown until every selected section earns
its own disposition.
REF composition authenticates the original identity, object hash and exact family
membership before reusing the existing discard verifier. It works even when the
ANY query precedes the domain's REF queries; it does not depend on census order.
The no-fold proof references complete `ref_discard_certificates` by identity, and
their original sections and incoming-reference obligations remain inventoried.
An absent MAP entry or ownerless, ambiguous or contradictory discard observation
cannot remove a competitor. No recursive ANY selection or report replay is used.
This conservative proof requires an explicitly nonincremental, relocation-only
link; ambiguous competitors stay unresolved. PDB module/extent correspondence
does not equate characteristics: a reserved-bit difference is retained explicitly
as unknown and cannot waive separate allocation or runtime checks.
Non-code section merges require a separately bounded nonexecutable output target
and no affected physical code section; this grants no data or initialization
exemption. Repeated per-key refusals are memoized only inside the same guarded
native batch. New batches recheck them; I/O and unexpected failures are not cached.
Only certified losing sections and their original associative descendants leave
the code-root domain; their physical identities and metadata remain in the report.
Loser bytes need not equal the winner. A winner proof is shared per key within the
guarded batch, never replayed from a report. Missing MAP labels still cannot prove
reference-elimination (`REF`) discard. This is not complete retained-section
selection, source ownership transfer, outside-scope preservation or acceptance.

Positive native ICF groups can compose with `ANY` selection. The original
same-key winner is distinct from the final shared-code storage owner: a MAP
owner may also contain a discarded copy of that key. Every original group
member must bind to one globally unique raw optimization event and selected
physical input, with all original code, references, frame records and companion
regions verified. Cross-member overlaps obey the native code graph's exact
extent and family restrictions; folded CodeView remains unsupported. The final
survivor needs its own exact MAP/PDB module and contribution. Qualified or
ambiguous labels, cross-group chains and contradictory REF evidence refuse.
Only the authenticated rewritten public key may name the losing owner's MAP
alias; other independently placed loser labels remain contradictions. The
`comdat_folded_groups` domain inventory holds each complete group proof once,
with per-key certificates referencing it. All original producers remain code
roots; only proven same-key duplicates stop seeding roots. These guarded,
derived observations do not establish outside-scope preservation or acceptance.

The same guarded native selection context can certify positive `/OPT:REF`
removal of an exact NODUP code root and its original associative forest. The
framed executable log must contain one owner-qualified discard event; explicit
REF, nonincremental and verbose arguments, one independently selected physical
candidate, exact root policy and conflict checks are required. Bare object or
archive-member labels must agree with their independently selected final-pass
family. Qualified paths currently refuse rather than borrowing a basename.
Repeated events, archive occurrences, weak aliases into private descendants,
forced retention and MAP/ICF contradictions cannot authorize removal. Folding
events remain contradictions even when no PDB-backed folding handle is available.
No event means no removal evidence. Original sections, relocation edges and
unresolved incoming references remain visible; only certified sections stop
seeding retained code/data roots. Parsed logs and certificates are derived
diagnostics, not source facts or replayable acceptance. Native compiler controls
exercise actual REF on/off behavior; these are not full-game acceptance tests.
Ordinary COFF function-definition auxiliaries are supported only with coherent
original `.bf`/`.lf`/`.ef` records, next-function links and bounded physical line
tables. Their debug-reported function size never shrinks the measured section or
waives alignment bytes. Parsed-state guards preserve exact scalar types and
record order independently of the native linking layer.

Supported i386 short-import thunks use a separate six-byte linker recipe. A unique
producer in the complete physical input population, final-pass library family,
named map uses, independent PE import identity and exact operand/fixup all must
agree. Repeated archive member names do not establish owner-only selection.
This verifies the thunk, not the imported DLL. Storage, indirect targets, runtime
effects and other residuals remain independent obligations; none of these checks
turns runtime-library code into recovered application code or grants acceptance.

Unplaced compiler-label hints are not relocation bindings. Dollar-prefixed local
labels, exception-data associations and repeated pooled-literal contents remain
diagnostics with unknown target agreement; none may mask an operand into a
passing strict comparison. Independently mapped companion code and explicit
targets still undergo ordinary byte/addend/loader-fixup comparison. Unknown
artifact references do not hide unrelated opcode or known-target failures.

Synthetic linker-module contributions can separately identify an isolated,
six-byte import-thunk output recipe. A native generated-module identity, exact
PE/PDB coordinates, executable code characteristics, independently validated
readable import slot and exact loader relocation are required. Duplicate or
intersecting contributions remain unmatched. Import-shaped bytes alone are not
evidence of valid generated storage, and this output assignment does not supply
the physical-producer proof above. The real VC6 control attributes its import
thunk to the library module and leaves the synthetic linker's debug contribution
unmatched; neither is reassigned to manufacture a closed allocation domain.

For reviewed generated changes, the native application graph additionally derives
a generated-root code residual inside both live build contexts. Only exact,
source-authorized slot-local roots can lose their bodies in that comparison;
protected source bindings, shared roots and exported aliases refuse. The union
of all remaining code producers must be independently observed in both builds
to a bounded fixed point. Producers already in a build's fresh complete graph
retain that evidence; only missing producers need additional resolution and
roots. This avoids duplicating verified folded entries under map/original-owner
aliases without weakening the graph's ambiguity refusal. A helper does not disappear from checks
when its only initializer caller is removed. Missing linked placement is unknown,
not inferred discard or permission. Native-proven local-root renamings retain
their source-qualified occurrence identity; unrelated names never match by bytes.

Residual correspondence retains original producer identities, entry offsets,
metadata, references into selected roots, and alias/folding/import-slot sharing.
Compiled producers use exact per-build receipts and stable source TUs; dependency
producers retain physical input/member/occurrence and content identity. Parsed
link evidence and acceleration caches are guarded throughout use and at close.
The result remains a code dimension, not checkpoint admission: other task helper
bodies gain no implicit waiver. A separate native storage residual now follows
ordinary data, common allocations and import slots reached by either build's
code, including dependencies of explicitly removed roots. Ordinary sections are
bound to independently selected physical inputs, then their complete initial
bytes, relocations, signed addends and loader fixups are checked against that
build's actual image. Compiled sections correspond by source TU and unique
section name; dependency sections retain their physical member/content identity.
Private labels cannot acquire placement from the pointer being checked.

The two-build dependency union expands through data-to-data cycles and feeds
callbacks back into the native code verifier. Newly observed code can add more
data in the next bounded round. Effective reference offsets must remain inside
the independently observed target extent. Removing a caller never removes its
former target from preservation. Section contents, reference identities, extents,
flags and observed sharing relationships compare independently in both builds;
this does not prove isolation from unobserved storage. Import-slot correspondence
does not prove the imported library's implementation or ABI. Unsupported special
or COMDAT data remains unknown. Section-producer parsing is shared only within a
live, guarded build epoch; neither cached reports nor descriptors grant scope.

A selected-input/linked-image data counterexample fails the application graph,
even if a later dependency lookup is incomplete. Cross-build storage differences
remain a separate diagnostic pending ownership composition; they are not
automatically classified as unauthorized changes. Complete linked selection,
allocation isolation and remaining residual effects still prevent graph admission.
These checks do not establish autonomous or runtime acceptance.

The same paired native observation also compares the complete final-pass input
families before tracing initializers. An authorized ordinary function can pull
in a library member that contributes other definitions outside that trace.
Traversed dependency code already retains its **whole-member** content identity;
the additional input boundary covers families not reached by the initializer
graph. It retains native link order, source-qualified compiled inputs and exact
dependency paths/member occurrences/content identities. Repeated archive names
remain candidate families, not an invented unique selected member. Compiled TU
deltas are reported separately from dependency-family changes.

This comparison and data-producer lookup share one expiring native input epoch
per build, with physical files, receipts and parsed indices guarded throughout.
Selection diagnostics survive a later incomplete initializer/code observation.
A changed dependency family requires complete contribution preservation before
admission; equal families alone cannot prove COMDAT selection, retained private
sections, aliases or allocation isolation. Those remaining unknowns still refuse
autonomous acceptance. No source mapping, task baseline or permission is inferred
from this comparison.

The package supplies the native `migration_check` command:
`recover tasks check-migration <task> --image <image>`. It consumes the driver's
JSON request on stdin and freshly runs source integrity, mapping/history checks,
configured audits, build, restoration and full-TU compiled-residual preservation.
The request includes the same exact `review_policy` used for the independent
review; the verifier re-derives that source snapshot before and throughout its
checks. A matching pair of stale snapshot tokens cannot admit a checkpoint.
Its exit 0 means the measurement envelope completed, **not** admission; only the
driver applies permission/review and can return a tracked checkpoint (exit 2).
Build-policy changes, missing compiler controls, unresolved references and
unmeasured obligations prevent admission. No target-specific verifier script is
needed for these supported checks. This is not full-link equivalence or runtime
acceptance. Preservation still refuses unsupported TU migrations and changes to
independent data/tables outside its function-ownership model.

Ordinary unchanged-build-policy changes must also supply qualified linked
outside-scope preservation. The native verifier no longer promotes the compiled
residual's overall pass while ignoring its `outside_scope_verification: unknown`.
Missing linked evidence produces actionable refusal, even when build, scoped
matches and independent review pass. Current native compiled preservation cannot
alone yield `change_accepted` or authorize local/destination publication. A live
graph result, where available, remains a separate required prerequisite; neither
input-family equality nor a cached report satisfies retained-selection proof.
The full-project native acceptance verifier requires the same qualified linked
preservation independently of configured structure/runtime proof commands.

The application residual now derives task-body extents from source ownership and
fresh physical COFF/link evidence. Verified direct CodeView/FPO debug children
do not disqualify an otherwise independent function, but remain outside its
runtime-body range. Runtime companions, shared definitions, folded bodies and
observed address aliases refuse whole-body scope. The final dependency graph is
checked again after expansion, before comparing all remaining PE file bytes when
both versions have identical geometry and body placements. Directory/debug payload
overlaps refuse scope; timestamps, debug data, padding and overlays are not masked.
That literal comparison is diagnostic byte accounting, not an acceptance grant.
A separate native qualifier now admits complete projected-image equality without
any body omission, including ICF builds. Differing images require a supported
same-layout domain: explicit `NOICF`, exclusive body contributions bound to
physical COFF/PDB/MAP inputs, and complete supported PE metadata ranges outside
task bodies. Projected fields cannot overlap executable storage or observed
runtime regions, including associated data. Only
authenticated build timestamps, NB10 signature/age and the exact declared PDB
locator may differ as build identification. Every other byte, including padding
and overlays, remains compared. Compiler-declared path encodings survive typed
transport even when an artifact uses an encoding absent from command operands.
This model does not establish debugger identity, executable-file or mapped
metadata self-inspection equivalence, or runtime correctness.

The metadata domain includes complete PE export tables, aliases, ordinal-only
slots, holes and forwarders. Only the export creation timestamp may be projected,
and only when it equals that image's COFF creation timestamp. Versions, names,
ordinals, target RVAs and forwarder strings remain literal. Indirect arrays and
strings remain protected even outside the directory span. A direct export protects
its entire containing raw section against identification projection; this is not
an inferred object extent, body waiver or transitive exported-pointer proof.
Overlapping metadata uses, executable/runtime backing and loader fixups still
refuse projection. The model's mapped-build-metadata self-inspection exclusion
also applies to accesses through exported pointers.

Fresh source-authorized composition can now satisfy linked outside-scope
preservation for that domain; permission, review, compiled preservation and all
other driver checks remain independent. A real compiler/linker control reaches
`change_accepted` through two worker rounds recovering a base/member model, with
deterministic reasoning agents and no application structure/runtime proof command
in change mode. It is not a live AI recovery or full-game acceptance result.
Differing ICF outputs, changed layouts/body placements, unsupported directories
and shared ownership still refuse body-omission qualification. OpenSMACX uses ICF;
its compiler policy and task baselines are not changed to pass.

Strict code-residual diagnostics retain cross-version debugger type differences.
A separate native runtime residual can disregard only independently bound
CodeView procedure type graphs; procedure/local offsets, FPO, runtime code/data,
references and aliases remain compared. Each build must still reproduce its own
complete source-to-PDB type evidence. This permits a caller's debug description
to reflect a legitimate class repair, not a waiver of source ABI/layout review.
Both runtime residual and qualified output are required to discharge that strict
type-only difference. Missing native evidence and other failures still refuse.
Residual diagnostics identify differing producers/field paths with bounded output;
they never supply acceptance evidence.
Both change and full-project orchestration consume this composition. With native
compile/link configuration, the full gate acquires a live paired application graph
at its build stage, after preparation, and uses that graph's exact current build.
Compiled preservation, linked evidence and structure/runtime proof commands share
the same native context through the final checks. The complete current replica
is retained under ignored build storage; native capabilities expire on exit.
Only artifact display paths are made checkout-relative; compiler/receipt evidence
keeps its original physical identity. A serialized result, another build or a
compiled-only PASS cannot substitute. A real compiler-backed tiny-application
control passes full acceptance through structural repair and byte restoration;
this is not full-game or live-agent recovery acceptance.

Native build snapshots avoid repeatedly testing outer ancestors while keeping
every in-root symlink check, path resolution and content hash. Linked associated
families reuse the enclosing graph's duplicate-preserving, exact-view map index
and expiring optional-constant lookup. Standalone verification and present
constants retain full resolver checks; no lookup persists across runs or replaces
input/alias/freshness evidence.
Direct initializer roots share one exact map index and the same expiring
constant lookup as their dependency traversal. The complete physical population
is checked before and after observation, including exceptional exits; measured
constant operands and present-constant resolution retain their ordinary checks.

## Package boundary and verification

### Pointer-member interpretations

An original-layout interpretation can require pointer storage and its class
target, not just an offset:

```cpp
// RECOVERY_MEMBER: v3 Window canvas_ Canvas* 0x10 0x00401000
Canvas *canvas_;
```

Use original constructor/caller evidence for the address and interpretation;
this example is not a bundled binding. `tasks check-layout` checks the source
expectation against every scoped compiler trial, reporting `offset_result`,
`type_result` and the observed storage shape. The standalone CodeView 5 backend
supports near32 ordinary pointers, distinguishing class values, references and
different pointer targets even at identical offsets. Its pointer format follows
[Microsoft's CodeView definitions](https://github.com/microsoft/microsoft-pdb/blob/master/include/cvinfo.h).
Malformed, cyclic, unsupported or missing type metadata stays unknown. Validated
const/volatile wrappers preserve unqualified class identity; this does not prove
qualifiers, ownership, lifetime or full structural correctness.

Version 1 member annotations require offsets only; version 2 additionally
requires an embedded class value. Refining or removing a type interpretation
preserves historical review requirements, and outside-scope annotation changes
remain visible even when object code is unchanged. Results are disposable;
only the original interpretation belongs in source.

### Bounded class-array interpretations

```cpp
// RECOVERY_MEMBER: v4 Window panes_ Rect[2] 0x10 0x20 0x00401000
Rect panes_[2];
```

Version 4 names the unqualified element class, positive element count, original
member offset, total array byte extent, and original evidence address, in that
order. Count and extent require original access/boundary evidence; do not copy
them from the candidate compiler. The example's two elements occupy 32 bytes.
The checker reports offset, element type, `array_count_result` and
`array_size_result` separately. All must agree. Same-sized scalars, pointers,
wrong classes and a different number of larger elements cannot substitute.

The standalone CodeView backend resolves element storage in the same object's
type stream, including an unambiguous complete definition for a forward class.
Missing stride evidence remains unknown. Malformed or cyclic types and arrays
extending past their containing class cannot supply a passing layout. This
version supports one-dimensional arrays of class values, not arrays of pointers
or a complete C++ type system. Qualifiers, lifetime, original indexing behavior
and runtime bounds safety remain separate requirements. Array changes retain
their historical interpretation/review obligations like other member changes.

### Original object initialization

`recover check-data --image <original> [--symbol <emitted-name>] [--summary]`
checks source-owned objects through the configured compiler. Repeated `--symbol`
selects exact emitted names; the default includes every object binding. A data-only
project needs no legacy function seeds. Source, compiler policy and image changes
during measurement invalidate the whole report. Results are disposable JSON.

Bind an object beside its definition. Version 1 names its original start only;
version 2 additionally records an independently established, exclusive-end extent
and the original evidence anchor requiring that interpretation:

```cpp
// RECOVERY_OBJECT: v2 0x00402000 0x00402004 ?Selected@@3PAVCanvas@@A 0x00401000
Canvas *Selected = &DefaultCanvas;
```

The addresses and symbols above are examples, not bundled project bindings.
Obtain bounds from original accesses/data evidence, never from the candidate's
size or the next symbol. This is an initial-storage requirement, not a stored
verdict. Rehoming preserves its identity; replacing/removing the interpretation
retains historical review requirements on the evidence anchor's task.

The PE/COFF backend uses standalone CodeView data records to measure candidate
widths, tying the exact emitted symbol to its debug record with SECTION/SECREL
relocations. It does not guess sizes from C++ spelling or neighboring symbols.
Forward class/struct/union storage records require one complete same-kind,
qualified-name definition in that standalone stream; missing or ambiguous
definitions remain unknown, and no other TU supplies a substitute layout.
The configured `layout_flags` request the compiler's supported standalone debug
metadata. Initial bytes include virtual zero fill, but exclude executable sections.
Pointer slots require the correct source-bound targets and original relocation
form; hardcoded original addresses cannot pass through byte equality alone.
Every selected object is checked in each configured trial, compiling shared TUs
once per trial and following referenced object owners without expanding edit scope.

Missing bounds/types/definitions/targets remain unknown. Nonmatching contents,
extents or resolved references fail. Exit 0 requires all requested observations
to pass; counterexamples exit 1 and incomplete evidence exits 3. Supported ordinary
external data excludes weak/common/TLS storage, overlapping definitions and
unsupported debug/relocation forms. Individual comparisons are bounded to 16 MiB.
No source or claims are stamped, and no initializer executes during verification.
Passing says nothing about CRT constructors, final-link selection or later lifetime.

Bounded objects also participate in task checks and candidate decks. An evidence
anchor in task scope schedules its object's owner even without a current code
reference. Candidate rankings include bounded data failures, not just function
bytes. Restoration reports join fresh initialization results to exact source
interpretations, requiring the owner in every trial—not duplicate definitions in
each caller. Legacy address comments still need explicit source-binding migration;
this check does not silently treat them as complete coverage.

### System import identities

For ordinary PE32 imports, declare the emitted pointer symbol and intended
DLL/export beside the source that uses the SDK declaration:

```cpp
// RECOVERY_IMPORT: v1 __imp__Paint@4 DISPLAY.dll name:Paint
// RECOVERY_IMPORT: v1 __imp__Legacy@0 LEGACY.dll ordinal:7
```

These are examples, not bundled SDK bindings. Source owns the symbol-to-export
interpretation; the pinned image supplies the IAT slot address on each check.
No address list or decoration heuristic is needed. DLL comparison ignores case;
export names do not. Ordinals are explicit, never inferred from name hints.
Duplicate matching slots, malformed tables, missing identities, and
defined/common/weak symbols stay unverified. Bound images require their original
lookup table; loaded DLL addresses cannot substitute for identities.

Task checks and candidate decks use the same resolver and expose `import_bindings`.
Decks cannot change import interpretations while borrowing baseline bindings;
checkpoint those source decisions first. Outside-scope preservation detects
changed import annotations even when compiled code is unchanged.
This proves correspondence to an original import slot, not the candidate link's
chosen DLL/version, its ABI or runtime behavior. Delay imports and generated-table
import references remain unsupported; full acceptance remains separate.

### Fresh build checks

`recover check-build [--root <project>] [--summary]` uses project configuration,
not a compiler-specific package adapter. Add `"build": "recovery/build.json"`
to `project.json`. For example:

```json
{
  "schema_version": 1,
  "artifacts": "build/recovery/builds",
  "commands": [
    {"argv": ["cmake", "-S", "{root}", "-B", "{build}", "-G", "Ninja"], "timeout": 120},
    {"argv": ["cmake", "--build", "{build}"], "timeout": 1800}
  ],
  "outputs": [{"path": "application", "format": "file"}],
  "inputs": ["CMakeLists.txt"],
  "input_roots": ["src", "cmake"],
  "environment": ["CC", "CXX"]
}
```

Commands are normal argument arrays, not shell fragments or JSON adapters.
`{root}`, `{build}` and `{env:NAME}` substitutions are supported. `{build}` is a
new empty directory for every invocation, under the verified Git-ignored,
untracked `artifacts` path. It must be outside input roots. Output paths are
relative to it; existing outputs elsewhere cannot satisfy the check. All commands
must succeed and all expected outputs must be nonempty regular files. `file`
checks only artifact existence/content identity; `pe32-i386` additionally parses
the PE format. `msvc-map` parses an MSVC executable public-symbol map, refusing
malformed or duplicate public rows; pairing it with a PE for address validation
is a separate linked-code check. `msvc-pdb` validates an old SmallMSF container;
linked observations separately require its NB10/DBI identity and section headers
to match the exact PE. Supported linker fixups can independently place a selected
COFF association family; every code/data region, relocation and associated FPO
record must verify. Native linked-build code observation also supports a bounded
associated CodeView 5 procedure child: exact root-owned SECTION/SECREL bindings,
complete procedure and register/frame-local records, and correspondence to the
same build's physically bound PDB module. Procedure extent, flags, locals and
joint type graphs must agree; type numbering may differ, but names, properties,
cycles and shared references remain checked. Unknown record forms, type-server
remapping, folded procedures and missing module evidence refuse. This does not
ignore debug sections, establish complete debug-output equivalence, or qualify
outside-scope preservation. Unsupported or missing companions remain unknown. Compiler
flags and sidecar generation are project policy, not inferred by the library.
None validates application behavior or complete
source coverage.

Input roots are re-enumerated and hashed, including headers without extensions.
Explicit input files may be internal file aliases; their link spelling, target
and contents are pinned. Directory aliases and external links are refused.
List build scripts and other inputs outside those roots explicitly. Configured
environment values and the package implementation contribute to the fingerprint.
External compiler/SDK installation identity remains project responsibility.
Source, policy and Git mutations are refused and preserved, never rolled back.
These are trusted commands with change detection, not a security sandbox.

Results are disposable; the temporary build and outputs are cleaned up even on
failure. `--summary` omits command stdout/stderr, but keeps exit statuses and
artifact results. Exit 0 is a successful configured build, 1 a failed command,
3 unavailable evidence (including timeout or missing/invalid artifacts).

Add a `build` command to `driver.json` to run it alongside scoped checks every
round. The worker and reviewer receive `check.build_check`; `check.recovery_result`
preserves the scoped result and `check.result` combines both. Build failures drive
another edit/recheck round. A reviewed failed/unknown build stops with
`build_verification_required`; it cannot enter migration or full acceptance.
Full acceptance still requires independent structure/reference/obligation/runtime
verification. Build parallelism and isolated compiler resources are project policy.

### Compiler recipe preparation

`recover prepare [--root <project>] [--summary]` generates compiler recipes before
measurements. Set `"preparation": "recovery/prepare.json"` in `project.json`, using
the same command/input/environment schema as a fresh build. Declare exactly one
output with `format: "compilation-database"`; the configured generator must write
that file into its fresh `{build}` directory. For CMake, a project can declare its
configure command with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`. Other build systems
can supply their own commands without adding package code.

The destination is `mappings.compile_commands`. It must be under a Git-ignored,
untracked directory outside preparation input roots. All commands must succeed
and the fresh database must parse before atomic publication. Existing databases
cannot satisfy missing outputs. Entry file/directory identities are made absolute
without splitting or rewriting compiler argv. Original source and Git state are
preserved; changed inputs or a changed destination refuse publication. A previous
database is retained beside the generated context for interruption recovery.

Unlike `check-build`, preparation retains its fresh directory: recipes can depend
on generated headers and other files in it. These directories are disposable
compiler caches, including unsuccessful generation attempts, not recovery records.
After deleting generated context, prepare again before measuring. Measurements
fingerprint preparation inputs, environment and the whole generated artifact root;
old context directories currently over-invalidate too. No mtime or cached verdict
can substitute for changed generated headers. External compiler installation
identity and the correctness of trusted generator commands remain project policy.

`recover check` automatically prepares when configured, before starting its
measurement snapshot. Preparation failure stops that invocation without using an
old database. Standalone task checks and recording do not mutate compiler setup;
the coordinator runs preparation before them and after relevant input changes.
Never prepare concurrently with measurements. This replaces no source coverage,
build, layout, reference or runtime proof and cannot make a task collection-ready.

### Source-policy audits

`recover audits check <name> [--root <project>] [--summary]` runs natively, with
no project Python or compiler. Add `"audits": "recovery/audits.json"` to the
project configuration. That file has `schema_version: 1` and an `audits` object
keyed by project-chosen audit names. Each audit specifies:

- `ruleset: "cpp-recovery-v1"`: an opt-in set of conventional C++ decompiler
  smells, such as manual dispatch, lifecycle seams and address-derived names.
  Alternatively, `cpp-model-v1` locates raw member/inheritance access, naming
  questions, source-mapped function-address casts and textual orphan candidates.
  `cpp-storage-v1` locates conflicting original-global names and suspicious
  cast-wrapped class-pointer offsets, using the configured image pin and symbol
  format. It does not need the original executable.
- `unit_extensions`, `interface_extensions`: disjoint subsets of configured
  source suffixes. Some patterns apply only to units, avoiding declaration counts.
- `exclude_roots`: explicit paths within configured source roots. Pattern scans
  also omit the configured seed roots; no directory basename is globally special.
- `ceilings`: nonnegative integer policy limits for every selected rule/counter.
  The rule names are exported by `match_recovery.cpp_audit_rules.CPP_RECOVERY_V1`.
  Model rule names are exported by `match_recovery.cpp_model_audit.MODEL_RULES`.
  Storage rule names are exported by `match_recovery.cpp_storage_audit.STORAGE_RULES`.
- `counters`: additional project-owned scopes. Each has `name`, `path`, and
  `kind`: `text` also requires a regex `pattern`; `files` requires `extension`
  and boolean `recursive`; `lines` counts physical lines. These explicit scopes
  are observed even within seeds/exclusions. Missing scopes are errors, not zero.

Exit 0 means every policy ceiling equals its fresh count; growth or slack exits 1,
and unavailable/malformed inputs exit 3. Reports include full relative paths and
downward-only `lowerings` to apply with the source change. Commands never edit
source, policy, or claims, and never cache a verdict. Policy limits are deliberate
constraints, not a stored census; original bindings remain source-owned.

This version preserves a line-wise regex lens, not a C++ lexer: literals, inline
block comments and conditional compilation are not interpreted. Hits require
investigation; legitimate indirect calls and placement construction exist. A zero
count does not prove recovered structure, behavior or absence of disguised debt.
Input files, configuration and detector implementation are checked for changes
around each census. Directory/dangling symlinks in source scope are refused instead
of silently hiding files from enumeration. No project-specific ceiling is in the
package.

Model findings include one-based source locations, occurrence counts and relevant
names/addresses, not duplicated function bodies. `--in <relative-source-path>`
(repeatable) filters details; counts, policy results and reference analysis remain
whole-product. A reference outside the selected file can disprove an orphan hint.
Excluded/seed selections are identified explicitly. Orphan rule totals count
unique names; per-file findings count mentions. A short-claim documentation hint
describes historical source prose, not a fresh byte or behavioral measurement.

Storage conflicts count unique original addresses, while per-file findings count
mentions. Each conflict includes `related` source locations across the product,
even when `--in` filters the displayed findings. Compact `RECOVERY_OBJECT`
bindings and conventional address comments/pointer bindings participate together;
duplicate compact identities are refused. Conventional class-member comments are
not global bindings: an embedded first member can share its containing object's
address. The legacy spelling lens recognizes six-to-sixteen-digit hex addresses
and uppercase class-pointer type names, not arbitrary C++ scope, aliases or
element sizes. Pin changes invalidate a census; empty product coverage is refused.
These are investigation hints, not aliasing, layout or lifetime verification.

An optional `context` list in the audit policy selects audits for native task
context. Only authorized-file details are shown, with structural questions ahead
of naming volume and a 64-location preview per audit. Omitted counts and a detail
command are explicit; findings are never journaled as decisions. Malformed or
missing configured policy is an error, not an empty hint list. These hints do not
change task scope, approve a rewrite or replace historical restoration obligations.

### Verification APIs

The package contains workflow/policy, strict JSON decisions, disposable cache
utilities, process supervision and reusable comparison/format helpers. Optional
i386 PE/COFF helpers do not impose that format on the driver. Recomputable reports
are not persisted as authority; no SQLite database is used.

`match_recovery.source` owns the annotation grammar, reader and writer. Its
read/write APIs need no optional dependencies. Legacy i386 disassembly and
masked-byte measurement in `source.asm` and `source.calls`
require the `x86` extra. They do not replace strict relocation-target verification
or prove structure; use the strict comparison APIs for those obligations.
`codeview.class_layout` and `generated_binary.check_generated` provide reusable
CodeView 5 and MSVC PE/COFF table verification, with unsupported cases explicit.
No compiler installation, executable location or flag policy is selected there.

Generated-table reports include a capped, position-based `diagnostic`: emitted
slot symbols, candidate addends applied to explicit source bindings, original
words, original relocation presence and reverse source-symbol correlations.
The first 128 emitted entries are shown, with omissions counted; that limit
never narrows full-table verification. A declared extent prevents diagnostic
reads beyond it. An unknown extent stays unknown: words following the start may
belong to another object, and emitted length cannot supply the original bound.
Neither word agreement nor an old wrapper name automatically resolves a target.
COFF weak-external fallbacks expose their search mode but still require link
selection evidence. These observations reach task checks and variant decks without
a project-specific table probe. They are disposable diagnostics, not new source
annotations or acceptance claims. Independently bounded tables still require
complete four-byte slots and strict whole-table byte/reference agreement.

Function checks expose `diagnostic.emitted_references` even when the candidate's
size differs or its original companion extent is missing. This reveals emitted
callees, globals and source bindings without claiming their original counterparts
agree. Offsets refer to candidate sections; binding addresses exclude operand
addends. The first 128 references are shown, with omissions counted, independently
of the complete byte/reference proof. Local and weak symbols do not borrow external
bindings. These reports help distinguish structural mistakes, such as an accidental
recursive call instead of a base initializer, from instruction-spelling work.

`recover tasks check-binary <task> --image <image> --retained-diagnostics`
can investigate a mapped header-defined function absent from its canonical
product object. The single source mapping must supply its explicit `// body`
header. Supported ordinary, unambiguous function names are retained by taking
their addresses in private same-TU compiler overlays, never by exporting them or
changing optimization flags. Replica, repeated-emission and body-damage controls
must succeed under each flag trial. Unsupported access, overloads, virtual or
generated methods and unobservable controls remain unknown. A present mismatching
or malformed definition never takes this fallback.

The optional `retained_definitions` reports contain separate strict byte/reference
measurements and control/compilation provenance. They do **not** replace ordinary
`functions`, affect the check verdict, bank claims, or satisfy restoration or
acceptance. Target identities come from ordinary product objects and source
bindings; probe objects never enter product data, table, layout or preservation
checks and are never linked. Taking an address can affect code generation and
does not prove the method's inlined callers or final-link selection. This is an
investigation facility; acceptance of retained-definition evidence remains
unfinished. The default check incurs no retention probes.

All public recovery commands, including `inventory`, `partition`, `components`
and task commands, belong to this package. `inventory` and `partition` run
natively when `recovery/project.json` supplies a `source` section. No local Python
adapter or target registration is needed for those operations. For example:

```json
{
  "schema_version": 1,
  "source": {
    "image_pin": "original/identity",
    "address_space": "custom:offset",
    "roots": ["code", "hints"],
    "seed_roots": ["hints"],
    "include_paths": ["code"]
  }
}
```

The pin file contains exactly one `sha256 <64 lowercase hex digits>` line.
Source files use the ORIGINAL-comment grammar; other analysis formats still need
an importer. Common C/C++ suffixes are supported, including `.cc` and `.hpp`;
`extensions` and `interface_extensions` can narrow/customize the scan. All roots
are explicit worktree-relative paths, seed directories are configured rather than
guessed by name, and missing roots/outside symlinks are refused. `symbol_format`
defaults to `none`; opt into `msvc` for decorated-class/lifetime search hints.

Class grouping follows the current source-mapped member symbol, including supported
public secondary-base adjustors. An old anonymous catalogue label therefore does
not strand a recovered method outside its class group. Catalogue labels remain a
fallback for free redirect wrappers and seed-only evidence; a seed's emitted
scaffold class never establishes current ownership. Each entity reports the
grouping basis and alternative catalogue/seed owner hints. Alternatives remain
questions for review, not permission to discard historical interpretations or
matching obligations. Conflicting current member owners, multiple current source
homes and incompatible original extents still produce conflicts. No class group
proves hierarchy, lifetime, compilation success or a historical TU boundary.
Hints are not original-design proof. Include impact remains conservative.

With this source configuration, `tasks create <task> --group <group> --target
<target> --author <author> --intent <intent> --evidence <evidence>` and `tasks
expand` also run without adapter code. Target may instead come from
`recovery/driver.json`. Creation requires committed source/configuration and
defaults to `HEAD`; optional `--base` takes a full ancestor revision whose source
and source configuration are unchanged. It does not guess an upstream branch or
replace an existing task baseline. `--edit-file` explicitly adds editable scope.
Task discovery, resolution and status operate directly on versioned decisions,
even without a source configuration. `tasks obligations` and `tasks context`
also run natively with configured source. Context refuses concurrent source or
decision changes and incorporates historical restoration requirements.

`tasks check-restoration <task> --image <image>` re-derives those historical
requirements and runs the native joint binary/layout check. It returns current
measurements keyed by obligation identity, their input fingerprints, and explicit
remaining requirements. It neither stamps source nor saves a verdict ledger.
Removed claims remain protected. Bytes and references must use one complete
compiler trial; known differences with unresolved targets remain unknown, not an
admissible matching failure. Strict exact agreement can also discharge a historical
instruction-shape requirement; non-exact shape remains unknown.

`structural_requirements` joins current member, base and generated-table checks
to exact source interpretation identities, retaining previous interpretations as
explicit review needs. A type/name/offset/original-anchor change cannot borrow a
different interpretation's measurement. Coverage uses every requested TU/trial;
missing definitions, missing extents and unsupported checks stay unknown.
Extra compiled dependency witnesses cannot hide a disagreement either. Read
`counts`, `trials` and `next_action` to distinguish repair work, missing evidence,
and historical supersession/retraction decisions. These reports are recomputed
from source/history and the same fresh candidate check, not another ledger or
an additional compilation pass.

Generated vtables need not be emitted by every checked translation unit.
`generated_coverage` records non-emission from each successfully compiled COFF
symbol table; an ordinary undefined external is a reference, not a definition.
`generated_units` retains all requested owner/dependency TUs, including failures;
an unsuccessful dependency compile cannot disappear from the coverage grid.
Every emitted copy still needs independent bounded content/reference verification,
and each configured flag trial needs at least one actual definition witness.
Missing compilations, absent coverage, all-absent tables and weak/common/static
symbol cases remain unknown. Coverage-only trials cannot prove table contents
or full hierarchy/lifetime. This uses the already compiled objects and neither
narrows the requested TU grid nor creates a stored source fact.

A measured counterexample to a current declaration marks its `declared_structure`
obligation failed. Source member/base/table/object annotations generate this
bounded dimension, not a claim of complete hierarchy or lifetime. Passing all
current requirements discharges only that dimension; historical replacements
also need explicit independent, snapshot-bound review. Retracting every old
interpretation without a measurable current model stays unknown. Explicit
`structure` and `runtime` obligations remain distinct and cannot be discharged
by these bounded checks or waived as matching regressions. Obligation inventories
are rederived from source/history, not converted by rewriting source annotations
or a second ledger. Historical measurements are not reused and no review
disposition is inferred from a passing replacement. `candidate` contains
the complete scoped diagnostics; `--summary` omits those, fingerprint maps and
repeated requirement trials while retaining identities, counts and next actions.

This command supplies measurement inputs for the native migration verifier,
not an admission report itself. Admission additionally requires fresh independent
review, build, structural prerequisites, references and outside-scope preservation.
Migration-check reports must now include `references` with a passing result, even
when historical source recorded only byte claims. Missing/failed/unknown references
cannot be waived by a matching-regression permission.

`tasks check-preservation <task> [--unit path ...] [--summary]` adds native
baseline/current compiled preservation evidence, without a project checker script
or another ledger. It reads the task's immutable Git baseline and uses the same
current measurement/compiler interaction on both source versions (explicit
versioned build inputs may change generated recipes). Every configured
non-seed TU is included by default, even units with no function annotations and
dependency units. A diagnostic unit subset leaves the overall result unknown.
No original executable argument is needed: this comparison does not claim that
the baseline or candidate matches the original.

Only exclusively owned function COMDATs selected by source origin identities and
their associative children may vary. All remaining code/data, BSS extents, link
directives, weak/common symbols, public aliases and exact symbolic relocations
are compared. Scoped entry renames use original entity identity; local relocation
destinations are never reduced to a generic placeholder. Ordinary undefined
callees reached by relocations only from scoped code are not outside definitions.
Otherwise-unreferenced externals remain visible: they can express implicit linker
requirements even without an instruction relocation. A retained code/data edge
or weak-external fallback also keeps its target protected; unknown external types
and auxiliary metadata are not dropped. Removing the last scoped function from
a TU does not change these rules. Bounded object interpretations use their
explicit evidence anchor for annotation scope, consistently with task data checks
and restoration; unbounded v1 objects still use their original storage identity.
This does not exclude independent object bytes from compiled preservation or waive
historical interpretations. Reanchoring an outside interpretation cannot erase
its previous binding. Outside function
bindings/claims and object/member/base/generated/component annotations also stay
protected; a comment-only address change cannot be hidden by unchanged objects.

Header-defined task functions can also own emitted copies in other TUs. Keep one
canonical original binding and its explicit `// body path/to/header.h` fact; the
header must be authorized by the task. No duplicate original markers or separate
copy ledger is needed. For each source version and flag set, the verifier requires
a fresh canonical replica, identical complete function/associative-section evidence
in the other TU, and a controlled body replacement that changes both definitions.
Only witnessed, exclusively owned COMDATs gain scope. The ordinary residual check
still protects outside callers (including inlined code in those callers), data,
type records, public aliases and linker directives. A new export is not waived.
Previously emitted but unbound code also retains its baseline protection.

Body probes support unambiguous ordinary definitions, including in-class and
qualified definitions. Overloads, complex declarators, unsupported lexical forms,
missing/unchanged probes, differing definitions, and dependencies on TU-local or
weak storage outside the owned section family remain unknown. Symbol spelling or
byte similarity alone cannot supply ownership. `inline_ownership` in each TU's
report records the fresh evidence/refusal; it is not reusable acceptance authority.
Probe compiles are counted and cached only within that invocation/source-version/
flag trial. Diagnostic mode still cannot establish preservation.

Each TU needs a matching direct/current-overlay control. Each changed file also
needs a compiler-observed unique `#error` control before substituted baseline
bytes can support a pass. Generic compiler failure is insufficient. Reports carry
digests, section identities, coverage and failures, never object bytes. Inputs,
task decisions, compiler policy and Git state are rechecked; no source, claim or
baseline is edited. Temporary copies are disposed through the same compiler overlay
path as candidate decks.

Prepared projects can declare optional `versioned_inputs` in their preparation
configuration, for example `"versioned_inputs": ["CMakeLists.txt"]`. These exact
regular files are also fingerprinted build inputs. Their committed baseline bytes
travel with baseline source; all undeclared compiler/provider implementation and
policy remains current. Files must exist in both versions, outside source and
artifact roots. No historical compiler command is inferred from a filename.

Added/removed production TUs require independently regenerated recipe inventories
that agree with each version's complete source coverage. Duplicate, missing or
aliased recipes refuse. A missing TU is an explicit absence, never an empty
synthetic object or a failed-compile inference. The present version needs fresh
compilation and replica controls; its strict non-scoped residual is compared with
absence. Types, directives, empty section metadata, private helpers, storage and
implicit linker requirements are not guessed away as harmless scaffolding. This
can conservatively fail a legitimate TU split; it is not full-link equivalence.
The complete baseline's emitted unowned names remain protected when a new TU adds
an annotation. Compiler `#error` controls run only against units present in the
relevant source version. Versioned build files use fresh generation/fingerprints,
not a claim that a C++ compiler observed their contents. Reports list changed
build inputs, recipe membership and preparation failures separately. Compiler
materialization without configured fresh preparation still cannot prove TU absence.

For inexpensive investigation before the full sweep, run `recover tasks
check-preservation <task> --unit code/model.cc --diagnostic --summary`. This
compiles the selected current TU directly, checks its unchanged replica, and
compiles the historical tree under each configured flag set. It deliberately
omits per-file damage probes. Both the command and every unit result remain
`unknown` (exit 3), even when `compiled_comparison` reports agreement. This mode
cannot establish preservation or admit a migration; omitted controls and units
remain explicit in the report.

An added-TU diagnostic checks the current object and unchanged replica without
compiling every baseline TU. It shows the raw present-object residual with no
scope subtraction; even task-mapped functions remain visible. Its row explicitly
marks baseline definition coverage and scope subtraction omitted, stays `unknown`,
and supplies no provisional `compiled_comparison` pass. Both source/recipe
inventories are still freshly generated. Strict checks of an added TU retain
complete baseline definition protection; removed-only checks and filters that
exclude added TUs do not need that global scan.

Full reports name baseline/current section symbols, changed dimensions and exact
removed/added relocation targets, including local offsets. Symbol-table changes
carry auxiliary fingerprints, not raw metadata or object bytes. No fuzzy pairing
of renamed symbols or compiler-local labels changes the verdict. Source annotations
provide navigation hints for function, object, generated-table and dependency
identities; all ambiguous candidates are retained. These hints do not prove which
source declaration produced an emitted symbol and do not authorize scope expansion.
Summaries retain changed-section/symbol counts, affected source bindings and
unbound public/weak linker symbols. Compiler-generated local symbols are not new
application recovery targets merely because they lack a source annotation.

Numeric COFF exception labels in explicitly associative sections are compared by
their owner, section, exact offset, type and storage, not their compiler-assigned
decimal spelling. Only the supported `$L` label and `$T` static forms without
auxiliary records use this normalization. Relocation sites, kinds, addend bytes,
target locations, contents and ownership remain protected. Distinct numeric labels
at one location retain every exact spelling and edge instead of being merged.
Duplicate raw labels and symbols impersonating the internal identity still refuse.
Numeric `$L` code labels in a non-associative root also qualify when one external
function at offset zero exclusively owns that code COMDAT. All other symbol tuples
remain part of the section identity; an owner name alone never replaces them.
Static-only, ambiguous or non-COMDAT ownership and root `$T` names remain strict.
Unowned independent-section labels, public names, opaque auxiliary metadata and
unnamed-namespace renames are not preservation aliases. This is compiler-output
comparison only, not original-image target resolution or recovery acceptance.

Current limits are explicit: source interpretation must agree across the baseline
and current tree. Changed header/seed file sets use compiler protocol v5 complete
trees: current-only files are absent from the baseline, historical-only files are
recreated only there. Deleted-file damage controls run against that historical tree;
added-file controls run against the current tree. Unobserved inputs, including
unused seeds, remain coverage gaps. Added/deleted/renamed product TUs still require
explicit recipe and build-coverage mapping; those gaps are reported before compiling.
It uses current compiler recipes, not historical build reconstruction, and does
not prove full-link selection, original references, runtime or lifecycle behavior.
`result:pass` means the covered compiled residuals and outside annotations agree;
`scope_complete`/`ready_for_collection` stay false and `outside_scope_verification`
stays unknown. The full migration/acceptance verifier still needs those additional
dimensions. Do not reset task history or substitute this diagnostic for restoration.

History uses each commit's source roots, suffixes, seed classifications and pin
location, including merged side histories. Moving/deleting a claim or treating
its old home as seed does not withdraw an earlier obligation. Member/base/table
parsers use the configured address space rather than assuming PE or 32-bit
addresses; format-specific binary checks still enforce their supported formats.

For tasks whose baseline predates source configuration, an optional
`source_history` section in `recovery/project.json` must explicitly supply
`before_configuration` (the historical source configuration object) and a
nonempty `evidence` array explaining that interpretation. New projects do not
need this section. It cannot excuse later removal of established configuration,
changed image pins or changed address spaces. Reports and historical blobs are
derived in memory; no inventory is persisted as authority. Old interpretation
identities and restoration requirements survive source/configuration migrations.

Public source operations use native `source` configuration. The old project-wide
`adapter` dispatch key is removed; it is not a route for overriding those commands.
The driver still accepts configured worker, reviewer and verification commands
through their explicit contracts. Generic source annotation readers stay in the
core. Compiler commands, environment
variables, flag choices and optional interaction scripts belong to the project.
The example project keeps its VC6/Wine interaction in `recovery/environment/`;
there is no separate compiler package or per-compiler CLI.

## Compiler interaction and dependency checks

With source configuration, `components list`, `components check-source` and
`components check-binary` are native commands; no project dispatcher is needed.
Manifests remain companion source annotations. They must match the configured
image pin/address space; seed manifests cannot override dependency ownership.
Source integrity needs the pinned upstream archive and is not binary proof.
The current strict binary backend supports i386 PE/COFF; unsupported formats or
unresolved bindings remain unmeasurable, not exemptions from recovery.

### Precompiled toolchain dependencies

`ToolchainManifest` represents an existing runtime/third-party library, separately
from `VendorManifest`'s upstream-source workflow. `render_component` emits the same
companion-source comment envelope with `kind: "toolchain_component"` and these
load-bearing fields:

- `component`: identity, `ownership` (`runtime` or `third_party`), `delivery:
  "toolchain"`, version and provenance evidence;
- `archive_sha256`: the selected library artifact's identity, not a measurement;
- `license_evidence`: provenance/license reference; no library redistribution is
  implied or performed;
- `bindings`: original image/spans, emitted symbol, `unit` naming the library
  member, and independent binding/boundary evidence, as for upstream components.

The companion uses one `image` pin/address space and hexadecimal binding spans.
It contains no archive location, extracted objects, derived per-member hashes,
compiler flags or current verdicts. Supply the installed library location locally:

```sh
uv run recover components check-binary runtime --image original.exe --archive /local/compiler/runtime.lib
```

This command requires no compiler invocation or project adapter. The current
backend reads Microsoft COFF archive records in memory, verifies the whole archive
pin, then checks each declared function's extent, bytes and references using the
existing strict comparator. Member identities use forward slashes, including
Windows-produced archive names; duplicate/colliding names refuse selection.
Linker indexes are not used as evidence of selected members. No paths are extracted.
Thin/BSD archives, unsupported name encodings and non-supported COFF function
layouts do not pass. Format details follow the
[Microsoft archive specification](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#archive-library-file-format).

Both upstream-source and precompiled binary checks use ordinary source
`RECOVERY_IMPORT` declarations to resolve unique original DLL/export slots.
The same resolver used for application code refuses ambiguous, weak, common or
defined import symbols and checks the actual relocated operand. The native CLI
checks the complete configured source epoch, including added or removed import
files; seed declarations do not supply identities. The report's `import_bindings`
separates original slot identity from still-unknown candidate link selection.
No original IAT addresses need to be stored in source.

Missing members, undeclared imports/data/function bindings and unsupported code layouts stay
unknown. A wrong archive pin or observed byte/boundary/reference disagreement
fails. Source `ORIGINAL` tokens are neither generated nor banked. Partition and
planning classify these functions as dependencies and navigate to the companion;
the manifest does not exempt any C++ file from source checks. Preservation retains
the full binding decision. `check-source` is inapplicable to precompiled inputs.

This verifies only the selected bounded library code. It does not prove the final
linker selected those members, identify every library function, or establish ABI,
runtime behavior or application acceptance. Reports are disposable and are not
automatically installed as application relocation targets. Those integration
proofs remain separate work.

### Fresh linked-library code

```sh
uv run recover components check-link runtime --archive /local/compiler/runtime.lib --library RUNTIME
```

For precompiled components, this native command runs the configured build in a
new retained ignored directory. Declare exactly one `pe32-i386` output and one
`msvc-map` output in the build configuration. `--library` supplies the explicit
Lib:Object label used by that linker; it is not an archive path or an inferred
dependency identity. No original image or cached verdict is consumed by this
check. Source annotations still supply the archive pin, member identities and
independently justified original function lengths.

The package validates map preferred base, entry point and public coordinates
against the actual PE, preserves library/member labels and refuses duplicate
publics or ambiguous member basenames. Each supported external single-COMDAT
function is compared at its linked address with the pinned member, including
every operand and PE base relocation. A public's address alone cannot replace a
source-declared DLL/export identity for an imported slot. Missing targets,
weak/local-data symbols and associated code fragments remain unknown.

Source, imports, build policy, implementation, Git state, archive and produced
artifacts are checked for changes across the operation. Build or code disagreement
fails; incomplete evidence exits 3. Outputs remain disposable ignored build
context. The API `verify_linked_toolchain` accepts in-memory artifacts for tests
and integrations but makes no freshness claim; `check_toolchain_link` owns the
fresh-build sequence used by the CLI.

An optional single `msvc-link-log` build output supplies the compiler's full
MSVC `/VERBOSE` stdout (not `/VERBOSE:LIB`). The environment captures that
artifact; the package never chooses a linker invocation. The native parser
requires complete first/final pass framing, separates nested export-library
generation, and preserves every final-pass input label, including repetitions.
When configured, each bound library function also needs one unambiguous matching
member label in that final input list. Missing/ambiguous observations remain
unknown; a map row cannot fill them in. The log is fingerprinted and checked
alongside the fresh PE/map, not consumed from a previous run.

`link_inputs` reports these observations and searched-library path spellings.
They are not complete input-file identities: basenames may omit directories,
import-member labels repeat, and sections may be discarded or folded after
member selection. Other verbose diagnostics are uninterpreted. File-content
fingerprints at the compiler boundary and common-allocation proof remain required;
the transcript alone cannot admit a dependency or export an application target.

A paired `link-input-files` build output can supply compiler-boundary file
observations. Its schema-2 JSON contains `link_log_sha256`, absolute normalized
host `cwd` and `tool` paths, expanded effective `arguments`, `explicit_inputs`, `files`,
`library_searches`, `generated_exports`, and `capture_gaps`. Each file has `path`, `before`, and
`after`; each non-null observation is `{ "sha256": "<digest>", "size": 123 }`.
Each search has its exact diagnostic `spelling` and resolved host `path` (null
when unresolved). Keep the full ordered search sequence, including nested
export-library searches. The environment captures before and after the actual
link; missing observations must stay null, not be filled from post-link files.
The boundary must include implicit option sources in those effective arguments
or refuse them; capturing only argv while an environment variable supplies
additional linker policy is incomplete evidence.

Each `generated_exports` row contains `path`, boolean `existed_before`, and
`after` (the output content observation or null). This specifically describes
MSVC export-object generation: do not label arbitrary post-link discoveries as
generated inputs. An explicitly supplied export object instead needs ordinary
pre/post file observations. Schema 1 caches must be regenerated, not migrated by
inventing absence observations.

The package validates this format, binds it to the log digest/search sequence,
and rereads physical files during fresh component verification. The selected
archive must be the uniquely observed physical path supplied by `--archive`,
with the source-pinned contents stable before/after linking and verification.
Symlinks, changed files, incomplete capture and ambiguous paths cannot pass.
Unused search candidates are reported separately from required observed inputs.
`archive_file_identity` and `link_files` expose this bounded evidence. Compiler
path resolution/capture is trusted interaction, not filesystem attestation;
the file-identity check alone does not prove selected-member identity or common allocations.
A saved artifact never replaces the fresh-build CLI sequence.

`link_input_coverage` now inventories the explicit objects, every member of every
searched archive, and uniquely observed export objects absent before this link.
It checks every final-pass label against that conservative candidate population.
Repeated/abbreviated member labels retain all candidates; unsupported records,
missing file observations and ambiguous physical object paths keep coverage
unknown. This includes archive records not selected into retained sections, so
their declarations cannot silently disappear from later common-storage analysis.
Export-object records are parsed as COFF and rechecked for content changes.
The report includes common requests and format gaps, not selected allocation
extents. A parsing/coverage pass is separate from the bounded linked-code result
and never exports application targets or proves full dependency acceptance.

For common symbols declared in the source-bound library members, the fresh
component check also reports `common_allocations`. The package rereads that
complete input population, including unselected members, to require uniform
size requests and rule out potential strong/absolute/import/weak overrides.
At least one request needs a participating explicit object or unique final
archive candidate. Actual map coordinates must select data-only `<common>`
storage; its full requested span must be readable/writable, non-executable,
zero-initialized, and free of other public starts within the span and loader fixups. Sizes are
never inferred from adjacent symbols. Unknown linker options/directives, aliases,
conflicting sizes and unsupported records remain unverified. This intentionally
does not apply the largest-request rule to a superset containing unselected
members. Ordinary non-aliasing exports (including DATA), library searches and
section merges are handled under the complete-population and actual-PE checks.
The allocation result is separate from linked-code correspondence: a storage
counterexample fails the command, but unsupported storage does not erase bounded
code evidence. Original correspondence, C++ type/lifetime and runtime ownership
still need separate proof; no application binding or overall storage acceptance
is published by this bounded check.

`compare_common_builds` compares explicitly requested common identities in two
live native application contexts, with mandatory entry/final checks of both
epochs. An added uniform request and moved address can preserve the same proved
size and zero initial contents; a changed selected size fails. The guarded
`LinkedBuild.common` path reuses parsed native inputs without accepting a saved
inventory or reparsing the population for each query. Returned diagnostics do
not expose mutable parsed state. Neither API grants residual coverage: an earlier
allocation can extend into a queried span, and private aliases are not ruled out
by checking public starts alone. Allocation isolation, guard writes, other linked
storage, initializer effects and runtime behavior remain separate obligations.
No task permission, COFF declaration exemption or original mapping is published.

The live `LinkedBuild.data_section(owner, number)` observer independently places
an ordinary data section using its exact selected physical object, declared COFF
extent and consistent owner-qualified map/uniquely bound embedded-debug anchors.
Nonzero symbol offsets locate the section base; neighboring labels never supply
its size. Uninitialized sections retain their declared zero-fill extent. Ambiguous
cross-section names, conflicting public/static/debug evidence, malformed relocation
writes, unsupported special/COMDAT forms and incompatible PE placement refuse.
This is a whole-section envelope, not an individual C++ object's extent or a
contents/reference proof. The native context guards all input/output/parsed state
before use and on normal or exceptional exit.

When ordinary private data lacks a same-owner static map label, this observer
also follows exact COFF references from retained same-owner callers. It verifies
each complete linked caller region and associated family before consuming its
image-bound PDB symbol-base fixups. The selected physical input, target symbol
index/section/offset and referring relocation must all agree. All independently
verified anchors must imply the same section base; inconsistent verified callers
or map/debug anchors refuse placement. Failed or unavailable callers remain
explicit gaps and cannot supply anchors. An independently verified caller can
establish bounded placement without proving all callers: caller coverage, target
contents and allocation isolation remain unknown. No address is inferred from
equal bytes, contribution sizes or a different owner's retained inline copy.
Potentially folded callers remain gaps until complete original-owner group
evidence is composed; the map's winning owner cannot stand in for another member.
Per-owner batch memoization is guarded by the live native epoch; returned reports
cannot mutate cached anchors. This does not grant residual coverage or acceptance.

The same observer supports one-hop private-data references from an ordinary data
section already placed by independent map/debug or verified code evidence. It
checks the entire referring section's bytes and relocations before publishing
anchors from its image-bound PDB fixups. Symbol bases are not inferred from linked
operands, and effective addends must stay inside the target section. Data-derived
placement cannot seed another hop or bootstrap an unanchored cycle. Corrupted or
unverified referring sections supply no anchors; target contents are checked
separately. The per-owner native batch refuses more than 4,096 candidate referring
sections. Selection completeness, allocation isolation and acceptance remain
separate obligations.

`LinkedBuild.storage_census()` derives every section from the complete physical
candidate population. It includes explicit objects missing final-pass labels,
every candidate in ambiguous archive families and unselected archive members.
Physical path/member/occurrence and hash identify records; basenames cannot
silently select one. Ordinary selected data uses the same independent envelope
observer. Common requests, imports, weak/absolute declarations, directives,
COMDAT, debug/discardable and other special sections remain visible. Zero-size
sections with storage symbols or relocation writes are explicit gaps. A section
resource bound refuses rather than returning a successful truncated census.
Enumeration success does not prove retention, discard, placement or isolation;
code and empty sections receive no implicit linked-effect exclusion. Shared
batch indices avoid rebuilding the debug/declaration index for each section,
while native entry/final physical and parsed-state guards remain mandatory.

`LinkedBuild.common_intersections(symbols)` now uses that full census by default.
An explicit optional `sections` subset still contains only `owner`/`section`
identities, never caller-supplied bounds. It can expose a
preceding section extending into a common allocation even when every named label
starts before that allocation. A positive overlap refutes isolation; an empty
intersection list remains unknown: even full candidate enumeration does not
establish retained-section or linker-generated-storage completeness. Missing sections stay
explicit gaps. Neither result authorizes projection or acceptance.

The optional PDB sidecar now supplies a separate native contribution inventory.
`LinkedBuild.contributions()` binds exact NB10/SmallMSF/V60 module and SC records
to this image through the existing PDB fixup/section-header verification. It
retains every table entry, including duplicate or intersecting intervals, exact
characteristics and CRCs. Module first contributions are not substituted for
the full table; nil first entries cannot erase a module's other contributions.
The reader bounds names, signed extents, module indices and stream references,
refuses unsupported versions/EC/TSM, and never opens a module path. Historical
pointer fields and alignment padding are not interpreted as offsets. Module
symbol/global-reference payload interpretation is not claimed by this inventory.
V60 SC records lack original COFF section indices: matching CRCs, sizes or module
labels cannot choose physical ownership or prove that missing rows were discarded.

`LinkedBuild.module_bindings()` joins module labels to the current build's
command operands, observed library spellings and full physical candidate
inventory, then independently checks native final-pass object selection. An
ordinary object must match an exact complete input operand; archive labels must
identify both the observed library path and the full member name. Basenames,
matching hashes or debug labels cannot choose an ambiguous occurrence. Fresh
generated exports use their existing native generation evidence. Duplicate
module-to-input bindings, ambiguous import families and the synthetic linker
module remain unknown. A verified compiled object also carries its source/output
membership. The native linked epoch now pins application command reports as well
as physical artifacts, with mandatory final guards on success and failure.
These are physical module bindings only: they do not identify a V60 contribution's
original COFF section, prove retention or authorize common residual coverage.

`LinkedBuild.ordinary_contributions()` connects that physical module binding to
the complete storage census and independently anchored ordinary section envelopes.
An envelope must intersect exactly one PDB contribution with the same physical
module, PE section, start and declared size; matching sizes or CRCs cannot place
an unanchored section. Duplicate/intersecting entries and multiple sections sharing
one contribution refuse correspondence. Unmatched contributions and every physical
candidate section remain visible, including unsupported or unplaced storage.
Contradictory kind, access and alignment refuse; all raw COFF/PDB characteristics
remain separate. A coordinate match does not certify unexplained extra flag bits,
contents, references, isolation or retention completeness. The default native
`common_intersections` path now includes this join and shares its census rather
than enumerating the population again. Missing command/module evidence remains
an explicit gap while independent section overlap counterexamples remain usable.
Explicit section subsets do not receive full-census or module coverage implicitly.

`LinkedBuild.common_contributions(symbols)` cross-checks freshly verified common
extents against that full contribution table. A unique, exact linker-labelled
interval can establish coordinate correspondence only; all intersecting records
remain visible. A broader contribution can aggregate the same storage and is not
automatically a distinct overlapping allocation. With a PDB, the default
`common_intersections` path includes this evidence automatically and preserves
unsupported contribution metadata as an explicit gap. No characteristic bits
are normalized or records excluded. Native entry/final guards apply to both
observers, and returned diagnostics cannot mutate their parsed inputs. Isolation,
retained-storage completeness, physical module binding and common residual
permissions remain unproved; neither API grants acceptance or a coverage waiver.

Passing means bounded **candidate linked-code correspondence**, not original
equivalence or dependency admission. A map's common/public data address does not
prove storage size, contents, initialization or ownership. Owner labels and
matching bytes also do not prove the linker read a particular archive path.
Original-image bindings, full dependency closure, archive selection provenance,
ABI, structure and runtime checks remain separate. No targets or claims are
published into application acceptance, and `ready_for_collection` stays false.

The linked-component report also includes `archive_inventory`: format coverage,
unsupported member records and common allocation requests from the entire
supplied pinned archive. This is a separate **parsing** result, not an additional
code or selection claim. An unsupported unselected member can leave inventory
unknown even when the bounded linked functions agree. Common allocations are
requests, not selected object extents, and no inventory address bindings are
exported to application code.

`coff_inventory.inventory_object` and `inventory_archive` expose full declaration
records to compiler-boundary integrations. Regular COFF includes i386 and
unspecified-machine metadata, traditional import descriptors (including their
PE32 optional header) and explicit weak fallback edges. Documented version-zero
i386 short imports preserve code/data/const kind, DLL identity, ordinal/hint and
name-transformation mode. Repeated archive member names retain separate
occurrences; no linker index or last-record-wins selection is used. Unsupported
records prevent complete inventory, and linker directives remain visible but
uninterpreted. The `Coff` function verifier still requires i386 without an
optional header and retains all of its COMDAT ownership checks; these broader
record forms cannot become function proof. Format details follow the
[Microsoft import-library specification](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#import-library-format).

This inventories only supplied bytes. Complete fresh link-input enumeration,
implicit library resolution, selected common allocation/alias semantics and
runtime storage verification remain separate work. A diagnostic system-call
trace is not automatically a complete or fingerprinted input manifest.

### Source compiler interaction

The project supplies compiler interaction through `recovery/project.json`:

```json
"compiler": {
  "command": {"argv": ["python", "recovery/compiler.py"], "timeout": 300},
  "inputs": ["recovery/compiler.py"],
  "environment": ["PROJECT_COMPILER_ROOT"]
}
```

The command receives one JSON object on stdin:
`{"schema_version":2,"profile":"default","source":"vendor/codec.c","arguments":[],"flags":["-O2"]}`.
Paths are project-relative and the working directory is the project root.
Arguments and flags are arrays, not shell fragments; their meaning belongs to
the local compiler implementation. It returns only
`{"object_base64":"<encoded object bytes>"}` with exit 0, or an error diagnostic
with nonzero exit. Empty/malformed objects, compiler failure and timeouts cannot
supply evidence. Format parsing and comparison happen in the library afterwards.
Protocol v2 adds an explicit project-defined compilation profile. The local
command can resolve an application build entry or choose a dependency invocation;
there is no profile registry or compiler-command parser to modify in the library.

Optional project-level `toolchain` configuration uses the same `command`, `inputs`
and `environment` shape to identify external tools and SDKs. Its command receives
`{"schema_version":1,"operation":"toolchain_inputs"}` and returns, for example,
`{"schema_version":1,"files":{"compiler":"/opt/sdk/bin/compiler"},"directories":{"sdk":"/opt/sdk/include"}}`.
Names are unique normalized relative labels; locations are canonical absolute
paths. The package hashes actual contents and re-enumerates directories, including
new/shadowing files, executable permission changes and internal filename/directory
aliases. Directory enumeration errors refuse observation rather than omitting
inaccessible inputs. Aliases cannot leave their declared tree; directory aliases are recorded,
not recursively followed. Same-size edits with restored modification times still
change the identity. Environment values and external locations are hashed in
observations rather than printed. The project command is trusted local code,
not a sandbox or authority to install/download a compiler.

Configured `Compiler` calls, measurement epochs, preparation and fresh builds
include this identity in their existing before/after checks. Unconfigured projects
report no external identity (`null`), not complete toolchain verification. Nothing
is added to source mappings or a second ledger. On coherent POSIX filesystems,
a bounded process-local cache reuses content hashes only while two fresh inventories
agree on paths, device/inode, mode, size, mtime and ctime. Any change causes fresh
content reads. Discovery commands and their policy are always rechecked; compiler
objects and verdicts are never cached here. Forked children start with an empty
cache. `MATCH_RECOVERY_TOOLCHAIN_CACHE=off` forces fresh reads and must produce the
same identities; use it on filesystems without reliable metadata coherency.
Non-POSIX hosts always use fresh reads, since ctime need not track content changes.
Even cached observations add inventory/discovery overhead for large SDKs.
The descriptor must enumerate relevant inputs: it does not prove all dynamic
loader, operating-system or remote-execution dependencies were captured. Target
execution, target ABI/backend support and compiler installation remain separate
capabilities; this interface does not make unsupported architectures measurable.

The historical `source.asm` listing APIs use this same callable boundary:
`compiled_asm`, `compare_source` and `compare_record` require `compiler=` (a
`Compiler` instance or a callable receiving source path, argument tuple and flag
tuple). There is no implicit compiler invocation or `compile_unit` fallback.
An optional `retry_compile(diagnostic)` callable can stop flag trials after a
compiler-specific parse failure; absent that policy, all alternatives are tried.
These APIs still provide relocation-masked diagnostics, not strict reference or
structural acceptance. They neither choose project recipes nor stamp source.
Flag trials may be native argument arrays or legacy strings. Native array tokens
are passed unchanged; quoted display labels are never reparsed into arguments.

`source.asm.compare_unit(records, image, source, arguments, flag_sets,
compiler=...)` measures selected mappings from one host together, using that same
per-function comparison. It compiles once per requested flag configuration and
reuses the object (or failed attempt) only within the call. Different functions
can select different successful trials; missing symbols do not discard measured
siblings. Its `UnitComparison` contains results in selection order plus actual
compiler/reuse counts. Duplicate selections, mixed hosts and changed candidate
or marker files are refused. The caller still owns whole-project/header/compiler
freshness and publication; this is neither a persistent cache nor acceptance.

`source.recording.record_claims` supplies the guarded historical publication
path. It invokes the supplied measurement callback after capturing selected
source, requires one unchanged record/result per selection, and calls the
coordinator's complete-input validator before measurement and publication.
Only newly proved byte obligations are added; unresolved or nonmatching results
cannot demote existing claims. Historical notes are preserved, not changed into
run status. This legacy measurement API requires the `x86` extra and is not a
strict-reference or task-acceptance API.

`recover record <addresses-or-symbols> --image <image> [--root <project>]
[--in <exact-relative-source-path>]` owns production selection, measurement and
publication through that API. It reads source roots and TU suffixes from project
configuration, derives historical identity coverage, and uses the configured
compiler profile/flag arrays through the JSON compiler client. It does not import
project Python modules, infer recipes, borrow another TU's command, configure a
build or accept precomputed verdicts. Default empty compiler arguments request the
local compiler's recipe; explicit argv can be supplied with `--arguments <relative
JSON-file>`. All arguments remain tokens, not a shell command string.

Functions in one TU share invocation-local objects and failures; `--jobs N` permits
multiple TUs concurrently, defaulting to 1. The project/coordinator must provide
safe compiler capacity. Header-only mappings need an explicit compilation host
before recording; the command will not pretend a header was a configured TU.
Source/build/compiler inputs, original image, argument-file bytes, environment and
HEAD are revalidated before publication. Staging must be ignored, untracked and
outside measured input roots. The JSON output reports selected/measured/updated
subjects, retained regressions, unresolved subjects and compile/reuse counts.
Exit 0 means all selected bodies freshly matched, 1 a measured miss, and 3
incomplete/refused evidence. Independently exact siblings may be published on a
nonzero run; existing obligations are never automatically weakened. This backend
records historical relocation-masked byte obligations, not strict target, structural,
task or full-project acceptance. No measured report is persisted as a new ledger.

`source.publication.publish` stages existing-file replacements and original
backups in a caller-supplied disposable directory on the output filesystem.
Each replacement is atomic; the batch is not. Stale bytes, aliases, hardlinked
outputs and changed permissions refuse publication. Ordinary failures undo only
this invocation's unchanged outputs. Concurrent later edits are preserved, with
original backup paths reported for manual recovery. Process termination may leave
partial output and staging files: use isolated coordinator-owned worktrees and
recheck source/gates after recovery. The byte writer preserves encoding and
per-line endings rather than normalizing unrelated source text.

`compilation_database` provides strict, compiler-neutral database parsing and
source identities. It preserves argv arrays exactly and leaves command strings
opaque; the environment owns quoting, launchers, option selection and include
resolution. Missing/conflicting recipes are not permission to guess a donor.

`Compiler.overlay(source, replacements, arguments, flags)` sends protocol v4:
the same fields plus `overrides`, a map of project-relative file paths to
`{"sha256":"<original file digest>","base64":"<proposed whole-file bytes>"}`.
One request may replace headers and several bodies together. All paths must name
existing canonical regular files. A single-file proposal is a one-entry overlay;
the superseded v3 transport and single-file compiler wrappers are removed.
The original `source` still selects the build recipe.
Commands must explicitly support this version, verify every original digest and
compile the complete overlay without overwriting product source. They own compiler
include/path semantics. `source_replica` supplies a reusable, symlink-refusing
temporary copy of explicitly declared roots, with input-change checks and no
hardlinks. Its optional `artifacts` directory must be outside those roots,
Git-ignored and free of tracked files. Neither an override response nor a replica alone establishes that
the compiler actually measured the intended candidate. See the controls below.
Local commands can use `CompilationRequest.read(root, parsed_json)` to validate
the package-owned protocol and decode/pin overlay bytes before compiler dispatch.
Only profile selection, compiler options and invocation need local implementation.

`Compiler.materialize(source, roots, files, arguments, flags)` sends protocol v5
for historical file sets. It replaces `overrides` with a `tree` object containing
canonical nonoverlapping `roots`, `current_sha256` (the current complete input
snapshot fingerprint), and `files` (relative paths to base64-encoded file contents).
This is a complete file map, not a patch: omitted current files must be absent.
The source must exist in the supplied tree but need not exist in today's checkout.
No historical build recipe is inferred; the local command must have an explicit
recipe or refuse the request. Current source roots must still exist. The package's
`CompilationRequest.read` validates the manifest and freshness; `source_tree`
materializes only those files in an isolated copy and detects concurrent original
changes without reverting them. Commands without v5 support must fail explicitly.
Complete trees are transient compiler inputs, not a saved source or verdict ledger.
Replica and damage controls remain necessary; protocol success is not verification.

`inputs` lists local compiler scripts/configuration files; `environment` names
additional compiler-affecting variables. PATH and command environment placeholders
are included automatically. The library rechecks these inputs around compilation;
environment values are hashed, not printed. This is a trusted command boundary,
not a sandbox or a complete fingerprint of an external compiler installation.
No persistent object or verdict cache is introduced here. Component source,
manifest and original/archive freshness are checked before reporting results.
Every component report keeps application-recovery accounting at zero and leaves
unproven structure/runtime behavior unknown.
## Scoped application verification

### Candidate decks

Add `--compiled-diagnostics` to `recover tasks variants` when a first mismatch or
extent failure does not explain the candidate. The optional `compiled_diagnostics`
envelope shows only the deck's primary target for each scored baseline/candidate
flag trial, using the objects already compiled by that run: no extra compilation,
object-file ingestion, source publication or acceptance path is introduced.
Default reports are unchanged. Candidate symbol proposals select their own symbol,
not the baseline spelling; missing or ambiguous functions remain unavailable.

Each observed entry reports exact raw COFF section sizes/digests, a bounded byte
prefix, section-relative linear instruction offsets, and relocation symbol/index,
type, storage, definition and raw-addend evidence. Explicit associative code
COMDATs remain separate sections; neighboring sections are not inferred as tails.
Neither a listing nor a relocation name proves a linked destination. In particular,
`operands_unlinked` is the decoder's rendering of unlinked bytes at section offset
zero, **not an original-image address**. Original spans and all ordinary strict
byte/reference results remain unchanged, including extent failures.

The canonical UTF-8 JSON for this optional envelope is capped at 4 MiB (the rest
of the native report is not capped or reduced). Trial shares are allocated before
parallel execution, at most 256 KiB each, in stable report order. Each section
reports at most 16 KiB of raw bytes and decodes at most 4096 instructions. Byte,
instruction, relocation, section and trial omissions are explicit; omitted trial
indices refer to scored baseline rows followed by candidate rows. Metadata too
large for its allocation omits that trial, not its ordinary verification evidence.
`decoded_instruction_count` covers only the reported prefix; undecoded bytes,
instruction limits and unavailable decoders are not complete coverage. Linear
decoding can include padding or jump-table data and does not infer function
extents. The optional x86 dependency supplies the decoder; absent decoding does
not change verdicts, rankings, compile counts or acceptance. Keep these derived
reports in ignored storage.

After establishing the ownership model, use `recover tasks variants <task>
--image <image> --deck <deck.json> [--jobs N]`. This is a native package command,
requiring source/measurement configuration and a v4-capable compiler command,
not another project dispatcher. Relative image/deck paths resolve against
`--root`. The deck names a primary task entity/TU, authorized source/header
anchors, and one compiled damage witness per edited file:

```json
{
  "schema_version": 2,
  "source": "src/widget.cpp",
  "entity_id": "<original entity id from task context>",
  "files": {
    "src/widget.cpp": {
      "source_sha256": "<current SHA-256 of widget.cpp>",
      "before": "return value_;",
      "control": "for (;;) {}",
      "entity_id": "<original entity id of this method>"
    },
    "src/widget.h": {
      "source_sha256": "<current SHA-256 of widget.h>",
      "before": "int value_;",
      "control": "int damage_padding_; int value_;",
      "entity_id": "<original entity id of a method reading value_>"
    }
  },
  "candidates": {
    "renamed-member": {"src/widget.cpp": "return count_;", "src/widget.h": "int count_;"},
    "original": {"src/widget.cpp": "return value_;", "src/widget.h": "int value_;"}
  }
}
```

The UTF-8 `before` text must occur exactly once in each byte-pinned file. No brace
parsing or marker edits are inferred. Replacement/control text supplies the
entire replacement for that anchor, not necessarily a whole function. Keep
experimental decks in ignored artifact storage; they are not another ledger.
For separated changes in one file, `before` and `control` may be nonempty arrays
of strings. Each candidate then supplies a same-length replacement array for
that file, in anchor order. For example, `before: ["int old_;", "return old_;"]`
can pair with a candidate `["int value_;", "return value_;"]`. Each anchor must
occur exactly once in the original file, and anchors must not overlap. All byte
ranges are resolved against that original snapshot before any replacement;
replacement text is never matched again by a later edit. Empty replacement strings
delete an anchor. Untouched bytes, including line endings, are preserved.
Controls still operate independently per file and must change its compiled witness;
multiple anchors do not narrow replica, task-TU or original-identity checks.
Every candidate supplies all declared files, including unchanged anchors where
appropriate. Each file's witness must be a mapped task subject. Schema v2 replaces
the old single-file deck format; a single-file experiment uses one `files` entry.
File creation/deletion and experimental remapping of original symbols/spans are
not supported; measurements use the current source bindings, never inferred ones.

Each configured flag trial first compiles every mapped task TU (plus edited TUs)
normally and through an identical-content multi-file replica. The PE/COFF control
compares code, data, relocation identities, types,
COMDAT policy, BSS extents and link directives. Compiler-local section/symbol
numbers are normalized through explicit names and associative ownership; ambiguous
identities are refused. Timestamps, file records and CodeView source/line records
are ignored. Known numeric local labels retain function/associative owner, offset,
type and storage instead of their unstable spelling. Simple VC6 unnamed-namespace
definitions are recognized only through the primary COFF FILE path and a single
namespace nonce; function/data names and signatures remain distinct. This rule
applies to replica and damage controls, never to original-image bindings or
baseline preservation aliases. Missing definitions and ambiguous identities
are refused; ordinary external names and data stay strict. Supported standalone
CodeView 5 type streams compare complete record contents modulo emission order
and type-index numbering. Only decoded type-reference fields are normalized;
names, attributes, sizes, member/base/vtable offsets, calling conventions and
duplicate-record counts remain protected. Cycles, unsupported/malformed records
and relocated type streams fall back to exact raw-stream comparison, not omission.
This type-content comparison also applies to compiled baseline preservation;
it does not prove full class ownership, lifetime or symbol-to-type associations.
Unused section checksums are replaced by actual content/reference comparison;
EXACT_MATCH checksums are retained. This is a
replica control for scoped measurement, not proof of an identical final link.
Then each file's explicit damage control, applied independently with the other
files unchanged, must compile and change its witness function's code or relocations.
A body control cannot substitute for a header control. Unsupported
replicas or unchanged/uncompilable controls prevent scoring under those flags.
Path/time-sensitive output can make a replica unsupported; never weaken a
control or compare only the coarse match tier to bypass that refusal.

The source map is read once per deck. Candidates compile at the requested width
(default 1), with strict byte/boundary/reference checks shared with ordinary task
measurement. Every TU in a candidate sees the same complete file overlay.
Successful unchanged-source replica controls for referenced object owners are
reused within that deck and exact flag set, including when concurrent candidates
discover the same dependency. The invocation retains immutable replica bytes;
parsed objects and candidate dependency compiles are not shared. Failed controls
are not cached, every candidate still compiles its complete overlay, and the
source/image/compiler/task freshness checks remain in force. Dependency rows
report `replica_reused`; compile-attempt counters count actual compiler calls.
Nothing survives as a persistent object or verdict cache.
An authorized task function may propose a different emitted `// symbol` without
editing the live tree. Each candidate reparses its edited annotations and resolves
its own function/reference bindings; `binding_changes` lists the old and proposed
names. Emitting only the old symbol cannot prove the proposal. Symbol collisions
and out-of-scope binding changes are refused before compilation. Original
identities, spans, historical claims and non-function recovery interpretations
remain fixed; changing those requires a reviewed checkpoint, not a spelling deck.
Results include unambiguous mapped neighbors in compiled TUs, tagged by whether
they are task subjects, and explicitly list unmeasured task entities and other
mapped product units. These lists are not proof of complete caller/ownership impact.
Failures in secondary TUs remain visible alongside the primary target's diagnostics.
Rankings are advisory and separate for each flag trial. `rankings[].details`
explains the ascending comparison order: compile-error count, number of lost
baseline passing measurements, unknown then failed scored results, unmeasured
byte-distance count, known differing bytes, and finally candidate name. A shared
unmeasurable neighbor no longer hides a newly exact function behind an alphabetical
tie. Strict reference/boundary failures remain failures even when compared bytes
agree. Missing distances are counted separately, never treated as zero mismatches.
The bounded-code comparator validates relocations against the candidate section,
not the original function's length. Valid relocation fields extending past the
original extent produce a measured boundary failure, with their offsets in
`candidate_relocations_outside_original`; they cannot be treated as alignment
padding. Overlapping fields or fields outside the candidate remain malformed,
unverifiable evidence. Neither case expands the original mapping or proves
reference agreement.

Losses include previously passing mapped neighbors outside the task and bounded
initial data, keyed by original identity and source owner rather than proposed
function spelling or report order. Scored-result counts cover the union of observed
task functions and bounded initial-data objects across that flag's baseline and
candidates; an absent observation stays unknown rather than shrinking the
denominator. These counts do not include uncompiled code, generated-table or import
selection, layout or lifetime proof. Read those diagnostics and ordinary task checks
separately. A loss is evidence for restoration/review, not authority to revert a
justified structural migration. Ranking neither applies a winner nor grants
preservation or acceptance. Flag trials are never combined into a fictitious
passing configuration. Exit 0 means the deck was scored, not that any candidate
passed recovery; exit 3 means invalid inputs or an incomplete experiment.

No source is applied or claims stamped. The worker deliberately applies a
supported source form and reruns task/layout/build/obligation checks. Cross-TU
structural migrations still use task expansion and review. Multi-file experiments
are not a baseline/current outside-scope preservation verifier or a migration gate.
Concurrency
requires sufficient isolated compiler capacity; the package does not invent a
safe Wine fleet size. Default serial compilation retains main-thread signal
cleanup; parallel compiler jobs require coordinator-provided process/job
containment for interruption, in addition to the individual command timeouts.
Reports and objects are not persisted.

`tasks check-binary`, `tasks check-layout` and joint `tasks check` are also native
package operations. They require source/compiler configuration and measurement
policy, for example this PE/COFF project entry:

```json
"measurement": {
  "backend": "pe-coff-i386",
  "compiler_profile": "application",
  "flag_sets": [["/c", "/O2", "/Gy"]],
  "layout_flags": ["/Z7"],
  "unit_extensions": [".cpp", ".cc"],
  "inputs": ["build/compile_commands.json"],
  "input_roots": ["cmake"]
}
```

The flags/profile above are project choices, not library defaults. This backend
currently reads strict i386 PE/COFF and standalone CodeView 5. Unsupported formats
remain unmeasurable. Source roots, seeds, names and TU suffixes come from config;
no application identity or branch name is required. Object annotations also use
configured source selection/address spaces, independently of binary backend limits.

Binary task checks follow actual COFF external references to source-declared object
owners, compiling those TUs even when they have no selected function. Each flag
trial gets fresh dependency objects; missing definitions, unsupported owner files
and compile failures remain unresolved. The `object_dependencies` report lists
these additional inputs separately from the selected function TUs. This is read-only
measurement scope, not permission to edit the owners or proof of object extent,
initialization, lifetime or linked selection. Weak/local symbols and unreferenced
declarations do not select owners. Candidate decks follow new references too, with
an unchanged-source replica control for each extra TU and the same candidate overlay.

One complete flag trial must match each body; partial trials cannot be combined.
Declared layouts must agree across every selected TU/trial. Joint checks share
objects only inside that invocation and still report all unknown/failing evidence.
Freshness covers re-enumerated source/build inputs, compiler configuration and
declared files/environment, library implementation, task decisions and original
image. Fingerprints are diagnostic, not reusable verdict-cache keys. These checks
do not prove full hierarchy/lifetime, a full application build, or runtime behavior;
passing them alone never enables collection.

The example project's old operation dispatcher and application adapter modules
have been removed. Its [recovery integration](../../recovery/README.md) now contains
configuration, compiler interaction and integration tests. The legacy production
gate/tools are removed; complete application structure/runtime acceptance remains
unfinished.

### Delegated ownership task selection

`recover start --planner recovery/planner.json --reviewer recovery/reviewer.json`
selects and runs one ownership task without a preselected task id. It uses the
same driver/worker/configuration/iteration options as `run`, including optional
`--coordinator` and `--migration`. Configure a read-only `planner` role in local
`recovery/agents.json`; it uses the same command/instructions/response transport
as the reviewer. No provider or project Python adapter is required in the package.

The planner delegation is explicit authority to choose application work and make
a **task-decision-only Git commit**, not to commit recovered source:

```json
{
  "schema_version": 1,
  "author": "coordinator",
  "command": {"argv": ["recover", "agent", "planner"], "timeout": 300},
  "max_entities": 100,
  "max_editable_files": 20,
  "commit_decision": true,
  "evidence": ["Authorize bounded ownership tasks and task-only checkpoint commits"]
}
```

Keep that policy and provider configuration committed. The author must differ
from worker/reviewer attribution. Requests contain a fresh `snapshot`, `target`,
`delegation`, derived ownership `groups`, existing `tasks`, `create_available` and
`instructions`, plus derived external `reservations`. Groups expose ownership summaries, not every function/disassembly;
use `recover partition --group <selector> --details --impact` for detailed members
and seed alternatives. Existing tasks retain pending discoveries and intent in the
request; `tasks context <task>` supplies deeper evidence. Eligibility is not a
completion score. Byte-exact groups can still need structural
recovery. Dependency, conflicting, unassigned and seed-only groups stay visible
with their required next actions, but cannot become application tasks this way.
Group scope uses source homes and their own interfaces, not a transitive include
closure or a guess at historical TU boundaries. This avoids rebuilding an include
graph separately for every group in the planner request.

Responses have exactly `snapshot`, `action`, `task_id`, `group`, `intent`,
`editable_files`, `evidence`. Actions are `create`, `resume`, `defer`. Creation
selects an eligible group and a fresh task name, with evidence-backed intent and
optional additional regular product source files within the file limit. Native
publication fixes all group identities and the committed source baseline; planners
cannot supply an older baseline, drop group members, edit seeds/vendor dependencies,
or overlap an existing task's entity/file reservations. Existing tasks reserve
their expanded scopes regardless of cached reviews or claims; they are not silently
retired. Resume names an existing task by the same author/target/original, without
changing scope or baseline. For resume/defer, `group` and `intent` are empty strings
and `editable_files` is empty; defer also has an empty `task_id`. Evidence is always
required. Planner proposals are not proof or direct mutation authority.
The two size limits bound selection of new and existing tasks; existing tasks retain
their authorized scopes even when too large for a particular planner invocation.

To leave an existing recovery checkout parked while scheduling other work, the
planner policy accepts optional `reserved_worktrees`:

```json
{
  "reserved_worktrees": [{
    "path": "build/recovery/worktrees/widget-owner",
    "tasks": ["widget-owner"],
    "evidence": ["Preserve this unfinished structural migration; schedule disjoint work"]
  }]
}
```

This is a scheduling decision, not copied source ownership or acceptance. Paths
are canonical, absolute or relative to the coordinator; each must be a registered
checkout in the same repository. The package derives the union of named tasks'
current destination/worker scopes, including worker-only append-only decisions.
Conflicting task identities/baselines, unknown decision streams or source changes
outside the named scopes refuse; no history is imported into the destination.
Omitted reservations do not waive safety: other registered checkouts with dirty,
staged or unmerged committed source/recovery changes refuse scheduling. Clean
ancestor and framework-only checkouts are not automatically recovery reservations.
Worker changes are measured from their merge base, so master-only advances are
not mistaken for worker work. Staged edits and uncommitted reversals cannot hide
unmerged commits. Ignored source/decision files still participate.

Planner group selection, extra editable files, existing-task resume and new task
IDs respect these reservations. Campaign resume also checks its saved assignment
and worker-expanded scope. Overlapping coordinator expansion grants refuse before
any delegated decision; narrow the grant or explicitly reconcile the parked work.
External driver leases remain held for scheduling/driver/campaign execution;
source, history and worktree-inventory freshness are rechecked around agent calls
and campaign stages. New or changed external work stops the run, never silently
changes the authorization. These guards coordinate cooperating drivers and detect
other edits; they are not a sandbox against malicious concurrent filesystem writes.
The package never prepares, checkpoints, resets or cleans a reserved checkout.

Creating a task requires a clean worktree, committed source/configuration, and a
working Git identity/signing setup. Only the package publishes and checkpoints its
new task JSON before invoking the normal driver. Unexpected edits or checkpoint
failures refuse dispatch and preserve work/index for inspection; no automatic reset
or rollback occurs. Task commits use the native guarded raw publisher, without Git
hooks or source-conversion filters. Resume uses the existing dirty-source/append-only-history rules.
The read-only planner itself must be externally sandboxed. Command hashes detect
writes and stale decisions, not malicious intent or false reasoning.

One `start` performs one planning call and one bounded recovery invocation. It does
not infer unmapped function bounds, author new source bindings, create worktrees,
automatically retire tasks, schedule a fleet, or commit/integrate recovered C++.
Empty or unresolved inventories need executable analysis and source-owned evidence;
deferral is not successful recovery. The compiler-backed local control exercises
selection, a task-only commit, structural recovery with a temporary byte regression,
caller discovery, independent review and native acceptance through installed CLI
subprocesses. Its reasoning agents are deterministic local doubles, not live-model
validation or an executable-only Alpha Centauri recovery claim.

### Request help from another task owner

`recover tasks route TASK --file DECISION.json [--target-root CHECKOUT]`
publishes a typed `discovery_routed` decision in the **originating** task journal.
Run it at a serial coordinator checkpoint. The explicitly located target must
already exist and own the routed original identities. No source, target journal,
task baseline or editing scope is changed. The command requires both checkout
driver leases and the target journal lease to be free; it never waits for a live
worker to finish. Relative decision and target paths belong to `--root`.

The decision file is an input, not a new authority ledger. Its durable contents
live in the originating journal after publication:

```json
{
  "request_id": "request-owner-help-1",
  "expected_revision": 1,
  "route": {
    "kind": "discovery_routed",
    "author": "source-task-coordinator",
    "discovery_id": "existing-finding-id",
    "discovery_event_id": "<exact-original-discovery-event-sha256>",
    "target": {
      "task_id": "existing-owner-task",
      "task_sha256": "<immutable-task-digest>",
      "base_revision": "<original-committed-task-baseline>",
      "history_head": "<current-owner-task-history-head-or-null>"
    },
    "subjects": ["entity-<affected-original-identity>"],
    "evidence": ["Why this existing owner should investigate this part of the finding"]
  }
}
```

Use the exact `reference` from `recover tasks status OWNER` for `target`;
`history_head` is JSON null when that task has no decisions. The existing
`recover history task:TASK` output provides the original discovery event identity.
Subjects must be nonempty, sorted, unique, affected by that pending finding,
outside the originating scope, and already owned by the pinned target scope.
The target must concern the same target image/address space. Visible conflicting
task owners refuse. No checkout location, compiler output, run transcript or
acceptance result is written into a route.

A new route requires the exact current owner reference. An exact request retry
does not append another event; later owner history may extend its pinned prefix,
but changing its task identity/baseline or replacing that history refuses.
Another request cannot reroute an already-routed part of the same discovery.
The original hypothesis, full affected set and pending status remain intact.
Historical route parsing does not require the owner's checkout to survive.

Native task context and planner task rows expose outgoing and derived incoming
requests from locally available task histories. The library's
`task_routes.incoming_routes(store, task, sources=...)` can inspect explicitly
supplied additional stores; it neither searches for external checkouts nor copies
their journals. Live owner identity/scope checks still apply. Requests are not
followed recursively, including when the dependency graph has cycles.
Explicit replicas of one source task must share immutable identity and compatible
history. Its newest supplied prefix determines pending requests; an older replica
cannot resurrect resolved work. Forked histories refuse rather than choosing a side.

This is a **help request, not execution authority**. Planner eligibility and
external reservations are unchanged. The campaign does not activate a parked
owner, and routing does not satisfy investigation permissions or resolve the
original finding. Complete all-findings accounting and fresh acceptance still
apply. Dependency activation, parallel waiting/investigation permissions and
verified cross-owner closure remain separate implementation work.

#### Coordinator-driven managed-owner handoffs

An optional `"route_discoveries": true` in the coordinator delegation enables
native campaign routing. Existing delegations remain unchanged unless explicitly
updated; a preserved campaign requires the bound coordinator reauthorization
described above, and retained workers need a compatible installed runtime.

The campaign supplies all original assignments in the current saved batch,
including inactive peers, plus the coordinator's task catalog and older/parked
reservations. It does not infer ownership from only the active dispatch list.
Native snapshots retain exact task/history/branch catalogs and expose overlapping
historical scopes as `conflicts`. Those conflicts exclude the affected owners from
routing, not unrelated disjoint tasks. Ambiguous or rewritten task replicas still
refuse; no task is silently retired or chosen as the winner of an overlap.

The read-only coordinator sees `routing` and derived `route_choices` alongside
the complete findings and normal scope facts. A response may use `action: "route"`
with `route: {"owner": "supplied-task-id", "subjects": ["entity-..."]}`. Its ordinary
scope lists remain empty and `binding` remains null. All other actions use
`route: null`. Only known, affected, outside-scope, not-already-routed identities
from a listed unambiguous **managed** owner are selectable. Parked, historically
retired (`retained`), and unassigned owners remain visible but unavailable. The provider never supplies a checkout
path, author, baseline, target digest or discovery-event identity.

A valid restricted-preplanning proposal requests fresh serial coordination; it
does not publish there. The serial decision uses the same per-task provider-call
budget, without replaying a proposal. The controller supplies the original finding
and current target references, borrows actual live driver/peer lease tokens, and
publishes only the originating route journal. New decisions invalidate earlier
investigation permissions, and routed subjects leave the available choices.
There is no stored permission implied by a route response or successful process.

Subsequent serial driver rounds derive `context.owner_requests` from the explicit
managed checkouts, including when the recipient has no local discoveries. Workers
and reviewers see the full original finding, not just the routed subset; source
journals are not copied and recipient discoveries are not fabricated. Native
guards check external source/Git state around provider and evidence calls, in
addition to task/history snapshots. Unauthorized edits remain preserved on refusal.
Changes within the recipient's existing authorized C++ scope still need all normal
checks, independent review and publication authorization.

A pending source route cannot be collected or cleaned up. An owner implicated by
another managed task's pending finding or incoming request is retained for fresh
resumption, even if its serial turn came before the source's routing decision.
Committed owner progress may nevertheless pass ordinary fresh candidate and
destination verification and integrate independently. Pending requests retain
both its worker and integration checkout. The existing retirement record marks
only the completed historical transfer; cleanup is separately deferred. Resume
checks the retained committed identities and ancestry and reports
`verification: not_replayed`, without reopening the old transfer against later
independently integrated work. It does not assert that historical acceptance is
current acceptance. Further owner edits need another authorized attempt, not a
rewritten retirement or integration decision. Failed verification never creates
that retirement, closes a request, or authorizes cleanup; a partial transfer still
uses fresh destination verification on resume.
Cleanup checks existing requests even if new automatic routing is subsequently
disabled. Requester leases stay held from that decision through removal, so a
cooperating native publisher cannot insert a new request in between.
Owner integration does not refresh another worker's uncommitted source, enlarge
its scope, or resolve its discovery. Those require separate source reevaluation
and the appropriate native integration/decision workflow.
Unrelated owners are not held back solely because another finding exists. This
handoff does not resolve either task, authorize a parked checkout, or establish
accepted cross-owner recovery. Cycles and dependencies beyond existing managed
owners remain explicit replanning work.

### Delegated discovery coordination

`recover run TASK --coordinator recovery/coordinator.json` enables a read-only
decision agent within an existing task. Omit it to retain manual discovery stops.
This role does not select tasks (`start` above does), invent original identities, manage
worktrees or collect code. Configure the optional `coordinator` role in the local agent-command
configuration using the same command/instructions/response shape as the reviewer.
No provider or application adapter is required in the package.

The delegation file has these required fields (populate `subjects` with additional
original entity IDs from the source inventory, and files with explicit source paths):

```json
{
  "schema_version": 1,
  "author": "coordinator",
  "command": {"argv": ["recover", "agent", "coordinator"], "timeout": 300},
  "subjects": [],
  "editable_files": [],
  "evidence": ["Explain the authorized investigation boundary"],
  "max_decisions": 8
}
```

`author` must be the task author, separate from worker and reviewer. The two lists
are upper bounds on additions, not a request to expand everything. Empty lists
permit only decisions within existing scope. Editing expansion accepts existing
regular configured source files only, not compiler policy or the image pin.
New files and unknown original identities need separate coordinator authorization.
Keep delegation configuration versioned; its digest accompanies published decisions.
An optional `seed_bindings: {"image": "path/to/original.exe"}` also delegates
unclaimed product bindings for existing selected seed-only identities. The image
may instead be one whole `{env:VARIABLE}` placeholder. Its contents must match
the source pin; there is no image-path inference or additional scope grant.

Requests contain a fresh snapshot token, pending discoveries, current/delegated
scope and affected source/seed records. Responses contain `snapshot`, `action`,
`discovery_id`, `subjects`, `editable_files`, `evidence`, and nullable `binding`.
Actions are `expand`, `incorporate`, `reject`, `defer`, `investigate`,
`investigate_local`, `bind_seed`;
only expansion uses nonempty scope arrays. Evidence
is mandatory. Incorporation requires affected originals already in task scope;
it records a source interpretation, never a passing proof. Unknown originals,
stale responses, writes, policy expansion and no-op expansion are refused.

Driver discovery handling and campaign resume preplanning first execute the
configured context and scoped checker under the existing worker/compiler lease.
The coordinator receives their full reports, exit codes and coverage in optional
`diagnostics`, bound to the current source/Git/task/runtime epoch. Context failure
is visible and skips checking; inconsistent checker status refuses coordination.
Each decision, including one after scope expansion, obtains fresh observations.
These are diagnostic inputs, not stored task evidence or acceptance receipts:
an old compiler failure is not a current measurement, but a current passing check
neither dismisses its finding nor proves anything outside its reported coverage.
No preparation, extra compiler allocation, automatic resolution or acceptance is
performed by this refresh. Direct API callers may omit diagnostic configuration;
their requests make no claim to contain fresh compiler observations.

For projects declaring native `source` configuration, both the driver and native
agent bridge enforce original-binding continuity around every worker invocation,
including failed or interrupted invocations. Ephemeral source snapshots retain
unclaimed product mappings as well as claims: original identity/spans/catalogue
symbol, source-versus-seed role, mapping mode/classification, rationale and explicit
header-body edges. Candidate multiplicity matters; another seed or duplicate
cannot stand in for a deleted candidate. Intact mappings may move between
authorized source hosts. Body text, line numbers, derived implementation state
and emitted-symbol hints are not pinned, so structural C++ recovery remains possible.
Configured-source changes outside scope are refused even when Git ignores them.
Violations preserve WIP and stop before response discoveries are consumed. The
refusal saves pre-worker source in private ignored `build/recovery/worker-source-failures/`
storage when safely available, retaining uncommitted mapping rationale for manual
repair. Failure to save a backup still refuses; never write into unsafe or
committable storage. No automatic rebinding or source rollback occurs. These
backups are historical recovery aids, not reusable checks or authority.
Native coordinator binding precedes
the next snapshot. This is metadata continuity, not proof of correct attachment,
matching or structure. Command-only integrations without native source
configuration retain their own metadata/checker contracts.
If replacing the initializer's native source scaffold with an external-only
protocol, remove the unused `source` declaration explicitly; a missing pin or
malformed declared source provider is an error, not an implicit opt-out.

Pure coordinator proposal-scope failures return an internal `proposal_rejected`
outcome after freshness validation, with the proposed action, bound snapshot,
discovery identity, reason code and feedback. This is not a provider-selectable
action or additional authority. The driver and central investigation planner may
request a corrected proposal within the existing allowance, supplying optional
`previous_rejection` feedback bound to the original pre-call source/task/Git/runtime
epoch and delegation digest. Each retry collects fresh configured diagnostics.
Changed inputs invalidate the feedback before another provider call. Successful
decisions clear it; repeated discovery/action/refusal-reason combinations or budget
exhaustion stop correction, retaining every discovery and source edit. Campaigns
leave refused/exhausted workers pending without cleanup/integration and can continue
unrelated work. Feedback and counters are transient, not new task journal events.
Covered refusals are outside-delegation resolution/investigation, investigation
or resolution before current-scope expansion, expansion beyond explicit delegation,
and expansion without progress. Invalid responses, stale inputs, mutations,
unknown original identities and native writer failures remain fatal. Restricted
preplanning validates proposal legality before routing valid publishing proposals
to fresh serial coordination; no writer runs in the restricted phase.

Coordinator requests include native `decision_facts` for every pending finding:
currently owned identities, known delegated repair subsets, outside-delegation
identities, unknown originals, candidate delegated files and conditional action
routes. The native agent bridge recomputes these facts. They are constraints,
not evidence that a repair, expansion or resolution is justified. In particular,
`conditional_local_work` still requires useful work in existing scope; the provider
must defer if only outside-owner work can help.

One process-local `DecisionBudget` per task and campaign batch attempt spans
restricted investigation preplanning and subsequent serial coordination. It is
charged immediately before each configured coordinator provider invocation,
including invalid/unavailable proposals and serial handoffs, but not diagnostics.
No parallel child receives coordinator authority or a serialized budget. Standalone
driver/planner invocations create their own bounded allowance; a fresh explicit
campaign resume starts a fresh attempt. Profiles, grants, manifests and baselines
are not rewritten to carry a remaining count. Exhaustion never establishes progress
or permits retirement of an unresolved clean worker.

`investigate` requests one bounded worker attempt for a pending discovery entirely
inside current scope. It publishes no resolution or acceptance: its snapshot-bound
authorization and evidence are transient worker context. Every pending discovery
must be authorized individually before dispatch. Requests retain the full pending
list; optional `investigating` IDs identify those already authorized in this round.
After source edits or published decisions, authorization must be obtained afresh.
Pending discoveries block all review/acceptance/checkpoint paths even if checks
pass. Only subsequent evidence-backed incorporation or rejection resolves them.
Deferral never grants investigation, and verification-only runs cannot request it.

`investigate_local` explicitly authorizes the same bounded, snapshot-bound attempt
when a finding also names known mapped originals outside the task/delegation.
Only the existing task identities and editable files are authorized. The coordinator
must explain useful local work; if only the other owner's work can help, defer.
This action does not grant outside edits, transfer ownership, classify the dependency
as nonblocking, or resolve anything. Every finding remains visible and blocks
acceptance. Unknown original identities still require a separate binding decision.
Ordinary `investigate` retains its entirely-in-scope requirement.

`bind_seed` requires explicit `seed_bindings` delegation and a `binding` object
with `entity_id`, `source`, positive `line`, current `source_sha256`, emitted
`name`, `reason`, and a nonempty array of distinct `evidence` references.
`binding.reason` and `binding.evidence` become source-owned identity rationale:
explain the original address/symbol, body and owner relationship. Top-level
`evidence` remains separately required for the coordinator decision and its run
context (diagnostics, request hashes, impact counts, compiler failures and
permission checks); it is not copied into source. The two channels have no
fallback: older proposals without explicit binding evidence are refused, while
existing source bindings remain unchanged. Source evidence must be single-line
text without embedded original markers.
The identity must already be selected and affected by the
pending discovery; its existing records must all be seeds with agreeing spans.
The product TU must already exist in the editing scope. The native binder retains
those spans, validates the pinned executable backing and fresh source, inserts
only unclaimed binding comments, and preserves every seed and historical claim.
Other actions must not supply a nonnull binding. No proposal can set a byte claim,
change an extent, replace an existing product binding, or add an unknown original.
This currently uses the native PE32 binder, not an inferred universal format.

Binding publication occurs between worker epochs and invalidates earlier context
and investigation grants. The discovery remains pending, so fresh coordination
and verification are still required. A mapped body is not thereby correct or
accepted, and an unannotated product body cannot borrow its seed's measurement.
If a newly bound method already compiled as unbound code in the baseline, the
preservation comparator retains its current same-named definition in the residual
too. Exact bytes, references, metadata and associated sections must still agree;
current names do not establish historical ownership or authorize changing that
old unbound code. Actual new definitions retain the existing scoped exclusion.
Reports expose this extra protection as `newly_bound_protected_functions`.
The compiler-backed seed control covers binding, structural repair, a temporary
byte regression, restoration and full acceptance on a test-owned executable.
Its reasoning roles are deterministic; this is not a live Alpha Centauri result.

The driver journals expansions and resolutions through TaskStore, refreshes scope/context and
obligations, and resumes the same worker budget and baseline. Decision calls have
a separate bounded budget. Deferral, unavailable providers or budget exhaustion
preserve source and pending discoveries. Independent review and full native
acceptance remain required. Only separate explicit checkpoint authorization permits
the driver to commit freshly verified source in change mode; valid append-only
task/review decisions can be resumed under the restart rules above. Commands
must be externally sandboxed: post-execution hashes detect writes but cannot
prevent a malicious command. Delegation validates authority, not the quality of
the agent's reasoning or the truth of its evidence.

From this development workspace:

```sh
uv run pytest packages/match-recovery/tests                 # portable core
uv run pytest packages/match-recovery/tests recovery/tests  # plus integration
uv build --package match-recovery
```

Tests include an unrelated target using a custom address space, `.cc` source under
`code/`, and real local subprocess commands with no framework imports. They drive
one synthetic recovery through editing, rechecking, independent review and
acceptance. This verifies the command boundary, not any real application's runtime.
