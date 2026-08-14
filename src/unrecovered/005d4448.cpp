// ORIGINAL: 0x005D4448 FILE
// RULED-OUT: byte-exact - `pushal`/`popal` at entry has no C++ spelling
//            (documented wall). Landed the nested do/while-with-goto gather
//            loop structurally instead: 9 stack args, an outer per-run
//            counter reset from a saved [ebp+0x24] and an inner index-table
//            lookup vs a direct 16-byte-stride struct-array fallback.
// working copy - scaffold materialised by --work
// name      sub_5d4448
// size      180 bytes
// spans     0x005D4448-0x005D44FC
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d4448  at 0x005D4448  (180 bytes)
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
extern "C" void __cdecl sub_5d4448(
    int *p1, int p2, int p3, int p4, int *p5, int p6, int p7, int p8, int p9)
{
    int saved_count = p8;
    int base8 = p6 - (p4 << 4);
    int base_c = p2 - p3;
    int stride = p7;
    int stride3 = stride * 3;
    int *src = p1;
    char *dst = reinterpret_cast<char *>(p3);
    int count;

    do {
        count = saved_count;
        for (;;) {
            int idx = *src;
            src += 1;
            if (idx < p4) {
                do {
                    int entry = p5[idx] + base_c + reinterpret_cast<int>(dst);
                    *reinterpret_cast<int *>(dst) = *reinterpret_cast<int *>(entry);
                    *reinterpret_cast<int *>(dst + stride) = *reinterpret_cast<int *>(entry + stride);
                    *reinterpret_cast<int *>(dst + stride * 2) = *reinterpret_cast<int *>(entry + stride * 2);
                    *reinterpret_cast<int *>(dst + stride3) = *reinterpret_cast<int *>(entry + stride3);
                    dst += 4;
                    count -= 1;
                    if (count <= 0) goto end_of_run;
                    idx = *src;
                    src += 1;
                } while (idx < p4);
            }
            {
                int entry2 = base8 + idx * 16;
                *reinterpret_cast<int *>(dst) = *reinterpret_cast<int *>(entry2);
                *reinterpret_cast<int *>(dst + stride) = *reinterpret_cast<int *>(entry2 + 4);
                *reinterpret_cast<int *>(dst + stride * 2) = *reinterpret_cast<int *>(entry2 + 8);
                *reinterpret_cast<int *>(dst + stride3) = *reinterpret_cast<int *>(entry2 + 0xc);
                dst += 4;
                count -= 1;
            }
            if (count <= 0) break;
        }
    end_of_run:
        dst += stride3;
        p9 -= 1;
    } while (p9 > 0);
}
