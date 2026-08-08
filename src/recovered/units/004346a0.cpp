// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004346A0
// name           ?draw_info@DesignWin@@QAEXXZ
// size           1318 bytes
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004346a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_info@DesignWin@@QAEXXZ  at 0x004346A0  (1318 bytes)
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
class DesignWin;
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
    int wrap_cent(char *, RECT *);
    int wrap_height(char *, int);
    int write_cent_l(char *, RECT *, int);
    void set_text_color(int, int, int, int);
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
    Font();
    int init(char *, int, int);
    ~Font();
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00654006 = (int *)0x00654006;
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00674668 = (int *)0x00674668;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e98 = (int *)0x00682E98;
static int *const g_00682e9c = (int *)0x00682E9C;
static int *const g_0068354c = (int *)0x0068354C;
static int *const g_00691b2c = (int *)0x00691B2C;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_0078d5a0 = (int *)0x0078D5A0;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007aec94 = (int *)0x007AEC94;
static int *const g_009ab893 = (int *)0x009AB893;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DesignWin { public:
    uint8_t pad_0_[0x12764];
    Spot spot_;

    void draw_info();
};

// Indexed-table bases (base+register addressing) the emitter's immediate
// scan does not catch: `extern T name[];`, not a pointer constant, per the
// measured indexed-table-base lever.
extern char g_9ab888x[]; // faction colour record, stride 0x34: [0]=int id, [4..7]=rgba bytes
extern char g_9ab893x[]; // per-civ colour-slot table, stride 0x34: [0]=colour id,[1]=colour mask,[5]=flags
extern char g_96d238x[]; // 64 x 0x20cc byte compatibility matrix
extern char g_96d438x[]; // 64 x 0x20cc byte compatibility matrix

typedef char *(__stdcall *CharUpperFn)(char *);

// _itoa's scaffold declaration is nullary (a fallback); the real 3-argument
// call is declared here under a different name. The relocation it needs is
// masked by the comparison regardless of the symbol it names.
extern "C" char *__cdecl my_itoa(int, char *, int);

