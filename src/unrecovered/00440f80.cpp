// ORIGINAL: 0x00440F80 FILE
// RULED-OUT: member-fn-pointer+union thunk for the two vtable-adjusted
//            "release" calls; raw pointer cast over fixed globals for
//            record base/bound (address itself does read-modify-write
//            via `rec += 0xc`). First divergence is #11 lea vs je.
// working copy - scaffold materialised by --work
// name      ?clear_hotseat_chat@@YAXXZ
// size      179 bytes
// spans     0x00440F80-0x00441033
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00440FBA 0x00440FCC 0x00440FEA

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?clear_hotseat_chat@@YAXXZ  at 0x00440F80  (179 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00737ce0 = (int *)0x00737CE0;
static int *const g_007388e0 = (int *)0x007388E0;

class HSShim { public:
    void call1(int);
};
union HSThunk {
    void (HSShim::*m)(int);
    void *raw;
};

void __cdecl clear_hotseat_chat() {
    int *rec = (int *)0x00737CE0;
    while (reinterpret_cast<int>(rec) < 0x007388E0) {
        int outer = 8;
        do {
            if (rec[0] != 0) {
                int j = 0;
                if (rec[2] > 0) {
                    do {
                        rec[1] = *(int *)(rec[0] + 0xc);
                        int *node = *(int **)(rec[0] + 8);
                        char *ebx = (char *)rec - 8;
                        void *vtbl_ebx = *(void **)ebx;
                        HSThunk t1;
                        t1.raw = *(void **)((char *)vtbl_ebx + 4);
                        (reinterpret_cast<HSShim *>(ebx)->*t1.m)((int)node);
                        if (node != 0) {
                            void *vtbl_node = *(void **)node;
                            int off = *(int *)((char *)vtbl_node + 4);
                            char *adj = (char *)node + off;
                            void *vtbl_adj = *(void **)adj;
                            HSThunk t0;
                            t0.raw = *(void **)vtbl_adj;
                            (reinterpret_cast<HSShim *>(adj)->*t0.m)(1);
                        }
                        *(int *)(rec[0] + 8) = 0;
                        int *node2 = (int *)rec[0];
                        if (node2 != 0) {
                            void *vtbl2 = *(void **)node2;
                            int off2 = *(int *)((char *)vtbl2 + 4);
                            char *adj2 = (char *)node2 + off2;
                            void *vtbl_adj2 = *(void **)adj2;
                            HSThunk t0b;
                            t0b.raw = *(void **)vtbl_adj2;
                            (reinterpret_cast<HSShim *>(adj2)->*t0b.m)(1);
                        }
                        rec[0] = rec[1];
                        ++j;
                    } while (j < rec[2]);
                }
                rec[0] = 0;
                rec[3] = 0;
                rec[2] = 0;
            }
            rec[3] = 0;
            rec[4] = 0;
            rec += 0xc;
            --outer;
        } while (outer != 0);
    }
}
