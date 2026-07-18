# Static recompilation micro-pilot

Static recompilation is an optional local recovery experiment, not a source-recovery state. Its
generated C++, assembly oracle, objects, manifests, and executables are mechanically derived from the
original executable and must remain ignored under `build/` or `.opensmacx/`.

The pilot currently targets `Wave_Device::is_group_disabled` at `0x004C5460`. It statically lowers a
small reviewed x86 subset into explicit CPU-state C++ basic blocks. There is no instruction decoder,
EIP dispatcher, general memory mapper, production redirect, or dependency from `OpenSMACX.dll`.

Run the complete pilot in both configurations:

```sh
cmake --build --preset mingw-i686-debug --target verify-static-recompile-pilot
cmake --build --preset mingw-i686-release --target verify-static-recompile-pilot
```

Each target regenerates a one-function original-byte oracle from the hash-pinned canonical
executable, regenerates the C++ translation, compiles a standalone generated-only executable, and
compares the generated core with the original implementation under Wine. The comparison covers full
`EAX`, partial-register `EDX` behavior, defined status flags, stack cleanup, object memory, and
canaries. The generated C++ boundary is explicitly a result-only `thiscall` adapter: exact volatile
register and flag comparisons apply to the explicit-state core, not the compiler-generated adapter.
ABI verification requires i386 COFF, a `thiscall` `ret 4`, no original symbol or address dependency,
complete source/oracle hashes, and byte-identical fresh regeneration.

Generated output is published under
`build/<preset>/static-recompile-pilot/`. It must never be committed, distributed, marked
`source_complete`, linked into `OpenSMACX.dll`, or counted as recovery progress.

## Value gate

Passing the leaf pilot validates only the lowering mechanism. The existing legacy-island path is
simpler and exact for this function. Expansion is justified only if a second experiment can handle a
call-bearing function, starting with `Wave_Device::enable` at `0x004C51C0`, without adding runtime
instruction decoding, a general emulator, unsafe address projection into PRACX, or distributable
derived code. Otherwise the experiment stops and ordinary source recovery remains the strategy.

## Outcome

The leaf milestone passes in Debug and Release, including deterministic regeneration, provenance and
ABI checks, standalone execution, and differential CPU-state fixtures. The value gate does not pass:
the generated implementation provides no capability beyond the simpler exact island already
available for this function.

The call-bearing `Wave_Device::enable` candidate was inspected but not lifted. Project policy requires
its indirect call to execute at the original address in a verified process. Canonical addresses
cannot be projected into the PRACX runtime, and linking ignored generated code into a separate
canonical-process injection DLL would add more machinery than directly recovering this 15-byte
function. Do not expand the pilot unless a future independently needed canonical-process harness
changes that cost or a new target demonstrates a concrete capability unavailable from islands and
the existing hybrid.
