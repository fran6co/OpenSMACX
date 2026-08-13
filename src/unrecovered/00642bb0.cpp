// ORIGINAL: 0x00642BB0 FILE
// RULED-OUT: zero-arg contract (disasm reads 3 stack args, so cdecl(int,int,int));
//            redeclaring sub_642b10 with 3 params (C2733, second C linkage) -
//            cast the existing 0-arg decl's address through a function-pointer
//            typedef instead. Register allocation still MISMATCHes the prologue.
// working copy - scaffold materialised by --work
// name      sub_642bb0
// size      110 bytes
// spans     0x00642BB0-0x00642C1E
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00642B10
// indirect  0x00642BBF 0x00642BDA 0x00642BEC

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_642bb0  at 0x00642BB0  (110 bytes)
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
extern "C" int __cdecl sub_642b10();
typedef void *(__cdecl *AllocFn)(void *, int, int);
typedef void (__cdecl *FreeFn)(void *, void *);
typedef int (__cdecl *Init3Fn)(int, int, int);

extern "C" int __cdecl sub_642bb0(int param1, int param2, int param3) {
    char *p1 = reinterpret_cast<char *>(param1);
    AllocFn alloc_fn = *reinterpret_cast<AllocFn *>(p1 + 0x20);
    FreeFn free_fn = *reinterpret_cast<FreeFn *>(p1 + 0x24);
    void *ctx = *reinterpret_cast<void **>(p1 + 0x28);

    int *obj = reinterpret_cast<int *>(alloc_fn(ctx, 1, 0x3c));
    if (obj == 0) {
        return 0;
    }
    int size = reinterpret_cast<int>(alloc_fn(ctx, 1, param3));
    obj[9] = size;
    if (size == 0) {
        free_fn(ctx, obj);
        return 0;
    }
    obj[10] = size + param3;
    obj[0xd] = param2;
    obj[0] = 0;
    ((Init3Fn)sub_642b10)(reinterpret_cast<int>(obj), param1, reinterpret_cast<int>(obj + 0xe));
    return reinterpret_cast<int>(obj);
}
