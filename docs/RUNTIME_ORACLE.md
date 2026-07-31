# In-process runtime oracle

The runtime oracle is the differential verification path for functions that are
ineligible for copied-byte oracle execution: methods with calls, absolute
globals, relocations, or process state. Suites run untouched original
functions at their canonical addresses inside the verified hybrid process,
before any redirect is installed, and compare full object state, canary
integrity, virtual-callback traces, watched globals, and return residues
against the source replacements. The hybrid smoke gate refuses to pass unless
every registered suite reports success.

## Architecture

- `src/runtime_oracle.h` / `src/runtime_oracle.cpp`: shared machinery — the
  canary-guarded `Fixture<T>`, the `ClassSpec` descriptor, deterministic pair
  initialization, the vtable `probe` with self-offset/field-snapshot/global
  tracing, the equivalence comparator, the suite table, and the
  `run_runtime_oracles()` dispatcher.
- `src/<class>_oracle.cpp`: one suite per class family (see
  `src/scroll_oracle.cpp` for the reference implementation of sixteen Scroll
  methods).
- `src/dllmain.cpp` calls `run_runtime_oracles()` after signature preflight
  and before installing any redirect, so every original body is still intact
  when it executes.
- Release paths that need the executable CRT run later through
  `run_deferred_oracles()`. Each suite temporarily suspends only its required
  redirect after CRT startup, compares separate real allocations on the
  original and source sides, and restores the redirect before returning.
- `tools/smoke_hybrid_game.py` exports `OPENSMACX_RUNTIME_ORACLE_RESULT` and
  refuses to pass unless the result file lists every suite as `passed` and
  ends with `all passed`.

## Adding a suite for a new class

1. Verify the class layout first: the object size must be pinned by a
   `static_assert(sizeof(...))` in the class header, and the legacy vtable
   size, probed virtual slot, and every object offset holding a vtable copy
   must be confirmed against the canonical disassembly.
2. Create `src/<class>_oracle.cpp` with a `ClassSpec` describing the layout:

   ```cpp
   const size_t RefOffsets[] = {0 /*, further vtable copies */};
   const runtime_oracle::ClassSpec Spec = {
       sizeof(MyClass), /*vtable_bytes=*/0xFC, /*probe_slot=*/0xF8,
       RefOffsets, ARRAYSIZE(RefOffsets),
   };
   ```

3. Write one `verify_*()` per method: build a
   `runtime_oracle::Fixture<MyClass>` pair with
   `runtime_oracle::initialize_pair`, call the original through a raw
   `__thiscall` pointer at its canonical address, call the source
   replacement on the twin fixture, and compare with
   `runtime_oracle::equivalent`. Use `begin_trace`/`current_trace` when the
   method may re-enter through a virtual slot, and
   `set_watched_global` for globals the method reads or writes.
4. Save, sentinel, and restore every touched global exactly as
   `verify_position()` does in the Scroll suite.
5. Export `bool run_<class>_oracle_suite();` from a matching header and
   append `{"<class>", run_<class>_oracle_suite}` to `Suites[]` in
   `src/runtime_oracle.cpp`.
6. Add the object file to `verify-recovery-abi` so the disassembly checks
   confirm the suite calls its original addresses (see the
   `--scroll-oracle-object` block in `tools/verify_recovery_abi.py`).
7. Run the full `verify-recovery-batch` in both presets - `tools/run_gate.py`
   runs the two lanes concurrently; the smoke gate now requires the new suite
   line in the oracle result.

Use a deferred suite for any release branch that needs a real executable-CRT
allocation. Keep resource-free shapes in phase one, register the deferred
entry separately, and suspend/resume only the original function being called.
`src/basebutton_oracle.cpp` is the reference for a close with two independently
optional allocations and a meaningful free-function return residue.

## Result format

```
scroll passed
<suite> passed|failed
all passed
```

The terminal `all passed` line appears only when every suite succeeded;
`tools/smoke_hybrid_game.py` treats anything else as a gate failure.
