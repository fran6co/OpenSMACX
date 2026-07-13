#include "stdafx.h"
#include "basepop.h"

/*
Purpose: Set the four fonts used by popup strings.
Original Offset: 006046F0
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int BasePop::set_string_font(Font *font1, Font *font2, Font *font3, Font *font4) {
    if (!font1) {
        return 3;
    }
    if (font1->is_initialized()) {
        string_font1_ = font1;
    }
    string_font2_ = font2;
    string_font3_ = font3;
    string_font4_ = font4;
    return 0;
}

int __fastcall base_pop_set_string_font_redirect(
        BasePop *self, void *, Font *font1, Font *font2, Font *font3, Font *font4) {
    return self->set_string_font(font1, font2, font3, font4);
}
