// ORIGINAL: 0x00632DB0 FILE
// name      ?handle_sys_msg@Net@@QAEXXZ
// size      487 bytes
// spans     0x00632DB0-0x00632F97
// prototype void (__thiscall ?handle_sys_msg@Net@@QAEXXZ)(Net* this)
// callers   7   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062E010 0x00632350 0x00632CC0 0x00633F70
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00632DB0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00632db0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?handle_sys_msg@Net@@QAEXXZ  at 0x00632DB0  (487 bytes)
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

class Net;
class NetFifo;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetFifo { public:
    int m_00633f70(void *, unsigned int *, int *, unsigned int *);
};
extern "C" int __stdcall sub_632cc0(int, int, int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 3, 4, 21
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();  // <-- used
    virtual void slot004();  // <-- used
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
    virtual void slot021();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009bc4bc = (int *)0x009BC4BC;
static int *const g_009bc4c0 = (int *)0x009BC4C0;
static int *const g_009bc4c4 = (int *)0x009BC4C4;
static int *const g_009bc4c8 = (int *)0x009BC4C8;
static int *const g_009bc4cc = (int *)0x009BC4CC;
static int *const g_009bc4d4 = (int *)0x009BC4D4;
static int *const g_009be4ec = (int *)0x009BE4EC;
static int *const g_009be4f8 = (int *)0x009BE4F8;
static int *const g_009be600 = (int *)0x009BE600;

class Net { public:
    void close();
    int drop_player(unsigned long, int);
    void handle_sys_msg();
};

// widened for the two-int-arg call at msg==0x103 (slot 3 of the primary
// dispatch vtable, `call dword ptr [eax+0xc]` preceded by two pushes)
class VCall2 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003(int, int);  // <-- used, widened
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
};

// The register-record table this loop walks by advancing a running
// pointer (`add esi, 0x28` each iteration) - the address itself is
// the loop base, so it needs the extern-array spelling, not a folded
// pointer constant.
extern int g_009bc4c8x[];

void Net::handle_sys_msg() {
    char *self = reinterpret_cast<char *>(this);

    if (*g_009be600 == 0) {
        return;
    }

    NetFifo *fifo = reinterpret_cast<NetFifo *>(self + 0x130);
    unsigned int local_8;

    while (fifo->m_00633f70(g_009bc4bc, &local_8, 0, 0)) {
        if (local_8 == 0) {
            switch (*reinterpret_cast<unsigned int *>(g_009bc4bc)) {
            case 3:
                if (*reinterpret_cast<int *>(self + 0x760) == *reinterpret_cast<int *>(self + 0x764) &&
                    *g_009bc4c0 == 1) {
                    int local_4 = 0x100;
                    void *obj = *reinterpret_cast<void **>(g_009be600);
                    typedef void(__stdcall * Slot21)(void *, unsigned long, const char *, int *);
                    Slot21 fn = reinterpret_cast<Slot21>((*reinterpret_cast<void ***>(obj))[21]);
                    fn(obj, *reinterpret_cast<unsigned long *>(g_009bc4c4),
                       reinterpret_cast<const char *>(g_009be4ec), &local_4);

                    unsigned char b;
                    if (*g_009bc4cc == 0) {
                        b = 0;
                    } else {
                        b = *reinterpret_cast<unsigned char *>(*g_009bc4cc);
                    }
                    sub_632cc0(*g_009be4f8, *g_009bc4c4, b);
                }
                break;
            case 5:
                if (*g_009bc4c0 == 1) {
                    drop_player(*g_009bc4c4, 1);
                }
                break;
            case 0x31:
                reinterpret_cast<VCall *>(this)->slot004();
                close();
                break;
            case 0x102: {
                int i;
                char *p = self + 0x154;
                for (i = 0; i < 0x10; ++i, p += 0x58) {
                    if (*reinterpret_cast<int *>(p) == *g_009bc4c4) break;
                }
                if (i != 0x10 && *g_009bc4c8 != 0) {
                    *reinterpret_cast<char *>(self + 0x168 + i * 0x58) = *reinterpret_cast<char *>(*g_009bc4c8);
                }
                break;
            }
            case 0x103:
                reinterpret_cast<VCall2 *>(this)->slot003(*g_009bc4d4, *g_009bc4c4);
                break;
            }
        } else {
            unsigned char flags = *reinterpret_cast<unsigned char *>(g_009bc4bc);
            if (flags & 0x80) {
                drop_player(*g_009bc4c4, 1);
            } else if (flags & 0x10) {
                if (*reinterpret_cast<char *>(g_009bc4c4) != 0) {
                    int *p = g_009bc4c8x;
                    int i = 0;
                    do {
                        sub_632cc0(reinterpret_cast<int>(p + 1), p[0], p[9]);
                        ++i;
                        p += 10;
                    } while (i < (int)(*g_009bc4c4 & 0xff));
                }
            }
        }
    }
}
