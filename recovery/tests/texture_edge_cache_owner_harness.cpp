#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <type_traits>

#ifndef __cdecl
#define __cdecl
#endif

// ACTUAL_EDGESCAN_DECLARATION
// ACTUAL_EDGEPOINT_DECLARATION
// ACTUAL_TEXTURESTORE_DECLARATION
// ACTUAL_TEXTURESTORE_LIFETIME

static int raster_max_y;
static int raster_vertex_count;
static int *const blit_max_y_index = &raster_max_y;
static int *const blit_vertex_count = &raster_vertex_count;
static int *raster_edge_a;
static int *raster_edge_b;
static int *raster_z_values;
static int **const blit_edge_uvs_a = &raster_edge_a;
static int **const blit_edge_uvs_b = &raster_edge_b;
static int **const g2_009bb490 = &raster_edge_a;
static int **const g2_009bb4f4 = &raster_edge_b;
static int **const g2_009bb4fc = &raster_z_values;

static int fixed_div(int numerator, int denominator) {
    return static_cast<int>((static_cast<int64_t>(numerator) * 65536) /
                            denominator);
}

// ACTUAL_EDGE_CACHE_HELPERS

class Buffer;
class Texture {
public:
    void sub_620a10(Buffer *, int, int, int, TextureStore *);
    void sub_624ca0(Buffer *, int, int, int, TextureStore *, int);
    int calls;
    Buffer *seen_buffer;
    int seen_vertices;
    int seen_count;
    int seen_edge_vertices;
    TextureStore *seen_cache;
    Texture() : calls(0), seen_buffer(0), seen_vertices(0), seen_count(0),
                seen_edge_vertices(0), seen_cache(0) {}
};

void Texture::sub_620a10(Buffer *buffer, int vertices, int count,
                         int edge_vertices, TextureStore *cache) {
    ++calls;
    seen_buffer = buffer;
    seen_vertices = vertices;
    seen_count = count;
    seen_edge_vertices = edge_vertices;
    seen_cache = cache;
}

// ACTUAL_SENTINEL_WRAPPER

static_assert(std::is_same<decltype(&sub_620100),
                           int (__cdecl *)(EdgeScan *, int)>::value,
              "edge setup receives the named EdgeScan record");
static_assert(std::is_same<decltype(&sub_620260),
                           int (__cdecl *)(EdgeScan *)>::value,
              "edge stepping receives the named EdgeScan record");
static_assert(std::is_same<decltype(&sub_624eb0),
                           int (__cdecl *)(EdgeScan *, int, TextureStore *)>::value,
              "cached setup receives its record and owning cache");
static_assert(sizeof(EdgeScan) == 0x3c, "edge record stride");
static_assert(sizeof(TextureStore) == 0x260, "cache object stride");
static_assert(offsetof(TextureStore, flags_) == 0, "cache flags offset");
static_assert(offsetof(TextureStore, next_record_) == 4, "cache cursor offset");
static_assert(offsetof(TextureStore, edges_) == 8, "cache records offset");

static EdgePoint geometry[3];
static EdgePoint uv[3];
static int lights[3];

static void install_scene() {
    raster_vertex_count = 3;
    raster_max_y = 2;
    raster_edge_a = reinterpret_cast<int *>(uv);
    raster_edge_b = reinterpret_cast<int *>(geometry);
    raster_z_values = lights;
    geometry[0] = EdgePoint{8, 0};
    geometry[1] = EdgePoint{1, 4};
    geometry[2] = EdgePoint{6, 8};
    uv[0] = EdgePoint{2, 3};
    uv[1] = EdgePoint{10, 11};
    uv[2] = EdgePoint{18, 19};
    lights[0] = 10;
    lights[1] = 14;
    lights[2] = 18;
}

static void poison_edge(EdgeScan &edge, unsigned char value) {
    memset(&edge, value, sizeof(edge));
}

static void test_plain_setup_cache_replay_and_wrap() {
    install_scene();
    TextureStore cache;
    memset(cache.edges_, 0x51, sizeof(cache.edges_));
    cache.flags_ = 1; // light channel valid; geometry must be replayed
    cache.next_record_ = 9;
    cache.edges_[9].deltaA = 0x12345;
    cache.edges_[9].deltaB = -0x23456;
    cache.edges_[9].deltaC = -7;
    cache.edges_[9].fracDelta = 5;
    EdgeScan record_before[10];
    memcpy(record_before, cache.edges_, sizeof(record_before));
    EdgeScan edge;
    poison_edge(edge, 0x33);
    edge.f0 = 1;
    edge.accumD = 0x11223344;
    edge.deltaD = 0x55667788;

    assert(sub_624eb0(&edge, 0, &cache) == 1);
    assert(cache.flags_ == 1);
    assert(cache.next_record_ == 0); // tenth entry wraps after selecting slot 9
    assert(edge.f0 == 1 && edge.counter == 4 && edge.id == 1);
    assert(edge.accumA == (2 << 16) && edge.accumB == (3 << 16));
    assert(edge.deltaA == 0x12345 && edge.deltaB == -0x23456);
    assert(edge.accumC == 8 && edge.deltaC == -7);
    assert(edge.extraDeltaC == -1 && edge.fracAccum == -3);
    assert(edge.fracDelta == 5 && edge.fracSub == 4);
    assert(edge.accumD == 0x11223344 && edge.deltaD == 0x55667788);
    assert(memcmp(cache.edges_, record_before, sizeof(record_before)) == 0);
}

