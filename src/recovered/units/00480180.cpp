// ORIGINAL: 0x00480180 ?draw_diffs@NetWin@@QAEXXZ 0x00480180-0x0048036F FILE
// size      495 bytes
// prototype void (__thiscall ?draw_diffs@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   13
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047B380 0x004E25E0 0x005882F0 0x005D8000 0x005DAC70 0x005DACB0 0x005DCF40 0x005E2DD3 0x005E4B9A 0x005FA960 0x005FAA90 0x006169A0 0x006453E0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00480180
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00480180/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_diffs@NetWin@@QAEXXZ  at 0x00480180  (495 bytes)
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

class AlphaNet;
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
class AlphaNet { public:
    uint32_t alignment_;
    uint8_t data_[0x149C];
    int pid_2_idx(unsigned long);
};
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
    int set_clip(RECT *);
    int set_font(Font *, Font *, Font *, Font *);
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
extern "C" unsigned int strlen(const char *);
int __cdecl find_font(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00788038 = (int *)0x00788038;
static int *const g_00788040 = (int *)0x00788040;
static int *const g_00788054 = (int *)0x00788054;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_0090dd14 = (int *)0x0090DD14;
static int *const g_0090e778 = (int *)0x0090E778;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_0096c85c = (int *)0x0096C85C;
static int *const g_009b90d8 = (int *)0x009B90D8;

class NetWin { public:
    int get_player_color(int);
    void draw_diffs();
};

// INDEXED TABLE BASE: the string-index table is accessed via a
// sign-extended byte index multiplied by 4 - arithmetic on the address, so
// it needs `extern T name[]` rather than the scaffold's plain pointer
// constant, which /O2 would otherwise fold into a memory read of the
// stored value instead of the address.
extern int32_t g_arr_96c85c[];

void NetWin::draw_diffs() {
    char *self = reinterpret_cast<char *>(this);

    reinterpret_cast<Spot *>(self + 0xd34)->kill_type(4);

    int i = 0;
    int8_t *rec = reinterpret_cast<int8_t *>(g_0090dd14);
    int32_t *rect = reinterpret_cast<int32_t *>(self + 0xb54);

    do {
        if (*g_0090e778 == 3 &&
            i >= *reinterpret_cast<uint8_t *>(*reinterpret_cast<int32_t *>(self + 0x772c))) {
            return;
        }

        Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
        buf->box_sprite(reinterpret_cast<RECT *>(rect),
                         reinterpret_cast<BoxSpriteParams *>(g_0078d528));

        if (*g_0090e778 != 3 || rec[0] != -1) {
            RECT r = *reinterpret_cast<RECT *>(rect);

            int idx = reinterpret_cast<AlphaNet *>(g_0093cd90)->pid_2_idx(*g_0093d4f0);
            if (i + 1 == idx || (*g_0093d4f0 == *g_0093d4f4 && rec[0] == -1)) {
                reinterpret_cast<Sprite *>(g_00788038)->draw(
                    this != 0 ? buf : 0,
                    *g_00788040 & 0xff,
                    r.right - 10,
                    (r.bottom - *g_00788054 - r.top) / 2 + r.top);
            }

            buf->set_font(reinterpret_cast<Font *>(find_font(0xe, 1)),
                          0, 0, 0);
            buf->set_text_color(get_player_color(i + 1), 0, 1, 1);

            RECT clip;
            clip.left = rect[0] + 4;
            clip.top = rect[1];
            clip.right = rect[2] - 4;
            clip.bottom = rect[3];
            buf->set_clip(&clip);

            int str = reinterpret_cast<Strings *>(g_009b90d8)->get(g_arr_96c85c[rec[2]]);
            if (str != 0) {
                buf->write_l(reinterpret_cast<char *>(str), &clip,
                             strlen(reinterpret_cast<const char *>(str)));
            }

            buf->set_clip(reinterpret_cast<RECT *>(reinterpret_cast<char *>(buf) + 0x30));

            reinterpret_cast<Spot *>(self + 0xd34)->add(
                i, 4, rect[0], rect[1], rect[2] - rect[0], rect[3] - rect[1]);
        }

        rect += 4;
        ++i;
        rec += 0x17c;
    } while (rec < reinterpret_cast<int8_t *>(g_0090e778));
}
