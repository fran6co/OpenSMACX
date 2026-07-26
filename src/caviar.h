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
  uint32_t field_4_;
  void *record_;   // 0x8, the renderer record close releases
};

class DLLEXPORT Caviar {
 public:
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
 private:
  float scene_scale_;
  // Fields are carved out of the opaque span rather than appended, keeping
  // the total at the pinned 0x13D0 - the static_assert below is what verifies
  // the carving did not shift anything.
  uint8_t unmapped_04_[0x2C - 0x04];
  int32_t field_2C_;
  int32_t field_30_;
  int32_t field_34_;
  uint8_t unmapped_38_[0x108 - 0x38];
  int32_t field_108_;
  uint8_t unmapped_10C_[0x13D0 - 0x10C];
};

static_assert(sizeof(CaviarData) == 0xC,
              "CaviarData layout must match the original executable");
static_assert(sizeof(Caviar) == 0x13D0,
              "Caviar layout must match the original executable");

CaviarData *__fastcall caviar_data_construct_redirect(CaviarData *self, void *);
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
