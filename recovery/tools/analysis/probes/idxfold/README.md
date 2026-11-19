# Constant index offset kept in the scaled index (MATCHING_GUIDE 3.4)

Image shape: `lea r,[r+r*2+0xD2]; shl r,4; mov x,[r+Facility]` (the +70 of
`Facility[70 + id]` stays inside the index) at 0x0041BD85, 0x00537781 and
0x0053780E only; every other `Facility[70 + id]` folds 70*0x30 into the
displacement (`[r+Facility+0xD20]`), and so did ours at the three sites.

`run.py` (small probe file, game flags): 50 spellings of the index. Folded:
order, enum/#define/const/static const member/extern const, local `int k = 70`,
unsigned/short/casts, inline helpers returning the pointer, the index or the
name (expression or statement bodies), pointer arithmetic, `(char *)` offsets,
address-taken or volatile ids, struct-member arrays, a named index variable in
a function without EH. Kept (INDEX): `eh_slot` only, a function with an EH
object where the index is a named variable `int i = K + id;` and `id` stays
live in a register. `var_eh` (the id dies at the add) gives `add eax,70;
lea [eax+eax*2]` instead.

`inunit.py deckN.py` replays spellings inside the real units: base_editor_proj
`fid`/`fid_nobrace`/`fid_const`/`fid_top` keep the index (55 -> 53 rows), a
bare brace block does not; process_message needs both named ids (A, B, AB).
