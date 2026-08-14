// ORIGINAL: 0x006340C0 FILE
// working copy - scaffold materialised by --work
// name      sub_6340c0
// size      153 bytes
// spans     0x006340C0-0x00634159
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645930
// indirect  0x006340C8 0x006340D5 0x006340F3 0x00634105 0x0063414C

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6340c0  at 0x006340C0  (153 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void *memcpy(void *, const void *, unsigned int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669174 = (int *)0x00669174;
static int *const g_0066917c = (int *)0x0066917C;
// SIGNATURE CHANGED: the disassembly reads [ecx+0] and [ecx+0xc] with no
// matching stack slot before the first push, so the receiver is `this`,
// not stack argument a1 - the fallback stdcall/int contract has no way to
// express that, so this lands as a __thiscall member of a shim record.
class Sub6340c0Rec { public:
    void *head_;
    char pad_[8];
    long crit_section_[6];
    int find(int argA, int argB, int *argC, int *argD);
};

typedef void (__stdcall *EnterCSFn)(void *);
typedef void (__stdcall *LeaveCSFn)(void *);

int Sub6340c0Rec::find(int argA, int argB, int *argC, int *argD) {
    void *cs = (void *)&crit_section_;
    (*(EnterCSFn *)g_0066917c)(cs);
    int *node = (int *)head_;
    if (node != 0) {
        while (node[1] != argB) {
            node = (int *)node[5];
            if (node == 0) {
                (*(LeaveCSFn *)g_00669174)(cs);
                return 0;
            }
        }
        if (node == 0) {
            (*(LeaveCSFn *)g_00669174)(cs);
            return 0;
        }
        if (argD != 0) {
            *argD = node[4];
        }
        if (node[3] != 0 && argA != 0) {
            memcpy((void *)argA, (void *)node[3], node[4]);
        }
        if (argC != 0) {
            *argC = node[0];
        }
        int result = node[2];
        (*(LeaveCSFn *)g_00669174)(cs);
        return result;
    }
    (*(LeaveCSFn *)g_00669174)(cs);
    return 0;
}
