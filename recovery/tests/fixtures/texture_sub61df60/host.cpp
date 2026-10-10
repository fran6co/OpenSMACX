#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#define __cdecl

struct RECT { int left, top, right, bottom; };
struct Vert { int x, y; };
static void set_vertex(Vert &vertex, int x, int y) {
    vertex.x = x; vertex.y = y;
}
struct EdgeScan {
    int f0, counter, id, accumA, accumB, deltaA, deltaB;
    int accumC, deltaC, extraDeltaC, fracAccum, fracDelta, fracSub;
    int accumD, deltaD;
};

static unsigned char destination[256] __attribute__((aligned(65536)));
static unsigned char source_page[65536] __attribute__((aligned(65536)));
static unsigned char mask_page[65536] __attribute__((aligned(65536)));
static unsigned char palette_data[65536];
static unsigned char remap_data[256];
static Vert test_vertices[5];
static Vert test_uv[5];
static int test_light[5];
static void *slot_pixel_base, *slot_pixels, *slot_shade;
static int slot560, slot564, slot_height, slot_count;
static uintptr_t slot_remap;
static int slot_stride, slot_width, slot_clip_right, slot_clip_left;
static int slot_clip_bottom, slot_clip_top, slot_max_y, slot_row, slot_4c4;
static int slot_4f8, slot_530, slot_53c, slot_span_right, slot_span_left;
static int slot_4e4, slot_4c8, slot_498;
static int TextureRowPitchScratch;
static int reset_calls, control_calls, free_calls, setup_calls;
static void **blit_pixel_base = &slot_pixel_base;
static int *blit_slot_560 = &slot560;
static int *blit_slot_564 = &slot564;
static Vert *edge_uvs_b_cell = 0;
static Vert **blit_edge_uvs_b = &edge_uvs_b_cell;
static int *z_values_cell = 0;
static int **blit_z_values = &z_values_cell;
static uintptr_t *blit_remap_table = &slot_remap;
static void **blit_texture_pixels = &slot_pixels;
static int *blit_texture_height = &slot_height;
static Vert *edge_uvs_a_cell = 0;
static Vert **blit_edge_uvs_a = &edge_uvs_a_cell;
static int *blit_vertex_count = &slot_count;
static int *blit_buffer_stride = &slot_stride;
static int *blit_texture_width = &slot_width;
static void **blit_shade_table = &slot_shade;
static int *blit_clip_right = &slot_clip_right;
static int *blit_clip_left = &slot_clip_left;
static int *blit_clip_bottom = &slot_clip_bottom;
static int *blit_clip_top = &slot_clip_top;
static int *blit_max_y_index = &slot_max_y;
static int *blit_row_offset = &slot_row;
static int *blit_slot_4c4 = &slot_4c4;
static int *blit_slot_4f8 = &slot_4f8;
static int *blit_slot_530 = &slot_530;
static int *blit_slot_53c = &slot_53c;
static int *blit_span_right = &slot_span_right;
static int *blit_span_left = &slot_span_left;
static int *blit_slot_4e4 = &slot_4e4;
static int *blit_slot_4c8 = &slot_4c8;
static int *blit_slot_498 = &slot_498;
static unsigned short light_pack_words[2] = {};
static unsigned short *blit_light_pack_a = &light_pack_words[0];

enum { _PC_24 = 1, _RC_CHOP = 2, _MCW_PC = 4, _MCW_RC = 8 };
static int _control87(int, int) { ++control_calls; return 0; }
static void texture_reset_fpu() { ++reset_calls; }
static uintptr_t texture_pointer_address(const void *pointer) {
    return reinterpret_cast<uintptr_t>(pointer);
}
static int fixed_div(int numerator, int denominator) {
    return static_cast<int>((static_cast<int64_t>(numerator) << 16) /
                            denominator);
}

/* ACTUAL_HELPERS */

struct Buffer {
    unsigned int stride_;
    RECT rect1_;
    Buffer() : stride_(16) {
        rect1_.left = 0; rect1_.top = 0; rect1_.right = 8; rect1_.bottom = 4;
    }
    void *get_data() { return destination; }
    void free_data(int value) { assert(value == 1); ++free_calls; }
};

