// ORIGINAL: 0x005CE4B0
// name      sub_5ce4b0
// size      362 bytes
// spans     0x005CE4B0-0x005CE61A
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005CE551 0x005CE568 0x005CE5A0 0x005CE5B0 0x005CE5D8 0x005CE5E7
// RULED-OUT: `ecx` does real work with no stack access at entry (reads
//            `[ecx]` before any `[ebp+..]`), so this is a __thiscall
//            member - written as `ClassX::sub_5ce4b0()`, same precedent as
//            0x004E3300's ClassX::sub_4e3300. The six indirect calls are
//            DirectDraw-COM-shaped (self pushed explicitly alongside the
//            args, not via ECX), spelled with `__stdcall` function-pointer
//            typedefs per call site rather than a shared VCall shim, since
//            two calls land on offset 0xc through DIFFERENT vtables (one
//            via `[this+0x54]`, two via `this->[0]->0x5c`) with different
//            arities (4 args vs 2). Local DDSURFACEDESC/DDPIXELFORMAT-like
//            scratch structs are modelled as plain structs, not the
//            overlapping/self-referential stack layout the original build
//            (a nested pointer field pointing back into the same frame) -
//            not chased further. 0.83 mnemonic similarity; first
//            divergence at #3 is the prologue register-save order, which
//            follows from local-variable layout this body does not
//            reproduce.

typedef unsigned short uint16_t;

struct PixelFormatDesc {
    uint16_t w1;
    uint16_t w2;
    int flags;
    int fourcc;
    int rgbBits;
};

struct SurfaceDesc {
    int dwSize;
    int dwFlags;
    int field3;
    int field4;
    PixelFormatDesc *pFormat;
};

struct SmallDesc {
    int dwSize;
    int a, b, c, d;
};

typedef int (__stdcall *LockFn2)(void *, void *);
typedef int (__stdcall *LockFn4)(void *, void *, void *, int);
typedef void (__stdcall *GetDcFn)(void *, int);
typedef void (__stdcall *ReleaseFn)(void *);
typedef void (__stdcall *Slot15Fn)(void *, int);
typedef int (__stdcall *Slot12Fn)(void *, int, int, int);

class ClassX {
public:
    int sub_5ce4b0();
};

int ClassX::sub_5ce4b0() {
    char *self = reinterpret_cast<char *>(this);
    int obj1 = *reinterpret_cast<int *>(self);

    if (*reinterpret_cast<int *>(obj1 + 0x54) == 0) {
        goto tail_reset;
    }

    {
        int field58 = *reinterpret_cast<int *>(self + 0x58);

        PixelFormatDesc fmt;
        fmt.w1 = 1;
        fmt.w2 = 2;
        fmt.flags = field58;
        fmt.fourcc = field58 << 2;
        fmt.rgbBits = 0x00100004;

        SurfaceDesc desc;
        desc.dwSize = 0x14;
        desc.dwFlags = 0x100e0;
        desc.field3 = *reinterpret_cast<int *>(self + 0x3c4);
        desc.field4 = 0;
        desc.pFormat = &fmt;

        int iface1 = *reinterpret_cast<int *>(obj1 + 0x54);
        void *arg2 = reinterpret_cast<void *>(obj1 + 0x5c);
        LockFn4 fn1 = *reinterpret_cast<LockFn4 *>(*reinterpret_cast<int *>(iface1) + 0xc);
        int rc = fn1(reinterpret_cast<void *>(iface1), &desc, arg2, 0);
        if (rc != 0) {
            goto tail_reset;
        }
    }

    {
        int surf = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self) + 0x5c);
        GetDcFn fnDc = *reinterpret_cast<GetDcFn *>(*reinterpret_cast<int *>(surf) + 0x44);
        fnDc(reinterpret_cast<void *>(surf), *reinterpret_cast<int *>(self + 0x58));

        SmallDesc small;
        small.dwSize = 0x14;
        small.a = 0;
        small.b = 0;
        small.c = 0;
        small.d = 0;

        obj1 = *reinterpret_cast<int *>(self);
        if (*reinterpret_cast<unsigned int *>(obj1 + 4) & 0x2000) {
            int iface2 = *reinterpret_cast<int *>(obj1 + 0x5c);
            int arg = *reinterpret_cast<int *>(obj1 + 0x84);
            Slot15Fn fn15 = *reinterpret_cast<Slot15Fn *>(*reinterpret_cast<int *>(iface2) + 0x3c);
            fn15(reinterpret_cast<void *>(iface2), arg);
        }

        int iface3 = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self) + 0x5c);
        LockFn2 fn2 = *reinterpret_cast<LockFn2 *>(*reinterpret_cast<int *>(iface3) + 0xc);
        int rc2 = fn2(reinterpret_cast<void *>(iface3), &small);
        if (rc2 != 0) {
            goto release;
        }

        *reinterpret_cast<int *>(self + 0x3bc) = 0;
        *reinterpret_cast<int *>(self + 0x368) = small.b;
        *reinterpret_cast<int *>(self + 0x36c) = 0;

        int iface4 = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self) + 0x5c);
        Slot12Fn fn12 = *reinterpret_cast<Slot12Fn *>(*reinterpret_cast<int *>(iface4) + 0x30);
        int rc3 = fn12(reinterpret_cast<void *>(iface4), 0, 0, 1);
        if (rc3 == 0) {
            goto done;
        }
    }

release:
    {
        int iface5 = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self) + 0x5c);
        ReleaseFn fn8 = *reinterpret_cast<ReleaseFn *>(*reinterpret_cast<int *>(iface5) + 8);
        fn8(reinterpret_cast<void *>(iface5));
    }

tail_reset:
    *reinterpret_cast<int *>(self + 0x5c) = 0;
    *reinterpret_cast<int *>(self + 0x3bc) = 1;
    *reinterpret_cast<int *>(self + 0x3b8) = 0;
    *reinterpret_cast<int *>(self + 0x368) = 0x8000;
    *reinterpret_cast<int *>(self + 0x36c) = 0;

done:
    *reinterpret_cast<int *>(self + 0x364) = 1;
    return 0;
}
