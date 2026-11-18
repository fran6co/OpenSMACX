#!/usr/bin/env python3
"""fe.py <unit> <addr[,addr]> [--src ALT] [--all] [--ctx N] [--pfx N] [--quiet] [--win lo-hi,lo-hi] [--batch]
Fast evaluator: raw-compile one unit (optionally from ALT source text), disassemble each function's
COMDAT and the image span, and diff them with relocations masked.
Summary line: 'FE <addr> <sym> exact=<bool> diff=<n> img=<n> ours=<n> ratio=<r>' (and 'WIN n n' with --win).
--batch: read ALT source paths from stdin, one per line; print one 'B <win...> <diff> <exact>' line each.
This is a screen only: confirm with `recover measure` and bank with `recover record`."""
import sys, os, re, json, subprocess, struct, difflib, argparse, hashlib
R=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SCR=os.environ.get('FE_SCRATCH', R+'/build/recovery/fe')
sys.path.insert(0, R+'/packages/match-recovery/src')
from match_recovery.pe_coff import Coff
import capstone
os.makedirs(SCR,exist_ok=True)
_img=open(R+'/.opensmacx/game/terranx_original.exe','rb').read()
_pe=struct.unpack_from('<I',_img,0x3c)[0]; _nsec=struct.unpack_from('<H',_img,_pe+6)[0]; _opt=struct.unpack_from('<H',_img,_pe+20)[0]
_base=struct.unpack_from('<I',_img,_pe+24+28)[0]; _secs=[]
for _i in range(_nsec):
    _o=_pe+24+_opt+40*_i; _vs,_va,_rs,_ro=struct.unpack_from('<IIII',_img,_o+8); _secs.append((_base+_va,_vs,_ro))
def imgbytes(lo,hi):
    for va,vs,ro in _secs:
        if va<=lo<va+vs: return _img[ro+lo-va:ro+hi-va]
md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
_CC=None
def compile_cmd(unit):
    global _CC
    if _CC is None: _CC={e['file']:e['command'].split() for e in json.load(open(R+'/build/compile_commands.json'))}
    return _CC.get(R+'/'+unit)
SENT=0x5A5A5A5A
def dis(data,org,relocs=(),lo=None,hi=None,selfsec=None,syms=None):
    rs=sorted(r[0] for r in relocs); res=[]; end=len(data)
    rmap={r[0]:r for r in relocs}
    if relocs:
        data=bytearray(data)
        for r in rs: data[r:r+4]=struct.pack('<I',SENT)
        data=bytes(data)
    num=r'(?:0x[0-9a-f]+|\d+)'
    addr=lambda m_: '<r>' if 0x400000<=int(m_.group(0),16)<0xa00000 else m_.group(0)
    for ins in md.disasm(data,org):
        off=ins.address-org
        if off>=end: break
        mine=[r for r in rs if off<=r<off+ins.size]; masked=bool(mine)
        if selfsec is not None:
            for r in mine:
                _,si,ty=rmap[r]; sy=syms.get(si)
                if ty==6 and sy is not None and sy.section==selfsec and sy.value>off: end=min(end,sy.value)
        op=ins.op_str; tgt=None
        if ins.mnemonic=='call' or ins.mnemonic.startswith('j') or ins.mnemonic.startswith('loop'):
            if re.fullmatch(num,op):
                t=int(op,0)
                if masked or (lo is not None and (not (lo<=t<hi) or t==lo)): op='<ext>'
                else: op='L'; tgt=t-org
            else:
                m=re.search(r'\*4 \+ ('+num+r')\]',op)
                if m and ins.mnemonic=='jmp' and lo is not None and lo<=int(m.group(1),0)<hi:
                    end=min(end,int(m.group(1),0)-org)
                op=op.replace('0x%x'%SENT,'<r>'); op=re.sub(r'0x[0-9a-f]{6,}',addr,op)
        else:
            op=op.replace('0x%x'%SENT,'<r>'); op=re.sub(r'0x[0-9a-f]{6,}',addr,op)
        op=op.replace('fs:[<r>]','fs:[0]')
        res.append((off,ins.mnemonic+' '+op,ins.size,tgt))
    res=[x for x in res if x[0]<end]
    while res and not res[-1][1].startswith(('ret','jmp')): res.pop()
    return res
