# Moving the harness to another machine

This is about the *development host* — where the recovery runs: the VC6 build
under Wine, and the gates that re-measure every claim in `src/`. For porting
the **game** to native platforms see docs/PORTING.md (retired), which is a
different problem.

The harness this document used to migrate — the lifter, the differential
oracle, the doctor that checked a new box over — went with its machinery
(docs/RETIRED_ROUTES.md). The host's job is now the byte-match loop, and the
acceptance is the gates, which RE-PROVE the claims on the new host rather
than trusting the move:

```
uv sync
cmake -S . -B build -G Ninja
cmake --build build
uv run tools/decomp_status.py --check
```

The last command is the ratchet: it compiles every BYTE_EXACT claim in `src/`
against the shipped image and fails if one no longer reproduces — the only
check in this tree that was ever load-bearing, and on a new host it is the
answer to "did the move cost anything".

## What has to travel

Almost nothing. The two large directories are both reproducible:

| Path | Size | Move it? |
| --- | --- | --- |
| the git repo | small | yes — as a bundle, see below |
| `.opensmacx/game/terranx_original.exe` | 3 MB | **yes** — your own copy, deliberately never in git |
| `build/` | 2.8 GB | no — regenerate |
| `.opensmacx/` (wine, prefixes, analysis) | 3.6 GB | no — reinstall |

`origin` points at a repository that is not yours, so pushing is not the
transfer path. A bundle carries the full history in one file:

```
git bundle create /tmp/opensmacx.bundle --all
```

Then on the new host:

```
git clone /path/to/opensmacx.bundle OpenSMACX
mkdir -p OpenSMACX/.opensmacx/game
cp /path/to/terranx_original.exe OpenSMACX/.opensmacx/game/
```

Every measurement in this repo was taken against that executable — the pinned
image the ratchet compiles against and `tools/disasm.py` disassembles. A
different build is not an error, but the numbers will not line up, and the
ratchet says so.

## Dependencies

Debian/Ubuntu x86-64:

```
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install wine wine32:i386 cmake ninja-build build-essential
```

`wine32:i386` is the one that matters. A `wine64`-only install answers
`wine --version` happily and then cannot load an i386 PE at all.

Python tooling needs `capstone` and `pefile`. Install
[uv](https://astral.sh/uv); it is the only supported route, and it fetches the
interpreter named in `.python-version` if the host lacks it:

```
uv sync
```

The system `python3` deliberately does not carry these.

VC6 under Wine is the only compiler, and its install is the one setup step
that neither apt nor uv performs. The README carries its one-time step — the
lowercase symlinks beside headers spelled UPPERCASE — beside the build
instructions; the toolchain finds it through `cmake/toolchains/vc6.cmake`.

## Rebuild

```
uv sync
cmake -S . -B build -G Ninja
cmake --build build
```

The VC6 toolchain defaults in, and the build needs no flags. The product is
`opensmacx-link-check.exe`: it exists so the recovered bodies must compile
and LINK, which is what catches a body that type-checks in isolation and
cannot resolve its callees. It is not the game — running the game is retired
for now, with its routes recorded in docs/RETIRED_ROUTES.md.

## The checks that decide whether the move paid off

Do not trust a number from the new host until these pass:

1. **The ratchet reproduces.** `uv run tools/decomp_status.py --check`
   compiles every BYTE_EXACT claim in `src/` against the shipped image and
   fails if one no longer reproduces. The claims live in the tree, not in
   anything the old host carried, so a host where the ratchet passes is a
   host where the recovery stands.

2. **The source map parses.** `uv run pytest decomp/tests` proves the
   annotation parse against `src/` itself — every record, every catalogue row,
   and the grammar's full pattern inventory — plus the read -> write -> read
   loop over every annotated file. (This was `uv run python -m decomp` until
   2026-08-19; that entry point held assertions the test module was already
   importing, so it became tests and there is one way to run them.)

3. **The exclusions still measure.** `uv run tools/measure_exclusions.py
   --check` re-derives every figure in docs/EXCLUSIONS.md from the image and
   fails if the document and the bytes disagree.

The checks this section used to carry — sweep summaries, undefined-flag
masks, three-fill arbitration — policed a comparison harness that is retired
in docs/RETIRED_ROUTES.md. Byte comparison IS the verification now, and these
three re-prove it.
