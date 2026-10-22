#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>

#define _PC_24 0x00020000u
#define _RC_CHOP 0x00000300u
#define _MCW_PC 0x00030000u
#define _MCW_RC 0x00000300u
enum { LIFE_GET_DATA = 1, LIFE_FREE_DATA, LIFE_RESET_FPU,
       LIFE_SETUP_GOURAUD, LIFE_CONTROL87 };
static int lifecycle_events[256];
static int lifecycle_count;
static void lifecycle(int event) {
    assert(lifecycle_count < 256);
    lifecycle_events[lifecycle_count++] = event;
}
static void reset_lifecycle() { lifecycle_count = 0; }
static unsigned int host_control_word;
static int control87_calls;
static unsigned int _control87(unsigned int value, unsigned int mask) {
    assert(value == (_PC_24 | _RC_CHOP));
    assert(mask == (_MCW_PC | _MCW_RC));
    lifecycle(LIFE_CONTROL87);
    ++control87_calls;
    const unsigned int old = host_control_word;
    host_control_word = (host_control_word & ~mask) | (value & mask);
    return old;
}

struct RECT { int left, top, right, bottom; };
struct Vert { int x, y; };
struct EdgeScan {
    int f0, counter, id, accumA, accumB, deltaA, deltaB, accumC, deltaC,
        extraDeltaC, fracAccum, fracDelta, fracSub, accumD, deltaD;
};
struct TextureStore {
    unsigned int flags_;
    int next_record_;
    EdgeScan edges_[10];
};

// TEXTURE_DIRECT_STORAGE_PRELUDE
class Buffer {
public:
    RECT rect1_;
    const RECT &current_clip() const { return rect1_; }
    int stride_;
    int frees_;
    void *get_data() { lifecycle(LIFE_GET_DATA); return active_data_; }
    void free_data(int) { lifecycle(LIFE_FREE_DATA); ++frees_; }
    static unsigned char *active_data_;
};
class Texture {
public:
    unsigned char *pixels_;
    uint32_t iWidth_, iHeight_;
    Vert corners_[4];
    Vert cornersAndMids_[8];
    uint32_t borrowed_;
    int setup_edge_gouraud(EdgeScan *, int);
    int setup_edge(EdgeScan *, int);
    void sub_61b780(Buffer *, Vert *, int *, int);
    void sub_620a10(Buffer *, Vert *, int, Vert *, TextureStore *);
    void sub_61d090(Buffer *, Vert *, int *, unsigned int);
};

unsigned char *Buffer::active_data_ = 0;
static int fpu_resets;
static int gouraud_test_light;
static int edge_helper_calls;
static TextureStore edge_cache_sentinel = {};
static TextureStore *expected_edge_cache;
static unsigned int edge_directions;
static int edge_helper_period = 4;
static int edge_helper_fail_at;
static int edge_helper_direction[16];
static int edge_helper_index[16];
static int gouraud_edge_calls;
static int gouraud_edge_period = 4;
static int gouraud_edge_fail_at;
static int gouraud_edge_index[32];
static int gouraud_test_light_delta;
static int gouraud_test_end = 4;
static int gouraud_test_observe_call;
static int gouraud_test_observe_offset;
static int gouraud_test_observed_byte = -1;

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
static void texture_reset_fpu() {
    lifecycle(LIFE_RESET_FPU);
    ++fpu_resets;
}

