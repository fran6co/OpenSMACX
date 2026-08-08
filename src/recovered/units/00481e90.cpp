// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00481E90
// name           ?config_players@NetWin@@QAEXXZ
// size           313 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00481e90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?config_players@NetWin@@QAEXXZ  at 0x00481E90  (313 bytes)
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
typedef void * LPVOID;
class NetWin;
class Strings;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Strings { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    BOOL is_populated_;
    int get(int);
};
extern "C" char *strcpy(char *, const char *);
#pragma function(strcpy)

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090dd15 = (int *)0x0090DD15;
static int *const g_0090e778 = (int *)0x0090E778;
static int *const g_0090e779 = (int *)0x0090E779;
static int *const g_0090e79c = (int *)0x0090E79C;
static int *const g_0090e8e0 = (int *)0x0090E8E0;
static int *const g_0090e8e1 = (int *)0x0090E8E1;
static int *const g_0090e8e2 = (int *)0x0090E8E2;
static int *const g_0090e8e4 = (int *)0x0090E8E4;
static int *const g_0090e8e5 = (int *)0x0090E8E5;
static int *const g_0090e8e6 = (int *)0x0090E8E6;
static int *const g_0090e8e7 = (int *)0x0090E8E7;
static int *const g_0090e8e8 = (int *)0x0090E8E8;
static int *const g_0090e8e9 = (int *)0x0090E8E9;
static int *const g_0090e8ea = (int *)0x0090E8EA;
static int *const g_0090e8eb = (int *)0x0090E8EB;
static int *const g_0093d4b0 = (int *)0x0093D4B0;
static int *const g_00947370 = (int *)0x00947370;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class NetWin { public:
    void config_players();
};

void NetWin::config_players() {
    uint8_t al_val = *reinterpret_cast<uint8_t *>(g_0093d4b0);
    uint8_t bl_val = 0xFF;

    *reinterpret_cast<int *>(g_0090e778) = 0;
    *reinterpret_cast<uint8_t *>(g_0090e79c) = 0;
    *reinterpret_cast<uint8_t *>(g_0090e8e0) = 0xFF;
    *reinterpret_cast<uint8_t *>(g_0090e8e4) = 0;
    *reinterpret_cast<uint8_t *>(g_0090e8e5) = 0;
    *reinterpret_cast<uint8_t *>(g_0090e8eb) = bl_val;
    *reinterpret_cast<uint8_t *>(g_0090e8e1) = al_val;
    *reinterpret_cast<uint8_t *>(g_0090e8e2) = 2;
    *reinterpret_cast<uint8_t *>(g_0090e8e6) = 2;
    *reinterpret_cast<uint8_t *>(g_0090e8e7) = 2;
    *reinterpret_cast<uint8_t *>(g_0090e8e8) = 2;
    *reinterpret_cast<uint8_t *>(g_0090e8e9) = 2;
    *reinterpret_cast<uint8_t *>(g_0090e8ea) = 2;

    int edi = reinterpret_cast<int>(g_00947370);
    int esi = reinterpret_cast<int>(g_0090dd15);
    do {
        *reinterpret_cast<int *>(esi + 0x177) = 0;
        *reinterpret_cast<uint8_t *>(esi - 1) = bl_val;
        *reinterpret_cast<uint8_t *>(esi) = 0;
        *reinterpret_cast<uint8_t *>(esi + 1) = 0;
        *reinterpret_cast<uint8_t *>(esi + 2) = bl_val;
        *reinterpret_cast<uint8_t *>(esi + 3) = 2;

        uint8_t cl_val = *reinterpret_cast<uint8_t *>(g_0090e8e2);
        *reinterpret_cast<uint8_t *>(esi - 1) = bl_val;
        *reinterpret_cast<uint8_t *>(esi + 2) = bl_val;
        *reinterpret_cast<uint8_t *>(esi + 1) = cl_val;
        *reinterpret_cast<uint8_t *>(esi) = 0;

        strcpy(reinterpret_cast<char *>(esi + 4), reinterpret_cast<char *>(edi - 0x350));
        strcpy(reinterpret_cast<char *>(esi + 0x1c), reinterpret_cast<char *>(edi));

        int *strings_ptr = *reinterpret_cast<int **>(g_009b90f8);
        int name_id = *reinterpret_cast<int *>(reinterpret_cast<char *>(strings_ptr) + 0xFE4);
        int str_ptr = reinterpret_cast<Strings *>(g_009b90d8)->get(name_id);
        strcpy(reinterpret_cast<char *>(esi + 0x44), reinterpret_cast<char *>(str_ptr));

        strcpy(reinterpret_cast<char *>(esi + 0x5c), reinterpret_cast<char *>(edi - 0x80));
        strcpy(reinterpret_cast<char *>(esi + 0xdc), reinterpret_cast<char *>(edi - 0x338));
        strcpy(reinterpret_cast<char *>(esi + 0xf4), reinterpret_cast<char *>(edi - 0x320));

        *reinterpret_cast<uint8_t *>(esi + 0x174) = *reinterpret_cast<uint8_t *>(edi - 0x384);

        esi += 0x17c;
        edi += 0x59c;
    } while (esi < reinterpret_cast<int>(g_0090e779));
}
