#include "stdafx.h"
#include "planwin.h"

// ORIGINAL: 0x0048B3F0 ?back_redraw@PlanWin@@QAEXXZ 0x0048B3F0-0x0048B445 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The body sets MapWin +0x1DD70, fills the GraphicWin virtual base, redraws the MapWin, then copies that base's Buffer into PlanWin's own Buffer using DIB width and top-down height.", "MapWin virtually inherits GraphicWin and PlanWin declares its own Buffer at +0x21A70. Typed base conversions follow the vbtable; Buffer::dib_ supplies the observed +0x4C4/+0x4C8 dimensions.", "Pinned48B432 forms ECX=virtual GraphicWin base+0x444 (Buffer source);48B43D pushes PlanWin+0x21A70 as destination before Buffer::copy5D95B0. Its five-argument wrapper and full blitter preserve receiver-as-source and pointer-argument-as-destination."], "reason": "Recover PlanWin's backing-image refresh through its real MapWin/GraphicWin hierarchy and typed buffers."}
void PlanWin::back_redraw() {
    field_1DD70_ = 0x84010002;
    GraphicWin::fill(0);
    draw_map(0);
    Buffer::copy(&buffer_, 0, 0, dib_.bmiHeader.biWidth,
                 -dib_.bmiHeader.biHeight);
}
