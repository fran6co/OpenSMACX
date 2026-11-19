#!/usr/bin/env python3
"""loopscan.py [unit ...] [--addr 0xADDR] [--src ALT.cpp] [--all] [--struct] [--json FILE]
Loop-form census (MATCHING_GUIDE 2.4, "loop-form lever").

Earlier recoveries often hand-wrote what VC6's induction-variable optimiser derives itself: a running
pointer beside a counter, a coordinate stepped in the for-clause, a trip-count down-counter, a
`T *p = &Arr[i]` re-bound each iteration, sibling loops each declaring their own counter, a hand-written
memset/memcpy loop.  VC6 then cannot reproduce the image's induction variables, frame slots or loop tests.

For every non-exact function (same list as feall.py) this prints
  - the loops of OUR source that contain one of those shapes (line number and shape tag);
  - the loop tests (back-edge compare + jump) of the image and of our compile, classified as
      dec/jne        down-counter (subscript-only counter, or explicit `n--`)
      reg<&G  jl/jb  pointer IV against a data address (linear subscript: jl from an int counter, jb from a T*)
      reg<K   jl     index or byte-offset IV against a constant (K large = offset of an indexed array)
      reg<reg / reg<[m] / [m]<K   bound in a register / memory, counter in a slot
    with signed (jl/jge/...) or unsigned (jb/jae/...) jumps;
  - the frame size (`sub esp,N`) on both sides.
A function is marked STRUCT when the frame or the multiset of loop-test shapes differs; those come first.
Tags: ptr+ctr (pointer stepped next to an integer counter), clause+= (coordinate stepped in the for-clause),
down (down-counter used for trip count only), rebind (`T *p = &A[i]` / `T &r = A[i]` in the body),
sibling counters (one name declared by several for-loops; one function-scope counter may be the original), memloop (memset/memcpy-shaped body).
Run from the worktree root:  FE_PFX_BASE=27 FE_JOBS=5 uv run python recovery/tools/analysis/loopscan.py
Screen only; uses the fe.py compile (one per unit)."""
import sys,os,re,json,argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
S=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0,S); import fe
R=fe.R
SIGNED={'jl','jle','jg','jge'}; UNSIGNED={'jb','jbe','ja','jae'}

# ---------------- machine side ----------------
def operand_kind(o):
    o=o.strip()
    if o=='<r>' or re.fullmatch(r'0x[0-9a-f]{6,}',o): return '&G'
    if '[' in o: return '[G]' if re.search(r'\[(<r>|0x[0-9a-f]{6,})\]',o) else '[m]'
    if re.fullmatch(r'-?(0x[0-9a-f]+|\d+)',o): return 'K'
    return 'reg'
def loops(ins):
    """ins: [(off, text, size, tgt)] -> [dict(head, tail, test)] for each back-edge."""
    by={x[0]:i for i,x in enumerate(ins)}; out=[]
    for i,(off,t,sz,tgt) in enumerate(ins):
        if tgt is None or tgt>off or not t.startswith('j'): continue
        mn=t.split()[0]; k=i
        if mn=='jmp':  # top-tested loop: the test is the first conditional jump at the head
            j=by.get(tgt)
            if j is None: continue
            for k in range(j,min(j+8,len(ins))):
                if ins[k][1].startswith('j') and not ins[k][1].startswith('jmp'): break
            else: out.append(dict(head=tgt,tail=off,test='jmp')); continue
            mn=ins[k][1].split()[0]
        q=k-1
        while q>0 and ins[q][1].split()[0] in ('mov','lea','push','movsx','movzx','pop'): q-=1  # flag-neutral
        prev=ins[q][1] if q>=0 else ''
        pm=prev.split()[0] if prev else ''
        if pm in ('dec','inc','sub','add') and mn in ('jne','je','jg','jns','js','jge'):
            test='%s/%s'%(pm,mn)
        elif pm in ('cmp','test'):
            a=prev.split(None,1)[1].split(',',1) if ' ' in prev else ['','']
            if pm=='test': test='test/%s'%mn
            else: test='%s<%s %s'%(operand_kind(a[0]),operand_kind(a[1]) if len(a)>1 else '?',mn)
        else: test='?(%s) %s'%(pm,mn)
        if mn in SIGNED: test+=' s'
        elif mn in UNSIGNED: test+=' u'
        out.append(dict(head=tgt,tail=off,test=test,cmp=prev))
    return out
