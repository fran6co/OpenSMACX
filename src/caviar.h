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

struct VOX_Vect {
  float values[3];
};

struct VOX_Matrix {
  float values[3][3];
};

class CaviarData {
 public:
  CaviarData();
  void close();

 private:
  // ALL THREE ARE OWNED ALLOCATIONS. `Caviar::close` frees and nulls every one
  // of them for all 200 slots - `[esi-4]`, `[esi]`, `[esi+4]` around 0x750 -
  // so modelling the first two as `uint32_t` said less than the image does.
  // `CaviarData::close` still only releases `record_`, through the helper;
  // the two teardowns are genuinely different and both are in the image.
  void *field_0_;
  void *fileDescriptor_;
  void *record_;   // 0x8, the renderer record close releases

  // The renderer owns these slots and tears them down in bulk.
  friend class Caviar;
};

class Caviar {
 public:
  void close();   // 0x00617020
  void UNK11(int *out1, int *out2, int *out3);
  Caviar();
  void set_camera_direct(const VOX_Vect *camera, const VOX_Matrix *matrix);
  void set_scaling(float scaling);
  void set_scaling_bits(uint32_t scaling_bits);
  float get_scaling();

  int UNK3(int a1, int a2, int a3);
  int UNK4(int a1, int a2, int a3, int a4);
  void UNK5(int a1, int a2);
  void UNK6(int a1, int a2);
  void UNK7(int a1, int a2);
  void UNK9(int a1, int a2, int a3);
  void UNK12(int a1, int a2, int a3);
  void UNK8(int a1);
  void UNK10(int a1, int a2, int a3);
  void set_scene_rotation(float x, float y, float z);

  // `static`, because the image's own names for these end in `QAA` - a public
  // member declared `__cdecl`, which takes no receiver in ecx and puts every
  // argument on the stack. WinMain's `call 0x6185a0` sets up no ecx at all, so
  // the only spelling a call site can use is `Caviar::init_class()`, and that
  // is `C2352: illegal call of non-static member function` unless it is
  // declared this way. Same rule tools/emit_translation_unit.py applies to
  // every other `*::init_class`, and the same one palette.h already follows
  // for `set_active_window`.
  static int init_class();
  static void close_class();

 private:
  float scene_scale_;
  // Fields are carved out of the opaque span rather than appended, keeping
  // the total at the pinned 0x13D0 - the static_assert below is what verifies
  // the carving did not shift anything.
  uint32_t field_4_;  // 0x4
  uint8_t field_8_[0xC];  // 0x8
  uint32_t field_14_;  // 0x14
  uint8_t field_18_[0x4];  // 0x18
  uint32_t field_1C_;  // 0x1C
  uint32_t field_20_;  // 0x20
  uint32_t field_24_;  // 0x24
  uint32_t field_28_;  // 0x28
  int32_t field_2C_;
  int32_t field_30_;
  int32_t field_34_;
  uint32_t field_38_;  // 0x38
  uint32_t field_3C_;  // 0x3C
  uint32_t field_40_;  // 0x40
  uint32_t field_44_;  // 0x44
  uint32_t field_48_;  // 0x48
  uint32_t field_4C_;  // 0x4C
  uint32_t field_50_;  // 0x50
  uint32_t field_54_;  // 0x54
  uint32_t field_58_;  // 0x58
  uint32_t field_5C_;  // 0x5C
  uint32_t field_60_;  // 0x60
  uint32_t field_64_;  // 0x64
  uint32_t field_68_;  // 0x68
  float field_6C_;  // 0x6C
  float field_70_;  // 0x70
  uint32_t field_74_;  // 0x74
  uint32_t field_78_;  // 0x78
  uint32_t field_7C_;  // 0x7C
  uint32_t field_80_;  // 0x80
  uint32_t field_84_;  // 0x84
  uint32_t field_88_;  // 0x88
  uint32_t field_8C_;  // 0x8C
  uint32_t field_90_;  // 0x90
  uint32_t field_94_;  // 0x94
  uint32_t field_98_;  // 0x98
  uint32_t field_9C_;  // 0x9C
  uint32_t field_A0_;  // 0xA0
  uint8_t field_A4_;  // 0xA4
  // RULED-OUT: naming what is in here. The arithmetic is known -
  // `set_camera_direct` memcpys a `VOX_Vect` to 0xA5 and a `VOX_Matrix` to
  // 0xB1, and 0xA5 + 12 is 0xB1 while 0xB1 + 36 is 0xD5, exactly where
  // `get_scaling` reads its float - so the temptation is to declare
  // `VOX_Vect camera_; VOX_Matrix camera_matrix_; float scaling_;` and let
  // `get_scaling` return the member, which is what the image does with one
  // `fld dword ptr [ecx + 0xd5]` against our five-instruction stack dance.
  //
  // IT DOES NOT COMPILE. 0xA5 is not 4-aligned and those types are, so VC6
  // pads and `sizeof(Caviar)` stops being 0x13D0 - caught by the static_assert
  // below, which is what that assert is for. The `uint8_t` blob is the tree's
  // way of holding unaligned data, and `memcpy` is the only portable way in
  // and out of it. Closing this would need `#pragma pack`, which changes the
  // whole class.
  uint8_t field_A5_[0x63];  // 0xA5
  int32_t field_108_;
  uint8_t field_10C_[0x640];  // 0x10C
  // THE OBJECT-DATA ARRAY, and the constructor already said so: it does
  // `memset(bytes + 0x74C, 0, sizeof(CaviarData) * 200)`. Declaring it means
  // neither that nor `Caviar::close` has to reach it through a cast.
  // The arithmetic reconciles exactly: 200 * 0xC is 0x960, from 0x74C to
  // 0x10AC, and 0x960 + 0x320 is 0xC80, which is what the four fields this
  // replaces occupied (4 + 4 + 4 + 0xC74).
  CaviarData slots_[200];       // 0x74C
  uint8_t field_10AC_[0x320];   // 0x10AC
  uint32_t field_13CC_;  // 0x13CC
};

