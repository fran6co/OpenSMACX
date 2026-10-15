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

class Sound;

// Process defaults occupy 37 original global slots, in the order loaded by
// AutoSound's constructor. Pointer types state only the destination interface;
// they do not establish provider ownership or lifetime.
struct AutoSoundDefaultsBlock {
  int val_1_;
  Sound *val_3_;
  Sound *val_4_;
  int val_2_;
  int val_5_;
  Sound *val_6_;
  Sound *val_7_;
  Sound *val_8_;
  Sound *val_9_;
  Sound *val_10_;
  Sound *val_11_;
  Sound *val_12_;
  Sound *val_13_;
  Sound *val_14_;
  Sound *val_15_;
  Sound *val_16_;
  Sound *val_17_;
  Sound *val_18_;
  Sound *val_19_;
  Sound *val_20_;
  Sound *val_21_;
  Sound *val_22_;
  Sound *val_23_;
  Sound *val_24_;
  Sound *val_25_;
  Sound *val_26_;
  Sound *val_27_;
  Sound *val_28_;
  Sound *val_29_;
  Sound *val_30_;
  Sound *val_31_;
  int val_32_;
  Sound *val_33_;
  Sound *event_window_;
  Sound *val_35_;
  Sound *val_36_;
  Sound *val_37_;
};

extern AutoSoundDefaultsBlock AutoSoundDefaults;
static_assert(sizeof(void *) != 4 || sizeof(AutoSoundDefaultsBlock) == 0x94,
              "AutoSound defaults must occupy the original 37-word block");
