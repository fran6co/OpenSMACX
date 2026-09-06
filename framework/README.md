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
uv run recover components list
uv run recover components check-source zlib --archive /path/to/zlib-1.0.2.tar.gz
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

The policy API distinguishes byte/instruction-shape, reference, structure and
runtime obligations. `check_obligations` returns 0 only if all required current
measurements pass. Missing/stale measurements return 3; unexpected failures
return 1; explicitly admitted matching regressions return 2, never green.
An independent review of the exact migration snapshot and passing build and
structural checks are prerequisites for admission. Closing requires restoration
of every protected obligation.

## Still under construction

The package is not yet the production gate. The durable task runner, target
verification, complete dependency checking, source annotation migration and performance
infrastructure are tracked in `docs/recovery/FRAMEWORK.md` at the repository
root. No compatibility wrappers will be retained at cutover. Removed tools'
useful verification responsibilities must be replaced, not silently dropped.
