// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0047F960
// name           ?draw_custom_rules@NetWin@@QAEXXZ
// size           1230 bytes
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0047f960/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_custom_rules@NetWin@@QAEXXZ  at 0x0047F960  (1230 bytes)
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
class NetWin;
struct RECT;
class Spot;
class Sprite;
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    int add(int, int, int, int, int, int);
    void kill_type(int);
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
    int box_sprite(RECT *, BoxSpriteParams *);
    int set_font(Font *, Font *, Font *, Font *);
    int write_cent_l(char *, RECT *, int);
    int write_l(char *, RECT *, int);
    void set_text_color(int, int, int, int);
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
    int draw(Buffer *, int, int, int);
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
char * __cdecl text_get();
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);
int __cdecl X_text_open(char *, const char *);
int __cdecl find_font(int, int);
void __cdecl text_close();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0047fe30 = (int *)0x0047FE30;
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_006868a4 = (int *)0x006868A4;
static int *const g_006868a8 = (int *)0x006868A8;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_007794d8 = (int *)0x007794D8;
static int *const g_007794e0 = (int *)0x007794E0;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_0078d5a0 = (int *)0x0078D5A0;
static int *const g_00800000 = (int *)0x00800000;
static int *const g_0090e8ec = (int *)0x0090E8EC;
static int *const g_0090e8f0 = (int *)0x0090E8F0;
static int *const g_0090e8f4 = (int *)0x0090E8F4;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class NetWin { public:
    void draw_custom_rules();
};

// The checkbox-icon sprite table is indexed by a register, so the address
// itself does work: a plain `static T *const` (g_007794d8 above) folds the
// literal into the load and loses the indexed addressing mode. A freshly
// named `extern T name[];` keeps it - the name need not match the address,
// since the comparison masks relocations either way (see 0x0045C940 for the
// same lever on a different table).
extern Sprite g_checkbox_sprites[];

typedef char *(__stdcall *CharUpperAFn)(char *);

