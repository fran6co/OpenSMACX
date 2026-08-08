// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0062A720
// name           ?decode_delta_flc@Flic@@QAEXXZ
// size           967 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0062a720/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?decode_delta_flc@Flic@@QAEXXZ  at 0x0062A720  (967 bytes)
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
    int get_data(int, int);
};
extern "C" int __cdecl _fread(void*, size_t, size_t, FILE*);
extern "C" int abs(int);
extern "C" void free(void *);
void * __cdecl mem_get(int);

class Flic { public:
    void decode_delta_flc();
};

void Flic::decode_delta_flc() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = *reinterpret_cast<Buffer **>(self + 0x5a8);
    int arg_5ac = *reinterpret_cast<int *>(self + 0x5ac);
    int arg_5b0 = *reinterpret_cast<int *>(self + 0x5b0);
    FILE *file = *reinterpret_cast<FILE **>(self + 0x5b8);
    int comp_flag = *reinterpret_cast<int *>(self + 0x5a4);
    signed char delta = *reinterpret_cast<signed char *>(self + 0x5a4);

    void *work_buf = 0;
    if (comp_flag != 0) {
        int stride0 = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x4a8);
        work_buf = mem_get(stride0);
    }
    if (comp_flag == 0 || work_buf != 0) {
        char *row = reinterpret_cast<char *>(buf->get_data(arg_5ac, arg_5b0));
        unsigned short num_lines;
        _fread(&num_lines, 2, 1, file);
        unsigned int min_touched = 0xffffffff;
        int first_row = -1;
        unsigned int max_touched = 0;
        int last_row = 0;
        int line_index = 0;
        if (static_cast<short>(num_lines) != 0) {
            do {
                unsigned short line_hdr;
                for (;;) {
                    _fread(&line_hdr, 2, 1, file);
                    if ((line_hdr & 0xc000) != 0xc000) {
                        break;
                    }
                    int stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x4a8);
                    row += abs(static_cast<short>(line_hdr)) * stride;
                }
                if (line_index == 0) {
                    int stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x4a8);
                    char *base = reinterpret_cast<char *>(buf->get_data(arg_5ac, arg_5b0));
                    first_row = static_cast<int>(row - base) / stride;
                }
                if (line_index == static_cast<int>(num_lines & 0xffff) - 1) {
                    int stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x4a8);
                    char *base = reinterpret_cast<char *>(buf->get_data(arg_5ac, arg_5b0));
                    last_row = static_cast<int>(row - base) / stride;
                }
                if (line_hdr & 0x8000) {
                    max_touched = *reinterpret_cast<unsigned int *>(self + 0x59c);
                    row[max_touched - 1] = static_cast<char>(line_hdr) + delta;
                    _fread(&line_hdr, 2, 1, file);
                }
                int packet_index = 0;
                char *dst = row;
                if (static_cast<short>(line_hdr) != 0) {
                    do {
                        unsigned char skip_count = 0;
                        _fread(&skip_count, 1, 1, file);
                        dst += skip_count;
                        if (packet_index == 0 && static_cast<unsigned int>(dst - row) < min_touched) {
                            min_touched = static_cast<unsigned int>(dst - row);
                        }
                        signed char run_count = 0;
                        _fread(&run_count, 1, 1, file);
                        if (run_count < 1) {
                            if (run_count < 0) {
                                unsigned short pixel;
                                _fread(&pixel, 2, 1, file);
                                unsigned char n = static_cast<unsigned char>(abs(run_count));
                                unsigned char lo = static_cast<unsigned char>(pixel) + delta;
                                unsigned char hi = static_cast<unsigned char>(pixel >> 8) + delta;
                                unsigned short packed = static_cast<unsigned short>((static_cast<unsigned short>(hi) << 8) | lo);
                                for (int i = 0; i < static_cast<int>(n); ++i) {
                                    *reinterpret_cast<unsigned short *>(dst) = packed;
                                    dst += 2;
                                }
                            }
                        } else if (comp_flag == 0) {
                            _fread(dst, 2, static_cast<unsigned char>(run_count), file);
                            dst += static_cast<unsigned char>(run_count) * 2;
                        } else {
                            unsigned char rc = static_cast<unsigned char>(run_count);
                            _fread(work_buf, 2, rc, file);
                            unsigned char *src = reinterpret_cast<unsigned char *>(work_buf);
                            int n2 = rc * 2;
                            for (int i = 0; i < n2; ++i) {
                                dst[i] = static_cast<char>(src[i] + delta);
                            }
                            dst += n2;
                        }
                        ++packet_index;
                    } while (packet_index < static_cast<int>(line_hdr & 0xffff));
                }
                if (max_touched < static_cast<unsigned int>(dst - row)) {
                    max_touched = static_cast<unsigned int>(dst - row);
                }
                int stride = *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 0x4a8);
                row += stride;
                ++line_index;
            } while (line_index < static_cast<int>(num_lines & 0xffff));
        }
        if (work_buf != 0) {
            free(work_buf);
        }
        *reinterpret_cast<unsigned int *>(self + 0x58c) = arg_5ac + min_touched;
        *reinterpret_cast<int *>(self + 0x590) = arg_5b0 + first_row;
        *reinterpret_cast<unsigned int *>(self + 0x594) = arg_5ac + 1 + max_touched;
        *reinterpret_cast<int *>(self + 0x598) = arg_5b0 + 1 + last_row;
    }
}
