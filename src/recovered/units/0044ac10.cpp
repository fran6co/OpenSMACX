// ORIGINAL: 0x0044AC10 FILE
// name      ?demote@FameWin@@QAEXH@Z
// size      583 bytes
// spans     0x0044AC10-0x0044AE42;0x00654EBA-0x00654ECF
// prototype void (__thiscall ?demote@FameWin@@QAEXH@Z)(FameWin* this, int)
// callers   1   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7DE0 0x005DF590 0x00645470 0x00645E90 0x0064FC88
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0044AC10
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0044ac10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?demote@FameWin@@QAEXH@Z  at 0x0044AC10  (583 bytes)
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
class FameWin;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
class Palette;
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
    int load_pcx(const char *, Palette *, int, int);
    int write_pcx(char *);
    ~Buffer();
};
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
extern "C" int __cdecl strncpy();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00654ec5 = (int *)0x00654EC5;
static int *const g_006751a8 = (int *)0x006751A8;
static int *const g_006845e4 = (int *)0x006845E4;
static int *const g_006845ec = (int *)0x006845EC;
static int *const g_006845f0 = (int *)0x006845F0;
static int *const g_006845f8 = (int *)0x006845F8;
static int *const g_00684600 = (int *)0x00684600;
static int *const g_00684604 = (int *)0x00684604;
static int *const g_009a3840 = (int *)0x009A3840;
static int *const g_009b86a0 = (int *)0x009B86A0;

class FameWin { public:
    uint8_t pad_0_[0xA918];
    uint32_t field_a918_;

    void demote(int);
};

// The scaffold's `_itoa`/`strncpy` declarations take no parameters (stale -
// the calls below push three args each). Rather than redeclare the same
// extern "C" name with a different signature (a hard conflict with the
// declaration already in scope), their addresses are reinterpreted through
// the correct function-pointer type at the call site.
typedef char *(__cdecl *ItoaFn)(int, char *, int);
typedef char *(__cdecl *StrncpyFn)(char *, const char *, size_t);

void FameWin::demote(int a1) {
    Buffer buf;
    if (a1 < 4) {
        char numbuf[80];
        char *self = reinterpret_cast<char *>(this);
        char *msg = reinterpret_cast<char *>(g_009b86a0);
        int fame = *reinterpret_cast<int *>(self + 0xa4c);

        *msg = 0;
        strcat(msg, reinterpret_cast<const char *>(g_006845e4));
        reinterpret_cast<ItoaFn>(&_itoa)(fame, numbuf, 10);
        strcat(msg, numbuf);
        strcat(msg, reinterpret_cast<const char *>(g_006845ec));
        reinterpret_cast<ItoaFn>(&_itoa)(a1 + 1, numbuf, 10);
        strcat(msg, numbuf);
        strcat(msg, reinterpret_cast<const char *>(g_006845f0));
        buf.load_pcx(msg, 0, 0, 0xec);

        *msg = 0;
        strcat(msg, reinterpret_cast<const char *>(g_006845f8));
        reinterpret_cast<ItoaFn>(&_itoa)(fame, numbuf, 10);
        strcat(msg, numbuf);
        strcat(msg, reinterpret_cast<const char *>(g_00684600));
        reinterpret_cast<ItoaFn>(&_itoa)(a1 + 2, numbuf, 10);
        strcat(msg, numbuf);
        strcat(msg, reinterpret_cast<const char *>(g_00684604));
        buf.write_pcx(msg);

        // Records at 0x9A3840 are 0x8C bytes; the original copies
        // record[a1+fame*5-5] over record[a1+fame*5-4] field by field
        // rather than as one block, so the three char[0x18] fields are
        // moved with strncpy and everything else with typed loads/stores.
        int oldIndex = a1 + fame * 5 - 5;
        char *src = reinterpret_cast<char *>(g_009a3840) + oldIndex * 0x8c;
        char *dst = src + 0x8c;

        *reinterpret_cast<uint8_t *>(dst) = *reinterpret_cast<uint8_t *>(src);
        *reinterpret_cast<uint8_t *>(dst + 1) = *reinterpret_cast<uint8_t *>(src + 1);
        *reinterpret_cast<int *>(dst + 4) = *reinterpret_cast<int *>(src + 4);
        *reinterpret_cast<int *>(dst + 8) = *reinterpret_cast<int *>(src + 8);
        *reinterpret_cast<int *>(dst + 0xc) = *reinterpret_cast<int *>(src + 0xc);
        *reinterpret_cast<int *>(dst + 0x10) = *reinterpret_cast<int *>(src + 0x10);
        *reinterpret_cast<int *>(dst + 0x14) = *reinterpret_cast<int *>(src + 0x14);
        *reinterpret_cast<int *>(dst + 0x18) = *reinterpret_cast<int *>(src + 0x18);
        *reinterpret_cast<int *>(dst + 0x1c) = *reinterpret_cast<int *>(src + 0x1c);
        *reinterpret_cast<uint8_t *>(dst + 0x20) = *reinterpret_cast<uint8_t *>(src + 0x20);
        *reinterpret_cast<uint8_t *>(dst + 0x21) = *reinterpret_cast<uint8_t *>(src + 0x21);
        reinterpret_cast<StrncpyFn>(&strncpy)(dst + 0x22, src + 0x22, 0x18);
        reinterpret_cast<StrncpyFn>(&strncpy)(dst + 0x3a, src + 0x3a, 0x18);
        reinterpret_cast<StrncpyFn>(&strncpy)(dst + 0x52, src + 0x52, 0x18);
        for (int k = 0; k < 8; ++k) {
            *reinterpret_cast<int *>(dst + 0x6c + k * 4) =
                *reinterpret_cast<int *>(src + 0x6c + k * 4);
        }
    }
}
