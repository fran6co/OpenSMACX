// ORIGINAL: 0x00633D90 ?get@NetFifo@@QAEHPAXPAIPAH1@Z 0x00633D90-0x00633E75 FILE
// TRIED: bare memcpy() gets intrinsic-expanded to rep movsd/movsb; `#pragma function(memcpy)` forces the real call, matching. a2 is compared as a raw dword (never dereferenced) - the given "unsigned int *" contract is likely wrong (should be a plain unsigned int), kept as-is to match the existing class member declaration and cast at the compare site instead (see PROPOSAL). Ghidra's redundant double-null-check structure (`if(p!=0){ while(...){...} if(p!=0){...}}`) reproduces the original's extra `test/je` pair around the match; a single flat `if(cur==0) return; while(...); <process>` (this candidate) drops it as dead code the optimizer removes - 4 edit ops, 200/229 bytes, still the closest reached.
// working copy - scaffold materialised by --work
// size      229 bytes
// prototype int (__thiscall ?get@NetFifo@@QAEHPAXPAIPAH1@Z)(NetFifo* this, void*, unsigned int*, int*, unsigned int*)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2 0x00645930
// indirect  0x00633D9A 0x00633DA7 0x00633DCF 0x00633DE3 0x00633E66

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get@NetFifo@@QAEHPAXPAIPAH1@Z  at 0x00633D90  (229 bytes)
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

class NetFifo;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void *memcpy(void *, const void *, unsigned int);
extern "C" void free(void *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669174 = (int *)0x00669174;
static int *const g_0066917c = (int *)0x0066917C;

class NetFifo { public:
    int get(void *, unsigned int *, int *, unsigned int *);
};
typedef void (__stdcall *CritSectFn)(void *);
#pragma function(memcpy)

int NetFifo::get(void * a1, unsigned int * a2, int * a3, unsigned int * a4) {
    char *self = reinterpret_cast<char *>(this);
    void *cs = self + 0xc;
    (*reinterpret_cast<CritSectFn *>(g_0066917c))(cs);

    char *cur = *reinterpret_cast<char **>(self);
    if (cur == 0) {
        (*reinterpret_cast<CritSectFn *>(g_00669174))(cs);
        return 0;
    }

    char *prev = 0;
    while (*reinterpret_cast<unsigned int *>(cur + 4) != reinterpret_cast<unsigned int>(a2)) {
        char *next = *reinterpret_cast<char **>(cur + 0x14);
        prev = cur;
        if (next == 0) {
            (*reinterpret_cast<CritSectFn *>(g_00669174))(cs);
            return 0;
        }
        cur = next;
    }

    if (a4 != 0) {
        *a4 = *reinterpret_cast<unsigned int *>(cur + 0x10);
    }
    char *buf = *reinterpret_cast<char **>(cur + 0xc);
    if (buf != 0) {
        if (a1 != 0) {
            memcpy(a1, buf, *reinterpret_cast<unsigned int *>(cur + 0x10));
        }
        buf = *reinterpret_cast<char **>(cur + 0xc);
        if (buf != 0) {
            free(buf);
        }
        *reinterpret_cast<char **>(cur + 0xc) = 0;
    }

    if (prev == 0) {
        *reinterpret_cast<int *>(self) = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self) + 0x14);
    } else {
        *reinterpret_cast<int *>(prev + 0x14) = *reinterpret_cast<int *>(cur + 0x14);
    }

    if (a3 != 0) {
        *a3 = *reinterpret_cast<int *>(cur);
    }
    int result = *reinterpret_cast<int *>(cur + 8);
    free(cur);
    *reinterpret_cast<int *>(self + 8) = *reinterpret_cast<int *>(self + 8) - 1;
    (*reinterpret_cast<CritSectFn *>(g_00669174))(cs);
    return result;
}
