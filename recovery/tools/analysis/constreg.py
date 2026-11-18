#!/usr/bin/env python3
"""constreg.py [unit ...] [--addr 0xADDR] [--src ALT.cpp] [--all] [--agree] [--json FILE]
Constant-register diagnosis (MATCHING_GUIDE "Constant registers").

For every non-exact function (same list as feall.py) find, on the image side and on ours, each
stretch where a callee-saved register holds a constant: a `xor reg,reg` / `or reg,-1` /
`mov reg,K` after which the register is only read (`push reg`, `mov [mem],reg`, `cmp x,reg`, ...)
until it is written again.  Print the functions where the two sides disagree:

  IMG_ONLY   the image dedicates a register to K, ours does not  -> a store of K is missing in our source
  OURS_ONLY  ours dedicates a register, the image does not       -> one store too many, or the register is not free
  START      both do, but from a different point (ours is given as the aligned image address)
  REG        both do, from the same point, in a different register

Each row gives the constant, the register, the start address and the uses of the register inside the
stretch (n = instructions it spans, st = memory stores, pu = pushes, cm = compares, ot = other reads), then the function-wide
count of immediate uses of the same constant on each side (`mov [mem],K` stores, `push K`, `cmp x,K`).
VC6's threshold is four memory stores (loop x2, nested x4, counted before merging), so
"imm st" on our side against "st" in the image's stretch shows how many stores are missing.

Short local stretches (a known-zero register reused for two stores) are hidden unless --weak.
Run from the worktree root:  FE_PFX_BASE=27 FE_JOBS=5 uv run python recovery/tools/analysis/constreg.py
Screen only; uses the fe.py compile (one per unit)."""
import sys,os,re,json,argparse
from concurrent.futures import ThreadPoolExecutor
S=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0,S); import fe
R=fe.R
NEAR=0x60
SUB={'ebx':('ebx','bx','bl','bh'),'esi':('esi','si'),'edi':('edi','di'),'ebp':('ebp','bp')}
OWNER={s:r for r,ss in SUB.items() for s in ss}
WRITERS={'mov','lea','xor','or','and','add','sub','inc','dec','pop','movsx','movzx','imul','shl','shr','sar','neg','not','sbb','adc','xchg','rol','ror','shld','shrd','bsf','bsr'}
SELF={'add','sub','inc','dec','and','or','xor','shl','shr','sar','neg','not','sbb','adc','imul','rol','ror','lea'}
def sval(tok,bits=32):
    try: v=int(tok,0)
    except ValueError: return None
    v&=(1<<bits)-1
    return v-(1<<bits) if v>>(bits-1) else v
def ops_of(text):
    mn,_,rest=text.partition(' ')
    return mn,[o.strip() for o in rest.split(',')] if rest.strip() else []
def width(op):
    if op.startswith('byte'): return 8
    if op.startswith('word'): return 16
    return 32
