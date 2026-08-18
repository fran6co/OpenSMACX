// ORIGINAL: 0x0059D250 ?exec@PopMenu@@QAEXHHP6AHXZ@Z 0x0059D250-0x0059D392 FILE
// size      322 bytes
// prototype void (__thiscall ?exec@PopMenu@@QAEXHHP6AHXZ@Z)(PopMenu* this, int, int, int (__cdecl *)())
// callers   16   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005F8920 0x005F89D0 0x005F8BE0 0x005F9170 0x005F9F40 0x00613740 0x00644F3A 0x00645660
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0059D250
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0059d250/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?exec@PopMenu@@QAEXHHP6AHXZ@Z  at 0x0059D250  (322 bytes)
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

class Dialogs;
class Menu;
class PopMenu;
class PullDown;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Dialogs { public:
    int get_num_items();
};
class PullDown { public:
    int add_item(int, char *);
    int add_separator(int);
    int get_selected();
    int show(int, int, int (__cdecl *)());
    void init(Menu *);
};
extern "C" int abs(int);
extern "C" int strcmp(const char *, const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006900c8 = (int *)0x006900C8;

class PopMenu { public:
    void exec(int, int, int (__cdecl *)());
};

void PopMenu::exec(int a1, int a2, int (__cdecl *a3)()) {
    char *self = reinterpret_cast<char *>(this);
    char *pulldown = self + 0x537C;

    reinterpret_cast<PullDown *>(pulldown)->init(0);

    int count = reinterpret_cast<Dialogs *>(self + 0x21D0)->get_num_items();
    if (count > 0) {
        int i = 0;
        do {
            int vtbl = *reinterpret_cast<int *>(self + 0x21D0);
            int field8 = *reinterpret_cast<int *>(vtbl + 8);
            int count2 = *reinterpret_cast<int *>(self + 0x229C + field8) - 1;
            char *rec = self + 0x228C + field8;

            if (i <= count2) {
                int result = i;
                *reinterpret_cast<int *>(rec + 0xC) = *reinterpret_cast<int *>(rec + 8);
                if (i < 0) {
                    int n = abs(i);
                    if (*reinterpret_cast<int *>(rec + 0x10) < n) {
                        goto after_list;
                    }
                    n = abs(i);
                    if (n > 0) {
                        do {
                            --n;
                            *reinterpret_cast<int *>(rec + 0xC) =
                                *reinterpret_cast<int *>(*reinterpret_cast<int *>(rec + 0xC) + 0x10);
                        } while (n != 0);
                    }
                    result = *reinterpret_cast<int *>(rec + 0x10) + i;
                } else if (i > 0) {
                    int n = i;
                    do {
                        --n;
                        *reinterpret_cast<int *>(rec + 0xC) =
                            *reinterpret_cast<int *>(*reinterpret_cast<int *>(rec + 0xC) + 0xC);
                    } while (n != 0);
                }
                *reinterpret_cast<int *>(rec + 0x14) = result;
            }

        after_list:
            int vtbl2 = *reinterpret_cast<int *>(self + 0x21D0);
            int field8b = *reinterpret_cast<int *>(vtbl2 + 8);
            char *rec2 = self + 0x228C + field8b;

            char *str;
            if (*reinterpret_cast<int *>(self + 0x2294 + field8b) == 0) {
                str = 0;
            } else {
                int node = *reinterpret_cast<int *>(rec2 + 0xC);
                int inner = *reinterpret_cast<int *>(node + 8);
                str = *reinterpret_cast<char **>(inner + 4);
            }

            int val;
            if (*reinterpret_cast<int *>(rec2 + 8) == 0) {
                val = 0;
            } else {
                int node2 = *reinterpret_cast<int *>(rec2 + 0xC);
                val = *reinterpret_cast<int *>(node2 + 4);
            }

            if (strcmp(str, reinterpret_cast<const char *>(g_006900c8)) == 0) {
                reinterpret_cast<PullDown *>(pulldown)->add_separator(val);
            } else {
                reinterpret_cast<PullDown *>(pulldown)->add_item(val, str);
            }

            ++i;
            count = reinterpret_cast<Dialogs *>(self + 0x21D0)->get_num_items();
        } while (i < count);
    }

    reinterpret_cast<PullDown *>(pulldown)->show(a1, a2, a3);
    reinterpret_cast<PullDown *>(pulldown)->get_selected();
}
