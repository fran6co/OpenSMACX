// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00454D10
// name           ?load_misc@@YAXXZ
// size           678 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00454d10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load_misc@@YAXXZ  at 0x00454D10  (678 bytes)
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

class Sprite;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Sprite { public:
    int ppszFileName_;
    int pcBits_;
    char cTransparentIndex_;
    char pad1_;
    char pad2_;
    char pad3_;
    int iSpriteWidth2_;
    int iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_;
    int iHeight_;
    int iLeftOffset_;
    int iTopOffset_;
    int fObj1Exists_;
    int init(char *, int, int);
};
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00685090 = (int *)0x00685090;
static int *const g_0068509c = (int *)0x0068509C;
static int *const g_006850a0 = (int *)0x006850A0;
static int *const g_006850a8 = (int *)0x006850A8;
static int *const g_006850ac = (int *)0x006850AC;
static int *const g_006850b4 = (int *)0x006850B4;
static int *const g_006850b8 = (int *)0x006850B8;
static int *const g_006850c4 = (int *)0x006850C4;
static int *const g_006850c8 = (int *)0x006850C8;
static int *const g_006850d4 = (int *)0x006850D4;
static int *const g_006850e0 = (int *)0x006850E0;
static int *const g_006850ec = (int *)0x006850EC;
static int *const g_006850fc = (int *)0x006850FC;
static int *const g_0068510c = (int *)0x0068510C;
static int *const g_00759338 = (int *)0x00759338;
static int *const g_007594f0 = (int *)0x007594F0;
static int *const g_00759e38 = (int *)0x00759E38;
static int *const g_00759e40 = (int *)0x00759E40;
static int *const g_00759ff8 = (int *)0x00759FF8;
static int *const g_0075ad8c = (int *)0x0075AD8C;
static int *const g_0076dc30 = (int *)0x0076DC30;
static int *const g_0076dc48 = (int *)0x0076DC48;
static int *const g_0076dc4c = (int *)0x0076DC4C;
static int *const g_00779200 = (int *)0x00779200;
static int *const g_00779218 = (int *)0x00779218;
static int *const g_0077921c = (int *)0x0077921C;
static int *const g_00779570 = (int *)0x00779570;
static int *const g_00779588 = (int *)0x00779588;
static int *const g_0077958c = (int *)0x0077958C;
static int *const g_00787218 = (int *)0x00787218;
static int *const g_007873d0 = (int *)0x007873D0;
static int *const g_007877f0 = (int *)0x007877F0;
static int *const g_00787d18 = (int *)0x00787D18;
static int *const g_00787e20 = (int *)0x00787E20;
static int *const g_00787e70 = (int *)0x00787E70;
static int *const g_00787e88 = (int *)0x00787E88;
static int *const g_00787e8c = (int *)0x00787E8C;
static int *const g_0078a4a0 = (int *)0x0078A4A0;
static int *const g_0078a4b8 = (int *)0x0078A4B8;
static int *const g_0078a4bc = (int *)0x0078A4BC;
static int *const g_007ac000 = (int *)0x007AC000;
static int *const g_007ac018 = (int *)0x007AC018;
static int *const g_007ac01c = (int *)0x007AC01C;
static int *const g_009b86a0 = (int *)0x009B86A0;

// The emitter's scaffold guessed `_itoa` as nullary (arity unknown to it);
// the real CRT `_itoa` takes (value, buffer, radix). A function-pointer
// cast calls it correctly without redeclaring the extern "C" symbol.
typedef char *(__cdecl *ItoaFn)(int, char *, int);

