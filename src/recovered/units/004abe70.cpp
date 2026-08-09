// ORIGINAL: 0x004ABE70 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004ABE70
// name           ?draw_score@ReportIf@@QAEXXZ
// size           695 bytes
// measured tier  MISMATCH
// divergence     8
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004abe70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_score@ReportIf@@QAEXXZ  at 0x004ABE70  (695 bytes)
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
struct BoxSpriteParams;
class Buffer;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class ReportIf;
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
    int box_sprite(RECT *, BoxSpriteParams *);
    int set_clip(RECT *);
    int set_font(Font *, Font *, Font *, Font *);
    int write_cent_l(char *, int, int, int, int);
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
void __cdecl say_year(char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e90 = (int *)0x00682E90;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007aec94 = (int *)0x007AEC94;
static int *const g_00946d54 = (int *)0x00946D54;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class ReportIf { public:
    void draw_score();
};

// INDEXED TABLE BASE: the string table is indexed by a register
// (`[ecx*4 + 0x946d54]` after scaling by the 0x59c record stride), so it
// needs real linkage rather than a folded pointer constant.
extern uint8_t g_00946d54x[];

// g_0066931c is an import thunk slot (`call dword ptr [addr]`), not a
// plain global - it holds a __stdcall function pointer (CharUpperA).
typedef char *(__stdcall *CharUpperFn)(char *);

void ReportIf::draw_score() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(g_007aec64);
    char *msgBuf = reinterpret_cast<char *>(g_009b86a0);

    RECT *r1 = reinterpret_cast<RECT *>(self + 0x20);
    buf->set_clip(r1);

    int left = r1->left + 6;
    int top = r1->top + 6;
    *reinterpret_cast<int *>(self + 0xc) = left;
    *reinterpret_cast<int *>(self + 0x10) = top;
    int width = (r1->right - r1->left) - 0xc;
    *reinterpret_cast<int *>(self + 0x14) = width;
    int height = (r1->bottom - r1->top) - 0xc;
    *reinterpret_cast<int *>(self + 0x18) = height;

    RECT *r2 = reinterpret_cast<RECT *>(self + 0x1e1c8);
    r2->left = left;
    r2->top = top;
    r2->right = left + width;
    r2->bottom = top + height;
    buf->box_sprite(r2, reinterpret_cast<BoxSpriteParams *>(g_0078d528));
    buf->set_clip(reinterpret_cast<RECT *>(g_007aec94));

    RECT *r3 = reinterpret_cast<RECT *>(self + 0x50);
    buf->set_clip(r3);
    *reinterpret_cast<int *>(self + 0x10) = *reinterpret_cast<int *>(self + 0x54);
    int adj = (*reinterpret_cast<int *>(self + 0x1e230) != 0) ? 0x4c : 0;
    *reinterpret_cast<int *>(self + 0xc) = r3->left + adj;
    *reinterpret_cast<int *>(self + 0x14) = (r3->right - adj) - r3->left;
    *reinterpret_cast<int *>(self + 0x18) = r3->bottom - r3->top;

    *msgBuf = 0;

    Font *f0 = *reinterpret_cast<Font **>(self + 0x1e190);
    Font *f1 = *reinterpret_cast<Font **>(self + 0x1e194);
    Font *f2 = *reinterpret_cast<Font **>(self + 0x1e198);
    buf->set_font(f0, f1, f2, 0);
    buf->set_text_color(0xe1, -1, 1, 1);

    int idx = *reinterpret_cast<int *>(self + 0x1e228);
    strcat(msgBuf, reinterpret_cast<char *>(g_00946d54x + idx * 0x59c));
    (*reinterpret_cast<CharUpperFn *>(g_0066931c))(msgBuf);
    strcat(msgBuf, reinterpret_cast<char *>(g_00682820));

    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
    int *table = *reinterpret_cast<int **>(g_009b90f8);
    strcat(msgBuf, reinterpret_cast<char *>(strings->get(table[0x2b7])));

    if (*reinterpret_cast<int *>(self + 0x1e230) != 0) {
        strcat(msgBuf, reinterpret_cast<char *>(g_00682e90));
        strcat(msgBuf, reinterpret_cast<char *>(g_00682820));
        int *table2 = *reinterpret_cast<int **>(g_009b90f8);
        strcat(msgBuf, reinterpret_cast<char *>(strings->get(table2[0x2da])));
        strcat(msgBuf, reinterpret_cast<char *>(g_00682820));
        say_year(msgBuf);
    }

    int y = *reinterpret_cast<int *>(self + 0x10);
    int w = *reinterpret_cast<int *>(self + 0x14);
    int x = *reinterpret_cast<int *>(self + 0xc);
    if (msgBuf != 0) {
        unsigned int len = strlen(msgBuf);
        buf->write_cent_l(msgBuf, x, y + 10, w, len);
    }

    Font *f3 = *reinterpret_cast<Font **>(self + 0x1e180);
    buf->set_font(f3, f3, f3, 0);
    buf->set_text_color(0x99, -1, 1, 1);

    *reinterpret_cast<int *>(self + 0x10) += *reinterpret_cast<int *>(self + 0x1c) * 2;
    buf->set_clip(reinterpret_cast<RECT *>(g_007aec94));

    RECT *r4 = reinterpret_cast<RECT *>(self + 0x30);
    buf->set_clip(r4);

    int left2 = r4->left + 6;
    int top2 = r4->top + 6;
    *reinterpret_cast<int *>(self + 0xc) = left2;
    *reinterpret_cast<int *>(self + 0x10) = top2;
    int width2 = (r4->right - r4->left) - 0xc;
    *reinterpret_cast<int *>(self + 0x14) = width2;
    int height2 = (r4->bottom - r4->top) - 0xc;
    *reinterpret_cast<int *>(self + 0x18) = height2;

    r2->left = left2;
    r2->top = top2;
    r2->right = left2 + width2;
    r2->bottom = top2 + height2;
    buf->box_sprite(r2, reinterpret_cast<BoxSpriteParams *>(g_0078d528));
    buf->set_clip(reinterpret_cast<RECT *>(g_007aec94));
}
