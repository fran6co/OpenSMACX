// ORIGINAL: 0x00479C00 FILE
// name      ?hide@MultiWin@@QAEXXZ
// size      931 bytes
// spans     0x00479C00-0x00479F8E;0x0065640A-0x0065641F
// prototype void (__thiscall ?hide@MultiWin@@QAEXXZ)(MultiWin* this)
// callers   7   call targets   17
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6600 0x004C6920 0x005D7210 0x005D7410 0x005D7470 0x005D7670 0x005D79C0 0x005D8000 0x005D84C0 0x005DFB50 0x005DFF00 0x005ECF20 0x005ECFE0 0x005EDCD0 0x005EFD20 0x005F8140 0x005FD2B0
// indirect  0x00479CAD 0x00479DF0 0x00479E59 0x00479E73 0x00479F41 0x00479F50 0x00479F57 0x00479F5E
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00479C00
// measured tier  MISMATCH
// divergence     9
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00479c00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?hide@MultiWin@@QAEXXZ  at 0x00479C00  (931 bytes)
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
class MultiWin;
typedef void * PVOID;
struct RECT;
class Scroll;
class Sound;
class Spot;
class Sprite;
class Wave;
class Win;

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    Buffer();
    int copy(Buffer *, int, int, int, int, int, int);
    int draw(Buffer *, int, int, int, int, int);
    int fill(int);
    int init(int, int, int, ExtDirectDraw *);
    int set_clip(RECT *);
    void close();
    void resize(int, int);
    ~Buffer();
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

class Sound { public:
    void fade(unsigned long);
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
    void __cdecl flip(RECT *);
    void client_to_screen(RECT *);
    void hide();
    void screen_to_client(RECT *);
    void update_window_to_buffer(Buffer *);
};

int __cdecl do_sound();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 2, 12, 63
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
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
    virtual void slot062();
    virtual void slot063();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00656415 = (int *)0x00656415;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_006761e0 = (int *)0x006761E0;
static int *const g_00691af8 = (int *)0x00691AF8;
static int *const g_00696d14 = (int *)0x00696D14;
static int *const g_007cff04 = (int *)0x007CFF04;
static int *const g_007fff00 = (int *)0x007FFF00;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00915af4 = (int *)0x00915AF4;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009b7490 = (int *)0x009B7490;
static int *const g_009b74c0 = (int *)0x009B74C0;
static int *const g_009b7ad8 = (int *)0x009B7AD8;

class MultiWin { public:
    void hide();
};

typedef unsigned long (__stdcall *TimeGetTimeFn)();

void MultiWin::hide() {
    char *const self = reinterpret_cast<char *>(this);
    Buffer buf;

    if (*g_0093f660 != 0 || *reinterpret_cast<int *>(self + 0xa24) != 0) {
        if (*reinterpret_cast<unsigned int *>(g_009a6490) & 0x400) {
            reinterpret_cast<Wave *>(g_007fff00)->play();
        }
        *reinterpret_cast<int *>(self + 0x1168) = -1;
        *g_007cff04 = 0;

        int w = *reinterpret_cast<int *>(self + 0x4c4);
        int h = *reinterpret_cast<int *>(self + 0x4c8);
        buf.resize(w, -h);
        buf.fill(*reinterpret_cast<uint8_t *>(g_00696d14));
        reinterpret_cast<Win *>(this)->update_window_to_buffer(&buf);
        reinterpret_cast<VCall *>(this)->slot012();

        RECT rect1 = *reinterpret_cast<RECT *>(self + 0x474);
        reinterpret_cast<Win *>(this)->client_to_screen(&rect1);

        RECT rect2 = rect1;
        reinterpret_cast<Win *>(
            reinterpret_cast<char *>(g_009156b0) +
            (*reinterpret_cast<int **>(g_009156b0))[1])->screen_to_client(&rect2);

        Buffer *const buf2 = reinterpret_cast<Buffer *>(self + 0xa54);
        buf2->init(w, -h, 0, 0);

        reinterpret_cast<Buffer *>(
            reinterpret_cast<char *>(g_00915af4) +
            (*reinterpret_cast<int **>(g_009156b0))[1])->copy(
                buf2, rect2.left, rect2.top, 0, 0, w, -h);

        int slide = rect1.bottom - *reinterpret_cast<int *>(self + 0x1100) - rect1.top;
        if (*g_0093f660 != 0) {
            slide += -0xe - *reinterpret_cast<int *>(self + 0x112c);
        }
        slide /= 5;

        reinterpret_cast<Buffer *>(g_009b7490)->set_clip(&rect1);

        int limit = rect1.bottom - *reinterpret_cast<int *>(self + 0x1100);
        if (*g_0093f660 != 0) {
            limit += -0xe - *reinterpret_cast<int *>(self + 0x112c);
        }

        if ((*reinterpret_cast<unsigned int *>(g_009a6490) & 0x8000000) &&
            *g_0093f660 == 0) {
            *g_009b7ad8 = 1;
            int pos = rect1.top;
            int count = 6;
            do {
                unsigned long t0 = (*reinterpret_cast<TimeGetTimeFn *>(g_00669368))();
                buf2->copy(reinterpret_cast<Buffer *>(g_009b7490), 0, 0, rect1.left, rect1.top, w, -h);
                buf.draw(reinterpret_cast<Buffer *>(g_009b7490), 9, rect1.left, pos, 1, 1);
                reinterpret_cast<Win *>(this)->flip(&rect1);
                pos += slide;
                if (limit <= pos) {
                    pos = limit;
                }
                do_sound();
                unsigned long t1 = (*reinterpret_cast<TimeGetTimeFn *>(g_00669368))();
                if (t1 - t0 < *reinterpret_cast<unsigned int *>(g_00691af8)) {
                    do {
                        do_sound();
                        t1 = (*reinterpret_cast<TimeGetTimeFn *>(g_00669368))();
                    } while (t1 - t0 < *reinterpret_cast<unsigned int *>(g_00691af8));
                }
                --count;
            } while (count != 0);

            if (pos < limit) {
                buf2->copy(reinterpret_cast<Buffer *>(g_009b7490), 0, 0, rect1.left, rect1.top, w, -h);
                reinterpret_cast<Buffer *>(self + 0x444)->draw(
                    reinterpret_cast<Buffer *>(g_009b7490), 9, rect1.left, limit, 1, 1);
                reinterpret_cast<Win *>(this)->flip(&rect1);
            }

            reinterpret_cast<Buffer *>(g_009b7490)->set_clip(reinterpret_cast<RECT *>(g_009b74c0));
            *g_009b7ad8 = 0;
        }

        if (*reinterpret_cast<unsigned int *>(g_009a6490) & 0x400) {
            reinterpret_cast<Sound *>(g_007fff00)->fade(0x32);
        }

        *reinterpret_cast<int *>(self + 0xa24) = 0;

        if (*g_0093f660 == 0) {
            reinterpret_cast<Win *>(this)->hide();
        } else {
            reinterpret_cast<VCall *>(self + 0x1d10)->slot002();
            reinterpret_cast<VCall *>(self + 0x11c4)->slot002();
            reinterpret_cast<VCall *>(this)->slot012();
            reinterpret_cast<VCall *>(this)->slot063();
        }

        buf2->close();
    }
}
