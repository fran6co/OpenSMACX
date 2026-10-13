#include <cassert>
#include <cstdint>
#include <cstring>

typedef long long __int64;
typedef unsigned short ushort;
typedef unsigned char byte;
static int ptr32(const void *pointer) {
    const uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
    assert(address <= 0xffffffffu);
    return static_cast<int>(address);
}
static const unsigned int _PC_24 = 0x0000;
static const unsigned int _PC_64 = 0x0200;
static const unsigned int _RC_CHOP = 0x0c00;
static const unsigned int _RC_NEAR = 0x0000;
static const unsigned int _MCW_PC = 0x0300;
static const unsigned int _MCW_RC = 0x0c00;
static unsigned int _control87(unsigned int, unsigned int) { return 0; }
static void texture_reset_fpu() {}

struct Vert { int x; int y; };
struct EdgeScan {
    int f0, counter, id, accumA, accumB, deltaA, deltaB, accumC, deltaC;
    int extraDeltaC, fracAccum, fracDelta, fracSub, accumD, deltaD;
};
static_assert(sizeof(EdgeScan) == 0x3c, "native EdgeScan width");

// TEXTURE_DIRECT_STORAGE_PRELUDE

static void *&pixel_base = TextureRasterPixelBase;
static int &slot494 = TextureRasterScratch494;
static Vert *&edge_uvs_a = TextureRasterEdgeVerticesA;
static int &slot498 = TextureRasterScratch498;
static int &buffer_stride = TextureRasterBufferStride;
static int &texture_width = TextureRasterTextureWidth;
static int &texture_height = TextureRasterTextureHeight;
static int &max_y_index = TextureRasterMaxYIndex;
static int &vertex_count = TextureRasterVertexCount;
static int &clip_left = TextureRasterClipLeft;
static int &clip_top = TextureRasterClipTop;
static int &clip_right = TextureRasterClipRight;
static int &clip_bottom = TextureRasterClipBottom;
static int &slot4c4 = TextureRasterScratch4C4;
static int &slot4c8 = TextureRasterScratch4C8.dword;
static int &span_right = TextureRasterSpanRight;
static int &slot4e4 = TextureRasterScratch4E4.dword;
static int &span_left = TextureRasterSpanLeft;
static int &slot4a4 = TextureRasterScratch4A4;
static int &slot504 = TextureRasterScratch504;
static Vert *&edge_uvs_b = TextureRasterEdgeVerticesB;
static int *&z_values_address = TextureRasterZValues;
static int &slot4f8 = TextureRasterUAccumulator.dword;
static int &slot530 = TextureRasterScratch530;
static int &row_offset = TextureRasterRowOffset;
static int &slot528 = TextureRasterScratch528;
static unsigned short &light_pack_a = TextureRasterLightPackA.low_word;
static int &slot53c = TextureRasterVAccumulator.dword;
static void *&texture_pixels = TextureRasterScratch544;
static const void *&remap_table = TextureRasterScratch550;
static void *&shade_table = TextureRasterScratch554;
static uintptr_t &slot558 = TextureRasterShadeCursor;
static int &dither = TextureRasterDither;
static const float &scale_65536 = TextureRasterScale65536;
static const float &scale_256 = TextureRasterScale256;

struct RECT { int left, top, right, bottom; };
struct Buffer {
    unsigned char prefix[0x20];
    RECT rect1_;
    unsigned char reserved[0x4a8 - 0x30];
    unsigned int stride_;
    unsigned char *pixels;
    int freed;
    unsigned char *get_data() { return pixels; }
    void free_data(int) { ++freed; }
};

static int setup_calls;
static int setup_selectors[16];
struct Texture {
    unsigned char *pixels_;
    uint32_t iWidth_;
    uint32_t iHeight_;
    void draw_coast(Buffer *, Vert *, int *, Texture *, unsigned char *, int, Vert *);
    void draw_coast(Buffer *, Vert *, int *, Texture *, unsigned char *, int, Vert *, int);
    int setup_edge_gouraud(EdgeScan *, int);
};

void Texture::draw_coast(Buffer *, Vert *, int *, Texture *, unsigned char *, int, Vert *) {}
int Texture::setup_edge_gouraud(EdgeScan *edge, int selector) {
    const int call = setup_calls++;
    setup_selectors[call] = selector;
    if (call == 3) {
        edge->counter = 0;
        edge->id = 2;
        edge->accumA = 0x00038000;
        edge->accumB = 0x00048000;
        edge->accumC = 3;
        edge->accumD = 0x00030000;
        return 0; // right-edge refresh stops after the second row
    }
    if (call == 2) {
        edge->counter = 1;
        edge->id = 0;
        edge->accumA = 0x00008000;
        edge->accumB = 0x00018000;
        edge->accumC = 0;
        edge->accumD = 0x00030000;
        return 1;
    }
    edge->counter = call == 2 ? 1 : 2;
    edge->id = edge->f0 < 0 ? 0 : 2;
    edge->accumA = edge->f0 < 0 ? 0x00008000 : 0x00038000;
    edge->accumB = edge->f0 < 0 ? 0x00008000 : 0x00038000;
    edge->accumC = edge->f0 < 0 ? 0 : 3;
    edge->deltaA = edge->deltaB = edge->deltaC = 0;
    edge->extraDeltaC = edge->fracAccum = edge->fracDelta = edge->fracSub = 0;
    edge->accumD = 0x00020000;
    edge->deltaD = 0x00010000;
    edge->deltaB = 0x00010000;
    return 1;
}

