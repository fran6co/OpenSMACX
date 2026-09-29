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

#include <stddef.h>

// FileBox owns the editable paths and filter text passed to the common-dialog
// API. The last setter writes at +0x418 and accepts strings through length
// 0x104, so the old 0x41C-byte catalogue extent was only a lower bound.
class FileBox {
 public:
  FileBox();
  ~FileBox();
  void reset();

  int set_def_ext(const char *extension);
  int set_def_name(const char *name);
  int set_initial_dir(const char *directory);
  void save();
  void load();
  int add_filter(char *pattern, char *label);

  char file_[0x104];
  char def_name_[0x104];
  char field_208_[0x104];
  char filter_[0x104];
  char *filter_cursor_;
  char def_ext_[4];
  char initial_dir_[0x104];
};

static_assert(offsetof(FileBox, def_name_) == 0x104, "FileBox name offset");
static_assert(offsetof(FileBox, filter_) == 0x30C, "FileBox filter offset");
static_assert(offsetof(FileBox, filter_cursor_) == 0x410, "FileBox cursor offset");
static_assert(offsetof(FileBox, def_ext_) == 0x414, "FileBox extension offset");
static_assert(offsetof(FileBox, initial_dir_) == 0x418, "FileBox directory offset");
