#include "stdafx.h"
#include "graphicwin.h"

// Keep the update implementation out of the TU that emits the inline virtual
// callback: the original callback calls this method rather than expanding it.
// This current source boundary does not identify an original filename.
/*
Purpose: When visible, tell the window to update and flip a rect the size of
         the RECT at this+0x474 (the same field redraw() reads), positioned
         at the origin.
// ORIGINAL: 0x005D56B0 ?update@GraphicWin@@QAEXPAUGraphicWin@@@Z 0x005D56B0-0x005D571D BYTE_EXACT
// symbol    ?update@GraphicWin@@QAEXPAV1@@Z
// size      109 bytes
// prototype void (__thiscall ?update@GraphicWin@@QAEXPAUGraphicWin@@@Z)(GraphicWin* this, GraphicWin*)
// callers   8   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ECFE0 0x005EFD20 0x005F74A0 0x005F7E90
Return Value: n/a
Status: Complete
*/
void GraphicWin::update(GraphicWin *) {
    // Four separate field reads of the surface RECT, not a copied RECT: the
    // latter compiles a real `call memcpy` here (unlike redraw()'s own read
    // of the same field, which VC6 happens to inline), where the image loads
    // each of the four dwords directly.
    int height = rect2_.bottom - rect2_.top;
    int width = rect2_.right - rect2_.left;

    if (static_cast<Win *>(this)->is_visible()) {
        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = width;
        rect.bottom = height;
        static_cast<Win *>(this)->update_window(&rect);
        static_cast<Win *>(this)->client_to_screen(&rect);
        Win::flip(&rect);
    }
}
