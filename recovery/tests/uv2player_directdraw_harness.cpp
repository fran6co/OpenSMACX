#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

typedef int32_t HRESULT;
typedef uint32_t ULONG;
typedef uint32_t DWORD;
typedef void *HWND;
struct GUID { uint32_t d1; uint16_t d2, d3; uint8_t d4[8]; };
typedef const GUID &REFIID;
struct RECT { int32_t left, top, right, bottom; };

static const HRESULT DD_OK = 0;
static const DWORD DDSCL_FULLSCREEN = 1;
static const DWORD DDSCL_EXCLUSIVE = 0x10;
static const DWORD DDSD_CAPS = 1;
static const DWORD DDSD_BACKBUFFERCOUNT = 0x20;
static const DWORD DDSCAPS_BACKBUFFER = 4;
static const DWORD DDSCAPS_COMPLEX = 8;
static const DWORD DDSCAPS_FLIP = 0x10;
static const DWORD DDSCAPS_PRIMARYSURFACE = 0x200;
static const DWORD DDBLT_COLORFILL = 0x400;
static const DWORD DDBLT_WAIT = 0x01000000;

struct DDSCAPS { DWORD dwCaps; };
struct DDSURFACEDESC {
    DWORD dwSize, dwFlags, dwHeight, dwWidth, lPitch, dwBackBufferCount;
    DWORD dwMipMapCount, dwAlphaBitDepth, dwReserved;
    uint32_t lpSurface;
    unsigned char color_keys_and_format[0x40];
    DDSCAPS ddsCaps;
};
struct DDBLTFX {
    DWORD dwSize;
    unsigned char opaque[0x4C];
    DWORD dwFillColor;
    DWORD tail[4];
};
static_assert(sizeof(DDSURFACEDESC) == 0x6C, "SDK surface descriptor size");
static_assert(sizeof(DDBLTFX) == 0x64, "SDK blit-effects size");

struct IDirectDrawSurface2;
struct IDirectDrawSurface;
struct IDirectDrawClipper;
struct IDirectDraw2;
struct IDirectDraw;
typedef IDirectDrawSurface *LPDIRECTDRAWSURFACE;
typedef IDirectDrawSurface2 *LPDIRECTDRAWSURFACE2;
typedef IDirectDrawClipper *LPDIRECTDRAWCLIPPER;
typedef IDirectDraw2 *LPDIRECTDRAW2;
typedef IDirectDraw *LPDIRECTDRAW;

static std::vector<std::string> trace;
static void note(const char *event) { trace.push_back(event); }
static int where(const char *event) {
    for (size_t i = 0; i < trace.size(); ++i)
        if (trace[i] == event) return static_cast<int>(i);
    return -1;
}
static void before(const char *a, const char *b) {
    assert(where(a) >= 0 && where(b) >= 0 && where(a) < where(b));
}

static const GUID IID_IDirectDraw2 = {1, 2, 3, {0}};
static const GUID IID_IDirectDrawSurface2 = {4, 5, 6, {0}};

struct IDirectDraw {
    virtual HRESULT QueryInterface(REFIID, void **) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT SetCooperativeLevel(HWND, DWORD) = 0;
};
struct IDirectDraw2 {
    virtual HRESULT RestoreDisplayMode() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT SetDisplayMode(DWORD, DWORD, DWORD, DWORD, DWORD) = 0;
    virtual HRESULT CreateSurface(DDSURFACEDESC *, LPDIRECTDRAWSURFACE *, void *) = 0;
    virtual HRESULT CreateClipper(DWORD, LPDIRECTDRAWCLIPPER *, void *) = 0;
};
struct IDirectDrawSurface {
    virtual HRESULT QueryInterface(REFIID, void **) = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT GetAttachedSurface(DDSCAPS *, LPDIRECTDRAWSURFACE *) = 0;
};
struct IDirectDrawSurface2 {
    virtual HRESULT QueryInterface(REFIID, void **) = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT Blt(RECT *, LPDIRECTDRAWSURFACE2, RECT *, DWORD, DDBLTFX *) = 0;
};
struct IDirectDrawClipper { virtual ULONG Release() = 0; };

