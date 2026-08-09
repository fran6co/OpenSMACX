// ORIGINAL: 0x00435B30 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00435B30
// name           ?draw_special_1@DesignWin@@QAEXH@Z
// size           673 bytes
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00435b30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_special_1@DesignWin@@QAEXH@Z  at 0x00435B30  (673 bytes)
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

struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class DesignWin;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;
class Strings;

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
    int copy(Buffer *, RECT *);
    int set_clip(RECT *);
    int set_font(Font *, Font *, Font *, Font *);
    int wrap_cent(Font *, char *, RECT *);
    int wrap_cent(char *, RECT *);
    int wrap_height(char *, int);
    int write_cent_l(char *, RECT *, int);
    void set_text_color(int, int, int, int);
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
extern "C" unsigned int strlen(const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_00683554 = (int *)0x00683554;
static int *const g_00683558 = (int *)0x00683558;
static int *const g_0068355c = (int *)0x0068355C;
static int *const g_008c6dcc = (int *)0x008C6DCC;
static int *const g_008c6dd0 = (int *)0x008C6DD0;
static int *const g_008c6dd4 = (int *)0x008C6DD4;
static int *const g_008c6dd8 = (int *)0x008C6DD8;
static int *const g_008c6ddc = (int *)0x008C6DDC;
static int *const g_008c6de0 = (int *)0x008C6DE0;
static int *const g_009ab538 = (int *)0x009AB538;
static int *const g_009ab53c = (int *)0x009AB53C;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DesignWin { public:
    uint8_t pad_0_[0x12764];
    Spot spot_;

    void draw_special_1(int);
};

// Indexed table bases: g_009ab538/g_009ab53c are added to a fused
// scale-4 lea (idx*7 pre-multiplied into the index, then [reg*4+CONST]),
// so the scaffold's const-pointer spelling for them is left unused and
// fresh extern arrays stand in.
extern int tbl_ab538[];
extern int tbl_ab53c[];

void DesignWin::draw_special_1(int a1) {
    char *self = reinterpret_cast<char *>(this);
    Buffer *textBuf = reinterpret_cast<Buffer *>(self + 0x444);

    RECT rect1;
    rect1.left = *reinterpret_cast<int *>(self + 0xaec);
    rect1.top = *reinterpret_cast<int *>(self + 0xaf0);
    rect1.right = *reinterpret_cast<int *>(self + 0xaf4);
    rect1.bottom = *reinterpret_cast<int *>(self + 0xaf8);

    textBuf->set_clip(&rect1);

    Buffer *dst = reinterpret_cast<Buffer *>(self + 0x13c6c);
    Buffer *src = (this != 0) ? textBuf : 0;
    dst->copy(src, &rect1);

    Font *font1 = reinterpret_cast<Font *>(self + 0x14568);
    int bottomVal = *reinterpret_cast<int *>(self + 0x14568);
    if (bottomVal < 0) {
        bottomVal = *reinterpret_cast<int *>(self + 0x14574);
    } else {
        bottomVal = *reinterpret_cast<int *>(self + 0x14578) + bottomVal;
    }

    RECT rect2;
    rect2.left = rect1.left + 5;
    rect2.top = rect1.top + 2;
    rect2.right = rect1.right - 5;
    rect2.bottom = bottomVal + rect1.top;

    *reinterpret_cast<char *>(g_009b86a0) = 0;
    int strId = *reinterpret_cast<int *>(*reinterpret_cast<int *>(g_009b90f8) + 0x228);
    char *s = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(strId));
    strcat(reinterpret_cast<char *>(g_009b86a0), s);
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<const char *>(g_00682820));
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<const char *>(g_00683554));

    typedef char *(__stdcall *CharUpperAFn)(char *);
    (*reinterpret_cast<CharUpperAFn *>(g_0066931c))(reinterpret_cast<char *>(g_009b86a0));

    textBuf->set_text_color(*g_008c6dcc, *g_008c6dd0, 1, 1);
    rect1.top = textBuf->wrap_cent(font1, reinterpret_cast<char *>(g_009b86a0), &rect2);
    textBuf->set_font(reinterpret_cast<Font *>(self + 0x14540), 0, 0, 0);
    textBuf->set_text_color(*g_008c6ddc, *g_008c6de0, 1, 1);

    if (*reinterpret_cast<int *>(self + 0x14218) < 0) {
        *reinterpret_cast<char *>(g_009b86a0) = 0;
        int strId2 = *reinterpret_cast<int *>(*reinterpret_cast<int *>(g_009b90f8) + 0x64);
        char *s2 = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(strId2));
        strcat(reinterpret_cast<char *>(g_009b86a0), s2);
        unsigned int len = strlen(reinterpret_cast<char *>(g_009b86a0));
        textBuf->write_cent_l(reinterpret_cast<char *>(g_009b86a0), &rect1, len);
        return;
    }

    textBuf->set_text_color(*g_008c6dd4, *g_008c6dd8, 1, 1);
    *reinterpret_cast<char *>(g_009b86a0) = 0;
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<const char *>(g_00683558));
    int idx1 = *reinterpret_cast<int *>(self + 0x14218);
    char *s3 = reinterpret_cast<char *>(
        reinterpret_cast<Strings *>(g_009b90d8)->get(tbl_ab538[idx1 * 7]));
    strcat(reinterpret_cast<char *>(g_009b86a0), s3);
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<const char *>(g_00682e94));
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<const char *>(g_00682820));
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<const char *>(g_0068355c));
    int idx2 = *reinterpret_cast<int *>(self + 0x14218);
    char *s4 = reinterpret_cast<char *>(
        reinterpret_cast<Strings *>(g_009b90d8)->get(tbl_ab53c[idx2 * 7]));
    strcat(reinterpret_cast<char *>(g_009b86a0), s4);

    rect1.top = textBuf->wrap_height(reinterpret_cast<char *>(g_009b86a0), rect1.right - rect1.left);
    rect1.top = rect1.bottom - rect1.top;
    textBuf->wrap_cent(reinterpret_cast<char *>(g_009b86a0), &rect1);
}