// ACTUAL_RESULT_STRUCT
// ACTUAL_HELPERS
// ACTUAL_METHOD

static void reset_slots() {
    pixel_base = 0;
    slot494 = slot498 = buffer_stride = texture_width = 0;
    texture_height = max_y_index = vertex_count = clip_left = clip_top = 0;
    clip_right = clip_bottom = slot4c4 = slot4c8 = span_right = slot4e4 = 0;
    span_left = slot4f8 = slot530 = row_offset = slot528 = 0;
    light_pack_a = slot53c = 0;
    remap_table = 0;
    texture_pixels = shade_table = 0;
    slot558 = slot4a4 = slot504 = dither = 0;
    TextureRowPitchScratch = 0;
    edge_uvs_a = 0;
    edge_uvs_b = 0;
    z_values_address = 0;
    setup_calls = 0;
    std::memset(setup_selectors, 0, sizeof setup_selectors);
}

int main() {
    static unsigned char destination[16];
    static unsigned char base_pixels[16];
    static unsigned char coast_pixels[16];
    static unsigned char remap[256];
    static int z_values[3] = {0, 0, 0};
    static Vert screen[3] = {{0, 0}, {3, 2}, {0, 2}};
    static Vert uv[3] = {{0, 0}, {0, 0}, {0, 0}};
    static Buffer buffer;
    static Texture texture;
    static Texture shade_owner;
    assert(texture_draw_source_index(
               7, static_cast<int>(0x80010000u),
               static_cast<int>(0x80020000u)) == 0x8002u * 7u + 0x8001u);
    reset_slots();
    std::memset(destination, 0x7e, sizeof destination);
    std::memset(base_pixels, 20, sizeof base_pixels);
    base_pixels[9] = 40;
    std::memset(coast_pixels, 0, sizeof coast_pixels);
    std::memset(remap, 0, sizeof remap);
    remap[1] = 1;
    remap[2] = 7;
    remap[3] = 9;
    coast_pixels[0] = 2;   // clipped even-row texel must not be visited
    coast_pixels[5] = 0;   // even row: transparent
    coast_pixels[10] = 2;  // even row: nonzero fill
    coast_pixels[9] = 3;   // odd row: direct mapped byte
    coast_pixels[14] = 1;  // odd row: uses the unconditional next-source preload
    texture.pixels_ = base_pixels;
    texture.iWidth_ = 4;
    texture.iHeight_ = 4;
    shade_owner.pixels_ = coast_pixels;
    buffer.pixels = destination;
    buffer.freed = 0;
    buffer.rect1_.left = 1;
    buffer.rect1_.top = 0;
    buffer.rect1_.right = 3;
    buffer.rect1_.bottom = 2;
    buffer.stride_ = 8;

    texture.draw_coast(&buffer, screen, z_values, &shade_owner, remap, 3, uv, 0x31);

    assert(destination[0] == 0x7e && destination[3] == 0x7e);
    assert(destination[1] == 0x7e); // mapped zero leaves even-dither destination
    assert(destination[2] == 0x31); // nonzero mapped byte selects flat fill
    assert(destination[9] == 9);    // other mapped byte is written directly
    assert(destination[10] == 23);  // next preload supplies base pixel 14, plus light
    assert(destination[8] == 0x7e && destination[11] == 0x7e);
    assert(destination[0] == 0x7e); // clipped mapped texel is not visited
    assert(slot504 == ptr32(base_pixels + 9));
    assert(static_cast<unsigned int>(slot4a4) == 0x80000000u);
    assert(slot498 == 0x00000300);
    assert(buffer.freed == 1);

    reset_slots();
    buffer.freed = 0;
    screen[0] = Vert{0, 32767};
    screen[1] = Vert{3, 32768};
    buffer.rect1_.top = 32767;
    buffer.rect1_.bottom = 32767;
    texture.draw_coast(&buffer, screen, z_values, &shade_owner, remap, 2, uv, 0x31);
    assert(setup_calls == 2);
    assert(setup_selectors[0] == 0x31 && setup_selectors[1] == 0x31);
    assert(buffer.freed == 1);

    // Preserve the original distinction between a missing shade owner
    // (not guarded here) and a live shade Texture with no pixel storage.
    reset_slots();
    std::memset(destination, 0x7e, sizeof destination);
    buffer.freed = 0;
    shade_owner.pixels_ = 0;
    texture.draw_coast(&buffer, screen, z_values, &shade_owner, remap, 2, uv, 0x31);
    assert(setup_calls == 0 && buffer.freed == 1);
    for (unsigned int i = 0; i < sizeof destination; ++i) assert(destination[i] == 0x7e);
}
