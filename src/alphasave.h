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

#include "filewin.h"
#include "graphicwin.h"
#include "scroll.h"
#include "sprite.h"

// The constructor at 0x00407350 builds a GraphicWin base followed by these
// owned objects in declaration order. The destructor at 0x00408170 tears
// them down in the reverse order; the established component sizes close the
// complete AlphaSave extent without opaque byte slabs.
class AlphaSave : public GraphicWin {
 public:
  AlphaSave();
  virtual ~AlphaSave();

  void close();
  void on_double_clicked(int button_id);
  int on_listbox_scroll_delete(Scroll *scroll);

 private:
  FileWin file_win_;             // 0x0A14
  Scroll scroll_;                // 0x3DD8
  Sprite sprite1_;               // 0x5F24
  Sprite sprites_[3];             // 0x5F50
  Sprite sprite5_;               // 0x5FD4
  Sprite sprite6_;               // 0x6000
  Sprite sprite7_;               // 0x602C
};

// The file-box draw/show/hide/save/load paths share this process-wide active
// owner pointer. Its image slot is 0x006A7524; it is not a FileWin member.
extern AlphaSave *ActiveAlphaSave;

static_assert(sizeof(AlphaSave) == 0x6058,
              "AlphaSave ownership layout must match the image");
