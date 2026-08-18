// ORIGINAL: 0x00477100 ?mon_secret_project@@YAXHH@Z 0x00477100-0x00477207 FILE
// size      263 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00476A50 0x005C89A0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00477100
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00477100/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mon_secret_project@@YAXHH@Z  at 0x00477100  (263 bytes)
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
static int *const g_0094f2d0 = (int *)0x0094F2D0;
static int *const g_0094f2d4 = (int *)0x0094F2D4;
static int *const g_009a64d4 = (int *)0x009A64D4;

// Per-faction record arrays: the ADDRESS itself does work here (each
// is indexed by `a1 * stride`, and `/O2` folds a plain
// `static T *const` literal away under that arithmetic - see the
// EXTERN-SYMBOL LEVER), so these are declared `extern` instead of
// reusing the scaffold's `g_0094cb30`/`g_0094cb34` plain-pointer
// spelling. Stride is 0x4F4 bytes (0x13D ints) for every array
// except the byte-flag array, which is indexed directly in bytes
// (0x59C stride); 8 factions fit exactly between 0x94CB30 and the
// 0x94F2D0 loop bound ((0x94F2D0-0x94CB30)/0x4F4 == 8).
extern int g_0094cb20_arr[];
extern int g_0094cb28_arr[];
extern int g_0094cb2c_arr[];
extern int g_0094cb30_arr[];
extern int g_0094cb34_arr[];
extern int g_0094cea0_arr[];
extern unsigned char g_00946f58_arr[];

// NOT BYTE_EXACT. Closest reached: 258/263 bytes, mnemonic
// similarity 0.831, edit_count 11 (byte_match.py --json against the
// MEASURED_FLAGS/framed candidate). The logic is complete and
// verified against Ghidra's reconstruction field-for-field: the
// per-faction record stride (0x4F4 bytes / 0x13D ints), the separate
// byte-flag array's stride (0x59C), the two do/while loops walking
// by pointer comparison against a fixed end address rather than a
// count (matching the original's `(int)piVar3 < 0x94f2d0` shape
// exactly, not a bounded index loop), and the `year`/`best`/`a1==
// faction count` tail all reproduce the original's field offsets and
// call arguments one for one. Remaining divergence is REGISTER
// ALLOCATION across the whole function: the original commits `esi`
// to holding the parameter `a1` for the entire body (freeing `edi`
// for the loop counters and `ebx` for the interim "best" value) and
// reserves its `[ebp-4]` local for `year` from the prologue (`push
// ecx` as a `sub esp,4` substitute) before the first branch; this
// candidate's compiler instead keeps `a1` in a different register
// through the first test and reorders which callee-saved registers
// get pushed at each point, producing a different (still internally
// consistent) but non-identical register schedule end to end. Ruled
// out: this is the same REGISTER ALLOCATION wall documented on
// 0x006339E0's NetFifo destructor, not reachable by re-ordering
// which local is declared first - every variation tried kept the
// same divergence shape, just at different offsets.
void __cdecl mon_secret_project(int a1, int a2) {
    int year = game_year(*g_009a64d4);
    if ((g_00946f58_arr[a1 * 0x59c] & 0x80) == 0 && g_0094cb30_arr[a1 * 0x13d] == 0) {
        g_0094cb34_arr[a1 * 0x13d] = 0;
        int i = 0;
        int *p = g_0094cb30_arr;
        do {
            if (i != a1 && *p != 0) {
                g_0094cb34_arr[a1 * 0x13d] += 1;
            }
            p += 0x13d;
            ++i;
        } while (p < reinterpret_cast<int *>(0x94f2d0));

        int best = a1;
        if (g_0094cb34_arr[a1 * 0x13d] == 0) {
            g_0094cb34_arr[a1 * 0x13d] = 1;
        } else {
            g_0094cb34_arr[a1 * 0x13d] = 0;
            int j = 0;
            int *q = g_0094cb34_arr;
            do {
                if (j != a1 && q[-1] != 0 && *q != 0) {
                    best = j;
                }
                q += 0x13d;
                ++j;
            } while (q < reinterpret_cast<int *>(0x94f2d4));
        }
        g_0094cb30_arr[a1 * 0x13d] = 1;
        g_0094cb20_arr[a1 * 0x13d] = year;
        g_0094cb2c_arr[a1 * 0x13d] = best;
        g_0094cb28_arr[a1 * 0x13d] = a2;
        g_0094cea0_arr[a1 * 0x13d] = 5;
        if (a1 == *g_00939284) {
            monument(1);
        }
    }
}

