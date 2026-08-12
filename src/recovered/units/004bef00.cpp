// ORIGINAL: 0x004BEF00 FILE
// name      ?UNK3@UV2Player@@QAEXXZ
// size      1237 bytes
// spans     0x004BEF00-0x004BF380;0x00659D00-0x00659D55
// prototype void (__thiscall ?UNK3@UV2Player@@QAEXXZ)(UV2Player* this)
// callers   1   call targets   22
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401000 0x00401060 0x00402970 0x005D7670 0x005DAC70 0x005DACB0 0x005DACE0 0x005DAD10 0x005DAD40 0x005DAD70 0x005DB040 0x005FD530 0x005FD550 0x005FD570 0x005FD670 0x005FD740 0x00608980 0x00608BA0 0x00618F40 0x00645470 0x006456E4 0x006457C2
// indirect  0x004BF224 0x004BF228 0x004BF233 0x004BF27C 0x004BF28E 0x004BF2AA 0x004BF2FC 0x004BF30E 0x004BF32A
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004BEF00
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004bef00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK3@UV2Player@@QAEXXZ  at 0x004BEF00  (1237 bytes)
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
struct ExtDirectDraw;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;
class StringList;
class StringStruct;
struct StringStructEntry;
class UV2Player;

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

struct StringStructEntry {
    uint32_t abi_word;
    int id;
    int payload;
    StringStructEntry * next;
    StringStructEntry * previous;
    uint32_t secondary_abi_word;
    void * allocation_owner;
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
    int init(int, int, int, ExtDirectDraw *);
    int set_font(Font *, Font *, Font *, Font *);
    int write_strings(StringList *, int, int, int, int);
    int write_strings_height(StringList *, int, int);
    void set_text_color(int, int, int, int);
    void set_text_color2(int, int, int, int);
    void set_text_color3(int, int, int, int);
    void set_text_color_hyper(int, int, int, int);
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
class StringList { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;
    int add_special(char *);
    int load(char *, char *, int, void (__cdecl *)(char *));
};
class StringStruct { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;
    StringStruct(int);
    void close();
};
char * __cdecl text_get();
char * __cdecl text_item();
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl fn_00402970();
int __cdecl text_item_number();
int __cdecl text_open(char *, char *);
void __cdecl text_close();
void __stdcall fn_006456e4(void*, unsigned int, int, void (*)(void*));
void __stdcall fn_006457c2(void*, unsigned int, int, void (*)(void*), void (*)(void*));

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0
class VCall { public:
    virtual void slot000();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_004bee60 = (int *)0x004BEE60;
static int *const g_00618ea0 = (int *)0x00618EA0;
static int *const g_00618ee0 = (int *)0x00618EE0;
static int *const g_00659d4b = (int *)0x00659D4B;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_006693a0 = (int *)0x006693A0;
static int *const g_006693a4 = (int *)0x006693A4;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_006698c0 = (int *)0x006698C0;
static int *const g_006698c4 = (int *)0x006698C4;
static int *const g_0066b0ec = (int *)0x0066B0EC;
static int *const g_00678f40 = (int *)0x00678F40;
static int *const g_00687ac0 = (int *)0x00687AC0;
static int *const g_00691b00 = (int *)0x00691B00;
static int *const g_009b3374 = (int *)0x009B3374;

class UV2Player { public:
    void UNK3();
};

void UV2Player::UNK3() {
    char *self = reinterpret_cast<char *>(this);

    Font fonts[3];
    char nameBuf[3][0x80];
    int lineHeight[3];
    int leading[3];
    int ascent[3];
    int descent[3];
    int itemE[3];
    int itemF[3];

    fn_006457c2(fonts, 0x28, 3, (void (*)(void *))g_00618ea0, (void (*)(void *))g_00618ee0);

    StringStruct textList(0);

    int openResult = text_open((char *)g_00687ac0, self);
    if (openResult == 0) {
        for (int i = 0; i < 3; ++i) {
            char *buf = nameBuf[i];
            buf[0] = 0;
            text_get();
            char *item = text_item();
            strcat(buf, item);

            lineHeight[i] = text_item_number();
            leading[i] = text_item_number();

            text_get();
            ascent[i] = text_item_number() + 0xa;
            descent[i] = text_item_number() + 0xa;
            itemE[i] = text_item_number();
            itemF[i] = text_item_number();

            fonts[i].init(buf, leading[i], lineHeight[i]);
        }

        text_get();
        int a = text_item_number();
        int b = text_item_number();
        int c1 = text_item_number() + a;
        int c2 = text_item_number() + b;

        text_get();
        *reinterpret_cast<int *>(self + 0xe64) = text_item_number();
        int timeout = text_item_number();

        text_get();
        char *helpLine = text_get();

        fn_00402970();

        while (helpLine != 0 && *helpLine != 0) {
            if (*helpLine == '@') {
                reinterpret_cast<StringList *>(&textList)->load((char *)g_00691b00, helpLine + 1, 0, 0);
            } else {
                reinterpret_cast<StringList *>(&textList)->add_special(helpLine);
            }
            helpLine = text_get();
        }

        text_close();

        Buffer *buf2 = reinterpret_cast<Buffer *>(self + 0x8dc);
        int p1 = *reinterpret_cast<int *>(self + 0x110);
        int p2 = *reinterpret_cast<int *>(self + 0x118);
        buf2->init(0x280, 0x1e0, 4, reinterpret_cast<ExtDirectDraw *>(buf2));
        buf2->set_font(fonts + 0, fonts + 1, fonts + 2, fonts + 0);
        buf2->set_text_color(ascent[0], descent[0], itemE[0], itemF[0]);
        buf2->set_text_color2(ascent[1], descent[1], itemE[1], itemF[1]);
        buf2->set_text_color3(ascent[2], descent[2], itemE[2], itemF[2]);
        buf2->set_text_color_hyper(ascent[0], descent[0], itemE[0], itemF[0]);

        int height = itemF[0] - lineHeight[0];
        int written = buf2->write_strings_height(reinterpret_cast<StringList *>(&textList), height, 0);
        int x = c2 - written - leading[0];
        x += (c1 - x - lineHeight[0]) / 2;
        *reinterpret_cast<void (**)()>(self + 0x8e4) = reinterpret_cast<void (*)()>(g_004bee60);
        buf2->write_strings(reinterpret_cast<StringList *>(&textList), x, lineHeight[0], height, 0);
        *reinterpret_cast<int *>(self + 0x8e4) = 0;

        typedef unsigned long (__stdcall *TimeGetTimeFn)();
        TimeGetTimeFn timeGetTime = reinterpret_cast<TimeGetTimeFn>(*reinterpret_cast<void **>(g_00669368));
        unsigned long t0 = timeGetTime();
        unsigned long t1 = timeGetTime();
        while ((t1 - t0) < static_cast<unsigned long>(timeout)) {
            t1 = timeGetTime();
        }

        fn_00402970();
    } else {
        fn_00402970();
    }

    fn_006456e4(fonts, 0x28, 3, (void (*)(void *))g_00618ee0);
}
