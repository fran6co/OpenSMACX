import sys,os,re,subprocess
sys.path.insert(0,'recovery/tools'); import fe
unit,src,func=sys.argv[1],sys.argv[2],sys.argv[3]
cmd=fe.compile_cmd(unit); out=[]
asm=fe.SCR+'/vs_%d.asm'%os.getpid(); obj=fe.SCR+'/vs_%d.obj'%os.getpid()
for t in cmd:
    if t.startswith('/Fo'): t='/Fo'+obj
    elif t.startswith('/Fd') or t.startswith('--repro-root') or t=='/W4': continue
    elif t==fe.R+'/'+unit: t=os.path.abspath(src)
    out.append(t)
out.insert(-1,'/Fa'+asm)
env=dict(os.environ, VC6_PREFIX=f'{fe.MAIN}/.opensmacx/wine/worker-{sys.argv[4] if len(sys.argv)>4 else 2}')
p=subprocess.run(out,cwd=fe.SCR,env=env,capture_output=True,text=True)
t=open(asm,errors='replace').read()
i=t.find(func+' PROC')
# var defs precede PROC
j=t.rfind('_TEXT\tSEGMENT',0,i)
for l in t[j:i].split('\n'):
    m=re.match(r'_(\w+)\$(\d*) = (-?\d+)',l)
    if m: print('%s%s %s0x%x'%(m.group(1),('$'+m.group(2)) if m.group(2) else '', '-' if int(m.group(3))<0 else '+',abs(int(m.group(3)))))
os.remove(asm); os.path.exists(obj) and os.remove(obj)
