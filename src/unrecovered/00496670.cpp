// ORIGINAL: 0x00496670 ?quayle@@YAXH@Z 0x00496670-0x0049670F FILE
// RULED-OUT: MISMATCH #15 add/mov, table-index lea chain differs from the do/while division idiom form used
// working copy - scaffold materialised by --work
// size      159 bytes
// prototype void (__cdecl ?quayle@@YAXH@Z)(int factionID)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004951A0 0x005ABFF0
// indirect  0x00496705

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?quayle@@YAXH@Z  at 0x00496670  (159 bytes)
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

class QuayleWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
class QuayleWin { public:
    int init();
};

int get_rating(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00872cb0 = (int *)0x00872CB0;
static int *const g_00874df4 = (int *)0x00874DF4;
static int *const g_00874df8 = (int *)0x00874DF8;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0096ea7c = (int *)0x0096EA7C;
extern "C" int g_96ea7c_table[];

// Vtable shim for the QuayleWin singleton at g_00872cb0: slot057 takes
// three ints (all literal 0 at the call site).
class VCall { public:
    virtual void slot000(); virtual void slot001(); virtual void slot002();
    virtual void slot003(); virtual void slot004(); virtual void slot005();
    virtual void slot006(); virtual void slot007(); virtual void slot008();
    virtual void slot009(); virtual void slot010(); virtual void slot011();
    virtual void slot012(); virtual void slot013(); virtual void slot014();
    virtual void slot015(); virtual void slot016(); virtual void slot017();
    virtual void slot018(); virtual void slot019(); virtual void slot020();
    virtual void slot021(); virtual void slot022(); virtual void slot023();
    virtual void slot024(); virtual void slot025(); virtual void slot026();
    virtual void slot027(); virtual void slot028(); virtual void slot029();
    virtual void slot030(); virtual void slot031(); virtual void slot032();
    virtual void slot033(); virtual void slot034(); virtual void slot035();
    virtual void slot036(); virtual void slot037(); virtual void slot038();
    virtual void slot039(); virtual void slot040(); virtual void slot041();
    virtual void slot042(); virtual void slot043(); virtual void slot044();
    virtual void slot045(); virtual void slot046(); virtual void slot047();
    virtual void slot048(); virtual void slot049(); virtual void slot050();
    virtual void slot051(); virtual void slot052(); virtual void slot053();
    virtual void slot054(); virtual void slot055(); virtual void slot056();
    virtual void slot057(int, int, int);
};

void __cdecl quayle(int a1) {
    int rating = get_rating(a1, g_96ea7c_table[a1 * 2099]);
    int count = 0;
    int n = 2;
    do {
        if (rating < (n * n) / 2) {
            break;
        }
        count = count + 1;
        n = n + 1;
    } while (n - 2 < 0x18);

    if (*g_0093f660 != 0) {
        return;
    }

    QuayleWin *win = (QuayleWin *)g_00872cb0;
    if (win->init() != 0) {
        return;
    }

    if (count < 0x19) {
        if (count < -1) {
            count = -1;
        }
    } else {
        count = 0x18;
    }

    *g_00874df8 = count;
    *g_00874df4 = -1;
    ((VCall *)g_00872cb0)->slot057(0, 0, 0);
}
