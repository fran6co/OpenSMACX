// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0062B190
// name           ?decode_delta_flc_trans@Flic@@QAEXXZ
// size           1117 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0062b190/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?decode_delta_flc_trans@Flic@@QAEXXZ  at 0x0062B190  (1117 bytes)
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
    int get_data();
    int get_data(int, int);
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
};

extern "C" int __cdecl _fread(void*, size_t, size_t, FILE*);
extern "C" int abs(int);

class Flic { public:
    void decode_delta_flc_trans();
};
void Flic::decode_delta_flc_trans() {
    unsigned char *self = reinterpret_cast<unsigned char *>(this);

    Buffer *buf1 = *reinterpret_cast<Buffer **>(self + 0x5a8);
    Buffer *buf2 = reinterpret_cast<Buffer *>(self + 4);
    FILE *file = *reinterpret_cast<FILE **>(self + 0x5b8);
    int width2 = *reinterpret_cast<int *>(self + 0x4ac);
    unsigned char colorBase = self[0x5a4];
    unsigned char skipColor = self[0];
    int colOffset = *reinterpret_cast<int *>(self + 0x59c);

    unsigned int minX = 0xffffffff;
    int minY = -1;
    unsigned int maxX = 0;
    int maxY = 0;

    unsigned char *pbVar4 =
        reinterpret_cast<unsigned char *>(buf1->get_data(*reinterpret_cast<int *>(self + 0x5ac),
                                                          *reinterpret_cast<int *>(self + 0x5b0)));
    unsigned char *pbVar5 = reinterpret_cast<unsigned char *>(buf2->get_data());

    unsigned short lineCount;
    _fread(&lineCount, 2, 1, file);

    int row = 0;
    if (lineCount != 0) {
        do {
            unsigned short opcode;
            _fread(&opcode, 2, 1, file);
            while ((opcode & 0xc000) == 0xc000) {
                int skip = static_cast<short>(opcode);
                skip = abs(skip);
                pbVar4 = pbVar4 + buf1->field_4A8_ * (skip & 0xffff);
                pbVar5 = pbVar5 + width2 * (skip & 0xffff);
                _fread(&opcode, 2, 1, file);
            }

            if (row == 0) {
                minY = static_cast<int>(pbVar4 - reinterpret_cast<unsigned char *>(
                                                     buf1->get_data(*reinterpret_cast<int *>(self + 0x5ac),
                                                                    *reinterpret_cast<int *>(self + 0x5b0)))) /
                       buf1->field_4A8_;
            }
            if (row == (lineCount & 0xffff) - 1) {
                maxY = static_cast<int>(pbVar4 - reinterpret_cast<unsigned char *>(
                                                     buf1->get_data(*reinterpret_cast<int *>(self + 0x5ac),
                                                                    *reinterpret_cast<int *>(self + 0x5b0)))) /
                       buf1->field_4A8_;
            }

            if (opcode & 0x8000) {
                unsigned char lastByte = colorBase + static_cast<char>(opcode);
                if (lastByte == skipColor) {
                    pbVar4[colOffset - 1] = pbVar5[colOffset - 1];
                } else {
                    pbVar4[colOffset - 1] = lastByte;
                }
                _fread(&opcode, 2, 1, file);
            }

            int packet = 0;
            unsigned char *pbVar11 = pbVar4;
            if (static_cast<short>(opcode) != 0) {
                do {
                    unsigned char skipCount;
                    _fread(&skipCount, 1, 1, file);
                    pbVar11 = pbVar11 + skipCount;
                    pbVar5 = pbVar5 + skipCount;
                    if (packet == 0 && static_cast<unsigned int>(pbVar11 - pbVar4) < minX) {
                        minX = static_cast<unsigned int>(pbVar11 - pbVar4);
                    }

                    signed char runCount;
                    _fread(&runCount, 1, 1, file);
                    if (runCount >= 1) {
                        unsigned char doubled = static_cast<unsigned char>(runCount << 1);
                        int n = 0;
                        pbVar4 = pbVar4; // unchanged in this branch (matches local_2c reload below)
                        if (doubled != 0) {
                            do {
                                unsigned char raw;
                                _fread(&raw, 1, 1, file);
                                raw = static_cast<unsigned char>(raw + colorBase);
                                if (skipColor == raw) {
                                    *pbVar11 = *pbVar5;
                                } else {
                                    *pbVar11 = raw;
                                }
                                pbVar11++;
                                pbVar5++;
                                n++;
                            } while (n < (doubled & 0xff));
                        }
                    } else {
                        unsigned short word;
                        _fread(&word, 2, 1, file);
                        signed char reps = abs(static_cast<signed char>(runCount));
                        unsigned char lowByte = static_cast<unsigned char>(word);
                        unsigned char highByte = static_cast<unsigned char>(word >> 8);
                        lowByte = colorBase + lowByte;
                        highByte = colorBase + highByte;

                        if (lowByte == skipColor) {
                            if (highByte == skipColor) {
                                int n = 0;
                                if (reps != 0) {
                                    do {
                                        *reinterpret_cast<unsigned short *>(pbVar11) =
                                            *reinterpret_cast<unsigned short *>(pbVar5);
                                        pbVar5 += 2;
                                        pbVar11 += 2;
                                        n++;
                                    } while (n < (reps & 0xff));
                                }
                            } else {
                                int n = 0;
                                if (reps != 0) {
                                    do {
                                        pbVar11[0] = highByte;
                                        pbVar11[1] = pbVar5[1];
                                        pbVar11 += 2;
                                        pbVar5 += 2;
                                        n++;
                                    } while (n < (reps & 0xff));
                                }
                            }
                        } else if (highByte == skipColor) {
                            int n = 0;
                            if (reps != 0) {
                                do {
                                    unsigned char b = *pbVar5;
                                    pbVar5 += 2;
                                    pbVar11[0] = b;
                                    pbVar11[1] = lowByte;
                                    pbVar11 += 2;
                                    n++;
                                } while (n < (reps & 0xff));
                            }
                        } else {
                            int n = 0;
                            if (reps != 0) {
                                do {
                                    *reinterpret_cast<unsigned short *>(pbVar11) =
                                        static_cast<unsigned short>(word);
                                    pbVar11 += 2;
                                    pbVar5 += 2;
                                    n++;
                                } while (n < (reps & 0xff));
                            }
                        }
                    }

                    packet++;
                } while (packet < (opcode & 0xffff));
            }

            if (maxX < static_cast<unsigned int>(pbVar11 - pbVar4)) {
                maxX = static_cast<unsigned int>(pbVar11 - pbVar4);
            }
            pbVar4 = pbVar4 + buf1->field_4A8_;
            pbVar5 = pbVar5 + width2;
            row++;
        } while (row < (lineCount & 0xffff));
    }

    *reinterpret_cast<unsigned int *>(self + 0x58c) =
        *reinterpret_cast<int *>(self + 0x5ac) + minX;
    *reinterpret_cast<int *>(self + 0x590) = *reinterpret_cast<int *>(self + 0x5b0) + minY;
    *reinterpret_cast<unsigned int *>(self + 0x594) =
        *reinterpret_cast<int *>(self + 0x5ac) + 1 + maxX;
    *reinterpret_cast<int *>(self + 0x598) = *reinterpret_cast<int *>(self + 0x5b0) + 1 + maxY;
}
