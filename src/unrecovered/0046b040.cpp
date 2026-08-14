// ORIGINAL: 0x0046B040 FILE
// RULED-OUT: nothing yet - MISMATCH #0 push/mov (60% mnemonic sim). The
//            original keeps an EBP frame (`push ebp; mov ebp,esp`, flags
//            say `frame`); this loop shape gives /O2 too few live locals to
//            need one, so it omits it (FPO). Not chased further. Same
//            MapWinTable/MapWinActiveOffset/draw_radius idiom already
//            proven byte-exact in src/mapwin.cpp's draw_tile/draw_tiles at
//            the neighbouring addresses.
// name      ?draw_tile_fixup@@YAXHHHHHH@Z
// size      242 bytes
// spans     0x0046B040-0x0046B132
// prototype
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A2A0
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x0046B040

class __single_inheritance OriginalObject;

template <class Method>
Method original_method(unsigned long address) {
    union {
        unsigned long address;
        Method method;
    } cast;
    cast.address = address;
    return cast.method;
}

#define ORIGINAL(pointer) (reinterpret_cast<OriginalObject *>(pointer))

typedef void (OriginalObject::*DrawRadiusFn)(int, int, int, int);

extern void *MapWinTable[];   // at 0x007D3C3C, 8 slots ending 0x007D3C5C
const unsigned long MapWinActiveOffset = 0x1DD74;
const unsigned int MapWinTableSlots = 8;

void __cdecl draw_tile_fixup(int a1, int a2, int a3, int a4, int a5, int a6) {
    DrawRadiusFn const drawRadius = original_method<DrawRadiusFn>(0x0046A2A0);

    if ((*(unsigned char *)0x009A6494 & 1) == 0) {
        for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
            void *const window = MapWinTable[slot];
            if (window == 0) {
                continue;
            }
            if (slot != 0) {
                const unsigned int active = *(const volatile unsigned int *)(
                    (const char *)window + MapWinActiveOffset);
                if (active == 0) {
                    continue;
                }
            }
            (ORIGINAL(window)->*drawRadius)(a1, a2, 0, a6);
        }
    } else {
        for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
            void *const window = MapWinTable[slot];
            if (window == 0) {
                continue;
            }
            if (slot != 0) {
                const unsigned int active = *(const volatile unsigned int *)(
                    (const char *)window + MapWinActiveOffset);
                if (active == 0) {
                    continue;
                }
            }
            (ORIGINAL(window)->*drawRadius)(a1, a2, (a5 != 0) + 1, *(int *)0x00939284);
        }
        if (a3 != -1) {
            for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
                void *const window = MapWinTable[slot];
                if (window == 0) {
                    continue;
                }
                if (slot != 0) {
                    const unsigned int active = *(const volatile unsigned int *)(
                        (const char *)window + MapWinActiveOffset);
                    if (active == 0) {
                        continue;
                    }
                }
                (ORIGINAL(window)->*drawRadius)(a3, a4, (a5 != 0) + 1, *(int *)0x00939284);
            }
        }
    }
}
