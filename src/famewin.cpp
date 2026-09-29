#include "stdafx.h"
#include "famewin.h"
#include "halloffame.h"

// hall_of_fame's receiver at 0x0074DAF8 and the original g_FAMEWIN
// initializer/finalizer at 0x004483D0/0x004483F0 identify this as a
// process-lifetime object. Its end is the adjacent HallOfFameIOMode slot.
// RECOVERY_OBJECT: v2 0x0074DAF8 0x00759194 ?g_FAMEWIN@@3VFameWin@@A 0x004483D0
FameWin g_FAMEWIN;

// C++ member lifetime follows the constructor and destructor traces: the
// GraphicWin base is first/last; the typed members follow their declaration
// order. This body is intentionally empty so the compiler owns that work.
// ORIGINAL: 0x0044B200 ??0FameWin@@QAE@XZ 0x0044B200-0x0044B2F3;0x00654E00-0x00654E8E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0044b200.cpp"], "reason": "Typed C++ member lifetime follows the original constructor call sequence; the seed remains the historical body interpretation."}
FameWin::FameWin() {}

// ORIGINAL: 0x0044B100 ??1FameWin@@QAE@XZ 0x0044B100-0x0044B1FF;0x00654D60-0x00654DF6
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0044b100.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// symbol    ??1FameWin@@UAE@XZ
FameWin::~FameWin() {}

// ORIGINAL: 0x00448D60 ?close@FameWin@@QAEXXZ 0x00448D60-0x00448D81 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/00448d60.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
void FameWin::close() {
  buffer1_.close();
  release_modal();
  GraphicWin::close();
}

// ORIGINAL: 0x0044A4B0 ?on_key_click@FameWin@@QAEHHH@Z 0x0044A4B0-0x0044A4F3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0044a4b0.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
int FameWin::on_key_click(int, int code) {
  if ((code > 0x2F && code < 0x5B) || code == 0x1B || code == 0xD) {
    buffer1_.close();
    release_modal();
    GraphicWin::close();
  }
  return 1;
}

// ORIGINAL: 0x0044A640 ?on_group_clicked@FameWin@@QAEXHHH@Z 0x0044A640-0x0044A655 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/0044a640.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
void FameWin::on_group_clicked(int, int, int) {
  selectedRecord_ = 0;
  redraw();
}

// ORIGINAL: 0x0044A570 ?on_button_clicked@FameWin@@QAEXH@Z 0x0044A570-0x0044A640
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0044a570.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// symbol    ?on_button_clicked@FameWin@@UAEXH@Z
void FameWin::on_button_clicked(int button_id) {
  if (button_id == 0x7B8) {
    buffer1_.close();
    release_modal();
    GraphicWin::close();
    return;
  }
  int award = button_id - 0x8AE;
  if (award < 0 || award > 5) return;
  for (int row = 0; row < 7; ++row) {
    for (int column = 0; column < 5; ++column) {
      const HallOfFameRecord &entry = HallOfFameRecords[row * 5 + column];
      if ((entry.present & 1) && (entry.award_flags & (1 << award))) {
        buttonGroup_.set(row + 1, 1);
        selectedRecord_ = column;
        redraw();
        return;
      }
    }
  }
  redraw();
}

// ORIGINAL: 0x00448D90 ?on_redraw@FameWin@@QAEXXZ 0x00448D90-0x0044A4AD;0x00654EA5-0x00654EBA
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00448d90.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// symbol    ?on_redraw@FameWin@@UAEXXZ
void FameWin::on_redraw() {
  // BODY GOES HERE. The preserved seed at src/unrecovered/00448d90.cpp
  // records the large painter; its behavior has not been migrated.
}
