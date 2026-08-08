// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0049FFD0
// name           ?start_attack@ReportWin@@QAEXHHHH@Z
// size           289 bytes
// measured tier  NO_COMPILE
// refusal        u0049ffd0.cpp(215) : error C2664: 'init' : cannot convert parameter 1 from 'void (__cdecl *)(int)' to 'int' This conversion requires a reinterpret_cast, a C-sty
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0049ffd0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?start_attack@ReportWin@@QAEXHHHH@Z  at 0x0049FFD0  (289 bytes)
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
struct ExtDirectDraw;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class ReportWin;
class Spot;
class Time;
typedef unsigned int UINT_PTR;

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
    int copy(Buffer *, int, int, int, int, int, int);
    int init(int, int, int, ExtDirectDraw *);
};
class Time { public:
    int unk_tgl_;
    UINT_PTR id_event_;
    void * callback1_;
    void * callback2_;
    int cb_param2_;
    int cb_param1_;
    uint32_t count_;
    int unk_1_;
    uint32_t resolution_;
    int unk_2_;
    void init(void (__cdecl *)(int), int, int, int);
    void start();
};
extern "C" int __cdecl _ftol();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_004a0250 = (int *)0x004A0250;
static int *const g_00876478 = (int *)0x00876478;

class ReportWin { public:
    void sat_anim();
    void start_attack(int, int, int, int);
};

typedef void (__cdecl *TimeCallback)(int);

void ReportWin::start_attack(int a1, int a2, int a3, int a4) {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int32_t *>(self + 0xF924) = a1;
    *reinterpret_cast<int32_t *>(self + 0xF928) = a2;
    *reinterpret_cast<int32_t *>(self + 0xF92C) = a3;
    *reinterpret_cast<int32_t *>(self + 0xF930) = a4;
    *reinterpret_cast<int32_t *>(self + 0xF920) = 0;
    *reinterpret_cast<int32_t *>(self + 0xF91C) = 0;
    *reinterpret_cast<int32_t *>(self + 0xF934) = 0;
    if (*reinterpret_cast<int32_t *>(self + 0xF918) != 0) {
        int32_t counter = *reinterpret_cast<int32_t *>(self + 0xF918) - 1;
        *reinterpret_cast<int32_t *>(self + 0xF918) = counter;
        int idx = counter % 16;
        if (counter < 0x10) {
            reinterpret_cast<int32_t *>(self + 0xFA78)[idx] = 0;
        }
        sat_anim();
        float *arr_f978 = reinterpret_cast<float *>(self + 0xF978);
        float *arr_f938 = reinterpret_cast<float *>(self + 0xF938);
        int32_t base_e2ac = *reinterpret_cast<int32_t *>(self + 0xE2AC);
        int32_t v_e314 = static_cast<int32_t>(arr_f978[idx]) + base_e2ac - 0x17;
        int32_t base_e2a8 = *reinterpret_cast<int32_t *>(self + 0xE2A8);
        int32_t v_e310 = static_cast<int32_t>(arr_f938[idx]) + base_e2a8 - 0x15;
        *reinterpret_cast<int32_t *>(self + 0xE314) = v_e314;
        *reinterpret_cast<int32_t *>(self + 0xE310) = v_e310;
        *reinterpret_cast<int32_t *>(self + 0xE318) = v_e310 + 0x2b;
        *reinterpret_cast<int32_t *>(self + 0xE31C) = v_e314 + 0x2f;
        Buffer *dest = reinterpret_cast<Buffer *>(self + 0xE320);
        // NOTE: the scaffold's `Buffer::init` declaration is corrupted - see
        // PROPOSALS in the report. It was rewritten to `Time::init`'s
        // signature by a name+arity collision (both take 4 args) in
        // mizuchi_declfix.py, which matches callee declarations by base name
        // without a class qualifier. The cast below matches the CORRUPTED
        // declared type so this still compiles to the same argument bytes.
        dest->init((TimeCallback)0x2b, 0x2f, 0, 0);
        Buffer *panel = reinterpret_cast<Buffer *>(self + 0x444);
        panel->copy(dest, v_e310, v_e314, 0, 0, 0x2b, 0x2f);
        Time *timer = reinterpret_cast<Time *>(self + 0xE2E8);
        timer->init((TimeCallback)g_004a0250,
                    reinterpret_cast<int>(g_00876478), 0x32, 5);
        timer->start();
    }
}
