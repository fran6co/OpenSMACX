// ORIGINAL: 0x0055D2A0 FILE
// name      ?enemies_team_up@@YAXHHH@Z
// size      389 bytes
// spans     0x0055D2A0-0x0055D425
// prototype void (__cdecl ?enemies_team_up@@YAXHHH@Z)(int factionID, int factionIDTeam, int factionIDAgainst)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x00539C00 0x0055B870 0x00625EC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0055D2A0
// measured tier  MISMATCH
// divergence     61
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0055d2a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?enemies_team_up@@YAXHHH@Z  at 0x0055D2A0  (389 bytes)
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

class NetMsg;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
int __cdecl great_satan(int, int);
int __cdecl parse_says(int, char *, int, int);
void __cdecl treaty_on(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068e638 = (int *)0x0068E638;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096ca58 = (int *)0x0096CA58;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// Diplomatic per-faction-pair flag table at 0x96C9F8, and the per-faction-pair
// treaty-value table at 0x96CA58: both indexed by a register with a scale-4
// SIB addressing mode and no preceding base-pointer load - the ADDRESS
// itself does work, so each needs `extern T name[]` instead of the
// scaffold's pointer-constant spelling.
extern uint8_t g_0096c9f8_flags[];
extern int g_0096ca58_treaty[];

// The per-faction message-template record array at 0x946D34 (stride 0x59C)
// is likewise reached with the address itself doing the work (`lea` builds
// the record pointer directly rather than loading a base pointer first).
extern char g_00946d34_records[];

// NOT BYTE_EXACT. Closest reached: 381/389 bytes, mnemonic similarity 0.96,
// edit_count 6. One wall, not moved by respelling: the 0x59C-byte record
// stride (`a1 * 0x59c`) strength-reduces to a DIFFERENT lea/shl chain than
// the original's here - tried both a bare multiply and a manual
// `t=a1*5; t+=t*8; t<<=3; t-=a1; t<<=2` decomposition matching the
// original's own instruction order byte-for-byte, and got the identical
// rebuilt bytes either way, so the harness compiler's canonicalization for
// this constant does not depend on source spelling. The logic is otherwise
// complete and verified field-for-field against Ghidra's reconstruction,
// including the three independent `a*0x833+b` pairwise-index formulas
// (0x833 multiplies a different operand in each of the three call sites)
// and the parameter-slot reuse the original makes for `idx1`/`idx2` across
// the `great_satan` calls (this body uses separate locals instead; `a1`/
// `a2`/`a3` stay live for the `treaty_on` calls that need the ORIGINAL
// values, matching where edi/esi are read back late in the disassembly).
void __cdecl enemies_team_up(int a1, int a2, int a3) {
    int idx1 = a3 + a1 * 0x833;
    if ((g_0096c9f8_flags[idx1 * 4] & 0x10) == 0 &&
        (static_cast<uint8_t>(1 << a3) & *reinterpret_cast<uint8_t *>(g_009a64e8)) != 0) {
        int idx2 = a3 + a2 * 0x833;
        if (g_0096c9f8_flags[idx2 * 4] & 0x10) {
            if (great_satan(a3, 0) == 0 && (g_0096c9f8_flags[idx1 * 4] & 0x20) == 0) {
                treaty_on(a1, a3, 0x2000);
                return;
            }
            if (a3 == *g_00939284) {
                char *rec = g_00946d34_records + a1 * 0x59c;
                *g_009bbfec = *reinterpret_cast<int *>(rec + 0x18);
                *g_009bbff0 = *reinterpret_cast<int *>(rec + 0x1c);
                parse_says(0, rec, -1, -1);
                reinterpret_cast<NetMsg *>(g_00805338)->pop(
                    reinterpret_cast<const char *>(g_0068e638), 5000, 0, 0);
            }
            int value = *g_009a64d4;
            g_0096ca58_treaty[a3 * 0x833 + a2] = value;
            g_0096ca58_treaty[a3 * 0x833 + a1] = value;
            treaty_on(a1, a3, 0x10);
            return;
        }
        if (great_satan(a3, 0) == 0 && (g_0096c9f8_flags[idx1 * 4] & 0x20) == 0 &&
            (g_0096c9f8_flags[idx2 * 4] & 0x20) == 0) {
            treaty_on(a1, a3, 0x2000);
            return;
        }
        *reinterpret_cast<uint32_t *>(g_0096c9f8_flags + idx1 * 4) |= 0x400;
    }

}
