// ORIGINAL: 0x006283E0 ??1Filemap@@QAE@XZ 0x006283E0-0x00628423 FILE BYTE_EXACT
// size      67 bytes
// prototype void (__thiscall ??1Filemap@@QAE@XZ)(Filemap* this)
// callers   15   call targets   0
// kind      
// flags     
// calls     (none)
// indirect  0x006283EB 0x00628405 0x00628417
// notes     Staged hybrid export redirect calls the source-owned destructor
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1Filemap@@QAE@XZ  at 0x006283E0  (67 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066914c = (int *)0x0066914C;
static int *const g_00669150 = (int *)0x00669150;

class Filemap { public:
    LPVOID map_view_addr_;
    HANDLE file_;
    HANDLE file_map_;
    uint32_t file_size_;

    ~Filemap();
};
Filemap::~Filemap() {
    typedef void (__stdcall *UnmapViewFn)(void *);
    typedef void (__stdcall *CloseHandleFn)(void *);

    if (map_view_addr_) {
        ((UnmapViewFn)(*(void **)0x00669150))(map_view_addr_);
        map_view_addr_ = 0;
    }
    CloseHandleFn close_handle = (CloseHandleFn)(*(void **)0x0066914C);
    if (file_map_) {
        close_handle(file_map_);
        file_map_ = 0;
    }
    if (file_ != (HANDLE)-1) {
        close_handle(file_);
        file_ = 0;
    }
}
