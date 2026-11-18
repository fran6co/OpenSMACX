# blockn.spec: probes for rule 4 (block weight = distinct candidates referenced in the block).
# python3 homes.py 40 cd < blockn.spec   (per K: home of c then d; S=esi D=edi B=ebx P=ebp m=memory *=changes)
# Expected first K at which c takes esi, from weight = n_b * 2^depth: see MATCHING_GUIDE 2.7 rule 4.
## no other candidates: entry {c} = 1, last {c,d} = 2 -> d counts double: 16
N1 | | Dc c{K} [ C ] Dd d8 c
N3 | | Dc c{K} { Dd d8 } c
n1_0pq_1if | | Dc c{K} [ C ] Dd d8 c
n1_0pq_2if | | Dc c{K} [ C ] [ C ] Dd d8 c
n1_1p_1if | | Dp Dc p2 c{K} [ C ] Dd d8 p c
n1_2pq_1if | | Dp Dq Dc p2 q2 c{K} [ C ] Dd d8 p q c
n1_2pq_1ifb | | Dp Dq Dc p2 q2 c{K} [ C ] Dd d8 c p q
n1_1p_late | | Dc c{K} [ C ] Dp Dd p2 d8 p c
n1_1p_loc | | Dc c{K} [ C ] Dd d8 Dp p2 c
live_p | | Dp Dc p2 c{K} [ C ] Dd d8 c [ C ] p
ref_p | | Dp Dc p2 c{K} [ C ] Dd d8 c p
twoE | | Dc De c{K} e2 [ C ] Dd d8 c e
glob | | Dc c{K} sink_a~=~1; sink_b~=~2; [ C ] Dd d8 c
argE | int e | Dc c{K} use_e(e); [ C ] Dd d8 c
loopd | | Dc Dd c{K} { d4 } c d
loopde | | Dc Dd De c{K} { d4 e2 } c d e
loop2d | | Dc Dd c{K} { { d4 } } c d
armde | | Dc Dd De c{K} [ d4 e2 ] c d e
armd | | Dc Dd c{K} [ d4 ] c d
swd | | Dc Dd c{K} switch~(cond())~{ case~1: d4 break; case~2: C break; } c d
forv | | Dc Dd c{K} F{ d4 } c d
inf | | Dc Dd c{K} for~(;;)~{ d4 if~(cond())~break; } c d
wtrue | | Dc Dd c{K} while~(1)~{ d4 if~(cond())~break; } c d
base | | Dc c{K} [ C ] Dd d8 c
e1 | int e | Dc c{K} use_e(e); [ C ] Dd d8 c
e2 | int e | Dc c{K} use_e(e); use_e(e); [ C ] Dd d8 c
e2l | int e | Dc c{K} [ C ] Dd d8 c use_e(e); use_e(e);
e1E1L | int e | Dc c{K} use_e(e); [ C ] Dd d8 c use_e(e);
loc1 | | Dc c{K} De e [ C ] Dd d8 c
cse | int i | Dc c{K} sink_a~=~gobj[i].f1; sink_b~=~gobj[i].f2; sink_e~=~gobj[i].f3; [ C ] Dd d8 c
cse2 | int i | Dc c{K} [ C ] Dd d8 sink_a~=~gobj[i].f1; sink_b~=~gobj[i].f2; sink_e~=~gobj[i].f3; c
konst | | Dc c{K} [ C ] Dd d8 use_a(7); use_a(7); use_a(7); use_a(7); use_a(7); c
konstE | | Dc c{K} use_a(7); use_a(7); use_a(7); use_a(7); use_a(7); [ C ] Dd d8 c
