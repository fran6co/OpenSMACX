// Actual Sprite declaration and Buffer bodies, with surface/draw doubles.
#include <assert.h>
#include <string.h>
typedef unsigned char uint8_t;
typedef void *LPVOID;
#define nullptr 0
#define MEASURED
struct RECT { int left, top, right, bottom; };
struct DDCOLORKEY { unsigned long dwColorSpaceLowValue, dwColorSpaceHighValue; };
enum { DDCKEY_SRCBLT = 8, DDBLT_WAIT = 0x1000000, DDBLT_KEYSRC = 0x8000 };
class Buffer;
class TexHeap;
#define private public
// LIVE_SPRITE
#undef private
Sprite::Sprite() { memset(this, 0, sizeof(*this)); }
void Sprite::close() {}
Sprite g_BUFFER_SPRITE;
const unsigned char WinFillColour = 201;
int key_calls, blits, unlocks, draws;
struct Surface {
    DDCOLORKEY key;
    RECT rectangle;
    Surface *source;
    int SetColorKey(int flags, DDCOLORKEY *value) {
        assert(flags == DDCKEY_SRCBLT); key = *value; ++key_calls; return -11;
    }
    int Blt(RECT *area, Surface *from, RECT *source_area, int flags, void *effects) {
        assert(!source_area && !effects && flags == (DDBLT_WAIT | DDBLT_KEYSRC));
        rectangle = *area; source = from; ++blits; return -12;
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
    static void release_buff_sprite();
    int draw(Buffer *, int, int, int, int, int);
};
Buffer *expected_destination;
char *expected_bits;
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
// LIVE_SETUP
// LIVE_RELEASE
// LIVE_DRAW

int main() {
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
        expected_bits = static_cast<char *>(source.dib_bits_);
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
    g_BUFFER_SPRITE.pcBits_ = expected_bits = pixels;
    expected_index = -1; // Other descriptor fields retained from final setup.
    assert(source.draw(&destination, -1, 5, 7, 2, 3) == 71);
    assert(!g_BUFFER_SPRITE.pcBits_);
    g_BUFFER_SPRITE.pcBits_ = pixels;
    Buffer::release_buff_sprite();
    assert(!g_BUFFER_SPRITE.pcBits_ && !unlocks);
    return 0;
}
