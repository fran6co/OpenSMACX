
/* ---- source-owned scalar and shadow raster kernels ---- */
// Shared window types and parent globals are defined in caviar.h/caviar.cpp;
// fresh standalone CodeView/COFF resolves their v3 child paths, but not the
// original allocation bounds, lifetime, or final-linked placement.

// Seed provenance: build/recovery/next-game-coverage-oct5/product-closure-overlay-v6/src/caviar_scalar_kernel_candidate.inc
// Include after the Caviar renderer's real storage definitions. This code
// intentionally uses the current source owners (projection cells/output
// window, setup copies, active angle/palette and scene surfaces); the local
// struct is a non-owning invocation view, not another allocation or ABI type.
#include <float.h>
#include <limits.h>
#include <string.h>
#if !defined(_MSC_VER)
#include <fenv.h>
#endif
#include <math.h>


struct CaviarRasterKernelViewCandidate {
    const uint8_t *angle_rows_;
    const CaviarKernelStepRowCandidate *step_rows_;
    const int32_t *destination_row_offsets_;
    const uint8_t *colour_table_;
    uint8_t *colour_lane_base_[4];
    uint16_t *depth_lane_base_[4];
    int pixel_size_bytes_;
    int depth_greater_wins_;
    int shadow_target_;
    int clipped_;
    int clip_[4];
};

static uint32_t caviar_kernel_u32_add(uint32_t left, uint32_t right) {
    return left + right;
}

static int32_t caviar_kernel_i32_from_bits(uint32_t bits) {
    int32_t result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}

static int caviar_kernel_rounding_mode_candidate() {
#if defined(_MSC_VER)
    const unsigned int control = _controlfp(0, 0) & _MCW_RC;
    if (control == _RC_NEAR) return 0;
    if (control == _RC_DOWN) return 1;
    if (control == _RC_UP) return 2;
    if (control == _RC_CHOP) return 3;
    return -1;
#else
    const int mode = fegetround();
    if (mode == FE_TONEAREST) return 0;
    if (mode == FE_DOWNWARD) return 1;
    if (mode == FE_UPWARD) return 2;
    if (mode == FE_TOWARDZERO) return 3;
    return -1;
#endif
}

static int32_t caviar_kernel_fistp_dword_candidate(long double value) {
    // Native `fistp dword ptr` returns the integer-indefinite bit pattern for
    // masked NaN/out-of-range inputs. The selected x87 RC mode controls normal
    // rounding, including ties-to-even. Trap/status-bit fidelity remains an
    // explicit platform obligation; this helper preserves the numeric result.
    if (!(value == value))
        return caviar_kernel_i32_from_bits(0x80000000u);

    const int mode = caviar_kernel_rounding_mode_candidate();
    if (mode < 0) return caviar_kernel_i32_from_bits(0x80000000u);
    long double rounded;
    if (mode == 1) {
        rounded = floor(value);
    } else if (mode == 2) {
        rounded = ceil(value);
    } else if (mode == 3) {
        rounded = value < 0 ? ceil(value) : floor(value);
    } else {
        const long double lower = floor(value);
        const long double fraction = value - lower;
        if (fraction < 0.5L) rounded = lower;
        else if (fraction > 0.5L) rounded = lower + 1.0L;
        else rounded = fmod(fabs(lower), 2.0L) == 0.0L
            ? lower : lower + 1.0L;
    }
    if (rounded < -2147483648.0L || rounded >= 2147483648.0L)
        return caviar_kernel_i32_from_bits(0x80000000u);
    return static_cast<int32_t>(rounded);
}

static int32_t caviar_kernel_sar8(int32_t value) {
    // The original x86 SAR rounds negative fixed-point values toward -inf.
    uint32_t shifted = static_cast<uint32_t>(value) >> 8;
    if ((static_cast<uint32_t>(value) & 0x80000000u) != 0)
        shifted |= 0xff000000u;
    return caviar_kernel_i32_from_bits(shifted);
}

static void caviar_kernel_project_coordinate(const uint8_t *header,
                                              int32_t &x, int32_t &y,
                                              int32_t &z) {
    const float a = static_cast<float>(static_cast<int8_t>(header[5]));
    const float b = static_cast<float>(static_cast<int8_t>(header[6]));
    const float c = static_cast<float>(static_cast<int8_t>(header[7]));
    // Preserve x87's extended intermediate precision where the host provides
    // it, and apply the live rounding mode at each native FISTP destination.
    const long double x0 = static_cast<long double>(c) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_ec_;
    const long double x1 = static_cast<long double>(a) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_f0_;
    const long double x2 = static_cast<long double>(b) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_f4_;
    const long double y0 = static_cast<long double>(c) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_e0_;
    const long double y1 = static_cast<long double>(a) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_e4_;
    const long double y2 = static_cast<long double>(b) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_e8_;
    const long double z0 = static_cast<long double>(c) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_fc_;
    const long double z1 = static_cast<long double>(a) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_a00_;
    const long double z2 = static_cast<long double>(b) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_a04_;
    const int32_t projected_x = caviar_kernel_fistp_dword_candidate((x0 + x1) + x2);
    const int32_t projected_y = caviar_kernel_fistp_dword_candidate((y0 + y1) + y2);
    const int32_t projected_z = caviar_kernel_fistp_dword_candidate((z0 + z1) + z2);
    x = static_cast<int32_t>(caviar_kernel_u32_add(
        static_cast<uint32_t>(x), static_cast<uint32_t>(projected_x)) -
        static_cast<uint32_t>(CaviarKernelProjectionWindow.current_views_.cells_[0].x_));
    y = static_cast<int32_t>(caviar_kernel_u32_add(
        static_cast<uint32_t>(y), static_cast<uint32_t>(projected_y)) -
        static_cast<uint32_t>(CaviarKernelProjectionWindow.current_views_.cells_[0].y_));
    z = static_cast<int32_t>(caviar_kernel_u32_add(
        static_cast<uint32_t>(z), static_cast<uint32_t>(projected_z)) -
        static_cast<uint32_t>(CaviarKernelProjectionWindow.current_views_.cells_[0].z_));
}

