// ORIGINAL: 0x00510F50 ?ready@Console@@QAEXXZ 0x00510F50-0x00511471 FILE
// size      1313 bytes
// prototype void (__thiscall ?ready@Console@@QAEXXZ)(Console* this)
// callers   3   call targets   15
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00446960 0x00446D10 0x00458900 0x00460DD0 0x0046A550 0x0046AD60 0x0046AE00 0x0046B1F0 0x0046FB10 0x004B9EA0 0x004E3A50 0x005108A0 0x005109B0 0x00510CE0 0x005FD120
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00510F50
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00510f50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?ready@Console@@QAEXXZ  at 0x00510F50  (1313 bytes)
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
class FX;
class Font;
class GraphicWin;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
class InfoWin;
typedef char * LPSTR;
typedef void * LPVOID;
class MainMenu;
class MapWin;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class StatusWin;
class Win;

// ---- callees, declared and never defined (a definition would be inlined) ----
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

class FX { public:
    uint8_t effects_[0x61 * 0x6C];
    uint32_t field_28EC_;
    void play(int, int, int);
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

class InfoWin { public:
    void change(int);
};

class MainMenu { public:
    void check(int);
};

class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;
    void draw_cursor();
    void draw_map(int);
    void main_caption();
    void set_center(int, int, int);
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

class StatusWin { public:
    void redraw();
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

int base_at(int, int);
void ambience_veh(int);
void __cdecl draw_cursor();
void flush_input();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00749cf8 = (int *)0x00749CF8;
static int *const g_007ad2a0 = (int *)0x007AD2A0;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_008c5568 = (int *)0x008C5568;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939288 = (int *)0x00939288;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0095283c = (int *)0x0095283C;
static int *const g_00952844 = (int *)0x00952844;
static int *const g_00952852 = (int *)0x00952852;
static int *const g_0097d150 = (int *)0x0097D150;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009ab88c = (int *)0x009AB88C;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void focus(int, int, int);
    void cursor_next(int, int);
    int ready_search(int);
    void ready();
};
extern unsigned char tbl_952839[];
extern short tbl_95282a[];
extern short tbl_952828[];
extern unsigned int tbl_95282c[];
extern short tbl_952832[];
extern unsigned char tbl_952836[];
extern unsigned short tbl_952830[];
extern short tbl_95283c[];
extern short tbl_952844[];
extern unsigned char tbl_952852[];
extern short tbl_97d150[];
extern unsigned char tbl_94a379[];
extern signed char tbl_94a37a[];
extern unsigned char tbl_9ab88c[];
extern unsigned char tbl_9ab88d[];
extern unsigned char tbl_94ae68[];
extern short tbl_97d040[];
extern short tbl_97d042[];
extern unsigned char tbl_94a30c[];

void Console::ready() {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<int *>(self + 0x23bf8) = 0;
    draw_cursor();

    int arg;
    if (*reinterpret_cast<int *>(self + 0x23d18) == 0)
        arg = *reinterpret_cast<int *>(self + 0x23bdc);
    else
        arg = -1;

    int base = ready_search(arg);
    *reinterpret_cast<int *>(self + 0x23bdc) = base;
    *reinterpret_cast<int *>(self + 0x23d18) = 0;

    if (base < 0)
        return;
    if (*g_00939288 == 0)
        return;

    if (tbl_952839[base * 0x34] == 0 || static_cast<signed char>(tbl_952839[base * 0x34]) > 0x17) {
        short y = tbl_95282a[base * 0x1a];
        short x = tbl_952828[base * 0x1a];
        *reinterpret_cast<int *>(self + 0x23bf8) = 0;
        if (*reinterpret_cast<int *>(self + 0x23c00) != 0) {
            *reinterpret_cast<int *>(self + 0x23c00) = 0;
            reinterpret_cast<MapWin *>(*g_007d3c3c)->draw_map(1);
        }
        reinterpret_cast<MapWin *>(*g_007d3c3c)->draw_cursor();
        cursor_next(x, y);
        reinterpret_cast<InfoWin *>(g_007ad2a0)->change(0);
        reinterpret_cast<StatusWin *>(g_008c5568)->redraw();
        reinterpret_cast<MapWin *>(*g_007d3c3c)->main_caption();

        if ((*g_009a6490 & 0x80000) == 0 ||
            ((tbl_952839[*reinterpret_cast<int *>(self + 0x23bdc) * 0x34] == 0 &&
              (tbl_95282c[*reinterpret_cast<int *>(self + 0x23bdc) * 0xd] & 0x200) == 0) &&
             (tbl_95282c[*reinterpret_cast<int *>(self + 0x23bdc) * 0xd] & 0x1000200) != 0x1000200)) {
            int b = *reinterpret_cast<int *>(self + 0x23bdc);
            focus(tbl_952828[b * 0x1a], tbl_95282a[b * 0x1a], tbl_952836[b * 0x34]);
            {
                int yv = tbl_95282a[*reinterpret_cast<int *>(self + 0x23bdc) * 0x1a];
                int xv = tbl_952828[*reinterpret_cast<int *>(self + 0x23bdc) * 0x1a];
                bool skip = false;
                unsigned char bVar1 = 0;
                if (xv >= 0 && xv < *g_00949870 && yv >= 0 && yv < *g_00949874) {
                    bVar1 = tbl_94a30c[(*g_0068faf0 * yv + (xv >> 1)) * 0x2c];
                    if ((bVar1 & 0xe0) >= 0x61 &&
                        *reinterpret_cast<int *>(self + 0x1dda8) + 3 >
                            static_cast<int>((yv - (bVar1 >> 5)) + 3)) {
                        skip = true;
                    }
                }
                if (!skip) {
                    if (*reinterpret_cast<int *>(self + 0x1dda8) + 3 <= yv &&
                        yv < *reinterpret_cast<int *>(self + 0x1ddd8) +
                                 *reinterpret_cast<int *>(self + 0x1ddd0) - 3 +
                                 *reinterpret_cast<int *>(self + 0x1dda8)) {
                        int xv2 = xv;
                        if ((*g_0094988c & 1) == 0) {
                            if (xv < *reinterpret_cast<int *>(self + 0x1dda4) + 3)
                                xv2 = xv + *g_00949870;
                            if (*reinterpret_cast<int *>(self + 0x1ddd4) +
                                    *reinterpret_cast<int *>(self + 0x1ddcc) - 3 +
                                    *reinterpret_cast<int *>(self + 0x1dda4) <=
                                xv2)
                                xv2 = xv2 - *g_00949870;
                        }
                        if (*reinterpret_cast<int *>(self + 0x1dda4) + 3 <= xv2 &&
                            xv2 < *reinterpret_cast<int *>(self + 0x1ddd4) +
                                      *reinterpret_cast<int *>(self + 0x1ddcc) - 3 +
                                      *reinterpret_cast<int *>(self + 0x1dda4))
                            goto skip_check;
                    }
                }
                reinterpret_cast<MapWin *>(*g_007d3c3c)->set_center(xv, yv, 1);
            }
        }
    skip_check:
        reinterpret_cast<MainMenu *>(self + 0x22a2c)->check(*reinterpret_cast<int *>(self + 0x23bdc));
        flush_input();
        tbl_952852[*reinterpret_cast<int *>(self + 0x23bdc) * 0x34] = 0;
    }

    int bx = tbl_952828[*reinterpret_cast<int *>(self + 0x23bdc) * 0x1a];
    int by = tbl_95282a[*reinterpret_cast<int *>(self + 0x23bdc) * 0x1a];
    int spot = base_at(bx, by);
    int b2 = *reinterpret_cast<int *>(self + 0x23bdc);

    if (tbl_952839[b2 * 0x34] == 0 && (tbl_95282c[b2 * 0xd] & 0x200) == 0 &&
        (tbl_95282c[b2 * 0xd] & 0x1000200) != 0x1000200 &&
        (tbl_95282c[b2 * 0xd] & 0x4000) == 0 && spot >= 0) {
        int idx1 = tbl_952832[b2 * 0x1a];
        int idx2 = tbl_9ab88c[idx1 * 0x34];
        int off = tbl_94a379[idx2 * 0x90];
        int cand = tbl_97d150[off + spot * 0x9a];
        if (cand >= 0 && cand != spot) {
            tbl_952839[b2 * 0x34] = 0x18;
            tbl_95283c[*reinterpret_cast<int *>(self + 0x23bdc) * 0x1a] = tbl_97d040[cand * 0x9a];
            tbl_952844[*reinterpret_cast<int *>(self + 0x23bdc) * 0x1a] = tbl_97d042[cand * 0x9a];
        }
    }

    int b3 = *reinterpret_cast<int *>(self + 0x23bdc) * 0x34;
    if (tbl_952836[b3] == *g_00939284 && (tbl_952830[b3 >> 2] & 0x1000) != 0 &&
        tbl_952839[b3] == 0 &&
        (tbl_95282c[*reinterpret_cast<int *>(self + 0x23bdc) * 0xd] & 0x200) == 0) {
        tbl_952830[b3 >> 2] = tbl_952830[b3 >> 2] & 0xefff;
        int bb = *reinterpret_cast<int *>(self + 0x23bdc);
        int idx1 = tbl_952832[bb * 0x1a] * 0x34;
        int idx2 = tbl_9ab88c[idx1] * 0x90;
        signed char c = tbl_94a379[idx2];

        int px, py;
        int soundId;
        if (c == 0) {
            if (tbl_9ab88c[idx1] == 0 && tbl_94ae68[tbl_9ab88d[idx1] * 0x10] != 0) {
                py = tbl_95282a[bb * 0x1a];
                px = tbl_952828[bb * 0x1a];
                soundId = 0x40;
            } else {
                px = tbl_952828[bb * 0x1a];
                py = tbl_95282a[bb * 0x1a];
                soundId = (tbl_94ae68[tbl_9ab88d[idx1] * 0x10] == 0) ? 0x42 : 0x41;
            }
        } else if (c == 1) {
            px = tbl_952828[bb * 0x1a];
            py = tbl_95282a[bb * 0x1a];
            soundId = (tbl_94ae68[tbl_9ab88d[idx1] * 0x10] == 0) ? 0x46 : 0x45;
        } else if (c == 2) {
            px = tbl_952828[bb * 0x1a];
            py = tbl_95282a[bb * 0x1a];
            soundId = (tbl_94a37a[idx2] == 0) ? 0x43 : (tbl_94a37a[idx2] == 1 ? 0x44 : 0x47);
        } else {
            goto done;
        }
        reinterpret_cast<FX *>(g_00749cf8)->play(soundId, px, py);
    }
done:
    ambience_veh(*reinterpret_cast<int *>(self + 0x23bdc));
    flush_input();
}
