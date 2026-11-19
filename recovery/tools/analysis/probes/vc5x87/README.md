# VC5 x87 operand order and schedule (MATCHING_GUIDE 3.8)

| script | question | answer |
| --- | --- | --- |
| v5.py | library: compile probe text with the Caviar VC5 recipe (/O2 /Ob1 `--compiler=vc5`), return each function's listing | used by the probes below |
| x87sym.py UNIT ADDR [--ours] [--names reg+off=name,..] | the expression tree of every x87 store, in the operand order the code used (`{..}` = reloaded from a slot the function stored) | image and our trees side by side: spills and operand order without reading the schedule |

Measured with v5.py (2026-11-19): commutative `*`/`+` operand order and association ignore the spelling;
same-base loads go higher displacement first; other symbols by a compiler key that shifts in bands with
earlier declarations. Every named float local is stored on definition.
