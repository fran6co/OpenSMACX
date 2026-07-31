// Recovery leaf tests: buffer_writers.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "buffer_writers"

namespace {

void *g_pw_slot_self;

int g_pw_slot_calls;

void __thiscall observe_pw_blink_slot(void *self) {
    g_pw_slot_self = self; ++g_pw_slot_calls;
}

struct CopyCall {
    void *self; Buffer *src;
    int x, y, sx, sy, w, h;
};

CopyCall g_copy;

int g_copy_calls;

int g_copy_result;

int __thiscall observe_buffer_copy_full(void *self, Buffer *src, int x, int y,
                                        int sx, int sy, int w, int h) {
    g_copy = {self, src, x, y, sx, sy, w, h};
    ++g_copy_calls;
    return g_copy_result;
}

struct BoxLineCall {
    int kind;      // 0 = hline, 1 = vline
    Buffer *self;
    int a, b, c;   // hline: x1, x2, y   vline: x, y1, y2
    int color;
};

BoxLineCall g_box_line[8];

int g_box_line_calls;

RECT *g_box_mutate_rect;   // when set, the FIRST seam call rewrites the rect

void record_box_line(int kind, Buffer *self, int a, int b, int c, int color) {
    if (g_box_line_calls < 8) {
        g_box_line[g_box_line_calls] = {kind, self, a, b, c, color};
    }
    ++g_box_line_calls;
    // The original reads all four fields before the first call, so a callee
    // that rewrites the rectangle must not move the remaining three edges.
    // Distinguishes up-front reads from per-call re-reads.
    if (g_box_mutate_rect != nullptr && g_box_line_calls == 1) {
        g_box_mutate_rect->left = 0x7A7A7A7A;
        g_box_mutate_rect->top = 0x7B7B7B7B;
        g_box_mutate_rect->right = 0x7C7C7C7C;
        g_box_mutate_rect->bottom = 0x7D7D7D7D;
    }
}

void __thiscall observe_box_hline(Buffer *self, int x1, int x2, int y,
                                  int color) {
    record_box_line(0, self, x1, x2, y, color);
}

void __thiscall observe_box_vline(Buffer *self, int x, int y1, int y2,
                                  int color) {
    record_box_line(1, self, x, y1, y2, color);
}

// Named poison: the null-rectangle path dispatches nothing, so reaching
// either seam at all on that path is the failure.
void __thiscall poison_box_line_must_not_run(Buffer *, int, int, int, int) {
    expect(false);
}

}  // namespace

void test_buffer_box() {
    func_buffer_line *const saved_h = BufferHLine;
    func_buffer_line *const saved_v = BufferVLine;
    BufferHLine = &observe_box_hline;
    BufferVLine = &observe_box_vline;
    g_box_mutate_rect = nullptr;

    // Shape A: an exactly sized seeded allocation; the rectangle on the
    // stack. The dispatcher must not read or write the Buffer at all.
    std::vector<uint8_t> bs(sizeof(Buffer), 0);
    std::vector<uint8_t> bexp(bs.size());
    Buffer *const buf = reinterpret_cast<Buffer *>(bs.data());
    seed_storage(bs.data(), bexp.data(), bs.size());

    // Two distinct colors: real callers pass the same color twice, so only
    // distinct values make a crossed color1/color2 visible.
    const int color1 = 0x01111111;
    const int color2 = 0x02222222;

    // All four fields distinct, and no derived edge coincides with another,
    // so a swapped span, an off-by-one, a reused edge, or a wrong-order call
    // lands on a number no correct transcript contains.
    RECT rect = {10, 20, 33, 47};   // left, top, right, bottom
    g_box_line_calls = 0;
    expect(buf->box(&rect, color1, color2) == 0);
    expect(g_box_line_calls == 4);
    // [0] top edge: hline(left+1, right-1, top, color1).
    expect(g_box_line[0].kind == 0 && g_box_line[0].self == buf);
    expect(g_box_line[0].a == 11 && g_box_line[0].b == 32);
    expect(g_box_line[0].c == 20 && g_box_line[0].color == color1);
    // [1] bottom edge: hline(left, right-2, bottom-1, color2).
    expect(g_box_line[1].kind == 0 && g_box_line[1].self == buf);
    expect(g_box_line[1].a == 10 && g_box_line[1].b == 31);
    expect(g_box_line[1].c == 46 && g_box_line[1].color == color2);
    // [2] left edge: vline(left, top, bottom-2, color1).
    expect(g_box_line[2].kind == 1 && g_box_line[2].self == buf);
    expect(g_box_line[2].a == 10 && g_box_line[2].b == 20);
    expect(g_box_line[2].c == 45 && g_box_line[2].color == color1);
    // [3] right edge: vline(right-1, top+1, bottom-1, color2).
    expect(g_box_line[3].kind == 1 && g_box_line[3].self == buf);
    expect(g_box_line[3].a == 32 && g_box_line[3].b == 21);
    expect(g_box_line[3].c == 46 && g_box_line[3].color == color2);
    // The dispatcher touches neither the object nor the rectangle.
    expect_storage_bytes(bs.data(), bexp.data(), bs.size());
    expect(rect.left == 10 && rect.top == 20);
    expect(rect.right == 33 && rect.bottom == 47);

    // Signed-extreme wrap through the redirect adapter: the original's
    // inc/dec are raw 32-bit ops, so every edge wraps rather than saturates.
    RECT wrap = {INT_MAX, INT_MAX, INT_MIN, INT_MIN};
    g_box_line_calls = 0;
    expect(buffer_box_redirect(buf, nullptr, &wrap, color1, color2) == 0);
    expect(g_box_line_calls == 4);
    expect(g_box_line[0].a == INT_MIN);          // left+1 wraps
    expect(g_box_line[0].b == INT_MAX);          // right-1 wraps
    expect(g_box_line[0].c == INT_MAX && g_box_line[0].color == color1);
    expect(g_box_line[1].a == INT_MAX);          // left
    expect(g_box_line[1].b == INT_MAX - 1);      // right-2 wraps
    expect(g_box_line[1].c == INT_MAX && g_box_line[1].color == color2);
    expect(g_box_line[2].a == INT_MAX && g_box_line[2].b == INT_MAX);
    expect(g_box_line[2].c == INT_MAX - 1);      // bottom-2 wraps
    expect(g_box_line[2].color == color1);
    expect(g_box_line[3].a == INT_MAX);          // right-1 wraps
    expect(g_box_line[3].b == INT_MIN);          // top+1 wraps
    expect(g_box_line[3].c == INT_MAX && g_box_line[3].color == color2);
    expect_storage_bytes(bs.data(), bexp.data(), bs.size());

    // Shape B: the object embedded mid-arena between seeded guard bands with
    // the rectangle inside the arena too, and a reversed/degenerate
    // rectangle (right < left, bottom < top): box derives edges without
    // comparing, so the raw values flow straight through.
    std::vector<uint8_t> arena(sizeof(Buffer) + 128, 0);
    std::vector<uint8_t> aexp(arena.size());
    seed_storage(arena.data(), aexp.data(), arena.size());
    Buffer *const embedded = reinterpret_cast<Buffer *>(arena.data() + 64);
    RECT *const arect = reinterpret_cast<RECT *>(arena.data() + 16);
    const RECT reversed = {-100, -200, -299, -403};
    std::memcpy(arena.data() + 16, &reversed, sizeof(reversed));
    std::memcpy(aexp.data() + 16, &reversed, sizeof(reversed));
    g_box_line_calls = 0;
    expect(embedded->box(arect, -7, -8) == 0);
    expect(g_box_line_calls == 4);
    expect(g_box_line[0].self == embedded);
    expect(g_box_line[0].a == -99 && g_box_line[0].b == -300);
    expect(g_box_line[0].c == -200 && g_box_line[0].color == -7);
    expect(g_box_line[1].a == -100 && g_box_line[1].b == -301);
    expect(g_box_line[1].c == -404 && g_box_line[1].color == -8);
    expect(g_box_line[2].a == -100 && g_box_line[2].b == -200);
    expect(g_box_line[2].c == -405 && g_box_line[2].color == -7);
    expect(g_box_line[3].a == -300 && g_box_line[3].b == -199);
    expect(g_box_line[3].c == -404 && g_box_line[3].color == -8);
    // Guard bands, object bytes, and the in-arena rectangle all unchanged.
    expect_storage_bytes(arena.data(), aexp.data(), arena.size());

    // Pre-read probe: the first seam call rewrites the rectangle; the three
    // remaining calls must still carry edges derived from the original
    // fields, exactly as the original's up-front register loads do.
    RECT mutable_rect = {10, 20, 33, 47};
    g_box_mutate_rect = &mutable_rect;
    g_box_line_calls = 0;
    expect(buf->box(&mutable_rect, color1, color2) == 0);
    g_box_mutate_rect = nullptr;
    expect(g_box_line_calls == 4);
    expect(g_box_line[0].a == 11 && g_box_line[0].b == 32 &&
           g_box_line[0].c == 20);
    expect(g_box_line[1].a == 10 && g_box_line[1].b == 31 &&
           g_box_line[1].c == 46);
    expect(g_box_line[2].a == 10 && g_box_line[2].b == 20 &&
           g_box_line[2].c == 45);
    expect(g_box_line[3].a == 32 && g_box_line[3].b == 21 &&
           g_box_line[3].c == 46);
    // The observer's writes stand - box does not restore the rectangle.
    expect(mutable_rect.left == 0x7A7A7A7A);
    expect(mutable_rect.top == 0x7B7B7B7B);
    expect(mutable_rect.right == 0x7C7C7C7C);
    expect(mutable_rect.bottom == 0x7D7D7D7D);
    expect_storage_bytes(bs.data(), bexp.data(), bs.size());

    // Null rectangle: EAX residue 3, nothing dispatched, nothing written.
    // The poison seams turn any dispatch on this path into a hard failure,
    // through both the direct call and the redirect adapter.
    BufferHLine = &poison_box_line_must_not_run;
    BufferVLine = &poison_box_line_must_not_run;
    expect(buf->box(nullptr, color1, color2) == 3);
    expect(buffer_box_redirect(buf, nullptr, nullptr, 5, 6) == 3);
    expect_storage_bytes(bs.data(), bexp.data(), bs.size());

    BufferHLine = saved_h;
    BufferVLine = saved_v;
}

