#include <cstdlib>

typedef int HDC;
typedef unsigned int UINT;
typedef int BOOL;
typedef unsigned int DWORD;
static const DWORD SRCCOPY = 0x00CC0020U;

enum Event {
    BUFFER_SYNC = 1,
    WIN_SYNC,
    PALETTE_SET,
    WIN_GET,
    BUFFER_GET,
    WIN_RELEASE,
    BUFFER_RELEASE,
    BITBLT
};

static int events[64];
static int event_count;
static int window_dc;
static int buffer_dc;
static int palette_set_count;
static int blit_y[512];
static int blit_count;
static int failure_count;

static void event(Event value) {
    if (event_count >= 64) std::exit(91);
    events[event_count++] = value;
}

struct Palette {
    int set() { event(PALETTE_SET); ++palette_set_count; return 0; }
};

Palette *PaletteActive;

struct DibHeader {
    int biWidth;
    int biHeight;
};
struct Dib {
    DibHeader bmiHeader;
};

struct Buffer {
    Dib dib_;
    void sync_to_palette(Palette *) { event(BUFFER_SYNC); }
    HDC get_hdc() { event(BUFFER_GET); return buffer_dc; }
    void release_hdc(int count) {
        event(BUFFER_RELEASE);
        if (count != 1) std::exit(92);
    }
};

struct Win {
    void sync_palette() { event(WIN_SYNC); }
    static HDC get_hdc() { event(WIN_GET); return window_dc; }
    static void release_hdc() { event(WIN_RELEASE); }
};

struct GraphicWin : Win, Buffer {
    void blit_palette_pattern_rows();
    void blit_palette_pattern_rows_entry();
};

static unsigned int bit_count_signed(int value) {
    unsigned int count = 0;
    if (value != 0) {
        do { value >>= 1; ++count; } while (value != 0);
    }
    return count;
}

// Deterministic synthetic data for the host harness. The generated include
// used by product source is intentionally not copied into this fixture.
static unsigned int GraphicWinBlitPolynomialWords[33] = {
    1U, 2U, 3U, 6U
};

static BOOL BitBlt(HDC dest, int dest_x, int y, int width, int height,
                   HDC source, int source_x, int source_y, DWORD raster_op) {
    if (dest != window_dc || source != buffer_dc || dest_x != 0
        || source_x != 0 || source_y != y || width != 7 || height != 1
        || raster_op != SRCCOPY) std::exit(94);
    event(BITBLT);
    if (blit_count >= 512) std::exit(93);
    blit_y[blit_count++] = y;
    return 1;
}

/* PRODUCT DEFINITIONS */

static void reset(int win_dc, int buf_dc) {
    event_count = 0;
    palette_set_count = 0;
    blit_count = 0;
    failure_count = 0;
    window_dc = win_dc;
    buffer_dc = buf_dc;
}

static void record_expected(const int *expected, int count) {
    if (event_count != count) std::exit(10);
    for (int i = 0; i < count; ++i) {
        if (events[i] != expected[i]) std::exit(11);
    }
}

int main() {
    Palette palette;
    PaletteActive = &palette;

    // First acquisition failure preserves palette work but acquires no
    // Buffer HDC and performs no cleanup call.
    {
        GraphicWin object;
        object.dib_.bmiHeader.biWidth = 7;
        object.dib_.bmiHeader.biHeight = -4;
        reset(0, 22);
        object.blit_palette_pattern_rows();
        const int expected[] = {BUFFER_SYNC, WIN_SYNC, PALETTE_SET, WIN_GET};
        record_expected(expected, 4);
        if (blit_count != 0 || palette_set_count != 1) return 1;
    }

    // Buffer acquisition failure releases only the already-held window DC.
    {
        GraphicWin object;
        object.dib_.bmiHeader.biWidth = 7;
        object.dib_.bmiHeader.biHeight = -4;
        reset(11, 0);
        object.blit_palette_pattern_rows();
        const int expected[] = {BUFFER_SYNC, WIN_SYNC, PALETTE_SET, WIN_GET,
                                BUFFER_GET, WIN_RELEASE};
        record_expected(expected, 6);
        if (blit_count != 0) return 2;
    }

    // Height four selects polynomial[3] == 6 and yields the native sequence
    // 1,0,6,5,2,7,3,1; signed y values below four are blitted.
    {
        GraphicWin object;
        object.dib_.bmiHeader.biWidth = 7;
        object.dib_.bmiHeader.biHeight = -4;
        reset(11, 22);
        object.blit_palette_pattern_rows_entry();
        const int expected[] = {BUFFER_SYNC, WIN_SYNC, PALETTE_SET, WIN_GET,
                                BUFFER_GET, BITBLT, BITBLT, BITBLT, BITBLT,
                                WIN_RELEASE, BUFFER_RELEASE};
        record_expected(expected, 11);
        if (blit_count != 4 || blit_y[0] != 1 || blit_y[1] != 0
            || blit_y[2] != 2 || blit_y[3] != 3 || palette_set_count != 1)
            return 3;
    }

    return failure_count;
}