// PROLOGUE-ORDER WALL: the original establishes its SEH frame with
// push ebp; mov ebp,esp; push -1; push handler; mov eax,fs:0; push eax;
// mov fs:0,esp; sub esp,N. A real local `Font` (non-trivial dtor) makes
// this translation unit emit the same SEH scaffold, but with `push -1`
// ahead of `mov ebp,esp` instead of after it - a toolchain-level prologue
// ordering choice, not reachable from source form (compare the
// EH-funclet walls documented on 0x0047C530-family functions). The rest
// of the body reproduces the call sequence, offsets, and every branch
// (civ-colour match / palette-reassignment search / fallback string) as
// given.
void DesignWin::draw_info() {
    char *self = reinterpret_cast<char *>(this);
    Font font;
    Buffer *buf = reinterpret_cast<Buffer *>(g_007aec64);

    RECT *srcRect = reinterpret_cast<RECT *>(self + 0xa4c);
    RECT clip = *srcRect;
    buf->set_clip(&clip);

    font.init(*reinterpret_cast<char **>(g_00691b2c), 0x12, 1);

    buf->box_sprite(&clip, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    clip.left += 3;
    clip.top += 3;
    clip.right -= 3;
    clip.bottom -= 3;
    buf->box_sprite(&clip, reinterpret_cast<BoxSpriteParams *>(g_0078d528));

    clip.left += 3;
    clip.top += 3;
    clip.right -= 3;
    clip.bottom -= 3;
    buf->set_text_color(0xe0, -1, 1, 1);

    buf->set_font(&font, 0, 0, 0);

    RECT textRect;
    textRect.left = clip.left;
    textRect.top = clip.top;
    textRect.right = clip.right;
    int h;
    if (font.unk_1_ >= 0) {
        h = font.unk_1_ + font.height_;
    }
    textRect.bottom = clip.top + h;

    char *msg = reinterpret_cast<char *>(g_009b86a0);
    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
    char *tbl = *reinterpret_cast<char **>(g_009b90f8);

    msg[0] = 0;
    strcat(msg, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(tbl + 0xe4c))));
    if (msg) {
        unsigned int len = strlen(msg);
        buf->write_cent_l(msg, &textRect, len);
    }

    msg[0] = 0;
    int civ = *reinterpret_cast<int *>(self + 0x141f8);
    int strId;
    bool useFallback = false;

    if (civ < 0x40) {
        char *rec = g_9ab888x + civ * 0x34;
        if (*reinterpret_cast<unsigned int *>(self + 0x141fc) == static_cast<unsigned char>(rec[4]) &&
            *reinterpret_cast<unsigned int *>(self + 0x14200) == static_cast<unsigned char>(rec[5]) &&
            *reinterpret_cast<unsigned int *>(self + 0x14204) == static_cast<unsigned char>(rec[6]) &&
            *reinterpret_cast<unsigned int *>(self + 0x1420c) == static_cast<unsigned char>(rec[7]) &&
            *reinterpret_cast<int *>(self + 0x14208) == *reinterpret_cast<int *>(rec)) {
            strId = *reinterpret_cast<int *>(tbl + 0xe44);
            goto label_a3a;
        } else {
            goto label_a2e;
        }
    } else if ((g_9ab893x[civ * 0x34 + 5] & 1) != 0) {
        int bit = *reinterpret_cast<int *>(self + 0x141f4);
        unsigned char mask = static_cast<unsigned char>(1 << (bit & 0x1f));
        if ((mask & static_cast<unsigned char>(g_9ab893x[civ * 0x34 + 1])) == 0) {
            char *rec = g_9ab888x + civ * 0x34;
            if (*reinterpret_cast<unsigned int *>(self + 0x141fc) == static_cast<unsigned char>(rec[4]) &&
                *reinterpret_cast<unsigned int *>(self + 0x14200) == static_cast<unsigned char>(rec[5]) &&
                *reinterpret_cast<unsigned int *>(self + 0x14204) == static_cast<unsigned char>(rec[6]) &&
                *reinterpret_cast<unsigned int *>(self + 0x1420c) == static_cast<unsigned char>(rec[7]) &&
                *reinterpret_cast<int *>(self + 0x14208) == *reinterpret_cast<int *>(rec)) {
                strId = *reinterpret_cast<int *>(tbl + 0xe50);
                goto label_a3a;
            } else {
                bool found = false;
                int cur = bit << 6;
                char *p = g_9ab893x + bit * 0xd00;
                for (int k = 0; k < 0x40; ++k, p += 0x34, ++cur) {
                    if ((p[5] & 1) == 0) {
                        *reinterpret_cast<int *>(self + 0x141f8) = cur;
                        goto label_a2e;
                    }
                    if ((p[1] & mask) != 0) {
                        char curColor = g_9ab893x[*reinterpret_cast<int *>(self + 0x141f8) * 0x34];
                        if (p[0] == curColor || g_96d238x[bit * 0x20cc + cur] == 0) {
                            int idx = cur + bit * 0x20cc;
                            if (g_96d238x[idx] == 0 &&
                                (g_96d438x[idx] == 0 || (!found && p[0] == curColor))) {
                                *reinterpret_cast<int *>(self + 0x141f8) = cur;
                                found = true;
                            }
                        }
                    }
                }
                if (found) {
                    goto label_a2e;
                }
                strId = *reinterpret_cast<int *>(tbl + 0xc38);
                goto label_a3a;
            }
        } else {
            useFallback = true;
        }
    } else {
        useFallback = true;
    }

    if (useFallback) {
        strcat(msg, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(tbl + 0xc34))));
        strcat(msg, reinterpret_cast<char *>(g_00682820));
        strcat(msg, reinterpret_cast<char *>(g_00682e9c));
        strcat(msg, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(tbl + 0xe48))));
        strcat(msg, reinterpret_cast<char *>(g_0068354c));

        unsigned int v = *reinterpret_cast<unsigned int *>(self + 0x141f8) & 0x8000003f;
        if (static_cast<int>(v) < 0) {
            v = (v - 1 | 0xffffffc0) + 1;
        }
        char numbuf[80];
        my_itoa(v + 1, numbuf, 10);
        strcat(msg, numbuf);
        strcat(msg, reinterpret_cast<char *>(g_00682e98));
        (*reinterpret_cast<CharUpperFn *>(g_0066931c))(msg);
        goto label_b32;
    }

label_a2e:
    strId = *reinterpret_cast<int *>(tbl + 0xe54);
label_a3a:
    strcat(msg, reinterpret_cast<char *>(strings->get(strId)));
label_b32:
    int wrapH = buf->wrap_height(msg, textRect.right - textRect.left);
    int vshift = (clip.bottom - clip.top) - wrapH;
    textRect.top += vshift;
    textRect.bottom += vshift;
    buf->wrap_cent(msg, &textRect);

    reinterpret_cast<GraphicWin *>(g_007ae820)->soft_update(srcRect);
    buf->set_clip(reinterpret_cast<RECT *>(g_007aec94));
}