static int caviar_kernel_clip_coordinate(int &x, int &y,
                                          const int clip[4]) {
    // 664000's two-stage bounds test and one-pixel edge repair. The first
    // stage's strict comparisons, second stage's inclusive comparisons, and
    // repair order are kept separate because they are not a conventional
    // rectangle clamp.
    if (x < clip[0] || x >= clip[2] || y < clip[1] || y >= clip[3]) {
        if (x > clip[2] || x < clip[0] - 1 ||
            y > clip[3] || y < clip[1] - 1)
            return 0;
        if (x >= clip[2]) --x;
        else if (x <= clip[0] - 1) ++x;
        if (y >= clip[3]) --y;
        else if (y <= clip[1] - 1) ++y;
    }
    return 1;
}

static void caviar_kernel_store_lane(CaviarRasterKernelViewCandidate *view,
                                     int lane, int pixel_index,
                                     uint16_t depth, uint16_t colour) {
    uint16_t *depth_at = view->depth_lane_base_[lane] + pixel_index;
    if (view->depth_greater_wins_) {
        if (depth <= *depth_at) return; // odd-alignment native JBE skip
    } else if (depth >= *depth_at) {
        return; // aligned native unsigned strict-less test
    }
    *depth_at = depth;
    uint8_t *colour_at = view->colour_lane_base_[lane] +
        pixel_index * view->pixel_size_bytes_;
    if (view->pixel_size_bytes_ == 1)
        *colour_at = static_cast<uint8_t>(colour);
    else
        *reinterpret_cast<uint16_t *>(colour_at) = colour;
}

static void caviar_kernel_draw_pixel(CaviarRasterKernelViewCandidate *view,
                                     int32_t &x_fixed, int32_t &y_fixed,
                                     int32_t &z_fixed,
                                     const uint8_t *pixel,
                                     uint32_t phase,
                                     const int clip[4], int flags) {
    const CaviarKernelStepRowCandidate &step = view->step_rows_[phase >> 3];
    x_fixed = static_cast<int32_t>(
        static_cast<uint32_t>(x_fixed) + static_cast<uint32_t>(step.x_));
    y_fixed = static_cast<int32_t>(
        static_cast<uint32_t>(y_fixed) + static_cast<uint32_t>(step.y_));
    z_fixed = static_cast<int32_t>(
        static_cast<uint32_t>(z_fixed) + static_cast<uint32_t>(step.z_));
    int screen_x = caviar_kernel_sar8(x_fixed);
    int screen_y = caviar_kernel_sar8(y_fixed);
    if (view->clipped_ && !caviar_kernel_clip_coordinate(
            screen_x, screen_y, clip))
        return;

    const uint16_t angle = static_cast<uint16_t>(
        ((static_cast<uint16_t>(pixel[0]) << 8) | pixel[1]) & 0x7ff);
    const uint8_t ramp = view->angle_rows_[angle];
    const uint16_t colour_index = static_cast<uint16_t>(
        (static_cast<uint16_t>(ramp) << 8) | pixel[2]);
    uint16_t colour;
    if (view->pixel_size_bytes_ == 1)
        colour = view->colour_table_[colour_index];
    else
        colour = reinterpret_cast<const uint16_t *>(view->colour_table_)[colour_index];

    const uint16_t candidate_depth = static_cast<uint16_t>(
        static_cast<uint32_t>(z_fixed) >> 16);
    uint32_t destination_offset_bits;
    if ((flags & 0x10) != 0) {
        // The angle-table branch uses native IMUL y,[setup+0x10] followed by
        // x; the count branch uses the patched ramp-B row-offset lookup.
        destination_offset_bits =
            static_cast<uint32_t>(screen_y) * vox_setup_copy_[4] +
            static_cast<uint32_t>(screen_x);
    } else {
        destination_offset_bits =
            static_cast<uint32_t>(view->destination_row_offsets_[screen_y]) +
            static_cast<uint32_t>(screen_x);
    }
    const int pixel_index =
        caviar_kernel_i32_from_bits(destination_offset_bits);
    const int lane_count = (flags & 1) == 0 ? 4 : 1;
    int i;
    for (i = 0; i != lane_count; ++i) {
        // Each native lane has a separately patched base operand. The setup
        // producers populate those four source views; all lanes share the
        // same row-table-adjusted pixel index and no second clip guard occurs.
        caviar_kernel_store_lane(view, i, pixel_index,
                                 candidate_depth, colour);
    }
}

static void caviar_kernel_walk_stream(CaviarRasterKernelViewCandidate *view,
                                      int32_t x, int32_t y, int32_t z,
                                      uint32_t raw_count,
                                      const uint8_t *stream, int flags,
                                      uint32_t class_mask,
                                      const int clip[4]) {
    const uint8_t *record = stream;
    uint32_t remaining = raw_count;
    if ((flags & 0x20) != 0 && (flags & 0x10) == 0)
        --remaining;

    for (;;) {
        const uint32_t selector_mask =
            *reinterpret_cast<const uint32_t *>(record) & class_mask;
        if (selector_mask == 0) {
            if ((record[8] & 0x80) != 0) return;
            record += 12 + 3 * record[4];
            continue;
        }

        const unsigned int selector = record[8] & 0x1f;
        const uint8_t *pixel = record + 9;
        int32_t part_x = x;
        int32_t part_y = y;
        int32_t part_z = caviar_kernel_i32_from_bits(
            static_cast<uint32_t>(z) << 8);
        if (selector < 0x1a) {
            if ((flags & 0x10) != 0) {
                const unsigned int word = 4 + selector * 4;
                part_x = static_cast<int32_t>(static_cast<uint32_t>(part_x) +
                    CaviarOutputMessageWindow.projection_words_[word]);
                part_y = static_cast<int32_t>(static_cast<uint32_t>(part_y) +
                    CaviarOutputMessageWindow.projection_words_[word + 1]);
                part_z = static_cast<int32_t>(static_cast<uint32_t>(part_z) +
                    CaviarOutputMessageWindow.projection_words_[word + 2]);
            } else {
                part_x = static_cast<int32_t>(static_cast<uint32_t>(part_x) +
                    static_cast<uint32_t>(CaviarKernelProjectionWindow.current_views_.cells_[selector].x_));
                part_y = static_cast<int32_t>(static_cast<uint32_t>(part_y) +
                    static_cast<uint32_t>(CaviarKernelProjectionWindow.current_views_.cells_[selector].y_));
                part_z = static_cast<int32_t>(static_cast<uint32_t>(part_z) +
                    static_cast<uint32_t>(CaviarKernelProjectionWindow.current_views_.cells_[selector].z_));
            }
        } else {
            caviar_kernel_project_coordinate(record, part_x, part_y, part_z);
        }

        for (;;) {
            const unsigned int end_step = pixel[2] & 0xf8;
            caviar_kernel_draw_pixel(view, part_x, part_y, part_z,
                                     pixel, end_step, clip, flags);
            pixel += 3;
            if ((flags & 0x10) == 0) {
                --remaining;
                if (remaining == 0) return;
            } else if (end_step == 0xd0) {
                return;
            } else if (end_step > 0xd0) {
                break;
            }
        }
        record = pixel;
    }
}

