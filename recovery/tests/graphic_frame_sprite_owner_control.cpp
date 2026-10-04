#include <assert.h>
#include <limits.h>
#include <string.h>
class Sprite {
public:
    int width_value, height_value;
    int width() const { return width_value; }
    int height() const { return height_value; }
};
// ACTUAL_SOURCE_TYPES
class GraphicWin {
public:
    Sprite *frame_top_left_, *frame_top_right_;
    Sprite *frame_bottom_left_, *frame_bottom_right_;
    Sprite *frame_top_, *frame_left_, *frame_right_, *frame_bottom_;
    Sprite *frame_noncaption_left_, *frame_noncaption_right_, *frame_noncaption_middle_;
    unsigned char iFlags_;
    int field_134_, field_138_;
    void apply_frame_sprites(const NCSprites *sprites);
};
// ACTUAL_SOURCE_BODY
int main() {
    Sprite a = {20, 3}, b = {30, 5}, c = {40, 7}, d = {50, 11};
    Sprite e = {60, 13}, f = {70, 17}, g = {80, 19}, h = {90, 23};
    Sprite i = {100, 29}, j = {110, 31}, k = {120, 37};
    NCSprites sprites = {&a, &b, &c, &d, &e, &f, &g, &h, &i, &j, &k};
    GraphicWin w;
    memset(&w, 0, sizeof w);
    w.iFlags_ = 0x90;
    w.field_134_ = 777; w.field_138_ = 888;
    w.apply_frame_sprites(&sprites);
    assert(w.frame_top_left_ == &a && w.frame_top_right_ == &b);
    assert(w.frame_bottom_left_ == &c && w.frame_bottom_right_ == &d);
    assert(w.frame_top_ == &e && w.frame_left_ == &f);
    assert(w.frame_right_ == &g && w.frame_bottom_ == &h);
    assert(w.frame_noncaption_middle_ == &i);
    assert(w.frame_noncaption_left_ == &j && w.frame_noncaption_right_ == &k);
    assert(w.field_134_ == 90 && w.field_138_ == 16);
    w.iFlags_ = 0x80;
    w.apply_frame_sprites(&sprites);
    assert(w.field_134_ == 230 && w.field_138_ == 48);
    w.field_134_ = 777; w.field_138_ = 888;
    GraphicWin before = w;
    w.apply_frame_sprites(0);
    assert(memcmp(&before, &w, sizeof w) == 0);

    // Every selected corner is independently necessary. Input publication
    // still completes, but incomplete selected geometry preserves both minima.
    Sprite **corners[] = {&sprites.top_left, &sprites.top_right,
                         &sprites.bottom_left, &sprites.bottom_right};
    w.iFlags_ = 0x10;
    for (int n = 0; n != 4; ++n) {
        Sprite *saved = *corners[n];
        *corners[n] = 0;
        w.apply_frame_sprites(&sprites);
        assert(w.field_134_ == 777 && w.field_138_ == 888);
        assert(w.frame_noncaption_middle_ == &i);
        *corners[n] = saved;
    }
    w.iFlags_ = 0;
    sprites.top_left = 0; sprites.top_right = 0;
    w.apply_frame_sprites(&sprites);
    assert(w.field_134_ == 230 && w.field_138_ == 48);
    sprites.noncaption_left = 0;
    w.field_134_ = 777; w.field_138_ = 888;
    w.apply_frame_sprites(&sprites);
    assert(w.field_134_ == 777 && w.field_138_ == 888);

    // Native ADD wraps before signed maximum selection, independently by axis.
    sprites.top_left = &a; sprites.top_right = &b;
    sprites.noncaption_left = &j;
    w.iFlags_ = 0x10;
    a.width_value = INT_MAX; b.width_value = 1;
    c.width_value = d.width_value = 0;
    a.height_value = INT_MAX; c.height_value = 1;
    b.height_value = d.height_value = 0;
    w.apply_frame_sprites(&sprites);
    assert(w.field_134_ == 0 && w.field_138_ == 0);
    a.width_value = -20; b.width_value = -30;
    c.width_value = -40; d.width_value = -50;
    a.height_value = -3; b.height_value = -5;
    c.height_value = -7; d.height_value = -11;
    w.apply_frame_sprites(&sprites);
    assert(w.field_134_ == -50 && w.field_138_ == -10);
    return 0;
}
