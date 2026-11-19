# census of cross-jump merges in our compile: JJ (into a block that ends with jmp), FT (into a block that falls into a label), RET
# usage: jjcensus.py src/unit.cpp ... (FE_PFX_BASE..+4 lanes). Heuristic: a jmp into the middle of a statement whose block then ends with jmp = JJ.
import sys,os,re,json
R=os.path.abspath(os.path.dirname(os.path.abspath(__file__))+'/../../../..')
sys.path.insert(0,R+'/recovery/tools'); sys.path.insert(0,R+'/recovery/tools/analysis')
import fe, tailscan
from concurrent.futures import ThreadPoolExecutor
def scan_unit(unit,pfx):
    text=open(R+'/'+unit,encoding='latin-1').read()
    c,cod=tailscan.compile_listing(unit,R+'/'+unit,str(pfx))
    if c is None: return []
    out=[]
    for m in re.finditer(r'// ORIGINAL: 0x([0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)',text):
        addr=int(m.group(1),16); exact='BYTE_EXACT' in m.group(2)
        try:
            sym,lo,hi=fe.ident(text,addr); s=c.function(sym)
        except Exception: continue
        sec=c.sections[s.section-1]
        I=fe.dis(sec.data,0,sec.relocations,selfsec=s.section,syms=c.symbols)
        L=tailscan.lines_of(cod,sym,os.path.basename(unit).lower())
        idx={x[0]:i for i,x in enumerate(I)}
        labels={x[3] for x in I if x[3] is not None}
        jj=ft=0; sites=[]
        for i,x in enumerate(I):
            if not x[1].startswith('jmp') or x[3] is None: continue
            t=x[3]; k=idx.get(t)
            if k is None or k==0: continue
            p=I[k-1]
            if p[1].startswith(('jmp','ret')) or p[1].startswith('j'): continue
            if L.get(p[0])!=L.get(t) or L.get(t) is None: continue
            if L.get(x[0])==L.get(t): continue
            # walk
            j=k; kind=None
            while j<len(I):
                y=I[j]
                if j>k and y[0] in labels: kind='FT'; break
                if y[1].startswith('jmp'): kind='JJ'; break
                if y[1].startswith('ret'): kind='RET'; break
                j+=1
            if kind=='JJ': jj+=1; sites.append(('%x'%x[0],'%x'%t,L.get(x[0]),L.get(t)))
            elif kind=='FT': ft+=1
        out.append(dict(unit=unit,addr='%08X'%addr,sym=sym,exact=exact,jj=jj,ft=ft,sites=sites[:5]))
    return out
if __name__=='__main__':
    units=sys.argv[1:]
    fe.compile_cmd(units[0])
    def lane(k): 
        r=[]
        for u in units[k::5]: r+=scan_unit(u,21+k)
        return r
    res=[]
    with ThreadPoolExecutor(5) as ex:
        for o in ex.map(lane,range(5)): res+=o
    ex_=[r for r in res if r['exact']]; ne=[r for r in res if not r['exact']]
    print('exact fns',len(ex_),'with JJ',sum(1 for r in ex_ if r['jj']),'JJ sites',sum(r['jj'] for r in ex_),'FT',sum(r['ft'] for r in ex_))
    print('nonexact fns',len(ne),'with JJ',sum(1 for r in ne if r['jj']),'JJ sites',sum(r['jj'] for r in ne),'FT',sum(r['ft'] for r in ne))
    for r in ex_:
        if r['jj']: print('EX',r['addr'],r['unit'],r['sym'][:40],r['jj'],r['sites'][:3])
    for r in ne:
        if r['jj']: print('NE',r['addr'],r['unit'],r['sym'][:40],r['jj'],r['sites'][:3])
