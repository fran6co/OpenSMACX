// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005905C0
// name           ?say_weapon@@YAXPAXHH@Z
// size           300 bytes
// measured tier  MISMATCH
// divergence     27
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005905c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_weapon@@YAXPAXHH@Z  at 0x005905C0  (300 bytes)
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

typedef int BOOL;
typedef void * LPVOID;
class Strings;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Strings { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    BOOL is_populated_;
    int get(int);
};
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_00682e98 = (int *)0x00682E98;
static int *const g_00682e9c = (int *)0x00682E9C;
static int *const g_0068fae8 = (int *)0x0068FAE8;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

// `#pragma function` is load-bearing: without it /O2 expands `strcat`
// inline (a scasb+rep movsd/movsb copy) since the scaffold's `extern "C"`
// declaration doesn't stop the compiler recognizing it by name. The
// disassembly shows a real call, so the intrinsic form is forced back out.
#pragma function(strcat)

// A per-weapon record, stride 0x10, indexed by `a2` (register-scaled index
// seen at [edi+0x94ae60]/[edi+0x94ae68]/[edi+0x94ae6a] with edi=a2*0x10).
// Not in the scaffold's fixed-globals list at all - the indexed-table lever
// applies (`extern T name[];`), plus this table has no catalogued base yet.
struct WeaponRecord {
    int32_t string_id_;
    uint8_t pad_04_[4];
    int8_t flag_08_;
    uint8_t pad_09_;
    uint8_t count_0a_;
    uint8_t pad_0b_[5];
};

extern WeaponRecord g_weapons[];

// The scaffold's `_itoa` slot is nullary (the catalogue has no recorded
// arity for it) but the call needs three arguments, so a shim retypes the
// same symbol instead of redeclaring it.
typedef char *(__cdecl *ItoaShim)(int, char *, int);

void __cdecl say_weapon(void * a1, int a2, int a3) {
    char *dest = reinterpret_cast<char *>(a1);
    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
    strcat(dest, reinterpret_cast<char *>(strings->get(g_weapons[a2].string_id_)));
    if (a3 != 0) {
        strcat(dest, reinterpret_cast<char *>(g_00682820));
        strcat(dest, reinterpret_cast<char *>(g_00682e9c));
        int8_t flag = g_weapons[a2].flag_08_;
        char *base = *reinterpret_cast<char **>(g_009b90f8);
        if (flag == 0) {
            int32_t field = *reinterpret_cast<int32_t *>(base + 0x284);
            strcat(dest, reinterpret_cast<char *>(strings->get(field)));
        } else {
            int32_t field = *reinterpret_cast<int32_t *>(base + 0x204);
            strcat(dest, reinterpret_cast<char *>(strings->get(field)));
            strcat(dest, reinterpret_cast<char *>(g_00682e94));
            if (g_weapons[a2].flag_08_ < 0) {
                char *base2 = *reinterpret_cast<char **>(g_009b90f8);
                int32_t field2 = *reinterpret_cast<int32_t *>(base2 + 0x310);
                strcat(dest, reinterpret_cast<char *>(strings->get(field2)));
            } else {
                char buf[0x50];
                reinterpret_cast<ItoaShim>(&_itoa)(g_weapons[a2].flag_08_, buf, 10);
                strcat(dest, buf);
            }
            int i = 0;
            if (g_weapons[a2].count_0a_ != 0) {
                do {
                    strcat(dest, reinterpret_cast<char *>(g_0068fae8));
                    i++;
                } while (i < g_weapons[a2].count_0a_);
            }
        }
        strcat(dest, reinterpret_cast<char *>(g_00682e98));
    }
}
