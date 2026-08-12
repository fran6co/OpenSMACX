// ORIGINAL: 0x005C32A0 FILE
// name      ?world_rocky@@YAXXZ
// size      415 bytes
// spans     0x005C32A0-0x005C343F
// prototype 
// callers   2   call targets   5
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00591BC0 0x00591D60 0x00591F00 0x005FCB20 0x0064601D
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005C32A0
// measured tier  MISMATCH
// divergence     66
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005c32a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?world_rocky@@YAXXZ  at 0x005C32A0  (415 bytes)
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
int __cdecl minerals_at(int, int);
void __cdecl bit_set(int, int, int, int);
void __cdecl do_all_non_input();
void __cdecl rocky_set(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094a30c = (int *)0x0094A30C;

void __cdecl world_rocky() {
    for (int y = 0; y < *g_00949874; y++) {
        for (int x = y & 1; x < *g_00949870; x += 2) {
            int half_x = x >> 1;
            unsigned int *rec = reinterpret_cast<unsigned int *>(
                *g_0094a30c + (*g_0068faf0 * y + half_x) * 0x2c);
            unsigned int val3 = *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(rec) + 0xc);

            bool skip = false;
            if ((val3 & 0x80000001) == 1 && static_cast<int>(val3 & 0xff000000) < 0x15000000) {
                skip = true;
            } else if ((val3 & 0x80004000) == 0x4000) {
                unsigned int hi = val3 & 0xff000000;
                if (static_cast<int>(hi) < 0x9000000 || hi == 0xc000000 || hi == 0xa000000 ||
                    hi == 0x13000000 || hi == 0xf000000 || hi == 0x14000000 || hi == 0x10000000) {
                    skip = true;
                }
            }

            if (skip) {
                continue;
            }

            int minerals = minerals_at(x, y);
            rocky_set(x, y, minerals);

            unsigned char *tile = reinterpret_cast<unsigned char *>(
                *g_0094a30c + (*g_0068faf0 * y + half_x) * 0x2c);
            unsigned int flags = *reinterpret_cast<unsigned int *>(tile + 0xc);

            if ((flags & 0x80) != 0 && (*tile & 0xe0) <= 0x5f) {
                continue;
            }

            unsigned int bitval;
            if ((*tile & 0xe0) < 0x60) {
                bitval = 0x10000;
            } else {
                bool useRandomFlat = false;
                if ((flags & 0x80000004) == 4) {
                    unsigned int r = rand();
                    r &= 0x80000001;
                    bool zero;
                    if (static_cast<int>(r) < 0) {
                        zero = ((r - 1) | 0xfffffffe) == 0xffffffff;
                    } else {
                        zero = (r == 0);
                    }
                    useRandomFlat = zero;
                }
                if (useRandomFlat) {
                    bitval = 0x10000;
                } else {
                    unsigned char b = *tile;
                    if ((b & 0xe0) < 0x81 && (b & 0x18) < 9) {
                        continue;
                    }
                    bitval = 0x20000;
                }
            }
            bit_set(x, y, bitval, 1);
        }
        do_all_non_input();
    }
}
