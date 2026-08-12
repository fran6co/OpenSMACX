// ORIGINAL: 0x00477210 BYTE_EXACT FILE
// name      ?mon_enemy_destroyed@@YAXHH@Z
// size      272 bytes
// spans     0x00477210-0x00477320
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00476A50 0x005C89A0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00477210
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00477210/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mon_enemy_destroyed@@YAXHH@Z  at 0x00477210  (272 bytes)
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
int __cdecl game_year(int);
void __cdecl monument(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0094cb68 = (int *)0x0094CB68;
static int *const g_0094cb6c = (int *)0x0094CB6C;
static int *const g_0094f308 = (int *)0x0094F308;
static int *const g_0094f30c = (int *)0x0094F30C;
static int *const g_009a64d4 = (int *)0x009A64D4;

// Indexed table bases: the address itself does work (lea-chain scaling by
// the non-power-of-two record stride), so these need extern arrays rather
// than the scaffold's pointer constants above (which would let /O2 fold the
// literal address away). New symbol names avoid colliding with the
// scaffold's own g_0094cbXX declarations; the relocation operand is masked
// by the comparison, so the numeric value these bind to does not matter.
struct MonFlags {
    uint8_t flag;
    char pad[0x59C - 1];
};
extern MonFlags g_monflags[];

struct MonRecord {
    int year;            // -> 0x94cb58
    char pad_04[0x08 - 0x04];
    int target;            // -> 0x94cb60
    int attacker;             // -> 0x94cb64
    int active;                  // -> 0x94cb68
    int count;                      // -> 0x94cb6c
    char pad_18[0x348 - 0x18];
    int state;                         // -> 0x94cea0
    char pad_end[0x4F4 - 0x348 - 4];
};
extern MonRecord g_monrec[];

void __cdecl mon_enemy_destroyed(int a1, int a2) {
    int year = game_year(*g_009a64d4);
    int off = a1 * 0x4f4;
    if (!(g_monflags[a1].flag & 0x80) && g_monrec[a1].active == 0 && a2 != 0) {
        g_monrec[a1].count = 0;
        for (int i = 0; i < 8; i++) {
            if (i != a1 && g_monrec[i].active != 0) {
                g_monrec[a1].count++;
            }
        }
        int winner;
        if (g_monrec[a1].count != 0) {
            g_monrec[a1].count = 0;
            winner = a2;
            for (int j = 0; j < 8; j++) {
                if (j != a1 && g_monrec[j].active != 0 && g_monrec[j].count != 0) {
                    winner = j;
                }
            }
        } else {
            g_monrec[a1].count = 1;
            winner = a1;
        }
        g_monrec[a1].active = 1;
        *reinterpret_cast<int *>(reinterpret_cast<char *>(g_monrec) + off) = year;
        g_monrec[a1].attacker = winner;
        g_monrec[a1].target = a2;
        g_monrec[a1].state = 6;
        if (a1 == *g_00939284) {
            monument(1);
        }
    }
}
