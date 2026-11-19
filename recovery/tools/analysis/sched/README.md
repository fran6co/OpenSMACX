# Scheduling and scratch-register probes (VC6, game unit flags)

Micro-experiments behind `recovery/MATCHING_GUIDE.md` section 3.2, "Scratch registers and
scheduling" (rules S1..S6). They compile with the flags of a game unit (`/Og /Ot /Gs /Gy /Ob2
/GX /Oy-`, or `/Oy` with `--oy`) and print the `/FAs` listing per function. Prefix:
`FE_PFX_BASE` (par-5 used 27).

    python3 run.py p05_tempcount.cpp --cut 'push 0$' --upto 'call Sp::extract' --brief
    python3 run.py p04c_symorder.cpp --cut 'call f0' --brief
    python3 padprobe.py p05_tempcount.cpp -f 'FW::t1$' --cut 'push 0$' --upto 'call Sp::extract'

`run.py` groups functions by the name before `__`: the first member prints in full, the
others as a diff against it. `--cut RX[@K]` keeps the instructions after the K-th match,
`--upto RX` stops at the next match, `--brief` prints one line per function.
`padprobe.py` recompiles a probe with 0..63 dummy declarations in front (the front-end symbol
counter) and prints the distinct results: every family here gave one result.

| file | question | rule |
| --- | --- | --- |
| p05_tempcount | one more register-held value earlier in the function rotates every later scratch choice eax -> ecx -> edx | S1 |
| p06_cursor | which earlier statements advance the rotation (loads, lea, arithmetic, byte extension: yes; immediates, call results, `this` into ecx: no) | S1 |
| p07_cursor2 | the rotation continues across calls and through the whole function, per argument evaluation | S1 |
| p08_postinc | `p++->m()` against `p->m()` with the increment elsewhere: the receiver temporary is one more allocation, and a post-call increment (`for (...; ++p)`) moves the add after the call | S1, S5 |
| p03_commut, p04_symorder, p04b_symorder, p04c_symorder | which operand of `a + b`, `a * b`, `a & b` is loaded first and becomes the destination | S4 |
| p01_fieldorder | two struct fields loaded in one block: their eax/ecx assignment follows the order of the fields' first direct references earlier in the function (`title.left += K; title.right -= K` against the reverse) | S4 |
| p01_inline_helper | the same fields first referenced inside an inline helper (`expand_rect`) instead: the order flips without changing the earlier code (SocialWin::draw_*_alloc) | S4 |
| p02_fieldorder_neutral | control: with one operand clearly more used, the order of earlier references does not matter | S4 |
| p09_twocopies, p09_accessor_copies | two struct copies of one source: member access interleaves the stores field by field, reference-returning accessor inlines copy one struct after the other (StatusWin::init) | S6 |
| p10_sched | small scheduling cases: `xor eax,eax` before or after a final store, store order of two loaded values, a thiscall receiver against local stores | S3 |
| p11_byteonce | Console::set_ai's "byte loaded once, movsx per use": char/signed char globals and locals, casts, `!= 0`, shifts all give one `movsx` and `shr/and` pieces here; the image form is not reproduced | open |
| p12_leaadd | CouncWin::set_vote's `lea ecx,[ebx+K]` for a last virtual call on a member array element: every probe spelling gives the image's `lea`; the real function's `add ebx,K; mov ecx,ebx` comes from its context, not the call's spelling | open |
| p13_argtemps | single vs multi-use values after one cursor step, pending vs flushed stack pops, call-result arguments: pops do not move the cursor | S1, S7 |
| p14_multiuse | a value used twice takes eax and leaves the cursor; single-use values continue the cursor around it | S7 |
| p15_named, p17_named2, p18_namedarg, p19_namedmem, p21_csreg, p24_namedarith, p24b_namedarith | a named local / inline parameter of a plain load as a call argument: forward-substituted, identical to the expression in every isolated shape (cdecl, thiscall, members, params, callee-saved bases) | S7 (caution) |
| p16_thiscall | `char *v = gpp[5]; gst.get(v)` (label_get's shape): the named argument takes eax outside the cursor; the written-out argument takes the cursor | S7 |
| p20_flush | `if (gc) return;` loads into eax without moving the cursor; `nv()` changes nothing in isolation | S7 |
| p22_subchain, p23_subchain2 | `a - b - c` order and constant folding are canonical: 16 spellings, casts and helpers give one order | S8 |
