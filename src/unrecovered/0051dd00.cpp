// ORIGINAL: 0x0051DD00 sub_51dd00 0x0051DD00-0x0051DDE0 FILE BYTE_EXACT
// symbol    ?reset@Obj0051DD00@@QAEXXZ
// working copy - scaffold materialised by --work
// size      224 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004E25B0 0x005900D0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_51dd00  at 0x0051DD00  (224 bytes)
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

class AlphaNet;
class Lock;

// ---- callees, declared and never defined (a definition would be inlined) ----
class AlphaNet { public:
    uint32_t field_0_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint8_t field_2C_[0x18];
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint8_t field_5C_[0x54];
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    uint32_t field_C4_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint8_t field_E8_[0x6C];
    uint32_t field_154_;
    uint32_t field_158_;
    uint32_t field_15C_;
    uint8_t field_160_[0x4];
    uint32_t field_164_;
    uint8_t field_168_;
    uint8_t field_169_[0x56B];
    uint32_t field_6D4_;
    uint8_t field_6D8_[0x4];
    uint32_t field_6DC_;
    uint32_t field_6E0_;
    uint32_t field_6E4_;
    uint32_t field_6E8_;
    uint8_t field_6EC_[0x10];
    uint32_t field_6FC_;
    uint32_t field_700_;
    uint32_t field_704_;
    uint32_t field_708_;
    uint32_t field_70C_;
    uint32_t field_710_;
    uint32_t field_714_;
    uint32_t field_718_;
    uint32_t field_71C_;
    uint32_t field_720_;
    uint8_t field_724_[0x8];
    uint32_t field_72C_;
    uint32_t field_730_;
    uint32_t field_734_;
    uint32_t field_738_;
    uint32_t field_73C_;
    uint32_t field_740_;
    uint32_t field_744_;
    uint8_t field_748_[0x8];
    uint32_t field_750_;
    uint32_t field_754_;
    uint32_t field_758_;
    uint32_t field_75C_;
    uint32_t field_760_;
    uint32_t field_764_;
    uint8_t field_768_[0x18];
    uint8_t data_[0xD20];
    void close();
};

class Lock { public:
    void clear();
};

class Obj0051DD00 { public:
    void reset();
};

void Obj0051DD00::reset() {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0x1b30) = 0;
    *reinterpret_cast<int *>(self + 0x1b34) = 0;
    *reinterpret_cast<int *>(self + 0x1b38) = 0;
    *reinterpret_cast<int *>(self + 0x1b3c) = 0;
    *reinterpret_cast<int *>(self + 0x1b60) = 0;
    *reinterpret_cast<int *>(self + 0x1b5c) = 0;
    *reinterpret_cast<int *>(self + 0x1b6c) = 0;
    *reinterpret_cast<int *>(self + 0x1b68) = 0;
    *reinterpret_cast<int *>(self + 0x1b7c) = 0;
    *reinterpret_cast<int *>(self + 0x1ba8) = 0;
    *reinterpret_cast<int *>(self + 0x1ba0) = 0;
    *reinterpret_cast<int *>(self + 0x1ba4) = 0;
    *reinterpret_cast<int *>(self + 0x1bcc) = 0;
    *reinterpret_cast<int *>(self + 0x1bc8) = 0;
    *reinterpret_cast<int *>(self + 0x1b50) = 0;
    *reinterpret_cast<int *>(self + 0x1b54) = 0;
    *reinterpret_cast<int *>(self + 0x1b58) = 0;
    *reinterpret_cast<int *>(self + 0x1bc4) = 0;
    *reinterpret_cast<int *>(self + 0x1bc0) = 0;
    *reinterpret_cast<int *>(self + 0x1bb0) = 0;
    *reinterpret_cast<int *>(self + 0x1bac) = 0;
    *reinterpret_cast<int *>(self + 0x1bb4) = 0;
    *reinterpret_cast<int *>(self + 0x1b40) = 0;
    *reinterpret_cast<int *>(self + 0x1b44) = 0;
    *reinterpret_cast<int *>(self + 0x1bd8) = 0;
    *reinterpret_cast<int *>(self + 0x1bd4) = 0;
    *reinterpret_cast<int *>(self + 0x1b48) = 0;
    *reinterpret_cast<int *>(self + 0x1b4c) = 0;
    *reinterpret_cast<int *>(self + 0x1bd0) = 0xff;
    *reinterpret_cast<int *>(self + 0x1b78) = 0;

    int *dst = reinterpret_cast<int *>(self + 0x1b80);
    for (int i = 8; i != 0; --i) {
        *dst = 0;
        ++dst;
    }

    reinterpret_cast<Lock *>(self + 0x14a0)->clear();
    reinterpret_cast<AlphaNet *>(self)->close();
}
