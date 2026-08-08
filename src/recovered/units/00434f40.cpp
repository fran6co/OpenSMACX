// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00434F40
// name           ?draw_weapon@DesignWin@@QAEXH@Z
// size           1215 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00434f40/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_weapon@DesignWin@@QAEXH@Z  at 0x00434F40  (1215 bytes)
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
class Caviar;
class CaviarData;
class DesignWin;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;
class Strings;

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    int copy(Buffer *, RECT *);
    int set_clip(RECT *);
    int set_font(Font *, Font *, Font *, Font *);
    int wrap_cent(char *, RECT *);
    int write_cent_l(Font *, char *, RECT *, int);
    int write_cent_l(char *, RECT *, int);
    void set_text_color(int, int, int, int);
};

class Caviar { public:
    float scene_scale_;
    uint8_t unmapped_04_[0x2C - 0x04];
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint8_t unmapped_38_[0x108 - 0x38];
    int32_t field_108_;
    uint8_t unmapped_10C_[0x13D0 - 0x10C];
    int add_object(CaviarData *, int);
    int render(Buffer *, int, int, int);
    void set_scaling(float);
    void set_scene_rotation(float, float, float);
};

class CaviarData { public:
    uint32_t field_0_;
    uint32_t fileDescriptor_;
    void * record_;
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
    int width(char *);
};

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
extern "C" unsigned int strlen(const char *);
int vehdraw_construct_weapon(int, int, int, Caviar *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00683550 = (int *)0x00683550;
static int *const g_00733974 = (int *)0x00733974;
static int *const g_007871c0 = (int *)0x007871C0;
static int *const g_008c6dcc = (int *)0x008C6DCC;
static int *const g_008c6dd0 = (int *)0x008C6DD0;
static int *const g_008c6dd4 = (int *)0x008C6DD4;
static int *const g_008c6dd8 = (int *)0x008C6DD8;
static int *const g_008c6ddc = (int *)0x008C6DDC;
static int *const g_008c6de0 = (int *)0x008C6DE0;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DesignWin { public:
    uint8_t pad_0_[0x12764];
    Spot spot_;

    void draw_weapon(int);
};
extern "C" char *__cdecl my_itoa(int, char *, int);
extern "C" char *__stdcall CharUpperA(char *);
extern unsigned char tbl_94ae60[];

void DesignWin::draw_weapon(int a1) {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x444);
    char *strBuf = reinterpret_cast<char *>(g_009b86a0);

    RECT localRect;
    localRect.left = *reinterpret_cast<long *>(self + 0xabc);
    localRect.top = *reinterpret_cast<long *>(self + 0xac0);
    localRect.right = *reinterpret_cast<long *>(self + 0xac4);
    localRect.bottom = *reinterpret_cast<long *>(self + 0xac8);

    buffer->set_clip(&localRect);
    buffer->copy(this ? buffer : (Buffer *)0, &localRect);

    int lineH = *reinterpret_cast<int *>(self + 0x14550);
    int weaponIdx = *reinterpret_cast<int *>(self + 0x14200);

    int adj = *reinterpret_cast<int *>(self + 0x14568);
    if (adj < 0)
        adj = *reinterpret_cast<int *>(self + 0x14574);
    else
        adj = *reinterpret_cast<int *>(self + 0x14578) + adj;
    adj += localRect.top;

    int textTop = adj;
    int textBottom = localRect.bottom;

    *strBuf = 0;
    int weaponBase = weaponIdx * 0x10;
    int strId;
    int *strTable = reinterpret_cast<int *>(*g_009b90f8);
    if (tbl_94ae60[weaponBase + 8] == 0)
        strId = *reinterpret_cast<int *>(reinterpret_cast<char *>(strTable) + 0x200);
    else
        strId = *reinterpret_cast<int *>(reinterpret_cast<char *>(strTable) + 0x204);

    char *str = reinterpret_cast<char *>(
        reinterpret_cast<Strings *>(g_009b90d8)->get(strId));
    strcat(strBuf, str);
    CharUpperA(strBuf);
    buffer->set_text_color(*g_008c6dcc, *g_008c6dd0, 1, 1);
    if (strBuf[0] != 0) {
        unsigned int len = strlen(strBuf);
        RECT r2;
        r2.left = localRect.left;
        r2.top = textTop;
        r2.right = localRect.right;
        r2.bottom = textBottom;
        buffer->write_cent_l(reinterpret_cast<Font *>(self + 0x14568), strBuf, &r2, len);
    }

    *reinterpret_cast<int *>(self + 0x13b68) = 0;
    textTop = textTop + lineH * -2 - 5;

    Caviar *caviar = reinterpret_cast<Caviar *>(self + 0x1279c);
    if (weaponIdx == 0) {
        CaviarData cd;
        caviar->add_object(&cd, 0);
        caviar->set_scaling(static_cast<float>(0.15f));
    } else {
        int arg1, arg2, arg3;
        arg3 = weaponIdx;
        if (weaponIdx == 0x15 && *g_00733974 == 0) {
            arg1 = *reinterpret_cast<int *>(self + 0x141f4);
            arg2 = -1;
        } else {
            arg1 = *reinterpret_cast<int *>(self + 0x141f4);
            arg2 = *reinterpret_cast<int *>(self + 0x141fc);
        }
        vehdraw_construct_weapon(arg1, arg2, arg3, caviar);

        unsigned char rarity = tbl_94ae60[weaponBase + 0xa];
        float scale;
        if (rarity < 3)
            scale = 0.15f;
        else if (rarity == 9)
            scale = 0.06f;
        else
            scale = 0.08f;
        caviar->set_scaling(scale);
    }

    int cx = (*reinterpret_cast<int *>(self + 0xac4) + *reinterpret_cast<int *>(self + 0xabc)) / 2;
    int cy = (*reinterpret_cast<int *>(self + 0xac8) + *reinterpret_cast<int *>(self + 0xac0)) / 2;
    caviar->set_scene_rotation(0.5323889f, 0.0f, 0.0f);
    caviar->render(this ? buffer : (Buffer *)0, cx, cy, 6);

    Font **fontSlot = reinterpret_cast<Font **>(self + 0x14540);
    buffer->set_font(0, 0, 0, reinterpret_cast<Font *>(fontSlot));

    int textLeft = textTop;
    textTop = textTop + lineH;
    buffer->set_text_color(*g_008c6dd4, *g_008c6dd8, 1, 1);
    *strBuf = 0;
    int nameStrId = *reinterpret_cast<int *>(tbl_94ae60 + weaponBase);
    str = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(nameStrId));
    strcat(strBuf, str);
    CharUpperA(strBuf);

    RECT r3;
    r3.left = localRect.left;
    r3.top = localRect.top;
    r3.right = localRect.right;
    r3.bottom = localRect.bottom;

    int avail = *reinterpret_cast<int *>(self + 0xac4) - *reinterpret_cast<int *>(self + 0xabc);
    int textW = reinterpret_cast<Font *>(fontSlot)->width(strBuf);
    if (avail < textW) {
        int shift;
        if (*reinterpret_cast<int *>(fontSlot) < 0)
            shift = *reinterpret_cast<int *>(self + 0x1454c);
        else
            shift = *reinterpret_cast<int *>(self + 0x14550) + *reinterpret_cast<int *>(fontSlot);
        r3.left = textLeft - shift;
        r3.right = textTop - shift;
        buffer->wrap_cent(strBuf, &r3);
        if (*reinterpret_cast<int *>(fontSlot) < 0)
            shift = *reinterpret_cast<int *>(self + 0x1454c);
        else
            shift = *reinterpret_cast<int *>(self + 0x14550) + *reinterpret_cast<int *>(fontSlot);
        r3.left = r3.left + shift;
        r3.right = r3.right + shift;
    } else if (strBuf[0] != 0) {
        unsigned int len = strlen(strBuf);
        buffer->write_cent_l(strBuf, &r3, len);
    }

    int step = *reinterpret_cast<int *>(self + 0x14550) + 3;
    r3.left += step;
    r3.right += step;

    buffer->set_text_color(*g_008c6ddc, *g_008c6de0, 1, 1);
    *strBuf = 0;
    unsigned char rarity2 = tbl_94ae60[weaponBase + 0xa];
    if (rarity2 < 3) {
        int *base2 = reinterpret_cast<int *>(*g_009b90f8);
        str = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(reinterpret_cast<char *>(base2) + 0x204)));
        strcat(strBuf, str);
        strcat(strBuf, reinterpret_cast<char *>(g_00682820));
        signed char c = static_cast<signed char>(tbl_94ae60[weaponBase + 8]);
        if (c < 0) {
            str = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
                *reinterpret_cast<int *>(reinterpret_cast<char *>(base2) + 0x310)));
            strcat(strBuf, str);
        } else if (c < 0x63) {
            char numBuf[80];
            my_itoa(static_cast<int>(c), numBuf, 10);
            strcat(strBuf, numBuf);
        } else {
            strcat(strBuf, reinterpret_cast<char *>(g_00683550));
        }
    } else {
        int *base2 = reinterpret_cast<int *>(*g_009b90f8);
        str = reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(reinterpret_cast<char *>(base2) + 0x200)));
        strcat(strBuf, str);
        strcat(strBuf, reinterpret_cast<char *>(g_00682820));
        char numBuf[80];
        my_itoa(tbl_94ae60[weaponBase + 0xb], numBuf, 10);
        strcat(strBuf, numBuf);
    }

    if (strBuf[0] != 0) {
        unsigned int len = strlen(strBuf);
        buffer->write_cent_l(strBuf, &r3, len);
    }
}
