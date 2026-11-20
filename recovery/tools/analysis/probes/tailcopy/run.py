#!/usr/bin/env python3
"""run.py TAILS.txt: when does VC6 copy an inline's join tail into the arm that jumps to it? (MATCHING_GUIDE 3.7,
communicate L8497 entry, par-5 round 41). Each line of TAILS.txt is a tail written after an expanded two-arm inline
(`nto`, net_treaty_off's shape) and before `goto cont;` in a loop; prints arm A (after message_data) and arm B (after
treaty_off) up to the first jmp. A copied tail shows in both arms; a joined one as `jmp $L` in A. NTO=... replaces the
inline body. Finding: copied whole when its code before the final jmp is <= 15 bytes. FE_PFX_BASE selects the prefix."""
import sys, os, re
H = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(os.path.dirname(H)) + '/inline')
import ib
PFX=os.environ.get('FE_PFX_BASE','21')
HDR='''extern int IsNet; extern int H, K, M, N; extern int ga[]; struct S2 { int x, y; }; extern S2 s2a, s2b;
void log_say(const char *, int, int, int); void message_data(int, int, int, int, int, int);
void treaty_off(int, int, int); void cap(int, int); void cap1(int); void cap0(); int pops(int); void cap3(int,int,int);
int capr(int, int); void __stdcall caps(int, int); void __fastcall capf(int, int);
struct ND { void await(int); }; extern ND nd;
extern const char LBL[];
inline void nto(int a, int b, int t, int w) { NTO }
'''
def body(tail, post=''):
    return ''' for (int i = 0; i < c; i++) {
  if (!pops(i)) { nto(a, b, 0x1000000, 0); %s goto cont; }
  ga[i] = 5; cap1(i);
  cont: %s ga[i+1] = 7;
 }
''' % (tail, post)
NTO=os.environ.get('NTO','if (IsNet != 0) { log_say(LBL, a, b, t); message_data(0x2442, 0, a, b, t, 0); if (w != 0) nd.await(0x442); } else { treaty_off(a, b, t); }')
HDR=HDR.replace('NTO',NTO)
T=[l.rstrip('\n') for l in open(sys.argv[1])]
V={'t%d'%i:body(t) for i,t in enumerate(T)}
src=HDR+''.join('void %s(int a, int b, int c) {\n%s\n}\n'%(n,V[n]) for n in V)
asm,out=ib.compile_asm(None,PFX,text=src)
F={f[1]:f[2] for f in ib.functions(asm)}
for i,t in enumerate(T):
    n='t%d'%i
    L=next((l for k,l in F.items() if ('?%s@@'%n) in k),None)
    if L is None: print(n,'MISSING',out[-600:] if out else ''); continue
    lines=[x.strip() for x in (L if isinstance(L,list) else str(L).split('\n')) if x.strip()]
    # region after message_data call until first jmp to cont
    try:
        s=next(j for j,x in enumerate(lines) if 'message_data' in x)
    except StopIteration: print(n,'?'); continue
    seg=[]
    for x in lines[s+1:]:
        seg.append(x.replace('\t',' '))
        if x.startswith('jmp'): break
    seg2=[]
    k=next((j for j,x in enumerate(lines) if 'treaty_off' in x and 'call' in x),None)
    for x in lines[k+1:]:
        seg2.append(x.replace('\t',' '))
        if x.startswith('jmp'): break
    print('%-40s | A: %s\n%-40s | B: %s'%(t,' ; '.join(seg),'',' ; '.join(seg2)))
