# Switch tail-merge direction (MATCHING_GUIDE 3.7)

`const.py` builds switches whose shared tails are register-free (`use(1)..use(7)`), with a long join
after the switch so VC6 does not duplicate the join into the cases, and prints the layout order of the
case blocks and, per case, its own `use` count and the case it jumps into (`->k`) or `->join`.
`run.py` is the first attempt with struct-copy tails; its tails never merge because register
allocation makes them differ, which is itself the lesson for real code.

| variant | result |
| --- | --- |
| 3,7 shared (if/else around the switch) | 7 jumps into 3 (first copy kept) |
| 3,7 text 7 first | layout follows text; 3 jumps into 7 |
| 3,7 no guard (last case falls into the join) | 3 jumps into 7 |
| 3,7 return (`return;` tails) | 3 jumps into 7 |
| 1,3,5,7 shared | 3, 5, 7 jump into 1 |
| default / L=2 / identical short even bodies | unchanged (later into first) |
