// ORIGINAL: 0x00476780 ?on_left_click@MonuWin@@QAEXHH@Z 0x00476780-0x00476841 FILE
// RULED-OUT: fixed const-pointer global for 0x94cea0 (indexed table base, address itself scales - own raw cast used). First divergence #1 mov/push (prologue reg alloc), 214 vs 193 B.
// working copy - scaffold materialised by --work
// size      193 bytes
// prototype void (__thiscall ?on_left_click@MonuWin@@QAEXHH@Z)(MonuWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FAB00
// indirect  0x004767D1 0x00476836

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_click@MonuWin@@QAEXHH@Z  at 0x00476780  (193 bytes)
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

class MonuWin;
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
    int check(int, int, int *, int *);
};


// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 62
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093e8c8 = (int *)0x0093E8C8;
static int *const g_0093e8ec = (int *)0x0093E8EC;
static int *const g_0093e944 = (int *)0x0093E944;
static int *const g_0094cea0 = (int *)0x0094CEA0;

class MonuWin { public:
    void on_left_click(int, int);
};
void MonuWin::on_left_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    unsigned char shiftAmt = *reinterpret_cast<unsigned char *>(g_00939284) & 0x1f;
    if ((*reinterpret_cast<unsigned int *>(g_0093e8ec) & (1u << shiftAmt)) != 0) {
        return;
    }
    if (*g_0093e944 != 0) {
        return;
    }
    if (*g_0093e8c8 != 0) {
        return;
    }
    if (*reinterpret_cast<int *>(self + 0xa4c) != 0) {
        *reinterpret_cast<int *>(self + 0xa4c) = 0;
        reinterpret_cast<VCall *>(this)->slot062();
        return;
    }
    int x = a1;
    int y = a2;
    Spot *spot = reinterpret_cast<Spot *>(self + 0x20f0);
    int result = spot->check(a1, a2, &y, &x);
    if (result != -1) {
        if (y == 0x45) {
            y = *reinterpret_cast<int *>(self + 0xa54);
        }
        int idx = *g_00939284 * 0x13d;
        int *table = reinterpret_cast<int *>(0x0094CEA0);
        table[idx] = y;
        *reinterpret_cast<int *>(self + 0xa54) = y;
        *reinterpret_cast<int *>(self + 0xa4c) = 1;
        reinterpret_cast<VCall *>(this)->slot062();
    }
}
