// ORIGINAL: 0x005DA4D0
// RULED-OUT: MISMATCH #6 mov/push - get_data()/free_data() logic inlined by
//            hand (raw uint32_t offsets, matching Buffer::get_data's own
//            style, as in the sibling change_color at 0x005DE580) since
//            neither is a catalogued callee of this address; register
//            allocation differs from the original past that point.
// name      ?map_colors@Buffer@@QAEHPAURECT@@PAE@Z
// size      429 bytes
// spans     0x005DA4D0-0x005DA67D
// prototype int (__thiscall ?map_colors@Buffer@@QAEHPAURECT@@PAE@Z)(Buffer* this, RECT*, unsigned int8*)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005DA530 0x005DA5B5 0x005DA666
// To start: tools/decomp_status.py --work 0x005DA4D0

int Buffer::map_colors(RECT * a1, unsigned char * a2) {
    volatile uint32_t *const ordered = reinterpret_cast<volatile uint32_t *>(this);

    if (a1 == 0) {
        return 0x10;
    }
    if (a2 == 0) {
        return 0x10;
    }

    RECT rect;
    rect.left = a1->left;
    rect.top = a1->top;
    rect.right = a1->right - 1;
    rect.bottom = a1->bottom - 1;

    if (!(*reinterpret_cast<int (__stdcall *const *)(RECT *, RECT *, RECT *)>(g_00669338))(
            &rect, &rect, &rect1_)) {
        return 0;
    }

    int32_t row_count = rect.bottom - rect.top;
    int32_t row_width = rect.right - rect.left;
    int32_t stride_remainder = static_cast<int32_t>(ordered[0x4A8 / 4]) - row_width;

    if (rect.left < static_cast<long>(width_) && rect.top < -static_cast<long>(height_)) {
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
            unsigned char *dest = reinterpret_cast<unsigned char *>(
                static_cast<int32_t>(ordered[0x4A8 / 4]) * rect.top + pixel_base + rect.left);
            if (dest != 0) {
                unsigned char *p = dest;
                int32_t y = row_count;
                do {
                    int32_t x = row_width;
                    do {
                        *p = a2[*p];
                        ++p;
                        --x;
                    } while (x != 0);
                    p += stride_remainder;
                    --y;
                } while (y != 0);

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
    }
    return 7;
}
