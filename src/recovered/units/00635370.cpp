// ORIGINAL: 0x00635370 ?get_tex_mem@TexHeap@@QAEHH@Z 0x00635370-0x006353BD FILE
// size      77 bytes
// prototype int (__thiscall ?get_tex_mem@TexHeap@@QAEHH@Z)(TexHeap* this, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4680
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00635370
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00635370/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_tex_mem@TexHeap@@QAEHH@Z  at 0x00635370  (77 bytes)
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

class Heap;
typedef void * LPVOID;
class TexHeap;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    void * get(int);
};

class TexHeap { public:
    int get_tex_mem(int);
};
// Closest reached: MISMATCH at instruction #3. Logic, field offsets and the
// mid-loop `cmp [node+0x10], a1; jge` polarity all match the original, but
// this toolchain gives the rebuild a push-ebp/mov-ebp,esp frame and only
// two callee-saved pushes (esi, edi) where the original manages three
// (ebx, esi, edi) with no frame at all, plus a bottom-of-loop `jl` back
// edge here instead of a `jge`-then-`jmp`. Ruled out as a source-form
// issue: array indexing and manual pointer-walking (matching the
// original's `add edx,4` stride) both produced byte-identical rebuilds,
// and reducing 0x00617670 - a function that DID reach BYTE_EXACT with a
// stack parameter and no frame - to a two-statement body reintroduces the
// same spurious frame there too. So frame use here tracks total
// register pressure across the WHOLE function in a way no local rewrite
// of this body's shape moved; a register allocator/scheduler wall, not a
// source-form one.
int TexHeap::get_tex_mem(int a1) {
    if (*reinterpret_cast<int *>(this) != 0) {
        int n = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xc) + 1;
        if (n > 0) {
            Heap **arr = *reinterpret_cast<Heap ***>(
                reinterpret_cast<char *>(this) + 8);
            int i = 0;
            do {
                Heap *node = arr[i];
                if ((*reinterpret_cast<unsigned int *>(
                        reinterpret_cast<char *>(node) + 8) & 0xffff) == 0 &&
                    *reinterpret_cast<int *>(
                        reinterpret_cast<char *>(node) + 0x10) >= a1) {
                    return reinterpret_cast<int>(node->get(a1));
                }
                i++;
            } while (i < n);
        }
    }
    return 0;
}
