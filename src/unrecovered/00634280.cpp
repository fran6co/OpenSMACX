// ORIGINAL: 0x00634280 sub_634280 0x00634280-0x00634326 FILE
// RULED-OUT: `if (dr == 1) { ...; return deflateEnd_real(); } else {...}` flipped je/jne against the original (0.78 sim). `if (dr != 1) {cleanup path} ...; success path falls through` matches polarity (0.95 sim); the remaining diff is a duplicated epilogue the compiler didn't share between the two returns.
// working copy - scaffold materialised by --work
// size      166 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063CF00 0x0063D1A0 0x0063D4D0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634280  at 0x00634280  (166 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl deflateInit_();
extern "C" int __cdecl sub_63d1a0();
extern "C" int __cdecl sub_63d4d0();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00697f28 = (int *)0x00697F28;

// zlib z_stream_s, sizeof == 0x38 (matches the `push 0x38` size arg).
struct ZStream634280 {
    unsigned char *next_in;
    unsigned int avail_in;
    unsigned long total_in;
    unsigned char *next_out;
    unsigned int avail_out;
    unsigned long total_out;
    char *msg;
    void *state;
    void *zalloc;
    void *zfree;
    void *opaque;
    int data_type;
    unsigned long adler;
    unsigned long reserved;
};

// deflateInit_/sub_63d1a0(deflate)/sub_63d4d0(deflateEnd) above are declared
// nullary by the scaffold; redeclare with their real signatures under
// different names to avoid C2733.
extern "C" int __cdecl deflateInit_real(void *strm, int level, const char *version, int stream_size);
extern "C" int __cdecl deflate_real(void *strm, int flush);
extern "C" int __cdecl deflateEnd_real(void *strm);

extern "C" int __cdecl sub_634280(unsigned char *next_out, unsigned long *avail_out_ptr, unsigned char *next_in, unsigned int avail_in) {
    ZStream634280 stream;
    stream.next_in = next_in;
    stream.avail_in = avail_in;
    stream.next_out = next_out;
    stream.avail_out = *avail_out_ptr;
    stream.zalloc = 0;
    stream.zfree = 0;
    stream.opaque = 0;
    int rc = deflateInit_real(&stream, -1, reinterpret_cast<const char *>(g_00697f28), sizeof(ZStream634280));
    if (rc != 0) {
        return rc;
    }
    int dr = deflate_real(&stream, 4);
    if (dr != 1) {
        deflateEnd_real(&stream);
        rc = -5;
        if (dr != 0) {
            return dr;
        }
        return rc;
    }
    *avail_out_ptr = stream.total_out;
    return deflateEnd_real(&stream);
}
