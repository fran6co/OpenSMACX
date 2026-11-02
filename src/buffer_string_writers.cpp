// Buffer layout and drawing for string-list entries.
#include "stdafx.h"
#include "buffer.h"
#include "stringstruct.h"

int Buffer::write_strings_height(StringList *strings, int width, int flags) {
    if (strings == 0) {
        return 0;
    }
    if ((flags & 2) == 0 && strings->entry_count_ - 1 >= 0) {
        strings->current_position_ = 0;
        strings->current_ = strings->head_;
    }

    // The payload node's metadata word (+8) and text (+4) are what
    // StringStructPayload declares; `current_entry` walks to the current one.
    int payload = strings->head_ != 0 ? strings->current_entry()->field_8_ : 0;

    if (field_C_ != 0) {
        const int id = strings->head_ != 0 ? strings->current_->id : 0;
        field_C_(id, payload);
    }

    int total_height = 0;
    if (payload == 3) {
        if (fonts_[0] == 0) {
            fonts_[0] = FontDefault;
        }
        total_height = fonts_[0]->line_advance();
        field_18_ = 0;
        field_14_ = field_10_;
        markup_pending_ = 0;
        BufferFlyingWidth = width;
        BufferFlyingY = 0;
        BufferStrHeight = 0;
        wrap_height_flying(nullptr);
    } else if (payload == 2) {
        char *text = strings->head_ != 0 ? strings->current_entry()->text : 0;
        if (text != 0) {
            field_18_ = 0;
            BufferStrHeight = 0;
            field_14_ = field_10_;
            int remaining_width = width;
            do {
                const int len = strlen(text);
                int break_width = remaining_width;
                text = find_line_break_l(text, &break_width, len);
                BufferStrHeight = 0;
                total_height += text_line_height();
                field_18_ = field_18_ + 1;
                if (field_14_ != 0) {
                    remaining_width -= field_14_;
                    field_14_ = 0;
                }
            } while (text != 0);
        }
        field_18_ = 0;
        field_14_ = field_10_;
        markup_pending_ = 0;
        BufferStrHeight = 0;
        BufferFlyingWidth = width;
        BufferFlyingY = 0;
        wrap_height_flying(nullptr);
    } else {
        const int id = strings->head_ != 0 ? strings->current_->id : 0;
        char *text = strings->head_ != 0 ? strings->current_entry()->text : 0;
        field_18_ = 0;
        field_14_ = field_10_;
        markup_pending_ = 0;
        BufferStrHeight = 0;
        BufferFlyingWidth = width;
        BufferFlyingY = 0;
        wrap_height_flying(text);
    }

    if (strings->current_position_ < strings->entry_count_ - 1) {
        do {
            strings->next_entry();
            if (strings->current_ != 0) {
                strings->current_entry();
            }

            const int line_payload =
                strings->head_ != 0 ? strings->current_entry()->field_8_ : 0;

            if (field_C_ != 0) {
                const int id =
                    strings->head_ != 0 ? strings->current_->id : 0;
                field_C_(id,
                                                                 line_payload);
            }

            if (strings->head_ == 0 || strings->current_entry()->field_8_ == 0) {
                wrap_height_flying(strings->current_text());
            } else {
                total_height += wrap_height_flying_flush();

                if (line_payload == 3) {
                    total_height += text_line_height();
                    wrap_height_flying(nullptr, width);
                } else if (line_payload == 2) {
                    total_height += wrap_height(strings->current_text(), width);
                    wrap_height_flying(nullptr, width);
                } else {
                    wrap_height_flying(strings->current_text(), width);
                }
            }
        } while (strings->current_position_ < strings->entry_count_ - 1);
    }

    return wrap_height_flying_flush() + total_height;
}

int Buffer::write_strings(StringList *strings, int left, int top, int width,
                          int flags) {
    if (strings == 0) return top;
    if ((flags & 2) == 0 && strings->entry_count_ - 1 >= 0) {
        strings->current_position_ = 0;
        strings->current_ = strings->head_;
    }
    int kind = strings->head_ != 0 ? strings->current_entry()->field_8_ : 0;
    if (field_C_ != 0) {
        int id = strings->head_ != 0 ? strings->current_->id : 0;
        field_C_(id, kind);
    }
    int extent = top;
    char *first_text = 0;
    if (kind == 3) {
        if (fonts_[0] == 0) fonts_[0] = FontDefault;
        extent += fonts_[0]->line_advance();
    } else {
        first_text = strings->head_ != 0
            ? strings->current_entry()->text : 0;
        if (kind == 2 && first_text != 0) {
            BufferStrHeight = 0;
            field_18_ = 0;
            char *cursor = first_text;
            do {
                char *next = find_line_break(cursor, width);
                if (next != 0) {
                    write_cent_l(cursor, left, extent, width,
                                 static_cast<int>(next - cursor));
                } else {
                    write_cent(cursor, left, extent, width);
                }
                ++field_18_;
                extent += text_line_height();
                cursor = next;
            } while (cursor != 0);
        }
    }
    // Original stores branch3's count before width, the other branches do
    // width before count. No intervening callback observes that distinction.
    if (kind == 3) {
        field_18_ = 0;
        field_14_ = field_10_;
    } else {
        field_14_ = field_10_;
        field_18_ = 0;
    }
    BufferFlyingX = left;
    BufferFlyingY = extent;
    BufferFlyingWidth = width;
    BufferStrHeight = 0;
    markup_pending_ = 0;
    BufferFlyingText[0] = 0;
    wrap_flying(kind == 3 || kind == 2 ? 0 : first_text);

    while (strings->current_position_ < strings->entry_count_ - 1) {
        if (extent > -dib_.bmiHeader.biHeight) break;
        strings->next_entry();
        if (strings->current_ != 0) strings->current_entry();
        int entry_kind = strings->head_ != 0
            ? strings->current_entry()->field_8_ : 0;
        bool plain = strings->head_ == 0;
        if (!plain) plain = strings->current_entry()->field_8_ == 0;
        if (plain) {
            if (field_C_ != 0) {
                // The native evaluates current_id then RELOADS field_C_;
                // unlike its two earlier callbacks, it does not capture the
                // procedure before this service call.
                int id = strings->current_id();
                field_C_(id, entry_kind);
            }
            wrap_flying(strings->current_text());
        } else {
            if (BufferFlyingText[0] != 0) {
                write_multi_font_raw(BufferFlyingText, BufferFlyingX, BufferFlyingY);
                int advance = text_line_height();
                BufferFlyingY += advance;
                ++field_18_;
            }
            BufferFlyingText[0] = 0;
            BufferFlyingWidth = 0;
            select_text_slot(0);
            int saved_y = BufferFlyingY;
            if (field_C_ != 0) {
                int id = strings->head_ != 0 ? strings->current_->id : 0;
                field_C_(id, entry_kind);
            }
            if (saved_y >= extent) extent = saved_y;
            if (entry_kind == 3) {
                if (fonts_[0] == 0) fonts_[0] = FontDefault;
                extent += fonts_[0]->line_advance();
                wrap_flying(0, left, extent, width);
            } else if (entry_kind == 2) {
                extent = wrap_cent(strings->current_text(), left, extent, width);
                wrap_flying(0, left, extent, width);
            } else {
                wrap_flying(strings->current_text(), left, extent, width);
            }
        }
    }
    int flushed = wrap_flying_flush();
    if (flushed >= extent) extent = flushed;
    return extent;
}
