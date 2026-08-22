// ORIGINAL: 0x004C73C0 sub_4c73c0 0x004C73C0-0x004C74A4 FILE
// TRIED: this is a __thiscall member whose `this` (ecx) is a raw `int *` array (indexed `param_1[uVar6]`, no fixed-offset fields), and param_2 is `Entry*` (3 uints + a 0x104-byte name buffer = 0x110 bytes, matching the `operator new(0x110)` allocation size exactly - `local_8` and `param_2` share this same layout). operator new/delete (mangled ??2@.../??3@...) reached via differently-named externs `op_new`/`op_delete` with the real signatures, since the scaffold's fn_0064557f/fn_0064558a are nullary and the call targets are masked from comparison anyway. strcpy is called UNCONDITIONALLY on `local_8->name` even on the `operator new` failure path (ghidra confirms this, not an artifact) - preserved as-is rather than "fixed".
// TRIED: not chased past the initial translation (0.71 mnemonic similarity); this function's 4-local stack frame (ebp-4/-8/-0xc/-0x10) is not reproduced.
// working copy - scaffold materialised by --work
// size      228 bytes
// prototype 
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00645460 0x0064557F 0x0064558A 0x006465F0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c73c0  at 0x004C73C0  (228 bytes)
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
extern "C" char *strcpy(char *, const char *);
extern "C" void *memset(void *, int, unsigned int);
int __cdecl fn_0064557f();
int __cdecl fn_0064558a();
extern "C" void *op_new(unsigned int);
extern "C" void op_delete(void *);

struct Entry {
    unsigned int a;
    unsigned int b;
    unsigned int c;
    char name[0x104];
};

class Table { public:
    void method(Entry *param_2);
};

void Table::method(Entry *param_2) {
    int *param_1 = reinterpret_cast<int *>(this);

    unsigned int uVar1 = param_2->b;
    unsigned int uVar2 = param_2->a;

    Entry *local_8 = reinterpret_cast<Entry *>(op_new(0x110));
    if (local_8 != 0) {
        memset(local_8->name, 0, 0x104);
        local_8->b = 0;
        local_8->a = 0;
        local_8->c = 1;
    }

    strcpy(local_8->name, param_2->name);

    if (local_8->a < uVar1) {
        local_8->a = uVar1;
    }
    local_8->b = uVar1;
    if (uVar2 < uVar1) {
        local_8->b = uVar2;
    }
    local_8->a = uVar2;
    local_8->c = param_2->c;

    unsigned int uVar6 = uVar1;
    if (uVar1 <= uVar2) {
        do {
            int *piVar3 = reinterpret_cast<int *>(param_1[uVar6]);
            if (piVar3 != 0) {
                unsigned int uVar4 = piVar3[1];
                uVar6 = piVar3[0] + 1;
                op_delete(reinterpret_cast<void *>(param_1[uVar4]));
                if (uVar4 < uVar6) {
                    int *puVar7 = param_1 + uVar4;
                    for (int iVar5 = uVar6 - uVar4; iVar5 != 0; iVar5--) {
                        *puVar7 = 0;
                        puVar7++;
                    }
                }
            }
            uVar6 = uVar6 + 1;
        } while (uVar6 <= uVar2);

        if (uVar1 <= uVar2) {
            int *puVar7 = param_1 + uVar1;
            for (int iVar5 = (uVar2 - uVar1) + 1; iVar5 != 0; iVar5--) {
                *puVar7 = reinterpret_cast<int>(local_8);
                puVar7++;
            }
        }
    }
}
