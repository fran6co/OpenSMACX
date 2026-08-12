#include "stdafx.h"
#include "basepop.h"

/*
Purpose: Set the four fonts used by popup strings.
ORIGINAL: 0x006046F0
// name      ?set_string_font@BasePop@@QAEHPAUFont@@PAUFont@@PAUFont@@PAUFont@@@Z
// size      64 bytes
// spans     0x006046F0-0x00604730
// prototype int (__thiscall ?set_string_font@BasePop@@QAEHPAUFont@@PAUFont@@PAUFont@@PAUFont@@@Z)(BasePop* this, Font*, Font*, Font*, Font*)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
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

/*
Purpose: Install the three button fonts, refusing a null primary.

             mov eax,[esp+4] / test eax,eax / jne have
             mov eax,3 / ret 0xC
             have: mov edx,[eax+8] / test edx,edx / je skip
             mov [ecx+0x316C],eax
             skip: .. store 0x3170 and 0x3174 .. / xor eax,eax / ret 0xC

         Exactly the shape set_string_font above has, with three fonts instead
         of four: a null primary is refused with 3 and NOTHING is stored, while
         an uninitialised primary - `[eax+8]` zero, which is Font::
         is_initialized - is silently skipped while the other two are still
         installed. Those are different failures and the return code does not
         distinguish them, which is the original's behaviour and not an
         oversight here.
ORIGINAL: 0x006047F0
// name      ?set_button_font@BasePop@@QAEHPAUFont@@PAUFont@@PAUFont@@@Z
// size      54 bytes
// spans     0x006047F0-0x00604826
// prototype int (__thiscall ?set_button_font@BasePop@@QAEHPAUFont@@PAUFont@@PAUFont@@@Z)(BasePop* this, Font*, Font*, Font*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 3 when the primary font is null, otherwise 0
Status: Complete
*/
int BasePop::set_button_font(Font *font1, Font *font2, Font *font3) {
    if (!font1) {
        return 3;
    }
    if (font1->is_initialized()) {
        button_font1_ = font1;
    }
    button_font2_ = font2;
    button_font3_ = font3;
    return 0;
}

int __fastcall base_pop_set_button_font_redirect(
        BasePop *self, void *, Font *font1, Font *font2, Font *font3) {
    return self->set_button_font(font1, font2, font3);
}
