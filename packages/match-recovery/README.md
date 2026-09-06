# Match recovery

An independently installable, standard-library-only Python package for
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
  project.json      source-operation adapter behind the package's public CLI
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

The package contains workflow/policy, strict JSON decisions, disposable cache
utilities, process supervision and reusable comparison/format helpers. Optional
i386 PE/COFF helpers do not impose that format on the driver. Recomputable reports
are not persisted as authority; no SQLite database is used.

All public recovery commands, including `inventory`, `partition`, `components`
and task commands, belong to this package. Source operations use the command in
local `recovery/project.json`, sending a versioned arguments object and receiving
an `output` envelope. This is a JSON adapter protocol, not a second project CLI.
Generic source annotation readers stay in the core. Compiler commands, environment
variables, flag choices and optional interaction scripts belong to the project.
The example project keeps its VC6/Wine interaction in `recovery/environment/`;
there is no separate compiler package or per-compiler CLI.
The repository's [recovery integration](../../recovery/README.md) still contains
a transitional legacy bridge and project verification glue. That bridge is not
the intended minimal configuration for a new project; reusable orchestration
is still being extracted from it. Integration tests stay outside the core package.

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
