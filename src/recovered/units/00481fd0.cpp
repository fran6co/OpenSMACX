// ORIGINAL: 0x00481FD0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00481FD0
// name           ?setup_faction_data@NetWin@@QAEXPAUMessageFactionData@@@Z
// size           1229 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00481fd0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?setup_faction_data@NetWin@@QAEXPAUMessageFactionData@@@Z  at 0x00481FD0  (1229 bytes)
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

struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct MessageFactionData;
class NetWin;
class Palette;
struct Player;
struct RECT;
class Spot;

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
};

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
class Buffer { public:
    LPVOID vtable_;
    uint32_t poOwner_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    RECT rect1_;
    RECT rect2_;
    uint32_t field_40_[4];
    uint32_t field_50_;
    LPVOID * ppv_bits_;
    uint32_t field_58_;
    uint32_t field_5C_;
    HDC hdc2_;
    HDC hdc_;
    uint32_t field_68_;
    uint32_t field_6C_;
    HRGN field_70_;
    uint32_t field_74_;
    HBITMAP bitmap_handle_;
    const BITMAPINFO * bitmap_info_;
    uint32_t width_;
    uint32_t height_;
    uint16_t field_88_;
    uint16_t field_8A_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    int32_t dib_[256];
    uint32_t field_4A4_;
    uint32_t field_4A8_;
    uint32_t field_4AC_;
    Spot spot_;
    uint8_t field_4BC_[80];
    uint32_t field_50C_;
    uint32_t field_510_;
    uint32_t field_514_;
    uint32_t field_518_;
    uint32_t field_51C_;
    uint32_t field_520_;
    uint32_t field_524_;
    uint32_t field_528_;
    Font * font1_;
    Font * font2_;
    Font * font3_;
    Font * font4_;
    uint32_t color_val_1_;
    uint32_t color_2_val_1_;
    uint32_t color_3_val_1_;
    uint32_t color_hyper_val_1_;
    uint32_t color_val_2_;
    uint32_t color_2_val_2_;
    uint32_t color_3_val_2_;
    uint32_t color_hyper_val_2_;
    uint32_t color_val_3_;
    uint32_t color_2_val_3_;
    uint32_t color_3_val_3_;
    uint32_t color_hyper_val_3_;
    uint32_t color_val_4_;
    uint32_t color_2_val_4_;
    uint32_t color_3_val_4_;
    uint32_t color_hyper_val_4_;
    uint32_t field_57C_;
    int8_t field_580_;
    uint32_t field_584_;
    Buffer();
    int get_pixel(int, int);
    int load_pcx(const char *, Palette *, int, int);
    ~Buffer();
};
char * __cdecl text_get();
char * __cdecl text_item();
extern "C" char *strcat(char *, const char *);
extern "C" char *strcpy(char *, const char *);
extern "C" char *strncpy(char *, const char *, unsigned int);
extern "C" void *memcpy(void *, const void *, unsigned int);
extern "C" void free(void *);
int __cdecl text_open(char *, char *);
void * __cdecl mem_get(int);
void __cdecl read_faction(Player *, int);
void __cdecl text_close();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006574f4 = (int *)0x006574F4;
static int *const g_00677008 = (int *)0x00677008;
static int *const g_00686908 = (int *)0x00686908;
static int *const g_00686918 = (int *)0x00686918;
static int *const g_00686924 = (int *)0x00686924;
static int *const g_00686930 = (int *)0x00686930;
static int *const g_00686940 = (int *)0x00686940;
static int *const g_00686948 = (int *)0x00686948;
static int *const g_00691afc = (int *)0x00691AFC;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_009b86a0 = (int *)0x009B86A0;

class NetWin { public:
    void setup_faction_data(MessageFactionData *);
};

// strncpy's real 3-argument CRT shape isn't the catalogued nullary one -
// still true after `mizuchi_declfix`, since the census doesn't track it as
// a callee of this address - so it is called through its address directly,
// matching how 0x0042C2E0 handled `_itoa`'s analogous case.
extern "C" char *__cdecl fn_00645e90(char *, const char *, unsigned int);

// A record's byte size is computed twice from a record count `n` via the
// same lea-chain (n -> n*32 -> n*33 -> n*132+n -> *3, +0x194): WALL, not a
// near miss - MSVC is free to fold this to a single imul either way
// (STRENGTH REDUCTION), so the arithmetic is written for its VALUE, not to
// force one encoding over the other.
static int record_block_size(int n) {
    return n * 399 + 0x194;
}

