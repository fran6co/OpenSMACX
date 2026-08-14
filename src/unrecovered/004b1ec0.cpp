// ORIGINAL: 0x004B1EC0 FILE
// RULED-OUT: nothing yet - MISMATCH #8 call/mov (90% mnemonic sim); not
//            chased further. On a match, `break` only the inner (column)
//            loop - the outer (row) loop keeps running, so a LATER row's
//            match overwrites an earlier one (last match wins, not first).
// name      ?on_mouse_move@SocialWin@@QAEXHH@Z
// size      242 bytes
// spans     0x004B1EC0-0x004B1FB2
// prototype void (__thiscall ?on_mouse_move@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B1080 0x005B4730 0x005F8500 0x005FA7E0
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x004B1EC0

void SocialWin::on_mouse_move(int a1, int a2) {
    char *const self = (char *)this;
    Win::clear_bubble_text();
    Win::clear_bubble_text();

    char *rowRectPtr = self + 0x1F110;
    int best = -1;
    int row = 0;
    for (;;) {
        int col = 0;
        for (;;) {
            int avail = society_avail(row, col, *(int *)(self + 0xCF4));
            if (avail != 0) {
                RECT cell;
                cell.top = *(long *)(rowRectPtr + 4);
                cell.bottom = *(long *)(rowRectPtr + 0xC);
                cell.left = *(long *)(self + (col + 0x1F15 + row * 4) * 0x10);
                cell.right = *(long *)(self + (col + row * 4) * 0x10 + 0x1F158);
                int hit = in_box(a1, a2, &cell);
                if (hit != 0) {
                    best = col + row * 4;
                    break;
                }
            }
            ++col;
            if (col >= 4) break;
        }
        ++row;
        rowRectPtr += 0x10;
        if (row >= 4) {
            break;
        }
    }

    int result = in_box(a1, a2, (RECT *)(self + 0x1F2A0));
    int chosen = -2;
    if (result == 0) {
        chosen = best;
    }
    if (chosen != *(int *)(self + 0xA2C)) {
        *(int *)(self + 0xA2C) = chosen;
        ((SocialWin *)self)->draw_mouseover();
    }
}
