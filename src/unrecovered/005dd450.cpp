// ORIGINAL: 0x005DD450 ?write_right_l@Buffer@@QAEHPADPAURECT@@H@Z 0x005DD450-0x005DD52B FILE
// TRIED: bare `strlen` (MSVC inlines it as `repne scasb`, needs `#pragma function(strlen)` to force the real call). With the pragma the structure and call sequence match, but the register allocator puts a1/this in different registers than the original (ebp/esi here vs ebx/edi there); not reproduced. 78/85 shared.
// working copy - scaffold materialised by --work
// size      219 bytes
// prototype int (__thiscall ?write_right_l@Buffer@@QAEHPADPAURECT@@H@Z)(Buffer* this, int8*, RECT*, int)
// callers   18   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC7C0 0x005DCAE0 0x006453E0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?write_right_l@Buffer@@QAEHPADPAURECT@@H@Z  at 0x005DD450  (219 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
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

extern "C" unsigned int strlen(const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009bb484 = (int *)0x009BB484;

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

    int text_width(char *, int);
    int write_multi_font_raw_l(char *, int, int, int);
    int write_right_l(char *, RECT *, int);
};
extern "C" unsigned int strlen(const char *);
#pragma function(strlen)

int Buffer::write_right_l(char * a1, RECT * a2, int a3) {
    if (a1 != 0 && a2 != 0) {
        if (font1_ == 0 || font1_->font_obj_ == 0) {
            return 3;
        }
        int sVar2 = strlen(a1);
        int sVar3 = a3;
        if (sVar2 < a3) {
            sVar3 = strlen(a1);
        }
        if (sVar3 >= 0) {
            sVar3 = strlen(a1);
            if (sVar3 < a3) {
                a3 = strlen(a1);
            }
            if (a3 != 0) {
                int width = text_width(a1, a3);
                if (font1_ == 0) {
                    font1_ = reinterpret_cast<Font *>(*g_009bb484);
                }
                return write_multi_font_raw_l(a1, a2->right - width,
                    (a2->bottom - font1_->height_ - a2->top) / 2 + a2->top, a3);
            }
        }
    }
    return 0;
}
