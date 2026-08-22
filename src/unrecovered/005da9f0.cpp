// ORIGINAL: 0x005DA9F0 ?tile@Buffer@@QAEHPAVSprite@@HHHHHH@Z 0x005DA9F0-0x005DAB8A
// TRIED: same shape as 0x005DA860/0x005DA6C0 - straight transcription diverges at #2 on the prologue register-save set (fewer registers stay live in this form than the original chose).
// size      410 bytes
// prototype int (__thiscall ?tile@Buffer@@QAEHPAVSprite@@HHHHHH@Z)(Buffer* this, Sprite*, int, int, int, int, int, int)
// callers   13   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005E4B4A
// indirect  0x005DAA74

typedef int (__stdcall *IntersectRectFn)(RECT *, const RECT *, const RECT *);

int Buffer::tile(Sprite * a1, int a2, int a3, int a4, int a5, int a6, int a7) {
    if (ppv_bits_ == 0 && field_58_ == 0) {
        return 7;
    }

    RECT clipRect;
    clipRect.left = a4;
    clipRect.top = a5;
    int rectRight = a6 + a4;
    clipRect.right = rectRight;
    clipRect.bottom = a5 + a7;

    RECT selfRect = rect1_;

    if ((reinterpret_cast<IntersectRectFn>(*g_00669338))(&clipRect, &clipRect, &selfRect)) {
        set_clip(&clipRect);

        int tileWidth = a1->iWidth_;
        int tileHeight = a1->iHeight_;

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
            if (tilesY * tileHeight + startY < a5 + a7) {
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
                            a1->draw(this, a1->cTransparentIndex_, x, y, 1, 1);
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
