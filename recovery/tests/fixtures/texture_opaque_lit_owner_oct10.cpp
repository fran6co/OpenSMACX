#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

typedef long long __int64;
struct RECT { int left, top, right, bottom; };
// ACTUAL_VERTEX_AND_EDGE_DECLARATIONS

static unsigned char *surface;
static int events[32];
static int event_count;
class Buffer {
public:
    RECT rect1_;
    unsigned int stride_;
    int frees;
    void *get_data() { events[event_count++] = 1; return surface; }
    void free_data(int value) {
        assert(value == 1);
        events[event_count++] = 4;
        ++frees;
    }
};

class Texture {
public:
    unsigned char *pixels_;
    unsigned int iWidth_;
    static int setup_edge_gouraud(EdgeScan *, int);
    void draw(Buffer *, Vert *, int *, int, Vert *);
};

static int slots[64];
static void *pixel_base_cell;
static void **blit_pixel_base = &pixel_base_cell;
static Vert *uv_a_cell = 0;
static Vert **blit_edge_uvs_a = &uv_a_cell;
static Vert *uv_b_cell = 0;
static Vert **blit_edge_uvs_b = &uv_b_cell;
static int *blit_vertex_count = &slots[3];
static int *z_values_cell = 0;
static int **blit_z_values = &z_values_cell;
static void *texture_pixels_cell;
static void **blit_texture_pixels = &texture_pixels_cell;
static int *blit_buffer_stride = &slots[6];
static int *blit_texture_width = &slots[7];
static int *blit_clip_right = &slots[8];
static int *blit_clip_left = &slots[9];
static int *blit_clip_bottom = &slots[10];
static int *blit_clip_top = &slots[11];
static int *blit_max_y_index = &slots[12];
static int *blit_row_offset = &slots[13];
static int *blit_slot_4e4 = &slots[14];
static int *blit_slot_4c8 = &slots[15];
static unsigned short light_pack_words[2] = {};
static unsigned short *blit_light_pack_a = &light_pack_words[0];
static int TextureLightCeiling = 256;
static int TextureLightFloor = -256;
static int raster_threshold = 91;
static int *blit_light_threshold_2 = &raster_threshold;
static int *blit_light_threshold = &slots[17];
static int setup_calls;
static int setup_indices[8];
static int setup_directions[8];
static int fail_right_initial;
static int multirow_step_case;
static int high_light_step_case;
static int capture_span_light;
static int span_light_observed[8];
static int span_light_observed_count;
static int fpu_resets;
static int control_calls;

static void texture_reset_fpu() {
    events[event_count++] = 2;
    ++fpu_resets;
}
static uintptr_t texture_pointer_address(const void *pointer) {
    return reinterpret_cast<uintptr_t>(pointer);
}
#define _PC_24 0x00020000
#define _RC_CHOP 0x00000300
#define _MCW_PC 0x00030000
#define _MCW_RC 0x00000300
static unsigned int _control87(unsigned int value, unsigned int mask) {
    assert(value == (_PC_24 | _RC_CHOP));
    assert(mask == (_MCW_PC | _MCW_RC));
    events[event_count++] = 3;
    ++control_calls;
    return 0;
}

int Texture::setup_edge_gouraud(EdgeScan *edge, int index) {
    const int call = setup_calls++;
    assert(call < 8);
    assert(pixel_base_cell == surface);
    assert(*blit_light_threshold == raster_threshold);
    assert(*blit_vertex_count == 3);
    assert(*blit_max_y_index == 1);
    assert(reinterpret_cast<int *>(*blit_z_values) != 0);
    setup_indices[call] = index;
    setup_directions[call] = edge->f0;
    if ((call == 1) && fail_right_initial) return 0;
    if (call == 4) return 0; // left refresh failure suppresses right refresh

    edge->counter = (multirow_step_case && call < 2) ? 2 : 1;
    edge->id = 1;
    edge->deltaA = edge->deltaB = edge->deltaC = edge->deltaD = 0;
    edge->fracAccum = edge->fracDelta = edge->fracSub = edge->extraDeltaC = 0;
    if (multirow_step_case && call < 2) {
        edge->deltaA = 1 << 16;
        edge->deltaC = 1;
    }
    if (call == 0) {
        edge->accumC = 1;
        edge->accumA = 1 << 16;
        edge->accumB = 1 << 16;
        edge->accumD = 1 << 16;
    } else if (call == 1) {
        edge->accumC = 5;
        edge->accumA = 3 << 16;
        edge->accumB = 1 << 16;
        edge->accumD = high_light_step_case ? 0x02010000 : 1 << 16;
    } else if (call == 2) {
        edge->accumC = 2;
        edge->accumA = 2 << 16;
        edge->accumB = 1 << 16;
        edge->accumD = 1 << 16;
    } else {
        edge->accumC = 6;
        edge->accumA = 4 << 16;
        edge->accumB = 1 << 16;
        edge->accumD = high_light_step_case ? 0x02010000 : 1 << 16;
    }
    return 1;
}

// ACTUAL_HELPER_INSERTION
// ACTUAL_METHOD_INSERTION
// ACTUAL_MAIN_INSERTION
