# Hybrid image

The hybrid image is a local migration artifact between the current patched-executable runtime and a
standalone source-built executable. It preserves a supported user-owned PE file exactly so legacy
code and data can continue to occupy their original virtual addresses while individual subsystems
are replaced. It is not itself a new executable or a distributable build product.

## Prepare the image

Install `tools/requirements.txt`, place the supported executable at
`.opensmacx/game/terranx_original.exe`, and run:

```sh
.opensmacx/venv/bin/python tools/prepare_hybrid_image.py
```

An existing CMake configuration can invoke the same operation:

```sh
cmake --preset mingw-i686-release \
  -DOPENSMACX_PYTHON="$PWD/.opensmacx/venv/bin/python"
cmake --build --preset mingw-i686-release --target prepare-hybrid-image
```

Override `OPENSMACX_LEGACY_EXE` and `OPENSMACX_HYBRID_DIR` to use other local paths. The generated
directory defaults to `.opensmacx/hybrid-image/`, which is ignored by Git.

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
map records whether the executable is the canonical IDB input or the independently analyzed Ghidra
cross-build. On the cross-build, only `exact`, `entry_range`, and `start_only` same-entry
correlations use Ghidra body ranges and receive byte hashes. `containing`, `split`, and `missing`
candidates remain explicitly ambiguous or unmapped instead of being projected by canonical address.
Inputs matching neither analysis are rejected.

Generation is deterministic for the same inputs: absolute paths and timestamps are excluded, JSON
keys are sorted, and every blob is content-hashed. Output is first completed in a temporary
directory; regeneration replaces only a recognized hybrid-image directory and restores the prior
pack if publication fails. The generator also reconstructs the complete source in memory and
refuses to publish the pack if any byte is missing or changed. Relocation entries in the compact
sidecar are encoded as `[type, page_offset]` pairs.

## Distribution boundary

Everything under the generated directory derives from a proprietary executable and must remain
local. Do not commit, package, upload, or distribute it. Source code, recovery metadata, and the
generator are the only repository artifacts.

The future native target must not consume this pack. Its release gate must require all of the
following:

- No legacy section, header, gap, overlay, or function-body blobs.
- No fixed function or data addresses.
- Source-owned startup, state, platform services, rendering, input, and persistence.
- `open_source_ready: true` from a source-only manifest generated without proprietary input.

Until those conditions hold, the current Wine executable remains the behavioral reference and the
hybrid image remains a local recovery aid.
