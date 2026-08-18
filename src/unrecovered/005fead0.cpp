// ORIGINAL: 0x005FEAD0 ?timer_callback@Palette@@QAAXHH@Z 0x005FEAD0-0x005FEBA8 FILE
// RULED-OUT: 228 vs 216 bytes; the extra named locals (idx/found/entries/ iStartIndex/cEntries/i) push more callee-saved registers before the first `mov ebp,a2`, where the original loads a2 into ebp before any other push. Did not try collapsing to fewer locals given budget.
// working copy - scaffold materialised by --work
// size      216 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005FEB91

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?timer_callback@Palette@@QAAXHH@Z  at 0x005FEAD0  (216 bytes)
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

class Palette;
class Time;
typedef unsigned int UINT_PTR;

class Time { public:
    int unk_tgl_;
    UINT_PTR id_event_;
    void * callback1_;
    void * callback2_;
    int cb_param2_;
    int cb_param1_;
    uint32_t count_;
    int unk_1_;
    uint32_t resolution_;
    int unk_2_;
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006690a8 = (int *)0x006690A8;
static int *const g_009b8178 = (int *)0x009B8178;
static int *const g_009b8184 = (int *)0x009B8184;
static int *const g_009b8188 = (int *)0x009B8188;

class Palette { public:
    struct PaletteInternal {
        uint32_t field_0;
        Time * time;
        uint8_t field_8;
        uint8_t field_9;
        uint8_t field_A;
        uint8_t field_B;
        void * field_C;
    };
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
    uint32_t seed_;
    PaletteInternal internal_[5];

    void __cdecl timer_callback(int, int);
};
// `?...@Palette@@QAAXHH@Z` mangles __cdecl (the "A" after Q), and the body
// never reads a receiver - both ints come straight off the stack, matching
// the class's own `__cdecl` declaration with no `this` use. `a2` doubles as
// the Palette base pointer the original indexes with (`ebp` in the
// disassembly), not a hidden receiver.
struct PalEntry {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char flags;
};

typedef int (__stdcall *AnimatePaletteFn)(void *, unsigned int, unsigned int, const PalEntry *);

void __cdecl Palette::timer_callback(int a1, int a2) {
    if (a2 == 0 || *g_009b8188 != 0) {
        return;
    }

    char *base = reinterpret_cast<char *>(a2);
    int idx = 0;
    char *entry = base + 0x404;
    do {
        int field0 = *reinterpret_cast<int *>(entry);
        if (a1 == field0 || field0 == -1) {
            break;
        }
        idx++;
        entry += 0x10;
    } while (idx < 5);

    char *found = base + idx * 0x10;
    if (*reinterpret_cast<int *>(found + 0x404) != a1) {
        return;
    }

    unsigned int iStartIndex = *reinterpret_cast<unsigned char *>(found + 0x40c);
    unsigned int cEntries = *reinterpret_cast<unsigned char *>(found + 0x40d);

    PalEntry *entries = reinterpret_cast<PalEntry *>(base + iStartIndex * 4);
    unsigned char lastBlue = entries[0].blue;
    unsigned char lastGreen = entries[0].green;
    unsigned char lastRed = entries[0].red;

    unsigned int i;
    for (i = 0; i < cEntries - 1; i++) {
        entries[i].red = entries[i + 1].red;
        entries[i].green = entries[i + 1].green;
        entries[i].blue = entries[i + 1].blue;
    }

    entries[cEntries - 1].red = lastRed;
    entries[cEntries - 1].green = lastGreen;
    entries[cEntries - 1].blue = lastBlue;

    (reinterpret_cast<AnimatePaletteFn>(*g_006690a8))(
        reinterpret_cast<void *>(*g_009b8178), iStartIndex, cEntries, entries);

    *g_009b8184 = *reinterpret_cast<int *>(base + 0x400);
}
