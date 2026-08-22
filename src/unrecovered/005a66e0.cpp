// ORIGINAL: 0x005A66E0 ?encrypt_write@@YAHPAXIIPAUFILE@@@Z 0x005A66E0-0x005A67A8 FILE
// TRIED: nothing ruled out; the [ebp+0xb]/[ebp+0xf]/[ebp+0x13] scratch bytes are stack-slot reuse of a1/a2/a3's high byte, not real locals - modelled as plain unsigned char key1/key2/byte1/byte2 instead. MISMATCH #3, push vs mov, whole-function frame order
// working copy - scaffold materialised by --work
// size      200 bytes
// prototype int (__cdecl ?encrypt_write@@YAHPAXIIPAUFILE@@@Z)(void*, unsigned int, unsigned int, FILE* file)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x0064603F

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?encrypt_write@@YAHPAXIIPAUFILE@@@Z  at 0x005A66E0  (200 bytes)
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
extern "C" int __cdecl _fwrite(void*, size_t, size_t, FILE*);
extern "C" void free(void *);
void * mem_get(int);
int __cdecl encrypt_write(void * a1, unsigned int a2, unsigned int a3, FILE * a4) {
    unsigned char *src = reinterpret_cast<unsigned char *>(a1);
    unsigned int raw_size = a2 * a3;
    unsigned int size = raw_size;
    if ((raw_size & 1) != 0) {
        size = raw_size + 1;
    }
    unsigned char *buf = reinterpret_cast<unsigned char *>(mem_get(size));
    if (buf != 0) {
        unsigned char key1 = 0x80;
        unsigned char key2 = 0x80;
        unsigned int i = 0;
        unsigned char *dst = buf;
        unsigned int pairs = size >> 1;
        if (pairs != 0) {
            do {
                key2 = key2 + 1;
                key1 = key1 - 1;
                unsigned char byte1 = *src;
                unsigned char byte2;
                unsigned char *next = src + 1;
                if (i < raw_size) {
                    byte2 = *next;
                    next = src + 2;
                } else {
                    byte2 = 0;
                }
                unsigned char enc1 = byte1;
                if (byte1 != 0 && (byte1 ^ key2) != 0) {
                    enc1 = byte1 ^ key2;
                }
                unsigned char enc2 = byte2;
                if (byte2 != 0) {
                    unsigned char x = byte2 ^ key1;
                    if (x != 0) {
                        enc2 = x;
                    }
                }
                *dst = enc2;
                dst[1] = enc1;
                dst += 2;
                ++i;
                src = next;
            } while (i < pairs);
        }
        int result = _fwrite(buf, size, 1, a4);
        free(buf);
        return result;
    }
    return 0;
}
