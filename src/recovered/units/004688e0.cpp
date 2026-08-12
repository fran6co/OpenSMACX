// ORIGINAL: 0x004688E0 FILE
// name      ?gen_overlays@MapWin@@QAEXHH@Z
// size      741 bytes
// spans     0x004688E0-0x00468BC5
// prototype void (__thiscall ?gen_overlays@MapWin@@QAEXHH@Z)(MapWin* this, int xCoord, int yCoord)
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00462F00 0x005DAC70 0x005DACB0 0x005DD130 0x006453E0 0x00645470 0x0064FC88
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004688E0
// measured tier  NO_COMPILE
// refusal        u004688e0.cpp(204) : error C2079: 'buffer_' uses undefined class 'Buffer'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004688e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?gen_overlays@MapWin@@QAEXHH@Z  at 0x004688E0  (741 bytes)
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

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
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

class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
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
    int set_font(Font *, Font *, Font *, Font *);
    int write_cent_l(char *, RECT *, int);
    void set_text_color(int, int, int, int);
};
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
extern "C" unsigned int strlen(const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00468bc8 = (int *)0x00468BC8;
static int *const g_00686360 = (int *)0x00686360;
static int *const g_00686364 = (int *)0x00686364;
static int *const g_00686368 = (int *)0x00686368;
static int *const g_0068636c = (int *)0x0068636C;
static int *const g_00686370 = (int *)0x00686370;
static int *const g_00686374 = (int *)0x00686374;
static int *const g_00686378 = (int *)0x00686378;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_009472ec = (int *)0x009472EC;
static int *const g_00949a30 = (int *)0x00949A30;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b86a1 = (int *)0x009B86A1;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;

    void tile_to_pixel(int, int, int *, int *);
    void gen_overlays(int, int);
};

void MapWin::gen_overlays(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    int flags = *reinterpret_cast<int *>(self + 0x1dd70);
    if ((flags & 0x20) == 0) {
        return;
    }

    int pixel_x, pixel_y;
    tile_to_pixel(a1, a2, &pixel_x, &pixel_y);

    int loop_i = 1;
    int loop_offset = 0x20cc;
    int *table_ptr = (int *)0x9472ec;

    while ((int)table_ptr < 0x949a30) {
        int flags2 = *((int *)0x9a64c0) & 0x80;

        if (flags2 == 0 && loop_i != *((int *)0x939284)) {
            table_ptr++;
            loop_i++;
            loop_offset += 0x20cc;
            continue;
        }

        int row_idx = 0;
        short *entry_ptr = (short *)((int)0x96e3e4 + loop_offset - 8);

        while (row_idx < 0x4b) {
            int entry_x = *(int *)((char *)entry_ptr + 4);
            int entry_y = *(int *)((char *)entry_ptr + 8);
            short flags_word = *entry_ptr;
            short height = entry_ptr[1];

            if (a1 == entry_x && a2 == entry_y && flags_word >= 0 && height != 0) {
                if (height < 0) {
                    int cmp_idx = 0;
                    int *cmp_ptr = (int *)((int)0x96e3e4 + loop_offset);

                    while (cmp_idx < 0x4b) {
                        if (cmp_idx != row_idx && a1 == cmp_ptr[-1] && a2 == *cmp_ptr) {
                            if (flags_word == (short)cmp_ptr[-2]) {
                                *entry_ptr = -1;
                                break;
                            }
                        }
                        cmp_idx++;
                        cmp_ptr += 4;
                    }
                }

                if (*entry_ptr < 0) {
                    goto next_row;
                }

                // Draw the overlay
                int px = *reinterpret_cast<int *>(self + 0x1ddb4) + pixel_x;
                int py = *reinterpret_cast<int *>(self + 0x1ddb8) + pixel_y;

                *((char *)0x9b86a0) = 0;

                if (flags2 != 0) {
                    int *str_table = table_ptr;
                    *((int *)0x9bbfec) = str_table[-1];
                    *((int *)0x9bbff0) = *str_table;
                    strcat((char *)0x9b86a0, (const char *)((int)str_table - 0x1c));
                    *((char *)0x9b86a1) = 0;
                }

                // Handle sprite type
                int tile_type = *(short *)((char *)0x96e3e4 + (row_idx << 4) + loop_offset);
                int color = 0xff;

                if (tile_type <= 0x29) {
                    char case_val = *((char *)0x468be8 + tile_type);
                    switch (case_val) {
                        case 0: strcat((char *)0x9b86a0, (const char *)0x686360); color = 0xf9; break;
                        case 2: strcat((char *)0x9b86a0, (const char *)0x686364); color = 0xfb; break;
                        case 3: strcat((char *)0x9b86a0, (const char *)0x686378); color = 0xfd; break;
                        case 6: strcat((char *)0x9b86a0, (const char *)0x686370); color = 0xfc; break;
                        case 8: strcat((char *)0x9b86a0, (const char *)0x68636c); color = 0xfe; break;
                        case 9: strcat((char *)0x9b86a0, (const char *)0x686368); color = 0xfa; break;
                        case 0x29: strcat((char *)0x9b86a0, (const char *)0x686374); color = 0xff; break;
                    }
                }

                short height_val = *(short *)((char *)0x96e3e4 + (row_idx << 4) + loop_offset + 2);
                int abs_height = height_val;
                if (height_val < 0) abs_height = -height_val;

                char height_str[80];
                ((int (*)(int, char *, int))_itoa)(abs_height, height_str, 10);
                strcat((char *)0x9b86a0, height_str);

                // Call drawing functions through vtable
                void *vtable = *((void **)self);
                void **vtable_ptr = (void **)vtable;
                Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x1ddb4);

                // set_text_color
                typedef void (*SetTextColorFn)(Buffer *, int, int, int, int);
                SetTextColorFn set_text_color = (SetTextColorFn)vtable_ptr[1];
                set_text_color(buffer, color, 0, 1, 1);

                // set_font
                typedef int (*SetFontFn)(Buffer *, Font *, Font *, Font *, Font *);
                SetFontFn set_font = (SetFontFn)vtable_ptr[1];
                set_font(buffer, 0, 0, 0, (Font *)(self + 0x1eef4));

                // write_cent_l
                int len = strlen((const char *)0x9b86a0);
                typedef int (*WriteCentLFn)(Buffer *, char *, RECT *, int);
                WriteCentLFn write_cent_l = (WriteCentLFn)vtable_ptr[1];
                write_cent_l(buffer, (char *)0x9b86a0, (RECT *)&px, len);

                break;
            }

        next_row:
            row_idx++;
            entry_ptr = (short *)((char *)entry_ptr + 0x10);
        }

        table_ptr = (int *)((char *)table_ptr + 0x167 * 4);
        loop_i++;
        loop_offset += 0x20cc;
    }
}
