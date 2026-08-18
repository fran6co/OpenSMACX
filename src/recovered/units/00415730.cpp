// ORIGINAL: 0x00415730 ?draw_misc@BaseWin@@QAEXH@Z 0x00415730-0x00415B15 FILE
// size      997 bytes
// prototype void (__thiscall ?draw_misc@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   1   call targets   13
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005882F0 0x005C89B0 0x005D5630 0x005D5930 0x005D8000 0x005DAC70 0x005DACB0 0x005DCEA0 0x005E2DD3 0x006169A0 0x006453E0 0x00645470 0x0064FC88
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00415730
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00415730/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_misc@BaseWin@@QAEXH@Z  at 0x00415730  (997 bytes)
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
class BaseWin;
struct BoxSpriteParams;
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
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Strings;
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
    int box_sprite(RECT *, BoxSpriteParams *);
    int set_clip(RECT *);
    int set_font(Font *, Font *, Font *, Font *);
    int write_l(char *, int, int, int);
    void set_text_color(int, int, int, int);
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
    void soft_update(RECT *);
    void update(RECT *, GraphicWin *);
};
class Strings { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    BOOL is_populated_;
    int get(int);
};
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
extern "C" unsigned int strlen(const char *);
int __cdecl find_font(int, int);
void __cdecl say_year(char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_0078d5a0 = (int *)0x0078D5A0;
static int *const g_008c6ce4 = (int *)0x008C6CE4;
static int *const g_008c6ce8 = (int *)0x008C6CE8;
static int *const g_008c6cec = (int *)0x008C6CEC;
static int *const g_008c6cf0 = (int *)0x008C6CF0;
static int *const g_008c6cfc = (int *)0x008C6CFC;
static int *const g_008c6d00 = (int *)0x008C6D00;
static int *const g_008c6d04 = (int *)0x008C6D04;
static int *const g_008c6d08 = (int *)0x008C6D08;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void draw_misc(int);
};

// INDEXED TABLE BASE: 0x96cc00 is added to a register-computed byte offset
// (`[ebx + 0x96cc00]`), which the emitter's immediate-operand scan does not
// catch - it only lists plain-address globals, not base-immediate/
// offset-register forms. An extern array keeps the relocation the indexing
// needs; a `static int *const` would let /O2 fold the literal and drop the
// addressing mode entirely.
extern int g_0096cc00[];

// The four `test eax,eax; je ...` checks in this function test the ADDRESS
// of this scratch buffer, not its contents. Reproducing them needs a symbol
// VC6 cannot constant-fold, unlike the scaffold's `static int *const
// g_009b86a0`, whose value the compiler knows at compile time and would
// fold the always-true test away entirely.
extern char g_009b86a0_buf[];

// The scaffolding's `_itoa()` takes no arguments (a fallback); the real
// 3-argument call is declared here under a different name. The relocation
// it needs is masked by the comparison regardless of the symbol it names.
extern "C" char *__cdecl my_itoa(int, char *, int);