struct FakeSurface2 : IDirectDrawSurface2 {
    const char *name;
    explicit FakeSurface2(const char *n) : name(n) {}
    HRESULT QueryInterface(REFIID, void **out) { *out = this; note("surface2-qi"); return DD_OK; }
    ULONG Release() { note(name); return 1; }
    HRESULT Blt(RECT *, LPDIRECTDRAWSURFACE2, RECT *, DWORD flags, DDBLTFX *fx) {
        assert(flags == (DDBLT_WAIT | DDBLT_COLORFILL));
        assert(fx->dwSize == 0x64 && fx->dwFillColor == 0);
        note(name);
        return DD_OK;
    }
};
struct FakeClipper : IDirectDrawClipper {
    ULONG Release() { note("clipper-release"); return 1; }
};
struct FakeSurface : IDirectDrawSurface {
    FakeSurface2 *surface2;
    FakeSurface *back;
    HRESULT attached_result;
    FakeSurface2 *qi_target;
    const char *label;
    FakeSurface(const char *n, FakeSurface2 *s) : surface2(s), back(0),
        attached_result(DD_OK), qi_target(s), label(n) {}
    HRESULT QueryInterface(REFIID iid, void **out) {
        assert(iid.d1 == IID_IDirectDrawSurface2.d1);
        *out = qi_target;
        note(label);
        return DD_OK;
    }
    ULONG Release() { note("surface1-release"); return 1; }
    HRESULT GetAttachedSurface(DDSCAPS *caps, LPDIRECTDRAWSURFACE *out) {
        assert(caps->dwCaps == DDSCAPS_BACKBUFFER);
        note("get-attached");
        *out = back;
        return attached_result;
    }
};

static FakeSurface2 primary2("primary2-release-or-blt");
static FakeSurface2 back2("back2-release-or-blt");
static FakeSurface primary1("primary-qi", &primary2);
static FakeSurface back1("back-qi", &back2);
static FakeClipper clipper;
static IDirectDraw2 **mode_rebind_slot;
static IDirectDraw2 *mode_rebind_target;

struct FakeDraw2 : IDirectDraw2 {
    HRESULT mode_results[4];
    int mode_calls;
    HRESULT create_clipper_result;
    FakeDraw2() : mode_calls(0), create_clipper_result(DD_OK) {
        for (int i = 0; i != 4; ++i) mode_results[i] = DD_OK;
    }
    HRESULT RestoreDisplayMode() { note("restore-mode"); return DD_OK; }
    ULONG Release() { note("draw2-release"); return 1; }
    HRESULT SetDisplayMode(DWORD w, DWORD h, DWORD depth, DWORD refresh, DWORD flags) {
        assert(w == 640 && h == 480 && refresh == 0 && flags == 0);
        const DWORD expected[] = {0x10, 0x18, 0x20, 8};
        assert(mode_calls < 4 && depth == expected[mode_calls]);
        char event[24];
        sprintf(event, "mode-%d", static_cast<int>(depth));
        note(event);
        if (mode_rebind_slot) {
            *mode_rebind_slot = mode_rebind_target;
            mode_rebind_slot = 0;
        }
        return mode_results[mode_calls++];
    }
    HRESULT CreateSurface(DDSURFACEDESC *desc, LPDIRECTDRAWSURFACE *out, void *) {
        assert(desc->dwSize == 0x6C);
        assert(desc->dwFlags == (DDSD_CAPS | DDSD_BACKBUFFERCOUNT));
        assert(desc->dwBackBufferCount == 1);
        assert(desc->ddsCaps.dwCaps == (DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE));
        *out = &primary1;
        note("create-surface");
        return DD_OK;
    }
    HRESULT CreateClipper(DWORD flags, LPDIRECTDRAWCLIPPER *out, void *) {
        assert(flags == 0);
        *out = &clipper;
        note("create-clipper");
        return create_clipper_result;
    }
};
struct FakeDraw : IDirectDraw {
    FakeDraw2 *draw2;
    HRESULT coop_result;
    HRESULT query_result;
    FakeDraw() : draw2(0), coop_result(DD_OK), query_result(DD_OK) {}
    HRESULT QueryInterface(REFIID iid, void **out) {
        assert(iid.d1 == IID_IDirectDraw2.d1);
        *out = draw2;
        note("draw-qi");
        return query_result;
    }
    ULONG AddRef() { note("draw-addref"); return 1; }
    ULONG Release() { note("draw-release"); return 1; }
    HRESULT SetCooperativeLevel(HWND hwnd, DWORD flags) {
        assert(hwnd == reinterpret_cast<HWND>(0x1234));
        assert(flags == (DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE));
        note("cooperative-level");
        return coop_result;
    }
};

static FakeDraw draw;
static FakeDraw2 draw2;
static HRESULT create_result;
static int pref_result;
static std::vector<std::string> mode_resolutions;
static int screen_fill_count, screen_flip_count;
static HWND HandleMain = reinterpret_cast<HWND>(0x1234);
static int WinScreenWidth;

