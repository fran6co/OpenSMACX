// ORIGINAL: 0x00440080 FILE
// name      ?draw_hq@DiploPop@@QAEXXZ
// size      529 bytes
// spans     0x00440080-0x00440291
// prototype void (__thiscall ?draw_hq@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   3   call targets   12
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0045C290 0x0050BA00 0x005D5930 0x005D8000 0x005DAC70 0x005DACB0 0x005DD130 0x005E2DD3 0x005E4B4A 0x006169A0 0x006453E0 0x00645470
// indirect  0x004401E0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00440080
// measured tier  MISMATCH
// divergence     38
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00440080/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_hq@DiploPop@@QAEXXZ  at 0x00440080  (529 bytes)
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
struct BoxSpriteParams;
class Buffer;
class DiploPop;
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
class MainInterface;
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
    int write_cent_l(char *, RECT *, int);
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
};
class MainInterface { public:
    void restore_back(int);
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
void __cdecl bitmask(int, int *, int *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007aec94 = (int *)0x007AEC94;
static int *const g_0097d053 = (int *)0x0097D053;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DiploPop { public:
    uint8_t pad_0_[0x1074];
    uint32_t field_1074_;

    void draw_hq();
};

// Per-a1 record table, stride 0x134 (same base established at 0x00564b90's
// g_0097d040): a byte "chassis id" field at +4 and a parallel mask/flags
// byte table at +0x8c (0097d0cc = 0097d040 + 0x8c). Walked by a register
// that genuinely increments by 0x134 across loop iterations, so - matching
// 00564b90's proven `g_0096fc28` - a materialized const pointer is correct
// here, not the indexed-table-base lever.
static uint8_t *const g_0097d044 = (uint8_t *)0x0097D044;
static uint8_t *const g_0097d0cc = (uint8_t *)0x0097D0CC;

extern "C" __declspec(dllimport) char *__stdcall CharUpperA(char *);

void DiploPop::draw_hq() {
    char *self = reinterpret_cast<char *>(this);

    RECT rect = *reinterpret_cast<RECT *>(self + 0x2114);

    if (*reinterpret_cast<int32_t *>(self + 0x4208) == 0) {
        reinterpret_cast<Buffer *>(g_007aec64)->set_clip(&rect);
        reinterpret_cast<MainInterface *>(g_007ae820)->restore_back(0);
        reinterpret_cast<Buffer *>(g_007aec64)->set_clip(
            reinterpret_cast<RECT *>(g_007aec94));
        reinterpret_cast<GraphicWin *>(g_007ae820)->soft_update(
            reinterpret_cast<RECT *>(self + 0x2114));
        return;
    }

    rect.bottom -= 4;
    reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(
        &rect, reinterpret_cast<BoxSpriteParams *>(g_0078d528));

    int left = rect.left + 1;
    int top = rect.top + 1;
    int right = rect.right - 1;
    rect.bottom -= 1;
    int textTop = top + 0x15;

    rect.left = left;
    rect.top = top;
    rect.right = right;

    int count = 0;
    *reinterpret_cast<uint8_t *>(g_009b86a0) = 0;

    if (*g_009a64cc > 0) {
        int off = 0;
        do {
            if (g_0097d044[off] ==
                static_cast<unsigned>(*reinterpret_cast<int32_t *>(self + 0xa94))) {
                int m_off;
                int m_mask;
                bitmask(1, &m_off, &m_mask);
                if ((g_0097d0cc[off + m_off] &
                     reinterpret_cast<uint8_t *>(&m_mask)[0]) != 0) {
                    break;
                }
            }
            count++;
            off += 0x134;
        } while (count < *g_009a64cc);
    }

    const char *name;
    if (count == *g_009a64cc) {
        int idx = *reinterpret_cast<int32_t *>(
            *reinterpret_cast<char **>(g_009b90f8) + 0xea4);
        name = reinterpret_cast<const char *>(
            reinterpret_cast<Strings *>(g_009b90d8)->get(idx));
    } else {
        name = reinterpret_cast<const char *>(g_0097d053) + count * 0x134;
    }
    strcat(reinterpret_cast<char *>(g_009b86a0), name);
    CharUpperA(reinterpret_cast<char *>(g_009b86a0));

    reinterpret_cast<Buffer *>(g_007aec64)->set_font(
        reinterpret_cast<Font *>(self + 0xa6c), 0, 0, 0);
    reinterpret_cast<Buffer *>(g_007aec64)->set_text_color(0xa0, -1, 1, 1);

    if (*reinterpret_cast<uint8_t *>(g_009b86a0) != 0) {
        unsigned len = strlen(reinterpret_cast<char *>(g_009b86a0));
        reinterpret_cast<Buffer *>(g_007aec64)->write_cent_l(
            reinterpret_cast<char *>(g_009b86a0), &rect, len);
    }

    rect.top = textTop + 1;

    Sprite *spr = *reinterpret_cast<Sprite **>(self + 0x20c0);
    int y = (rect.right - spr->iWidth_ - rect.left) / 2 + rect.left;
    uint8_t transIdx = static_cast<uint8_t>(spr->cTransparentIndex_);
    spr->draw(reinterpret_cast<Buffer *>(g_007aec64), transIdx, y, rect.top, 1, 1);

    reinterpret_cast<GraphicWin *>(g_007ae820)->soft_update(
        reinterpret_cast<RECT *>(self + 0x2114));
}
