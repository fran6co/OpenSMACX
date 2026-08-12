// ORIGINAL: 0x00403830 FILE
// name      ?hide@AlphaMenu@@QAEXXZ
// size      693 bytes
// spans     0x00403830-0x00403AE5
// prototype void (__thiscall ?hide@AlphaMenu@@QAEXXZ)(AlphaMenu* this)
// callers   8   call targets   12
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6920 0x005D7470 0x005D7670 0x005D8000 0x005D84C0 0x005DFF00 0x005ECF20 0x005ECFE0 0x005EDCD0 0x005EFD20 0x005F7E90 0x005FD2B0
// indirect  0x00403872 0x004039A2 0x00403A0B 0x00403A25 0x00403AD8
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00403830
// measured tier  MISMATCH
// divergence     13
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00403830/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?hide@AlphaMenu@@QAEXXZ  at 0x00403830  (693 bytes)
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

class AlphaMenu;
class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
class Buffer;
struct ExtDirectDraw;
class Font;
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
class Wave;
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
    int copy(Buffer *, int, int, int, int, int, int);
    int draw(Buffer *, int, int, int, int, int);
    int init(int, int, int, ExtDirectDraw *);
    int set_clip(RECT *);
    void close();
};
class Wave { public:
    uint32_t vtable_storage_;
    uint32_t volume_;
    uint32_t field_8_;
    uint8_t memset_region_[0x24];
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    void * device_;
    uint32_t field_40_;
    Wave * chain_prev_;
    Wave * chain_next_;
    void * fname_;
    uint32_t field_50_;
    uint8_t flags_54_;
    uint8_t pad_55_[3];
    int32_t pitch_;
    float reverb_mix_;
    int32_t ms_length_;
    uint32_t start_time_;
    uint32_t group_slot_;
    int play();
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
    int is_visible();
    void __cdecl flip(RECT *);
    void client_to_screen(RECT *);
    void hide();
    void screen_to_client(RECT *);
};
int __cdecl do_sound();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 12
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_00691af8 = (int *)0x00691AF8;
static int *const g_00696e3c = (int *)0x00696E3C;
static int *const g_006a7090 = (int *)0x006A7090;
static int *const g_007b1804 = (int *)0x007B1804;
static int *const g_007b22a0 = (int *)0x007B22A0;
static int *const g_007cff08 = (int *)0x007CFF08;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00915af4 = (int *)0x00915AF4;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009b7490 = (int *)0x009B7490;
static int *const g_009b74c0 = (int *)0x009B74C0;
static int *const g_009b7ad8 = (int *)0x009B7AD8;
static int *const g_009b7b1c = (int *)0x009B7B1C;

class AlphaMenu { public:
    void hide();
};

// ADDRESS ARITHMETIC: 0x9156b0 and 0x915af4 are each used to resolve an
// "active window" pointer via a double indirection plus a self-relative
// adjustment (vtbl = *addr; adjusted = addr + *(int*)(vtbl+4)), so the
// address itself does work and needs real linkage, not a folded literal.
extern int g_009156b0_sym;
extern int g_00915af4_sym;

// Vtable shim for the offset-0xF8 call on the fixed-address object at
// 0x7b1804 (slot 62, well past the primary VCall's 13 slots) - a
// separate shim rather than growing the shared one, per its own note.
class VCall63 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
};

void AlphaMenu::hide() {
    char *self = reinterpret_cast<char *>(this);

    if (reinterpret_cast<Win *>(this)->is_visible() != 0) {
        if ((*g_009a6490 & 0x400) != 0) {
            reinterpret_cast<Wave *>(g_006a7090)->play();
        }
        *g_00696e3c = -1;
        *g_007cff08 = 0;
        reinterpret_cast<VCall *>(this)->slot012();

        RECT r1 = *reinterpret_cast<RECT *>(self + 0x474);
        reinterpret_cast<Win *>(this)->client_to_screen(&r1);

        RECT r2 = r1;
        char *consoleBase = reinterpret_cast<char *>(g_009156b0_sym);
        int consoleAdjust = *reinterpret_cast<int *>(consoleBase + 4);
        Win *consoleWin = reinterpret_cast<Win *>(
            reinterpret_cast<char *>(&g_009156b0_sym) + consoleAdjust);
        consoleWin->screen_to_client(&r2);

        int w = *reinterpret_cast<int *>(self + 0x4c4);
        int h = *reinterpret_cast<int *>(self + 0x4c8);
        Buffer *localBuf = reinterpret_cast<Buffer *>(self + 0xb64);
        localBuf->init(w, -h, 0, 0);

        char *dockedBase = reinterpret_cast<char *>(g_00915af4_sym);
        int dockedAdjust = *reinterpret_cast<int *>(dockedBase + 4);
        Buffer *dockedBuf = reinterpret_cast<Buffer *>(
            reinterpret_cast<char *>(&g_00915af4_sym) + dockedAdjust);
        dockedBuf->copy(localBuf, r2.left, r2.top, 0, 0, w, -h);

        int frames = (*g_009b7b1c == 800) ? 0 : (*g_009b7b1c - 0x400) / 2;
        int step = (r1.bottom - r1.top) / 5;
        reinterpret_cast<Buffer *>(g_009b7490)->set_clip(&r1);

        int target = r1.bottom - *reinterpret_cast<int *>(self + 0x1108);
        int curY = r1.top;

        if ((*g_009a6490 & 0x8000000) != 0 && *g_0093f660 == 0) {
            *g_009b7ad8 = 1;
            int count = 6;
            typedef unsigned long(__stdcall * TimeFn)();
            do {
                unsigned long t1 = (*reinterpret_cast<TimeFn *>(g_00669368))();
                localBuf->copy(reinterpret_cast<Buffer *>(g_009b7490), 0, 0,
                               r1.left, r1.top, w, -h);
                reinterpret_cast<Buffer *>(g_009b7490)->draw(
                    reinterpret_cast<Buffer *>(self + 0x444), 9, frames, curY, 1, 1);
                reinterpret_cast<Win *>(this)->flip(&r1);
                curY += step;
                if (target <= curY) {
                    curY = target;
                }
                do_sound();
                unsigned long t2 = (*reinterpret_cast<TimeFn *>(g_00669368))();
                if (t2 - t1 < static_cast<unsigned long>(*g_00691af8)) {
                    do {
                        do_sound();
                        t2 = (*reinterpret_cast<TimeFn *>(g_00669368))();
                    } while (t2 - t1 < static_cast<unsigned long>(*g_00691af8));
                }
            } while (--count != 0);

            if (curY < target) {
                localBuf->copy(reinterpret_cast<Buffer *>(g_009b7490), 0, 0,
                               r1.left, r1.top, w, -h);
                reinterpret_cast<Buffer *>(g_009b7490)->draw(
                    reinterpret_cast<Buffer *>(self + 0x444), 9, r1.left, target, 1, 1);
                reinterpret_cast<Win *>(this)->flip(&r1);
            }
            reinterpret_cast<Buffer *>(g_009b7490)->set_clip(
                reinterpret_cast<RECT *>(g_009b74c0));
            *g_009b7ad8 = 0;
        }

        reinterpret_cast<Win *>(this)->hide();
        localBuf->close();

        *g_007b22a0 = 0;
        reinterpret_cast<VCall63 *>(g_007b1804)->slot062();
    }
}