struct Texture {
    unsigned char *pixels_;
    unsigned int iWidth_, iHeight_;
    unsigned int borrowed_;
    Texture(unsigned char *pixels)
        : pixels_(pixels), iWidth_(256), iHeight_(256), borrowed_(1) {}
    static int setup_edge_gouraud(EdgeScan *, int);
    void sub_61df60(Buffer *, Vert *, int *, Texture *, unsigned char *, int,
                    Vert *, int, unsigned char *);
    void sub_624630(Buffer *, Vert *, int *, Texture *, unsigned char *, int,
                    Vert *, int, unsigned char *, int);
};

/* ACTUAL_SETUP */
/* ACTUAL_CANDIDATE */
/* ACTUAL_CALLER */

static void reset_state() {
    memset(destination, 0x31, sizeof(destination));
    memset(source_page, 0, sizeof(source_page));
    memset(mask_page, 0, sizeof(mask_page));
    memset(palette_data, 0, sizeof(palette_data));
    memset(remap_data, 0, sizeof(remap_data));
    reset_calls = control_calls = free_calls = setup_calls = 0;
}

static void test_dual_texture_modes_and_light() {
    reset_state();
    // With aligned 64 KiB page bases the native packed byte lanes address
    // offsets y*256+x. Give the second texture's mask texture explicit classes.
    source_page[0] = 10; source_page[1] = 20; source_page[2] = 30;
    source_page[256] = 40; source_page[257] = 50; source_page[258] = 60;
    mask_page[0] = 10; mask_page[1] = 11; mask_page[2] = 12;
    mask_page[256] = 11; mask_page[257] = 12; mask_page[258] = 10;
    remap_data[10] = 0; remap_data[11] = 1; remap_data[12] = 7;
    palette_data[0x0500 + 0x31] = 0xA1;
    palette_data[0x0500 + 0x32] = 0xA2;
    palette_data[0x0500 + 0x31] = 0xA1;

    set_vertex(test_vertices[0], 0, 0); set_vertex(test_vertices[1], 3, 0);
    set_vertex(test_vertices[2], 3, 2); set_vertex(test_vertices[3], 0, 2);
    set_vertex(test_uv[0], 0, 0); set_vertex(test_uv[1], 3, 0);
    set_vertex(test_uv[2], 3, 2); set_vertex(test_uv[3], 0, 2);
    test_light[0] = test_light[1] = test_light[2] = test_light[3] = 5;
    Buffer buffer;
    Texture source(source_page), mask(mask_page);
    source.sub_624630(&buffer, test_vertices, test_light, &mask, remap_data,
                      4, test_uv, 10, palette_data, -1);

    // Remap 0 leaves original destination bytes; mapped class 1 selects the
    // palette below threshold; another nonzero class writes itself.
    assert(destination[0] == 0x31);
    assert(destination[1] == 0xA1);
    assert(destination[2] == 7);
    assert(destination[16] == 0xA1);
    assert(destination[17] == 7);
    assert(destination[18] == 0x31);
    assert(free_calls == 1);
    assert(reset_calls == 2);
    assert(control_calls == 1);
    assert(setup_calls >= 2);
    assert(slot_498 == (5 << 8));
}

static void test_short_right_edge_refresh_changes_geometry() {
    reset_state();
    memset(remap_data, 0, sizeof(remap_data));
    remap_data[0] = 7;
    set_vertex(test_vertices[0], 0, 0);
    set_vertex(test_vertices[1], 3, 1);
    set_vertex(test_vertices[2], 4, 3);
    set_vertex(test_vertices[3], 0, 3);
    set_vertex(test_vertices[4], -1, 2);
    set_vertex(test_uv[0], 0, 0); set_vertex(test_uv[1], 0, 0);
    set_vertex(test_uv[2], 0, 0); set_vertex(test_uv[3], 0, 0);
    set_vertex(test_uv[4], 0, 0);
    test_light[0] = test_light[1] = test_light[2] = test_light[3] =
        test_light[4] = 0;
    Buffer buffer;
    Texture source(source_page), mask(mask_page);
    source.sub_61df60(&buffer, test_vertices, test_light, &mask, remap_data,
                      5, test_uv, 10, palette_data);
    // The right edge refreshes from vertex 1 to vertex 2 before the third
    // visible row, extending its endpoint to x=4. Without that refresh the
    // stale x=3 endpoint leaves pixel 3 untouched on row 2.
    assert(destination[2 * 16 + 3] == 7);
    assert(destination[2 * 16 + 4] == 0x31);
    assert(setup_calls >= 3);
    assert(slot_span_right == 0);
}