namespace {

struct RawWriteCall {
    Buffer *self;
    LPSTR text;
    int x, y, len;
};

RawWriteCall g_raw_write;

int g_raw_write_calls;

int g_raw_write_result;

struct TextWidthCall {
    Buffer *self;
    LPSTR text;
    size_t length;
};

TextWidthCall g_tw;

int g_tw_calls;

int g_tw_result;

// When set, the measured overload clears the buffer's font1_ slot, standing
// in for the default-font rebind the real measured body performs at
// 0x005DC7D3. Only write_cent_l's rectangle overload can observe it.
Buffer *g_tw_clear_font_of;

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

int __thiscall observe_raw_write(Buffer *self, LPSTR text, int x, int y,
                                 int len) {
    g_raw_write = {self, text, x, y, len};
    ++g_raw_write_calls;
    return g_raw_write_result;
}

// Named poison: every rejection path dispatches nothing, so reaching the
// raster writer at all on one of them is the failure.
int __thiscall poison_raw_write(Buffer *, LPSTR, int, int, int) {
    expect(false);
    return 0;
}

int __thiscall observe_writer_text_width(Buffer *self, LPSTR text,
                                         size_t length) {
    g_tw = {self, text, length};
    ++g_tw_calls;
    if (g_tw_clear_font_of != nullptr) {
        write_at(reinterpret_cast<uint8_t *>(g_tw_clear_font_of), 0x52C,
                 static_cast<Font *>(nullptr));
    }
    return g_tw_result;
}

// The two left-aligned overloads measure nothing at all; reaching the
// measured overload from either is the failure.
int __thiscall poison_text_width(Buffer *, LPSTR, size_t) {
    expect(false);
    return 0;
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

// A Font shaped just enough for these writers: font_obj_ at 0x08 is the
// initialised flag they guard on, height_ at 0x10 drives vertical centring.
void make_writer_font(uint8_t *font_storage, uint32_t object_handle,
                      int height) {
    std::memset(font_storage, 0, sizeof(Font));
    write_at(font_storage, 0x08, object_handle);
    write_at(font_storage, 0x10, height);
}

void reset_writer_probes() {
    g_raw_write = RawWriteCall{};
    g_raw_write_calls = 0;
    g_tw = TextWidthCall{};
    g_tw_calls = 0;
    g_tw_result = 0;
    g_tw_clear_font_of = nullptr;
}

}  // namespace

// The four length-limited text writers. Their whole reason for being four
// separate transcriptions is three asymmetries that a shared helper would
// erase, so each is pinned here explicitly:
//   - the font guard returns 3 from 0x005DCEA0, 0x005DD020 and 0x005DD130 but
//     0 from 0x005DCF40, so the two RECT overloads disagree with each other;
//   - 0x005DD020 measures the CLAMPED count while 0x005DD130 measures a fresh
//     strlen of the whole string;
//   - an empty draw returns the incoming x from the scalar overloads and 0
//     from the RECT overloads.
void test_buffer_text_writers() {
    auto *const saved_raw = BufferWriteMultiFontRawL;
    auto *const saved_width = BufferTextWidthMeasured;

    alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
    uint8_t expected[sizeof(storage)];
    auto *const buffer = reinterpret_cast<Buffer *>(storage + 16);
    alignas(Font) uint8_t font_storage[sizeof(Font)];
    auto *const font = reinterpret_cast<Font *>(font_storage);

    char text[] = "hello world";   // 11 characters
    char empty[] = "";

    // Which overload, spelled once so every shape below runs through all four.
    enum Which { ScalarPlain, RectPlain, ScalarCent, RectCent };
    RECT rect = {10, 20, 110, 61};
    auto call = [&](Which which, LPSTR subject, RECT *box, int len) {
        switch (which) {
            case ScalarPlain:
                return buffer->write_l(subject, 4242, 77, len);
            case RectPlain:
                return buffer->write_l(subject, box, len);
            case ScalarCent:
                return buffer->write_cent_l(subject, 1234, 77, 100, len);
            default:
                return buffer->write_cent_l(subject, box, len);
        }
    };
    auto call_adapter = [&](Which which, LPSTR subject, RECT *box, int len) {
        switch (which) {
            case ScalarPlain:
                return buffer_write_l_redirect(buffer, nullptr, subject, 4242,
                                               77, len);
            case RectPlain:
                return buffer_write_l_rect_redirect(buffer, nullptr, subject,
                                                    box, len);
            case ScalarCent:
                return buffer_write_cent_l_redirect(buffer, nullptr, subject,
                                                    1234, 77, 100, len);
            default:
                return buffer_write_cent_l_rect_redirect(buffer, nullptr,
                                                         subject, box, len);
        }
    };
    auto install = [&](uint32_t font_obj, int height, bool font_set) {
        make_writer_font(font_storage, font_obj, height);
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16 + 0x52C,
                 font_set ? font : static_cast<Font *>(nullptr));
        std::memcpy(expected, storage, sizeof(storage));
        reset_writer_probes();
    };

