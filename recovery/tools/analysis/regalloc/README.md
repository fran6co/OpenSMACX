# Register-allocation probes (VC6, game unit flags)

Micro-experiments behind `recovery/MATCHING_GUIDE.md` section 2.7. They compile with the flags of a
game unit (`/Og /Ot /Gs /Gy /Ob2 /Oy-`, no /Oi since 2026-11-18, or `/Oy` with `--oy`) and read the result from the `/FAs`
listing. Prefix: `FE_PFX_BASE` (default 2).

    python3 gen.py                       # write the generated families g_<family>.cpp (not committed, 5 MB)
    python3 run.py p01_count.cpp         # home of every variable per function: a:esi b:edi c:ebx d:[d]
    python3 run.py g_split.cpp -f stagger --asm
    python3 thresh.py g_calib.cpp        # duel families: score of d's context in entry-block uses
    python3 tab.py g_weights.cpp cd -v   # one row per family, one column per K
    python3 homes.py 40 cd < blockn.spec  # rule 4 (block weight = candidates referenced in the block): register per K

Conventions (`probe.h`): variable `x` is defined by `def_x()` and read by `use_x(x)`, so the listing shows
where it lives at every site. `[x]` means read from its frame slot; `esi/[x]` means the home changes along
the function. `gen.py` has a small spec language (`Da a3 { d2 } [ C ]`); its docstring lists the tokens.

A duel (`<tag>_cd_k<K>`, `<tag>_dc_k<K>`): `p` and `q` have 30 uses and take esi and edi; `c` has K uses
in the entry block and `d` has the references under test; both are live over the whole function. The K at
which `c` takes ebx is the score of `d`'s references in entry-block uses. `cd`/`dc` is the order of the
two definitions: a threshold that moves by one between them is an exact tie.

| file | question |
| --- | --- |
| p01_count | order of the registers, ties, more candidates than registers |
| g_weights, g_weights2, g_weights3, g_calib, g_depth, g_shapes, g_const | weight of a reference by syntactic position, per-block term |
| g_middle, g_middle2, g_position | entry and last block against blocks in between |
| g_factors, g_factors2, g_parpos | parameters, redefinitions, kinds of reference, tie-breaks, widths, address-taken |
| g_split, g_cache | live ranges, idle stretches, cached pieces of memory-homed variables, write-back |
| g_rangelen, g_span | a block-local candidate against one that spans blocks |
| g_temps | cached pieces against whole variables, `this`, address temporaries, globals |
| p04..p08 | the shape of success_rates (0x0059EEE0), which the model does not predict |
| blockn.spec (homes.py) | rule 4: a reference weighs the number of distinct candidates referenced in its block |
| ties.spec (ties.py) | tie-breaking: live-through penalty (4b), exact-tie order (4c), symbol counter (4d); one function per TU |
| tiegen.py, tiemodel.py | random exact-tie probes and the scoring model they validate (`tiegen.py --count 300 --seed 7 [--others 2] [--loops]`) |
| cand.py, c_*.spec | candidate-set probes: which temporaries VC6 forms and how they rank (`python3 cand.py c_cse.spec`; `--asm NAME`, `--grep RE`) |
| c_sr.spec | strength reduction of a large-stride table index: blocked by an index read in an inner loop; a pointer stepped beside the index (setup_game) |
| c_zero.spec | constant pieces against immediates below the four-store rule: other references (push, memory compare), free register over the range, shared register, typed zero webs, compares against a known-zero variable (MATCHING_GUIDE 2.7 "Constant pieces and immediates") |
| c_zform.spec, zgrid.py | constant formation (which references make 0 a candidate), materialisation (four stores weighted 2^depth; compares and pushes never count) and rank against a rival (memory compares count, pushes do not) (MATCHING_GUIDE 2.7 Z9; `python3 zgrid.py [stores loop arms rank]`) |
| gen_zsplit.py | what splits a constant-zero web: calls, ifs, loops, returns do not; three interfering webs over an unreferenced stretch do (MATCHING_GUIDE 2.7 Z10) |
| gen_loopctr.py | 64-way bisection of TutWin::tour's dir loop body: when a call-free loop counter leaves the scratch registers and inherits the zero web's register (MATCHING_GUIDE 2.7 G1/G2) |
| c_wthru.spec | write-through pieces: rule-9 prefixes, constant initialisers, memory-homed losers with a free stretch, address-taken locals (MATCHING_GUIDE 2.7 "Write-through pieces") |
| c_looppiece.spec | a memory-homed parameter read once per iteration of two loops: scratch read against per-iteration callee-saved load |
| duel.py, m_*.spec | which removed references count: CSE, copy, dead code (no) against tail-merged copies (yes) (`python3 duel.py m_tail.spec`) |
| tiescan.py | non-exact functions whose diff is a pure permutation of callee-saved registers (`uv run python tiescan.py feall.json`) |
| p_consttie | callee-saved assignment has no cursor (4e), two constants that tie go to the later first reference (4f), a constant store weighs half a reference (4g) |
| p_headmerge | head merge of the arms' identical first instruction above the jcc (Labels hoist) and when `mov al,[m]; cmp al,K` folds to a memory compare (MATCHING_GUIDE 3.2) |