static void test_row_gradient_uses_pending_previous_span() {
    reset_state();
    int row, column;
    for (row = 0; row != 4; ++row) {
        for (column = 0; column != 8; ++column) {
            source_page[row * 256 + column] =
                static_cast<unsigned char>(10 + column);
            mask_page[row * 256 + column] = 0;
        }
    }
    remap_data[0] = 1;
    set_vertex(test_vertices[0], 0, 0); set_vertex(test_vertices[1], 2, 0);
    set_vertex(test_vertices[2], 2, 3); set_vertex(test_vertices[3], 0, 3);
    set_vertex(test_uv[0], 0, 0); set_vertex(test_uv[1], 4, 0);
    set_vertex(test_uv[2], 0, 3); set_vertex(test_uv[3], 0, 3);
    test_light[0] = test_light[1] = test_light[2] = test_light[3] = 0;
    Buffer buffer;
    Texture source(source_page), mask(mask_page);
    source.sub_61df60(&buffer, test_vertices, test_light, &mask, remap_data,
                      4, test_uv, -1, palette_data);
    // The top-row span's U slope is 2 texels/pixel. After advancing the
    // right edge, the next slope falls below 2 and is committed one row later.
    assert(destination[1] == 12);
    assert(destination[16 + 1] == 11);
}

static void test_light_at_or_above_threshold_is_direct_add() {
    reset_state();
    source_page[0] = 10;
    mask_page[0] = 11;
    remap_data[11] = 1;
    set_vertex(test_vertices[0], 0, 0); set_vertex(test_vertices[1], 2, 0);
    set_vertex(test_vertices[2], 2, 1); set_vertex(test_vertices[3], 0, 1);
    set_vertex(test_uv[0], 0, 0); set_vertex(test_uv[1], 2, 0);
    set_vertex(test_uv[2], 2, 1); set_vertex(test_uv[3], 0, 1);
    test_light[0] = test_light[1] = test_light[2] = test_light[3] = 5;
    Buffer buffer;
    Texture source(source_page), mask(mask_page);
    source.sub_61df60(&buffer, test_vertices, test_light, &mask, remap_data,
                      4, test_uv, 4, palette_data);
    assert(destination[0] == 15);
    assert(destination[1] == 0x31);
    assert(slot_498 == (5 << 8));
}

static void test_signed_threshold_and_page_wrap() {
    reset_state();
    // U=255 plus a +1 fixed-point step wraps the low source byte; V=255 plus
    // a +1 step wraps the vertical byte. Distinct samples keep both lanes
    // observable instead of allowing one palette result to hide bad reads.
    source_page[0xffff] = 0x20;
    source_page[0xff00] = 0x21;
    source_page[0x00ff] = 0x22;
    source_page[0x0000] = 0x23;
    mask_page[0xffff] = 1; mask_page[0xff00] = 2;
    mask_page[0x00ff] = 3; mask_page[0x0000] = 4;
    remap_data[1] = remap_data[2] = remap_data[3] = remap_data[4] = 1;
    set_vertex(test_vertices[0], 0, 0); set_vertex(test_vertices[1], 2, 0);
    set_vertex(test_vertices[2], 2, 3); set_vertex(test_vertices[3], 0, 3);
    set_vertex(test_uv[0], 255, 255); set_vertex(test_uv[1], 256, 256);
    set_vertex(test_uv[2], 256, 256); set_vertex(test_uv[3], 255, 255);
    test_light[0] = test_light[1] = test_light[2] = test_light[3] = 0;
    Buffer buffer;
    Texture source(source_page), mask(mask_page);
    source.sub_61df60(&buffer, test_vertices, test_light, &mask, remap_data,
                      4, test_uv, -1, palette_data);
    assert(texture_sub61df60_page_address(source_page, 255 << 16, 255 << 16) ==
           &source_page[0xffff]);
    assert(texture_sub61df60_page_address(source_page, 256 << 16, 255 << 16) ==
           &source_page[0xff00]);
    assert(texture_sub61df60_page_address(source_page, 255 << 16, 256 << 16) ==
           &source_page[0x00ff]);
    assert(destination[0] == 0x20);
    // Both packed byte lanes carry on the second pixel of the top row.
    assert(destination[1] == 0x23);
    assert(destination[16] == 0x20);
    assert(destination[17] == 0x20);
    assert(destination[32] == 0x20);
    assert(destination[33] == 0x20);
}

