// ORIGINAL: 0x005DE580 ?change_color@Buffer@@QAEHHH@Z 0x005DE580-0x005DE721
// TRIED: MISMATCH #7 xor/mov - get_data()/free_data() logic inlined by hand (raw uint32_t offsets, matching Buffer::get_data's own style) rather than calling the real methods, since neither is a catalogued callee of this address; register allocation differs from the original past that point.
// size      417 bytes
// prototype int (__thiscall ?change_color@Buffer@@QAEHHH@Z)(Buffer* this, int, int)
// callers   12   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005DE5D3 0x005DE642 0x005DE70A
// To start: tools/decomp_status.py --work 0x005DE580

int Buffer::change_color(int a1, int a2) {
    volatile uint32_t *const ordered = reinterpret_cast<volatile uint32_t *>(this);

    RECT rect;
    rect.right = static_cast<long>(width_);
    rect.bottom = -static_cast<long>(height_);
    if (ordered[0x54 / 4] == 0 && ordered[0x58 / 4] == 0) {
        return 7;
    }
    rect.left = 0;
    rect.top = 0;
    if (!(*reinterpret_cast<int (__stdcall *const *)(RECT *, RECT *, RECT *)>(g_00669338))(
            &rect, &rect1_, &rect)) {
        return 0;
    }
    if (rect.left >= static_cast<long>(width_) || rect.top >= -static_cast<long>(height_)) {
        return 7;
    }

    int pixel_base;
    void *const surface = reinterpret_cast<void *>(ordered[0x58 / 4]);
    if (!surface) {
        const uint32_t storage = ordered[0x54 / 4];
        ordered[0x50 / 4] = storage;
        if (!storage) {
            return 7;
        }
        ordered[0x6C / 4] = ordered[0x6C / 4] + 1;
        pixel_base = static_cast<int>(storage);
    } else {
        const uint32_t existing = ordered[0x50 / 4];
        if (existing) {
            ordered[0x6C / 4] = ordered[0x6C / 4] + 1;
            pixel_base = static_cast<int>(existing);
        } else {
            uint8_t descriptor[0x6C];
            *reinterpret_cast<uint32_t *>(descriptor) = 0x6C;
            void **const vtable = *reinterpret_cast<void ***>(surface);
            long (__stdcall *const lock)(void *, void *, void *, uint32_t, void *) =
                reinterpret_cast<long (__stdcall *)(void *, void *, void *, uint32_t, void *)>(
                    vtable[0x64 / 4]);
            if (lock(surface, 0, descriptor, 1, 0) != 0) {
                return 7;
            }
            ordered[0x6C / 4] = ordered[0x6C / 4] + 1;
            uint32_t pitch = *reinterpret_cast<uint32_t *>(descriptor + 0x10);
            uint32_t data = *reinterpret_cast<uint32_t *>(descriptor + 0x24);
            ordered[0x4A8 / 4] = pitch;
            ordered[0x50 / 4] = data;
            pixel_base = static_cast<int>(data);
        }
    }

    if (pixel_base != 0) {
        char *dest = reinterpret_cast<char *>(
            static_cast<int32_t>(ordered[0x4A8 / 4]) * rect.top + pixel_base + rect.left);
        if (dest != 0) {
            int32_t row_width = rect.right - rect.left;
            int32_t row_count = rect.bottom - rect.top;
            int32_t stride_remainder = static_cast<int32_t>(ordered[0x4A8 / 4]) - row_width;
            char *p = dest;
            for (int32_t y = row_count; y != 0; --y) {
                for (int32_t x = row_width; x != 0; --x) {
                    if (*p == static_cast<char>(a1)) {
                        *p = static_cast<char>(a2);
                    }
                    ++p;
                }
                p += stride_remainder;
            }

            void *const surface2 = reinterpret_cast<void *>(ordered[0x58 / 4]);
            const int32_t remaining = static_cast<int32_t>(ordered[0x6C / 4]) - 1;
            ordered[0x6C / 4] = static_cast<uint32_t>(remaining);
            if (!surface2) {
                if (remaining <= 0) {
                    ordered[0x50 / 4] = 0;
                    ordered[0x6C / 4] = 0;
                }
                return 0;
            }
            const uint32_t data2 = ordered[0x50 / 4];
            if (data2 != 0 && remaining <= 0) {
                void **const vtable2 = *reinterpret_cast<void ***>(surface2);
                long (__stdcall *const unlock)(void *, void *) =
                    reinterpret_cast<long (__stdcall *)(void *, void *)>(vtable2[0x80 / 4]);
                unlock(surface2, reinterpret_cast<void *>(data2));
                ordered[0x50 / 4] = 0;
                ordered[0x6C / 4] = 0;
            }
            return 0;
        }
    }
    return 7;
}
