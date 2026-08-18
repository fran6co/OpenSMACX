// ORIGINAL: 0x00477320 ?mon_conquer_base@@YAXHPAD@Z 0x00477320-0x00477434 FILE
// size      276 bytes
// prototype void (__cdecl ?mon_conquer_base@@YAXHPAD@Z)(int factionID, int8* lpszBaseName)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00476A50 0x005C89A0 0x00645E90
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00477320
// measured tier  MISMATCH
// divergence     16
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00477320/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mon_conquer_base@@YAXHPAD@Z  at 0x00477320  (276 bytes)
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
extern "C" int __cdecl strncpy();
int __cdecl game_year(int);
void __cdecl monument(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0094cba0 = (int *)0x0094CBA0;
static int *const g_0094cba4 = (int *)0x0094CBA4;
static int *const g_0094f340 = (int *)0x0094F340;
static int *const g_0094f344 = (int *)0x0094F344;
static int *const g_009a64d4 = (int *)0x009A64D4;

// A per-faction site record, stride 0x4F4, laid out from field offsets seen
// indexed by `a1` below (see STRUCTURE report). Not catalogued anywhere else.
struct SiteRecord {
    int32_t built_year_;
    uint8_t pad_04_[8];
    int32_t owner_;
    int32_t taken_;
    int32_t claim_count_;
    uint8_t pad_18_[0x2F8];
    int32_t status_;
    uint8_t pad_314_[0x40];
    char name_[0x18];
    uint8_t pad_36C_[0x4F4 - 0x36C];
};

// A per-faction flag byte, stride 0x59C; only byte 0 (bit 0x80) is read here.
struct FactionFlags {
    uint8_t flags_;
    uint8_t pad_[0x59C - 1];
};

// Both are read through a REGISTER-COMPUTED index (a1 scaled by the record
// stride), so the fixed-globals lever applies: `extern T name[];`, not the
// scaffold's folded `static T *const` literal, which /O2 would fold away.
extern SiteRecord g_site_records[];
extern FactionFlags g_faction_flags[];

// The scaffold's `strncpy` slot is nullary (the catalogue has no recorded
// arity for it) but the call needs three arguments, so a shim retypes the
// same symbol instead of redeclaring it (redeclaring with a real prototype
// conflicts with the scaffold's extern "C" zero-arg one).
typedef char *(__cdecl *StrncpyShim)(char *, const char *, unsigned int);

void __cdecl mon_conquer_base(int a1, char * a2) {
    int year = game_year(*g_009a64d4);
    SiteRecord *rec = &g_site_records[a1];
    FactionFlags *flags = &g_faction_flags[a1];
    if (flags->flags_ & 0x80) {
        return;
    }
    if (rec->taken_ != 0) {
        return;
    }
    rec->claim_count_ = 0;
    for (int i = 0; i < 8; i++) {
        if (i != a1 && g_site_records[i].taken_ != 0) {
            rec->claim_count_++;
        }
    }
    int owner = a1;
    if (rec->claim_count_ == 0) {
        rec->claim_count_ = 1;
    } else {
        rec->claim_count_ = 0;
        for (int i = 0; i < 8; i++) {
            if (i != a1 && g_site_records[i].taken_ != 0 &&
                g_site_records[i].claim_count_ != 0) {
                owner = i;
            }
        }
    }
    rec->owner_ = owner;
    rec->taken_ = 1;
    rec->built_year_ = year;
    reinterpret_cast<StrncpyShim>(&strncpy)(rec->name_, a2, 0x18);
    rec->status_ = 7;
    if (a1 == *g_00939284) {
        monument(1);
    }
}
