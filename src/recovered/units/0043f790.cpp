// ORIGINAL: 0x0043F790 ?draw_portrait@DiploPop@@QAEXXZ 0x0043F790-0x0043FA8E FILE
// size      766 bytes
// prototype void (__thiscall ?draw_portrait@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   1   call targets   11
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0053A090 0x0058F610 0x005DAC70 0x005DACB0 0x005DCEA0 0x005E1A80 0x005E2210 0x005E2DD3 0x006169A0 0x006453E0 0x00645470
// indirect  0x0043F8A0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043F790
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043f790/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_portrait@DiploPop@@QAEXXZ  at 0x0043F790  (766 bytes)
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
class DiploPop;
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
    int box_sprite(RECT *, BoxSpriteParams *);
    int get_pixel(int, int);
    int set_font(Font *, Font *, Font *, Font *);
    int write_l(char *, int, int, int);
    void hline(int, int, int, int);
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
int __cdecl get_mood(int);
void __cdecl say_special(char *, char *, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946a30 = (int *)0x00946A30;
static int *const g_00946a84 = (int *)0x00946A84;
static int *const g_0094c9e4 = (int *)0x0094C9E4;
static int *const g_0096ca38 = (int *)0x0096CA38;
static int *const g_0096cb04 = (int *)0x0096CB04;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DiploPop { public:
    uint8_t pad_0_[0x1074];
    uint32_t field_1074_;

    void draw_portrait();
};

// Indexed table bases: the scaffold spells these as fixed-address pointer
// constants, which is right for a plain load/store but wrong here because
// each is indexed by a register (faction id). `extern T name[]` keeps the
// indexed-addressing instruction form instead of letting /O2 fold the
// constant base into the arithmetic.
extern int g_0096cb04_tbl[];
extern int g_00946a30_tbl[];
extern int g_0096ca38_tbl[];
extern int g_0094c9e4_tbl[];
extern int g_00939284_val;

void DiploPop::draw_portrait() {
    char *self = reinterpret_cast<char *>(this);

    RECT rect = *reinterpret_cast<RECT *>(self + 0x20e4);
    reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(
        &rect, reinterpret_cast<BoxSpriteParams *>(g_0078d528));

    int left1   = rect.left + 1;
    int right1  = rect.right - 1;
    int top1    = rect.top + 1;
    int bottom1 = rect.bottom - 1;
    rect.left   = left1;
    rect.right  = right1;
    rect.top    = top1;
    rect.bottom = bottom1;
    int extra = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 0x20b4) + 0x1c);
    top1 += extra;
    int orig_left = left1 - 1;
    int pixel = reinterpret_cast<Buffer *>(g_007aec64)->get_pixel(orig_left, top1);
    rect.top = top1;
    reinterpret_cast<Buffer *>(g_007aec64)->hline(rect.left, rect.right, rect.top, pixel);

    reinterpret_cast<Buffer *>(g_007aec64)->set_font(
        reinterpret_cast<Font *>(self + 0xa6c), 0, 0, 0);
    reinterpret_cast<Buffer *>(g_007aec64)->set_text_color(0xa0, -1, 1, 1);

    rect.left += 4;
    rect.right -= 4;
    rect.top += 4;
    rect.bottom -= 4;

    char *buf = reinterpret_cast<char *>(g_009b86a0);
    *buf = 0;
    strcat(buf, reinterpret_cast<char *>(reinterpret_cast<char *>(g_00946a84) +
                                          *reinterpret_cast<int *>(self + 0xa94) * 0x59c));
    (*reinterpret_cast<char *(__stdcall **)(char *)>(0x0066931C))(buf);

    if (buf) {
        int save_left = rect.left;
        unsigned int len = strlen(buf);
        reinterpret_cast<Buffer *>(g_007aec64)->write_l(buf, save_left, rect.top, static_cast<int>(len));
    }

    int a6c = *reinterpret_cast<int *>(self + 0xa6c);
    int y_adv;
    if (a6c < 0)
        y_adv = *reinterpret_cast<int *>(self + 0xa78);
    else
        y_adv = *reinterpret_cast<int *>(self + 0xa7c) + a6c;
    rect.top += y_adv;

    reinterpret_cast<Buffer *>(g_007aec64)->set_font(
        reinterpret_cast<Font *>(self + 0xa44), 0, 0, 0);

    *buf = 0;
    int leader_offset = *reinterpret_cast<int *>(*reinterpret_cast<int *>(g_009b90f8) + 0xe90);
    char *s1 = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(leader_offset));
    strcat(buf, s1);
    strcat(buf, reinterpret_cast<char *>(g_00682e94));

    int faction = *reinterpret_cast<int *>(self + 0xa94);
    int mood_idx = g_0096cb04_tbl[faction * 2099];
    if (mood_idx < 0)
        mood_idx = 0;
    else if (mood_idx > 7)
        mood_idx = 7;
    char *s2 = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(g_00946a30_tbl[mood_idx]));
    say_special(buf, s2, faction);

    if (buf) {
        int save_left = rect.left;
        unsigned int len = strlen(buf);
        reinterpret_cast<Buffer *>(g_007aec64)->write_l(buf, save_left, rect.top, static_cast<int>(len));
    }

    int a44 = *reinterpret_cast<int *>(self + 0xa44);
    int y_adv2;
    if (a44 < 0)
        y_adv2 = *reinterpret_cast<int *>(self + 0xa50);
    else
        y_adv2 = *reinterpret_cast<int *>(self + 0xa54) + a44;
    rect.top += y_adv2;

    *buf = 0;
    int spouse_offset = *reinterpret_cast<int *>(*reinterpret_cast<int *>(g_009b90f8) + 0xe94);
    char *s3 = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(spouse_offset));
    strcat(buf, s3);
    strcat(buf, reinterpret_cast<char *>(g_00682e94));

    faction = *reinterpret_cast<int *>(self + 0xa94);
    int mood_val = g_0096ca38_tbl[g_00939284_val + faction * 2099];
    int mood = get_mood(mood_val);
    char *s4 = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(g_0094c9e4_tbl[mood]));
    say_special(buf, s4, faction);

    if (buf) {
        int save_left = rect.left;
        unsigned int len = strlen(buf);
        reinterpret_cast<Buffer *>(g_007aec64)->write_l(buf, save_left, rect.top, static_cast<int>(len));
    }
}
