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

The command deterministically writes:

- `docs/recovery/functions.csv`: all IDA functions, types, classifications, source evidence, and
  call-graph degree counts.
- `docs/recovery/callgraph.json`: function-to-function call edges and their original call sites.
- `docs/recovery/summary.json`: input hashes, coverage counts, unresolved matches, and all active
  fixed-address source bindings.

The IDB itself remains the primary analysis database because it contains the retained community
names, prototypes, and comments. Ghidra should preferably analyze the exact same executable and map
its functions to this inventory by virtual address. The correlation summary records executable
hashes and labels mismatched inputs as `cross_build`; that evidence is useful for shared entry
points, but it is not proof of identical function bodies.

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
python3 tools/export_idc_functions.py --idc /path/to/ida-export.idc
python3 tools/correlate_recovery_analyses.py
```

The correlation step writes `analysis-correlation.csv`, `analysis-summary.json`, and
`priorities.csv` under `docs/recovery`. Boundary relationships are recorded rather than reconciled
automatically: `exact` has the same complete body ranges, `entry_range` shares the entry point and
primary range but differs in tails, and `start_only` shares only the entry point. `containing` and
`split` indicate a primary-range disagreement, while `missing` means no primary-range overlap was
found.

The raw IDC and IDA 9 `.i64` database remain local. The normalized `ida9-functions.csv` retains
only function boundaries, flags, names, prototypes, and body ranges needed for reproducible
correlation. The older annotated IDB remains checked in because it is the canonical recovery source.

The priority score puts all live original-function bindings first, including bindings whose source
annotation takes precedence in `recovery_state` and CRT/library bindings. It then ranks unrecovered
game functions while excluding ordinary library and thunk code.
Canonical callers are weighted more heavily than callees. Analyzer agreement contributes only a
small confidence bonus and cannot outweigh runtime impact.

Every live function binding must have an entry in `docs/recovery-binding-classifications.csv`.
Correlation fails when a binding is unclassified, keeping CRT ownership, platform services, timer
callbacks, source fallbacks, and unrecovered game services visible as distinct migration work.
