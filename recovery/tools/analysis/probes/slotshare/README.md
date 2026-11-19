# Frame slot sharing with inlined callees' locals (MATCHING_GUIDE 3.3)

`run.py` prints every named frame variable's offset from the `/FAs` listing (`_name$ = -N`), so shared
slots show as one offset with several names. The callee is a has_fac-like inline with address-taken
`offset`/`mask` (`bitmask(id, &offset, &mask)`); the later loop counter is address-taken (`for_addr`),
register-allocated (`for_plain`), spilled by register pressure (`for_pressure`) or block-scoped.
`pick.py` adds an earlier spilled counter to see which free slot a spilled counter picks.

Answers: spilled scalars pack by live range into the slots of inline locals, earlier counters and dead
parameters; two address-taken locals with disjoint lives stay apart; with both free, the spilled
counter took the inline local's slot.
