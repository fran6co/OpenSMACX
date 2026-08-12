// ORIGINAL: 0x0062E010 FILE
// name      ?close@Net@@QAEXXZ
// size      570 bytes
// spans     0x0062E010-0x0062E24A
// prototype void (__thiscall ?close@Net@@QAEXXZ)(Net* this)
// callers   12   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00633A50 0x00644EF2 0x006465F0
// indirect  0x0062E01D 0x0062E056 0x0062E05C 0x0062E06A 0x0062E075 0x0062E082 0x0062E089 0x0062E090 0x0062E093 0x0062E0AE 0x0062E0C2 0x0062E0CF 0x0062E0DC 0x0062E0E9 0x0062E0F6 0x0062E1A4 0x0062E1B2 0x0062E1CE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0062E010
// measured tier  MISMATCH
// divergence     25
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0062e010/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?close@Net@@QAEXXZ  at 0x0062E010  (570 bytes)
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
    void close();
};
extern "C" void *memset(void *, int, unsigned int);
extern "C" void free(void *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1, 2, 4, 5
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();  // <-- used
    virtual void slot002();  // <-- used
    virtual void slot003();
    virtual void slot004();  // <-- used
    virtual void slot005();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066912c = (int *)0x0066912C;
static int *const g_0066914c = (int *)0x0066914C;
static int *const g_00669170 = (int *)0x00669170;
static int *const g_00669174 = (int *)0x00669174;
static int *const g_00669178 = (int *)0x00669178;
static int *const g_0066917c = (int *)0x0066917C;
static int *const g_009be600 = (int *)0x009BE600;

class Net { public:
    void close();
};

// The globals at 0x9be600 hold a pointer to a C-style (non-C++) interface
// object: its methods are called via an explicit self argument pushed on
// the stack plus an indirect call through *(*self + offset), not through
// __thiscall. Modelled with a raw function-pointer type rather than VCall.
typedef void(__stdcall *IfaceFn)(void *);

// Real __thiscall member calls (this in ECX) can't be spelled as a raw
// function-pointer type under VC6 (C4234), so the two list-node calls below
// go through a second vtable shim with the argument shapes they need.
class VCallX { public:
    virtual void slot000();
    virtual void slot001(void *);
    virtual void slot002(int);
};

void Net::close() {
    char *self = reinterpret_cast<char *>(this);

    reinterpret_cast<VCall *>(self + 0x58)->slot005();

    *reinterpret_cast<uint32_t *>(self + 0xe0) = 0x4e20;
    *reinterpret_cast<uint32_t *>(self + 0x760) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x764) = 0;
    *reinterpret_cast<uint32_t *>(self + 0xdc) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x6dc) = 0;

    if (*g_009be600 != 0) {
        typedef void(__cdecl * VisitFn)(void *);
        VisitFn visit = *reinterpret_cast<VisitFn *>(g_0066917c);
        visit(self + 0x2c);

        (*reinterpret_cast<void(__cdecl **)(int)>(g_00669178))(*reinterpret_cast<int *>(self + 0x14));

        void *obj = *reinterpret_cast<void **>(g_009be600);
        (*reinterpret_cast<IfaceFn *>(*reinterpret_cast<char **>(obj) + 0x10))(obj);

        obj = *reinterpret_cast<void **>(g_009be600);
        (*reinterpret_cast<IfaceFn *>(*reinterpret_cast<char **>(obj) + 8))(obj);

        *g_009be600 = 0;
        *reinterpret_cast<uint32_t *>(self + 0x1c) = 0;

        (*reinterpret_cast<void(__cdecl **)(void *)>(g_00669174))(self + 0x2c);
        (*reinterpret_cast<void(__cdecl **)(int)>(g_0066912c))(0);
        visit(self + 0x2c);
        (*reinterpret_cast<void(__cdecl **)(void *)>(g_00669174))(self + 0x2c);

        *reinterpret_cast<uint32_t *>(self + 0x20) = 0;
        *reinterpret_cast<uint32_t *>(self + 0x24) = 0;
        *reinterpret_cast<uint32_t *>(self + 4) = 0;
    }

    if (*reinterpret_cast<uint32_t *>(self + 0x28) != 0) {
        *reinterpret_cast<uint32_t *>(self + 0x28) = 0;
        (*reinterpret_cast<void(__cdecl **)(void *)>(g_00669170))(self + 0x2c);
    }

    typedef void(__cdecl * ReleaseFn)(void *);
    ReleaseFn release = *reinterpret_cast<ReleaseFn *>(g_0066914c);

    if (*reinterpret_cast<void **>(self + 8) != 0) {
        release(*reinterpret_cast<void **>(self + 8));
        *reinterpret_cast<uint32_t *>(self + 8) = 0;
    }
    if (*reinterpret_cast<void **>(self + 0xc) != 0) {
        release(*reinterpret_cast<void **>(self + 0xc));
        *reinterpret_cast<uint32_t *>(self + 0xc) = 0;
    }
    if (*reinterpret_cast<void **>(self + 0x10) != 0) {
        release(*reinterpret_cast<void **>(self + 0x10));
        *reinterpret_cast<uint32_t *>(self + 0x10) = 0;
    }
    if (*reinterpret_cast<void **>(self + 0x14) != 0) {
        release(*reinterpret_cast<void **>(self + 0x14));
        *reinterpret_cast<uint32_t *>(self + 0x14) = 0;
    }
    if (*reinterpret_cast<void **>(self + 0x18) != 0) {
        release(*reinterpret_cast<void **>(self + 0x18));
        *reinterpret_cast<uint32_t *>(self + 0x18) = 0;
    }

    if (*reinterpret_cast<void **>(self + 0x704) != 0) {
        free(*reinterpret_cast<void **>(self + 0x704));
        *reinterpret_cast<uint32_t *>(self + 0x704) = 0;
    }
    if (*reinterpret_cast<void **>(self + 0x708) != 0) {
        free(*reinterpret_cast<void **>(self + 0x708));
        *reinterpret_cast<uint32_t *>(self + 0x708) = 0;
    }
    if (*reinterpret_cast<void **>(self + 0x6e0) != 0) {
        free(*reinterpret_cast<void **>(self + 0x6e0));
        *reinterpret_cast<uint32_t *>(self + 0x6e0) = 0;
    }
    *reinterpret_cast<uint32_t *>(self + 0x6e4) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x6fc) = 0xc8;
    *reinterpret_cast<uint32_t *>(self + 0xd8) = 0;

    {
        char *p = self + 0x158;
        int n = 0x10;
        do {
            *reinterpret_cast<uint32_t *>(p - 4) = 0;
            *reinterpret_cast<uint32_t *>(p) = 0;
            *reinterpret_cast<uint32_t *>(p + 4) = 0;
            *reinterpret_cast<uint8_t *>(p + 0x10) = 0;
            p += 0x58;
        } while (--n);
    }

    if (*reinterpret_cast<uint32_t *>(self + 0xb8) != 0) {
        char *node = self + 0xb0;
        if (*reinterpret_cast<int *>(node + 0x10) > 0) {
            int i = 0;
            do {
                char *cur = *reinterpret_cast<char **>(node + 8);
                *reinterpret_cast<uint32_t *>(node + 0xc) = *reinterpret_cast<uint32_t *>(cur + 0xc);
                char *item = *reinterpret_cast<char **>(cur + 8);
                reinterpret_cast<VCallX *>(node)->slot001(item);
                if (item != 0) {
                    reinterpret_cast<VCallX *>(item)->slot002(1);
                }
                char *field8 = *reinterpret_cast<char **>(node + 8);
                *reinterpret_cast<uint32_t *>(field8 + 8) = 0;
                field8 = *reinterpret_cast<char **>(node + 8);
                if (field8 != 0) {
                    (*reinterpret_cast<IfaceFn *>(*reinterpret_cast<char **>(field8) + *reinterpret_cast<uint32_t *>(*reinterpret_cast<char **>(field8) + 4)))(field8);
                }
                *reinterpret_cast<uint32_t *>(node + 8) = *reinterpret_cast<uint32_t *>(node + 0xc);
                ++i;
            } while (i < *reinterpret_cast<int *>(node + 0x10));
        }
        *reinterpret_cast<uint32_t *>(node + 8) = 0;
        *reinterpret_cast<uint32_t *>(node + 0x14) = 0;
        *reinterpret_cast<uint32_t *>(node + 0x10) = 0;
    }

    reinterpret_cast<NetFifo *>(self + 0xe8)->close();
    *reinterpret_cast<uint32_t *>(self + 0xd4) = 0;
    *reinterpret_cast<uint32_t *>(self + 0xd8) = 0;

    reinterpret_cast<NetFifo *>(self + 0x10c)->close();
    reinterpret_cast<NetFifo *>(self + 0x130)->close();

    memset(self + 0x6ec, 0, 0x10);

    *reinterpret_cast<uint32_t *>(self + 0xe4) = 1;
    *reinterpret_cast<uint32_t *>(self + 0x48) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x4c) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x50) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x54) = 0;
}
