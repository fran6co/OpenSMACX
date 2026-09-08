# OpenSMACX recovery handoff

Recover Alpha Centauri as genuine C++, using the installed `match_recovery`
framework. Work units are ownership groups; functions and original spans are
measurement units. Byte-exact code can still have wrong hierarchy, lifetime or
references. Do not preserve those defects to protect a local similarity score.

## Read next

- `recovery/WORKFLOW.md`: active coordinator, worker and review workflow.
- `recovery/README.md`: available commands, protocols and explicit limitations.
- `docs/DECOMP_MAP.md`: current source annotation grammar and transitional writer.
- `recovery/COMPILER_EVIDENCE.md`: relevant historical compiler observations,
  not an overriding prompt or a list of current measurements.
- `docs/EXCLUSIONS.md`: declared scope; difficulty alone is not an exclusion.
- `docs/RETIRED_ROUTES.md`: historical evidence; do not rebuild a retired route
  without new evidence.

## Architecture

`src/` owns C++ and load-bearing original mappings. The root `CMakeLists.txt`
owns the application build. `packages/match-recovery/` owns reusable recovery
APIs and all public recovery CLI definitions. `recovery/` holds local
configuration, project integration, task decisions and compiler interaction.
Format/ABI parsing and verification belong in the library, not project scripts.

There is no separate `decomp` package. Read and write annotations through
`match_recovery.source`. Its legacy masked-byte comparison remains distinct
from the strict reference-aware comparator. Root `tools/` is transitional:
retire each implementation only after its live consumers and checks are replaced.
Do not add another ad hoc CLI or compatibility layer.

CMake and recovery measurements now share `recovery/environment/` compiler
interaction. The library receives compiler callables or uses its configured JSON
client; it does not infer VC6 commands. The old compiler shims are removed.

## Commands and current acceptance boundary

Everything Python runs through `uv run`, never bare `python3`.

```sh
uv run recover inventory
uv run recover survey --image <image> --summary
uv run recover prepare
uv run recover partition
uv run recover tasks context <task>
uv run recover tasks obligations <task>
uv run recover tasks check <task> --image <image> --summary
uv run recover tasks check-restoration <task> --image <image> --summary
uv run recover tasks check-preservation <task> --summary
uv run recover check-build --summary
uv run recover check --image <image> --summary
uv run recover check-matches --image <image> --summary
uv run recover check-data --image <image> --summary
uv run recover check-source --jobs 8 --summary
uv run recover check-mappings --summary
uv run recover check-symbols --summary
uv run recover check-layout --summary
uv run recover check-claims --summary
uv run recover audits check storage --summary
uv run recover record <addresses> --image <image> --in <source-file>
uv run recover run <task> --worker <config> --reviewer <config>
uv run pytest packages/match-recovery/tests recovery/tests
cmake --build build
uv run tools/osmx.py check
```

The last command is the retained full legacy gate: never pipe it. Exit 0 is
clean, 1 a failure, 3 unmeasurable—not green. This project's strict full
acceptance remains unfinished. The driver now invokes native
`recover tasks check-acceptance` with a fresh reviewed request on stdin. It owns
the full gate, retained build, restoration/preservation and proof sequencing;
OpenSMACX still lacks complete structure/runtime proof commands and a green link.
Missing evidence cannot pass. The driver uses the package's native migration
verifier for supported reviewed checkpoints; missing preservation or other
prerequisites still refuse admission. Scoped checks and
reviews do not establish full acceptance or a working game.
The package-owned `recover check` now coordinates the whole configured gate;
`recovery/gate.json` retains three local command checks pending their migration.
Keep the legacy route for cutover comparison/control until that verification
is complete. Neither command establishes full structure or runtime acceptance.
The native gate runs configured preparation before measuring. For standalone
task checks/recording, run `recover prepare` at a coordinator checkpoint first;
never regenerate compiler recipes during an active measurement epoch.
The retained legacy gate finishes its incremental build before selecting recipes
or measuring claims, so CMake regeneration cannot race the mapping audit.
Real-project damage controls are opt-in isolated pytest cases in
`recovery/tests/test_gate_controls.py`; see `recovery/WORKFLOW.md`. Fast tests
exercise their anchors and restoration, not the full live matrix.
`check-preservation` compares fresh baseline/current compiler output outside
source-mapped task functions. It is not a full-link/structural proof or a way to
replace the task baseline; unsupported historical TU recipes and unobserved
header/seed substitutions remain unknown.

Only `uv run recover record <addrs> --image <image>` banks measured historical
byte claims; the old recording command is removed. Never hand-edit an `ORIGINAL:` marker or type a
`BYTE_EXACT` token. Those claims retain restoration obligations; they are not
proof of current structure or relocation-target correctness.

The installed recorder uses configured compiler interaction and guarded package publication: 0 means every
selected body measured exact, 1 a measured miss, 3 incomplete/refused evidence.
It requires the pinned image and intact historical source obligations. The
count floor is removed: `recover check-claims` detects missing original identities,
including same-count swaps. Automatic demotion and weaker-claim grant/withdraw are
removed. Existing obligations and historical notes remain. After interruption,
inspect the source diff and ignored `build/recovery/publications/` backups;
multi-file publication is not globally atomic. See `recovery/README.md`.

## Working rules

`recovery/worker.json` and `recovery/reviewer.json` use the package's native
agent-command bridge; provider arguments live in `recovery/agents.json`.
Verify provider access and isolated worktree/compiler resources before launching.
Synthetic protocol tests are not proof of successful autonomous recovery.

- Prefer real bases, members, globals, virtual methods and lifecycle code over
  vtable arrays, fixed executable addresses, raw self-offset access or stand-ins.
  No `__asm`, `_emit` or copied machine code. Names/types require evidence.
- Preserve cross-function/class discoveries and request scope expansion before
  editing outside the authorized task. Do not discard a discovery as unrelated.
- Do not automatically revert a justified structural migration on a byte
  regression. Preserve its source work and historical obligations for reviewed
  restoration; do not demote claims, weaken gates or label it accepted.
- Source owns essential bindings and structural evidence. Versioned task JSON
  owns non-derivable scope/review/discovery decisions; Git messages explain
  completed changes. Recompute reports or cache them in ignored locations.
  Existing LEVER/TRIED comments are seed history, not new run-log destinations.
- Workers use isolated, coordinator-prepared worktrees/builds/compiler prefixes.
  Only the coordinator runs Git mutations, integration and cleanup. No shared
  stash, unapproved scope changes, or automatic worker fast-forwards.
- One commit per verified step. Preserve other agents' edits and recheck after
  integration. Never discard or reap work that has not been safely preserved.
- Keep proprietary bytes and derivatives in ignored `.opensmacx/` or `build/`.
  Retain dependency licenses/provenance; recognized libraries are verification
  dependencies, not application-function recovery targets.
- Measure with existing facilities first. Remove replaced tools, prompts and
  stale instructions together; retain useful lessons, not obsolete workflows.
