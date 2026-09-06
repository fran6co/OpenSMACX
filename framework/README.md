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
uv run recover import legacy --image /path/to/the/pinned/original.exe
uv run recover history
uv run pytest framework/tests
uv build --package match-recovery
```

Reports are JSON on stdout. Errors go to stderr and return exit 3. Inventory is
read-only. Import stores a content-addressed snapshot under the selected root's
ignored `.opensmacx/recovery/objects/` directory and prints its id. It does not
rewrite source, declare current matches, or choose between conflicting source
candidates. Raw files preserve all notes, including unparsed or non-UTF-8 text.
Each import also publishes a durable, idempotent JSON event under the versioned
`recovery/history/` directory. `history` reads these events without reparsing
source; reading absent history creates nothing. No database is required.

The text journal uses atomic, revision-checked appends. Concurrent stale writers
are refused instead of overwriting discoveries, and retrying the same request
does not duplicate it. Payload corruption and missing event positions are errors.
Input fingerprints use file contents and explicit configuration, never timestamps
alone; equivalent inputs in another worktree produce the same fingerprint.
Cross-cutting hypotheses, migration checkpoints and dependency manifests belong
in versioned text alongside source. Raw compiler output and binary derivatives
stay ignored. An optional future index must be rebuildable from these records.

The policy API distinguishes byte/instruction-shape, reference, structure and
runtime obligations. `check_obligations` returns 0 only if all required current
measurements pass. Missing/stale measurements return 3; unexpected failures
return 1; explicitly admitted matching regressions return 2, never green.
An independent review of the exact migration snapshot and passing build and
structural checks are prerequisites for admission. Closing requires restoration
of every protected obligation.

## Still under construction

The package is not yet the production gate. The durable task runner, target
verification, component manifests, source annotation migration and performance
infrastructure are tracked in `docs/recovery/FRAMEWORK.md` at the repository
root. No compatibility wrappers will be retained at cutover. Removed tools'
useful verification responsibilities must be replaced, not silently dropped.