def frame(ins):
    for off,t,sz,tgt in ins[:12]:
        m=re.match(r'sub esp, (0x[0-9a-f]+|\d+)$',t)
        if m: return int(m.group(1),0)
        if t.startswith('call'): break
    return 0

# ---------------- source side ----------------
def strip_comments(t):
    def rep(m): return re.sub(r'[^\n]',' ',m.group(0))
    return re.sub(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',rep,t,flags=re.S)
def match(t,i,o,c):
    d=0
    for j in range(i,len(t)):
        if t[j]==o: d+=1
        elif t[j]==c:
            d-=1
            if d==0: return j
    return len(t)-1
def func_body(text,clean,addr):
    m=re.search(r'// ORIGINAL: 0x%08X [^\n]*\n((?://(?! ORIGINAL:)[^\n]*\n)*)'%addr,text)
    if not m or re.search(r'// body ',m.group(1)): return None
    i=clean.find('{',m.end())
    if i<0: return None
    j=match(clean,i,'{','}')
    return i,j
def stmt_end(t,i):
    """end index (inclusive) of the statement starting at i"""
    while i<len(t) and t[i].isspace(): i+=1
    if i<len(t) and t[i]=='{': return i,match(t,i,'{','}')
    m=re.match(r'(for|while|if|switch)\s*\(',t[i:])
    if m:
        p=match(t,i+m.end()-1,'(',')'); s,e=stmt_end(t,p+1)
        if m.group(1)=='if':
            k=e+1
            while k<len(t) and t[k].isspace(): k+=1
            if t.startswith('else',k): s2,e=stmt_end(t,k+4)
        return i,e
    if re.match(r'do\b',t[i:]):
        s,e=stmt_end(t,i+2); k=t.find(';',e); return i,k
    d=0
    for j in range(i,len(t)):
        if t[j] in '({[': d+=1
        elif t[j] in ')}]': d-=1
        elif t[j]==';' and d==0: return i,j
    return i,len(t)-1
def split_top(s,ch):
    out=[]; d=0; cur=''
    for c in s:
        if c in '([{': d+=1
        elif c in ')]}': d-=1
        if c==ch and d==0: out.append(cur); cur=''
        else: cur+=c
    out.append(cur); return out
DECL=r'(?:int|short|char|long|unsigned|signed|size_t|uint\d+_t|int\d+_t|BOOL|DWORD|WORD|BYTE|UINT)'
def src_loops(clean,lo,hi):
    body=clean[lo:hi+1]; base=lo
    ptrs=set(re.findall(r'(?:^|[;{}(])\s*(?:const\s+)?(?:(?:struct|unsigned|signed)\s+)?[A-Za-z_][\w:]*(?:\s+const)?\s*\*+\s*(?:const\s+)?([A-Za-z_]\w*)\s*(?=[=;,)\[])',body,re.M))
    sig=clean[max(0,clean.rfind('\n',0,clean.rfind(')',0,lo))-400):lo]
    ptrs|=set(re.findall(r'\*\s*([A-Za-z_]\w*)\s*(?=[,)])',sig[sig.rfind('('):] if '(' in sig else ''))
    res=[]
    for m in re.finditer(r'\b(for|while|do)\b',body):
        k=m.start(); kind=m.group(1)
        if kind=='while':
            q=body[:k].rstrip()
            if q.endswith('}'):  # tail of a do-while?
                o=0; d=0
                for j in range(len(q)-1,-1,-1):
                    if q[j]=='}': d+=1
                    elif q[j]=='{':
                        d-=1
                        if d==0: o=j; break
                if re.search(r'\bdo\s*$',q[:o]): continue
        if kind in ('for','while'):
            p0=body.find('(',m.end())
            if p0<0 or body[m.end():p0].strip(): continue
            p1=match(body,p0,'(',')'); head=body[p0+1:p1]; s,e=stmt_end(body,p1+1); blk=body[s:e+1]
            if kind=='for':
                parts=split_top(head,';')
                if len(parts)!=3: continue
                init,cond,incr=parts
            else: init,cond,incr='',head,''
        else:
            s,e=stmt_end(body,m.end()); blk=body[s:e+1]
            w=re.match(r'\s*while\s*\(',body[e+1:])
            if not w: continue
            p0=e+1+w.end()-1; p1=match(body,p0,'(',')'); init,cond,incr='',body[p0+1:p1],''
        line=clean.count('\n',0,base+k)+1
        tags=[]; inc=[x.strip() for x in split_top(incr,',') if x.strip()]
        ctrs=set(re.findall(r'(?:\+\+|--)\s*([A-Za-z_]\w*)|([A-Za-z_]\w*)\s*(?:\+\+|--|[+-]=)',incr))
        ctrs={a or b for a,b in ctrs}
        stepped=lambda v,t_: re.search(r'(?:\+\+|--)\s*%s\b|\b%s\s*(?:\+\+|--|[+-]=)|\b%s\s*=\s*%s\s*[+-]'%(v,v,v,v),t_)
        sp=[p for p in ptrs if stepped(p,incr) or stepped(p,blk)]
        ints=[c for c in ctrs if c not in ptrs]
        if sp and (ints or re.search(r'\b(\w+)\s*(?:<|<=|>|>=|!=)',cond) and not any(re.search(r'\b%s\b'%p,cond) for p in sp)):
            tags.append('ptr+ctr(%s)'%','.join(sorted(sp)))
        for x in inc:
            mm=re.match(r'([A-Za-z_][\w.]*)\s*([+-]=)\s*(.+)',x)
            if mm and mm.group(3).strip() not in ('1',) and len(inc)>=2 or (mm and not re.search(r'\b%s\b'%re.escape(mm.group(1)),cond)):
                tags.append('clause+=(%s)'%x)
        for v in set(re.findall(r'([A-Za-z_]\w*)\s*--|--\s*([A-Za-z_]\w*)',incr+' '+cond+' '+blk)):
            v=v[0] or v[1]
            uses=len(re.findall(r'\b%s\b'%v,blk))
            dec=len(re.findall(r'\b%s\s*--|--\s*%s\b'%(v,v),blk))
            if re.search(r'\b%s\b'%v,cond) and uses==dec: tags.append('down(%s)'%v)
        for mm in re.finditer(r'\b[A-Za-z_][\w:]*(?:\s+const)?\s*([*&])\s*([A-Za-z_]\w*)\s*=\s*(&?\s*[A-Za-z_][\w.:>-]*\s*\[[^\]]*\][^;]*);',blk):
            tags.append('rebind(%s%s=%s)'%(mm.group(1),mm.group(2),re.sub(r'\s+',' ',mm.group(3))[:40]))
        inner=blk.strip()
        if inner.startswith('{'): inner=inner[1:-1].strip()
        st=[x for x in split_top(inner,';') if x.strip()]
        if len(st)<=2 and st and all(re.fullmatch(r'\s*(\*\s*\w+\s*\+\+|[\w.>-]+\s*\[[^\]]+\](?:\.\w+|\[[^\]]+\])*)\s*=\s*(-?\w+|0x[0-9a-fA-F]+|\*\s*\w+\s*\+\+|[\w.>-]+\s*\[[^\]]+\](?:\.\w+|\[[^\]]+\])*)\s*',x) for x in st):
            tags.append('memloop')
        res.append(dict(line=line,kind=kind,head=re.sub(r'\s+',' ',(init+';'+cond+';'+incr) if kind=='for' else cond).strip()[:70],tags=tags,decl=re.match(r'\s*%s\s+(\w+)'%DECL,init) and re.match(r'\s*%s\s+(\w+)'%DECL,init).group(1)))
    return res
def siblings(sl):
    """names declared as counters by more than one loop of the function"""
    c=Counter(x['decl'] for x in sl if x['decl'])
    return {k:v for k,v in c.items() if v>=2}

def markers(text,all_):
    return [int(m.group(1),16) for m in re.finditer(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)',text) if all_ or 'BYTE_EXACT' not in m.group(2)]
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('units',nargs='*'); ap.add_argument('--addr'); ap.add_argument('--src'); ap.add_argument('--all',action='store_true')
    ap.add_argument('--struct',action='store_true',help='only functions whose frame or loop tests differ'); ap.add_argument('--tagged',action='store_true',help='only functions with a tagged source loop')
    ap.add_argument('--json'); ap.add_argument('--jobs',type=int,default=int(os.environ.get('FE_JOBS','14')))
    a=ap.parse_args(); base=int(os.environ.get('FE_PFX_BASE','21'))
    cc=json.load(open(fe.MAIN+'/build/compile_commands.json'))
    units=[e['file'][len(fe.MAIN)+1:] for e in cc if e['file'].startswith(fe.MAIN+'/src/')]
    if a.units: units=[u for u in units if u in a.units or os.path.basename(u) in a.units or u[4:] in a.units]
    work=[]
    for u in units:
        p=os.path.abspath(a.src) if a.src else R+'/'+u
        try: t=open(p,encoding='latin-1').read()
        except OSError: continue
        ad=[int(a.addr,16)] if a.addr else markers(t,a.all)
        ad=[x for x in ad if re.search(r'// ORIGINAL: 0x%08X'%x,t)]
        if ad: work.append((u,p,t,ad))
    fe.compile_cmd(work[0][0]) if work else None
    def lane(k): return [(work[i],fe.compile_unit(work[i][0],work[i][1],base+k)) for i in range(k,len(work),a.jobs)]
    comp=[]
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane,range(a.jobs)): comp+=o
    found=[]
    for (u,p,t,ad),(c,err) in comp:
        if c is None: print('COMPILE FAIL',u,(err or '')[:200],file=sys.stderr); continue
        clean=strip_comments(t)
        for addr in ad:
            r=fe.evaluate(c,t,addr)
            if 'error' in r or (r['exact'] and not a.addr and not a.all): continue
            O=[x[1] for x in r['rows'] if x[1] is not None]; C=[x[2] for x in r['rows'] if x[2] is not None]
            li,lo_=loops(O),loops(C)
            fi,fo=frame(O),frame(C)
            sl=[]
            fb=func_body(t,clean,addr)
            if fb: sl=src_loops(clean,*fb)
            tagged=[x for x in sl if x['tags']]
            ci,co=Counter(x['test'] for x in li),Counter(x['test'] for x in lo_)
            struct=fi!=fo or ci!=co
            if a.struct and not struct: continue
            if a.tagged and not tagged: continue
            if not (li or lo_ or sl): continue
            found.append(dict(unit=u,addr=addr,sym=r['sym'],diff=r['diff'],img=r['img'],struct=struct,frame=[fi,fo],
                              img_loops=[dict(at='%x'%(r['lo']+x['head']),test=x['test'],cmp=x.get('cmp','')) for x in li],
                              our_loops=[dict(at='+%x'%x['head'],test=x['test'],cmp=x.get('cmp','')) for x in lo_],src=sl,
                              img_only=sorted((ci-co).elements()),ours_only=sorted((co-ci).elements()),sibling=siblings(sl)))
    found.sort(key=lambda f:(not f['struct'],not any(x['tags'] for x in f['src']),-f['diff']))
    tally=Counter()
    for f in found:
        print('%08X %-22s %-40s diff=%d/%d%s frame img %#x ours %#x'%(f['addr'],f['unit'][4:],f['sym'][:40],f['diff'],f['img'],' STRUCT' if f['struct'] else '',f['frame'][0],f['frame'][1]))
        if len(f['img_loops'])<=12:
            print('   img : '+'; '.join('%s %s'%(x['at'],x['test']) for x in f['img_loops']))
            print('   ours: '+'; '.join('%s %s'%(x['at'],x['test']) for x in f['our_loops']))
        if f['img_only'] or f['ours_only']:
            print('   loop tests img-only: %s | ours-only: %s'%(', '.join(f['img_only']) or '-',', '.join(f['ours_only']) or '-'))
            for x in f['img_loops']:
                if x['test'] in f['img_only']: print('      img %s  %s'%(x['at'],x['cmp']))
        if f['sibling']: print('   sibling counters: '+' '.join('%s x%d'%kv for kv in sorted(f['sibling'].items())))
        for x in f['src']:
            if x['tags']:
                for tg in x['tags']: tally[tg.split('(')[0]]+=1
                print('   L%-5d %-5s %-50s %s'%(x['line'],x['kind'],x['head'][:50],' '.join(x['tags'])))
    print('functions',len(found),'struct',sum(f['struct'] for f in found),' '.join('%s=%d'%kv for kv in sorted(tally.items())))
    if a.json: json.dump(found,open(a.json,'w'),indent=0)
if __name__=='__main__': main()
