#pragma once
#include "font.h"

// Share the observed signed Font metric operation with clients whose pinned
// bodies contain it inline. This helper name is descriptive; its original
// spelling is unknown. The out-of-line Font::line_advance API remains owned
// by font.cpp for the original direct-call consumers.
inline int font_line_advance(const Font &font) {
  const int adjustment = font.unk_1_;
  if (adjustment >= 0) {
    return static_cast<int>(static_cast<unsigned int>(font.pixel_height()) +
                            static_cast<unsigned int>(adjustment));
  }
  return font.line_height();
}
