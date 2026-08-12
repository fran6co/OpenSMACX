// ORIGINAL: 0x005DFF00 FILE
// name      ?copy@Buffer@@QAEHPAVBuffer@@HHHHHH@Z
// size      2203 bytes
// spans     0x005DFF00-0x005E079B
// prototype int (__thiscall ?copy@Buffer@@QAEHPAVBuffer@@HHHHHH@Z)(Buffer* this, Buffer* buffer, int xCoord, int yCoord, int, int, int width, int height)
// callers   51   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3
// indirect  0x005DFFDD 0x005E0705
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005DFF00
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005dff00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?copy@Buffer@@QAEHPAVBuffer@@HHHHHH@Z  at 0x005DFF00  (2203 bytes)
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

#include <string.h>

struct buffer;
struct height;
struct width;
struct xCoord;
struct yCoord;

// ---- callees, declared and never defined (a definition would be inlined) ----

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669338 = (int *)0x00669338;
static int *const g_009b39e4 = (int *)0x009B39E4;

class Buffer { public:
    int get_data();
    void free_data(int);
    int copy(Buffer* buffer, int xCoord, int yCoord, int, int, int width, int height);
};

typedef int (__stdcall *FnIntersectRect)(int *, int *, int *);
typedef int (__stdcall *FnUnlock)(void *, void *);

// Shared by every "this-side clip/get_data/pointer failed" site: releases the
// reference already taken on `buffer` (the write side) via its own inlined
// free_data(1) body, then returns 3. The no-surface case is small enough that
// the original duplicates it at every call site instead of sharing it; the
// has-surface case is bigger and every site instead jumps into one shared
// copy (label LRelSurfR3 below), matching 0x005E06ED being a fan-in target.
#define REL_BUF_R3 \
    surface = *reinterpret_cast<int *>(bb + 0x58); \
    if (surface == 0) { \
        rc = *reinterpret_cast<int *>(bb + 0x6c) - 1; \
        *reinterpret_cast<int *>(bb + 0x6c) = rc; \
        if (rc <= 0) { \
            *reinterpret_cast<int *>(bb + 0x50) = 0; \
            *reinterpret_cast<int *>(bb + 0x6c) = 0; \
        } \
        return 3; \
    } \
    goto LRelSurfR3;

// The same-object branch mirrors this shape four times in the original (once
// per combination of "write-before-read in X" and "write-before-read in Y"),
// because with std/cld each combination needs a different byte-copy
// direction. This translation unit cannot reproduce std's reversed intra-row
// copy through a plain memcpy, so all four still copy each row low-to-high;
// only the row (Y) traversal actually varies, via ROW_ANCHOR/ROW_STEP. The
// four call sites are kept distinct anyway (rather than factored down to two)
// so the clip/get_data()/pointer-compute code is repeated with the same
// multiplicity as the original.
#define SAME_BUF_COPY(ROW_ANCHOR, ROW_STEP) \
    if (rectbuf[0] >= *reinterpret_cast<int *>(bb + 0x80)) goto LR3; \
    if (rectbuf[1] >= -*reinterpret_cast<int *>(bb + 0x84)) goto LR3; \
    if (buffer->get_data() == 0) goto LR3; \
    writePtr = *reinterpret_cast<int *>(bb + 0x50) + \
               *reinterpret_cast<int *>(bb + 0x4a8) * rectbuf[1] + \
               rectbuf[0]; \
    if (writePtr == 0) goto LR3; \
    if (xCoord >= *reinterpret_cast<int *>(self + 0x80)) { REL_BUF_R3 } \
    if (yCoord >= -*reinterpret_cast<int *>(self + 0x84)) { REL_BUF_R3 } \
    if (this->get_data() == 0) { REL_BUF_R3 } \
    readPtr = *reinterpret_cast<int *>(self + 0x50) + \
              *reinterpret_cast<int *>(self + 0x4a8) * yCoord + xCoord; \
    if (readPtr == 0) { REL_BUF_R3 } \
    dstPitch = *reinterpret_cast<int *>(bb + 0x4a8); \
    srcPitch = *reinterpret_cast<int *>(self + 0x4a8); \
    dstRow = reinterpret_cast<char *>(writePtr) + (ROW_ANCHOR) * dstPitch; \
    srcRow = reinterpret_cast<char *>(readPtr) + (ROW_ANCHOR) * srcPitch; \
    rows = height; \
    do { \
        memcpy(dstRow, srcRow, static_cast<unsigned int>(width)); \
        dstRow += (ROW_STEP) * dstPitch; \
        srcRow += (ROW_STEP) * srcPitch; \
    } while (--rows != 0);