static CaviarRasterKernelViewCandidate caviar_make_kernel_view_candidate(
    int pixel_size, int shadow, int clipped, const uint8_t *angle_rows,
    const CaviarKernelStepRowCandidate *step_rows,
    const int32_t *destination_row_offsets,
    const uint8_t *colour_table, uint8_t *const colour_lane_base[4],
    uint16_t *const depth_lane_base[4],
    int depth_greater_wins, const int clip[4]) {
    CaviarRasterKernelViewCandidate view;
    view.angle_rows_ = angle_rows;
    view.step_rows_ = step_rows;
    view.destination_row_offsets_ = destination_row_offsets;
    view.colour_table_ = colour_table;
    int lane;
    for (lane = 0; lane != 4; ++lane) {
        view.colour_lane_base_[lane] = colour_lane_base[lane];
        view.depth_lane_base_[lane] = depth_lane_base[lane];
    }
    view.pixel_size_bytes_ = pixel_size;
    view.depth_greater_wins_ = depth_greater_wins;
    view.shadow_target_ = shadow;
    view.clipped_ = clipped;
    view.clip_[0] = clip[0]; view.clip_[1] = clip[1];
    view.clip_[2] = clip[2]; view.clip_[3] = clip[3];
    return view;
}

// This proposal captures the common scalar stream protocol. The mode-specific
// native entries remain separate source wrappers at integration: their exact
// selected scene surfaces and byte-vs-word palette width are passed here from
// the real setup state, not selected by address or an executable trampoline.
static void caviar_kernel_contained_scalar_candidate(
    int x, int y, int z, int count, const uint8_t *stream, int flags,
    int pixel_size, int shadow, const uint8_t *angle_rows,
    const CaviarKernelStepRowCandidate *step_rows,
    const int32_t *destination_row_offsets,
    const uint8_t *colour_table, uint8_t *const colour_lane_base[4],
    uint16_t *const depth_lane_base[4],
    int depth_greater_wins) {
    const int no_clip[4] = {0, 0, 0, 0};
    CaviarRasterKernelViewCandidate view = caviar_make_kernel_view_candidate(
        pixel_size, shadow, 0, angle_rows, step_rows, destination_row_offsets,
        colour_table, colour_lane_base, depth_lane_base,
        depth_greater_wins, no_clip);
    caviar_kernel_walk_stream(&view, x, y, z, static_cast<uint32_t>(count),
                              stream, flags, vox_projection_octant_mask_, no_clip);
}

static void caviar_kernel_clipped_scalar_candidate(
    int x, int y, int z, const int *rectangle, int count,
    const uint8_t *stream, int flags, int pixel_size, int shadow,
    const uint8_t *angle_rows,
    const CaviarKernelStepRowCandidate *step_rows,
    const int32_t *destination_row_offsets,
    const uint8_t *colour_table, uint8_t *const colour_lane_base[4],
    uint16_t *const depth_lane_base[4],
    int depth_greater_wins) {
    const int clip[4] = {rectangle[0], rectangle[1],
                         rectangle[2], rectangle[3]};
    CaviarRasterKernelViewCandidate view = caviar_make_kernel_view_candidate(
        pixel_size, shadow, 1, angle_rows, step_rows, destination_row_offsets,
        colour_table, colour_lane_base, depth_lane_base,
        depth_greater_wins, clip);
    caviar_kernel_walk_stream(&view, x, y, z, static_cast<uint32_t>(count),
                              stream, flags, vox_projection_octant_mask_, clip);
}


// Candidate contribution: build/recovery/next-game-coverage-oct5/product-closure-overlay-v6/src/caviar_shadow_kernel_candidate.inc
// Scalar C++ proposal for the shared 666FD0/666A80 shadow kernels. Include
// after caviar_kernel_storage_candidate.h. These callbacks are not part
// color/depth raster kernels: native 666A80 uses the active-size byte as its
// store value, makes no depth test and has no palette lookup.

struct CaviarShadowKernelViewCandidate {
    const CaviarKernelStepRowCandidate *step_rows_;
    const int32_t *destination_row_offsets_;
    uint8_t *shadow_lane_base_[4];
    uint8_t value_;
    int clipped_;
    int has_rectangle_;
    int rectangle_[4];
};

static int caviar_shadow_clip_repair_candidate(int &x, int &y,
                                                const int clip[4]) {
    if (x < clip[0] || x > clip[2] || y < clip[1] || y > clip[3]) {
        if (x > clip[2] || x < clip[0] - 1 ||
            y > clip[3] || y < clip[1] - 1)
            return 0;
        if (x >= clip[2]) --x;
        else if (x <= clip[0] - 1) ++x;
        if (y >= clip[3]) --y;
        else if (y <= clip[1] - 1) ++y;
    }
    return 1;
}

static int caviar_shadow_clip_inside_candidate(int x, int y,
                                                const int clip[4]) {
    return x >= clip[0] && x <= clip[2] &&
           y >= clip[1] && y <= clip[3];
}

static int caviar_shadow_clip_half_open_candidate(int x, int y,
                                                   const int clip[4]) {
    return x >= clip[0] && x < clip[2] &&
           y >= clip[1] && y < clip[3];
}

