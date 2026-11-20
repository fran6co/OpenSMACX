#!/usr/bin/env python3
"""run.py [name ...]: where VC6 places a forward-jumped block (the if-false target / else arm) relative to the
unconditional jumps after it (MATCHING_GUIDE 3.7, "Deferred blocks").  Prints blockorder/run.py's skeleton
(branches, RET, use(K) markers as uK).  SHOW=1 prints the full listing.  FE_PFX_BASE selects the prefix."""
import sys, os
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(H) + '/blockorder')
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
import run as bo
PFX = os.environ.get('FE_PFX_BASE', '21')
HDR = bo.HDR + 'void pop(int);\n#define U(K) use(K); g5 = K\n'
LOOP = 'int t; int i; for (i = -1; i < 19; i++) { t = i < 0 ? 9 : i; if (ga[t] & b) goto found; } '
FOUND = 'found: use(40); if (b & 0x100) { use(41); use(42); } else if (b & 0x200) { use(43); } else { use(44); } use(45); if (c) { use(46); } '
V = {
 # action_destroy 0x004CAA50 shape: scan loop with goto found; fail path tests a flag, two arms both goto fin
 'ad':        LOOP + 'use(1); if (a) { if (g == c) pop(2); goto fin; } if (g2 == c) pop(3); goto fin; ' + FOUND + 'fin: use(9); use(10); use(11); return 1;',
 # the non-flag path as an explicit else
 'ad_else':   LOOP + 'use(1); if (a) { if (g == c) pop(2); } else { if (g2 == c) pop(3); } goto fin; ' + FOUND + 'fin: use(9); use(10); use(11); return 1;',
 # negated test, non-flag path first
 'ad_neg':    LOOP + 'use(1); if (!a) { if (g2 == c) pop(3); goto fin; } if (g == c) pop(2); goto fin; ' + FOUND + 'fin: use(9); use(10); use(11); return 1;',
 # found block written in the loop
 'ad_inloop': 'int t; int i; for (i = -1; i < 19; i++) { t = i < 0 ? 9 : i; if (ga[t] & b) { use(40); if (b & 0x100) { use(41); use(42); } else if (b & 0x200) { use(43); } else { use(44); } use(45); if (c) { use(46); } goto fin; } } use(1); if (a) { if (g == c) pop(2); goto fin; } if (g2 == c) pop(3); fin: use(9); use(10); use(11); return 1;',
 # found as a flag after the loop: if (i >= 19) {fail} else {found}
 'ad_flag':   LOOP.replace('goto found;', 'break;') + 'if (i >= 19) { use(1); if (a) { if (g == c) pop(2); } else { if (g2 == c) pop(3); } } else { ' + FOUND[7:] + '} use(9); return 1;',
 'ad_flag2':  LOOP.replace('goto found;', 'break;') + 'if (i >= 19) { use(1); if (a) { if (g == c) pop(2); goto fin; } if (g2 == c) pop(3); goto fin; } ' + FOUND[7:] + 'fin: use(9); use(10); use(11); return 1;',
 # no scan loop: plain if before (does the queue need the loop's goto?)
 'ad_noloop': 'if (ga[b]) goto found; use(1); if (a) { if (g == c) pop(2); goto fin; } if (g2 == c) pop(3); goto fin; ' + FOUND + 'fin: use(9); use(10); use(11); return 1;',
 # the non-flag arm is call-only (no inner test)
 'ad_call':   LOOP + 'use(1); if (a) { if (g == c) pop(2); goto fin; } pop(3); goto fin; ' + FOUND + 'fin: use(9); use(10); use(11); return 1;',
 # the flag arm without inner test
 'ad_call2':  LOOP + 'use(1); if (a) { pop(2); goto fin; } if (g2 == c) pop(3); goto fin; ' + FOUND + 'fin: use(9); use(10); use(11); return 1;',
 # found's first arm is not an if/else (no jmp in found until later)
 'ad_nojmp':  LOOP + 'use(1); if (a) { if (g == c) pop(2); goto fin; } if (g2 == c) pop(3); goto fin; found: use(40); use(41); if (c) { use(46); } fin: use(9); use(10); use(11); return 1;',
 # found written after fin (goto back to fin)
 'ad_after':  LOOP + 'use(1); if (a) { if (g == c) pop(2); goto fin; } if (g2 == c) pop(3); fin: use(9); return 1; ' + FOUND + 'goto fin;',
 # if (found) {found} else {fail} with a flag set in the loop
 'ad_fl3':    'int t; int i; int f = 0; for (i = -1; i < 19; i++) { t = i < 0 ? 9 : i; if (ga[t] & b) { f = 1; break; } } if (!f) { use(1); if (a) { if (g == c) pop(2); } else if (g2 == c) pop(3); } else { ' + FOUND[7:] + '} use(9); return 1;',
 # fail path ends in return (no goto fin)
 'ad_ret':    LOOP + 'use(1); if (a) { if (g == c) pop(2); use(9); return 1; } if (g2 == c) pop(3); use(9); return 1; ' + FOUND + 'use(9); return 1;',
 # the AlphaMenu::check form: else { E; if (!d) { H; goto past; } } J; past:
 'ad_am':     LOOP + 'use(1); if (a) { if (g == c) pop(2); } else { if (g2 == c) { pop(3); goto fin; } } goto fin; ' + FOUND + 'fin: use(9); use(10); use(11); return 1;',
 # NA arm's test inverted to a goto
 'ad_ng':     LOOP + 'use(1); if (a) { if (g == c) pop(2); goto fin; } if (g2 != c) goto fin; pop(3); goto fin; ' + FOUND + 'fin: use(9); use(10); use(11); return 1;',
 # NA arm without goto: falls into found? (wrong behaviour, layout only)
 'ad_nog':    LOOP + 'use(1); if (a) { if (g == c) pop(2); goto fin; } if (g2 == c) { pop(3); goto fin; } ' + FOUND + 'fin: use(9); use(10); use(11); return 1;',
 # NA with second label
 'ad_lbl':    LOOP + 'use(1); if (a) { if (g == c) pop(2); goto fin; } if (g2 == c) pop(3); goto fin2; ' + FOUND + 'fin: fin2: use(9); return 1;',
 # minimal: deferred NA block against the found block's first if/else
 'm1':        LOOP + 'use(1); if (a) { use(2); goto fin; } use(3); goto fin; found: use(40); if (b & 0x100) { use(41); } else { use(44); } use(45); fin: use(9); use(10); use(11); return 1;',
 'm2':        LOOP + 'use(1); if (a) { use(2); goto fin; } use(3); goto fin; found: use(40); if (b & 0x100) { use(41); } else if (b & 0x200) { use(43); } else { use(44); } use(45); fin: use(9); use(10); use(11); return 1;',
 'm3':        LOOP + 'use(1); if (a) { use(2); goto fin; } use(3); goto fin; found: use(40); if (b & 0x100) { use(41); } else { use(44); } if (b & 0x400) { use(47); } else { use(48); } use(45); fin: use(9); use(10); use(11); return 1;',
 'm1n':       'if (ga[b]) goto found; use(1); if (a) { use(2); goto fin; } use(3); goto fin; found: use(40); if (b & 0x100) { use(41); } else { use(44); } use(45); fin: use(9); use(10); use(11); return 1;',
 'm1w':       'while (ga[b]) { if (ga[c]) goto found; b++; } use(1); if (a) { use(2); goto fin; } use(3); goto fin; found: use(40); if (b & 0x100) { use(41); } else { use(44); } use(45); fin: use(9); use(10); use(11); return 1;',
 'm1f':       'if (ga[b]) { use(1); if (a) { use(2); goto fin; } use(3); goto fin; } use(40); if (b & 0x100) { use(41); } else { use(44); } use(45); fin: use(9); use(10); use(11); return 1;',
 'm1g':       'if (ga[b]) { use(1); if (a) { use(2); } else { use(3); } } else { use(40); if (b & 0x100) { use(41); } else { use(44); } use(45); } use(9); return 1;',
 'm1h':       'if (!ga[b]) { use(40); if (b & 0x100) { use(41); } else { use(44); } use(45); } else { use(1); if (a) { use(2); } else { use(3); } } use(9); return 1;',
 # distinct arm ends (U(K) = use(K); g5 = K) so no tail is cross-jumped
 'n1':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 'n2':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 'n2t':       LOOP + 'U(1); if (a) { if (g) U(2); goto fin; } if (g2) U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 'n2tt':      LOOP + 'U(1); if (a) { if (g) U(2); goto fin; } if (g2 && g3) U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 'n2n':       'if (ga[b]) goto found; U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 'n2e':       LOOP + 'U(1); if (a) { U(2); } else { U(3); } goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 # fail path written after the found block (reached by goto from the loop exit)
 'f1':        LOOP + 'goto fail; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1; fail: U(1); if (a) { U(2); goto fin; } U(3); goto fin;',
 'f2':        LOOP.replace('goto found;', 'break;') + 'if (i >= 19) goto fail; ' + 'U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1; fail: U(1); if (a) { U(2); goto fin; } U(3); goto fin;',
 # found's end jumps to fin explicitly (the last goto fin is found's)
 'f3':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); goto fin; fin: use(9); use(10); use(11); return 1;',
 # an extra goto fin after found (from inside found's last if)
 'f4':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } if (c) { U(46); goto fin; } U(45); fin: use(9); use(10); use(11); return 1;',
 # NA arm ends by falling out of an if/else that is the else of the flag test (no goto from NA)
 'f5':        LOOP + 'U(1); if (a) { U(2); } else { U(3); } if (1) goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 # found ends in an if/else chain (the image's ATESTUFF arm falls into fin)
 'e1':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } if (c) { U(46); } else if (g3) { U(47); } else { U(48); } fin: use(9); use(10); use(11); return 1;',
 'e2':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } if (c) { U(46); } else if (g3) { U(47); } fin: use(9); use(10); use(11); return 1;',
 'e3':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } if (c) { U(46); } else { U(48); } fin: use(9); use(10); use(11); return 1;',
 'e4':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else { U(44); } if (c) { U(46); } else { U(48); } fin: use(9); use(10); use(11); return 1;',
 'e5':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (c) { U(46); } else { U(48); } fin: use(9); use(10); use(11); return 1;',
 # closer to action_destroy: top air check with its own goto fin, bits test with goto fin
 'r1':        'if (g7) { if (g8 == c) { U(30); } goto fin; } if (b > 0) { b &= ga[c]; if (!b) goto fin; U(31); } ' + LOOP + 'U(1); if (a) { if (g == c) { U(2); } goto fin; } if (g2 == 2 && g3 == c) { U(3); } goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } if (c) { U(46); } else if (g3) { U(47); } else { U(48); } fin: use(9); use(10); use(11); return 1;',
 'r2':        'if (g7) { if (g8 == c) { U(30); } } else { if (b > 0) { b &= ga[c]; if (!b) goto fin; U(31); } ' + LOOP + 'U(1); if (a) { if (g == c) { U(2); } goto fin; } if (g2 == 2 && g3 == c) { U(3); } goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } if (c) { U(46); } else if (g3) { U(47); } else { U(48); } } fin: use(9); use(10); use(11); return 1;',
 'r3':        'if (g7) { if (g8 == c) { U(30); } } else { if (b > 0) { b &= ga[c]; if (!b) goto fin; U(31); } ' + LOOP + 'U(1); if (a) { if (g == c) { U(2); } goto fin; } if (g2 == 2 && g3 == c) { U(3); } goto fin; found: { U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } if (c) { U(46); } else if (g3) { U(47); } else { U(48); } } } fin: use(9); use(10); use(11); return 1;',
 # no shared join: every path returns its own value (does NA still go last?)
 'q1':        LOOP + 'U(1); if (a) { U(2); return 2; } U(3); return 3; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); use(9); use(10); use(11); return 1;',
 'q2':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); return 3; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 'q3':        LOOP + 'U(1); if (a) { U(2); return 2; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 'q4':        'if (ga[b]) goto found; U(1); if (a) { U(2); return 2; } U(3); return 3; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); use(9); use(10); use(11); return 1;',
 # found's then-arm ends in an explicit goto to the chain's join
 'gj1':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); goto j; } if (b & 0x200) { U(43); } else { U(44); } j: U(45); fin: use(9); use(10); use(11); return 1;',
 'gj2':        LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); goto j; } else if (b & 0x200) { U(43); } else { U(44); } j: U(45); fin: use(9); use(10); use(11); return 1;',
 'gj3':        'if (ga[b]) goto found; U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); goto j; } if (b & 0x200) { U(43); } else { U(44); } j: U(45); fin: use(9); use(10); use(11); return 1;',
 'gj4':        LOOP + 'U(1); if (a) { U(2); } else { U(3); } goto fin; found: U(40); if (b & 0x100) { U(41); goto j; } if (b & 0x200) { U(43); } else { U(44); } j: U(45); fin: use(9); use(10); use(11); return 1;',
 # toggles of the host-orientation family (MATCHING_GUIDE 3.7): a bare `||` return before the shape
 'tg1':       'if (g8 || g9) return 0; ' + LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 'tg2':       'if (g8 || g9) return 0; if (g6 || g7) return 0; ' + LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 'tg3':       'if (g8) { if (g9) U(20); return 0; } ' + LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 'tg4':       LOOP + 'U(1); if (a || g8) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
 'tg5':       'if (g6 && (g8 || g9)) { U(19); return 0; } ' + LOOP + 'U(1); if (a) { U(2); goto fin; } U(3); goto fin; found: U(40); if (b & 0x100) { U(41); } else if (b & 0x200) { U(43); } else { U(44); } U(45); fin: use(9); use(10); use(11); return 1;',
}
if __name__ == '__main__':
    sel = sys.argv[1:] or list(V)
    src = HDR + ''.join('int %s(int a, int b, int c) {\n%s\n}\n' % (n, V[n]) for n in sel)
    asm, out = ib.compile_asm(None, PFX, text=src)
    F = {f[1]: f[2] for f in ib.functions(asm)}
    for n in sel:
        L = next((l for k, l in F.items() if ('?%s@@' % n) in k), None)
        if L is None:
            print(n, 'MISSING', out[-300:] if out else ''); continue
        L = [l for l in L if not l.startswith(';')]
        print('%-12s %s' % (n, ' '.join(bo.skeleton(L))))
        if os.environ.get('SHOW'):
            print('   ' + ' ; '.join(L))
