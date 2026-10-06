/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once

/*
 * Vector is three 32-bit floating-point components. Original subtraction
 * loads this+0, this+4 and this+8 with fld dword at 0x0063443F,
 * 0x00634447 and 0x00634455. The constructor initializes these three
 * components; veh_scoot constructs arrays with a 12-byte element stride.
 *
 * The catalogue's output parameters for arithmetic results are MSVC hidden
 * class-return slots, not source parameters. At 0x0055A82F veh_scoot calls
 * subtraction with a separate result object, copies the returned value, then
 * closes that temporary. At 0x0055A873 it calls scale, destroys the returned
 * temporary, and subsequently uses the modified receiver. Scale therefore
 * mutates this object AND returns its value; addition, subtraction and cross
 * product return a separate value without modifying their inputs.
 *
 * veh_scoot pairs constructor 0x006343C0 with destructor 0x0055B750 for two
 * arrays of eight elements (0x0055A197-0x0055A1D9), including exceptional
 * cleanup. The destructor tail-jumps to close at 0x006343D0.
 *
 * Historical notes in commit 0995e492 claimed a 75-byte value-returning
 * subtraction probe. No reproducing source or result was retained with that
 * claim. It is not a current measurement or a constraint on this class.
 * The extra original stack slot is not a fourth component; return-object
 * construction and temporary lifetime must be recovered as C++, not padding.
 *
 * Adjacent 36-byte matrix operations are a different ownership group, not
 * additional Vector members.
 */
class Vector {
 public:
  ~Vector();                                  // 0x0055B750
  Vector();                                   // 0x006343C0
  void close();                               // 0x006343D0

  Vector operator+(Vector &right);            // 0x006343E0
  Vector operator-(Vector &right);            // 0x00634430
  Vector &operator+=(Vector &right);          // 0x00634480
  Vector &operator-=(Vector &right);          // 0x006344B0
  Vector cross(Vector &right);                // 0x006345E0
  Vector normalize();                        // 0x00634520
  float dot(const Vector &right) const;       // 0x00634650
  bool operator==(const Vector &right) const;  // 0x006344E0

  // Scale the receiver in place and return its updated value.
  Vector scale(float scalar);                 // 0x00634670

  // RECOVERY_MEMBER: v1 Vector x_ 0x0 0x00634430
  float x_;
  // RECOVERY_MEMBER: v1 Vector y_ 0x4 0x00634430
  float y_;
  // RECOVERY_MEMBER: v1 Vector z_ 0x8 0x00634430
  float z_;
};

// Normal singleton/temporary cleanup in veh_scoot calls close directly, while
// arrays and exception cleanup take this destructor's address. Keep the small
// destructor visible here, but close's definition in vector.cpp, so callers
// cannot turn the destructor callback into component stores.
inline Vector::~Vector() {
  close();
}

inline Vector::Vector() {
  x_ = 0.0f;
  y_ = 0.0f;
  z_ = 0.0f;
}

static_assert(sizeof(Vector) == 0xC, "Vector layout must match the legacy ABI");

// Three-component voxel array addition, not a gameplay Vector lifetime.
// Inputs are reloaded between stores, preserving overlapping-array behavior.
void __cdecl vector_add(const float *left, const float *right, float *output);
void __cdecl vec3_subtract(const float *left, const float *right, float *result);
void __cdecl vec3_scale(const float *source, float scale, float *result);
float __cdecl vec3_length(const float *vector);
