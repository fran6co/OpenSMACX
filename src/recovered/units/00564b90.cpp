// ORIGINAL: 0x00564B90 FILE
// name      ?assemble_passengers@@YAXHHH@Z
// size      512 bytes
// spans     0x00564B90-0x00564D90
// prototype void (__cdecl ?assemble_passengers@@YAXHHH@Z)(int baseID, int, int)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0053A780 0x00579A30 0x005B9580 0x005BFE90
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00564B90
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00564b90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?assemble_passengers@@YAXHHH@Z  at 0x00564B90  (512 bytes)
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
int __cdecl contiguous(int, int, int, int);
int __cdecl stack_check(int, int, int, int, int);
int __cdecl veh_at(int, int);
void __cdecl add_goal(int, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096dd5d = (int *)0x0096DD5D;
static int *const g_009a64d4 = (int *)0x009A64D4;

// Per-a1 record table, stride 0x134 bytes: word iVar5 @0, word iVar6 @2,
// byte "chassis id" uVar1 @4, dword flags @0x30, byte state @9. A single
// indexed access per field (not a register-advanced walk), so the folded
// pointer-constant spelling is correct here - matches 0041a8b0's proven
// `static char *const g_0097d040` for this exact table.
static char *const g_0097d040 = (char *)0x0097D040;

// Per-uVar2 walk base: `local_c` genuinely advances by 0x20cc as a register
// across loop iterations in the original, so a materialized pointer is
// the right spelling here (unlike the single-access tables above).
static uint8_t *const g_0096fc28 = (uint8_t *)0x0096FC28;

static uint8_t *const g_0096cdc0 = (uint8_t *)0x0096CDC0;
static uint8_t *const g_0096cdbc = (uint8_t *)0x0096CDBC;

// Same fixed address as the context's g_0096dd5d, but that one is a plain
// `int *` pointer constant; this access is byte-typed and computed from a
// register expression, so it needs its own declaration under a new name.
static uint8_t *const g_0096dd5d_flags = (uint8_t *)0x0096DD5D;

// Loop-indexed flags table - genuinely walked by the register uVar7, so the
// indexed-table-base lever applies (extern T[], not a folded pointer).
extern int32_t g_0096c9f8_tbl[];

void __cdecl assemble_passengers(int a1, int a2, int a3) {
    int local_10 = 0;
    unsigned uVar7 = 1;

    int iVar5 = *reinterpret_cast<int16_t *>(g_0097d040 + a1 * 0x134);
    int iVar6 = *reinterpret_cast<int16_t *>(g_0097d040 + a1 * 0x134 + 2);
    unsigned uVar1 = *reinterpret_cast<uint8_t *>(g_0097d040 + a1 * 0x134 + 4);

    int t = *g_0068faf0 * iVar6 + (iVar5 >> 1);
    uint8_t *base = *reinterpret_cast<uint8_t **>(g_0094a30c);
    unsigned uVar2 = base[t * 0x2c + 3];

    uint8_t *local_c = g_0096fc28 + uVar2;
    do {
        if (uVar7 != uVar1) {
            if (g_0096c9f8_tbl[uVar1 * 2099 + uVar7] & 0x10) {
                if (*local_c == 0 || contiguous(uVar7, uVar1, uVar2, 1) == 0) {
                    local_10++;
                }
            }
        }
        uVar7++;
        local_c += 0x20cc;
    } while ((int)uVar7 < 8);

    *reinterpret_cast<int32_t *>(g_0097d040 + a1 * 0x134 + 0x30) |= 1;
    *reinterpret_cast<uint8_t *>(g_0097d040 + a1 * 0x134 + 9) = 2;

    if (a2 == 8 &&
        !(g_0096dd5d_flags[uVar2 * 2 + uVar1 * 0x20cc] & 8)) {
        goto add_common_goals;
    }

    {
        int iVar3 = (*reinterpret_cast<int32_t *>(g_0096cdc0 + uVar1 * 0x20cc) -
                     *reinterpret_cast<int32_t *>(g_0096cdbc + uVar1 * 0x20cc)) -
                    *g_009a64d4 / 100 + 2;
        if (iVar3 < 0) {
            iVar3 = 0;
        } else if (iVar3 > 0x63) {
            iVar3 = 0x63;
        }
        if (local_10 < iVar3) {
            int vehResult = veh_at(iVar5, iVar6);
            int stackResult = stack_check(vehResult, 2, 8, uVar1, -1);
            int uVar4 = (stackResult == 0) ? a3 : 1;
            add_goal(uVar1, 8, uVar4, iVar5, iVar6, -1);
        }
        if (a2 < 3) {
            goto after_goals;
        }
    }

add_common_goals:
    add_goal(uVar1, 2, a3, iVar5, iVar6, -1);
    add_goal(uVar1, 0, a3, iVar5, iVar6, -1);
    add_goal(uVar1, 0xb, a3, iVar5, iVar6, -1);

after_goals:
    if (a2 == 7) {
        add_goal(uVar1, 3, 1, iVar5, iVar6, -1);
    }
}