    // ---- the rejection matrix, and with it the return-code asymmetry ----
    // Each row is a reason to reject; each column is what that overload hands
    // back. The 3-vs-0 split between the two RECT overloads is the single
    // most likely thing to get wrong by sharing a guard helper.
    struct RejectCase {
        const char *label;
        bool font_set; uint32_t font_obj; LPSTR subject; bool use_rect;
        int len;
        int want[4];      // ScalarPlain, RectPlain, ScalarCent, RectCent
        bool applies[4];  // a null rect is not a rejection for the scalars,
                          // which never look at one
    };
    const RejectCase rejects[] = {
        {"null string",   true,  0xF0F0F0F0U, nullptr, true,  5,
         {4242, 0, 1234, 0}, {true, true, true, true}},
        {"null rect",     true,  0xF0F0F0F0U, text,    false, 5,
         {0, 0, 0, 0},       {false, true, false, true}},
        {"no font",       false, 0U,          text,    true,  5,
         {3, 0, 3, 3},       {true, true, true, true}},
        {"font unopened", true,  0U,          text,    true,  5,
         {3, 0, 3, 3},       {true, true, true, true}},
        {"negative len",  true,  0xF0F0F0F0U, text,    true,  -1,
         {4242, 0, 1234, 0}, {true, true, true, true}},
        {"zero len",      true,  0xF0F0F0F0U, text,    true,  0,
         {4242, 0, 1234, 0}, {true, true, true, true}},
        {"empty string",  true,  0xF0F0F0F0U, empty,   true,  5,
         {4242, 0, 1234, 0}, {true, true, true, true}},
        {"signed extreme", true, 0xF0F0F0F0U, text,    true,  INT_MIN,
         {4242, 0, 1234, 0}, {true, true, true, true}},
    };
    for (const RejectCase &shape : rejects) {
        for (int which = 0; which < 4; ++which) {
            if (!shape.applies[which]) {
                continue;
            }
            for (int adapter = 0; adapter < 2; ++adapter) {
                BufferWriteMultiFontRawL = &poison_raw_write;
                BufferTextWidthMeasured = &poison_text_width;
                install(shape.font_obj, 7, shape.font_set);
                RECT box = rect;
                RECT *const arg = shape.use_rect ? &box : nullptr;
                const int result = adapter
                    ? call_adapter(static_cast<Which>(which), shape.subject,
                                   arg, shape.len)
                    : call(static_cast<Which>(which), shape.subject, arg,
                           shape.len);
                expect(result == shape.want[which]);
                expect(g_raw_write_calls == 0);
                expect(g_tw_calls == 0);
                // No writer touches the object on a rejection.
                expect_storage_bytes(storage, expected, sizeof(storage));
                expect(box.left == rect.left && box.top == rect.top);
                expect(box.right == rect.right && box.bottom == rect.bottom);
            }
        }
    }

