# Recovery inventory

The recovery inventory makes the annotated IDA database, current source coverage, redirects, and
remaining fixed-address dependencies comparable by original virtual address. It is the baseline for
finishing the decompilation; historical source `Status:` labels are evidence, not verification that
an implementation is behaviorally correct.

## Generate the inventory

Install the pinned tools and run the exporter from the repository root:

```sh
python3 -m pip install -r tools/requirements.txt
python3 tools/export_recovery_inventory.py
```

The hash-pinned annotated IDB is a local proprietary analysis input. Place the verified database at
`.opensmacx/analysis/terranx_ORIG_200_v3_7.5.SP3.idb`, or pass its ignored local path with `--idb`.
The exporter rejects any database whose SHA-256 is not explicitly supported. Never commit or
distribute an IDB because it retains original executable bytes.

All tools that write proprietary or derived local artifacts restrict their outputs to nonsymlinked
descendants of `.opensmacx/` or `build/`. The checks reject both direct symlinks and symlinked parent
components so a nonexistent output leaf cannot escape through an existing link.

The command deterministically writes:

- `docs/recovery/functions.csv`: all IDA functions, types, classifications, source evidence, and
  call-graph degree counts.
- `docs/recovery/callgraph.json`: function-to-function call edges and their original call sites.
- `docs/recovery/summary.json`: input hashes, coverage counts and their byte weighting, unresolved
  matches, and all active fixed-address source bindings.

Read the `functions.bytes` block before any count in that file. A count of functions flatters every
coverage claim here, because the recovered functions are the small ones - 40% of the functions has
been 8% of the bytes. The block states the denominator it uses (the **lift scope**: 5,673 functions
and 2,410,317 bytes), states the exclusion that produces it from the 6,000 catalogued rows
(`recovery_state == external_library`, the CRT and Windows library code that `tools/lift_whole_image.py`
calls and never translates), and publishes `machine_carried` - the bytes whose behaviour is still
supplied by machine-derived code rather than recovered source. That last figure is the only one that
falls when the project succeeds; every other number in the file rises.

The local IDB remains the primary analysis database because it contains the retained community names,
prototypes, and comments. Ghidra should preferably analyze the exact same executable and map its
functions to this inventory by virtual address. The correlation summary records executable hashes
and labels mismatched inputs as `cross_build`; that evidence is useful for shared entry points, but
it is not proof of identical function bodies.

The call graph uses Capstone to decode the original bytes stored in the IDB. Direct calls to a
function start or one of its interior addresses are mapped to that function; interior targets are
retained separately for review. Indirect calls are also listed separately because their runtime
target cannot be established statically from the call instruction alone.

## Classifications

`binary_kind` describes IDA's view of the original function:

- `game`: code not marked as a library function or thunk.
- `library`: compiler or runtime library code identified by IDA.
- `thunk`: a forwarding function identified by IDA.

`recovery_state` describes repository evidence, with source-address evidence taking precedence:

- `source_complete`: a matching `Original Offset:` has a historical status beginning with
  `Complete`.
- `source_in_progress`: a matching source annotation exists but is not marked complete.
- `redirect_name_match`: an export name exactly matches the IDA function name, without a matching
  source address annotation.
- `original_dependency`: current source still calls this original function by fixed address.
- `external_library`: no source evidence exists and IDA classified the function as library code.
- `thunk`: no source evidence exists and IDA classified the function as a thunk.
- `unrecovered`: no current repository evidence maps to the game function.

Use `docs/recovery-overrides.csv` for reviewed exceptions and prioritization. Addresses must be IDA
function starts. Regeneration fails on duplicate or unknown override addresses so manual decisions
cannot silently drift after database changes.

## Independent analysis

Ghidra 12.1 or newer can analyze the original executable and export address-keyed metadata:

```sh
python3 tools/run_ghidra_analysis.py --exe /path/to/terranx.exe
python3 tools/export_idb_members.py
python3 tools/correlate_recovery_analyses.py
```

The correlation step writes `analysis-correlation.csv`, `analysis-summary.json`, and
`priorities.csv` under `docs/recovery`. Boundary relationships are recorded rather than reconciled
automatically: `exact` has the same complete body ranges, `entry_range` shares the entry point and
primary range but differs in tails, and `start_only` shares only the entry point. `containing` and
`split` indicate a primary-range disagreement, while `missing` means no primary-range overlap was
found.

Ghidra also exports `ghidra-interior-references.csv`, containing only references from outside a
function to one of its interior addresses. Local assembly-island extraction uses this evidence to
reject bodies that have externally reachable secondary entries. Its hash is bound into
`analysis-summary.json` with the Ghidra function inventory.

### External analysis leads

`docs/recovery/external-analysis-sources.json` records immutable identities for useful historical
analysis notes that are not licensed for redistribution. These artifacts are hypothesis sources,
not authoritative metadata or behavioral oracles. Do not commit their prose, generated
disassembly, game text, or derived assembly.

