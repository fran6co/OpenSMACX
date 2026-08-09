// ORIGINAL: 0x00612450 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00612450
// name           ?on_redraw@EditGroup@@QAEXXZ
// size           537 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00612450/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@EditGroup@@QAEXXZ  at 0x00612450  (537 bytes)
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
class EditGroup;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
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
    int copy(Buffer *, int, int, int, int, int, int);
    int write_l(char *, RECT *, int);
};
extern "C" int abs(int);
extern "C" unsigned int strlen(const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7ab8 = (int *)0x009B7AB8;

class EditGroup { public:
    void on_redraw();
};

void EditGroup::on_redraw() {
    char *self = reinterpret_cast<char *>(this);

    int curLine = *reinterpret_cast<int *>(
        *reinterpret_cast<int *>(*reinterpret_cast<int *>(self - 0x8c) + 4) +
        reinterpret_cast<int>(self) + 0x38);
    if (curLine == 0) {
        return;
    }
    *g_009b7ab8 = curLine;

    int rowPtr;
    if (reinterpret_cast<int>(self - 0x8c) != 0) {
        rowPtr = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self - 0x8c) + 4) +
                 reinterpret_cast<int>(self) + 0x3b8;
    } else {
        rowPtr = 0;
    }

    int baseA = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self - 0x8c) + 4) +
                reinterpret_cast<int>(self);
    int baseB = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self - 0x8c) + 8) +
                reinterpret_cast<int>(self);

    int negWidth = -*reinterpret_cast<int *>(baseA + 0x43c);
    int strideArg = *reinterpret_cast<int *>(baseA + 0x438);
    int p2 = *reinterpret_cast<int *>(baseB - 0x64);
    int p1 = *reinterpret_cast<int *>(baseB - 0x68);

    reinterpret_cast<Buffer *>(curLine + 0x444)
        ->copy(reinterpret_cast<Buffer *>(rowPtr), p1, p2, 0, 0, strideArg, negWidth);

    int base3 = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self - 0x8c) + 8) +
                reinterpret_cast<int>(self);
    int rowOriginY = *reinterpret_cast<int *>(base3 - 0x58) + *reinterpret_cast<int *>(base3 - 0x4c);
    int halfWidthSrc = *reinterpret_cast<int *>(base3 - 0x48);
    int halfWidth0 = halfWidthSrc >> 1;
    int baseX = *reinterpret_cast<int *>(base3 - 0x28) + rowOriginY;
    int baseY = (*reinterpret_cast<int *>(base3 - 0x24) - halfWidthSrc) + halfWidth0;

    if (*reinterpret_cast<int *>(base3 + 0x40) - 1 >= -1) {
        *reinterpret_cast<int *>(base3 + 0x3c) = *reinterpret_cast<int *>(base3 + 0x38);
        int n1 = abs(-1);
        if (n1 <= *reinterpret_cast<int *>(base3 + 0x40)) {
            int n2 = abs(-1);
            if (n2 > 0) {
                do {
                    int cur = *reinterpret_cast<int *>(base3 + 0x3c);
                    *reinterpret_cast<int *>(base3 + 0x3c) = *reinterpret_cast<int *>(cur + 0x10);
                } while (--n2);
            }
            *reinterpret_cast<int *>(base3 + 0x44) = *reinterpret_cast<int *>(base3 + 0x40) - 1;
        }
    }

    int arrBase = *reinterpret_cast<int *>(self - 0x8c) + 8;
    int i = 0;
    if (*reinterpret_cast<int *>(*reinterpret_cast<int *>(arrBase) + reinterpret_cast<int>(self) - 0x38) > 0) {
        int halfWidth = halfWidth0;
        do {
            int stride = *reinterpret_cast<int *>(*reinterpret_cast<int *>(arrBase) + reinterpret_cast<int>(self) - 0x28);
            int strideI = stride * i;

            RECT rect;
            rect.left = strideI + rowOriginY;
            rect.top = halfWidth;
            rect.right = baseX + strideI;
            rect.bottom = baseY;

            int rowsPtr = *reinterpret_cast<int *>(arrBase);
            int rowBase = rowsPtr + reinterpret_cast<int>(self);
            int colCount = *reinterpret_cast<int *>(rowBase - 0x34);
            if (colCount > 0) {
                int j = 0;
                do {
                    int selPtr = *reinterpret_cast<int *>(rowBase + 0x38);
                    if (selPtr != 0) {
                        int cur0 = *reinterpret_cast<int *>(rowBase + 0x3c);
                        int cnt = *reinterpret_cast<int *>(rowBase + 0x40);
                        int nxt = *reinterpret_cast<int *>(cur0 + 0xc);
                        *reinterpret_cast<int *>(rowBase + 0x3c) = nxt;
                        int idx = *reinterpret_cast<int *>(rowBase + 0x44) + 1;
                        if (idx == cnt) {
                            idx = 0;
                        }
                        *reinterpret_cast<int *>(rowBase + 0x44) = idx;
                    }

                    int cur2 = *reinterpret_cast<int *>(rowBase + 0x3c);
                    int textVal;
                    if (cur2 == 0) {
                        textVal = 0;
                    } else {
                        int tmp = (selPtr != 0) ? *reinterpret_cast<int *>(cur2 + 8) : 0;
                        textVal = *reinterpret_cast<int *>(tmp + 4);
                    }

                    Buffer *textBuf = reinterpret_cast<Buffer *>(
                        *reinterpret_cast<int *>(*reinterpret_cast<int *>(self - 0x8c) + 4) +
                        reinterpret_cast<int>(self) + 0x3b8);
                    if (textVal != 0) {
                        unsigned int len = strlen(reinterpret_cast<char *>(textVal));
                        textBuf->write_l(reinterpret_cast<char *>(textVal), &rect, len);
                    }

                    int stride2a = *reinterpret_cast<int *>(
                        *reinterpret_cast<int *>(*reinterpret_cast<int *>(self - 0x8c) + 8) +
                        reinterpret_cast<int>(self) - 0x24);
                    rect.top += stride2a;
                    int stride2b = *reinterpret_cast<int *>(
                        *reinterpret_cast<int *>(*reinterpret_cast<int *>(self - 0x8c) + 8) +
                        reinterpret_cast<int>(self) - 0x24);
                    rect.bottom += stride2b;

                    rowsPtr = *reinterpret_cast<int *>(arrBase);
                    rowBase = rowsPtr + reinterpret_cast<int>(self);
                    ++j;
                } while (j < colCount);
                halfWidth = halfWidth0;
            }
            ++i;
        } while (i < *reinterpret_cast<int *>(*reinterpret_cast<int *>(arrBase) + reinterpret_cast<int>(self) - 0x38));
    }
}
