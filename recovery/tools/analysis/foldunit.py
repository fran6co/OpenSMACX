"""foldunit.py unit : greedy pass of the foldsite.py rewrite over every function of a unit (keeps a site when rows do not rise);
writes the result to build/recovery/fe/foldunit_out.cpp.  Prefix FE_PFX_BASE."""
import sys,os,re
S=os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0,S)
import accsweep as A
fe=A.fe
unit=sys.argv[1]; pfx=os.environ.get('FE_PFX_BASE','2')
t=open(A.R+'/'+unit,encoding='latin-1').read()
pat=re.compile(r'([ \t]*)PlayersData\[([^\[\]]+)\]\.diplo_(treaties|agenda)\[([^\[\]]+)\] (&=|\|=) (~?)(\w+);\n\s*PlayersData\[\4\]\.diplo_\3\[\2\] \5 \6\7;\n')
skipfn=('treaty_off','treaty_on','agenda_on','agenda_off','set_treaty','set_agenda')
def rep(m):
    ind,x,kind,y,op,neg,k=m.groups()
    if (op=='&=') != bool(neg): return m.group(0)
    fn={('treaties','&='):'treaty_off',('treaties','|='):'treaty_on',('agenda','&='):'agenda_off',('agenda','|='):'agenda_on'}[(kind,op)]
    return '%s%s(%s, %s, %s);\n'%(ind,fn,x,y,k.rstrip('uU'))
marks=[int(x.group(1),16) for x in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) (\S+)',t)]
c0,_=fe.compile_unit(unit,A.R+'/'+unit,pfx)
for addr in marks:
    try: a,b=A.span(t,addr)
    except Exception: continue
    g=t[a:b]
    if any(re.search(r'\b%s\('%f,g[:200]) for f in skipfn): continue
    sites=list(pat.finditer(g))
    if not sites: continue
    r0=fe.evaluate(c0,t,addr)
    for m in reversed(sites):
        r=rep(m)
        if r==m.group(0): continue
        t2=t[:a]+g[:m.start()]+r+g[m.end():]+t[b:]
        p='%s/build/recovery/fe/fu%s.cpp'%(A.R,pfx); open(p,'w',encoding='latin-1').write(t2)
        c,err=fe.compile_unit(unit,p,pfx)
        if c is None: print('FAIL',r.strip(),err[:200]); continue
        r1=fe.evaluate(c,t2,addr)
        print('%08X %-22s %-45s %d -> %d'%(addr,fe.ident(t,addr)[0][:22],r.strip(),r0['diff'],r1['diff']))
        if r1['diff']<=r0['diff']:
            t=t2; g=t[a:b]; b=a+len(g) if False else b-(len(m.group(0))-len(r)); g=t[a:b]; r0=r1; c0=c
open(A.R+'/build/recovery/fe/foldunit_out.cpp','w',encoding='latin-1').write(t)
