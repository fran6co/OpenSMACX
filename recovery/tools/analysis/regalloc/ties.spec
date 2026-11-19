# ties.spec: tie-break families for MATCHING_GUIDE 2.7 'Tie-breaking' (4b live-through penalty, 4c exact-tie order, 4d counter).
# python3 ties.py ties.spec --n 1-40 --vars cd   (cell = register of c then d; {N} and PAD are swept)
## 4c single block: first definition wins; declaration order irrelevant
cd4 | | | Dc Dd c4 d4 c d
dc4 | | | Dd Dc c4 d4 c d
decl | | | Nd Nc Sc Sd c4 d4 c d
par_first_read | | int~c,~int~d | d4 c4 d c
## 4c later start block wins an exact tie (S7: d defined after the branch)
S7 | | | Dc c3 [ C ] Dd d3 [ C ] c d
S7b | | | Dd d3 [ C ] Dc c3 [ C ] c d
## 4c one reference decides (no epsilon): c{N} with N = tie-1, tie, tie+1
B4 | | | Dc c{N} [ C ] [ C ] [ C ] [ C ] Dd d3 [ C ] c d
## 4b live-through penalty: c wins from N = tie + penalty (1/2 n_b 2^depth per hole block)
h_d | | | Dc Dd c4 d4 [ C ] d [ C ] d [ C ] d [ C ] d [ C ] c{N}
h_d2 | | | Dc Dd c4 d4 [ C ] d2 [ C ] d2 [ C ] d2 [ C ] d2 [ C ] c{N}
h_d3 | | | Dc Dd c4 d4 [ C ] d3 [ C ] d3 [ C ] d3 [ C ] d3 [ C ] c{N}
h_de | | | Dc Dd De c4 d4 e2 [ C ] d e [ C ] d e [ C ] d e [ C ] d e [ C ] c{N}
h_def | | | Dc Dd De Df c4 d4 e2 f2 [ C ] d e f [ C ] d e f [ C ] d e f [ C ] d e f [ C ] c{N}
h_dE | | | Dc Dd De c4 d4 e2 [ C ] d e [ C ] d [ C ] d [ C ] d [ C ] c{N}
h_arm | | | Dc Dd c4 d4 [ d ] [ d ] [ d ] [ d ] [ C ] c{N}
h_live | | | Dc Dd c4 d4 [ C ] d [ C ] d [ C ] d [ C ] d [ C ] c{N} d
L2 | | | Dc Dd c4 d4 { d } { d }  [ C ] c{N}
L4 | | | Dc Dd c4 d4 { d } { d } { d } { d }  [ C ] c{N}
L8 | | | Dc Dd c4 d4 { d } { d } { d } { d } { d } { d } { d } { d }  [ C ] c{N}
## 4d symbol counter: one result over 64 pads
S9b | PAD | | Dc Dd c4 d4 [ C ] d [ C ] c
S7 | PAD | | Dc c3 [ C ] Dd d3 [ C ] c d
S6 | PAD | | Dc Dd [ c4 ] [ d4 ] d c
