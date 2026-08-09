// ORIGINAL: 0x0045D970 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0045D970
// name           ?slide_hide@@YAXPAUGraphicWin@@H@Z
// size           1371 bytes
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045d970/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?slide_hide@@YAXPAUGraphicWin@@H@Z  at 0x0045D970  (1371 bytes)
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
class Sound;
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
    Buffer();
    int copy(Buffer *, int, int, int, int, int, int);
    int draw(Buffer *, int, int, int, int, int);
    int fill(int);
    int init(int, int, int, ExtDirectDraw *);
    int set_clip(RECT *);
    ~Buffer();
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
    int fill(int, int, int, int, int);
    void soft_update();
};
class MainInterface { public:
    void restore_back(int);
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
    int draw(Buffer *, int, int, int, int, int);
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
    void update_back_to_window(Buffer *);
    void update_window_to_buffer(Buffer *);
};
extern "C" int __cdecl _alloca_probe();
int __cdecl do_sound();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00655798 = (int *)0x00655798;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_006757e0 = (int *)0x006757E0;
static int *const g_00691af8 = (int *)0x00691AF8;
static int *const g_00696d14 = (int *)0x00696D14;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007ae95c = (int *)0x007AE95C;
static int *const g_007ae960 = (int *)0x007AE960;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007aec94 = (int *)0x007AEC94;
static int *const g_007aece4 = (int *)0x007AECE4;
static int *const g_007aece8 = (int *)0x007AECE8;
static int *const g_007af55c = (int *)0x007AF55C;
static int *const g_007af560 = (int *)0x007AF560;
static int *const g_007af5c0 = (int *)0x007AF5C0;
static int *const g_007af5c4 = (int *)0x007AF5C4;
static int *const g_007af5c8 = (int *)0x007AF5C8;
static int *const g_007af5cc = (int *)0x007AF5CC;
static int *const g_007cffc4 = (int *)0x007CFFC4;
static int *const g_007cffcc = (int *)0x007CFFCC;
static int *const g_007cffdc = (int *)0x007CFFDC;
static int *const g_007cfff0 = (int *)0x007CFFF0;
static int *const g_007cfff8 = (int *)0x007CFFF8;
static int *const g_007d001c = (int *)0x007D001C;
static int *const g_007d0024 = (int *)0x007D0024;
static int *const g_007d0034 = (int *)0x007D0034;
static int *const g_007d0048 = (int *)0x007D0048;
static int *const g_007d0050 = (int *)0x007D0050;
static int *const g_007d0060 = (int *)0x007D0060;
static int *const g_007d1750 = (int *)0x007D1750;
static int *const g_007d1758 = (int *)0x007D1758;
static int *const g_007d38b8 = (int *)0x007D38B8;
static int *const g_007d3930 = (int *)0x007D3930;
static int *const g_0093a934 = (int *)0x0093A934;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009b7490 = (int *)0x009B7490;
static int *const g_009b74c0 = (int *)0x009B74C0;
static int *const g_009b7ad8 = (int *)0x009B7AD8;
static int *const g_009b7b1c = (int *)0x009B7B1C;

typedef unsigned long (__stdcall *TimeGetTimeFn)();
struct ExtDirectDraw;

