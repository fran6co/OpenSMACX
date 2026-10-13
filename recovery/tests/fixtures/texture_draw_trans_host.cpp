#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef long long __int64;
struct RECT { int left, top, right, bottom; };
struct Vert { int x, y; };
struct EdgeScan {
    int f0, counter, id, accumA, accumB, deltaA, deltaB, accumC, deltaC,
        extraDeltaC, fracAccum, fracDelta, fracSub, accumD, deltaD;
};

// TEXTURE_DIRECT_STORAGE_PRELUDE

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
    void draw_trans(Buffer *, Vert *, int *, int, Vert *);
};

static int TextureLightCeiling = 0x70000000;
static int TextureLightFloor = -0x70000000;
static int setup_calls;
static int setup_indices[8];
static int setup_directions[8];
static int fpu_resets;

static int texture_difference(int a, int b) {
    return static_cast<int>(static_cast<unsigned int>(a) -
                            static_cast<unsigned int>(b));
}
static int texture_span_step(int delta, int width) {
    return static_cast<int>(static_cast<int64_t>(delta) / width);
}
static unsigned short texture_light_step(int delta, int width) {
    return static_cast<unsigned short>((static_cast<int64_t>(delta) << 8) / width);
}
static unsigned int texture_span_pixels(int difference) {
    const unsigned int pixels = static_cast<unsigned int>(difference) & 0xffffu;
    return pixels - 1u < 0x8001u ? pixels : 1u;
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
static void texture_reset_fpu() { ++fpu_resets; }
#define _PC_24 0x00020000
#define _RC_CHOP 0x00000300
#define _MCW_PC 0x00030000
#define _MCW_RC 0x00000300
static unsigned int _control87(unsigned int, unsigned int) { return 0; }

int Texture::setup_edge_gouraud(EdgeScan *edge, int index) {
    const int call = setup_calls++;
    assert(call < 8);
    setup_indices[call] = index;
    setup_directions[call] = edge->f0;
    assert(TextureRasterMaxYIndex == 1);
    assert(TextureRasterZValues != 0);
    if (call == 4) return 0; // left refresh failure suppresses right refresh

    edge->counter = 1;
    edge->id = 1;
    edge->deltaA = edge->deltaB = edge->deltaC = edge->deltaD = 0;
    edge->fracAccum = edge->fracDelta = edge->fracSub = edge->extraDeltaC = 0;
    if (call == 0) {
        edge->accumC = -1;
        edge->accumA = 1 << 16;
        edge->accumB = 1 << 16;
        edge->accumD = 0x00010000;
    } else if (call == 1) {
        edge->accumC = 5;
        edge->accumA = 7 << 16;
        edge->accumB = 7 << 16;
        edge->accumD = 0x00010000;
    } else if (call == 2) {
        edge->accumC = 0;
        edge->accumA = 2 << 16;
        edge->accumB = 1 << 16;
        edge->accumD = 0x00020000;
    } else {
        edge->accumC = 4;
        edge->accumA = 6 << 16;
        edge->accumB = 1 << 16;
        edge->accumD = 0x00020000;
    }
    return 1;
}

// ACTUAL_HELPER_INSERTION
// ACTUAL_METHOD_INSERTION

int main() {
    // High-word coordinates are zero-extended by the native MOV DX path.
    // Check the wrapped linear offset without dereferencing that address.
    assert(texture_draw_source_index(8,
        static_cast<int>(0x80000000u), 0) == 0x8000u);
    assert(texture_draw_source_index(8, 0,
        static_cast<int>(0x80000000u)) == 0x40000u);

    static unsigned char pixels[8 * 8];
    for (int i = 0; i < 8 * 8; ++i) pixels[i] = static_cast<unsigned char>(i + 10);
    pixels[27] = 40; // row 0 starts at U=V=1, clipped left by two pixels
    pixels[36] = 99; // keyed texel leaves destination untouched
    pixels[45] = 50;
    pixels[11] = 60; // row 1 has a distinct V and starts after one clipped pixel
    pixels[12] = 70;
    pixels[13] = 80;

    static Vert vertices[3] = {{0, 0}, {4, 2}, {0, 0}};
    static Vert uv[3] = {{0, 0}, {0, 0}, {0, 0}};
    static int zvals[3] = {0, 0, 0};
    static unsigned char storage[8 * 4];
    memset(storage, 77, sizeof(storage));
    surface = storage;
    TextureRasterKey = 99;
    setup_calls = fpu_resets = 0;

    Buffer buffer = {};
    buffer.rect1_ = RECT{1, 0, 4, 2};
    buffer.stride_ = 8;
    Texture texture = {};
    texture.pixels_ = pixels;
    texture.iWidth_ = 8;
    texture.iHeight_ = 8;
    texture.draw_trans(&buffer, vertices, zvals, 3, uv);


    assert(buffer.frees == 1 && fpu_resets == 2);
    assert(setup_calls == 5);
    assert(setup_indices[0] == 0 && setup_indices[1] == 0);
    assert(setup_directions[0] == -1 && setup_directions[1] == 1);
    assert(setup_indices[2] == 1 && setup_indices[3] == 1 &&
           setup_indices[4] == 1);
    assert(setup_directions[2] == -1 && setup_directions[3] == 1 &&
           setup_directions[4] == -1);
    // Clipped row 0 proves both U and V sampling; the middle keyed texel is
    // not written, and light adds one to each non-key source texel.
    assert(storage[1] == 41 && storage[2] == 77 && storage[3] == 51);
    // Row 1 uses the refreshed edge endpoints and a different V coordinate.
    assert(storage[8 + 1] == 62 && storage[8 + 2] == 72 &&
           storage[8 + 3] == 82);
    assert(storage[0] == 77 && storage[4] == 77);
    assert(storage[8] == 77 && storage[12] == 77);
    return 0;
}
