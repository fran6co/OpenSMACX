#!/usr/bin/env python3
"""slotflow.py <unit> <addr> [--src ALT] [--pfx N]: transcription-error screen.
For every image instruction that reads a frame slot and is aligned with one of ours reading a named variable v,
take the image's reaching definitions of that slot; map each definition through the fe alignment to our
instruction and the variable it writes (from our /FAsc names). If none of them is v, our source probably reads
the wrong variable there (live-range packing let two variables share a slot and the transcription picked one)."""
import sys,os,re,collections,argparse
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
sys.path.insert(0,R+'/recovery/tools'); sys.path.insert(0,R+'/recovery/tools/analysis'); os.chdir(R)
import fe, regprio, regmap, candset
ap=argparse.ArgumentParser(); ap.add_argument('unit'); ap.add_argument('addr'); ap.add_argument('--src'); ap.add_argument('--pfx',default=os.environ.get('FE_PFX_BASE','29'))
a=ap.parse_args()
unit=a.unit; addr=int(a.addr,16); src=os.path.abspath(a.src) if a.src else R+'/'+unit
text=open(src,encoding='latin-1').read()
c,err=fe.compile_unit(unit,src,a.pfx)
if c is None: sys.exit('COMPILE FAIL '+str(err))
r=fe.evaluate(c,text,addr); rows=r['rows']
items,frame=regprio.cod_listing(unit,src,a.pfx,r['sym'])
F=regprio.Func(items); lst={off:(ln,tx) for off,ln,tx in F.ins}
otab={}
for off,ln,tx in F.ins:
    m=re.search(r'(\$L\d+)\[',tx)
    if tx.startswith('jmp') and m and m.group(1) in F.tables:
        otab[off]=[F.labels[x] for x in F.tables[m.group(1)] if x in F.labels]
oins=[(x[0],x[1],x[3]) for k,o,x in rows if x is not None]
iins=[(x[0],x[1],x[3]) for k,x,cc in rows if x is not None]
ocode=candset.Code(oins,otab)
icode=candset.Code(iins,candset.image_tables(r['lo'],r['lo']+max(o[0] for o in iins)+16,rows))
i2o={}; o2i={}
for k,o,cc in rows:
    if o is not None and cc is not None: i2o[o[0]]=cc[0]; o2i[cc[0]]=o[0]
SL=re.compile(r'\[ebp ([-+]) (0x[0-9a-f]+|\d+)\]')
def slotacc(t):
    """-> (slot, reads, writes) for a dword frame access, else None"""
    m=SL.search(t)
    if not m: return None
    s=int(m.group(2),0)*(-1 if m.group(1)=='-' else 1)
    mn=t.split(' ',1)[0]; ops=[x.strip() for x in t.split(' ',1)[1].split(',')]
    if mn=='lea': return (s,'addr')
    dst=SL.search(ops[0]) is not None
    if dst:
        if mn=='mov': return (s,False,True)
        if mn in ('cmp','test','push'): return (s,True,False)
        return (s,True,True)
    return (s,True,False)
def name_at(off):
    if off not in lst: return None
    m=re.search(r'_(\w+)\$(?:\d+)?\[ebp\]',lst[off][1])
    if m: return m.group(1)
    m=re.search(r'(\$T\d+|\$S\d+)\[ebp\]',lst[off][1])
    return m.group(1) if m else None
def reaching(code):
    n=len(code.ins); acc=[slotacc(t) for _,t,_ in code.ins]
    addr_taken={x[0] for x in acc if x and x[1]=='addr'}
    defs=[i for i in range(n) if acc[i] and acc[i][1]!='addr' and acc[i][2]]
    did={d:k for k,d in enumerate(defs)}
    slotdefs=collections.defaultdict(int)
    for d in defs: slotdefs[acc[d][0]]|=1<<did[d]
    IN=[0]*n; OUT=[0]*n; work=list(range(n)); inq=[True]*n
    while work:
        i=work.pop(); inq[i]=False
        x=0
        for p in code.ipred[i]: x|=OUT[p]
        IN[i]=x
        o=x
        if acc[i] and acc[i][1]!='addr' and acc[i][2]: o=(x&~slotdefs[acc[i][0]])|(1<<did[i])
        if o!=OUT[i]:
            OUT[i]=o
            for s in code.isucc[i]:
                if not inq[s]: inq[s]=True; work.append(s)
    return acc,defs,IN,slotdefs,addr_taken
iacc,idefs,iIN,islotdefs,iaddr=reaching(icode)
oacc,odefs,oIN,oslotdefs,oaddr=reaching(ocode)
lines=text.split('\n')
hits=collections.defaultdict(list)
for i,(off,t,g) in enumerate(icode.ins):
    a_=iacc[i]
    if not a_ or a_[1]=='addr' or not a_[1]: continue
    if a_[0] in iaddr: continue
    oo=i2o.get(off)
    if oo is None or oo not in lst: continue
    v=name_at(oo)
    if not v: continue
    rd=[idefs[k] for k in range(len(idefs)) if (iIN[i]>>k)&1 and iacc[idefs[k]][0]==a_[0]]
    ws=collections.Counter()
    for d in rd:
        od=i2o.get(icode.ins[d][0])
        if od is None: continue
        w=name_at(od)
        if w and slotacc(lst[od][1].split(';')[0] if False else [x for x in ocode.ins if x[0]==od][0][1]): ws[w]+=1
    if ws and v not in ws:
        ln=lst[oo][0]
        hits[(v,tuple(sorted(ws)))].append((off+r['lo'],ln))
for (v,ws),L in sorted(hits.items(),key=lambda x:x[1][0][0]):
    print('READ %-22s image value written as %-30s at %s' % (v, ','.join(ws), ' '.join('%x(L%d)'%x for x in L[:6])))