    // ---- the clamp: min(strlen, len), signed, all three orderings ----
    struct ClampCase { int len; int drawn_len; };
    const ClampCase clamps[] = {
        {20, 11},          // strlen wins
        {4, 4},            // len wins
        {11, 11},          // equal
        {1, 1},            // the smallest drawing count
        {INT_MAX, 11},     // signed extreme on the losing side
    };
    for (const ClampCase &shape : clamps) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            BufferWriteMultiFontRawL = &observe_raw_write;
            BufferTextWidthMeasured = &poison_text_width;
            install(0xF0F0F0F0U, 7, true);
            g_raw_write_result = -5150;   // the residue passes straight out
            const int result = adapter
                ? call_adapter(ScalarPlain, text, nullptr, shape.len)
                : call(ScalarPlain, text, nullptr, shape.len);
            expect(result == -5150);
            expect(g_raw_write_calls == 1);
            expect(g_raw_write.self == buffer);
            expect(g_raw_write.text == text);
            expect(g_raw_write.x == 4242);   // forwarded verbatim
            expect(g_raw_write.y == 77);
            expect(g_raw_write.len == shape.drawn_len);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    // ---- one glyph is still a draw ----
    // The clamp table above reaches len == 1 through the scalar plain overload
    // only, which left `if (limit <= 0)` unobserved in the other three:
    // widening it to `<= 1` would silently swallow every single-character
    // draw and no assertion here would have moved. One row per overload.
    for (int which = 0; which < 4; ++which) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            const bool measures = which == ScalarCent || which == RectCent;
            BufferWriteMultiFontRawL = &observe_raw_write;
            BufferTextWidthMeasured =
                measures ? &observe_writer_text_width : &poison_text_width;
            install(0xF0F0F0F0U, 7, true);
            g_tw_result = 6;
            g_raw_write_result = 1717;
            RECT box = rect;
            const int result = adapter
                ? call_adapter(static_cast<Which>(which), text, &box, 1)
                : call(static_cast<Which>(which), text, &box, 1);
            expect(result == 1717);
            expect(g_raw_write_calls == 1);
            expect(g_raw_write.len == 1);
            expect(g_tw_calls == (measures ? 1 : 0));
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    // Signed-extreme coordinates travel through untouched: the scalar plain
    // overload performs no arithmetic on them at all.
    BufferWriteMultiFontRawL = &observe_raw_write;
    BufferTextWidthMeasured = &poison_text_width;
    install(0xF0F0F0F0U, 7, true);
    g_raw_write_result = 0;
    expect(buffer->write_l(text, INT_MIN, INT_MAX, 3) == 0);
    expect(g_raw_write.x == INT_MIN && g_raw_write.y == INT_MAX);
    expect(g_raw_write.len == 3);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // ---- vertical centring, write_l's RECT overload ----
    // top + (bottom - height - top) / 2, truncating toward zero, and x is
    // rect->left with no arithmetic and nothing measured.
    struct VCase { LONG left, top, right, bottom; int height;
                   int want_x, want_y; };
    const VCase vertical[] = {
        {10, 20, 33, 61, 1, 10, 40},        // span 40 -> +20
        {-5, 100, 7, 50, 3, -5, 74},        // span -53 -> -26 toward zero
        {77, 0, 1, 7, 0, 77, 3},            // span 7 -> +3 toward zero
        // The subtraction wraps: the span is INT_MIN, half of it is
        // 0xC0000000, and the sum back onto top wraps positive. Signed
        // saturation would give a different row.
        {INT_MIN, INT_MIN, 0, 0, 0, INT_MIN, 0x40000000},
    };
    for (const VCase &shape : vertical) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            BufferWriteMultiFontRawL = &observe_raw_write;
            BufferTextWidthMeasured = &poison_text_width;
            install(0xF0F0F0F0U, shape.height, true);
            g_raw_write_result = 909;
            RECT box = {shape.left, shape.top, shape.right, shape.bottom};
            const int result = adapter
                ? call_adapter(RectPlain, text, &box, 4)
                : call(RectPlain, text, &box, 4);
            expect(result == 909);
            expect(g_raw_write_calls == 1);
            expect(g_raw_write.x == shape.want_x);
            expect(g_raw_write.y == shape.want_y);
            expect(g_raw_write.len == 4);
            expect(box.left == shape.left && box.top == shape.top);
            expect(box.right == shape.right && box.bottom == shape.bottom);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    // ---- horizontal centring, and the measured-length asymmetry ----
    // write_cent_l's scalar overload measures the CLAMPED count.
    struct HCase { int x, width, drawn, len; int want_len, want_x; };
    const HCase horizontal[] = {
        {10, 100, 30, 4, 4, 45},          // clamped to 4; span 70 -> +35
        {10, 100, 30, 20, 11, 45},        // clamped to 11 by strlen
        {0, 100, 31, 11, 11, 34},         // odd span 69 -> +34 toward zero
        {10, 10, 13, 11, 11, 9},          // negative span -3 -> -1
        {INT_MIN, INT_MIN, 0, 11, 11, 0x40000000},   // wraps positive
    };
    for (const HCase &shape : horizontal) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            BufferWriteMultiFontRawL = &observe_raw_write;
            BufferTextWidthMeasured = &observe_writer_text_width;
            install(0xF0F0F0F0U, 7, true);
            g_tw_result = shape.drawn;
            g_raw_write_result = -31337;
            const int result = adapter
                ? buffer_write_cent_l_redirect(buffer, nullptr, text, shape.x,
                                               77, shape.width, shape.len)
                : buffer->write_cent_l(text, shape.x, 77, shape.width,
                                       shape.len);
            expect(result == -31337);
            expect(g_tw_calls == 1);
            expect(g_tw.self == buffer);
            expect(g_tw.text == text);
            // The clamped count, NOT strlen - this is the discriminator
            // against the rectangle overload below.
            expect(g_tw.length == static_cast<size_t>(shape.want_len));
            expect(g_raw_write_calls == 1);
            expect(g_raw_write.x == shape.want_x);
            expect(g_raw_write.y == 77);      // forwarded verbatim
            expect(g_raw_write.len == shape.want_len);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    // ---- write_cent_l's RECT overload: both centres, whole-string measure --
    struct BothCase { LONG left, top, right, bottom; int height, drawn, len;
                      int want_len, want_x, want_y; };
    const BothCase both[] = {
        {10, 20, 110, 61, 1, 30, 4, 4, 45, 40},     // measures 11, draws 4
        {10, 20, 110, 61, 1, 30, 20, 11, 45, 40},
        {-5, 100, -60, 50, 3, 8, 11, 11, -36, 74},  // both spans negative odd
        {0, 0, 7, 7, 0, 0, 11, 11, 3, 3},           // odd spans toward zero
        {INT_MIN, INT_MIN, 0, 0, 0, 0, 11, 11, 0x40000000, 0x40000000},
    };
    for (const BothCase &shape : both) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            BufferWriteMultiFontRawL = &observe_raw_write;
            BufferTextWidthMeasured = &observe_writer_text_width;
            install(0xF0F0F0F0U, shape.height, true);
            g_tw_result = shape.drawn;
            g_raw_write_result = 4711;
            RECT box = {shape.left, shape.top, shape.right, shape.bottom};
            const int result = adapter
                ? call_adapter(RectCent, text, &box, shape.len)
                : call(RectCent, text, &box, shape.len);
            expect(result == 4711);
            expect(g_tw_calls == 1);
            // Always strlen, never the clamped count.
            expect(g_tw.length == 11);
            expect(g_raw_write_calls == 1);
            expect(g_raw_write.x == shape.want_x);
            expect(g_raw_write.y == shape.want_y);
            expect(g_raw_write.len == shape.want_len);
            expect(box.left == shape.left && box.top == shape.top);
            expect(box.right == shape.right && box.bottom == shape.bottom);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    // ---- the rebind only write_cent_l's RECT overload can reach ----
    // The measured overload clears font1_; this body re-reads it afterwards
    // and substitutes the process default without re-running the guard, so an
    // unopened default font is used as-is. The other three would have to
    // dereference a null font to get here at all.
    alignas(Font) uint8_t default_font_storage[sizeof(Font)];
    auto *const default_font =
        reinterpret_cast<Font *>(default_font_storage);
    make_writer_font(default_font_storage, 0U, 9);   // deliberately unopened
    Font **const saved_default_ptr = FontDefaultPtr;
    Font *default_slot = default_font;
    FontDefaultPtr = &default_slot;
    BufferWriteMultiFontRawL = &observe_raw_write;
    BufferTextWidthMeasured = &observe_writer_text_width;
    install(0xF0F0F0F0U, 1, true);
    write_at(expected, 16 + 0x52C, default_font);   // the rebind is stored
    g_tw_clear_font_of = buffer;
    g_tw_result = 30;
    g_raw_write_result = 0;
    RECT rebind_rect = {10, 20, 110, 61};
    expect(buffer_write_cent_l_rect_redirect(buffer, nullptr, text,
                                             &rebind_rect, 11) == 0);
    g_tw_clear_font_of = nullptr;
    FontDefaultPtr = saved_default_ptr;
    expect(g_raw_write_calls == 1);
    expect(g_raw_write.x == 45);
    expect(g_raw_write.y == 36);          // (61 - 9 - 20) / 2 + 20
    expect_storage_bytes(storage, expected, sizeof(storage));

