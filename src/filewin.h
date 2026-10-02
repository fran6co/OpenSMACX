/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "original_seam.h"
#include "flatbutton.h"
#include "editbox.h"
#include "listbox.h"
#include "filewin_construct.h"

 /*
  * FileWin class
  *
  * The original does not derive this from FlatButton: the constructor builds
  * one at +0x660 on an offset `this`, making it a member. That distinction
  * was missed when this header was first written, because the check looked
  * only at which constructor ran first and not at whether `this` had been
  * adjusted before it.
  *
  * The four constructed controls and final handle pin its size. Several
  * leading fields remain unknown, but directory storage and suffix filters
  * now have roles established by the directory-enumeration body.
*/
class FileWin {
  // AlphaSave embeds this owner and forwards its keyboard/scroll events to
  // these actual EditBox and ListBox subobjects.
  friend class AlphaSave;
 public:
  // `static`: the image's name ends in `QAA` - a public member declared
  // __cdecl, taking no receiver - and jackal_init_real calls it with no
  // ecx set up, so `Class::method()` is the only legal spelling.
  static int init_class();   // 00614D90
  int init(char *unused, GraphicWin *parent, FileWinConstruct *parameters);
  void on_double_clicked(int a1);
  void on_button_clicked(int button_id);
  int populate_directory(char *directory);
  void save();
  char *load();
  FileWin();
  ~FileWin();
  void UNK6();
  void UNK1();
  void UNK2(int pos);
  int set_directory(char *directory);
  int prepare_sprites();
  void UNK5();
  // 0x006146A0 ?UNK4@FileWin@@QAEXXZ - public, __thiscall, void(void). Still
  // unrecovered; declared because the recovered on_double_clicked calls it
  // with a direct `call rel32`, which is what makes that body byte-exact.
  void UNK4();
  // 0x00613900, a pending_bodies forwarder. ~FileWin calls it first.
  void close();
  int add_def_ext(char *extension);
  // 0x00614E30, recovered in filewin.cpp. __CDECL MEMBER - the image's name
  // ends in `QAA`, a member taking no receiver (jackal_close calls it with
  // no ecx set up), and this is the spelling that emits it.
  void __cdecl close_class();

 private:
  // Not a base class: the constructor builds a FlatButton at +0x660 on an
  // offset `this`. The constructor reaches 0x33C0, so the object is at least
  // that large; nothing else about it is established.
  // The FlatButton member sits at 0x660, so these five all precede it and
  // belong to FileWin itself. The spans hold their offsets in place; the test
  // reads each back where the original writes it.
  // RECOVERY_MEMBER: v1 FileWin previous_directory_ 0x0 0x00614360
  char previous_directory_[260];  // GetCurrentDirectoryA output
  // RECOVERY_MEMBER: v1 FileWin current_directory_ 0x104 0x00613D50
  char current_directory_[260];
  // UNK4 composes the directory-qualified result and basename separately;
  // load returns the first buffer, not the editable default at +0x410.
  // RECOVERY_MEMBER: v1 FileWin selected_path_ 0x208 0x006146A0
  char selected_path_[260];
  // RECOVERY_MEMBER: v1 FileWin selected_basename_ 0x30C 0x006146A0
  char selected_basename_[260];
  // RECOVERY_MEMBER: v1 FileWin file_name_ 0x410 0x00613D50
  char file_name_[260];
  // RECOVERY_MEMBER: v1 FileWin extension_count_ 0x514 0x00614360
  int32_t extension_count_;  // number of five-byte suffix entries
  // RECOVERY_MEMBER: v1 FileWin extension_filters_ 0x518 0x00614360
  char extension_filters_[5][5];  // 0x518..0x530
  char directory_[0x107];  // 0x531: bounded path storage
  uint32_t field_638_;  // 0x638
  GraphicWin *modal_window_;  // 0x63C: init parent and Dialog attachment target
  RECT list_box_bounds_;  // 0x640
  RECT edit_box_bounds_;  // 0x650
  // Four member subobjects, evidenced by the constructor's placement-new
  // sites and the destructor's teardown sequence (0x00611940-family:
  // FlatButton@0x660/0x11AC, EditBox@0x1CF8, ListBox@0x286C - each span
  // exactly matches that member type's own established sizeof, which is
  // what pins these offsets rather than the IDB alone).
  FlatButton flat_button2_;  // 0x660, sizeof 0xB4C, ends 0x11AC
  FlatButton flat_button1_;  // 0x11AC, sizeof 0xB4C, ends 0x1CF8
  EditBox edit_box_;  // 0x1CF8, sizeof 0xB74, ends 0x286C
  ListBox list_box_;  // 0x286C, sizeof 0xB54, ends 0x33C0
  // RECOVERY_MEMBER: v1 FileWin directory_list_handle_ 0x33C0 0x00614360
  HWND directory_list_handle_;

  char *run_file_dialog(bool saving);
};

static_assert(sizeof(FileWin) == 0x33C4,
              "FileWin layout must match the original executable");

// The image labels this as a FileWin __cdecl member, but the entered stack
// has six explicit arguments and no hidden receiver. It is a free callback.
void __cdecl filebox_draw(GraphicWin *window, int item_kind, int item_id,
                         int x, int y, int height);

// The modal file-dialog path publishes its active FileWin here.
extern FileWin *ActiveFileWin;


// ?UNK4@FileWin@@QAEXXZ (0x006146A0). Unrecovered; on_double_clicked calls it,
// so its definition is a seam into the original image.
