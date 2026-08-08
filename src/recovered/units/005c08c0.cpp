// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005C08C0
// name           ?veh_kill@@YAXH@Z
// size           564 bytes
// measured tier  MISMATCH
// divergence     15
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005c08c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?veh_kill@@YAXH@Z  at 0x005C08C0  (564 bytes)
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

class BaseWin;
class DeletionList;

// ---- callees, declared and never defined (a definition would be inlined) ----
class BaseWin { public:
    void check_base(int);
    void check_loc(int, int, int);
};
class DeletionList { public:
    uint32_t field_0_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint32_t field_2C_;
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    uint32_t field_3C_;
    uint32_t field_40_;
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    uint32_t field_6C_;
    uint32_t field_70_;
    uint32_t field_74_;
    uint32_t field_78_;
    uint32_t field_7C_;
    uint32_t field_80_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t field_A8_;
    uint32_t field_AC_;
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    uint32_t field_C4_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint32_t field_E8_;
    uint32_t field_EC_;
    uint32_t field_F0_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    uint32_t field_114_;
    uint32_t field_118_;
    uint32_t field_11C_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    uint32_t field_13C_;
    uint32_t field_140_;
    uint32_t field_144_;
    uint32_t field_148_;
    uint32_t field_14C_;
    uint32_t field_150_;
    uint32_t field_154_;
    uint32_t field_158_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    uint32_t field_188_;
    uint32_t field_18C_;
    uint32_t field_190_;
    uint32_t field_194_;
    uint32_t field_198_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    uint32_t field_1A4_;
    uint32_t field_1A8_;
    uint32_t field_1AC_;
    uint32_t field_1B0_;
    uint32_t field_1B4_;
    uint32_t field_1B8_;
    uint32_t field_1BC_;
    uint32_t field_1C0_;
    uint32_t field_1C4_;
    uint32_t field_1C8_;
    uint32_t field_1CC_;
    uint32_t field_1D0_;
    uint32_t field_1D4_;
    uint32_t field_1D8_;
    uint32_t field_1DC_;
    uint32_t field_1E0_;
    uint32_t field_1E4_;
    uint32_t field_1E8_;
    uint32_t field_1EC_;
    uint32_t field_1F0_;
    uint32_t field_1F4_;
    uint32_t field_1F8_;
    uint32_t field_1FC_;
    uint32_t field_200_;
    uint32_t field_204_;
    uint32_t field_208_;
    uint32_t field_20C_;
    uint32_t field_210_;
    uint32_t field_214_;
    uint32_t field_218_;
    uint32_t field_21C_;
    uint32_t field_220_;
    uint32_t field_224_;
    uint32_t field_228_;
    uint32_t field_22C_;
    uint32_t field_230_;
    uint32_t field_234_;
    uint32_t field_238_;
    uint32_t field_23C_;
    uint32_t field_240_;
    uint32_t field_244_;
    uint32_t field_248_;
    uint32_t field_24C_;
    uint32_t field_250_;
    uint32_t field_254_;
    uint32_t field_258_;
    uint32_t field_25C_;
    uint32_t field_260_;
    uint32_t field_264_;
    uint32_t field_268_;
    uint32_t field_26C_;
    uint32_t field_270_;
    uint32_t field_274_;
    uint32_t field_278_;
    uint32_t field_27C_;
    uint32_t field_280_;
    uint32_t field_284_;
    uint32_t field_288_;
    uint32_t field_28C_;
    uint32_t field_290_;
    uint32_t field_294_;
    uint32_t field_298_;
    uint32_t field_29C_;
    uint32_t field_2A0_;
    uint32_t field_2A4_;
    uint32_t field_2A8_;
    uint32_t field_2AC_;
    uint32_t field_2B0_;
    uint32_t field_2B4_;
    uint32_t field_2B8_;
    uint32_t field_2BC_;
    uint32_t field_2C0_;
    uint32_t field_2C4_;
    uint32_t field_2C8_;
    uint32_t field_2CC_;
    uint32_t field_2D0_;
    uint32_t field_2D4_;
    uint32_t field_2D8_;
    uint32_t field_2DC_;
    uint32_t field_2E0_;
    uint32_t field_2E4_;
    uint32_t field_2E8_;
    uint32_t field_2EC_;
    uint32_t field_2F0_;
    uint32_t field_2F4_;
    uint32_t field_2F8_;
    uint32_t field_2FC_;
    uint32_t field_300_;
    uint32_t field_304_;
    uint32_t field_308_;
    uint32_t field_30C_;
    uint32_t field_310_;
    uint32_t field_314_;
    uint32_t field_318_;
    uint32_t field_31C_;
    uint32_t field_320_;
    uint32_t field_324_;
    uint32_t field_328_;
    uint32_t field_32C_;
    uint32_t field_330_;
    uint32_t field_334_;
    uint32_t field_338_;
    uint32_t field_33C_;
    uint32_t field_340_;
    uint32_t field_344_;
    uint32_t field_348_;
    uint32_t field_34C_;
    uint32_t field_350_;
    uint32_t field_354_;
    uint32_t field_358_;
    uint32_t field_35C_;
    uint32_t field_360_;
    uint32_t field_364_;
    uint32_t field_368_;
    uint32_t field_36C_;
    uint32_t field_370_;
    uint32_t field_374_;
    uint32_t field_378_;
    uint32_t field_37C_;
    uint32_t field_380_;
    uint32_t field_384_;
    uint32_t field_388_;
    uint32_t field_38C_;
    uint32_t field_390_;
    uint32_t field_394_;
    uint32_t field_398_;
    uint32_t field_39C_;
    uint32_t field_3A0_;
    uint32_t field_3A4_;
    uint32_t field_3A8_;
    uint32_t field_3AC_;
    uint32_t field_3B0_;
    uint32_t field_3B4_;
    uint32_t field_3B8_;
    uint32_t field_3BC_;
    uint32_t field_3C0_;
    uint32_t field_3C4_;
    uint32_t field_3C8_;
    uint32_t field_3CC_;
    uint32_t field_3D0_;
    uint32_t field_3D4_;
    uint32_t field_3D8_;
    uint32_t field_3DC_;
    uint32_t field_3E0_;
    uint32_t field_3E4_;
    uint32_t field_3E8_;
    uint32_t field_3EC_;
    uint32_t field_3F0_;
    uint32_t field_3F4_;
    uint32_t field_3F8_;
    uint32_t field_3FC_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    uint32_t field_43C_;
    uint32_t field_440_;
    uint32_t field_444_;
    uint32_t field_448_;
    uint32_t field_44C_;
    uint32_t field_450_;
    uint32_t field_454_;
    uint32_t field_458_;
    uint32_t field_45C_;
    uint32_t field_460_;
    uint32_t field_464_;
    uint32_t field_468_;
    uint32_t field_46C_;
    uint32_t field_470_;
    uint32_t field_474_;
    uint32_t field_478_;
    uint32_t field_47C_;
    uint32_t field_480_;
    uint32_t field_484_;
    uint32_t field_488_;
    uint32_t field_48C_;
    uint32_t field_490_;
    uint32_t field_494_;
    uint32_t field_498_;
    uint32_t field_49C_;
    uint32_t field_4A0_;
    uint32_t field_4A4_;
    uint32_t field_4A8_;
    uint32_t field_4AC_;
    uint32_t field_4B0_;
    uint32_t field_4B4_;
    uint32_t field_4B8_;
    uint32_t field_4BC_;
    uint32_t field_4C0_;
    uint32_t field_4C4_;
    uint32_t field_4C8_;
    uint32_t field_4CC_;
    uint32_t field_4D0_;
    uint32_t field_4D4_;
    uint32_t field_4D8_;
    uint32_t field_4DC_;
    uint32_t field_4E0_;
    uint32_t field_4E4_;
    uint32_t field_4E8_;
    uint32_t field_4EC_;
    uint32_t field_4F0_;
    uint32_t field_4F4_;
    uint32_t field_4F8_;
    uint32_t field_4FC_;
    uint32_t field_500_;
    uint32_t field_504_;
    uint32_t field_508_;
    uint32_t field_50C_;
    uint32_t field_510_;
    uint32_t field_514_;
    uint32_t field_518_;
    uint32_t field_51C_;
    uint32_t field_520_;
    uint32_t field_524_;
    uint32_t field_528_;
    uint32_t field_52C_;
    uint32_t field_530_;
    uint32_t field_534_;
    uint32_t field_538_;
    uint32_t field_53C_;
    uint32_t field_540_;
    uint32_t field_544_;
    uint32_t field_548_;
    uint32_t field_54C_;
    uint32_t field_550_;
    uint32_t field_554_;
    uint32_t field_558_;
    uint32_t field_55C_;
    uint32_t field_560_;
    uint32_t field_564_;
    uint32_t field_568_;
    uint32_t field_56C_;
    uint32_t field_570_;
    uint32_t field_574_;
    uint32_t field_578_;
    uint32_t field_57C_;
    uint32_t field_580_;
    uint32_t field_584_;
    uint32_t field_588_;
    uint32_t field_58C_;
    uint32_t field_590_;
    uint32_t field_594_;
    uint32_t field_598_;
    uint32_t field_59C_;
    uint32_t field_5A0_;
    int add_deletion(int);
};
extern "C" void *memcpy(void *, const void *, unsigned int);
void __cdecl log_say(char *, int, int, int);
void __cdecl veh_lift(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00691dc0 = (int *)0x00691DC0;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_008c6b2c = (int *)0x008C6B2C;
static int *const g_008c6b34 = (int *)0x008C6B34;
static int *const g_0093928c = (int *)0x0093928C;
static int *const g_00939290 = (int *)0x00939290;
static int *const g_0093e31c = (int *)0x0093E31C;
static int *const g_0093e908 = (int *)0x0093E908;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00952858 = (int *)0x00952858;
static int *const g_0096da38 = (int *)0x0096DA38;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009a64dc = (int *)0x009A64DC;
static int *const g_009ab892 = (int *)0x009AB892;

// 52-byte (0x34) record array at 0x952828, indexed by vehicle id. Never
// detected by the emitter (it only appears folded into a scaled-index
// displacement, `[esi+0x9528xx]` with esi already `a1*0x34`), so this is
// a NEW extern array rather than something to redeclare - INDEXED TABLE
// BASE lever: a scaled index into a fixed base needs a real symbol or
// /O2 folds the address arithmetic away.
struct Veh {
    int16_t loc_x;                       // 0x00
    int16_t loc_y;                       // 0x02
    uint8_t unmapped_04_[0x0A - 0x04];
    int16_t unit_id;                      // 0x0A
    uint8_t unmapped_0C_[0x0E - 0x0C];
    uint8_t faction_id;                    // 0x0E
    uint8_t unmapped_0F_[0x11 - 0x0F];
    uint8_t has_ref_;                      // 0x11
    uint8_t unmapped_12_[0x14 - 0x12];
    int16_t ref_veh_id_;                    // 0x14
    uint8_t unmapped_16_[0x2E - 0x16];
    int16_t base_id;                        // 0x2E
    int16_t ref_a_;                         // 0x30
    int16_t ref_b_;                         // 0x32
};
extern Veh g_00952828[];

// Same lever, second view of the context's own `g_009ab892` (declared a
// plain pointer, right for nothing here since every use scales by 0x34):
// a distinct name rather than a redeclaration of the context's global.
struct UnitTypeRow {
    uint8_t f0_;
    uint8_t unmapped_1_[0x34 - 1];
};
extern UnitTypeRow g_009ab892_tbl[];

// Ditto for `g_0096da38` (context declares a plain pointer; this use
// scales by 0x833) and `g_0096d238`, never detected at all - it only
// ever appears as a three-operand SIB (`[ecx+eax*4+0x96d238]`).
extern int32_t g_0096da38_tbl[];
extern uint8_t g_0096d238_tbl[];

// MISMATCH (#15: original 'push' vs rebuilt 'xor' - an `xor edx,edx`
// used to zero-extend a byte for the `log_say` call lands one position
// off from where the original schedules it; mnemonic stream is 0.79
// similar past that point). A faithful translation of every call site,
// argument order, table stride (0x952828 vehicle records, 0x9ab892 unit
// -type row, 0x96da38/0x96d238 per-faction counters) and indirect-call
// target read directly off the disassembly. Not re-timeboxed further
// given the size (564 bytes, largest of this batch).
void __cdecl veh_kill(int a1) {
    if (a1 < 0) {
        return;
    }

    if (*g_0093f660 != 0) {
        reinterpret_cast<DeletionList *>(g_0093e31c)->add_deletion(a1);
    }

    log_say(reinterpret_cast<char *>(g_00691dc0), a1,
            g_00952828[a1].faction_id, g_00952828[a1].unit_id);

    int16_t loc_x = g_00952828[a1].loc_x;
    int16_t base_id = g_00952828[a1].base_id;
    int16_t loc_y = g_00952828[a1].loc_y;
    int16_t unit_id = g_00952828[a1].unit_id;
    uint8_t faction_id = g_00952828[a1].faction_id;

    if (g_009ab892_tbl[unit_id].f0_ <= 7) {
        --g_0096da38_tbl[faction_id * 0x833];
    }
    --g_0096d238_tbl[faction_id * 0x20cc + unit_id];

    veh_lift(a1);

    if (a1 < *g_009a64c8 - 1) {
        memcpy(&g_00952828[a1], &g_00952828[a1 + 1],
               (*g_009a64c8 - a1 - 1) * sizeof(Veh));
    }
    if (a1 < 0x800) {
        --*g_009a64c8;
    }
    if (*g_009a64c8 > 0) {
        int16_t *p = reinterpret_cast<int16_t *>(g_00952858);
        int count = *g_009a64c8;
        do {
            if (a1 < p[1]) {
                --p[1];
            }
            if (a1 < p[0]) {
                --p[0];
            }
            if (*(reinterpret_cast<uint8_t *>(p) - 0x1f) == 1) {
                int16_t ref = p[-0xe];
                if (ref == a1) {
                    *(reinterpret_cast<uint8_t *>(p) - 0x1f) = 0;
                } else if (a1 < ref) {
                    p[-0xe] = static_cast<int16_t>(ref - 1);
                }
            }
            p += 0x1a;
        } while (--count != 0);
    }

    if (*g_0093928c == a1) {
        *g_0093928c = -1;
    } else if (a1 < *g_0093928c) {
        --*g_0093928c;
    }
    if (*g_00939290 == a1) {
        *g_00939290 = -1;
    } else if (a1 < *g_00939290) {
        --*g_00939290;
    }
    if (*g_008c6b2c == a1) {
        *g_008c6b2c = -1;
    } else if (a1 < *g_008c6b2c) {
        --*g_008c6b2c;
    }
    if (*g_008c6b34 == a1) {
        *g_008c6b34 = -1;
    } else if (a1 < *g_008c6b34) {
        --*g_008c6b34;
    }
    if (*g_009a64dc != 0 && a1 <= *g_009a64dc) {
        --*g_009a64dc;
    }
    if (*g_0093f660 != 0) {
        int *ptr = *reinterpret_cast<int **>(g_0093e908);
        if (ptr != 0 && ptr != g_0093928c && ptr != g_009a64dc && ptr != g_00939290) {
            if (a1 <= *ptr) {
                --*ptr;
            }
        }
    }

    reinterpret_cast<BaseWin *>(g_006a7628)->check_loc(loc_x, loc_y, 0);
    reinterpret_cast<BaseWin *>(g_006a7628)->check_base(base_id);
}