    BufferTextWidthMeasured = saved_width;
    BufferWriteMultiFontRawL = saved_raw;
}

void test_buffer_copy_overload() {
    // The five-argument copy exists only to hand the destination coordinates
    // over a second time as the source coordinates. Distinct values in every
    // position are what makes a shuffled argument visible.
    auto *const saved = BufferCopyFull;
    BufferCopyFull = &observe_buffer_copy_full;

    std::vector<uint8_t> bs(sizeof(Buffer), 0);
    std::vector<uint8_t> bexp(bs.size());
    auto *buf = reinterpret_cast<Buffer *>(bs.data());
    seed_storage(bs.data(), bexp.data(), bs.size());
    std::memcpy(bexp.data(), bs.data(), bs.size());

    auto other = reinterpret_cast<Buffer *>(0x11223344);
    g_copy_calls = 0; g_copy_result = 0x5A5A;
    expect(buf->copy(other, 11, 22, 33, 44) == 0x5A5A);
    expect(g_copy_calls == 1);
    expect(g_copy.self == reinterpret_cast<void *>(buf));
    expect(g_copy.src == other);
    expect(g_copy.x == 11 && g_copy.y == 22);
    expect(g_copy.sx == 11 && g_copy.sy == 22);   // the coordinates repeat
    expect(g_copy.w == 33 && g_copy.h == 44);
    // Negative extents must survive; PlanWin::UNK1 relies on passing one.
    g_copy_calls = 0;
    expect(buffer_copy_redirect(buf, nullptr, other, -1, -2, -3, -4) == 0x5A5A);
    expect(g_copy_calls == 1);
    expect(g_copy.x == -1 && g_copy.sx == -1);
    expect(g_copy.y == -2 && g_copy.sy == -2);
    expect(g_copy.w == -3 && g_copy.h == -4);
    // The overload is a pure shuffle - it must not touch the buffer.
    expect_storage_bytes(bs.data(), bexp.data(), bs.size());

    // The rectangle overload derives the extents instead of taking them, and
    // the rectangle is half-open: right-left, not right-left+1. Sides are
    // chosen so a swapped corner or an inclusive span shows up as a distinct
    // number rather than coinciding.
    RECT rect = {100, 200, 130, 250};   // left, top, right, bottom
    g_copy_calls = 0;
    expect(buf->copy(other, &rect) == 0x5A5A);
    expect(g_copy_calls == 1);
    expect(g_copy.self == reinterpret_cast<void *>(buf));
    expect(g_copy.src == other);
    expect(g_copy.x == 100 && g_copy.y == 200);
    expect(g_copy.sx == 100 && g_copy.sy == 200);   // the corner repeats
    expect(g_copy.w == 30 && g_copy.h == 50);       // half-open extents
    // The rectangle is read, never written.
    expect(rect.left == 100 && rect.top == 200);
    expect(rect.right == 130 && rect.bottom == 250);
    g_copy_calls = 0;
    expect(buffer_copy_rect_redirect(buf, nullptr, other, &rect) == 0x5A5A);
    expect(g_copy_calls == 1 && g_copy.w == 30 && g_copy.h == 50);
    expect_storage_bytes(bs.data(), bexp.data(), bs.size());

    BufferCopyFull = saved;
}

void test_plan_win_blink_and_unk1() {
    std::vector<uint8_t> pw(sizeof(PlanWin) + 64, 0);
    uint8_t *const obj = pw.data();
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v = 0; std::memcpy(&v, obj + off, 4); return v;
    };
    const int32_t vbtable[2] = {0, 0x22050};
    set32(0, reinterpret_cast<uintptr_t>(&vbtable[0]));
    auto *plan = reinterpret_cast<PlanWin *>(obj);

    // blink dispatches slot 0x30 of the virtual base's own vtable, so the
    // table is planted on the base and the neighbouring slots are poisoned:
    // an off-by-one slot would jump to a null and fault rather than pass.
    void *vt[16] = {};
    vt[0x30 / sizeof(void *)] = reinterpret_cast<void *>(&observe_pw_blink_slot);
    set32(0x22050, reinterpret_cast<uintptr_t>(&vt[0]));

    // Blinking off: no toggle, no redraw.
    set32(0x21A68, 0);
    set32(0x21A6C, 7);
    g_pw_slot_calls = 0;
    plan->blink();
    expect(g_pw_slot_calls == 0);
    expect(get32(0x21A6C) == 7);

    // On, phase 0 -> 1, and the redraw runs against the virtual base.
    set32(0x21A68, 1);
    set32(0x21A6C, 0);
    plan->blink();
    expect(get32(0x21A6C) == 1);
    expect(g_pw_slot_calls == 1);
    expect(g_pw_slot_self == reinterpret_cast<void *>(obj + 0x22050));

    // 1 -> 0, and any other non-zero phase also lands on 0 rather than
    // flipping a bit or decrementing.
    plan->blink();
    expect(get32(0x21A6C) == 0);
    set32(0x21A6C, 5);
    plan_win_blink_redirect(plan, nullptr);
    expect(get32(0x21A6C) == 0);
    expect(g_pw_slot_calls == 3);
    expect(get32(0x21A68) == 1);   // the enable flag is never written

    // UNK1 blits the window's own buffer from PlanWin's, at the size the
    // window buffer records - with the height negated.
    auto *const saved_copy = BufferCopyFull;
    BufferCopyFull = &observe_buffer_copy_full;
    set32(0x22050 + 0x4C4, 640);
    set32(0x22050 + 0x4C8, 480);
    g_copy_calls = 0; g_copy_result = 0;
    plan->UNK1();
    expect(g_copy_calls == 1);
    // Destination is the *window's* buffer subobject, source is PlanWin's own.
    expect(g_copy.self == reinterpret_cast<void *>(obj + 0x22050 + 0x444));
    expect(g_copy.src == reinterpret_cast<Buffer *>(obj + 0x21A70));
    expect(g_copy.x == 0 && g_copy.y == 0);
    expect(g_copy.w == 640);
    expect(g_copy.h == -480);      // negated, as the original's `neg` does
    g_copy_calls = 0;
    plan_win_unk1_redirect(plan, nullptr);
    expect(g_copy_calls == 1 && g_copy.w == 640 && g_copy.h == -480);
    BufferCopyFull = saved_copy;
}

namespace {

// The redraw handler can retarget the button, which is how the test pins that
// set() re-reads the id and the parent link rather than caching them.
uint8_t *g_bb_obj;

int g_bb_refresh_calls;

uint32_t g_bb_retarget_id;

void *g_bb_retarget_parent;

bool g_bb_retarget;

void __thiscall observe_bb_refresh(void *self) {
    ++g_bb_refresh_calls;
    g_bb_obj = reinterpret_cast<uint8_t *>(self);
    if (g_bb_retarget) {
        std::memcpy(g_bb_obj + 0xA78, &g_bb_retarget_id, 4);
        std::memcpy(g_bb_obj + 0xC4, &g_bb_retarget_parent, 4);
    }
}

void *g_bb_notify_self;

int g_bb_notify_calls;

int g_bb_notify_id;

int g_bb_notify_value;

void __thiscall observe_bb_notify(void *self, int id, int value) {
    g_bb_notify_self = self; g_bb_notify_id = id; g_bb_notify_value = value;
    ++g_bb_notify_calls;
}

}  // namespace