void NetWin::draw_custom_rules() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    RECT *rect_d04 = reinterpret_cast<RECT *>(self + 0xd04);
    RECT *rect_cf4 = reinterpret_cast<RECT *>(self + 0xcf4);
    RECT *rect_d14 = reinterpret_cast<RECT *>(self + 0xd14);
    RECT *rect_d24 = reinterpret_cast<RECT *>(self + 0xd24);
    int value_d18 = *reinterpret_cast<int *>(self + 0xd18);
    Spot *spot = reinterpret_cast<Spot *>(self + 0xd34);
    char *strbuf = reinterpret_cast<char *>(g_009b86a0);
    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);

    spot->kill_type(1);
    buf->box_sprite(rect_d04, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));
    buf->box_sprite(rect_cf4, reinterpret_cast<BoxSpriteParams *>(g_0078d528));

    strbuf[0] = 0;
    int rulesTextId = *reinterpret_cast<int *>(*g_009b90f8 + 0xf9c);
    strcat(strbuf, reinterpret_cast<char *>(strings->get(rulesTextId)));
    (*reinterpret_cast<CharUpperAFn *>(g_0066931c))(strbuf);

    buf->set_font(reinterpret_cast<Font *>(find_font(0xe, 1)), 0, 0, 0);
    buf->set_text_color(0xe1, -1, 1, 1);

    if (strbuf) {
        buf->write_cent_l(strbuf, rect_cf4, strlen(strbuf));
    }

    buf->box_sprite(rect_d14, reinterpret_cast<BoxSpriteParams *>(g_0078d528));
    buf->box_sprite(rect_d24, reinterpret_cast<BoxSpriteParams *>(g_0078d528));

    int leftPos = rect_d14->left - 0x14;
    buf->set_font(reinterpret_cast<Font *>(find_font(0xc, 1)), 0, 0, 0);

    int topPos = value_d18;
    int bottomBase = value_d18 + 0x12;
    int width = rect_d14->right - rect_d14->left;
    int bottomEdge = width + leftPos - 6;

    unsigned int hostFlag = (~*g_0090e8f4 >> 4) & 1;
    Buffer *guardBuf = self ? buf : 0;
    int idx = (hostFlag != 0) ? 2 : 0;
    Sprite *icon = &g_checkbox_sprites[idx];
    unsigned char transparent = static_cast<unsigned char>(icon->cTransparentIndex_);

    icon->draw(guardBuf, transparent, leftPos, topPos);
    if (*g_0093e8c0 != 0) {
        spot->add(0x64, 1, leftPos, topPos, bottomEdge - leftPos, bottomBase - topPos);
    }

    leftPos += 0x19;
    buf->set_text_color((hostFlag != 0) ? 0xde : 0xe4, -1, 1, 1);

    char titleBuf[256];
    titleBuf[0] = 0;
    int scriptTextId = *reinterpret_cast<int *>(*g_009b90f8 + 0x10a4);
    strcat(titleBuf, reinterpret_cast<char *>(strings->get(scriptTextId)));
    strcat(titleBuf, reinterpret_cast<char *>(g_006868a4));

    if (titleBuf) {
        buf->write_l(titleBuf, reinterpret_cast<RECT *>(&leftPos), strlen(titleBuf));
    }

    topPos += 0x14;
    leftPos -= 0x19;
    bottomBase += 0x14;

    int openResult = X_text_open(*reinterpret_cast<char **>(g_00691b0c),
                                  reinterpret_cast<char *>(g_006868a8));

    if (openResult == 0) {
        char *line = text_get();
        while (*line == '#') {
            line = text_get();
        }

        int i = 0;
        do {
            unsigned int flag;
            switch (i) {
            case 0:  flag = *g_0090e8ec & 0x800; break;
            case 1:  flag = *g_0090e8ec & 2; break;
            case 2:  flag = *g_0090e8ec & 8; break;
            case 3:  flag = *g_0090e8ec & 4; break;
            case 4:  flag = *g_0090e8ec & 0x1000; break;
            case 5:  flag = *g_0090e8ec & 1; break;
            case 6:  flag = *g_0090e8ec & 0x10; break;
            case 7:  flag = *g_0090e8ec & 0x20; break;
            case 8:  flag = *g_0090e8ec & 0x4000; break;
            case 9:  flag = *g_0090e8ec & 0x200; break;
            case 10: flag = *g_0090e8ec & 0x40; break;
            case 11: flag = (~*g_0090e8ec >> 8) & 1; break;
            case 12: flag = *g_0090e8ec & 0x2000; break;
            case 13: flag = *g_0090e8ec & 0x8000; break;
            case 14: flag = *g_0090e8ec & 0x80; break;
            case 16: flag = *g_0090e8f0 & 0x800000; break;
            case 17: flag = *g_0090e8f0 & 0x1000000; break;
            default:
                line = text_get();
                goto next_iter;
            }

            {
                Buffer *rowGuard = self ? buf : 0;
                int rowIdx = (flag != 0) ? 2 : 0;
                Sprite *rowIcon = &g_checkbox_sprites[rowIdx];
                unsigned char rowTransparent = static_cast<unsigned char>(rowIcon->cTransparentIndex_);
                rowIcon->draw(rowGuard, rowTransparent, leftPos, topPos);
            }
            if (*g_0093e8c0 != 0) {
                spot->add(i, 1, leftPos, topPos, bottomEdge - leftPos, bottomBase - topPos);
            }

            leftPos += 0x19;
            if (line != 0) {
                buf->set_text_color((flag != 0) ? 0xde : 0xe4, -1, 1, 1);
                buf->write_l(line, reinterpret_cast<RECT *>(&leftPos), strlen(line));
            }

            if (i == 7) {
                leftPos += 0x171;
                bottomEdge += 0x18a;
                topPos -= 0xa0;
                bottomBase -= 0xa0;
                line = text_get();
            } else {
                topPos += 0x14;
                bottomBase += 0x14;
                leftPos -= 0x19;
                line = text_get();
            }

        next_iter:
            ++i;
        } while (i < 0x12);

        text_close();
    }
}
