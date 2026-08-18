// ORIGINAL: 0x00458420 ?draw_text@InfoWin@@QAEXH@Z 0x00458420-0x004588A9 FILE
// size      1161 bytes
// prototype void (__thiscall ?draw_text@InfoWin@@QAEXH@Z)(InfoWin* this, int)
// callers   6   call targets   11
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDEA0 0x005D5630 0x005D8000 0x005DAC70 0x005DACB0 0x005DACE0 0x005DC790 0x005DCEA0 0x005DD300 0x006453E0 0x00645460
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00458420
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00458420/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_text@InfoWin@@QAEXH@Z  at 0x00458420  (1161 bytes)
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
class InfoWin;
typedef char * LPSTR;
typedef void * LPVOID;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class TutWin;
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
    int set_clip(RECT *);
    int set_font(Font *, Font *, Font *, Font *);
    int text_width(char *);
    int write_l(char *, int, int, int);
    int write_right_l(char *, int, int, int, int);
    void set_text_color(int, int, int, int);
    void set_text_color2(int, int, int, int);
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
    void update(RECT *, GraphicWin *);
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

class TutWin { public:
    uint8_t unmapped_0_[0x537C];
    int32_t field_537C_;
    int32_t field_5380_;
    uint8_t unmapped_5384_[0x539C - 0x5384];
    int32_t field_539C_;
    uint8_t unmapped_53A0_[0x53A4 - 0x53A0];
    int32_t field_53A4_;
    int32_t field_53A8_;
    int32_t field_53AC_;
    uint8_t unmapped_53B0_[0x53B8 - 0x53B0];
    int32_t field_53B8_;
    uint8_t unmapped_53BC_[0x53C4 - 0x53BC];
    int32_t field_53C4_;
    uint8_t unmapped_53C8_[0x53D4 - 0x53C8];
    int32_t field_53D4_;
    void draw_arrow();
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

extern "C" char *strcpy(char *, const char *);
extern "C" unsigned int strlen(const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068f9f0 = (int *)0x0068F9F0;
static int *const g_0068fa10 = (int *)0x0068FA10;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007aec94 = (int *)0x007AEC94;
static int *const g_007af174 = (int *)0x007AF174;
static int *const g_007af178 = (int *)0x007AF178;
static int *const g_007af17c = (int *)0x007AF17C;
static int *const g_007af180 = (int *)0x007AF180;
static int *const g_008c6bb4 = (int *)0x008C6BB4;
static int *const g_008c6bb8 = (int *)0x008C6BB8;
static int *const g_008c6bbc = (int *)0x008C6BBC;
static int *const g_008c6bc0 = (int *)0x008C6BC0;
static int *const g_008c6bc4 = (int *)0x008C6BC4;
static int *const g_008c6bc8 = (int *)0x008C6BC8;
static int *const g_008c6bcc = (int *)0x008C6BCC;
static int *const g_008c6bd0 = (int *)0x008C6BD0;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_009a650c = (int *)0x009A650C;

class InfoWin { public:
    void draw_text(int);
};

extern int g_009a650c_tbl[];

void InfoWin::draw_text(int a1) {
    char *const self = reinterpret_cast<char *>(this);
    Buffer *const buf = reinterpret_cast<Buffer *>(g_007aec64);

    buf->set_font(reinterpret_cast<Font *>(self + 0x58), 0, 0, 0);

    int rowIndex = 0;
    int start, end;
    if (a1 == -1) {
        start = 0;
        end = 2;
    } else {
        start = a1;
        end = a1 + 1;
    }

    for (int col = start; col < end; ++col) {
        RECT *const clip = reinterpret_cast<RECT *>(self + (col + 1) * 0x10);
        buf->set_clip(clip);

        int left = clip->left + 3;
        int right = clip->right - 3;

        int *const colInfo = reinterpret_cast<int *>(self + 0x9b8 + col * 4);
        int height = colInfo[4];
        int rowCount = colInfo[0];
        int base = col * 0xc + 0x23e;

        while (rowIndex < rowCount) {
            if (height < 0) {
                break;
            }

            char *const text = self - 0xb2a8 + (base + rowIndex) * 0x50;
            unsigned int textLen = strlen(text);

            bool cont = (colInfo[-0x24c] == 4) && (rowIndex >= 1);

            if (textLen != 0) {
                if (!cont) {
                    int colorIdx = *reinterpret_cast<int *>(
                        self + (base + rowIndex) * 4);
                    if (colorIdx == 0) {
                        int c0, c1;
                        if (*reinterpret_cast<int *>(self + 0x94) == 0) {
                            buf->set_text_color(*g_008c6bb4, *g_008c6bb8, 1, 1);
                            c0 = *g_008c6bc4;
                            c1 = *g_008c6bc8;
                        } else {
                            buf->set_text_color(*g_008c6bbc, *g_008c6bc0, 1, 1);
                            c0 = *g_008c6bcc;
                            c1 = *g_008c6bd0;
                        }
                        buf->set_text_color2(c0, c1, 1, 1);
                    } else if (*reinterpret_cast<int *>(self + 0x94) == 0) {
                        buf->set_text_color(colorIdx, -1, 1, 1);
                    } else {
                        buf->set_text_color(*reinterpret_cast<int *>(
                            self + (base + rowIndex) * 4 + 0x60), -1, 1, 1);
                    }
                } else {
                    int *const slot = g_009a650c_tbl - rowIndex;
                    buf->set_text_color(g_0068f9f0[*slot], g_0068fa10[*slot], 1, 1);
                }

                if (static_cast<int>(textLen) < height) {
                    if (!cont) {
                        int color = *reinterpret_cast<int *>(
                            self + (base + rowIndex) * 4 - 0x60);
                        int arg = *reinterpret_cast<int *>(
                            self + (base + rowIndex) * 4 - 0xc0);
                        if (text != 0) {
                            unsigned int len = strlen(text);
                            buf->write_l(text, arg, color, len);
                        }
                    } else {
                        int color = *reinterpret_cast<int *>(
                            self + (base + rowIndex) * 4 - 0x60);
                        if (text != 0) {
                            unsigned int len = strlen(text);
                            buf->write_right_l(text, 2, color, (right - left) - 4, len);
                        }
                    }
                } else {
                    char row[80];
                    int width;
                    int color;
                    if (height != 0) {
                        strcpy(row, text);
                        row[height - 1] = 0;
                        if (!cont) {
                            color = *reinterpret_cast<int *>(
                                self + (base + rowIndex) * 4 - 0x60);
                            width = *reinterpret_cast<int *>(
                                self + (base + rowIndex) * 4 - 0xc0);
                        } else {
                            int textWidth = buf->text_width(text);
                            width = ((right - left) - textWidth) - 2;
                            color = *reinterpret_cast<int *>(
                                self + (base + rowIndex) * 4 - 0x60);
                        }
                        unsigned int len = strlen(row);
                        width = buf->write_l(row, width, color, len);
                    }
                    strcpy(row, text);
                    bool colorFromField = !cont;
                    row[height] = 0;
                    if (colorFromField) {
                        if (*reinterpret_cast<int *>(
                                self + (base + rowIndex) * 4) == 0) {
                            buf->set_text_color(*g_008c6bbc, *g_008c6bb8, 1, 1);
                            buf->set_text_color2(*g_008c6bcc, *g_008c6bd0, 1, 1);
                        } else {
                            buf->set_text_color(*reinterpret_cast<int *>(
                                self + (base + rowIndex) * 4 + 0x60), -1, 1, 1);
                        }
                    }
                    color = *reinterpret_cast<int *>(
                        self + (base + rowIndex) * 4 - 0x60);
                    char *const tail = row + height - 1;
                    if (tail != 0) {
                        unsigned int len = strlen(tail);
                        buf->write_l(tail, width, color, len);
                    }
                    *g_007af174 = 0;
                    *g_007af17c = 0;
                    *g_007af178 = 0;
                    *g_007af180 = 0;
                }
            }

            height -= textLen;
            ++rowIndex;
        }
    }

    buf->set_clip(reinterpret_cast<RECT *>(g_007aec94));

    RECT *finalRect;
    if (a1 == -1) {
        finalRect = reinterpret_cast<RECT *>(self);
    } else {
        finalRect = reinterpret_cast<RECT *>(self + (a1 + 1) * 0x10);
    }
    reinterpret_cast<GraphicWin *>(g_007ae820)->update(finalRect, 0);
    reinterpret_cast<TutWin *>(g_008c6e68)->draw_arrow();
}
