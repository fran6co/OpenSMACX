#!/usr/bin/env python3
"""g16.py: frame order of same-size address-taken local arrays (colortables_init 0x00423570: the image's
top-down order of three int[8] arrays is independent of declaration order and names). Variants separate
declaration order, reference count, first/last address-taking use, and aggregate vs element initialisation."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import t as T
HDR = '#include "probe.h"\nvoid take(int *);\nextern int garr[];\n'
INIT = '= {1, 2, 3, 4, 5, 6, 7, 8}'


def t(v, k=1):
    return ' '.join(['take(%s);' % v] * k)


def fn(name, body):
    return 'void %s(int s) {\n%s}\n' % (name, body)


CASES = {
    # same counts, declaration order varies
    'dABC': '  int A[8]; int B[8]; int C[8]; %s %s %s\n' % (t('A'), t('B'), t('C')),
    'dCBA': '  int C[8]; int B[8]; int A[8]; %s %s %s\n' % (t('A'), t('B'), t('C')),
    # take order varies
    'dABC_tCBA': '  int A[8]; int B[8]; int C[8]; %s %s %s\n' % (t('C'), t('B'), t('A')),
    # counts vary (A1 B3 C2)
    'cnt_132': '  int A[8]; int B[8]; int C[8]; %s %s %s\n' % (t('A', 1), t('B', 3), t('C', 2)),
    'cnt_312': '  int A[8]; int B[8]; int C[8]; %s %s %s\n' % (t('A', 3), t('B', 1), t('C', 2)),
    # element references (constant subscripts) instead of address-taking calls
    'elem_A3': '  int A[8]; int B[8]; int C[8]; %s %s %s garr[0]=A[1]+A[2]+A[3];\n' % (t('A'), t('B'), t('C')),
    'elem_C3': '  int A[8]; int B[8]; int C[8]; %s %s %s garr[0]=C[1]+C[2]+C[3];\n' % (t('A'), t('B'), t('C')),
    # aggregate initialiser on one
    'init_B': '  int A[8]; int B[8] %s; int C[8]; %s %s %s\n' % (INIT, t('A'), t('B'), t('C')),
    'init_ABC': '  int A[8] %s; int B[8] %s; int C[8] %s; %s %s %s\n' % (INIT, INIT, INIT, t('A'), t('B'), t('C')),
    'init_A_C': '  int A[8] %s; int B[8]; int C[8] %s; %s %s %s\n' % (INIT, INIT, t('A'), t('B'), t('C')),
    # loop references (variable subscript), counted once each
    'loop_B': '  int A[8]; int B[8]; int C[8]; %s %s %s for (int i = 0; i < s; i++) B[i] = garr[i];\n' % (
        t('A'), t('B'), t('C')),
    'loop_B2': '  int A[8]; int B[8]; int C[8]; %s %s %s for (int i = 0; i < s; i++) B[i] = garr[i]; '
               'for (int k = 0; k < s; k++) garr[k] = B[k];\n' % (t('A'), t('B'), t('C')),
    # pre-CSE count: A read three times in one expression (one load after CSE) against C taken twice
    'cse_A3_C2': '  int A[8]; int B[8]; int C[8]; %s %s %s garr[0] = A[1] + A[1] + A[1]; %s\n' % (t('A'), t('B'), t('C'), t('C')),
    'cse_A2_C2': '  int A[8]; int B[8]; int C[8]; %s %s %s garr[0] = A[1] + A[1]; %s\n' % (t('A'), t('B'), t('C'), t('C')),
    # loop weight: one reference inside a loop against two outside
    'loopw_C': '  int A[8]; int B[8]; int C[8]; %s %s %s %s %s for (int i = 0; i < s; i++) take(&C[i]);\n' % (t('A'), t('B'), t('C'), t('A'), t('B')),
    'loopw_C2': '  int A[8]; int B[8]; int C[8]; %s %s %s %s %s for (int i = 0; i < s; i++) for (int k = 0; k < s; k++) take(&C[i+k]);\n' % (t('A'), t('B'), t('C'), t('A'), t('B')),
    # pre-CSE 4 against 3: A take + three reads of A[1] (one load after CSE), C taken three times
    'cse_A4_C3': '  int A[8]; int B[8]; int C[8]; %s %s %s garr[0] = A[1] + A[1] + A[1]; %s\n' % (t('A'), t('B'), t('C'), t('C', 2)),
    # same, A's reads all distinct elements (three loads)
    'elm_A4_C3': '  int A[8]; int B[8]; int C[8]; %s %s %s garr[0] = A[1] + A[2] + A[3]; %s\n' % (t('A'), t('B'), t('C'), t('C', 2)),
    # sizes differ: a small array with fewer references against a larger one with more
    'size_s1_L3': '  int S[4]; int L[8]; %s %s\n' % (t('S'), t('L', 3)),
    'size_L3_s1': '  int L[8]; int S[4]; %s %s\n' % (t('L', 3), t('S')),
    'size_x1_L3': '  int X; int L[8]; %s %s\n' % (t('&X'), t('L', 3)),
    'size_s1_L3_x2': '  int S[4]; int L[8]; int X; %s %s %s\n' % (t('S'), t('L', 3), t('&X', 2)),
    'size_e3_L8': '  int E[3]; int L[8]; %s %s\n' % (t('E'), t('L', 3)),
    'size_e3_s4': '  int E[3]; int S[4]; %s %s\n' % (t('E', 3), t('S')),
    'size_e3_L8_init': '  int E[3]; int L[8] = {1,2,3,4,5,6,7,8}; %s %s\n' % (t('E', 3), t('L')),
    'size_e3_s4_init': '  int E[3]; int S[4] = {1,2,3,4}; %s %s\n' % (t('E', 3), t('S')),
    'size_e12_L8': '  int E[12]; int L[8]; %s %s\n' % (t('E'), t('L', 3)),
    'size_e5_L8': '  int E[5]; int L[8]; %s %s\n' % (t('E', 3), t('L', 1)),
    # dead code reference
    'dead_C': '  int A[8]; int B[8]; int C[8]; %s %s %s if (0) { %s }\n' % (t('A'), t('B'), t('C'), t('C', 3)),
}
V = [fn(k, b) for k, b in CASES.items()]
if __name__ == '__main__':
    T.show(HDR + ''.join(V), asm_of=sys.argv[1] if len(sys.argv) > 1 else None)
