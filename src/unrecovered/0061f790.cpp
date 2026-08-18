// ORIGINAL: 0x0061F790 sub_61f790 0x0061F790-0x0061F7FD FILE
// working copy - scaffold materialised by --work
// size      109 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00624D80

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61f790  at 0x0061F790  (109 bytes)
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

struct EdgeScan;
class Texture;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Texture { public:
    void * pixels_;
    uint32_t iWidth_;
    uint32_t iHeight_;
    uint8_t unmapped_[0x60];
    uint32_t borrowed_;
    int setup_edge(EdgeScan *, int);
};

extern "C" int __cdecl sub_61f790(Texture *tex) {
    char *self = reinterpret_cast<char *>(tex);
    int refcount = *reinterpret_cast<int *>(self + 4) - 1;
    *reinterpret_cast<int *>(self + 4) = refcount;
    if (refcount == 0) {
        int count = *reinterpret_cast<int *>(self + 8);
        int hr = tex->setup_edge(reinterpret_cast<EdgeScan *>(tex), count);
        return hr != 0;
    }

    *reinterpret_cast<int *>(self + 0x10) += *reinterpret_cast<int *>(self + 0x18);
    int sum1c = *reinterpret_cast<int *>(self + 0x1c) + *reinterpret_cast<int *>(self + 0x20);
    *reinterpret_cast<int *>(self + 0xc) += *reinterpret_cast<int *>(self + 0x14);
    *reinterpret_cast<int *>(self + 0x1c) = sum1c;
    int sum28 = *reinterpret_cast<int *>(self + 0x28) + *reinterpret_cast<int *>(self + 0x2c);
    *reinterpret_cast<int *>(self + 0x28) = sum28;
    if (sum28 > 0) {
        *reinterpret_cast<int *>(self + 0x1c) = *reinterpret_cast<int *>(self + 0x24) + sum1c;
        *reinterpret_cast<int *>(self + 0x28) = sum28 - *reinterpret_cast<int *>(self + 0x30);
    }
    return 1;
}
