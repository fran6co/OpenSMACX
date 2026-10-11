#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define __cdecl

struct RECT { int left, top, right, bottom; };
struct Vert { int x, y; };
struct EdgeScan {
    int f0, counter, id, accumA, accumB, deltaA, deltaB, accumC, deltaC;
    int extraDeltaC, fracAccum, fracDelta, fracSub, accumD, deltaD;
};

static unsigned char destination_bytes[256];
static void *slot_pixel_base, *slot_pixels, *slot_shade;
static const void *slot_remap;
static int slot_height, slot_count;
static int slot_stride, slot_width, slot_clip_right, slot_clip_left;
static int slot_clip_bottom, slot_clip_top, slot_max_y, slot_row, slot_du, slot_dv;
static int slot_498, slot_4c4, slot_530, slot_4f8, slot_53c;
static int slot_span_left, slot_span_right;

static void **blit_pixel_base = &slot_pixel_base;
static const void **blit_remap_table = &slot_remap;
static Vert *uv_b_cell = 0;
static Vert **blit_edge_uvs_b = &uv_b_cell;
static void **blit_texture_pixels = &slot_pixels;
static int *blit_texture_height = &slot_height;
static int *blit_vertex_count = &slot_count;
static Vert *uv_a_cell = 0;
static Vert **blit_edge_uvs_a = &uv_a_cell;
static int *blit_buffer_stride = &slot_stride;
static int *blit_texture_width = &slot_width;
static void **blit_shade_table = &slot_shade;
static int *blit_clip_right = &slot_clip_right;
static int *blit_clip_left = &slot_clip_left;
static int *blit_clip_bottom = &slot_clip_bottom;
static int *blit_clip_top = &slot_clip_top;
static int *blit_max_y_index = &slot_max_y;
static int *blit_row_offset = &slot_row;
static int *blit_slot_4e4 = &slot_du;
static int *blit_slot_4c8 = &slot_dv;
static int *blit_slot_498 = &slot_498;
static int *blit_slot_4c4 = &slot_4c4;
static int *blit_slot_530 = &slot_530;
static int *blit_slot_4f8 = &slot_4f8;
static int *blit_slot_53c = &slot_53c;
static int *blit_span_left = &slot_span_left;
static int *blit_span_right = &slot_span_right;
static int TextureRowPitchScratch;
static int reset_calls, control_calls, fixed_div_calls;
static uintptr_t texture_pointer_address(const void *pointer) {
    return reinterpret_cast<uintptr_t>(pointer);
}

static void texture_reset_fpu() { ++reset_calls; }
enum { _PC_24 = 1, _RC_CHOP = 2, _MCW_PC = 4, _MCW_RC = 8 };
static int _control87(int, int) { ++control_calls; return 0; }
static int texture_difference(int a, int b) {
    return static_cast<int>(static_cast<unsigned int>(a) -
                            static_cast<unsigned int>(b));
}
static int fixed_div(int numerator, int denominator) {
    ++fixed_div_calls;
    return static_cast<int>((static_cast<int64_t>(numerator) << 16) / denominator);
}

struct Buffer {
    unsigned int stride_;
    RECT rect1_;
    int free_calls;
    Buffer() : stride_(16), free_calls(0) {
        rect1_.left = 0; rect1_.top = 0; rect1_.right = 8; rect1_.bottom = 4;
    }
    void *get_data() { return destination_bytes; }
    void free_data(int n) { assert(n == 1); ++free_calls; }
};

struct Texture {
    unsigned char *pixels_;
    uint32_t iWidth_, iHeight_;
    Vert corners_[4];
    Vert cornersAndMids_[8];
    uint32_t borrowed_;
    Texture() : pixels_(0), iWidth_(256), iHeight_(256), borrowed_(1) {
        int i;
        for (i = 0; i != 4; ++i) {
            corners_[i].x = 0; corners_[i].y = 0;
        }
        for (i = 0; i != 8; ++i) {
            cornersAndMids_[i].x = 0; cornersAndMids_[i].y = 0;
        }
    }
    static int setup_edge(EdgeScan *, int);
    void sub_61cb50(Buffer *, Vert *, Texture *, unsigned char *, int);
};

/* ACTUAL_HELPERS */
/* ACTUAL_SETUP_EDGE */
/* ACTUAL_OWNER */

static void fill_texture(unsigned char *pixels, unsigned char value) {
    int i;
    for (i = 0; i != 65536; ++i) pixels[i] = value;
}

static void init_square(Vert *screen) {
    screen[0].x = 0; screen[0].y = 0;
    screen[1].x = 4; screen[1].y = 0;
    screen[2].x = 4; screen[2].y = 4;
    screen[3].x = 0; screen[3].y = 4;
}