static void caviar_shadow_kernel_project_candidate(const uint8_t *header,
                                                    int32_t &x, int32_t &y) {
    const float a = static_cast<float>(static_cast<int8_t>(header[5]));
    const float b = static_cast<float>(static_cast<int8_t>(header[6]));
    const float c = static_cast<float>(static_cast<int8_t>(header[7]));
    const long double x0 = static_cast<long double>(c) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_ec_;
    const long double x1 = static_cast<long double>(a) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_f0_;
    const long double x2 = static_cast<long double>(b) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_f4_;
    const long double y0 = static_cast<long double>(c) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_e0_;
    const long double y1 = static_cast<long double>(a) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_e4_;
    const long double y2 = static_cast<long double>(b) * CaviarKernelProjectionWindow.current_views_.tail_.scratch_e8_;
    const int32_t projected_x =
        caviar_kernel_fistp_dword_candidate((x0 + x1) + x2);
    const int32_t projected_y =
        caviar_kernel_fistp_dword_candidate((y0 + y1) + y2);
    x = static_cast<int32_t>(static_cast<uint32_t>(x) +
                             static_cast<uint32_t>(projected_x) -
                             static_cast<uint32_t>(CaviarKernelProjectionWindow.current_views_.cells_[0].x_));
    y = static_cast<int32_t>(static_cast<uint32_t>(y) +
                             static_cast<uint32_t>(projected_y));
}

static void caviar_shadow_kernel_walk_candidate(
    CaviarShadowKernelViewCandidate *view, int x, int y, uint32_t raw_count,
    const uint8_t *stream, int flags, uint32_t selector_mask) {
    const uint8_t *record = stream;
    uint32_t remaining = raw_count;
    if ((flags & 0x10) == 0 && (flags & 0x20) != 0) --remaining;

    for (;;) {
        if ((*reinterpret_cast<const uint32_t *>(record) & selector_mask) == 0) {
            if ((record[8] & 0x80) != 0) return;
            record += 12 + 3 * record[4];
            continue;
        }

        const unsigned int selector = record[8] & 0x1f;
        const uint8_t *pixel = record + 9;
        int32_t part_x = x;
        int32_t part_y = y;
        if (selector < 0x1a) {
            part_x = static_cast<int32_t>(static_cast<uint32_t>(part_x) +
                                          static_cast<uint32_t>(CaviarKernelProjectionWindow.current_views_.cells_[selector].x_));
            part_y = static_cast<int32_t>(static_cast<uint32_t>(part_y) +
                                          static_cast<uint32_t>(CaviarKernelProjectionWindow.current_views_.cells_[selector].y_));
        } else {
            caviar_shadow_kernel_project_candidate(record, part_x, part_y);
        }

        for (;;) {
            const uint8_t phase = static_cast<uint8_t>(pixel[2] & 0xf8);
            const CaviarKernelStepRowCandidate &step = view->step_rows_[phase >> 3];
            part_x = static_cast<int32_t>(static_cast<uint32_t>(part_x) +
                                          static_cast<uint32_t>(step.x_));
            part_y = static_cast<int32_t>(static_cast<uint32_t>(part_y) +
                                          static_cast<uint32_t>(step.y_));
            int pixel_x = caviar_kernel_sar8(part_x);
            int pixel_y = caviar_kernel_sar8(part_y);
            int accepted = 1;
            if (view->clipped_ && view->has_rectangle_) {
                if ((flags & 0x10) != 0) {
                    accepted = caviar_shadow_clip_inside_candidate(
                        pixel_x, pixel_y, view->rectangle_);
                } else if ((flags & 1) != 0) {
                    accepted = caviar_shadow_clip_half_open_candidate(
                        pixel_x, pixel_y, view->rectangle_);
                } else {
                    accepted = caviar_shadow_clip_repair_candidate(
                        pixel_x, pixel_y, view->rectangle_);
                }
            }

            if (accepted) {
                const int stores = ((flags & 0x10) == 0 && (flags & 1) == 0)
                    ? 4 : 1;
                const int pixel_index = caviar_kernel_i32_from_bits(
                    static_cast<uint32_t>(
                        view->destination_row_offsets_[pixel_y]) +
                    static_cast<uint32_t>(pixel_x));
                int lane;
                for (lane = 0; lane != stores; ++lane) {
                    // The setup routine supplies separate byte destinations;
                    // the native loop shares one row-table-adjusted index.
                    view->shadow_lane_base_[lane][pixel_index] = view->value_;
                }
            }

            pixel += 3;
            if ((flags & 0x10) == 0) {
                --remaining;
                if (remaining == 0) return;
            } else if (phase == 0xd0) {
                return;
            } else if (phase > 0xd0) {
                break;
            }
        }
        record = pixel;
    }
}

static void caviar_shadow_contained_scalar_candidate(
    int x, int y, int count, const uint8_t *stream, int flags,
    const uint8_t *active_size, const CaviarKernelStepRowCandidate *step_rows,
    const int32_t *destination_row_offsets,
    uint8_t *const shadow_lane_base[4], uint32_t selector_mask) {
    CaviarShadowKernelViewCandidate view;
    view.step_rows_ = step_rows;
    view.destination_row_offsets_ = destination_row_offsets;
    int lane;
    for (lane = 0; lane != 4; ++lane)
        view.shadow_lane_base_[lane] = shadow_lane_base[lane];
    view.value_ = *active_size;
    view.clipped_ = 0;
    view.has_rectangle_ = 0;
    view.rectangle_[0] = view.rectangle_[1] = 0;
    view.rectangle_[2] = view.rectangle_[3] = 0;
    caviar_shadow_kernel_walk_candidate(&view, x, y,
        static_cast<uint32_t>(count), stream, flags, selector_mask);
}

