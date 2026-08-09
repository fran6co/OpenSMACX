// ORIGINAL: 0x004589C0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004589C0
// name           ?right_menu@InfoWin@@QAEXHH@Z
// size           911 bytes
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004589c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?right_menu@InfoWin@@QAEXHH@Z  at 0x004589C0  (911 bytes)
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
class Font;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
class InfoWin;
typedef char * LPSTR;
typedef void * LPVOID;
class Menu;
typedef void * PVOID;
class PullDown;
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
};
class PullDown { public:
    PullDown();
    int add_item(int, char *);
    int add_separator(int);
    int get_selected();
    int show(int, int, int (__cdecl *)());
    void init(Menu *);
    ~PullDown();
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
    void client_to_screen(int *, int *);
};
void __cdecl help_any();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00539920 = (int *)0x00539920;
static int *const g_0065528b = (int *)0x0065528B;
static int *const g_006754e8 = (int *)0x006754E8;
static int *const g_0078d7f8 = (int *)0x0078D7F8;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class InfoWin { public:
    void setup_text();
    void draw_text(int);
    int state_valid(int);
    void right_menu(int, int);
};

void InfoWin::right_menu(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    PullDown pd;
    reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&a1, &a2);
    pd.init(0);
    if (state_valid(0)) {
        pd.add_item(0, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x92c))));
    }
    if (state_valid(1)) {
        pd.add_item(1, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x930))));
    }
    if (state_valid(2)) {
        pd.add_item(2, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x934))));
    }
    if (state_valid(3)) {
        pd.add_item(3, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x938))));
    }
    if (state_valid(4)) {
        pd.add_item(4, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x948))));
    }
    if (state_valid(5)) {
        pd.add_item(5, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x94c))));
    }
    if (state_valid(6)) {
        pd.add_item(6, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x950))));
    }
    if (state_valid(7)) {
        pd.add_item(7, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x954))));
    }
    pd.add_separator(0x63);
    pd.add_item(0x14, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
        *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x8fc))));
    pd.show(a1, a2, reinterpret_cast<int (__cdecl *)()>(g_00539920));

    if (!(*g_009a64c0 & 0x800)) {
        int sel = pd.get_selected();
        if (sel < 0x40) {
            // PullDown's real layout (item id at object offset 0xA20 + sel*20,
            // per src/pulldown.h's PullDownItem::id_ and items_[64] array) is
            // known, but the class is deliberately opaque here (methods only,
            // no agreed size) so this local `PullDown pd;` cannot get the
            // original's 0xF40-byte frame slot - see the WALL note filed for
            // this address.
            int id = *reinterpret_cast<int *>(reinterpret_cast<char *>(&pd) + 0xA20 + sel * 20);
            if (id >= 0 && id <= 0x10 && state_valid(id)) {
                int idx = *reinterpret_cast<int *>(self + 0x80);
                *reinterpret_cast<int *>(self + 0x88 + idx * 4) = id;
                while (!state_valid(*reinterpret_cast<int *>(self + 0x88 + idx * 4))) {
                    unsigned int v = *reinterpret_cast<unsigned int *>(self + 0x88 + idx * 4) + 1;
                    v &= 0x80000007u;
                    if (static_cast<int>(v) < 0) {
                        v = ((v - 1) | 0xfffffff8u) + 1;
                    }
                    *reinterpret_cast<unsigned int *>(self + 0x88 + idx * 4) = v;
                }
                *reinterpret_cast<int *>(self + 0x94) = 0;
                *reinterpret_cast<int *>(self + 0x9c8) = *reinterpret_cast<int *>(self + 0x9c0) + 1;
                *reinterpret_cast<int *>(self + 0x9cc) = *reinterpret_cast<int *>(self + 0x9c4) + 1;
                *reinterpret_cast<int *>(self + 0xa8) = 0;
                reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(
                    reinterpret_cast<RECT *>(self + (idx + 1) * 0x10),
                    reinterpret_cast<BoxSpriteParams *>(g_0078d7f8));
                if (reinterpret_cast<void *>(self + 0x88) != 0) {
                    *reinterpret_cast<int *>(self + 0x9c8) = 0;
                    *reinterpret_cast<int *>(self + 0x9cc) = 0;
                }
                setup_text();
                draw_text(*reinterpret_cast<int *>(self + 0x80));
                return;
            }
            if (id == 0x14) {
                help_any();
            }
        }
    }
}