int Buffer::copy(Buffer *buffer, int xCoord, int yCoord, int wx, int wy,
                 int width, int height) {
    char *bb;
    char *self;
    int rectbuf[4];
    int self80, negSelf84;
    int writePtr, readPtr;
    int dstPitch, srcPitch, rows;
    char *dstRow, *srcRow;
    int rc, surface, rc2, data, vtbl;

    self = reinterpret_cast<char *>(this);
    bb = reinterpret_cast<char *>(buffer);

    // Reads self+0x54 (ppv_bits_) and self+0x58 (surface): no local storage
    // and no DirectDraw surface means there is nothing to copy into/out of.
    if (*reinterpret_cast<int *>(self + 0x54) == 0 &&
        *reinterpret_cast<int *>(self + 0x58) == 0) {
        return 7;
    }
    if (!buffer) {
        goto LR3;
    }

    // xCoord/yCoord are the READ position inside `this`. Going negative
    // shrinks the extent and shifts the WRITE position (wx/wy) by the
    // overhang, then clamps to 0 - standard blit clip.
    if (xCoord < 0) {
        width += xCoord;
        wx -= xCoord;
        xCoord = 0;
    }
    if (yCoord < 0) {
        height += yCoord;
        wy -= yCoord;
        yCoord = 0;
    }

    // Clip the READ rect (in `this`) against this's own raw bounds
    // (self+0x80 / self+0x84, the latter stored/used negated).
    self80 = *reinterpret_cast<int *>(self + 0x80);
    if (xCoord > self80) {
        goto LR0;
    }
    negSelf84 = -*reinterpret_cast<int *>(self + 0x84);
    if (yCoord > negSelf84) {
        goto LR0;
    }
    if (xCoord + width > self80) {
        width = self80 - xCoord;
    }
    if (yCoord + height > negSelf84) {
        height = -(yCoord + *reinterpret_cast<int *>(self + 0x84));
    }

    // Clip the WRITE rect (wx,wy,wx+width,wy+height) against buffer's
    // logical clip rect (buffer+0x20, i.e. buffer->rect1_) via the real
    // IntersectRect, reached through its IAT slot at g_00669338.
    rectbuf[0] = wx;
    rectbuf[1] = wy;
    rectbuf[2] = wx + width;
    rectbuf[3] = wy + height;
    if (!(*reinterpret_cast<FnIntersectRect *>(g_00669338))(
            rectbuf, rectbuf, reinterpret_cast<int *>(bb + 0x20))) {
        goto LR0;
    }

    // Shift the READ position by however much intersection moved the WRITE
    // rect's left/top, and take the final extent from the intersected rect.
    xCoord += rectbuf[0] - wx;
    yCoord += rectbuf[1] - wy;
    width = rectbuf[2] - rectbuf[0];
    height = rectbuf[3] - rectbuf[1];

    if (buffer != this) {
        // Different objects: read from `this`, write into `buffer`, no
        // overlap is possible, so a straight per-row memcpy suffices.
        if (rectbuf[0] >= *reinterpret_cast<int *>(bb + 0x80)) goto LR3;
        if (rectbuf[1] >= -*reinterpret_cast<int *>(bb + 0x84)) goto LR3;
        if (buffer->get_data() == 0) goto LR3;
        writePtr = *reinterpret_cast<int *>(bb + 0x50) +
                   *reinterpret_cast<int *>(bb + 0x4a8) * rectbuf[1] +
                   rectbuf[0];
        if (writePtr == 0) goto LR3;

        if (xCoord >= *reinterpret_cast<int *>(self + 0x80)) { REL_BUF_R3 }
        if (yCoord >= -*reinterpret_cast<int *>(self + 0x84)) { REL_BUF_R3 }
        if (this->get_data() == 0) { REL_BUF_R3 }
        readPtr = *reinterpret_cast<int *>(self + 0x50) +
                  *reinterpret_cast<int *>(self + 0x4a8) * yCoord + xCoord;
        if (readPtr == 0) { REL_BUF_R3 }

        dstPitch = *reinterpret_cast<int *>(bb + 0x4a8);
        srcPitch = *reinterpret_cast<int *>(self + 0x4a8);
        dstRow = reinterpret_cast<char *>(writePtr);
        srcRow = reinterpret_cast<char *>(readPtr);
        rows = height;
        do {
            memcpy(dstRow, srcRow, static_cast<unsigned int>(width));
            srcRow += srcPitch;
            dstRow += dstPitch;
        } while (--rows != 0);
    } else if (wx < xCoord) {
        if (wy < yCoord) {
            SAME_BUF_COPY(0, 1)
        } else {
            SAME_BUF_COPY(height - 1, -1)
        }
    } else {
        if (wy < yCoord) {
            SAME_BUF_COPY(0, 1)
        } else {
            SAME_BUF_COPY(height - 1, -1)
        }
    }

    this->free_data(1);
    buffer->free_data(1);
LR0:
    return 0;

LRelSurfR3:
    rc2 = *reinterpret_cast<int *>(bb + 0x6c) - 1;
    data = *reinterpret_cast<int *>(bb + 0x50);
    *reinterpret_cast<int *>(bb + 0x6c) = rc2;
    if (data != 0 && rc2 <= 0) {
        vtbl = *reinterpret_cast<int *>(surface);
        (*reinterpret_cast<FnUnlock *>(vtbl + 0x80))(
            reinterpret_cast<void *>(surface), reinterpret_cast<void *>(data));
        *reinterpret_cast<int *>(bb + 0x50) = 0;
        *reinterpret_cast<int *>(bb + 0x6c) = 0;
    }
LR3:
    return 3;
}
