// Actual Sprite declaration and Buffer bodies, with surface/draw doubles.
#include <assert.h>
#include <string.h>
#include <stdio.h>
typedef unsigned char uint8_t;
typedef void *LPVOID;
#define nullptr 0
#define MEASURED
struct RECT { int left, top, right, bottom; };
struct DDCOLORKEY { unsigned long dwColorSpaceLowValue, dwColorSpaceHighValue; };
enum { DDCKEY_SRCBLT = 8, DDBLT_WAIT = 0x1000000, DDBLT_KEYSRC = 0x8000 };
class Buffer;
class TexHeap;
// LIVE_COLOR_TABLE
#define private public
// LIVE_SPRITE
#undef private
Sprite::Sprite() { memset(this, 0, sizeof(*this)); }
void Sprite::close() {}
Sprite g_BUFFER_SPRITE;
// Behavior-only owner seam: both product accesses and existing oracles use
// this one Sprite. This double makes no allocation or lifetime-layout claim.
struct BufferSpriteOwnerDouble {
    Sprite &sprite() { return g_BUFFER_SPRITE; }
} BufferSpriteOwner;
const unsigned char WinFillColour = 201;
int key_calls, blits, unlocks, draws, whole_surface_blits, draws4;
struct Surface {
    DDCOLORKEY key;
    RECT rectangle;
    Surface *source;
    int SetColorKey(int flags, DDCOLORKEY *value) {
        assert(flags == DDCKEY_SRCBLT); key = *value; ++key_calls; return -11;
    }
    int Blt(RECT *area, Surface *from, RECT *source_area, int flags, void *effects) {
        assert(!source_area && !effects && flags == (DDBLT_WAIT | DDBLT_KEYSRC));
        if (area != 0) {
            rectangle = *area;
        } else {
            ++whole_surface_blits;
        }
        source = from; ++blits; return -12;
    }
    int Unlock(void *) { ++unlocks; return 0; }
};
class Buffer {
 public:
    Surface *surface_;
    void *locked_bits_, *dib_bits_;
    int surface_lock_count_, stride_;
    struct { struct { int biWidth, biHeight; } bmiHeader; } dib_;
    void setup_buff_sprite(int);
    void prepare_shared_sprite(int);
    static void release_buff_sprite();
    int draw(Buffer *, int, int, int, int, int);
    int draw(Buffer *, int, int, int);
};
Buffer *expected_destination;
unsigned char *expected_bits;
int expected_index, expected_colour, expected_stride, expected_width, expected_height;
int Sprite::draw(Buffer *destination, int index, int x, int y, int numerator, int denominator) {
    assert(this == &g_BUFFER_SPRITE && destination == expected_destination);
    assert(index == expected_index && x == 5 && y == 7 && numerator == 2 && denominator == 3);
    assert(pcBits_ == expected_bits && cTransparentIndex_ == expected_colour);
    assert(iSpriteWidth2_ == expected_stride && iSpriteWidth_ == expected_width);
    assert(iSpriteHeight_ == expected_height && iWidth_ == expected_width && iHeight_ == expected_height);
    assert(!iLeftOffset_ && !iTopOffset_ && !fObj1Exists_);
    ++draws;
    return 71;
}
int Sprite::draw(Buffer *destination, int index, int x, int y) {
    assert(this == &g_BUFFER_SPRITE && destination == expected_destination);
    assert(index == expected_index && x == 5 && y == 7);
    assert(pcBits_ == expected_bits && cTransparentIndex_ == expected_colour);
    assert(iSpriteWidth2_ == expected_stride && iSpriteWidth_ == expected_width);
    assert(iSpriteHeight_ == expected_height && iWidth_ == expected_width && iHeight_ == expected_height);
    assert(!iLeftOffset_ && !iTopOffset_ && !fObj1Exists_);
    ++draws4;
    return 73;
}
// LIVE_PREPARE
// LIVE_SETUP
// LIVE_RELEASE
// LIVE_DRAW
// LIVE_FOUR_ARGUMENT_DRAW

