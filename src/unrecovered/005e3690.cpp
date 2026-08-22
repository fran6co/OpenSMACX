// ORIGINAL: 0x005E3690 sub_5e3690 0x005E3690-0x005E372A FILE
// TRIED: naked strlen() got inlined to repne scasb (0.36 mnemonic sim); an untyped `_strlen` nullary alias + StrlenFn cast keeps the CALL and lifted similarity to 0.88. Buffer_5e3690 : public Buffer gives a real __thiscall receiver since explicit __thiscall is C4234.
// working copy - scaffold materialised by --work
// size      154 bytes
// prototype int (__stdcall sub_5E3690)(LPCSTR lpString, int x, int y)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DCAE0 0x006453E0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5e3690  at 0x005E3690  (154 bytes)
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
typedef const char * LPCSTR;
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
    int write_multi_font_raw_l(char *, int, int, int);
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

// `strlen` above is the fast intrinsic MSVC recognises by name and inlines
// under /O2; the original calls out to it. Reach the real symbol through an
// untyped nullary alias and a function-pointer cast.
extern "C" int __cdecl _strlen();
typedef unsigned int (__cdecl *StrlenFn)(const char *);

class Buffer_5e3690 : public Buffer {
public:
    int sub_5e3690(LPCSTR a1, int a2, int a3);
};

int Buffer_5e3690::sub_5e3690(LPCSTR a1, int a2, int a3) {
    if (a1 == 0) {
        return a2;
    }
    unsigned int len1 = reinterpret_cast<StrlenFn>(_strlen)(a1);
    if (font1_ != 0 && font1_->font_obj_ != 0) {
        unsigned int len2 = reinterpret_cast<StrlenFn>(_strlen)(a1);
        unsigned int len3 = len1;
        if ((int)len2 < (int)len1) {
            len3 = reinterpret_cast<StrlenFn>(_strlen)(a1);
        }
        if ((int)len3 >= 0) {
            len3 = reinterpret_cast<StrlenFn>(_strlen)(a1);
            if ((int)len3 < (int)len1) {
                len1 = reinterpret_cast<StrlenFn>(_strlen)(a1);
            }
            if (len1 != 0) {
                return write_multi_font_raw_l(const_cast<char *>(a1), a2, a3, len1);
            }
        }
        return a2;
    }
    return 3;
}
