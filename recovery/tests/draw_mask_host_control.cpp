#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef long long __int64;

struct RECT { int left, top, right, bottom; };
struct Vert { int x, y; };
struct EdgeScan {
    int f0, counter, id, accumA, accumB, deltaA, deltaB, accumC, deltaC,
        extraDeltaC, fracAccum, fracDelta, fracSub, accumD, deltaD;
};

static unsigned char *surface;
class Buffer {
public:
    RECT rect1_;
    unsigned int stride_;
    int frees;
    void *get_data() { return surface; }
    void free_data(int value) { assert(value == 1); ++frees; }
};

class Texture {
public:
    unsigned char *pixels_;
    unsigned int iWidth_;
    unsigned int iHeight_;
    static int setup_edge_gouraud(EdgeScan *, int);
    void draw_mask_dest(Buffer *, Vert *, unsigned char *, unsigned char *, int,
                        Vert *);
};

static int slots[64];
static Vert *edge_uvs_a_cell = 0;
static Vert **blit_edge_uvs_a = &edge_uvs_a_cell;
static int *blit_slot_498 = &slots[1];
static int *blit_buffer_stride = &slots[2];
static int *blit_slot_4a4 = &slots[3];
static int *blit_texture_width = &slots[4];
static int *blit_vertex_count = &slots[5];
static int *blit_max_y_index = &slots[6];
static int *blit_texture_height = &slots[7];
static int *blit_slot_4c4 = &slots[8];
static int *blit_slot_4c8 = &slots[9];
static int *blit_span_right = &slots[10];
static int *blit_clip_left = &slots[11];
static int *blit_clip_top = &slots[12];
static int *blit_slot_4e4 = &slots[13];
static void *pixel_base_cell;
static void **blit_pixel_base = &pixel_base_cell;
static int *blit_span_left = &slots[15];
static Vert *edge_uvs_b_cell = 0;
static Vert **blit_edge_uvs_b = &edge_uvs_b_cell;
static int *blit_slot_4f8 = &slots[17];
static int *blit_slot_4fa = &slots[18];
static int *z_values_cell = 0;
static int **blit_z_values = &z_values_cell;
static int *blit_row_offset = &slots[20];
union TextureRasterRelocatedRowCell { int *initial_target; int value; };
static int alternate_row_520, alternate_row_524;
static TextureRasterRelocatedRowCell TextureRasterAlternateRowBase = { &alternate_row_520 };
static TextureRasterRelocatedRowCell TextureRasterAlternateRowNext = { &alternate_row_524 };
static int *blit_slot_528 = &slots[23];
static int *blit_clip_right = &slots[24];
static int *blit_clip_bottom = &slots[25];
static void *texture_pixels_cell;
static void **blit_texture_pixels = &texture_pixels_cell;
static uintptr_t remap_table_cell;
static uintptr_t *blit_remap_table = &remap_table_cell;
static int *blit_slot_530 = &slots[28];
static int *blit_slot_534 = &slots[29];
static int *blit_slot_538 = &slots[30];
static int *blit_slot_53c = &slots[31];
static int *blit_slot_564 = &slots[32];
static unsigned short light_pack_words[2] = {};
static unsigned short *blit_light_pack_a = &light_pack_words[0];
static int TextureRowPitchScratch;
static int ambient_z[4];
static int edge_setup_calls;
static int edge_indices[64];
static int edge_directions[64];
static int edge_mode;
static int edge_refreshes;
static int observed_gradient_before_row2;
static int fpu_reset_calls;
static int rounding_control_calls;

static unsigned int concat22(unsigned short hi, unsigned short lo) {
    return (static_cast<unsigned int>(hi) << 16) | lo;
}
static int texture_difference(int a, int b) {
    return static_cast<int>(static_cast<unsigned int>(a) -
                            static_cast<unsigned int>(b));
}
static int carry4(unsigned int a, unsigned int b) { return a + b < a; }
static int carry2(unsigned short a, unsigned short b) {
    return static_cast<unsigned int>(a) + b > 0xffffu;
}
static unsigned int texture_span_pixels(int difference) {
    const unsigned int pixels = static_cast<unsigned int>(difference) & 0xffffu;
    return pixels - 1u < 0x8001u ? pixels : 1u;
}
static int texture_span_step(int delta, int width) {
    // Candidate tests use exactly divisible edge deltas. Native x87 rounding
    // proof remains a separate obligation; these controls exercise source flow.
    return static_cast<int>((static_cast<int64_t>(delta) << 16) /
                            (static_cast<int64_t>(width) << 16));
}
static void texture_reset_fpu() { ++fpu_reset_calls; }
#define _PC_24 0x00020000
#define _RC_CHOP 0x00000300
#define _MCW_PC 0x00030000
#define _MCW_RC 0x00000300
static unsigned int _control87(unsigned int, unsigned int) {
    ++rounding_control_calls;
    return 0;
}
static void edge_step(EdgeScan &edge) {
    edge.accumA += edge.deltaA;
    edge.accumB += edge.deltaB;
    edge.accumC += edge.deltaC;
    edge.fracAccum += edge.fracDelta;
    if (edge.fracAccum > 0) {
        edge.accumC += edge.extraDeltaC;
        edge.fracAccum -= edge.fracSub;
    }
    edge.accumD += edge.deltaD;
}
static int fixed_div(int a, int b) { return a / b; }

