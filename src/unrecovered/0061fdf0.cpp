// ORIGINAL: 0x0061FDF0 sub_61fdf0 0x0061FDF0-0x0061FE68 FILE
// TRIED: calling through the scaffold's `Texture::setup_edge_gouraud` member (thiscall, this-in-ECX) - the disasm shows two bare pushes and no ECX setup, so a free __cdecl decl was used instead; MISMATCH is at #20 (near the tail), not tried further.
// working copy - scaffold materialised by --work
// size      120 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0061FC90

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_61fdf0  at 0x0061FDF0  (120 bytes)
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
    int setup_edge_gouraud(EdgeScan *, int);
};

extern "C" int __cdecl setup_edge_gouraud_raw(int, int);

extern "C" int __cdecl sub_61fdf0(int param_1) {
    int *base = reinterpret_cast<int *>(param_1);
    int count = base[1] - 1;
    base[1] = count;
    if (count == 0) {
        int result = setup_edge_gouraud_raw(param_1, base[2]);
        return result != 0;
    }
    base[4] += base[6];
    int t = base[7] + base[8];
    base[3] += base[5];
    base[7] = t;
    base[0xd] += base[0xe];
    int t2 = base[0xa] + base[0xb];
    base[0xa] = t2;
    if (t2 > 0) {
        base[7] = base[9] + t;
        base[0xa] = t2 - base[0xc];
    }
    return 1;
}
