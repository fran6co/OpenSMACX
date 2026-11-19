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
| tm7.py | shared generator for the host-choice families: an if/else-if chain of arms, the join after it, `analyse()` reports call copies and the arm (`mkK`) that falls into the join |
| tm7a.py | factor screen with a source fallthrough arm (EH, return value, overloads, goto label, distinct callees) |
| tm7b.py / tm7c.py | joins without a source fallthrough predecessor: who hosts |
| tm7d.py / tm7e.py | factors on the host pair: EH regions, empty destructors, big frame, argument kinds |
| tm7f.py | function size (filler blocks before/after) |
| tm7g.py SPEC,... | host choice by arm pattern letters (F f3 tail, P f2 in an `if`, Q unshared) |
| tm7h.py SPEC,... [eh] | host choice with more arm shapes: then-arm / else-arm / lone-if tails, ternary-join pops, deferred-pop mismatches, labels |
| tm7i.py / tm7j.py / tm7l.py / tm7m.py / tm7n.py | first-arm complexity, sibling pops in one arm, invisible differences, labels and other persistent blocks |
| tm7k.py | cap on merges per function (none up to 400 pairs) |
