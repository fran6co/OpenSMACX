// ORIGINAL: 0x005C86E0 ?world_build@@YAXXZ 0x005C86E0-0x005C899D FILE
// size      701 bytes
// prototype 
// callers   4   call targets   33
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0046FD90 0x004C45E0 0x00591040 0x00592940 0x005C28F0 0x005C2B40 0x005C2CB0 0x005C32A0 0x005C3440 0x005C3680 0x005C3F70 0x005C40F0 0x005C4170 0x005C58E0 0x005C5A30 0x005C5AE0 0x005C5C70 0x005C5EF0 0x005C6200 0x005C64A0 0x005C6740 0x005C69E0 0x005C6C40 0x005C6DB0 0x005C7020 0x005C7540 0x005C7750 0x005C7A80 0x005C7F40 0x005C8150 0x005C83B0 0x0064601D
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005C86E0
// measured tier  MISMATCH
// divergence     12
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005c86e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?world_build@@YAXXZ  at 0x005C86E0  (701 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class Font;
class GraphicWin;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;
class MapWin;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Win;
class WorldWin;

class AutoSound { public:
    PVOID vtable_;
    int val_1_;
    int val_2_;
    int val_3_;
    int val_4_;
    int val_5_;
    int val_6_;
    int val_7_;
    int val_8_;
    int val_9_;
    int val_10_;
    int val_11_;
    int val_12_;
    int val_13_;
    int val_14_;
    int val_15_;
    int val_16_;
    int val_17_;
    int val_18_;
    int val_19_;
    int val_20_;
    int val_21_;
    int val_22_;
    int val_23_;
    int val_24_;
    int val_25_;
    int val_26_;
    int val_27_;
    int val_28_;
    int val_29_;
    int val_30_;
    int val_31_;
    int val_32_;
    int val_33_;
    int val_34_;
    int val_35_;
    int val_36_;
    int val_37_;
};

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
};

class Buffer { public:
    LPVOID vtable_;
    uint32_t poOwner_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    RECT rect1_;
    RECT rect2_;
    uint32_t field_40_[4];
    uint32_t field_50_;
    LPVOID * ppv_bits_;
    uint32_t field_58_;
    uint32_t field_5C_;
    HDC hdc2_;
    HDC hdc_;
    uint32_t field_68_;
    uint32_t field_6C_;
    HRGN field_70_;
    uint32_t field_74_;
    HBITMAP bitmap_handle_;
    const BITMAPINFO * bitmap_info_;
    uint32_t width_;
    uint32_t height_;
    uint16_t field_88_;
    uint16_t field_8A_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    int32_t dib_[256];
    uint32_t field_4A4_;
    uint32_t field_4A8_;
    uint32_t field_4AC_;
    Spot spot_;
    uint8_t field_4BC_[80];
    uint32_t field_50C_;
    uint32_t field_510_;
    uint32_t field_514_;
    uint32_t field_518_;
    uint32_t field_51C_;
    uint32_t field_520_;
    uint32_t field_524_;
    uint32_t field_528_;
    Font * font1_;
    Font * font2_;
    Font * font3_;
    Font * font4_;
    uint32_t color_val_1_;
    uint32_t color_2_val_1_;
    uint32_t color_3_val_1_;
    uint32_t color_hyper_val_1_;
    uint32_t color_val_2_;
    uint32_t color_2_val_2_;
    uint32_t color_3_val_2_;
    uint32_t color_hyper_val_2_;
    uint32_t color_val_3_;
    uint32_t color_2_val_3_;
    uint32_t color_3_val_3_;
    uint32_t color_hyper_val_3_;
    uint32_t color_val_4_;
    uint32_t color_2_val_4_;
    uint32_t color_3_val_4_;
    uint32_t color_hyper_val_4_;
    uint32_t field_57C_;
    int8_t field_580_;
    uint32_t field_584_;
};

class Font { public:
    int unk_1_;
    BOOL is_fot_set_;
    HFONT font_obj_;
    int line_height_;
    int height_;
    int internal_leading_;
    int ascent_;
    int descent_;
    int pad_;
    LPSTR fot_file_name_;
};

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
};

