# MATCHING_GUIDE 2.7 4j: writes pb.cpp here; run with  python3 ../run.py --oy pb.cpp  (k = 4) and read the homes of
# this / z / x (x is stored as a byte: only ebx can hold it among the callee-saved registers).  `_a1` functions
# have an `__asm { nop }` (ebp frame, k = 3).  a and b take esi/edi; `this` has T member stores, z Z reads,
# x X byte stores.  Expected (measured 2026-11-20): T10 Z0: this ebp, x bl (k = 4); this ebx, x memory (k = 3).
# T10 Z5: this ebp, z ebx, x memory (`this` leaves ebx although x then loses it to z).  T16: this esi, a edi,
# b ebp, z ebx.
out = ['#include "../probe.h"', 'extern char gch[16]; extern void use_x(int); extern void use_z(int);']
fns = []
for asm in (0, 1):
    for T in (10, 16):
        for Z in (0, 5, 12):
            name = 't%d_z%d_a%d' % (T, Z, asm)
            fns.append(name)
            b = ['void W::%s(int x, int z) {' % name]
            if asm:
                b.append('  __asm { nop }')
            b.append('  D(a); D(b);')
            for i in range(16):
                b.append('  U(a); U(b);')
                if i < T:
                    b.append('  f[%d] = def_e();' % (i % 8))
                if i < 3:
                    b.append('  gch[%d] = (char)x;' % i)
                if i < Z:
                    b.append('  use_z(z);')
            b.append('  U(a); U(b); gch[9] = (char)x; use_z(z); f[0] = 1;')
            b.append('}')
            out += b
out.insert(2, 'struct W { int f[8];' + ''.join(' void %s(int x, int z);' % n for n in fns) + ' };')
open('pb.cpp', 'w').write('\n'.join(out) + '\n')
