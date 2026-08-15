// ORIGINAL: 0x0060EB80 BYTE_EXACT FILE
// name      ?set_state_id@CheckBox@@QAEXHH@Z
// size      139 bytes
// spans     0x0060EB80-0x0060EC0B
// prototype void (__thiscall ?set_state_id@CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   3   call targets   0
// kind      
// flags     
// calls     (none)
// LEVER: same node-list search as get_state_id@CheckBox (0x0060EC10, this
//        exact repo), but `idx = 0` is initialised INSIDE the `head != 0`
//        guard (right after loading count), not before it - the disassembly
//        places `xor esi, esi` after the `je` that skips an empty list,
//        unlike the getter where it comes first. A `for` loop (not an
//        `if (0<count) { do {...} while(...); }`) matches the loop's single
//        `cmp;jl` back-edge exactly. The tail's `if (a2 != 0) { or } else
//        { and }` polarity matters: the original falls through on
//        `a2 != 0` (the set/or path) and only jumps for the clear/and path,
//        so the condition has to be written positive (`a2 != 0` first) to
//        get the branch the right way around. Every value the tail needs is
//        re-derived through a fresh `(int)this + *(int*)(*(int*)this+8)`
//        each time rather than cached in a local - caching it (even as
//        `int base` reused twice) reorders the register pops relative to
//        the original and changes the `lea`/`add` used to build the
//        pointer.
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_state_id@CheckBox@@QAEXHH@Z  at 0x0060EB80  (139 bytes)
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
    void set_state_id(int, int);
};
void CheckBox::set_state_id(int a1, int a2) {
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
    if (a2 != 0) {
        *(unsigned int *)((int)this + *(int *)(*(int *)this + 8) + 0xec) |=
            1 << *(int *)((int)this + *(int *)(*(int *)this + 8) + 0xd0);
    } else {
        *(unsigned int *)((int)this + *(int *)(*(int *)this + 8) + 0xec) &=
            ~(1 << *(int *)((int)this + *(int *)(*(int *)this + 8) + 0xd0));
    }
}
