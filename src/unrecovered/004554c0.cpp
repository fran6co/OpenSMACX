// ORIGINAL: 0x004554C0 ?def_caption_draw@@YAHPAUGraphicWin@@PADPAUBuffer@@@Z 0x004554C0-0x00455606
// TRIED: the four locals (local_14/local_10/local_c/local_8, adjacent on the stack in that order) are exactly a RECT (left, top, right, bottom) in memory order, so assigning through a local RECT was tried directly - it compiles but the register-save schedule differs (rebuilt saves ebx/edi immediately, the original defers them past both early-exit guards). Best reached: MISMATCH, edit_count 25, 317 bytes vs 326, mnemonic similarity ~0.73.
// size      326 bytes
// prototype
// callers   0   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D8000 0x005DAC70 0x005DACB0 0x005DCF40 0x005DD130 0x005DD450 0x006453E0
int __cdecl def_caption_draw(GraphicWin * a1, char * a2, Buffer * a3) {
    RECT rect;
    unsigned int len;
    char *base = (char *)a1;
    char *bbase = (char *)a3;

    if ((void *)a1 == (void *)g_008e9f60 || a2 == 0) {
        return 0;
    }
    rect.right = *(int *)(bbase + 0x38) - 0x28;
    rect.left = *(int *)(bbase + 0x30) + 0x23 + *(int *)(*(int *)(base + 0x9dc) + 0x20);
    rect.top = *(int *)(bbase + 0x34) + 0xb + *(int *)(*(int *)(base + 0x9e8) + 0x24);
    rect.bottom = *(int *)(bbase + 0x3c) - 0xc;
    a3->set_text_color(*(int *)(base + 0x104), *(int *)(base + 0x108),
                        *(int *)(base + 0x10c), *(int *)(base + 0x110));
    a3->set_font((Font *)*(int *)(base + 0xf8), 0, 0, 0);
    a3->set_clip(&rect);
    if ((*(unsigned int *)(base + 0x98) & 0x10000) != 0) {
        len = strlen(a2);
        a3->write_l(a2, &rect, len);
        return a3->set_clip((RECT *)(bbase + 0x30));
    }
    if ((*(unsigned int *)(base + 0x98) & 0x40000) != 0) {
        len = strlen(a2);
        a3->write_right_l(a2, &rect, len);
        return a3->set_clip((RECT *)(bbase + 0x30));
    }
    len = strlen(a2);
    a3->write_cent_l(a2, &rect, len);
    return a3->set_clip((RECT *)(bbase + 0x30));
}
