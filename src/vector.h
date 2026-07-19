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

class DLLEXPORT Vector {
 public:
  Vector();

  void close();
  void __mi(Vector &output, Vector &right);
  void __apl(Vector &right);
  void __ami(Vector &right);
  void scale(Vector &output, float scalar);

 private:
  uint32_t values_[3];
};

static_assert(sizeof(Vector) == 0xC, "Vector layout must match the legacy ABI");

// Component-wise addition of two vectors into a third, component order
// preserved because the output may overlap either input.
void __cdecl vector_add(Vector *left, Vector *right, Vector *output);

Vector *__fastcall vector_construct_redirect(Vector *self, void *);
uintptr_t __fastcall vector_close_redirect(Vector *self, void *);
Vector *__fastcall vector_subtract_redirect(
    Vector *self, void *, Vector *output, Vector *right);
Vector *__fastcall vector_add_assign_redirect(Vector *self, void *, Vector *right);
Vector *__fastcall vector_subtract_assign_redirect(Vector *self, void *, Vector *right);
Vector *__fastcall vector_scale_redirect(
    Vector *self, void *, Vector *output, uint32_t scalar_bits);
