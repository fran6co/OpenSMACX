// ORIGINAL: 0x0045F740 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0045F740
// name           ??1Interlude@@QAE@XZ
// size           510 bytes
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045f740/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1Interlude@@QAE@XZ  at 0x0045F740  (510 bytes)
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
class Interlude;
typedef char * LPSTR;
typedef void * LPVOID;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
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
    void close();
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
static int *const g_0065584f = (int *)0x0065584F;
static int *const g_006693a0 = (int *)0x006693A0;
static int *const g_006693a4 = (int *)0x006693A4;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_006698c0 = (int *)0x006698C0;
static int *const g_006698c4 = (int *)0x006698C4;
static int *const g_0066c700 = (int *)0x0066C700;
static int *const g_0066c708 = (int *)0x0066C708;
static int *const g_006758b8 = (int *)0x006758B8;
static int *const g_009b3374 = (int *)0x009B3374;

class Interlude { public:
    ~Interlude();
};

// Extra shim: the emitted VCall's slot000/slot001 are nullary, but this body
// dispatches slot 1 with one pushed argument (a payload pointer) and slot 0
// with one pushed argument (the scalar-deleting-destructor flag, 1). Same
// vtable-slot indices as VCall, different signatures.
class VCallArg { public:
    virtual void slot000(int);
    virtual void slot001(void *);
};

Interlude::~Interlude() {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<int **>(self) = g_0066c708;
    *reinterpret_cast<int **>(self + 0x444) = g_0066c700;

    reinterpret_cast<GraphicWin *>(self)->close();

    // Destroy the linked structure at offset 0xA24: guarded by a head
    // pointer at 0xA2C, iterated `[0x10]` (count) times, unlinking through
    // `[0xC]` and releasing the payload at `[8]` through two vtable calls.
    char *listA = self + 0xA24;
    void *anchor = *reinterpret_cast<void **>(self + 0xA2C);
    if (anchor != 0) {
        int count = *reinterpret_cast<int *>(listA + 0x10);
        for (int i = 0; i < count; ++i) {
            char *node = *reinterpret_cast<char **>(listA + 8);
            VCallArg *ops = *reinterpret_cast<VCallArg **>(listA);
            char *next = *reinterpret_cast<char **>(node + 0xC);
            *reinterpret_cast<char **>(listA + 0xC) = next;
            char *payload = *reinterpret_cast<char **>(node + 8);

            ops->slot001(payload);

            if (payload != 0) {
                VCallArg **pvt = reinterpret_cast<VCallArg **>(payload);
                int adjust = *reinterpret_cast<int *>(
                    reinterpret_cast<char *>(*pvt) + 4);
                VCallArg *adjusted =
                    reinterpret_cast<VCallArg *>(payload + adjust);
                adjusted->slot000(1);
            }

            char *head = *reinterpret_cast<char **>(listA + 8);
            *reinterpret_cast<void **>(head + 8) = 0;
            char *head2 = *reinterpret_cast<char **>(listA + 8);
            if (head2 != 0) {
                VCallArg **hvt = reinterpret_cast<VCallArg **>(head2);
                int adjust2 = *reinterpret_cast<int *>(
                    reinterpret_cast<char *>(*hvt) + 4);
                VCallArg *adjusted2 =
                    reinterpret_cast<VCallArg *>(head2 + adjust2);
                adjusted2->slot000(1);
            }

            *reinterpret_cast<char **>(listA + 8) =
                *reinterpret_cast<char **>(listA + 0xC);
        }
        *reinterpret_cast<void **>(listA + 8) = 0;
        *reinterpret_cast<void **>(listA + 0x14) = 0;
        *reinterpret_cast<void **>(listA + 0x10) = 0;
    }
    *reinterpret_cast<void **>(listA + 0x14) = 0;

    Buffer *ownBuffer = reinterpret_cast<Buffer *>(self + 0xA54);
    ownBuffer->close();
    ownBuffer->~Buffer();

    // Two more embedded sub-objects, both anchored 0x28 bytes apart from the
    // list above, each resetting its own vtable and an adjacent indirect
    // vtable slot before an unnamed cleanup call.
    char *sub1 = self + 0xA24;
    *reinterpret_cast<int **>(sub1) = g_006698c4;
    void *aux1 = *reinterpret_cast<void **>(self + 0xA28);
    int **auxTbl1 =
        reinterpret_cast<int **>(*reinterpret_cast<int **>(aux1) + 1);
    *reinterpret_cast<int **>(*auxTbl1 + reinterpret_cast<long>(sub1)) =
        g_006698c0;
    fn_00402970();
    *reinterpret_cast<void **>(sub1 + 0x14) = 0;

    char *sub2 = self + 0xA24;
    *reinterpret_cast<int **>(sub2) = g_006693a4;
    void *aux2 = *reinterpret_cast<void **>(self + 0xA28);
    int **auxTbl2 =
        reinterpret_cast<int **>(*reinterpret_cast<int **>(aux2) + 1);
    *reinterpret_cast<int **>(*auxTbl2 + reinterpret_cast<long>(sub2)) =
        g_006693a0;

    void *sub2anchor = *reinterpret_cast<void **>(sub2 + 8);
    if (sub2anchor != 0) {
        int count2 = *reinterpret_cast<int *>(sub2 + 0x10);
        for (int j = 0; j < count2; ++j) {
            char *node = *reinterpret_cast<char **>(sub2 + 8);
            VCallArg *ops = *reinterpret_cast<VCallArg **>(sub2);
            char *next = *reinterpret_cast<char **>(node + 0xC);
            *reinterpret_cast<char **>(sub2 + 0xC) = next;
            char *payload = *reinterpret_cast<char **>(node + 8);

            ops->slot001(payload);

            if (payload != 0) {
                VCallArg **pvt = reinterpret_cast<VCallArg **>(payload);
                int adjust = *reinterpret_cast<int *>(
                    reinterpret_cast<char *>(*pvt) + 4);
                VCallArg *adjusted =
                    reinterpret_cast<VCallArg *>(payload + adjust);
                adjusted->slot000(1);
            }

            char *head = *reinterpret_cast<char **>(sub2 + 8);
            *reinterpret_cast<void **>(head + 8) = 0;
            char *head2 = *reinterpret_cast<char **>(sub2 + 8);
            if (head2 != 0) {
                VCallArg **hvt = reinterpret_cast<VCallArg **>(head2);
                int adjust2 = *reinterpret_cast<int *>(
                    reinterpret_cast<char *>(*hvt) + 4);
                VCallArg *adjusted2 =
                    reinterpret_cast<VCallArg *>(head2 + adjust2);
                adjusted2->slot000(1);
            }

            *reinterpret_cast<char **>(sub2 + 8) =
                *reinterpret_cast<char **>(sub2 + 0xC);
        }
        *reinterpret_cast<void **>(sub2 + 8) = 0;
        *reinterpret_cast<void **>(sub2 + 0x14) = 0;
        *reinterpret_cast<void **>(sub2 + 0x10) = 0;
    }

    char *saved18 = self + 0xA4C;
    char *savedThis = self;
    *reinterpret_cast<void **>(sub2 + 0x14) = 0;

    int carried = *reinterpret_cast<int *>(saved18 + 4);
    *reinterpret_cast<int **>(saved18) = g_006693ac;
    *g_009b3374 = carried;

    reinterpret_cast<GraphicWin *>(savedThis)->~GraphicWin();
}
