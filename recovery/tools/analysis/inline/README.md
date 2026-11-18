# /Ob2 inlining probes (VC6, game unit flags)

Micro-experiments behind `recovery/MATCHING_GUIDE.md` section 2.8. Every script compiles generated C++
with the flags of a game unit plus /GX (`ib.compile_asm`), or with a real unit's full command and headers
(`ib.compile_in_unit`), and reads the decisions from the `/FAs` listing. Prefix: `FE_PFX_BASE` (default 21).
Run them from this directory with `python3 <script> [args]`.

Conventions (`probe.h`): an inline body calls `mk(k)` (or `mk2(k)` for an inner level) with the site's
constant, so `push K; call mk` means site K expanded and `push K; call <inline>` means called. Return-site
teardowns: `at(i)` before each return; an inline destructor calls `dm()`/`dm2()`/`dm3()` first, and
`ib.tails` walks from each `at(i)` (following jumps into tail-merged copies) to list E/c per destructor.

| script | question (guide 2.8 rule) |
| --- | --- |
| level1_cutoff.py [N] | first-level sites: source-order cutoff, cost against callee size (1, 2) |
| caller_growth.py N kind | budget growth per caller statement kind: store, call, if, add (2) |
| costs.py | cost of a callee body against a 2-if reference: free threshold, dead code, __forceinline (3) |
| dtor_sites.py | return-site teardowns of one costly destructor: source order, closing teardown last (1) |
| nested_breadth.py | top-level spend before/between/after the outer sites; pass 2 after pass 1 (1) |
| nested_explicit.py 'OUTER_BODY' ni N... | explicit outer(k){...; inn(k);} chains: suffix / periodic / prefix (4) |
| nested_filler.py ni no N k0 k1 step | the same under growing caller filler (phase of the periodic pattern) (2, 4) |
| fam1.py j ni [N...] | outer with j flat statements: fits room_i = A − N·Y + i·D − accepted (4) |
| fitftb.py PATTERN... | forward token-bucket fit of one accept/refuse string |
| dtor_nested.py [M...] | P{S} destructor chain at M returns: the periodic pattern (4) |
| dtor_nested_filler.py M top K | the same with K filler stores: floor B0, then one site per two stores (2) |
| dtor_chain3.py n1 n2 n3 M... | three-level member chain (5) |
| popup_chain.py M... | Popup-like chain: P : Bs { S s; }, S { FB f1, f2; } (4, 5) |
| probe_position.py M n if1/if4 | dead probes after return site k: reservation for earlier sites only (4) |
| real_popup.py M... | real `Popup` (popup.h, probe.cpp's command) at M returns (6) |
| realdeck.py | `run(unit, addr, [(key, text)])`: whole-file variants of a real unit, call-count differences |

`deck.py` (`F`, `run`, `show`) and `ib.py` (`compile_asm`, `compile_in_unit`, `functions`, `sites`, `tails`)
are the shared runner.
