// ORIGINAL: 0x004CD4E0 FILE
// name      ?action_oblit@@YAXHH@Z
// size      445 bytes
// spans     0x004CD4E0-0x004CD69D
// prototype void (__cdecl ?action_oblit@@YAXHH@Z)(int vehID, int baseID)
// callers   3   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046B190 0x0048C0A0 0x004E5250 0x0055EEE0 0x00625E30 0x00625EC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004CD4E0
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004cd4e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?action_oblit@@YAXHH@Z  at 0x004CD4E0  (445 bytes)
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
int __cdecl parse_num(int, int);
int __cdecl parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
void __cdecl atrocity(int, int, int, int);
void __cdecl base_kill(int);
void __cdecl draw_map(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00687ffc = (int *)0x00687FFC;
static int *const g_0068800c = (int *)0x0068800C;
static int *const g_00688018 = (int *)0x00688018;
static int *const g_00688028 = (int *)0x00688028;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946d54 = (int *)0x00946D54;
static int *const g_00949860 = (int *)0x00949860;
static int *const g_00952836 = (int *)0x00952836;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// The emitter left five `DAT_0097dXXX` fields and three `DAT_00946aXX`/
// `DAT_00946d54` table addresses out of the fixed-globals list entirely -
// every one of them is read through a per-call computed row/byte offset
// (`[edi+0x97d046]`, `[eax+0x946a50]`, `[ecx*4+0x946d54]`, ...), the
// INDEXED TABLE BASE shape, so each gets its own `extern T name[]` here
// rather than the const-pointer form the emitter does supply for the
// plain-load globals.
extern unsigned char g_00952836_tbl[];
extern unsigned char g_0097d044_tbl[];
extern unsigned char g_0097d045_tbl[];
extern signed char g_0097d046_tbl[];
extern unsigned char g_0097d04a_tbl[];
extern char g_0097d053_tbl[];
extern int g_00946a50_tbl[];
extern unsigned char g_00946a84_tbl[];
extern unsigned char g_00946a9c_tbl[];
extern unsigned char g_00946d54_tbl[];

// MISMATCH (#2: original 'mov' vs rebuilt 'push' - the original loads a1
// into eax immediately after the frame is set up, BEFORE saving
// ebx/esi/edi; this rebuild pushes a callee-saved register first instead.
// Everything downstream of the prologue lines up well (mnemonic
// similarity 0.90, edit_count 15 of 138, identical total byte count -
// 445 both sides). Two structural findings worth recording as fixes, not
// walls: `a1` is genuinely REASSIGNED mid-function (`a1 = uVar2;`) -
// the original reuses its own `[ebp+8]` argument slot as a variable's
// home, confirmed because the final `uVar2 != g_00939284` comparison
// reads that same stack slot; and `uVar3`'s row (`uVar3 * 0x167`) is
// computed ONCE and reused across the 3rd and 4th `parse_says` calls
// without recomputing, matching a single named local reused across two
// statements rather than two independent expressions. The prologue
// register-save ORDER itself is the one piece left unmatched -
// REGISTER ALLOCATION, not chased further.
void __cdecl action_oblit(int a1, int a2) {
    unsigned uVar3 = g_00952836_tbl[a1 * 0x34];
    int iVar4 = a2 * 0x134;
    unsigned uVar2 = g_0097d045_tbl[iVar4];
    parse_num(0, static_cast<int>(g_0097d046_tbl[iVar4]) * 10);
    a1 = uVar2;
    parse_says(0, g_0097d053_tbl + iVar4, -1, -1);

    int G = *g_00939284;
    *g_009bbff0 = 0;
    int row1 = G * 0x167;
    *g_009bbfec = g_00946a50_tbl[row1];
    parse_says(1, reinterpret_cast<char *>(g_00946a9c_tbl + row1 * 4), -1, -1);

    int row2 = static_cast<int>(uVar2) * 0x167;
    parse_says(2, reinterpret_cast<char *>(g_00946d54_tbl + row2 * 4), -1, -1);

    *g_009bbff0 = 0;
    int row3 = static_cast<int>(uVar3) * 0x167;
    *g_009bbfec = g_00946a50_tbl[row3];
    parse_says(3, reinterpret_cast<char *>(g_00946a9c_tbl + row3 * 4), -1, -1);

    *g_009bbfec = g_00946a50_tbl[row3];
    *g_009bbff0 = 0;
    parse_says(4, reinterpret_cast<char *>(g_00946a84_tbl + row3 * 4), -1, -1);

    bool cond;
    if (g_0097d044_tbl[iVar4] == *g_00939284) {
        cond = true;
    } else if ((g_0097d04a_tbl[iVar4] & (1 << (*g_00939284 & 0x1f))) != 0) {
        cond = true;
    } else {
        cond = false;
    }

    base_kill(a2);
    draw_map(1);

    int G2 = *g_00939284;
    if (static_cast<int>(uVar3) == G2) {
        popp(reinterpret_cast<char *>(*g_00691b0c), reinterpret_cast<const char *>(g_0068800c), 0,
             reinterpret_cast<const char *>(g_00687ffc), 0);
    } else if (static_cast<int>(uVar2) == G2 || cond) {
        popp(reinterpret_cast<char *>(*g_00691b0c), reinterpret_cast<const char *>(g_00688028), 0,
             reinterpret_cast<const char *>(g_00688018), 0);
    }

    if (*g_00949860 != 0) {
        atrocity(uVar3, uVar2, 2, 0);
    }
}