static void caviar_shadow_clipped_scalar_candidate(
    int x, int y, const int *rectangle, int count, const uint8_t *stream,
    int flags, const uint8_t *active_size,
    const CaviarKernelStepRowCandidate *step_rows,
    const int32_t *destination_row_offsets,
    uint8_t *const shadow_lane_base[4], uint32_t selector_mask) {
    CaviarShadowKernelViewCandidate view;
    view.step_rows_ = step_rows;
    view.destination_row_offsets_ = destination_row_offsets;
    int lane;
    for (lane = 0; lane != 4; ++lane)
        view.shadow_lane_base_[lane] = shadow_lane_base[lane];
    view.value_ = *active_size;
    view.clipped_ = 1;
    view.has_rectangle_ = 1;
    view.rectangle_[0] = rectangle[0]; view.rectangle_[1] = rectangle[1];
    view.rectangle_[2] = rectangle[2]; view.rectangle_[3] = rectangle[3];
    caviar_shadow_kernel_walk_candidate(&view, x, y,
        static_cast<uint32_t>(count), stream, flags, selector_mask);
}


// Candidate contribution: build/recovery/next-game-coverage-oct5/product-closure-overlay-v6/src/caviar_product_kernel_bindings_candidate.inc
// Ignored C++ product-binding proposal. Include after caviar.h, Buffer,
// caviar_kernel_storage_candidate.h, the two scalar loop includes, and the
// existing Caviar renderer storage definitions. Every function below is an
// ordinary source callback; no pinned-code addresses or code-page patching are
// retained. Four color/depth bases are supplied independently to the loop.

// One process-wide derived view matches the original draw scratch lifetime:
// vox_draw_scene refreshes its globals and invokes the record's +0x7c setup
// slot before any part/shadow callback. It owns no allocations and is not a
// second Caviar scene/context object.
struct CaviarPreparedKernelBindingsCandidate {
    const CaviarKernelStepRowCandidate *step_rows_;
    const int32_t *row_offsets_;
    const uint8_t *angle_rows_;
    const uint8_t *colour_table_;
    const uint8_t *active_size_;
    uint8_t *colour_lane_base_[4];
    uint16_t *depth_lane_base_[4];
    uint8_t *shadow_lane_base_[4];
};
static CaviarPreparedKernelBindingsCandidate caviar_prepared_kernel_bindings_candidate;

static void caviar_part_output_bases_candidate(
    int pixel_size, uint8_t *colour_base[4], uint16_t *depth_base[4]) {
    uint8_t *const colour = static_cast<uint8_t *>(Caviar::render_color_dib_bits());
    uint16_t *const depth = reinterpret_cast<uint16_t *>(CaviarSceneMemory);
    // The setup records row width in pixels; size-2 kernels use SHL-by-one
    // before patching color row addresses, while size-1 uses it unchanged.
    const uint32_t colour_pitch =
        vox_setup_copy_[3] * static_cast<uint32_t>(pixel_size);
    const uint32_t depth_pitch = vox_setup_copy_[4];
    colour_base[0] = colour;
    colour_base[1] = colour + pixel_size;
    colour_base[2] = colour + colour_pitch;
    colour_base[3] = colour + colour_pitch + pixel_size;
    depth_base[0] = depth;
    depth_base[1] = depth + 1;
    depth_base[2] = depth + depth_pitch;
    depth_base[3] = depth + depth_pitch + 1;
}

static void caviar_shadow_output_bases_candidate(uint8_t *shadow_base[4]) {
    uint8_t *const shadow = static_cast<uint8_t *>(Caviar::render_shadow_dib_bits());
    const uint32_t pitch = vox_setup_copy_[5];
    shadow_base[0] = shadow;
    shadow_base[1] = shadow + 1;
    shadow_base[2] = shadow + pitch;
    shadow_base[3] = shadow + pitch + 1;
}

static const CaviarKernelStepRowCandidate *caviar_kernel_steps_candidate() {
    return CaviarKernelProjectionWindow.raster_step_rows_;
}

static const int32_t *caviar_kernel_row_offsets_candidate() {
    return static_cast<const int32_t *>(vox_active_ramp_b_);
}

static void caviar_prepare_kernel_bindings_candidate(int pixel_size) {
    caviar_prepared_kernel_bindings_candidate.step_rows_ =
        caviar_kernel_steps_candidate();
    caviar_prepared_kernel_bindings_candidate.row_offsets_ =
        caviar_kernel_row_offsets_candidate();
    caviar_prepared_kernel_bindings_candidate.angle_rows_ =
        vox_projection_selected_angle_;
    caviar_prepared_kernel_bindings_candidate.colour_table_ =
        static_cast<const uint8_t *>(vox_active_colour_table_);
    caviar_prepared_kernel_bindings_candidate.active_size_ =
        &vox_active_size_code_;
    caviar_part_output_bases_candidate(
        pixel_size,
        caviar_prepared_kernel_bindings_candidate.colour_lane_base_,
        caviar_prepared_kernel_bindings_candidate.depth_lane_base_);
    caviar_shadow_output_bases_candidate(
        caviar_prepared_kernel_bindings_candidate.shadow_lane_base_);
}

static void __cdecl caviar_setup_size1_aligned_candidate() {
    caviar_prepare_kernel_bindings_candidate(1);
}
static void __cdecl caviar_setup_size1_odd_candidate() {
    caviar_prepare_kernel_bindings_candidate(1);
}
static void __cdecl caviar_setup_size2_aligned_candidate() {
    caviar_prepare_kernel_bindings_candidate(2);
}
static void __cdecl caviar_setup_size2_odd_candidate() {
    caviar_prepare_kernel_bindings_candidate(2);
}

static void caviar_part_kernel_candidate(
    int pixel_size, int depth_greater_wins, int clipped,
    int32_t x, int32_t y, int32_t z, const int32_t *rectangle,
    uint32_t raw_count, const uint8_t *stream, int32_t flags) {
    CaviarPreparedKernelBindingsCandidate &bindings =
        caviar_prepared_kernel_bindings_candidate;
    if (clipped) {
        caviar_kernel_clipped_scalar_candidate(
            x, y, z, reinterpret_cast<const int *>(rectangle),
            caviar_kernel_i32_from_bits(raw_count), stream, flags, pixel_size, 0,
            bindings.angle_rows_, bindings.step_rows_, bindings.row_offsets_,
            bindings.colour_table_, bindings.colour_lane_base_,
            bindings.depth_lane_base_, depth_greater_wins);
    } else {
        caviar_kernel_contained_scalar_candidate(
            x, y, z, caviar_kernel_i32_from_bits(raw_count), stream, flags,
            pixel_size, 0, bindings.angle_rows_, bindings.step_rows_,
            bindings.row_offsets_, bindings.colour_table_,
            bindings.colour_lane_base_, bindings.depth_lane_base_,
            depth_greater_wins);
    }
}

