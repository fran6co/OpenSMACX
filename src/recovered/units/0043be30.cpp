// ORIGINAL: 0x0043BE30 FILE
// name      ?select_name@DesignWin@@QAEXXZ
// size      284 bytes
// spans     0x0043BE30-0x0043BF4C
// prototype void (__thiscall ?select_name@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   1   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00436090 0x0057E040 0x005BFAB0 0x006453E0 0x00645460 0x00645660 0x00645E90
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0043BE30
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043be30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?select_name@DesignWin@@QAEXXZ  at 0x0043BE30  (284 bytes)
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

class DesignWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" char *strcpy(char *, const char *);
extern "C" int __cdecl strncpy();
extern "C" int strcmp(const char *, const char *);
extern "C" unsigned int strlen(const char *);
int __cdecl X_pop_ask(const char *, int, char *, int (__cdecl *)(), int);
void __cdecl name_proto(char *, int, int, int, int, int, int, int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 63
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
    virtual void slot062();
    virtual void slot063();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006838f4 = (int *)0x006838F4;
static int *const g_009ab898 = (int *)0x009AB898;
static int *const g_009bb5e8 = (int *)0x009BB5E8;

class DesignWin { public:
    void draw_vehicle(int);
    void select_name();
};

// The address itself does work here (indexed by idx*0x34, not just
// dereferenced once), so the table is declared as an extern array rather
// than reusing the scaffold's constant pointer.
extern unsigned char g_009ab898_tbl[];

// The scaffold's zero-arg placeholder for `strncpy` cannot be redeclared
// with the real arity in the same unit (C2733), so the call goes through
// a locally-typed function pointer onto the same extern "C" symbol - the
// same lever 0042a0b0.cpp uses for `atoi`.
typedef char *(__cdecl *StrncpyFn)(char *, const char *, unsigned int);

void DesignWin::select_name() {
    char *self = reinterpret_cast<char *>(this);
    char name[0x50];
    char *stored_name = self + 0x14220;

    strcpy(name, stored_name);
    if (strlen(name) == 0) {
        name_proto(name,
                   *reinterpret_cast<int *>(self + 0x141f8),
                   *reinterpret_cast<int *>(self + 0x141f4),
                   *reinterpret_cast<int *>(self + 0x141fc),
                   *reinterpret_cast<int *>(self + 0x14200),
                   *reinterpret_cast<int *>(self + 0x14204),
                   *reinterpret_cast<int *>(self + 0x14208),
                   *reinterpret_cast<int *>(self + 0x1420c));
        int idx = *reinterpret_cast<int *>(self + 0x141f8);
        g_009ab898_tbl[idx * 0x34] &= 0xfd;
    }

    name[31] = 0;
    int result = X_pop_ask(reinterpret_cast<const char *>(g_006838f4), 0x1f,
                           name, 0, 0);
    if (result == 0) {
        if (strlen(reinterpret_cast<const char *>(g_009bb5e8)) == 0) {
            *stored_name = 0;
            int idx = *reinterpret_cast<int *>(self + 0x141f8);
            g_009ab898_tbl[idx * 0x34] &= 0xfd;
        } else if (strcmp(name, reinterpret_cast<const char *>(g_009bb5e8)) != 0) {
            reinterpret_cast<StrncpyFn>(strncpy)(
                stored_name, reinterpret_cast<const char *>(g_009bb5e8), 0x20);
            *reinterpret_cast<char *>(self + 0x1423f) = 0;
            int idx = *reinterpret_cast<int *>(self + 0x141f8);
            g_009ab898_tbl[idx * 0x34] |= 2;
        }
        draw_vehicle(0);
        reinterpret_cast<VCall *>(this)->slot063();
    }
}