static void test_remap_clip_and_page_wrap(Texture &receiver, Texture &shade,
                                          unsigned char *base,
                                          unsigned char *lookup,
                                          unsigned char *remap, Vert *screen) {
    Buffer buffer;
    int i;
    fill_texture(base, 7);
    fill_texture(lookup, 10);
    for (i = 0; i != 256; ++i) remap[i] = static_cast<unsigned char>(i);
    remap[10] = 0;
    receiver.pixels_ = base;
    shade.pixels_ = lookup;
    init_square(screen);
    buffer.rect1_.left = 2; buffer.rect1_.top = 1;
    buffer.rect1_.right = 3; buffer.rect1_.bottom = 3;
    receiver.sub_61cb50(&buffer, screen, &shade, remap, 0);
    assert(buffer.free_calls == 1);
    assert(destination_bytes[16 + 1] == 0 && destination_bytes[16 + 2] == 7);
    remap[10] = 0xe7;
    for (i = 0; i != 256; ++i) destination_bytes[i] = 0;
    receiver.sub_61cb50(&buffer, screen, &shade, remap, 0);
    assert(destination_bytes[16 + 2] == 0xe7);
}

static void test_fractional_page_wrap(Texture &receiver, Texture &shade,
                                      unsigned char *base,
                                      unsigned char *lookup,
                                      unsigned char *remap, Vert *screen,
                                      bool negative) {
    Buffer buffer;
    int i;
    fill_texture(base, 9);
    for (i = 0; i != 65536; ++i)
        lookup[i] = static_cast<unsigned char>((i & 0xff) ^ (i >> 8));
    for (i = 0; i != 256; ++i)
        remap[i] = static_cast<unsigned char>(i + 17);
    init_square(screen);
    receiver.pixels_ = base;
    shade.pixels_ = lookup;
    buffer.rect1_.left = 0; buffer.rect1_.top = 1;
    buffer.rect1_.right = 4; buffer.rect1_.bottom = 2;
    // At row one the horizontal UV starts straddle 0xff/0x100 and advance
    // by fractional fixed-point steps. The second case reverses both slopes.
    receiver.corners_[0].x = negative ? 258 : 254;
    receiver.corners_[0].y = negative ? 515 : 510;
    receiver.corners_[1].x = negative ? 242 : 270;
    receiver.corners_[1].y = negative ? 493 : 530;
    receiver.corners_[2].x = receiver.corners_[1].x;
    receiver.corners_[2].y = receiver.corners_[1].y;
    receiver.corners_[3].x = negative ? 254 : 252;
    receiver.corners_[3].y = negative ? 511 : 506;
    receiver.sub_61cb50(&buffer, screen, &shade, remap, 0);
    // Native setup starts the left edge at corner 0 and the right edge at
    // corner 1 (the top horizontal edge is skipped). At row one, the paired
    // 16.16 endpoints and horizontal steps sample (U,V) pages
    // (FD,FD),(01,02),(05,07),(09,0C) in the positive case. Reversing the
    // slopes samples (01,02),(FD,FC),(F9,F7),(F5,F2). The lookup byte is U xor
    // V and remap adds 17; these values cross the 0xff/0x00 page boundary in
    // both directions and distinguish the two independent coordinate pages.
    const unsigned char positive[4] = {17, 20, 19, 22};
    const unsigned char negative_bytes[4] = {20, 18, 31, 24};
    const unsigned char *expected = negative ? negative_bytes : positive;
    for (i = 0; i != 4; ++i)
        assert(destination_bytes[16 + i] == expected[i]);
}

static void test_eight_points_and_expiry(Texture &receiver, Texture &shade,
                                         unsigned char *base,
                                         unsigned char *lookup,
                                         unsigned char *remap, Vert *screen) {
    Buffer buffer;
    int before = fixed_div_calls;
    int i;
    fill_texture(base, 5); fill_texture(lookup, 10);
    for (i = 0; i != 256; ++i) remap[i] = 0;
    screen[0].x = 0; screen[0].y = 0;
    screen[1].x = 2; screen[1].y = 0;
    screen[2].x = 4; screen[2].y = 0;
    screen[3].x = 4; screen[3].y = 2;
    screen[4].x = 4; screen[4].y = 4;
    screen[5].x = 2; screen[5].y = 4;
    screen[6].x = 0; screen[6].y = 4;
    screen[7].x = 0; screen[7].y = 2;
    for (i = 0; i != 8; ++i) {
        receiver.cornersAndMids_[i].x = 0;
        receiver.cornersAndMids_[i].y = 0;
    }
    receiver.pixels_ = base; shade.pixels_ = lookup;
    buffer.rect1_.left = 0; buffer.rect1_.top = 0;
    buffer.rect1_.right = 4; buffer.rect1_.bottom = 4;
    receiver.sub_61cb50(&buffer, screen, &shade, remap, 1);
    assert(slot_count == 8 && buffer.free_calls == 1);
    assert(fixed_div_calls >= before + 8);
}

int main() {
    static unsigned char base[65536];
    static unsigned char lookup[65536];
    static unsigned char remap[256];
    static Vert screen[8];
    static Texture receiver, shade;
    test_remap_clip_and_page_wrap(receiver, shade, base, lookup, remap, screen);
    test_fractional_page_wrap(receiver, shade, base, lookup, remap, screen, false);
    test_fractional_page_wrap(receiver, shade, base, lookup, remap, screen, true);
    test_eight_points_and_expiry(receiver, shade, base, lookup, remap, screen);
    assert(control_calls >= 4 && reset_calls >= 8);
    return 0;
}