struct ScreenBufferType {
    void fill(int color) { assert(color == 0); ++screen_fill_count; note("screen-fill"); }
};
static ScreenBufferType ScreenBuffer;
struct Win {
    static void flip(RECT *area) { assert(area == 0); ++screen_flip_count; note("screen-flip"); }
    static int set_display_mode(int w, int h, int depth, int toggle) {
        assert(depth == 8 && toggle == 0);
        char buf[32]; sprintf(buf, "restore-%dx%d", w, h);
        mode_resolutions.push_back(buf); note(buf); return 0;
    }
};
static int prefs_get(const char *key, int default_value, int use_default) {
    assert(strcmp(key, "DirectDraw") == 0 && default_value == 1 && use_default == 0);
    note("prefs-DirectDraw"); return pref_result;
}
static HRESULT DirectDrawCreate(GUID *, LPDIRECTDRAW *out, void *) {
    note("draw-create");
    if (create_result == DD_OK) *out = &draw;
    return create_result;
}

struct MediaDescriptor;
struct UV2Player {
    uint32_t field_0_, field_4_, field_8_, field_C_;
    uint32_t field_10_, field_14_, field_18_, field_1C_;
    uint32_t field_20_, field_24_, field_28_, field_2C_;
    uint32_t field_30_, field_34_, field_38_, field_3C_;
    uint32_t field_40_, field_44_, field_48_, field_4C_;
    uint32_t field_50_, field_54_, field_58_, field_5C_;
    uint32_t field_60_, field_64_, field_68_, field_6C_;
    uint32_t field_70_, field_74_, field_78_, field_7C_;
    uint32_t field_80_, field_84_, field_88_, field_8C_;
    uint32_t field_90_, field_94_, field_98_, field_9C_;
    uint32_t field_A0_, field_A4_, field_A8_, field_AC_;
    uint32_t field_B0_, field_B4_, field_B8_, field_BC_;
    uint32_t field_C0_, field_C4_, field_C8_, field_CC_;
    uint32_t field_D0_, field_D4_, field_D8_, field_DC_;
    uint32_t field_E0_, field_E4_, field_E8_, field_EC_;
    uint32_t field_F0_, field_F4_, field_F8_, field_FC_;
    uint32_t field_100_, field_104_;
    IDirectDraw *direct_draw_;
    IDirectDraw2 *direct_draw2_;
    IDirectDrawSurface2 *primary_surface_;
    IDirectDrawSurface2 *back_buffer_surface_;
    IDirectDrawClipper *clipper_;
    uint32_t field_11C_, field_120_;
    MediaDescriptor *media_session_;
    void close();
    int play(int width, int height);
};
extern UV2Player *UV2PlayerActive;

/* PRODUCT GLOBALS */
/* PRODUCT METHODS */

static void reset() {
    trace.clear(); mode_resolutions.clear();
    screen_fill_count = screen_flip_count = 0;
    create_result = DD_OK; pref_result = 1; WinScreenWidth = 800;
    draw.draw2 = &draw2; draw.coop_result = DD_OK; draw.query_result = DD_OK;
    draw2.mode_calls = 0; draw2.create_clipper_result = DD_OK;
    draw2.mode_results[0] = draw2.mode_results[1] =
        draw2.mode_results[2] = draw2.mode_results[3] = DD_OK;
    primary1.back = &back1; primary1.attached_result = DD_OK;
    UV2PlayerActive = 0;
    mode_rebind_slot = 0;
}
static void setup_player(UV2Player &player) {
    memset(&player, 0, sizeof(player));
    player.primary_surface_ = &primary2;
    player.back_buffer_surface_ = &back2;
}
static int has(const char *event) { return where(event) >= 0; }

