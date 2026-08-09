// ORIGINAL: 0x00434BC0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00434BC0
// name           ?draw_chassis@DesignWin@@QAEXH@Z
// size           886 bytes
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00434bc0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_chassis@DesignWin@@QAEXH@Z  at 0x00434BC0  (886 bytes)
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
class Caviar;
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
    int wrap_cent(char *, RECT *);
    int write_cent_l(Font *, char *, RECT *, int);
    int write_cent_l(char *, RECT *, int);
    void set_text_color(int, int, int, int);
};
class Caviar { public:
    float scene_scale_;
    uint8_t unmapped_04_[0x2C - 0x04];
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint8_t unmapped_38_[0x108 - 0x38];
    int32_t field_108_;
    uint8_t unmapped_10C_[0x13D0 - 0x10C];
    int render(Buffer *, int, int, int);
    void set_scaling(float);
    void set_scene_rotation(float, float, float);
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
    int width(char *);
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
int __cdecl vehdraw_construct_chassis(int, int, int, int, int, Caviar *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_008c6dcc = (int *)0x008C6DCC;
static int *const g_008c6dd0 = (int *)0x008C6DD0;
static int *const g_008c6dd4 = (int *)0x008C6DD4;
static int *const g_008c6dd8 = (int *)0x008C6DD8;
static int *const g_008c6ddc = (int *)0x008C6DDC;
static int *const g_008c6de0 = (int *)0x008C6DE0;
static int *const g_0094f1a8 = (int *)0x0094F1A8;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DesignWin { public:
    uint8_t pad_0_[0x12764];
    Spot spot_;

    void draw_chassis(int);
};

extern unsigned char g_tbl_94a330[];
extern unsigned char g_tbl_94a378[];
extern unsigned char g_tbl_94a379[];

void DesignWin::draw_chassis(int a1) {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    char *msg = reinterpret_cast<char *>(g_009b86a0);

    RECT rect = *reinterpret_cast<RECT *>(self + 0xaac);

    buf->set_clip(&rect);
    reinterpret_cast<Buffer *>(self + 0x13c6c)->copy(this ? buf : (Buffer *)0, &rect);

    RECT rect2;
    rect2.left = rect.left;
    rect2.right = rect.right;

    Font *font1 = reinterpret_cast<Font *>(self + 0x14568);
    int adj1 = (font1->unk_1_ < 0) ? font1->line_height_ : (font1->height_ + font1->unk_1_);
    rect.top = rect.top + adj1;
    rect2.top = rect.top - adj1;

    *msg = 0;
    {
        int idx = *reinterpret_cast<int *>(*reinterpret_cast<int *>(g_009b90f8) + 0x140);
        strcat(msg, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(idx)));
    }
    (*reinterpret_cast<void(__stdcall **)(char *)>(g_0066931c))(msg);

    buf->set_text_color(*g_008c6dcc, *g_008c6dd0, 1, 1);

    {
        unsigned int len = strlen(msg);
        buf->write_cent_l(font1, msg, &rect2, len);
    }

    *reinterpret_cast<int *>(self + 0x13b68) = 0;

    int index = *reinterpret_cast<int *>(self + 0x141fc);
    int lineHeight = *reinterpret_cast<int *>(self + 0x14550);

    rect.bottom = rect.bottom - lineHeight * 2 - 5;

    vehdraw_construct_chassis(*reinterpret_cast<int *>(self + 0x141f4), index,
                              *reinterpret_cast<int *>(self + 0x14200), 0,
                              *reinterpret_cast<int *>(self + 0x1420c),
                              reinterpret_cast<Caviar *>(self + 0x1279c));

    Caviar *caviar = reinterpret_cast<Caviar *>(self + 0x1279c);
    caviar->set_scaling(0.05f);
    caviar->set_scene_rotation(0.5233333706855774f, 0, 0);
    caviar->render(this ? buf : (Buffer *)0, (rect.right + rect.left) / 2,
                   (rect.bottom + rect.top) / 2, 2);

    buf->set_font(reinterpret_cast<Font *>(self + 0x14540), 0, 0, 0);

    rect2.top = rect.bottom;
    rect.bottom = rect.bottom + lineHeight;

    buf->set_text_color(*g_008c6dd4, *g_008c6dd8, 1, 1);

    *msg = 0;
    {
        int val = *reinterpret_cast<int *>(g_tbl_94a330 + index * 0x90);
        strcat(msg, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(val)));
    }
    (*reinterpret_cast<void(__stdcall **)(char *)>(g_0066931c))(msg);

    int avail = rect.right - rect.left;
    Font *font2 = reinterpret_cast<Font *>(self + 0x14540);
    int need = font2->width(msg);

    if (avail < need) {
        Font *font3 = reinterpret_cast<Font *>(self + 0x14568);
        int adj2 = (font3->unk_1_ < 0) ? font3->line_height_ : (font3->height_ + font3->unk_1_);
        rect.bottom = rect.bottom - adj2;
        rect2.top = rect2.top - adj2;

        buf->wrap_cent(msg, &rect);

        Font *font4 = reinterpret_cast<Font *>(self + 0x14568);
        int adj3 = (font4->unk_1_ < 0) ? font4->line_height_ : (font4->height_ + font4->unk_1_);
        rect2.top = rect2.top + adj3;
        rect.bottom = rect.bottom + adj3;
    } else {
        unsigned int len = strlen(msg);
        buf->write_cent_l(msg, &rect, len);
    }

    int lineAdvance = *reinterpret_cast<int *>(self + 0x14550) + 3;
    rect2.top = rect2.top + lineAdvance;
    rect.bottom = rect.bottom + lineAdvance;

    buf->set_text_color(*g_008c6ddc, *g_008c6de0, 1, 1);

    *msg = 0;
    {
        unsigned char code = g_tbl_94a379[index * 0x90];
        int val = reinterpret_cast<int *>(g_0094f1a8)[code];
        strcat(msg, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(val)));
    }
    strcat(msg, reinterpret_cast<char *>(g_00682820));

    {
        char numbuf[80];
        unsigned char code = g_tbl_94a378[index * 0x90];
        reinterpret_cast<char *(__cdecl *)(int, char *, int)>(&_itoa)(code, numbuf, 10);
        strcat(msg, numbuf);
    }

    unsigned int len = strlen(msg);
    buf->write_cent_l(msg, &rect, len);
}
