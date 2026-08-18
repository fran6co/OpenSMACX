// ORIGINAL: 0x005ED5C0 ?center@Win@@QAEHXZ 0x005ED5C0-0x005ED6F7
// RULED-OUT: named `parent` local for win_parent_ (extra register pressure, forces a 4th callee-save + frame pointer); computing both local_8 and local_4 divisions fully inside each if/else branch (extra register); calling screen_to_client() on win_parent_ itself instead of win_parent_->win_parent_ (asm's ecx at the call site is loaded from [win_parent_+0xc4], i.e. the grandparent, reused straight from the preceding null check). Best reached: MISMATCH, edit_count 10/95 mnemonics, 312 bytes vs 311 - a register-colouring difference only (rebuilt keeps `this` in esi where the original keeps it in ebx).
// size      311 bytes
// prototype int (__thiscall ?center@Win@@QAEHXZ)(Win* this)
// callers   6   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0 0x005ED7D0
int Win::center() {
    int local_8;
    int local_4;
    int top;
    int bottom;

    if ((iSomeFlag_ & 2) != 0) {
        local_8 = (client_rect_.left - client_rect_.right + *g_009b7b1c) / 2;
        bottom = client_rect_.bottom;
        top = client_rect_.top;
    } else {
        local_8 = (outer_rect_.left - outer_rect_.right + *g_009b7b1c) / 2;
        bottom = outer_rect_.bottom;
        top = outer_rect_.top;
    }
    local_4 = (top - bottom + *g_009b7b20) / 2;
    if (win_parent_ != 0) {
        local_8 -= win_parent_->client_rect_.left + win_parent_->outer_rect_.left;
        local_4 -= win_parent_->client_rect_.top + win_parent_->outer_rect_.top;
        if ((win_parent_->iFlags_ & 0x20) != 0 && win_parent_->win_parent_ != 0) {
            win_parent_->win_parent_->screen_to_client(&local_8, &local_4);
            if ((win_parent_->iFlags_ & 0x8000) != 0) {
                local_8 += win_parent_->win_parent_->outer_rect_.left;
                local_4 += win_parent_->win_parent_->outer_rect_.top;
                return move(local_8, local_4);
            }
        }
    }
    return move(local_8, local_4);
}