int Texture::setup_edge_gouraud(EdgeScan *edge, int index) {
    assert(edge_setup_calls < 64);
    edge_indices[edge_setup_calls] = index;
    edge_directions[edge_setup_calls] = edge->f0;
    ++edge_setup_calls;
    assert(*blit_max_y_index == 2);
    int *z = *blit_z_values;
    assert(z == ambient_z);
    (void)z[index]; // The untouched ambient pointer is consumed by setup.
    if (edge_mode == 4 && index != 0) {
        if (edge->f0 < 0) {
            ++edge_refreshes;
            edge->accumC = edge_refreshes == 1 ? 3 : 0;
            if (edge_refreshes == 2) edge->accumA = 0x18000;
            if (edge_refreshes == 3)
                observed_gradient_before_row2 = *blit_slot_4e4;
        } else {
            edge->accumC = 2;
        }
        edge->counter = 1;
        return 1;
    }
    if (index != 0) return 0;
    edge->counter = (edge_mode == 3 || edge_mode == 4) ? 1 : 2;
    edge->id = edge->f0 < 0 ? 3 : 1;
    if (edge_mode == 5) {
        edge->accumA = 0x8000;
        edge->deltaA = 0;
    } else if (edge_mode == 1) {
        edge->accumA = edge->f0 < 0 ? 0x18000 : 0x20000;
        edge->deltaA = edge->f0 < 0 ? -0x10000 : 0;
    } else if (edge_mode == 2) {
        edge->accumA = edge->f0 < 0 ? 0xc000 : 0x20000;
        edge->deltaA = 0;
    } else {
        edge->accumA = edge->f0 < 0 ? 0x8000 : 0x28000;
        edge->deltaA = 0;
    }
    edge->accumB = 0x8000;
    edge->deltaB = 0;
    edge->accumC = edge_mode == 4
        ? (edge->f0 < 0 ? 0 : 2)
        : (edge->f0 < 0 ? 0 : 2);
    edge->deltaC = 0;
    edge->fracAccum = edge->fracDelta = edge->fracSub = edge->extraDeltaC = 0;
    edge->accumD = z[index] << 16;
    edge->deltaD = 0;
    return 1;
}

static void reset_case_state() {
    memset(slots, 0, sizeof(slots));
    pixel_base_cell = 0;
    texture_pixels_cell = 0;
    remap_table_cell = 0;
    z_values_cell = ambient_z;
    edge_setup_calls = 0;
    edge_refreshes = 0;
    observed_gradient_before_row2 = 0;
    fpu_reset_calls = 0;
    rounding_control_calls = 0;
}

// ACTUAL_METHOD_INSERTION

