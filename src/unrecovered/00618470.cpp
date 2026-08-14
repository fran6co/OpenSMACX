// ORIGINAL: 0x00618470 FILE
// working copy - scaffold materialised by --work
// name      ?clear_buffers@Caviar@@QAEXH@Z
// size      173 bytes
// spans     0x00618470-0x0061851D
// prototype void (__thiscall ?clear_buffers@Caviar@@QAEXH@Z)(Caviar* this, int)
// callers   8   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8240 0x005DFB50 0x0063AF60

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?clear_buffers@Caviar@@QAEXH@Z  at 0x00618470  (173 bytes)
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
class Caviar;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    int fill(int);
    int fill(int, int, int, int, int);
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

extern "C" int __cdecl sub_63af60();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b9100 = (int *)0x009B9100;
static int *const g_009b9108 = (int *)0x009B9108;
static int *const g_009b9690 = (int *)0x009B9690;
static int *const g_009b9694 = (int *)0x009B9694;
static int *const g_009b9698 = (int *)0x009B9698;
static int *const g_009b969c = (int *)0x009B969C;
static int *const g_009b96b0 = (int *)0x009B96B0;
static int *const g_009bb438 = (int *)0x009BB438;
static int *const g_009bb43c = (int *)0x009BB43C;
static int *const g_009bb440 = (int *)0x009BB440;
static int *const g_009bb444 = (int *)0x009BB444;

class Caviar { public:
    float scene_scale_;
    uint32_t field_4_;
    uint8_t field_8_[0xC];
    uint32_t field_14_;
    uint8_t field_18_[0x4];
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint32_t field_38_;
    uint32_t field_3C_;
    uint32_t field_40_;
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    float field_6C_;
    float field_70_;
    uint32_t field_74_;
    uint32_t field_78_;
    uint32_t field_7C_;
    uint32_t field_80_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    uint8_t field_A4_;
    uint8_t field_A5_[0x63];
    int32_t field_108_;
    uint8_t field_10C_[0x640];
    uint32_t field_74C_;
    uint32_t field_750_;
    uint32_t field_754_;
    uint8_t field_758_[0xC74];
    uint32_t field_13CC_;

    void clear_buffers(int);
};
extern "C" int __cdecl sub_63af60_real(int, int, int);

void Caviar::clear_buffers(int a1) {
    if (*g_009b9100 != 0) {
        int height = *g_009b9694;
        int width = *g_009b9690;
        int bottom = *g_009b969c;
        int rowSpan = bottom - height;
        int base = *g_009b9100 + (height * 0x100 + width) * 2;
        int colSpan = *g_009b9698 - width;
        reinterpret_cast<Buffer *>(g_009b9108)->fill(width, height, colSpan, rowSpan, 9);
        reinterpret_cast<Buffer *>(g_009b96b0)->fill(0);
        if (a1 == 0) {
            if (rowSpan > 0) {
                int addr = base;
                int count = rowSpan;
                do {
                    sub_63af60_real(addr, 0xffff, colSpan);
                    addr += 0x200;
                    count--;
                } while (count != 0);
            }
            *g_009b9690 = 0;
            *g_009b9698 = 0;
            *g_009b969c = 0;
            *g_009b9694 = 0;
            *g_009bb438 = 0;
            *g_009bb440 = 0;
            *g_009bb444 = 0;
            *g_009bb43c = 0;
        }
    }
}
