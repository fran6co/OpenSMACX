# reuse.py TARGETS [LINEFILTER] : rename each block-local int of enemy_move into a function-scope variable
import sys,os,re,time
R_='/home/fran6co/code/OpenSMACX/.opensmacx/worktrees/par-4'
os.chdir(R_); sys.path.insert(0,R_+'/recovery/tools'); import perm
os.environ.setdefault('FE_PFX_BASE','21'); os.environ.setdefault('FE_JOBS','5')
U='src/enemy.cpp'; A='0x0056B5B0'
base=open(R_+'/'+U,encoding='latin-1').read()
L=base.split('\n')
targets=sys.argv[1].split(',')
fs=base.index('int __cdecl enemy_move(int veh_id) {'); fl=base.count('\n',0,fs)+1
fe_=base.index('\n}\n',fs); el=base.count('\n',0,fe_)+1
deck=[('BASE',base)]
for ln in range(fl+25,el):
    m=re.match(r'^( +)int ([a-z_0-9]+)( = |;)',L[ln-1])
    if not m: continue
    name=m.group(2); ind=len(m.group(1))
    # scope end: first line after ln with indentation < ind that is a closing brace
    end=ln
    while end<el and not (L[end].strip().startswith('}') and len(L[end])-len(L[end].lstrip())<ind): end+=1
    for T in targets:
        if T==name: continue
        new=L[:]
        if m.group(3)==';': new[ln-1]=''
        else: new[ln-1]=re.sub(r'^( +)int ',r'\1',L[ln-1])
        for k in range(ln-1,end):
            new[k]=re.sub(r'(?<![\w.>])%s(?!\w)'%name,T,new[k].split('//')[0])+('//'+new[k].split('//',1)[1] if '//' in new[k] else '')
        deck.append(('%s@L%d->%s'%(name,ln,T),'\n'.join(new)))
print(len(deck)-1,'variants',flush=True)
t0=time.time()
res=perm.run_deck(U,A,deck,jobs=int(os.environ['FE_JOBS']))
b=[d for w,d,ex,k in res if k=='BASE'][0]
for w,d,ex,k in sorted(res,key=lambda r:r[1]):
    if d<b or k=='BASE': print('%5d %+5d %s'%(d,d-b,k))
print('worse:',sum(1 for r in res if r[1]>b),'%.0fs'%(time.time()-t0))
