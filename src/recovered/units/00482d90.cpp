// ORIGINAL: 0x00482D90 FILE
// name      ?prepare_game@NetWin@@QAEXXZ
// size      1040 bytes
// spans     0x00482D90-0x0048318E;0x006574FE-0x00657510
// prototype void (__thiscall ?prepare_game@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   12
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00453710 0x00453F70 0x00586050 0x005B0D70 0x00625730 0x00625740 0x00625750 0x00625770 0x00645460 0x00645470 0x00645660 0x0064FC88
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00482D90
// measured tier  MISMATCH
// divergence     13
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00482d90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?prepare_game@NetWin@@QAEXXZ  at 0x00482D90  (1040 bytes)
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

class NetWin;
class Random;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Random { public:
    uint32_t seed_;
    Random();
    unsigned int get(int, int);
    void reseed(unsigned long);
    ~Random();
};

extern "C" char *strcat(char *, const char *);
extern "C" char *strcpy(char *, const char *);
extern "C" int __cdecl _itoa();
extern "C" int strcmp(const char *, const char *);
void compute_faction_modifiers(int);
void load_faction_art(int);
void load_flags();
void read_faction(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00657506 = (int *)0x00657506;
static int *const g_00677030 = (int *)0x00677030;
static int *const g_0090db98 = (int *)0x0090DB98;
static int *const g_0090db9b = (int *)0x0090DB9B;
static int *const g_0090dd17 = (int *)0x0090DD17;
static int *const g_0090dd19 = (int *)0x0090DD19;
static int *const g_0090dd31 = (int *)0x0090DD31;
static int *const g_0090de95 = (int *)0x0090DE95;
static int *const g_0090e778 = (int *)0x0090E778;
static int *const g_0090e77b = (int *)0x0090E77B;
static int *const g_0090e77d = (int *)0x0090E77D;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_0093d520 = (int *)0x0093D520;
static int *const g_0093d6be = (int *)0x0093D6BE;
static int *const g_0093e202 = (int *)0x0093E202;
static int *const g_00946ff0 = (int *)0x00946FF0;
static int *const g_00947008 = (int *)0x00947008;
static int *const g_00949734 = (int *)0x00949734;
static int *const g_0096c9e8 = (int *)0x0096C9E8;

class NetWin { public:
    void prepare_game();
};
typedef unsigned int size_t;
extern "C" void *memcpy(void *, const void *, size_t);
extern "C" char *__cdecl _strcpy(char *, const char *);
extern "C" int __cdecl _strcmp(const char *, const char *);
extern "C" char *__cdecl my_itoa(int, char *, int);
extern char tbl_946a84[];
extern char tbl_946dd4[];
extern char tbl_946d34[];
extern char tbl_946d54[];
extern char tbl_946a9c[];
extern char tbl_946ab4[];
extern char tbl_946a50[];

extern "C" char *__cdecl _strcat(char *, const char *);

void NetWin::prepare_game() {
    char *self = reinterpret_cast<char *>(this);

    Random rng;
    unsigned int usedMask = 0;
    rng.reseed(1000);

    unsigned char *roster = *reinterpret_cast<unsigned char **>(self + 0x772c);
    int count = roster[1];

    if (count > 0) {
        unsigned char *rec = roster + 9;
        for (int i = 0; i < count; ++i) {
            if (*rec == 0)
                usedMask |= 1 << i;
            rec += 0x190;
        }
    }

    unsigned char *p = reinterpret_cast<unsigned char *>(g_0090dd17);
    do {
        if (*p != 0xff)
            usedMask |= 1 << *p;
        p += 0x17c;
    } while (p < reinterpret_cast<unsigned char *>(g_0090e77b));

    int needed = 0;
    if (count > 0) {
        unsigned char *rec = roster + 9;
        for (int i = 0; i < count; ++i) {
            if (*rec != 0 && (usedMask & (1 << i)) == 0)
                needed++;
            rec += 0x190;
        }
    }

    unsigned int hiddenMask = 0;
    p = reinterpret_cast<unsigned char *>(g_0090dd17);
    int slot = 1;
    int remaining = needed;
    do {
        if (*p == 0xff) {
            hiddenMask |= 1 << slot;
            unsigned int picked;
            if (remaining == 0) {
                picked = rng.get(0, count);
            } else {
                do {
                    picked = rng.get(0, count);
                } while ((usedMask & (1 << picked)) != 0);
                remaining--;
            }
            *p = static_cast<unsigned char>(picked);
            usedMask |= 1 << picked;
        }
        p += 0x17c;
        slot++;
    } while (p < reinterpret_cast<unsigned char *>(g_0090e77b));

    if (*g_0090e778 == 0 || *g_0090e778 == 1) {
        char *dst = reinterpret_cast<char *>(g_0093d520);
        char *src = reinterpret_cast<char *>(g_0090db98);
        unsigned char id = 0;
        do {
            memcpy(dst, src, 0x176);
            *dst = static_cast<char>(id);
            id++;
            dst += 0x19c;
            src += 0x17c;
        } while (src < reinterpret_cast<char *>(g_0090e778));
    } else {
        char *dst = reinterpret_cast<char *>(g_0093d520);
        char *src = reinterpret_cast<char *>(g_0090db9b);
        do {
            memcpy(dst, src - 3, 0x176);
            signed char id = *src;
            if (id == -1)
                id = 0;
            *dst = id;
            src += 0x17c;
            dst += 0x19c;
        } while (src < reinterpret_cast<char *>(g_0090e77b));
    }

    if (*g_0090e778 == 0 || *g_0090e778 == 1) {
        char *name = reinterpret_cast<char *>(g_0090de95);
        int slot2 = 2;
        do {
            int dupCount = 0;
            if (name > reinterpret_cast<char *>(g_0090dd19)) {
                char *resv = reinterpret_cast<char *>(g_0090dd19);
                int n = slot2 - 1;
                do {
                    if (_strcmp(name, resv) == 0)
                        dupCount++;
                    resv += 0x17c;
                    n--;
                } while (n != 0);
                if (dupCount != 0) {
                    char buf[80];
                    my_itoa(dupCount, buf, 10);
                    _strcat(name, buf);
                }
            }
            name += 0x17c;
            slot2++;
        } while (name < reinterpret_cast<char *>(g_0090e77d));

        int factionId = 1;
        unsigned char *names = reinterpret_cast<unsigned char *>(g_0090dd17);
        char *rosterEntries = reinterpret_cast<char *>(g_00947008);
        do {
            _strcpy(rosterEntries - 0x18,
                   reinterpret_cast<char *>(roster + *names * 0x190 + 10));
            _strcpy(rosterEntries,
                   reinterpret_cast<char *>(roster + *names * 0x190 + 10));
            read_faction(factionId);
            compute_faction_modifiers(factionId);
            names += 0x17c;
            factionId++;
            rosterEntries += 0x59c;
        } while (names < reinterpret_cast<unsigned char *>(g_0090e77b));

        char *table = reinterpret_cast<char *>(g_0090dd31);
        int slot3 = 1;
        char *src2 = reinterpret_cast<char *>(g_0093d6be);
        do {
            int fid = static_cast<signed char>(src2[-2]);
            *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096c9e8) + fid * 0x20cc) =
                static_cast<signed char>(*src2);
            if ((hiddenMask & (1 << slot3)) == 0) {
                int off = fid * 0x59c;
                _strcpy(tbl_946a84 + off, table - 0x18);
                _strcpy(tbl_946dd4 + off, table);
                _strcpy(tbl_946d34 + off, table + 0x28);
                _strcpy(tbl_946d54 + off, table + 0x40);
                _strcpy(tbl_946a9c + off, table + 0xc0);
                _strcpy(tbl_946ab4 + off, table + 0xd8);
                *reinterpret_cast<int *>(tbl_946a50 + fid * 0x59c) =
                    static_cast<unsigned char>(table[0x158]);
            }
            src2 += 0x19c;
            slot3++;
            table += 0x17c;
        } while (src2 < reinterpret_cast<char *>(g_0093e202));
    } else if (*g_0093d4f0 != *g_0093d4f4) {
        int fid = 1;
        char *table = reinterpret_cast<char *>(g_00946ff0);
        int off = 400;
        do {
            _strcpy(table, reinterpret_cast<char *>(roster + off + 10));
            load_faction_art(fid);
            table += 0x59c;
            fid++;
            off += 400;
        } while (table < reinterpret_cast<char *>(g_00949734));
    }

    load_flags();
}