def align(A,B):
    n,m=len(A),len(B)
    if n*m>1500000:
        return difflib.SequenceMatcher(None,A,B,autojunk=False).get_opcodes()
    L=[[0]*(m+1) for _ in range(n+1)]
    for i in range(n-1,-1,-1):
        Li=L[i]; Ln=L[i+1]; ai=A[i]
        for j in range(m-1,-1,-1):
            Li[j]=Ln[j+1]+1 if ai==B[j] else (Ln[j] if Ln[j]>=Li[j+1] else Li[j+1])
    ops=[]; i=j=0
    while i<n and j<m:
        if A[i]==B[j]: ops.append(('equal',i,i+1,j,j+1)); i+=1; j+=1
        elif L[i+1][j]>=L[i][j+1]: ops.append(('delete',i,i+1,j,j)); i+=1
        else: ops.append(('insert',i,i,j,j+1)); j+=1
    if i<n: ops.append(('delete',i,n,j,j))
    if j<m: ops.append(('insert',i,i,j,m))
    out=[]
    for t,i1,i2,j1,j2 in ops:
        if out and (out[-1][0]!='equal') and t!='equal':
            o=out[-1]; out[-1]=('replace',o[1],i2,o[3],j2)
        elif out and out[-1][0]=='equal' and t=='equal':
            o=out[-1]; out[-1]=('equal',o[1],i2,o[3],j2)
        else: out.append((t,i1,i2,j1,j2))
    return out
def ident(text,addr):
    m=re.search(r'// ORIGINAL: 0x%08X (\S+) 0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)[^\n]*\n((?://[^\n]*\n)*)'%addr,text)
    if not m: return None
    sym=m.group(1); m2=re.search(r'// symbol\s+(\S+)',m.group(4))
    if m2: sym=m2.group(1)
    return sym,int(m.group(2),16),int(m.group(3),16)
_IMGC={}
def imgdis(lo,hi):
    if (lo,hi) not in _IMGC:
        O=dis(imgbytes(lo,hi),lo,(),lo,hi)
        _IMGC[(lo,hi)]=O
    return _IMGC[(lo,hi)]
def compile_unit(unit,srcpath,pfx):
    cmd=compile_cmd(unit)
    if not cmd: return None,'unit not in compile_commands'
    tag=hashlib.md5((srcpath+str(pfx)).encode()).hexdigest()[:8]
    obj=f'{SCR}/{tag}.obj'; out=[]
    for t in cmd:
        if t.startswith('/Fo'): t='/Fo'+obj
        elif t.startswith('/Fd') or t.startswith('--repro-root') or t=='/W4': continue
        elif t==R+'/'+unit: t=srcpath
        out.append(t)
    env=dict(os.environ, VC6_PREFIX=f'{R}/.opensmacx/wine/worker-{pfx}')
    if os.path.exists(obj): os.remove(obj)
    p=subprocess.run(out,cwd=SCR,env=env,capture_output=True,text=True)
    if not os.path.exists(obj):
        return None,'\n'.join(l for l in (p.stdout+p.stderr).split('\n') if 'error' in l)[:1500]
    return Coff(open(obj,'rb').read()),None
