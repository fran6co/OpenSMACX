#include <cassert>
#include <stdint.h>
#include <cstring>
#include <vector>

#define __int64 long long
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ulonglong;
typedef long long longlong;
static ulonglong CONCAT44(uint hi, uint lo) { return (ulonglong(hi) << 32) | lo; }
static uint CONCAT22(ushort hi, ushort lo) { return (uint(hi) << 16) | lo; }
static uint CONCAT31(uint hi, byte lo) { return (hi << 8) | lo; }
static bool CARRY1(byte a, byte b) { return uint(a) + uint(b) > 0xff; }
static bool CARRY4(uint a, uint b) { return ulonglong(a) + b > 0xffffffffULL; }

struct RECT { int left, top, right, bottom; };
struct Vert { int x, y; };
struct EdgeScan {
    int f0, counter, id, accumA, accumB, deltaA, deltaB, accumC, deltaC,
        extraDeltaC, fracAccum, fracDelta, fracSub, accumD, deltaD;
};
static const unsigned int _PC_24 = 1, _RC_CHOP = 2, _MCW_PC = 4, _MCW_RC = 8;
static unsigned int current_fpu, release_fpu;
static int control_calls;
static unsigned int _control87(unsigned int mode, unsigned int mask) {
    assert(mode == (_PC_24 | _RC_CHOP) && mask == (_MCW_PC | _MCW_RC));
    ++control_calls;
    current_fpu = mode;
    return mode;
}
struct Buffer {
    int stride_;
    RECT rect1_;
    RECT rect2_;
    unsigned char *data_;
    std::vector<int> events_;
    void *get_data() {
        events_.push_back(1);
        return data_;
    }
    void free_data(int mode) {
        assert(mode == 1);
        release_fpu = current_fpu;
        events_.push_back(3);
    }
};
static uintptr_t texture_pointer_address(const void *pointer) {
    return reinterpret_cast<uintptr_t>(pointer);
}
static uintptr_t texture_pointer_offset(const void *base, uint32_t offset) {
    return texture_pointer_address(base) +
        static_cast<intptr_t>(static_cast<int32_t>(offset));
}

class Texture {
public:
    unsigned char *pixels_;
    uint32_t iWidth_;
    uint32_t iHeight_;
    Vert corners_[4];
    Vert cornersAndMids_[8];
    uint32_t borrowed_;
    static int setup_edge_gouraud(EdgeScan *, int);
    int setup_edge(EdgeScan *, int);
    void sub_61f0d0(Buffer *, Vert *, int *, int, int, uint32_t, int);
    int draw_trans2(Buffer *, Vert *, int);
    int draw_trans_3(Buffer *, Vert *, int *, int, Vert *, int);
    void draw_trans(Buffer *, Vert *, int *, int, Vert *);
};

// The extracted body refers only to this mapped set of raster cells before
// the stubbed edge setup returns zero. Definitions also keep the full method
// well-typed, though the host checks make no claims about its inner rasterizer.
static int TextureRowPitchScratch;
static int cell_buffer_stride, cell_clip_bottom, cell_clip_left, cell_clip_right,
    cell_clip_top, cell_colour_step, cell_light_accum_b, cell_light_pack_a,
    cell_light_step_b, cell_max_y,
    cell_row_offset, cell_slot_4d8, cell_slot_4e8,
    cell_slot_55c, cell_texture_height,
    cell_texture_width, cell_vertex_count, cell_light_threshold, cell_span_left,
    cell_span_right, cell_slot_4c4, cell_slot_4c8, cell_slot_4ca,
    cell_slot_494, cell_slot_498, cell_slot_4e4, cell_slot_4e6, cell_slot_4f8, cell_slot_4fa,
    cell_slot_530, cell_slot_53c, cell_slot_53e, cell_dither,
    cell_z_key;
