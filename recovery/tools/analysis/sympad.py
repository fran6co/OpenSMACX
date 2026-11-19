"""sympad.py FEALL.json maxrows OUT.jsonl: caller-budget screen (MATCHING_GUIDE 3.6 / 2.1). For every non-exact function of a
feall.py --out report up to maxrows, fe with 0,1,2,3,4,6,8,12 block-scope `{ extern int X; (void)&X; }` statements before the
closing brace. They grow the caller's source size (the /Ob2 budget) and never move a frame-symbol tie, so a function that
moves here and under locpad.py at the same count is a budget case, one that moves only under locpad.py is a tie.
Appends one JSON line per function (resumable); prints the functions whose rows move. FE_PFX_BASE.., FE_JOBS lanes."""
import sys,os,re,json
sys.path.insert(0,os.getcwd()+'/recovery/tools'); import perm,fe
r=json.load(open(sys.argv[1])); mx=int(sys.argv[2]); out=open(sys.argv[3],'a')
NS=[0,1,2,3,4,6,8,12]
done={json.loads(l)['addr'] for l in open(sys.argv[3])}
todo=[x for x in r if not x['exact'] and 0<x["diff"]<=mx and x['addr'] not in done]
print('functions',len(todo),flush=True)
for x in sorted(todo,key=lambda x:(x['diff'],x['unit'],x['addr'])):
    u=x['unit']
    if not os.path.exists(u): continue
    s=open(u,encoding='latin-1').read()
    m=re.search(r'// ORIGINAL: 0x%s [^\n]*\n'%x['addr'],s)
    if not m: continue
    nxt=s.find('// ORIGINAL: ',m.end()); nxt=len(s) if nxt<0 else nxt
    if '// body ' in s[m.end():m.end()+400] or '// compile_context' in s[m.end():m.end()+600]: continue
    b=s.find('{',m.end()); e=s.find('\n}\n',b)
    if b<0 or e<0 or e>nxt: continue
    a=m.start()
    deck=[(n,s[:e+1]+''.join('    { extern int FeSymPad%d; (void)&FeSymPad%d; }\n'%(i,i) for i in range(n))+s[e+1:]) for n in NS]
    try: res=perm.run_deck(u,'0x'+x['addr'],deck)
    except Exception as ex: print('ERR',u,x['addr'],ex); continue
    d={k:dd for w,dd,exa,k in res}
    v=[d.get(n) for n in NS]
    out.write(json.dumps(dict(unit=u,addr=x['addr'],sym=x['sym'],base=x['diff'],v=v))+'\n'); out.flush()
    if len(set(v))>1: print(u,x['addr'],x['sym'][:40],'base',x['diff'],v,flush=True)
print('done')
