// ORIGINAL: 0x0062ACD0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0062ACD0
// name           ?decode_chunk@Flic@@QAEXPAUDataChunk@@@Z
// size           541 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0062acd0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?decode_chunk@Flic@@QAEXPAUDataChunk@@@Z  at 0x0062ACD0  (541 bytes)
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
struct DataChunk;
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
    int copy(Buffer *, int, int, int, int, int, int);
    int fill(int, int, int, int, int);
};
extern "C" int __cdecl _fread(void*, size_t, size_t, FILE*);
extern "C" int __cdecl fseek();
extern "C" int __cdecl ftell();
extern "C" int abs(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0062aef0 = (int *)0x0062AEF0;
static int *const g_0062af18 = (int *)0x0062AF18;

class Flic { public:
    void decode_byte_run();
    void decode_delta_flc();
    void decode_fli_copy();
    void decode_byte_run_trans();
    void decode_delta_flc_trans();
    void decode_fli_copy_trans();
    void decode_chunk(DataChunk *);
};

struct DataChunk {
    int32_t size;
    uint16_t type;
    uint16_t count;
};

// `fseek`/`ftell` are declared nullary above (the emitter's placeholder
// for an unknown-arity CRT extern); cast through a real prototype to call
// them with arguments, same trick the sibling FileWin recoveries use for
// `_strcmpi`.
typedef int(__cdecl *FseekFn)(FILE *, long, int);
typedef int(__cdecl *FtellFn)(FILE *);

// MISMATCH (#3: original 'mov' vs rebuilt 'push' - the original loads
// `ebx = *(int*)edi` (the chunk size) once up front and carries it in a
// callee-saved register across both switches; this rebuild re-derives it
// from the frame instead. A faithful translation of every call site,
// argument order and indirect-call target read off the disassembly (the
// trans/non-trans split collapses to one switch with the flag tested
// per-case, since both switches share one physical default handler at
// 0x62aea1). Register-allocation wall, not re-timeboxed given the size.
void Flic::decode_chunk(DataChunk * a1) {
    char *self = reinterpret_cast<char *>(this);

    if (a1->type == 0xf1fa) {
        int count = a1->count;
        if (count <= 0) {
            return;
        }
        do {
            DataChunk chunk;
            _fread(&chunk, 6, 1, *reinterpret_cast<FILE **>(self + 0x5b8));
            decode_chunk(&chunk);
        } while (--count != 0);
        return;
    }

    int size = a1->size;
    int start = (*reinterpret_cast<FtellFn *>(&ftell))(*reinterpret_cast<FILE **>(self + 0x5b8));
    bool trans = (*reinterpret_cast<uint8_t *>(self + 0x5b4) & 2) != 0;

    switch (a1->type) {
    case 7:
        if (trans) {
            decode_delta_flc_trans();
        } else {
            decode_delta_flc();
        }
        break;
    case 0xd: {
        int left = *reinterpret_cast<int32_t *>(self + 0x5ac);
        int top = *reinterpret_cast<int32_t *>(self + 0x5b0);
        int width = *reinterpret_cast<int32_t *>(self + 0x59c);
        int height = *reinterpret_cast<int32_t *>(self + 0x5a0);
        *reinterpret_cast<int32_t *>(self + 0x58c) = left;
        *reinterpret_cast<int32_t *>(self + 0x590) = top;
        *reinterpret_cast<int32_t *>(self + 0x594) = left + width;
        *reinterpret_cast<int32_t *>(self + 0x598) = top + height;
        Buffer *other = *reinterpret_cast<Buffer **>(self + 0x5a8);
        if (trans) {
            if (*self != 0) {
                reinterpret_cast<Buffer *>(self + 4)->copy(other, 0, 0, left, top, width, height);
            }
        } else {
            other->fill(left, top, width, height, 0);
        }
        break;
    }
    case 0xf:
        if (trans) {
            decode_byte_run_trans();
        } else {
            decode_byte_run();
        }
        break;
    case 0x10:
        if (trans) {
            decode_fli_copy_trans();
        } else {
            decode_fli_copy();
        }
        break;
    default:
        (*reinterpret_cast<FseekFn *>(&fseek))(*reinterpret_cast<FILE **>(self + 0x5b8), size - 6, 1);
        break;
    }

    int end = (*reinterpret_cast<FtellFn *>(&ftell))(*reinterpret_cast<FILE **>(self + 0x5b8));
    int delta = abs(start - 6 + size - end);
    if (delta != 0) {
        (*reinterpret_cast<FseekFn *>(&fseek))(*reinterpret_cast<FILE **>(self + 0x5b8), delta, 1);
    }
}
