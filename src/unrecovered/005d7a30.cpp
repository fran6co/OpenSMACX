// ORIGINAL: 0x005D7A30 FILE
// RULED-OUT: plain local `Filemap fm;` with early returns (no SEH
//            prologue at all, 90 vs 170 B). Wrapping in
//            try{}catch(...){throw;} recovers most of the /GX frame
//            shape (179 vs 170 B) but still diverges at #1 push/mov -
//            same wall as 0x004C4BF0's WorldWin ctor.
// working copy - scaffold materialised by --work
// name      sub_5d7a30
// size      188 bytes
// spans     0x005D7A30-0x005D7ADA;0x00662B80-0x00662B92
// prototype 
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628380 0x006283E0 0x00628430

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d7a30  at 0x005D7A30  (188 bytes)
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

class Filemap;
typedef void * HANDLE;
typedef void * LPVOID;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Filemap { public:
    LPVOID map_view_addr_;
    HANDLE file_;
    HANDLE file_map_;
    uint32_t file_size_;
    Filemap();
    int open_read(char *, int);
    ~Filemap();
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00662b88 = (int *)0x00662B88;
static int *const g_0067f740 = (int *)0x0067F740;
extern "C" int __stdcall sub_5d7a30(int a1, int a2, int a3, int a4) {
    try {
        Filemap fm;
        if (a1 == 0) {
            return 3;
        }
        if (fm.open_read((char *)a1, 0) == 0) {
            return 6;
        }
        return 8;
    } catch (...) {
        throw;
    }
}
