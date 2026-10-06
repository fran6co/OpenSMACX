// Behavioral boundary models only: product algorithms are included unchanged.
// Native SDK layout/link/ABI verification remains the coordinator's job.
#include <cassert>
#include <climits>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#define __cdecl
#define CALLBACK
typedef unsigned int DWORD;
typedef int HRESULT;
typedef int BOOL;
typedef char *LPSTR;
typedef void *LPVOID;
typedef void *HDC;
typedef void *HWND;
struct GUID { unsigned char bytes[16]; };
struct RECT { int left, top, right, bottom; };
struct RGNDATAHEADER { DWORD dwSize, iType, nCount, nRgnSize; RECT rcBound; };
struct RGNDATA { RGNDATAHEADER rdh; char Buffer[1]; };
struct DDPIXELFORMAT { DWORD dwSize, dwFlags, dwRGBBitCount; };
struct DDSCAPS { DWORD dwCaps; };
struct DDSURFACEDESC {
    DWORD dwSize, dwFlags, dwHeight, dwWidth;
    int lPitch;
    DWORD dwBackBufferCount;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS ddsCaps;
    DWORD untouched_witness;
};
typedef DDSURFACEDESC *LPDDSURFACEDESC;
struct DDCAPS { DWORD dwSize, dwVidMemFree, untouched_witness; };
struct PALETTEENTRY { unsigned char red, green, blue, flags; };
struct Palette {
    PALETTEENTRY colors[256];
    const PALETTEENTRY *entries() const { return colors; }
};
struct IDirectDrawPalette {};
struct IDirectDrawClipper {
    int id;
    HRESULT SetClipList(RGNDATA *, DWORD);
};
struct IDirectDrawSurface2 {
    int id;
    HRESULT SetPalette(IDirectDrawPalette *);
    HRESULT SetClipper(IDirectDrawClipper *);
    HRESULT GetDC(HDC *);
    HRESULT ReleaseDC(HDC);
};
struct IDirectDrawSurface {
    int id;
    HRESULT GetAttachedSurface(DDSCAPS *, IDirectDrawSurface **);
    HRESULT QueryInterface(const GUID &, void **);
};
struct IDirectDraw2 {
    int id;
    DWORD Release();
    HRESULT GetDisplayMode(DDSURFACEDESC *);
    HRESULT GetCaps(DDCAPS *, DDCAPS *);
    HRESULT EnumDisplayModes(DWORD, DDSURFACEDESC *, LPVOID,
                            HRESULT (*)(LPDDSURFACEDESC, LPVOID));
    HRESULT SetCooperativeLevel(HWND, DWORD);
    HRESULT SetDisplayMode(DWORD, DWORD, DWORD, DWORD, DWORD);
    HRESULT CreateSurface(DDSURFACEDESC *, IDirectDrawSurface **, LPVOID);
    HRESULT CreatePalette(DWORD, PALETTEENTRY *, IDirectDrawPalette **, LPVOID);
    HRESULT CreateClipper(DWORD, IDirectDrawClipper **, LPVOID);
};
struct IDirectDraw {
    HRESULT QueryInterface(const GUID &, void **);
    DWORD Release();
};
struct Buffer {
    IDirectDrawSurface *surface_;
    IDirectDrawClipper *clipper_;
    LPVOID locked_bits_;
    HDC hdc2_;
    void close();
};
struct Dialogs { int item(LPSTR, int); };
struct BasePop {
    Dialogs dialogs_;
    BasePop();
    ~BasePop();
    int init(int, long);
    int exec(int, int (*)());
    Dialogs &dialogs() { return dialogs_; }
};

DDCAPS DirectDrawEmulationCaps, DirectDrawDriverCaps;
int DirectDrawDeviceCount, DDActive, DirectDrawSelectionCancelled, WinHdcRefCount;
GUID *DirectDrawDeviceGuids[10];
DWORD DirectDrawDensityLimit;
HDC DirectDrawHdc;
IDirectDrawClipper *DirectDrawClipper;
IDirectDraw2 *BufferDirectDraw;
IDirectDrawSurface2 *DirectDrawSurface, *DirectDrawBackBuffer;
IDirectDrawPalette *DirectDrawPalette;
RECT DirectDrawClipRect;
Buffer ScreenBuffer;
Palette *PaletteActive;
HWND HandleMain;
GUID IID_IDirectDraw2, IID_IDirectDrawSurface2;