void __cdecl load_misc() {
    ItoaFn itoa_fn = reinterpret_cast<ItoaFn>(&_itoa);
    char local_a4[80];
    char local_54[80];
    char *msgBuf = reinterpret_cast<char *>(g_009b86a0);

    int i = 0;
    char *p = reinterpret_cast<char *>(g_00759e40);
    do {
        *msgBuf = 0;
        strcat(msgBuf, reinterpret_cast<char *>(g_00685090));
        if (p < reinterpret_cast<char *>(g_00759ff8)) {
            strcat(msgBuf, reinterpret_cast<char *>(g_0068509c));
        }
        itoa_fn(i, local_54, 10);
        strcat(msgBuf, local_54);
        reinterpret_cast<Sprite *>(p - 0x18)->init(msgBuf, 0, 0);
        *reinterpret_cast<int *>(p) = 0xd7;
        *reinterpret_cast<int *>(p + 4) = 0xd7;
        p += 0x2c;
        ++i;
    } while (p < reinterpret_cast<char *>(g_0075ad8c));

    i = 0;
    p = reinterpret_cast<char *>(g_00787218);
    do {
        *msgBuf = 0;
        strcat(msgBuf, reinterpret_cast<char *>(g_006850a0));
        if (p >= reinterpret_cast<char *>(g_007877f0) && p <= reinterpret_cast<char *>(g_00787d18)) {
            strcat(msgBuf, reinterpret_cast<char *>(g_006850a8));
        }
        strcat(msgBuf, reinterpret_cast<char *>(g_006850ac));
        if (p < reinterpret_cast<char *>(g_007873d0)) {
            strcat(msgBuf, reinterpret_cast<char *>(g_006850b4));
        }
        if (p > reinterpret_cast<char *>(g_00787d18)) {
            itoa_fn(i - 0x1f, local_54, 10);
            strcat(msgBuf, local_54);
        } else {
            itoa_fn(i, local_a4, 10);
            strcat(msgBuf, local_a4);
        }
        reinterpret_cast<Sprite *>(p - 0x18)->init(msgBuf, 0, 0);
        *reinterpret_cast<int *>(p) = 0xd7;
        *reinterpret_cast<int *>(p + 4) = 0xd7;
        p += 0x2c;
        ++i;
    } while (p < reinterpret_cast<char *>(g_00787e20));

    i = 0;
    p = reinterpret_cast<char *>(g_00759338);
    do {
        *msgBuf = 0;
        strcat(msgBuf, reinterpret_cast<char *>(g_006850b8));
        if (p < reinterpret_cast<char *>(g_007594f0)) {
            strcat(msgBuf, reinterpret_cast<char *>(g_006850c4));
        }
        itoa_fn(i, local_a4, 10);
        strcat(msgBuf, local_a4);
        reinterpret_cast<Sprite *>(p - 0x18)->init(msgBuf, 0, 0);
        *reinterpret_cast<int *>(p) = 0xd7;
        *reinterpret_cast<int *>(p + 4) = 0xd7;
        p += 0x2c;
        ++i;
    } while (p < reinterpret_cast<char *>(g_00759e38));

    reinterpret_cast<Sprite *>(g_00779200)->init(reinterpret_cast<char *>(g_006850c8), 0, 0);
    *reinterpret_cast<int *>(g_00779218) = 0x60;
    *reinterpret_cast<int *>(g_0077921c) = 0x73;

    reinterpret_cast<Sprite *>(g_0076dc30)->init(reinterpret_cast<char *>(g_006850d4), 0, 0);
    *reinterpret_cast<int *>(g_0076dc48) = 0x60;
    *reinterpret_cast<int *>(g_0076dc4c) = 0x73;

    reinterpret_cast<Sprite *>(g_0078a4a0)->init(reinterpret_cast<char *>(g_006850e0), 0, 0);
    *reinterpret_cast<int *>(g_0078a4b8) = 0x60;
    *reinterpret_cast<int *>(g_0078a4bc) = 0x73;

    reinterpret_cast<Sprite *>(g_00787e70)->init(reinterpret_cast<char *>(g_006850ec), 0, 0);
    *reinterpret_cast<int *>(g_00787e88) = 0x60;
    *reinterpret_cast<int *>(g_00787e8c) = 0x73;

    reinterpret_cast<Sprite *>(g_007ac000)->init(reinterpret_cast<char *>(g_006850fc), 0, 0);
    *reinterpret_cast<int *>(g_007ac018) = 0x60;
    *reinterpret_cast<int *>(g_007ac01c) = 0x73;

    reinterpret_cast<Sprite *>(g_00779570)->init(reinterpret_cast<char *>(g_0068510c), 0, 0);
    *reinterpret_cast<int *>(g_00779588) = 0x60;
    *reinterpret_cast<int *>(g_0077958c) = 0x73;
}
