# Structure-led recovery implementation

This is the implementation contract and progress record for the independent
`match-recovery` package in `framework/`. The existing recovery tools remain
active until replacement coverage and a measured cutover are demonstrated.
Their continued presence during construction is not a compatibility promise.

## Accepted design

- Identify application, third-party, runtime, system, generated, and unknown
  components before dispatching recovery. Reuse pinned upstream code and system
  bindings; verify dependencies without counting them as application recovery.
- Begin with an ownership/partition pass. Distinguish historical compilation
  units (only when supported by evidence), current source homes, and structural
  task scope. A unit owns a collection of original code/data spans, not a single
  contiguous executable range. Propose units from source/class/component evidence;
  leave uncertain assignments open instead of inventing a file per function.
- Use classes, object ownership and dependent functions as work units; use
  functions/spans as measurement units. Real C++ structure takes priority over
  locally convenient source forms. VC6 remains the OpenSMACX compiler target.
- Source owns the load-bearing mappings, symbols, obligations, structural
  decisions and provenance needed to understand its relationship to the image.
  Ordinary C++ expresses relationships wherever possible; annotations must not
  duplicate what declarations and definitions already express. Names and source
  paths are not ids.
- Recompute anything derivable from the authoritative inputs. Current match
  verdicts, censuses, call/dependency graphs, parsed mappings, context cards and
  stale/fresh status are outputs, not durable records. Persist them only as
  disposable, input-keyed caches; deleting every cache must preserve correctness
  and obligations. Historical observations never become current verdicts.
- Keep all lasting recovery information in Git-readable source or text records.
  No SQLite authority: the uncommitted prototype was replaced by hash-chained JSON
  events with atomic publication and ignored kernel-lock files. A fresh checkout
  can read the history without a database or private artifact cache.
  Versionability alone is not a reason to persist an output: tracked JSON is for
  non-derivable workflow decisions and checkpoints, not derived reports.
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

## Persistence boundary

Store inputs and decisions; compute conclusions. Essential image/source bindings,
accepted layout interpretations and restoration obligations belong beside their
owning source declarations or definitions. A required match is an obligation,
not a stored assertion that the current source matches. The gate computes the
current result, including during an accepted structural migration.

Tracked workflow records may preserve agent/human hypotheses, externally supplied
hints, approved scope, review decisions and choices needed to resume work. Promote
accepted load-bearing discoveries into the source instead of leaving JSON as a
second authority. Use Git messages for completed-change rationale, not as the only
home of an outstanding obligation. Do not copy source bodies, parsed inventories
or reproducible measurement summaries into durable history.

Cache keys must cover every relevant source, image, toolchain, configuration and
analysis-version input. A cache miss triggers recomputation; unavailable required
inputs produce an explicit unmeasurable result, never a cached claim of success.
External hints and execution decisions are not reproducible merely because an
agent could be asked again. Preserve those inputs, not duplicate conclusions that
can be regenerated from them.

## Delivery checkpoints

Each unchecked item remains required; passing a narrow unit suite does not
establish completion of this program.

- [ ] Reproducible baseline: source census, independent build, full gate, timings,
  existing test failures, component inventory and unknown ownership.
- [x] Core identity/evidence/dependency/migration model and adversarial tests.
- [ ] Minimal non-derivable knowledge/decision checkpoints and strict schema I/O;
  derived outputs only in disposable caches (storage primitives implemented;
  derived import history removed; legacy parse snapshots are ignored caches).
- [ ] One-time lossless legacy annotation/seed import and compact source writer.
- [ ] Component manifests and dependency verification; zlib, CRT and system pilot.
  Zlib's 39 original bindings now live in a source companion header, with pinned
  archive source verification and bounded relocation-aware binary checks. Missing
  data/function bindings, ABI checks and the other components remain.
- [ ] Relocation-target and original-boundary verification; wrong-global fixture.
  The portable comparator and PE/COFF target reader reject wrong/missing targets,
  hardcoded absolute addresses and undeclared extra code. Synthetic damage tests
  and the zlib integration run; application-gate cutover and ownership checks remain.
- [ ] Declarative scheduler, discovery consumers, context bundles, agent adapters,
  independent review, safe collection and resumable migrations.
  Ownership partitions and conservative include-impact views are derived live;
  task scope, cross-function discoveries, coordinator expansion and explicit
  resolution are versioned decisions. Task-scoped binary checks now execute
  through a shared strict application/dependency verifier, compiling once per
  selected TU/flag set without stamping claims. Dispatch, proof-driven stage transitions,
  independent review execution and collection remain under construction.
- [ ] InfoWin ownership and constructor-family end-to-end structural pilots.
  The redraw slot is now an InfoWin member instead of independent storage;
  its object-start binding lives beside ConsoleInfoWin's definition. Constructor
  subobjects, object extent, lifetime, full admission and restoration remain.
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
- The private prefix's empty bootstrap hung and was stopped by its exact owned
  process id. Its incomplete directory was preserved for diagnosis. A separate
  copy of the known-working prefix configured successfully; registry hashes
  agreed and distinct inodes/link counts confirmed no hardlinks to the template.

## Rebase and compiler baseline (2026-09-06)

- Clean-checkpoint rebase incorporated master 8919cb7d (17 functions homed);
  framework foundation became 12ea6b40. No integration conflict.
- Independent configure: 127 build inputs. Full unpiped old gate: exit **3**,
  2,395 total claims, 2,143 byte-verified, 38 instruction-shape matches,
  0 regressions, 214 unverifiable claims (186 NO_COMPILE, 28 UNRESOLVED).
  All 214 are artifact-only claims. This is not a green baseline, despite the
  old tool's final wording `OK, with unverifiable claims present`.
- The old audits also report 11 symbol-name discrepancies and skip an image-data
  audit because it does not find the image in this independent worktree. Those
  integration gaps remain required work; a successful byte sub-check does not
  stand in for them.
- Combined foundation/offline coverage tests: 140 passed, 2 skipped. After durable
  journal and content fingerprint additions: 58 framework tests pass.
- Runtime task execution, scoped migration checkpoint manifests, current binary
  verification and tool retirement are not yet complete.
