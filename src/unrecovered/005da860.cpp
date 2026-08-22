// ORIGINAL: 0x005DA860 ?tile@Buffer@@QAEHPAVBuffer@@HHHHHH@Z 0x005DA860-0x005DA9EC
// TRIED: straight transcription diverges at #2 (original pushes ebx/ebp/esi/edi as one prologue block; this form only needs three, so the compiler's register save set differs) - 122/144 mnemonics still agree. Landing the closest.
// size      396 bytes
// prototype int (__thiscall ?tile@Buffer@@QAEHPAVBuffer@@HHHHHH@Z)(Buffer* this, Buffer*, int, int, int, int, int, int)
// callers   12   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DFF00
// indirect  0x005DA8E4

typedef int (__stdcall *IntersectRectFn)(RECT *, const RECT *, const RECT *);

int Buffer::tile(Buffer * a1, int a2, int a3, int a4, int a5, int a6, int a7) {
    if (ppv_bits_ == 0 && field_58_ == 0) {
        return 7;
    }

    RECT clipRect;
    clipRect.left = a4;
    clipRect.top = a5;
    int rectRight = a6 + a4;
    clipRect.right = rectRight;
    clipRect.bottom = a7 + a5;

    RECT selfRect = rect1_;

    if ((reinterpret_cast<IntersectRectFn>(*g_00669338))(&clipRect, &clipRect, &selfRect)) {
        set_clip(&clipRect);

        int tileWidth = a1->width_;
        int tileHeight = -a1->height_;

        if (tileWidth != 0 && tileHeight != 0) {
            int tilesX = a6 / tileWidth;
            if (tilesX * tileWidth < a6) {
                tilesX++;
            }
            int tilesY = a7 / tileHeight;
            if (tilesY * tileHeight < a7) {
                tilesY++;
            }

            int startX = a4 - a2 % tileWidth;
            int startY = a5 - a3 % tileHeight;

            if (tilesX * tileWidth + startX < rectRight) {
                tilesX++;
            }
            if (tilesY * tileHeight + startY < a7 + a5) {
                tilesY++;
            }

            int rowsLeft = tilesY;
            if (rowsLeft > 0) {
                int y = startY;
                do {
                    int x = startX;
                    int colsLeft = tilesX;
                    if (colsLeft > 0) {
                        do {
                            a1->copy(this, 0, 0, x, y, tileWidth, tileHeight);
                            colsLeft--;
                            x += tileWidth;
                        } while (colsLeft != 0);
                    }
                    y += tileHeight;
                    rowsLeft--;
                } while (rowsLeft != 0);
            }

            set_clip(&selfRect);
        }
    }

    return 0;
}
