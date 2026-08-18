// ORIGINAL: 0x005D8290 ?setup_buff_sprite@Buffer@@QAEXH@Z 0x005D8290-0x005D835C FILE
// working copy - scaffold materialised by --work
// size      204 bytes
// prototype void (__thiscall ?setup_buff_sprite@Buffer@@QAEXH@Z)(Buffer* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005D834B

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?setup_buff_sprite@Buffer@@QAEXH@Z  at 0x005D8290  (204 bytes)
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
class Buffer;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;

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
    struct SpotInternal {
        RECT rect;
        int type;
        int position;
    };
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 32
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
    virtual void slot032();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00696d14 = (int *)0x00696D14;
static int *const g_009b3a54 = (int *)0x009B3A54;
static int *const g_009b3a58 = (int *)0x009B3A58;
static int *const g_009b3a5c = (int *)0x009B3A5C;
static int *const g_009b3a60 = (int *)0x009B3A60;
static int *const g_009b3a64 = (int *)0x009B3A64;
static int *const g_009b3a68 = (int *)0x009B3A68;
static int *const g_009b3a6c = (int *)0x009B3A6C;
static int *const g_009b3a70 = (int *)0x009B3A70;
static int *const g_009b3a74 = (int *)0x009B3A74;
static int *const g_009b3a78 = (int *)0x009B3A78;

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
    uint32_t field_4BC_;
    uint8_t field_4C0_[0x4C];
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

    void setup_buff_sprite(int);
};
void Buffer::setup_buff_sprite(int a1) {
    char *self = (char *)this;
    if (*(int *)(self + 0x58) != 0) {
        return;
    }
    if (a1 == -1) {
        a1 = *(unsigned char *)g_00696d14;
    }
    int val54 = *(int *)(self + 0x54);
    *(int *)(self + 0x50) = val54;
    if (val54 != 0) {
        ++*(int *)(self + 0x6c);
    }
    *g_009b3a54 = val54;
    *g_009b3a5c = *(int *)(self + 0x4a8);
    *g_009b3a60 = *(int *)(self + 0x80);
    *g_009b3a64 = -*(int *)(self + 0x84);
    *g_009b3a68 = *(int *)(self + 0x80);
    *g_009b3a6c = -*(int *)(self + 0x84);
    *g_009b3a70 = 0;
    *g_009b3a74 = 0;
    *(unsigned char *)g_009b3a58 = (unsigned char)a1;
    *g_009b3a78 = 0;

    void **field58 = *(void ***)(self + 0x58);
    int *field6c = (int *)(self + 0x6c);
    if (field58 == 0) {
        int newCount = *field6c - 1;
        *field6c = newCount;
        if (newCount <= 0) {
            *(int *)(self + 0x50) = 0;
            *field6c = 0;
        }
        return;
    }
    int newCount = *field6c - 1;
    *field6c = newCount;
    int owner = *(int *)(self + 0x50);
    if (owner != 0 && newCount <= 0) {
        typedef void (__stdcall *Fn)(void *, int);
        void **vtbl = *(void ***)field58;
        ((Fn)vtbl[0x20])(field58, owner);
        *(int *)(self + 0x50) = 0;
        *field6c = 0;
    }
}