class GraphicWin { public:
    AutoSound auto_sound_;
    uint32_t iFlags_;
    uint32_t iSomeFlag_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t poWinBase_;
    uint32_t iVertScaleDenom_;
    uint32_t iVertScaleNum_;
    Buffer * buffer1_;
    Buffer * buffer2_;
    Buffer * buffer3_;
    Buffer * buffer4_;
    Win * win_parent_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    Heap heap_;
    Menu * menu_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    RECT outer_rect_;
    RECT client_rect_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    Sprite * cursor_sprite_;
    uint32_t field_18C_;
    uint32_t field_190_;
    HCURSOR * cursor_handle_;
    int cursor_name_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    Win * children_[150];
    int child_count_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    Scroll * scroll_vert_;
    Scroll * scroll_horz_;
    Buffer buffer_;
    uint32_t field_9CC_;
    uint32_t field_9D0_;
    uint32_t field_9D4_;
    uint32_t field_9D8_;
    uint32_t field_9DC_;
    uint32_t field_9E0_;
    uint32_t field_9E4_;
    uint32_t field_9E8_;
    uint32_t field_9EC_;
    uint32_t field_9F0_;
    uint32_t field_9F4_;
    uint32_t field_9F8_;
    uint32_t field_9FC_;
    uint32_t field_A00_;
    uint32_t field_A04_;
    uint32_t poCanvas_;
    uint32_t field_A0C_;
    uint32_t field_A10_;
};

class Sprite { public:
    int ppszFileName_;
    int pcBits_;
    char cTransparentIndex_;
    char pad1_;
    char pad2_;
    char pad3_;
    int iSpriteWidth2_;
    int iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_;
    int iHeight_;
    int iLeftOffset_;
    int iTopOffset_;
    int fObj1Exists_;
};

