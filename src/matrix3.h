#ifndef OPENSMACX_MATRIX3_H
#define OPENSMACX_MATRIX3_H

// A mathematical nine-float view supported by this address family. This
// source name and prefix do not establish the original class, full extent,
// allocation, lifetime, callers, or translation unit.
struct Matrix3 {
  float cells[9];

  Matrix3 *identity();
  void reset_identity();
  Matrix3 add(const Matrix3 *other);
  Matrix3 &add_assign(const Matrix3 *other);
  Matrix3 &multiply_assign(const Matrix3 *other);
  Matrix3 &scale_assign(float scale);
  Matrix3 &subtract_assign(const Matrix3 *other);
  bool equals(const float *other);
  Matrix3 multiply(const Matrix3 *other);
  void transform(float *out, const float *in);
  Matrix3 scaled(float scale);
  Matrix3 *write_identity();
};

#endif