void test_base_button_set() {
    // set() stores the new value, dispatches slot 0xF8 of its own live vtable,
    // then tells its parent through slot 0xB4 of the *parent's* vtable. Both
    // tables are planted with their neighbours left null so an off-by-one slot
    // faults rather than quietly passing.
    std::vector<uint8_t> bb(sizeof(BaseButton) + 64, 0);
    uint8_t *const obj = bb.data();
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v = 0; std::memcpy(&v, obj + off, 4); return v;
    };
    auto *button = reinterpret_cast<BaseButton *>(obj);

    void *own_vt[64] = {};
    own_vt[0xF8 / sizeof(void *)] = reinterpret_cast<void *>(&observe_bb_refresh);
    set32(0, reinterpret_cast<uintptr_t>(&own_vt[0]));

    void *parent_vt[64] = {};
    parent_vt[0xB4 / sizeof(void *)] = reinterpret_cast<void *>(&observe_bb_notify);
    std::vector<uint8_t> parent(64, 0);
    const uintptr_t pvt = reinterpret_cast<uintptr_t>(&parent_vt[0]);
    std::memcpy(parent.data(), &pvt, 4);

    g_bb_retarget = false;

    // An unchanged value is inert: no store, no redraw, no notification.
    set32(0xA18, 42);
    set32(0xA78, 7);
    set32(0xC4, reinterpret_cast<uintptr_t>(parent.data()));
    g_bb_refresh_calls = 0; g_bb_notify_calls = 0;
    button->set(42);
    expect(g_bb_refresh_calls == 0 && g_bb_notify_calls == 0);
    expect(get32(0xA18) == 42);

    // A changed value stores, redraws, and notifies with (id, value).
    button->set(99);
    expect(get32(0xA18) == 99);
    expect(g_bb_refresh_calls == 1);
    expect(g_bb_obj == obj);                       // dispatched on the button
    expect(g_bb_notify_calls == 1);
    expect(g_bb_notify_self == reinterpret_cast<void *>(parent.data()));
    expect(g_bb_notify_id == 7);                   // the id, not the value
    expect(g_bb_notify_value == 99);

    // A parentless button still stores and redraws, but tells nobody.
    set32(0xC4, 0);
    g_bb_refresh_calls = 0; g_bb_notify_calls = 0;
    button->set(123);
    expect(get32(0xA18) == 123);
    expect(g_bb_refresh_calls == 1 && g_bb_notify_calls == 0);

    // Ordering: the redraw handler retargets the button, and the notification
    // must use the *new* id and the *new* parent - proving neither was read
    // before the dispatch.
    std::vector<uint8_t> parent2(64, 0);
    std::memcpy(parent2.data(), &pvt, 4);
    g_bb_retarget = true;
    g_bb_retarget_id = 555;
    // `parent2` dies at the closing brace; restored at the end of the case.
    void *const saved_retarget_parent = g_bb_retarget_parent;
    g_bb_retarget_parent = parent2.data();
    set32(0xA78, 7);
    set32(0xC4, reinterpret_cast<uintptr_t>(parent.data()));
    g_bb_notify_calls = 0;
    button->set(1000);
    expect(g_bb_notify_calls == 1);
    expect(g_bb_notify_id == 555);                 // re-read after the redraw
    expect(g_bb_notify_self == reinterpret_cast<void *>(parent2.data()));
    g_bb_retarget = false;

    // The redirect drives the identical path.
    set32(0xA78, 7);
    set32(0xC4, reinterpret_cast<uintptr_t>(parent.data()));
    g_bb_refresh_calls = 0; g_bb_notify_calls = 0;
    base_button_set_redirect(button, nullptr, 2000);
    expect(get32(0xA18) == 2000);
    expect(g_bb_refresh_calls == 1 && g_bb_notify_calls == 1);
    expect(g_bb_notify_id == 7 && g_bb_notify_value == 2000);
    // Nothing past the object was disturbed.
    expect(get32(sizeof(BaseButton)) == 0);
    g_bb_retarget_parent = saved_retarget_parent;
}

void *g_tex_freed;

int g_tex_free_calls;

void *observe_texture_free(void *p) { g_tex_freed = p; ++g_tex_free_calls; return nullptr; }

void test_texture_lifecycle() {
    // The constructor and close agree on four fields - pixels at 0, two
    // descriptors at 4 and 8, a borrowed flag at 0x6C - and touch nothing
    // else. close frees the pixels only when there are pixels AND the flag is
    // clear; a borrowed texture keeps its block and even keeps its pointer,
    // which is the asymmetry worth pinning.
    auto *const saved = TextureFree;
    TextureFree = &observe_texture_free;
    g_tex_free_calls = 0;

    std::vector<uint8_t> ts(sizeof(Texture) + 32);
    std::vector<uint8_t> texp(ts.size());
    uint8_t *const obj = ts.data();
    auto read32 = [&](size_t off) {
        uint32_t v = 0; std::memcpy(&v, obj + off, 4); return v;
    };
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };

    // Construction zeroes exactly those four and leaves the rest alone.
    seed_storage(ts.data(), texp.data(), ts.size());
    std::memcpy(texp.data(), ts.data(), ts.size());
    // Construction is driven through the redirect rather than a placement new
    // here: the constructor lives in another translation unit, so the compiler
    // cannot fold its stores against the memcpy writes this test makes into
    // the same storage afterwards.
    auto *tex = reinterpret_cast<Texture *>(obj);
    expect(texture_ctor_redirect(tex, nullptr) == tex);
    expect(read32(0) == 0 && read32(4) == 0 && read32(8) == 0);
    expect(read32(0x6C) == 0);
    // Everything between 0xC and 0x6B is untouched by construction.
    for (size_t off = 0xC; off < 0x6C; ++off) {
        expect(obj[off] == texp[off]);
    }

    // Owned pixels: freed, and the pointer cleared.
    int block = 0;
    void *pixels = &block;
    std::memcpy(obj, &pixels, sizeof(pixels));
    set32(4, 0x1111); set32(8, 0x2222); set32(0x6C, 0);
    g_tex_free_calls = 0;
    tex->close();
    expect(g_tex_free_calls == 1 && g_tex_freed == pixels);
    expect(read32(0) == 0 && read32(4) == 0 && read32(8) == 0 && read32(0x6C) == 0);

    // Borrowed pixels: not freed, and the pointer is deliberately left set.
    std::memcpy(obj, &pixels, sizeof(pixels));
    set32(4, 0x1111); set32(8, 0x2222); set32(0x6C, 1);
    g_tex_free_calls = 0;
    tex->close();
    expect(g_tex_free_calls == 0);
    void *still = nullptr;
    std::memcpy(&still, obj, sizeof(still));
    expect(still == pixels);                 // kept, not cleared
    expect(read32(4) == 0 && read32(8) == 0 && read32(0x6C) == 0);

    // No pixels at all: nothing freed, and the descriptors still clear.
    void *none = nullptr;
    std::memcpy(obj, &none, sizeof(none));
    set32(4, 0x3333); set32(0x6C, 0);
    g_tex_free_calls = 0;
    tex->close();
    expect(g_tex_free_calls == 0 && read32(4) == 0);

    // The constructor answers `this`, as MSVC constructors do.
    std::memcpy(obj, &pixels, sizeof(pixels));
    expect(texture_ctor_redirect(tex, nullptr) == tex);
    expect(read32(0) == 0);
    std::memcpy(obj, &pixels, sizeof(pixels));
    set32(0x6C, 0);
    g_tex_free_calls = 0;
    texture_close_redirect(tex, nullptr);
    expect(g_tex_free_calls == 1);

    TextureFree = saved;

    // TextureStore's destructor leaves 3 at 0 and clears 4, calling nothing.
    std::vector<uint8_t> st(sizeof(TextureStore) + 32);
    std::vector<uint8_t> sexp(st.size());
    seed_storage(st.data(), sexp.data(), st.size());
    std::memcpy(sexp.data(), st.data(), st.size());
    auto *store = reinterpret_cast<TextureStore *>(st.data());
    store->~TextureStore();
    uint32_t f0 = 0, f4 = 0;
    std::memcpy(&f0, st.data(), 4);
    std::memcpy(&f4, st.data() + 4, 4);
    expect(f0 == 3 && f4 == 0);
    // Nothing past the two fields moved.
    for (size_t off = 8; off < st.size(); ++off) {
        expect(st[off] == sexp[off]);
    }
    texture_store_dtor_redirect(store, nullptr);
    std::memcpy(&f0, st.data(), 4);
    expect(f0 == 3);
}

