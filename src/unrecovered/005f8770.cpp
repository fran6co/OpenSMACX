// ORIGINAL: 0x005F8770 sub_5f8770 0x005F8770-0x005F87F6 FILE
// working copy - scaffold materialised by --work
// size      134 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2 0x0064557F

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5f8770  at 0x005F8770  (134 bytes)
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
extern "C" void free(void *);
int __cdecl fn_0064557f();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066ff30 = (int *)0x0066FF30;
class Sub5F8770 { public:
    int destroy(int flag);
};

int Sub5F8770::destroy(int flag) {
    char *self = reinterpret_cast<char *>(this);
    int *head = *reinterpret_cast<int **>(self + 4);
    *reinterpret_cast<int *>(self) = 0x66ff30;
    if (head != 0) {
        if (*reinterpret_cast<int *>(self + 0x14) == 0) {
            int count = *reinterpret_cast<int *>(self + 0xc);
            if (count > 0) {
                int i = 0;
                do {
                    char *cur = *reinterpret_cast<char **>(self + 4);
                    int next = *reinterpret_cast<int *>(cur + 0xc);
                    *reinterpret_cast<int *>(self + 8) = next;
                    int *data = *reinterpret_cast<int **>(cur + 8);
                    if (data != 0) {
                        free(data);
                    }
                    char *cur2 = *reinterpret_cast<char **>(self + 4);
                    *reinterpret_cast<int *>(cur2 + 8) = 0;
                    char *cur3 = *reinterpret_cast<char **>(self + 4);
                    if (cur3 != 0) {
                        free(cur3);
                    }
                    i++;
                    *reinterpret_cast<int *>(self + 4) = *reinterpret_cast<int *>(self + 8);
                } while (i < *reinterpret_cast<int *>(self + 0xc));
            }
        }
        *reinterpret_cast<int *>(self + 4) = 0;
        *reinterpret_cast<int *>(self + 0x10) = 0;
        *reinterpret_cast<int *>(self + 0xc) = 0;
    }
    *reinterpret_cast<int *>(self + 0x10) = 0;
    if (flag & 1) {
        operator delete(this);
    }
    return reinterpret_cast<int>(this);
}
