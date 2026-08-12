// ORIGINAL: 0x00592940 FILE
// name      ?fixup_landmarks@@YAXXZ
// size      308 bytes
// spans     0x00592940-0x00592A74
// prototype 
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00592940
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00592940/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?fixup_landmarks@@YAXXZ  at 0x00592940  (308 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068577c = (int *)0x0068577C;
static int *const g_006857f0 = (int *)0x006857F0;
static int *const g_00685864 = (int *)0x00685864;
static int *const g_0068593c = (int *)0x0068593C;
static int *const g_006859b0 = (int *)0x006859B0;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_007d3a78 = (int *)0x007D3A78;
static int *const g_007d3b74 = (int *)0x007D3B74;
static int *const g_007f64e8 = (int *)0x007F64E8;
static int *const g_007f655c = (int *)0x007F655C;
static int *const g_007f65d0 = (int *)0x007F65D0;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094a30c = (int *)0x0094A30C;

extern "C" void *memcpy(void *, const void *, unsigned int);

// Indexed table bases - extern arrays, not the const-pointer spelling.
extern int32_t g_94a30c_records[];  // landmark records, 11 ints (44 B) each; flags at +0xc
extern uint8_t g_685684_tbl[];      // 5-byte digit table, indexed by typeId * 5
extern uint8_t g_7d3b74_flags[];    // per-value flag array, cleared
extern uint8_t g_7f65d0_flags[];    // per-value flag array, set from the last digit read

void __cdecl fixup_landmarks() {
    memcpy(g_007f64e8, g_006859b0, 113);
    memcpy(g_007d3a78, g_00685864, 113);
    memcpy(g_007f655c, g_0068593c, 113);
    memcpy(g_007d3b74, g_0068577c, 113);
    memcpy(g_007f65d0, g_006857f0, 113);

    if (*g_00949874 > 0) {
        int32_t acc = 0;
        int32_t k = 0;
        do {
            int32_t j = k & 1;
            if (j < *g_00949870) {
                do {
                    int32_t idx = (j >> 1) + acc;
                    int32_t flags = g_94a30c_records[idx * 11 + 3];
                    if (flags != 0) {
                        int32_t topByte = flags >> 24;
                        if (topByte < 0) {
                            int32_t typeId = -1 - topByte;
                            uint8_t *digits = g_685684_tbl + typeId * 5;
                            uint8_t bl = 0;
                            if (flags & 2) {
                                for (int32_t d = 0; d < 5; d++) {
                                    bl = digits[d];
                                    g_7d3b74_flags[bl] = 0;
                                }
                            }
                            if (flags & 1) {
                                for (int32_t d = 0; d < 5; d++) {
                                    uint8_t dv = digits[d];
                                    g_7f65d0_flags[dv] = bl;
                                }
                            }
                        }
                    }
                    j += 2;
                } while (j < *g_00949870);
            }
            acc += *g_0068faf0;
            k++;
        } while (k < *g_00949874);
    }
}
