// ORIGINAL: 0x0062A120 FILE
// RULED-OUT: `fseek` (scaffold declares it nullary) called via a differently
//        named extern `fseek_real` with the real 3-arg signature - the call
//        target address is masked from the comparison, so the rename costs
//        nothing. The flags read/OR/store keeps the POST-or value in one
//        local and reuses it for both the bit-0 test and the `& ~2` clear
//        (matching the original's edx reuse) rather than re-deriving from
//        the pre-or value the way ghidra's uVar1 does - 0.61 -> 0.92
//        mnemonic similarity.
// RULED-OUT: could not reproduce one extra `mov eax,edx`/`mov ecx,edx`
//            register-copy pair the original has ahead of `test al,1`
//            even after removing all other redundant re-reads.
// working copy - scaffold materialised by --work
// name      ?UNK1@Flic@@QAEHHH@Z
// size      226 bytes
// spans     0x0062A120-0x0062A202
// prototype int (__thiscall ?UNK1@Flic@@QAEHHH@Z)(Flic* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3373 0x0062ACD0 0x00646178 0x00647330

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK1@Flic@@QAEHHH@Z  at 0x0062A120  (226 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Spot { public:
    struct SpotInternal {
        RECT rect;
        int type;
        int position;
    };
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
    uint32_t field_4BC_;
    uint8_t field_4C0_[0x4C];
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
extern "C" int __cdecl fseek();

class Flic { public:
    void decode_chunk(DataChunk *);
    int UNK1(int, int);
};
extern "C" int __cdecl fseek_real(void *, long, int);

int Flic::UNK1(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    unsigned int flags = *reinterpret_cast<unsigned int *>(self + 0x5b4) | 2;
    *reinterpret_cast<unsigned int *>(self + 0x5b4) = flags;
    if ((flags & 1) == 0) {
        *reinterpret_cast<unsigned int *>(self + 0x5b4) = flags & 0xfffffffd;
        return 7;
    }

    Buffer *buf = *reinterpret_cast<Buffer **>(self + 0x5a8);
    if (buf->get_data() == 0) {
        *reinterpret_cast<unsigned int *>(self + 0x5b4) =
            *reinterpret_cast<unsigned int *>(self + 0x5b4) & 0xfffffffd;
        return 0x10;
    }

    *reinterpret_cast<int *>(self + 0x5ac) = a1;
    *reinterpret_cast<int *>(self + 0x5b0) = a2;

    if (*reinterpret_cast<unsigned int *>(self + 0xa10) ==
        (unsigned int)*reinterpret_cast<unsigned short *>(self + 0xa1a)) {
        fseek_real(reinterpret_cast<void *>(*reinterpret_cast<int *>(self + 0x5b8)),
                   *reinterpret_cast<int *>(self + 0xa68), 0);
        *reinterpret_cast<int *>(self + 0xa10) = 0;
    }

    _fread(self + 0xab4, 0x10, 1,
           reinterpret_cast<FILE *>(*reinterpret_cast<int *>(self + 0x5b8)));

    decode_chunk(reinterpret_cast<DataChunk *>(self + 0xab4));

    *reinterpret_cast<int *>(self + 0xa10) =
        *reinterpret_cast<int *>(self + 0xa10) + 1;
    *reinterpret_cast<unsigned int *>(self + 0x5b4) =
        *reinterpret_cast<unsigned int *>(self + 0x5b4) & 0xfffffffd;
    return 0;
}
