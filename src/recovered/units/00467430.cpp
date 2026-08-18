// ORIGINAL: 0x00467430 ?dest_line@MapWin@@QAEXHHHHHHHHHHH@Z 0x00467430-0x00467958 FILE
// size      1320 bytes
// prototype void (__thiscall ?dest_line@MapWin@@QAEXHHHHHHHHHHH@Z)(MapWin* this, int xCoord1, int yCoord1, int xCoord2, int yCoord2, int vehID, int, int, int, int, int, int)
// callers   4   call targets   10
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004672C0 0x004E3A50 0x0050DE50 0x0050E160 0x00593510 0x0059A530 0x005A6630 0x005C13B0 0x005F1820 0x00644F3A
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00467430
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00467430/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?dest_line@MapWin@@QAEXHHHHHHHHHHH@Z  at 0x00467430  (1320 bytes)
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
class Path;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Win;

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

// ---- callees, declared and never defined (a definition would be inlined) ----
class Path { public:
    int find(int, int, int, int, int, int, int, int);
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
    int __cdecl update_cursor(Win *, int);
};
extern "C" int abs(int);
int __cdecl base_at(int, int);
int __cdecl base_on_sea(int, int);
int __cdecl compass_move(int, int, int, int);
int __cdecl hex_cost(int, int, int, int, int, int, int);
int __cdecl port_to_port(int, int);
int __cdecl speed_proto(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066ef50 = (int *)0x0066EF50;
static int *const g_0066ef74 = (int *)0x0066EF74;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_007d3b70 = (int *)0x007D3B70;
static int *const g_00945b00 = (int *)0x00945B00;
static int *const g_00949738 = (int *)0x00949738;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a6534 = (int *)0x009A6534;
static int *const g_009ab88d = (int *)0x009AB88D;

class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;

    void direct_dest_line(int, int, int, int, int, int, int, int);
    void dest_line(int, int, int, int, int, int, int, int, int, int, int);
};

// Indexed table bases: each address does arithmetic work (index scaled
// by a computed register), so a fresh extern array is needed instead of
// the scaffold's const-pointer spelling for these.
extern unsigned char g_x9ab88d[];
extern unsigned char g_x94ae68[];
extern unsigned char g_x97d044[];
extern int g_x66ef50[];
extern int g_x66ef74[];
extern unsigned int g_x96c9e0[];

