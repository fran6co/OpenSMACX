// ORIGINAL: 0x00634C30 ?save@FileBox@@QAEXXZ 0x00634C30-0x00634CEE FILE
// working copy - scaffold materialised by --work
// size      190 bytes
// prototype void (__thiscall ?save@FileBox@@QAEXXZ)(FileBox* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645470 0x006501F6

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?save@FileBox@@QAEXXZ  at 0x00634C30  (190 bytes)
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
extern "C" int __cdecl GetSaveFileNameA();

class FileBox { public:
    void save();
};
struct FileBoxOFN76 {
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

extern "C" int __stdcall GetSaveFileNameA_real(FileBoxOFN76 *);

void FileBox::save() {
    char *self = reinterpret_cast<char *>(this);
    char *fileTitle = self + 0x104;
    char *filter = self + 0x30c;
    if (*fileTitle != 0) {
        *self = 0;
        strcat(self, fileTitle);
    }
    FileBoxOFN76 ofn;
    ofn.lStructSize = 0x4c;
    ofn.hwndOwner = 0;
    ofn.hInstance = 0;
    ofn.lpstrFilter = filter;
    ofn.lpstrCustomFilter = 0;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = self;
    ofn.nMaxFile = 0x104;
    ofn.lpstrFileTitle = fileTitle;
    ofn.nMaxFileTitle = 0x104;
    ofn.lpstrInitialDir = (self[0x418] != 0) ? (self + 0x418) : 0;
    ofn.lpstrTitle = (self[0x414] != 0) ? (self + 0x414) : 0;
    ofn.Flags = 0;
    ofn.nFileOffset = 0xe;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = 0;
    ofn.lCustData = 0;
    ofn.lpfnHook = 0;
    ofn.lpTemplateName = 0;
    GetSaveFileNameA_real(&ofn);
}
