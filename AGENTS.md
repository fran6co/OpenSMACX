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

## Commands and current acceptance boundary

Everything Python runs through `uv run`, never bare `python3`.

```sh
uv run recover inventory
uv run recover partition
uv run recover tasks context <task>
uv run recover tasks obligations <task>
uv run recover tasks check <task> --image <image> --summary
uv run recover tasks check-restoration <task> --image <image> --summary
uv run recover check-build --summary
uv run recover run <task> --worker <config> --reviewer <config>
uv run pytest packages/match-recovery/tests recovery/tests
cmake --build build
uv run tools/osmx.py check
```

The last command is the retained full legacy gate: never pipe it. Exit 0 is
clean, 1 a failure, 3 unmeasurable—not green. This project's strict full
acceptance and migration-check adapters are unfinished. Scoped checks and
reviews do not establish full acceptance or a working game.

Until the stamping command migrates, only `uv run tools/osmx.py record <addrs>`
banks measured legacy claims. Never hand-edit an `ORIGINAL:` marker or type a
`BYTE_EXACT` token. Those claims retain restoration obligations; they are not
proof of current structure or relocation-target correctness.

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
