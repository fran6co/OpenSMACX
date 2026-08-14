// ORIGINAL: 0x0051E3C0 FILE
// RULED-OUT: second slot000 dispatch expects 'lea' for the adjustor-thunk
//            address but every tried spelling (add, swapped operands, fully
//            inlined) still emits 'add'; jl/jb fixed via reinterpret_cast<int>
//            on the loop-bound pointer compare
// working copy - scaffold materialised by --work
// name      ?clear_all_player_messages@@YAXXZ
// size      185 bytes
// spans     0x0051E3C0-0x0051E479
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0051E404 0x0051E416 0x0051E434

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?clear_all_player_messages@@YAXXZ  at 0x0051E3C0  (185 bytes)
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
static int *const g_0093a7b8 = (int *)0x0093A7B8;
static int *const g_0093a938 = (int *)0x0093A938;
class VCallX { public:
    virtual void slot000(int);
    virtual void slot001(int);
};

extern int g_93a98c_arr[];
extern int g_93a96c_arr[];

void __cdecl clear_all_player_messages() {
    int byteOff = 0;
    char *entry = reinterpret_cast<char *>(g_0093a7b8);
    do {
        int head = *reinterpret_cast<int *>(entry);
        *reinterpret_cast<int *>(reinterpret_cast<char *>(g_93a98c_arr) + byteOff) = 0;
        *reinterpret_cast<int *>(reinterpret_cast<char *>(g_93a96c_arr) + byteOff) = 0;
        char *mgr = entry - 8;
        if (head != 0) {
            int count = *reinterpret_cast<int *>(entry + 8);
            int i = 0;
            if (count > 0) {
                do {
                    head = *reinterpret_cast<int *>(entry);
                    int next = *reinterpret_cast<int *>(reinterpret_cast<char *>(head) + 0xc);
                    *reinterpret_cast<int *>(entry + 4) = next;
                    int payload = *reinterpret_cast<int *>(reinterpret_cast<char *>(head) + 8);
                    reinterpret_cast<VCallX *>(mgr)->slot001(payload);
                    if (payload != 0) {
                        int *vt = *reinterpret_cast<int **>(payload);
                        int off = *reinterpret_cast<int *>(reinterpret_cast<char *>(vt) + 4);
                        char *obj = reinterpret_cast<char *>(payload) + off;
                        reinterpret_cast<VCallX *>(obj)->slot000(1);
                    }
                    int h2 = *reinterpret_cast<int *>(entry);
                    *reinterpret_cast<int *>(reinterpret_cast<char *>(h2) + 8) = 0;
                    int h3 = *reinterpret_cast<int *>(entry);
                    if (h3 != 0) {
                        int *vt = *reinterpret_cast<int **>(h3);
                        int off = *reinterpret_cast<int *>(reinterpret_cast<char *>(vt) + 4);
                        char *obj = off + reinterpret_cast<char *>(h3);
                        reinterpret_cast<VCallX *>(obj)->slot000(1);
                    }
                    int nextSaved = *reinterpret_cast<int *>(entry + 4);
                    *reinterpret_cast<int *>(entry) = nextSaved;
                    count = *reinterpret_cast<int *>(entry + 8);
                    ++i;
                } while (i < count);
            }
            *reinterpret_cast<int *>(mgr + 8) = 0;
            *reinterpret_cast<int *>(mgr + 0x14) = 0;
            *reinterpret_cast<int *>(mgr + 0x10) = 0;
        }
        entry += 0x30;
        byteOff += 4;
        *reinterpret_cast<int *>(mgr + 0x14) = 0;
        *reinterpret_cast<int *>(mgr + 0x18) = 0;
    } while (reinterpret_cast<int>(entry) < reinterpret_cast<int>(g_0093a938));
}
