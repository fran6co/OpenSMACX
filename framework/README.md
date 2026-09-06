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

Reports are JSON on stdout. Errors go to stderr and return exit 3. Inventory is
read-only. Import stores a content-addressed snapshot under the selected root's
ignored `.opensmacx/recovery/objects/` directory and prints its id. It does not
rewrite source, declare current matches, or choose between conflicting source
candidates. Raw files preserve all notes, including unparsed or non-UTF-8 text.
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

```sh
uv run recover tasks create example-layout --group class:InfoWin --author coordinator \
  --intent "Recover InfoWin storage ownership" --evidence "source:src/infowin.h"
uv run recover tasks status example-layout
uv run recover tasks context example-layout
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
Data/import bindings are not yet imported, so references to those stay unknown.

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
It writes no context file. A coordinator can use the output directly or cache it
in ignored storage; neither a stale brief nor a resolved hypothesis is a verdict.

## Still under construction

The package is not yet the production gate. The durable task runner, target
verification, complete dependency checking, source annotation migration and performance
infrastructure are tracked in `docs/recovery/FRAMEWORK.md` at the repository
root. No compatibility wrappers will be retained at cutover. Removed tools'
useful verification responsibilities must be replaced, not silently dropped.
