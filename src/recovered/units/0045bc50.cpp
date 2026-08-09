// ORIGINAL: 0x0045BC50 FILE
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0045BC50
// name           ?on_redraw@MainInterface@@QAEXXZ
// size           557 bytes
// measured tier  NO_COMPILE
// refusal        u0045bc50.cpp(209) : error C2079: 'buffer_' uses undefined class 'Buffer'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045bc50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@MainInterface@@QAEXXZ  at 0x0045BC50  (557 bytes)
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
class Console;
class Font;
class GraphicWin;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
class InfoWin;
typedef char * LPSTR;
typedef void * LPVOID;
class MainInterface;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Time;
typedef unsigned int UINT_PTR;
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
class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;
    void update_data(int);
};
class InfoWin { public:
    void reset();
};
class Time { public:
    int unk_tgl_;
    UINT_PTR id_event_;
    void * callback1_;
    void * callback2_;
    int cb_param2_;
    int cb_param1_;
    uint32_t count_;
    int unk_1_;
    uint32_t resolution_;
    int unk_2_;
    void start();
    void stop();
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
    void hide();
    void show(int);
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1, 2
class VCall { public:
    virtual void slot000();
    virtual void slot001();  // <-- used
    virtual void slot002();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0078d5a0 = (int *)0x0078D5A0;
static int *const g_0078d7f8 = (int *)0x0078D7F8;
static int *const g_007ad2a0 = (int *)0x007AD2A0;
static int *const g_007ad2b0 = (int *)0x007AD2B0;
static int *const g_007ad2c0 = (int *)0x007AD2C0;
static int *const g_007ad2d0 = (int *)0x007AD2D0;
static int *const g_007ae778 = (int *)0x007AE778;
static int *const g_007d392c = (int *)0x007D392C;
static int *const g_007f685c = (int *)0x007F685C;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_009b7b1c = (int *)0x009B7B1C;

class MainInterface { public:
    void draw_edit_pane();
    void restore_back(int);
    void redraw_complete();
    void on_redraw();
};

// g_007ae778 is indexed by a register (`[eax*4 + addr]`), so it needs
// `extern int name[]`, not the plain const-pointer spelling above - the
// emitter does not detect base-immediate/offset-register on its own.
extern int g_007ae778_tbl[];

// The two `hide()`/`show()` call sites read a pointer at a fixed address,
// dereference +4, then ADD THE ADDRESS ITSELF (not its value) to the
// result - the address does arithmetic, so it needs `extern`, not the
// plain const-pointer spelling above (which /O2 would fold to a literal).
extern int g_007f685c_val;

// Slot 1 is called with an int argument at one call site in this body,
// but the emitter's shared VCall shim declares it nullary. A second shim
// with the real signature stands in for that call only.
class VCall1 { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used
};

void MainInterface::on_redraw() {
    if (g_007ae778_tbl[*g_007d392c] == 0) {
        char *self = reinterpret_cast<char *>(this);

        restore_back(1);

        RECT rect;
        rect.left   = *reinterpret_cast<long *>(self + 0xda0);
        rect.top    = *reinterpret_cast<long *>(self + 0xda4);
        rect.right  = *reinterpret_cast<long *>(self + 0xda8);
        rect.bottom = *reinterpret_cast<long *>(self + 0xdac);

        int state = *reinterpret_cast<int *>(self + 0x1e8fc);
        switch (state) {
        case 1000: {
            rect.left += 4;
            rect.top += 4;
            rect.right -= 4;
            rect.bottom -= 4;
            reinterpret_cast<Buffer *>(self + 0x444)->box_sprite(
                &rect, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

            int a = g_007f685c_val;
            int b = *reinterpret_cast<int *>(a + 4);
            b += reinterpret_cast<int>(&g_007f685c_val);
            reinterpret_cast<Win *>(b)->show(0);

            char *p = self + 0x8a44;
            for (int i = 0x1f; i != 0; i--) {
                reinterpret_cast<VCall *>(p)->slot002();
                p += 0xb4c;
            }
            break;
        }
        case 1001: {
            rect.bottom -= 4;
            rect.left += 4;
            rect.right -= 4;
            rect.top += 4;

            Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
            buf->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));
            buf->box_sprite(reinterpret_cast<RECT *>(g_007ad2b0),
                             reinterpret_cast<BoxSpriteParams *>(g_0078d7f8));
            buf->box_sprite(reinterpret_cast<RECT *>(g_007ad2c0),
                             reinterpret_cast<BoxSpriteParams *>(g_0078d7f8));

            reinterpret_cast<InfoWin *>(g_007ad2a0)->reset();

            int a = g_007f685c_val;
            int b = *reinterpret_cast<int *>(a + 4);
            b += reinterpret_cast<int>(&g_007f685c_val);
            reinterpret_cast<Win *>(b)->hide();

            reinterpret_cast<Time *>(g_007ad2d0)->start();

            char *p = self + 0x8a44;
            for (int i = 0x1f; i != 0; i--) {
                reinterpret_cast<VCall *>(p)->slot002();
                p += 0xb4c;
            }
            break;
        }
        case 1002: {
            reinterpret_cast<Buffer *>(self + 0x444)->box_sprite(
                reinterpret_cast<RECT *>(self + 0xd4c),
                reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));
            if (*g_009b7b1c != 0x320) {
                reinterpret_cast<Buffer *>(self + 0x444)->box_sprite(
                    reinterpret_cast<RECT *>(self + 0xd5c),
                    reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));
            }

            int a = g_007f685c_val;
            int b = *reinterpret_cast<int *>(a + 4);
            b += reinterpret_cast<int>(&g_007f685c_val);
            reinterpret_cast<Win *>(b)->hide();

            reinterpret_cast<Time *>(g_007ad2d0)->stop();

            char *p = self + 0x8a44;
            int i = 0xb;
            do {
                if (*g_009b7b1c == 0x320 && i == 0x24) break;
                reinterpret_cast<VCall1 *>(p)->slot001(0);
                i++;
                p += 0xb4c;
            } while (i < 0x2a);

            draw_edit_pane();
            break;
        }
        }
    }

    reinterpret_cast<Console *>(g_009156b0)->update_data(1);
    redraw_complete();
}
