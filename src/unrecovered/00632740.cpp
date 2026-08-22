// ORIGINAL: 0x00632740 sub_632740 0x00632740-0x006327D2 FILE
// TRIED: EnterCriticalSection called only once per iteration - calling through `[dword ptr g_0066917c]` directly (not cached in a local like leaveCS/wait, which are each called twice) got the prologue's push/mov schedule to within 1 byte of the original.
// PROPOSAL: sub_632740() -> sub_632740(NetThreadInfo *param_1); [esp+0x1c]
//           reads a real stack argument, no eax set at any `ret`, so `int`
//           return is unproven either way (kept it, matches the contract).
// working copy - scaffold materialised by --work
// size      146 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     noret;sp_ready
// calls     0x006327E0 0x00647067
// indirect  0x0063276A 0x0063277A 0x006327A2 0x006327AF 0x006327C3

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_632740  at 0x00632740  (146 bytes)
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
struct NetThreadInfo;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Net { public:
    int internal_receive(NetThreadInfo *);
};

extern "C" int __cdecl _endthread();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669174 = (int *)0x00669174;
static int *const g_0066917c = (int *)0x0066917C;
static int *const g_00669184 = (int *)0x00669184;
typedef unsigned long (__stdcall *WaitFn)(unsigned long, void *, int, unsigned long);
typedef void (__stdcall *EnterLeaveFn)(void *);

extern "C" int __cdecl sub_632740(NetThreadInfo *param_1) {
    WaitFn wait = reinterpret_cast<WaitFn>(*g_00669184);
    char *self = reinterpret_cast<char *>(param_1);
    void *handles[2];
    handles[0] = *reinterpret_cast<void **>(self + 0xc);
    handles[1] = *reinterpret_cast<void **>(self + 0x10);

    if (wait(2, handles, 0, 0xffffffff) == 0) {
        EnterLeaveFn leaveCS = reinterpret_cast<EnterLeaveFn>(*g_00669174);
        void *cs = self + 0x28;
        Net *net = reinterpret_cast<Net *>(param_1);
        for (;;) {
            (*reinterpret_cast<EnterLeaveFn *>(g_0066917c))(cs);
            if (*reinterpret_cast<int *>(self + 0x18) == 0) {
                break;
            }
            if (net->internal_receive(param_1) != 0) {
                do {
                } while (net->internal_receive(param_1) != 0);
            }
            leaveCS(cs);
            if (wait(2, handles, 0, 0xffffffff) != 0) {
                _endthread();
                return 0;
            }
        }
        leaveCS(cs);
    }
    _endthread();
    return 0;
}