static void test_plain_setup_cache_fill_and_partial_write() {
    install_scene();
    TextureStore cache;
    memset(cache.edges_, 0x6a, sizeof(cache.edges_));
    cache.flags_ = 3; // geometry cache is enabled; unrelated flag remains set
    cache.next_record_ = 3;
    EdgeScan before[10];
    memcpy(before, cache.edges_, sizeof(before));
    EdgeScan edge;
    poison_edge(edge, 0x7b);
    edge.f0 = 1;
    edge.accumD = 0x11112222;
    edge.deltaD = 0x33334444;

    assert(sub_624eb0(&edge, 0, &cache) == 1);
    assert(cache.flags_ == 3 && cache.next_record_ == 4);
    assert(edge.counter == 4 && edge.id == 1);
    assert(edge.accumA == (2 << 16) && edge.accumB == (3 << 16));
    assert(edge.deltaA == (2 << 16) && edge.deltaB == (2 << 16));
    assert(edge.accumC == 8 && edge.deltaC == -1);
    assert(edge.extraDeltaC == -1 && edge.fracAccum == -3);
    assert(edge.fracDelta == 3 && edge.fracSub == 4);
    assert(edge.accumD == 0x11112222 && edge.deltaD == 0x33334444);
    assert(cache.edges_[3].deltaA == edge.deltaA);
    assert(cache.edges_[3].deltaB == edge.deltaB);
    assert(cache.edges_[3].deltaC == edge.deltaC);
    assert(cache.edges_[3].fracDelta == edge.fracDelta);
    assert(cache.edges_[3].deltaD == before[3].deltaD);
    assert(cache.edges_[3].accumA == before[3].accumA);
    for (int i = 0; i < 10; ++i) {
        if (i != 3) assert(memcmp(&cache.edges_[i], &before[i], sizeof(EdgeScan)) == 0);
    }
}

static void test_horizontal_and_descending_edges_preserve_cache() {
    install_scene();
    TextureStore cache;
    memset(cache.edges_, 0x29, sizeof(cache.edges_));
    cache.flags_ = 2;
    cache.next_record_ = 6;
    EdgeScan saved[10];
    memcpy(saved, cache.edges_, sizeof(saved));

    EdgeScan edge;
    poison_edge(edge, 0x44);
    edge.f0 = 1;
    EdgeScan before = edge;
    geometry[0].b = geometry[1].b = geometry[2].b = 0;
    assert(sub_624eb0(&edge, 0, &cache) == 0);
    assert(edge.counter == 0 && edge.f0 == before.f0);
    before.counter = 0;
    assert(memcmp(&edge, &before, sizeof(edge)) == 0);
    assert(cache.next_record_ == 6 && cache.flags_ == 2);
    assert(memcmp(cache.edges_, saved, sizeof(saved)) == 0);

    install_scene();
    edge.f0 = -1;
    before = edge;
    assert(sub_624eb0(&edge, 1, &cache) == 0);
    assert(edge.counter == -4);
    before.counter = -4;
    assert(memcmp(&edge, &before, sizeof(edge)) == 0);
    assert(cache.next_record_ == 6 && cache.flags_ == 2);
    assert(memcmp(cache.edges_, saved, sizeof(saved)) == 0);
}

static void test_shared_edge_setup_and_step_fields() {
    install_scene();
    EdgeScan edge;
    poison_edge(edge, 0x17);
    edge.f0 = 1;
    assert(sub_620100(&edge, 0) == 1);
    assert(edge.counter == 4 && edge.id == 1);
    assert(edge.accumA == (2 << 16) && edge.accumB == (3 << 16));
    assert(edge.deltaA == (2 << 16) && edge.deltaB == (2 << 16));
    assert(edge.accumC == 8 && edge.deltaC == -1);
    assert(edge.extraDeltaC == -1 && edge.fracAccum == -3);
    assert(edge.fracDelta == 3 && edge.fracSub == 4);
    assert(edge.accumD == (10 << 16) && edge.deltaD == (1 << 16));

    edge.counter = 2;
    edge.accumA = 100; edge.deltaA = 2;
    edge.accumB = 200; edge.deltaB = 3;
    edge.accumC = 300; edge.deltaC = 4;
    edge.accumD = 400; edge.deltaD = 5;
    edge.extraDeltaC = -1;
    edge.fracAccum = 1; edge.fracDelta = 2; edge.fracSub = 3;
    assert(sub_620260(&edge) == 1);
    assert(edge.counter == 1);
    assert(edge.accumA == 102 && edge.accumB == 203);
    assert(edge.accumC == 303); // 304 then negative-wrap correction -1
    assert(edge.accumD == 405 && edge.fracAccum == 0);

    edge.f0 = 1;
    edge.counter = 1;
    edge.id = 0;
    assert(sub_620260(&edge) == 1); // refresh dispatches through sub_620100
    assert(edge.counter == 4 && edge.id == 1 && edge.accumC == 8);
}

static void test_sentinel_wrapper_only_forwards_minus_one() {
    Texture texture;
    Buffer *buffer = reinterpret_cast<Buffer *>(static_cast<uintptr_t>(0x1234));
    TextureStore cache;
    texture.sub_624ca0(buffer, 11, 12, 13, &cache, 0);
    texture.sub_624ca0(buffer, 11, 12, 13, &cache, 7);
    assert(texture.calls == 0);
    texture.sub_624ca0(buffer, 11, 12, 13, &cache, -1);
    assert(texture.calls == 1);
    assert(texture.seen_buffer == buffer && texture.seen_vertices == 11);
    assert(texture.seen_count == 12 && texture.seen_edge_vertices == 13);
    assert(texture.seen_cache == &cache);
}

int main() {
    test_plain_setup_cache_replay_and_wrap();
    test_plain_setup_cache_fill_and_partial_write();
    test_horizontal_and_descending_edges_preserve_cache();
    test_shared_edge_setup_and_step_fields();
    test_sentinel_wrapper_only_forwards_minus_one();
    return 0;
}