// The wrappers deliberately fix all measured per-entry profile distinctions;
// dispatch does not infer alignment or pixel width from a guessed name.
static void __cdecl caviar_part_s1a_contained_candidate(
    int32_t x, int32_t y, int32_t z, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_kernel_candidate(1, 0, 0, x, y, z, 0, count, stream, flags);
}
static void __cdecl caviar_part_s1a_clipped_candidate(
    int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_kernel_candidate(1, 0, 1, x, y, z, rect, count, stream, flags);
}
static void __cdecl caviar_part_s1o_contained_candidate(
    int32_t x, int32_t y, int32_t z, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_kernel_candidate(1, 1, 0, x, y, z, 0, count, stream, flags);
}
static void __cdecl caviar_part_s1o_clipped_candidate(
    int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_kernel_candidate(1, 1, 1, x, y, z, rect, count, stream, flags);
}
static void __cdecl caviar_part_s2a_contained_candidate(
    int32_t x, int32_t y, int32_t z, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_kernel_candidate(2, 0, 0, x, y, z, 0, count, stream, flags);
}
static void __cdecl caviar_part_s2a_clipped_candidate(
    int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_kernel_candidate(2, 0, 1, x, y, z, rect, count, stream, flags);
}
static void __cdecl caviar_part_s2o_contained_candidate(
    int32_t x, int32_t y, int32_t z, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_kernel_candidate(2, 1, 0, x, y, z, 0, count, stream, flags);
}
static void __cdecl caviar_part_s2o_clipped_candidate(
    int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_kernel_candidate(2, 1, 1, x, y, z, rect, count, stream, flags);
}

static void caviar_shadow_kernel_candidate(
    int clipped, int32_t x, int32_t y, const int32_t *rectangle,
    uint32_t raw_count, const uint8_t *stream, int32_t flags) {
    CaviarPreparedKernelBindingsCandidate &bindings =
        caviar_prepared_kernel_bindings_candidate;
    if (clipped) {
        caviar_shadow_clipped_scalar_candidate(
            x, y, reinterpret_cast<const int *>(rectangle),
            caviar_kernel_i32_from_bits(raw_count), stream, flags,
            bindings.active_size_, bindings.step_rows_, bindings.row_offsets_,
            bindings.shadow_lane_base_, vox_projection_octant_mask_);
    } else {
        caviar_shadow_contained_scalar_candidate(
            x, y, caviar_kernel_i32_from_bits(raw_count), stream, flags,
            bindings.active_size_, bindings.step_rows_, bindings.row_offsets_,
            bindings.shadow_lane_base_, vox_projection_octant_mask_);
    }
}

static void __cdecl caviar_shadow_clipped_callback_candidate(
    int32_t x, int32_t y, const int32_t *rectangle, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_shadow_kernel_candidate(1, x, y, rectangle, count, stream, flags);
}

static void caviar_install_record_kernels_candidate(
    VoxRenderRecord *record, uint8_t size_code, int odd) {
    VoxRenderRecord *const slots = record;
    slots->shadow_contained_ = &caviar_original_666fd0;
    slots->shadow_clipped_ = &caviar_original_666a80;

    if (size_code == 1) {
        if (odd) {
            slots->part_contained_ = &caviar_original_665640;
            slots->part_clipped_ = &caviar_original_664e50;
            slots->setup_callback_ = &caviar_setup_size1_odd_candidate;
        } else {
            slots->part_contained_ = &caviar_original_6647f0;
            slots->part_clipped_ = &caviar_original_664000;
            slots->setup_callback_ = &caviar_setup_size1_aligned_candidate;
        }
    } else if (size_code == 2) {
        if (odd) {
            slots->part_contained_ = &caviar_original_667b20;
            slots->part_clipped_ = &caviar_original_667380;
            slots->setup_callback_ = &caviar_setup_size2_odd_candidate;
        } else {
            slots->part_contained_ = &caviar_original_666440;
            slots->part_clipped_ = &caviar_original_665ca0;
            slots->setup_callback_ = &caviar_setup_size2_aligned_candidate;
        }
    }
}

// Candidate contribution: build/recovery/next-game-coverage-oct5/product-closure-overlay-v6/src/caviar_draw_callbacks_candidate.inc
// Candidate complete bodies for 0x63E950 and 0x63F000. These are distinct
// render protocols: ordinary parts carry three projected coordinates and use
// slots +0x6C/+0x70; shadows carry two and use +0x74/+0x78. The callback view
// is a typed non-virtual slot owner over the existing record bytes.
extern int32_t vox_projection_prepared_;
extern const float CaviarProjectionDirectionScale;
extern const float CaviarProjectionRoundThreshold;
extern const float CaviarProjectionRoundBelow;
extern const float CaviarProjectionRoundAbove;
void __cdecl sub_63fa80(float *);
void __cdecl caviar_direction_to_projection_index(
    const VOX_Matrix *, const VOX_Vect *, float, const VOX_Matrix *);
void __cdecl sub_6401b0(const int16_t *, const int16_t *, int *);
void __cdecl caviar_build_projection_tables();
void __cdecl vox_direction_to_octant(const VoxRenderRecord *);
void __cdecl vox_inverse_direction_to_octant(const VOX_Matrix *);
static int caviar_box_is_inside(const int box[6], const int clip[4]);

static int caviar_fistp_qword_low_dword(float value) {
    // Source-level float-to-int conversion intentionally delegates to the
    // selected compiler's ordinary _ftol lowering, which must be measured
    // against 0x6458C8. It is not evidence for out-of-range, NaN, status,
    // precision, or unmasked-trap equivalence.
    return static_cast<int>(value);
}

static int caviar_round_projection_coordinate(float value) {
    const float scaled = value * CaviarProjectionDirectionScale;
    const float adjustment = scaled < CaviarProjectionRoundThreshold
        ? CaviarProjectionRoundBelow : CaviarProjectionRoundAbove;
    return caviar_fistp_qword_low_dword(scaled + adjustment);
}