static void six_argument_control() {
    puts("CONTROL_OVERLOAD=6"); fflush(stdout);
    Buffer source, destination;
    memset(&source, 0, sizeof(source)); memset(&destination, 0, sizeof(destination));
    Surface from, to;
    source.surface_ = &from; destination.surface_ = &to;
    source.dib_.bmiHeader.biWidth = 12; source.dib_.bmiHeader.biHeight = -9;
    assert(source.draw(&destination, -1, 5, 7, 2, 3) == 3);
    assert(key_calls == 1 && blits == 1 && !draws);
    assert(from.key.dwColorSpaceLowValue == 0xFFFFFFFFUL && from.key.dwColorSpaceHighValue == 0xFFFFFFFFUL);
    assert(to.source == &from && to.rectangle.left == 5 && to.rectangle.top == 7);
    assert(to.rectangle.right == 13 && to.rectangle.bottom == 13);
    assert(source.draw(&destination, 260, 5, 7, -2, 3) == 3);
    assert(to.rectangle.right == -3 && to.rectangle.bottom == 1);
    assert(from.key.dwColorSpaceLowValue == 260);

    char pixels[32];
    expected_destination = &destination;
    expected_width = 12; expected_height = 9; expected_stride = 16;
    source.stride_ = 16; source.surface_ = 0;
    for (int mode = 0; mode < 8; ++mode) {
        source.dib_bits_ = mode & 1 ? pixels : 0;
        destination.surface_ = mode & 2 ? &to : 0;
        expected_bits = static_cast<unsigned char *>(source.dib_bits_);
        expected_index = mode & 4 ? -1 : 260;
        expected_colour = mode & 4 ? 201 : 4;
        int before = draws;
        assert(source.draw(&destination, expected_index, 5, 7, 2, 3) == 71);
        assert(draws == before + 1 && !g_BUFFER_SPRITE.pcBits_);
        assert(!source.locked_bits_ && !source.surface_lock_count_ && !unlocks);
    }
    // A surface source with a software destination leaves the shared
    // descriptor untouched until the normal-return borrowed-pointer release.
    source.surface_ = &from; destination.surface_ = 0;
    char retained_pixels[64], retained_lock[8];
    source.dib_bits_ = pixels;
    source.locked_bits_ = retained_lock;
    source.surface_lock_count_ = 3;
    g_BUFFER_SPRITE.pcBits_ = reinterpret_cast<unsigned char *>(retained_pixels);
    expected_bits = reinterpret_cast<unsigned char *>(retained_pixels);
    g_BUFFER_SPRITE.cTransparentIndex_ = expected_colour = 17;
    g_BUFFER_SPRITE.iSpriteWidth2_ = expected_stride = 40;
    g_BUFFER_SPRITE.iSpriteWidth_ = g_BUFFER_SPRITE.iWidth_ = expected_width = 23;
    g_BUFFER_SPRITE.iSpriteHeight_ = g_BUFFER_SPRITE.iHeight_ = expected_height = 19;
    Sprite retained_descriptor = g_BUFFER_SPRITE;
    expected_index = -1;
    assert(source.draw(&destination, -1, 5, 7, 2, 3) == 71);
    assert(!g_BUFFER_SPRITE.pcBits_);
    assert(source.locked_bits_ == retained_lock && source.surface_lock_count_ == 3);
    assert(!unlocks);
    retained_descriptor.pcBits_ = 0;
    assert(memcmp(&g_BUFFER_SPRITE, &retained_descriptor, sizeof(Sprite)) == 0);
    g_BUFFER_SPRITE.pcBits_ = reinterpret_cast<unsigned char *>(pixels);
    Buffer::release_buff_sprite();
    assert(!g_BUFFER_SPRITE.pcBits_ && !unlocks);
}