Fetch the exact cataloged snapshots into ignored local storage, then correlate either the Yitzi
function notebook or Dio address labels:

```sh
.opensmacx/venv/bin/python tools/fetch_external_analysis.py
.opensmacx/venv/bin/python tools/correlate_external_analysis.py \
  --source-path Information/Yitzi/functionlisting.txt
.opensmacx/venv/bin/python tools/correlate_external_analysis.py \
  --source-path "Information/Dio/Label addresses in assembly code.txt"
.opensmacx/venv/bin/python tools/build_export_recovery_queue.py
```

The tools validate cataloged sizes and SHA-256 values. Correlation outputs contain only source line
numbers, addresses, canonical function identities, and per-function lead counts under ignored
`.opensmacx/external-analysis/`; each summary is bound to its cataloged source path and hash. Every
useful lead must still be verified independently against the hash-bound canonical executable before
source, tests, or metadata are committed.
The exported-first queue combines reviewed recovery state, canonical call counts, and local external
lead density while keeping all generated correlations ignored.

The raw IDC, annotated IDB, and IDA 9 `.i64` database remain local. The normalized
`ida9-functions.csv` retains only function boundaries, flags, names, prototypes, and body ranges
needed for reproducible correlation. Committed recovery metadata is hash-bound to the local analysis
inputs without distributing those databases.

The priority score puts all live original-function bindings first, including bindings whose source
annotation takes precedence in `recovery_state` and CRT/library bindings. It then ranks unrecovered
game functions while excluding ordinary library and thunk code.
Canonical callers are weighted more heavily than callees. Analyzer agreement contributes only a
small confidence bonus and cannot outweigh runtime impact.

Every live function binding must have an entry in `docs/recovery-binding-classifications.csv`.
Correlation fails when a binding is unclassified, keeping CRT ownership, platform services, timer
callbacks, source fallbacks, and unrecovered game services visible as distinct migration work.

Before staging a recovery batch, regenerate the catalogs and compare them with the committed state:

```sh
cmake --build --preset mingw-i686-release --target verify-recovery-metadata
```

The target regenerates all canonical and correlation outputs under the ignored build directory and
fails on any difference. Verified runs are stamped in `build/recovery-metadata-cache/` keyed on the
content hashes of the complete analysis/tool input closure, the committed catalogs, and the exact
source-metadata projection consumed by the exporter (address/status annotations, fixed-address
bindings, and their locations). Ordinary implementation, test, and runtime-oracle edits therefore
reuse the verified catalog result, while any catalog-relevant source edit invalidates it. The
ignored per-preset verification directory also checkpoints the expensive canonical IDB export
against its binary-analysis input closure and binary-only projections of `functions.csv` and
`summary.json`, plus the complete `callgraph.json`. Source annotations, fixed-address bindings, DEF
redirects, and recovery overrides are deliberately outside this expensive checkpoint: the verifier
reapplies them to the checkpointed 6,000-function inventory before rerunning correlation and the
final byte comparisons. A catalog-relevant source edit, classification failure, or promotion of
regenerated catalogs therefore invalidates the strict verification stamp without repeating the
multi-minute IDB parse. Binary inventory drift still invalidates the checkpoint, and
`tools/verify_recovery_metadata.py --force` discards it and regenerates unconditionally.
`prepare-hybrid-image` depends on this check. The local-only umbrella target
also runs behavioral tests, ABI checks, differential oracles, island regeneration, staging, and the
runtime smoke gate:

```sh
cmake --build --preset mingw-i686-release --target verify-recovery-batch
```

Run the source and ABI portions in both Debug and Release; the distributable build remains independent
of the local IDB, original executable, and generated oracle objects. Both presets at once is the
normal way to do that, since the two lanes share no writable state:

```sh
.opensmacx/venv/bin/python tools/run_gate.py
```

Measured on this tree, 332.44 s serial against 190.11 s concurrent (1.75x) with identical verdicts.
`--preset <name>` runs one lane alone; `--serial` runs both in sequence.

Functions recovered after the legacy import table was fixed can be redirected at process attach.
Each runtime redirect validates an original-code byte signature before installing an x86 relative
jump. A signature mismatch fails DLL loading; this guards the patched function but is not a
substitute for validating the complete executable hash during staging.

## Hybrid image input

The recovery inventory can be projected onto a supported user-owned executable as a deterministic
local image pack:

```sh
.opensmacx/venv/bin/python tools/prepare_hybrid_image.py
```

The command accepts the canonical IDB input or the executable independently analyzed by Ghidra. It
rejects other executable hashes because matching virtual addresses alone do not prove matching
function bodies. For the Ghidra build, `legacy-functions.json` uses Ghidra body ranges only where
the analyses share an entry point; candidates with ambiguous or missing correlations receive no
byte hash. See `docs/HYBRID.md` for the artifact contract and distribution boundary.