void *mem_get(int);
HRESULT DirectDrawCreate(GUID *, IDirectDraw **, LPVOID);
HRESULT DirectDrawEnumerateA(BOOL (*)(GUID *, LPSTR, LPSTR, LPVOID), LPVOID);
BOOL IntersectRect(RECT *, const RECT *, const RECT *);
BOOL EqualRect(const RECT *, const RECT *);

/* PRODUCT_DISPLAY_BODIES */

static std::vector<std::string> trace;
static IDirectDraw initial;
static IDirectDraw2 device = {1}, device_alt = {2};
static IDirectDrawSurface primary = {1}, back = {2};
static IDirectDrawSurface2 queried_primary = {1}, queried_back = {2}, surface_alt = {3};
static IDirectDrawClipper clipper = {1}, clipper_alt = {2};
static IDirectDrawPalette palette, palette_alt;
static Palette owner_palette;
static GUID guid, allocations[16];
static int allocated_count, selection, fail_stage, mutate, enum_kind, alloc_null;
static int item_count;
static DWORD *retained_width;
static DDSURFACEDESC *retained_description;
static int result_code;
static void log(const char *s) { trace.push_back(s); }
static int failures(int stage) { return fail_stage == stage ? -stage : 0; }
static void reset() {
    trace.clear(); allocated_count = 0; selection = 0; fail_stage = 0;
    mutate = 0; enum_kind = 0; alloc_null = 0; item_count = 0;
    result_code = 123; retained_width = 0; retained_description = 0;
    DirectDrawDeviceCount = 0; DirectDrawSelectionCancelled = 0;
    std::memset(DirectDrawDeviceGuids, 0, sizeof(DirectDrawDeviceGuids));
    std::memset(&DirectDrawDriverCaps, 0, sizeof(DirectDrawDriverCaps));
    std::memset(&DirectDrawEmulationCaps, 0, sizeof(DirectDrawEmulationCaps));
    DirectDrawDriverCaps.untouched_witness = 0x1122;
    DirectDrawEmulationCaps.untouched_witness = 0x3344;
    BufferDirectDraw = 0; DirectDrawSurface = 0; DirectDrawBackBuffer = 0;
    DirectDrawClipper = &clipper_alt; DirectDrawPalette = &palette_alt;
    DirectDrawHdc = 0; WinHdcRefCount = 0; DDActive = 0;
    DirectDrawDensityLimit = 0; PaletteActive = &owner_palette;
    std::memset(&ScreenBuffer, 0, sizeof(ScreenBuffer));
    std::memset(&DirectDrawClipRect, 0, sizeof(DirectDrawClipRect));
    std::memset(&guid, 0xA5, sizeof(guid));
    HandleMain = reinterpret_cast<HWND>(100);
}
BasePop::BasePop() { log("popup-construct"); }
BasePop::~BasePop() {
    log("popup-destroy");
    if (mutate == 1) DirectDrawDeviceGuids[0] = &guid;
}
int BasePop::init(int group, long flags) {
    assert(group == 0x42 && flags == 0); log("popup-init"); return 77;
}
int BasePop::exec(int flags, int (*callback)()) {
    assert(flags == 0 && callback == 0); log("popup-exec");
    if (mutate == 2) throw std::runtime_error("exec");
    return selection;
}
int Dialogs::item(LPSTR description, int index) {
    assert(description != 0); assert(index == DirectDrawDeviceCount);
    ++item_count; log("item");
    if (mutate == 3) DirectDrawDeviceCount = 2;
    return -5;
}
void *mem_get(int count) {
    assert(count == 16); log("allocate-guid");
    if (mutate == 4) DirectDrawDeviceCount = 3;
    return alloc_null ? 0 : &allocations[allocated_count++];
}
HRESULT DirectDrawEnumerateA(BOOL (*callback)(GUID *, LPSTR, LPSTR, LPVOID), LPVOID ctx) {
    log("enumerate-devices");
    if (enum_kind != 2) assert(callback(0, const_cast<char *>("primary"), 0, ctx) == 1);
    if (enum_kind == 1) assert(callback(&guid, const_cast<char *>("secondary"), 0, ctx) == 1);
    return -77;
}
HRESULT DirectDrawCreate(GUID *selected, IDirectDraw **out, LPVOID outer) {
    assert(selected == 0 || selected == &allocations[0]); assert(outer == 0);
    log("create-device"); *out = &initial; return failures(1);
}
HRESULT IDirectDraw::QueryInterface(const GUID &iid, void **out) {
    assert(&iid == &IID_IDirectDraw2); assert(out == reinterpret_cast<void **>(&BufferDirectDraw));
    log("query-device2"); *out = mutate == 5 ? &device_alt : &device;
    return failures(2);
}
DWORD IDirectDraw::Release() { log("release-created-device"); return 0; }
DWORD IDirectDraw2::Release() { log(id == 1 ? "release-device" : "release-device-alt"); return 0; }
HRESULT IDirectDraw2::GetDisplayMode(DDSURFACEDESC *desc) {
    assert(desc->dwSize == 0x6C); log("get-display-mode");
    std::memset(desc, 0, sizeof(*desc)); desc->dwSize = 0x6C;
    desc->dwHeight = 600; desc->dwWidth = 800; desc->lPitch = 1600;
    desc->dwBackBufferCount = 1; desc->untouched_witness = 0x5566;
    retained_description = desc; return failures(3);
}
HRESULT IDirectDraw2::GetCaps(DDCAPS *driver, DDCAPS *emulation) {
    assert(driver == &DirectDrawDriverCaps && emulation == &DirectDrawEmulationCaps);
    assert(driver->dwSize == 0x16C && emulation->dwSize == 0x16C);
    assert(driver->untouched_witness == 0x1122 && emulation->untouched_witness == 0x3344);
    log("get-caps"); driver->dwVidMemFree = 3000000; return failures(4);
}
HRESULT IDirectDraw2::EnumDisplayModes(DWORD flags, DDSURFACEDESC *desc, LPVOID ctx,
                                      HRESULT (*callback)(LPDDSURFACEDESC, LPVOID)) {
    assert(flags == 0 && desc == retained_description && desc->dwFlags == 0x1000);
    assert(desc->ddpfPixelFormat.dwSize == 0x20 && desc->ddpfPixelFormat.dwFlags == 0x40);
    assert(desc->ddpfPixelFormat.dwRGBBitCount == 8 || desc->ddpfPixelFormat.dwRGBBitCount == 16);
    assert(desc->untouched_witness == 0x5566 && *static_cast<DWORD *>(ctx) == 0);
    assert(DirectDrawDensityLimit == 4920000); log("enumerate-modes");
    retained_width = static_cast<DWORD *>(ctx);
    DDSURFACEDESC mode; std::memset(&mode, 0, sizeof(mode));
    mode.dwHeight = 600; mode.lPitch = 800; mode.dwBackBufferCount = 1;
    mode.dwWidth = 800;
    if (enum_kind != 3) assert(callback(&mode, ctx) == 1);
    mode.dwWidth = 1024; if (enum_kind != 3) assert(callback(&mode, ctx) == 1);
    return failures(5);
}
HRESULT IDirectDraw2::SetCooperativeLevel(HWND window, DWORD flags) {
    assert(window == HandleMain && flags == 0x13); log("set-cooperative-level");
    return failures(6);
}
HRESULT IDirectDraw2::SetDisplayMode(DWORD w, DWORD h, DWORD depth, DWORD rate, DWORD flags) {
    assert(w == 1024 && h == 768 && (depth == 8 || depth == 16) && rate == 0 && flags == 0);
    log("set-display-mode"); if (mutate == 6) *retained_width = 800;
    return failures(7);
}
HRESULT IDirectDraw2::CreateSurface(DDSURFACEDESC *desc, IDirectDrawSurface **out, LPVOID outer) {
    assert(desc == retained_description && desc->dwSize == 0x6C && desc->dwFlags == 0x21);
    assert(desc->ddsCaps.dwCaps == 0x4218 && desc->dwBackBufferCount == 1 && outer == 0);
    assert(desc->dwWidth == 800 && desc->dwHeight == 600 && desc->untouched_witness == 0x5566);
    log("create-surface"); *out = &primary; return failures(8);
}
HRESULT IDirectDrawSurface::GetAttachedSurface(DDSCAPS *caps, IDirectDrawSurface **out) {
    assert(id == 1 && caps->dwCaps == 4); log("get-attached"); *out = &back;
    return failures(9);
}
HRESULT IDirectDrawSurface::QueryInterface(const GUID &iid, void **out) {
    assert(&iid == &IID_IDirectDrawSurface2);
    assert(out == (id == 1 ? reinterpret_cast<void **>(&DirectDrawSurface)
                          : reinterpret_cast<void **>(&DirectDrawBackBuffer)));
    log(id == 1 ? "query-primary2" : "query-back2");
    *out = id == 1 ? &queried_primary : &queried_back;
    return -99; // deliberately ignored failures with nonnull outputs
}
HRESULT IDirectDraw2::CreatePalette(DWORD flags, PALETTEENTRY *entries,
                                    IDirectDrawPalette **out, LPVOID outer) {
    assert(flags == 4 && entries == reinterpret_cast<PALETTEENTRY *>(PaletteActive) && out == &DirectDrawPalette && outer == 0);
    log("create-palette"); *out = &palette; return failures(10);
}
HRESULT IDirectDrawSurface2::SetPalette(IDirectDrawPalette *p) {
    log(id == 1 ? "palette-primary" : "palette-back");
    assert(p == DirectDrawPalette);
    if (id == 1 && mutate == 7) { DirectDrawBackBuffer = &surface_alt; DirectDrawPalette = &palette_alt; }
    return -99;
}
HRESULT IDirectDraw2::CreateClipper(DWORD flags, IDirectDrawClipper **out, LPVOID outer) {
    assert(flags == 0 && out == &DirectDrawClipper && outer == 0); log("create-clipper");
    *out = &clipper; return failures(11);
}
void Buffer::close() {
    assert(this == &ScreenBuffer && DDActive == 1); log("screen-close");
    if (mutate == 8) { DirectDrawBackBuffer = &surface_alt; DirectDrawClipper = &clipper_alt; }
    surface_ = 0; clipper_ = 0;
    locked_bits_ = reinterpret_cast<LPVOID>(200); hdc2_ = reinterpret_cast<HDC>(300);
}
HRESULT IDirectDrawSurface2::GetDC(HDC *out) {
    assert(out == &DirectDrawHdc); log("get-dc"); *out = reinterpret_cast<HDC>(1000); return -7;
}
HRESULT IDirectDrawSurface2::ReleaseDC(HDC handle) {
    assert(handle == reinterpret_cast<HDC>(1000)); log("release-dc");
    DirectDrawHdc = reinterpret_cast<HDC>(2000); return -8;
}
BOOL IntersectRect(RECT *out, const RECT *a, const RECT *b) {
    assert(a == &DirectDrawClipRect && b != 0); log("intersect");
    if (mutate == 10) return 0;
    *out = *b; if (mutate == 11) out->right -= 1;
    if (mutate == 12) DirectDrawBackBuffer = &surface_alt;
    return 1;
}
BOOL EqualRect(const RECT *a, const RECT *b) {
    assert(a == &DirectDrawClipRect); log("equal");
    return std::memcmp(a, b, sizeof(RECT)) == 0;
}
HRESULT IDirectDrawClipper::SetClipList(RGNDATA *data, DWORD flags) {
    assert(data->rdh.dwSize == 0x20 && data->rdh.iType == 1 && data->rdh.nCount == 1);
    assert(data->rdh.nRgnSize == 0x10 && flags == 0);
    assert(std::memcmp(&data->rdh.rcBound, data->Buffer, sizeof(RECT)) == 0);
    log("set-clip-list");
    if (mutate == 9) { DirectDrawBackBuffer = &surface_alt; DirectDrawClipper = &clipper_alt; }
    return -18;
}
HRESULT IDirectDrawSurface2::SetClipper(IDirectDrawClipper *p) {
    assert(p == 0 || p == DirectDrawClipper); log(p == 0 ? "clear-clipper" : "set-clipper");
    if (mutate == 9 || mutate == 12) assert(id == 3);
    return result_code;
}
static bool saw(const char *what) {
    for (unsigned i = 0; i != trace.size(); ++i) if (trace[i] == what) return true;
    return false;
}
int main() {
    reset(); enum_kind = 1; selection = 1;
    GUID *selected = select_direct_draw_device();
    assert(selected == &allocations[0] && std::memcmp(selected, &guid, 16) == 0);
    assert(DirectDrawDeviceCount == 2 && item_count == 2 && saw("popup-destroy"));
    reset(); enum_kind = 1; alloc_null = 1; selection = 1;
    assert(select_direct_draw_device() == 0 && DirectDrawDeviceCount == 1 && item_count == 2);
    reset(); selection = -1; assert(select_direct_draw_device() == 0 && DirectDrawSelectionCancelled == 1);
    reset(); selection = -2; assert(select_direct_draw_device() == 0 && DirectDrawSelectionCancelled == 0);
    reset(); enum_kind = 2; selection = 0; assert(select_direct_draw_device() == 0);
    reset(); enum_kind = 1; selection = 0; mutate = 1;
    assert(select_direct_draw_device() == 0 && DirectDrawDeviceGuids[0] == &guid);
    reset(); mutate = 2;
    try { select_direct_draw_device(); assert(false); } catch (const std::runtime_error &) {}
    assert(saw("popup-destroy"));
    reset(); mutate = 3; BasePop popup;
    direct_draw_device_callback(0, const_cast<char *>("x"), 0, &popup);
    assert(DirectDrawDeviceCount == 3 && DirectDrawDeviceGuids[2] == 0);
    reset(); mutate = 4; direct_draw_device_callback(&guid, const_cast<char *>("x"), 0, &popup);
    assert(DirectDrawDeviceCount == 4 && DirectDrawDeviceGuids[3] == &allocations[0]);
    reset(); selection = -1; BufferDirectDraw = &device;
    ScreenBuffer.surface_ = &primary; ScreenBuffer.clipper_ = &clipper;
    assert(sub_62cd50(8) == 0 && saw("release-device") && !saw("create-device"));
    assert(ScreenBuffer.surface_ == 0 && ScreenBuffer.clipper_ == 0 && DDActive == 0);
    for (int stage = 1; stage <= 11; ++stage) {
        reset(); fail_stage = stage;
        assert(sub_62cd50(8) == 0x12 && DDActive == 0 && !saw("screen-close"));
        if (stage == 2) assert(BufferDirectDraw == &device && saw("release-created-device") && !saw("release-device"));
        else assert(BufferDirectDraw == 0);
        if (stage >= 3) assert(saw("release-device"));
        if (stage == 11) assert(DirectDrawSurface != 0 && DirectDrawBackBuffer != 0 && DirectDrawPalette == &palette);
    }
    reset(); enum_kind = 3; assert(sub_62cd50(8) == 0x12 && !saw("set-cooperative-level"));
    reset(); assert(sub_62cd50(8) == 0 && DDActive == 1 && BufferDirectDraw == &device);
    assert(ScreenBuffer.surface_ == reinterpret_cast<IDirectDrawSurface *>(&queried_back));
    assert(ScreenBuffer.clipper_ == &clipper && ScreenBuffer.locked_bits_ == 0 && ScreenBuffer.hdc2_ == 0);
    assert(saw("create-palette") && DirectDrawClipRect.right == 1024 && DirectDrawClipRect.bottom == 768);
    reset(); assert(sub_62cd50(16) == 0 && !saw("create-palette") && DirectDrawPalette == &palette_alt);
    reset(); PaletteActive = 0; assert(sub_62cd50(8) == 0 && saw("create-palette"));
    reset(); mutate = 6; assert(sub_62cd50(8) == 0 && DirectDrawClipRect.right == 800 && DirectDrawClipRect.bottom == 600);
    reset(); mutate = 7; assert(sub_62cd50(8) == 0 && DirectDrawBackBuffer == &surface_alt && DirectDrawPalette == &palette_alt);
    reset(); mutate = 8; assert(sub_62cd50(8) == 0 && ScreenBuffer.surface_ == reinterpret_cast<IDirectDrawSurface *>(&surface_alt) && ScreenBuffer.clipper_ == &clipper_alt);
    reset(); mutate = 5; fail_stage = 2; assert(sub_62cd50(8) == 0x12 && BufferDirectDraw == &device_alt);
    reset(); DirectDrawSelectionCancelled = 1; assert(sub_62d0f0() == 0 && DirectDrawSelectionCancelled == 1);
    reset(); BufferDirectDraw = &device; ScreenBuffer.surface_ = &primary; ScreenBuffer.clipper_ = &clipper;
    DirectDrawPalette = &palette; DirectDrawClipper = &clipper; DirectDrawHdc = reinterpret_cast<HDC>(1000); DDActive = 1;
    assert(sub_62d100() == 0 && BufferDirectDraw == 0 && ScreenBuffer.surface_ == 0 && ScreenBuffer.clipper_ == 0 && DDActive == 0);
    assert(DirectDrawPalette == &palette && DirectDrawClipper == &clipper && DirectDrawHdc == reinterpret_cast<HDC>(1000));
    reset(); assert(sub_62d160() == 0 && WinHdcRefCount == 1);
    DirectDrawSurface = &queried_primary; assert(sub_62d160() == reinterpret_cast<HDC>(1000) && WinHdcRefCount == 2);
    trace.clear(); assert(sub_62d160() == reinterpret_cast<HDC>(1000) && WinHdcRefCount == 3 && !saw("get-dc"));
    sub_62d1a0(); assert(WinHdcRefCount == 2 && DirectDrawHdc == 0 && saw("release-dc"));
    trace.clear(); sub_62d1a0(); assert(WinHdcRefCount == 1 && !saw("release-dc"));
    WinHdcRefCount = INT_MIN; sub_62d1a0(); assert(WinHdcRefCount == INT_MAX);
    WinHdcRefCount = INT_MAX; sub_62d160(); assert(WinHdcRefCount == INT_MIN);
    reset(); DDSURFACEDESC mode; std::memset(&mode, 0, sizeof(mode));
    mode.dwWidth = 1024; mode.dwHeight = 1; mode.lPitch = 1; DWORD width = 0;
    DirectDrawDensityLimit = 2; assert(sub_62cd10(&mode, &width) == 1 && width == 0);
    DirectDrawDensityLimit = 3; sub_62cd10(&mode, &width); assert(width == 1024);
    mode.dwWidth = 1025; width = 0; sub_62cd10(&mode, &width); assert(width == 0);
    mode.dwWidth = 800; mode.dwBackBufferCount = 0xFFFFFFFFu; DirectDrawDensityLimit = 1;
    sub_62cd10(&mode, &width); assert(width == 800);
    mode.dwWidth = 600; sub_62cd10(&mode, &width); assert(width == 800);
    reset(); sub_62d1e0(0); assert(trace.empty()); assert(sub_62d2c0() == 0);
    DirectDrawBackBuffer = &queried_back; DirectDrawClipper = &clipper;
    DirectDrawClipRect.right = 1024; DirectDrawClipRect.bottom = 768;
    RECT partial = DirectDrawClipRect; partial.right = 800;
    sub_62d1e0(&partial); assert(saw("set-clip-list") && saw("set-clipper"));
    trace.clear(); sub_62d1e0(&DirectDrawClipRect); assert(saw("clear-clipper") && !saw("set-clip-list"));
    trace.clear(); assert(sub_62d2c0() == 123 && saw("clear-clipper"));
    trace.clear(); mutate = 11; assert(sub_62d2c0() == 123 && saw("set-clip-list") && saw("set-clipper"));
    trace.clear(); mutate = 9; sub_62d1e0(&partial); assert(saw("set-clipper") && DirectDrawClipper == &clipper_alt);
    trace.clear(); mutate = 10; sub_62d1e0(&partial); assert(trace.size() == 1 && sub_62d2c0() == 0);
    std::cout << "display actual-body controls: PASS (10 native identities)\n";
}
