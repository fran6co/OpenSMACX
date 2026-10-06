/* OpenSMACX shared word storage and its mapped operations. The original
 * state domain and caller/lifetime relationship remain unidentified. */
#pragma once

#include <cstdint>

extern int32_t SharedWordValue;
void set_shared_word(int value);
void toggle_shared_word();
