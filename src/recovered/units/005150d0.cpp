// ORIGINAL: 0x005150D0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005150D0
// name           ?zoom@Console@@QAEXHH@Z
// size           783 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005150d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?zoom@Console@@QAEXHH@Z  at 0x005150D0  (783 bytes)
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
class Console;
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

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    void redraw();
};
class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;
    int pixel_to_tile(int, int, int *, int *);
    void draw_map(int);
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
    void get_mouse_pos(int *, int *);
};
class WorldWin { public:
    int pixel_to_tile(int, int, int *, int *);
};
int __cdecl in_box(int, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005153e0 = (int *)0x005153E0;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_007d3c5c = (int *)0x007D3C5C;
static int *const g_008e9f60 = (int *)0x008E9F60;
static int *const g_008ea424 = (int *)0x008EA424;
static int *const g_008ea428 = (int *)0x008EA428;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_0096c9e1 = (int *)0x0096C9E1;
static int *const g_009a64c0 = (int *)0x009A64C0;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void zoom(int, int);
};

extern int *g_city_table[];
extern unsigned int g_civ_visible_flags[];

void Console::zoom(int a1, int a2) {
    int idx;

    if (a2 < 0) {
        idx = 0;
        int mx, my;
        reinterpret_cast<Win *>(g_008e9f60)->get_mouse_pos(&mx, &my);

        bool matched = false;
        int tx = 0, ty = 0;

        if (in_box(mx, my, 0, 0, *g_008ea424, -*g_008ea428) != 0 &&
            reinterpret_cast<WorldWin *>(g_008e9f60)->pixel_to_tile(mx, my, &tx, &ty) == 0 &&
            tx >= 0 && tx < *g_00949874 &&
            ty >= 0 && ty < *g_00949870) {

            int civ = *g_00939284;
            int rec = civ << 6;
            rec = rec + civ;
            rec = civ + rec * 2;
            rec = civ + rec * 8;
            rec = civ + rec * 2;

            if ((g_civ_visible_flags[rec] & 0x200) != 0) {
                matched = true;
            } else {
                int stride = *g_0068faf0;
                int cell = stride * tx + (ty >> 1);
                int bitidx = cell + cell * 4;
                bitidx = cell + bitidx * 2;
                unsigned char bit = static_cast<unsigned char>(1 << (civ & 0x1f));
                unsigned char *explored = *reinterpret_cast<unsigned char **>(g_0094a30c);
                if ((explored[bitidx * 4 + 4] & bit) != 0) {
                    matched = true;
                } else if ((*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) != 0) {
                    matched = true;
                }
            }
        }

        if (matched) {
            char *city = reinterpret_cast<char *>(g_city_table[0]);
            *reinterpret_cast<int *>(city + 0x1dd9c) = ty;
            *reinterpret_cast<int *>(city + 0x1dda0) = tx;
            idx = 0;
        } else {
            for (int i = 0; i < 8; ++i) {
                char *city = reinterpret_cast<char *>(g_city_table[i]);
                if (city == 0) {
                    continue;
                }
                if (i != 0 && *reinterpret_cast<int *>(city + 0x1dd74) == 0) {
                    continue;
                }
                if (*reinterpret_cast<int *>(city + 0x1dd98) >= 0) {
                    continue;
                }

                int m8, m4;
                int *vbtable = *reinterpret_cast<int **>(city);
                int adj = *reinterpret_cast<int *>(reinterpret_cast<char *>(vbtable) + 4);
                reinterpret_cast<Win *>(city + adj)->get_mouse_pos(&m8, &m4);

                vbtable = *reinterpret_cast<int **>(city);
                adj = *reinterpret_cast<int *>(reinterpret_cast<char *>(vbtable) + 4);
                int w = *reinterpret_cast<int *>(city + adj + 0x4c8);
                int h = *reinterpret_cast<int *>(city + adj + 0x4c4);

                int ttx = 0, tty = 0;
                bool hit = in_box(m8, m4, 0, 0, h, -w) != 0 &&
                    reinterpret_cast<MapWin *>(city)->pixel_to_tile(m8, m4, &ttx, &tty) == 0 &&
                    ttx >= 0 && ttx < *g_00949874 &&
                    tty >= 0 && tty < *g_00949870;
                if (!hit) {
                    continue;
                }

                int civ = *g_00939284;
                int rec = civ << 6;
                rec = rec + civ;
                rec = civ + rec * 2;
                rec = civ + rec * 8;
                rec = civ + rec * 2;

                bool found = (reinterpret_cast<unsigned char *>(g_civ_visible_flags)[rec * 4 + 1] & 2) != 0;
                if (!found) {
                    int stride = *g_0068faf0;
                    int cell = stride * ttx + (tty >> 1);
                    int bitidx = cell + cell * 4;
                    bitidx = cell + bitidx * 2;
                    unsigned char bit = static_cast<unsigned char>(1 << (civ & 0x1f));
                    unsigned char *explored = *reinterpret_cast<unsigned char **>(g_0094a30c);
                    found = (explored[bitidx * 4 + 4] & bit) != 0;
                    if (!found) {
                        found = (*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) != 0;
                    }
                }
                if (!found) {
                    continue;
                }

                *reinterpret_cast<int *>(city + 0x1dd9c) = tty;
                *reinterpret_cast<int *>(city + 0x1dda0) = ttx;
                idx = i;
                break;
            }
        }

        a2 = idx;
    }

    int ebx = a1;
    for (int i = 0; i < 8; ++i) {
        char *city = reinterpret_cast<char *>(g_city_table[i]);
        if (city == 0) {
            continue;
        }
        if (i != 0 && *reinterpret_cast<int *>(city + 0x1dd74) == 0) {
            continue;
        }
        if (a2 >= 0 && i != a2) {
            continue;
        }

        switch (ebx) {
        case 0x203:
            if (*reinterpret_cast<int *>(city + 0x1dd98) < 0x10) {
                *reinterpret_cast<int *>(city + 0x1dd98) += 2;
            }
            break;
        case 0x204:
            if (*reinterpret_cast<int *>(city + 0x1dd98) > -0xE) {
                *reinterpret_cast<int *>(city + 0x1dd98) -= 2;
            }
            break;
        case 0x205:
            *reinterpret_cast<int *>(city + 0x1dd98) = 0;
            break;
        case 0x206:
            *reinterpret_cast<int *>(city + 0x1dd98) = -6;
            break;
        case 0x207:
            *reinterpret_cast<int *>(city + 0x1dd98) = 0x10;
            break;
        case 0x208:
            *reinterpret_cast<int *>(city + 0x1dd98) = -0xE;
            break;
        default:
            break;
        }

        reinterpret_cast<MapWin *>(city)->draw_map(1);
    }

    reinterpret_cast<GraphicWin *>(g_008e9f60)->redraw();
}
