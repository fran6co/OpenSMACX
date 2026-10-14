// The live painter is inserted below; these doubles establish observable
// formatting/dataflow, not the real GUI, member layout or Net name capacity.
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct Font {};
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
// LIVE_PLAYER
struct Net {
    NetPlayer players_[16];
    unsigned int local_player_id_, host_player_id_;
};
struct Buffer {
    void set_font(Font *, int, int, int);
    void set_text_color(int, int, int, int);
    int write_l(char *, int, int, int);
    int text_line_height();
    int buffer_identity;
};
struct Win {
    virtual void on_redraw() { assert(false); }
    int win_identity;
};
struct GraphicWin : Win, Buffer { void fill(int); };
struct MultiDebug : GraphicWin {
    Font font_;
    Net *net_;
    virtual void on_redraw();
};
MultiDebug window;
Net sessions[17];
struct Draw { char text[128]; int x, y; } expected[16];
int expected_count, expected_heights, draws, heights, setup_stage;
int height_after_draw[14];

int height_value(int index) { return 5 + index * 2; }
int prefix_result(int draw_index) { return 100 + draw_index * 7; }
void expect(const char *text, int x, int y) {
    assert(expected_count < 16);
    strcpy(expected[expected_count].text, text);
    expected[expected_count].x = x;
    expected[expected_count++].y = y;
}
int expect_height() {
    height_after_draw[expected_heights] = expected_count;
    return height_value(expected_heights++);
}
void Buffer::set_font(Font *font, int a, int b, int c) {
    assert(this == static_cast<Buffer *>(&window));
    assert(font == &window.font_ && !a && !b && !c && setup_stage++ == 0);
}
void Buffer::set_text_color(int color, int background, int a, int b) {
    assert(this == static_cast<Buffer *>(&window));
    assert(color == 255 && background == -1 && a == 1 && b == 1 && setup_stage++ == 1);
}
void GraphicWin::fill(int color) {
    assert(this == static_cast<GraphicWin *>(&window));
    assert(color == 0 && setup_stage++ == 2);
}
int Buffer::write_l(char *text, int x, int y, int length) {
    assert(this == static_cast<Buffer *>(&window) && setup_stage == 3);
    assert(draws < expected_count);
    assert(length == static_cast<int>(strlen(text)));
    assert(strcmp(text, expected[draws].text) == 0);
    assert(x == expected[draws].x && y == expected[draws].y);
    int result = prefix_result(draws);
    ++draws;
    window.net_ = &sessions[draws];
    return result;
}
int Buffer::text_line_height() {
    assert(this == static_cast<Buffer *>(&window));
    assert(heights < expected_heights && draws == height_after_draw[heights]);
    return height_value(heights++);
}

// LIVE_PAINT

void prepare(int empty) {
    memset(sessions, 0, sizeof(sessions));
    draws = heights = setup_stage = expected_count = expected_heights = 0;
    window.net_ = &sessions[0];
    for (int s = 0; s < 17; ++s) {
        sessions[s].host_player_id_ = static_cast<unsigned int>(-1 - s);
        sessions[s].local_player_id_ = static_cast<unsigned int>(-100 - s);
        for (int player = 0; player < 16; ++player) {
            NetPlayer &p = sessions[s].players_[player];
            // Later draws can introduce the final slot. All intermediate
            // slots remain absent even though their other fields are nonzero.
            p.field_0_ = !empty && (player == 0 || (player == 15 && s >= 9));
            p.field_4_ = static_cast<unsigned int>(-1000 - player * 10 - s);
            p.field_8_ = static_cast<unsigned int>(-1100 - player * 10 - s);
            p.field_C_ = static_cast<unsigned int>(-1200 - player * 10 - s);
            p.stamp_10_ = static_cast<unsigned int>(-1300 - player * 10 - s);
            p.flags_14_ = static_cast<unsigned char>(200 + s);
            sprintf(p.name_and_tail_, "name%d-slot%d", s, player);
        }
    }

    expect("Host ID: -1", 3, 3);
    int y = 3 + expect_height();
    expect("Local ID: -101", 3, y);
    y += (expect_height() * 3) / 2;
    if (!empty) {
        const char *labels[5] = {"Guaranteed count: ", "Sending: ",
            "Average reply time: ", "Last time: ", "Flags: "};
        for (int row = 0; row < 2; ++row) {
            int player = row ? 15 : 0;
            char text[128];
            sprintf(text, "%d.  ", player); // The original has TWO spaces.
            int value_x = prefix_result(expected_count);
            expect(text, 3, y);
            sprintf(text, "name%d-slot%d", expected_count, player);
            expect(text, value_x, y);
            y += expect_height();
            for (int field = 0; field < 5; ++field) {
                int value = field == 4 ? 200 + expected_count
                    : -1000 - field * 100 - player * 10 - expected_count;
                sprintf(text, "%s%d", labels[field], value);
                expect(text, value_x, y);
                y += expect_height();
            }
        }
    }
}
int main() {
    for (int empty = 0; empty != 2; ++empty) {
        prepare(empty);
        Win *base = &window;
        base->on_redraw();
        assert(draws == (empty ? 2 : 16));
        assert(heights == (empty ? 2 : 14));
        assert(draws == expected_count && heights == expected_heights && setup_stage == 3);
        assert(window.net_ == &sessions[draws]);
    }
    return 0;
}