def ismem(op): return '[' in op
def analyse(ins):
    """ins: list of (addr, text).  -> (segments, imm) where
    segments = [dict(reg,k,start,end,st,pu,cm,ot,var)], imm = {k: [stores, pushes, compares]}"""
    segs=[]; cur={}; imm={}
    def close(reg,addr,var):
        s=cur.pop(reg,None)
        if s is not None: s['end']=addr; s['var']=var; segs.append(s)
    for addr,text in ins:
        for s_ in cur.values(): s_['n']+=1
        mn,ops=ops_of(text)
        if mn.startswith('rep'): mn,ops=ops_of(text.split(' ',1)[1])
        # immediate uses of constants
        if mn=='mov' and len(ops)==2 and ismem(ops[0]):
            v=sval(ops[1],width(ops[0]))
            if v is not None: imm.setdefault(v,[0,0,0])[0]+=1
        elif mn in('and','or') and len(ops)==2 and ismem(ops[0]):
            v=sval(ops[1],width(ops[0]))
            if (mn,v) in(('and',0),('or',-1)): imm.setdefault(v,[0,0,0])[0]+=1
        elif mn=='push' and ops:
            v=sval(ops[0])
            if v is not None: imm.setdefault(v,[0,0,0])[1]+=1
        elif mn=='cmp' and len(ops)==2:
            v=sval(ops[1],width(ops[0]) if ismem(ops[0]) else 32)
            if v is not None: imm.setdefault(v,[0,0,0])[2]+=1
        # string instructions write esi/edi
        if mn[:4] in('movs','cmps','lods') and not ops or mn in('movsd','movsb','movsw','cmpsb','cmpsd','lodsb','lodsd'):
            if mn[:4]!='movs' or not ops or not ops[0].startswith('x'):
                close('esi',addr,True); close('edi',addr,True); continue
        if mn in('stosd','stosb','stosw','scasb','scasd','scasw'): close('edi',addr,True); continue
        dst=OWNER.get(ops[0]) if ops else None
        # reads of live constant registers
        for reg,s in list(cur.items()):
            names=SUB[reg]
            def has(o): return any(re.search(r'\b%s\b'%n,o) for n in names)
            if not any(has(o) for o in ops): continue
            if dst==reg and (mn in WRITERS or mn.startswith('set') or mn.startswith('cmov')):
                continue  # handled below as a write
            if mn=='push' and ops[0] in names: s['pu']+=1
            elif mn=='mov' and len(ops)==2 and ismem(ops[0]) and ops[1] in names: s['st']+=1
            elif mn=='cmp' and (ops[0] in names or ops[1] in names): s['cm']+=1
            elif mn=='xchg' and len(ops)==2 and ops[1] in names: close(reg,addr,True)
            else: s['ot']+=1
        if dst and (mn in WRITERS or mn.startswith('set') or mn.startswith('cmov')):
            k=None
            if ops[0]==dst:
                if mn=='xor' and ops[1]==dst: k=0
                elif mn=='or' and sval(ops[1])==-1: k=-1
                elif mn=='mov' and not ismem(ops[1]): k=sval(ops[1])
            selfmod=mn in SELF and k is None and (mn in('inc','dec','neg','not') or any(n in o for o in ops[1:] for n in SUB[dst]) or mn!='lea')
            close(dst,addr,selfmod)
            if k is not None: cur[dst]=dict(reg=dst,k=k,start=addr,st=0,pu=0,cm=0,ot=0,n=0)
    for reg in list(cur): close(reg,None,False)
    return segs,imm
def dedicated(s):
    """A stretch counts as a constant register when the register is used as the constant in memory
    stores, and is not simply a counter that is still zero."""
    uses=s['st']+s['pu']+s['cm']
    if s['var']: return s['st']>=3
    return s['st']>=1 and uses>=2
def strong(s):
    """Function-scale stretch (the four-store rule), as opposed to a local known-zero reuse."""
    return s is not None and (s['st']>=3 or s['n']>=60)
def sides(res):
    rows=res['rows']; lo=res['lo']
    O=[(lo+o[0],o[1]) for k,o,c in rows if o]
    C=[(c[0],c[1]) for k,o,c in rows if c]
    nxt=None; omap={}
    for k,o,c in reversed(rows):
        if o: nxt=lo+o[0]
        if c: omap[c[0]]=nxt
    return O,C,omap