void NetWin::setup_faction_data(MessageFactionData *a1) {
    // Another SEH-guarded constructor-shaped function (push -1 / push
    // 0x6574f4 / fs:[0] chain in the prologue, cleanup funclet at
    // 0x006574e9), here because of the local `Buffer` needing unwind-safe
    // destruction. That frame is not something a call-by-call translation
    // makes the compiler re-derive, so this was never expected to score
    // BYTE_EXACT; bodied for coverage as a faithful control-flow and
    // argument-order translation of the disassembly.
    char *self = reinterpret_cast<char *>(this);
    Buffer buf;

    if (a1 != 0 && *g_0093d4f0 == *g_0093d4f4) {
        return;
    }

    void *old_block = *reinterpret_cast<void **>(self + 0x772c);
    if (old_block != 0) {
        free(old_block);
        *reinterpret_cast<void **>(self + 0x772c) = 0;
    }

    if (a1 != 0) {
        // Fast path: the caller already has a serialized block: clone it.
        char *src = reinterpret_cast<char *>(a1);
        unsigned char raw_n = *reinterpret_cast<unsigned char *>(src + 0x11);
        int n = (raw_n < 8) ? 8 : static_cast<int>(raw_n);
        int size = record_block_size(n);
        void *block = mem_get(size);
        *reinterpret_cast<void **>(self + 0x772c) = block;
        memcpy(block, src + 0x10, size);
        return;
    }

    // Slow path: rebuild from the localized text file.
    char *text_base = reinterpret_cast<char *>(*reinterpret_cast<int *>(g_00691afc));
    int eco_count = 0;
    if (text_open(text_base, reinterpret_cast<char *>(g_00686908)) == 0) {
        text_get();
        if (*text_item() != 0) {
            do {
                eco_count++;
                text_get();
            } while (*text_item() != 0);
        }
        if (eco_count > 10) {
            eco_count = 10;
        } else if (eco_count < 0) {
            eco_count = 0;
        }
    }

    int record_count = eco_count + 0xe;
    if (record_count < 8) {
        record_count = 8;
    }
    int record_size = record_count * 399 + 0x194;

    void *block1 = mem_get(record_size);
    *reinterpret_cast<void **>(self + 0x772c) = block1;
    void *block2 = mem_get(record_size);
    *reinterpret_cast<void **>(self + 0x7730) = block2;

    char *hdr = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c));
    hdr[1] = static_cast<char>(record_count);
    reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c))[0] = 7;

    text_base = reinterpret_cast<char *>(*reinterpret_cast<int *>(g_00691afc));
    if (text_open(text_base, reinterpret_cast<char *>(g_00686918)) != 0) {
        return;
    }
    for (int off = 0; off < 7 * 0x190; off += 0x190) {
        char *rec = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c));
        rec[off + 9] = 1;
        text_get();
        char *dest = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c)) + off + 0xa;
        strcpy(dest, text_item());
    }

    text_base = reinterpret_cast<char *>(*reinterpret_cast<int *>(g_00691afc));
    if (text_open(text_base, reinterpret_cast<char *>(g_00686924)) != 0) {
        return;
    }
    {
        int flag_off = 0;
        int name_off = 0xaf0;
        for (; flag_off < 0xaf0; flag_off += 0x190, name_off += 0x190) {
            char *rec = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c));
            rec[flag_off + 9] = 1;
            text_get();
            char *dest = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c)) + name_off + 0xa;
            strcpy(dest, text_item());
        }
    }

    text_base = reinterpret_cast<char *>(*reinterpret_cast<int *>(g_00691afc));
    if (text_open(text_base, reinterpret_cast<char *>(g_00686930)) != 0) {
        return;
    }
    if (eco_count > 0) {
        int flag_off = 0;
        int name_off = 0x15e0;
        for (int i = 0; i < eco_count; i++, flag_off += 0x190, name_off += 0x190) {
            char *rec = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c));
            rec[flag_off + 9] = 1;
            text_get();
            char *dest = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c)) + name_off + 0xa;
            strcpy(dest, text_item());
        }
    }

    if (record_count > 0) {
        int off = 0;
        for (int i = 0; i < record_count; i++, off += 0x190) {
            char *rec_base = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c));
            char *key = rec_base + off + 0xa;
            if (text_open(key, key) != 0) {
                return;
            }
            rec_base = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c));
            rec_base[off + 9] = 1;
            // `text_open` just returned 0 (the branch above only falls
            // through on success) - the compiler reused that value here
            // instead of re-deriving it.
            *reinterpret_cast<int *>(rec_base + off + 4) = 0;

            text_get();
            text_item();
            text_item();
            char *short_name = text_item();
            rec_base = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c));
            fn_00645e90(rec_base + off + 0x22, short_name, 0x18);
            rec_base[off + 0x39] = 0;

            char *pcx_path = reinterpret_cast<char *>(g_009b86a0);
            pcx_path[0] = 0;
            strcat(pcx_path, rec_base + off + 0xa);
            strcat(pcx_path, reinterpret_cast<char *>(g_00686940));

            if (buf.load_pcx(pcx_path, 0, 0xa, 0xec) != 0) {
                buf.load_pcx(reinterpret_cast<char *>(g_00686948), 0, 0xa, 0xec);
            }
            unsigned char pixel = static_cast<unsigned char>(buf.get_pixel(4, 0x2f3));
            rec_base = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c));
            rec_base[off + 8] = static_cast<char>(pixel);

            // `Player` is opaque (forward-declared only): modeled as a raw
            // byte buffer and reached by the offsets the disassembly shows,
            // same technique as every other opaque class in this tree.
            // `read_faction` takes the two fields written at +4 and +0x1c
            // (both the record's own name) and fills in - among others -
            // the six string fields read back below.
            unsigned char player_storage[0x400];
            char *player = reinterpret_cast<char *>(player_storage);
            rec_base = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c));
            strcpy(player + 4, rec_base + off + 0xa);
            strcpy(player + 0x1c, rec_base + off + 0xa);
            read_faction(reinterpret_cast<Player *>(player), 0);

            rec_base = reinterpret_cast<char *>(*reinterpret_cast<void **>(self + 0x772c));
            strcpy(rec_base + off + 0x22, player + 0x34);
            strcpy(rec_base + off + 0x3a, player + 0x384);
            strcpy(rec_base + off + 0x62, player + 0x2e4);
            strcpy(rec_base + off + 0x7a, player + 0x304);
            strcpy(rec_base + off + 0xfa, player + 0x4c);
            strcpy(rec_base + off + 0x112, player + 0x64);
            rec_base[off + 0x192] = player[0];
        }
    }

    text_close();
    memcpy(*reinterpret_cast<void **>(self + 0x7730),
           *reinterpret_cast<void **>(self + 0x772c), record_size);
}
