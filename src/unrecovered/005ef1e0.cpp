// ORIGINAL: 0x005EF1E0 sub_5ef1e0 0x005EF1E0-0x005EF310 FILE
// TRIED: the scaffold's own Win/Palette classes cannot be reused (they are fully defined already, and body-mode's brace-counted extractor would stop at the first balanced `{}` - our own helper class - before reaching the function), so this stays FILE mode with its own minimal Win/Palette. A plain `*(FuncPtr*)addr` call for GetDC/SelectObject/etc. always loaded the pointer into a register first (mov+call reg); declaring them `__declspec(dllimport)` gets the single `call dword ptr [addr]` form the original uses. Landed at MISMATCH #29 (a `push edi` the original hoists earlier, before the palette-sync compare, that this body only emits once `oldPen` is first needed).
// size      304 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE4F0
// indirect  0x005EF20A 0x005EF21B 0x005EF280 0x005EF297 0x005EF2AE 0x005EF2BB 0x005EF2DE 0x005EF2FC

typedef void *HDC;

class Win;

class Palette {
public:
    void set_active_window(Win *);
};

// The receiver is `this` (ecx) with 5 real stack args plus one that every
// caller pushes but this body never reads (callee-pop is 0x18 = 6 dwords,
// one more than the 5 offsets touched here) - a __thiscall member, not the
// __stdcall(int,int,int,int,int,int) the scaffold guessed.
class Win {
public:
    char pad_0000_[0x184];
    unsigned int field_184_;

    void sub_5ef1e0(int x1, int y1, int x2, int y2, void *pen, int unused6);
};

typedef int (__stdcall *IfaceGetHdcProc)(void *, int *);
typedef int (__stdcall *IfaceReleaseHdcProc)(void *, int);

extern "C" __declspec(dllimport) HDC __stdcall GetDC(void *);
extern "C" __declspec(dllimport) int __stdcall ReleaseDC(void *, HDC);
extern "C" __declspec(dllimport) void *__stdcall SelectObject(HDC, void *);
extern "C" __declspec(dllimport) int __stdcall MoveToEx(HDC, int, int, void *);
extern "C" __declspec(dllimport) int __stdcall LineTo(HDC, int, int);

static int *const g_009b3ab0 = (int *)0x009B3AB0;
static int *const g_009b7b2c = (int *)0x009B7B2C;
static int *const g_009bc498 = (int *)0x009BC498;
static int *const g_009b7b28 = (int *)0x009B7B28;
static int *const g_009b8180 = (int *)0x009B8180;

void Win::sub_5ef1e0(int x1, int y1, int x2, int y2, void *pen, int unused6) {
    (void)unused6;
    int hdc;
    if (*g_009b3ab0 != 0) {
        *g_009b3ab0 = *g_009b3ab0 + 1;
        hdc = *g_009b7b2c;
    } else {
        void *iface = reinterpret_cast<void *>(*g_009bc498);
        if (iface != 0) {
            IfaceGetHdcProc fn = (*reinterpret_cast<IfaceGetHdcProc **>(iface))[17];
            fn(iface, g_009b7b2c);
            hdc = *g_009b7b2c;
        } else {
            *g_009b7b2c = reinterpret_cast<int>(GetDC(reinterpret_cast<void *>(*g_009b7b28)));
            hdc = *g_009b7b2c;
        }
        if (hdc == 0) {
            return;
        }
        *g_009b3ab0 = 1;
    }
    if (hdc != 0) {
        if ((int)field_184_ != *reinterpret_cast<int *>(*g_009b8180 + 0x400)) {
            (*reinterpret_cast<Palette **>(g_009b8180))->set_active_window(this);
            field_184_ = *reinterpret_cast<unsigned int *>(*g_009b8180 + 0x400);
        }

        void *oldPen = SelectObject(reinterpret_cast<HDC>(*g_009b7b2c), pen);

        MoveToEx(reinterpret_cast<HDC>(*g_009b7b2c), x1, y1, 0);

        LineTo(reinterpret_cast<HDC>(*g_009b7b2c), x2, y2);

        SelectObject(reinterpret_cast<HDC>(*g_009b7b2c), oldPen);

        *g_009b3ab0 = *g_009b3ab0 - 1;
        if (*g_009b3ab0 == 0) {
            void *iface2 = reinterpret_cast<void *>(*g_009bc498);
            if (iface2 != 0) {
                IfaceReleaseHdcProc fn2 = (*reinterpret_cast<IfaceReleaseHdcProc **>(iface2))[26];
                fn2(iface2, *g_009b7b2c);
                *g_009b7b2c = 0;
                return;
            }
            ReleaseDC(reinterpret_cast<void *>(*g_009b7b28), reinterpret_cast<HDC>(*g_009b7b2c));
            *g_009b7b2c = 0;
        }
    }
}
