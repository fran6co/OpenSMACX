"""joinhost.py SPEC [--eh] [-v]: where VC6 lays the join of an if/else-if dispatch whose last arm ends in
goto (no fall-through into the join), and which identical arm tails it cross-jumps (MATCHING_GUIDE 3.7
"Join host", 2026-11-20 par-4). SPEC: one letter per arm: d use(k) tail, p pp(k,k) tail (shared callee),
q use2(k,k), x xk(k,k,k) (distinct callee, shared add esp,0xc), s store only, P/Q/R arms ending in if/else-if,
if/else-of-ifs and a lone if around pp, X/Y x arms with an inner return, g goto out (last arm).
Output: arm numbers in layout order, * = a pp call left in place (merged copies vanish), J = join, O = out."""
import sys,os,re; sys.path.insert(0,'.'); import ib
# arms: 'd' = distinct tail (ak(); u(k)), 'p' = shared-callee tail pp(k, k) (cross-jump candidate)
# last arm 'g' ends with goto out (no fall into join).  Report: arm order with J and which arms' pp call sites exist (merged ones vanish)
def build(spec, joinsize=8, eh=False):
    out=[]
    for k,ch in enumerate(spec,1):
        if ch=='d': body='a%d(); use(%d);'%(k,k*100)
        elif ch=='p': body='a%d(); pp(%d, %d);'%(k,k*100,k*100+1)
        elif ch=='x': body='a%d(); x%d(%d, %d, %d);'%(k,k,k,k,k)
        elif ch=='X': body='a%d(); if (g2) return; x%d(%d, %d, %d);'%(k,k,k,k,k)
        elif ch=='Y': body='if (g2 == 99) { pp(9, 9); return; } a%d(); x%d(%d, %d, %d);'%(k,k,k,k,k)
        elif ch=='P': body='a%d(); if (g2) pp(%d, 1); else if (g3) pp(%d, 2);'%(k,k,k)
        elif ch=='Q': body='a%d(); if (g2) { if (g3) pp(%d, 1); } else { if (g4) pp(%d, 2); }'%(k,k,k)
        elif ch=='R': body='a%d(); if (g2) pp(%d, 1);'%(k,k)
        elif ch=='s': body='a%d(); g3 = %d;'%(k,k)
        elif ch=='q': body='a%d(); use2(%d, %d);'%(k,k*100,k*100+1)
        elif ch=='g': body='a%d(); goto out;'%k
        elif ch=='r': body='a%d(); return;'%k
        out.append(('if' if k==1 else 'else if')+' (g == %d) { %s }'%(k,body))
    decl=' '.join('void a%d(); void x%d(int, int, int);'%(k,k) for k in range(1,len(spec)+1))
    j=' '.join('jn(%d);'%i for i in range(1,joinsize+1))
    return '#include "probe.h"\nextern void jn(int); struct D { int v; D(); ~D(); }; void pp(int, int); void use2(int, int); %s\nvoid f() {\n %s %s\n %s\n out: use(9999);\n}\n'%(decl,'D d;' if eh else '','\n '.join(out),j)
def order(src,pfx):
    asm,_=ib.compile_asm(None,pfx,text=src)
    lines=[L for n,s,L in ib.functions(asm) if n=='f'][0]
    seq=[]
    for i,l in enumerate(lines):
        m=re.match(r'call\s+\?(a\d+|x\d+|jn|pp|use2|use)@',l)
        if not m: continue
        n=m.group(1)
        if n=='jn':
            if ib.last_push(lines,i)==1: seq.append('J')
        elif n=='pp': seq.append('*')
        elif n=='use2': seq.append('q')
        elif n=='use':
            if ib.last_push(lines,i)==9999: seq.append('O')
        elif n[0]=='x': seq.append('x')
        else: seq.append(n[1:])
    return ' '.join(seq),lines
if __name__=='__main__':
    s,lines=order(build(sys.argv[1],eh='--eh' in sys.argv),os.environ.get('FE_PFX_BASE','21'))
    print('%-12s %s'%(sys.argv[1],s))
    if '-v' in sys.argv: print('\n'.join(lines))