def evaluate(c,text,addr,wins=()):
    """-> dict(sym, exact, diff, img, ours, ratio, rows, win) or dict(error=...)"""
    idn=ident(text,addr)
    if not idn: return dict(error='no ORIGINAL marker')
    sym,lo,hi=idn
    try: s=c.function(sym)
    except Exception as e: return dict(error='nosymbol '+sym,sym=sym)
    sec=c.sections[s.section-1]
    O=imgdis(lo,hi); C=dis(sec.data,0,sec.relocations,selfsec=s.section,syms=c.symbols)
    A=[x[1] for x in O]; B=[x[1] for x in C]
    ops=align(A,B); eq=sum(i2-i1 for t,i1,i2,j1,j2 in ops if t=='equal')
    nd=0; rows=[]
    for t,i1,i2,j1,j2 in ops:
        if t=='equal':
            for k in range(i2-i1): rows.append(('  ',O[i1+k],C[j1+k]))
        else:
            k_=max(i2-i1,j2-j1); nd+=k_
            for k in range(k_): rows.append(('!!',O[i1+k] if i1+k<i2 else None,C[j1+k] if j1+k<j2 else None))
    # local jump targets must land on aligned instructions
    omap={}
    for k,o,c_ in rows:
        if k=='  ': omap[o[0]]=c_[0]
    for i,(k,o,c_) in enumerate(rows):
        if k=='  ' and o[3] is not None and c_[3] is not None and omap.get(o[3],-1)!=c_[3]:
            rows[i]=('!>',o,c_); nd+=1
    exact = nd==0 and len(A)==len(B) and all(o[2]==c_[2] for o,c_ in zip(O,C))
    cnt=[0]*len(wins)
    if wins:
        nxt=[None]*len(rows); last=None
        for i in range(len(rows)-1,-1,-1):
            if rows[i][1] is not None: last=lo+rows[i][1][0]
            nxt[i]=last
        for i,r in enumerate(rows):
            if r[0]!='  ' and nxt[i] is not None:
                for wi,(w0,w1) in enumerate(wins):
                    if w0<=nxt[i]<=w1: cnt[wi]+=1
    jt=sum(1 for r_ in rows if r_[0]=='!>')
    return dict(sym=sym,lo=lo,exact=exact,jt=jt,diff=nd,img=len(A),ours=len(B),ratio=2.0*eq/max(1,len(A)+len(B)),rows=rows,win=cnt)
def show(res,all_=False,ctx=1):
    rows=res['rows']; lo=res['lo']; shown=set()
    for i,r in enumerate(rows):
        if all_ or r[0]!='  ': shown.update(range(max(0,i-ctx),min(len(rows),i+ctx+1)))
    last=-2
    for i in sorted(shown):
        if i!=last+1: print('   ...')
        k,o,c_=rows[i]
        print(f"{k} {('%x'%(lo+o[0])) if o else '':6s} {o[1] if o else '':44s} | {('%3x'%c_[0]) if c_ else '':4s} {c_[1] if c_ else ''}")
        last=i
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('unit'); ap.add_argument('addr'); ap.add_argument('--src'); ap.add_argument('--all',action='store_true')
    ap.add_argument('--ctx',type=int,default=1); ap.add_argument('--pfx',default='21'); ap.add_argument('--quiet',action='store_true'); ap.add_argument('--win',default=''); ap.add_argument('--batch',action='store_true')
    a=ap.parse_args()
    unit=a.unit if a.unit.startswith('src/') else 'src/'+a.unit
    addrs=[int(x,16) for x in a.addr.split(',')]
    wins=[tuple(int(x,16) for x in w.split('-')) for w in a.win.split(',')] if a.win else []
    if a.batch:
        for line in sys.stdin:
            path=line.strip()
            if not path: continue
            c,err=compile_unit(unit,path,a.pfx)
            if c is None: print('B FAIL',flush=True); continue
            r=evaluate(c,open(path,encoding='latin-1').read(),addrs[0],wins)
            if 'error' in r: print('B FAIL',flush=True); continue
            print('B',' '.join(str(x) for x in r['win']),r['diff'],r['exact'],flush=True)
        return
    srcpath=os.path.abspath(a.src) if a.src else R+'/'+unit
    text=open(srcpath,encoding='latin-1').read()
    c,err=compile_unit(unit,srcpath,a.pfx)
    if c is None: print('COMPILE FAIL'); print(err); raise SystemExit(2)
    for addr in addrs:
        r=evaluate(c,text,addr,wins)
        if 'error' in r: print('FE %08X %s'%(addr,r['error'])); continue
        if wins: print('WIN',' '.join(str(x) for x in r['win']))
        if not a.quiet: show(r,a.all,a.ctx)
        print('FE %08X %s exact=%s diff=%d img=%d ours=%d ratio=%.4f jt=%d'%(addr,r['sym'][:40],r['exact'],r['diff'],r['img'],r['ours'],r['ratio'],r['jt']))
if __name__=='__main__': main()
