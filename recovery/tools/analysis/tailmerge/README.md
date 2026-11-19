# Cross-jump (tail-merge) probes

Micro-experiments behind the cross-jump lines of `recovery/MATCHING_GUIDE.md` 3.7 ("Layout"). They compile
generated C++ with the game unit flags through `../inline/ib.py` (prefix `FE_PFX_BASE`, default 21) and read
the `/FAs` listing. Run from this directory with `uv run python <script>`.

| script | question |
| --- | --- |
| tm.py | switch cases with identical `tl(g2); call();` tails: copies kept against filler after the switch, case count, EH |
| tm2.py | inline toggles in cases: one shared inline vs distinct inlines, constant vs global-loaded argument |
| tm3.py | which tail shapes merge jumper-into-jumper (`g3` stores copied into both arms = merged) |
| tm6.py ['TAIL'] | the T1 merge against the unit's label counter (0..79 dummy functions before it): stable |
| jjcensus.py src/unit.cpp ... | real units: jumper-into-jumper merges per function, exact vs non-exact (heuristic) |
