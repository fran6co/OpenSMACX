// ORIGINAL: 0x0051E110 FILE
// name      ?check_tamper@@YAXXZ
// size      635 bytes
// spans     0x0051E110-0x0051E38B
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF310
// indirect  0x0051E153 0x0051E181 0x0051E1A2 0x0051E1A8 0x0051E26B 0x0051E28E 0x0051E2A7 0x0051E2AD 0x0051E32D 0x0051E350 0x0051E369 0x0051E36F
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0051E110
// measured tier  MISMATCH
// divergence     26
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0051e110/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?check_tamper@@YAXXZ  at 0x0051E110  (635 bytes)
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
int __cdecl X_pop(const char *, int (__cdecl *)());

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669000 = (int *)0x00669000;
static int *const g_00669004 = (int *)0x00669004;
static int *const g_00669008 = (int *)0x00669008;
static int *const g_00669010 = (int *)0x00669010;
static int *const g_0068ad88 = (int *)0x0068AD88;
static int *const g_0068add0 = (int *)0x0068ADD0;
static int *const g_0068add4 = (int *)0x0068ADD4;
static int *const g_0068add8 = (int *)0x0068ADD8;
static int *const g_0068ae20 = (int *)0x0068AE20;
static int *const g_0068ae24 = (int *)0x0068AE24;
static int *const g_0068ae28 = (int *)0x0068AE28;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a060 = (int *)0x0093A060;
static int *const g_0093a064 = (int *)0x0093A064;
static int *const g_0093a070 = (int *)0x0093A070;
static int *const g_0093a514 = (int *)0x0093A514;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0093a9ac = (int *)0x0093A9AC;
static int *const g_0093a9b0 = (int *)0x0093A9B0;
static int *const g_0093a9b4 = (int *)0x0093A9B4;
static int *const g_0093a9f8 = (int *)0x0093A9F8;
static int *const g_0093a9fc = (int *)0x0093A9FC;
static int *const g_0093aa00 = (int *)0x0093AA00;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a64e9 = (int *)0x009A64E9;

typedef int (__stdcall *RegCreateKeyExA_fn)(int, const char *, int, void *, int, int, void *, int *, void *);
typedef int (__stdcall *RegQueryValueExA_fn)(int, const char *, void *, int *, void *, int *);
typedef int (__stdcall *RegSetValueExA_fn)(int, const char *, int, int, void *, int);
typedef int (__stdcall *RegCloseKey_fn)(int);

struct TamperRecord {
    int a;
    int b;
    int c;
    int d;
    int count;
    int e;
};

extern const char g_0068ad88_str[];
extern const char g_0068add0_str[];
extern const char g_0068add4_str[];
extern const char g_0068add8_str[];
extern const char g_0068ae20_str[];
extern const char g_0068ae24_str[];
extern const char g_0068ae28_str[];
extern int g_0093a9fc_buf[];
extern int g_0093aa00_buf[];
extern int g_0093a9f8_val;
extern TamperRecord g_records[];
extern int g_0093a9bc_arr[];
extern int g_0093a9dc_arr[];

void __cdecl check_tamper() {
    if (*g_0093a95c != 0 && *g_0093a9ac != 0) {
        int hKey;
        int dwType;
        int cbData;

        int status = (*reinterpret_cast<RegCreateKeyExA_fn *>(g_00669004))(
            0x80000002, g_0068ad88_str, 0, g_0093a9fc_buf, 0, 0xf003f, 0, &hKey, 0);
        if (status == 0) {
            cbData = 4;
            (*reinterpret_cast<RegQueryValueExA_fn *>(g_00669000))(
                hKey, g_0068add0_str, 0, &dwType, &g_0093a9f8_val, &cbData);
            cbData = 0x4b0;
            (*reinterpret_cast<RegQueryValueExA_fn *>(g_00669000))(
                hKey, g_0068add4_str, 0, &dwType, g_records, &cbData);
            (*reinterpret_cast<RegCloseKey_fn *>(g_00669010))(hKey);
        }

        int edx_val = *g_0093a9ac;
        int esi_val = *g_0093a9b0;
        int edi_val = *g_009a64d4;
        int iVar6 = 0;
        bool matched = false;
        for (; iVar6 < 50; iVar6++) {
            TamperRecord *rec = &g_records[iVar6];
            if (edx_val == rec->a && esi_val == rec->b && edi_val == rec->c &&
                *g_00939284 == rec->d && *g_0093a9b4 == rec->e) {
                matched = true;
                break;
            }
        }

        if (matched) {
            TamperRecord *rec = &g_records[iVar6];
            int newCount = rec->count + 1;
            rec->count = newCount;

            int edi2 = *g_00939284;
            int esi2 = 1;
            int edx2 = 0;
            do {
                if (esi2 != edi2) {
                    unsigned char bit = (unsigned char)(1 << (esi2 & 0x1f));
                    if ((*reinterpret_cast<uint8_t *>(g_009a64e8) & bit) != 0 &&
                        (*reinterpret_cast<uint8_t *>(g_009a64e9) & bit) != 0) {
                        g_0093a9bc_arr[edx2 / 4] = edi2;
                        g_0093a9dc_arr[edx2 / 4] = newCount;
                    }
                }
                edx2 += 4;
                esi2++;
            } while (edx2 < 0x1c);

            int status2 = (*reinterpret_cast<RegCreateKeyExA_fn *>(g_00669004))(
                0x80000002, g_0068add8_str, 0, g_0093aa00_buf, 0, 0xf003f, 0, &hKey, 0);
            if (status2 == 0) {
                (*reinterpret_cast<RegSetValueExA_fn *>(g_00669008))(hKey, g_0068ae20_str, 0, 3, &g_0093a9f8_val, 4);
                (*reinterpret_cast<RegSetValueExA_fn *>(g_00669008))(hKey, g_0068ae24_str, 0, 3, g_records, 0x4b0);
                (*reinterpret_cast<RegCloseKey_fn *>(g_00669010))(hKey);
            }
            X_pop(g_0068ae28_str, 0);
            return;
        }

        int idx = g_0093a9f8_val;
        TamperRecord *newRec = &g_records[idx];
        newRec->a = edx_val;
        newRec->b = esi_val;
        newRec->c = edi_val;
        newRec->d = *g_00939284;
        newRec->count = 0;
        newRec->e = *g_0093a9b4;
        g_0093a9f8_val = (idx + 1) % 50;

        int status3 = (*reinterpret_cast<RegCreateKeyExA_fn *>(g_00669004))(
            0x80000002, g_0068add8_str, 0, g_0093aa00_buf, 0, 0xf003f, 0, &hKey, 0);
        if (status3 == 0) {
            (*reinterpret_cast<RegSetValueExA_fn *>(g_00669008))(hKey, g_0068ae20_str, 0, 3, &g_0093a9f8_val, 4);
            (*reinterpret_cast<RegSetValueExA_fn *>(g_00669008))(hKey, g_0068ae24_str, 0, 3, g_records, 0x4b0);
            (*reinterpret_cast<RegCloseKey_fn *>(g_00669010))(hKey);
        }
    }
}
