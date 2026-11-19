#!/usr/bin/env python3
"""okc_layout.py [SRC.cpp]: landmark order of on_key_click's final switch region (from `and ecx,0x20000`),
ours (objdis) against the image. Landmarks: N2/N3 tree nodes, ST direction stubs, MD5 (a2==5 test),
MD1 (a2==1 test), MDR (order 0x19 store), Pyu/Pyd/Pxl/Pxr pan bodies, J (post-switch cursor_active test
reached from the pans), CK (cursor_key push sequence), CM (close_messages)."""
import sys, os, re, subprocess
R = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '/../../../../..')
def norm(l):
    l = re.sub(r'0x([0-9a-f]+)', lambda m: str(int(m.group(1), 16)), l)
    return re.sub(r'\b([0-9]+)\b', lambda m: hex(int(m.group(1))), l)
def marks(lines):
    lines = [norm(l) for l in lines]
    out = []
    for i, l in enumerate(lines):
        nxt = ' ; '.join(lines[i + 1:i + 6])
        if re.search(r'cmp eax, 0x10025', l): out.append('N2')
        elif 'add eax, 0xfffeffda' in l: out.append('N3')
        elif re.search(r'mov eax, 0x[1-7]$', l) and 'jmp' in lines[i + 1] and 'ST' not in out[-1:]: out.append('ST')
        elif re.search(r'cmp eax, 0x5$', l): out.append('MD5')
        elif re.search(r'cmp eax, 0x1$', l) and 'MD5' in out: out.append('MD1')
        elif re.search(r', 0x19$', l) and 'byte ptr' in l: out.append('MDR')
        elif 'mov eax, 0x55555556' in l:
            w = ' ; '.join(lines[i - 2:i + 14])
            if '0x1ddd8' in w: out.append('P28')
            elif '0x1ddd0' in w: out.append('P26')
            elif re.search(r'sub e\w+, edx', w): out.append('P25')
            else: out.append('P27')
        elif re.search(r'lea edi, \[esi - 0x23d94\]', l):
            out.append('MDE' if 'ebp - 0x2c' in nxt else 'J')
        elif re.search(r'ret 0x8', l): out.append('RET')
    return out
def image():
    sp = '/tmp/claude-1000/-home-fran6co-code-OpenSMACX/be235c8c-71b6-49f3-abea-875f51ac16da/scratchpad/img.dis'
    L = []
    for l in open(sp):
        m = re.match(r'\s*([0-9a-f]+):\s+(.*)', l)
        if m and 0x51b081 <= int(m.group(1), 16) < 0x51b670:
            L.append(re.sub(r'\s+', ' ', m.group(2)).split(' <')[0].replace('0x8', '0x8'))
    return L
def ours(src):
    o = subprocess.run(['uv', 'run', 'python', R + '/recovery/tools/objdis.py', 'src/console.cpp', os.path.abspath(src), '0x005178C0'],
                       cwd=R, capture_output=True, text=True, env=dict(os.environ, FE_PFX_BASE=os.environ.get('FE_PFX_BASE', '14'))).stdout
    L = [l.split(' ', 1)[1] if ' ' in l else l for l in o.split('\n')]
    k = max(i for i, l in enumerate(L) if 'and ecx, 0x20000' in l)
    return L[k:]
if __name__ == '__main__':
    print('img ', ' '.join(marks(image())))
    print('ours', ' '.join(marks(ours(sys.argv[1] if len(sys.argv) > 1 else R + '/src/console.cpp'))))