int main(int argc, char **argv) {
    assert(argc == 2);
    reset();
    UV2Player player;
    setup_player(player);
    if (strcmp(argv[1], "receiver_rebind") == 0) {
        FakeDraw2 replacement;
        replacement.mode_calls = 1;
        draw2.mode_results[0] = -1;
        mode_rebind_slot = &player.direct_draw2_;
        mode_rebind_target = &replacement;
        assert(player.play(640, 480) == 0);
        assert(draw2.mode_calls == 1 && replacement.mode_calls == 2);
        assert(player.direct_draw2_ == &replacement);
        return 0;
    }
    const char *scenario = argv[1];
    if (strcmp(scenario, "guard") == 0) {
        player.direct_draw2_ = &draw2;
        assert(player.play(640, 480) == 1);
        assert(trace.empty());
    } else if (strcmp(scenario, "create_failure") == 0) {
        create_result = -1;
        assert(player.play(640, 480) == 1);
        assert(has("screen-fill") && has("screen-flip") && has("draw-create"));
        assert(player.direct_draw_ == 0 && player.direct_draw2_ == 0);
        assert(!has("draw-release") && !has("restore-mode"));
    } else if (strcmp(scenario, "cooperative_failure") == 0) {
        draw.coop_result = -1;
        assert(player.play(640, 480) == 1);
        assert(player.direct_draw_ == &draw && player.direct_draw2_ == 0);
        assert(!has("draw-release") && !has("restore-mode"));
    } else if (strcmp(scenario, "all_modes_fail") == 0) {
        draw2.mode_results[0] = draw2.mode_results[1] =
            draw2.mode_results[2] = draw2.mode_results[3] = -1;
        assert(player.play(640, 480) == 0x12);
        assert(draw2.mode_calls == 4);
        assert(has("restore-mode") && has("primary2-release-or-blt"));
        assert(has("draw2-release") && player.direct_draw2_ == 0);
        assert(mode_resolutions.size() == 1 && mode_resolutions[0] == "restore-800x600");
        assert(UV2PlayerActive == 0);
    } else if (strcmp(scenario, "attached_failure") == 0) {
        primary1.attached_result = -1;
        UV2PlayerActive = &player;
        assert(player.play(640, 480) == 1);
        assert(has("get-attached") && has("restore-mode") && has("draw2-release"));
        assert(UV2PlayerActive == 0);
    } else if (strcmp(scenario, "clipper_failure") == 0) {
        draw2.create_clipper_result = -1;
        UV2PlayerActive = &player;
        assert(player.play(640, 480) == 0x12);
        assert(has("create-clipper") && !has("restore-mode") && !has("draw2-release"));
        assert(UV2PlayerActive == &player && player.direct_draw2_ == &draw2);
    } else if (strcmp(scenario, "success") == 0) {
        draw2.mode_results[0] = -1;
        draw2.mode_results[1] = DD_OK;
        assert(player.play(640, 480) == 0);
        assert(draw2.mode_calls == 2);
        before("screen-fill", "screen-flip");
        before("screen-flip", "draw-create");
        before("cooperative-level", "draw-qi");
        before("draw-qi", "draw-release");
        before("mode-24", "create-surface");
        before("get-attached", "primary-qi");
        before("primary-qi", "back-qi");
        before("back-qi", "create-clipper");
        before("create-clipper", "back2-release-or-blt");
        assert(where("back2-release-or-blt") < where("primary2-release-or-blt"));
        assert(primary1.back == &back1);
    } else if (strcmp(scenario, "close_disabled") == 0) {
        player.direct_draw2_ = &draw2;
        player.direct_draw_ = &draw;
        player.primary_surface_ = &primary2;
        player.back_buffer_surface_ = &back2;
        player.field_11C_ = 1; player.field_120_ = 2;
        player.media_session_ = reinterpret_cast<MediaDescriptor *>(3);
        UV2PlayerActive = &player;
        pref_result = 0;
        player.close();
        assert(has("restore-mode") && has("draw2-release") && has("primary2-release-or-blt"));
        assert(!has("prefs-DirectDraw") || mode_resolutions.empty());
        assert(player.direct_draw_ == 0 && player.direct_draw2_ == 0);
        assert(player.primary_surface_ == 0 && player.back_buffer_surface_ == 0);
        assert(player.field_11C_ == 0 && player.field_120_ == 0 && player.media_session_ == 0);
        assert(UV2PlayerActive == 0);
    } else if (strcmp(scenario, "close_windowed_large") == 0) {
        player.direct_draw2_ = &draw2; player.primary_surface_ = &primary2;
        pref_result = 1; WinScreenWidth = 1024;
        player.close();
        assert(mode_resolutions.size() == 1 && mode_resolutions[0] == "restore-1024x768");
        before("restore-mode", "primary2-release-or-blt");
        before("primary2-release-or-blt", "draw2-release");
        before("draw2-release", "prefs-DirectDraw");
    } else if (strcmp(scenario, "close_without_device") == 0) {
        player.direct_draw2_ = 0; player.primary_surface_ = &primary2;
        player.close();
        assert(trace.empty() && mode_resolutions.empty());
        assert(player.direct_draw_ == 0 && player.back_buffer_surface_ == 0);
        assert(player.field_11C_ == 0 && player.field_120_ == 0 && player.media_session_ == 0);
    } else {
        assert(false);
    }
    return 0;
}
