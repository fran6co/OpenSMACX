// ORIGINAL: 0x00588AA0 ?config_popups@@YAXXZ 0x00588AA0-0x00588F8E FILE
// size      1262 bytes
// prototype 
// callers   1   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005882F0 0x00588460 0x0059DB40 0x0059E530 0x005FAEE0 0x006043D0 0x00618F40 0x00645470
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00588AA0
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00588aa0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?config_popups@@YAXXZ  at 0x00588AA0  (1262 bytes)
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
class BasePop;
class Font;
typedef void * HFONT;
typedef char * LPSTR;
class Menu;

// ---- callees, declared and never defined (a definition would be inlined) ----
class BasePop { public:
    int __cdecl set_def_file(char *);
};
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
    int init(char *, int, int);
};
class Menu { public:
    void __cdecl set_def_font(Font *);
};
extern "C" char *strcat(char *, const char *);
int __cdecl find_font(int, int);
int __cdecl prefs_get(char *, int, int);
void __cdecl popups_normal();
void __cdecl prefs_put(char *, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068f220 = (int *)0x0068F220;
static int *const g_0068f250 = (int *)0x0068F250;
static int *const g_0068f260 = (int *)0x0068F260;
static int *const g_0068f4a4 = (int *)0x0068F4A4;
static int *const g_0068f4ac = (int *)0x0068F4AC;
static int *const g_0068f4bc = (int *)0x0068F4BC;
static int *const g_0068f4d0 = (int *)0x0068F4D0;
static int *const g_0068f4e0 = (int *)0x0068F4E0;
static int *const g_0068f4f0 = (int *)0x0068F4F0;
static int *const g_0068f500 = (int *)0x0068F500;
static int *const g_0068f510 = (int *)0x0068F510;
static int *const g_0068f520 = (int *)0x0068F520;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_00691b2c = (int *)0x00691B2C;
static int *const g_00691b30 = (int *)0x00691B30;
static int *const g_00939e58 = (int *)0x00939E58;
static int *const g_00939e5c = (int *)0x00939E5C;
static int *const g_0093fc58 = (int *)0x0093FC58;
static int *const g_0093fc80 = (int *)0x0093FC80;
static int *const g_0093fca8 = (int *)0x0093FCA8;
static int *const g_009403d8 = (int *)0x009403D8;
static int *const g_009403dc = (int *)0x009403DC;
static int *const g_0094575c = (int *)0x0094575C;
static int *const g_00945760 = (int *)0x00945760;
static int *const g_00945764 = (int *)0x00945764;
static int *const g_00945768 = (int *)0x00945768;
static int *const g_0094576c = (int *)0x0094576C;
static int *const g_00945770 = (int *)0x00945770;
static int *const g_00945774 = (int *)0x00945774;
static int *const g_00945778 = (int *)0x00945778;
static int *const g_009457ec = (int *)0x009457EC;
static int *const g_009457f0 = (int *)0x009457F0;
static int *const g_009457f4 = (int *)0x009457F4;
static int *const g_009457f8 = (int *)0x009457F8;
static int *const g_009457fc = (int *)0x009457FC;
static int *const g_00945800 = (int *)0x00945800;
static int *const g_00945804 = (int *)0x00945804;
static int *const g_00945808 = (int *)0x00945808;
static int *const g_0094580c = (int *)0x0094580C;
static int *const g_00945810 = (int *)0x00945810;
static int *const g_0094581c = (int *)0x0094581C;
static int *const g_009b7a54 = (int *)0x009B7A54;
static int *const g_009b7af4 = (int *)0x009B7AF4;
static int *const g_009b7b1c = (int *)0x009B7B1C;
static int *const g_009b7b20 = (int *)0x009B7B20;
static int *const g_009b7b68 = (int *)0x009B7B68;
static int *const g_009b86a0 = (int *)0x009B86A0;

extern int g_0068f220x[];
extern int g_0068f250x[];
extern Font g_0093fc58x[];

static int find_closest_index(int target) {
    int bestIdx = 0;
    int bestDiff = 9999;
    for (int i = 0; i < 12; ++i) {
        int diff = g_0068f220x[i] - target;
        if (diff < 0) {
            diff = target - g_0068f220x[i];
        }
        if (diff < bestDiff) {
            bestDiff = diff;
            bestIdx = i;
        }
    }
    return bestIdx;
}

void __cdecl config_popups() {
    *g_00939e5c = *g_009b7b1c;
    *g_00939e58 = *g_009b7b20;

    char *buf = reinterpret_cast<char *>(g_009b86a0);
    buf[0] = 0;
    strcat(buf, *reinterpret_cast<char **>(g_00691b0c));
    strcat(buf, reinterpret_cast<char *>(g_0068f4a4));
    reinterpret_cast<BasePop *>(0)->set_def_file(buf);

    int mainFontSize = prefs_get(reinterpret_cast<char *>(g_0068f4ac), 0x10, 0);
    int interludeFontSize = prefs_get(reinterpret_cast<char *>(g_0068f4bc), 0x10, 0);
    *g_0094581c = prefs_get(reinterpret_cast<char *>(g_0068f4d0), 0, 0);
    prefs_put(reinterpret_cast<char *>(g_0068f4e0), *g_0094581c, 0);
    if (*g_0094581c == 0) {
        int smooth = prefs_get(reinterpret_cast<char *>(g_0068f4f0), 0, 0);
        if (smooth != 0) {
            *g_0094581c = -1;
        }
    }
    prefs_put(reinterpret_cast<char *>(g_0068f500), *g_0094581c < 0, 0);
    prefs_put(reinterpret_cast<char *>(g_0068f510), mainFontSize, 0);
    prefs_put(reinterpret_cast<char *>(g_0068f520), interludeFontSize, 0);

    if (*g_009b7b1c == 800) {
        mainFontSize -= 2;
        interludeFontSize -= 2;
    }

    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 4; ++j) {
            int size = g_0068f220x[i];
            char *fontName = (size > 0xc) ? *reinterpret_cast<char **>(g_00691b30)
                                           : *reinterpret_cast<char **>(g_00691b2c);
            int flags = (size > 0xc ? 1 : 0) | g_0068f250x[j];
            g_0093fc58x[i * 4 + j].init(fontName, size, flags);
        }
    }

    *reinterpret_cast<Font **>(g_009403d8) = &g_0093fc58x[find_closest_index(mainFontSize) * 4 + 0];
    *reinterpret_cast<Font **>(g_00945760) = &g_0093fc58x[find_closest_index(mainFontSize) * 4 + 2];
    *reinterpret_cast<Font **>(g_0094575c) = &g_0093fc58x[find_closest_index(mainFontSize) * 4 + 1];
    *reinterpret_cast<Font **>(g_00945770) = &g_0093fc58x[find_closest_index(interludeFontSize) * 4 + 0];
    *reinterpret_cast<Font **>(g_009457f8) = &g_0093fc58x[find_closest_index(interludeFontSize) * 4 + 2];
    *reinterpret_cast<Font **>(g_009457fc) = &g_0093fc58x[find_closest_index(interludeFontSize) * 4 + 1];
    *reinterpret_cast<Font **>(g_0094576c) = &g_0093fc58x[find_closest_index(0x10) * 4 + 0];
    *reinterpret_cast<Font **>(g_00945808) = &g_0093fc58x[find_closest_index(0x10) * 4 + 2];
    *reinterpret_cast<Font **>(g_0094580c) = &g_0093fc58x[find_closest_index(0x10) * 4 + 1];
    *reinterpret_cast<Font **>(g_00945774) = &g_0093fc58x[find_closest_index(0x10) * 4 + 0];

    *g_00945800 = find_font(0x10, 2);
    *g_00945804 = find_font(0x10, 1);

    int sizeChoice;
    if (*g_009b7b1c == 800) {
        *g_009457ec = find_font(0xc, 0);
        *g_00945768 = find_font(0xc, 2);
        sizeChoice = 0xc;
    } else {
        *g_009457ec = find_font(0x10, 0);
        *g_00945768 = find_font(0x10, 2);
        sizeChoice = 0x10;
    }
    *g_00945764 = find_font(sizeChoice, 1);

    *g_009403dc = find_font(0xc, 0);
    *g_009457f0 = find_font(0xc, 2);
    *g_009457f4 = find_font(0xc, 1);
    *g_009b7a54 = *g_009403dc;
    *g_00945810 = find_font(0xc, 1);
    *g_00945778 = find_font(0xa, 1);

    reinterpret_cast<Menu *>(0)->set_def_font(reinterpret_cast<Font *>(*g_009457ec));
    *g_009b7b68 = *g_009457ec;
    *g_009b7af4 = *g_00945810;

    popups_normal();
}