void __cdecl slide_hide(GraphicWin *a1, int a2) {
    Buffer bufA;
    Buffer bufB;
    Buffer bufC;

    if (a1 != 0 && reinterpret_cast<Win *>(a1)->is_visible() != 0 && (*g_009a6490 & 0x8000000) != 0 && *g_0093f660 == 0) {
        if ((*g_009a6490 & 0x400) != 0) {
            reinterpret_cast<Wave *>(g_007d38b8)->play();
        }

        int local_38 = (*g_009b7b1c != 0x320 ? 0x90 : 0) + 0x5e;
        int frames = (*g_0093f660 != 0 || *g_0093a934 != 0 || *g_0093a938 != 0) ? 3 : 5;

        char *a1b = reinterpret_cast<char *>(a1);
        int *rectSrc = reinterpret_cast<int *>(a1b + 0x474);
        RECT r1;
        r1.left = rectSrc[0];
        r1.top = rectSrc[1];
        r1.right = rectSrc[2];
        r1.bottom = rectSrc[3];
        reinterpret_cast<Win *>(a1)->client_to_screen(&r1);

        bufB.init(*g_007aece4, -(*g_007aece8), 0, 0);
        bufB.fill(*reinterpret_cast<unsigned char *>(g_00696d14));
        reinterpret_cast<Win *>(g_007ae820)->update_window_to_buffer(&bufB);

        int f4c4 = *reinterpret_cast<int *>(a1b + 0x4c4);
        int f4c8 = *reinterpret_cast<int *>(a1b + 0x4c8);
        bufC.init(f4c4, -f4c8, 0, 0);
        bufA.init(f4c4, -f4c8, 0, 0);
        bufA.fill(*reinterpret_cast<unsigned char *>(g_00696d14));

        a1->fill(0, 0, 0x17, -f4c8, 9);
        a1->fill(f4c4 - 0x17, 0, 0x17, -f4c8, 9);

        reinterpret_cast<Win *>(a1)->update_window_to_buffer(&bufA);
        reinterpret_cast<Win *>(a1)->update_back_to_window(&bufC);

        RECT r2;
        r2.left = r1.left;
        r2.top = 0;
        r2.right = r1.right - r1.left + r1.left;
        r2.bottom = 0x39;
        reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&r2);

        int local_20 = (r1.bottom - r1.top) / frames;
        bufC.set_clip(&r1);

        char *bufAraw = reinterpret_cast<char *>(&bufA);
        int bufAHeight = *reinterpret_cast<int *>(bufAraw + 0x80);

        int local_2c = *g_007cffdc + r1.left;
        int local_30 = r1.left;
        int local_58 = r1.left;
        int edxTmp = bufAHeight - *g_007d0060;
        edxTmp -= *g_007d0034;
        int local_28 = edxTmp + r1.left - 0x20;
        *g_009b7ad8 = 1;
        int ecxTmp = bufAHeight - *g_007d0060;
        int local_24 = ecxTmp + r1.left - 0x20;
        int local_54 = r1.top;
        int local_50 = r1.right;
        int local_4c = r1.bottom;
        int edi = r1.top + local_20;

        int loopCount = a2;
        if (loopCount > 0) {
            TimeGetTimeFn timeGetTime = *reinterpret_cast<TimeGetTimeFn *>(g_00669368);
            unsigned long local_34 = timeGetTime();

            do {
                bufC.copy(reinterpret_cast<Buffer *>(g_009b7490), 0, 0, local_58, f4c4, -f4c8, edi);
                bufA.draw(reinterpret_cast<Buffer *>(g_009b7490), 9, r1.left, edi, 1, 1);

                int hEdge = (edi > local_38) ? edi : local_38;
                reinterpret_cast<Sprite *>(g_007cffc4)->draw(
                    reinterpret_cast<Buffer *>(g_009b7490), (*g_007cffcc) & 0xff, local_30, hEdge, 1, 1);
                reinterpret_cast<Sprite *>(g_007cfff0)->draw(
                    reinterpret_cast<Buffer *>(g_009b7490), (*g_007cfff8) & 0xff, local_2c, edi, 1, 1);
                reinterpret_cast<Sprite *>(g_007d001c)->draw(
                    reinterpret_cast<Buffer *>(g_009b7490), (*g_007d0024) & 0xff, local_28, edi, 1, 1);

                if (edi > local_58)
                    local_58 = edi;
                reinterpret_cast<Sprite *>(g_007d0048)->draw(
                    reinterpret_cast<Buffer *>(g_009b7490), (*g_007d0050) & 0xff, local_24, local_58, 1, 1);

                RECT clipR;
                clipR.left = local_2c;
                clipR.top = 0;
                clipR.right = local_54;
                clipR.bottom = 0x39;
                reinterpret_cast<Buffer *>(g_009b7490)->set_clip(&clipR);
                bufB.draw(reinterpret_cast<Buffer *>(g_009b7490), 9, *g_007ae95c, *g_007ae960, 1, 1);

                RECT flipR;
                flipR.left = local_30;
                flipR.top = local_54;
                flipR.right = local_50;
                flipR.bottom = local_4c;
                reinterpret_cast<Win *>(a1)->flip(&flipR);

                edi += local_20;
                reinterpret_cast<Buffer *>(g_009b7490)->set_clip(&r1);

                do {
                    do_sound();
                    int waitTarget;
                    if (*g_0093a934 == 0 && *g_0093a938 == 0) {
                        waitTarget = *g_00691af8;
                    } else {
                        int dv = *g_00691af8;
                        waitTarget = dv / 2;
                    }
                    unsigned long now = timeGetTime();
                    if (static_cast<long>(now - local_34) >= waitTarget)
                        break;
                } while (true);

                loopCount -= 1;
            } while (loopCount != 0);
        }

        if ((*g_009a6490 & 0x400) != 0) {
            reinterpret_cast<Sound *>(g_007d38b8)->fade(0x32);
        }
        reinterpret_cast<Buffer *>(g_009b7490)->set_clip(reinterpret_cast<RECT *>(g_009b74c0));

        if ((a2 & 2) != 0) {
            reinterpret_cast<Sprite *>(g_007d1750)
                ->draw(reinterpret_cast<Buffer *>(g_007d1758), (*g_007af560) & 0xff, *g_007af55c,
                       reinterpret_cast<int>(g_007aec64), 1, 1);

            RECT r3;
            r3.left = *g_007af5c0;
            r3.top = *g_007af5c4;
            r3.right = *g_007af5c8;
            r3.bottom = *g_007af5cc + 0x64;
            reinterpret_cast<Buffer *>(g_007aec64)->set_clip(&r3);
            *g_007d3930 = 0;
            reinterpret_cast<MainInterface *>(g_007ae820)->restore_back(0);
            reinterpret_cast<Buffer *>(g_007aec64)->set_clip(reinterpret_cast<RECT *>(g_007aec94));
            reinterpret_cast<GraphicWin *>(g_007ae820)->soft_update();
        }
        *g_009b7ad8 = 0;
    }
}