void test_base_pop_default_colors() {
    // Two interleaved tables with different geometry: the string table has
    // four tiers so its slots are 0x10 apart, the button table three at 0xC.
    // Both are compared whole after every call, because the failure mode a
    // wrong stride produces is writing a sibling slot rather than a sibling
    // tier - which a check of only the four intended cells would miss.
    uint32_t string_table[16];
    uint32_t button_table[12];
    uint32_t *const saved_string = BasePopDefaultStringColors;
    uint32_t *const saved_button = BasePopDefaultButtonColors;
    BasePopDefaultStringColors = string_table;
    BasePopDefaultButtonColors = button_table;

    const int colors[4] = {INT_MIN, -1, 0x5A5A5A5A, INT_MAX};

    struct TableCase {
        uint32_t *table;
        size_t cells;
        size_t stride;
        size_t tier;
        void (*member)(int, int, int, int);
        void (__cdecl *redirect)(int, int, int, int);
    };
    const TableCase cases[] = {
        {string_table, 16, 0x10, 0, &BasePop::set_def_string_color,
         base_pop_set_def_string_color_redirect},
        {string_table, 16, 0x10, 1, &BasePop::set_def_string_color2,
         base_pop_set_def_string_color2_redirect},
        {string_table, 16, 0x10, 2, &BasePop::set_def_string_color3,
         base_pop_set_def_string_color3_redirect},
        {string_table, 16, 0x10, 3, &BasePop::set_def_string_color_hyper,
         base_pop_set_def_string_color_hyper_redirect},
        {button_table, 12, 0x0C, 0, &BasePop::set_def_button_color,
         base_pop_set_def_button_color_redirect},
        {button_table, 12, 0x0C, 1, &BasePop::set_def_button_color2,
         base_pop_set_def_button_color2_redirect},
        {button_table, 12, 0x0C, 2, &BasePop::set_def_button_color3,
         base_pop_set_def_button_color3_redirect},
    };
    for (const TableCase &test : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            uint32_t expected[16];
            for (size_t index = 0; index < test.cells; ++index) {
                test.table[index] = 0xA5000000U ^ static_cast<uint32_t>(index);
                expected[index] = test.table[index];
            }
            for (size_t slot = 0; slot < 4; ++slot) {
                expected[(slot * test.stride + test.tier * 4) / 4] =
                    static_cast<uint32_t>(colors[slot]);
            }
            if (adapter) {
                test.redirect(colors[0], colors[1], colors[2], colors[3]);
            } else {
                test.member(colors[0], colors[1], colors[2], colors[3]);
            }
            for (size_t index = 0; index < test.cells; ++index) {
                expect(test.table[index] == expected[index]);
            }
        }
    }

    BasePopDefaultButtonColors = saved_button;
    BasePopDefaultStringColors = saved_string;
}

void test_win_set_def_focus() {
    int focus = 0x5A5A5A5A;
    int *const saved = WinDefaultFocus;
    WinDefaultFocus = &focus;
    const int values[] = {0, 1, -1, INT_MIN, INT_MAX};
    for (int value : values) {
        Win::set_def_focus(value);
        expect(focus == value);
        win_set_def_focus_redirect(~value);
        expect(focus == ~value);
    }
    WinDefaultFocus = saved;
}

void test_base_pop_instance_colors() {
    // The two families have different storage: string slots are four dwords
    // at 0x3120 with a 0x10 stride, while button slot one is a *byte* at
    // 0x3178 + tier and the remaining three are dwords at 0x317C stride 0xC.
    // A dword written to that byte would overwrite the neighbouring tiers, so
    // the whole object is compared after every call.
    std::vector<uint8_t> bytes(sizeof(BasePop) + 32);
    std::vector<uint8_t> expected_bytes(sizeof(BasePop) + 32);
    uint8_t *const storage = bytes.data();
    uint8_t *const expected = expected_bytes.data();
    auto *popup = reinterpret_cast<BasePop *>(storage + 16);

    const int colors[4] = {INT_MIN, -1, 0x5A5A5A5A, INT_MAX};

    struct StringCase {
        size_t tier;
        void (BasePop::*member)(int, int, int, int);
        void (__fastcall *redirect)(BasePop *, void *, int, int, int, int);
    };
    const StringCase strings[] = {
        {0, &BasePop::set_string_color, base_pop_set_string_color_redirect},
        {1, &BasePop::set_string_color2, base_pop_set_string_color2_redirect},
        {2, &BasePop::set_string_color3, base_pop_set_string_color3_redirect},
        {3, &BasePop::set_string_color_hyper,
         base_pop_set_string_color_hyper_redirect},
    };
    for (const StringCase &test : strings) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, bytes.size());
            std::memcpy(expected, storage, bytes.size());
            for (size_t slot = 0; slot < 4; ++slot) {
                write_at(expected, 16 + 0x3120 + slot * 0x10 + test.tier * 4,
                         colors[slot]);
            }
            if (adapter) {
                test.redirect(popup, nullptr,
                              colors[0], colors[1], colors[2], colors[3]);
            } else {
                (popup->*test.member)(
                    colors[0], colors[1], colors[2], colors[3]);
            }
            expect_storage_bytes(storage, expected, bytes.size());
        }
    }

    const StringCase buttons[] = {
        {0, &BasePop::set_button_color, base_pop_set_button_color_redirect},
        {1, &BasePop::set_button_color2, base_pop_set_button_color2_redirect},
        {2, &BasePop::set_button_color3, base_pop_set_button_color3_redirect},
    };
    for (const StringCase &test : buttons) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, bytes.size());
            std::memcpy(expected, storage, bytes.size());
            // Slot one is a single byte; the truncation is part of the
            // contract, not an accident of the fixture's values.
            expected[16 + 0x3178 + test.tier] =
                static_cast<uint8_t>(colors[0]);
            for (size_t slot = 1; slot < 4; ++slot) {
                write_at(expected,
                         16 + 0x317C + (slot - 1) * 0xC + test.tier * 4,
                         colors[slot]);
            }
            if (adapter) {
                test.redirect(popup, nullptr,
                              colors[0], colors[1], colors[2], colors[3]);
            } else {
                (popup->*test.member)(
                    colors[0], colors[1], colors[2], colors[3]);
            }
            expect_storage_bytes(storage, expected, bytes.size());
        }
    }
}