int main() {
    static unsigned char pixels[16] = {
        0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
    };
    static unsigned char remap[256];
    static unsigned char palette[256 * 256];
    static Vert vertices[4] = {{0,0},{2,0},{2,2},{0,2}};
    static Vert uv[4] = {{0,0},{2,0},{2,2},{0,2}};
    static unsigned char storage[4 * 4] = {
        77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77
    };
    memset(remap, 0, sizeof(remap));
    remap[2] = 2;
    remap[3] = 3;
    remap[4] = 4;
    for (int i = 0; i < 256 * 256; ++i) palette[i] = static_cast<unsigned char>(i);
    for (int i = 0; i < 256; ++i)
        palette[256 + i] = static_cast<unsigned char>(i + 1);
    for (int i = 0; i < 256; ++i)
        palette[512 + i] = static_cast<unsigned char>(i + 2);
    for (int i = 0; i < 256; ++i)
        palette[768 + i] = static_cast<unsigned char>(i + 3);
    memset(ambient_z, 0, sizeof(ambient_z));
    z_values_cell = ambient_z;
    surface = storage;
    Buffer buffer = {};
    buffer.rect1_ = RECT{0, 0, 4, 4};
    buffer.stride_ = 4;
    Texture texture = {};
    texture.pixels_ = pixels;
    texture.iWidth_ = 4;
    texture.iHeight_ = 4;
    texture.draw_mask_dest(&buffer, vertices, remap, palette, 4, uv);
    assert(buffer.frees == 1);
    assert(fpu_reset_calls == 2 && rounding_control_calls == 1);
    assert(edge_setup_calls == 3);
    assert(edge_indices[0] == 0 && edge_indices[1] == 0);
    assert(edge_directions[0] == -1 && edge_directions[1] == 1);
    assert(storage[0] == 77); // remap zero retains the destination pixel
    assert(storage[1] == 78); // remapped source adds through palette + old dst
    assert(storage[4] == 77);
    assert(storage[5] == 78);
    assert(storage[2] == 77 && storage[3] == 77);
    assert(storage[8] == 77 && storage[9] == 77);
    assert(storage[12] == 77 && storage[13] == 77);

    pixels[0] = 2;
    pixels[1] = 3;
    pixels[2] = 4;
    // The first row uses the initial fractional gradient (du=0x4000); after
    // the left edge advances, the next row uses du=0xc000. A same-row
    // recomputation incorrectly carries into source[2] on row 0's x=1.
    reset_case_state();
    edge_mode = 1;
    memset(storage, 77, sizeof(storage));
    buffer.rect1_ = RECT{0, 0, 4, 4};
    texture.draw_mask_dest(&buffer, vertices, remap, palette, 4, uv);
    assert(storage[1] == 79 && storage[5] == 79);
    assert(edge_setup_calls == 3);

    // Fractional U endpoint and step sum across 0xffff. The wrapped carry
    // advances the texture source offset before the second destination pixel.
    reset_case_state();
    edge_mode = 2;
    memset(storage, 77, sizeof(storage));
    buffer.rect1_ = RECT{0, 0, 4, 4};
    texture.draw_mask_dest(&buffer, vertices, remap, palette, 4, uv);
    assert(storage[0] == 78 && storage[1] == 79);

    // A failed left-edge refresh jumps over all right-edge decrement/refresh
    // work. The setup double would observe a fourth call if that work ran.
    reset_case_state();
    edge_mode = 3;
    memset(storage, 77, sizeof(storage));
    buffer.rect1_ = RECT{0, 0, 4, 4};
    texture.draw_mask_dest(&buffer, vertices, remap, palette, 4, uv);
    assert(edge_setup_calls == 3);

    // Row 0 starts with a positive span; its edge update collapses the span.
    // Row 1's TEST/JLE skips conversion and leaves the global gradient at
    // 0x10000 while the restored positive span prepares 0x8000. Row 2 sees
    // the prior global before publishing the newly converted 0x8000 value.
    reset_case_state();
    edge_mode = 4;
    memset(storage, 77, sizeof(storage));
    buffer.rect1_ = RECT{0, 0, 4, 3};
    texture.draw_mask_dest(&buffer, vertices, remap, palette, 4, uv);
    assert(edge_setup_calls == 8);
    assert(observed_gradient_before_row2 == 0x10000);
    assert(storage[8] == 79 && storage[9] == 80);
    assert(slots[13] == 0x8000);

    // The clipped endpoints collapse to zero pixels. The original packed
    // counter admits low16 difference zero as one pixel; a signed `x < end`
    // loop silently drops this compatibility write.
    reset_case_state();
    edge_mode = 5;
    pixels[0] = 2;
    memset(storage, 77, sizeof(storage));
    buffer.rect1_ = RECT{1, 0, 1, 1};
    texture.draw_mask_dest(&buffer, vertices, remap, palette, 4, uv);
    assert(storage[0] == 77 && storage[1] == 78);

    reset_case_state();
    edge_mode = 0;
    pixels[0] = 0;
    pixels[1] = 2;
    memset(storage, 77, sizeof(storage));
    buffer.frees = 0;
    buffer.rect1_ = RECT{1, 0, 2, 1};
    texture.draw_mask_dest(&buffer, vertices, remap, palette, 4, uv);
    assert(buffer.frees == 1);
    assert(storage[0] == 77 && storage[1] == 78);
    assert(storage[4] == 77 && storage[5] == 77);
    assert(storage[8] == 77 && storage[9] == 77);
    const int calls_before_guard = edge_setup_calls;
    texture.draw_mask_dest(&buffer, 0, remap, palette, 4, uv);
    assert(buffer.frees == 2);
    assert(fpu_reset_calls == 3); // one exit reset on the rejected call
    assert(edge_setup_calls == calls_before_guard);
    return 0;
}
