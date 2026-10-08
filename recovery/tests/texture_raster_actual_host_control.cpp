#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>

struct RECT { int left, top, right, bottom; };
struct EdgePoint { int a, b; };
struct EdgeScan {
    int f0, counter, id, accumA, accumB, deltaA, deltaB, accumC, deltaC,
        extraDeltaC, fracAccum, fracDelta, fracSub, accumD, deltaD;
};
struct TextureStore {
    unsigned int flags_;
    int next_record_;
    EdgeScan edges_[10];
};
class Buffer {
public:
    RECT rect1_;
    const RECT &current_clip() const { return rect1_; }
    int stride_;
    int frees_;
    int get_data() { return static_cast<int>(reinterpret_cast<intptr_t>(active_data_)); }
    void free_data(int) { ++frees_; }
    static unsigned char *active_data_;
};
class Texture {
public:
    void *pixels_;
    uint32_t iWidth_, iHeight_;
    EdgePoint corners_[4];
    EdgePoint cornersAndMids_[8];
    uint32_t borrowed_;
    int setup_edge_gouraud(EdgeScan *, int);
    int setup_edge(EdgeScan *, int);
    void sub_61b780(Buffer *, int *, int, int);
    void sub_620a10(Buffer *, int, int, int, TextureStore *);
    void sub_61d090(Buffer *, int *, int *, unsigned int);
};

static int slots[64];
static int *blit_pixel_base = &slots[0];
static int *blit_light_threshold = &slots[1];
static int *blit_light_threshold_2 = &slots[2];
static int *edge_uvs_b_cell = 0;
static int **blit_edge_uvs_b = &edge_uvs_b_cell;
static int *edge_uvs_a_cell = 0;
static int **blit_edge_uvs_a = &edge_uvs_a_cell;
static int *blit_vertex_count = &slots[5];
static int *z_values_cell = 0;
static int **blit_z_values = &z_values_cell;
static int *blit_texture_pixels = &slots[7];
static int *blit_buffer_stride = &slots[8];
static int *blit_texture_width = &slots[9];
static int *blit_texture_height = &slots[10];
static int *blit_clip_right = &slots[11];
static int *blit_clip_left = &slots[12];
static int *blit_clip_bottom = &slots[13];
static int *blit_clip_top = &slots[14];
static int *blit_row_offset = &slots[15];
static int *blit_max_y_index = &slots[16];
static int TextureRowPitchScratch = 4;
static unsigned int TextureRasterKey = 0;
unsigned char *Buffer::active_data_ = 0;
static int fpu_resets;
static int edge_helper_calls;
static TextureStore edge_cache_sentinel = {};
static TextureStore *expected_edge_cache;
static unsigned int edge_directions;
static int edge_helper_period = 4;
static int edge_helper_fail_at;
static int edge_helper_direction[16];
static int edge_helper_index[16];

static int texture_difference(int a, int b) { return a - b; }
static int texture_span_step(int delta, int width) { return delta / width; }
static unsigned short texture_light_step(int delta, int width) {
    return static_cast<unsigned short>(delta / width);
}
static int sub_624eb0(EdgeScan *edge, int top_index, TextureStore *cache) {
    assert(edge != 0);
    assert(cache != 0);
    assert(cache == expected_edge_cache);
    assert(edge->f0 == -1 || edge->f0 == 1);
    assert(top_index == 0);
    edge_directions |= edge->f0 < 0 ? 1u : 2u;
    ++edge_helper_calls;
    edge_helper_direction[edge_helper_calls - 1] = edge->f0;
    edge_helper_index[edge_helper_calls - 1] = top_index;
    edge->counter = edge_helper_period;
    edge->id = top_index;
    edge->accumA = edge->f0 < 0 ? 0 : 4 << 16;
    edge->accumB = 0;
    edge->accumC = edge->f0 < 0 ? 0 : 4;
    edge->deltaA = edge->deltaC = 0;
    edge->deltaB = 1 << 16;
    edge->fracAccum = edge->fracDelta = edge->fracSub = edge->extraDeltaC = 0;
    return edge_helper_calls == edge_helper_fail_at ? 0 : 1;
}
static void texture_reset_fpu() { ++fpu_resets; }

