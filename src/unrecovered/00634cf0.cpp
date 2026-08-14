// ORIGINAL: 0x00634CF0 FILE
// RULED-OUT: void return kept per mangled QAEXXZ (tail neg/sbb/and masked-eax
//            not reproduced); own OpenFileNameShim struct (classic 0x4C
//            OPENFILENAMEA layout) + differently-named GetOpenFileNameA_real
// working copy - scaffold materialised by --work
// name      ?load@FileBox@@QAEXXZ
// size      190 bytes
// spans     0x00634CF0-0x00634DAE
// prototype void (__thiscall ?load@FileBox@@QAEXXZ)(FileBox* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645470 0x006501FC

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load@FileBox@@QAEXXZ  at 0x00634CF0  (190 bytes)
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

class FileBox;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl GetOpenFileNameA();

class FileBox { public:
    void load();
};

struct OpenFileNameShim {
    unsigned int lStructSize;
    void *hwndOwner;
    void *hInstance;
    const char *lpstrFilter;
    char *lpstrCustomFilter;
    unsigned int nMaxCustFilter;
    unsigned int nFilterIndex;
    char *lpstrFile;
    unsigned int nMaxFile;
    char *lpstrFileTitle;
    unsigned int nMaxFileTitle;
    const char *lpstrInitialDir;
    const char *lpstrTitle;
    unsigned int Flags;
    unsigned short nFileOffset;
    unsigned short nFileExtension;
    const char *lpstrDefExt;
    long lCustData;
    void *lpfnHook;
    const char *lpTemplateName;
};

extern "C" int __stdcall GetOpenFileNameA_real(OpenFileNameShim *);

void FileBox::load() {
    char *self = reinterpret_cast<char *>(this);
    char *filter = self + 0x30c;
    char *file = self + 0x104;
    if (*file != '\0') {
        *self = '\0';
        strcat(self, file);
    }
    OpenFileNameShim ofn;
    ofn.lStructSize = 0x4c;
    ofn.hwndOwner = 0;
    ofn.hInstance = 0;
    ofn.lpstrCustomFilter = 0;
    ofn.nMaxCustFilter = 0;
    ofn.lpstrFilter = filter;
    ofn.nMaxFile = 0x104;
    ofn.nMaxFileTitle = 0x104;
    ofn.lpstrFile = self;
    ofn.lpstrFileTitle = file;
    ofn.lpstrInitialDir = (self[0x418] != '\0') ? (self + 0x418) : 0;
    ofn.lpstrTitle = 0;
    ofn.Flags = 0x180e;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = (self[0x414] != '\0') ? (self + 0x414) : 0;
    ofn.lCustData = 0;
    ofn.lpfnHook = 0;
    ofn.lpTemplateName = 0;
    GetOpenFileNameA_real(&ofn);
}
