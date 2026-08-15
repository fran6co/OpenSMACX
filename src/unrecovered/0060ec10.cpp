// ORIGINAL: 0x0060EC10 BYTE_EXACT FILE
// name      ?get_state_id@CheckBox@@QAEHH@Z
// size      105 bytes
// spans     0x0060EC10-0x0060EC79
// prototype int (__thiscall ?get_state_id@CheckBox@@QAEHH@Z)(CheckBox* this, int)
// callers   6   call targets   0
// kind      
// flags     
// calls     (none)
// LEVER: a `for (idx = 0; idx < count; idx = idx + 1) { ...; if (match) break; ... }`
//        matches the original's single `cmp;jl` loop back-edge exactly; the
//        equivalent `if (0 < count) { do { ...; if (match) break; ... }
//        while (idx < count); }` phrasing compiled the SAME loop with an
//        extra `cmp;jge` ahead of the back-edge test every time it was tried
//        (11 candidates, always MISMATCH at the loop tail) even though it is
//        the textbook lowering of a for-loop and should be identical at -O2.
//        The list-search structure: base = *(*this+8); the node list lives
//        at this+base+0xbc, with head/current at +8, count at +0x10, a
//        running index at +0x14, and each node's key/next at +4/+0xc. The
//        tail re-derives `*(*this+8)` fresh rather than reusing the cached
//        `base` local - reusing it compiles to extra reload instructions
//        the original does not have.
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_state_id@CheckBox@@QAEHH@Z  at 0x0060EC10  (105 bytes)
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

class CheckBox;

class CheckBox { public:
    int get_state_id(int);
};
int CheckBox::get_state_id(int a1) {
    int base = *(int *)(*(int *)this + 8);
    char *list = (char *)this + base + 0xbc;
    int head = *(int *)(list + 8);
    if (head != 0) {
        int count = *(int *)(list + 0x10);
        int idx;
        *(int *)(list + 0x14) = 0;
        *(int *)(list + 0xc) = head;
        for (idx = 0; idx < count; idx = idx + 1) {
            int cur = *(int *)(list + 0xc);
            if (*(int *)(cur + 4) == a1) break;
            *(int *)(list + 0x14) = *(int *)(list + 0x14) + 1;
            *(int *)(list + 0xc) = *(int *)(cur + 0xc);
        }
    }
    return (1 << *(int *)((int)this + *(int *)(*(int *)this + 8) + 0xd0)) &
           *(unsigned int *)((int)this + *(int *)(*(int *)this + 8) + 0xec);
}
