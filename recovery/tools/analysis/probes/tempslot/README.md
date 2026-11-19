# Frame slot packing of named locals and compiler temporaries (MATCHING_GUIDE 3.3)

`t.py` compiles probe text with the game unit flags and prints, per function, every frame offset with its
named symbols (named-access counts) and `~N` anonymous accesses (`-N+[ebp]`: optimizer temporaries).

| file | question | answer |
| --- | --- | --- |
| g1, g2 | which dead slot does a loop's spilled temporary take | the most recently created compatible one |
| g3, g4 | named units: first-fit, last-fit or smallest slot | last-fit, by raw count order |
| g5 | which liveness (cached register copy, reassignment, goto, scope) | flow liveness of the home |
| g6, g7, g9 | does loop depth weigh a temporary's count | no: raw access count |
| g8 | offset order of address-taken groups | creation order (founder) |
| g10 | does a strength-reduced counter keep its variable's home | no |
| g11 | (pressure attempt; inconclusive) | |
| g12 | are two loops' temporaries one unit when they share a counter | no: one unit per web |
| g13 | is a maybe-uninitialised home live where none of its stores reaches | no: live AND defined; exclusive-arm variables share a slot |

Run: `FE_PFX_BASE=N uv run python g3.py [FUNC]` (FUNC prints its listing).
