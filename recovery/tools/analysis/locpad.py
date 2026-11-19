"""locpad.py FEALL.json [maxrows=8] [minrows=0]: function-local tie screen (MATCHING_GUIDE 3.6). For every non-exact function of a
feall.py --out report within the row range, fe with 0..7 dead referenced locals (`{ int q = 0; }`) before the closing brace;
prints the functions whose rows move. One exact count = the original had that many more named locals mod 8 (or 8-n fewer).
EH functions move for budget reasons instead (large, non-periodic jumps). FE_PFX_BASE.., FE_JOBS lanes. Screen only."""
import sys,os,re,json
sys.path.insert(0,os.getcwd()+'/recovery/tools'); import perm,fe
r=json.load(open(sys.argv[1])); mx=int(sys.argv[2]) if len(sys.argv)>2 else 8
todo=[x for x in r if not x['exact'] and (int(sys.argv[3]) if len(sys.argv)>3 else 0)<x["diff"]<=mx]
print('functions',len(todo),flush=True)
for x in sorted(todo,key=lambda x:(x['unit'],x['addr'])):
    u=x['unit']; s=open(u,encoding='latin-1').read()
    m=re.search(r'// ORIGINAL: 0x%s [^\n]*\n'%x['addr'],s)
    if not m: continue
    nxt=s.find('// ORIGINAL: ',m.end()); nxt=len(s) if nxt<0 else nxt
    if '// body ' in s[m.end():m.end()+400] or '// compile_context' in s[m.end():m.end()+600]: continue
    b=s.find('{',m.end()); e=s.find('\n}\n',b)
    if b<0 or e<0 or e>nxt: continue
    deck=[(n,s[:e+1]+''.join('    { int fepadq%d = 0; }\n'%i for i in range(n))+s[e+1:]) for n in range(0,8)]
    try: res=perm.run_deck(u,'0x'+x['addr'],deck)
    except Exception as ex: print('ERR',u,x['addr'],ex); continue
    d={k:dd for w,dd,exa,k in res}
    if len(set(d.values()))>1: print(u,x['addr'],x['sym'][:40],'base',x['diff'],[d.get(n) for n in range(8)],flush=True)
print('done')
