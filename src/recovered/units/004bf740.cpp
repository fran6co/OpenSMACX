// ORIGINAL: 0x004BF740 FILE
// name      ?compute_camera@@YAXPAUVOX_Vect@@PAUVOX_Matrix@@@Z
// size      385 bytes
// spans     0x004BF740-0x004BF8C1
// prototype 
// callers   4   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00627D00 0x00627E20 0x006280E0 0x006281E0 0x00646664
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004BF740
// measured tier  MISMATCH
// divergence     18
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004bf740/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?compute_camera@@YAXPAUVOX_Vect@@PAUVOX_Matrix@@@Z  at 0x004BF740  (385 bytes)
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

struct VOX_Matrix;
struct VOX_Vect;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl atan();
extern "C" int __cdecl sub_627d00();
extern "C" int __cdecl sub_627e20();
extern "C" int __cdecl sub_6280e0();
extern "C" int __cdecl sub_6281e0();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066df08 = (int *)0x0066DF08;
static int *const g_008e9f40 = (int *)0x008E9F40;
static int *const g_008e9f44 = (int *)0x008E9F44;
static int *const g_008e9f48 = (int *)0x008E9F48;

struct LocalVec3 { float x, y, z; };
struct LocalMat36 { float f[9]; };

typedef void(__cdecl *Fn_627d00)(LocalVec3 *, LocalMat36 *);
typedef void(__cdecl *Fn_6280e0)(LocalMat36 *, VOX_Vect *, LocalVec3 *);
typedef float(__cdecl *Fn_6281e0)(LocalVec3 *);
typedef void(__cdecl *Fn_627e20)(LocalMat36 *, LocalMat36 *, VOX_Matrix *);
typedef double(__cdecl *Fn_atan)(double);

#define SUB_627D00 (reinterpret_cast<Fn_627d00>(&sub_627d00))
#define SUB_6280E0 (reinterpret_cast<Fn_6280e0>(&sub_6280e0))
#define SUB_6281E0 (reinterpret_cast<Fn_6281e0>(&sub_6281e0))
#define SUB_627E20 (reinterpret_cast<Fn_627e20>(&sub_627e20))
#define ATAN (reinterpret_cast<Fn_atan>(&atan))

void __cdecl compute_camera(VOX_Vect * a1, VOX_Matrix * a2) {
    LocalVec3 vecA;
    LocalMat36 matrixTemp1;
    LocalMat36 matrixTemp2;
    LocalVec3 outVec1;
    LocalVec3 copyVec;

    vecA.x = 0.0f;
    vecA.y = 0.0f;
    vecA.z = -1.0466667f;
    SUB_627D00(&vecA, &matrixTemp1);

    SUB_6280E0(&matrixTemp1, a1, &outVec1);

    copyVec.x = outVec1.x;
    copyVec.y = outVec1.y;
    copyVec.z = outVec1.z;

    if (outVec1.x == *reinterpret_cast<float *>(g_0066df08)) {
        vecA.x = 0.0f;
    } else if (outVec1.y == *reinterpret_cast<float *>(g_0066df08)) {
        vecA.x = 1.57f;
    } else {
        vecA.x = (float)ATAN((double)(outVec1.x / outVec1.y));
    }

    vecA.y = 0.0f;
    *reinterpret_cast<float *>(g_008e9f40) = vecA.x;
    *reinterpret_cast<float *>(g_008e9f44) = vecA.y;
    vecA.z = 0.0f;
    *reinterpret_cast<float *>(g_008e9f48) = vecA.z;
    SUB_627D00(&vecA, &matrixTemp1);

    copyVec.x = outVec1.x;
    copyVec.y = outVec1.y;
    copyVec.z = 0.0f;
    float scratch = SUB_6281E0(&copyVec);

    if (scratch == *reinterpret_cast<float *>(g_0066df08)) {
        vecA.z = 0.0f;
    } else if (outVec1.z == *reinterpret_cast<float *>(g_0066df08)) {
        vecA.z = 1.57f;
    } else {
        vecA.z = (float)ATAN((double)(scratch / outVec1.z));
    }

    vecA.x = 0.0f;
    vecA.y = 0.0f;
    SUB_627D00(&vecA, &matrixTemp2);

    SUB_627E20(&matrixTemp1, &matrixTemp2, a2);

    char *out = reinterpret_cast<char *>(a2);
    *reinterpret_cast<float *>(out + 0x18) = -*reinterpret_cast<float *>(out + 0x18);
    *reinterpret_cast<float *>(out + 0x1c) = -*reinterpret_cast<float *>(out + 0x1c);
    *reinterpret_cast<float *>(out + 0x20) = -*reinterpret_cast<float *>(out + 0x20);
}
