// ORIGINAL: 0x005A67B0 ?encrypt_read@@YAHPAXIIPAUFILE@@@Z 0x005A67B0-0x005A6886 FILE
// working copy - scaffold materialised by --work
// size      214 bytes
// prototype int (__cdecl ?encrypt_read@@YAHPAXIIPAUFILE@@@Z)(void* buffer, unsigned int block, unsigned int count, FILE* file)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x00646178

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?encrypt_read@@YAHPAXIIPAUFILE@@@Z  at 0x005A67B0  (214 bytes)
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

struct FILE;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl _fread(void*, size_t, size_t, FILE*);
extern "C" void free(void *);
void * mem_get(int);
int __cdecl encrypt_read(void * a1, unsigned int a2, unsigned int a3, FILE * a4) {
    unsigned int total = a2 * a3;
    unsigned int allocSize = total;
    if (allocSize & 1) {
        allocSize = allocSize + 1;
    }
    unsigned char *buf = reinterpret_cast<unsigned char *>(mem_get(allocSize));
    if (buf == 0) {
        return 0;
    }
    int n = _fread(buf, allocSize, 1, a4);
    if (n == 0) {
        free(buf);
        return 0;
    }
    unsigned char prevA = 0x80;
    unsigned char prevB = 0x80;
    unsigned char *src = buf;
    unsigned char *dst = reinterpret_cast<unsigned char *>(a1);
    unsigned int remaining = total;
    for (unsigned int pairs = allocSize >> 1; pairs != 0; --pairs) {
        ++prevB;
        unsigned char b1 = src[1];
        --prevA;
        unsigned char b0 = src[0];
        src += 2;
        if (b1 != 0 && (unsigned char)(b1 ^ prevB) != 0) {
            b1 = b1 ^ prevB;
        }
        if (b0 != 0 && (unsigned char)(b0 ^ prevA) != 0) {
            b0 = b0 ^ prevA;
        }
        *dst = b1;
        unsigned char *next = dst + 1;
        if (remaining > 1) {
            *next = b0;
            next = dst + 2;
        }
        remaining -= 2;
        dst = next;
    }
    free(buf);
    return 1;
}
