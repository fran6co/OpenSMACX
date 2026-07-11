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
names, prototypes, and comments. Ghidra should analyze the same executable independently and map
its functions to this inventory by virtual address. Side-by-side decompiler output can then be
reviewed without discarding the IDA annotations.

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