static void four_argument_control() {
    puts("CONTROL_OVERLOAD=4"); fflush(stdout);
    Buffer source, destination;
    memset(&source, 0, sizeof(source)); memset(&destination, 0, sizeof(destination));
    Surface from, to;
    source.surface_ = &from; destination.surface_ = &to;
    source.dib_.bmiHeader.biWidth = 12; source.dib_.bmiHeader.biHeight = -9;
    int old_keys = key_calls, old_blits = blits, old_draws = draws4;
    assert(source.draw(0, -1, 5, 7) == 0x10);
    assert(key_calls == old_keys && blits == old_blits && draws4 == old_draws);
    int old_whole = whole_surface_blits;
    assert(source.draw(&destination, -1, 5, 7) == -1);
    assert(key_calls == old_keys + 1 && blits == old_blits + 1 && draws4 == old_draws);
    assert(whole_surface_blits == old_whole + 1 && to.source == &from);
    assert(from.key.dwColorSpaceLowValue == 0xFFFFFFFFUL && from.key.dwColorSpaceHighValue == 0xFFFFFFFFUL);
    assert(source.draw(&destination, 260, 5, 7) == 260);
    assert(whole_surface_blits == old_whole + 2 && to.source == &from);
    assert(from.key.dwColorSpaceLowValue == 260 && from.key.dwColorSpaceHighValue == 260);

    char pixels[32];
    expected_destination = &destination;
    expected_width = 12; expected_height = 9; expected_stride = 16;
    source.stride_ = 16; source.surface_ = 0;
    for (int mode = 0; mode < 8; ++mode) {
        source.dib_bits_ = mode & 1 ? pixels : 0;
        destination.surface_ = mode & 2 ? &to : 0;
        expected_bits = static_cast<unsigned char *>(source.dib_bits_);
        expected_index = mode & 4 ? -1 : 260;
        expected_colour = mode & 4 ? 201 : 4;
        int before = draws4;
        assert(source.draw(&destination, expected_index, 5, 7) == 73);
        assert(draws4 == before + 1 && !g_BUFFER_SPRITE.pcBits_);
        assert(!source.locked_bits_ && !source.surface_lock_count_ && !unlocks);
    }
    source.surface_ = &from; destination.surface_ = 0;
    char retained_pixels[64], retained_lock[8];
    source.dib_bits_ = pixels;
    source.locked_bits_ = retained_lock;
    source.surface_lock_count_ = 3;
    g_BUFFER_SPRITE.pcBits_ = reinterpret_cast<unsigned char *>(retained_pixels);
    expected_bits = reinterpret_cast<unsigned char *>(retained_pixels);
    g_BUFFER_SPRITE.cTransparentIndex_ = expected_colour = 17;
    g_BUFFER_SPRITE.iSpriteWidth2_ = expected_stride = 40;
    g_BUFFER_SPRITE.iSpriteWidth_ = g_BUFFER_SPRITE.iWidth_ = expected_width = 23;
    g_BUFFER_SPRITE.iSpriteHeight_ = g_BUFFER_SPRITE.iHeight_ = expected_height = 19;
    Sprite retained_descriptor = g_BUFFER_SPRITE;
    expected_index = -1;
    assert(source.draw(&destination, -1, 5, 7) == 73);
    assert(!g_BUFFER_SPRITE.pcBits_);
    assert(source.locked_bits_ == retained_lock && source.surface_lock_count_ == 3);
    assert(!unlocks);
    retained_descriptor.pcBits_ = 0;
    assert(memcmp(&g_BUFFER_SPRITE, &retained_descriptor, sizeof(Sprite)) == 0);
}

static const bool four_argument_first = false;
int main() {
    if (four_argument_first) {
        four_argument_control();
        six_argument_control();
    } else {
        six_argument_control();
        four_argument_control();
    }
    return 0;
}