static int caviar_truncate_projection_coordinate(float value) {
    return caviar_fistp_qword_low_dword(value);
}

static void caviar_project_raster_point(const VOX_Vect *position,
                                        const VOX_Matrix *part_matrix,
                                        const float point[3],
                                        int callback_output[3],
                                        int bounds_offset[2]) {
    float transformed[3];
    float translated[3];
    vox_matrix_vector_mul(part_matrix->values, point, transformed);
    vector_add(position->values, transformed, translated);
    // The native callback tuple and the box offset are different streams:
    // callback x is scaled transformed-x, callback y/z are scaled translated
    // y/z, while the box receives unscaled translated x/y.
    callback_output[0] = caviar_round_projection_coordinate(transformed[0]);
    callback_output[1] = caviar_round_projection_coordinate(translated[1]);
    callback_output[2] = caviar_round_projection_coordinate(translated[2]);
    bounds_offset[0] = caviar_truncate_projection_coordinate(translated[0]);
    bounds_offset[1] = caviar_truncate_projection_coordinate(translated[1]);
}

static int caviar_intersect_raster_box(int box[6], const int clip[4]) {
    return clip[0] <= box[2] && clip[1] <= box[3] &&
           box[0] <= clip[2] && box[1] <= clip[3];
}

static void caviar_clip_record_box(const VoxRenderRecord *record,
                                   const CaviarRenderClipWindowCandidate *scene_clip,
                                   int output[4]) {
    output[0] = record->draw_left_;
    output[1] = record->draw_top_;
    output[2] = record->draw_right_;
    output[3] = record->draw_bottom_;
    if (scene_clip->width_ != 0 && scene_clip->height_ != 0) {
        if (output[0] < scene_clip->left_) output[0] = scene_clip->left_;
        if (output[1] < scene_clip->top_) output[1] = scene_clip->top_;
        if (scene_clip->right_ < output[2]) output[2] = scene_clip->right_;
        if (scene_clip->bottom_ < output[3]) output[3] = scene_clip->bottom_;
    }
}

static int caviar_flat_raster_box(const CvrShape *shape,
                                  const VOX_Vect *position,
                                  const VOX_Matrix *part_matrix,
                                  int box[6]) {
    const float anchor[3] = {
        static_cast<float>(shape->raster_origin_[0]),
        static_cast<float>(shape->raster_origin_[1]),
        static_cast<float>(shape->raster_origin_[2])
    };
    int projected[3];
    int box_offset[2];
    caviar_project_raster_point(position, part_matrix, anchor, projected,
                                box_offset);
    sub_6401b0(&shape->offset_x_,
               &shape->axis_x_, box);
    if (box[4] != 0 && box[5] != 0) {
        box[0] += box_offset[0]; box[2] += box_offset[0];
        box[1] += box_offset[1]; box[3] += box_offset[1];
    }
    return 1;
}

static int caviar_raster_item_box(const CvrRasterItemHeaderCandidate *item,
                                 const VOX_Vect *position,
                                 const VOX_Matrix *part_matrix,
                                 int box[6], int projected[3],
                                 int box_offset[2]) {
    float point[3] = {
        static_cast<float>(static_cast<int>(item->endpoint_[0]) + item->base_[0]),
        static_cast<float>(static_cast<int>(item->endpoint_[1]) + item->base_[1]),
        static_cast<float>(static_cast<int>(item->endpoint_[2]) + item->base_[2])
    };
    caviar_project_raster_point(position, part_matrix, point, projected,
                                box_offset);
    int16_t delta[3];
    delta[0] = static_cast<int16_t>(item->endpoint_[3] - item->base_[0]);
    delta[1] = static_cast<int16_t>(item->endpoint_[4] - item->base_[1]);
    delta[2] = static_cast<int16_t>(item->endpoint_[5] - item->base_[2]);
    sub_6401b0(delta, item->extent_, box);
    return 1;
}

static void caviar_apply_projected_offset(int box[6], const int offset[2]) {
    box[0] += offset[0]; box[2] += offset[0];
    box[1] += offset[1]; box[3] += offset[1];
}

static int caviar_box_is_inside(const int box[6], const int clip[4]) {
    return clip[0] <= box[0] && clip[1] <= box[1] &&
           box[2] <= clip[2] && box[3] <= clip[3];
}

