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

class DLLEXPORT CaviarData {
 public:
  CaviarData();
  void close();

 private:
  uint32_t field_0_;
  uint32_t fileDescriptor_;
  void *record_;   // 0x8, the renderer record close releases
};

class DLLEXPORT Caviar {
 public:
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
  uint8_t field_A5_[0x63];  // 0xA5
  int32_t field_108_;
  uint8_t field_10C_[0x640];  // 0x10C
  uint32_t field_74C_;  // 0x74C
  uint32_t field_750_;  // 0x750
  uint32_t field_754_;  // 0x754
  uint8_t field_758_[0xC74];  // 0x758
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
typedef void(__cdecl func_caviar_free_record)(void *record);
extern func_caviar_free_record *CaviarDataFreeRecord;
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
typedef void (__cdecl func_apply_rotation)(float *, void *);
extern func_apply_rotation *CaviarOriginalApplyRotation;

void __fastcall caviar_set_scene_rotation_redirect(Caviar *self, void *,
                                                   float x, float y, float z);
