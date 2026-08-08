// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004B4A20
// name           ?set_title@StatusWin@@QAEXH@Z
// size           451 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004b4a20/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_title@StatusWin@@QAEXH@Z  at 0x004B4A20  (451 bytes)
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
class StatusWin;
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
extern "C" char *strcat(char *, const char *);
void __cdecl say_loc(char *, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_00939288 = (int *)0x00939288;
static int *const g_0093928c = (int *)0x0093928C;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009ab868 = (int *)0x009AB868;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class StatusWin { public:
    void set_title(int);
};

// Reached only through a computed index (record * 0x34), never a bare
// [imm32] - the indexed-table-base lever, `extern T name[]` over a
// pointer constant. Same table family as 0x0041A620's finding.
extern char g_00952828[];
extern char g_0095282a[];
extern char g_00952832[];

void StatusWin::set_title(int a1) {
    char *self = reinterpret_cast<char *>(this);
    unsigned int flags = static_cast<unsigned int>(a1);
    *reinterpret_cast<char *>(g_009b86a0) = 0;

    if (flags != 0) {
        int field;
        if (flags & 1) {
            field = *reinterpret_cast<int *>(*g_009b90f8 + 0x6b4);
        } else if (flags & 0x10) {
            field = *reinterpret_cast<int *>(*g_009b90f8 + 0x6f8);
        } else if (flags & 8) {
            field = *reinterpret_cast<int *>(*g_009b90f8 + 0x6fc);
        } else if (flags & 2) {
            field = *reinterpret_cast<int *>(*g_009b90f8 + 0x6b8);
        } else {
            field = *reinterpret_cast<int *>(*g_009b90f8 + 0x250);
        }
        int str = reinterpret_cast<Strings *>(g_009b90d8)->get(field);
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<const char *>(str));
        return;
    }

    int str = reinterpret_cast<Strings *>(g_009b90d8)->get(
        *reinterpret_cast<int *>(*g_009b90f8 + 0x8c));
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<const char *>(str));

    if (*g_009a64d4 != 0) {
        strcat(reinterpret_cast<char *>(g_009b86a0),
               reinterpret_cast<const char *>(g_00682e94));

        int x, y;
        if (*g_00939288 == 0) {
            str = reinterpret_cast<Strings *>(g_009b90d8)->get(
                *reinterpret_cast<int *>(*g_009b90f8 + 0xc));
            strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<const char *>(str));
            x = *reinterpret_cast<int *>(self + 0x15b4);
            y = *reinterpret_cast<int *>(self + 0x15b8);
        } else {
            str = reinterpret_cast<Strings *>(g_009b90d8)->get(
                *reinterpret_cast<int *>(*g_009b90f8 + 8));
            strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<const char *>(str));
            if (*g_0093928c < 0) {
                return;
            }
            strcat(reinterpret_cast<char *>(g_009b86a0),
                   reinterpret_cast<const char *>(g_00682820));

            int recordIndex = *g_0093928c;
            int byteOffset = recordIndex * 0x34;
            x = *reinterpret_cast<short *>(g_00952828 + byteOffset);
            y = *reinterpret_cast<short *>(g_0095282a + byteOffset);
            int nameIdx = *reinterpret_cast<short *>(g_00952832 + byteOffset);
            strcat(reinterpret_cast<char *>(g_009b86a0),
                   reinterpret_cast<const char *>(reinterpret_cast<char *>(g_009ab868) +
                                                    nameIdx * 0x34));
            strcat(reinterpret_cast<char *>(g_009b86a0),
                   reinterpret_cast<const char *>(g_00682820));
        }
        strcat(reinterpret_cast<char *>(g_009b86a0),
               reinterpret_cast<const char *>(g_00682820));
        say_loc(reinterpret_cast<char *>(g_009b86a0), x, y, *g_00939288,
                *g_00939288 == 0, *g_00939288);
    }
}
