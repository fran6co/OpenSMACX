// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005DA330
// name           ?map_colors@Buffer@@QAEHHHHHPAE@Z
// size           401 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005da330/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?map_colors@Buffer@@QAEHHHHHPAE@Z  at 0x005DA330  (401 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under BOTH cl 12.00.8168 and
// i686-w64-mingw32-g++ -std=c++11. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

struct int8;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669338 = (int *)0x00669338;

class Buffer { public:
    int map_colors(int, int, int, int, unsigned char*);
};

int Buffer::map_colors(int a1, int a2, int a3, int a4, unsigned char* a5) {
    char *self = reinterpret_cast<char *>(this);

    if (!a5) {
        return 0x10;
    }

    int rect[4];
    rect[0] = a1;
    rect[1] = a2;
    rect[2] = a3;
    rect[3] = a4;

    typedef int (__stdcall *FnIntersectRect)(int *, int *, int *);
    if (!(*reinterpret_cast<FnIntersectRect *>(g_00669338))(
            rect, rect, reinterpret_cast<int *>(self + 0x20))) {
        return 0;
    }

    int height = rect[3] - rect[1];
    int width = rect[2] - rect[0];
    a2 = height;
    a1 = width;
    a3 = *reinterpret_cast<int *>(self + 0x4a8) - width;

    if (rect[0] >= *reinterpret_cast<int *>(self + 0x80)) {
        goto fail;
    }
    if (rect[1] >= -*reinterpret_cast<int *>(self + 0x84)) {
        goto fail;
    }

    int data;
    if (*reinterpret_cast<int *>(self + 0x58) == 0) {
        data = *reinterpret_cast<int *>(self + 0x54);
        *reinterpret_cast<int *>(self + 0x50) = data;
        if (data == 0) {
            goto fail;
        }
        ++*reinterpret_cast<int *>(self + 0x6c);
    } else {
        data = *reinterpret_cast<int *>(self + 0x50);
        if (data != 0) {
            ++*reinterpret_cast<int *>(self + 0x6c);
        } else {
            unsigned char descriptor[0x6c];
            *reinterpret_cast<int *>(descriptor) = 0x6c;
            int surface = *reinterpret_cast<int *>(self + 0x58);
            typedef int (__stdcall *FnLock)(void *, void *, void *, unsigned int, void *);
            FnLock lock = *reinterpret_cast<FnLock *>(*reinterpret_cast<int *>(surface) + 0x64);
            if (lock(reinterpret_cast<void *>(surface), 0, descriptor, 1, 0) != 0) {
                goto fail;
            }
            ++*reinterpret_cast<int *>(self + 0x6c);
            *reinterpret_cast<int *>(self + 0x4a8) = *reinterpret_cast<int *>(descriptor + 0x10);
            data = *reinterpret_cast<int *>(descriptor + 0x24);
            *reinterpret_cast<int *>(self + 0x50) = data;
        }
    }

    if (data == 0) {
        goto fail;
    }

    a4 = *reinterpret_cast<int *>(self + 0x4a8) * rect[1]
        + *reinterpret_cast<int *>(self + 0x50) + rect[0];
    if (a4 == 0) {
        goto fail;
    }

    {
        unsigned char *table = a5;
        int i;
        do {
            i = a1;
            do {
                *reinterpret_cast<unsigned char *>(a4) =
                    table[*reinterpret_cast<unsigned char *>(a4)];
                ++a4;
            } while (--i);
            a4 += a3;
        } while (--a2);
    }

    if (*reinterpret_cast<int *>(self + 0x58) == 0) {
        int remaining = --*reinterpret_cast<int *>(self + 0x6c);
        if (remaining > 0) {
            return 0;
        }
        *reinterpret_cast<int *>(self + 0x50) = 0;
        *reinterpret_cast<int *>(self + 0x6c) = 0;
        return 0;
    }

    {
        int remaining = --*reinterpret_cast<int *>(self + 0x6c);
        int data2 = *reinterpret_cast<int *>(self + 0x50);
        if (data2 != 0 && remaining <= 0) {
            int surface = *reinterpret_cast<int *>(self + 0x58);
            typedef int (__stdcall *FnUnlock)(void *, void *);
            FnUnlock unlock = *reinterpret_cast<FnUnlock *>(*reinterpret_cast<int *>(surface) + 0x80);
            unlock(reinterpret_cast<void *>(surface), reinterpret_cast<void *>(data2));
            *reinterpret_cast<int *>(self + 0x50) = 0;
            *reinterpret_cast<int *>(self + 0x6c) = 0;
        }
    }
    return 0;

fail:
    return 7;
}
