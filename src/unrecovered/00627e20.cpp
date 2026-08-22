// ORIGINAL: 0x00627E20 sub_627e20 0x00627E20-0x00627F4F FILE
// TRIED: this is a plain 3x3 row-major matrix multiply, out = a * b (verified term-by-term against the FPU trace). Every term ordering tried (Ghidra's, the natural k=0,1,2 loop order, and several explicit parenthesisations) makes VC6 fold the three-product sum to a flat fld/fmul/faddp x2/fstp sequence with no `fxch`, while the original holds all three products on the FPU stack at once (needs 2 `fxch` per component). Whatever source shape produces that (a real 3-deep dot-product helper?) was not found; landed at MISMATCH #4 with correct values.
// size      303 bytes
// prototype
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

extern "C" void __cdecl sub_627e20(float *a, float *b, float *out) {
    out[0] = a[1]*b[3] + a[2]*b[6] + b[0]*a[0];
    out[1] = a[0]*b[1] + b[4]*a[1] + b[7]*a[2];
    out[2] = a[0]*b[2] + a[2]*b[8] + a[1]*b[5];
    out[3] = b[0]*a[3] + b[6]*a[5] + b[3]*a[4];
    out[4] = b[1]*a[3] + b[7]*a[5] + b[4]*a[4];
    out[5] = a[5]*b[8] + a[4]*b[5] + a[3]*b[2];
    out[6] = b[0]*a[6] + a[7]*b[3] + a[8]*b[6];
    out[7] = a[8]*b[7] + a[7]*b[4] + a[6]*b[1];
    out[8] = a[6]*b[2] + a[7]*b[5] + a[8]*b[8];
}
