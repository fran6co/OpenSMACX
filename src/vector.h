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
 * Vector - the game's three-component float vector.
 *
 * THE STORAGE IS `float`, AND THAT IS MEASURED, NOT INFERRED. This header
 * declared `uint32_t values_[3]` until 2026-08-12, which forced every body
 * below to launder its own members through `memcpy` before it could do
 * arithmetic on them. `tools/verify_member_offsets.py --class Vector`
 * withdrew that spelling: the class's own thiscall methods read all three
 * offsets with x87 instructions, and an x87 instruction names the type.
 *
 *   0x0063443F  fld dword ptr [ecx]        Vector::__mi, member at 0x0
 *   0x00634447  fld dword ptr [ecx + 4]                   member at 0x4
 *   0x00634455  fld dword ptr [ecx + 8]                   member at 0x8
 *
 * `fld dword ptr` is a 4-byte float load; `fld qword ptr` would be a double.
 * Two independent readings agree. The two Vector methods already recovered
 * BYTE_EXACT elsewhere - 0x006344E0 (operator==) and 0x00634650 (dot) in
 * src/leaf_recoveries.cpp - reach the same three offsets through
 * `const float *`, and both reproduce the shipped bytes exactly that way.
 *
 * SIZEOF IS 0xC AND THERE IS NO FOURTH COMPONENT. The constructor at
 * 0x006343C0 zeroes exactly three dwords; no catalogued Vector method
 * touches `this + 0xC` or past it. The thing that looks like a fourth
 * component is a stack slot, not a member - see "the frame slot" below.
 *
 * THE CATALOGUE'S PROTOTYPES FOR THIS CLASS ARE WRONG, and the bytes say so.
 * The image carries no symbols, so every mangled name in functions.csv is
 * IDA's reconstruction; for Vector it is demonstrably a reconstruction and
 * not a linker record, because it spells the argument types out in full
 * (`AAVVector@@AAVVector@@`) where VC6 back-references the second one to `0`.
 * Recompiling with the original compiler settles each signature:
 *
 *   0x00634480  ?__apl  catalogued `void __apl(Vector &)`. The body opens
 *               `mov eax, ecx` and leaves `this` in EAX at `ret 4`. Written
 *               as `Vector &__apl(Vector &)` returning `*this`, VC6 12.00.8168
 *               reproduces all 33 bytes; written `void`, it emits a different
 *               and shorter sequence. It is `operator+=`. ADOPTED below.
 *   0x006344B0  ?__ami  the same, with fsub. `operator-=`. ADOPTED below.
 *   0x00634430  ?__mi   catalogued `void __mi(Vector &output, Vector &right)`.
 *               There is no `output` parameter: that stack slot is MSVC's
 *               hidden return-object pointer, which is why the original
 *               leaves it in EAX. The true signature is `Vector __mi(Vector &)`
 *               - `operator-` returning by value - and with a destructor
 *               declared, VC6 reproduces all 75 bytes including the frame
 *               slot. NOT adopted: see "why two are adopted and two are not".
 *   0x00634670  ?scale  same shape: `Vector scale(float)`, which multiplies
 *               `this` in place and returns a copy. All 65 bytes reproduce.
 *               NOT adopted, same reason.
 *
 * THE FRAME SLOT. Every Vector method that returns a Vector by value opens by
 * storing zero to one dead 4-byte stack slot below its temporary
 * (`mov dword ptr [esp], 0` at 0x00634437, at 0x006343E7, at 0x0063467E, and
 * the same slot under the Matrix temporaries at 0x00634529 and 0x0063472B).
 * It is not a fourth member and not padding: it is the flag MSVC emits under
 * /GX to record whether the return object has been constructed, and it
 * appears if and only if the returned class has a destructor. Vector has one,
 * at 0x0055B750, which is a `jmp` to close. Removing the destructor from the
 * probe removes the store and shortens the body by exactly those 8 bytes.
 *
 * WHY TWO ARE ADOPTED AND TWO ARE NOT. Adopting a signature means correcting
 * the catalogued name in tools/catalogue_corrections.py, because that name is
 * what tools/emit_translation_unit.py declares the method as when it builds a
 * verification unit. `emit.decode_signature` understands a reference return
 * (`AAVVector@@`) and does not understand a by-value class return (`?AV...`),
 * so __apl and __ami can be corrected and measured today and __mi and scale
 * cannot. Neither could reach BYTE_EXACT anyway: the emitted shell declares a
 * class's data members and the subject method only, never a destructor, so
 * the return-object flag above can never appear in a rebuilt body. Teaching
 * the emitter to declare a destructor when the class has one is what would
 * close 0x00634430, 0x006343E0, 0x00634520, 0x006345E0 and 0x00634670 - and
 * it changes codegen for every class in the tree, so it is a measured
 * tooling change, not a header edit.
 *
 * THE REST OF THE CLASS, catalogued and read but not declared here. Each was
 * disassembled for this note; the arithmetic is what names them.
 *
 *   0x006343E0  Vector __pl(Vector &)      operator+; mirrors __mi
 *   0x006344E0  bool operator==(Vector &)  recovered: src/leaf_recoveries.cpp
 *   0x00634520  Vector normalize()         len = sqrt(x*x + y*y + z*z), then
 *                                          `return *this` when len == 1.0f
 *                                          (the float 1.0f at 0x0066AA34),
 *                                          else scale by 1.0f / len. Does not
 *                                          modify `this`.
 *   0x006345E0  Vector cross(Vector &)     the standard cross product,
 *                                          this x right, component order
 *                                          x = y*rz - ry*z and so on
 *   0x00634650  float dot(Vector &)        recovered: src/leaf_recoveries.cpp
 *   0x0055B750  ~Vector()                  `jmp close`
 *
 * NEXT DOOR, and not a Vector. 0x006346C0 / 0x006346F0 / 0x00634720 are a
 * 3x3 float matrix: two 36-byte identity initialisers (1.0f on the diagonal,
 * offsets 0x0 to 0x20) and a component-wise add over 3x3 with the same
 * return-object flag slot. It has no class in src/ yet.
 */
class Vector {
 public:
  Vector();                                   // 0x006343C0
  void close();                               // 0x006343D0

  // Catalogued `void __mi(Vector &, Vector &)`; really `Vector operator-`
  // returning by value, with `output` the hidden return slot. See above.
  void __mi(Vector &output, Vector &right);   // 0x00634430

  Vector &__apl(Vector &right);               // 0x00634480  operator+=
  Vector &__ami(Vector &right);               // 0x006344B0  operator-=

  // Catalogued `void scale(Vector &, float)`; really `Vector scale(float)`,
  // which scales `this` in place and returns a copy through `output`.
  void scale(Vector &output, float scalar);   // 0x00634670

  float x_;  // 0x0
  float y_;  // 0x4
  float z_;  // 0x8
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
