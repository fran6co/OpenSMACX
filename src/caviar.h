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

class DLLEXPORT CaviarData {
 public:
  CaviarData();

 private:
  uint32_t fields_[3];
};

class DLLEXPORT Caviar {
 public:
  Caviar();
  void set_scaling(float scaling);
  void set_scaling_bits(uint32_t scaling_bits);

 private:
  float scene_scale_;
  uint8_t data_[0x13CC];
};

static_assert(sizeof(CaviarData) == 0xC,
              "CaviarData layout must match the original executable");
static_assert(sizeof(Caviar) == 0x13D0,
              "Caviar layout must match the original executable");

CaviarData *__fastcall caviar_data_construct_redirect(CaviarData *self, void *);
Caviar *__fastcall caviar_construct_redirect(Caviar *self, void *);
uint32_t __fastcall caviar_set_scaling_redirect(
    Caviar *self, void *, uint32_t scaling_bits);