static void test_receiver_and_mask_borrowed_guards() {
    reset_state();
    Buffer buffer;
    Texture source(source_page), mask(mask_page);
    source.borrowed_ = 0;
    source.sub_61df60(&buffer, test_vertices, test_light, &mask, remap_data,
                      4, test_uv, 0, palette_data);
    assert(free_calls == 1 && reset_calls == 2 && control_calls == 0);
    assert(setup_calls == 2);

    reset_state();
    source.borrowed_ = 1;
    mask.borrowed_ = 0;
    source.sub_61df60(&buffer, test_vertices, test_light, &mask, remap_data,
                      4, test_uv, 0, palette_data);
    assert(free_calls == 1 && reset_calls == 2 && control_calls == 0);
    assert(setup_calls == 2);
}

static void test_unsigned_clipped_light_and_scratch() {
    reset_state();
    source_page[1] = 10;
    mask_page[1] = 0;
    remap_data[0] = 1;
    set_vertex(test_vertices[0], 0, 0); set_vertex(test_vertices[1], 2, 0);
    set_vertex(test_vertices[2], 2, 1); set_vertex(test_vertices[3], 0, 1);
    set_vertex(test_uv[0], 0, 0); set_vertex(test_uv[1], 2, 0);
    set_vertex(test_uv[2], 2, 1); set_vertex(test_uv[3], 0, 1);
    test_light[0] = test_light[3] = 1;
    test_light[1] = test_light[2] = -1;
    Buffer buffer;
    buffer.rect1_.left = 1;
    Texture source(source_page), mask(mask_page);
    source.sub_61df60(&buffer, test_vertices, test_light, &mask, remap_data,
                      4, test_uv, -1, palette_data);
    assert(light_pack_words[0] == 0xff00u);
    assert(slot_span_right == 1);
    assert(slot_498 == 0x10000);
    assert(destination[0] == 0x31 && destination[1] == 10);
}

static void test_cleanup_asymmetric_guards() {
    reset_state();
    Buffer buffer;
    Texture source(source_page), mask(mask_page);
    source.sub_61df60(&buffer, 0, 0, &mask, remap_data, 0, 0, 0,
                      palette_data);
    assert(free_calls == 1 && reset_calls == 1);
    reset_state();
    source.sub_61df60(0, 0, 0, &mask, remap_data, 0, 0, 0, palette_data);
    assert(free_calls == 0 && reset_calls == 1);
    reset_state();
    source.sub_61df60(&buffer, 0, 0, &mask, remap_data, 0, 0, 0,
                      palette_data);
    assert(free_calls == 1 && reset_calls == 1);
}

int main() {
    test_dual_texture_modes_and_light();
    test_short_right_edge_refresh_changes_geometry();
    test_row_gradient_uses_pending_previous_span();
    test_light_at_or_above_threshold_is_direct_add();
    test_signed_threshold_and_page_wrap();
    test_receiver_and_mask_borrowed_guards();
    test_unsigned_clipped_light_and_scratch();
    test_cleanup_asymmetric_guards();
    return 0;
}
