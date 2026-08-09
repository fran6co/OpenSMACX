// ORIGINAL: 0x00443170 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00443170
// name           ?draw_offers@DiploWin@@QAEXXZ
// size           1327 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00443170/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_offers@DiploWin@@QAEXXZ  at 0x00443170  (1327 bytes)
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
class DiploWin;
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    int add(int, int, int, int, int, int);
    void kill_type(int);
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
    int set_font(Font *, Font *, Font *, Font *);
    int write_cent_l(char *, int, int, int, int);
    void set_text_color(int, int, int, int);
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
bool __cdecl has_tech(int, int);
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
extern "C" unsigned int strlen(const char *);
int __cdecl find_font(int, int);
void __cdecl say_tech(char *, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_004436a0 = (int *)0x004436A0;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_006839f4 = (int *)0x006839F4;
static int *const g_006839f8 = (int *)0x006839F8;
static int *const g_0068a5c4 = (int *)0x0068A5C4;
static int *const g_0068fa30 = (int *)0x0068FA30;
static int *const g_0068fa50 = (int *)0x0068FA50;
static int *const g_0078eee4 = (int *)0x0078EEE4;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class DiploWin { public:
    uint8_t pad_0_[0xA1C];
    uint32_t field_a1c_;

    void draw_offers();
};

extern int g_0068fa30_arr[];
extern int g_0068fa50_arr[];
extern char g_00946a50_arr[];
extern char g_00946d34_arr[];
extern char g_0078eee4_arr[];

typedef char *(__cdecl *ItoaFn)(int, char *, int);

void DiploWin::draw_offers() {
    char *self = reinterpret_cast<char *>(this);
    Spot *spot = reinterpret_cast<Spot *>(self + 0xed0);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    char *strBuf = reinterpret_cast<char *>(g_009b86a0);
    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);

    int fontHdr = find_font(0xe, 0);
    int local_10;
    if (*reinterpret_cast<int *>(fontHdr) < 0) {
        local_10 = *reinterpret_cast<int *>(fontHdr + 0xc);
    } else {
        local_10 = *reinterpret_cast<int *>(fontHdr + 0x10) + *reinterpret_cast<int *>(fontHdr);
    }

    int j = 0;
    do {
        spot->kill_type(j);

        Font *fB = reinterpret_cast<Font *>(find_font(0xe, 2));
        Font *fC = reinterpret_cast<Font *>(find_font(0xe, 1));
        Font *fD = reinterpret_cast<Font *>(find_font(0xe, 0));
        buf->set_font(fD, fC, fB, reinterpret_cast<Font *>(0));

        int local_2c, panelY, panelH;
        if (j == 0) {
            panelY = *reinterpret_cast<int *>(self + 0xf60);
            panelH = *reinterpret_cast<int *>(self + 0xf64);
            local_2c = *reinterpret_cast<int *>(self + 0xf5c) + 1 +
                       *reinterpret_cast<int *>(g_0078eee4_arr + *reinterpret_cast<int *>(self + 0xab4) * 0x65c);
        } else {
            local_2c = *reinterpret_cast<int *>(self + 0xf6c);
            panelY = *reinterpret_cast<int *>(self + 0xf70);
            panelH = *reinterpret_cast<int *>(self + 0xf74) +
                     (-1 - *reinterpret_cast<int *>(g_0078eee4_arr + *reinterpret_cast<int *>(self + 0xab8) * 0x65c));
        }
        local_2c = local_2c + 3;
        int local_8 = (panelH - 3) - local_2c;

        int player = (j == 0) ? *reinterpret_cast<int *>(self + 0xab4) : *reinterpret_cast<int *>(self + 0xab8);
        buf->set_text_color(g_0068fa30_arr[player], g_0068fa50_arr[player], 1, 1);

        *reinterpret_cast<char *>(g_009b86a0) = 0;
        player = (j == 0) ? *reinterpret_cast<int *>(self + 0xab4) : *reinterpret_cast<int *>(self + 0xab8);
        *reinterpret_cast<int *>(g_009bbff0) = 0;
        *reinterpret_cast<int *>(g_009bbfec) = *reinterpret_cast<int *>(g_00946a50_arr + player * 0x59c);
        strcat(strBuf, g_00946a50_arr + 0x4c + player * 0x59c);
        strcat(strBuf, reinterpret_cast<char *>(g_00682820));

        player = (j == 0) ? *reinterpret_cast<int *>(self + 0xab4) : *reinterpret_cast<int *>(self + 0xab8);
        *reinterpret_cast<int *>(g_009bbff0) = 0;
        *reinterpret_cast<int *>(g_009bbfec) = *reinterpret_cast<int *>(g_00946a50_arr + player * 0x59c);
        strcat(strBuf, g_00946a50_arr + 0x34 + player * 0x59c);
        strcat(strBuf, reinterpret_cast<char *>(g_00682820));
        *reinterpret_cast<char *>(g_009b86a0) = 0;

        unsigned int slen;
        if (*reinterpret_cast<int *>(self + 0xa1c + j * 4) == 0) {
            int r = strings->get(*reinterpret_cast<int *>(*g_009b90f8 + 0x734));
            strcat(strBuf, reinterpret_cast<char *>(r));
            slen = strlen(strBuf);
        } else {
            int r = strings->get(*reinterpret_cast<int *>(*g_009b90f8 + 0x738));
            strcat(strBuf, reinterpret_cast<char *>(r));
            slen = strlen(strBuf);
        }
        buf->write_cent_l(strBuf, local_2c, panelY + 3, local_8, slen);

        int local_14 = panelY + 3 + local_10;
        int i = 0;
        if (*reinterpret_cast<int *>(self + 0xa1c + j * 4) > 0) {
            int local_1c = local_8;
            int *local_c = reinterpret_cast<int *>(self + 0xa8c);
            int *local_18 = reinterpret_cast<int *>(self + 0xa2c + j * 0x20);
            do {
                *reinterpret_cast<char *>(g_009b86a0) = 0;
                player = (j == 0) ? *reinterpret_cast<int *>(self + 0xab4) : *reinterpret_cast<int *>(self + 0xab8);
                buf->set_text_color(g_0068fa30_arr[player], g_0068fa50_arr[player], 1, 1);

                int r = 0;
                bool haveResult = false;
                switch (*local_18) {
                case 0: {
                    bool special = false;
                    if (j == 1 && *local_c < 0x59) {
                        if (has_tech(*local_c, *reinterpret_cast<int *>(self + 0xab4))) {
                            buf->set_text_color(*g_0068a5c4, 0, j, j);
                            strcat(strBuf, reinterpret_cast<char *>(g_006839f4));
                            char tmp[80];
                            say_tech(tmp, *local_c, j);
                            strcat(strBuf, tmp);
                            strcat(strBuf, reinterpret_cast<char *>(g_006839f8));
                            special = true;
                        }
                    }
                    if (!special) {
                        char tmp[80];
                        say_tech(tmp, *reinterpret_cast<int *>(self + 0xa6c + (i + j * 8) * 4), j);
                        strcat(strBuf, tmp);
                    }
                    break;
                }
                case 1: {
                    char numBuf[80];
                    reinterpret_cast<ItoaFn>(reinterpret_cast<void *>(0x64fc88))(
                        *reinterpret_cast<int *>(self + 0xa6c + (i + j * 8) * 4), numBuf, 10);
                    strcat(strBuf, numBuf);
                    strcat(strBuf, reinterpret_cast<char *>(g_00682820));
                    r = *reinterpret_cast<int *>(*g_009b90f8 + 0x90);
                    haveResult = true;
                    break;
                }
                case 2:
                    r = *reinterpret_cast<int *>(*g_009b90f8 + 0x34c);
                    haveResult = true;
                    break;
                case 3:
                    r = *reinterpret_cast<int *>(*g_009b90f8 + 0x350);
                    haveResult = true;
                    break;
                case 4:
                    r = *reinterpret_cast<int *>(*g_009b90f8 + 0x354);
                    haveResult = true;
                    break;
                case 5: {
                    int r2 = strings->get(*reinterpret_cast<int *>(*g_009b90f8 + 0x980));
                    strcat(strBuf, reinterpret_cast<char *>(r2));
                    strcat(strBuf, reinterpret_cast<char *>(g_00682820));
                    int idx = *reinterpret_cast<int *>(self + 0xa6c + (i + j * 8) * 4);
                    *reinterpret_cast<int *>(g_009bbfec) = *reinterpret_cast<int *>(g_00946d34_arr + 0x18 + idx * 0x59c);
                    *reinterpret_cast<int *>(g_009bbff0) = *reinterpret_cast<int *>(g_00946d34_arr + 0x1c + idx * 0x59c);
                    strcat(strBuf, g_00946d34_arr + idx * 0x59c);
                    break;
                }
                default:
                    haveResult = false;
                    break;
                }
                if (haveResult) {
                    int r3 = strings->get(r);
                    strcat(strBuf, reinterpret_cast<char *>(r3));
                }

                slen = strlen(strBuf);
                buf->write_cent_l(strBuf, local_2c, local_14, local_8, slen);
                int y0 = local_14;
                int h0 = (local_14 + local_10) - local_14;
                local_14 = local_14 + local_10;
                spot->add(i, j, local_2c, y0, local_1c, h0);

                i = i + 1;
                local_18 = local_18 + 1;
                local_c = local_c + 1;
            } while (i < *reinterpret_cast<int *>(self + 0xa1c + j * 4));
        }

        j = j + 1;
        if (j > 1)
            return;
    } while (true);
}
