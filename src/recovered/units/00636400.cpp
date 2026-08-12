// ORIGINAL: 0x00636400 FILE
// name      ??0Video@@QAE@XZ
// size      359 bytes
// spans     0x00636400-0x0063652E;0x00663890-0x006638C9
// prototype void (__thiscall ??0Video@@QAE@XZ)(Video* this)
// callers   0   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8620 0x005D7210 0x005FE2A0 0x00626230 0x0063E520 0x006465F0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00636400
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00636400/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0Video@@QAE@XZ  at 0x00636400  (359 bytes)
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
class Palette;
struct RECT;
class Spot;
class Video;

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
};
class Palette { public:
    Palette();
};
extern "C" int __cdecl sub_4c8620();
extern "C" int __cdecl sub_63e520();
extern "C" void *memset(void *, int, unsigned int);
void __cdecl log_reset();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006638bf = (int *)0x006638BF;
static int *const g_006803f8 = (int *)0x006803F8;

class Video { public:
    Video();
};

// Vtable-free thiscall shim: cast a nullary __cdecl function's address to a
// nullary member-function pointer so the call carries `this` in ecx without
// VC6's C4234 on a free `__thiscall` pointer. Precedent: build/byte-match/00406670.
class __ShimThis {};
typedef int (__ShimThis::*Nullary)();
union NullaryCaster {
    int (__cdecl *free_fn)();
    Nullary member_fn;
};

// Placement new, normally from <new>; declared locally since this unit
// avoids standard headers.
inline void *__cdecl operator new(unsigned int, void *p) throw() { return p; }

// MISMATCH (#3: original 'push' vs rebuilt 'call' - the SEH frame prologue
// `push -1; push &funclet; mov eax,fs:0; push eax; mov fs:0,esp` never
// appears). The original installs a frame-based EH handler because Palette
// and Buffer are true subobjects: if either constructor throws, the funclets
// at 0x663898/0x6638a3 destroy whatever was already constructed. Placement
// new onto a `this`-relative offset does not make the compiler treat the
// target as an owned subobject needing that unwind bookkeeping - confirmed
// here and matching the same shape already recorded MISMATCH in
// build/byte-match/00404010, 005ffd80, 00629110 and 00633750, all
// constructors placement-constructing a subobject at a `this + offset`
// address. Reproducing the frame would need Video to hold Palette/Buffer as
// REAL by-value members at the true offsets, which needs their true sizes;
// both are opaque shells here (no committed layout), so the offsets cannot
// be hit by real member declarations without guessing padding. Not
// attempted further - a layout question, not a source-form one.
Video::Video() {
    char *self = reinterpret_cast<char *>(this);

    NullaryCaster caster;
    caster.free_fn = &sub_4c8620;
    (reinterpret_cast<__ShimThis *>(self)->*caster.member_fn)();

    new (self + 0x54) Palette();

    caster.free_fn = &sub_63e520;
    (reinterpret_cast<__ShimThis *>(self + 0xa5c)->*caster.member_fn)();

    new (self + 0xa6c) Buffer();

    log_reset();

    *reinterpret_cast<int *>(self + 0x1018) = 0;
    *reinterpret_cast<int *>(self + 0x101c) = 0;
    *reinterpret_cast<int *>(self + 0x1020) = 0;
    *reinterpret_cast<int *>(self + 0x1024) = 0;
    *reinterpret_cast<int *>(self + 0x4a8) = 0;
    *reinterpret_cast<int *>(self + 0x1010) = 0;
    *reinterpret_cast<int *>(self + 0x1014) = 0;
    *reinterpret_cast<int *>(self + 0x4ac) = 0;
    *reinterpret_cast<int *>(self + 0xff8) = 0;
    *reinterpret_cast<int *>(self + 0x1000) = 0;
    *reinterpret_cast<int *>(self + 0x1004) = 0;
    *reinterpret_cast<int *>(self + 0x4b0) = 0;
    *reinterpret_cast<int *>(self + 0x598) = 0;
    *reinterpret_cast<int *>(self + 0x59c) = 0;
    *reinterpret_cast<int *>(self + 0x4dc) = 0;
    memset(self + 0x4c8, 0, 0x14);
    memset(self + 0x4b4, 0, 0x14);
    memset(self + 0x9dc, 0, 0x54);
    memset(self + 0xa30, 0, 0x2c);
    *reinterpret_cast<int *>(self + 0x1008) = 0;
    *reinterpret_cast<int *>(self + 0x100c) = 0;
    *reinterpret_cast<int *>(self + 0xff4) = 0;
    *reinterpret_cast<int *>(self + 0xffc) = 0;
    memset(self + 0x1028, 0, 0x10);
}
