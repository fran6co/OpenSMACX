# Local legacy islands

Legacy islands provide local x86 implementations while source recovery moves through the call
graph. They are generated from a user-owned executable, never checked in, and never eligible for
distribution. Their purpose is to make every eligible unresolved leaf an explicit linker symbol
that can be replaced by recovered source without introducing copied assembly into the repository.

## Generate

Install `tools/requirements.txt`, prepare the supported local executable, and run:

```sh
.opensmacx/venv/bin/python extract_legacy_leaves (retired)
```

The generated `.opensmacx/legacy-leaves/` directory contains:

- `legacy-leaves.S`: local assembly with exact machine bytes.
- `symbols.csv`: address, name, size, source offset, and byte hash for every island.
- `manifest.json`: extraction criteria, rejection counts, source identity, and recovery debt count.

The MinGW CMake build can also assemble the generated source into an i386 COFF object:

```sh
cmake --build --preset mingw-i686-release --target assemble-legacy-leaves
```

The object is written to the active CMake binary directory as `legacy-leaves.obj`. Every function
has a separate COFF section and stable address-derived symbol. It contains no COFF relocations
because exact bytes are emitted with `.byte`; this does not prove references are absent and prevents
the linker from repairing any missed reference. The extractor therefore rejects reference-bearing
candidates before emission. The object remains architecture- and ABI-specific and proprietary.

Extractor output is restricted to ignored subdirectories of `.opensmacx/` or `build/`. An explicit
`--address` selects an eligible canonical function even after it is marked source-complete, allowing
the local implementation to remain a differential oracle without restoring it to the ordinary
legacy-island inventory. The CMake `verify-recovery-oracles` target builds and runs the currently
configured source-versus-original fixtures:

```sh
cmake --build --preset mingw-i686-release --target verify-recovery-oracles
```

The generated assembly, object, manifest, and executable remain local-only and must not be committed
or distributed.

## Conservative eligibility

An unrecovered game function is accepted only when all of these conditions hold:

- Canonical and Ghidra analyses have exactly matching boundaries.
- The body is one contiguous, file-backed range no larger than the configured limit.
- Capstone decodes every byte.
- No other Ghidra entry point or PE relocation falls inside the body.
- There are no calls, indirect branches, external branches, system instructions, or segment memory.
- There are no absolute memory operands or immediate values inside the PE image range.
- At least one return instruction exists.

Passing these checks does not prove behavioral correctness. It only establishes a conservative
position-independent leaf candidate. Calling code must still use the recovered MSVC x86 calling
convention and a reviewed prototype.

## Recovery lifecycle

1. Generate and hash the local island.
2. Give the corresponding source implementation the same logical symbol boundary.
3. Differentially test source and island with identical inputs and state.
4. Mark the function recovered, regenerate the inventory, and regenerate the island manifest.
5. Repeat until the proprietary blob count is zero.

The fully open-source target must not invoke this extractor or link its output. Release validation
must require zero legacy islands, zero original-address calls, zero fixed executable data bindings,
and no original executable input. Independently written assembly is source; copied instructions in
this local object are not.
