#ifndef OPENSMACX_MATRIX3_H
#define OPENSMACX_MATRIX3_H

// A mathematical nine-float view supported by this address family. This
// source name and prefix do not establish the original class, full extent,
// allocation, lifetime, callers, or translation unit.
struct Matrix3 {
  float cells[9];

  Matrix3 *identity();
  void reset_identity();
  Matrix3 *add_into(Matrix3 *destination, const Matrix3 *other);
  void add_assign(const Matrix3 *other);
  void multiply_assign(const Matrix3 *other);
  void scale_assign(float scale);
  void subtract_assign(const Matrix3 *other);
  bool equals(const float *other);
  Matrix3 *multiply_into(Matrix3 *out, const Matrix3 *other);
  void transform(float *out, const float *in);
  Matrix3 *scale_into(Matrix3 *dest, float scale);
  Matrix3 *write_identity();
};

#endif