void BaseWin::draw_misc(int a1) {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);

    RECT r;
    r.left = *reinterpret_cast<int *>(self + 0x40d1c);
    r.top = *reinterpret_cast<int *>(self + 0x40d20);
    r.right = *reinterpret_cast<int *>(self + 0x40d24);
    r.bottom = *reinterpret_cast<int *>(self + 0x40d28);
    buf->set_clip(&r);

    buf->box_sprite(&r, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));
    r.top += 3;
    r.right -= 3;
    r.left += 3;
    r.bottom -= 3;

    buf->box_sprite(&r, reinterpret_cast<BoxSpriteParams *>(g_0078d528));
    r.left += 3;
    r.right -= 3;
    r.top += 3;
    r.bottom -= 3;

    Font *font = reinterpret_cast<Font *>(find_font(0xc, 0));
    buf->set_font(font, 0, 0, 0);

    int advance = (font->unk_1_ < 0) ? font->line_height_
                                      : font->height_ + font->unk_1_;
    int rowHeight = (r.bottom - advance * 3) - r.top;
    int y = r.top + rowHeight / 2;
    int x = r.left;

    buf->set_text_color(*g_008c6cfc, *g_008c6d00, 1, 1);
    g_009b86a0_buf[0] = 0;
    {
        int *base = *reinterpret_cast<int **>(g_009b90f8);
        int id = *reinterpret_cast<int *>(reinterpret_cast<char *>(base) + 0xb68);
        char *s = reinterpret_cast<char *>(
            reinterpret_cast<Strings *>(g_009b90d8)->get(id));
        strcat(g_009b86a0_buf, s);
    }
    if (g_009b86a0_buf) {
        int len = strlen(g_009b86a0_buf);
        x = buf->write_l(g_009b86a0_buf, x, y, len);
    }

    buf->set_text_color(*g_008c6d04, *g_008c6d08, 1, 1);
    g_009b86a0_buf[0] = 0;
    strcat(g_009b86a0_buf, reinterpret_cast<char *>(g_00682820));
    say_year(g_009b86a0_buf);
    if (g_009b86a0_buf) {
        int len = strlen(g_009b86a0_buf);
        buf->write_l(g_009b86a0_buf, x, y, len);
    }

    int advance2 = (font->unk_1_ < 0) ? font->line_height_
                                       : font->height_ + font->unk_1_;
    y += advance2;
    x = r.left;
    unsigned char b = *reinterpret_cast<unsigned char *>(
        reinterpret_cast<char *>(*reinterpret_cast<int **>(g_0090ea30)) + 4);

    g_009b86a0_buf[0] = 0;
    {
        int *base2 = *reinterpret_cast<int **>(g_009b90f8);
        int id2 = *reinterpret_cast<int *>(reinterpret_cast<char *>(base2) + 0x90);
        char *s2 = reinterpret_cast<char *>(
            reinterpret_cast<Strings *>(g_009b90d8)->get(id2));
        strcat(g_009b86a0_buf, s2);
    }
    strcat(g_009b86a0_buf, reinterpret_cast<char *>(g_00682e94));
    buf->set_text_color(*g_008c6cfc, *g_008c6d00, 1, 1);
    if (g_009b86a0_buf) {
        int len = strlen(g_009b86a0_buf);
        x = buf->write_l(g_009b86a0_buf, x, y, len);
    }

    int idx = static_cast<int>(b) * 2099;
    int val1 = g_0096cc00[idx];
    int val2 = g_0096cc00[idx + 1];
    g_009b86a0_buf[0] = 0;
    {
        char numbuf[80];
        my_itoa(val1 - val2, numbuf, 10);
        strcat(g_009b86a0_buf, numbuf);
    }
    buf->set_text_color(*g_008c6d04, *g_008c6d08, 1, 1);
    if (g_009b86a0_buf) {
        int len = strlen(g_009b86a0_buf);
        buf->write_l(g_009b86a0_buf, x, y, len);
    }

    int advance3 = (font->unk_1_ < 0) ? font->line_height_
                                       : font->height_ + font->unk_1_;
    y += advance3;
    x = r.left;
    g_009b86a0_buf[0] = 0;
    {
        int *base3 = *reinterpret_cast<int **>(g_009b90f8);
        int id3 = *reinterpret_cast<int *>(reinterpret_cast<char *>(base3) + 0xef8);
        char *s3 = reinterpret_cast<char *>(
            reinterpret_cast<Strings *>(g_009b90d8)->get(id3));
        strcat(g_009b86a0_buf, s3);
    }
    strcat(g_009b86a0_buf, reinterpret_cast<char *>(g_00682e94));
    strcat(g_009b86a0_buf, reinterpret_cast<char *>(g_00682820));
    int someVal = *reinterpret_cast<int *>(
        reinterpret_cast<char *>(*reinterpret_cast<int **>(g_0090ea30)) + 0x48);
    {
        char numbuf2[80];
        my_itoa(someVal, numbuf2, 10);
        strcat(g_009b86a0_buf, numbuf2);
    }
    int colorA, colorB;
    if (someVal != 0) {
        colorA = *g_008c6cec;
        colorB = *g_008c6cf0;
    } else {
        colorA = *g_008c6ce4;
        colorB = *g_008c6ce8;
    }
    buf->set_text_color(colorA, colorB, 1, 1);
    if (g_009b86a0_buf) {
        int len = strlen(g_009b86a0_buf);
        buf->write_l(g_009b86a0_buf, x, y, len);
    }

    if (a1 != 0) {
        reinterpret_cast<GraphicWin *>(self)->update(
            reinterpret_cast<RECT *>(self + 0x40d1c), 0);
        return;
    }
    reinterpret_cast<GraphicWin *>(self)->soft_update(
        reinterpret_cast<RECT *>(self + 0x40d1c));
}