class Win { public:
    AutoSound auto_sound_;
    uint32_t iFlags_;
    uint32_t iSomeFlag_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t poWinBase_;
    uint32_t iVertScaleDenom_;
    uint32_t iVertScaleNum_;
    Buffer * buffer1_;
    Buffer * buffer2_;
    Buffer * buffer3_;
    Buffer * buffer4_;
    Win * win_parent_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    Heap heap_;
    Menu * menu_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    RECT outer_rect_;
    RECT client_rect_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    Sprite * cursor_sprite_;
    uint32_t field_18C_;
    uint32_t field_190_;
    HCURSOR * cursor_handle_;
    int cursor_name_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    Win * children_[150];
    int child_count_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    Scroll * scroll_vert_;
    Scroll * scroll_horz_;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;
    void clear_terrain();
};
class WorldWin { public:
    void clear_terrain();
};
extern "C" int rand();
int __cdecl world_validate();
void __cdecl build_continent(int);
void __cdecl build_hills(int);
void __cdecl draw_map(int);
void __cdecl fixup_landmarks();
void __cdecl map_wipe();
void __cdecl world_borehole(int, int);
void __cdecl world_climate();
void __cdecl world_crater(int, int);
void __cdecl world_diamond(int, int);
void __cdecl world_dune(int, int);
void __cdecl world_erosion();
void __cdecl world_fossil(int, int);
void __cdecl world_fresh(int, int);
void __cdecl world_fungus();
void __cdecl world_geothermal(int, int);
void __cdecl world_linearize_contours();
void __cdecl world_mesa(int, int);
void __cdecl world_monsoon(int, int);
void __cdecl world_polar_caps();
void __cdecl world_ridge(int, int);
void __cdecl world_riverbeds();
void __cdecl world_rocky();
void __cdecl world_ruin(int, int);
void __cdecl world_sargasso(int, int);
void __cdecl world_shorelines();
void __cdecl world_temperature();
void __cdecl world_temple(int, int);
void __cdecl world_unity(int, int);
void __cdecl world_volcano(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068f21c = (int *)0x0068F21C;
static int *const g_0068faf4 = (int *)0x0068FAF4;
static int *const g_0068fb4c = (int *)0x0068FB4C;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_007d3c5c = (int *)0x007D3C5C;
static int *const g_008e9f60 = (int *)0x008E9F60;
static int *const g_00949884 = (int *)0x00949884;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a2a0 = (int *)0x0094A2A0;
static int *const g_0094a2a4 = (int *)0x0094A2A4;
static int *const g_0094a2ac = (int *)0x0094A2AC;
static int *const g_0094a2bc = (int *)0x0094A2BC;
static int *const g_009502a8 = (int *)0x009502A8;
static int *const g_009502ac = (int *)0x009502AC;
static int *const g_009502b8 = (int *)0x009502B8;
static int *const g_009502bc = (int *)0x009502BC;
static int *const g_00950320 = (int *)0x00950320;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009b22b0 = (int *)0x009B22B0;
static int *const g_009b22b4 = (int *)0x009B22B4;
static int *const g_009b22e8 = (int *)0x009B22E8;
static int *const g_009b22ec = (int *)0x009B22EC;

extern "C" int rand();

void __cdecl world_build() {
    int iVar2 = 0;
    int local_8 = 0;
    do {
        {
            int *src = g_0068faf4;
            int *dst = g_0068fb4c;
            for (int i = 0x0b; i != 0; --i) {
                *dst = *src;
                ++src;
                ++dst;
            }
        }
        *g_009b22e8 = 1;
        *g_009b22ec = 1;

        for (int *p = g_0094a2a0; p < g_0094a2bc; ++p) {
            if (*p < 0) {
                *p = rand() % 3;
            }
        }

        map_wipe();

        int iVar1 = *g_00950320 * *g_00949884;
        *g_009b22b4 = (((2 - *g_0094a2a4) * (*g_009502ac) + *g_009502a8) * (*g_00949884)) / 0xc80;
        *g_009b22b0 = 0;

        if ((*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) != 0) {
            reinterpret_cast<MapWin *>(*g_007d3c3c)->clear_terrain();
            draw_map(1);
        }

        int target = iVar1 / 0xc80;
        int iVar5 = 0;
        while ((*g_009b22b0 < *g_009b22b4 || iVar2 < target) && (++iVar5, iVar5 < 100)) {
            ++iVar2;
            build_continent(iVar2);
        }

        if (*g_0094a2ac > 0) {
            int hills = (((*g_0094a2ac - 1) * (*g_009502bc) + *g_009502b8) * (*g_00949884)) / 0xc80;
            if (*g_0094a2a4 == 2) {
                hills = hills / 2;
            }
            if (hills > 0) {
                do {
                    build_hills(4);
                    --hills;
                } while (hills != 0);
            }
        }

        world_erosion();

        if ((*reinterpret_cast<unsigned char *>(g_0094988c) & 1) == 0 && *g_0094a2a4 < 2 &&
            (*g_0094a2a4 == 0 || rand() % 3 == 0)) {
            world_polar_caps();
        }

        world_shorelines();
    } while ((*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) == 0 &&
             world_validate() != 0 && (++local_8, local_8 < 4));

    world_temperature();
    world_riverbeds();
    world_fungus();
    world_monsoon(-1, -1);
    world_crater(-1, -1);
    world_volcano(-1, -1, 0);
    world_mesa(-1, -1);
    world_ridge(-1, -1);
    world_diamond(-1, -1);
    world_ruin(-1, -1);
    if (*g_009a6488 != 0) {
        world_unity(-1, -1);
        if (*g_009a6488 != 0) {
            world_fossil(-1, -1);
        }
    }
    world_temple(-1, -1);
    world_borehole(-1, -1);
    world_sargasso(-1, -1);
    world_dune(-1, -1);
    world_fresh(-1, -1);
    world_geothermal(-1, -1);

    fixup_landmarks();
    world_linearize_contours();
    *g_009b22e8 = 0;
    world_climate();
    world_rocky();
    *g_009b22ec = 0;

    if (*g_0068f21c == 0) {
        for (int *p = g_007d3c3c; p < g_007d3c5c; ++p) {
            if (*reinterpret_cast<int *>(*p + 0x1dd74) != 0) {
                reinterpret_cast<MapWin *>(*p)->clear_terrain();
            }
        }
        reinterpret_cast<WorldWin *>(g_008e9f60)->clear_terrain();
    }
}
