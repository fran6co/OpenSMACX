# Moving the harness to another machine

This is about the *development host* — where the lifter, the differential
oracle and the test suites run. For porting the **game** to native platforms
see `docs/PORTING.md`, which is a different problem.

Start with:

```
python3 tools/host_doctor.py
```

It imports nothing outside the standard library, so it works on a box that has
none of the dependencies yet — reporting that is half its job.

## Why an x86-64 host is worth the trouble

The harness is a 32-bit x86 PE. On an arm64 Mac it runs under Rosetta 2, and
two of the ugliest mechanisms in the oracle exist *only* to work around that:

| On a translated host | On native x86-64 |
| --- | --- |
| IDIV/IMUL leave flags architecturally undefined, and the translator's answers are not even self-consistent — one instruction class was measured giving three different answers. `lifted_oracle_plan.py` computes a per-function `undef=<hex>` mask so those flags are **excluded from comparison**. | Real silicon is at least self-consistent. The mask should become droppable, and those flags get compared instead of ignored. |
| Guest `0xFFFF0000..0xFFFFFFFF` is committed memory that `VirtualQuery` refuses, so it can be neither walled nor sealed, and every small negative pointer lands there. Needs a three-fill arbitration: re-run the original under three page fills, discard the case if its answer moves. | The address is above 32-bit `TASK_SIZE` and simply faults. The arbitration becomes dead weight. |

Neither is a tuning knob. They are the difference between comparing a flag and
declining to.

A `linux/amd64` container on an arm64 machine does **not** count — that is QEMU
or Rosetta translation wearing a native machine name. The doctor checks
`binfmt_misc` for exactly this and warns.

## What has to travel

Almost nothing. The two large directories are both reproducible:

| Path | Size | Move it? |
| --- | --- | --- |
| the git repo | small | yes — as a bundle, see below |
| `.opensmacx/game/terranx_original.exe` | 3 MB | **yes** — your own copy, deliberately never in git |
| `build/` | 2.8 GB | no — regenerate |
| `.opensmacx/` (wine, prefixes, IDA, extracted) | 3.6 GB | no — reinstall |

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

The doctor verifies the image against the sha256 every recorded address and
byte total in this repo was measured against. A different build is not an
error, but the numbers will not line up, and it says so.

## Dependencies

Debian/Ubuntu x86-64:

```
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install g++-mingw-w64-i686 wine wine32:i386 cmake build-essential
```

`wine32:i386` is the one that matters. A `wine64`-only install answers
`wine --version` happily and then cannot load an i386 PE at all.

Python tooling needs `capstone` and `pefile`. Either install
[uv](https://astral.sh/uv) and run tools through it:

```
uv run --with-requirements tools/requirements.txt python3 tools/lifted_oracle_plan.py
```

or make a venv and point CMake at it with `-DOPENSMACX_PYTHON=<that python>`.
The system `python3` deliberately does not carry these.

## Rebuild, in order

```
build/lifted/build.sh          # the lifted image objects
tools/lifted_oracle_build.sh   # the differential oracle
tools/lifted_oracle_sweep.sh   # the sweep (hours; use a small --list first)
```

`$CXX` overrides the mingw compiler if it is not on `PATH`.

## The three checks that decide whether the move paid off

Do not trust a number from the new host until these pass. The doctor prints
them too.

1. **The old PASSes reproduce.** Sweep, then
   `python3 tools/lifted_oracle_summary.py build/oracle/report.tsv --json new.json`
   and diff `new.json` against the record you brought. Compare the BYTE
   figures, not the counts, and compare `agreed_full_strength` as well as
   `agreed`: a move that trades large single-seed PASSes for small
   full-strength ones changes the byte totals in opposite directions while the
   function count barely moves. `must_go_down` is the block to watch - if any
   of those three rises, the move cost something.

2. **The undefined-flag masks become droppable.** Regenerate the plan with the
   `undef=` token suppressed and re-sweep. On native silicon this should cost
   **zero** new FAILs. If it does not, the masks were hiding a real lowering
   bug — which is worth more than the migration was.

3. **A guest read at `0xFFFF0000` faults.** If it does, the three-fill
   arbitration in `lifted_oracle.cpp` is dead weight and can be retired.

Check 2 is the one to care about. It converts a comparison the harness was
declining to make into one it can make, across every function that divides or
multiplies.
