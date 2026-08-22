// ORIGINAL: 0x00615F50 ?text_fits@EditBox@@QAEHXZ 0x00615F50-0x00616029 FILE
// TRIED: plain `strcat(buf, ...)` compiles to an inlined `repne scasb`/copy sequence, not the original's `call _strcat`; `#pragma function(strcat)` restores the call. Landed at 82.4% mnemonic similarity (172 vs 217 bytes): the original duplicates its `text_width` + threshold-compare tail in BOTH branches (masked and unmasked text), but this source's identical trailing 3 statements in each branch let VC6 tail-merge them into one shared block with a `jmp`, which the original does not do even though its two tails are mnemonic-for-mnemonic identical too. Tried: a shared vs per-branch `threshold`/`base` local, folding the compare inline vs a named `threshold` variable - the merge persisted across all of them.
// working copy - scaffold materialised by --work
// size      217 bytes
// prototype int (__thiscall ?text_fits@EditBox@@QAEHXZ)(EditBox* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC7C0 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?text_fits@EditBox@@QAEHXZ  at 0x00615F50  (217 bytes)
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
class EditBox;
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
    int text_width(char *, int);
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

extern "C" char *strcat(char *, const char *);

class EditBox { public:
    uint8_t pad_0_[0xB48];
    uint32_t field_b48_;

    int text_fits();
};
#pragma function(strcat)
int EditBox::text_fits() {
    char *self = reinterpret_cast<char *>(this);
    char buf[256];
    if (*reinterpret_cast<unsigned char *>(self + 0xb3c) & 2) {
        buf[0] = 0;
        strcat(buf, self + 0xa14);
        char *p = buf;
        char c = buf[0];
        while (c != 0) {
            *p = '*';
            char next = p[1];
            ++p;
            c = next;
        }
        if (*reinterpret_cast<unsigned char *>(self + 0xb3c) & 2) {
            int base = *reinterpret_cast<int *>(self + 0x4c4);
            int width = reinterpret_cast<Buffer *>(self + 0x444)->text_width(
                buf, *reinterpret_cast<int *>(self + 0xb44) -
                     *reinterpret_cast<int *>(self + 0xb18));
            if (width < base - *reinterpret_cast<int *>(self + 0xb34) * 2) {
                return 1;
            }
            return 0;
        }
    }
    char *textPtr = self + 0xa14 + *reinterpret_cast<int *>(self + 0xb18);
    int base2 = *reinterpret_cast<int *>(self + 0x4c4);
    int width2 = reinterpret_cast<Buffer *>(self + 0x444)->text_width(
        textPtr, *reinterpret_cast<int *>(self + 0xb44) -
                 *reinterpret_cast<int *>(self + 0xb18));
    if (width2 < base2 - *reinterpret_cast<int *>(self + 0xb34) * 2) {
        return 1;
    }
    return 0;
}
