// ORIGINAL: 0x00588F90 FILE
// name      ?alien_start@@YAXXZ
// size      492 bytes
// spans     0x00588F90-0x0058917C
// prototype 
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E5250 0x005C08C0 0x00645470 0x00645660
// indirect  0x00588FF1 0x00589078
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00588F90
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00588f90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?alien_start@@YAXXZ  at 0x00588F90  (492 bytes)
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
extern "C" char *strcat(char *, const char *);
extern "C" int strcmp(const char *, const char *);
void __cdecl base_kill(int);
void __cdecl veh_kill(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_0068f534 = (int *)0x0068F534;
static int *const g_0068f53c = (int *)0x0068F53C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_009474f4 = (int *)0x009474F4;
static int *const g_00949c38 = (int *)0x00949C38;
static int *const g_00952836 = (int *)0x00952836;
static int *const g_0096eaac = (int *)0x0096EAAC;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a64e9 = (int *)0x009A64E9;
static int *const g_009b86a0 = (int *)0x009B86A0;

typedef char *LPSTR;

// INDEXED TABLE BASE: 0x946a54/0x946f58 never appear as bare immediates in
// the disassembly, always as [reg + imm] with a runtime-computed offset, so
// the emitter left them out of the fixed-globals list above. A Faction-like
// struct, stride 0x59c: name buffer at offset 0, active flag byte at 0x504.
extern unsigned char g_00946a54_tbl[];

void __cdecl alien_start() {
    typedef LPSTR(__stdcall * CharUpperFn)(LPSTR);

    if (*g_009a64d4 != 0) return;
    if (*g_009a6488 == 0) return;

    int off = *g_00939284 * 0x59c;
    if ((g_00946a54_tbl[off + 0x504] & 0x80) != 0) return;

    *reinterpret_cast<char *>(g_009b86a0) = 0;
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00946a54_tbl + off));
    reinterpret_cast<CharUpperFn>(*g_0066931c)(reinterpret_cast<LPSTR>(g_009b86a0));
    if (strcmp(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_0068f534)) == 0) return;

    unsigned char *flagp = reinterpret_cast<unsigned char *>(g_009474f4);
    int *fac = g_0096eaac;
    unsigned idx = 1;

loop_top:
    {
        unsigned bit = 1u << idx;
        if ((*reinterpret_cast<unsigned char *>(g_009a64e8) & bit) != 0) goto loop_tail;
        if (*g_0093a95c != 0) goto loop_tail;
        if (*g_0093f660 != 0) goto loop_tail;

        *reinterpret_cast<char *>(g_009b86a0) = 0;
        strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(flagp - 0x504));
        reinterpret_cast<CharUpperFn>(*g_0066931c)(reinterpret_cast<LPSTR>(g_009b86a0));
        if ((*flagp & 0x80) == 0 &&
            strcmp(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_0068f53c)) != 0)
            goto loop_tail;

        unsigned char mask = ~static_cast<unsigned char>(bit);
        *reinterpret_cast<unsigned char *>(g_009a64e8) &= mask;
        *reinterpret_cast<unsigned char *>(g_009a64e9) &= mask;

        int i = *g_009a64cc - 1;
        if (i >= 0) {
            unsigned char *pb = reinterpret_cast<unsigned char *>(g_0097d044) + i * 0x134;
            do {
                if (*pb == idx) {
                    base_kill(i);
                }
                i--;
                pb -= 0x134;
            } while (i >= 0);
        }

        i = *g_009a64c8 - 1;
        if (i >= 0) {
            unsigned char *pb = reinterpret_cast<unsigned char *>(g_00952836) + i * 0x34;
            do {
                if (i < *g_009a64c8 && *pb == idx) {
                    veh_kill(i);
                }
                i--;
                pb -= 0x34;
            } while (i >= 0);
        }

        fac[0x417] = 0;
        *fac = 0xffffffff;
        fac[2] = 0xffffffff;
    }
loop_tail:
    flagp += 0x59c;
    idx++;
    fac += 0x833;
    if (flagp < reinterpret_cast<unsigned char *>(0x949c38)) goto loop_top;
}
