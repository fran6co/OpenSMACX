# Vector constructor iterator probes (MATCHING_GUIDE 2.6, "Arrays of objects")

When does VC6 call `??_H` (`vector constructor iterator`) instead of expanding the construction loop?
Each script compiles generated units with the game flags through `../../inline/ib.py` (prefix
`FE_PFX_BASE`) and prints, per probe function, `H` (`call ??_H`), `L`/`M` (EH iterators),
`ctor-call` (expanded loop calling the constructor) or `none` (constructor expanded too).
Run from this directory: `FE_PFX_BASE=N uv run python <script>`.

| script | question | answer |
| --- | --- | --- |
| run.py | element ctor kind (out of line before/after, extern, inline, generated), dtor, count 1..16, local/member/member-with-ctor | no dtor: always expanded; with dtor: `??_L`/`??_M` always called |
| more.py | `new T[n]`, `new T[7]`, virtual, 2-D, static local, global, default argument | all expanded (globals use the unit initializer) |
| eh.py | an EH frame (a local with a destructor) before/after the array | still expanded |
| budget.py | N branchy inline sites before/after the array, caller size | refused once the caller budget is spent before it (pre=16); later sites never refuse it; a bigger caller restores it |

Real case: NetWin::exec 0x00482490 (`#pragma inline_depth(1)`, see the guide).
