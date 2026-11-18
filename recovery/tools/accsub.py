#!/usr/bin/env python3
"""accsub.py <unit> <pfx> [--write] : try replacing each inline accessor call in non-exact functions of the
unit by the member it returns; accept a site when no fe-exact function is lost and the unit's total diff drops.
Run under `uv run python`."""
import sys,re,os,glob,json
S=os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0,S); import fe
R=fe.R
unit=sys.argv[1]; unit=unit if unit.startswith('src/') else 'src/'+unit; pfx=sys.argv[2]
acc={}
for p in glob.glob(R+'/src/**/*.h',recursive=True):
    t=open(p,encoding='latin-1').read()
    for m in re.finditer(r'\b[\w:<>]+ ?[&\*]? ?&?(\w+)\(\) (?:const )?\{ return (&?)(\w+_); \}',t):
        acc.setdefault(m.group(1),set()).add((m.group(2),m.group(3)))
acc={k:list(v)[0] for k,v in acc.items() if len(v)==1 and k not in ('height','width','transparent_index','pixel_height')}
path=R+'/'+unit; text=open(path,encoding='latin-1').read()
marks=[(m.start(),int(m.group(1),16),'BYTE_EXACT' in m.group(2)) for m in re.finditer(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)',text)]
addrs=[a for _,a,_ in marks]
tmp=fe.SCR+'/acc_%s_%s.cpp'%(os.path.basename(unit).replace('.','_'),pfx)
def score(t):
    open(tmp,'w',encoding='latin-1').write(t)
    c,err=fe.compile_unit(unit,tmp,pfx)
    if c is None: return None
    ex=set(); tot=0; per={}
    for a in addrs:
        r=fe.evaluate(c,t,a)
        if 'error' in r: continue
        per[a]=r['diff']; tot+=r['diff']
        if r['exact']: ex.add(a)
    return ex,tot,per
def in_nonexact(t,pos):
    ms=[(m.start(),'BYTE_EXACT' in m.group(1)) for m in re.finditer(r'// ORIGINAL: 0x[0-9A-F]{8} \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)',t)]
    prev=[x for x in ms if x[0]<pos]
    return bool(prev) and not prev[-1][1]
def expr_start(t,i):
    # scan backward from i (index of the '.'/'-' before name) over a postfix expression
    j=i
    while j>0:
        c=t[j-1]
        if c.isalnum() or c=='_' or c=='.' : j-=1
        elif c=='>' and t[j-2]=='-': j-=2
        elif c==':' and t[j-2]==':': j-=2
        elif c in ')]':
            d=0; k=j-1; op={')':'(',']':'['}[c]
            while k>=0:
                if t[k]==c: d+=1
                elif t[k]==op:
                    d-=1
                    if d==0: break
                k-=1
            if k<0: break
            j=k
        else: break
    return j
rx=re.compile(r'(\.|->)(%s)\(\)'%'|'.join(map(re.escape,acc)))
base=score(text)
if base is None: print(unit,'COMPILE FAIL'); sys.exit(0)
cur=text; pos=0; accd=[]; tried=0
while True:
    m=rx.search(cur,pos)
    if not m: break
    if not in_nonexact(cur,m.start()): pos=m.end(); continue
    amp,member=acc[m.group(2)]
    after=cur[m.end():m.end()+2]
    if amp:
        if after=='->': cand=cur[:m.start()]+m.group(1)+member+'.'+cur[m.end()+2:]
        else:
            s0=expr_start(cur,m.start()); cand=cur[:s0]+'&'+cur[s0:m.start()]+m.group(1)+member+cur[m.end():]
    else:
        cand=cur[:m.start()]+m.group(1)+member+cur[m.end():]
    tried+=1
    s=score(cand)
    if s and not (base[0]-s[0]) and s[1]<base[1]:
        line=cur.count('\n',0,m.start())+1
        ch=[('%08X'%a,base[2][a],s[2][a]) for a in s[2] if s[2][a]!=base[2].get(a)]
        accd.append((line,m.group(2),ch,['%08X'%a for a in s[0]-base[0]]))
        cur=cand; base=s; pos=m.start()+1
    else: pos=m.end()
print(json.dumps(dict(unit=unit,tried=tried,accepted=accd,total=base[1])))
if '--write' in sys.argv and accd: open(path,'w',encoding='latin-1').write(cur)
