#pragma once

#include <stdint.h>
#include <windows.h>

// 17 DWORDs. AlphaSave::init's frame (0x00407410) holds the caller's copy
// at [ebp-0x6c, ebp-0x28): the word at [ebp-0x28] that an earlier reading
// took for an 18th field is a separate temporary (it first holds the
// GraphicWin::init parent pointer and later the edit box's left edge).
struct FileWinConstructPrefix {
  int32_t field_00_;
  RECT list_box_bounds_;
  RECT edit_box_bounds_;
  RECT button2_bounds_;
  RECT button1_bounds_;
};

struct FileWinConstruct {
  int32_t field_00_;
  RECT list_box_bounds_;
  RECT edit_box_bounds_;
  RECT button2_bounds_;
  RECT button1_bounds_;
};

static_assert(sizeof(FileWinConstructPrefix) == 0x44,
              "FileWin init snapshots exactly 17 dwords");
static_assert(sizeof(FileWinConstruct) == 0x44,
              "AlphaSave constructs the same 17 dwords for FileWin::init");
