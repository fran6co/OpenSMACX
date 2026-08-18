// ORIGINAL: 0x0052A9B0 sub_52a9b0 0x0052A9B0-0x0052AA2D FILE
// RULED-OUT: redeclaring the scaffold's 0-arg `_strcmpi()` with the real (const char*, const char*) signature under extern "C" - C2733, two C-linkage decls of the same name can't overload. Called it through a `StrcmpiFn` function-pointer cast of the existing symbol instead. RegOpenKeyExA/RegQueryValueExA/ RegCloseKey declared `__declspec(dllimport)` (advapi32 IAT shape, same pattern as 0x0051E090's set_tamper). Diverges at instr #3 (original 'push' vs rebuilt 'lea') in the prologue.
// working copy - scaffold materialised by --work
// size      125 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064FD20
// indirect  0x0052A9C9 0x0052A9F8 0x0052AA20

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_52a9b0  at 0x0052A9B0  (125 bytes)
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
extern "C" int __cdecl _strcmpi();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669000 = (int *)0x00669000;
static int *const g_0066900c = (int *)0x0066900C;
static int *const g_00669010 = (int *)0x00669010;
static int *const g_0068be94 = (int *)0x0068BE94;
static int *const g_0068bedc = (int *)0x0068BEDC;
static int *const g_0068bee8 = (int *)0x0068BEE8;
typedef unsigned long DWORD;
typedef long LONG;
typedef void *HKEY;
typedef HKEY *PHKEY;
typedef unsigned char BYTE;
typedef DWORD *LPDWORD;
typedef const char *LPCSTR;

extern "C" __declspec(dllimport) LONG __stdcall RegOpenKeyExA(
    HKEY, LPCSTR, DWORD, DWORD, PHKEY);
extern "C" __declspec(dllimport) LONG __stdcall RegQueryValueExA(
    HKEY, LPCSTR, LPDWORD, LPDWORD, BYTE *, LPDWORD);
extern "C" __declspec(dllimport) LONG __stdcall RegCloseKey(HKEY);

extern "C" int __cdecl sub_52a9b0() {
    typedef int (__cdecl *StrcmpiFn)(const char *, const char *);
    HKEY key;
    LONG status = RegOpenKeyExA(reinterpret_cast<HKEY>(0x80000002),
        reinterpret_cast<LPCSTR>(g_0068be94), 0, 0, &key);
    if (status != 0) {
        return 0;
    }
    int result = 0;
    DWORD valueType;
    char valueBuf[12];
    DWORD valueSize = 10;
    status = RegQueryValueExA(key, reinterpret_cast<LPCSTR>(g_0068bedc), 0,
        &valueType, reinterpret_cast<BYTE *>(valueBuf), &valueSize);
    if (status == 0) {
        StrcmpiFn fn = reinterpret_cast<StrcmpiFn>(_strcmpi);
        if (fn(valueBuf, reinterpret_cast<const char *>(g_0068bee8)) == 0) {
            result = 1;
        }
    }
    RegCloseKey(key);
    return result;
}