// 0x63E950: args are (record, shape, position, part matrix, camera rotation,
// state direction, shape scale and visibility). The last pair is intentionally
// not a pair of clip rectangles.
void __cdecl caviar_draw_part_candidate(
    VoxRenderRecord *record, const CvrShape *shape,
    const VOX_Vect *position, VOX_Matrix *part_matrix,
    VOX_Matrix *camera_rotation, const VOX_Vect *state_direction,
    float shape_scale, int visible) {
    CaviarSceneProjectionScratchCandidate &scratch = CaviarSceneProjectionScratch;
    int clip[4];
    int item_box[6];
    int flags = visible != 0;

    sub_63fa80(&part_matrix->values[0][0]);
    caviar_direction_to_projection_index(part_matrix, state_direction,
                                         shape_scale, camera_rotation);
    caviar_clip_record_box(record, &scratch.part_clip_, clip);
    caviar_build_projection_tables();
    vox_projection_prepared_ = 0;

    if ((shape->field_0_ & 2) == 0) {
        int projected[3];
        caviar_flat_raster_box(shape, position, part_matrix, item_box);
        if ((shape->field_0_ & 4) != 0) {
            projected[0] = caviar_round_projection_coordinate(position->values[0]);
            projected[1] = caviar_round_projection_coordinate(position->values[1]);
            projected[2] = caviar_round_projection_coordinate(position->values[2]);
            vox_direction_to_octant(record);
            flags |= 0x10;
        } else {
            const float anchor[3] = {
                static_cast<float>(shape->raster_origin_[0]),
                static_cast<float>(shape->raster_origin_[1]),
                static_cast<float>(shape->raster_origin_[2])
            };
            int ignored_offset[2];
            caviar_project_raster_point(position, part_matrix, anchor, projected,
                                        ignored_offset);
        }
        if (!caviar_intersect_raster_box(item_box, clip)) return;
        const int contained = caviar_box_is_inside(item_box, clip);
        if ((shape->field_0_ & 8) != 0) flags |= 0x20;
        VoxRenderRecord *dispatch =
            record;
        if (contained) {
            dispatch->part_contained_(projected[0], projected[1], projected[2],
                static_cast<int>(shape->item_count_), shape->item_stream_, flags);
        } else {
            dispatch->part_clipped_(projected[0], projected[1], projected[2],
                clip, static_cast<int>(shape->item_count_), shape->item_stream_, flags);
        }
        return;
    }

    uint32_t visited_items = 0;
    const CvrRasterItemHeaderCandidate *item =
        reinterpret_cast<const CvrRasterItemHeaderCandidate *>(shape->item_stream_);
    if (shape->item_count_ == 0) return;
    do {
        int projected[3];
        int box_offset[2];
        caviar_raster_item_box(item, position, part_matrix, item_box,
                               projected, box_offset);
        if (item_box[4] != 0 && item_box[5] != 0)
            caviar_apply_projected_offset(item_box, box_offset);
        if ((item->flags_ & 1) == 0) {
            flags &= ~0x10;
        } else {
            if (vox_projection_prepared_ == 0) vox_direction_to_octant(record);
            flags |= 0x10;
        }
        if (caviar_intersect_raster_box(item_box, clip)) {
            const int contained = caviar_box_is_inside(item_box, clip);
            if ((shape->field_0_ & 8) != 0)
                flags |= 0x20;
            else
                flags &= ~0x20;
            VoxRenderRecord *dispatch =
                record;
            if (contained) {
                dispatch->part_contained_(projected[0], projected[1], projected[2],
                    static_cast<int>(item->item_count_), item->inline_payload_, flags);
            } else {
                dispatch->part_clipped_(projected[0], projected[1], projected[2],
                    clip, static_cast<int>(item->item_count_), item->inline_payload_, flags);
            }
        }
        visited_items += item->item_count_;
        const uint32_t extent = (item->flags_ & 1) == 0
            ? item->item_count_ * 3u
            : *reinterpret_cast<const uint32_t *>(item->inline_payload_);
        item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
            item->inline_payload_ + extent);
    } while (visited_items < shape->item_count_);
}

// 0x63F000: the shadow callback ABI has two coordinates and its final pointer
// argument is only tested for nonzero by the original body.
void __cdecl caviar_draw_shadow_candidate(
    VoxRenderRecord *record, const CvrShape *shape,
    const VOX_Vect *position, VOX_Matrix *part_matrix,
    const void *camera_nonnull) {
    CaviarSceneProjectionScratchCandidate &scratch = CaviarSceneProjectionScratch;
    int clip[4];
    int flags = camera_nonnull != 0;
    int item_box[6];

    sub_63fa80(&part_matrix->values[0][0]);
    caviar_clip_record_box(record, &scratch.shadow_clip_, clip);
    caviar_build_projection_tables();
    vox_projection_prepared_ = 0;

    if ((shape->field_0_ & 2) == 0) {
        int projected[3] = {0, 0, 0};
        if ((shape->field_0_ & 4) != 0) {
            projected[0] = caviar_round_projection_coordinate(position->values[0]);
            projected[1] = caviar_round_projection_coordinate(position->values[1]);
            vox_inverse_direction_to_octant(part_matrix);
            flags |= 0x10;
        } else {
            const float anchor[3] = {
                static_cast<float>(shape->raster_origin_[0]),
                static_cast<float>(shape->raster_origin_[1]),
                static_cast<float>(shape->raster_origin_[2])
            };
            int ignored_offset[2];
            caviar_project_raster_point(position, part_matrix, anchor, projected,
                                        ignored_offset);
        }
        caviar_flat_raster_box(shape, position, part_matrix, item_box);
        if (!caviar_intersect_raster_box(item_box, clip)) return;
        const int contained = caviar_box_is_inside(item_box, clip);
        if ((shape->field_0_ & 8) != 0)
            flags |= 0x20;
        else
            flags &= ~0x20;
        VoxRenderRecord *dispatch =
            record;
        if (contained) {
            dispatch->shadow_contained_(projected[0], projected[1],
                static_cast<int>(shape->item_count_), shape->item_stream_, flags);
        } else {
            dispatch->shadow_clipped_(projected[0], projected[1], clip,
                static_cast<int>(shape->item_count_), shape->item_stream_, flags);
        }
        return;
    }

    uint32_t visited_items = 0;
    const CvrRasterItemHeaderCandidate *item =
        reinterpret_cast<const CvrRasterItemHeaderCandidate *>(shape->item_stream_);
    if (shape->item_count_ == 0) return;
    do {
        int projected[3];
        int box_offset[2];
        caviar_raster_item_box(item, position, part_matrix, item_box,
                               projected, box_offset);
        if (item_box[4] != 0 && item_box[5] != 0)
            caviar_apply_projected_offset(item_box, box_offset);
        if ((item->flags_ & 1) == 0) {
            flags &= ~0x10;
        } else {
            if (vox_projection_prepared_ == 0)
                vox_inverse_direction_to_octant(part_matrix);
            flags |= 0x10;
        }
        if (caviar_intersect_raster_box(item_box, clip)) {
            const int contained = caviar_box_is_inside(item_box, clip);
            if ((shape->field_0_ & 8) != 0)
                flags |= 0x20;
            else
                flags &= ~0x20;
            VoxRenderRecord *dispatch =
                record;
            if (contained) {
                dispatch->shadow_contained_(projected[0], projected[1],
                    static_cast<int>(item->item_count_), item->inline_payload_, flags);
            } else {
                dispatch->shadow_clipped_(projected[0], projected[1], clip,
                    static_cast<int>(item->item_count_), item->inline_payload_, flags);
            }
        }
        visited_items += item->item_count_;
        const uint32_t extent = (item->flags_ & 1) == 0
            ? item->item_count_ * 3u
            : *reinterpret_cast<const uint32_t *>(item->inline_payload_);
        item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
            item->inline_payload_ + extent);
    } while (visited_items < shape->item_count_);
}
