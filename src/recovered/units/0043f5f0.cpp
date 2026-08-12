// ORIGINAL: 0x0043F5F0 FILE
// name      ?draw_bioscan@DiploPop@@QAEXXZ
// size      404 bytes
// spans     0x0043F5F0-0x0043F784
// prototype void (__thiscall ?draw_bioscan@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   1   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005DAC70 0x005DACB0 0x005DD130 0x005E2DD3 0x005E4B4A 0x006169A0 0x006453E0 0x00645470
// indirect  0x0043F6B1
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043F5F0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043f5f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_bioscan@DiploPop@@QAEXXZ  at 0x0043F5F0  (404 bytes)
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
    int set_font(Font *, Font *, Font *, Font *);
    int write_cent_l(char *, RECT *, int);
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
    int draw(Buffer *, int, int, int, int, int);
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
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_0078d618 = (int *)0x0078D618;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DiploPop { public:
    uint8_t pad_0_[0x1074];
    uint32_t field_1074_;

    void draw_bioscan();
};

// No catalogued callee for the indirect call through the function-pointer
// slot at 0x66931c (CharUpperA, per Ghidra); __stdcall free-function
// pointers are legal in VC6 (only __thiscall ones need the VCall shim), so
// this is a plain typed call through the fixed address.
typedef char *(__stdcall *CharUpperFn)(char *);

// MISMATCH (#4: original 'push esi' vs rebuilt 'mov' - the original caches
// the literal 3 in esi ONCE and reuses it for all eight rect +/-3
// adjustments (both RECTs), which reserves esi in the prologue before the
// first branch. Naming that 3 as a local (`int adj = 3;`) reused at all
// eight sites, tried here, did not change which register (if any) the
// compiler pins for it - REGISTER ALLOCATION again. The
// `guard ? &g_007aec64_buffer : 0` mask/and idiom at the end (feeding
// Sprite::draw) reproduces the original's `neg esi; sbb esi,esi; and
// esi,...` sequence by testing the fixed-global POINTER value itself
// (`g_007ae820`, never dereferenced) rather than `*g_007ae820` - the guard
// is a compile-time-always-true address check the original does not fold
// away either, matching the EXTERN-SYMBOL LEVER note's third measured
// shape. Otherwise a full structural match: same two box_sprite/RECT
// blocks, same set_font/set_text_color/get/strcat/CharUpperA/strlen/
// write_cent_l chain, same X/Y computation for the final Sprite::draw.
void DiploPop::draw_bioscan() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(g_007aec64);

    int adj = 3;
    RECT rect = *reinterpret_cast<RECT *>(self + 0x20c4);
    buf->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d618));
    rect.right -= adj;
    rect.left += adj;
    rect.top += adj;
    rect.bottom -= adj;

    reinterpret_cast<Buffer *>(self + 0xa6c)->set_font(0, 0, 0, 0);
    buf->set_text_color(0xa0, -1, 1, 1);

    *reinterpret_cast<char *>(g_009b86a0) = 0;
    int strObj = *g_009b90f8;
    int strIdx = *reinterpret_cast<int *>(strObj + 0xe8c);
    int getResult = reinterpret_cast<Strings *>(g_009b90d8)->get(strIdx);
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<const char *>(getResult));
    (*reinterpret_cast<CharUpperFn *>(g_0066931c))(reinterpret_cast<char *>(g_009b86a0));
    if (reinterpret_cast<char *>(g_009b86a0) != 0) {
        buf->write_cent_l(reinterpret_cast<char *>(g_009b86a0), &rect,
                           strlen(reinterpret_cast<char *>(g_009b86a0)));
    }

    RECT rect2 = *reinterpret_cast<RECT *>(self + 0x20d4);
    buf->box_sprite(&rect2, reinterpret_cast<BoxSpriteParams *>(g_0078d528));
    rect2.left += adj;
    rect2.right -= adj;
    rect2.top += adj;
    rect2.bottom -= adj;

    Sprite *spr = *reinterpret_cast<Sprite **>(self + 0x20b8);
    Buffer *target = g_007ae820 ? buf : 0;
    int x = *reinterpret_cast<int *>(self + 0x20dc) - *reinterpret_cast<int *>(reinterpret_cast<char *>(spr) + 0x18) * 2;
    int y = *reinterpret_cast<int *>(self + 0x20d8) +
            (*reinterpret_cast<int *>(self + 0x20e0) - *reinterpret_cast<int *>(reinterpret_cast<char *>(spr) + 0x1c)
             - *reinterpret_cast<int *>(self + 0x20d8)) / 2;
    int flag = *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(spr) + 8);
    spr->draw(target, flag, x, y, 1, 1);
}
