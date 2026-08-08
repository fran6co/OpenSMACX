// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0062AF40
// name           ?decode_byte_run_trans@Flic@@QAEXXZ
// size           582 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0062af40/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?decode_byte_run_trans@Flic@@QAEXXZ  at 0x0062AF40  (582 bytes)
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
struct FILE;
class Flic;
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
    int get_data();
    int get_data(int, int);
};
extern "C" int __cdecl _fread(void*, size_t, size_t, FILE*);
extern "C" int abs(int);
extern "C" void *memcpy(void *, const void *, unsigned int);
extern "C" void *memset(void *, int, unsigned int);
extern "C" void free(void *);
void * __cdecl mem_get(int);

class Flic { public:
    void decode_byte_run_trans();
};

void Flic::decode_byte_run_trans() {
    char *self = reinterpret_cast<char *>(this);
    void *freePtr = 0;

    if (*reinterpret_cast<int *>(self + 0x5a4) == 0 ||
        (freePtr = mem_get(*reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 0x5a8) + 0x4a8)), freePtr != 0)) {

        *reinterpret_cast<int *>(self + 0x58c) = *reinterpret_cast<int *>(self + 0x5ac);
        *reinterpret_cast<int *>(self + 0x590) = *reinterpret_cast<int *>(self + 0x5b0);
        *reinterpret_cast<int *>(self + 0x594) = *reinterpret_cast<int *>(self + 0x5ac) + *reinterpret_cast<int *>(self + 0x59c);
        *reinterpret_cast<int *>(self + 0x598) = *reinterpret_cast<int *>(self + 0x5b0) + *reinterpret_cast<int *>(self + 0x5a0);

        uint8_t *dst = reinterpret_cast<uint8_t *>(
            reinterpret_cast<Buffer *>(*reinterpret_cast<int *>(self + 0x5a8))
                ->get_data(*reinterpret_cast<int *>(self + 0x5ac), *reinterpret_cast<int *>(self + 0x5b0)));
        uint8_t *rowDst = dst;
        uint8_t *src = reinterpret_cast<uint8_t *>(reinterpret_cast<Buffer *>(self + 4)->get_data());
        uint8_t *rowSrc = src;

        int row = 0;
        if (*reinterpret_cast<int *>(self + 0x5a0) > 0) {
            do {
                int remaining = *reinterpret_cast<int *>(self + 0x59c);
                src = rowSrc;
                unsigned int count;
                _fread(&count, 1, 1, *reinterpret_cast<FILE **>(self + 0x5b8));
                dst = rowDst;
                for (; remaining > 0; remaining -= (count & 0xff)) {
                    _fread(&count, 1, 1, *reinterpret_cast<FILE **>(self + 0x5b8));
                    if (static_cast<signed char>(count) < 1) {
                        signed char n = static_cast<signed char>(abs(static_cast<signed char>(count)));
                        int i = 0;
                        if (n != 0) {
                            do {
                                unsigned int b;
                                _fread(&b, 1, 1, *reinterpret_cast<FILE **>(self + 0x5b8));
                                uint8_t val = static_cast<uint8_t>(b) + *reinterpret_cast<uint8_t *>(self + 0x5a4);
                                if (val == *reinterpret_cast<uint8_t *>(self)) {
                                    *dst = *src;
                                } else {
                                    *dst = val;
                                }
                                ++dst;
                                ++src;
                                ++i;
                            } while (i < (n & 0xff));
                        }
                    } else {
                        unsigned int b;
                        _fread(&b, 1, 1, *reinterpret_cast<FILE **>(self + 0x5b8));
                        uint8_t val = static_cast<uint8_t>(b) + *reinterpret_cast<uint8_t *>(self + 0x5a4);
                        if (val == *reinterpret_cast<uint8_t *>(self)) {
                            memcpy(dst, src, count & 0xff);
                        } else {
                            memset(dst, val, count & 0xff);
                        }
                        dst += (count & 0xff);
                        src += (count & 0xff);
                    }
                }
                dst = rowDst + *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 0x5a8) + 0x4a8);
                src = rowSrc + *reinterpret_cast<int *>(self + 0x4ac);
                ++row;
                rowDst = dst;
                rowSrc = src;
            } while (row < *reinterpret_cast<int *>(self + 0x5a0));
        }

        if (freePtr != 0) {
            free(freePtr);
        }
    }
}
