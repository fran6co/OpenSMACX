// ORIGINAL: 0x005F1480 ?OnActivate@Win@@QAAJPAXIPAXJ@Z 0x005F1480-0x005F15B6
// RULED-OUT: testing `a2 == 0` first (zero-set branch as the `if`) compiled to `test;jne`; testing `a2 != 0` first (zero-set branch as the `else`) matches the original's `test;je` polarity. Landed at MISMATCH #42 (up from #10) - same HDC acquire/release idiom as 0x5EF1E0's sub_5ef1e0.
// size      310 bytes
// prototype LRESULT (__cdecl ?OnActivate@Win@@QAAJPAXIPAXJ@Z)(HWND hWnd, LPARAM lParam, void*, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F148C 0x005F14DE 0x005F14EF 0x005F151A 0x005F1527 0x005F154D 0x005F1569 0x005F15AD

typedef int (__stdcall *IfaceGetHdcProc)(void *, int *);
typedef int (__stdcall *IfaceReleaseHdcProc)(void *, int);

// InvalidateRect, GetDC, SelectPalette, RealizePalette and DefWindowProcA
// come from the scaffold (emit_translation_unit.WIN32_IMPORTS),
// `__declspec(dllimport)` included so the calls keep the original's
// IAT-indirect shape.

long __cdecl Win::OnActivate(void *a1, unsigned int a2, void *a3, long a4) {
    InvalidateRect(reinterpret_cast<void *>(*g_009b7b28), 0, 0);
    if (a2 != 0) {
      if (a4 == 0 && *g_009bc494 == 0) {
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
            if (hdc != 0) {
                *g_009b3ab0 = 1;
            }
        }
        if (hdc != 0) {
            SelectPalette(reinterpret_cast<HDC>(*g_009b7b2c),
                          reinterpret_cast<void *>(*g_009b8178), 0);
            RealizePalette(reinterpret_cast<HDC>(*g_009b7b2c));

            *g_009b3ab0 = *g_009b3ab0 - 1;
            if (*g_009b3ab0 == 0) {
                void *iface2 = reinterpret_cast<void *>(*g_009bc498);
                if (iface2 != 0) {
                    IfaceReleaseHdcProc fn2 = (*reinterpret_cast<IfaceReleaseHdcProc **>(iface2))[26];
                    fn2(iface2, *g_009b7b2c);
                } else {
                    typedef int (__stdcall *ReleaseDCProc)(void *, HDC);
                    static int *const g_00669280 = (int *)0x00669280;
                    (*reinterpret_cast<ReleaseDCProc *>(g_00669280))(
                        reinterpret_cast<void *>(*g_009b7b28), reinterpret_cast<HDC>(*g_009b7b2c));
                }
                *g_009b7b2c = 0;
            }
        }
      }
    } else {
        *g_009b7acc = 0;
        *g_009b7ad0 = 0;
    }
    return DefWindowProcA(a1, 6, (static_cast<unsigned int>(a4) << 16) | (a2 & 0xffff), a3);
}
