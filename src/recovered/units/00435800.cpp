// ORIGINAL: 0x00435800 FILE
// name      ?draw_reactor@DesignWin@@QAEXH@Z
// size      809 bytes
// spans     0x00435800-0x00435B29
// prototype void (__thiscall ?draw_reactor@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   4   call targets   16
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C3800 0x005D8000 0x005D95E0 0x005DAC70 0x005DACB0 0x005DB720 0x005DC360 0x005DD130 0x005DDAB0 0x006169A0 0x00617230 0x00618370 0x006183B0 0x006453E0 0x00645470 0x0064FC88
// indirect  0x004358C2 0x0043598E
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00435800
// measured tier  NO_COMPILE
// refusal        u00435800.cpp(1) : error C2653: 'DesignWin' : is not a class or namespace name
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00435800/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_reactor@DesignWin@@QAEXH@Z  at 0x00435800  (809 bytes)
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
class DesignWin;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;
class Strings;

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
    int copy(Buffer *, RECT *);
    int set_clip(RECT *);
    int set_font(Font *, Font *, Font *, Font *);
    int wrap_cent(char *, RECT *);
    int wrap_height(char *, int);
    int write_cent_l(Font *, char *, RECT *, int);
    int write_cent_l(char *, RECT *, int);
    void set_text_color(int, int, int, int);
};
class Caviar { public:
    float scene_scale_;
    uint8_t unmapped_04_[0x2C - 0x04];
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint8_t unmapped_38_[0x108 - 0x38];
    int32_t field_108_;
    uint8_t unmapped_10C_[0x13D0 - 0x10C];
    int render(Buffer *, int, int, int);
    void set_scaling(float);
    void set_scene_rotation(float, float, float);
};
class Strings { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    BOOL is_populated_;
    int get(int);
};
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
extern "C" unsigned int strlen(const char *);
int __cdecl vehdraw_construct_reactor(int, int, int, int, int, Caviar *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_008c6dcc = (int *)0x008C6DCC;
static int *const g_008c6dd0 = (int *)0x008C6DD0;
static int *const g_008c6dd4 = (int *)0x008C6DD4;
static int *const g_008c6dd8 = (int *)0x008C6DD8;
static int *const g_008c6ddc = (int *)0x008C6DDC;
static int *const g_008c6de0 = (int *)0x008C6DE0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_009527ec = (int *)0x009527EC;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DesignWin { public:
    uint8_t pad_0_[0x12764];
    Spot spot_;

    void draw_reactor(int);
};

typedef char *(__stdcall *CharUpperFn)(char *);
typedef char *(__cdecl *ItoaFn)(int, char *, int);

void DesignWin::draw_reactor(int a1) {
    char *self = reinterpret_cast<char *>(this);
    char *strbuf = reinterpret_cast<char *>(g_009b86a0);
    CharUpperFn charUpper = *reinterpret_cast<CharUpperFn *>(g_0066931c);

    RECT rectA;
    rectA.left = *reinterpret_cast<int32_t *>(self + 0xadc);
    rectA.top = *reinterpret_cast<int32_t *>(self + 0xae0);
    rectA.right = *reinterpret_cast<int32_t *>(self + 0xae4);
    rectA.bottom = *reinterpret_cast<int32_t *>(self + 0xae8);

    Buffer &buf = *reinterpret_cast<Buffer *>(self + 0x444);

    buf.set_clip(&rectA);
    reinterpret_cast<Buffer *>(self + 0x13c6c)->copy(this ? &buf : 0, &rectA);

    RECT rectB;
    rectB.left = rectA.left;
    rectB.right = rectA.right;
    rectB.top = rectA.top;
    if (*reinterpret_cast<int32_t *>(self + 0x14568) < 0) {
        rectB.bottom = *reinterpret_cast<int32_t *>(self + 0x14574);
    } else {
        rectB.bottom = *reinterpret_cast<int32_t *>(self + 0x14578) +
                        *reinterpret_cast<int32_t *>(self + 0x14568);
    }
    rectB.bottom = rectB.bottom + rectA.top;
    *strbuf = 0;
    rectA.top = rectB.bottom;

    int32_t uVar1 = reinterpret_cast<Strings *>(0x9b90d8)->get(
        *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(g_009b90f8) + 0x13c));
    strcat(strbuf, reinterpret_cast<char *>(uVar1));
    charUpper(strbuf);
    buf.set_text_color(*g_008c6dcc, *g_008c6dd0, 1, 1);
    unsigned int sVar2 = strlen(strbuf);
    buf.write_cent_l(reinterpret_cast<Font *>(self + 0x14568), strbuf, &rectB,
                      static_cast<int>(sVar2));
    buf.set_font(reinterpret_cast<Font *>(self + 0x14540), 0, 0, 0);
    *strbuf = 0;

    if (*reinterpret_cast<uint32_t *>(self + 0x14208) & 0x8000) {
        uVar1 = reinterpret_cast<Strings *>(0x9b90d8)->get(
            *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(g_009b90f8) + 0x584));
        strcat(strbuf, reinterpret_cast<char *>(uVar1));
        strcat(strbuf, reinterpret_cast<char *>(g_00682820));
    }

    uVar1 = reinterpret_cast<Strings *>(0x9b90d8)->get(
        g_009527ec[*reinterpret_cast<int32_t *>(self + 0x1420c) * 3]);
    strcat(strbuf, reinterpret_cast<char *>(uVar1));
    charUpper(strbuf);

    int32_t iVar3 = buf.wrap_height(strbuf, rectA.right - rectA.left);
    *reinterpret_cast<int32_t *>(self + 0x13b68) = 0;
    rectA.bottom = rectA.bottom + ((-5 - iVar3) - *reinterpret_cast<int32_t *>(self + 0x14550));

    vehdraw_construct_reactor(*g_00939284, *reinterpret_cast<int32_t *>(self + 0x141fc),
                               *reinterpret_cast<int32_t *>(self + 0x14200),
                               *reinterpret_cast<int32_t *>(self + 0x1420c),
                               *reinterpret_cast<int32_t *>(self + 0x14208),
                               reinterpret_cast<Caviar *>(self + 0x1279c));

    reinterpret_cast<Caviar *>(self + 0x1279c)->set_scaling(0.2f);
    reinterpret_cast<Caviar *>(self + 0x1279c)->set_scene_rotation(0.523333371f, 0, 0);
    reinterpret_cast<Caviar *>(self + 0x1279c)
        ->render(this ? &buf : 0, (rectA.right + rectA.left) / 2,
                 (rectA.bottom + rectA.top) / 2, 6);

    rectA.top = rectA.bottom;
    rectA.bottom = rectA.bottom + *reinterpret_cast<int32_t *>(self + 0x14550);
    buf.set_text_color(*g_008c6dd4, *g_008c6dd8, 1, 1);

    rectA.top = buf.wrap_cent(strbuf, &rectA);
    rectA.bottom = rectA.top + *reinterpret_cast<int32_t *>(self + 0x14550);
    buf.set_text_color(*g_008c6ddc, *g_008c6de0, 1, 1);
    *strbuf = 0;

    uVar1 = reinterpret_cast<Strings *>(0x9b90d8)->get(
        *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(g_009b90f8) + 0x1e4));
    strcat(strbuf, reinterpret_cast<char *>(uVar1));
    strcat(strbuf, reinterpret_cast<char *>(g_00682820));

    char local_74[80];
    reinterpret_cast<ItoaFn>(_itoa)(*reinterpret_cast<int32_t *>(self + 0x1420c), local_74, 10);
    strcat(strbuf, local_74);

    unsigned int sVar2b = strlen(strbuf);
    buf.write_cent_l(strbuf, &rectA, static_cast<int>(sVar2b));
}
