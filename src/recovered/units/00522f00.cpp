// ORIGINAL: 0x00522F00 ?do_fungal_towers@@YAXXZ 0x00522F00-0x005230DD FILE
// size      477 bytes
// prototype 
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046AF40 0x00501350 0x00591D60 0x00591E50 0x005B6260 0x0064601D
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00522F00
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00522f00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?do_fungal_towers@@YAXXZ  at 0x00522F00  (477 bytes)
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
extern "C" int rand();
int __cdecl morale_alien(int, int);
void __cdecl bit_set(int, int, int, int);
void __cdecl draw_tile(int, int, int);
void __cdecl spot_all(int, int);
void __cdecl synch_bit(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066efc0 = (int *)0x0066EFC0;
static int *const g_0066f440 = (int *)0x0066F440;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0095282c = (int *)0x0095282C;
static int *const g_009a64c8 = (int *)0x009A64C8;

// INDEXED TABLE BASE: each is read through a register-scaled address
// ([reg*4 + ADDR]), so the address itself does work; the folded pointer
// constant the emitter gave us would let /O2 collapse the addressing mode
// away. Re-declared as arrays under a shortened name (same address, fewer
// leading zero digits) so it does not collide with the emitter's own
// `static T *const` declaration of the same symbol.
extern int32_t g_66efc0[];
extern int32_t g_66f440[];
extern uint8_t g_95282c[];

void __cdecl do_fungal_towers() {
    // Vehs array (0x00952828), one record past its base: this walks the
    // struct's `state` field (offset 4), so the loop's record pointer sits
    // 4 bytes into each 0x34-byte Veh. proto_id == 0x13 (BSC_FUNGAL_TOWER).
    int i = *g_009a64c8 - 1;
    if (i < 0)
        return;
    uint8_t *rec = g_95282c + i * 0x34;
    do {
        if (*reinterpret_cast<int16_t *>(rec + 6) == 0x13) {
            *reinterpret_cast<uint32_t *>(rec) |= 4;
            int x = *reinterpret_cast<int16_t *>(rec - 4);
            int y = *reinterpret_cast<int16_t *>(rec - 2);

            if (rand() % 5 == 0) {
                int dir = rand() % 8;
                int nx = g_66efc0[dir] + x;
                if ((*reinterpret_cast<uint8_t *>(g_0094988c) & 1) == 0) {
                    if (nx < 0)
                        nx += *g_00949870;
                    else if (nx >= *g_00949870)
                        nx -= *g_00949870;
                }
                int ny = g_66f440[dir + 1] + y;

                if (ny >= 0 && ny < *g_00949874 && nx >= 0 && nx < *g_00949870) {
                    bit_set(nx, ny, 0x20, 1);
                    bit_set(nx, ny, 0x28805c, 0);
                    synch_bit(nx, ny, 1);
                    synch_bit(nx, ny, 2);
                    synch_bit(nx, ny, 3);
                    synch_bit(nx, ny, 4);
                    synch_bit(nx, ny, 5);
                    synch_bit(nx, ny, 6);
                    synch_bit(nx, ny, 7);
                    draw_tile(nx, ny, 2);
                }
            }

            bit_set(x, y, 0x20, 1);
            bit_set(x, y, 0x28805c, 0);
            synch_bit(x, y, 1);
            synch_bit(x, y, 2);
            synch_bit(x, y, 3);
            synch_bit(x, y, 4);
            synch_bit(x, y, 5);
            synch_bit(x, y, 6);
            synch_bit(x, y, 7);
            draw_tile(x, y, 2);

            rec[0x20] = static_cast<uint8_t>(morale_alien(i, 0));
            spot_all(i, 1);
        }
        i--;
        rec -= 0x34;
    } while (i >= 0);
}
