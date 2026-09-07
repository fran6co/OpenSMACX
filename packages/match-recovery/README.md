# Match recovery

An independently installable Python package with a standard-library-only core for
structure-led matching decompilation. It does not register projects, import a
project's Python code or bundle a game's adapters. Project integration is local
configuration plus commands exchanging JSON. Those commands can use any language.

## Start a project

Install a built wheel into your environment, then work in your own Git repository:

```sh
python -m pip install /path/to/match_recovery-0.1.0-py3-none-any.whl
recover init --target my-application
```

This creates only local, editable files:

```text
recovery/
  driver.json       context/check commands and arbitrary target identity
  project.json      native source inputs and optional local operation adapter
  adapter.py        starter context command; checks deliberately return unknown
  worker.json       your worker command and attribution
  reviewer.json     your independent reviewer command and attribution
  agents.json       prompt-driven provider commands used by the native bridge
```

Initialization refuses to overwrite existing files. The starter interpreter is
the interpreter running `recover`; use `--python` to select another one. You can
replace `adapter.py` entirely, call existing tools directly, or use separate
commands for every role. No Python inheritance, plugin registration, package fork
or library modification is required. The package does not supply an agent service,
compiler installation or project verification policy.

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

The bridge verifies the task and editing scope before launch, rejects reviewer
writes, worker scope violations, Git mutations and changed policy, and cleans its
temporary directory on success or failure. It never reverts source edits. Missing,
oversized, symlinked, malformed or stale responses, nonzero provider exits and
timeouts exit 3 with structured error details. The driver reports `agent_error`
and stops; it does not retry quota failures or invent a reset time. Review schemas
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
| Worker | Task, context, check, authorized files and review feedback | `discoveries` array; may edit only authorized files |
| Reviewer | Task, context, check and source `snapshot` | Same `snapshot`, `decision`, nonempty `evidence`, `feedback`; read-only |
| `obligations` | Task | Source-derived `obligations` and optional `interpretations`; read-only |
| `acceptance` | Task, reviewed snapshot, protected obligations and review | Full-project gate report; read-only |

Only `context` and `check` are required in the driver configuration. Add source
`obligations` to preserve restoration requirements across rewrites. Without
`acceptance`, even a passing scoped check and approved review stop short of
collection. Review approval is evidence-backed judgment, not a verifier result.

Worker discoveries have `hypothesis`, `affects` (original entity identities), and
`evidence`; discoveries affecting other entities pause for a coordinator's scope
decision rather than disappearing at a function boundary. An empty response is
`{"discoveries": []}`. Reviewer decisions are `approve` or `request_changes`.
Source-derived historical interpretations require explicit reviewed supersession
or retraction; generic approval cannot silently erase them.

Acceptance must report exactly `scope: "full_project"` and `result`, `build`,
`structure`, `references`, `obligations`, `runtime`, each one `pass`, `fail` or
`unknown`. All dimensions must pass for acceptance. Source and decision freshness
are rechecked around commands; unrelated writes and Git mutations are refused.

Optional `migration_check` and coordinator `--migration` permission allow reviewed
temporary matching regressions while structural work proceeds. They require
source obligations, fresh build/structure checks and independent review. Unknown
proof is not permission to regress. A migration checkpoint exits 2, never green;
closing it requires restoring all protected obligations. The reusable policy APIs
live in `migration.py`, `migration_protocol.py`, `reviews.py` and `tasks.py`.

## Package boundary and verification

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
the PE format. Neither validates application behavior or complete source coverage.

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

### Source-policy audits

`recover audits check <name> [--root <project>] [--summary]` runs natively, with
no project Python or compiler. Add `"audits": "recovery/audits.json"` to the
project configuration. That file has `schema_version: 1` and an `audits` object
keyed by project-chosen audit names. Each audit specifies:

- `ruleset: "cpp-recovery-v1"`: an opt-in set of conventional C++ decompiler
  smells, such as manual dispatch, lifecycle seams and address-derived names.
  Alternatively, `cpp-model-v1` locates raw member/inheritance access, naming
  questions, source-mapped function-address casts and textual orphan candidates.
