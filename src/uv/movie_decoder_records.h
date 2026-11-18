#pragma once

#include <stdint.h>

// One 16-byte record shared by the original static table owner and the
// frame-initialization consumer. Fields are named only where the decoder
// reads them; the remaining word stays explicitly unresolved.
struct MediaDecoderCodebookRecord {
  uint32_t word_before_cursor;
  uint32_t word_at_cursor;
  uint32_t unresolved_word_4;
  uint32_t selector_word_at_plus8;
};
