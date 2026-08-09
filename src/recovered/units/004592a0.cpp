// ORIGINAL: 0x004592A0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004592A0
// name           ?init@InfoWin@@QAEXXZ
// size           372 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004592a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@InfoWin@@QAEXXZ  at 0x004592A0  (372 bytes)
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

typedef int BOOL;
class Font;
typedef void * HFONT;
class InfoWin;
typedef char * LPSTR;
class Time;
typedef unsigned int UINT_PTR;

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
    int init(char *, int, int);
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
    void start(void (__cdecl *)(int), int, int, int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00459260 = (int *)0x00459260;
static int *const g_00669328 = (int *)0x00669328;
static int *const g_00691b2c = (int *)0x00691B2C;
static int *const g_007af5c0 = (int *)0x007AF5C0;
static int *const g_007af5c4 = (int *)0x007AF5C4;
static int *const g_007af5c8 = (int *)0x007AF5C8;
static int *const g_007af5cc = (int *)0x007AF5CC;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00939e5c = (int *)0x00939E5C;
static int *const g_00939ed8 = (int *)0x00939ED8;
static int *const g_00939ef8 = (int *)0x00939EF8;
static int *const g_00939efc = (int *)0x00939EFC;
static int *const g_00939f00 = (int *)0x00939F00;
static int *const g_00939f04 = (int *)0x00939F04;
static int *const g_009b7b1c = (int *)0x009B7B1C;

class InfoWin { public:
    void init();
};

// Fixed table indexed by a raw byte offset, not a scaled element index -
// `extern char[]` reproduces the unscaled `[reg + 0x915b78]` addressing;
// an `int*` constant would fold the address away under /O2.
extern char g_00915b78[];

struct Rect {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};

typedef int(__stdcall *UnionRectFn)(Rect *, const Rect *, const Rect *);

void InfoWin::init() {
    char *const self = reinterpret_cast<char *>(this);

    *reinterpret_cast<int32_t *>(self + 0x88) = -1;
    *reinterpret_cast<int32_t *>(self + 0x8c) = -1;
    *reinterpret_cast<int32_t *>(self + 0x90) = -1;
    *reinterpret_cast<int32_t *>(self + 0xb4) = -1;
    *reinterpret_cast<int32_t *>(self + 0xac) = 0;
    *reinterpret_cast<int32_t *>(self + 0x94) = 0;
    *reinterpret_cast<int32_t *>(self + 0xa8) = 0;
    *reinterpret_cast<int32_t *>(self + 0x84) = 1;
    *reinterpret_cast<int32_t *>(self + 0x80) = 1;

    if ((*g_00939ed8 & 4) == 0) {
        int32_t base_size, extra;
        if (*g_00939e5c < 1000) {
            base_size = 0xc8;
            extra = 0xb4;
        } else {
            base_size = 0xfa;
            extra = 0xd4;
        }

        int32_t table_val = *reinterpret_cast<int32_t *>(
            g_00915b78 + (*reinterpret_cast<int **>(g_009156b0))[1]);
        *g_00939ef8 = 0;
        *g_00939f00 = base_size;
        *g_00939efc = -table_val;
        *g_00939f04 = -table_val + extra;
        *g_00939ed8 |= 4;
    }

    Font *const font = reinterpret_cast<Font *>(self + 0x58);
    if (*g_009b7b1c == 0x320) {
        font->init(*reinterpret_cast<char **>(g_00691b2c), 0xa, 0);
    } else {
        font->init(*reinterpret_cast<char **>(g_00691b2c), 0xc, 0);
    }
    font->unk_1_ = 4;

    Rect *const r1 = reinterpret_cast<Rect *>(self + 0x10);
    Rect *const r2 = reinterpret_cast<Rect *>(self + 0x20);
    int32_t left_val = *g_007af5c0 + 7;
    int32_t top_val = *g_007af5c4 + 7;
    int32_t right_val = *g_007af5c8 - 0x27;
    int32_t bottom_val = *g_007af5cc - 7;

    if (*g_009b7b1c != 0x320) {
        right_val = (right_val - left_val - 3) / 2 + left_val;
        r1->left = left_val;
        r1->top = top_val;
        r1->right = right_val;
        r1->bottom = bottom_val;
        int32_t width2 = right_val - left_val + 3;
        left_val += width2;
        right_val += width2;
    } else {
        r1->left = left_val;
        r1->top = top_val;
        r1->right = right_val;
        r1->bottom = bottom_val;
    }
    r2->left = left_val;
    r2->top = top_val;
    r2->right = right_val;
    r2->bottom = bottom_val;

    UnionRectFn const union_rect = *reinterpret_cast<UnionRectFn *>(g_00669328);
    union_rect(reinterpret_cast<Rect *>(self), r1, r2);

    reinterpret_cast<Time *>(self + 0x30)->start(
        reinterpret_cast<void(__cdecl *)(int)>(g_00459260), 0xa, 0x32, 0x32);
}
