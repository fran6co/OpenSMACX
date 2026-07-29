# Static recompilation

Static recompilation is a local verification strategy, not a source-recovery state. Its generated
C++, oracles, objects, manifests, and executables are mechanically derived from the original
executable and must remain ignored under `build/` or `.opensmacx/`. Nothing it produces may be
committed, distributed, marked `source_complete`, linked into `OpenSMACX.dll`, or counted as
recovery progress.

The project now lifts the WHOLE IMAGE. `tools/lift_whole_image.py` mechanically translates every
catalogued function into explicit-CPU-state C++ - 99.987% of instructions - and
`tools/lifted_oracle_*` verifies the result differentially: the original function runs for real at
its canonical address under Wine, beside the lifted version, over sixteen seeded states per
function.

## Scope, and the one denominator

The catalogue holds 6,000 functions. The lift plans 5,673 of them, because `lift_whole_image.py`
excludes `external_library` rows by written policy: CRT and Windows library code that the lift must
call and must never translate. That exclusion is 327 functions and 45,155 bytes.

Every percentage this project publishes about the lift is over the remainder - the **lift scope**,
5,673 functions and 2,410,317 bytes - and the exclusion is printed or serialised beside the figure
wherever it appears. `tools/recovery_metrics.py` is the only place that decides this, and
`tools/test_recovery_metrics.py` holds it to it.

Bytes lead, counts follow. This project's defining measurement error was reporting functions: 40% of
the functions was 8% of the bytes, because the recovered functions are small. A verdict on a few
large functions outranks a verdict on many small ones, and only the byte column says so.

## Run it

```sh
tools/lifted_oracle_build.sh          # build the harness
tools/lifted_oracle_sweep.sh          # sweep the plan (hours; use --only while iterating)
python3 tools/lifted_oracle_summary.py [--json build/oracle/summary.json]
```

The sweep's own end-of-run tally covers only the last segment: every counter resets when the sweep
restarts the process after a hang. `build/oracle/report.tsv` is the only whole-image record, so
`lifted_oracle_summary.py` is where the whole-image numbers come from.

### The plan's blocking flags are a hint, not a refusal

The flags are computed statically and propagated along every call edge with no reachability filter,
so `iat` means "some path COULD reach an import", not "a run DID". The driver therefore ATTEMPTS a
flagged function by default and lets the run decide, producing `SKIP-reached-blocked` when a seed
really walked into the construct and `PASS-paths-taken` when every judged seed agreed and none did.
`PASS-paths-taken` is **never** added to `compared and agreed`; it gets its own line under the
numbers that must go down. `--refuse-blocked` restores the old refusal and can only under-count.

Two costs of that default, both measured and neither yet paid down:

* On the 470 `extcall`-only functions the yield was 55 `PASS-paths-taken` (19,876 bytes, 26.7% of
  the cohort), below the ~30% gate the change set itself.
* On this arm64 host, some previously-unexecuted `iat` functions kill the process outright with
  `rosetta error: unsupported privilege level: 0` - no guest fault, no guard, no report row. Over
  the first 40 `iat` functions, 5 die that way and the other 35 come back 30 `PASS-paths-taken` and
  5 `INCONCLUSIVE-original-fault`. The sweep records those five as `KILLED-host-refused`, not
  `HANG`, because the watchdog never observed anything. Each one still costs a watchdog period and
  a process restart, so a whole-image sweep under the default is materially slower than one with
  `--refuse-blocked`.

## What the summary publishes

* The byte-weighted verdict table, over the lift scope.
* `swept` - how much of the scope the report actually covers. A sweep that died half way through
  used to print percentages of the part it finished, and they read exactly like percentages of the
  image; the denominator is now the catalogue and the shortfall is stated.
* `compared and agreed`, and immediately under it the FULL-STRENGTH share.
* The evidence tiers, which partition the agreeing bytes: `full-strength` (every attempted seed
  compared, nothing thrown out), `seeds-incomplete`, `arbitrated` (cases discarded because the
  original read the unmodellable top 64 KiB), and `single-seed`.
* Three numbers that must go DOWN: bytes still carried by machine-derived code, bytes the oracle has
  never compared, and bytes whose agreement was obtained under a weakened condition.

A PASS carries its own evidence quality, because byte-weighted the qualification is not a footnote.
Measured on the current report: 1,465 functions agree, 178,248 bytes, 7.40% of the scope - and only
767 of them, 42,236 bytes, 1.75% of the scope, compared every seed they attempted with nothing
thrown out. 76.30% of the agreeing bytes rest on weakened evidence. By function count the
full-strength share looks like 52.4%; by bytes it is 23.70%, because the functions whose original
side could not be run on most seeds are the large ones.

`docs/recovery/summary.json` carries the same denominator and the same exclusion in its
`functions.bytes` block, together with `machine_carried` - the recovery debt, 2,214,194 bytes /
91.86% of the scope on 2026-07-29, which falls only when a function becomes `source_complete`.
That pair drifts as recovery lands; the JSON is the generated source of truth, not this sentence.

## History: the abandoned one-function pilot

Before the whole-image lift there was a time-boxed micro-pilot that lowered exactly one leaf,
`Wave_Device::is_group_disabled` at `0x004C5460`, with no instruction decoder, EIP dispatcher, or
general memory mapper. It passed in Debug and Release: deterministic fresh regeneration, complete
source/oracle provenance, `pe-i386` and `thiscall` ABI checks, standalone execution, and
differential fixtures over full `EAX`, partial `EDX`, defined flags, stack movement, object memory,
and canaries.

**Its value gate failed, and the reasoning is worth keeping.** Passing a leaf validated only the
lowering mechanism, and for that function the existing legacy-island path was simpler and exact - so
the generated implementation provided no capability the project did not already have. The follow-up
candidate, the call-bearing `Wave_Device::enable` at `0x004C51C0`, was inspected but never lifted:
policy requires its indirect call to execute at the original address in a verified process, canonical
addresses cannot be projected into the PRACX runtime, and linking ignored generated code into a
separate canonical-process injection DLL would have been more machinery than directly recovering a
15-byte function. The pilot was therefore stopped rather than expanded.

What changed was the last clause of that gate: "unless a future independently needed
canonical-process harness changes that cost". The differential oracle is exactly that harness. It
runs the original at its canonical address in a real process under Wine, which is the capability the
pilot could not justify building for one function and which is worth building once for 5,673. The
per-function verdict is therefore no longer "does this duplicate an island" but "does the lifted
body agree with the original", and the answer is measured rather than argued.

The pilot's machinery is still in the tree and still builds - `tools/static_recompile_pilot.py`,
`tools/static_recompile_runtime.h`, `tools/verify_static_recompile_pilot.py`, and the
`verify-static-recompile-pilot` target:

```sh
cmake --build --preset mingw-i686-debug   --target verify-static-recompile-pilot
cmake --build --preset mingw-i686-release --target verify-static-recompile-pilot
```

It is retained as a narrow, fast, self-contained check of the lowering idea and of the ownership
rules, not as the project's direction. Nothing in the whole-image path depends on it, and its output
is still local-only and still not recovery progress. What survives it beyond the code is the rule
that generated code is never recovery progress, and the habit of writing the value gate down before
running the experiment rather than after reading the result.
