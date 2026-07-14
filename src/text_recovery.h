#pragma once

#include "text.h"

class Strings;

LPSTR __cdecl text_get_source(Text *text);
LPSTR __cdecl text_string_source(Text *text, Strings *strings);
LPSTR __cdecl text_item_source(Text *text);
LPSTR __cdecl text_item_string_source(Text *text, Strings *strings);
int __cdecl text_item_number_source(Text *text);
