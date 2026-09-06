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
```

Initialization refuses to overwrite existing files. The starter interpreter is
the interpreter running `recover`; use `--python` to select another one. You can
replace `adapter.py` entirely, call existing tools directly, or use separate
commands for every role. No Python inheritance, plugin registration, package fork
or library modification is required. The package does not supply an agent service,
compiler installation or project verification policy.

Configure these commands before starting real recovery. Configure an independent
worker and reviewer by editing their JSON files or setting `RECOVERY_WORKER` and
`RECOVERY_REVIEWER` to executable paths implementing the protocol below. Add
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

### Source-policy audits

`recover audits check <name> [--root <project>] [--summary]` runs natively, with
no project Python or compiler. Add `"audits": "recovery/audits.json"` to the
project configuration. That file has `schema_version: 1` and an `audits` object
keyed by project-chosen audit names. Each audit specifies:

- `ruleset: "cpp-recovery-v1"`: an opt-in set of conventional C++ decompiler
  smells, such as manual dispatch, lifecycle seams and address-derived names.
- `unit_extensions`, `interface_extensions`: disjoint subsets of configured
  source suffixes. Some patterns apply only to units, avoiding declaration counts.
- `exclude_roots`: explicit paths within configured source roots. Pattern scans
  also omit the configured seed roots; no directory basename is globally special.
- `ceilings`: nonnegative integer policy limits for every selected rule/counter.
  The rule names are exported by `match_recovery.cpp_audit_rules.CPP_RECOVERY_V1`.
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
