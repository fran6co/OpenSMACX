// ORIGINAL: 0x00627F50
// name      sub_627f50
// size      399 bytes
// spans     0x00627F50-0x006280DF
// prototype
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: 3x3 matrix inverse (adjugate/determinant, classic cofactor
//            transpose) - the math matches Ghidra's flattened SSA form
//            exactly, but this form diverges at #4 on x87 stack scheduling:
//            the original reloads each element repeatedly rather than
//            caching it (9 values through an 8-deep FPU stack), which a
//            single-read-per-element C form does not reproduce.

extern "C" void __cdecl sub_627f50(float *m, float *out) {
    float m00 = m[0], m01 = m[1], m02 = m[2];
    float m10 = m[3], m11 = m[4], m12 = m[5];
    float m20 = m[6], m21 = m[7], m22 = m[8];

    float c00 = m11 * m22 - m12 * m21;
    float c01 = m10 * m22 - m20 * m12;
    float c02 = m10 * m21 - m20 * m11;

    float r = *reinterpret_cast<float *>(g_00670c08) /
              (c02 * m02 + (m00 * c00 - c01 * m01));

    out[0] = r * c00;
    out[1] = -(r * (m01 * m22 - m02 * m21));
    out[2] = r * (m01 * m12 - m02 * m11);
    out[3] = -(r * c01);
    out[5] = -(r * (m00 * m12 - m02 * m10));
    out[4] = r * (m00 * m22 - m02 * m20);
    out[6] = r * c02;
    out[7] = -(r * (m00 * m21 - m01 * m20));
    out[8] = r * (m00 * m11 - m01 * m10);
}
