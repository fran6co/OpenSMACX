// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00428880
// name           ?load@Credits@@QAEXXZ
// size           314 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00428880/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load@Credits@@QAEXXZ  at 0x00428880  (314 bytes)
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
class Credits;
class Font;
typedef void * HFONT;
typedef char * LPSTR;
class Wave;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Font { public:
    int unk_1_;
    BOOL is_fot_set_;
    HFONT font_obj_;
    int line_height_;
    int height_;
    int internal_leading_;
    int ascent_;
    int descent_;
    int pad_;
    LPSTR fot_file_name_;
    int width(char *);
};
class Wave { public:
    uint32_t vtable_storage_;
    uint32_t volume_;
    uint32_t field_8_;
    uint8_t memset_region_[0x24];
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    void * device_;
    uint32_t field_40_;
    Wave * chain_prev_;
    Wave * chain_next_;
    void * fname_;
    uint32_t field_50_;
    uint8_t flags_54_;
    uint8_t pad_55_[3];
    int32_t pitch_;
    float reverb_mix_;
    int32_t ms_length_;
    uint32_t start_time_;
    uint32_t group_slot_;
    int load(char *, unsigned long);
    void unload();
};
char * __cdecl text_get();
extern "C" char *strcpy(char *, const char *);
extern "C" unsigned int strlen(const char *);
extern "C" void free(void *);
int __cdecl text_open(char *, char *);
void * __cdecl mem_get(int);
void __cdecl text_close();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068328c = (int *)0x0068328C;
static int *const g_006832a0 = (int *)0x006832A0;
static int *const g_006832a8 = (int *)0x006832A8;
static int *const g_00703e30 = (int *)0x00703E30;
static int *const g_009b7d00 = (int *)0x009B7D00;

class Credits { public:
    uint8_t pad_0_[0xA14];
    uint32_t field_a14_;

    void load();
};

#pragma function(strlen, strcpy)

void Credits::load() {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<int32_t *>(self + 0xa18) = 0;

    void **entries = reinterpret_cast<void **>(self + 0xa1c);
    int32_t remaining = 0x200;
    do {
        if (*entries != 0) {
            free(*entries);
            *entries = 0;
        }
        entries++;
    } while (--remaining);
    entries = reinterpret_cast<void **>(self + 0xa1c);

    *reinterpret_cast<int32_t *>(self + 0xa18) = 0;
    *reinterpret_cast<int32_t *>(self + 0xa14) = 0;

    reinterpret_cast<Wave *>(g_00703e30)->unload();
    reinterpret_cast<Wave *>(g_00703e30)->load(reinterpret_cast<char *>(g_0068328c), 2);

    if (text_open(reinterpret_cast<char *>(g_006832a8), reinterpret_cast<char *>(g_006832a0)) == 0) {
        *reinterpret_cast<int32_t *>(self + 0x1230) = 0;

        while (*text_get() != '#') {
            char *line = reinterpret_cast<char *>(g_009b7d00);
            void *buffer = mem_get(strlen(line) + 1);
            int32_t index = *reinterpret_cast<int32_t *>(self + 0xa18);
            entries[index] = buffer;

            void *stored = entries[*reinterpret_cast<int32_t *>(self + 0xa18)];
            if (stored != 0) {
                *reinterpret_cast<int32_t *>(self + 0xa18) = index + 1;
                strcpy(reinterpret_cast<char *>(stored), reinterpret_cast<char *>(g_009b7d00));

                int32_t new_index = *reinterpret_cast<int32_t *>(self + 0xa18);
                int32_t max_width = *reinterpret_cast<int32_t *>(self + 0x1230);
                Font *font = reinterpret_cast<Font *>(self + 0x1c48);
                char *msg = reinterpret_cast<char *>(entries[new_index - 1]);

                if (max_width <= font->width(msg)) {
                    max_width = font->width(msg);
                }
                *reinterpret_cast<int32_t *>(self + 0x1230) = max_width;
            }
        }
        text_close();
    }
}
