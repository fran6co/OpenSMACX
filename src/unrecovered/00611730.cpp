// ORIGINAL: 0x00611730 BYTE_EXACT FILE
// name      sub_611730
// size      147 bytes
// spans     0x00611730-0x006117C3
// prototype 
// callers   3   call targets   0
// kind      
// flags     
// calls     (none)
// indirect  0x0061176E 0x00611780 0x0061179C
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_611730  at 0x00611730  (147 bytes)
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
static int *const g_00669438 = (int *)0x00669438;
static int *const g_0066943c = (int *)0x0066943C;
// LEVER: [ecx+N] reads with no stack access -> __thiscall member, not a free
// function. `self = (char*)this - 0x28` (an adjustor: `this` here is a
// sub-object's `this`, and offset -0x28 reaches the owning object). VC6
// forces local-class member functions to carry a body ("must be defined
// within the class"), so the file-scope `VCall` shim (nullary, fixed by the
// scaffold, can't be redeclared without an emitted-thunk / redefinition
// conflict) can't carry the two argument-taking calls this body needs.
// Fix: declare a *local*, *pure virtual* class (`LocalVCall`) instead - pure
// virtuals need no out-of-line body, so it costs nothing extra in the
// object, and its slot signatures are free to match the calls exactly.
// Getting to BYTE_EXACT also needed: reading `*(self+8)` fresh at each of
// its four uses instead of caching it in a named local (matches the
// original's repeated reloads); computing the FIRST adjusted-vtable call's
// `this` through a named `adjusted` local (`add ecx,edi` then a separate
// deref) while leaving the SECOND identical-shaped call fully inlined as
// one expression (`lea ecx,[edx+eax]` then a refetch) - the two calls are
// semantically identical but the original's register allocator picked
// different shapes for each, and matching that asymmetry needed asymmetric
// source; and marking the first of the two `self+0x14 = 0` stores (the
// second is 0x18 bytes later, past the loop) `volatile` so VC6's dead-store
// elimination doesn't fold away the original's genuine (if redundant)
// double write.
class Obj611730 { public:
    void run();
};

void Obj611730::run() {
    class LocalVCall {
    public:
        virtual void slot0(int) = 0;
        virtual void slot1(void *) = 0;
    };

    char *self = reinterpret_cast<char *>(this) - 0x28;
    char *this_char = reinterpret_cast<char *>(this);

    *reinterpret_cast<int *>(self) = 0x66943c;

    int *base_ptr = *reinterpret_cast<int **>(this_char - 0x24);
    int adjust = *reinterpret_cast<int *>(reinterpret_cast<char *>(base_ptr) + 4);
    *reinterpret_cast<int *>(this_char - 0x24 + adjust) = 0x669438;

    void *head = *reinterpret_cast<void **>(self + 8);
    if (head != 0) {
        int count = *reinterpret_cast<int *>(self + 0x10);
        int i = 0;
        if (count > 0) {
            do {
                *reinterpret_cast<void **>(self + 0xc) =
                    *reinterpret_cast<void **>(reinterpret_cast<char *>(
                        *reinterpret_cast<void **>(self + 8)) + 0xc);
                void *child = *reinterpret_cast<void **>(reinterpret_cast<char *>(
                    *reinterpret_cast<void **>(self + 8)) + 8);

                reinterpret_cast<LocalVCall *>(self)->slot1(child);

                if (child != 0) {
                    char *vtbl = *reinterpret_cast<char **>(child);
                    int adj = *reinterpret_cast<int *>(vtbl + 4);
                    char *adjusted = reinterpret_cast<char *>(child) + adj;
                    reinterpret_cast<LocalVCall *>(adjusted)->slot0(1);
                }

                *reinterpret_cast<int *>(reinterpret_cast<char *>(
                    *reinterpret_cast<void **>(self + 8)) + 8) = 0;

                void *cur2 = *reinterpret_cast<void **>(self + 8);
                if (cur2 != 0) {
                    reinterpret_cast<LocalVCall *>(
                        reinterpret_cast<char *>(cur2) +
                        *reinterpret_cast<int *>(
                            *reinterpret_cast<char **>(cur2) + 4))->slot0(1);
                }

                i++;
                *reinterpret_cast<void **>(self + 8) = *reinterpret_cast<void **>(self + 0xc);
            } while (i < *reinterpret_cast<int *>(self + 0x10));
        }
        *reinterpret_cast<volatile int *>(self + 0x14) = 0;
        *reinterpret_cast<int *>(self + 8) = 0;
        *reinterpret_cast<int *>(self + 0x10) = 0;
        *reinterpret_cast<int *>(self + 0x14) = 0;
    } else {
        *reinterpret_cast<int *>(self + 0x14) = 0;
    }
}
