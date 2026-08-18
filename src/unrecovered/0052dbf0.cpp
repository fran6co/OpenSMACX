// ORIGINAL: 0x0052DBF0 ?next_player_turn@@YAHXZ 0x0052DBF0-0x0052DC68 FILE
// RULED-OUT: do-while loop over i=[*g_009a6820+1, 8) with hi/lo byte reads hoisted before the loop, matching the Ghidra hypothesis closely; diverges at instr #14 (original 'lea' vs rebuilt 'inc'), a register/addressing-mode choice around the `+1`.
// working copy - scaffold materialised by --work
// size      120 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00592EE0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?next_player_turn@@YAHXZ  at 0x0052DBF0  (120 bytes)
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
void message_data(int, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0093a950 = (int *)0x0093A950;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a64e9 = (int *)0x009A64E9;
static int *const g_009a681c = (int *)0x009A681C;
static int *const g_009a6820 = (int *)0x009A6820;
int __cdecl next_player_turn() {
    if ((*reinterpret_cast<unsigned char *>(g_009a681c) & 0x10) == 0) {
        return 0;
    }
    if (*g_0093e8c0 == 0) {
        return 0;
    }
    int i = *g_009a6820 + 1;
    if (i < 8) {
        unsigned int hi_byte = *reinterpret_cast<unsigned char *>(g_009a64e9);
        unsigned char lo_byte = *reinterpret_cast<unsigned char *>(g_009a64e8);
        do {
            unsigned int bit = 1 << i;
            if ((bit & hi_byte) != 0 && (static_cast<unsigned char>(bit) & lo_byte) != 0) {
                message_data(0x4309, 0, i, 0, 0, 0);
                return 0;
            }
            i = i + 1;
        } while (i < 8);
    }
    *g_0093a950 = 1;
    return 1;
}
