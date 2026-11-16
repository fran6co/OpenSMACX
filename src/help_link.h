#pragma once

#include <string.h>

// Append the original help markup to an existing caller-owned text buffer.
// An inline: help_tech_info and Datalink's draw_unit, draw_chassis and
// draw_social expand it, and its "$LINK<", "=" and ">" literals are the
// linker-merged ??_C copies at 0x00683544/0x00683540/0x0068353C (bound in
// help.cpp). The out-of-line copy (0x0044CB80) is help.cpp's.
// WORKAROUND(decl): the original header is unknown; defining the inline in
// datalink.h moves Datalink::set_cat_social's spill reloads, so only its
// two users include it.
inline void __cdecl help_create_link(char *out, char *label, char *id) {
    strcat(out, "$LINK<");
    strcat(out, label);
    strcat(out, "=");
    strcat(out, id);
    strcat(out, ">");
}
