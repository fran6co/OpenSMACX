// ORIGINAL: 0x00630B80 FILE
// working copy - scaffold materialised by --work
// name      sub_630b80
// size      99 bytes
// spans     0x00630B80-0x00630BE3
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00630BD4

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_630b80  at 0x00630B80  (99 bytes)
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

// The callee here is pushed an explicit object pointer rather than
// receiving it via ECX, so it is not a thiscall v-call - it is a
// manual __stdcall dispatch through a function-pointer table (slot
// 25, i.e. offset 0x64) whose callee pops its own args (no post-call
// stack fixup in the caller).
typedef int (__stdcall *Slot25Fn)(void *, int *, int *, int, int, int *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009bc4bc = (int *)0x009BC4BC;
static int *const g_009be600 = (int *)0x009BE600;

// ecx is used purely as `this` for the two field reads below (offsets
// 0x760 and 0x6e4); the class is deliberately opaque.
class Obj630b80 { public:
    void sub_630b80(int a1);
};

void Obj630b80::sub_630b80(int a1) {
    void *obj = *reinterpret_cast<void **>(g_009be600);
    if (obj != 0) {
        int iVar1 = a1;
        int result;
        do {
            int uStack_8 = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x760);
            a1 = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x6e4);
            int iStack_4 = iVar1;
            int flag = (iVar1 != 0) ? 6 : 3;
            Slot25Fn fn = reinterpret_cast<Slot25Fn *>(*reinterpret_cast<void **>(obj))[25];
            result = fn(obj, &iStack_4, &uStack_8, flag, (int)g_009bc4bc, &a1);
            obj = *reinterpret_cast<void **>(g_009be600);
        } while (result == 0);
    }
}
