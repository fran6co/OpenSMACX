# Hybrid image

The hybrid image is a local migration artifact between the current patched-executable runtime and a
standalone source-built executable. It preserves a supported user-owned PE file exactly so legacy
code and data can continue to occupy their original virtual addresses while individual subsystems
are replaced. It is not itself a new executable or a distributable build product.

## Build the hybrid executable

Install `tools/requirements.txt`, stage the supported game files under `.opensmacx/game/`, and run:

```sh
.opensmacx/venv/bin/python tools/prepare_hybrid_image.py
.opensmacx/venv/bin/python tools/assemble_hybrid_image.py
```

An existing CMake configuration can invoke the same operation:

```sh
cmake --preset mingw-i686-release \
  -DOPENSMACX_PYTHON="$PWD/.opensmacx/venv/bin/python"
cmake --build --preset mingw-i686-release --target assemble-hybrid-executable
```

`prepare-hybrid-image` creates the verified pack. `assemble-hybrid-executable` consumes that pack
and reconstructs `.opensmacx/hybrid/terranx_pracx_legacy.exe`. The assembled file must have the source
SHA-256 recorded by the pack, satisfy the PE32/i386 manifest fields, and remain outside the pack so
pack ownership validation cannot be bypassed by generated extras.

To build the DLL, assemble the PE, add the 462-import OpenSMACX ABI, and redirect the implemented
functions in one command, run:

```sh
cmake --build --preset mingw-i686-release --target stage-hybrid-game
```

This writes `.opensmacx/game/terranx_hybrid.exe` alongside the existing local game data. Launch it
with:

```sh
.opensmacx/venv/bin/python tools/run_game.py \
  --game-dir .opensmacx/game \
  --executable terranx_hybrid.exe \
  --wine "/Applications/Wine Staging.app/Contents/Resources/wine/bin/wine" \
  --wine-prefix .opensmacx/wineprefix
```

Run the automated startup gate with:

```sh
cmake --build --preset mingw-i686-release --target smoke-hybrid-game
```

The smoke harness snapshots matching processes before launch, captures Wine loader diagnostics,
and requires the hybrid executable, `OpenSMACX.dll`, `prax.dll`, Wine's built-in `DDRAW.dll`, and a
DirectDraw surface flip. It fails on unhandled-exception diagnostics and writes
`hybrid-smoke-result.json` beside the build output. It never terminates pre-existing or newly
launched game processes.

Override `OPENSMACX_LEGACY_EXE`, `OPENSMACX_HYBRID_DIR`, and `OPENSMACX_HYBRID_EXE` to use other
local hybrid paths. `OPENSMACX_LEGACY_LEAF_EXE` remains the independently analyzed pre-PRACX
executable used for local island extraction. All default outputs remain under `.opensmacx/`, which
is ignored by Git.

On macOS, always use `tools/run_game.py` rather than invoking the Wine CLI directly. The launcher
uses Launch Services for the native window driver and explicitly passes the configured Wine prefix.
The hybrid defaults to the hash-pinned PRACX executable because it works with Wine's built-in
DirectDraw. The pre-PRACX executable reaches an unsupported DirectDraw path on current Wine, while
forcing the bundled native DDrawCompat proxy is not compatible with Wine Staging 11.10.

## Artifact contract

`manifest.json` records:

- The source filename, size, and SHA-256.
- PE32 machine, image-base, entry-point, alignment, subsystem, and image-size fields.
- Every PE data directory, imported symbol, resource, section, and file-layout range.
- Hashed header, section, gap, and overlay blobs that reconstruct every source byte exactly.
- Hashed references to the relocation and unresolved-function sidecars.
- `distribution: local_only` and `open_source_ready: false` while legacy content remains.

`relocations.json` contains every parsed base-relocation block and entry. `legacy-functions.json`
maps the current recovery backlog from `priorities.csv` to source file ranges and byte hashes. The
map records whether the executable is the canonical IDB input, the independently analyzed Ghidra
cross-build, or the hash-pinned PRACX runtime build. On the cross-build, only `exact`,
`entry_range`, and `start_only` same-entry correlations use Ghidra body ranges and receive byte
hashes. `containing`, `split`, and `missing` candidates remain explicitly ambiguous or unmapped.
The PRACX runtime build is accepted for local compatibility but all function mappings are marked
`not_analyzed` and unmapped rather than projected by canonical address. Other inputs are rejected.

Generation is deterministic for the same inputs: absolute paths and timestamps are excluded, JSON
keys are sorted, and every blob is content-hashed. Output is first completed in a temporary
directory; regeneration replaces only a recognized hybrid-image directory and restores the prior
pack if publication fails. The generator also reconstructs the complete source in memory and
refuses to publish the pack if any byte is missing or changed. Relocation entries in the compact
sidecar are encoded as `[type, page_offset]` pairs.

The assembler validates the complete pack before reading it, requires contiguous coverage of the
source file, checks the function and relocation sidecars against the manifest, reconstructs the PE
in memory, and verifies its source hash and header contract before publishing it. An existing output
is replaced only when it already has the expected source hash.

## Local assembly islands

Self-contained unrecovered leaves can be extracted into a local x86 COFF object so a future hybrid
link can resolve them by symbol and replace them individually with source:

```sh
cmake --build --preset mingw-i686-release --target assemble-legacy-leaves
```

The extractor accepts only exact cross-build function boundaries with one contiguous body, complete
x86 decoding, at least one return, and no calls, external branches, nested entries, PE relocations,
segment memory, absolute memory, or image-address immediates. Each accepted body is emitted into its
own `.text$legacy$<address>` section with a stable `opensmacx_legacy_<address>` symbol. See
`docs/LEGACY_ISLANDS.md` for the ownership and release rules.

## Distribution boundary

The pack, assembled legacy PE, and staged hybrid executable all derive from a proprietary
executable and must remain local. Do not commit, package, upload, or distribute them. Source code,
recovery metadata, and the generator/assembler tools are the only repository artifacts.

The future native target must not consume this pack. Its release gate must require all of the
following:

- No legacy section, header, gap, overlay, or function-body blobs.
- No fixed function or data addresses.
- Source-owned startup, state, platform services, rendering, input, and persistence.
- `open_source_ready: true` from a source-only manifest generated without proprietary input.

Until those conditions hold, the current Wine executable remains the behavioral reference and the
hybrid image remains a local recovery aid.