void test_guarded_store_recoveries() {
    // Win::UNK8/UNK9 publish into two optional scrollbars, each tested
    // separately, so an attached bar is still updated when the other is null.
    std::vector<uint8_t> win_bytes(sizeof(Win) + 32);
    std::vector<uint8_t> win_expected(sizeof(Win) + 32);
    uint8_t *const ws = win_bytes.data();
    uint8_t *const we = win_expected.data();
    auto *win = reinterpret_cast<Win *>(ws + 16);
    std::vector<uint8_t> vert(sizeof(Scroll));
    std::vector<uint8_t> horz(sizeof(Scroll));
    auto *scroll_v = reinterpret_cast<Scroll *>(vert.data());
    auto *scroll_h = reinterpret_cast<Scroll *>(horz.data());

    struct SlotCase { size_t slot; void (Win::*member)(int);
                      void (__fastcall *redirect)(Win *, void *, int); };
    const SlotCase slots[] = {
        {0x2144, &Win::UNK8, win_unk8_redirect},
        {0x2148, &Win::UNK9, win_unk9_redirect},
    };
    // Both attached, only vertical, only horizontal, neither.
    const int attach[][2] = {{1, 1}, {1, 0}, {0, 1}, {0, 0}};
    for (const SlotCase &test : slots) {
        for (const auto &which : attach) {
            for (int adapter = 0; adapter < 2; ++adapter) {
                seed_storage(ws, we, win_bytes.size());
                std::memset(vert.data(), 0xA5, vert.size());
                std::memset(horz.data(), 0xA5, horz.size());
                write_at(ws, 16 + 0x43C,
                         which[0] ? scroll_v : static_cast<Scroll *>(nullptr));
                write_at(ws, 16 + 0x440,
                         which[1] ? scroll_h : static_cast<Scroll *>(nullptr));
                std::memcpy(we, ws, win_bytes.size());
                const int value = 0x1234ABCD;
                if (adapter) {
                    test.redirect(win, nullptr, value);
                } else {
                    (win->*test.member)(value);
                }
                // The window itself is never modified.
                expect_storage_bytes(ws, we, win_bytes.size());
                uint32_t stored;
                if (which[0]) {
                    std::memcpy(&stored, vert.data() + test.slot, sizeof(stored));
                    expect(stored == 0x1234ABCDU);
                }
                if (which[1]) {
                    std::memcpy(&stored, horz.data() + test.slot, sizeof(stored));
                    expect(stored == 0x1234ABCDU);
                }
            }
        }
    }

    // Menu::requested_height doubles the resolved font's height, preferring
    // the menu's own font and falling back to the process default.
    alignas(Font) uint8_t menu_font[sizeof(Font)];
    alignas(Font) uint8_t default_font[sizeof(Font)];
    std::memset(menu_font, 0xA5, sizeof(menu_font));
    std::memset(default_font, 0xA5, sizeof(default_font));
    write_at(menu_font, 0x10, 21);
    write_at(default_font, 0x10, -3);
    Font *menu_slot = reinterpret_cast<Font *>(menu_font);
    Font *default_slot = reinterpret_cast<Font *>(default_font);
    Font **const saved_menu = MenuFont;
    Font **const saved_default = FontDefaultPtr;
    MenuFont = &menu_slot;
    FontDefaultPtr = &default_slot;
    alignas(Menu) uint8_t menu_storage[sizeof(Menu) + 32];
    uint8_t menu_expected[sizeof(menu_storage)];
    auto *menu = reinterpret_cast<Menu *>(menu_storage + 16);
    seed_storage(menu_storage, menu_expected, sizeof(menu_storage));
    std::memcpy(menu_expected, menu_storage, sizeof(menu_storage));
    expect(menu->requested_height() == 42);
    expect(menu_requested_height_redirect(menu, nullptr) == 42);
    // With no menu font the default is resolved instead; the doubling is
    // signed, so a negative height doubles rather than saturating.
    menu_slot = nullptr;
    expect(menu->requested_height() == -6);
    expect_storage_bytes(menu_storage, menu_expected, sizeof(menu_storage));
    FontDefaultPtr = saved_default;
    MenuFont = saved_menu;

    // BasePop::fallout raises its flag only while the gate is set.
    int gate = 0;
    int flag = 0;
    int *const saved_gate = BasePopFalloutGate;
    int *const saved_flag = BasePopFalloutFlag;
    BasePopFalloutGate = &gate;
    BasePopFalloutFlag = &flag;
    BasePop::fallout();
    expect(flag == 0);
    gate = 1;
    BasePop::fallout();
    expect(flag == 1);
    flag = 0;
    gate = -1;   // any nonzero gate, not just one
    base_pop_fallout_redirect();
    expect(flag == 1);
    BasePopFalloutFlag = saved_flag;
    BasePopFalloutGate = saved_gate;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(177, test_buffer_box);
LEAF_CASE(178, test_buffer_text_writers);
LEAF_CASE(176, test_buffer_copy_overload);
LEAF_CASE(179, test_plan_win_blink_and_unk1);
LEAF_CASE(180, test_base_button_set);
LEAF_CASE(181, test_texture_lifecycle);
LEAF_CASE(117, test_base_pop_default_colors);
LEAF_CASE(118, test_win_set_def_focus);
LEAF_CASE(119, test_base_pop_instance_colors);
LEAF_CASE(120, test_guarded_store_recoveries);
}  // namespace
