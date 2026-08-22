// ORIGINAL: 0x00441040 sub_441040 0x00441040-0x004410F1 FILE
// TRIED: void __cdecl(int a1, int a2), direct linked-list walk transcription; diverges at #29 mov vs cmp, not chased further
// working copy - scaffold materialised by --work
// size      177 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401100 0x00628A50

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_441040  at 0x00441040  (177 bytes)
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

class StringStruct;
struct StringStructEntry;

// ---- callees, declared and never defined (a definition would be inlined) ----
class StringStruct { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;
    int add(int);
};

struct StringStructEntry {
    uint32_t abi_word;
    int id;
    int payload;
    StringStructEntry * next;
    StringStructEntry * previous;
    uint32_t secondary_abi_word;
    void * allocation_owner;
};

void swap(int *, int *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00737cd8 = (int *)0x00737CD8;
static int *const g_00747bd8 = (int *)0x00747BD8;
static int *const g_00747bdc = (int *)0x00747BDC;
static int *const g_00747be0 = (int *)0x00747BE0;
static int *const g_00747be4 = (int *)0x00747BE4;
extern "C" void __cdecl sub_441040(int a1, int a2) {
    if (a1 > a2) {
        swap(&a1, &a2);
    }
    int loopBound = *g_00747be0;
    for (int i = loopBound - 3; i < loopBound; ++i) {
        if (i >= 0) {
            int curCount = *g_00747be0;
            int head = *g_00747bd8;
            int node;
            if (i <= curCount - 1) {
                node = head;
                if (i > 0) {
                    int n = i;
                    do {
                        node = *(int *)(node + 0xc);
                        --n;
                    } while (n != 0);
                }
                *g_00747bdc = node;
                *g_00747be4 = i;
            } else {
                node = *g_00747bdc;
            }
            int f1, f2;
            if (head == 0) {
                f1 = 0;
                f2 = 0;
            } else {
                f1 = *(int *)(node + 4);
                int p = *(int *)(node + 8);
                f2 = *(int *)(p + 4);
            }
            int idx = (a2 + a1 * 8) * 0x30;
            char *entry = (char *)g_00737cd8 + idx;
            *(int *)(entry + 0x1c) = f2;
            *(int *)(entry + 0x20) = 0;
            *(int *)(entry + 0x24) = 0;
            ((StringStruct *)entry)->add(f1);
        }
    }
}