def compare(res):
    O,C,omap=sides(res)
    si,ii=analyse(O); so,io=analyse(C)
    si=[s for s in si if dedicated(s)]; so=[s for s in so if dedicated(s)]
    for s in so: s['at']=omap.get(s['start'])
    out=[]; used=set()
    # pair stretches of the same constant: same aligned start first, then the nearest start
    # within NEAR bytes (a START row); anything else is one-sided
    pair={}
    for near in (0,NEAR):
        for i,s in enumerate(si):
            if i in pair: continue
            cands=[(abs((t['at'] or 0)-s['start']),j) for j,t in enumerate(so) if j not in used and t['k']==s['k']]
            cands=[c for c in cands if c[0]<=near]
            if cands: d,j=min(cands); pair[i]=j; used.add(j)
    for i,s in enumerate(si):
        if i not in pair: out.append(('IMG_ONLY',s,None)); continue
        t=so[pair[i]]
        if t['at']!=s['start']: out.append(('START',s,t))
        elif t['reg']!=s['reg']: out.append(('REG',s,t))
        else: out.append(('same',s,t))
    for j,t in enumerate(so):
        if j not in used: out.append(('OURS_ONLY',None,t))
    return out,ii,io
def fmt(s,ours=False):
    if s is None: return '-'
    a=('@%x(+%x)'%(s['at'] or 0,s['start'])) if ours else '@%x'%s['start']
    return '%s %s n%d st%d pu%d cm%d ot%d%s'%(s['reg'],a,s['n'],s['st'],s['pu'],s['cm'],s['ot'],' var' if s['var'] else '')
def markers(text,all_):
    return [int(m.group(1),16) for m in re.finditer(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)',text) if all_ or 'BYTE_EXACT' not in m.group(2)]
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('units',nargs='*'); ap.add_argument('--addr'); ap.add_argument('--src'); ap.add_argument('--all',action='store_true')
    ap.add_argument('--agree',action='store_true',help='also print stretches that agree'); ap.add_argument('--json'); ap.add_argument('--weak',action='store_true',help='also print short local stretches (fewer than 3 stores and under 60 instructions)')
    ap.add_argument('--jobs',type=int,default=int(os.environ.get('FE_JOBS','14')))
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
        if ad: work.append((u,p,t,ad))
    fe.compile_cmd(work[0][0]) if work else None  # fill the command cache before the lanes start
    def lane(k): return [(work[i],fe.compile_unit(work[i][0],work[i][1],base+k)) for i in range(k,len(work),a.jobs)]
    comp=[]
    with ThreadPoolExecutor(a.jobs) as ex:
        for o in ex.map(lane,range(a.jobs)): comp+=o
    found=[]
    for (u,p,t,ad),(c,err) in comp:
        if c is None: print('COMPILE FAIL',u,(err or '')[:200],file=sys.stderr); continue
        for addr in ad:
            r=fe.evaluate(c,t,addr)
            if 'error' in r or (r['exact'] and not a.addr and not a.all): continue
            out,ii,io=compare(r)
            if not a.agree and not a.addr: out=[x for x in out if x[0]!='same']
            if not a.weak and not a.addr: out=[x for x in out if strong(x[1]) or strong(x[2])]
            if out: found.append(dict(unit=u,addr=addr,sym=r['sym'],diff=r['diff'],img=r['img'],rows=[dict(kind=k,k=(s or t_)['k'],img=s,ours=t_,imm_img=ii.get((s or t_)['k'],[0,0,0]),imm_ours=io.get((s or t_)['k'],[0,0,0])) for k,s,t_ in out]))
    found.sort(key=lambda f:-f['diff'])
    tally={}
    for f in found:
        print('%08X %-22s %-44s diff=%d/%d'%(f['addr'],f['unit'][4:],f['sym'][:44],f['diff'],f['img']))
        for x in f['rows']:
            tally[x['kind']]=tally.get(x['kind'],0)+1
            print('   %-9s K=%-6s img: %-40s ours: %-46s imm st/pu/cm img %d/%d/%d ours %d/%d/%d'%((x['kind'],'%#x'%x['k'] if abs(x['k'])>9 else x['k'],fmt(x['img']),fmt(x['ours'],True))+tuple(x['imm_img'])+tuple(x['imm_ours'])))
    print('functions',len(found),' '.join('%s=%d'%kv for kv in sorted(tally.items())))
    if a.json: json.dump(found,open(a.json,'w'),indent=0)
if __name__=='__main__': main()