void MapWin::dest_line(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11) {
    char *self = reinterpret_cast<char *>(this);
    bool direct = false;
    int half1 = a1 >> 1;
    int half2 = a3 >> 1;
    unsigned char *terrain = *reinterpret_cast<unsigned char **>(g_0094a30c);

    if (a7 == 0) {
        if (terrain[(*g_0068faf0 * a2 + half1) * 0x2c + 3] ==
            terrain[(*g_0068faf0 * a4 + half2) * 0x2c + 3]) {
            direct = true;
        }
    } else if (a7 == 1) {
        int b1 = base_at(a1, a2);
        int b2 = base_at(a3, a4);
        int r;
        bool haveR = false;
        if (b1 < 0) {
            if (b2 >= 0) {
                unsigned char t = terrain[(*g_0068faf0 * a2 + half1) * 0x2c + 3];
                r = base_on_sea(b2, t);
                haveR = true;
            } else if (terrain[(*g_0068faf0 * a2 + half1) * 0x2c + 3] ==
                       terrain[(*g_0068faf0 * a4 + half2) * 0x2c + 3]) {
                direct = true;
            }
        } else if (b2 < 0) {
            unsigned char t = terrain[(*g_0068faf0 * a4 + half2) * 0x2c + 3];
            r = base_on_sea(b1, t);
            haveR = true;
        } else {
            r = port_to_port(b1, b2);
            haveR = true;
        }
        if (haveR && r != 0) {
            direct = true;
        }
    } else if (a7 != 2) {
        direct = true;
    }

    int retryCount = 0;
    for (;;) {
        if (direct) {
            int dx = abs(a1 - a3);
            if ((*g_0094988c & 1) == 0 && *g_0068faf0 < dx) {
                dx = *g_00949870 - dx;
            }
            int dy = abs(a2 - a4);
            *g_007d3b70 = ((dy + dx) >> 1) * *g_00949738;
            direct_dest_line(a1, a2, a3, a4, a7, a9, a10, a11);
            return;
        }
        if (retryCount > 1) {
            return;
        }

        reinterpret_cast<Win *>(0)->update_cursor(0, 1);
        int lastStep = -1;
        int steps = 0;
        *g_007d3b70 = 0;
        int curX = a1;
        int curY = a2;

        for (;;) {
            reinterpret_cast<Win *>(0)->update_cursor(0, 1);
            unsigned int flags = (a8 == 0) ? 0xc0 : 0x80;
            if (g_x94ae68[(unsigned int)g_x9ab88d[a5 * 0x34] * 0x10] == 0 &&
                (*g_009a6534 < 0 || g_x97d044[*g_009a6534 * 0x134] != (unsigned int)a6)) {
                flags |= 0x10;
            }

            int dx = abs(curX - a3);
            if ((*g_0094988c & 1) == 0 && *g_0068faf0 < dx) {
                dx = *g_00949870 - dx;
            }
            int dy = abs(curY - a4);

            int step;
            if (((dy + dx) & 0xfffffffe) > 2) {
                step = reinterpret_cast<Path *>(g_00945b00)->find(curX, curY, a3, a4, a5, a6, static_cast<int>(flags), lastStep);
                if (step < 0) {
                    direct = true;
                    break;
                }
            } else {
                step = compass_move(curX, curY, a3, a4);
            }
            lastStep = step;

            int nx = g_x66ef50[step] + curX;
            if ((*g_0094988c & 1) == 0) {
                if (nx < 0) {
                    nx = *g_00949870 + nx;
                } else if (*g_00949870 <= nx) {
                    nx = nx - *g_00949870;
                }
            }
            int ny = curY + g_x66ef74[step];

            if (steps != 0) {
                if (a5 >= 0) {
                    int sp = speed_proto(a5);
                    int cost = hex_cost(a5, a6, curX, curY, nx, ny, (sp == *g_00949738) ? 1 : 0);
                    *g_007d3b70 = *g_007d3b70 + cost;
                }
                bool doDirect = false;
                {
                    int rectY0 = *reinterpret_cast<int *>(self + 0x1dda8);
                    int rectYSpan = *reinterpret_cast<int *>(self + 0x1ddd8) + *reinterpret_cast<int *>(self + 0x1ddd0) + rectY0;
                    if (curY >= rectY0 && curY < rectYSpan) {
                        int rectX0 = *reinterpret_cast<int *>(self + 0x1dda4);
                        int xv = curX;
                        if ((*g_0094988c & 1) == 0) {
                            if (curX < rectX0) {
                                xv = *g_00949870 + curX;
                            }
                            int rectXSpan0 = *reinterpret_cast<int *>(self + 0x1ddd4) + *reinterpret_cast<int *>(self + 0x1ddcc) + rectX0;
                            if (rectXSpan0 <= xv) {
                                xv = xv - *g_00949870;
                            }
                        }
                        int rectXSpan = *reinterpret_cast<int *>(self + 0x1ddd4) + *reinterpret_cast<int *>(self + 0x1ddcc) + rectX0;
                        if (xv >= rectX0 && xv < rectXSpan) {
                            doDirect = true;
                        }
                    }
                }
                if (!doDirect) {
                    int rectY0 = *reinterpret_cast<int *>(self + 0x1dda8);
                    int rectYSpan = *reinterpret_cast<int *>(self + 0x1ddd8) + *reinterpret_cast<int *>(self + 0x1ddd0) + rectY0;
                    if (ny >= rectY0 && ny < rectYSpan) {
                        int rectX0 = *reinterpret_cast<int *>(self + 0x1dda4);
                        int xv = nx;
                        if ((*g_0094988c & 1) == 0) {
                            if (nx < rectX0) {
                                xv = *g_00949870 + nx;
                            }
                            int rectXSpan0 = *reinterpret_cast<int *>(self + 0x1ddd4) + *reinterpret_cast<int *>(self + 0x1ddcc) + rectX0;
                            if (rectXSpan0 <= xv) {
                                xv = xv - *g_00949870;
                            }
                        }
                        int rectXSpan = *reinterpret_cast<int *>(self + 0x1ddd4) + *reinterpret_cast<int *>(self + 0x1ddcc) + rectX0;
                        if (xv >= rectX0 && xv < rectXSpan) {
                            doDirect = true;
                        }
                    }
                }
                if (doDirect) {
                    direct_dest_line(curX, curY, nx, ny, a7, a9, a10, a11);
                }
            }

            if (nx == a3 && ny == a4) {
                break;
            }
            if ((g_x96c9e0[a6 * 0x833] & 0x200) == 0 &&
                (terrain[4 + (*g_0068faf0 * ny + (nx >> 1)) * 0x2c] & (1 << (a6 & 0x1f))) == 0 &&
                (*g_009a64c0 & 0x80) == 0 &&
                (*g_009a649c & 0x100) == 0) {
                direct = true;
            }
            steps = steps + 1;
            if (*g_00949870 * 2 < steps) {
                direct = true;
                break;
            }
            curX = nx;
            curY = ny;
            if (direct) {
                break;
            }
        }
        retryCount = retryCount + 1;
    }
}
