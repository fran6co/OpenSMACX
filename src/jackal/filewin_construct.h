#pragma once

#include <stdint.h>
#include <windows.h>

// The caller constructs 18 DWORDs. FileWin::init snapshots/consumes only the
// first 17; the trailing caller word remains represented but unused here.
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
  int32_t field_44_;
};

static_assert(sizeof(FileWinConstructPrefix) == 0x44,
              "FileWin init snapshots exactly 17 dwords");
static_assert(sizeof(FileWinConstruct) == 0x48,
              "AlphaSave constructs 18 dwords for FileWin::init");
