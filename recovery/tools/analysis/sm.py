#!/usr/bin/env python3
"""sm.py <unit> <addr> <src> [lo hi]: fe diff with frame-slot numbers masked; prints masked-differing count and (optionally) rows in the image window."""
import sys,subprocess,re,os
unit,addr,src=sys.argv[1:4]
out=subprocess.run(['uv','run','python','recovery/tools/fe.py',unit,addr,'--pfx',os.environ.get('PFX','8'),'--src',src,'--all'],capture_output=True,text=True).stdout
lo=int(sys.argv[4],16) if len(sys.argv)>4 else None; hi=int(sys.argv[5],16) if len(sys.argv)>5 else None
def mask(s): return re.sub(r'ebp - (0x[0-9a-f]+|\d+)\]','ebp - S]',s)
n=0;raw=0;last=0;rows=[]
for l in out.splitlines():
    m=re.match(r'(!!|!>|  ) ([0-9a-f]{6}| {6}) (.*?)\s*\| *([0-9a-f]+)? *(.*)$',l)
    if not m:
        if l.startswith('FE') or 'error' in l: print(l[:200])
        continue
    if m.group(2).strip(): last=int(m.group(2),16)
    tag=m.group(1)
    if tag=='!!':
        raw+=1
        a=mask(m.group(3)); b=mask(m.group(5) or '')
        if m.group(2).strip() and m.group(4) and a.strip()==b.strip(): tag='~~'
        else: n+=1
    if lo is not None and lo<=last<(hi or 1<<32): rows.append(tag+l[2:150])
print('masked-diff',n,'raw',raw)
print('\n'.join(rows))