int Texture::setup_edge_gouraud(EdgeScan *edge, int) {
    edge->counter = 4;
    edge->id = 0;
    edge->accumA = edge->f0 < 0 ? 0 : 4 << 16;
    edge->accumB = 0;
    edge->accumC = edge->f0 < 0 ? 0 : 4;
    edge->accumD = 1 << 16;
    edge->deltaA = edge->deltaB = edge->deltaC = edge->deltaD = 0;
    edge->fracAccum = edge->fracDelta = edge->fracSub = edge->extraDeltaC = 0;
    return 1;
}
int Texture::setup_edge(EdgeScan *edge, int) {
    edge->counter = 4;
    edge->id = 0;
    edge->accumA = edge->f0 < 0 ? 0 : 4 << 16;
    edge->accumB = 0;
    edge->accumC = edge->f0 < 0 ? 0 : 4;
    edge->deltaA = edge->deltaB = edge->deltaC = 0;
    edge->fracAccum = edge->fracDelta = edge->fracSub = edge->extraDeltaC = 0;
    return 1;
}

// ACTUAL_TEXTURE_DEFINITIONS_INSERTED_BY_TEST

int main() {
    unsigned char source[4] = {0, 5, 10, 255};
    unsigned char owned_source[16] = {
        0, 5, 10, 255, 0, 15, 20, 255,
        0, 25, 30, 255, 0, 35, 40, 255,
    };
    unsigned char borrowed_source[4 * 256];
    memset(borrowed_source, 0, sizeof(borrowed_source));
    for (int y = 0; y < 4; ++y) {
        borrowed_source[y * 256 + 1] = static_cast<unsigned char>(5 + y * 10);
        borrowed_source[y * 256 + 2] = static_cast<unsigned char>(10 + y * 10);
        borrowed_source[y * 256 + 3] = 255;
    }
    unsigned char destination[4] = {77, 77, 77, 77};
    TextureRasterKey = 0;
    texture_raster_span(destination, source, 4, 0, 4, 0, 4,
                        0, 4, 0, 0, 0, 0, 1 << 8, 1 << 16, 0, 0,
                        0, 1, 0);
    assert(destination[0] == 77); // keyed sample preserves destination
    assert(destination[1] == 6);
    assert(destination[2] == 11);
    assert(destination[3] == 0); // byte addition wraps, it does not clamp

    unsigned char phase_output[1] = {77};
    texture_raster_span(phase_output, source, 4, 0, 1, 0, 1,
                        0, 1, 0x8000, 0, 0x8000, 0, 0,
                        0, 0, 0, 0, 0, 0);
    assert(phase_output[0] == 5); // phase crosses the texel boundary before sample

    unsigned char borrowed_light_output[3] = {77, 77, 77};
    texture_raster_span(borrowed_light_output, source, 4, 0, 3, 0, 3,
                        0, 3, 0, 0, 0, 0, 0xff00, 1 << 16, 0, 1,
                        0, 0, 1);
    assert(borrowed_light_output[0] == 255);
    assert(borrowed_light_output[1] == 4);
    assert(borrowed_light_output[2] == 9);

    assert(texture_raster_advance_light(0x0000ffffu, 1) == 0x00000000u);
    assert(texture_raster_advance_light(0x0001ffffu, 1) == 0x00010000u);
    assert(texture_raster_skip_light(0x0000ffffu, 1, 1) == 0x00010000u);
    assert(texture_raster_skip_light(0x12340000u, 0x8000, 3) == 0x12358000u);
    // Preserve signed IMUL behavior of the 32-bit 16.16 skip operand.
    assert(texture_raster_skip_light(0, 1, 32768) == 0xffff8000u);
    assert(texture_raster_skip_light(0, 1, 65535) == 0xffffffffu);
    assert(texture_raster_skip_light(0, 1, 65536) == 0x00000000u);
    // Independently model native signed IMUL's 64-bit product and SHRD.
    // Exercise every retained skip word, not just ordinary screen widths.
    const unsigned short clip_steps[] = {0, 1, 0x7fff, 0x8000, 0xffff};
    for (uint32_t skip = 0; skip < 0x10000u; ++skip) {
        const uint32_t fixed_bits = skip << 16;
        const int64_t signed_fixed = static_cast<int64_t>(fixed_bits) -
            ((fixed_bits & 0x80000000u) ? (static_cast<int64_t>(1) << 32) : 0);
        for (unsigned int n = 0; n < sizeof(clip_steps)/sizeof(clip_steps[0]); ++n) {
            const uint64_t product_bits = static_cast<uint64_t>(
                signed_fixed * static_cast<int64_t>(clip_steps[n]));
            const uint32_t increment = static_cast<uint32_t>(product_bits >> 16);
            const uint32_t expected = 0xfffffff0u + increment;
            assert(texture_raster_skip_light(0xfffffff0u, clip_steps[n], skip) == expected);
            assert(texture_raster_skip_light(0xfffffff0u, clip_steps[n], skip + 0x10000u) == expected);
        }
    }

    unsigned char borrowed_signed_light[2] = {77, 77};
    const unsigned char one_texel[1] = {1};
    texture_raster_span(borrowed_signed_light, one_texel, 1, 0, 2, 0, 2,
                        0, 2, 0, 0, 0, 0, 0x00018000, 0, 0, 0xffff,
                        0, 0, 1);
    assert(borrowed_signed_light[0] == 129);
    assert(borrowed_signed_light[1] == 128);

    unsigned char clipped_light[2] = {77, 77};
    texture_raster_span(clipped_light, one_texel, 1, 0, 1, 0, 2,
                        1, 2, 0, 0, 0, 0, 0x0000ff00, 0, 0, 1,
                        0, 0, 1);
    assert(clipped_light[0] == 77);
    assert(clipped_light[1] == 0);

    unsigned char wrapped_source[1] = {7};
    unsigned char wrapped_output[1] = {77};
    texture_raster_span(wrapped_output, wrapped_source, 1, 0, 1, 0, 1,
                        0, 1, 256 << 16, 0, 0, 0, 0x00018000, 0, 0, 0,
                        0, 0, 1);
    assert(wrapped_output[0] == 135);
    unsigned char unwrapped_output[1] = {77};
    const unsigned char unwrapped_source[257] = {0};
    texture_raster_span(unwrapped_output, unwrapped_source, 257, 0, 1, 0, 1,
                        0, 1, 256 << 16, 0, 0, 0, 0x00018000, 0, 0, 0,
                        0, 0, 0);
    assert(unwrapped_output[0] == 128);

    unsigned char owned_light_output[2] = {77, 77};
    texture_raster_span(owned_light_output, source, 4, 0, 2, 0, 2,
                        0, 2, 0, 0, 0, 0, 0xffff, 1 << 16, 0, 1,
                        0, 0, 0);
    assert(owned_light_output[0] == 255);
    assert(owned_light_output[1] == 5); // low-word wrap drops carry into high word

    unsigned char *surface = static_cast<unsigned char *>(
        mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0));
    assert(surface != MAP_FAILED);
    memset(surface, 77, 16);
    Buffer buffer;
    buffer.rect1_ = RECT{0, 0, 4, 3};
    buffer.stride_ = 4;
    buffer.frees_ = 0;
    Buffer::active_data_ = surface;
    Texture texture;
    texture.pixels_ = owned_source;
    texture.iWidth_ = 4;
    texture.iHeight_ = 4;
    texture.borrowed_ = 0;
    expected_edge_cache = &edge_cache_sentinel;
    int points[8] = {0, 0, 4, 0, 4, 3, 0, 3};
    int shade[4] = {1, 1, 1, 1};

    TextureRasterKey = 0;
    texture.sub_61b780(&buffer, points, reinterpret_cast<intptr_t>(shade), 0);
    assert(buffer.frees_ == 1 && fpu_resets == 2);
    for (int y = 0; y < 3; ++y) {
        assert(surface[y * 4] == 0); // owned integer path uses light bits 8..15
        assert(surface[y * 4 + 1] == 5);
        assert(surface[y * 4 + 2] == 10);
        assert(surface[y * 4 + 3] == 255);
    }

    memset(surface, 77, 16);
    buffer.frees_ = 0;
    texture.borrowed_ = 1;
    texture.pixels_ = borrowed_source;
    texture.sub_61b780(&buffer, points, reinterpret_cast<intptr_t>(shade), 0);
    assert(buffer.frees_ == 1);
    for (int y = 0; y < 3; ++y) {
        assert(surface[y * 4] == 77); // borrowed float path compares the texture key
        assert(surface[y * 4 + 1] == 5);
        assert(surface[y * 4 + 2] == 10);
        assert(surface[y * 4 + 3] == 255);
    }
    texture.borrowed_ = 0;
    texture.pixels_ = owned_source;

    memset(surface, 77, 16);
    buffer.frees_ = 0;
    texture.sub_61d090(&buffer, points, shade, 0);
    assert(buffer.frees_ == 1 && fpu_resets == 6);
    for (int y = 0; y < 3; ++y) {
        assert(surface[y * 4] == 0);
        assert(surface[y * 4 + 1] == 5);
        assert(surface[y * 4 + 2] == 10);
        assert(surface[y * 4 + 3] == 255);
    }

    memset(surface, 77, 16);
    buffer.frees_ = 0;
    texture.borrowed_ = 1;
    texture.pixels_ = borrowed_source;
    texture.sub_61d090(&buffer, points, shade, 0);
    assert(buffer.frees_ == 1 && fpu_resets == 8);
    for (int y = 0; y < 3; ++y) {
        assert(surface[y * 4] == 0);
        assert(surface[y * 4 + 1] == 5);
        assert(surface[y * 4 + 2] == 10);
        assert(surface[y * 4 + 3] == 255);
    }
    texture.borrowed_ = 0;
    texture.pixels_ = owned_source;

    memset(surface, 77, 16);
    buffer.frees_ = 0;
    TextureStore out_cache = {};
    out_cache.flags_ = 0xA5A5A5A5u;
    out_cache.next_record_ = 9;
    expected_edge_cache = &out_cache;
    int *points_low = static_cast<int *>(
        mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0));
    assert(points_low != MAP_FAILED);
    memcpy(points_low, points, sizeof(points));
    texture.sub_620a10(&buffer, static_cast<int>(reinterpret_cast<intptr_t>(points_low)), 4,
                       static_cast<int>(reinterpret_cast<intptr_t>(points_low)), &out_cache);
    assert(buffer.frees_ == 1 && fpu_resets == 10);
    assert(out_cache.next_record_ == 0 && out_cache.flags_ == 1);
    for (int y = 0; y < 3; ++y) {
        assert(surface[y * 4] == 77); // 0x620a10 keys in owned and borrowed modes
        assert(surface[y * 4 + 1] == 5 + y * 10);
        assert(surface[y * 4 + 2] == 10 + y * 10);
        assert(surface[y * 4 + 3] == 255);
    }
    assert(edge_helper_calls == 2);
    assert(edge_directions == 3u);
    assert(edge_helper_direction[0] == -1 && edge_helper_direction[1] == 1);
    assert(edge_helper_index[0] == 0 && edge_helper_index[1] == 0);

    memset(surface, 77, 16);
    buffer.frees_ = 0;
    texture.borrowed_ = 1;
    texture.pixels_ = borrowed_source;
    texture.sub_620a10(&buffer, static_cast<int>(reinterpret_cast<intptr_t>(points_low)), 4,
                       static_cast<int>(reinterpret_cast<intptr_t>(points_low)), &out_cache);
    assert(buffer.frees_ == 1 && fpu_resets == 12);
    assert(edge_helper_calls == 4);
    for (int y = 0; y < 3; ++y) {
        assert(surface[y * 4] == 77); // borrowed path compares the texture key
        assert(surface[y * 4 + 1] == 5 + y * 10);
        assert(surface[y * 4 + 2] == 10 + y * 10);
        assert(surface[y * 4 + 3] == 255);
    }

    // A failed left-edge refresh terminates before the right-edge helper call;
    // both calls mutate the shared edge cache cursor in the original.
    memset(surface, 77, 16);
    buffer.frees_ = 0;
    edge_helper_calls = 0;
    edge_directions = 0;
    edge_helper_period = 2;
    edge_helper_fail_at = 3;
    texture.sub_620a10(&buffer, static_cast<int>(reinterpret_cast<intptr_t>(points_low)), 4,
                       static_cast<int>(reinterpret_cast<intptr_t>(points_low)), &out_cache);
    assert(buffer.frees_ == 1 && edge_helper_calls == 3 && fpu_resets == 14);
    assert(edge_helper_direction[0] == -1 && edge_helper_direction[1] == 1 &&
           edge_helper_direction[2] == -1);
    edge_helper_fail_at = 0;
    munmap(points_low, 4096);
    munmap(surface, 4096);
    return 0;
}