int Texture::setup_edge_gouraud(EdgeScan *edge, int) {
    lifecycle(LIFE_SETUP_GOURAUD);
    ++gouraud_edge_calls;
    gouraud_edge_index[gouraud_edge_calls - 1] = edge->id;
    if (gouraud_edge_calls == gouraud_test_observe_call) {
        assert(Buffer::active_data_ != 0);
        gouraud_test_observed_byte =
            Buffer::active_data_[gouraud_test_observe_offset];
    }
    edge->counter = gouraud_edge_period;
    edge->id = 0;
    edge->accumA = edge->f0 < 0 ? 0 : 4 << 16;
    edge->accumB = 0;
    edge->accumC = edge->f0 < 0 ? 0 : gouraud_test_end;
    edge->accumD = gouraud_test_light +
        (edge->f0 > 0 ? gouraud_test_light_delta : 0);
    edge->deltaA = edge->deltaB = edge->deltaC = edge->deltaD = 0;
    edge->fracAccum = edge->fracDelta = edge->fracSub = edge->extraDeltaC = 0;
    return gouraud_edge_calls == gouraud_edge_fail_at ? 0 : 1;
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
    Vert points[4] = {{0, 0}, {4, 0}, {4, 3}, {0, 3}};
    int shade[4] = {1, 1, 1, 1};

    TextureRasterKey = 0;
    texture.sub_61b780(&buffer, points, shade, 0);
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
    gouraud_test_light = 0x00008000;
    texture.sub_61b780(&buffer, points, shade, 0);
    assert(buffer.frees_ == 1);
    for (int y = 0; y < 3; ++y) {
        assert(surface[y * 4] == 77); // borrowed path preserves the key-valued destination
        assert(surface[y * 4 + 1] == 133);
        assert(surface[y * 4 + 2] == 138);
        assert(surface[y * 4 + 3] == 127); // byte addition wraps
    }
    gouraud_test_light = 0;
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
    Vert affine_vertices[4], affine_uv[4];
    static_assert(sizeof(affine_vertices) == sizeof(points), "coordinate-pair extent");
    memcpy(affine_vertices, points, sizeof(points));
    memcpy(affine_uv, points, sizeof(points));
    texture.sub_620a10(&buffer, affine_vertices, 4, affine_uv, &out_cache);
    assert(TextureRasterEdgeVerticesB == affine_vertices &&
           TextureRasterEdgeVerticesA == affine_uv);
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
    texture.sub_620a10(&buffer, affine_vertices, 4, affine_uv, &out_cache);
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
    texture.sub_620a10(&buffer, affine_vertices, 4, affine_uv, &out_cache);
    assert(buffer.frees_ == 1 && edge_helper_calls == 3 && fpu_resets == 14);
    assert(edge_helper_direction[0] == -1 && edge_helper_direction[1] == 1 &&
           edge_helper_direction[2] == -1);
    edge_helper_fail_at = 0;

    // The keyed Gouraud body snapshots the borrowed row, refills its edge,
    // then paints that row even when the refill fails. A one-row bottom clip
    // admits just the initial edge pair and this row's left refill.
    memset(surface, 77, 16);
    buffer.rect1_ = RECT{0, 0, 4, 1};
    buffer.frees_ = 0;
    texture.borrowed_ = 1;
    texture.pixels_ = borrowed_source;
    gouraud_test_light = 0x00008000;
    gouraud_edge_period = 1;
    gouraud_edge_calls = 0;
    gouraud_edge_fail_at = 3;
    control87_calls = 0;
    reset_lifecycle();
    texture.sub_61b780(&buffer, points, shade, 0);
    assert(gouraud_edge_calls == 3 && buffer.frees_ == 1);
    assert(control87_calls == 1 && fpu_resets == 16);
    assert(surface[0] == 77 && surface[1] == 133 && surface[2] == 138 &&
           surface[3] == 127);
    assert(lifecycle_count == 8);
    assert(lifecycle_events[0] == LIFE_GET_DATA &&
           lifecycle_events[1] == LIFE_RESET_FPU &&
           lifecycle_events[2] == LIFE_SETUP_GOURAUD &&
           lifecycle_events[3] == LIFE_SETUP_GOURAUD &&
           lifecycle_events[4] == LIFE_CONTROL87 &&
           lifecycle_events[5] == LIFE_SETUP_GOURAUD &&
           lifecycle_events[6] == LIFE_FREE_DATA &&
           lifecycle_events[7] == LIFE_RESET_FPU);

    // Top clipping suppresses stores but does not suppress edge refills.
    // The three scanned rows each perform left/right period-one refreshes.
    memset(surface, 77, 16);
    buffer.rect1_ = RECT{0, 2, 4, 3};
    buffer.frees_ = 0;
    gouraud_edge_calls = 0;
    gouraud_edge_fail_at = 0;
    reset_lifecycle();
    texture.sub_61b780(&buffer, points, shade, 0);
    assert(gouraud_edge_calls == 8 && buffer.frees_ == 1);
    assert(surface[0] == 77 && surface[4] == 77 && surface[8] == 77);
    assert(surface[9] == 133 && surface[10] == 138 && surface[11] == 127);

    // Conversely, once the bottom address is reached no further edge setup
    // is called; the fifth-call failure sentinel must remain untouched.
    memset(surface, 77, 16);
    buffer.rect1_ = RECT{0, 0, 4, 1};
    buffer.frees_ = 0;
    gouraud_edge_calls = 0;
    gouraud_edge_fail_at = 5;
    reset_lifecycle();
    texture.sub_61b780(&buffer, points, shade, 0);
    assert(gouraud_edge_calls == 4 && buffer.frees_ == 1);
    assert(surface[1] == 133 && surface[5] == 77);

    // The mapped 0x61D090 body has distinct borrowed/owned row ordering.
    // Borrowed snapshots then refills before it paints the final admitted row.
    memset(surface, 77, 16);
    buffer.rect1_ = RECT{0, 0, 4, 1};
    buffer.frees_ = 0;
    texture.borrowed_ = 1;
    texture.pixels_ = borrowed_source;
    gouraud_test_light = 0;
    gouraud_edge_period = 1;
    gouraud_edge_calls = 0;
    gouraud_edge_fail_at = 3;
    gouraud_test_observe_call = 3;
    gouraud_test_observe_offset = 1;
    gouraud_test_observed_byte = -1;
    texture.sub_61d090(&buffer, points, shade, 0);
    assert(gouraud_test_observed_byte == 77);
    assert(gouraud_edge_calls == 3 && buffer.frees_ == 1);
    assert(surface[0] == 0 && surface[1] == 5 && surface[2] == 10 &&
           surface[3] == 255);

    // Owned 0x61D090 paints before refilling; the failed refresh observes its
    // first output pixel already committed, unlike the borrowed path above.
    memset(surface, 77, 16);
    buffer.frees_ = 0;
    texture.borrowed_ = 0;
    texture.pixels_ = owned_source;
    gouraud_test_light = 0x00008000;
    gouraud_edge_period = 1;
    gouraud_edge_calls = 0;
    gouraud_edge_fail_at = 3;
    gouraud_test_observe_call = 3;
    gouraud_test_observe_offset = 0;
    gouraud_test_observed_byte = -1;
    texture.sub_61d090(&buffer, points, shade, 0);
    assert(gouraud_test_observed_byte == 128);
    assert(gouraud_edge_calls == 3 && buffer.frees_ == 1);
    assert(surface[0] == 128 && surface[1] == 133 &&
           surface[2] == 138 && surface[3] == 127);

    // Both 0x61D090 branches still run edge callbacks through top-clipped
    // rows, but the bottom address terminates further callbacks.
    texture.borrowed_ = 1;
    texture.pixels_ = borrowed_source;
    gouraud_test_light = 0;
    gouraud_edge_fail_at = 0;
    gouraud_edge_period = 1;
    memset(surface, 77, 16);
    buffer.rect1_ = RECT{0, 2, 4, 3};
    buffer.frees_ = 0;
    gouraud_edge_calls = 0;
    texture.sub_61d090(&buffer, points, shade, 0);
    assert(gouraud_edge_calls == 8 && buffer.frees_ == 1);
    assert(surface[1] == 77 && surface[5] == 77 && surface[9] == 5);

    memset(surface, 77, 16);
    buffer.rect1_ = RECT{0, 0, 4, 1};
    buffer.frees_ = 0;
    gouraud_edge_calls = 0;
    texture.sub_61d090(&buffer, points, shade, 0);
    assert(gouraud_edge_calls == 4 && buffer.frees_ == 1);
    assert(surface[1] == 5 && surface[5] == 77);

    const size_t wide_surface_size = 200000;
    unsigned char *wide_surface = static_cast<unsigned char *>(mmap(
        0, wide_surface_size, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0));
    assert(wide_surface != MAP_FAILED);
    Buffer wide_buffer;
    wide_buffer.stride_ = 100001;
    wide_buffer.frees_ = 0;
    Vert wide_points[4] = {{0, 0}, {100000, 0}, {100000, 1}, {0, 1}};

    // The second mapped owner retains the same full-width clipped skip and
    // 16-bit accumulator carry, independently exercised in both row modes.
    memset(wide_surface, 77, wide_surface_size);
    wide_buffer.rect1_ = RECT{65536, 0, 65537, 1};
    wide_buffer.frees_ = 0;
    Buffer::active_data_ = wide_surface;
    texture.borrowed_ = 1;
    texture.pixels_ = borrowed_source;
    gouraud_test_end = 100000;
    gouraud_edge_calls = 0;
    texture.sub_61d090(&wide_buffer, wide_points, shade, 0);
    assert(wide_buffer.frees_ == 1 && wide_surface[65536] == 10);
    Buffer::active_data_ = surface;
    gouraud_test_end = 4;

    texture.borrowed_ = 1;
    texture.pixels_ = borrowed_source;
    gouraud_test_light = 0x12ff;
    gouraud_test_light_delta = 4;
    memset(surface, 77, 16);
    buffer.frees_ = 0;
    gouraud_edge_period = 4;
    texture.sub_61d090(&buffer, points, shade, 0);
    assert(buffer.frees_ == 1 && surface[0] == 18 && surface[1] == 24 &&
           surface[2] == 29 && surface[3] == 18);

    texture.borrowed_ = 0;
    texture.pixels_ = owned_source;
    memset(surface, 77, 16);
    buffer.frees_ = 0;
    texture.sub_61d090(&buffer, points, shade, 0);
    assert(buffer.frees_ == 1 && surface[0] == 18 && surface[1] == 24 &&
           surface[2] == 29 && surface[3] == 18);
    gouraud_test_light = 0;
    gouraud_test_light_delta = 0;
    gouraud_test_observe_call = 0;

    // Initial setup failure short-circuits its right edge but still performs
    // the method's paired resource/FPU cleanup.
    memset(surface, 77, 16);
    buffer.frees_ = 0;
    gouraud_edge_calls = 0;
    gouraud_edge_fail_at = 1;
    control87_calls = 0;
    reset_lifecycle();
    const int setup_failure_resets_before = fpu_resets;
    texture.sub_61b780(&buffer, points, shade, 0);
    assert(gouraud_edge_calls == 1 && control87_calls == 0);
    assert(buffer.frees_ == 1 && fpu_resets == setup_failure_resets_before + 2);
    assert(surface[1] == 77);
    assert(lifecycle_count == 5 && lifecycle_events[0] == LIFE_GET_DATA &&
           lifecycle_events[1] == LIFE_RESET_FPU &&
           lifecycle_events[2] == LIFE_SETUP_GOURAUD &&
           lifecycle_events[3] == LIFE_FREE_DATA &&
           lifecycle_events[4] == LIFE_RESET_FPU);

    // The owned path paints before its edge refill and intentionally writes
    // the key-valued sample; this is distinct from the borrowed branch above.
    memset(surface, 77, 16);
    buffer.frees_ = 0;
    buffer.rect1_ = RECT{0, 0, 4, 1};
    texture.borrowed_ = 0;
    texture.pixels_ = owned_source;
    gouraud_test_light = 0x00008000;
    gouraud_edge_period = 1;
    gouraud_edge_calls = 0;
    gouraud_edge_fail_at = 3;
    control87_calls = 0;
    reset_lifecycle();
    texture.sub_61b780(&buffer, points, shade, 0);
    assert(gouraud_edge_calls == 3 && control87_calls == 0);
    assert(buffer.frees_ == 1 && surface[0] == 128 && surface[1] == 133 &&
           surface[2] == 138 && surface[3] == 127);
    assert(lifecycle_events[lifecycle_count - 2] == LIFE_FREE_DATA &&
           lifecycle_events[lifecycle_count - 1] == LIFE_RESET_FPU);

    // Eight-vertex selection remains an entry-level mode, not row-helper
    // configuration. Exercise both argument forms through the actual bodies.
    Vert eight_points[8] = {
        {0, 0}, {4, 0}, {4, 1}, {0, 1},
        {0, 2}, {4, 2}, {4, 3}, {0, 3},
    };
    buffer.rect1_ = RECT{0, 0, 4, 3};
    texture.borrowed_ = 0;
    texture.sub_61b780(&buffer, eight_points, shade, 1);
    assert(TextureRasterVertexCount == 8 &&
           TextureRasterEdgeVerticesA == texture.cornersAndMids_);
    texture.sub_61d090(&buffer, eight_points, shade, 1);
    assert(TextureRasterVertexCount == 8 &&
           TextureRasterEdgeVerticesA == texture.cornersAndMids_);

    // The original clip-skip path keeps all 32 bits of the horizontal skip.
    // A 16-bit narrowing would revisit the key texel at x=0 instead of the
    // independently visible source texel at x=2.
    memset(wide_surface, 77, wide_surface_size);
    wide_buffer.rect1_ = RECT{65536, 0, 65537, 1};
    wide_buffer.stride_ = 100001;
    wide_buffer.frees_ = 0;
    Buffer::active_data_ = wide_surface;
    texture.borrowed_ = 1;
    texture.pixels_ = borrowed_source;
    gouraud_test_end = 100000;
    gouraud_test_light = 0;
    gouraud_test_light_delta = 0;
    gouraud_edge_calls = 0;
    gouraud_edge_fail_at = 0;
    wide_buffer.frees_ = 0;
    texture.sub_61b780(&wide_buffer, wide_points, shade, 0);
    assert(wide_buffer.frees_ == 1);
    assert(wide_surface[65536] == 10);
    gouraud_test_end = 4;
    texture.borrowed_ = 0;
    texture.pixels_ = owned_source;
    Buffer::active_data_ = surface;

    // Per-pixel 16-bit low-word accumulation carries from 0x12ff to 0x1300;
    // narrowing that accumulator to one byte changes the next three samples.
    texture.borrowed_ = 0;
    texture.pixels_ = owned_source;
    gouraud_test_light = 0x12ff;
    gouraud_test_light_delta = 4;
    buffer.rect1_ = RECT{0, 0, 4, 1};
    buffer.frees_ = 0;
    gouraud_edge_calls = 0;
    gouraud_edge_fail_at = 0;
    texture.sub_61b780(&buffer, points, shade, 0);
    assert(buffer.frees_ == 1);
    assert(surface[0] == 18 && surface[1] == 24 &&
           surface[2] == 29 && surface[3] == 18);
    gouraud_test_light = 0;
    gouraud_test_light_delta = 0;
    munmap(wide_surface, wide_surface_size);
    munmap(surface, 4096);
    return 0;
}