static void *cell_pixel_base, *cell_shade_table, *cell_texture_pixels;
static uintptr_t cell_remap_table, cell_slot_558;
static int *cell_z_values;
static Vert *cell_edge_a;
static Vert *cell_edge_b;
static int *blit_buffer_stride = &cell_buffer_stride;
static int *blit_clip_bottom = &cell_clip_bottom;
static int *blit_clip_left = &cell_clip_left;
static int *blit_clip_right = &cell_clip_right;
static int *blit_clip_top = &cell_clip_top;
static int *blit_colour_step = &cell_colour_step;
static int *blit_light_accum_b = &cell_light_accum_b;
static unsigned short *blit_light_pack_a = reinterpret_cast<unsigned short *>(&cell_light_pack_a);
static int *blit_light_step_b = &cell_light_step_b;
static int *blit_max_y_index = &cell_max_y;
static void **blit_pixel_base = &cell_pixel_base;
static uintptr_t *blit_remap_table = &cell_remap_table;
static int *blit_row_offset = &cell_row_offset;
static void **blit_shade_table = &cell_shade_table;
static int *blit_slot_4d8 = &cell_slot_4d8;
static int *blit_slot_4e8 = &cell_slot_4e8;
static uintptr_t *blit_slot_558 = &cell_slot_558;
static int *blit_slot_55c = &cell_slot_55c;
static int *blit_texture_height = &cell_texture_height;
static void **blit_texture_pixels = &cell_texture_pixels;
static int *blit_texture_width = &cell_texture_width;
static int *blit_vertex_count = &cell_vertex_count;
static int **blit_z_values = &cell_z_values;
static Vert **blit_edge_uvs_a = &cell_edge_a;
static Vert **blit_edge_uvs_b = &cell_edge_b;
static int *blit_light_threshold = &cell_light_threshold;
static int *blit_light_threshold_2 = &cell_z_key;
static int *blit_span_left = &cell_span_left;
static int *blit_span_right = &cell_span_right;
static int *blit_slot_4c4 = &cell_slot_4c4;
static int *blit_slot_4c8 = &cell_slot_4c8;
static unsigned char *blit_slot_4ca = reinterpret_cast<unsigned char *>(&cell_slot_4ca);
static int *blit_slot_4e4 = &cell_slot_4e4;
static unsigned char *blit_slot_4e6 = reinterpret_cast<unsigned char *>(&cell_slot_4e6);
static int *blit_slot_4f8 = &cell_slot_4f8;
static unsigned short *blit_slot_4fa = reinterpret_cast<unsigned short *>(&cell_slot_4fa);
static int *blit_slot_530 = &cell_slot_530;
static int *blit_slot_53c = &cell_slot_53c;
static unsigned short *blit_slot_53e = reinterpret_cast<unsigned short *>(&cell_slot_53e);
static int *blit_dither = &cell_dither;
static int *blit_slot_494 = &cell_slot_494;
static int *blit_slot_498 = &cell_slot_498;
static int TextureLightCeiling = 256;
static int TextureLightFloor = -256;
static int cell_slot_504, cell_slot_528;
static int *blit_slot_504 = &cell_slot_504;
static int *blit_slot_528 = &cell_slot_528;
union TextureRasterRelocatedRowCell {
    int *initial_target;
    int value;
};
static TextureRasterRelocatedRowCell TextureRasterAlternateRowBase;
static TextureRasterRelocatedRowCell TextureRasterAlternateRowNext;
static void texture_reset_fpu();

static int setup_calls;
static int setup_edge_calls;
static int setup_index;
static int setup_selected_uv;
static int setup_edge_result;
static int forward_calls;
static Buffer *active_buffer;
static int sub_620100(EdgeScan *, int) { return 0; }

int Texture::setup_edge_gouraud(EdgeScan *, int index) {
    ++setup_calls;
    active_buffer->events_.push_back(2);
    setup_index = index;
    setup_selected_uv = static_cast<int>(reinterpret_cast<uintptr_t>(cell_edge_a));
    return setup_edge_result;
}

int Texture::setup_edge(EdgeScan *, int) {
    ++setup_edge_calls;
    active_buffer->events_.push_back(2);
    return 0;
}

void Texture::draw_trans(Buffer *, Vert *, int *, int, Vert *) {
    ++forward_calls;
}

struct EdgeLocal {
    int setupSentinel, counter, refreshIdx, accumA, accumB, stepA, stepB,
        accumC, stepC, stepCExtra, accumD, stepD, threshold;
};

// ACTUAL_TRANS3_HELPER_INSERTION
// ACTUAL_METHOD_INSERTION
// ACTUAL_DRAW_TRANS2_INSERTION
// ACTUAL_DRAW_TRANS3_INSERTION

static Texture texture;
static Buffer buffer;
static unsigned char pixels[64];
static unsigned char texels[256];
static int z_values[8];
static int remap_words[256];
static int shade_words[256];
static Vert input_vertices[8];
static Vert input_xy[8];
static int input_z[8];
static Vert input_uv[8];
static int reset_calls;

static void texture_reset_fpu() { ++reset_calls; current_fpu = 0; }

static int as_int(const void *pointer) {
    const uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
    assert(address <= 0xffffffffu);
    return static_cast<int>(address);
}

