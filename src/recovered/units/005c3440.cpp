// ORIGINAL: 0x005C3440 FILE
// name      ?world_fungus@@YAXXZ
// size      568 bytes
// spans     0x005C3440-0x005C3678
// prototype 
// callers   2   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C03D0 0x005C1E20 0x005C1F20 0x005FCB20 0x0064601D
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005C3440
// measured tier  MISMATCH
// divergence     9
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005c3440/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?world_fungus@@YAXXZ  at 0x005C3440  (568 bytes)
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

class Fractal;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Fractal { public:
    int UNK1(int, int);
    void init();
};
extern "C" int rand();
int __cdecl veh_init(int, int, int, int);
void __cdecl do_all_non_input();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_006919ec = (int *)0x006919EC;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094a2b8 = (int *)0x0094A2B8;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_00950308 = (int *)0x00950308;
static int *const g_009a6488 = (int *)0x009A6488;

// Fractal is opaque with no data members, so a bare local would allocate
// nothing where the original reserves 0x2E4 bytes for it (`sub esp, 0x2e4` /
// `lea ecx, [ebp-0x2e4]`). Pad it with a shadow buffer of the right size.
struct FractalStorage { char bytes[0x2E4]; };

void __cdecl world_fungus() {
    FractalStorage storage;
    Fractal *fractal = reinterpret_cast<Fractal *>(&storage);
    fractal->init();

    int y = 0;
    int distanceMod = (*g_0094a2b8 - 1) * (*g_00950308);

    if (*g_00949874 > 0) {
        do {
            int x = y & 1;
            if (x < *g_00949870) {
                int y4 = y << 4;
                int y8 = y4 << 4;
                do {
                    int a = fractal->UNK1(x << 4, y4);
                    int b = fractal->UNK1(x << 8, y8);
                    int level = ((a * 5 + b * 2) * 7) >> 8;
                    if (level < 0)
                        level = 0;
                    else if (level > 100)
                        level = 100;

                    int halfX = x >> 1;
                    int tileIndex = (*g_0068faf0) * y + halfX;
                    int flags = *reinterpret_cast<int *>(
                        *reinterpret_cast<char **>(g_0094a30c) + tileIndex * 0x2C + 0xC);

                    if ((flags & 0x80000002) != 2 || (flags & 0xff000000) >= 0x9000000) {
                        if ((level >= 20 - distanceMod && level <= distanceMod * 2 + 25) ||
                            (level >= 40 - distanceMod && level <= distanceMod * 2 + 45) ||
                            (level >= 60 - distanceMod && level <= distanceMod * 2 + 65)) {
                            goto modifyTile;
                        }
                        if (y != 0 && y != *g_00949874 - 1)
                            goto tail;
                        if (rand() % 2 == 0)
                            goto tail;
                    modifyTile:
                        {
                            char *tileBase = *reinterpret_cast<char **>(g_0094a30c) + tileIndex * 0x2C;
                            if ((static_cast<unsigned char>(tileBase[5]) & 0xc0) < 0x80 ||
                                (static_cast<unsigned char>(tileBase[0]) & 0xe0) < 0x60) {
                                *reinterpret_cast<unsigned int *>(tileBase + 8) |= 0x20;
                                *reinterpret_cast<unsigned int *>(
                                    *reinterpret_cast<char **>(g_0094a30c) + tileIndex * 0x2C + 8) &=
                                    ~(*reinterpret_cast<unsigned int *>(g_006919ec));
                                if (rand() % 40 == 0) {
                                    if ((static_cast<unsigned char>(tileBase[0]) & 0xe0) >= 0x60 &&
                                        *g_009a6488 != 0) {
                                        veh_init(0x13, 0, x, y);
                                    }
                                }
                            }
                        }
                    }
                tail:
                    x += 2;
                } while (x < *g_00949870);
            }
            do_all_non_input();
            y++;
        } while (y < *g_00949874);
    }
}
