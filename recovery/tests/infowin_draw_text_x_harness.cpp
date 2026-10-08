// Host-only fixture for the defined cross-row x carryover. The zero-budget
// original pointer is intercepted by host_strlen before dereference; this does
// not validate that invalid native pointer or the entry-zero uninitialized-x
// path.
#include <assert.h>
#include <stdint.h>

struct RECT { int left, top, right, bottom; };
static char *last_prefix;
static int one_before_prefix_strlen_count;
static int zero_tail_x;
static int write_calls;

static unsigned long address(const void *p) {
    return (unsigned long)(uintptr_t)p;
}

static int infowin_host_strlen(const char *p) {
    if (last_prefix && address(p) + 1 == address(last_prefix)) {
        ++one_before_prefix_strlen_count;
        return 0;
    }
    int n = 0;
    while (p[n]) ++n;
    return n;
}

static char *infowin_host_strcpy(char *to, const char *from) {
    last_prefix = to;
    char *result = to;
    while ((*to++ = *from++) != 0) {}
    return result;
}

static int InfoFactionTextColors[8];
static int InfoFactionShadowColors[8];
static int InfoTextColors[4][2];
struct GameDataFields { int faction_rankings_[9]; };
struct GameDataState { GameDataFields fields_; };
static GameDataState GameDataStateStorage;
struct Tutorial { void draw_arrow() {} };
static Tutorial TutorialWindow;

struct Buffer {
    void set_font(void *, int, int, int) {}
    void set_clip(RECT *) {}
    void restore_clip() {}
    void update(RECT *, int) {}
    int text_width(const char *) { return 0; }
    void set_text_color(int, int, int, int) {}
    void set_text_color2(int, int, int, int) {}
    int write_right_l(char *, int, int, int, int) { return 0; }
    int write_l(char *text, int x, int, int) {
        ++write_calls;
        if (last_prefix && address(text) + 1 == address(last_prefix))
            zero_tail_x = x;
        // The first positive-budget prefix advances x to 321. Other returns
        // are immaterial; the final-character path does not assign them.
        return x == 55 ? 321 : x;
    }
    void reset_text_state() {}
};
struct MainInterface : Buffer {};
static MainInterface MainInterfaceGlobal;

struct InfoWin {
    void *font_;
    int row_count_[2], visible_characters_[2], pane_states_[2];
    RECT pane_rects_[2], bounds_;
    char *row_text_[2][2];
    int row_color_[2][2], row_reveal_color_[2][2];
    int row_y_[2][2], row_x_[2][2], field_94_;
    void draw_text(int);
};

#define strlen infowin_host_strlen
#define strcpy infowin_host_strcpy
/*ACTUAL_SOURCE*/
#undef strlen
#undef strcpy

int main() {
    char first[] = "ab";
    char second[] = "cd";
    InfoWin win = {};
    win.row_count_[0] = 2;
    win.visible_characters_[0] = 2;
    win.pane_rects_[0].left = 0;
    win.pane_rects_[0].top = 0;
    win.pane_rects_[0].right = 100;
    win.pane_rects_[0].bottom = 40;
    win.row_text_[0][0] = first;
    win.row_text_[0][1] = second;
    win.row_color_[0][0] = win.row_color_[0][1] = 1;
    win.row_x_[0][0] = 55;
    win.row_y_[0][0] = 10;
    win.row_y_[0][1] = 20;

    win.draw_text(0);

    assert(one_before_prefix_strlen_count == 1);
    assert(zero_tail_x == 321);
    assert(write_calls == 3);
    return 0;
}