static_assert(sizeof(CaviarData) == 0xC,
              "CaviarData layout must match the original executable");
static_assert(sizeof(Caviar) == 0x13D0,
              "Caviar layout must match the original executable");

CaviarData *__fastcall caviar_data_construct_redirect(CaviarData *self, void *);
void __fastcall caviar_unk11_redirect(Caviar *self, void *, int *out1, int *out2, int *out3);
void __fastcall caviar_data_close_redirect(CaviarData *self, void *);

// The 413-byte helper that walks the record and frees its members is not yet
// source-owned, so close reaches it through a rebindable dependency.
// 0x00638430, a pending_bodies forwarder. A POINTER here cost CaviarData::close
// a `call dword ptr [...]` where the image has `call rel32`.
void __cdecl caviar_free_record(void *record);
Caviar *__fastcall caviar_construct_redirect(Caviar *self, void *);
void __fastcall caviar_set_camera_direct_redirect(
    Caviar *self, void *, const VOX_Vect *camera, const VOX_Matrix *matrix);
uint32_t __fastcall caviar_set_scaling_redirect(
    Caviar *self, void *, uint32_t scaling_bits);
float __fastcall caviar_get_scaling_redirect(Caviar *self, void *);
int __fastcall caviar_unk3_redirect(Caviar *self, void *, int a1, int a2, int a3);
int __fastcall caviar_unk4_redirect(Caviar *self, void *, int a1, int a2, int a3, int a4);
void __fastcall caviar_unk5_redirect(Caviar *self, void *, int a1, int a2);
void __fastcall caviar_unk6_redirect(Caviar *self, void *, int a1, int a2);
void __fastcall caviar_unk7_redirect(Caviar *self, void *, int a1, int a2);
void __fastcall caviar_unk9_redirect(Caviar *self, void *, int a1, int a2, int a3);
void __fastcall caviar_unk12_redirect(Caviar *self, void *, int a1, int a2, int a3);
void __fastcall caviar_unk8_redirect(Caviar *self, void *, int a1);
void __fastcall caviar_unk10_redirect(Caviar *self, void *, int a1, int a2, int a3);

// The rotation is applied by a helper that is not recovered yet.
// 0x00627D00, a pending_bodies forwarder, for the same reason as
// `caviar_free_record` above.
void __cdecl caviar_apply_rotation(float *angles, void *matrix);

void __fastcall caviar_set_scene_rotation_redirect(Caviar *self, void *,
                                                   float x, float y, float z);
