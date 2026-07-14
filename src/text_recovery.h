#pragma once

#include "text.h"

class Strings;

void __cdecl text_close_source(Text *text);
void __cdecl text_set_get_ptr_source(Text *text, LPSTR *output);
void __cdecl text_set_item_ptr_source(Text *text, LPSTR *output);
LPSTR __cdecl text_get_source(Text *text);
LPSTR __cdecl text_string_source(Text *text, Strings *strings);
LPSTR __cdecl text_item_source(Text *text);
LPSTR __cdecl text_item_string_source(Text *text, Strings *strings);
int __cdecl text_item_number_source(Text *text);
int __cdecl text_item_binary_source(Text *text);
int __cdecl text_item_hex_source(Text *text);
int __cdecl text_get_number_source(Text *text, int min, int max);
