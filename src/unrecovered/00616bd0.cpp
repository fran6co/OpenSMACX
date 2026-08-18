// ORIGINAL: 0x00616BD0 ?init@CaviarData@@QAEHPAD@Z 0x00616BD0-0x00616C55 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      133 bytes
// prototype int (__thiscall ?init@CaviarData@@QAEHPAD@Z)(CaviarData* this, int8*)
// callers   1   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006381F0 0x00638430 0x006493C6 0x0064CF73

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@CaviarData@@QAEHPAD@Z  at 0x00616BD0  (133 bytes)
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

class CaviarData;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl _open();
extern "C" int __cdecl sub_6381f0();
extern "C" int __cdecl sub_638430();
extern "C" int _close(int);

class CaviarData { public:
    uint32_t field_0_;
    uint32_t fileDescriptor_;
    void * record_;

    int init(char *);
};
extern "C" int __cdecl caviar_sub_638430(int);
extern "C" int __cdecl caviar_open(const char *, int);
extern "C" int __cdecl caviar_sub_6381f0(int);

int CaviarData::init(char * a1) {
    if (a1 == 0) {
        return 0x10;
    }
    if (record_) {
        caviar_sub_638430(reinterpret_cast<int>(record_));
        record_ = 0;
    }
    int fd = caviar_open(a1, 0x8000);
    fileDescriptor_ = static_cast<uint32_t>(fd);
    if (fd == -1) {
        return 6;
    }
    int rec = caviar_sub_6381f0(fd);
    record_ = reinterpret_cast<void *>(rec);
    if (rec == 0) {
        return 0x17;
    }
    if (fileDescriptor_ != 0) {
        _close(static_cast<int>(fileDescriptor_));
        fileDescriptor_ = 0;
    }
    return 0;
}
