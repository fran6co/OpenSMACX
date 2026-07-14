#include "stdafx.h"
#include "font.h"

Font::Font()
    : unk_1_(-1),
      is_fot_set_(0),
      font_obj_(nullptr),
      line_height_(0),
      height_(0),
      ascent_(0),
      descent_(0),
      fot_file_name_(nullptr) {
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

Font::Font(LPSTR font_name, int height, int style) {
    init(font_name, height, static_cast<uint32_t>(style));
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

Font::~Font() {
    close();
}