static void reset_case() {
    setup_calls = setup_index = setup_selected_uv = 0;
    setup_edge_calls = forward_calls = reset_calls = 0;
    current_fpu = release_fpu = control_calls = 0;
    setup_edge_result = 0;
    buffer.events_.clear();
    active_buffer = &buffer;
    buffer.stride_ = 73;
    buffer.rect1_.left = 11;
    buffer.rect1_.top = 12;
    buffer.rect1_.right = 113;
    buffer.rect1_.bottom = 114;
    buffer.rect2_.left = 201;
    buffer.rect2_.top = 202;
    buffer.rect2_.right = 303;
    buffer.rect2_.bottom = 304;
    buffer.data_ = pixels;
    texture.pixels_ = texels;
    texture.iWidth_ = 31;
    texture.iHeight_ = 47;
    std::memset(&texture.corners_, 0x41, sizeof(texture.corners_));
    std::memset(&texture.cornersAndMids_, 0x82, sizeof(texture.cornersAndMids_));
    for (int i = 0; i < 8; ++i) {
        input_vertices[i].x = 10 + i;
        input_vertices[i].y = 30 + i;
    }
    input_vertices[2].y = -3;
    input_vertices[5].y = 80;
    cell_pixel_base = 0;
    cell_clip_bottom = cell_clip_left = cell_clip_right = cell_clip_top = 0;
    cell_edge_a = cell_edge_b = 0;
    cell_z_values = 0;
    cell_texture_pixels = 0;
    cell_vertex_count = cell_texture_width = cell_texture_height = cell_buffer_stride = 0;
}

static void invoke(int count_mode, bool null_pixels = false, bool null_shade = false) {
    if (null_pixels) texture.pixels_ = 0;
    if (null_shade) shade_words[0] = 0; else shade_words[0] = 1;
    texture.sub_61f0d0(&buffer, input_vertices, z_values,
                       as_int(shade_words), as_int(remap_words), 0x12345678u,
                       count_mode);
}

static void test_four_and_eight_vertex_member_bindings() {
    reset_case();
    invoke(0);
    assert(setup_calls == 1 && setup_index == 2);
    assert(setup_selected_uv == as_int(texture.corners_));
    assert(cell_vertex_count == 4);
    assert(cell_texture_pixels == texture.pixels_);
    assert(TextureRowPitchScratch == 31 && cell_texture_width == 31 && cell_texture_height == 47);
    assert(cell_edge_b == input_vertices);
    assert(cell_z_values == z_values);
    assert(cell_remap_table == reinterpret_cast<uintptr_t>(remap_words));
    assert(cell_buffer_stride == 73);
    assert(cell_clip_left == 11 && cell_clip_top == 12 && cell_clip_right == 113 && cell_clip_bottom == 114);
    assert(buffer.events_.size() == 3 && buffer.events_[0] == 1 &&
           buffer.events_[1] == 2 && buffer.events_[2] == 3);

    reset_case();
    invoke(1);
    assert(setup_calls == 1 && setup_index == 2);
    assert(setup_selected_uv == as_int(texture.cornersAndMids_));
    assert(cell_vertex_count == 8);
    assert(cell_clip_left == 11 && cell_clip_top == 12 && cell_clip_right == 113 && cell_clip_bottom == 114);
    assert(buffer.events_.size() == 3 && buffer.events_[0] == 1 &&
           buffer.events_[1] == 2 && buffer.events_[2] == 3);
}

static void test_high_y_retains_original_pointer_bits_as_index() {
    reset_case();
    for (int i = 0; i < 8; ++i) input_vertices[i].y = 0x7fff + i;
    invoke(0);
    assert(setup_calls == 1);
    assert(setup_index == as_int(input_vertices));
    assert(cell_edge_b == input_vertices && cell_z_values == z_values);
    assert(buffer.events_.size() == 3 && buffer.events_[0] == 1 &&
           buffer.events_[1] == 2 && buffer.events_[2] == 3);
}

