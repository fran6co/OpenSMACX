// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00435400
// name           ?draw_armor@DesignWin@@QAEXH@Z
// size           1024 bytes
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00435400/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_armor@DesignWin@@QAEXH@Z  at 0x00435400  (1024 bytes)
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
    void render_multi_table_dest(Buffer *, int, int, int, unsigned char *, int);
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
int __cdecl vehdraw_construct_armor(int, int, int, int, int, Caviar *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_006eef7c = (int *)0x006EEF7C;
static int *const g_008c6dcc = (int *)0x008C6DCC;
static int *const g_008c6dd0 = (int *)0x008C6DD0;
static int *const g_008c6dd4 = (int *)0x008C6DD4;
static int *const g_008c6dd8 = (int *)0x008C6DD8;
static int *const g_008c6ddc = (int *)0x008C6DDC;
static int *const g_008c6de0 = (int *)0x008C6DE0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DesignWin { public:
    uint8_t pad_0_[0x12764];
    Spot spot_;

    void draw_armor(int);
};

typedef char *(__stdcall *CharUpperAProc)(char *);
typedef char *(__cdecl *ItoaProc)(int, char *, int);

extern int g_armor_stat_table[]; // stride 0x10(4 ints), indexed by self.14204
extern signed char g_armor_stat_flags[]; // stride 0x10, byte at +1 signed, byte at +5 unsigned

void DesignWin::draw_armor(int a1) {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);

    RECT r = *reinterpret_cast<RECT *>(self + 0xacc);
    buf->set_clip(&r);
    buf->copy(this ? buf : 0, &r);

    Font *fontA = reinterpret_cast<Font *>(self + 0x14540);
    Font *fontB = reinterpret_cast<Font *>(self + 0x14568);

    RECT r2 = r;
    int titleBottom = r.top + (fontB->unk_1_ < 0 ? fontB->line_height_ : fontB->height_ + fontB->unk_1_);
    r2.bottom = titleBottom;
    r.top = titleBottom;

    char *name = reinterpret_cast<char *>(g_009b86a0);
    name[0] = 0;
    strcat(name, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x1f8))));
    CharUpperAProc charUpperA = reinterpret_cast<CharUpperAProc>(*g_0066931c);
    charUpperA(name);

    buf->set_text_color(*g_008c6dcc, *g_008c6dd0, 1, 1);
    if (name) {
        buf->write_cent_l(fontB, name, &r2, strlen(name));
    }

    r.bottom += fontA->height_ * -2 - 5;

    if (*reinterpret_cast<int *>(self + 0x14204) != 0) {
        *reinterpret_cast<int *>(self + 0x13b68) = 0;
        vehdraw_construct_armor(*g_00939284, *reinterpret_cast<int *>(self + 0x141fc),
                                 *reinterpret_cast<int *>(self + 0x14200),
                                 *reinterpret_cast<int *>(self + 0x14204), 1,
                                 reinterpret_cast<Caviar *>(self + 0x1279c));
        Caviar *caviar = reinterpret_cast<Caviar *>(self + 0x1279c);
        caviar->set_scaling(0.07f);
        caviar->set_scene_rotation(0.5233333706855774f, 0, 0);
        if (*reinterpret_cast<int *>(self + 0x14204) == 5) {
            caviar->render_multi_table_dest(this ? buf : 0, (r.right + r.left) / 2,
                                            (r.bottom + r.top) / 2, 0x18,
                                            reinterpret_cast<unsigned char *>(g_006eef7c), 6);
        } else {
            caviar->render(this ? buf : 0, (r.right + r.left) / 2, (r.bottom + r.top) / 2, 6);
        }
    }

    buf->set_font(fontA, 0, 0, 0);
    int lineTop = r.bottom;
    r.bottom += fontA->height_;
    buf->set_text_color(*g_008c6dd4, *g_008c6dd8, 1, 1);

    name[0] = 0;
    int statSel = *reinterpret_cast<int *>(self + 0x14204);
    int statId = (statSel == 0) ? *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 100)
                                 : g_armor_stat_table[statSel * 4];
    strcat(name, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(statId)));
    charUpperA(name);

    int avail = r.right - r.left;
    int nameWidth = fontB->width(name);
    if (avail < nameWidth) {
        int offset = fontB->unk_1_ < 0 ? fontB->line_height_ : fontB->height_ + fontB->unk_1_;
        r.bottom -= offset;
        lineTop -= offset;
        r.top = lineTop;
        buf->wrap_cent(name, &r);
        offset = fontB->unk_1_ < 0 ? fontB->line_height_ : fontB->height_ + fontB->unk_1_;
        lineTop += offset;
        r.bottom += offset;
    } else if (name) {
        r.top = lineTop;
        buf->write_cent_l(name, &r, strlen(name));
    }

    int advance = fontA->height_ + 3;
    lineTop += advance;
    r.bottom += advance;
    r.top = lineTop;
    buf->set_text_color(*g_008c6ddc, *g_008c6de0, 1, 1);

    if (g_armor_stat_flags[statSel * 0x10 + 5] < 3) {
        name[0] = 0;
        strcat(name, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xa6c))));
        strcat(name, reinterpret_cast<char *>(g_00682820));
        signed char rawStat = g_armor_stat_flags[statSel * 0x10 + 4];
        if (rawStat < 0) {
            strcat(name, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x310))));
        } else {
            char numBuf[80];
            reinterpret_cast<ItoaProc>(_itoa)(rawStat, numBuf, 10);
            strcat(name, numBuf);
        }
        if (name) {
            buf->write_cent_l(name, &r, strlen(name));
        }
    }
}
