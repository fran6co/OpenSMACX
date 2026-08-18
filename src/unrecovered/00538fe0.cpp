// ORIGINAL: 0x00538FE0 ?checksum_file@@YAEPAD@Z 0x00538FE0-0x0053908F;0x0065E450-0x0065E462 FILE
// working copy - scaffold materialised by --work
// size      193 bytes
// prototype 
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00628380 0x006283E0 0x00628430 0x006287C0
// indirect  0x0053904A

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?checksum_file@@YAEPAD@Z  at 0x00538FE0  (193 bytes)
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
    void close();
    ~Filemap();
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0053905d = (int *)0x0053905D;
static int *const g_0065e458 = (int *)0x0065E458;
static int *const g_00669130 = (int *)0x00669130;
static int *const g_0067c5a0 = (int *)0x0067C5A0;
extern "C" unsigned long __stdcall GetFileSize(void *, unsigned long *);

unsigned char __cdecl checksum_file(char * a1) {
    Filemap fm;
    char *data = (char *)fm.open_read(a1, 1);
    if (data == 0) {
        return 0;
    }
    unsigned int size = GetFileSize(fm.file_, 0);
    unsigned char sum = 0;
    do {
        sum ^= *data;
        --size;
        ++data;
    } while (size != 0);
    fm.close();
    return sum;
}
