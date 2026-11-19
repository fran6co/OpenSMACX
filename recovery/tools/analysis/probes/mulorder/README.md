# Index multiply operand order and MapLongitude's type (MATCHING_GUIDE 3.4)

| script | question | answer |
| --- | --- | --- |
| run.py | `G*y` vs `y*G` vs casts, G int vs unsigned, y in memory/register/call result/address-taken | identical code: spelling and signedness never change the order |
| inl.py | an inline `loc(x, y)` passed named locals vs the field expressions, y live after or not | named local: `mov r,[G]; imul r,y`; expression: `mov r,y; imul r,[G]` |
| scan.py FEALL.json [unit ...] | per non-exact function, counts of `imul r,[G]` vs `mov r,[G]; imul r,y` on each side | lists functions whose counts differ (run from the repo root) |

MapLongitude evidence: x_dist's 132 signed compares, no unsigned use, unsigned declaration loses 37
exact functions (feall, 2026-11-19).