static void test_guard_order_and_release() {
    reset_case();
    texture.sub_61f0d0(0, input_vertices, z_values,
                       as_int(shade_words), as_int(remap_words), 1, 0);
    assert(setup_calls == 0 && buffer.events_.empty());

    for (int guard = 0; guard != 6; ++guard) {
        reset_case();
        Vert *vertices = guard == 0 ? 0 : input_vertices;
        int *zvals = guard == 1 ? 0 : z_values;
        const int shade_holder = guard == 2 ? 0 : as_int(shade_words);
        const int remap = guard == 3 ? 0 : as_int(remap_words);
        const uint32_t threshold = guard == 4 ? 0 : 1;
        if (guard == 5) texture.pixels_ = 0;
        texture.sub_61f0d0(&buffer, vertices, zvals, shade_holder, remap,
                           threshold, 0);
        assert(setup_calls == 0);
        assert(buffer.events_.size() == 1 && buffer.events_[0] == 3);
    }

    reset_case();
    // Non-null holder passes entry guard, then get_data runs before its
    // dereferenced shade pointer rejects setup.
    invoke(0, false, true);
    assert(setup_calls == 0 && buffer.events_.size() == 2 &&
           buffer.events_[0] == 1 && buffer.events_[1] == 3);

    reset_case();
    buffer.data_ = 0;
    invoke(0);
    assert(setup_calls == 0 && buffer.events_.size() == 2 &&
           buffer.events_[0] == 1 && buffer.events_[1] == 3);
}

static void test_draw_trans2_member_setup_without_raster() {
    for (int mode = 0; mode != 2; ++mode) {
        reset_case();
        for (int i = 0; i < 8; ++i) {
            input_xy[i].x = 3 + i;
            input_xy[i].y = 20; // zero-height bounds skip the raster/setup loop
        }
        const int result = texture.draw_trans2(&buffer, input_xy, mode);
        assert(result == 0 && setup_edge_calls == 0);
        assert(cell_edge_a == (mode == 0 ? texture.corners_ : texture.cornersAndMids_));
        assert(cell_edge_b == input_xy && cell_vertex_count == (mode == 0 ? 4 : 8));
        assert(cell_texture_pixels == texture.pixels_);
        assert(TextureRowPitchScratch == 31 && cell_texture_width == 31 && cell_texture_height == 47);
        assert(cell_buffer_stride == 73);
        assert(cell_clip_left == 11 && cell_clip_top == 12 && cell_clip_right == 113 && cell_clip_bottom == 114);
        assert(buffer.events_.size() == 2 && buffer.events_[0] == 1 && buffer.events_[1] == 3);
    }
}

static void test_draw_trans3_light_clamp_setup_and_sentinel() {
    reset_case();
    input_z[0] = 300;
    input_z[1] = -300;
    input_z[2] = 7;
    input_vertices[0].x = 4; input_vertices[0].y = 2;
    input_vertices[1].x = 20; input_vertices[1].y = 19;
    input_vertices[2].x = 7; input_vertices[2].y = 6;
    const int result = texture.draw_trans_3(&buffer, input_vertices, input_z, 3, input_uv, 0x1234);
    assert(result == 0 && setup_calls == 1 && setup_index == 0);
    assert(input_z[0] == 256 && input_z[1] == -256 && input_z[2] == 7);
    assert(cell_edge_a == input_uv);
    assert(cell_edge_b == input_vertices);
    assert(cell_z_values == input_z && cell_vertex_count == 3);
    assert(cell_texture_pixels == texture.pixels_ && cell_texture_width == 31);
    assert(cell_buffer_stride == 73);
    assert(cell_clip_left == 11 && cell_clip_right == 113);
    assert(cell_clip_top == 12 * 73 + static_cast<int>(texture_pointer_address(cell_pixel_base)) &&
           cell_clip_bottom == 114 * 73 + static_cast<int>(texture_pointer_address(cell_pixel_base)));
    assert(buffer.events_.size() == 3 && buffer.events_[0] == 1 &&
           buffer.events_[1] == 2 && buffer.events_[2] == 3);

    reset_case();
    TextureLightCeiling = 100;
    TextureLightFloor = 300;
    input_z[0] = 200;
    input_vertices[0].y = 2;
    input_vertices[1].y = 19;
    input_vertices[2].y = 6;
    texture.draw_trans_3(&buffer, input_vertices, input_z, 3, input_uv, 0x1234);
    assert(input_z[0] == 300); // Ceiling test precedes floor test in source.
    TextureLightCeiling = 256;
    TextureLightFloor = -256;

    reset_case();
    const int forwarded = texture.draw_trans_3(&buffer, input_vertices, input_z, 3, input_uv, -1);
    assert(forwarded == 0 && forward_calls == 1 && setup_calls == 0);
    assert(buffer.events_.empty()); // sentinel forward precedes Buffer acquisition
}

int main() {
    test_four_and_eight_vertex_member_bindings();
    test_high_y_retains_original_pointer_bits_as_index();
    test_guard_order_and_release();
    test_draw_trans2_member_setup_without_raster();
    test_draw_trans3_light_clamp_setup_and_sentinel();
}