- `unit_extensions`, `interface_extensions`: disjoint subsets of configured
  source suffixes. Some patterns apply only to units, avoiding declaration counts.
- `exclude_roots`: explicit paths within configured source roots. Pattern scans
  also omit the configured seed roots; no directory basename is globally special.
- `ceilings`: nonnegative integer policy limits for every selected rule/counter.
  The rule names are exported by `match_recovery.cpp_audit_rules.CPP_RECOVERY_V1`.
  Model rule names are exported by `match_recovery.cpp_model_audit.MODEL_RULES`.
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
masked-byte measurement in `source.asm`, `source.calls` and `source.record`
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
instruction-shape requirement; non-exact shape and full structural obligations
currently remain unknown. Declared offsets alone do not establish hierarchy or
lifetime. `candidate` contains the complete scoped diagnostics; `--summary` omits
those and fingerprint maps while retaining obligation details and refusals.

This command supplies measurement inputs for a future complete migration gate,
not an admission report itself. Admission additionally requires fresh independent
review, build, structural prerequisites, references and outside-scope preservation.
Migration-check reports must now include `references` with a passing result, even
when historical source recorded only byte claims. Missing/failed/unknown references
cannot be waived by a matching-regression permission.

`tasks check-preservation <task> [--unit path ...] [--summary]` adds native
baseline/current compiled preservation evidence, without a project checker script
or another ledger. It reads the task's immutable Git baseline and uses the same
current measurement/compiler policy on both source versions. Every configured
non-seed TU is included by default, even units with no function annotations and
dependency units. A diagnostic unit subset leaves the overall result unknown.
No original executable argument is needed: this comparison does not claim that
the baseline or candidate matches the original.

Only exclusively owned function COMDATs selected by source origin identities and
their associative children may vary. All remaining code/data, BSS extents, link
directives, weak/common symbols, public aliases and exact symbolic relocations
are compared. Scoped entry renames use original entity identity; local relocation
destinations are never reduced to a generic placeholder. Ordinary undefined
callees used only by scoped code are not outside definitions. Outside function
bindings/claims and object/member/base/generated/component annotations also stay
protected; a comment-only address change cannot be hidden by unchanged objects.

Each TU needs a matching direct/current-overlay control. Each changed file also
needs a compiler-observed unique `#error` control before substituted baseline
bytes can support a pass. Generic compiler failure is insufficient. Reports carry
digests, section identities, coverage and failures, never object bytes. Inputs,
task decisions, compiler policy and Git state are rechecked; no source, claim or
baseline is edited. Temporary copies are disposed through the same compiler overlay
path as candidate decks.

For inexpensive investigation before the full sweep, run `recover tasks
check-preservation <task> --unit code/model.cc --diagnostic --summary`. This
compiles the selected current TU directly, checks its unchanged replica, and
compiles the historical tree under each configured flag set. It deliberately
omits per-file damage probes. Both the command and every unit result remain
`unknown` (exit 3), even when `compiled_comparison` reports agreement. This mode
cannot establish preservation or admit a migration; omitted controls and units
remain explicit in the report.

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

Unmigrated operations and projects using another source provider use the optional
`adapter` command in the same file, exchanging versioned arguments and an
`output` envelope. This is a JSON protocol, not a second project CLI. The starter
configuration still provides that adapter; add `source` when using native readers.
Generic source annotation readers stay in the core. Compiler commands, environment
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
are ignored. Cached checksums for ANY/ASSOCIATIVE COMDATs are replaced by actual
content/reference comparison; EXACT_MATCH checksums are retained. This is a
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
Results include unambiguous mapped neighbors in compiled TUs, tagged by whether
they are task subjects, and explicitly list unmeasured task entities and other
mapped product units. These lists are not proof of complete caller/ownership impact.
Failures in secondary TUs remain visible alongside the primary target's diagnostics.
Rankings are per flag trial: successful compilation, measured task-wide strict
pass/fail/unknown, then known differing bytes. They neither combine partial trials nor choose code on
the worker's behalf. Exit 0 means the deck was scored, not that any candidate
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
gate/tools and full acceptance integration remain separate unfinished work.

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
