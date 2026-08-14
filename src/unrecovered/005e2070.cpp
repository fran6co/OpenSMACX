// ORIGINAL: 0x005E2070
// name      ?change_color@Buffer@@QAEHHHHHHH@Z
// size      413 bytes
// spans     0x005E2070-0x005E220D
// prototype int (__thiscall ?change_color@Buffer@@QAEHHHHHHH@Z)(Buffer* this, int, int, int, int, int, int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E20C5 0x005E212E 0x005E21F6
// RULED-OUT: clip against rect1_, then a DirectDraw Lock/search-replace/
//            Unlock byte fill on the locked surface (Lock/Unlock are
//            genuinely __stdcall COM methods - `this` is pushed as an
//            explicit arg, not passed via ecx, confirmed by the
//            disassembly never setting ecx before either indirect call).
//            Landed at 0.67 similarity; diverges at #8 on the width_/
//            height_ range-check scheduling.

typedef int (__stdcall *IntersectRectFn)(RECT *, const RECT *, const RECT *);
typedef long (__stdcall *DDLockFn)(void *, void *, void *, unsigned long, void *);
typedef long (__stdcall *DDUnlockFn)(void *, void *);

int Buffer::change_color(int a1, int a2, int a3, int a4, int a5, int a6) {
    if (ppv_bits_ == 0 && field_58_ == 0) {
        return 7;
    }

    RECT rect;
    rect.left = a1;
    rect.top = a2;
    rect.right = a1 + a3;
    rect.bottom = a2 + a4;

    RECT clip = rect1_;
    if (!(reinterpret_cast<IntersectRectFn>(*g_00669338))(&rect, &clip, &rect)) {
        return 0;
    }

    if (rect.left >= static_cast<int>(width_) || rect.top >= -static_cast<int>(height_)) {
        return 7;
    }

    unsigned char descBuf[0x6c];

    if (field_58_ != 0) {
        if (field_50_ != 0) {
            field_6C_++;
        } else {
            *reinterpret_cast<int *>(descBuf) = 0x6c;

            void *vtbl = *reinterpret_cast<void **>(field_58_);
            DDLockFn lockFn = *reinterpret_cast<DDLockFn *>(reinterpret_cast<char *>(vtbl) + 0x64);
            long hr = lockFn(reinterpret_cast<void *>(field_58_), 0, descBuf, 1, 0);
            if (hr != 0) {
                return 7;
            }

            field_4A8_ = *reinterpret_cast<uint32_t *>(descBuf + 0x10);
            field_6C_++;
            field_50_ = *reinterpret_cast<uint32_t *>(descBuf + 0x24);
        }
    } else {
        uint32_t bits = reinterpret_cast<uint32_t>(ppv_bits_);
        field_50_ = bits;
        if (bits == 0) {
            return 7;
        }
        field_6C_++;
    }

    if (field_50_ == 0) {
        return 7;
    }

    int clippedTop = rect.top;
    char *pixelPtr = reinterpret_cast<char *>(field_4A8_ * clippedTop + field_50_ + rect.left);

    if (pixelPtr != 0) {
        int width = rect.right - rect.left;
        int height = rect.bottom - clippedTop;
        int rowSkip = static_cast<int>(field_4A8_) - width;

        char searchColor = static_cast<char>(a5);
        char replaceColor = static_cast<char>(a6);

        char *p = pixelPtr;
        int rowsLeft = height;
        do {
            int colsLeft = width;
            do {
                if (*p == searchColor) {
                    *p = replaceColor;
                }
                p++;
                colsLeft--;
            } while (colsLeft != 0);
            p += rowSkip;
            rowsLeft--;
        } while (rowsLeft != 0);

        if (field_58_ == 0) {
            int count = field_6C_ - 1;
            field_6C_ = count;
            if (count > 0) {
                return 0;
            }
            field_50_ = 0;
            field_6C_ = 0;
            return 0;
        }

        int count = field_6C_ - 1;
        field_6C_ = count;
        if (field_50_ == 0) {
            return 0;
        }
        if (count > 0) {
            return 0;
        }

        void *vtbl = *reinterpret_cast<void **>(field_58_);
        DDUnlockFn unlockFn = *reinterpret_cast<DDUnlockFn *>(reinterpret_cast<char *>(vtbl) + 0x80);
        unlockFn(reinterpret_cast<void *>(field_58_), reinterpret_cast<void *>(field_50_));
        field_50_ = 0;
        field_6C_ = 0;
        return 0;
    }

    return 7;
}
