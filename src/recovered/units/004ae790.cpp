// ORIGINAL: 0x004AE790 FILE
// name      ??1SetupWin@@QAE@XZ
// size      611 bytes
// spans     0x004AE790-0x004AE99E;0x006596A0-0x006596F5
// prototype void (__thiscall ??1SetupWin@@QAE@XZ)(SetupWin* this)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00402970 0x005D4DD0 0x005D7410 0x005D7470 0x005FA830 0x005FA870
// indirect  0x004AE825 0x004AE837 0x004AE857 0x004AE8F9 0x004AE90B 0x004AE92B
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004AE790
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004ae790/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1SetupWin@@QAE@XZ  at 0x004AE790  (611 bytes)
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
class SetupWin;
class Spot;
class Sprite;
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
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    void shutdown();
    ~Spot();
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
    void close();
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
    ~GraphicWin();
};
extern "C" int __cdecl fn_00402970();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006596eb = (int *)0x006596EB;
static int *const g_006693a0 = (int *)0x006693A0;
static int *const g_006693a4 = (int *)0x006693A4;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_006698c0 = (int *)0x006698C0;
static int *const g_006698c4 = (int *)0x006698C4;
static int *const g_0066d8e0 = (int *)0x0066D8E0;
static int *const g_0066d8e8 = (int *)0x0066D8E8;
static int *const g_00678aa8 = (int *)0x00678AA8;
static int *const g_009b3374 = (int *)0x009B3374;

class SetupWin { public:
    ~SetupWin();
};

class StringStruct { public:
    void remove_all();
};
class VCallArgs { public:
    virtual void slot000(int);
    virtual void slot001(int);
};

SetupWin::~SetupWin() {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<void **>(self + 0) = g_0066d8e8;
    *reinterpret_cast<void **>(self + 0x444) = g_0066d8e0;

    *reinterpret_cast<int *>(self + 0xfd0) = 0;
    *reinterpret_cast<int *>(self + 0xfcc) = 0;
    *reinterpret_cast<int *>(self + 0xa14) = 0;
    *reinterpret_cast<int *>(self + 0xa18) = 0;

    reinterpret_cast<Buffer *>(self + 0xa44)->close();
    reinterpret_cast<Spot *>(self + 0xfd4)->shutdown();

    char *vec1 = self + 0xfe0;
    if (*reinterpret_cast<int *>(vec1 + 8) != 0) {
        int cap = *reinterpret_cast<int *>(vec1 + 0x10);
        if (cap > 0) {
            for (int i = 0; i < cap; i++) {
                char *data = *reinterpret_cast<char **>(vec1 + 8);
                int next = *reinterpret_cast<int *>(data + 0xc);
                *reinterpret_cast<int *>(vec1 + 0xc) = next;
                int item = *reinterpret_cast<int *>(data + 8);
                reinterpret_cast<VCallArgs *>(vec1)->slot001(item);
                if (item != 0) {
                    char *itemp = reinterpret_cast<char *>(item);
                    int *ivt = *reinterpret_cast<int **>(itemp);
                    int disp = ivt[1];
                    reinterpret_cast<VCallArgs *>(itemp + disp)->slot000(1);
                }
                char *data2 = *reinterpret_cast<char **>(vec1 + 8);
                *reinterpret_cast<int *>(data2 + 8) = 0;
                char *data3 = *reinterpret_cast<char **>(vec1 + 8);
                if (data3 != 0) {
                    int *ivt2 = *reinterpret_cast<int **>(data3);
                    int disp2 = ivt2[1];
                    reinterpret_cast<VCallArgs *>(data3 + disp2)->slot000(1);
                }
                *reinterpret_cast<char **>(vec1 + 8) = *reinterpret_cast<char **>(vec1 + 0xc);
            }
        }
        *reinterpret_cast<int *>(vec1 + 8) = 0;
        *reinterpret_cast<int *>(vec1 + 0x14) = 0;
        *reinterpret_cast<int *>(vec1 + 0x10) = 0;
    }
    *reinterpret_cast<int *>(vec1 + 0x14) = 0;

    reinterpret_cast<Buffer *>(self + 0x1118)->~Buffer();

    char *base2 = vec1 + 0x28;
    char *vbase_addr = base2 - 0x24;
    char *sub1 = base2 - 0x28;

    *reinterpret_cast<void **>(sub1) = g_006698c4;
    {
        int vbt = *reinterpret_cast<int *>(vbase_addr);
        int rel = *reinterpret_cast<int *>(vbt + 4);
        *reinterpret_cast<void **>(vbase_addr + rel) = g_006698c0;
    }
    reinterpret_cast<StringStruct *>(sub1)->remove_all();

    *reinterpret_cast<int *>(sub1 + 0x14) = 0;

    char *vec2 = base2 - 0x28;
    *reinterpret_cast<void **>(vec2) = g_006693a4;
    {
        int vbt = *reinterpret_cast<int *>(vbase_addr);
        int rel = *reinterpret_cast<int *>(vbt + 4);
        *reinterpret_cast<void **>(vbase_addr + rel) = g_006693a0;
    }

    if (*reinterpret_cast<int *>(vec2 + 8) != 0) {
        int cap2 = *reinterpret_cast<int *>(vec2 + 0x10);
        if (cap2 > 0) {
            for (int i = 0; i < cap2; i++) {
                char *data = *reinterpret_cast<char **>(vec2 + 8);
                int next = *reinterpret_cast<int *>(data + 0xc);
                *reinterpret_cast<int *>(vec2 + 0xc) = next;
                int item = *reinterpret_cast<int *>(data + 8);
                reinterpret_cast<VCallArgs *>(vec2)->slot001(item);
                if (item != 0) {
                    char *itemp = reinterpret_cast<char *>(item);
                    int *ivt = *reinterpret_cast<int **>(itemp);
                    int disp = ivt[1];
                    reinterpret_cast<VCallArgs *>(itemp + disp)->slot000(1);
                }
                char *data2 = *reinterpret_cast<char **>(vec2 + 8);
                *reinterpret_cast<int *>(data2 + 8) = 0;
                char *data3 = *reinterpret_cast<char **>(vec2 + 8);
                if (data3 != 0) {
                    int *ivt2 = *reinterpret_cast<int **>(data3);
                    int disp2 = ivt2[1];
                    reinterpret_cast<VCallArgs *>(data3 + disp2)->slot000(1);
                }
                *reinterpret_cast<char **>(vec2 + 8) = *reinterpret_cast<char **>(vec2 + 0xc);
            }
        }
        *reinterpret_cast<int *>(vec2 + 8) = 0;
        *reinterpret_cast<int *>(vec2 + 0x14) = 0;
        *reinterpret_cast<int *>(vec2 + 0x10) = 0;
    }
    *reinterpret_cast<int *>(vec2 + 0x14) = 0;

    *reinterpret_cast<void **>(base2) = g_006693ac;
    *g_009b3374 = *reinterpret_cast<int *>(base2 + 4);

    reinterpret_cast<Spot *>(self + 0xfd4)->~Spot();
    reinterpret_cast<Buffer *>(self + 0xa44)->~Buffer();
    reinterpret_cast<GraphicWin *>(self)->~GraphicWin();
}
