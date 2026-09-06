# Structure-led recovery implementation

This is the implementation contract and progress record for the independent
`match-recovery` package in `framework/`. The existing recovery tools remain
active until replacement coverage and a measured cutover are demonstrated.
Their continued presence during construction is not a compatibility promise.

## Accepted design

- Identify application, third-party, runtime, system, generated, and unknown
  components before dispatching recovery. Reuse pinned upstream code and system
  bindings; verify dependencies without counting them as application recovery.
- Use classes, object ownership and dependent functions as work units; use
  functions/spans as measurement units. Real C++ structure takes priority over
  locally convenient source forms. VC6 remains the OpenSMACX compiler target.
- Separate compact source mappings/obligations, dependency manifests, durable
  knowledge, and ignored run artifacts. Historical observations never become
  current verdicts merely because they exist. Names and source paths are not ids.
- Import old source, Kuna/Ghidra/IDA output, notes and prior experiments as seeds
  with provenance, including conflicting interpretations. Do not copy scaffolds
  into production source as a mandatory rehoming step.
- Admit scoped structural migrations through independent review and deterministic
  checks even with tracked matching regressions. Keep every old obligation;
  checkpoint on budget exhaustion, close only after restoration. A verified
  revision advances only on a full passing gate.
- Verify relocation targets, boundaries, layouts, lifetimes and bindings rather
  than accepting masked-byte agreement as structural or runtime correctness.
- Package core/target/provider/agent contracts independently. Declarative tasks
  own context, scope, discoveries, budgets, acceptance and durable resumption.
- Consolidate measurement, compile once per affected TU/configuration, lease
  isolated Wine prefixes, and use content/dependency-aware caches and candidate
  decks. Measure throughput, do not assert it.
- Preserve original and proprietary derivatives only in ignored artifact storage.
  Preserve upstream license/provenance and keep vendor sources unmodified.

## Delivery checkpoints

Each unchecked item remains required; passing a narrow unit suite does not
establish completion of this program.

- [ ] Reproducible baseline: source census, independent build, full gate, timings,
  existing test failures, component inventory and unknown ownership.
- [x] Core identity/evidence/dependency/migration model and adversarial tests.
- [ ] Versioned knowledge and durable run/checkpoint storage, strict schema I/O.
- [ ] One-time lossless legacy annotation/seed import and compact source writer.
- [ ] Component manifests and dependency verification; zlib, CRT and system pilot.
- [ ] Relocation-target and original-boundary verification; wrong-global fixture.
- [ ] Declarative scheduler, discovery consumers, context bundles, agent adapters,
  independent review, safe collection and resumable migrations.
- [ ] InfoWin ownership and constructor-family end-to-end structural pilots.
- [ ] Prefix leases, include/parse/measurement caches, candidate decks, benchmarks.
- [ ] Quota/interruption recovery and multiple-checkpoint restoration tests.
- [ ] Tool/documentation cutover, stale-reference audit, removal of superseded live
  workflows, preservation of retirement history and incoming recoveries.
- [ ] Mixed recovery batches, staged runtime tests, alternate-target fixtures.

## Isolation and integration

Work on `framework/structure-led-recovery` in the locked sibling worktree, not
the live rehoming checkout. Use separate build outputs, caches and Wine prefixes;
only the pinned image and immutable toolchain inputs may be shared read-only.
Do not run the old automatic fast-forward setup against this divergent branch.

At clean commit checkpoints, inspect and rebase on local `master` to incorporate
other agents' committed changes. Never stash or discard work to force a rebase.
Resolve integration conflicts explicitly, rederive affected inventories/context,
and rerun applicable tests. Do not collect/reap other agents or merge this work
back into `master` automatically.

## Initial evidence (2026-09-06, base 815433a4)

- New core/import tests: 44 passed before the baseline test repair.
- Standalone framework wheel and source distribution build successfully.
- Legacy import: 4,776 annotations, 4,772 original entry-point identities,
  2,333 source/header files, 2,072 legacy-seed candidates, 2,704 other source
  candidates, 2,353 historical byte obligations and 41 instruction-shape claims.
  The original image SHA-256 matches the pin. These are historical claims,
  not a completed compiler re-verification.
- Initial legacy offline tests: 93 passed, 2 skipped, 2 failed. Both failures
  were stale population thresholds (>3,000 files, >5,000 records), replaced by
  independent marker-site coverage checks rather than lowered constants.
- Existing stale-reference audit: exit 0, 286 files scanned.
- Full isolated compiler/build/gate baseline remains outstanding.
