// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005FE330
// name           ?init@Palette@@QAEXXZ
// size           303 bytes
// measured tier  MISMATCH
// divergence     22
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005fe330/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@Palette@@QAEXXZ  at 0x005FE330  (303 bytes)
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

class Palette;

// ---- callees, declared and never defined (a definition would be inlined) ----
unsigned int __cdecl random(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006690ac = (int *)0x006690AC;
static int *const g_0066927c = (int *)0x0066927C;
static int *const g_00669280 = (int *)0x00669280;
static int *const g_0067022c = (int *)0x0067022C;
static int *const g_00670252 = (int *)0x00670252;
static int *const g_00670253 = (int *)0x00670253;
static int *const g_00670254 = (int *)0x00670254;
static int *const g_009b8188 = (int *)0x009B8188;

class Palette { public:
    void close();
    void init();
};

typedef void *(__stdcall *GetDcFn)(void *);
typedef int (__stdcall *GetPalEntriesFn)(void *, int, int, void *);
typedef int (__stdcall *ReleaseDcFn)(void *, void *);

void Palette::init() {
    close();
    uint32_t self_addr = reinterpret_cast<uint32_t>(this);

    if (*g_009b8188 != 0) {
        void *hdc = (*reinterpret_cast<GetDcFn *>(g_0066927c))(0);
        (*reinterpret_cast<GetPalEntriesFn *>(g_006690ac))(hdc, 0, 0x100, this);
        (*reinterpret_cast<ReleaseDcFn *>(g_00669280))(0, hdc);
    } else {
        uint32_t edi = reinterpret_cast<uint32_t>(g_0067022c) - self_addr;
        uint32_t ebp = reinterpret_cast<uint32_t>(g_00670254) - self_addr;
        uint32_t edx = reinterpret_cast<uint32_t>(g_00670253) - self_addr;
        uint32_t ebx = reinterpret_cast<uint32_t>(g_00670252) - self_addr;
        uint32_t ecx = reinterpret_cast<uint32_t>(g_0067022c);
        uint32_t eax = self_addr + 2;
        do {
            uint8_t bl = *reinterpret_cast<uint8_t *>(edi + eax);
            ecx += 4;
            *reinterpret_cast<uint8_t *>(eax - 2) = bl;
            bl = *reinterpret_cast<uint8_t *>(ecx - 3);
            *reinterpret_cast<uint8_t *>(eax - 1) = bl;
            bl = *reinterpret_cast<uint8_t *>(ecx - 4);
            *reinterpret_cast<uint8_t *>(eax) = bl;
            *reinterpret_cast<uint8_t *>(eax + 1) = 4;
            bl = *reinterpret_cast<uint8_t *>(eax + ebp);
            *reinterpret_cast<uint8_t *>(eax + 0x3d6) = bl;
            bl = *reinterpret_cast<uint8_t *>(eax + edx);
            *reinterpret_cast<uint8_t *>(eax + 0x3d7) = bl;
            bl = *reinterpret_cast<uint8_t *>(eax + ebx);
            *reinterpret_cast<uint8_t *>(eax + 0x3d8) = bl;
            *reinterpret_cast<uint8_t *>(eax + 0x3d9) = 4;
            eax += 4;
        } while (ecx < reinterpret_cast<uint32_t>(g_00670254));

        uint8_t counter = 0xa;
        uint32_t ramp = self_addr + 0x2a;
        do {
            *reinterpret_cast<uint8_t *>(ramp - 2) = counter;
            *reinterpret_cast<uint8_t *>(ramp - 1) = counter;
            *reinterpret_cast<uint8_t *>(ramp) = counter;
            *reinterpret_cast<uint8_t *>(ramp + 1) = 5;
            counter++;
            ramp += 4;
        } while (counter < 0xf6);

        uint32_t index = 0;
        uint32_t last = self_addr + 0x21;
        do {
            uint8_t hi = static_cast<uint8_t>(index);
            last += 4;
            hi += 8;
            *reinterpret_cast<uint8_t *>(last - 5) = hi;
            uint8_t lo = static_cast<uint8_t>(index);
            *reinterpret_cast<uint8_t *>(last - 4) = 0;
            *reinterpret_cast<uint8_t *>(last - 3) = 0;
            lo -= 0xb;
            *reinterpret_cast<uint8_t *>(last - 2) = 2;
            *reinterpret_cast<uint8_t *>(last + 0x3b3) = lo;
            *reinterpret_cast<uint8_t *>(last + 0x3b4) = 0;
            *reinterpret_cast<uint8_t *>(last + 0x3b5) = 0;
            *reinterpret_cast<uint8_t *>(last + 0x3b6) = 2;
            index++;
        } while (index < 2);
    }

    *reinterpret_cast<int32_t *>(self_addr + 0x400) = 0;
    int32_t value;
    do {
        value = random(0, 0xffff);
        *reinterpret_cast<int32_t *>(self_addr + 0x400) = value;
    } while (value == 0);
}
