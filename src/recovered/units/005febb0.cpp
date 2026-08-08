// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005FEBB0
// name           ?init_palette_class@Palette@@QAAXH@Z
// size           320 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005febb0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init_palette_class@Palette@@QAAXH@Z  at 0x005FEBB0  (320 bytes)
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

class Palette;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void free(void *);
void * __cdecl mem_get(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669058 = (int *)0x00669058;
static int *const g_006690a4 = (int *)0x006690A4;
static int *const g_006690ac = (int *)0x006690AC;
static int *const g_0066927c = (int *)0x0066927C;
static int *const g_00669280 = (int *)0x00669280;
static int *const g_0067022d = (int *)0x0067022D;
static int *const g_0067024f = (int *)0x0067024F;
static int *const g_00670250 = (int *)0x00670250;
static int *const g_00670251 = (int *)0x00670251;
static int *const g_00670255 = (int *)0x00670255;
static int *const g_009b8178 = (int *)0x009B8178;
static int *const g_009b8188 = (int *)0x009B8188;

class Palette { public:
    void __cdecl init_palette_class(int);
};

// GDI/USER32 imports through the fixed IAT slots the emitter already gave us
// (g_00669058 etc hold the import thunk address) - call through them as
// stdcall function pointers rather than declaring the real headers.
typedef int BOOL;
typedef void *HGDIOBJ;
typedef void *HPALETTE;
typedef void *HWND;
typedef void *HDC;
struct PALETTEENTRY { unsigned char peRed, peGreen, peBlue, peFlags; };

typedef BOOL(__stdcall *DeleteObjectFn)(HGDIOBJ);
typedef HPALETTE(__stdcall *CreatePaletteFn)(const void *);
typedef HDC(__stdcall *GetDCFn)(HWND);
typedef int(__stdcall *ReleaseDCFn)(HWND, HDC);
typedef unsigned int(__stdcall *GetSystemPaletteEntriesFn)(HDC, unsigned int, unsigned int, PALETTEENTRY *);

void __cdecl Palette::init_palette_class(int a1) {
    int hpal = *g_009b8178;
    if (hpal != 0) {
        (*reinterpret_cast<DeleteObjectFn *>(g_00669058))(reinterpret_cast<HGDIOBJ>(hpal));
        *g_009b8178 = 0;
    }

    char *p = reinterpret_cast<char *>(mem_get(0x404));
    if (p != 0) {
        *reinterpret_cast<uint16_t *>(p) = 0x300;
        *reinterpret_cast<uint16_t *>(p + 2) = 0x100;
        *g_009b8188 = a1;
        if (a1 != 0) {
            HDC hdc = (*reinterpret_cast<GetDCFn *>(g_0066927c))(0);
            (*reinterpret_cast<GetSystemPaletteEntriesFn *>(g_006690ac))(
                hdc, 0, 0x100, reinterpret_cast<PALETTEENTRY *>(p + 4));
            (*reinterpret_cast<ReleaseDCFn *>(g_00669280))(0, hdc);
        } else {
            int off1 = reinterpret_cast<char *>(g_00670251) - p;
            int off2 = reinterpret_cast<char *>(g_00670250) - p;
            char *pBVar2 = reinterpret_cast<char *>(g_0067022d);
            char *pBVar1 = p + 5;
            int off3 = reinterpret_cast<char *>(g_0067024f) - p;
            char *pBVar3;
            do {
                pBVar3 = pBVar2 + 4;
                pBVar1[-1] = pBVar2[1];
                *pBVar1 = *pBVar2;
                pBVar1[1] = pBVar2[-1];
                pBVar1[2] = 0;
                pBVar1[0x3d7] = pBVar1[off1];
                pBVar1[0x3d8] = pBVar1[off2];
                pBVar1[0x3d9] = pBVar1[off3];
                pBVar1[0x3da] = 0;
                pBVar1 += 4;
                pBVar2 = pBVar3;
            } while (pBVar3 < reinterpret_cast<char *>(g_00670255));

            char *pBVar4 = p + 0x2f;
            int count = 0xec;
            do {
                *pBVar4 = 5;
                pBVar4 += 4;
            } while (--count != 0);

            int iVar4 = 0;
            char *cursor = p + 0x25;
            do {
                unsigned char dl = static_cast<unsigned char>(iVar4);
                cursor += 4;
                dl += 8;
                cursor[-5] = dl;
                dl = static_cast<unsigned char>(iVar4);
                cursor[-4] = 0;
                cursor[-3] = 0;
                dl -= 0xb;
                cursor[-2] = 2;
                cursor[0x3b3] = dl;
                cursor[0x3b4] = 0;
                cursor[0x3b5] = 0;
                cursor[0x3b6] = 2;
                iVar4++;
            } while (iVar4 < 2);
        }

        *g_009b8178 = reinterpret_cast<int>(
            (*reinterpret_cast<CreatePaletteFn *>(g_006690a4))(p));
        free(p);
    }
}
