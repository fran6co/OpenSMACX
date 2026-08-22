// ORIGINAL: 0x0045F200 ??0Interlude@@QAE@XZ 0x0045F200-0x0045F2DF;0x00655800-0x00655820 FILE
// TRIED: reproducing the `push -1/push handler/mov fs:[0]` SEH frame and the two unwind funclets (~GraphicWin, and a tail call into sub_406820 on `this+0xa24`) - explicit placement-new / raw pointer-cast statements in an ordinary function body do not make MSVC emit member-initializer-list unwind protection, so this is landed without it (same call as 0x004849D0).
// size      255 bytes
// prototype void (__thiscall ??0Interlude@@QAE@XZ)(Interlude* this)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x005D7210

inline void *__cdecl operator new(unsigned int, void *p) { return p; }

class GraphicWin { public:
    GraphicWin();
};
class Buffer { public:
    Buffer();
};

// The const-pointer spelling reproduces the original's encoding including
// the address; the numeric value is masked by the comparison.
static unsigned int *const g_009b3374 = (unsigned int *)0x009B3374;
static void *const g_0066b0ec = (void *)0x0066B0EC;
static void *const g_006693ac = (void *)0x006693AC;
static void *const g_006693a4 = (void *)0x006693A4;
static void *const g_006693a0 = (void *)0x006693A0;
static void *const g_006698c4 = (void *)0x006698C4;
static void *const g_006698c0 = (void *)0x006698C0;
static void *const g_0066c708 = (void *)0x0066C708;
static void *const g_0066c700 = (void *)0x0066C700;

class Interlude { public:
    char pad_00[0xB00];

    Interlude();
};

Interlude::Interlude() {
    char *base = reinterpret_cast<char *>(this);

    new (base) GraphicWin();

    *reinterpret_cast<void **>(base + 0xa28) = g_0066b0ec;
    *reinterpret_cast<void **>(base + 0xa4c) = g_006693ac;

    unsigned int saved = *g_009b3374;
    *reinterpret_cast<unsigned int *>(base + 0xa50) = saved;
    *g_009b3374 = 0;

    unsigned int ptr1 = *reinterpret_cast<unsigned int *>(base + 0xa28);
    *reinterpret_cast<void **>(base + 0xa24) = g_006693a4;
    unsigned int off1 = *reinterpret_cast<unsigned int *>(ptr1 + 4);
    *reinterpret_cast<void **>(base + 0xa28 + off1) = g_006693a0;

    unsigned int ptr2 = *reinterpret_cast<unsigned int *>(base + 0xa28);
    *reinterpret_cast<unsigned int *>(base + 0xa2c) = 0;
    *reinterpret_cast<unsigned int *>(base + 0xa30) = 0;
    *reinterpret_cast<unsigned int *>(base + 0xa34) = 0;
    *reinterpret_cast<unsigned int *>(base + 0xa38) = 0;
    *reinterpret_cast<unsigned int *>(base + 0xa3c) = 0;
    *reinterpret_cast<void **>(base + 0xa24) = g_006698c4;
    unsigned int off2 = *reinterpret_cast<unsigned int *>(ptr2 + 4);
    *reinterpret_cast<void **>(base + 0xa28 + off2) = g_006698c0;

    new (base + 0xa54) Buffer();

    *reinterpret_cast<void **>(base) = g_0066c708;
    *reinterpret_cast<void **>(base + 0x444) = g_0066c700;
}
