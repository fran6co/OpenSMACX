// ORIGINAL: 0x0042A150 ?on_dialog_item_back_draw@Datalink@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x0042A150-0x0042A260
// TRIED: computing width/height/x/y once and branching only on the `tile()` colour argument folds the compiler's output to 187 bytes (vs 272) - VC6 does not hoist a same-polarity guard's body out of line (see 0x0042A100's sibling note), so each of the three `a3` cases needs its own fully-duplicated width/height/x/y/call block. That form (cand2, below) reaches 0.834 mnemonic similarity but still differs on the exact ebx/esi/edi register allocation the original chose; landing the closest.
// size      272 bytes
// prototype int (__thiscall ?on_dialog_item_back_draw@Datalink@@QAEHPAUGraphicWin@@HHPAURECT@@@Z)(Datalink* this, GraphicWin*, int, int, RECT*)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005DA860 0x005DACB0

static int *const g_0078d548 = (int *)0x0078D548;
static int *const g_0078d980 = (int *)0x0078D980;
static int *const g_0078d9a8 = (int *)0x0078D9A8;

int Datalink::on_dialog_item_back_draw(GraphicWin * a1, int a2, int a3, RECT * a4) {
    char *base = reinterpret_cast<char *>(a1);
    int left = a4->left;
    int top = a4->top;
    Buffer *embedded = reinterpret_cast<Buffer *>(base + 0x444);

    if (a3 == 2) {
        int height = a4->bottom - top;
        int width = a4->right - left;
        int y = *reinterpret_cast<int *>(base + 0x140) + top;
        int x = *reinterpret_cast<int *>(base + 0x13c) + left;
        Buffer *globalBuf = *reinterpret_cast<Buffer **>(g_0078d9a8);
        embedded->tile(globalBuf, x, y, left, top, width, height);
    }
    else if (a3 == 1) {
        int height = a4->bottom - top;
        int width = a4->right - left;
        int y = *reinterpret_cast<int *>(base + 0x140) + top;
        int x = *reinterpret_cast<int *>(base + 0x13c) + left;
        Buffer *globalBuf = *reinterpret_cast<Buffer **>(g_0078d980);
        embedded->tile(globalBuf, x, y, left, top, width, height);
    }
    else {
        int height = a4->bottom - top;
        int width = a4->right - left;
        int y = *reinterpret_cast<int *>(base + 0x140) + top;
        int x = *reinterpret_cast<int *>(base + 0x13c) + left;
        Buffer *globalBuf = *reinterpret_cast<Buffer **>(g_0078d548);
        embedded->tile(globalBuf, x, y, left, top, width, height);
    }

    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x29dc) == 0xc) {
        int color = 0xff - (a2 % 10000) / 5;
        embedded->set_text_color(color, -1, 1, 1);
    }
    return 1;
}
