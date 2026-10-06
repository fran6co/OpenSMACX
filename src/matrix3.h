#pragma once

// A chosen source representation for the nine observed row-major float
// elements. This does not identify the original class, complete object size,
// translation unit, callers, or construction/destruction behavior.
struct FloatMatrix3 {
  FloatMatrix3 *sub_6346c0();
  void sub_6346f0();
  void sub_634720(FloatMatrix3 *destination, const FloatMatrix3 *other);
  void sub_6347c0(const FloatMatrix3 *other);
  void sub_6347f0(const FloatMatrix3 *other);
  void sub_6348c0(float scale);
  void sub_6348f0(const FloatMatrix3 *other);
  FloatMatrix3 *sub_634b70();

  float values[3][3];
};
